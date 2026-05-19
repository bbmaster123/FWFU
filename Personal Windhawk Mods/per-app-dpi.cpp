// ==WindhawkMod==
// @id              per-app-dpi-override
// @name            Per-App Custom DPI Override
// @description     Overrides the DPI scaling factor for specific applications.
// @version         1.1.0
// @author          AI
// @include         *
// @compilerOptions -lshcore

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
- scalePercentage: 111
  $name: Scale Percentage
  $description: The scale factor as a percentage to apply (e.g., 125 for 125% scale, 80 for 80% scale).
- processNames: msedge.exe, notepad.exe
  $name: Target Processes (comma-separated)
  $description: The process execution names to target. e.g., msedge.exe, chrome.exe, notepad.exe
- enableChromiumCmdLine: true
  $name: Enable Chromium Command Line Injection
  $description: Automatically injects --force-device-scale-factor for Chromium browsers. Highly recommended! Note* You must fully close the browser first.
- enableDpiHooks: true
  $name: Enable Windows DPI API Hooks
  $description: Hooks standard Windows DPI APIs (GetDpiForWindow, GetDeviceCaps, etc.). Can cause border artifacts in some apps. Disable if you only want Chromium scaling.
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <shellscalingapi.h>
#include <windhawk_api.h>
#include <string>
#include <vector>
#include <algorithm>
#include <stdio.h>

double g_scaleMultiplier = 1.0;
bool g_enableChromiumCmdLine = true;
bool g_enableDpiHooks = true;

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
    return (UINT)(dpi * g_scaleMultiplier);
}

typedef UINT(WINAPI* GetDpiForSystem_t)();
GetDpiForSystem_t GetDpiForSystem_Orig;

UINT WINAPI GetDpiForSystem_Hook() {
    UINT dpi = GetDpiForSystem_Orig();
    return (UINT)(dpi * g_scaleMultiplier);
}

typedef int(WINAPI* GetDeviceCaps_t)(HDC hdc, int index);
GetDeviceCaps_t GetDeviceCaps_Orig;

int WINAPI GetDeviceCaps_Hook(HDC hdc, int index) {
    if (index == LOGPIXELSX || index == LOGPIXELSY) {
        int dpi = GetDeviceCaps_Orig(hdc, index);
        return (int)(dpi * g_scaleMultiplier);
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
    int percentage = Wh_GetIntSetting(L"scalePercentage");
    if (percentage > 0) {
        g_scaleMultiplier = (double)percentage / 100.0;
    } else {
        g_scaleMultiplier = 1.0;
    }
    if (g_scaleMultiplier <= 0.1) g_scaleMultiplier = 1.0;

    g_enableChromiumCmdLine = Wh_GetIntSetting(L"enableChromiumCmdLine") != 0;
    g_enableDpiHooks = Wh_GetIntSetting(L"enableDpiHooks") != 0;
}

BOOL Wh_ModInit() {
    PCWSTR processesStr = Wh_GetStringSetting(L"processNames");
    bool shouldApply = false;
    
    if (processesStr && processesStr[0] != L'\0') {
        std::wstring processList(processesStr);
        
        wchar_t exePath[MAX_PATH];
        GetModuleFileNameW(NULL, exePath, MAX_PATH);
        std::wstring exeName = exePath;
        size_t lastSlash = exeName.find_last_of(L"\\/");
        if (lastSlash != std::wstring::npos) {
            exeName = exeName.substr(lastSlash + 1);
        }
        
        // simple comma separate check
        size_t start = 0;
        size_t end = processList.find(L',');
        while (start != std::wstring::npos) {
            std::wstring target = processList.substr(start, end - start);
            // trim
            target.erase(0, target.find_first_not_of(L" \t"));
            target.erase(target.find_last_not_of(L" \t") + 1);
            
            if (!target.empty() && CaseInsensitiveContains(exeName, target)) {
                shouldApply = true;
                break;
            }
            
            if (end == std::wstring::npos) {
                break;
            }
            start = end + 1;
            end = processList.find(L',', start);
        }
    }
    Wh_FreeStringSetting(processesStr);

    if (!shouldApply) {
        return TRUE;
    }

    LoadSettings();

    if (g_enableDpiHooks) {
        HMODULE hUser32 = GetModuleHandle(L"user32.dll");
        if (hUser32) {
            Wh_SetFunctionHook((void*)GetProcAddress(hUser32, "GetDpiForWindow"),
                               (void*)GetDpiForWindow_Hook,
                               (void**)&GetDpiForWindow_Orig);
            Wh_SetFunctionHook((void*)GetProcAddress(hUser32, "GetDpiForSystem"),
                               (void*)GetDpiForSystem_Hook,
                               (void**)&GetDpiForSystem_Orig);
        }

        HMODULE hGdi32 = GetModuleHandle(L"gdi32.dll");
        if (hGdi32) {
            Wh_SetFunctionHook((void*)GetProcAddress(hGdi32, "GetDeviceCaps"),
                               (void*)GetDeviceCaps_Hook,
                               (void**)&GetDeviceCaps_Orig);
        }

        HMODULE hShcore = GetModuleHandle(L"shcore.dll");
        if (hShcore) {
            Wh_SetFunctionHook((void*)GetProcAddress(hShcore, "GetDpiForMonitor"),
                               (void*)GetDpiForMonitor_Hook,
                               (void**)&GetDpiForMonitor_Orig);
        }
    }

    if (g_enableChromiumCmdLine) {
        HMODULE hKernel32 = GetModuleHandle(L"kernel32.dll");
        if (hKernel32) {
            Wh_SetFunctionHook((void*)GetProcAddress(hKernel32, "GetCommandLineW"),
                               (void*)GetCommandLineW_Hook,
                               (void**)&GetCommandLineW_Orig);
        }
    }

    return TRUE;
}

void Wh_ModSettingsChanged() {
    LoadSettings();
}
