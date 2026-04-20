// ==WindhawkMod==
// @id              this-pc-used-space
// @name            This PC Used Space
// @description     Adds "Used Space" to the drive details in "This PC" view in File Explorer.
// @version         0.9
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
- removeSpace: false
  $name: Remove Space from Units
  $description: Remove the space between the number and the unit (e.g., "100GB" instead of "100 GB")
- lineYOffset: 0
  $name: Line Y Offset
  $description: Adjust the vertical position of the entire line of text.
- boldYOffset: 0
  $name: Bold Text Y Offset
  $description: Adjust the vertical position of ONLY the bolded "Used Space" segment.
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
bool g_removeSpace = false;
int g_lineYOffset = 0;
int g_boldYOffset = 0;

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

    g_boldUsed = (bool)Wh_GetIntSetting(L"boldUsed");
    g_removeSpace = (bool)Wh_GetIntSetting(L"removeSpace");

    PCWSTR style = Wh_GetStringSetting(L"boldStyle");
    if (style) {
        if (wcscmp(style, L"serif") == 0) g_boldStyle = BoldStyle::Serif;
        else g_boldStyle = BoldStyle::SansSerif;
        Wh_FreeStringSetting(style);
    }

    g_lineYOffset = Wh_GetIntSetting(L"lineYOffset");
    g_boldYOffset = Wh_GetIntSetting(L"boldYOffset");
}

// Helper to format byte sizes (e.g., 1024 -> "1.00 KB")
std::wstring FormatByteSize(ULONGLONG bytes) {
    wchar_t buffer[64];
    StrFormatByteSizeW(bytes, buffer, ARRAYSIZE(buffer));
    std::wstring s(buffer);
    if (g_removeSpace) {
        s.erase(std::remove(s.begin(), s.end(), L' '), s.end());
    }
    return s;
}

// Helper to clean strings before parsing.
// Removes non-printable characters and formatting marks like LRM/RLM.
std::wstring CleanNumericString(const std::wstring& s) {
    std::wstring result;
    bool foundStart = false;
    for (wchar_t c : s) {
        if (!foundStart) {
            // Keep digits, decimal separators, and signs
            if (iswdigit(c) || c == L'.' || c == L',' || c == L'-') {
                foundStart = true;
                result += c;
            }
            continue;
        }
        // Preserve both . and , as swscanf is locale-aware for floats
        // We only strip spaces which are common in some thousands-grouping formats
        if (c == L' ') continue; 
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

struct TextSegments {
    std::wstring prefix;
    std::wstring used;
    std::wstring suffix;
};

TextSegments GetSegments(const std::wstring& freeStr, const std::wstring& usedStr, const std::wstring& totalStr) {
    std::wstring format = g_formatString;
    TextSegments segs;
    
    size_t pos1 = format.find(L"%s");
    if (pos1 != std::wstring::npos) {
        segs.prefix = format.substr(0, pos1) + freeStr;
        format.erase(0, pos1 + 2);
    }
    
    size_t pos2 = format.find(L"%s");
    if (pos2 != std::wstring::npos) {
        segs.prefix += format.substr(0, pos2);
        segs.used = usedStr;
        segs.suffix = format.substr(pos2 + 2);
    }
    
    size_t pos3 = segs.suffix.find(L"%s");
    if (pos3 != std::wstring::npos) {
        std::wstring afterUsed = segs.suffix.substr(0, pos3);
        std::wstring afterTotal = segs.suffix.substr(pos3 + 2);
        segs.suffix = afterUsed + totalStr + afterTotal;
    }
    
    return segs;
}

int WINAPI DrawTextW_Hook(HDC hdc, LPCWSTR lpchText, int cchText, LPRECT lprc, UINT format) {
    if (!lpchText || !lprc) return DrawTextW_Orig(hdc, lpchText, cchText, lprc, format);

    std::wstring text(lpchText, cchText == -1 ? wcslen(lpchText) : cchText);

    size_t freeOfPos = text.find(L" free of ");
    if (freeOfPos != std::wstring::npos) {
        if (text.find(L" used of ") != std::wstring::npos) {
            return DrawTextW_Orig(hdc, lpchText, cchText, lprc, format);
        }

        std::wstring freeSpaceStr = text.substr(0, freeOfPos);
        std::wstring totalSpaceStr = text.substr(freeOfPos + 9);
        
        if (g_removeSpace) {
            freeSpaceStr.erase(std::remove(freeSpaceStr.begin(), freeSpaceStr.end(), L' '), freeSpaceStr.end());
            totalSpaceStr.erase(std::remove(totalSpaceStr.begin(), totalSpaceStr.end(), L' '), totalSpaceStr.end());
        }

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

            TextSegments segs = GetSegments(freeSpaceStr, usedWStr, totalSpaceStr);
            
            if (format & DT_CALCRECT) {
                std::wstring fullText = segs.prefix + segs.used + segs.suffix;
                return DrawTextW_Orig(hdc, fullText.c_str(), fullText.length(), lprc, format);
            }

            // Strip ellipsis and clipping for chunked drawing
            UINT drawFlags = (format & ~(DT_END_ELLIPSIS | DT_PATH_ELLIPSIS | DT_WORD_ELLIPSIS)) | DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP;

            SIZE szPrefix, szUsed, szSuffix;
            GetTextExtentPoint32W(hdc, segs.prefix.c_str(), segs.prefix.length(), &szPrefix);
            GetTextExtentPoint32W(hdc, segs.used.c_str(), segs.used.length(), &szUsed);
            GetTextExtentPoint32W(hdc, segs.suffix.c_str(), segs.suffix.length(), &szSuffix);
            
            int totalWidth = szPrefix.cx + szUsed.cx + szSuffix.cx;
            int startX = lprc->left;
            if (format & DT_CENTER) startX += (lprc->right - lprc->left - totalWidth) / 2;
            else if (format & DT_RIGHT) startX = lprc->right - totalWidth;
            
            RECT rc = *lprc;
            rc.top += g_lineYOffset;
            rc.bottom += g_lineYOffset;

            rc.left = startX;
            rc.right = startX + szPrefix.cx;
            DrawTextW_Orig(hdc, segs.prefix.c_str(), segs.prefix.length(), &rc, drawFlags);
            
            rc.left = rc.right;
            rc.right = rc.left + szUsed.cx;
            rc.top += g_boldYOffset;
            rc.bottom += g_boldYOffset;
            DrawTextW_Orig(hdc, segs.used.c_str(), segs.used.length(), &rc, drawFlags);
            
            rc.left = rc.right;
            rc.right = rc.left + szSuffix.cx;
            rc.top -= g_boldYOffset;
            rc.bottom -= g_boldYOffset;
            return DrawTextW_Orig(hdc, segs.suffix.c_str(), segs.suffix.length(), &rc, drawFlags);
        }
    }

    return DrawTextW_Orig(hdc, lpchText, cchText, lprc, format);
}

HRESULT WINAPI DrawThemeTextEx_Hook(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int cchText, DWORD dwTextFlags, LPRECT pRect, const DTTOPTS *pOptions) {
    if (!pszText || !pRect) return DrawThemeTextEx_Orig(hTheme, hdc, iPartId, iStateId, pszText, cchText, dwTextFlags, pRect, pOptions);

    std::wstring text(pszText, cchText == -1 ? wcslen(pszText) : cchText);

    size_t freeOfPos = text.find(L" free of ");
    if (freeOfPos != std::wstring::npos) {
        if (text.find(L" used of ") != std::wstring::npos) {
            return DrawThemeTextEx_Orig(hTheme, hdc, iPartId, iStateId, pszText, cchText, dwTextFlags, pRect, pOptions);
        }

        std::wstring freeSpaceStr = text.substr(0, freeOfPos);
        std::wstring totalSpaceStr = text.substr(freeOfPos + 9);
        
        if (g_removeSpace) {
            freeSpaceStr.erase(std::remove(freeSpaceStr.begin(), freeSpaceStr.end(), L' '), freeSpaceStr.end());
            totalSpaceStr.erase(std::remove(totalSpaceStr.begin(), totalSpaceStr.end(), L' '), totalSpaceStr.end());
        }

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

            TextSegments segs = GetSegments(freeSpaceStr, usedWStr, totalSpaceStr);
            
            if (dwTextFlags & DT_CALCRECT) {
                std::wstring fullText = segs.prefix + segs.used + segs.suffix;
                return DrawThemeTextEx_Orig(hTheme, hdc, iPartId, iStateId, fullText.c_str(), fullText.length(), dwTextFlags, pRect, pOptions);
            }

            // Strip ellipsis for chunks
            DWORD drawFlags = (dwTextFlags & ~(DT_END_ELLIPSIS | DT_PATH_ELLIPSIS | DT_WORD_ELLIPSIS)) | DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP;

            RECT rcPrefix = {0}, rcUsed = {0}, rcSuffix = {0};
            GetThemeTextExtent(hTheme, hdc, iPartId, iStateId, segs.prefix.c_str(), segs.prefix.length(), drawFlags, nullptr, &rcPrefix);
            GetThemeTextExtent(hTheme, hdc, iPartId, iStateId, segs.used.c_str(), segs.used.length(), drawFlags, nullptr, &rcUsed);
            GetThemeTextExtent(hTheme, hdc, iPartId, iStateId, segs.suffix.c_str(), segs.suffix.length(), drawFlags, nullptr, &rcSuffix);
            
            int prefixW = rcPrefix.right - rcPrefix.left;
            int usedW = rcUsed.right - rcUsed.left;
            int suffixW = rcSuffix.right - rcSuffix.left;
            
            int totalWidth = prefixW + usedW + suffixW;
            int startX = pRect->left;
            if (dwTextFlags & DT_CENTER) startX += (pRect->right - pRect->left - totalWidth) / 2;
            else if (dwTextFlags & DT_RIGHT) startX = pRect->right - totalWidth;
            
            RECT rc = *pRect;
            rc.top += g_lineYOffset;
            rc.bottom += g_lineYOffset;

            rc.left = startX;
            rc.right = startX + prefixW;
            DrawThemeTextEx_Orig(hTheme, hdc, iPartId, iStateId, segs.prefix.c_str(), segs.prefix.length(), drawFlags, &rc, pOptions);
            
            rc.left = rc.right;
            rc.right = rc.left + usedW;
            rc.top += g_boldYOffset;
            rc.bottom += g_boldYOffset;
            DrawThemeTextEx_Orig(hTheme, hdc, iPartId, iStateId, segs.used.c_str(), segs.used.length(), drawFlags, &rc, pOptions);
            
            rc.left = rc.right;
            rc.right = rc.left + suffixW;
            rc.top -= g_boldYOffset;
            rc.bottom -= g_boldYOffset;
            return DrawThemeTextEx_Orig(hTheme, hdc, iPartId, iStateId, segs.suffix.c_str(), segs.suffix.length(), drawFlags, &rc, pOptions);
        }
    }

    return DrawThemeTextEx_Orig(hTheme, hdc, iPartId, iStateId, pszText, cchText, dwTextFlags, pRect, pOptions);
}

HRESULT WINAPI DrawThemeText_Hook(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int cchText, DWORD dwTextFlags, DWORD dwTextFlags2, LPRECT pRect) {
    if (!pszText || !pRect) return DrawThemeText_Orig(hTheme, hdc, iPartId, iStateId, pszText, cchText, dwTextFlags, dwTextFlags2, pRect);

    std::wstring text(pszText, cchText == -1 ? wcslen(pszText) : cchText);

    size_t freeOfPos = text.find(L" free of ");
    if (freeOfPos != std::wstring::npos) {
        if (text.find(L" used of ") != std::wstring::npos) {
            return DrawThemeText_Orig(hTheme, hdc, iPartId, iStateId, pszText, cchText, dwTextFlags, dwTextFlags2, pRect);
        }

        std::wstring freeSpaceStr = text.substr(0, freeOfPos);
        std::wstring totalSpaceStr = text.substr(freeOfPos + 9);
        
        if (g_removeSpace) {
            freeSpaceStr.erase(std::remove(freeSpaceStr.begin(), freeSpaceStr.end(), L' '), freeSpaceStr.end());
            totalSpaceStr.erase(std::remove(totalSpaceStr.begin(), totalSpaceStr.end(), L' '), totalSpaceStr.end());
        }

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

            TextSegments segs = GetSegments(freeSpaceStr, usedWStr, totalSpaceStr);
            
            if (dwTextFlags & DT_CALCRECT) {
                std::wstring fullText = segs.prefix + segs.used + segs.suffix;
                return DrawThemeText_Orig(hTheme, hdc, iPartId, iStateId, fullText.c_str(), fullText.length(), dwTextFlags, dwTextFlags2, pRect);
            }

            // Strip ellipsis for chunks
            DWORD drawFlags = (dwTextFlags & ~(DT_END_ELLIPSIS | DT_PATH_ELLIPSIS | DT_WORD_ELLIPSIS)) | DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP;

            RECT rcPrefix = {0}, rcUsed = {0}, rcSuffix = {0};
            GetThemeTextExtent(hTheme, hdc, iPartId, iStateId, segs.prefix.c_str(), segs.prefix.length(), drawFlags, nullptr, &rcPrefix);
            GetThemeTextExtent(hTheme, hdc, iPartId, iStateId, segs.used.c_str(), segs.used.length(), drawFlags, nullptr, &rcUsed);
            GetThemeTextExtent(hTheme, hdc, iPartId, iStateId, segs.suffix.c_str(), segs.suffix.length(), drawFlags, nullptr, &rcSuffix);
            
            int prefixW = rcPrefix.right - rcPrefix.left;
            int usedW = rcUsed.right - rcUsed.left;
            int suffixW = rcSuffix.right - rcSuffix.left;
            
            int totalWidth = prefixW + usedW + suffixW;
            int startX = pRect->left;
            if (dwTextFlags & DT_CENTER) startX += (pRect->right - pRect->left - totalWidth) / 2;
            else if (dwTextFlags & DT_RIGHT) startX = pRect->right - totalWidth;
            
            RECT rc = *pRect;
            rc.top += g_lineYOffset;
            rc.bottom += g_lineYOffset;

            rc.left = startX;
            rc.right = startX + prefixW;
            DrawThemeText_Orig(hTheme, hdc, iPartId, iStateId, segs.prefix.c_str(), segs.prefix.length(), drawFlags, dwTextFlags2, &rc);
            
            rc.left = rc.right;
            rc.right = rc.left + usedW;
            rc.top += g_boldYOffset;
            rc.bottom += g_boldYOffset;
            DrawThemeText_Orig(hTheme, hdc, iPartId, iStateId, segs.used.c_str(), segs.used.length(), drawFlags, dwTextFlags2, &rc);
            
            rc.left = rc.right;
            rc.right = rc.left + suffixW;
            rc.top -= g_boldYOffset;
            rc.bottom -= g_boldYOffset;
            return DrawThemeText_Orig(hTheme, hdc, iPartId, iStateId, segs.suffix.c_str(), segs.suffix.length(), drawFlags, dwTextFlags2, &rc);
        }
    }

    return DrawThemeText_Orig(hTheme, hdc, iPartId, iStateId, pszText, cchText, dwTextFlags, dwTextFlags2, pRect);
}

int WINAPI DrawTextExW_Hook(HDC hdc, LPWSTR lpchText, int cchText, LPRECT lprc, UINT format, LPDRAWTEXTPARAMS lpdtp) {
    if (!lpchText || !lprc) return DrawTextExW_Orig(hdc, lpchText, cchText, lprc, format, lpdtp);

    std::wstring text(lpchText, cchText == -1 ? wcslen(lpchText) : cchText);

    size_t freeOfPos = text.find(L" free of ");
    if (freeOfPos != std::wstring::npos) {
        if (text.find(L" used of ") != std::wstring::npos) {
            return DrawTextExW_Orig(hdc, lpchText, cchText, lprc, format, lpdtp);
        }

        std::wstring freeSpaceStr = text.substr(0, freeOfPos);
        std::wstring totalSpaceStr = text.substr(freeOfPos + 9);
        
        if (g_removeSpace) {
            freeSpaceStr.erase(std::remove(freeSpaceStr.begin(), freeSpaceStr.end(), L' '), freeSpaceStr.end());
            totalSpaceStr.erase(std::remove(totalSpaceStr.begin(), totalSpaceStr.end(), L' '), totalSpaceStr.end());
        }

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

            TextSegments segs = GetSegments(freeSpaceStr, usedWStr, totalSpaceStr);
            
            if (format & DT_CALCRECT) {
                std::wstring fullText = segs.prefix + segs.used + segs.suffix;
                std::vector<wchar_t> writeableBuffer(fullText.begin(), fullText.end());
                writeableBuffer.push_back(L'\0');
                return DrawTextExW_Orig(hdc, writeableBuffer.data(), fullText.length(), lprc, format, lpdtp);
            }

            // Strip ellipsis for chunks
            UINT drawFlags = (format & ~(DT_END_ELLIPSIS | DT_PATH_ELLIPSIS | DT_WORD_ELLIPSIS)) | DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP;

            SIZE szPrefix, szUsed, szSuffix;
            GetTextExtentPoint32W(hdc, segs.prefix.c_str(), segs.prefix.length(), &szPrefix);
            GetTextExtentPoint32W(hdc, segs.used.c_str(), segs.used.length(), &szUsed);
            GetTextExtentPoint32W(hdc, segs.suffix.c_str(), segs.suffix.length(), &szSuffix);
            
            int totalWidth = szPrefix.cx + szUsed.cx + szSuffix.cx;
            int startX = lprc->left;
            if (format & DT_CENTER) startX += (lprc->right - lprc->left - totalWidth) / 2;
            else if (format & DT_RIGHT) startX = lprc->right - totalWidth;
            
            RECT rc = *lprc;
            rc.top += g_lineYOffset;
            rc.bottom += g_lineYOffset;

            rc.left = startX;
            rc.right = startX + szPrefix.cx;
            std::vector<wchar_t> b1(segs.prefix.begin(), segs.prefix.end()); b1.push_back(L'\0');
            DrawTextExW_Orig(hdc, b1.data(), segs.prefix.length(), &rc, drawFlags, lpdtp);
            
            rc.left = rc.right;
            rc.right = rc.left + szUsed.cx;
            rc.top += g_boldYOffset;
            rc.bottom += g_boldYOffset;
            std::vector<wchar_t> b2(segs.used.begin(), segs.used.end()); b2.push_back(L'\0');
            DrawTextExW_Orig(hdc, b2.data(), segs.used.length(), &rc, drawFlags, lpdtp);
            
            rc.left = rc.right;
            rc.right = rc.left + szSuffix.cx;
            rc.top -= g_boldYOffset;
            rc.bottom -= g_boldYOffset;
            std::vector<wchar_t> b3(segs.suffix.begin(), segs.suffix.end()); b3.push_back(L'\0');
            return DrawTextExW_Orig(hdc, b3.data(), segs.suffix.length(), &rc, drawFlags, lpdtp);
        }
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
