// ==WindhawkMod==
// @id              per-app-dpi-override
// @name            Per-App Custom DPI Override
// @description     Overrides the DPI scaling factor for specific applications.
// @version         2.0.0
// @author          bbmaster123
// @compilerOptions -lshcore -lpsapi -lcomctl32 -lgdi32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
This mod allows you to artificially increase or decrease the DPI scaling factor for specific applications.
Normally, Windows uses the globally configured display DPI (like 100%, 150%, 200%).
This hooks application DPI queries to report a higher or lower scale factor.

### Chromium Browsers (Edge, Chrome, Brave)
For Chromium-based browsers, changing Windows DPI APIs usually doesn't work perfectly due to their multi-process architecture and internal caching.
To solve this, the mod can inject the `--force-device-scale-factor` command line argument when the browser starts.
**Important:** You must fully close the browser (including background tasks / Startup Boost) for the command line injection to take effect.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- apps:
  - - processName: msedge.exe
      $name: Process Name
      $description: "The execution name to target. e.g., msedge.exe, notepad.exe, wlmail.exe"
    - scalePercentage: 111
      $name: Scale Percentage
      $description: "The scale factor as a percentage to apply (e.g., 125 for 125% scale, 80 for 80% scale)."
    - enableChromiumCmdLine: true
      $name: Enable Chromium Command Line Injection
      $description: "Automatically injects --force-device-scale-factor for Chromium browsers. Note: You must fully close the browser first."
    - enableDpiHooks: true
      $name: Enable Windows DPI API Hooks
      $description: "Hooks standard Windows DPI APIs (GetDpiForWindow, GetDeviceCaps, etc.). Disable if you only want Chromium scaling or if it causes instability."
    - excludeMenuBar: false
      $name: Exclude Menu Bar from Scaling
      $description: "Prevents the menu bar and menu fonts from being scaled (useful for some legacy apps)."
  $name: App Settings
  $description: Add a new entry for each application you want to override.
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <shellscalingapi.h>
#include <windhawk_api.h>
#include <string>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <intrin.h>
#include <psapi.h>
#include <commctrl.h>
#include <uxtheme.h>
#include <unordered_set>
#include <unordered_map>
#include <mutex>

#if defined(__GNUC__) || defined(__clang__)
#define _ReturnAddress() __builtin_return_address(0)
#else
#pragma intrinsic(_ReturnAddress)
#endif

double g_scaleMultiplier = 1.0;
bool g_enableChromiumCmdLine = false;
bool g_enableDpiHooks = false;
bool g_excludeMenuBar = false;
bool g_shouldApply = false;
bool g_isWlmail = false;
bool g_isMmc = false;
bool g_isNotepad = false;

static void* g_user32_start = nullptr;
static void* g_user32_end = nullptr;
static void* g_gdi32_start = nullptr;
static void* g_gdi32_end = nullptr;
static void* g_comctl32_start = nullptr;
static void* g_comctl32_end = nullptr;
static void* g_uxtheme_start = nullptr;
static void* g_uxtheme_end = nullptr;

thread_local bool in_dpi_hook = false;

bool IsInternalOSCall(void* retAddr) {
    if (!retAddr) return false;
    if (g_user32_start && retAddr >= g_user32_start && retAddr < g_user32_end) return true;
    if (g_gdi32_start && retAddr >= g_gdi32_start && retAddr < g_gdi32_end) return true;
    if (g_comctl32_start && retAddr >= g_comctl32_start && retAddr < g_comctl32_end) return true;
    if (g_uxtheme_start && retAddr >= g_uxtheme_start && retAddr < g_uxtheme_end) return true;
    return false;
}

static void* g_mshtml_start = nullptr;
static void* g_mshtml_end = nullptr;
static void* g_ieframe_start = nullptr;
static void* g_ieframe_end = nullptr;

typedef int(WINAPI* GetSystemMetricsForDpi_t)(int nIndex, UINT dpi);
GetSystemMetricsForDpi_t GetSystemMetricsForDpi_Orig;

int WINAPI GetSystemMetricsForDpi_Hook(int nIndex, UINT dpi) {
    if (in_dpi_hook) return GetSystemMetricsForDpi_Orig(nIndex, dpi);
    in_dpi_hook = true;
    int res = GetSystemMetricsForDpi_Orig(nIndex, dpi);
    if (g_isWlmail) {
        if (nIndex == SM_CYCAPTION || nIndex == SM_CYSMCAPTION || nIndex == SM_CYSIZEFRAME || nIndex == SM_CXSIZEFRAME) {
            in_dpi_hook = false;
            return res;
        }
    }
    in_dpi_hook = false;
    return res;
}

void UpdateModuleRanges() {
    if (!g_mshtml_start) {
        HMODULE hMshtml = GetModuleHandle(L"mshtml.dll");
        if (hMshtml) {
            g_mshtml_start = hMshtml;
            PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)hMshtml;
            if (dosHeader->e_magic == IMAGE_DOS_SIGNATURE) {
                PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((PBYTE)hMshtml + dosHeader->e_lfanew);
                if (ntHeaders->Signature == IMAGE_NT_SIGNATURE) {
                    g_mshtml_end = (PBYTE)hMshtml + ntHeaders->OptionalHeader.SizeOfImage;
                }
            }
        }
    }
    if (!g_ieframe_start) {
        HMODULE hIeframe = GetModuleHandle(L"ieframe.dll");
        if (hIeframe) {
            g_ieframe_start = hIeframe;
            PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)hIeframe;
            if (dosHeader->e_magic == IMAGE_DOS_SIGNATURE) {
                PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((PBYTE)hIeframe + dosHeader->e_lfanew);
                if (ntHeaders->Signature == IMAGE_NT_SIGNATURE) {
                    g_ieframe_end = (PBYTE)hIeframe + ntHeaders->OptionalHeader.SizeOfImage;
                }
            }
        }
    }
}

UINT GetAdjustedDpiForModule(void* retAddr) {
    if (!retAddr) return 0;
    UpdateModuleRanges();
    if ((g_mshtml_start && retAddr >= g_mshtml_start && retAddr < g_mshtml_end) ||
        (g_ieframe_start && retAddr >= g_ieframe_start && retAddr < g_ieframe_end)) {
        return (UINT)(96.0 + (96.0 * g_scaleMultiplier - 96.0) / 2.0);
    }
    return 0;
}

bool CaseInsensitiveContains(const std::wstring& str, const std::wstring& sub) {
    auto it = std::search(
        str.begin(), str.end(),
        sub.begin(), sub.end(),
        [](wchar_t ch1, wchar_t ch2) { return std::towlower(ch1) == std::towlower(ch2); }
    );
    return it != str.end();
}

// -------------------------------------------------------------------------
// THREAD-SAFE FONT REGISTRY & CACHING
// -------------------------------------------------------------------------

std::unordered_map<HFONT, HFONT> g_fontScaleCache; 
std::unordered_set<HFONT> g_scaledFonts;
std::mutex g_fontMutex;
std::unordered_map<HTHEME, std::wstring> g_themeClasses;
std::mutex g_themeMutex;

HFONT GetOrCreateScaledFont(HFONT hOrigFont) {
    if (!hOrigFont) return NULL;
    std::lock_guard<std::mutex> lock(g_fontMutex);
    
    // If it's already a scaled font, return it directly
    if (g_scaledFonts.find(hOrigFont) != g_scaledFonts.end()) {
        return hOrigFont;
    }
    
    auto it = g_fontScaleCache.find(hOrigFont);
    if (it != g_fontScaleCache.end()) {
        return it->second;
    }
    
    LOGFONTW lf;
    if (GetObjectW(hOrigFont, sizeof(lf), &lf)) {
        // Heuristic to avoid double-scaling:
        // Standard UI fonts are usually 8-12pt, which at 100% is around -11 to -16 pixels.
        // If it's already > 25 pixels, it's likely already scaled by a hooked DPI API.
        if (std::abs(lf.lfHeight) > 25) {
            return hOrigFont;
        }

        lf.lfHeight = (LONG)(lf.lfHeight * g_scaleMultiplier);
        if (lf.lfWidth != 0) lf.lfWidth = (LONG)(lf.lfWidth * g_scaleMultiplier);
        
        HFONT hScaled = CreateFontIndirectW(&lf);
        if (hScaled) {
            g_scaledFonts.insert(hScaled);
            g_fontScaleCache[hOrigFont] = hScaled;
            return hScaled;
        }
    }
    return hOrigFont;
}

// -------------------------------------------------------------------------
// GLOBAL FONT HOOKS FOR MMC
// -------------------------------------------------------------------------

typedef HFONT(WINAPI* CreateFontIndirectW_t)(const LOGFONTW* plf);
CreateFontIndirectW_t CreateFontIndirectW_Orig;

HFONT WINAPI CreateFontIndirectW_Hook(const LOGFONTW* plf) {
    if (g_isMmc && plf && !in_dpi_hook) {
        in_dpi_hook = true;
        LOGFONTW lf = *plf;
        if (std::abs(lf.lfHeight) < 30) {
            lf.lfHeight = (LONG)(lf.lfHeight * g_scaleMultiplier);
            if (lf.lfWidth != 0) lf.lfWidth = (LONG)(lf.lfWidth * g_scaleMultiplier);
        }
        HFONT hFont = CreateFontIndirectW_Orig(&lf);
        if (hFont) {
            std::lock_guard<std::mutex> lock(g_fontMutex);
            g_scaledFonts.insert(hFont);
        }
        in_dpi_hook = false;
        return hFont;
    }
    return CreateFontIndirectW_Orig(plf);
}

typedef HFONT(WINAPI* CreateFontW_t)(int cHeight, int cWidth, int cEscapement, int cOrientation, int cWeight, DWORD bItalic, DWORD bUnderline, DWORD bStrikeOut, DWORD iCharSet, DWORD iOutPrecision, DWORD iClipPrecision, DWORD iQuality, DWORD iPitchAndFamily, LPCWSTR pszFaceName);
CreateFontW_t CreateFontW_Orig;

HFONT WINAPI CreateFontW_Hook(int cHeight, int cWidth, int cEscapement, int cOrientation, int cWeight, DWORD bItalic, DWORD bUnderline, DWORD bStrikeOut, DWORD iCharSet, DWORD iOutPrecision, DWORD iClipPrecision, DWORD iQuality, DWORD iPitchAndFamily, LPCWSTR pszFaceName) {
    if (g_isMmc && !in_dpi_hook) {
        in_dpi_hook = true;
        if (std::abs(cHeight) < 30) {
            cHeight = (int)(cHeight * g_scaleMultiplier);
            if (cWidth != 0) cWidth = (int)(cWidth * g_scaleMultiplier);
        }
        HFONT hFont = CreateFontW_Orig(cHeight, cWidth, cEscapement, cOrientation, cWeight, bItalic, bUnderline, bStrikeOut, iCharSet, iOutPrecision, iClipPrecision, iQuality, iPitchAndFamily, pszFaceName);
        if (hFont) {
            std::lock_guard<std::mutex> lock(g_fontMutex);
            g_scaledFonts.insert(hFont);
        }
        in_dpi_hook = false;
        return hFont;
    }
    return CreateFontW_Orig(cHeight, cWidth, cEscapement, cOrientation, cWeight, bItalic, bUnderline, bStrikeOut, iCharSet, iOutPrecision, iClipPrecision, iQuality, iPitchAndFamily, pszFaceName);
}

// -------------------------------------------------------------------------
// IMAGE LOADING HOOKS
// -------------------------------------------------------------------------

typedef HANDLE(WINAPI* LoadImageW_t)(HINSTANCE hInst, LPCWSTR name, UINT type, int cx, int cy, UINT fuLoad);
LoadImageW_t LoadImageW_Orig;

HANDLE WINAPI LoadImageW_Hook(HINSTANCE hInst, LPCWSTR name, UINT type, int cx, int cy, UINT fuLoad) {
    if (g_isMmc && cx > 0 && cy > 0 && (type == IMAGE_ICON || type == IMAGE_BITMAP)) {
        cx = (int)(cx * g_scaleMultiplier);
        cy = (int)(cy * g_scaleMultiplier);
    }
    return LoadImageW_Orig(hInst, name, type, cx, cy, fuLoad);
}

typedef HANDLE(WINAPI* LoadImageA_t)(HINSTANCE hInst, LPCSTR name, UINT type, int cx, int cy, UINT fuLoad);
LoadImageA_t LoadImageA_Orig;

HANDLE WINAPI LoadImageA_Hook(HINSTANCE hInst, LPCSTR name, UINT type, int cx, int cy, UINT fuLoad) {
    if (g_isMmc && cx > 0 && cy > 0 && (type == IMAGE_ICON || type == IMAGE_BITMAP)) {
        cx = (int)(cx * g_scaleMultiplier);
        cy = (int)(cy * g_scaleMultiplier);
    }
    return LoadImageA_Orig(hInst, name, type, cx, cy, fuLoad);
}

// -------------------------------------------------------------------------
// DPI HOOKS
// -------------------------------------------------------------------------

typedef UINT(WINAPI* GetDpiForWindow_t)(HWND hwnd);
GetDpiForWindow_t GetDpiForWindow_Orig;

UINT WINAPI GetDpiForWindow_Hook(HWND hwnd) {
    if (in_dpi_hook) {
        return GetDpiForWindow_Orig(hwnd);
    }
    in_dpi_hook = true;
    UINT dpi = GetDpiForWindow_Orig(hwnd);
    if (dpi == 0) {
        in_dpi_hook = false;
        return 0;
    }
    
    void* retAddr = _ReturnAddress();
    if (IsInternalOSCall(retAddr)) {
        in_dpi_hook = false;
        return dpi; // Bypass internal OS calls to prevent crashes
    }
    UINT adj = GetAdjustedDpiForModule(retAddr);
    if (adj > 0) {
        in_dpi_hook = false;
        return adj;
    }
    
    UINT result = (UINT)(96.0 * g_scaleMultiplier);
    in_dpi_hook = false;
    return result;
}

typedef UINT(WINAPI* GetDpiForSystem_t)();
GetDpiForSystem_t GetDpiForSystem_Orig;

UINT WINAPI GetDpiForSystem_Hook() {
    if (in_dpi_hook) {
        return GetDpiForSystem_Orig();
    }
    in_dpi_hook = true;
    UINT dpi = GetDpiForSystem_Orig();
    
    void* retAddr = _ReturnAddress();
    if (IsInternalOSCall(retAddr)) {
        in_dpi_hook = false;
        return dpi;
    }
    UINT adj = GetAdjustedDpiForModule(retAddr);
    if (adj > 0) {
        in_dpi_hook = false;
        return adj;
    }
    
    UINT result = (UINT)(96.0 * g_scaleMultiplier);
    in_dpi_hook = false;
    return result;
}

// removed GetSystemMetricsForDpi_Hook

typedef int(WINAPI* GetSystemMetrics_t)(int nIndex);
GetSystemMetrics_t GetSystemMetrics_Orig;

int WINAPI GetSystemMetrics_Hook(int nIndex) {
    if (in_dpi_hook) {
        return GetSystemMetrics_Orig(nIndex);
    }
    in_dpi_hook = true;
    int metric = GetSystemMetrics_Orig(nIndex);
    
    void* retAddr = _ReturnAddress();
    if (IsInternalOSCall(retAddr)) {
        if (g_isWlmail || g_isMmc || g_isNotepad) {
            in_dpi_hook = false;
            return metric;
        }
    }

    if (metric > 0) {
        switch (nIndex) {
            case SM_CXSCREEN:
            case SM_CYSCREEN:
            case SM_CXMAXIMIZED:
            case SM_CYMAXIMIZED:
            case SM_CXMAXTRACK:
            case SM_CYMAXTRACK:
            case SM_CXFULLSCREEN:
            case SM_CYFULLSCREEN:
            case SM_CXMINTRACK:
            case SM_CYMINTRACK:
                in_dpi_hook = false;
                return metric; // Skip scaling bounds/screen sizes
            case SM_CYCAPTION:
            case SM_CYSMCAPTION:
                if (g_isWlmail) {
                    in_dpi_hook = false;
                    return metric; // Scaled caption causes rendering gaps in WLM ribbon
                }
                // fallthrough
            case SM_CYSIZE:
            case SM_CXSIZE:
            case SM_CXVSCROLL:
            case SM_CYHSCROLL:

            case SM_CXSMICON:
            case SM_CYSMICON:
            case SM_CXICON:
            case SM_CYICON:
            case SM_CXCURSOR:
            case SM_CYCURSOR:
                metric = (int)(metric * g_scaleMultiplier);
                in_dpi_hook = false;
                return metric;
            case SM_CXSIZEFRAME:
            case SM_CYSIZEFRAME:
            case SM_CXPADDEDBORDER:
                if (g_isWlmail) {
                    in_dpi_hook = false;
                    return metric;
                }
                // fallthrough
            default:
            {
                if (g_excludeMenuBar && (nIndex == SM_CYMENU || nIndex == SM_CXMENUSIZE || nIndex == SM_CYMENUSIZE || nIndex == SM_CXMENUCHECK || nIndex == SM_CYMENUCHECK)) {
                    in_dpi_hook = false;
                    return metric;
                }
                UINT queryDpi = (UINT)(96.0 * g_scaleMultiplier);
                
                HMODULE hUser32 = GetModuleHandle(L"user32.dll");
                if (hUser32) {
                    typedef int(WINAPI* GetSystemMetricsForDpi_t)(int, UINT);
                    GetSystemMetricsForDpi_t pGetSystemMetricsForDpi = (GetSystemMetricsForDpi_t)GetProcAddress(hUser32, "GetSystemMetricsForDpi");
                    if (pGetSystemMetricsForDpi) {
                        int metricForDpi = pGetSystemMetricsForDpi(nIndex, queryDpi);
                        in_dpi_hook = false;
                        return metricForDpi;
                    }
                }
                
                int resultMetric = (int)(metric * g_scaleMultiplier);
                in_dpi_hook = false;
                return resultMetric;
            }
        }
    }
    
    in_dpi_hook = false;
    return metric;
}

// removed SystemParametersInfoForDpi_Hook

typedef BOOL(WINAPI* SystemParametersInfoW_t)(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni);
SystemParametersInfoW_t SystemParametersInfoW_Orig;

BOOL WINAPI SystemParametersInfoW_Hook(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni) {
    if (in_dpi_hook) {
        return SystemParametersInfoW_Orig(uiAction, uiParam, pvParam, fWinIni);
    }
    in_dpi_hook = true;
    void* retAddr = _ReturnAddress();
    if (IsInternalOSCall(retAddr)) {
        BOOL res = SystemParametersInfoW_Orig(uiAction, uiParam, pvParam, fWinIni);
        in_dpi_hook = false;
        return res;
    }
    
    BOOL ret = SystemParametersInfoW_Orig(uiAction, uiParam, pvParam, fWinIni);
    if (!ret) {
        in_dpi_hook = false;
        return ret;
    }
    
    double actualMultiplier = g_scaleMultiplier;
    if (GetDpiForSystem_Orig) {
        UINT sysDpi = GetDpiForSystem_Orig();
        if (sysDpi > 0) {
            actualMultiplier = (96.0 * g_scaleMultiplier) / (double)sysDpi;
        }
    }

    if (uiAction == SPI_GETNONCLIENTMETRICS && pvParam) {
        NONCLIENTMETRICSW* ncm = (NONCLIENTMETRICSW*)pvParam;
        
        auto scaleFont = [actualMultiplier](LOGFONTW& lf) {
            lf.lfHeight = (LONG)(lf.lfHeight * actualMultiplier);
            if (lf.lfWidth != 0) lf.lfWidth = (LONG)(lf.lfWidth * actualMultiplier);
        };
        
        scaleFont(ncm->lfCaptionFont);
        scaleFont(ncm->lfSmCaptionFont);
        if (!g_excludeMenuBar) {
            scaleFont(ncm->lfMenuFont);
        }
        // Scale status or message fonts only for specific apps that need it to prevent double-scaling in DirectUI apps like WLM
        if (g_isMmc) {
            scaleFont(ncm->lfStatusFont);
            scaleFont(ncm->lfMessageFont);
        }
        
        auto scaleDim = [actualMultiplier](int& dim, int minVal = 1) {
            dim = (int)(dim * actualMultiplier);
            if (dim < minVal) dim = minVal;
        };
        
        scaleDim(ncm->iBorderWidth, 1);
        scaleDim(ncm->iScrollWidth, 4);
        scaleDim(ncm->iScrollHeight, 4);
        scaleDim(ncm->iCaptionWidth, 1);
        scaleDim(ncm->iCaptionHeight, 4);
        scaleDim(ncm->iSmCaptionWidth, 1);
        scaleDim(ncm->iSmCaptionHeight, 4);
        if (!g_excludeMenuBar) {
            scaleDim(ncm->iMenuWidth, 1);
            scaleDim(ncm->iMenuHeight, 4);
        }
        
        // Check if cbSize is large enough to contain iPaddedBorderWidth (added in Vista)
        if (ncm->cbSize >= (unsigned)(offsetof(NONCLIENTMETRICSW, iPaddedBorderWidth) + sizeof(int))) {
            scaleDim(ncm->iPaddedBorderWidth, 0);
        }
    }
    else if (uiAction == SPI_GETICONTITLELOGFONT && pvParam) {
        LOGFONTW* lf = (LOGFONTW*)pvParam;
        lf->lfHeight = (LONG)(lf->lfHeight * actualMultiplier);
        if (lf->lfWidth != 0) lf->lfWidth = (LONG)(lf->lfWidth * actualMultiplier);
    }
    else if (uiAction == SPI_GETICONMETRICS && pvParam) {
        ICONMETRICSW* im = (ICONMETRICSW*)pvParam;
        if (im->cbSize >= (unsigned)(offsetof(ICONMETRICSW, lfFont) + sizeof(LOGFONTW))) {
            im->iHorzSpacing = (int)(im->iHorzSpacing * actualMultiplier);
            im->iVertSpacing = (int)(im->iVertSpacing * actualMultiplier);
            im->lfFont.lfHeight = (LONG)(im->lfFont.lfHeight * actualMultiplier);
            if (im->lfFont.lfWidth != 0) im->lfFont.lfWidth = (LONG)(im->lfFont.lfWidth * actualMultiplier);
        }
    }
    
    in_dpi_hook = false;
    return ret;
}

typedef BOOL(WINAPI* SystemParametersInfoA_t)(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni);
SystemParametersInfoA_t SystemParametersInfoA_Orig;

BOOL WINAPI SystemParametersInfoA_Hook(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni) {
    if (in_dpi_hook) {
        return SystemParametersInfoA_Orig(uiAction, uiParam, pvParam, fWinIni);
    }
    in_dpi_hook = true;
    void* retAddr = _ReturnAddress();
    if (IsInternalOSCall(retAddr)) {
        BOOL res = SystemParametersInfoA_Orig(uiAction, uiParam, pvParam, fWinIni);
        in_dpi_hook = false;
        return res;
    }
    
    BOOL ret = SystemParametersInfoA_Orig(uiAction, uiParam, pvParam, fWinIni);
    if (!ret) {
        in_dpi_hook = false;
        return ret;
    }
    
    double actualMultiplier = g_scaleMultiplier;
    if (GetDpiForSystem_Orig) {
        UINT sysDpi = GetDpiForSystem_Orig();
        if (sysDpi > 0) {
            actualMultiplier = (96.0 * g_scaleMultiplier) / (double)sysDpi;
        }
    }

    if (uiAction == SPI_GETNONCLIENTMETRICS && pvParam) {
        NONCLIENTMETRICSA* ncm = (NONCLIENTMETRICSA*)pvParam;
        
        auto scaleFont = [actualMultiplier](LOGFONTA& lf) {
            lf.lfHeight = (LONG)(lf.lfHeight * actualMultiplier);
            if (lf.lfWidth != 0) lf.lfWidth = (LONG)(lf.lfWidth * actualMultiplier);
        };
        
        scaleFont(ncm->lfCaptionFont);
        scaleFont(ncm->lfSmCaptionFont);
        if (!g_excludeMenuBar) {
            scaleFont(ncm->lfMenuFont);
        }
        // Scale status or message fonts only for specific apps that need it to prevent double-scaling in DirectUI apps like WLM
        if (g_isMmc) {
            scaleFont(ncm->lfStatusFont);
            scaleFont(ncm->lfMessageFont);
        }
        
        auto scaleDim = [actualMultiplier](int& dim, int minVal = 1) {
            dim = (int)(dim * actualMultiplier);
            if (dim < minVal) dim = minVal;
        };
        
        scaleDim(ncm->iBorderWidth, 1);
        scaleDim(ncm->iScrollWidth, 4);
        scaleDim(ncm->iScrollHeight, 4);
        scaleDim(ncm->iCaptionWidth, 1);
        scaleDim(ncm->iCaptionHeight, 4);
        scaleDim(ncm->iSmCaptionWidth, 1);
        scaleDim(ncm->iSmCaptionHeight, 4);
        if (!g_excludeMenuBar) {
            scaleDim(ncm->iMenuWidth, 1);
            scaleDim(ncm->iMenuHeight, 4);
        }
        
        if (ncm->cbSize >= (unsigned)(offsetof(NONCLIENTMETRICSA, iPaddedBorderWidth) + sizeof(int))) {
            scaleDim(ncm->iPaddedBorderWidth, 0);
        }
    }
    else if (uiAction == SPI_GETICONTITLELOGFONT && pvParam) {
        LOGFONTA* lf = (LOGFONTA*)pvParam;
        lf->lfHeight = (LONG)(lf->lfHeight * actualMultiplier);
        if (lf->lfWidth != 0) lf->lfWidth = (LONG)(lf->lfWidth * actualMultiplier);
    }
    else if (uiAction == SPI_GETICONMETRICS && pvParam) {
        ICONMETRICSA* im = (ICONMETRICSA*)pvParam;
        if (im->cbSize >= (unsigned)(offsetof(ICONMETRICSA, lfFont) + sizeof(LOGFONTA))) {
            im->iHorzSpacing = (int)(im->iHorzSpacing * actualMultiplier);
            im->iVertSpacing = (int)(im->iVertSpacing * actualMultiplier);
            im->lfFont.lfHeight = (LONG)(im->lfFont.lfHeight * actualMultiplier);
            if (im->lfFont.lfWidth != 0) im->lfFont.lfWidth = (LONG)(im->lfFont.lfWidth * actualMultiplier);
        }
    }
    
    in_dpi_hook = false;
    return ret;
}

// -------------------------------------------------------------------------
// UXTHEME HOOKS
// -------------------------------------------------------------------------

typedef int(WINAPI* GetThemeSysSize_t)(HANDLE hTheme, int iSizeId);
GetThemeSysSize_t GetThemeSysSize_Orig;

int WINAPI GetThemeSysSize_Hook(HANDLE hTheme, int iSizeId) {
    int metric = GetThemeSysSize_Orig(hTheme, iSizeId);
    if ((g_isMmc || g_isWlmail) && !in_dpi_hook && metric > 0) {
        in_dpi_hook = true;
        metric = (int)(metric * g_scaleMultiplier);
        in_dpi_hook = false;
    }
    return metric;
}

typedef HRESULT(WINAPI* GetThemeMetric_t)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, int iPropId, int* piVal);
GetThemeMetric_t GetThemeMetric_Orig;

HRESULT WINAPI GetThemeMetric_Hook(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, int iPropId, int* piVal) {
    HRESULT hr = GetThemeMetric_Orig(hTheme, hdc, iPartId, iStateId, iPropId, piVal);
    if ((g_isMmc || g_isWlmail) && SUCCEEDED(hr) && piVal) {
        // TMT_WIDTH=1201, TMT_HEIGHT=1202, TMT_TEXTSIZE=2403
        if (iPropId == 1201 || iPropId == 1202 || iPropId == 2403) {
            *piVal = (int)(*piVal * g_scaleMultiplier);
        }
    }
    return hr;
}

typedef HRESULT(WINAPI* GetThemePartSize_t)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT prc, THEMESIZE eSize, SIZE* psz);
GetThemePartSize_t GetThemePartSize_Orig;

HRESULT WINAPI GetThemePartSize_Hook(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT prc, THEMESIZE eSize, SIZE* psz) {
    HRESULT hr = GetThemePartSize_Orig(hTheme, hdc, iPartId, iStateId, prc, eSize, psz);
    if ((g_isMmc || g_isWlmail) && SUCCEEDED(hr) && psz) {
        psz->cx = (LONG)(psz->cx * g_scaleMultiplier);
        psz->cy = (LONG)(psz->cy * g_scaleMultiplier);
    }
    return hr;
}

typedef HRESULT(WINAPI* GetThemeMargins_t)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, int iPropId, LPCRECT prc, MARGINS* pMargins);
GetThemeMargins_t GetThemeMargins_Orig;

HRESULT WINAPI GetThemeMargins_Hook(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, int iPropId, LPCRECT prc, MARGINS* pMargins) {
    HRESULT hr = GetThemeMargins_Orig(hTheme, hdc, iPartId, iStateId, iPropId, prc, pMargins);
    if (g_isMmc && SUCCEEDED(hr) && pMargins) {
        pMargins->cxLeftWidth = (int)(pMargins->cxLeftWidth * g_scaleMultiplier);
        pMargins->cxRightWidth = (int)(pMargins->cxRightWidth * g_scaleMultiplier);
        pMargins->cyTopHeight = (int)(pMargins->cyTopHeight * g_scaleMultiplier);
        pMargins->cyBottomHeight = (int)(pMargins->cyBottomHeight * g_scaleMultiplier);
    }
    return hr;
}

typedef HRESULT(WINAPI* GetThemeRect_t)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, LPRECT pRect);
GetThemeRect_t GetThemeRect_Orig;

HRESULT WINAPI GetThemeRect_Hook(HTHEME hTheme, int iPartId, int iStateId, int iPropId, LPRECT pRect) {
    HRESULT hr = GetThemeRect_Orig(hTheme, iPartId, iStateId, iPropId, pRect);
    if (g_isMmc && SUCCEEDED(hr) && pRect) {
        pRect->left = (int)(pRect->left * g_scaleMultiplier);
        pRect->top = (int)(pRect->top * g_scaleMultiplier);
        pRect->right = (int)(pRect->right * g_scaleMultiplier);
        pRect->bottom = (int)(pRect->bottom * g_scaleMultiplier);
    }
    return hr;
}

// -------------------------------------------------------------------------
// IMAGE LIST HOOKS FOR MMC
// -------------------------------------------------------------------------

typedef HIMAGELIST(WINAPI* ImageList_Create_t)(int cx, int cy, UINT flags, int cInitial, int cGrow);
ImageList_Create_t ImageList_Create_Orig;

HIMAGELIST WINAPI ImageList_Create_Hook(int cx, int cy, UINT flags, int cInitial, int cGrow) {
    if (g_isMmc && cx > 0 && cy > 0) {
        cx = (int)(cx * g_scaleMultiplier);
        cy = (int)(cy * g_scaleMultiplier);
    }
    return ImageList_Create_Orig(cx, cy, flags, cInitial, cGrow);
}

// -------------------------------------------------------------------------
// MESSAGE HOOKS FOR MMC NAVPANE
// -------------------------------------------------------------------------

typedef int(WINAPI* GetDeviceCaps_t)(HDC hdc, int index);
GetDeviceCaps_t GetDeviceCaps_Orig;

typedef HRESULT(WINAPI* DrawThemeBackground_t)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pRect, LPCRECT pClipRect);
DrawThemeBackground_t DrawThemeBackground_Orig;

typedef HTHEME(WINAPI* OpenThemeData_t)(HWND hwnd, LPCWSTR pszClassList);
OpenThemeData_t OpenThemeData_Orig;

HTHEME WINAPI OpenThemeData_Hook(HWND hwnd, LPCWSTR pszClassList) {
    HTHEME hTheme = OpenThemeData_Orig(hwnd, pszClassList);
    if (hTheme && pszClassList) {
        std::lock_guard<std::mutex> lock(g_themeMutex);
        g_themeClasses[hTheme] = pszClassList;
    }
    return hTheme;
}

typedef HRESULT(WINAPI* CloseThemeData_t)(HTHEME hTheme);
CloseThemeData_t CloseThemeData_Orig;

HRESULT WINAPI CloseThemeData_Hook(HTHEME hTheme) {
    if (hTheme) {
        std::lock_guard<std::mutex> lock(g_themeMutex);
        g_themeClasses.erase(hTheme);
    }
    return CloseThemeData_Orig(hTheme);
}

HRESULT WINAPI DrawThemeBackground_Hook(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pRect, LPCRECT pClipRect) {
    if (g_isWlmail && pRect && hTheme) {
        std::wstring cls;
        {
            std::lock_guard<std::mutex> lock(g_themeMutex);
            auto it = g_themeClasses.find(hTheme);
            if (it != g_themeClasses.end()) cls = it->second;
        }
        
        // The artifact is a gap below the "File" button.
        // We'll try to find the ribbon background color more accurately.
        if (!cls.empty() && (cls.find(L"Ribbon") != std::wstring::npos || 
                           cls.find(L"AppMenu") != std::wstring::npos)) {
            
            if (pRect->left <= 2 && pRect->top < 150 && iStateId == 1) {
                // Use a slightly more accurate blue for WLM Ribbon File button area.
                COLORREF wlmBlue = RGB(19, 122, 247); 
                HBRUSH br = CreateSolidBrush(wlmBlue);
                
                RECT rc = *pRect;
                rc.bottom += 15; 
                rc.right += 2;
                FillRect(hdc, &rc, br);
                DeleteObject(br);
                
                if (iPartId == 1) return S_OK;
            }
        }
    }
    return DrawThemeBackground_Orig(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);
}

int WINAPI GetDeviceCaps_Hook(HDC hdc, int index) {
    if (index == LOGPIXELSX || index == LOGPIXELSY) {
        if (in_dpi_hook) {
            return GetDeviceCaps_Orig(hdc, index);
        }
        in_dpi_hook = true;
        int dpi = GetDeviceCaps_Orig(hdc, index);
        if (dpi > 0) {
            void* retAddr = _ReturnAddress();
            if (g_isMmc) {
                // For MMC and PowerDesk, we specifically want to scale even internal calls 
                // because the shell components often have mixed logic.
                int scaled = (int)(dpi * g_scaleMultiplier);
                in_dpi_hook = false;
                return scaled;
            }
            if (g_isWlmail && IsInternalOSCall(retAddr)) {
                in_dpi_hook = false;
                return dpi; // Bypass for user32/gdi32/comctl32/uxtheme internal calls
            }
            UINT adj = GetAdjustedDpiForModule(retAddr);
            if (adj > 0) {
                in_dpi_hook = false;
                return (int)adj;
            }
            int resultDpi = (int)(96.0 * g_scaleMultiplier);
            in_dpi_hook = false;
            return resultDpi;
        }
        in_dpi_hook = false;
    }
    return GetDeviceCaps_Orig(hdc, index);
}

typedef HRESULT(WINAPI* GetDpiForMonitor_t)(HMONITOR hmonitor, MONITOR_DPI_TYPE dpiType, UINT* dpiX, UINT* dpiY);
GetDpiForMonitor_t GetDpiForMonitor_Orig;

HRESULT WINAPI GetDpiForMonitor_Hook(HMONITOR hmonitor, MONITOR_DPI_TYPE dpiType, UINT* dpiX, UINT* dpiY) {
    if (in_dpi_hook) {
        return GetDpiForMonitor_Orig(hmonitor, dpiType, dpiX, dpiY);
    }
    in_dpi_hook = true;
    HRESULT hr = GetDpiForMonitor_Orig(hmonitor, dpiType, dpiX, dpiY);
    void* retAddr = _ReturnAddress();
    if (IsInternalOSCall(retAddr)) {
        in_dpi_hook = false;
        return hr;
    }
    if (SUCCEEDED(hr)) {
        if (dpiX) *dpiX = (UINT)(*dpiX * g_scaleMultiplier);
        if (dpiY) *dpiY = (UINT)(*dpiY * g_scaleMultiplier);
    }
    in_dpi_hook = false;
    return hr;
}

typedef int(WINAPI* GetScaleFactorForMonitor_t)(HMONITOR hMon, int* pScale);
GetScaleFactorForMonitor_t GetScaleFactorForMonitor_Orig;

HRESULT WINAPI GetScaleFactorForMonitor_Hook(HMONITOR hMon, int* pScale) {
    HRESULT hr = GetScaleFactorForMonitor_Orig(hMon, pScale);
    if (SUCCEEDED(hr) && pScale) {
        *pScale = (int)(*pScale * g_scaleMultiplier);
    }
    return hr;
}

typedef void(WINAPI* D2D1GetDesktopDpi_t)(FLOAT *dpiX, FLOAT *dpiY);
D2D1GetDesktopDpi_t D2D1GetDesktopDpi_Orig;

void WINAPI D2D1GetDesktopDpi_Hook(FLOAT *dpiX, FLOAT *dpiY) {
    D2D1GetDesktopDpi_Orig(dpiX, dpiY);
    if (dpiX) *dpiX *= (FLOAT)g_scaleMultiplier;
    if (dpiY) *dpiY *= (FLOAT)g_scaleMultiplier;
}

// -------------------------------------------------------------------------
// MORE COMPAT HOOKS (Removed for simplicity and stability)
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
// MESSAGE HOOKS FOR MMC NAVPANE
// -------------------------------------------------------------------------

typedef LRESULT(WINAPI* SendMessageW_t)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
SendMessageW_t SendMessageW_Orig;

typedef LRESULT(WINAPI* SendMessageA_t)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
SendMessageA_t SendMessageA_Orig;

LRESULT WINAPI SendMessageW_Hook(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    if (g_isMmc) {
        if (Msg == WM_SETFONT && wParam) {
            HFONT hScaled = GetOrCreateScaledFont((HFONT)wParam);
            if (hScaled) wParam = (WPARAM)hScaled;
        }

        if (Msg == TVM_SETITEMHEIGHT || Msg == TVM_SETINDENT ||
            Msg == TB_SETBUTTONSIZE || Msg == TB_SETBITMAPSIZE ||
            Msg == TB_SETPADDING || Msg == TB_SETINDENT) {
            
            wchar_t className[256];
            if (GetClassNameW(hWnd, className, ARRAYSIZE(className)) > 0) {
                std::wstring clsName(className);
                if (CaseInsensitiveContains(clsName, L"SysTreeView32")) {
                    if (Msg == TVM_SETITEMHEIGHT || Msg == TVM_SETINDENT) {
                        int val = (int)wParam;
                        if (val > 0 && val < 200) { 
                            wParam = (WPARAM)(int)(val * g_scaleMultiplier);
                        }
                    }
                } else if (CaseInsensitiveContains(clsName, L"ToolbarWindow32")) {
                    if (Msg == TB_SETBUTTONSIZE || Msg == TB_SETBITMAPSIZE) {
                        int cx = LOWORD(lParam);
                        int cy = HIWORD(lParam);
                        if (cx > 0 && cy > 0 && cx < 300) { 
                            lParam = MAKELPARAM((int)(cx * g_scaleMultiplier), (int)(cy * g_scaleMultiplier));
                        }
                    } else if (Msg == TB_SETPADDING) {
                        int padding = (int)lParam;
                        lParam = (LPARAM)(int)(padding * g_scaleMultiplier);
                    }
                }
            }
        }
    }
    LRESULT res = SendMessageW_Orig(hWnd, Msg, wParam, lParam);
    
    // Post-processing for TreeView to ensure height is recalculated after font change
    if (g_isMmc && Msg == WM_SETFONT) {
        char cls[256];
        if (GetClassNameA(hWnd, cls, sizeof(cls)) && strstr(cls, "SysTreeView32")) {
             SendMessageA_Orig(hWnd, TVM_SETITEMHEIGHT, (WPARAM)-1, 0);
        }
    }
    return res;
}

LRESULT WINAPI SendMessageA_Hook(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    if (g_isMmc) {
        if (Msg == WM_SETFONT && wParam) {
            HFONT hScaled = GetOrCreateScaledFont((HFONT)wParam);
            if (hScaled) wParam = (WPARAM)hScaled;
        }

        if (Msg == TVM_SETITEMHEIGHT || Msg == TVM_SETINDENT ||
            Msg == TB_SETBUTTONSIZE || Msg == TB_SETBITMAPSIZE ||
            Msg == TB_SETPADDING || Msg == TB_SETINDENT) {
            
            char className[256];
            if (GetClassNameA(hWnd, className, ARRAYSIZE(className)) > 0) {
                std::string clsName(className);
                std::transform(clsName.begin(), clsName.end(), clsName.begin(), ::tolower);
                if (clsName.find("systreeview32") != std::string::npos) {
                    if (Msg == TVM_SETITEMHEIGHT || Msg == TVM_SETINDENT) {
                        int val = (int)wParam;
                        if (val > 0 && val < 200) { 
                            wParam = (WPARAM)(int)(val * g_scaleMultiplier);
                        }
                    }
                } else if (clsName.find("toolbarwindow32") != std::string::npos) {
                    if (Msg == TB_SETBUTTONSIZE || Msg == TB_SETBITMAPSIZE) {
                        int cx = LOWORD(lParam);
                        int cy = HIWORD(lParam);
                        if (cx > 0 && cy > 0 && cx < 300) {
                            lParam = MAKELPARAM((int)(cx * g_scaleMultiplier), (int)(cy * g_scaleMultiplier));
                        }
                    } else if (Msg == TB_SETPADDING) {
                        int padding = (int)lParam;
                        lParam = (LPARAM)(int)(padding * g_scaleMultiplier);
                    }
                }
            }
        }
    }
    LRESULT res = SendMessageA_Orig(hWnd, Msg, wParam, lParam);
    if (g_isMmc && Msg == WM_SETFONT) {
        char cls[256];
        if (GetClassNameA(hWnd, cls, sizeof(cls)) && strstr(cls, "SysTreeView32")) {
             SendMessageA_Orig(hWnd, TVM_SETITEMHEIGHT, (WPARAM)-1, 0);
        }
    }
    return res;
}

// -------------------------------------------------------------------------
// CHROMIUM COMMAND LINE INJECTION
// -------------------------------------------------------------------------

typedef LPWSTR(WINAPI* GetCommandLineW_t)();
GetCommandLineW_t GetCommandLineW_Orig;

LPWSTR WINAPI GetCommandLineW_Hook() {
    static std::wstring fakeCmdLine;
    static bool initialized = false;

    LPWSTR orig = GetCommandLineW_Orig();

    if (!initialized && orig != nullptr) {
        initialized = true;
        fakeCmdLine = orig;
        
        // Append force-device-scale-factor if not already present
        if (fakeCmdLine.find(L"--force-device-scale-factor") == std::wstring::npos) {
            wchar_t appendBuf[256];
            swprintf_s(appendBuf, ARRAYSIZE(appendBuf), L" --force-device-scale-factor=%.3f", g_scaleMultiplier);
            fakeCmdLine += appendBuf;
        }
    }
    
    if (initialized) {
        return (LPWSTR)fakeCmdLine.c_str();
    }
    return orig;
}

// -------------------------------------------------------------------------
// MOD INITIALIZATION
// -------------------------------------------------------------------------

void LoadSettings() {
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    std::wstring exeName = exePath;
    size_t lastSlash = exeName.find_last_of(L"\\/");
    if (lastSlash != std::wstring::npos) {
        exeName = exeName.substr(lastSlash + 1);
    }

    g_shouldApply = false;
    g_isWlmail = CaseInsensitiveContains(exeName, L"wlmail.exe");
    g_isMmc = CaseInsensitiveContains(exeName, L"mmc.exe");
    g_isNotepad = CaseInsensitiveContains(exeName, L"notepad.exe");

    for (int i = 0;; i++) {
        wchar_t key[128];
        swprintf_s(key, ARRAYSIZE(key), L"apps[%d].processName", i);
        PCWSTR processName = Wh_GetStringSetting(key);
        if (!processName) {
            break; // No more items
        }
            
        std::wstring target(processName);
        Wh_FreeStringSetting(processName);
        
        // Trim target string
        target.erase(0, target.find_first_not_of(L" \t"));
        target.erase(target.find_last_not_of(L" \t") + 1);
        
        if (!target.empty() && CaseInsensitiveContains(exeName, target)) {
            swprintf_s(key, ARRAYSIZE(key), L"apps[%d].scalePercentage", i);
            int percentage = Wh_GetIntSetting(key);
            if (percentage > 0) {
                g_scaleMultiplier = (double)percentage / 100.0;
            } else {
                g_scaleMultiplier = 1.0;
            }
            if (g_scaleMultiplier <= 0.1) g_scaleMultiplier = 1.0;
            
            swprintf_s(key, ARRAYSIZE(key), L"apps[%d].enableChromiumCmdLine", i);
            g_enableChromiumCmdLine = Wh_GetIntSetting(key) != 0;
            
            swprintf_s(key, ARRAYSIZE(key), L"apps[%d].enableDpiHooks", i);
            g_enableDpiHooks = Wh_GetIntSetting(key) != 0;
            
            swprintf_s(key, ARRAYSIZE(key), L"apps[%d].excludeMenuBar", i);
            g_excludeMenuBar = Wh_GetIntSetting(key) != 0;
            
            g_shouldApply = true;
            break; // Match found, stop checking
        }
    }
}

BOOL Wh_ModInit() {
    LoadSettings();

    if (!g_shouldApply) {
        return TRUE;
    }

    HMODULE hUser32 = GetModuleHandle(L"user32.dll");
    if (hUser32) {
        MODULEINFO mi = {0};
        if (GetModuleInformation(GetCurrentProcess(), hUser32, &mi, sizeof(mi))) {
            g_user32_start = mi.lpBaseOfDll;
            g_user32_end = (PBYTE)mi.lpBaseOfDll + mi.SizeOfImage;
        }
    }

    HMODULE hGdi32 = GetModuleHandle(L"gdi32.dll");
    if (hGdi32) {
        MODULEINFO mi = {0};
        if (GetModuleInformation(GetCurrentProcess(), hGdi32, &mi, sizeof(mi))) {
            g_gdi32_start = mi.lpBaseOfDll;
            g_gdi32_end = (PBYTE)mi.lpBaseOfDll + mi.SizeOfImage;
        }
    }

    HMODULE hComctl32 = GetModuleHandle(L"comctl32.dll");
    if (hComctl32) {
        MODULEINFO mi = {0};
        if (GetModuleInformation(GetCurrentProcess(), hComctl32, &mi, sizeof(mi))) {
            g_comctl32_start = mi.lpBaseOfDll;
            g_comctl32_end = (PBYTE)mi.lpBaseOfDll + mi.SizeOfImage;
        }
    }

    HMODULE hUxTheme = GetModuleHandle(L"uxtheme.dll");
    if (hUxTheme) {
        MODULEINFO mi = {0};
        if (GetModuleInformation(GetCurrentProcess(), hUxTheme, &mi, sizeof(mi))) {
            g_uxtheme_start = mi.lpBaseOfDll;
            g_uxtheme_end = (PBYTE)mi.lpBaseOfDll + mi.SizeOfImage;
        }
    }

    if (g_enableDpiHooks) {
        if (hUser32) {
            void* pGetDpiForWindow = (void*)GetProcAddress(hUser32, "GetDpiForWindow");
            if (pGetDpiForWindow) {
                Wh_SetFunctionHook(pGetDpiForWindow,
                                   (void*)GetDpiForWindow_Hook,
                                   (void**)&GetDpiForWindow_Orig);
            }
            void* pGetDpiForSystem = (void*)GetProcAddress(hUser32, "GetDpiForSystem");
            if (pGetDpiForSystem) {
                Wh_SetFunctionHook(pGetDpiForSystem,
                                   (void*)GetDpiForSystem_Hook,
                                   (void**)&GetDpiForSystem_Orig);
            }
            void* pGetSystemMetricsForDpi = (void*)GetProcAddress(hUser32, "GetSystemMetricsForDpi");
            if (pGetSystemMetricsForDpi) {
                Wh_SetFunctionHook(pGetSystemMetricsForDpi,
                                   (void*)GetSystemMetricsForDpi_Hook,
                                   (void**)&GetSystemMetricsForDpi_Orig);
            }
            void* pGetSystemMetrics = (void*)GetProcAddress(hUser32, "GetSystemMetrics");
            if (pGetSystemMetrics) {
                Wh_SetFunctionHook(pGetSystemMetrics,
                                   (void*)GetSystemMetrics_Hook,
                                   (void**)&GetSystemMetrics_Orig);
            }
            void* pSystemParametersInfoForDpi = (void*)GetProcAddress(hUser32, "SystemParametersInfoForDpi");
            if (pSystemParametersInfoForDpi) {
                // Hook removed
            }
            void* pSystemParametersInfoW = (void*)GetProcAddress(hUser32, "SystemParametersInfoW");
            if (pSystemParametersInfoW) {
                Wh_SetFunctionHook(pSystemParametersInfoW,
                                   (void*)SystemParametersInfoW_Hook,
                                   (void**)&SystemParametersInfoW_Orig);
            }
            void* pSystemParametersInfoA = (void*)GetProcAddress(hUser32, "SystemParametersInfoA");
            if (pSystemParametersInfoA) {
                Wh_SetFunctionHook(pSystemParametersInfoA,
                                   (void*)SystemParametersInfoA_Hook,
                                   (void**)&SystemParametersInfoA_Orig);
            }
            void* pSendMessageW = (void*)GetProcAddress(hUser32, "SendMessageW");
            if (pSendMessageW) {
                Wh_SetFunctionHook(pSendMessageW,
                                   (void*)SendMessageW_Hook,
                                   (void**)&SendMessageW_Orig);
            }
            void* pSendMessageA = (void*)GetProcAddress(hUser32, "SendMessageA");
            if (pSendMessageA) {
                Wh_SetFunctionHook(pSendMessageA,
                                   (void*)SendMessageA_Hook,
                                   (void**)&SendMessageA_Orig);
            }
        }

        HMODULE hGdi32 = GetModuleHandle(L"gdi32.dll");
        if (hGdi32) {
            void* pGetDeviceCaps = (void*)GetProcAddress(hGdi32, "GetDeviceCaps");
            if (pGetDeviceCaps) {
                Wh_SetFunctionHook(pGetDeviceCaps,
                                   (void*)GetDeviceCaps_Hook,
                                   (void**)&GetDeviceCaps_Orig);
            }
            void* pCreateFontIndirectW = (void*)GetProcAddress(hGdi32, "CreateFontIndirectW");
            if (pCreateFontIndirectW) {
                Wh_SetFunctionHook(pCreateFontIndirectW,
                                   (void*)CreateFontIndirectW_Hook,
                                   (void**)&CreateFontIndirectW_Orig);
            }
            void* pCreateFontW = (void*)GetProcAddress(hGdi32, "CreateFontW");
            if (pCreateFontW) {
                Wh_SetFunctionHook(pCreateFontW,
                                   (void*)CreateFontW_Hook,
                                   (void**)&CreateFontW_Orig);
            }
        }

        if (hUxTheme) {
            void* pGetThemeSysSize = (void*)GetProcAddress(hUxTheme, "GetThemeSysSize");
            if (pGetThemeSysSize) {
                Wh_SetFunctionHook(pGetThemeSysSize,
                                   (void*)GetThemeSysSize_Hook,
                                   (void**)&GetThemeSysSize_Orig);
            }
            void* pOpenThemeData = (void*)GetProcAddress(hUxTheme, "OpenThemeData");
            if (pOpenThemeData) {
                Wh_SetFunctionHook(pOpenThemeData,
                                   (void*)OpenThemeData_Hook,
                                   (void**)&OpenThemeData_Orig);
            }
            void* pCloseThemeData = (void*)GetProcAddress(hUxTheme, "CloseThemeData");
            if (pCloseThemeData) {
                Wh_SetFunctionHook(pCloseThemeData,
                                   (void*)CloseThemeData_Hook,
                                   (void**)&CloseThemeData_Orig);
            }
            void* pDrawThemeBackground = (void*)GetProcAddress(hUxTheme, "DrawThemeBackground");
            if (pDrawThemeBackground) {
                Wh_SetFunctionHook(pDrawThemeBackground,
                                   (void*)DrawThemeBackground_Hook,
                                   (void**)&DrawThemeBackground_Orig);
            }
            void* pGetThemeMetric = (void*)GetProcAddress(hUxTheme, "GetThemeMetric");
            if (pGetThemeMetric) {
                Wh_SetFunctionHook(pGetThemeMetric,
                                   (void*)GetThemeMetric_Hook,
                                   (void**)&GetThemeMetric_Orig);
            }
            void* pGetThemePartSize = (void*)GetProcAddress(hUxTheme, "GetThemePartSize");
            if (pGetThemePartSize) {
                Wh_SetFunctionHook(pGetThemePartSize,
                                   (void*)GetThemePartSize_Hook,
                                   (void**)&GetThemePartSize_Orig);
            }
            void* pGetThemeMargins = (void*)GetProcAddress(hUxTheme, "GetThemeMargins");
            if (pGetThemeMargins) {
                Wh_SetFunctionHook(pGetThemeMargins,
                                   (void*)GetThemeMargins_Hook,
                                   (void**)&GetThemeMargins_Orig);
            }
            void* pGetThemeRect = (void*)GetProcAddress(hUxTheme, "GetThemeRect");
            if (pGetThemeRect) {
                Wh_SetFunctionHook(pGetThemeRect,
                                   (void*)GetThemeRect_Hook,
                                   (void**)&GetThemeRect_Orig);
            }
        }

        if (hUser32) {
            void* pLoadImageW = (void*)GetProcAddress(hUser32, "LoadImageW");
            if (pLoadImageW) {
                Wh_SetFunctionHook(pLoadImageW,
                                   (void*)LoadImageW_Hook,
                                   (void**)&LoadImageW_Orig);
            }
            void* pLoadImageA = (void*)GetProcAddress(hUser32, "LoadImageA");
            if (pLoadImageA) {
                Wh_SetFunctionHook(pLoadImageA,
                                   (void*)LoadImageA_Hook,
                                   (void**)&LoadImageA_Orig);
            }
        }

        HMODULE hComctl32 = GetModuleHandle(L"comctl32.dll");
        if (hComctl32) {
            void* pImageList_Create = (void*)GetProcAddress(hComctl32, "ImageList_Create");
            if (pImageList_Create) {
                Wh_SetFunctionHook(pImageList_Create,
                                   (void*)ImageList_Create_Hook,
                                   (void**)&ImageList_Create_Orig);
            }
        }

        HMODULE hShcore = GetModuleHandle(L"shcore.dll");
        if (hShcore) {
            void* pGetDpiForMonitor = (void*)GetProcAddress(hShcore, "GetDpiForMonitor");
            if (pGetDpiForMonitor) {
                Wh_SetFunctionHook(pGetDpiForMonitor,
                                   (void*)GetDpiForMonitor_Hook,
                                   (void**)&GetDpiForMonitor_Orig);
            }
            void* pGetScaleFactorForMonitor = (void*)GetProcAddress(hShcore, "GetScaleFactorForMonitor");
            if (pGetScaleFactorForMonitor) {
                Wh_SetFunctionHook(pGetScaleFactorForMonitor,
                                   (void*)GetScaleFactorForMonitor_Hook,
                                   (void**)&GetScaleFactorForMonitor_Orig);
            }
        }
        
        HMODULE hD2D1 = GetModuleHandle(L"d2d1.dll");
        if (hD2D1) {
            void* pD2D1GetDesktopDpi = (void*)GetProcAddress(hD2D1, "D2D1GetDesktopDpi");
            if (pD2D1GetDesktopDpi) {
                Wh_SetFunctionHook(pD2D1GetDesktopDpi,
                                   (void*)D2D1GetDesktopDpi_Hook,
                                   (void**)&D2D1GetDesktopDpi_Orig);
            }
        }
    }

    if (g_enableChromiumCmdLine) {
        HMODULE hKernel32 = GetModuleHandle(L"kernel32.dll");
        if (hKernel32) {
            void* pGetCommandLineW = (void*)GetProcAddress(hKernel32, "GetCommandLineW");
            if (pGetCommandLineW) {
                Wh_SetFunctionHook(pGetCommandLineW,
                                   (void*)GetCommandLineW_Hook,
                                   (void**)&GetCommandLineW_Orig);
            }
        }
    }

    return TRUE;
}

void Wh_ModSettingsChanged() {
    LoadSettings();
}
