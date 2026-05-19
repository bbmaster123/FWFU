// ==WindhawkMod==
// @id              per-app-dpi-override
// @name            Per-App Custom DPI Override
// @description     Overrides the DPI scaling factor for specific applications.
// @version         1.9.0
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

static void* g_user32_start = nullptr;
static void* g_user32_end = nullptr;

bool CaseInsensitiveContains(const std::wstring& str, const std::wstring& sub) {
    auto it = std::search(
        str.begin(), str.end(),
        sub.begin(), sub.end(),
        [](wchar_t ch1, wchar_t ch2) { return std::towlower(ch1) == std::towlower(ch2); }
    );
    return it != str.end();
}

// -------------------------------------------------------------------------
// DPI HOOKS
// -------------------------------------------------------------------------

typedef UINT(WINAPI* GetDpiForWindow_t)(HWND hwnd);
GetDpiForWindow_t GetDpiForWindow_Orig;

UINT WINAPI GetDpiForWindow_Hook(HWND hwnd) {
    UINT dpi = GetDpiForWindow_Orig(hwnd);
    if (dpi == 0) return 0;
    
    void* retAddr = _ReturnAddress();
    if (retAddr >= g_user32_start && retAddr < g_user32_end) {
        return dpi; // Bypass internal user32.dll calls to prevent crashes
    }
    
    return (UINT)(dpi * g_scaleMultiplier);
}

typedef UINT(WINAPI* GetDpiForSystem_t)();
GetDpiForSystem_t GetDpiForSystem_Orig;

UINT WINAPI GetDpiForSystem_Hook() {
    UINT dpi = GetDpiForSystem_Orig();
    
    void* retAddr = _ReturnAddress();
    if (retAddr >= g_user32_start && retAddr < g_user32_end) {
        return dpi;
    }
    
    return (UINT)(dpi * g_scaleMultiplier);
}

// removed GetSystemMetricsForDpi_Hook

typedef int(WINAPI* GetSystemMetricsForDpi_t)(int nIndex, UINT dpi);
GetSystemMetricsForDpi_t GetSystemMetricsForDpi_Orig;

int WINAPI GetSystemMetricsForDpi_Hook(int nIndex, UINT dpi) {
    if (dpi > 0) dpi = (UINT)(dpi * g_scaleMultiplier);
    int metric = GetSystemMetricsForDpi_Orig(nIndex, dpi);
    
    void* retAddr = _ReturnAddress();
    if (retAddr >= g_user32_start && retAddr < g_user32_end) {
        return metric;
    }
    
    return metric;
}

typedef int(WINAPI* GetSystemMetrics_t)(int nIndex);
GetSystemMetrics_t GetSystemMetrics_Orig;

int WINAPI GetSystemMetrics_Hook(int nIndex) {
    int metric = GetSystemMetrics_Orig(nIndex);
    
    void* retAddr = _ReturnAddress();
    if (retAddr >= g_user32_start && retAddr < g_user32_end) {
        return metric;
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
                return metric; // Skip scaling bounds/screen sizes
            case SM_CYMENU:
            case SM_CXMENUSIZE:
            case SM_CYMENUSIZE:
            case SM_CXMENUCHECK:
            case SM_CYMENUCHECK:
                return g_excludeMenuBar ? metric : (int)(metric * g_scaleMultiplier);
            default:
                return (int)(metric * g_scaleMultiplier);
        }
    }
    
    return metric;
}

// removed SystemParametersInfoForDpi_Hook

typedef BOOL(WINAPI* SystemParametersInfoForDpi_t)(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni, UINT dpi);
SystemParametersInfoForDpi_t SystemParametersInfoForDpi_Orig;

BOOL WINAPI SystemParametersInfoForDpi_Hook(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni, UINT dpi) {
    if (dpi > 0) dpi = (UINT)(dpi * g_scaleMultiplier);
    BOOL ret = SystemParametersInfoForDpi_Orig(uiAction, uiParam, pvParam, fWinIni, dpi);
    if (!ret) return ret;

    void* retAddr = _ReturnAddress();
    if (retAddr >= g_user32_start && retAddr < g_user32_end) {
        return ret;
    }
    
    return ret;
}

typedef BOOL(WINAPI* SystemParametersInfoW_t)(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni);
SystemParametersInfoW_t SystemParametersInfoW_Orig;

BOOL WINAPI SystemParametersInfoW_Hook(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni) {
    BOOL ret = SystemParametersInfoW_Orig(uiAction, uiParam, pvParam, fWinIni);
    if (!ret) return ret;

    void* retAddr = _ReturnAddress();
    if (retAddr >= g_user32_start && retAddr < g_user32_end) {
        return ret;
    }

    if (uiAction == SPI_GETNONCLIENTMETRICS && pvParam) {
        NONCLIENTMETRICSW* ncm = (NONCLIENTMETRICSW*)pvParam;
        
        auto scaleFont = [](LOGFONTW& lf) {
            lf.lfHeight = (LONG)(lf.lfHeight * g_scaleMultiplier);
            if (lf.lfWidth != 0) lf.lfWidth = (LONG)(lf.lfWidth * g_scaleMultiplier);
        };
        
        scaleFont(ncm->lfCaptionFont);
        scaleFont(ncm->lfSmCaptionFont);
        if (!g_excludeMenuBar) scaleFont(ncm->lfMenuFont);
        // Do not scale status or message fonts to prevent double-scaling in DirectUI apps like WLM
        // scaleFont(ncm->lfStatusFont);
        // scaleFont(ncm->lfMessageFont);
        
        ncm->iBorderWidth = (int)(ncm->iBorderWidth * g_scaleMultiplier);
        ncm->iScrollWidth = (int)(ncm->iScrollWidth * g_scaleMultiplier);
        ncm->iScrollHeight = (int)(ncm->iScrollHeight * g_scaleMultiplier);
        ncm->iCaptionWidth = (int)(ncm->iCaptionWidth * g_scaleMultiplier);
        ncm->iCaptionHeight = (int)(ncm->iCaptionHeight * g_scaleMultiplier);
        ncm->iSmCaptionWidth = (int)(ncm->iSmCaptionWidth * g_scaleMultiplier);
        ncm->iSmCaptionHeight = (int)(ncm->iSmCaptionHeight * g_scaleMultiplier);
        if (!g_excludeMenuBar) {
            ncm->iMenuWidth = (int)(ncm->iMenuWidth * g_scaleMultiplier);
            ncm->iMenuHeight = (int)(ncm->iMenuHeight * g_scaleMultiplier);
        }
        
        // Check if cbSize is large enough to contain iPaddedBorderWidth (added in Vista)
        if (ncm->cbSize >= (unsigned)(offsetof(NONCLIENTMETRICSW, iPaddedBorderWidth) + sizeof(int))) {
            ncm->iPaddedBorderWidth = (int)(ncm->iPaddedBorderWidth * g_scaleMultiplier);
        }
    }
    else if (uiAction == SPI_GETICONTITLELOGFONT && pvParam) {
        LOGFONTW* lf = (LOGFONTW*)pvParam;
        lf->lfHeight = (LONG)(lf->lfHeight * g_scaleMultiplier);
        if (lf->lfWidth != 0) lf->lfWidth = (LONG)(lf->lfWidth * g_scaleMultiplier);
    }
    else if (uiAction == SPI_GETICONMETRICS && pvParam) {
        ICONMETRICSW* im = (ICONMETRICSW*)pvParam;
        if (im->cbSize >= (unsigned)(offsetof(ICONMETRICSW, lfFont) + sizeof(LOGFONTW))) {
            im->iHorzSpacing = (int)(im->iHorzSpacing * g_scaleMultiplier);
            im->iVertSpacing = (int)(im->iVertSpacing * g_scaleMultiplier);
            im->lfFont.lfHeight = (LONG)(im->lfFont.lfHeight * g_scaleMultiplier);
            if (im->lfFont.lfWidth != 0) im->lfFont.lfWidth = (LONG)(im->lfFont.lfWidth * g_scaleMultiplier);
        }
    }
    
    return ret;
}

typedef BOOL(WINAPI* SystemParametersInfoA_t)(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni);
SystemParametersInfoA_t SystemParametersInfoA_Orig;

BOOL WINAPI SystemParametersInfoA_Hook(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni) {
    BOOL ret = SystemParametersInfoA_Orig(uiAction, uiParam, pvParam, fWinIni);
    if (!ret) return ret;

    void* retAddr = _ReturnAddress();
    if (retAddr >= g_user32_start && retAddr < g_user32_end) {
        return ret;
    }

    if (uiAction == SPI_GETNONCLIENTMETRICS && pvParam) {
        NONCLIENTMETRICSA* ncm = (NONCLIENTMETRICSA*)pvParam;
        
        auto scaleFont = [](LOGFONTA& lf) {
            lf.lfHeight = (LONG)(lf.lfHeight * g_scaleMultiplier);
            if (lf.lfWidth != 0) lf.lfWidth = (LONG)(lf.lfWidth * g_scaleMultiplier);
        };
        
        scaleFont(ncm->lfCaptionFont);
        scaleFont(ncm->lfSmCaptionFont);
        if (!g_excludeMenuBar) scaleFont(ncm->lfMenuFont);
        // Do not scale status or message fonts to prevent double-scaling in DirectUI apps like WLM
        // scaleFont(ncm->lfStatusFont);
        // scaleFont(ncm->lfMessageFont);
        
        ncm->iBorderWidth = (int)(ncm->iBorderWidth * g_scaleMultiplier);
        ncm->iScrollWidth = (int)(ncm->iScrollWidth * g_scaleMultiplier);
        ncm->iScrollHeight = (int)(ncm->iScrollHeight * g_scaleMultiplier);
        ncm->iCaptionWidth = (int)(ncm->iCaptionWidth * g_scaleMultiplier);
        ncm->iCaptionHeight = (int)(ncm->iCaptionHeight * g_scaleMultiplier);
        ncm->iSmCaptionWidth = (int)(ncm->iSmCaptionWidth * g_scaleMultiplier);
        ncm->iSmCaptionHeight = (int)(ncm->iSmCaptionHeight * g_scaleMultiplier);
        if (!g_excludeMenuBar) {
            ncm->iMenuWidth = (int)(ncm->iMenuWidth * g_scaleMultiplier);
            ncm->iMenuHeight = (int)(ncm->iMenuHeight * g_scaleMultiplier);
        }
        
        if (ncm->cbSize >= (unsigned)(offsetof(NONCLIENTMETRICSA, iPaddedBorderWidth) + sizeof(int))) {
            ncm->iPaddedBorderWidth = (int)(ncm->iPaddedBorderWidth * g_scaleMultiplier);
        }
    }
    else if (uiAction == SPI_GETICONTITLELOGFONT && pvParam) {
        LOGFONTA* lf = (LOGFONTA*)pvParam;
        lf->lfHeight = (LONG)(lf->lfHeight * g_scaleMultiplier);
        if (lf->lfWidth != 0) lf->lfWidth = (LONG)(lf->lfWidth * g_scaleMultiplier);
    }
    else if (uiAction == SPI_GETICONMETRICS && pvParam) {
        ICONMETRICSA* im = (ICONMETRICSA*)pvParam;
        if (im->cbSize >= (unsigned)(offsetof(ICONMETRICSA, lfFont) + sizeof(LOGFONTA))) {
            im->iHorzSpacing = (int)(im->iHorzSpacing * g_scaleMultiplier);
            im->iVertSpacing = (int)(im->iVertSpacing * g_scaleMultiplier);
            im->lfFont.lfHeight = (LONG)(im->lfFont.lfHeight * g_scaleMultiplier);
            if (im->lfFont.lfWidth != 0) im->lfFont.lfWidth = (LONG)(im->lfFont.lfWidth * g_scaleMultiplier);
        }
    }
    
    return ret;
}

typedef HANDLE(WINAPI* LoadImageW_t)(HINSTANCE hInst, LPCWSTR name, UINT type, int cx, int cy, UINT fuLoad);
LoadImageW_t LoadImageW_Orig;

HANDLE WINAPI LoadImageW_Hook(HINSTANCE hInst, LPCWSTR name, UINT type, int cx, int cy, UINT fuLoad) {
    if (cx > 0 && cy > 0 && type == IMAGE_ICON) {
        cx = (int)(cx * g_scaleMultiplier);
        cy = (int)(cy * g_scaleMultiplier);
    }
    return LoadImageW_Orig(hInst, name, type, cx, cy, fuLoad);
}

typedef HANDLE(WINAPI* LoadImageA_t)(HINSTANCE hInst, LPCSTR name, UINT type, int cx, int cy, UINT fuLoad);
LoadImageA_t LoadImageA_Orig;

HANDLE WINAPI LoadImageA_Hook(HINSTANCE hInst, LPCSTR name, UINT type, int cx, int cy, UINT fuLoad) {
    if (cx > 0 && cy > 0 && type == IMAGE_ICON) {
        cx = (int)(cx * g_scaleMultiplier);
        cy = (int)(cy * g_scaleMultiplier);
    }
    return LoadImageA_Orig(hInst, name, type, cx, cy, fuLoad);
}

// -------------------------------------------------------------------------
// UXTHEME HOOKS
// -------------------------------------------------------------------------

typedef int(WINAPI* GetThemeSysSize_t)(HANDLE hTheme, int iSizeId);
GetThemeSysSize_t GetThemeSysSize_Orig;

int WINAPI GetThemeSysSize_Hook(HANDLE hTheme, int iSizeId) {
    int metric = GetThemeSysSize_Orig(hTheme, iSizeId);
    if (metric > 0) return (int)(metric * g_scaleMultiplier);
    return metric;
}

typedef HRESULT(WINAPI* GetThemeSysFont_t)(HANDLE hTheme, int iFontId, LOGFONTW* plf);
GetThemeSysFont_t GetThemeSysFont_Orig;

HRESULT WINAPI GetThemeSysFont_Hook(HANDLE hTheme, int iFontId, LOGFONTW* plf) {
    HRESULT hr = GetThemeSysFont_Orig(hTheme, iFontId, plf);
    if (SUCCEEDED(hr) && plf) {
        plf->lfHeight = (LONG)(plf->lfHeight * g_scaleMultiplier);
        if (plf->lfWidth != 0) plf->lfWidth = (LONG)(plf->lfWidth * g_scaleMultiplier);
    }
    return hr;
}

typedef HRESULT(WINAPI* GetThemePartSize_t)(HANDLE hTheme, HDC hdc, int iPartId, int iStateId, PVOID prc, int eSize, SIZE* psz);
GetThemePartSize_t GetThemePartSize_Orig;

HRESULT WINAPI GetThemePartSize_Hook(HANDLE hTheme, HDC hdc, int iPartId, int iStateId, PVOID prc, int eSize, SIZE* psz) {
    HRESULT hr = GetThemePartSize_Orig(hTheme, hdc, iPartId, iStateId, prc, eSize, psz);
    if (SUCCEEDED(hr) && psz) {
        psz->cx = (int)(psz->cx * g_scaleMultiplier);
        psz->cy = (int)(psz->cy * g_scaleMultiplier);
    }
    return hr;
}

typedef int(WINAPI* GetDeviceCaps_t)(HDC hdc, int index);
GetDeviceCaps_t GetDeviceCaps_Orig;

int WINAPI GetDeviceCaps_Hook(HDC hdc, int index) {
    if (index == LOGPIXELSX || index == LOGPIXELSY) {
        int dpi = GetDeviceCaps_Orig(hdc, index);
        if (dpi > 0) {
            return (int)(dpi * g_scaleMultiplier);
        }
    }
    return GetDeviceCaps_Orig(hdc, index);
}

typedef HRESULT(WINAPI* GetDpiForMonitor_t)(HMONITOR hmonitor, MONITOR_DPI_TYPE dpiType, UINT* dpiX, UINT* dpiY);
GetDpiForMonitor_t GetDpiForMonitor_Orig;

HRESULT WINAPI GetDpiForMonitor_Hook(HMONITOR hmonitor, MONITOR_DPI_TYPE dpiType, UINT* dpiX, UINT* dpiY) {
    HRESULT hr = GetDpiForMonitor_Orig(hmonitor, dpiType, dpiX, dpiY);
    if (SUCCEEDED(hr)) {
        if (dpiX) *dpiX = (UINT)(*dpiX * g_scaleMultiplier);
        if (dpiY) *dpiY = (UINT)(*dpiY * g_scaleMultiplier);
    }
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
// MORE COMPAT HOOKS
// -------------------------------------------------------------------------

typedef LRESULT(WINAPI* SendMessageW_t)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
SendMessageW_t SendMessageW_Orig;

#ifndef TB_SETBITMAPSIZE
#define TB_SETBITMAPSIZE (WM_USER + 32)
#endif
#ifndef TB_SETBUTTONSIZE
#define TB_SETBUTTONSIZE (WM_USER + 31)
#endif

LRESULT WINAPI SendMessageW_Hook(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    if (Msg == TB_SETBITMAPSIZE || Msg == TB_SETBUTTONSIZE) {
        int cx = LOWORD(lParam);
        int cy = HIWORD(lParam);
        if (cx > 0 && cy > 0) {
            cx = (int)(cx * g_scaleMultiplier);
            cy = (int)(cy * g_scaleMultiplier);
            lParam = MAKELPARAM(cx, cy);
        }
    } else if (Msg == WM_SETFONT && wParam) {
        wchar_t className[256];
        if (GetClassNameW(hWnd, className, ARRAYSIZE(className)) > 0) {
            if (_wcsicmp(className, L"ToolbarWindow32") == 0) {
                HFONT hFont = (HFONT)wParam;
                LOGFONTW lf;
                if (GetObjectW(hFont, sizeof(lf), &lf)) {
                    lf.lfHeight = (LONG)(lf.lfHeight * g_scaleMultiplier);
                    if (lf.lfWidth != 0) lf.lfWidth = (LONG)(lf.lfWidth * g_scaleMultiplier);
                    HFONT hNewFont = CreateFontIndirectW(&lf);
                    if (hNewFont) {
                        wParam = (WPARAM)hNewFont;
                    }
                }
            }
        }
    }
    return SendMessageW_Orig(hWnd, Msg, wParam, lParam);
}

typedef LRESULT(WINAPI* SendMessageA_t)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
SendMessageA_t SendMessageA_Orig;

LRESULT WINAPI SendMessageA_Hook(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    if (Msg == TB_SETBITMAPSIZE || Msg == TB_SETBUTTONSIZE) {
        int cx = LOWORD(lParam);
        int cy = HIWORD(lParam);
        if (cx > 0 && cy > 0) {
            cx = (int)(cx * g_scaleMultiplier);
            cy = (int)(cy * g_scaleMultiplier);
            lParam = MAKELPARAM(cx, cy);
        }
    } else if (Msg == WM_SETFONT && wParam) {
        char className[256];
        if (GetClassNameA(hWnd, className, ARRAYSIZE(className)) > 0) {
            if (_stricmp(className, "ToolbarWindow32") == 0) {
                HFONT hFont = (HFONT)wParam;
                LOGFONTA lf;
                if (GetObjectA(hFont, sizeof(lf), &lf)) {
                    lf.lfHeight = (LONG)(lf.lfHeight * g_scaleMultiplier);
                    if (lf.lfWidth != 0) lf.lfWidth = (LONG)(lf.lfWidth * g_scaleMultiplier);
                    HFONT hNewFont = CreateFontIndirectA(&lf);
                    if (hNewFont) {
                        wParam = (WPARAM)hNewFont;
                    }
                }
            }
        }
    }
    return SendMessageA_Orig(hWnd, Msg, wParam, lParam);
}

// removed CreateFontIndirectA_Hook / CreateFontIndirectW_Hook

HBITMAP ScaleBitmap(HBITMAP hbm, double scaleMultiplier) {
    if (!hbm) return NULL;
    BITMAP bm;
    if (!GetObjectW(hbm, sizeof(bm), &bm)) return NULL;
    
    int newWidth = (int)(bm.bmWidth * scaleMultiplier);
    int newHeight = (int)(bm.bmHeight * scaleMultiplier);
    
    HDC hdcScreen = GetDC(NULL);
    HDC hdcSrc = CreateCompatibleDC(hdcScreen);
    HDC hdcDest = CreateCompatibleDC(hdcScreen);
    
    HBITMAP hbmNew = CreateCompatibleBitmap(hdcScreen, newWidth, newHeight);
    
    HGDIOBJ hOldSrc = SelectObject(hdcSrc, hbm);
    HGDIOBJ hOldDest = SelectObject(hdcDest, hbmNew);
    
    SetStretchBltMode(hdcDest, HALFTONE);
    SetBrushOrgEx(hdcDest, 0, 0, NULL);
    
    StretchBlt(hdcDest, 0, 0, newWidth, newHeight, hdcSrc, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
    
    SelectObject(hdcSrc, hOldSrc);
    SelectObject(hdcDest, hOldDest);
    
    DeleteDC(hdcSrc);
    DeleteDC(hdcDest);
    ReleaseDC(NULL, hdcScreen);
    
    return hbmNew;
}

typedef HIMAGELIST(WINAPI* ImageList_Create_t)(int cx, int cy, UINT flags, int cInitial, int cGrow);
ImageList_Create_t ImageList_Create_Orig;

HIMAGELIST WINAPI ImageList_Create_Hook(int cx, int cy, UINT flags, int cInitial, int cGrow) {
    if (cx > 0 && cy > 0) {
        cx = (int)(cx * g_scaleMultiplier);
        cy = (int)(cy * g_scaleMultiplier);
    }
    return ImageList_Create_Orig(cx, cy, flags, cInitial, cGrow);
}

typedef int(WINAPI* ImageList_Add_t)(HIMAGELIST himl, HBITMAP hbmImage, HBITMAP hbmMask);
ImageList_Add_t ImageList_Add_Orig;

int WINAPI ImageList_Add_Hook(HIMAGELIST himl, HBITMAP hbmImage, HBITMAP hbmMask) {
    HBITMAP hbmScaledImage = ScaleBitmap(hbmImage, g_scaleMultiplier);
    HBITMAP hbmScaledMask = hbmMask ? ScaleBitmap(hbmMask, g_scaleMultiplier) : NULL;
    
    int ret = ImageList_Add_Orig(himl, hbmScaledImage ? hbmScaledImage : hbmImage, hbmScaledMask ? hbmScaledMask : hbmMask);
    
    if (hbmScaledImage) DeleteObject(hbmScaledImage);
    if (hbmScaledMask) DeleteObject(hbmScaledMask);
    
    return ret;
}

typedef int(WINAPI* ImageList_AddMasked_t)(HIMAGELIST himl, HBITMAP hbmImage, COLORREF crMask);
ImageList_AddMasked_t ImageList_AddMasked_Orig;

int WINAPI ImageList_AddMasked_Hook(HIMAGELIST himl, HBITMAP hbmImage, COLORREF crMask) {
    HBITMAP hbmScaledImage = ScaleBitmap(hbmImage, g_scaleMultiplier);
    int ret = ImageList_AddMasked_Orig(himl, hbmScaledImage ? hbmScaledImage : hbmImage, crMask);
    if (hbmScaledImage) DeleteObject(hbmScaledImage);
    return ret;
}

// Ensure the toolbar's font itself scales (if the user wants the text big)
// Actually, earlier CreateFont did that. Wait, if CreateFont is disabled, pdexplo font doesn't scale.
// If the user wants the text beside it to scale, we must scale the font for Toolbar or the whole app.
// But we avoided it to not double-scale WLM.

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
                Wh_SetFunctionHook(pSystemParametersInfoForDpi,
                                   (void*)SystemParametersInfoForDpi_Hook,
                                   (void**)&SystemParametersInfoForDpi_Orig);
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
// removed CreateFontIndirect hook injection
        }
        
        HMODULE hComctl32 = GetModuleHandle(L"comctl32.dll");
        if (hComctl32) {
            void* pImageList_Create = (void*)GetProcAddress(hComctl32, "ImageList_Create");
            if (pImageList_Create) {
                Wh_SetFunctionHook(pImageList_Create,
                                   (void*)ImageList_Create_Hook,
                                   (void**)&ImageList_Create_Orig);
            }
            void* pImageList_Add = (void*)GetProcAddress(hComctl32, "ImageList_Add");
            if (pImageList_Add) {
                Wh_SetFunctionHook(pImageList_Add,
                                   (void*)ImageList_Add_Hook,
                                   (void**)&ImageList_Add_Orig);
            }
            void* pImageList_AddMasked = (void*)GetProcAddress(hComctl32, "ImageList_AddMasked");
            if (pImageList_AddMasked) {
                Wh_SetFunctionHook(pImageList_AddMasked,
                                   (void*)ImageList_AddMasked_Hook,
                                   (void**)&ImageList_AddMasked_Orig);
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
        
        HMODULE hUxTheme = GetModuleHandle(L"uxtheme.dll");
        if (hUxTheme) {
            void* pGetThemeSysSize = (void*)GetProcAddress(hUxTheme, "GetThemeSysSize");
            if (pGetThemeSysSize) {
                Wh_SetFunctionHook(pGetThemeSysSize,
                                   (void*)GetThemeSysSize_Hook,
                                   (void**)&GetThemeSysSize_Orig);
            }
            void* pGetThemeSysFont = (void*)GetProcAddress(hUxTheme, "GetThemeSysFont");
            if (pGetThemeSysFont) {
                Wh_SetFunctionHook(pGetThemeSysFont,
                                   (void*)GetThemeSysFont_Hook,
                                   (void**)&GetThemeSysFont_Orig);
            }
            void* pGetThemePartSize = (void*)GetProcAddress(hUxTheme, "GetThemePartSize");
            if (pGetThemePartSize) {
                Wh_SetFunctionHook(pGetThemePartSize,
                                   (void*)GetThemePartSize_Hook,
                                   (void**)&GetThemePartSize_Orig);
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
