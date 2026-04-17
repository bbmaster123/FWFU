// ==WindhawkMod==
// @id            fake-reveal-everywhere
// @name          Taskbar Fluent Border Glow
// @description   reveal effect using Win32 for Taskbar and UIA for Apps
// @version       1.4
// @author        Gemini
// @include       windhawk.exe
// @compilerOptions -lgdi32 -luser32 -ldwmapi -lole32 -loleaut32 -luiautomationcore -luuid -lshell32
// ==/WindhawkMod==

// ==WindhawkModSettings==
/*
- GlowColor: "#FFFFFFFF"
  $name: Color (Hex)
- AnimSpeed100: 30
  $name: Animation Speed (100=Instant)
- GlowOpacity: 120
  $name: Spotlight Intensity
- BorderOpacity: 255
  $name: Border Intensity
- BorderThickness10: 15
  $name: Border Thickness (10=1px)
- MarginTop: 0
  $name: Margin - Top
- MarginBottom: 0
  $name: Margin - Bottom
- MarginLeft: 0
  $name: Margin - Left
- MarginRight: 0
  $name: Margin - Right
- GlowSoftness: 350
  $name: Spotlight Spread
- CornerRadius: 6
  $name: Corner Radius
- TargetList: "Button,ListItem,Pane,Group,Image,Text"
  $name: Target Element Types
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <uiautomation.h>
#include <math.h>
#include <wchar.h>

struct {
    COLORREF color;
    float animSpeed;
    int maxGlow, maxBorder;
    float thickness, softness;
    int radius;
    int mTop, mBottom, mLeft, mRight;
    WCHAR targetBuffer[256];
} g_activeSettings;

HWND g_hGlowWnd = NULL;
IUIAutomation* g_pAutomation = NULL;
RECT g_targetRect = {0};
float g_opacitySmooth = 0.0f;
bool g_isToolProcess = false;
HANDLE g_hToolProcess = NULL;

struct GdiCache {
    HDC hdcMem; HBITMAP hBitmap; void* pBits; int w, h;
} g_cache = {0};

void CleanupGdiCache() {
    if (g_cache.hdcMem) {
        DeleteObject(g_cache.hBitmap);
        DeleteDC(g_cache.hdcMem);
        g_cache.hdcMem = NULL;
        g_cache.w = 0; g_cache.h = 0;
    }
}

void LoadSettings() {
    PCWSTR hex = Wh_GetStringSetting(L"GlowColor");
    int r = 255, g = 255, b = 255;
    if (hex && hex[0] == L'#') swscanf(hex + 1, L"%02x%02x%02x", &r, &g, &b);
    g_activeSettings.color = RGB(r, g, b);
    Wh_FreeStringSetting(hex);
    g_activeSettings.animSpeed = (float)Wh_GetIntSetting(L"AnimSpeed100") / 100.0f;
    g_activeSettings.maxGlow = Wh_GetIntSetting(L"GlowOpacity");
    g_activeSettings.maxBorder = Wh_GetIntSetting(L"BorderOpacity");
    g_activeSettings.softness = (float)Wh_GetIntSetting(L"GlowSoftness");
    g_activeSettings.radius = Wh_GetIntSetting(L"CornerRadius");
    g_activeSettings.thickness = (float)Wh_GetIntSetting(L"BorderThickness10") / 10.0f;
    g_activeSettings.mTop = Wh_GetIntSetting(L"MarginTop");
    g_activeSettings.mBottom = Wh_GetIntSetting(L"MarginBottom");
    g_activeSettings.mLeft = Wh_GetIntSetting(L"MarginLeft");
    g_activeSettings.mRight = Wh_GetIntSetting(L"MarginRight");
    PCWSTR targets = Wh_GetStringSetting(L"TargetList");
    wcsncpy(g_activeSettings.targetBuffer, targets ? targets : L"Button", 255);
    Wh_FreeStringSetting(targets);
}

float GetDistToRoundedRect(float x, float y, float w, float h, float r) {
    float dx = fmaxf(fmaxf(r - x, x - (w - r)), 0.0f);
    float dy = fmaxf(fmaxf(r - y, y - (h - r)), 0.0f);
    return sqrtf(dx * dx + dy * dy);
}

void RenderShine(int w, int h, float masterOpacity, POINT pt, RECT rect) {
    if (w <= 0 || h <= 0 || w > 2000 || h > 2000) return;
    if (w != g_cache.w || h != g_cache.h) {
        CleanupGdiCache();
        HDC hdc = GetDC(NULL);
        g_cache.hdcMem = CreateCompatibleDC(hdc);
        BITMAPINFO bmi = {{0}};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = w; bmi.bmiHeader.biHeight = -h; 
        bmi.bmiHeader.biPlanes = 1; bmi.bmiHeader.biBitCount = 32;
        g_cache.hBitmap = CreateDIBSection(g_cache.hdcMem, &bmi, DIB_RGB_COLORS, &g_cache.pBits, NULL, 0);
        SelectObject(g_cache.hdcMem, g_cache.hBitmap);
        g_cache.w = w; g_cache.h = h;
        ReleaseDC(NULL, hdc);
    }
    if (!g_cache.pBits) return;
    memset(g_cache.pBits, 0, w * h * 4);
    UINT32* pixels = (UINT32*)g_cache.pBits;
    BYTE rt = GetRValue(g_activeSettings.color), gt = GetGValue(g_activeSettings.color), bt = GetBValue(g_activeSettings.color);
    float rx = (float)(pt.x - rect.left), ry = (float)(pt.y - rect.top), r = (float)g_activeSettings.radius;
    for (int i = 0; i < w * h; i++) {
        float x = (float)(i % w), y = (float)(i / w);
        float d = GetDistToRoundedRect(x, y, (float)w, (float)h, r);
        float spot = expf(-((x - rx)*(x - rx) + (y - ry)*(y - ry)) / fmaxf(1.0f, g_activeSettings.softness));
        float glowVal = spot * (float)g_activeSettings.maxGlow;
        float borderVal = (d >= (r - g_activeSettings.thickness) && d <= r) ? (float)g_activeSettings.maxBorder : 0.0f;
        float mask = (d <= r) ? 1.0f : fmaxf(0.0f, 1.0f - (d - r));
        int a = (int)(fmaxf(glowVal, borderVal) * (masterOpacity / 255.0f) * mask);
        if (a > 255) a = 255; else if (a < 0) a = 0;
        pixels[i] = (a << 24) | (((a * rt) / 255) << 16) | (((a * gt) / 255) << 8) | ((a * bt) / 255);
    }
    POINT ps = {0, 0}; SIZE sz = {w, h}; BLENDFUNCTION bl = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    UpdateLayeredWindow(g_hGlowWnd, NULL, NULL, &sz, g_cache.hdcMem, &ps, 0, &bl, ULW_ALPHA);
}

void UpdateLogic() {
    POINT pt;
    GetCursorPos(&pt);
    bool found = false;
    RECT foundRect = {0};

    // --- STEP 1: TASKBAR FAST PASS (Win32 Only) ---
    HWND hTaskbar = FindWindowW(L"Shell_TrayWnd", NULL);
    if (hTaskbar) {
        RECT tbRect;
        GetWindowRect(hTaskbar, &tbRect);
        if (PtInRect(&tbRect, pt)) {
            // Find the specific button under the cursor within the taskbar
            POINT ptLocal = pt;
            ScreenToClient(hTaskbar, &ptLocal);
            HWND hChild = ChildWindowFromPointEx(hTaskbar, ptLocal, CWP_SKIPINVISIBLE | CWP_SKIPTRANSPARENT);
            if (hChild && hChild != hTaskbar) {
                if (GetWindowRect(hChild, &foundRect)) {
                    found = true;
                    goto apply_rendering;
                }
            }
        }
    }

    // --- STEP 2: UIA FALLBACK (For Folder items/Apps) ---
    if (g_pAutomation) {
        IUIAutomationElement* pEl = NULL;
        if (SUCCEEDED(g_pAutomation->ElementFromPoint(pt, &pEl)) && pEl) {
            // Check name - if it's Start or Search, skip UIA logic to prevent hangs
            BSTR name = NULL; pEl->get_CurrentName(&name);
            bool isShell = (name && (wcsstr(name, L"Start") || wcsstr(name, L"Search") || wcsstr(name, L"Task View")));
            if (name) SysFreeString(name);

            if (!isShell && SUCCEEDED(pEl->get_CurrentBoundingRectangle(&foundRect))) {
                if ((foundRect.right - foundRect.left) < (GetSystemMetrics(SM_CXSCREEN) / 2)) {
                    foundRect.top += g_activeSettings.mTop;
                    foundRect.bottom -= g_activeSettings.mBottom;
                    foundRect.left += g_activeSettings.mLeft;
                    foundRect.right -= g_activeSettings.mRight;
                    found = true;
                }
            }
            pEl->Release();
        }
    }

apply_rendering:
    if (found) g_targetRect = foundRect;
    g_opacitySmooth += ((found ? 255.0f : 0.0f) - g_opacitySmooth) * g_activeSettings.animSpeed;

    if (g_opacitySmooth > 2.0f) {
        int w = g_targetRect.right - g_targetRect.left;
        int h = g_targetRect.bottom - g_targetRect.top;
        if (w > 0 && h > 0) {
            RenderShine(w, h, g_opacitySmooth, pt, g_targetRect);
            SetWindowPos(g_hGlowWnd, HWND_TOPMOST, g_targetRect.left, g_targetRect.top, w, h, SWP_NOACTIVATE | SWP_SHOWWINDOW);
        }
    } else {
        if (IsWindowVisible(g_hGlowWnd)) ShowWindow(g_hGlowWnd, SW_HIDE);
    }
}

LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (m == WM_TIMER) { UpdateLogic(); return 0; }
    if (m == WM_DESTROY) { CleanupGdiCache(); PostQuitMessage(0); return 0; }
    return DefWindowProc(h, m, w, l);
}

DWORD WINAPI ThreadProc(LPVOID lp) {
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (SUCCEEDED(CoCreateInstance(CLSID_CUIAutomation, NULL, CLSCTX_INPROC_SERVER, IID_IUIAutomation, (void**)&g_pAutomation))) {
        WNDCLASSEXW wc = {sizeof(wc), 0, WndProc, 0, 0, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"FluentGlowWnd", NULL};
        RegisterClassExW(&wc);
        g_hGlowWnd = CreateWindowExW(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, 
                                    L"FluentGlowWnd", L"", WS_POPUP, 0, 0, 0, 0, NULL, NULL, wc.hInstance, NULL);
        SetTimer(g_hGlowWnd, 1, 16, NULL);
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) { TranslateMessage(&msg); DispatchMessage(&msg); }
        if (g_pAutomation) g_pAutomation->Release();
    }
    CoUninitialize();
    return 0;
}

void WINAPI EntryPoint_Hook() { ExitThread(0); }


BOOL WhTool_ModInit() {
    LoadSettings();
    int argc; LPWSTR* argv = CommandLineToArgvW(GetCommandLine(), &argc);
    for (int i = 0; i < argc - 1; i++) {
        if (wcscmp(argv[i], L"-tool-mod") == 0 && wcscmp(argv[i+1], WH_MOD_ID) == 0) g_isToolProcess = true;
    }
    LocalFree(argv);
    if (g_isToolProcess) {
        IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)GetModuleHandle(NULL);
        IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)((BYTE*)dos + dos->e_lfanew);
        Wh_SetFunctionHook((BYTE*)dos + nt->OptionalHeader.AddressOfEntryPoint, (void*)EntryPoint_Hook, NULL);
        CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
    }
    return TRUE;
}

void WhTool_ModAfterInit() {
    if (g_isToolProcess) return;
    WCHAR path[MAX_PATH], cmd[MAX_PATH + 128]; 
    GetModuleFileNameW(NULL, path, MAX_PATH);
    swprintf(cmd, MAX_PATH + 128, L"\"%s\" -tool-mod \"%s\"", path, WH_MOD_ID);
    STARTUPINFOW si = {sizeof(si)}; si.dwFlags = STARTF_FORCEOFFFEEDBACK;
    PROCESS_INFORMATION pi;
    if (CreateProcessW(path, cmd, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi)) {
        g_hToolProcess = pi.hProcess; CloseHandle(pi.hThread);
    }
}

void WhTool_ModSettingsChanged() { LoadSettings(); }
void WhTool_ModUninit() { 
    if (g_hToolProcess) { TerminateProcess(g_hToolProcess, 0); CloseHandle(g_hToolProcess); }
    if (g_hGlowWnd) SendMessage(g_hGlowWnd, WM_CLOSE, 0, 0);
}

////////////////////////////////////////////////////////////////////////////////
// Windhawk tool mod implementation for mods which don't need to inject to other
// processes or hook other functions. Context:
// https://github.com/ramensoftware/windhawk/wiki/Mods-as-tools:-Running-mods-in-a-dedicated-process
//
// The mod will load and run in a dedicated windhawk.exe process.
//
// Paste the code below as part of the mod code, and use these callbacks:
// * WhTool_ModInit
// * WhTool_ModSettingsChanged
// * WhTool_ModUninit
//
// Currently, other callbacks are not supported.

bool g_isToolModProcessLauncher;
HANDLE g_toolModProcessMutex;

BOOL Wh_ModInit() {
    DWORD sessionId;
    if (ProcessIdToSessionId(GetCurrentProcessId(), &sessionId) &&
        sessionId == 0) {
        return FALSE;
    }

    bool isExcluded = false;
    bool isToolModProcess = false;
    bool isCurrentToolModProcess = false;
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLine(), &argc);
    if (!argv) {
        Wh_Log(L"CommandLineToArgvW failed");
        return FALSE;
    }

    for (int i = 1; i < argc; i++) {
        if (wcscmp(argv[i], L"-service") == 0 ||
            wcscmp(argv[i], L"-service-start") == 0 ||
            wcscmp(argv[i], L"-service-stop") == 0) {
            isExcluded = true;
            break;
        }
    }

    for (int i = 1; i < argc - 1; i++) {
        if (wcscmp(argv[i], L"-tool-mod") == 0) {
            isToolModProcess = true;
            if (wcscmp(argv[i + 1], WH_MOD_ID) == 0) {
                isCurrentToolModProcess = true;
            }
            break;
        }
    }

    LocalFree(argv);

    if (isExcluded) {
        return FALSE;
    }

    if (isCurrentToolModProcess) {
        g_toolModProcessMutex =
            CreateMutex(nullptr, TRUE, L"windhawk-tool-mod_" WH_MOD_ID);
        if (!g_toolModProcessMutex) {
            Wh_Log(L"CreateMutex failed");
            ExitProcess(1);
        }

        if (GetLastError() == ERROR_ALREADY_EXISTS) {
            Wh_Log(L"Tool mod already running (%s)", WH_MOD_ID);
            ExitProcess(1);
        }

        if (!WhTool_ModInit()) {
            ExitProcess(1);
        }

        IMAGE_DOS_HEADER* dosHeader =
            (IMAGE_DOS_HEADER*)GetModuleHandle(nullptr);
        IMAGE_NT_HEADERS* ntHeaders =
            (IMAGE_NT_HEADERS*)((BYTE*)dosHeader + dosHeader->e_lfanew);

        DWORD entryPointRVA = ntHeaders->OptionalHeader.AddressOfEntryPoint;
        void* entryPoint = (BYTE*)dosHeader + entryPointRVA;

        Wh_SetFunctionHook(entryPoint, (void*)EntryPoint_Hook, nullptr);
        return TRUE;
    }

    if (isToolModProcess) {
        return FALSE;
    }

    g_isToolModProcessLauncher = true;
    return TRUE;
}

void Wh_ModAfterInit() {
    if (!g_isToolModProcessLauncher) {
        return;
    }

    WCHAR currentProcessPath[MAX_PATH];
    switch (GetModuleFileName(nullptr, currentProcessPath,
                              ARRAYSIZE(currentProcessPath))) {
        case 0:
        case ARRAYSIZE(currentProcessPath):
            Wh_Log(L"GetModuleFileName failed");
            return;
    }

    WCHAR
    commandLine[MAX_PATH + 2 +
                (sizeof(L" -tool-mod \"" WH_MOD_ID "\"") / sizeof(WCHAR)) - 1];
    swprintf_s(commandLine, L"\"%s\" -tool-mod \"%s\"", currentProcessPath,
               WH_MOD_ID);

    HMODULE kernelModule = GetModuleHandle(L"kernelbase.dll");
    if (!kernelModule) {
        kernelModule = GetModuleHandle(L"kernel32.dll");
        if (!kernelModule) {
            Wh_Log(L"No kernelbase.dll/kernel32.dll");
            return;
        }
    }

    using CreateProcessInternalW_t = BOOL(WINAPI*)(
        HANDLE hUserToken, LPCWSTR lpApplicationName, LPWSTR lpCommandLine,
        LPSECURITY_ATTRIBUTES lpProcessAttributes,
        LPSECURITY_ATTRIBUTES lpThreadAttributes, WINBOOL bInheritHandles,
        DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory,
        LPSTARTUPINFOW lpStartupInfo,
        LPPROCESS_INFORMATION lpProcessInformation,
        PHANDLE hRestrictedUserToken);
    CreateProcessInternalW_t pCreateProcessInternalW =
        (CreateProcessInternalW_t)GetProcAddress(kernelModule,
                                                 "CreateProcessInternalW");
    if (!pCreateProcessInternalW) {
        Wh_Log(L"No CreateProcessInternalW");
        return;
    }

    STARTUPINFO si{
        .cb = sizeof(STARTUPINFO),
        .dwFlags = STARTF_FORCEOFFFEEDBACK,
    };
    PROCESS_INFORMATION pi;
    if (!pCreateProcessInternalW(nullptr, currentProcessPath, commandLine,
                                 nullptr, nullptr, FALSE, NORMAL_PRIORITY_CLASS,
                                 nullptr, nullptr, &si, &pi, nullptr)) {
        Wh_Log(L"CreateProcess failed");
        return;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

void Wh_ModSettingsChanged() {
    if (g_isToolModProcessLauncher) {
        return;
    }

    WhTool_ModSettingsChanged();
}

void Wh_ModUninit() {
    if (g_isToolModProcessLauncher) {
        return;
    }

    WhTool_ModUninit();
    ExitProcess(0);
}
