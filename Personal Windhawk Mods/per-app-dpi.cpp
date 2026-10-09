// ==WindhawkMod==
// @id              per-app-dpi-override
// @name            Per-App Custom DPI Override
// @description     Overrides the DPI scaling factor for specific applications.
// @version         2.5.0
// @author          bbmaster123
// @compilerOptions -DWINVER=0x0A00 -lshcore -lpsapi -lcomctl32 -lgdi32 -lole32 -loleaut32 -lruntimeobject
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
This mod allows you to artificially increase or decrease the DPI scaling factor for specific applications.
Normally, Windows uses the globally configured display DPI (like 100%, 150%, 200%).
This hooks application DPI queries to report a higher or lower scale factor.

### System Safety & Process Inclusion
To maintain high system stability and performance, **this mod does not automatically inject into all processes**. By default, it only injects into common applications.

If you want to scale a custom application:
1. Open the **Windhawk** interface on your computer.
2. Under the **Per-App Custom DPI Override** mod, go to **Details**.
3. Under the **Advanced** tab, find the **Process Inclusion/Exclusion** list.
4. Add your application's executable name (e.g., `myprogram.exe`) to the inclusion list.
5. In the mod's standard settings, add an entry for `myprogram.exe` with your custom scale percentage (e.g., `125` for 125%).

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
      $description: "The execution name to target. e.g., msedge.exe, notepad.exe, wlmail.exe, regedit.exe, mmc.exe, calc.exe, SystemSettings.exe"
    - scalePercentage: 111
      $name: Scale Percentage
      $description: "The main DPI scale factor as a percentage to apply (e.g., 125 for 125% scale, 80 for 80% scale)."
    - enableChromiumCmdLine: true
      $name: Enable Chromium Command Line Injection
      $description: "Automatically injects --force-device-scale-factor for Chromium browsers. Note: You must fully close the browser first."
    - enableDpiHooks: true
      $name: Enable Windows DPI API Hooks
      $description: "Hooks standard Windows DPI APIs (GetDpiForWindow, GetDeviceCaps, etc.). Disable if you only want Chromium scaling or if it causes instability."
    - excludeMenuBar: false
      $name: Exclude Menu Bar from Scaling
      $description: "Prevents the menu bar and menu fonts from being scaled (useful for some legacy apps)."
    - enableGdiFontHooks: false
      $name: Enable Gdi32 Font Scaling
      $description: "Hooks GDI font creation (CreateFont, CreateFontIndirect) to scale fonts dynamically (crucial for console, MMC tree views, and legacy dialogs)."
    - fontScalePercentage: 0
      $name: Custom Font Scale Percentage (0 = follow App Scale)
      $description: "Separate scale factor for fonts (e.g., 125 for 125% font size). If set to 0 or left blank, it automatically follows the main Scale Percentage."
    - enableGdiImageHooks: false
      $name: Enable Gdi32/Comctl32 Image Scaling
      $description: "Hooks image loading (LoadImage) and ImageList creation to scale icons/bitmaps to prevent blurry or vanished toolbar buttons."
    - imageScalePercentage: 0
      $name: Custom Image Scale Percentage (0 = follow App Scale)
      $description: "Separate scale factor for icons and images (e.g., 125 for 125% image size). If set to 0 or left blank, it automatically follows the main Scale Percentage."
    - enableThemeHooks: false
      $name: Enable UxTheme Metric Scaling
      $description: "Hooks theme engine metrics (GetThemeSysSize, GetThemeMetric, etc.) to scale controls and containers in themes."
    - enableAdjustWindowRectHooks: false
      $name: Enable Non-Client Layout Sizing Hooks
      $description: "Hooks AdjustWindowRect and AdjustWindowRectEx to scale the window non-client frame. This prevents menu bars from being cut off on top."
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
#include <cmath>
#include <cstdlib>
#include <stdio.h>
#include <intrin.h>
#include <psapi.h>
#include <commctrl.h>
#include <uxtheme.h>
#include <unordered_set>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <cstdarg>

#define LOG_TAG L"PerAppDpi"

static void ModLog(const WCHAR* format, ...) {
    WCHAR buffer[1024];
    va_list args;
    va_start(args, format);
    _vsnwprintf_s(buffer, ARRAYSIZE(buffer), _TRUNCATE, format, args);
    va_end(args);

    Wh_Log(L"[%s] %s", LOG_TAG, buffer);
    OutputDebugStringW(buffer);
}

#if defined(__GNUC__) || defined(__clang__)
#define _ReturnAddress() __builtin_return_address(0)
#else
#pragma intrinsic(_ReturnAddress)
#endif

double g_scaleMultiplier = 1.0;
double g_fontScaleMultiplier = 1.0;
double g_imageScaleMultiplier = 1.0;
bool g_enableChromiumCmdLine = false;
bool g_enableDpiHooks = false;
bool g_excludeMenuBar = false;
bool g_enableGdiFontHooks = false;
bool g_enableGdiImageHooks = false;
bool g_enableThemeHooks = false;
bool g_enableAdjustWindowRectHooks = false;
bool g_shouldApply = false;
bool g_isWlmail = false;
bool g_isMmc = false;
bool g_isNotepad = false;
bool g_isOlk = false;
static bool g_hooksInstalled = false;

int MapToValidDeviceScaleFactor(double multiplier) {
    int percentage = (int)(multiplier * 100.0 + 0.5);
    int validScales[] = { 100, 120, 125, 140, 150, 160, 175, 180, 200, 225, 250, 300, 350, 400, 450, 500 };
    int bestScale = 100;
    int minDiff = 1000;
    for (int scale : validScales) {
        int diff = abs(percentage - scale);
        if (diff < minDiff) {
            minDiff = diff;
            bestScale = scale;
        }
    }
    return bestScale;
}

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

typedef UINT(WINAPI* GetDpiForSystem_t)();
GetDpiForSystem_t GetDpiForSystem_Orig;

typedef int(WINAPI* GetSystemMetricsForDpi_t)(int nIndex, UINT dpi);
GetSystemMetricsForDpi_t GetSystemMetricsForDpi_Orig;

typedef int(WINAPI* GetScaleFactorForDevice_t)(int deviceType);
GetScaleFactorForDevice_t GetScaleFactorForDevice_Orig;

typedef BOOL(WINAPI* AdjustWindowRect_t)(LPRECT lpRect, DWORD dwStyle, BOOL bMenu);
AdjustWindowRect_t AdjustWindowRect_Orig;

typedef BOOL(WINAPI* AdjustWindowRectEx_t)(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle);
AdjustWindowRectEx_t AdjustWindowRectEx_Orig;

typedef BOOL(WINAPI* AdjustWindowRectExForDpi_t)(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle, UINT dpi);
AdjustWindowRectExForDpi_t AdjustWindowRectExForDpi_Orig;

int WINAPI GetSystemMetricsForDpi_Hook(int nIndex, UINT dpi) {
    if (!g_shouldApply || !g_enableDpiHooks || in_dpi_hook || g_scaleMultiplier == 1.0) {
        return GetSystemMetricsForDpi_Orig(nIndex, dpi);
    }
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

int WINAPI GetScaleFactorForDevice_Hook(int deviceType) {
    if (!g_shouldApply || !g_enableDpiHooks || in_dpi_hook || g_scaleMultiplier == 1.0) {
        return GetScaleFactorForDevice_Orig(deviceType);
    }
    in_dpi_hook = true;
    int origScale = GetScaleFactorForDevice_Orig(deviceType);
    int baseScale = (origScale > 0) ? origScale : 100;
    double effectiveMultiplier = ((double)baseScale * g_scaleMultiplier) / 100.0;
    int scale = MapToValidDeviceScaleFactor(effectiveMultiplier);
    in_dpi_hook = false;
    return scale;
}

BOOL WINAPI AdjustWindowRect_Hook(LPRECT lpRect, DWORD dwStyle, BOOL bMenu) {
    if (!lpRect || !g_shouldApply || !g_enableAdjustWindowRectHooks || in_dpi_hook || g_scaleMultiplier == 1.0) {
        return AdjustWindowRect_Orig(lpRect, dwStyle, bMenu);
    }
    in_dpi_hook = true;
    RECT origClient = *lpRect;
    BOOL res = AdjustWindowRect_Orig(lpRect, dwStyle, bMenu);
    if (res && !IsInternalOSCall(_ReturnAddress())) {
        int leftMargin = origClient.left - lpRect->left;
        int rightMargin = lpRect->right - origClient.right;
        int topMargin = origClient.top - lpRect->top;
        int bottomMargin = lpRect->bottom - origClient.bottom;

        leftMargin = (int)(leftMargin * g_scaleMultiplier);
        rightMargin = (int)(rightMargin * g_scaleMultiplier);
        topMargin = (int)(topMargin * g_scaleMultiplier);
        bottomMargin = (int)(bottomMargin * g_scaleMultiplier);

        lpRect->left = origClient.left - leftMargin;
        lpRect->right = origClient.right + rightMargin;
        lpRect->top = origClient.top - topMargin;
        lpRect->bottom = origClient.bottom + bottomMargin;
    }
    in_dpi_hook = false;
    return res;
}

BOOL WINAPI AdjustWindowRectEx_Hook(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle) {
    if (!lpRect || !g_shouldApply || !g_enableAdjustWindowRectHooks || in_dpi_hook || g_scaleMultiplier == 1.0) {
        return AdjustWindowRectEx_Orig(lpRect, dwStyle, bMenu, dwExStyle);
    }
    in_dpi_hook = true;
    RECT origClient = *lpRect;
    BOOL res = AdjustWindowRectEx_Orig(lpRect, dwStyle, bMenu, dwExStyle);
    if (res && !IsInternalOSCall(_ReturnAddress())) {
        int leftMargin = origClient.left - lpRect->left;
        int rightMargin = lpRect->right - origClient.right;
        int topMargin = origClient.top - lpRect->top;
        int bottomMargin = lpRect->bottom - origClient.bottom;

        leftMargin = (int)(leftMargin * g_scaleMultiplier);
        rightMargin = (int)(rightMargin * g_scaleMultiplier);
        topMargin = (int)(topMargin * g_scaleMultiplier);
        bottomMargin = (int)(bottomMargin * g_scaleMultiplier);

        lpRect->left = origClient.left - leftMargin;
        lpRect->right = origClient.right + rightMargin;
        lpRect->top = origClient.top - topMargin;
        lpRect->bottom = origClient.bottom + bottomMargin;
    }
    in_dpi_hook = false;
    return res;
}

BOOL WINAPI AdjustWindowRectExForDpi_Hook(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle, UINT dpi) {
    if (!lpRect || !g_shouldApply || !g_enableAdjustWindowRectHooks || in_dpi_hook || g_scaleMultiplier == 1.0) {
        return AdjustWindowRectExForDpi_Orig(lpRect, dwStyle, bMenu, dwExStyle, dpi);
    }
    in_dpi_hook = true;

    UINT targetDpi = (UINT)(96.0 * g_scaleMultiplier);

    BOOL res = AdjustWindowRectExForDpi_Orig(lpRect, dwStyle, bMenu, dwExStyle, targetDpi);
    in_dpi_hook = false;
    return res;
}

#ifndef TVM_SETITEMHEIGHT
#define TVM_SETITEMHEIGHT (TV_FIRST + 27)
#endif

static void InitModuleRange(HMODULE hMod, void** pStart, void** pEnd) {
    if (!hMod || !pStart || !pEnd) return;
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)hMod;
    if (dosHeader->e_magic == IMAGE_DOS_SIGNATURE) {
        PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((PBYTE)hMod + dosHeader->e_lfanew);
        if (ntHeaders->Signature == IMAGE_NT_SIGNATURE) {
            *pStart = (void*)hMod;
            *pEnd = (void*)((PBYTE)hMod + ntHeaders->OptionalHeader.SizeOfImage);
        }
    }
}

void UpdateModuleRanges() {
    if (!g_mshtml_start) {
        InitModuleRange(GetModuleHandle(L"mshtml.dll"), &g_mshtml_start, &g_mshtml_end);
    }
    if (!g_ieframe_start) {
        InitModuleRange(GetModuleHandle(L"ieframe.dll"), &g_ieframe_start, &g_ieframe_end);
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
// THREAD-SAFE FONT REGISTRY, CACHING & STOCK FONTS
// -------------------------------------------------------------------------

std::unordered_map<HFONT, HFONT> g_fontScaleCache; 
std::unordered_set<HFONT> g_scaledFonts;
std::recursive_mutex g_fontMutex;
std::unordered_map<HTHEME, std::wstring> g_themeClasses;
std::recursive_mutex g_themeMutex;

static HFONT g_stockGuiFont = NULL;
static HFONT g_stockSystemFont = NULL;
static HFONT g_stockSystemFixedFont = NULL;

typedef HFONT(WINAPI* CreateFontIndirectW_t)(const LOGFONTW* plf);
CreateFontIndirectW_t CreateFontIndirectW_Orig;

typedef int(WINAPI* GetObjectW_t)(HANDLE h, int c, LPVOID pv);
GetObjectW_t GetObjectW_Orig;

typedef BOOL(WINAPI* DeleteObject_t)(HGDIOBJ ho);
DeleteObject_t DeleteObject_Orig;

void CleanupStockFonts() {
    std::lock_guard<std::recursive_mutex> lock(g_fontMutex);
    if (g_stockGuiFont) {
        if (DeleteObject_Orig) DeleteObject_Orig(g_stockGuiFont);
        else DeleteObject(g_stockGuiFont);
        g_scaledFonts.erase(g_stockGuiFont);
        g_stockGuiFont = NULL;
    }
    if (g_stockSystemFont) {
        if (DeleteObject_Orig) DeleteObject_Orig(g_stockSystemFont);
        else DeleteObject(g_stockSystemFont);
        g_scaledFonts.erase(g_stockSystemFont);
        g_stockSystemFont = NULL;
    }
    if (g_stockSystemFixedFont) {
        if (DeleteObject_Orig) DeleteObject_Orig(g_stockSystemFixedFont);
        else DeleteObject(g_stockSystemFixedFont);
        g_scaledFonts.erase(g_stockSystemFixedFont);
        g_stockSystemFixedFont = NULL;
    }
    for (auto& pair : g_fontScaleCache) {
        if (pair.second && pair.second != pair.first) {
            if (DeleteObject_Orig) DeleteObject_Orig(pair.second);
            else DeleteObject(pair.second);
        }
    }
    g_fontScaleCache.clear();
    g_scaledFonts.clear();
}

HFONT GetOrCreateScaledFont(HFONT hOrigFont) {
    if (!hOrigFont || !g_shouldApply) return hOrigFont;
    double fontMult = (g_fontScaleMultiplier != 1.0) ? g_fontScaleMultiplier : g_scaleMultiplier;
    if (fontMult == 1.0) return hOrigFont;
    std::lock_guard<std::recursive_mutex> lock(g_fontMutex);
    
    // If it's already a scaled font, return it directly
    if (g_scaledFonts.find(hOrigFont) != g_scaledFonts.end()) {
        return hOrigFont;
    }
    
    auto it = g_fontScaleCache.find(hOrigFont);
    if (it != g_fontScaleCache.end()) {
        return it->second;
    }
    
    LOGFONTW lf = {0};
    if (GetObjectW_Orig && GetObjectW_Orig(hOrigFont, sizeof(lf), &lf)) {
        // Heuristic to avoid double-scaling:
        // Standard UI fonts are usually 8-12pt, which at 100% is around -11 to -16 pixels.
        // If it's already > 25 pixels, it's likely already scaled by a hooked DPI API.
        if (abs(lf.lfHeight) > 25) {
            return hOrigFont;
        }

        lf.lfHeight = (LONG)std::round(lf.lfHeight * fontMult);
        if (lf.lfWidth != 0) lf.lfWidth = (LONG)std::round(lf.lfWidth * fontMult);
        
        in_dpi_hook = true;
        HFONT hScaled = CreateFontIndirectW_Orig ? CreateFontIndirectW_Orig(&lf) : CreateFontIndirectW(&lf);
        in_dpi_hook = false;
        if (hScaled) {
            g_scaledFonts.insert(hScaled);
            g_fontScaleCache[hOrigFont] = hScaled;
            return hScaled;
        }
    }
    return hOrigFont;
}

// -------------------------------------------------------------------------
// GLOBAL FONT HOOKS AND SPOOFING
// -------------------------------------------------------------------------

HFONT WINAPI CreateFontIndirectW_Hook(const LOGFONTW* plf) {
    double fontMult = (g_fontScaleMultiplier != 1.0) ? g_fontScaleMultiplier : g_scaleMultiplier;
    if (plf && !in_dpi_hook && g_shouldApply && (g_isMmc || g_enableGdiFontHooks) && fontMult != 1.0) {
        in_dpi_hook = true;
        LOGFONTW lf = *plf;
        if (abs(lf.lfHeight) < 30 || g_enableGdiFontHooks) {
            lf.lfHeight = (LONG)std::round(lf.lfHeight * fontMult);
            if (lf.lfWidth != 0) lf.lfWidth = (LONG)std::round(lf.lfWidth * fontMult);
        }
        HFONT hFont = CreateFontIndirectW_Orig(&lf);
        if (hFont) {
            std::lock_guard<std::recursive_mutex> lock(g_fontMutex);
            g_scaledFonts.insert(hFont);
        }
        in_dpi_hook = false;
        return hFont;
    }
    return CreateFontIndirectW_Orig(plf);
}

typedef HFONT(WINAPI* CreateFontIndirectA_t)(const LOGFONTA* plf);
CreateFontIndirectA_t CreateFontIndirectA_Orig;

HFONT WINAPI CreateFontIndirectA_Hook(const LOGFONTA* plf) {
    double fontMult = (g_fontScaleMultiplier != 1.0) ? g_fontScaleMultiplier : g_scaleMultiplier;
    if (plf && !in_dpi_hook && g_shouldApply && (g_isMmc || g_enableGdiFontHooks) && fontMult != 1.0) {
        in_dpi_hook = true;
        LOGFONTA lf = *plf;
        if (abs(lf.lfHeight) < 30 || g_enableGdiFontHooks) {
            lf.lfHeight = (LONG)std::round(lf.lfHeight * fontMult);
            if (lf.lfWidth != 0) lf.lfWidth = (LONG)std::round(lf.lfWidth * fontMult);
        }
        HFONT hFont = CreateFontIndirectA_Orig(&lf);
        if (hFont) {
            std::lock_guard<std::recursive_mutex> lock(g_fontMutex);
            g_scaledFonts.insert(hFont);
        }
        in_dpi_hook = false;
        return hFont;
    }
    return CreateFontIndirectA_Orig(plf);
}

typedef HFONT(WINAPI* CreateFontW_t)(int cHeight, int cWidth, int cEscapement, int cOrientation, int cWeight, DWORD bItalic, DWORD bUnderline, DWORD bStrikeOut, DWORD iCharSet, DWORD iOutPrecision, DWORD iClipPrecision, DWORD iQuality, DWORD iPitchAndFamily, LPCWSTR pszFaceName);
CreateFontW_t CreateFontW_Orig;

HFONT WINAPI CreateFontW_Hook(int cHeight, int cWidth, int cEscapement, int cOrientation, int cWeight, DWORD bItalic, DWORD bUnderline, DWORD bStrikeOut, DWORD iCharSet, DWORD iOutPrecision, DWORD iClipPrecision, DWORD iQuality, DWORD iPitchAndFamily, LPCWSTR pszFaceName) {
    double fontMult = (g_fontScaleMultiplier != 1.0) ? g_fontScaleMultiplier : g_scaleMultiplier;
    if (!in_dpi_hook && g_shouldApply && (g_isMmc || g_enableGdiFontHooks) && fontMult != 1.0) {
        in_dpi_hook = true;
        if (abs(cHeight) < 30 || g_enableGdiFontHooks) {
            cHeight = (int)std::round(cHeight * fontMult);
            if (cWidth != 0) cWidth = (int)std::round(cWidth * fontMult);
        }
        HFONT hFont = CreateFontW_Orig(cHeight, cWidth, cEscapement, cOrientation, cWeight, bItalic, bUnderline, bStrikeOut, iCharSet, iOutPrecision, iClipPrecision, iQuality, iPitchAndFamily, pszFaceName);
        if (hFont) {
            std::lock_guard<std::recursive_mutex> lock(g_fontMutex);
            g_scaledFonts.insert(hFont);
        }
        in_dpi_hook = false;
        return hFont;
    }
    return CreateFontW_Orig(cHeight, cWidth, cEscapement, cOrientation, cWeight, bItalic, bUnderline, bStrikeOut, iCharSet, iOutPrecision, iClipPrecision, iQuality, iPitchAndFamily, pszFaceName);
}

typedef HFONT(WINAPI* CreateFontA_t)(int cHeight, int cWidth, int cEscapement, int cOrientation, int cWeight, DWORD bItalic, DWORD bUnderline, DWORD bStrikeOut, DWORD iCharSet, DWORD iOutPrecision, DWORD iClipPrecision, DWORD iQuality, DWORD iPitchAndFamily, LPCSTR pszFaceName);
CreateFontA_t CreateFontA_Orig;

HFONT WINAPI CreateFontA_Hook(int cHeight, int cWidth, int cEscapement, int cOrientation, int cWeight, DWORD bItalic, DWORD bUnderline, DWORD bStrikeOut, DWORD iCharSet, DWORD iOutPrecision, DWORD iClipPrecision, DWORD iQuality, DWORD iPitchAndFamily, LPCSTR pszFaceName) {
    double fontMult = (g_fontScaleMultiplier != 1.0) ? g_fontScaleMultiplier : g_scaleMultiplier;
    if (!in_dpi_hook && g_shouldApply && (g_isMmc || g_enableGdiFontHooks) && fontMult != 1.0) {
        in_dpi_hook = true;
        if (abs(cHeight) < 30 || g_enableGdiFontHooks) {
            cHeight = (int)std::round(cHeight * fontMult);
            if (cWidth != 0) cWidth = (int)std::round(cWidth * fontMult);
        }
        HFONT hFont = CreateFontA_Orig(cHeight, cWidth, cEscapement, cOrientation, cWeight, bItalic, bUnderline, bStrikeOut, iCharSet, iOutPrecision, iClipPrecision, iQuality, iPitchAndFamily, pszFaceName);
        if (hFont) {
            std::lock_guard<std::recursive_mutex> lock(g_fontMutex);
            g_scaledFonts.insert(hFont);
        }
        in_dpi_hook = false;
        return hFont;
    }
    return CreateFontA_Orig(cHeight, cWidth, cEscapement, cOrientation, cWeight, bItalic, bUnderline, bStrikeOut, iCharSet, iOutPrecision, iClipPrecision, iQuality, iPitchAndFamily, pszFaceName);
}

int WINAPI GetObjectW_Hook(HANDLE h, int c, LPVOID pv) {
    int res = GetObjectW_Orig(h, c, pv);
    double fontMult = (g_fontScaleMultiplier != 1.0) ? g_fontScaleMultiplier : g_scaleMultiplier;
    if (!in_dpi_hook && pv && res > 0 && g_shouldApply && (g_isMmc || g_enableGdiFontHooks) && fontMult != 1.0) {
        if (c >= (int)sizeof(LOGFONTW) && GetObjectType(h) == OBJ_FONT) {
            std::lock_guard<std::recursive_mutex> lock(g_fontMutex);
            if (!g_scaledFonts.empty() && g_scaledFonts.find((HFONT)h) != g_scaledFonts.end()) {
                LOGFONTW* plf = (LOGFONTW*)pv;
                in_dpi_hook = true;
                plf->lfHeight = (LONG)std::round(plf->lfHeight / fontMult);
                if (plf->lfWidth != 0) plf->lfWidth = (LONG)std::round(plf->lfWidth / fontMult);
                in_dpi_hook = false;
            }
        }
    }
    return res;
}

typedef int(WINAPI* GetObjectA_t)(HANDLE h, int c, LPVOID pv);
GetObjectA_t GetObjectA_Orig;

int WINAPI GetObjectA_Hook(HANDLE h, int c, LPVOID pv) {
    int res = GetObjectA_Orig(h, c, pv);
    double fontMult = (g_fontScaleMultiplier != 1.0) ? g_fontScaleMultiplier : g_scaleMultiplier;
    if (!in_dpi_hook && pv && res > 0 && g_shouldApply && (g_isMmc || g_enableGdiFontHooks) && fontMult != 1.0) {
        if (c >= (int)sizeof(LOGFONTA) && GetObjectType(h) == OBJ_FONT) {
            std::lock_guard<std::recursive_mutex> lock(g_fontMutex);
            if (!g_scaledFonts.empty() && g_scaledFonts.find((HFONT)h) != g_scaledFonts.end()) {
                LOGFONTA* plf = (LOGFONTA*)pv;
                in_dpi_hook = true;
                plf->lfHeight = (LONG)std::round(plf->lfHeight / fontMult);
                if (plf->lfWidth != 0) plf->lfWidth = (LONG)std::round(plf->lfWidth / fontMult);
                in_dpi_hook = false;
            }
        }
    }
    return res;
}

typedef HGDIOBJ(WINAPI* GetStockObject_t)(int i);
GetStockObject_t GetStockObject_Orig;

HGDIOBJ WINAPI GetStockObject_Hook(int i) {
    HGDIOBJ res = GetStockObject_Orig(i);
    double fontMult = (g_fontScaleMultiplier != 1.0) ? g_fontScaleMultiplier : g_scaleMultiplier;
    if (!in_dpi_hook && g_shouldApply && (g_isMmc || g_enableGdiFontHooks) && fontMult != 1.0 && CreateFontIndirectW_Orig) {
        if (i == DEFAULT_GUI_FONT) {
            std::lock_guard<std::recursive_mutex> lock(g_fontMutex);
            if (!g_stockGuiFont) {
                LOGFONTW lf = {0};
                if (GetObjectW_Orig && GetObjectW_Orig(res, sizeof(lf), &lf)) {
                    in_dpi_hook = true;
                    lf.lfHeight = (LONG)std::round(lf.lfHeight * fontMult);
                    if (lf.lfWidth != 0) lf.lfWidth = (LONG)std::round(lf.lfWidth * fontMult);
                    g_stockGuiFont = CreateFontIndirectW_Orig(&lf);
                    in_dpi_hook = false;
                    if (g_stockGuiFont) {
                        g_scaledFonts.insert(g_stockGuiFont);
                    }
                }
            }
            if (g_stockGuiFont) return g_stockGuiFont;
        } else if (i == SYSTEM_FONT) {
            std::lock_guard<std::recursive_mutex> lock(g_fontMutex);
            if (!g_stockSystemFont) {
                LOGFONTW lf = {0};
                if (GetObjectW_Orig && GetObjectW_Orig(res, sizeof(lf), &lf)) {
                    in_dpi_hook = true;
                    lf.lfHeight = (LONG)std::round(lf.lfHeight * fontMult);
                    if (lf.lfWidth != 0) lf.lfWidth = (LONG)std::round(lf.lfWidth * fontMult);
                    g_stockSystemFont = CreateFontIndirectW_Orig(&lf);
                    in_dpi_hook = false;
                    if (g_stockSystemFont) {
                        g_scaledFonts.insert(g_stockSystemFont);
                    }
                }
            }
            if (g_stockSystemFont) return g_stockSystemFont;
        } else if (i == SYSTEM_FIXED_FONT) {
            std::lock_guard<std::recursive_mutex> lock(g_fontMutex);
            if (!g_stockSystemFixedFont) {
                LOGFONTW lf = {0};
                if (GetObjectW_Orig && GetObjectW_Orig(res, sizeof(lf), &lf)) {
                    in_dpi_hook = true;
                    lf.lfHeight = (LONG)std::round(lf.lfHeight * fontMult);
                    if (lf.lfWidth != 0) lf.lfWidth = (LONG)std::round(lf.lfWidth * fontMult);
                    g_stockSystemFixedFont = CreateFontIndirectW_Orig(&lf);
                    in_dpi_hook = false;
                    if (g_stockSystemFixedFont) {
                        g_scaledFonts.insert(g_stockSystemFixedFont);
                    }
                }
            }
            if (g_stockSystemFixedFont) return g_stockSystemFixedFont;
        }
    }
    return res;
}

BOOL WINAPI DeleteObject_Hook(HGDIOBJ ho) {
    if (ho) {
        std::lock_guard<std::recursive_mutex> lock(g_fontMutex);
        // Do not delete our global stock fonts through normal DeleteObject
        if (ho == g_stockGuiFont || ho == g_stockSystemFont || ho == g_stockSystemFixedFont) {
            return TRUE;
        }
        g_scaledFonts.erase((HFONT)ho);
        for (auto it = g_fontScaleCache.begin(); it != g_fontScaleCache.end(); ) {
            if (it->first == (HFONT)ho || it->second == (HFONT)ho) {
                if (it->second && it->second != (HFONT)ho && it->second != it->first) {
                    g_scaledFonts.erase(it->second);
                    if (DeleteObject_Orig) DeleteObject_Orig(it->second);
                }
                it = g_fontScaleCache.erase(it);
            } else {
                ++it;
            }
        }
    }
    return DeleteObject_Orig(ho);
}

// -------------------------------------------------------------------------
// IMAGE LOADING HOOKS
// -------------------------------------------------------------------------

typedef HANDLE(WINAPI* LoadImageW_t)(HINSTANCE hInst, LPCWSTR name, UINT type, int cx, int cy, UINT fuLoad);
LoadImageW_t LoadImageW_Orig;

HANDLE WINAPI LoadImageW_Hook(HINSTANCE hInst, LPCWSTR name, UINT type, int cx, int cy, UINT fuLoad) {
    double imgMult = (g_imageScaleMultiplier != 1.0) ? g_imageScaleMultiplier : g_scaleMultiplier;
    if (!in_dpi_hook && g_shouldApply && (g_isMmc || g_enableGdiImageHooks) && imgMult != 1.0 && cx > 0 && cy > 0 && (type == IMAGE_ICON || type == IMAGE_BITMAP)) {
        in_dpi_hook = true;
        int scaled_cx = (int)std::round(cx * imgMult);
        int scaled_cy = (int)std::round(cy * imgMult);
        HANDLE hRet = LoadImageW_Orig(hInst, name, type, scaled_cx, scaled_cy, fuLoad);
        in_dpi_hook = false;
        if (hRet) return hRet;
    }
    return LoadImageW_Orig(hInst, name, type, cx, cy, fuLoad);
}

typedef HANDLE(WINAPI* LoadImageA_t)(HINSTANCE hInst, LPCSTR name, UINT type, int cx, int cy, UINT fuLoad);
LoadImageA_t LoadImageA_Orig;

HANDLE WINAPI LoadImageA_Hook(HINSTANCE hInst, LPCSTR name, UINT type, int cx, int cy, UINT fuLoad) {
    double imgMult = (g_imageScaleMultiplier != 1.0) ? g_imageScaleMultiplier : g_scaleMultiplier;
    if (!in_dpi_hook && g_shouldApply && (g_isMmc || g_enableGdiImageHooks) && imgMult != 1.0 && cx > 0 && cy > 0 && (type == IMAGE_ICON || type == IMAGE_BITMAP)) {
        in_dpi_hook = true;
        int scaled_cx = (int)std::round(cx * imgMult);
        int scaled_cy = (int)std::round(cy * imgMult);
        HANDLE hRet = LoadImageA_Orig(hInst, name, type, scaled_cx, scaled_cy, fuLoad);
        in_dpi_hook = false;
        if (hRet) return hRet;
    }
    return LoadImageA_Orig(hInst, name, type, cx, cy, fuLoad);
}

// -------------------------------------------------------------------------
// DPI HOOKS
// -------------------------------------------------------------------------

typedef UINT(WINAPI* GetDpiForWindow_t)(HWND hwnd);
GetDpiForWindow_t GetDpiForWindow_Orig;

UINT WINAPI GetDpiForWindow_Hook(HWND hwnd) {
    if (!g_shouldApply || !g_enableDpiHooks || in_dpi_hook || g_scaleMultiplier == 1.0) {
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

UINT WINAPI GetDpiForSystem_Hook() {
    if (!g_shouldApply || !g_enableDpiHooks || in_dpi_hook || g_scaleMultiplier == 1.0) {
        return GetDpiForSystem_Orig();
    }
    in_dpi_hook = true;
    UINT dpi = GetDpiForSystem_Orig();
    
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

typedef int(WINAPI* GetSystemMetrics_t)(int nIndex);
GetSystemMetrics_t GetSystemMetrics_Orig;

int WINAPI GetSystemMetrics_Hook(int nIndex) {
    if (!g_shouldApply || !g_enableDpiHooks || in_dpi_hook || g_scaleMultiplier == 1.0) {
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
                if (GetSystemMetricsForDpi_Orig) {
                    UINT queryDpi = (UINT)(96.0 * g_scaleMultiplier);
                    int metricForDpi = GetSystemMetricsForDpi_Orig(nIndex, queryDpi);
                    in_dpi_hook = false;
                    return metricForDpi;
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

typedef BOOL(WINAPI* SystemParametersInfoW_t)(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni);
SystemParametersInfoW_t SystemParametersInfoW_Orig;

BOOL WINAPI SystemParametersInfoW_Hook(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni) {
    if (!g_shouldApply || !g_enableDpiHooks || in_dpi_hook || g_scaleMultiplier == 1.0) {
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
    double actualFontMultiplier = (g_fontScaleMultiplier != 1.0) ? g_fontScaleMultiplier : g_scaleMultiplier;
    if (GetDpiForSystem_Orig) {
        UINT sysDpi = GetDpiForSystem_Orig();
        if (sysDpi > 0) {
            actualMultiplier = (96.0 * g_scaleMultiplier) / (double)sysDpi;
            actualFontMultiplier = (96.0 * ((g_fontScaleMultiplier != 1.0) ? g_fontScaleMultiplier : g_scaleMultiplier)) / (double)sysDpi;
        }
    }

    if (uiAction == SPI_GETNONCLIENTMETRICS && pvParam) {
        NONCLIENTMETRICSW* ncm = (NONCLIENTMETRICSW*)pvParam;
        
        auto scaleFont = [actualFontMultiplier](LOGFONTW& lf) {
            lf.lfHeight = (LONG)std::round(lf.lfHeight * actualFontMultiplier);
            if (lf.lfWidth != 0) lf.lfWidth = (LONG)std::round(lf.lfWidth * actualFontMultiplier);
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
        lf->lfHeight = (LONG)std::round(lf->lfHeight * actualFontMultiplier);
        if (lf->lfWidth != 0) lf->lfWidth = (LONG)std::round(lf->lfWidth * actualFontMultiplier);
    }
    else if (uiAction == SPI_GETICONMETRICS && pvParam) {
        ICONMETRICSW* im = (ICONMETRICSW*)pvParam;
        if (im->cbSize >= (unsigned)(offsetof(ICONMETRICSW, lfFont) + sizeof(LOGFONTW))) {
            im->iHorzSpacing = (int)(im->iHorzSpacing * actualMultiplier);
            im->iVertSpacing = (int)(im->iVertSpacing * actualMultiplier);
            im->lfFont.lfHeight = (LONG)std::round(im->lfFont.lfHeight * actualFontMultiplier);
            if (im->lfFont.lfWidth != 0) im->lfFont.lfWidth = (LONG)std::round(im->lfFont.lfWidth * actualFontMultiplier);
        }
    }
    
    in_dpi_hook = false;
    return ret;
}

typedef BOOL(WINAPI* SystemParametersInfoA_t)(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni);
SystemParametersInfoA_t SystemParametersInfoA_Orig;

BOOL WINAPI SystemParametersInfoA_Hook(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni) {
    if (!g_shouldApply || !g_enableDpiHooks || in_dpi_hook || g_scaleMultiplier == 1.0) {
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
    double actualFontMultiplier = (g_fontScaleMultiplier != 1.0) ? g_fontScaleMultiplier : g_scaleMultiplier;
    if (GetDpiForSystem_Orig) {
        UINT sysDpi = GetDpiForSystem_Orig();
        if (sysDpi > 0) {
            actualMultiplier = (96.0 * g_scaleMultiplier) / (double)sysDpi;
            actualFontMultiplier = (96.0 * ((g_fontScaleMultiplier != 1.0) ? g_fontScaleMultiplier : g_scaleMultiplier)) / (double)sysDpi;
        }
    }

    if (uiAction == SPI_GETNONCLIENTMETRICS && pvParam) {
        NONCLIENTMETRICSA* ncm = (NONCLIENTMETRICSA*)pvParam;
        
        auto scaleFont = [actualFontMultiplier](LOGFONTA& lf) {
            lf.lfHeight = (LONG)std::round(lf.lfHeight * actualFontMultiplier);
            if (lf.lfWidth != 0) lf.lfWidth = (LONG)std::round(lf.lfWidth * actualFontMultiplier);
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
        lf->lfHeight = (LONG)std::round(lf->lfHeight * actualFontMultiplier);
        if (lf->lfWidth != 0) lf->lfWidth = (LONG)std::round(lf->lfWidth * actualFontMultiplier);
    }
    else if (uiAction == SPI_GETICONMETRICS && pvParam) {
        ICONMETRICSA* im = (ICONMETRICSA*)pvParam;
        if (im->cbSize >= (unsigned)(offsetof(ICONMETRICSA, lfFont) + sizeof(LOGFONTA))) {
            im->iHorzSpacing = (int)(im->iHorzSpacing * actualMultiplier);
            im->iVertSpacing = (int)(im->iVertSpacing * actualMultiplier);
            im->lfFont.lfHeight = (LONG)std::round(im->lfFont.lfHeight * actualFontMultiplier);
            if (im->lfFont.lfWidth != 0) im->lfFont.lfWidth = (LONG)std::round(im->lfFont.lfWidth * actualFontMultiplier);
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
    if (g_shouldApply && (g_isMmc || g_isWlmail || g_enableThemeHooks) && g_scaleMultiplier != 1.0 && !in_dpi_hook && metric > 0) {
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
    if (g_shouldApply && (g_isMmc || g_isWlmail || g_enableThemeHooks) && g_scaleMultiplier != 1.0 && SUCCEEDED(hr) && piVal) {
        if (iPropId == 1201 || iPropId == 1202 || iPropId == 2403) {
            *piVal = (int)std::round(*piVal * g_scaleMultiplier);
        } else if (!g_isWlmail && (iPropId == 2416 || iPropId == 2417 || iPropId == 2414 || iPropId == 2415)) {
            *piVal = (int)std::round(*piVal * g_scaleMultiplier);
        }
    }
    return hr;
}

typedef HRESULT(WINAPI* GetThemePartSize_t)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT prc, THEMESIZE eSize, SIZE* psz);
GetThemePartSize_t GetThemePartSize_Orig;

HRESULT WINAPI GetThemePartSize_Hook(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT prc, THEMESIZE eSize, SIZE* psz) {
    HRESULT hr = GetThemePartSize_Orig(hTheme, hdc, iPartId, iStateId, prc, eSize, psz);
    if (g_shouldApply && (g_isMmc || g_isWlmail || g_enableThemeHooks) && g_scaleMultiplier != 1.0 && SUCCEEDED(hr) && psz) {
        psz->cx = (LONG)(psz->cx * g_scaleMultiplier);
        psz->cy = (LONG)(psz->cy * g_scaleMultiplier);
    }
    return hr;
}

typedef HRESULT(WINAPI* GetThemeMargins_t)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, int iPropId, LPCRECT prc, MARGINS* pMargins);
GetThemeMargins_t GetThemeMargins_Orig;

HRESULT WINAPI GetThemeMargins_Hook(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, int iPropId, LPCRECT prc, MARGINS* pMargins) {
    HRESULT hr = GetThemeMargins_Orig(hTheme, hdc, iPartId, iStateId, iPropId, prc, pMargins);
    if (g_shouldApply && (g_isMmc || (g_enableThemeHooks && !g_isWlmail)) && g_scaleMultiplier != 1.0 && SUCCEEDED(hr) && pMargins) {
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
    if (g_shouldApply && (g_isMmc || (g_enableThemeHooks && !g_isWlmail)) && g_scaleMultiplier != 1.0 && SUCCEEDED(hr) && pRect) {
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
    double imgMult = (g_imageScaleMultiplier != 1.0) ? g_imageScaleMultiplier : g_scaleMultiplier;
    if (g_shouldApply && (g_isMmc || g_enableGdiImageHooks) && imgMult != 1.0 && cx > 0 && cy > 0) {
        int scaled_cx = (int)std::round(cx * imgMult);
        int scaled_cy = (int)std::round(cy * imgMult);
        HIMAGELIST hRet = ImageList_Create_Orig(scaled_cx, scaled_cy, flags, cInitial, cGrow);
        if (hRet) return hRet;
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
    if (g_shouldApply && hTheme && pszClassList) {
        std::lock_guard<std::recursive_mutex> lock(g_themeMutex);
        g_themeClasses[hTheme] = pszClassList;
    }
    return hTheme;
}

typedef HRESULT(WINAPI* CloseThemeData_t)(HTHEME hTheme);
CloseThemeData_t CloseThemeData_Orig;

HRESULT WINAPI CloseThemeData_Hook(HTHEME hTheme) {
    if (g_shouldApply && hTheme) {
        std::lock_guard<std::recursive_mutex> lock(g_themeMutex);
        g_themeClasses.erase(hTheme);
    }
    return CloseThemeData_Orig(hTheme);
}

HRESULT WINAPI DrawThemeBackground_Hook(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pRect, LPCRECT pClipRect) {
    if (g_shouldApply && g_isWlmail && pRect && hTheme) {
        std::wstring cls;
        {
            std::lock_guard<std::recursive_mutex> lock(g_themeMutex);
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
        if (!g_shouldApply || !g_enableDpiHooks || in_dpi_hook || g_scaleMultiplier == 1.0) {
            return GetDeviceCaps_Orig(hdc, index);
        }
        in_dpi_hook = true;
        int dpi = GetDeviceCaps_Orig(hdc, index);
        if (dpi > 0) {
            void* retAddr = _ReturnAddress();
            if (g_isMmc) {
                // For MMC and PowerDesk, scale even internal calls
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
    if (!g_shouldApply || !g_enableDpiHooks || in_dpi_hook || g_scaleMultiplier == 1.0) {
        return GetDpiForMonitor_Orig(hmonitor, dpiType, dpiX, dpiY);
    }
    in_dpi_hook = true;
    HRESULT hr = GetDpiForMonitor_Orig(hmonitor, dpiType, dpiX, dpiY);
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
    if (!g_shouldApply || !g_enableDpiHooks || in_dpi_hook || g_scaleMultiplier == 1.0) {
        return GetScaleFactorForMonitor_Orig(hMon, pScale);
    }
    in_dpi_hook = true;
    HRESULT hr = GetScaleFactorForMonitor_Orig(hMon, pScale);
    if (SUCCEEDED(hr) && pScale) {
        *pScale = MapToValidDeviceScaleFactor(g_scaleMultiplier);
    }
    in_dpi_hook = false;
    return hr;
}

typedef void(WINAPI* D2D1GetDesktopDpi_t)(FLOAT *dpiX, FLOAT *dpiY);
D2D1GetDesktopDpi_t D2D1GetDesktopDpi_Orig;

void WINAPI D2D1GetDesktopDpi_Hook(FLOAT *dpiX, FLOAT *dpiY) {
    D2D1GetDesktopDpi_Orig(dpiX, dpiY);
    if (g_shouldApply && g_enableDpiHooks && g_scaleMultiplier != 1.0) {
        if (dpiX) *dpiX = (FLOAT)(*dpiX * g_scaleMultiplier);
        if (dpiY) *dpiY = (FLOAT)(*dpiY * g_scaleMultiplier);
    }
}

// -------------------------------------------------------------------------
// WINRT / UWP / MODERN DISPLAY & UI SETTINGS HOOKS
// -------------------------------------------------------------------------

typedef struct HSTRING__* HSTRING;

typedef HRESULT(WINAPI* RoGetActivationFactory_t)(HSTRING activatableClassId, REFIID iid, void** factory);
RoGetActivationFactory_t RoGetActivationFactory_Orig = nullptr;

typedef HRESULT(WINAPI* RoActivateInstance_t)(HSTRING activatableClassId, void** instance);
RoActivateInstance_t RoActivateInstance_Orig = nullptr;

typedef PCWSTR(WINAPI* WindowsGetStringRawBuffer_t)(HSTRING string, UINT32* length);
WindowsGetStringRawBuffer_t pWindowsGetStringRawBuffer = nullptr;

typedef UINT(WINAPI* GetSystemDpiForProcess_t)(HANDLE hProcess);
GetSystemDpiForProcess_t GetSystemDpiForProcess_Orig = nullptr;

UINT WINAPI GetSystemDpiForProcess_Hook(HANDLE hProcess) {
    if (!g_shouldApply || !g_enableDpiHooks || in_dpi_hook || g_scaleMultiplier == 1.0) {
        return GetSystemDpiForProcess_Orig ? GetSystemDpiForProcess_Orig(hProcess) : 96;
    }
    in_dpi_hook = true;
    UINT dpi = GetSystemDpiForProcess_Orig ? GetSystemDpiForProcess_Orig(hProcess) : 96;
    void* retAddr = _ReturnAddress();
    if (IsInternalOSCall(retAddr) || g_isWlmail) {
        in_dpi_hook = false;
        return dpi;
    }
    in_dpi_hook = false;
    return (UINT)std::round(96.0 * g_scaleMultiplier);
}

typedef UINT(WINAPI* GetDpiFromDpiAwarenessContext_t)(DPI_AWARENESS_CONTEXT value);
GetDpiFromDpiAwarenessContext_t GetDpiFromDpiAwarenessContext_Orig = nullptr;

UINT WINAPI GetDpiFromDpiAwarenessContext_Hook(DPI_AWARENESS_CONTEXT value) {
    if (!g_shouldApply || !g_enableDpiHooks || in_dpi_hook || g_scaleMultiplier == 1.0) {
        return GetDpiFromDpiAwarenessContext_Orig ? GetDpiFromDpiAwarenessContext_Orig(value) : 96;
    }
    in_dpi_hook = true;
    UINT dpi = GetDpiFromDpiAwarenessContext_Orig ? GetDpiFromDpiAwarenessContext_Orig(value) : 96;
    void* retAddr = _ReturnAddress();
    if (IsInternalOSCall(retAddr) || g_isWlmail) {
        in_dpi_hook = false;
        return dpi;
    }
    in_dpi_hook = false;
    return (UINT)(96.0 * g_scaleMultiplier);
}

// DisplayInformation GUID definitions
static const GUID IID_IDisplayInformation2 = { 0x99a292c6, 0xe082, 0x4469, { 0xa3, 0x50, 0x13, 0x8b, 0x21, 0x89, 0x4d, 0x5a } };
static const GUID IID_IUISettings2 = { 0xbad82401, 0x2721, 0x44f9, { 0xbb, 0x57, 0x0a, 0x18, 0xbc, 0x54, 0x70, 0xde } };

typedef HRESULT(STDMETHODCALLTYPE* QueryInterface_t)(void* thisPtr, REFIID riid, void** ppvObject);

typedef HRESULT(STDMETHODCALLTYPE* get_ResolutionScale_t)(void* thisPtr, int* pValue);
get_ResolutionScale_t get_ResolutionScale_Orig = nullptr;

HRESULT STDMETHODCALLTYPE get_ResolutionScale_Hook(void* thisPtr, int* pValue) {
    if (!g_shouldApply || !g_enableDpiHooks || in_dpi_hook || g_scaleMultiplier == 1.0) {
        return get_ResolutionScale_Orig ? get_ResolutionScale_Orig(thisPtr, pValue) : S_OK;
    }
    in_dpi_hook = true;
    HRESULT hr = get_ResolutionScale_Orig ? get_ResolutionScale_Orig(thisPtr, pValue) : S_OK;
    if (SUCCEEDED(hr) && pValue) {
        *pValue = MapToValidDeviceScaleFactor(g_scaleMultiplier);
    }
    in_dpi_hook = false;
    return hr;
}

typedef HRESULT(STDMETHODCALLTYPE* get_LogicalDpi_t)(void* thisPtr, float* pValue);
get_LogicalDpi_t get_LogicalDpi_Orig = nullptr;

HRESULT STDMETHODCALLTYPE get_LogicalDpi_Hook(void* thisPtr, float* pValue) {
    if (!g_shouldApply || !g_enableDpiHooks || in_dpi_hook || g_scaleMultiplier == 1.0) {
        return get_LogicalDpi_Orig ? get_LogicalDpi_Orig(thisPtr, pValue) : S_OK;
    }
    in_dpi_hook = true;
    HRESULT hr = get_LogicalDpi_Orig ? get_LogicalDpi_Orig(thisPtr, pValue) : S_OK;
    if (SUCCEEDED(hr) && pValue) {
        *pValue = (float)(*pValue * g_scaleMultiplier);
    }
    in_dpi_hook = false;
    return hr;
}

typedef HRESULT(STDMETHODCALLTYPE* get_RawDpiX_t)(void* thisPtr, float* pValue);
get_RawDpiX_t get_RawDpiX_Orig = nullptr;

HRESULT STDMETHODCALLTYPE get_RawDpiX_Hook(void* thisPtr, float* pValue) {
    if (!g_shouldApply || !g_enableDpiHooks || in_dpi_hook || g_scaleMultiplier == 1.0) {
        return get_RawDpiX_Orig ? get_RawDpiX_Orig(thisPtr, pValue) : S_OK;
    }
    in_dpi_hook = true;
    HRESULT hr = get_RawDpiX_Orig ? get_RawDpiX_Orig(thisPtr, pValue) : S_OK;
    if (SUCCEEDED(hr) && pValue) {
        *pValue = (float)(*pValue * g_scaleMultiplier);
    }
    in_dpi_hook = false;
    return hr;
}

typedef HRESULT(STDMETHODCALLTYPE* get_RawDpiY_t)(void* thisPtr, float* pValue);
get_RawDpiY_t get_RawDpiY_Orig = nullptr;

HRESULT STDMETHODCALLTYPE get_RawDpiY_Hook(void* thisPtr, float* pValue) {
    if (!g_shouldApply || !g_enableDpiHooks || in_dpi_hook || g_scaleMultiplier == 1.0) {
        return get_RawDpiY_Orig ? get_RawDpiY_Orig(thisPtr, pValue) : S_OK;
    }
    in_dpi_hook = true;
    HRESULT hr = get_RawDpiY_Orig ? get_RawDpiY_Orig(thisPtr, pValue) : S_OK;
    if (SUCCEEDED(hr) && pValue) {
        *pValue = (float)(*pValue * g_scaleMultiplier);
    }
    in_dpi_hook = false;
    return hr;
}

typedef HRESULT(STDMETHODCALLTYPE* get_RawPixelsPerViewPixel_t)(void* thisPtr, double* pValue);
get_RawPixelsPerViewPixel_t get_RawPixelsPerViewPixel_Orig = nullptr;

HRESULT STDMETHODCALLTYPE get_RawPixelsPerViewPixel_Hook(void* thisPtr, double* pValue) {
    if (!g_shouldApply || !g_enableDpiHooks || in_dpi_hook || g_scaleMultiplier == 1.0) {
        return get_RawPixelsPerViewPixel_Orig ? get_RawPixelsPerViewPixel_Orig(thisPtr, pValue) : S_OK;
    }
    in_dpi_hook = true;
    HRESULT hr = get_RawPixelsPerViewPixel_Orig ? get_RawPixelsPerViewPixel_Orig(thisPtr, pValue) : S_OK;
    if (SUCCEEDED(hr) && pValue) {
        *pValue = (double)(*pValue * g_scaleMultiplier);
    }
    in_dpi_hook = false;
    return hr;
}

// -------------------------------------------------------------------------
// UISETTINGS (Windows.UI.ViewManagement.UISettings) HOOKS
// -------------------------------------------------------------------------

typedef HRESULT(STDMETHODCALLTYPE* get_UISettingsTextScaleFactor_t)(void* thisPtr, double* pValue);
get_UISettingsTextScaleFactor_t get_UISettingsTextScaleFactor_Orig = nullptr;

HRESULT STDMETHODCALLTYPE get_UISettingsTextScaleFactor_Hook(void* thisPtr, double* pValue) {
    if (!g_shouldApply || !g_enableDpiHooks || in_dpi_hook || g_scaleMultiplier == 1.0) {
        return get_UISettingsTextScaleFactor_Orig ? get_UISettingsTextScaleFactor_Orig(thisPtr, pValue) : S_OK;
    }
    in_dpi_hook = true;
    HRESULT hr = get_UISettingsTextScaleFactor_Orig ? get_UISettingsTextScaleFactor_Orig(thisPtr, pValue) : S_OK;
    if (SUCCEEDED(hr) && pValue) {
        double orig = *pValue;
        *pValue = (double)(orig * g_scaleMultiplier);
        ModLog(L"IUISettings2::get_TextScaleFactor -> %f (orig: %f)", *pValue, orig);
    }
    in_dpi_hook = false;
    return hr;
}

typedef HRESULT(STDMETHODCALLTYPE* get_UISettingsScrollBarSize_t)(void* thisPtr, SIZE* pValue);
get_UISettingsScrollBarSize_t get_UISettingsScrollBarSize_Orig = nullptr;

HRESULT STDMETHODCALLTYPE get_UISettingsScrollBarSize_Hook(void* thisPtr, SIZE* pValue) {
    if (!g_shouldApply || !g_enableDpiHooks || in_dpi_hook || g_scaleMultiplier == 1.0) {
        return get_UISettingsScrollBarSize_Orig ? get_UISettingsScrollBarSize_Orig(thisPtr, pValue) : S_OK;
    }
    in_dpi_hook = true;
    HRESULT hr = get_UISettingsScrollBarSize_Orig ? get_UISettingsScrollBarSize_Orig(thisPtr, pValue) : S_OK;
    if (SUCCEEDED(hr) && pValue) {
        pValue->cx = (LONG)(pValue->cx * g_scaleMultiplier);
        pValue->cy = (LONG)(pValue->cy * g_scaleMultiplier);
    }
    in_dpi_hook = false;
    return hr;
}

typedef HRESULT(STDMETHODCALLTYPE* get_UISettingsCursorSize_t)(void* thisPtr, SIZE* pValue);
get_UISettingsCursorSize_t get_UISettingsCursorSize_Orig = nullptr;

HRESULT STDMETHODCALLTYPE get_UISettingsCursorSize_Hook(void* thisPtr, SIZE* pValue) {
    if (!g_shouldApply || !g_enableDpiHooks || in_dpi_hook || g_scaleMultiplier == 1.0) {
        return get_UISettingsCursorSize_Orig ? get_UISettingsCursorSize_Orig(thisPtr, pValue) : S_OK;
    }
    in_dpi_hook = true;
    HRESULT hr = get_UISettingsCursorSize_Orig ? get_UISettingsCursorSize_Orig(thisPtr, pValue) : S_OK;
    if (SUCCEEDED(hr) && pValue) {
        pValue->cx = (LONG)(pValue->cx * g_scaleMultiplier);
        pValue->cy = (LONG)(pValue->cy * g_scaleMultiplier);
    }
    in_dpi_hook = false;
    return hr;
}

static std::recursive_mutex g_winrtMutex;
static std::unordered_set<void*> g_hookedVtables;

void HookUISettingsVTable(void* pInstance) {
    if (!pInstance) return;
    void** vtbl = *(void***)pInstance;
    if (!vtbl) return;

    std::lock_guard<std::recursive_mutex> lock(g_winrtMutex);
    if (g_hookedVtables.find((void*)vtbl) == g_hookedVtables.end()) {
        g_hookedVtables.insert((void*)vtbl);
        ModLog(L"Hooking IUISettings vtable (%p)", vtbl);
        if (vtbl[7]) {
            Wh_SetFunctionHook(vtbl[7], (void*)get_UISettingsCursorSize_Hook, (void**)&get_UISettingsCursorSize_Orig);
        }
        if (vtbl[8]) {
            Wh_SetFunctionHook(vtbl[8], (void*)get_UISettingsScrollBarSize_Hook, (void**)&get_UISettingsScrollBarSize_Orig);
        }
    }

    // Query for IUISettings2 for get_TextScaleFactor
    QueryInterface_t pQI = (QueryInterface_t)vtbl[0];
    void* pUISettings2 = nullptr;
    if (pQI && SUCCEEDED(pQI(pInstance, IID_IUISettings2, &pUISettings2)) && pUISettings2) {
        void** vtbl2 = *(void***)pUISettings2;
        if (vtbl2 && g_hookedVtables.find((void*)vtbl2) == g_hookedVtables.end()) {
            g_hookedVtables.insert((void*)vtbl2);
            ModLog(L"Hooking IUISettings2 vtable (%p)", vtbl2);
            if (vtbl2[6]) {
                Wh_SetFunctionHook(vtbl2[6], (void*)get_UISettingsTextScaleFactor_Hook, (void**)&get_UISettingsTextScaleFactor_Orig);
            }
        }
        typedef ULONG(STDMETHODCALLTYPE* Release_t)(void*);
        ((Release_t)vtbl2[2])(pUISettings2);
    }
}

void HookDisplayInformationVTable(void* pInstance) {
    if (!pInstance) return;
    void** vtbl = *(void***)pInstance;
    if (!vtbl) return;

    std::lock_guard<std::recursive_mutex> lock(g_winrtMutex);
    if (g_hookedVtables.find((void*)vtbl) == g_hookedVtables.end()) {
        g_hookedVtables.insert((void*)vtbl);
        ModLog(L"Hooking IDisplayInformation instance vtable (%p)", vtbl);
        if (vtbl[8]) {
            Wh_SetFunctionHook(vtbl[8], (void*)get_RawDpiX_Hook, (void**)&get_RawDpiX_Orig);
        }
        if (vtbl[9]) {
            Wh_SetFunctionHook(vtbl[9], (void*)get_RawDpiY_Hook, (void**)&get_RawDpiY_Orig);
        }
        if (vtbl[10]) {
            Wh_SetFunctionHook(vtbl[10], (void*)get_LogicalDpi_Hook, (void**)&get_LogicalDpi_Orig);
        }
        if (vtbl[11]) {
            Wh_SetFunctionHook(vtbl[11], (void*)get_ResolutionScale_Hook, (void**)&get_ResolutionScale_Orig);
        }
    }

    // Query IDisplayInformation2 for get_RawPixelsPerViewPixel
    QueryInterface_t pQI = (QueryInterface_t)vtbl[0];
    void* pInfo2 = nullptr;
    if (pQI && SUCCEEDED(pQI(pInstance, IID_IDisplayInformation2, &pInfo2)) && pInfo2) {
        void** vtbl2 = *(void***)pInfo2;
        if (vtbl2 && g_hookedVtables.find((void*)vtbl2) == g_hookedVtables.end()) {
            g_hookedVtables.insert((void*)vtbl2);
            ModLog(L"Hooking IDisplayInformation2 vtable (%p)", vtbl2);
            if (vtbl2[6]) {
                Wh_SetFunctionHook(vtbl2[6], (void*)get_RawPixelsPerViewPixel_Hook, (void**)&get_RawPixelsPerViewPixel_Orig);
            }
        }
        typedef ULONG(STDMETHODCALLTYPE* Release_t)(void*);
        ((Release_t)vtbl2[2])(pInfo2);
    }
}

typedef HRESULT(STDMETHODCALLTYPE* GetForCurrentView_t)(void* thisPtr, void** ppDisplayInfo);
GetForCurrentView_t GetForCurrentView_Orig = nullptr;
GetForCurrentView_t ComponentGetForCurrentView_Orig = nullptr;

HRESULT STDMETHODCALLTYPE GetForCurrentView_Hook(void* thisPtr, void** ppDisplayInfo) {
    HRESULT hr = GetForCurrentView_Orig ? GetForCurrentView_Orig(thisPtr, ppDisplayInfo) : E_FAIL;
    if (SUCCEEDED(hr) && ppDisplayInfo && *ppDisplayInfo) {
        HookDisplayInformationVTable(*ppDisplayInfo);
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE ComponentGetForCurrentView_Hook(void* thisPtr, void** ppDisplayInfo) {
    HRESULT hr = ComponentGetForCurrentView_Orig ? ComponentGetForCurrentView_Orig(thisPtr, ppDisplayInfo) : E_FAIL;
    if (SUCCEEDED(hr) && ppDisplayInfo && *ppDisplayInfo) {
        HookDisplayInformationVTable(*ppDisplayInfo);
    }
    return hr;
}

void HookDisplayInfoStaticsVTable(void** vtbl) {
    if (!vtbl) return;
    std::lock_guard<std::recursive_mutex> lock(g_winrtMutex);
    if (g_hookedVtables.find((void*)vtbl) != g_hookedVtables.end()) return;
    g_hookedVtables.insert((void*)vtbl);

    ModLog(L"Hooking DisplayInformation Statics vtable (%p)", vtbl);
    // IDisplayInformationStatics: slot 14 is GetForCurrentView
    if (vtbl[14]) {
        Wh_SetFunctionHook(vtbl[14], (void*)GetForCurrentView_Hook, (void**)&GetForCurrentView_Orig);
    }
}

void HookComponentDisplayInfoStaticsVTable(void** vtbl) {
    if (!vtbl) return;
    std::lock_guard<std::recursive_mutex> lock(g_winrtMutex);
    if (g_hookedVtables.find((void*)vtbl) != g_hookedVtables.end()) return;
    g_hookedVtables.insert((void*)vtbl);

    ModLog(L"Hooking ComponentDisplayInformation Statics vtable (%p)", vtbl);
    // ComponentDisplayInformationStatics: slot 6 is GetForCurrentView
    if (vtbl[6]) {
        Wh_SetFunctionHook(vtbl[6], (void*)ComponentGetForCurrentView_Hook, (void**)&ComponentGetForCurrentView_Orig);
    }
}

HRESULT WINAPI RoGetActivationFactory_Hook(HSTRING activatableClassId, REFIID iid, void** factory) {
    HRESULT hr = RoGetActivationFactory_Orig(activatableClassId, iid, factory);
    if (SUCCEEDED(hr) && factory && *factory && pWindowsGetStringRawBuffer) {
        UINT32 len = 0;
        PCWSTR className = pWindowsGetStringRawBuffer(activatableClassId, &len);
        if (className) {
            ModLog(L"RoGetActivationFactory: %s", className);
            if (wcscmp(className, L"Windows.Graphics.Display.DisplayInformation") == 0 ||
                wcscmp(className, L"Microsoft.Graphics.Display.DisplayInformation") == 0) {
                void** vtbl = *(void***)*factory;
                HookDisplayInfoStaticsVTable(vtbl);
            } else if (wcscmp(className, L"Windows.UI.Core.ComponentDisplayInformation") == 0) {
                void** vtbl = *(void***)*factory;
                HookComponentDisplayInfoStaticsVTable(vtbl);
            } else if (wcscmp(className, L"Windows.UI.ViewManagement.UISettings") == 0) {
                HookUISettingsVTable(*factory);
            }
        }
    }
    return hr;
}

HRESULT WINAPI RoActivateInstance_Hook(HSTRING activatableClassId, void** instance) {
    HRESULT hr = RoActivateInstance_Orig(activatableClassId, instance);
    if (SUCCEEDED(hr) && instance && *instance && pWindowsGetStringRawBuffer) {
        UINT32 len = 0;
        PCWSTR className = pWindowsGetStringRawBuffer(activatableClassId, &len);
        if (className) {
            ModLog(L"RoActivateInstance: %s", className);
            if (wcscmp(className, L"Windows.Graphics.Display.DisplayInformation") == 0 ||
                wcscmp(className, L"Microsoft.Graphics.Display.DisplayInformation") == 0 ||
                wcscmp(className, L"Windows.UI.Core.ComponentDisplayInformation") == 0) {
                HookDisplayInformationVTable(*instance);
            } else if (wcscmp(className, L"Windows.UI.ViewManagement.UISettings") == 0) {
                HookUISettingsVTable(*instance);
            }
        }
    }
    return hr;
}

// -------------------------------------------------------------------------
// MESSAGE HOOKS FOR MMC NAVPANE
// -------------------------------------------------------------------------

typedef LRESULT(WINAPI* SendMessageW_t)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
SendMessageW_t SendMessageW_Orig;

typedef LRESULT(WINAPI* SendMessageA_t)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
SendMessageA_t SendMessageA_Orig;

LRESULT WINAPI SendMessageW_Hook(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    if (g_shouldApply && (g_isMmc || g_enableGdiFontHooks) && g_scaleMultiplier != 1.0) {
        if (Msg == WM_SETFONT && wParam) {
            HFONT hScaled = GetOrCreateScaledFont((HFONT)wParam);
            if (hScaled) wParam = (WPARAM)hScaled;
        }

        if (Msg == TVM_SETITEMHEIGHT || Msg == TVM_SETINDENT ||
            Msg == TB_SETBUTTONSIZE || Msg == TB_SETBITMAPSIZE ||
            Msg == TB_SETPADDING || Msg == TB_SETINDENT) {
            
            wchar_t className[64];
            if (GetClassNameW(hWnd, className, ARRAYSIZE(className)) > 0) {
                if (_wcsicmp(className, L"SysTreeView32") == 0) {
                    if (Msg == TVM_SETITEMHEIGHT || Msg == TVM_SETINDENT) {
                        int val = (int)wParam;
                        if (val > 0 && val < 200) { 
                            wParam = (WPARAM)(int)std::round(val * g_scaleMultiplier);
                        }
                    }
                } else if (_wcsicmp(className, L"ToolbarWindow32") == 0) {
                    if (Msg == TB_SETBUTTONSIZE || Msg == TB_SETBITMAPSIZE) {
                        int cx = LOWORD(lParam);
                        int cy = HIWORD(lParam);
                        if (cx > 0 && cy > 0 && cx < 300) { 
                            lParam = MAKELPARAM((int)std::round(cx * g_scaleMultiplier), (int)std::round(cy * g_scaleMultiplier));
                        }
                    } else if (Msg == TB_SETPADDING) {
                        int padding = (int)lParam;
                        lParam = (LPARAM)(int)std::round(padding * g_scaleMultiplier);
                    }
                }
            }
        }
    }
    LRESULT res = SendMessageW_Orig(hWnd, Msg, wParam, lParam);
    
    // Post-processing for TreeView to ensure height is recalculated after font change
    if (g_shouldApply && (g_isMmc || g_enableGdiFontHooks) && g_scaleMultiplier != 1.0 && Msg == WM_SETFONT) {
        wchar_t cls[64];
        if (GetClassNameW(hWnd, cls, ARRAYSIZE(cls)) > 0 && _wcsicmp(cls, L"SysTreeView32") == 0) {
             SendMessageW_Orig(hWnd, TVM_SETITEMHEIGHT, (WPARAM)-1, 0);
        }
    }
    return res;
}

LRESULT WINAPI SendMessageA_Hook(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    if (g_shouldApply && (g_isMmc || g_enableGdiFontHooks) && g_scaleMultiplier != 1.0) {
        if (Msg == WM_SETFONT && wParam) {
            HFONT hScaled = GetOrCreateScaledFont((HFONT)wParam);
            if (hScaled) wParam = (WPARAM)hScaled;
        }

        if (Msg == TVM_SETITEMHEIGHT || Msg == TVM_SETINDENT ||
            Msg == TB_SETBUTTONSIZE || Msg == TB_SETBITMAPSIZE ||
            Msg == TB_SETPADDING || Msg == TB_SETINDENT) {
            
            char className[64];
            if (GetClassNameA(hWnd, className, ARRAYSIZE(className)) > 0) {
                if (_stricmp(className, "SysTreeView32") == 0) {
                    if (Msg == TVM_SETITEMHEIGHT || Msg == TVM_SETINDENT) {
                        int val = (int)wParam;
                        if (val > 0 && val < 200) { 
                            wParam = (WPARAM)(int)std::round(val * g_scaleMultiplier);
                        }
                    }
                } else if (_stricmp(className, "ToolbarWindow32") == 0) {
                    if (Msg == TB_SETBUTTONSIZE || Msg == TB_SETBITMAPSIZE) {
                        int cx = LOWORD(lParam);
                        int cy = HIWORD(lParam);
                        if (cx > 0 && cy > 0 && cx < 300) { 
                            lParam = MAKELPARAM((int)std::round(cx * g_scaleMultiplier), (int)std::round(cy * g_scaleMultiplier));
                        }
                    } else if (Msg == TB_SETPADDING) {
                        int padding = (int)lParam;
                        lParam = (LPARAM)(int)std::round(padding * g_scaleMultiplier);
                    }
                }
            }
        }
    }
    LRESULT res = SendMessageA_Orig(hWnd, Msg, wParam, lParam);
    if (g_shouldApply && (g_isMmc || g_enableGdiFontHooks) && g_scaleMultiplier != 1.0 && Msg == WM_SETFONT) {
        char cls[64];
        if (GetClassNameA(hWnd, cls, ARRAYSIZE(cls)) > 0 && _stricmp(cls, "SysTreeView32") == 0) {
             SendMessageA_Orig(hWnd, TVM_SETITEMHEIGHT, (WPARAM)-1, 0);
        }
    }
    return res;
}

// -------------------------------------------------------------------------
// CHROMIUM COMMAND LINE INJECTION & CHILD PROCESS HOOKS
// -------------------------------------------------------------------------

typedef LPWSTR(WINAPI* GetCommandLineW_t)();
GetCommandLineW_t GetCommandLineW_Orig;

LPWSTR WINAPI GetCommandLineW_Hook() {
    if (!g_shouldApply || !g_enableChromiumCmdLine || g_isOlk) {
        return GetCommandLineW_Orig();
    }
    static std::wstring fakeCmdLine;
    static std::atomic<bool> initialized{false};
    static std::mutex cmdMutex;

    LPWSTR orig = GetCommandLineW_Orig();

    if (!initialized && orig != nullptr) {
        std::lock_guard<std::mutex> lock(cmdMutex);
        if (!initialized) {
            fakeCmdLine = orig;
            if (fakeCmdLine.find(L"--force-device-scale-factor") == std::wstring::npos) {
                wchar_t appendBuf[256];
                swprintf_s(appendBuf, ARRAYSIZE(appendBuf), L" --force-device-scale-factor=%.3f", g_scaleMultiplier);
                fakeCmdLine += appendBuf;
                ModLog(L"GetCommandLineW: Injected Chromium scale argument '%s'", fakeCmdLine.c_str());
            }
            initialized = true;
        }
    }
    
    if (initialized) {
        return (LPWSTR)fakeCmdLine.c_str();
    }
    return orig;
}

typedef BOOL(WINAPI* CreateProcessW_t)(
    LPCWSTR lpApplicationName,
    LPWSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCWSTR lpCurrentDirectory,
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
);
CreateProcessW_t CreateProcessW_Orig;

BOOL WINAPI CreateProcessW_Hook(
    LPCWSTR lpApplicationName,
    LPWSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCWSTR lpCurrentDirectory,
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
) {
    if (g_shouldApply && g_enableChromiumCmdLine && g_scaleMultiplier != 1.0 && lpCommandLine) {
        std::wstring cmd(lpCommandLine);
        if (cmd.find(L"--force-device-scale-factor") == std::wstring::npos) {
            wchar_t appendBuf[256];
            swprintf_s(appendBuf, ARRAYSIZE(appendBuf), L" --force-device-scale-factor=%.3f", g_scaleMultiplier);
            cmd += appendBuf;
            ModLog(L"CreateProcessW_Hook: Appending scale factor to child process: %s", cmd.c_str());
            std::vector<wchar_t> modifiableCmd(cmd.begin(), cmd.end());
            modifiableCmd.push_back(L'\0');
            return CreateProcessW_Orig(lpApplicationName, modifiableCmd.data(), lpProcessAttributes, lpThreadAttributes,
                                       bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory,
                                       lpStartupInfo, lpProcessInformation);
        }
    }
    return CreateProcessW_Orig(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes,
                               bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory,
                               lpStartupInfo, lpProcessInformation);
}

// -------------------------------------------------------------------------
// MOD INITIALIZATION
// -------------------------------------------------------------------------

static bool MatchesProcessName(const std::wstring& exeName, const std::wstring& target) {
    if (target.empty() || exeName.empty()) return false;
    if (CaseInsensitiveContains(exeName, target)) return true;
    
    // Check if target without .exe matches exeName without .exe
    std::wstring cleanTarget = target;
    if (cleanTarget.size() > 4 && _wcsicmp(cleanTarget.c_str() + cleanTarget.size() - 4, L".exe") == 0) {
        cleanTarget = cleanTarget.substr(0, cleanTarget.size() - 4);
    }
    std::wstring cleanExe = exeName;
    if (cleanExe.size() > 4 && _wcsicmp(cleanExe.c_str() + cleanExe.size() - 4, L".exe") == 0) {
        cleanExe = cleanExe.substr(0, cleanExe.size() - 4);
    }
    if (CaseInsensitiveContains(cleanExe, cleanTarget)) return true;

    // Special handling for New Outlook and WebView2 helper processes
    if ((_wcsicmp(cleanExe.c_str(), L"msedgewebview2") == 0 || _wcsicmp(cleanExe.c_str(), L"olk") == 0) &&
        (_wcsicmp(cleanTarget.c_str(), L"olk") == 0 || _wcsicmp(cleanTarget.c_str(), L"outlook") == 0)) {
        return true;
    }

    // Special handling for Calculator / CalculatorApp
    if ((CaseInsensitiveContains(cleanExe, L"Calculator") || CaseInsensitiveContains(cleanExe, L"CalculatorApp") || CaseInsensitiveContains(cleanExe, L"calc")) &&
        (CaseInsensitiveContains(cleanTarget, L"Calculator") || CaseInsensitiveContains(cleanTarget, L"CalculatorApp") || CaseInsensitiveContains(cleanTarget, L"calc"))) {
        return true;
    }

    // Special handling for Settings
    if ((CaseInsensitiveContains(cleanExe, L"SystemSettings") || CaseInsensitiveContains(cleanExe, L"Settings")) &&
        (CaseInsensitiveContains(cleanTarget, L"SystemSettings") || CaseInsensitiveContains(cleanTarget, L"Settings"))) {
        return true;
    }

    // Special handling for MS Store
    if ((CaseInsensitiveContains(cleanExe, L"WinStore") || CaseInsensitiveContains(cleanExe, L"WindowsStore") || CaseInsensitiveContains(cleanExe, L"Store")) &&
        (CaseInsensitiveContains(cleanTarget, L"WinStore") || CaseInsensitiveContains(cleanTarget, L"WindowsStore") || CaseInsensitiveContains(cleanTarget, L"Store"))) {
        return true;
    }

    return false;
}

void LoadSettings() {
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    std::wstring exeName = exePath;
    size_t lastSlash = exeName.find_last_of(L"\\/");
    if (lastSlash != std::wstring::npos) {
        exeName = exeName.substr(lastSlash + 1);
    }

    DWORD pid = GetCurrentProcessId();
    ModLog(L"LoadSettings: Scanning rules for process '%s' (PID %u)", exeName.c_str(), pid);

    g_shouldApply = false;
    g_scaleMultiplier = 1.0;
    g_isWlmail = CaseInsensitiveContains(exeName, L"wlmail.exe");
    g_isMmc = CaseInsensitiveContains(exeName, L"mmc.exe");
    g_isNotepad = CaseInsensitiveContains(exeName, L"notepad.exe");
    g_isOlk = CaseInsensitiveContains(exeName, L"olk.exe");

    for (int i = 0;; i++) {
        wchar_t key[128];
        swprintf_s(key, ARRAYSIZE(key), L"apps[%d].processName", i);
        PCWSTR processName = Wh_GetStringSetting(key);
        if (!processName) {
            break; // No more items
        }
            
        std::wstring target(processName);
        Wh_FreeStringSetting(processName);
        
        // Trim target string safely
        size_t first = target.find_first_not_of(L" \t\r\n");
        if (first == std::wstring::npos) {
            continue; // All whitespace
        }
        size_t last = target.find_last_not_of(L" \t\r\n");
        target = target.substr(first, (last - first + 1));
        
        if (!target.empty() && MatchesProcessName(exeName, target)) {
            swprintf_s(key, ARRAYSIZE(key), L"apps[%d].scalePercentage", i);
            int percentage = Wh_GetIntSetting(key);
            if (percentage > 0) {
                g_scaleMultiplier = (double)percentage / 100.0;
            } else {
                g_scaleMultiplier = 1.0; // Fallback to 100% if not specified or empty
            }
            if (g_scaleMultiplier <= 0.1) g_scaleMultiplier = 1.0;
            
            swprintf_s(key, ARRAYSIZE(key), L"apps[%d].enableChromiumCmdLine", i);
            g_enableChromiumCmdLine = Wh_GetIntSetting(key) != 0;
            
            swprintf_s(key, ARRAYSIZE(key), L"apps[%d].enableDpiHooks", i);
            g_enableDpiHooks = Wh_GetIntSetting(key) != 0;
            
            swprintf_s(key, ARRAYSIZE(key), L"apps[%d].excludeMenuBar", i);
            g_excludeMenuBar = Wh_GetIntSetting(key) != 0;

            swprintf_s(key, ARRAYSIZE(key), L"apps[%d].enableGdiFontHooks", i);
            g_enableGdiFontHooks = Wh_GetIntSetting(key) != 0;

            swprintf_s(key, ARRAYSIZE(key), L"apps[%d].fontScalePercentage", i);
            int fontPercentage = Wh_GetIntSetting(key);
            if (fontPercentage > 0) {
                g_fontScaleMultiplier = (double)fontPercentage / 100.0;
            } else {
                g_fontScaleMultiplier = g_scaleMultiplier;
            }
            if (g_fontScaleMultiplier <= 0.1) g_fontScaleMultiplier = g_scaleMultiplier;

            swprintf_s(key, ARRAYSIZE(key), L"apps[%d].enableGdiImageHooks", i);
            g_enableGdiImageHooks = Wh_GetIntSetting(key) != 0;

            swprintf_s(key, ARRAYSIZE(key), L"apps[%d].imageScalePercentage", i);
            int imagePercentage = Wh_GetIntSetting(key);
            if (imagePercentage > 0) {
                g_imageScaleMultiplier = (double)imagePercentage / 100.0;
            } else {
                g_imageScaleMultiplier = g_scaleMultiplier;
            }
            if (g_imageScaleMultiplier <= 0.1) g_imageScaleMultiplier = g_scaleMultiplier;

            swprintf_s(key, ARRAYSIZE(key), L"apps[%d].enableThemeHooks", i);
            g_enableThemeHooks = Wh_GetIntSetting(key) != 0;

            swprintf_s(key, ARRAYSIZE(key), L"apps[%d].enableAdjustWindowRectHooks", i);
            g_enableAdjustWindowRectHooks = Wh_GetIntSetting(key) != 0;
            
            g_shouldApply = true;

            // Set WebView2 environment variable for apps using WebView2 / Chromium backend (e.g. New Outlook olk.exe)
            if (g_enableChromiumCmdLine && g_scaleMultiplier != 1.0) {
                wchar_t wv2Arg[256];
                swprintf_s(wv2Arg, ARRAYSIZE(wv2Arg), L"--force-device-scale-factor=%.3f", g_scaleMultiplier);
                SetEnvironmentVariableW(L"WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS", wv2Arg);
                ModLog(L"LoadSettings: Set WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS='%s'", wv2Arg);
            }

            ModLog(L"LoadSettings: MATCHED rule #%d ('%s') for '%s' -> Scale: %.2f (%d%%), FontScale: %.2f, ImgScale: %.2f, DpiHooks: %d, ChromiumCmd: %d, GdiFont: %d, GdiImg: %d, Theme: %d, AdjRect: %d",
                   i, target.c_str(), exeName.c_str(), g_scaleMultiplier, percentage, g_fontScaleMultiplier, g_imageScaleMultiplier, g_enableDpiHooks,
                   g_enableChromiumCmdLine, g_enableGdiFontHooks, g_enableGdiImageHooks, g_enableThemeHooks, g_enableAdjustWindowRectHooks);
            break; // Match found, stop checking
        }
    }

    // If the process was included in Windhawk process inclusion list but has no entry configured in app settings,
    // ensure g_scaleMultiplier safely defaults to 1.0 (100%) and g_shouldApply is false so no intrusive hooks run.
    if (!g_shouldApply) {
        g_scaleMultiplier = 1.0;
        g_fontScaleMultiplier = 1.0;
        g_imageScaleMultiplier = 1.0;
        g_enableDpiHooks = false;
        g_enableChromiumCmdLine = false;
        g_enableGdiFontHooks = false;
        g_enableGdiImageHooks = false;
        g_enableThemeHooks = false;
        g_enableAdjustWindowRectHooks = false;
        ModLog(L"LoadSettings: Process '%s' (PID %u) is NOT configured in settings. g_shouldApply=false (all hooks bypassed).", exeName.c_str(), pid);
    }
}

void InstallHooksIfNeeded() {
    if (g_hooksInstalled) return;
    g_hooksInstalled = true;

    ModLog(L"InstallHooksIfNeeded: Installing hooks with ScaleMultiplier=%.2f", g_scaleMultiplier);

    HMODULE hUser32 = GetModuleHandle(L"user32.dll");
    if (hUser32) {
        InitModuleRange(hUser32, &g_user32_start, &g_user32_end);
    }

    HMODULE hGdi32 = GetModuleHandle(L"gdi32.dll");
    if (hGdi32) {
        InitModuleRange(hGdi32, &g_gdi32_start, &g_gdi32_end);
    }

    HMODULE hComctl32 = GetModuleHandle(L"comctl32.dll");
    if (hComctl32) {
        InitModuleRange(hComctl32, &g_comctl32_start, &g_comctl32_end);
    }

    HMODULE hUxTheme = GetModuleHandle(L"uxtheme.dll");
    if (hUxTheme) {
        InitModuleRange(hUxTheme, &g_uxtheme_start, &g_uxtheme_end);
    }

    // Always install DPI, GDI, theme, and layout hooks unconditionally to support real-time toggles
    {
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
            void* pGetDpiFromDpiAwarenessContext = (void*)GetProcAddress(hUser32, "GetDpiFromDpiAwarenessContext");
            if (pGetDpiFromDpiAwarenessContext) {
                Wh_SetFunctionHook(pGetDpiFromDpiAwarenessContext,
                                   (void*)GetDpiFromDpiAwarenessContext_Hook,
                                   (void**)&GetDpiFromDpiAwarenessContext_Orig);
            }
            void* pGetSystemDpiForProcess = (void*)GetProcAddress(hUser32, "GetSystemDpiForProcess");
            if (pGetSystemDpiForProcess) {
                Wh_SetFunctionHook(pGetSystemDpiForProcess,
                                   (void*)GetSystemDpiForProcess_Hook,
                                   (void**)&GetSystemDpiForProcess_Orig);
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
                // Hook removed to prevent crashes in WLM and DirectUI/Ribbon apps
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
            void* pAdjustWindowRect = (void*)GetProcAddress(hUser32, "AdjustWindowRect");
            if (pAdjustWindowRect) {
                Wh_SetFunctionHook(pAdjustWindowRect,
                                   (void*)AdjustWindowRect_Hook,
                                   (void**)&AdjustWindowRect_Orig);
            }
            void* pAdjustWindowRectEx = (void*)GetProcAddress(hUser32, "AdjustWindowRectEx");
            if (pAdjustWindowRectEx) {
                Wh_SetFunctionHook(pAdjustWindowRectEx,
                                   (void*)AdjustWindowRectEx_Hook,
                                   (void**)&AdjustWindowRectEx_Orig);
            }
            void* pAdjustWindowRectExForDpi = (void*)GetProcAddress(hUser32, "AdjustWindowRectExForDpi");
            if (pAdjustWindowRectExForDpi) {
                Wh_SetFunctionHook(pAdjustWindowRectExForDpi,
                                   (void*)AdjustWindowRectExForDpi_Hook,
                                   (void**)&AdjustWindowRectExForDpi_Orig);
            }
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
            void* pCreateFontIndirectA = (void*)GetProcAddress(hGdi32, "CreateFontIndirectA");
            if (pCreateFontIndirectA) {
                Wh_SetFunctionHook(pCreateFontIndirectA,
                                   (void*)CreateFontIndirectA_Hook,
                                   (void**)&CreateFontIndirectA_Orig);
            }
            void* pCreateFontW = (void*)GetProcAddress(hGdi32, "CreateFontW");
            if (pCreateFontW) {
                Wh_SetFunctionHook(pCreateFontW,
                                   (void*)CreateFontW_Hook,
                                   (void**)&CreateFontW_Orig);
            }
            void* pCreateFontA = (void*)GetProcAddress(hGdi32, "CreateFontA");
            if (pCreateFontA) {
                Wh_SetFunctionHook(pCreateFontA,
                                   (void*)CreateFontA_Hook,
                                   (void**)&CreateFontA_Orig);
            }
            void* pGetObjectW = (void*)GetProcAddress(hGdi32, "GetObjectW");
            if (pGetObjectW) {
                Wh_SetFunctionHook(pGetObjectW,
                                   (void*)GetObjectW_Hook,
                                   (void**)&GetObjectW_Orig);
            }
            void* pGetObjectA = (void*)GetProcAddress(hGdi32, "GetObjectA");
            if (pGetObjectA) {
                Wh_SetFunctionHook(pGetObjectA,
                                   (void*)GetObjectA_Hook,
                                   (void**)&GetObjectA_Orig);
            }
            void* pGetStockObject = (void*)GetProcAddress(hGdi32, "GetStockObject");
            if (pGetStockObject) {
                Wh_SetFunctionHook(pGetStockObject,
                                   (void*)GetStockObject_Hook,
                                   (void**)&GetStockObject_Orig);
            }
            void* pDeleteObject = (void*)GetProcAddress(hGdi32, "DeleteObject");
            if (pDeleteObject) {
                Wh_SetFunctionHook(pDeleteObject,
                                   (void*)DeleteObject_Hook,
                                   (void**)&DeleteObject_Orig);
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
            void* pGetScaleFactorForDevice = (void*)GetProcAddress(hShcore, "GetScaleFactorForDevice");
            if (pGetScaleFactorForDevice) {
                Wh_SetFunctionHook(pGetScaleFactorForDevice,
                                   (void*)GetScaleFactorForDevice_Hook,
                                   (void**)&GetScaleFactorForDevice_Orig);
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

        HMODULE hCombase = GetModuleHandle(L"combase.dll");
        if (hCombase) {
            pWindowsGetStringRawBuffer = (WindowsGetStringRawBuffer_t)GetProcAddress(hCombase, "WindowsGetStringRawBuffer");
            void* pRoGetActivationFactory = (void*)GetProcAddress(hCombase, "RoGetActivationFactory");
            if (pRoGetActivationFactory) {
                Wh_SetFunctionHook(pRoGetActivationFactory,
                                   (void*)RoGetActivationFactory_Hook,
                                   (void**)&RoGetActivationFactory_Orig);
            }
            void* pRoActivateInstance = (void*)GetProcAddress(hCombase, "RoActivateInstance");
            if (pRoActivateInstance) {
                Wh_SetFunctionHook(pRoActivateInstance,
                                   (void*)RoActivateInstance_Hook,
                                   (void**)&RoActivateInstance_Orig);
            }
        }
    }

    {
        HMODULE hKernel32 = GetModuleHandle(L"kernel32.dll");
        if (hKernel32) {
            void* pGetCommandLineW = (void*)GetProcAddress(hKernel32, "GetCommandLineW");
            if (pGetCommandLineW) {
                Wh_SetFunctionHook(pGetCommandLineW,
                                   (void*)GetCommandLineW_Hook,
                                   (void**)&GetCommandLineW_Orig);
            }
            void* pCreateProcessW = (void*)GetProcAddress(hKernel32, "CreateProcessW");
            if (pCreateProcessW) {
                Wh_SetFunctionHook(pCreateProcessW,
                                   (void*)CreateProcessW_Hook,
                                   (void**)&CreateProcessW_Orig);
            }
        }
    }
 
    ModLog(L"InstallHooksIfNeeded: Completed successfully for PID %u", GetCurrentProcessId());
}

BOOL Wh_ModInit() {
    LoadSettings();

    // If this process is not configured in mod settings, defer hook registration until settings are added.
    if (!g_shouldApply) {
        ModLog(L"Wh_ModInit: Process not configured in mod settings. Skipping hook installation.");
        return TRUE;
    }

    InstallHooksIfNeeded();
    return TRUE;
}

void Wh_ModAfterInit() {
    ModLog(L"Wh_ModAfterInit: Initialized for PID %u", GetCurrentProcessId());
}

void Wh_ModUninit() {
    CleanupStockFonts();
}

void Wh_ModSettingsChanged() {
    CleanupStockFonts();
    LoadSettings();
    if (g_shouldApply) {
        InstallHooksIfNeeded();
    }
}
