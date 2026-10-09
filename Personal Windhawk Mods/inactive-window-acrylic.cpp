// ==WindhawkMod==
// @id              inactive-window-acrylic
// @name            Inactive Window Acrylic
// @description     Fades background app windows over individual Acrylic underlays
// @version         0.1.1
// @author          bbmaster123 / Sol
// @include         *
// @compilerOptions -ldwmapi -luser32 -lgdi32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
Experimental companion to Drag Fade 0.5.15. The foreground window and its
owned dialogs stay opaque. Other visible, ordinary top-level app windows fade
over an Acrylic helper placed immediately behind each window. The animation
starts when the foreground window changes. Existing layered windows, desktop
and taskbar surfaces, minimized/cloaked windows, and windows that reject the
style change are skipped.

Install as a separate local mod and test without Drag Fade first. Like Drag
Fade, this mod loads into each application and changes only windows belonging
to its own process. Its WinEvent controller uses a message-loop thread; the
target application's normal UI thread still owns its windows. Disabling the
mod restores window opacity and destroys the helpers.

This build needs live testing on your setup. In particular, check
Alt+Tab, owned dialogs, moving/resizing background windows, and disabling
the mod while several windows are faded. It intentionally does not modify
system windows, app windows that are already layered, or other desktops.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- inactiveOpacity: 160
  $name: Background Window Opacity (0-255)
  $description: Opacity of inactive window content; 255 disables the fade.
- transitionMs: 160
  $name: Focus Transition Duration (ms)
  $description: Fade duration when the foreground window changes; 0 is instant.
- acrylicRenderer: legacy
  $name: Acrylic Renderer
  $options:
    - legacy: "Tinted Acrylic"
    - system: "Windows 11 System Acrylic (tinted fallback)"
- tintColorHex: "202020"
  $name: Acrylic Tint Color (HEX)
- tintOpacity: 60
  $name: Acrylic Tint Opacity (0-255)
- minWindowWidth: 120
  $name: Minimum Window Width
- minWindowHeight: 80
  $name: Minimum Window Height
*/
// ==/WindhawkModSettings==

#define NOMINMAX
#include <windows.h>
#include <dwmapi.h>
#include <windhawk_api.h>
#include <algorithm>
#include <atomic>
#include <string>
#include <unordered_map>
#include <vector>
#include <cwchar>

#ifndef DWMWA_SYSTEMBACKDROP_TYPE
#define DWMWA_SYSTEMBACKDROP_TYPE 38
#endif
#ifndef DWMSBT_TRANSIENTWINDOW
#define DWMSBT_TRANSIENTWINDOW 3
#endif
#ifndef DWMWA_CLOAKED
#define DWMWA_CLOAKED 14
#endif
#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
#define DWMWA_WINDOW_CORNER_PREFERENCE 33
#endif
#ifndef DWMWA_BORDER_COLOR
#define DWMWA_BORDER_COLOR 34
#endif

namespace {

struct AccentPolicy {
    int state;
    DWORD flags;
    DWORD gradientColor;
    DWORD animationId;
};
struct CompositionData {
    int attribute;
    PVOID data;
    SIZE_T size;
};
using SetComposition = BOOL(WINAPI*)(HWND, CompositionData*);

struct WindowState {
    HWND helper = nullptr;
    DWORD pid = 0;
    DWORD tid = 0;
    std::wstring className;
    BYTE alpha = 255;
    BYTE from = 255;
    BYTE to = 255;
    ULONGLONG start = 0;
    bool ownsLayeredStyle = false;
};

constexpr UINT kRescanMessage = WM_APP + 37;
constexpr WCHAR kControllerClass[] = L"InactiveAcrylicController-0.1";
constexpr WCHAR kHelperClass[] = L"InactiveAcrylicUnderlay-0.1";
std::unordered_map<HWND, WindowState> g_states;
std::atomic<HWND> g_controller{nullptr};
std::atomic<bool> g_stopping{false};
HANDLE g_thread = nullptr;
DWORD g_threadId = 0;
HINSTANCE g_module = nullptr;
HWINEVENTHOOK g_foregroundHook = nullptr;
SetComposition g_setComposition = nullptr;
BYTE g_inactiveOpacity = 160;
BYTE g_tintOpacity = 60;
BYTE g_tintR = 32, g_tintG = 32, g_tintB = 32;
DWORD g_transitionMs = 160;
int g_minWidth = 120, g_minHeight = 80;
bool g_systemRenderer = false;
HWND g_lastForeground = nullptr;
ULONGLONG g_lastScan = 0;
UINT g_timerPeriod = 0;

void ParseColor(PCWSTR text) {
    if (text && *text == L'#') ++text;
    if (!text || std::wcslen(text) != 6) return;
    unsigned color = 0;
    for (int i = 0; i < 6; ++i) {
        wchar_t ch = text[i];
        int digit = ch >= L'0' && ch <= L'9' ? ch - L'0' :
                    ch >= L'a' && ch <= L'f' ? ch - L'a' + 10 :
                    ch >= L'A' && ch <= L'F' ? ch - L'A' + 10 : -1;
        if (digit < 0) return;
        color = (color << 4) | static_cast<unsigned>(digit);
    }
    g_tintR = (color >> 16) & 255;
    g_tintG = (color >> 8) & 255;
    g_tintB = color & 255;
}

bool SameWindow(HWND hwnd, const WindowState& state) {
    DWORD pid = 0;
    DWORD tid = GetWindowThreadProcessId(hwnd, &pid);
    if (!IsWindow(hwnd) || pid != state.pid || tid != state.tid) return false;
    WCHAR name[128]{};
    return GetClassNameW(hwnd, name, 128) && state.className == name;
}

bool GetVisibleRect(HWND hwnd, RECT* rect) {
    return SUCCEEDED(DwmGetWindowAttribute(hwnd,
               DWMWA_EXTENDED_FRAME_BOUNDS, rect, sizeof(*rect))) ||
           GetWindowRect(hwnd, rect);
}

bool IsEligible(HWND hwnd, HWND foregroundRoot) {
    if (!IsWindowVisible(hwnd) || IsIconic(hwnd) || hwnd == GetShellWindow())
        return false;
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid != GetCurrentProcessId() ||
        GetAncestor(hwnd, GA_ROOT) != hwnd ||
        GetAncestor(hwnd, GA_ROOTOWNER) == foregroundRoot) return false;
    LONG_PTR ex = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    // Existing layered windows are rejected only when first tracked. Our own
    // fade adds WS_EX_LAYERED, so subsequent rescans must still accept them.
    if (ex & (WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE)) return false;
    WCHAR name[128]{};
    if (!GetClassNameW(hwnd, name, 128) ||
        _wcsicmp(name, kControllerClass) == 0 ||
        _wcsicmp(name, kHelperClass) == 0 ||
        _wcsicmp(name, L"Shell_TrayWnd") == 0 ||
        _wcsicmp(name, L"Shell_SecondaryTrayWnd") == 0 ||
        _wcsicmp(name, L"Progman") == 0 ||
        _wcsicmp(name, L"WorkerW") == 0) return false;
    BOOL cloaked = FALSE;
    if (SUCCEEDED(DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED,
                                        &cloaked, sizeof(cloaked))) && cloaked)
        return false;
    RECT rect{};
    return GetWindowRect(hwnd, &rect) &&
           rect.right - rect.left >= g_minWidth &&
           rect.bottom - rect.top >= g_minHeight;
}

bool Restore(HWND hwnd, WindowState& state) {
    bool restored = true;
    if (state.ownsLayeredStyle && SameWindow(hwnd, state)) {
        LONG_PTR ex = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
        if (ex & WS_EX_LAYERED) {
            SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
            SetLastError(ERROR_SUCCESS);
            LONG_PTR prior = SetWindowLongPtrW(hwnd, GWL_EXSTYLE,
                                               ex & ~WS_EX_LAYERED);
            if (prior || GetLastError() == ERROR_SUCCESS) {
                SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                    SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE |
                    SWP_FRAMECHANGED | SWP_ASYNCWINDOWPOS);
            } else {
                restored = false;
            }
        }
    }
    if (state.helper) {
        DestroyWindow(state.helper); // Helpers belong to this controller thread.
        state.helper = nullptr;
    }
    if (restored) state.ownsLayeredStyle = false;
    return restored;
}

LRESULT CALLBACK HelperProc(HWND hwnd, UINT message, WPARAM w, LPARAM l) {
    if (message == WM_NCHITTEST) return HTTRANSPARENT;
    if (message == WM_NCCALCSIZE) return 0;
    if (message == WM_ERASEBKGND) return 1;
    if (message == WM_PAINT) {
        PAINTSTRUCT ps{};
        BeginPaint(hwnd, &ps);
        EndPaint(hwnd, &ps);
        return 0;
    }
    return DefWindowProcW(hwnd, message, w, l);
}

HWND MakeUnderlay(HWND target) {
    RECT rect{};
    if (!GetVisibleRect(target, &rect)) return nullptr;
    HWND helper = CreateWindowExW(WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW |
                                 WS_EX_NOACTIVATE,
        kHelperClass, nullptr, WS_POPUP, rect.left, rect.top,
        rect.right - rect.left, rect.bottom - rect.top,
        nullptr, nullptr, g_module, nullptr);
    if (!helper) return nullptr;
    BOOL disableTransitions = TRUE;
    DwmSetWindowAttribute(helper, 3, &disableTransitions,
                          sizeof(disableTransitions));
    BOOL dark = FALSE;
    if (SUCCEEDED(DwmGetWindowAttribute(target, 20, &dark, sizeof(dark))))
        DwmSetWindowAttribute(helper, 20, &dark, sizeof(dark));
    MARGINS margins{-1, -1, -1, -1};
    DwmExtendFrameIntoClientArea(helper, &margins);
    int corner = 2; // DWMWCP_ROUND
    DwmGetWindowAttribute(target, DWMWA_WINDOW_CORNER_PREFERENCE,
                          &corner, sizeof(corner));
    DwmSetWindowAttribute(helper, DWMWA_WINDOW_CORNER_PREFERENCE,
                          &corner, sizeof(corner));
    COLORREF border = 0xFFFFFFFF; // DWMWA_COLOR_DEFAULT
    DwmGetWindowAttribute(target, DWMWA_BORDER_COLOR,
                          &border, sizeof(border));
    DwmSetWindowAttribute(helper, DWMWA_BORDER_COLOR,
                          &border, sizeof(border));

    bool applied = false;
    if (g_systemRenderer) {
        int backdrop = DWMSBT_TRANSIENTWINDOW;
        applied = SUCCEEDED(DwmSetWindowAttribute(helper,
            DWMWA_SYSTEMBACKDROP_TYPE, &backdrop, sizeof(backdrop)));
    }
    if (!applied && g_setComposition) {
        AccentPolicy policy{4, 2,
            (static_cast<DWORD>(std::max<BYTE>(1, g_tintOpacity)) << 24) |
            (static_cast<DWORD>(g_tintB) << 16) |
            (static_cast<DWORD>(g_tintG) << 8) | g_tintR, 0};
        CompositionData data{19, &policy, sizeof(policy)};
        applied = g_setComposition(helper, &data) != FALSE;
    }
    if (!applied && !g_systemRenderer) {
        int backdrop = DWMSBT_TRANSIENTWINDOW;
        applied = SUCCEEDED(DwmSetWindowAttribute(helper,
            DWMWA_SYSTEMBACKDROP_TYPE, &backdrop, sizeof(backdrop)));
    }
    if (g_setComposition) {
        DWORD noShadow = 0;
        CompositionData shadow{12, &noShadow, sizeof(noShadow)};
        g_setComposition(helper, &shadow);
    }
    if (!applied || !SetWindowPos(helper, target,
            rect.left, rect.top, rect.right - rect.left,
            rect.bottom - rect.top,
            SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOSENDCHANGING |
            SWP_ASYNCWINDOWPOS)) {
        DestroyWindow(helper);
        return nullptr;
    }
    return helper;
}

void BeginTransition(WindowState& state, BYTE to) {
    if (state.to == to) return;
    state.from = state.alpha;
    state.to = to;
    state.start = GetTickCount64();
}

bool StartInactive(HWND hwnd) {
    if (g_inactiveOpacity == 255 ||
        (GetWindowLongPtrW(hwnd, GWL_EXSTYLE) & WS_EX_LAYERED) ||
        GetAncestor(GetForegroundWindow(), GA_ROOTOWNER) ==
            GetAncestor(hwnd, GA_ROOTOWNER)) return false;
    DWORD pid = 0;
    DWORD tid = GetWindowThreadProcessId(hwnd, &pid);
    WCHAR name[128]{};
    if (!tid || !GetClassNameW(hwnd, name, 128)) return false;
    WindowState state{};
    state.pid = pid;
    state.tid = tid;
    state.className = name;
    state.helper = MakeUnderlay(hwnd);
    if (!state.helper) return false;
    LONG_PTR ex = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    if (ex & WS_EX_LAYERED) {
        Restore(hwnd, state);
        return false;
    }
    SetLastError(ERROR_SUCCESS);
    LONG_PTR prior = SetWindowLongPtrW(hwnd, GWL_EXSTYLE, ex | WS_EX_LAYERED);
    if (!prior && GetLastError() != ERROR_SUCCESS) {
        Restore(hwnd, state);
        return false;
    }
    state.ownsLayeredStyle = true;
    if (!SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA)) {
        if (!Restore(hwnd, state))
            g_states.emplace(hwnd, std::move(state)); // Retry style restore.
        return false;
    }
    state.to = g_inactiveOpacity;
    state.start = GetTickCount64();
    g_states.emplace(hwnd, std::move(state));
    return true;
}

void SyncHelper(HWND hwnd, WindowState& state, bool reanchor) {
    if (!state.helper || !SameWindow(hwnd, state)) return;
    RECT target{}, helper{};
    if (!GetVisibleRect(hwnd, &target) ||
        !GetWindowRect(state.helper, &helper)) return;
    if (target.left != helper.left || target.top != helper.top ||
        target.right != helper.right || target.bottom != helper.bottom ||
        (reanchor && GetWindow(hwnd, GW_HWNDNEXT) != state.helper)) {
        SetWindowPos(state.helper, hwnd, target.left, target.top,
                     target.right - target.left, target.bottom - target.top,
                     SWP_NOACTIVATE | SWP_NOSENDCHANGING | SWP_ASYNCWINDOWPOS);
    }
}

BOOL CALLBACK CollectOwnWindows(HWND hwnd, LPARAM value) {
    auto* windows = reinterpret_cast<std::vector<HWND>*>(value);
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid == GetCurrentProcessId() && windows->size() < 256)
        windows->push_back(hwnd);
    return windows->size() < 256;
}

void Tick() {
    const ULONGLONG now = GetTickCount64();
    HWND foreground = GetForegroundWindow();
    HWND root = foreground ? GetAncestor(foreground, GA_ROOTOWNER) : nullptr;
    bool focusChanged = foreground != g_lastForeground;
    bool rescan = focusChanged || now - g_lastScan > 500;
    if (rescan) {
        g_lastForeground = foreground;
        g_lastScan = now;
        std::vector<HWND> candidates;
        EnumWindows(CollectOwnWindows,
                    reinterpret_cast<LPARAM>(&candidates));
        for (HWND hwnd : candidates) {
            bool eligible = root && IsEligible(hwnd, root);
            auto it = g_states.find(hwnd);
            if (it != g_states.end() && !SameWindow(hwnd, it->second)) {
                Restore(hwnd, it->second);
                g_states.erase(it);
                it = g_states.end();
            }
            if (eligible && it == g_states.end()) {
                StartInactive(hwnd);
            } else if (!eligible && it != g_states.end()) {
                BeginTransition(it->second, 255);
            } else if (eligible && it != g_states.end() &&
                       it->second.helper) {
                BeginTransition(it->second, g_inactiveOpacity);
            }
        }
    }
    for (auto it = g_states.begin(); it != g_states.end();) {
        HWND hwnd = it->first;
        WindowState& state = it->second;
        if (!SameWindow(hwnd, state)) {
            Restore(hwnd, state);
            it = g_states.erase(it);
            continue;
        }
        // Check the foreground on every animation tick as well as on rescans.
        // A queued event must never leave the newly active window translucent.
        if (!root || GetAncestor(hwnd, GA_ROOTOWNER) == root ||
            !IsWindowVisible(hwnd) || IsIconic(hwnd) || !state.helper)
            BeginTransition(state, 255);
        if (state.from != state.to) {
            // BeginTransition and StartInactive can set start after Tick's
            // scan timestamp. Subtracting that earlier timestamp from start
            // wraps ULONGLONG and finishes the fade on its very first frame.
            const ULONGLONG animationNow = GetTickCount64();
            const ULONGLONG elapsed = animationNow >= state.start ?
                animationNow - state.start : 0;
            double t = g_transitionMs ?
                std::min(1.0, double(elapsed) / g_transitionMs) : 1.0;
            int alpha = static_cast<int>(state.from +
                (static_cast<int>(state.to) - state.from) * t + 0.5);
            state.alpha = static_cast<BYTE>(std::clamp(alpha, 0, 255));
            if (!SetLayeredWindowAttributes(hwnd, 0, state.alpha, LWA_ALPHA)) {
                if (Restore(hwnd, state)) it = g_states.erase(it);
                else ++it;
                continue;
            }
            if (t >= 1.0) state.from = state.to;
        }
        if (state.to == 255 && state.alpha == 255) {
            if (Restore(hwnd, state)) {
                it = g_states.erase(it);
                continue;
            }
        }
        SyncHelper(hwnd, state, focusChanged);
        ++it;
    }
}

void CALLBACK ForegroundChanged(HWINEVENTHOOK, DWORD, HWND,
                                LONG, LONG, DWORD, DWORD) {
    HWND controller = g_controller.load(std::memory_order_acquire);
    if (controller && !g_stopping.load(std::memory_order_relaxed))
        PostMessageW(controller, kRescanMessage, 0, 0);
}

bool RefreshTimer(HWND controller) {
    UINT period = g_states.empty() ? 250 : 16;
    if (period == g_timerPeriod) return true;
    if (g_timerPeriod) KillTimer(controller, 1);
    g_timerPeriod = SetTimer(controller, 1, period, nullptr) ? period : 0;
    return g_timerPeriod != 0;
}

LRESULT CALLBACK ControllerProc(HWND hwnd, UINT message, WPARAM w, LPARAM l) {
    if (message == kRescanMessage || message == WM_TIMER) {
        if (!g_stopping.load(std::memory_order_relaxed)) {
            Tick();
            if (!RefreshTimer(hwnd))
                Wh_Log(L"Inactive Acrylic: controller timer stopped");
        }
        return 0;
    }
    if (message == WM_CLOSE) {
        g_stopping.store(true, std::memory_order_release);
        if (g_foregroundHook) {
            UnhookWinEvent(g_foregroundHook);
            g_foregroundHook = nullptr;
        }
        KillTimer(hwnd, 1);
        g_timerPeriod = 0;
        for (auto& [target, state] : g_states) {
            if (!Restore(target, state))
                Wh_Log(L"Inactive Acrylic: could not remove layered style from %p",
                       target);
        }
        g_states.clear();
        g_controller.store(nullptr, std::memory_order_release);
        DestroyWindow(hwnd);
        return 0;
    }
    if (message == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, message, w, l);
}

DWORD WINAPI ControllerThread(void*) {
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.hInstance = g_module;
    wc.lpfnWndProc = ControllerProc;
    wc.lpszClassName = kControllerClass;
    if (!RegisterClassExW(&wc)) {
        Wh_Log(L"Inactive Acrylic: controller class registration failed (%lu)",
               GetLastError());
        return 0;
    }
    wc.lpfnWndProc = HelperProc;
    wc.lpszClassName = kHelperClass;
    if (!RegisterClassExW(&wc)) {
        Wh_Log(L"Inactive Acrylic: helper class registration failed (%lu)",
               GetLastError());
        UnregisterClassW(kControllerClass, g_module);
        return 0;
    }
    HWND controller = CreateWindowExW(0, kControllerClass, nullptr, 0,
        0, 0, 0, 0, HWND_MESSAGE, nullptr, g_module, nullptr);
    if (controller) {
        g_controller.store(controller, std::memory_order_release);
        g_foregroundHook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND,
            EVENT_SYSTEM_FOREGROUND, nullptr, ForegroundChanged,
            0, 0, WINEVENT_OUTOFCONTEXT);
        if (g_foregroundHook && !g_stopping.load(std::memory_order_acquire) &&
            RefreshTimer(controller)) {
            Wh_Log(L"Inactive Acrylic: controller ready in process %lu",
                   GetCurrentProcessId());
            Tick();
            RefreshTimer(controller);
            MSG msg{};
            while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
        }
        else Wh_Log(L"Inactive Acrylic: controller hook or timer unavailable");
        if (IsWindow(controller)) ControllerProc(controller, WM_CLOSE, 0, 0);
    }
    UnregisterClassW(kHelperClass, g_module);
    UnregisterClassW(kControllerClass, g_module);
    return 0;
}

} // namespace

BOOL Wh_ModInit() {
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCWSTR>(&ControllerProc), &g_module)) return FALSE;
    g_inactiveOpacity = static_cast<BYTE>(
        std::clamp(Wh_GetIntSetting(L"inactiveOpacity"), 0, 255));
    g_tintOpacity = static_cast<BYTE>(
        std::clamp(Wh_GetIntSetting(L"tintOpacity"), 0, 255));
    g_transitionMs = std::clamp(Wh_GetIntSetting(L"transitionMs"), 0, 1000);
    g_minWidth = std::max(20, Wh_GetIntSetting(L"minWindowWidth"));
    g_minHeight = std::max(20, Wh_GetIntSetting(L"minWindowHeight"));
    PCWSTR renderer = Wh_GetStringSetting(L"acrylicRenderer");
    g_systemRenderer = renderer && std::wcscmp(renderer, L"system") == 0;
    if (renderer) Wh_FreeStringSetting(renderer);
    PCWSTR color = Wh_GetStringSetting(L"tintColorHex");
    ParseColor(color);
    if (color) Wh_FreeStringSetting(color);
    HMODULE user32 = GetModuleHandleW(L"user32.dll");
    g_setComposition = user32 ? reinterpret_cast<SetComposition>(
        GetProcAddress(user32, "SetWindowCompositionAttribute")) : nullptr;
    g_stopping.store(false, std::memory_order_release);
    g_thread = CreateThread(nullptr, 0, ControllerThread, nullptr, 0,
                            &g_threadId);
    if (!g_thread) return FALSE;
    return TRUE;
}

void Wh_ModBeforeUninit() {
    g_stopping.store(true, std::memory_order_release);
    HWND controller = g_controller.load(std::memory_order_acquire);
    if (controller) PostMessageW(controller, WM_CLOSE, 0, 0);
    if (g_thread && g_threadId != GetCurrentThreadId()) {
        if (WaitForSingleObject(g_thread, 5000) != WAIT_OBJECT_0) {
            Wh_Log(L"Inactive Acrylic: controller cleanup still running");
        }
    }
}

void Wh_ModUninit() {
    if (g_thread) {
        if (WaitForSingleObject(g_thread, 0) != WAIT_OBJECT_0) {
            // Keep callback code mapped if a foreign window stalled cleanup.
            HMODULE pinned = nullptr;
            BOOL pinnedOk = GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                GET_MODULE_HANDLE_EX_FLAG_PIN,
                reinterpret_cast<LPCWSTR>(&ControllerProc), &pinned);
            Wh_Log(L"Inactive Acrylic: controller still running; module pin=%u",
                   pinnedOk);
        }
        CloseHandle(g_thread);
        g_thread = nullptr;
    }
}

BOOL Wh_ModSettingsChanged(BOOL* reload) {
    *reload = TRUE;
    return TRUE;
}
