// ==WindhawkMod==
// @id              this-pc-used-space
// @name            This PC Used Space
// @description     Adds "Used Space" to the drive details in "This PC" view in File Explorer.
// @version         0.0.1
// @author          Gemini
// @include         explorer.exe
// @compilerOptions -lcomctl32 -lole32 -luuid -luser32 -lgdi32 -luxtheme -lshlwapi
// ==/WindhawkMod==

// ==WindhawkModSettings==
/*
- formatString: "%s free | %s used of %s"
  $name: Display Format
  $description: Use %s for Free Space, %s for Used Space, and %s for Total Space (e.g., "%s free | %s used of %s")
- boldUsed: true
  $name: Bold Used Space
  $description: Use bold-looking Unicode characters for the used space value
- boldStyle: sans-serif
  $name: Bold Style
  $description: Choose the visual style for the bold characters
  $options:
    - serif: Serif Bold (Classic)
    - sans-serif: Sans-Serif Bold (Modern)
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <windhawk_api.h>
#include <string>
#include <vector>
#include <cwchar>
#include <uxtheme.h>
#include <shlwapi.h>
#include <algorithm>

// We need to hook the function that formats the "XX GB free of YY GB" string.
// In modern Windows, this is often handled by shell32.dll or windows.storage.dll
// specifically when formatting the tile text for drives in the "This PC" folder view.

// A common approach for this in Windhawk is to hook the text drawing or item formatting
// functions of the SysListView32 or DirectUIHWND that renders the "This PC" view.
// However, a cleaner and more robust way is to hook the function that actually
// retrieves the free space string for the drive item.

// Let's start by hooking ExtTextOutW as a broad net to find where the string is drawn,
// then we can replace it.

typedef BOOL(WINAPI* ExtTextOutW_t)(HDC hdc, int x, int y, UINT options, const RECT* lprect, LPCWSTR lpString, UINT c, const INT* lpDx);
ExtTextOutW_t ExtTextOutW_Orig;

typedef int (WINAPI *DrawTextW_t)(HDC hdc, LPCWSTR lpchText, int cchText, LPRECT lprc, UINT format);
DrawTextW_t DrawTextW_Orig;

typedef HRESULT (WINAPI *DrawThemeTextEx_t)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int cchText, DWORD dwTextFlags, LPRECT pRect, const DTTOPTS *pOptions);
DrawThemeTextEx_t DrawThemeTextEx_Orig;

typedef HRESULT (WINAPI *DrawThemeText_t)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int cchText, DWORD dwTextFlags, DWORD dwTextFlags2, LPRECT pRect);
DrawThemeText_t DrawThemeText_Orig;

typedef int (WINAPI *DrawTextExW_t)(HDC hdc, LPWSTR lpchText, int cchText, LPRECT lprc, UINT format, LPDRAWTEXTPARAMS lpdtp);
DrawTextExW_t DrawTextExW_Orig;

std::wstring g_formatString = L"%s free | %s used of %s";
bool g_boldUsed = true;

enum class BoldStyle {
    Serif,
    SansSerif
};
BoldStyle g_boldStyle = BoldStyle::SansSerif;

void LoadSettings() {
    PCWSTR format = Wh_GetStringSetting(L"formatString");
    if (format) {
        g_formatString = format;
        Wh_FreeStringSetting(format);
    }

    g_boldUsed = Wh_GetIntSetting(L"boldUsed");

    PCWSTR style = Wh_GetStringSetting(L"boldStyle");
    if (style) {
        if (wcscmp(style, L"serif") == 0) g_boldStyle = BoldStyle::Serif;
        else g_boldStyle = BoldStyle::SansSerif;
        Wh_FreeStringSetting(style);
    }
}

// Helper to format byte sizes (e.g., 1024 -> "1.00 KB")
std::wstring FormatByteSize(ULONGLONG bytes) {
    wchar_t buffer[64];
    StrFormatByteSizeW(bytes, buffer, ARRAYSIZE(buffer));
    return std::wstring(buffer);
}

// Helper to clean strings before parsing.
// Removes non-printable characters and formatting marks like LRM/RLM.
std::wstring CleanNumericString(const std::wstring& s) {
    std::wstring result;
    bool foundStart = false;
    for (wchar_t c : s) {
        if (!foundStart) {
            if (iswdigit(c) || c == L'.' || c == L'-') {
                foundStart = true;
                result += c;
            }
            continue;
        }
        if (c == L',') continue; // Skip thousands separators
        result += c;
    }
    return result;
}

double GetUnitMultiplier(const wchar_t* unit) {
    if (wcsstr(unit, L"KB")) return 1024.0;
    if (wcsstr(unit, L"MB")) return 1024.0 * 1024.0;
    if (wcsstr(unit, L"GB")) return 1024.0 * 1024.0 * 1024.0;
    if (wcsstr(unit, L"TB")) return 1024.0 * 1024.0 * 1024.0 * 1024.0;
    return 1.0;
}

// Helper to make text look bold using Unicode Mathematical Alphanumeric Symbols.
std::wstring MakeBold(const std::wstring& s) {
    std::wstring result;
    for (wchar_t c : s) {
        if (c >= L'0' && c <= L'9') {
            result += (wchar_t)0xD835;
            if (g_boldStyle == BoldStyle::Serif)
                result += (wchar_t)(0xDFCE + (c - L'0'));
            else
                result += (wchar_t)(0xDFEC + (c - L'0'));
        } else if (c >= L'A' && c <= L'Z') {
            result += (wchar_t)0xD835;
            if (g_boldStyle == BoldStyle::Serif)
                result += (wchar_t)(0xDC00 + (c - L'A'));
            else
                result += (wchar_t)(0xDDD4 + (c - L'A'));
        } else if (c >= L'a' && c <= L'z') {
            result += (wchar_t)0xD835;
            if (g_boldStyle == BoldStyle::Serif)
                result += (wchar_t)(0xDC1A + (c - L'a'));
            else
                result += (wchar_t)(0xDDEE + (c - L'a'));
        } else {
            result += c;
        }
    }
    return result;
}

BOOL WINAPI ExtTextOutW_Hook(HDC hdc, int x, int y, UINT options, const RECT* lprect, LPCWSTR lpString, UINT c, const INT* lpDx) {
    if (!lpString || c == 0) return ExtTextOutW_Orig(hdc, x, y, options, lprect, lpString, c, lpDx);

    std::wstring text(lpString, c);

    // Look for the typical " free of " string pattern in English.
    // Note: This is highly localization-dependent. A better approach is needed for non-English systems.
    size_t freeOfPos = text.find(L" free of ");
    if (freeOfPos != std::wstring::npos) {
        // Prevent double processing if we already formatted it
        if (text.find(L" used of ") != std::wstring::npos) {
            return ExtTextOutW_Orig(hdc, x, y, options, lprect, lpString, c, lpDx);
        }

        std::wstring freeSpaceStr = text.substr(0, freeOfPos);
        std::wstring totalSpaceStr = text.substr(freeOfPos + 9); // length of " free of "
        
        std::wstring newText = text; // Default to original text
        
        // Clean strings for easier parsing
        std::wstring cleanFree = CleanNumericString(freeSpaceStr);
        std::wstring cleanTotal = CleanNumericString(totalSpaceStr);
        
        double freeVal = 0, totalVal = 0;
        wchar_t freeUnit[16] = {0}, totalUnit[16] = {0};
        
        int parsedFree = swscanf(cleanFree.c_str(), L"%lf %15s", &freeVal, freeUnit);
        int parsedTotal = swscanf(cleanTotal.c_str(), L"%lf %15s", &totalVal, totalUnit);
        
        if (parsedFree == 2 && parsedTotal == 2) {
            double freeBytes = freeVal * GetUnitMultiplier(freeUnit);
            double totalBytes = totalVal * GetUnitMultiplier(totalUnit);
            double usedBytes = totalBytes - freeBytes;
            if (usedBytes < 0) usedBytes = 0;

            std::wstring usedWStr = FormatByteSize((ULONGLONG)usedBytes);
            if (g_boldUsed) {
                usedWStr = MakeBold(usedWStr);
            }

            wchar_t newBuffer[256];
            swprintf(newBuffer, ARRAYSIZE(newBuffer), g_formatString.c_str(), 
                     freeSpaceStr.c_str(),
                     usedWStr.c_str(), 
                     totalSpaceStr.c_str());
            newText = newBuffer;
        } else {
            Wh_Log(L"ExtTextOutW: Parse failed. CleanFree: '%s' (%d), CleanTotal: '%s' (%d)", cleanFree.c_str(), parsedFree, cleanTotal.c_str(), parsedTotal);
        }
        
        return ExtTextOutW_Orig(hdc, x, y, options, lprect, newText.c_str(), newText.length(), nullptr);
    }

    return ExtTextOutW_Orig(hdc, x, y, options, lprect, lpString, c, lpDx);
}

int WINAPI DrawTextW_Hook(HDC hdc, LPCWSTR lpchText, int cchText, LPRECT lprc, UINT format) {
    if (!lpchText) return DrawTextW_Orig(hdc, lpchText, cchText, lprc, format);

    std::wstring text(lpchText, cchText == -1 ? wcslen(lpchText) : cchText);

    size_t freeOfPos = text.find(L" free of ");
    if (freeOfPos != std::wstring::npos) {
        if (text.find(L" used of ") != std::wstring::npos) {
            return DrawTextW_Orig(hdc, lpchText, cchText, lprc, format);
        }

        std::wstring freeSpaceStr = text.substr(0, freeOfPos);
        std::wstring totalSpaceStr = text.substr(freeOfPos + 9);
        
        std::wstring newText = text;
        
        std::wstring cleanFree = CleanNumericString(freeSpaceStr);
        std::wstring cleanTotal = CleanNumericString(totalSpaceStr);
        
        double freeVal = 0, totalVal = 0;
        wchar_t freeUnit[16] = {0}, totalUnit[16] = {0};
        
        int parsedFree = swscanf(cleanFree.c_str(), L"%lf %15s", &freeVal, freeUnit);
        int parsedTotal = swscanf(cleanTotal.c_str(), L"%lf %15s", &totalVal, totalUnit);
        
        if (parsedFree == 2 && parsedTotal == 2) {
            double freeBytes = freeVal * GetUnitMultiplier(freeUnit);
            double totalBytes = totalVal * GetUnitMultiplier(totalUnit);
            double usedBytes = totalBytes - freeBytes;
            if (usedBytes < 0) usedBytes = 0;

            std::wstring usedWStr = FormatByteSize((ULONGLONG)usedBytes);
            if (g_boldUsed) {
                usedWStr = MakeBold(usedWStr);
            }

            wchar_t newBuffer[256];
            swprintf(newBuffer, ARRAYSIZE(newBuffer), g_formatString.c_str(), 
                     freeSpaceStr.c_str(),
                     usedWStr.c_str(), 
                     totalSpaceStr.c_str());
            newText = newBuffer;
        } else {
            Wh_Log(L"DrawTextW: Parse failed. CleanFree: '%s' (%d), CleanTotal: '%s' (%d)", cleanFree.c_str(), parsedFree, cleanTotal.c_str(), parsedTotal);
        }
        
        return DrawTextW_Orig(hdc, newText.c_str(), newText.length(), lprc, format);
    }

    return DrawTextW_Orig(hdc, text.c_str(), text.length(), lprc, format);
}
HRESULT WINAPI DrawThemeTextEx_Hook(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int cchText, DWORD dwTextFlags, LPRECT pRect, const DTTOPTS *pOptions) {
    if (!pszText) return DrawThemeTextEx_Orig(hTheme, hdc, iPartId, iStateId, pszText, cchText, dwTextFlags, pRect, pOptions);

    std::wstring text(pszText, cchText == -1 ? wcslen(pszText) : cchText);

    size_t freeOfPos = text.find(L" free of ");
    if (freeOfPos != std::wstring::npos) {
        if (text.find(L" used of ") != std::wstring::npos) {
            return DrawThemeTextEx_Orig(hTheme, hdc, iPartId, iStateId, pszText, cchText, dwTextFlags, pRect, pOptions);
        }

        std::wstring freeSpaceStr = text.substr(0, freeOfPos);
        std::wstring totalSpaceStr = text.substr(freeOfPos + 9);
        
        std::wstring newText = text;
        
        std::wstring cleanFree = CleanNumericString(freeSpaceStr);
        std::wstring cleanTotal = CleanNumericString(totalSpaceStr);
        
        double freeVal = 0, totalVal = 0;
        wchar_t freeUnit[16] = {0}, totalUnit[16] = {0};
        
        int parsedFree = swscanf(cleanFree.c_str(), L"%lf %15s", &freeVal, freeUnit);
        int parsedTotal = swscanf(cleanTotal.c_str(), L"%lf %15s", &totalVal, totalUnit);
        
        if (parsedFree == 2 && parsedTotal == 2) {
            double freeBytes = freeVal * GetUnitMultiplier(freeUnit);
            double totalBytes = totalVal * GetUnitMultiplier(totalUnit);
            double usedBytes = totalBytes - freeBytes;
            if (usedBytes < 0) usedBytes = 0;

            std::wstring usedWStr = FormatByteSize((ULONGLONG)usedBytes);
            if (g_boldUsed) {
                usedWStr = MakeBold(usedWStr);
            }

            wchar_t newBuffer[256];
            swprintf(newBuffer, ARRAYSIZE(newBuffer), g_formatString.c_str(), 
                     freeSpaceStr.c_str(),
                     usedWStr.c_str(), 
                     totalSpaceStr.c_str());
            newText = newBuffer;
        } else {
            Wh_Log(L"DrawThemeTextEx: Parse failed. CleanFree: '%s' (%d), CleanTotal: '%s' (%d)", cleanFree.c_str(), parsedFree, cleanTotal.c_str(), parsedTotal);
        }
        
        return DrawThemeTextEx_Orig(hTheme, hdc, iPartId, iStateId, newText.c_str(), newText.length(), dwTextFlags, pRect, pOptions);
    }

    return DrawThemeTextEx_Orig(hTheme, hdc, iPartId, iStateId, pszText, cchText, dwTextFlags, pRect, pOptions);
}

HRESULT WINAPI DrawThemeText_Hook(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int cchText, DWORD dwTextFlags, DWORD dwTextFlags2, LPRECT pRect) {
    if (!pszText) return DrawThemeText_Orig(hTheme, hdc, iPartId, iStateId, pszText, cchText, dwTextFlags, dwTextFlags2, pRect);

    std::wstring text(pszText, cchText == -1 ? wcslen(pszText) : cchText);

    size_t freeOfPos = text.find(L" free of ");
    if (freeOfPos != std::wstring::npos) {
        if (text.find(L" used of ") != std::wstring::npos) {
            return DrawThemeText_Orig(hTheme, hdc, iPartId, iStateId, pszText, cchText, dwTextFlags, dwTextFlags2, pRect);
        }

        std::wstring freeSpaceStr = text.substr(0, freeOfPos);
        std::wstring totalSpaceStr = text.substr(freeOfPos + 9);
        
        std::wstring newText = text;
        
        std::wstring cleanFree = CleanNumericString(freeSpaceStr);
        std::wstring cleanTotal = CleanNumericString(totalSpaceStr);
        
        double freeVal = 0, totalVal = 0;
        wchar_t freeUnit[16] = {0}, totalUnit[16] = {0};
        
        int parsedFree = swscanf(cleanFree.c_str(), L"%lf %15s", &freeVal, freeUnit);
        int parsedTotal = swscanf(cleanTotal.c_str(), L"%lf %15s", &totalVal, totalUnit);
        
        if (parsedFree == 2 && parsedTotal == 2) {
            double freeBytes = freeVal * GetUnitMultiplier(freeUnit);
            double totalBytes = totalVal * GetUnitMultiplier(totalUnit);
            double usedBytes = totalBytes - freeBytes;
            if (usedBytes < 0) usedBytes = 0;

            std::wstring usedWStr = FormatByteSize((ULONGLONG)usedBytes);
            if (g_boldUsed) {
                usedWStr = MakeBold(usedWStr);
            }

            wchar_t newBuffer[256];
            swprintf(newBuffer, ARRAYSIZE(newBuffer), g_formatString.c_str(), 
                     freeSpaceStr.c_str(),
                     usedWStr.c_str(), 
                     totalSpaceStr.c_str());
            newText = newBuffer;
        } else {
            Wh_Log(L"DrawThemeText: Parse failed. CleanFree: '%s' (%d), CleanTotal: '%s' (%d)", cleanFree.c_str(), parsedFree, cleanTotal.c_str(), parsedTotal);
        }
        
        return DrawThemeText_Orig(hTheme, hdc, iPartId, iStateId, newText.c_str(), newText.length(), dwTextFlags, dwTextFlags2, pRect);
    }

    return DrawThemeText_Orig(hTheme, hdc, iPartId, iStateId, pszText, cchText, dwTextFlags, dwTextFlags2, pRect);
}

int WINAPI DrawTextExW_Hook(HDC hdc, LPWSTR lpchText, int cchText, LPRECT lprc, UINT format, LPDRAWTEXTPARAMS lpdtp) {
    if (!lpchText) return DrawTextExW_Orig(hdc, lpchText, cchText, lprc, format, lpdtp);

    std::wstring text(lpchText, cchText == -1 ? wcslen(lpchText) : cchText);

    size_t freeOfPos = text.find(L" free of ");
    if (freeOfPos != std::wstring::npos) {
        if (text.find(L" used of ") != std::wstring::npos) {
            return DrawTextExW_Orig(hdc, lpchText, cchText, lprc, format, lpdtp);
        }

        std::wstring freeSpaceStr = text.substr(0, freeOfPos);
        std::wstring totalSpaceStr = text.substr(freeOfPos + 9);
        
        std::wstring newText = text;
        
        std::wstring cleanFree = CleanNumericString(freeSpaceStr);
        std::wstring cleanTotal = CleanNumericString(totalSpaceStr);
        
        double freeVal = 0, totalVal = 0;
        wchar_t freeUnit[16] = {0}, totalUnit[16] = {0};
        
        int parsedFree = swscanf(cleanFree.c_str(), L"%lf %15s", &freeVal, freeUnit);
        int parsedTotal = swscanf(cleanTotal.c_str(), L"%lf %15s", &totalVal, totalUnit);
        
        if (parsedFree == 2 && parsedTotal == 2) {
            double freeBytes = freeVal * GetUnitMultiplier(freeUnit);
            double totalBytes = totalVal * GetUnitMultiplier(totalUnit);
            double usedBytes = totalBytes - freeBytes;
            if (usedBytes < 0) usedBytes = 0;

            std::wstring usedWStr = FormatByteSize((ULONGLONG)usedBytes);
            if (g_boldUsed) {
                usedWStr = MakeBold(usedWStr);
            }

            wchar_t newBuffer[256];
            swprintf(newBuffer, ARRAYSIZE(newBuffer), g_formatString.c_str(), 
                     freeSpaceStr.c_str(),
                     usedWStr.c_str(), 
                     totalSpaceStr.c_str());
            newText = newBuffer;
        } else {
            Wh_Log(L"DrawTextExW: Parse failed. CleanFree: '%s' (%d), CleanTotal: '%s' (%d)", cleanFree.c_str(), parsedFree, cleanTotal.c_str(), parsedTotal);
        }
        
        return DrawTextExW_Orig(hdc, (LPWSTR)newText.c_str(), newText.length(), lprc, format, lpdtp);
    }

    return DrawTextExW_Orig(hdc, lpchText, cchText, lprc, format, lpdtp);
}

// The property key for "Free Space" is PKEY_FreeSpace (System.FreeSpace).
// The property key for "Capacity" is PKEY_Capacity (System.Capacity).
// The property key for the formatted string is often PKEY_Drive_FreeSpacePercent or similar,
// or it's constructed dynamically by the view.

// Let's try hooking the function that formats the tile info string.
// In Windows 10/11, this is often done by `windows.storage.dll` or `shell32.dll`.

BOOL Wh_ModInit() {
    LoadSettings();
    
    // Hooking ExtTextOutW as a broad net.
    Wh_SetFunctionHook((void*)GetProcAddress(GetModuleHandle(L"gdi32.dll"), "ExtTextOutW"),
                       (void*)ExtTextOutW_Hook,
                       (void**)&ExtTextOutW_Orig);
                       
    Wh_SetFunctionHook((void*)GetProcAddress(GetModuleHandle(L"user32.dll"), "DrawTextW"),
                       (void*)DrawTextW_Hook,
                       (void**)&DrawTextW_Orig);
                       
    Wh_SetFunctionHook((void*)GetProcAddress(GetModuleHandle(L"uxtheme.dll"), "DrawThemeTextEx"),
                       (void*)DrawThemeTextEx_Hook,
                       (void**)&DrawThemeTextEx_Orig);
                       
    Wh_SetFunctionHook((void*)GetProcAddress(GetModuleHandle(L"uxtheme.dll"), "DrawThemeText"),
                       (void*)DrawThemeText_Hook,
                       (void**)&DrawThemeText_Orig);
                       
    Wh_SetFunctionHook((void*)GetProcAddress(GetModuleHandle(L"user32.dll"), "DrawTextExW"),
                       (void*)DrawTextExW_Hook,
                       (void**)&DrawTextExW_Orig);

    return TRUE;
}

void Wh_ModUninit() {
}

void Wh_ModSettingsChanged() {
    LoadSettings();
}
