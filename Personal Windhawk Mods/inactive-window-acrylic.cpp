// ==WindhawkMod==
// @id              inactive-window-acrylic
// @name            Inactive Window Acrylic
// @description     Acrylic on inactive apps or a full-screen dim, with separate taskbar controls
// @version         0.2.8
// @author          bbmaster123 / Sol
// @include         explorer.exe
// @compilerOptions -ldwmapi -luser32 -lgdi32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
Experimental companion to Drag Fade 0.5.15. Per-window mode keeps the
foreground window and its owned dialogs opaque, fading other ordinary app
windows over rounded Acrylic helpers. Full-screen mode instead draws a
click-through dim layer behind the active window. A small dim layer fades the
previous window on direct focus changes. The taskbar has a separate dim toggle,
opacity, and optional hover fade in either mode.

Install as a separate local mod and test without Drag Fade first. Like Drag
Fade, this mod loads into each application and changes only windows belonging
to its own process. Its WinEvent controller uses a message-loop thread; the
target application's normal UI thread still owns its windows. Disabling the
mod restores window opacity and destroys the helpers.

The full-screen and taskbar layers are managed by one Explorer controller per
session. They never modify system window styles and do not run on the UAC
secure desktop. Existing layered apps, minimized/cloaked windows, and windows
that reject the style change are skipped in per-window mode. Check Alt+Tab,
owned dialogs, moving/resizing, multiple monitors, and disabling the mod.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- dimMode: windows
  $name: Dimming Mode
  $options:
    - windows: "Inactive app windows (Acrylic)"
    - fullscreen: "Full screen behind active window"
- inactiveOpacity: 160
  $name: Background Window Opacity (0-255)
  $description: Per-window mode only. Opacity of inactive window content; 255 disables the fade.
- fullscreenDimOpacity: 100
  $name: Full-Screen Dim Opacity (0-255)
  $description: Full-screen mode only. Opacity of the black screen layer; 0 disables it.
- dimTaskbar: false
  $name: Dim Taskbar Separately
  $description: Draw a separate click-through dim layer over visible taskbars.
- undimTaskbarOnHover: false
  $name: Undim Taskbar on Hover
  $description: Fade the separate taskbar dim away while the pointer is over a taskbar.
- taskbarDimOpacity: 100
  $name: Taskbar Dim Opacity (0-255)
  $description: Used when Dim Taskbar Separately is enabled, in either mode.
- taskbarDimColorHex: "000000"
  $name: Taskbar Dim Color (HEX)
- taskbarTransitionMs: 160
  $name: Taskbar Transition Duration (ms)
- cornerRadius: 8
  $name: Acrylic Corner Radius (DIP)
  $description: Per-window Acrylic helper radius; set to 0 for square corners.
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
#include <cstdlib>
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
    int originalCornerPreference = -1;
    bool changedCornerPreference = false;
};

struct DimLayer {
    HWND hwnd = nullptr;
    BYTE alpha = 0;
    BYTE from = 0;
    BYTE to = 0;
    ULONGLONG start = 0;
};

constexpr UINT kRescanMessage = WM_APP + 37;
constexpr WCHAR kControllerClass[] = L"InactiveAcrylicController-0.1";
constexpr WCHAR kHelperClass[] = L"InactiveAcrylicUnderlay-0.1";
constexpr WCHAR kDimClass[] = L"InactiveAcrylicScreenDim-0.2";
constexpr WCHAR kOverlayMutex[] = L"Local\\InactiveWindowAcrylicScreenOwner-0.2";
std::unordered_map<HWND, WindowState> g_states;
DimLayer g_screenDim, g_outgoingDim, g_taskbarDim;
HWND g_screenRoot = nullptr;
HWND g_pendingTaskbarRoot = nullptr;
ULONGLONG g_pendingTaskbarSince = 0;
ULONGLONG g_lastTaskbarFocus = 0;
HWND g_focusOldRoot = nullptr, g_focusNewRoot = nullptr;
RECT g_focusOldRect{};
bool g_focusFading = false;
HANDLE g_overlayMutex = nullptr;
bool g_overlayOwner = false;
bool g_isExplorer = false;
bool g_fullscreenMode = false;
bool g_dimTaskbar = false;
bool g_undimTaskbarOnHover = false;
BYTE g_fullscreenOpacity = 100, g_taskbarOpacity = 100;
COLORREF g_taskbarColor = RGB(0, 0, 0);
DWORD g_taskbarTransitionMs = 160;
int g_cornerRadius = 8;
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
ULONGLONG g_lastRegionUpdate = 0;
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

COLORREF ParseTaskbarColor(PCWSTR text) {
    if (text && *text == L'#') ++text;
    if (!text || std::wcslen(text) != 6) return RGB(0, 0, 0);
    unsigned value = 0;
    for (int i = 0; i < 6; ++i) {
        wchar_t ch = text[i];
        int digit = ch >= L'0' && ch <= L'9' ? ch - L'0' :
                    ch >= L'a' && ch <= L'f' ? ch - L'a' + 10 :
                    ch >= L'A' && ch <= L'F' ? ch - L'A' + 10 : -1;
        if (digit < 0) return RGB(0, 0, 0);
        value = (value << 4) | static_cast<unsigned>(digit);
    }
    return RGB((value >> 16) & 255, (value >> 8) & 255, value & 255);
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

int CornerPixels(HWND target) {
    if (IsZoomed(target)) return 0;
    int preference = 0;
    if (FAILED(DwmGetWindowAttribute(target, DWMWA_WINDOW_CORNER_PREFERENCE,
                                     &preference, sizeof(preference))) ||
        preference == 1) return 0; // Windows 10, or DWMWCP_DONOTROUND.
    int dip = preference == 3 ? std::min(g_cornerRadius, 4) : g_cornerRadius;
    UINT dpi = GetDpiForWindow(target);
    return MulDiv(dip, dpi ? dpi : 96, 96);
}

void RoundHelper(HWND helper, HWND target, int width, int height) {
    int radius = CornerPixels(target);
    HRGN region = radius > 0 ?
        CreateRoundRectRgn(0, 0, width + 1, height + 1,
                           radius * 2, radius * 2) :
        CreateRectRgn(0, 0, width, height);
    if (region && !SetWindowRgn(helper, region, TRUE)) DeleteObject(region);
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
    if (state.changedCornerPreference && SameWindow(hwnd, state)) {
        DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE,
            &state.originalCornerPreference,
            sizeof(state.originalCornerPreference));
        state.changedCornerPreference = false;
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

LRESULT CALLBACK DimProc(HWND hwnd, UINT message, WPARAM w, LPARAM l) {
    if (message == WM_NCHITTEST) return HTTRANSPARENT;
    if (message == WM_ERASEBKGND) return 1;
    if (message == WM_PAINT) {
        PAINTSTRUCT ps{};
        HDC dc = BeginPaint(hwnd, &ps);
        HBRUSH brush = hwnd == g_taskbarDim.hwnd ?
            CreateSolidBrush(g_taskbarColor) : nullptr;
        FillRect(dc, &ps.rcPaint, brush ? brush :
                 static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
        if (brush) DeleteObject(brush);
        EndPaint(hwnd, &ps);
        return 0;
    }
    return DefWindowProcW(hwnd, message, w, l);
}

bool EnsureOverlayOwner() {
    if (!g_isExplorer) return false;
    if (!g_overlayMutex) g_overlayMutex = CreateMutexW(nullptr, FALSE, kOverlayMutex);
    if (!g_overlayMutex) return false;
    if (!g_overlayOwner) {
        DWORD result = WaitForSingleObject(g_overlayMutex, 0);
        g_overlayOwner = result == WAIT_OBJECT_0 || result == WAIT_ABANDONED;
    }
    return g_overlayOwner;
}

RECT VirtualScreen() {
    int x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    return {x, y, x + GetSystemMetrics(SM_CXVIRTUALSCREEN),
                  y + GetSystemMetrics(SM_CYVIRTUALSCREEN)};
}

bool EnsureDimLayer(DimLayer& layer, const RECT& screen,
                    HWND owner = nullptr) {
    if (layer.hwnd && IsWindow(layer.hwnd)) return true;
    layer = {};
    layer.hwnd = CreateWindowExW(WS_EX_LAYERED | WS_EX_TRANSPARENT |
            WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
        kDimClass, nullptr, WS_POPUP, screen.left, screen.top,
        screen.right - screen.left, screen.bottom - screen.top,
        owner, nullptr, g_module, nullptr);
    if (!layer.hwnd) return false;
    SetLayeredWindowAttributes(layer.hwnd, 0, 0, LWA_ALPHA);
    return true;
}

void TransitionDim(DimLayer& layer, BYTE target) {
    if (layer.to == target) return;
    layer.from = layer.alpha;
    layer.to = target;
    layer.start = GetTickCount64();
}

void AnimateDim(DimLayer& layer, DWORD duration) {
    if (!layer.hwnd || layer.from == layer.to) return;
    const ULONGLONG now = GetTickCount64();
    const ULONGLONG elapsed = now >= layer.start ? now - layer.start : 0;
    double t = duration ?
        std::min(1.0, double(elapsed) / duration) : 1.0;
    int alpha = static_cast<int>(layer.from +
        (static_cast<int>(layer.to) - layer.from) * t + 0.5);
    layer.alpha = static_cast<BYTE>(std::clamp(alpha, 0, 255));
    SetLayeredWindowAttributes(layer.hwnd, 0, layer.alpha, LWA_ALPHA);
    if (t >= 1.0) layer.from = layer.to;
}

void FinishFocusFade() {
    if (!g_focusFading) return;
    HWND after = IsWindow(g_focusNewRoot) &&
        IsWindowVisible(g_focusNewRoot) && !IsIconic(g_focusNewRoot) ?
        g_focusNewRoot : HWND_TOPMOST;
    constexpr UINT moveFlags = SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE;
    HDWP batch = BeginDeferWindowPos(2);
    if (batch) batch = DeferWindowPos(batch, g_screenDim.hwnd, after,
                                      0, 0, 0, 0, moveFlags);
    if (batch) batch = DeferWindowPos(batch, g_outgoingDim.hwnd, nullptr,
                                      0, 0, 0, 0,
                                      moveFlags | SWP_NOZORDER |
                                      SWP_HIDEWINDOW);
    if (!batch || !EndDeferWindowPos(batch)) {
        SetWindowPos(g_screenDim.hwnd, after, 0, 0, 0, 0, moveFlags);
        ShowWindow(g_outgoingDim.hwnd, SW_HIDE);
    }
    g_outgoingDim.alpha = g_outgoingDim.from = g_outgoingDim.to = 0;
    g_focusOldRoot = g_focusNewRoot = nullptr;
    g_focusFading = false;
}

void AnimateScreenDim() {
    AnimateDim(g_screenDim, g_transitionMs);
    if (!g_focusFading || !g_outgoingDim.hwnd) return;
    RECT current{};
    if (!IsWindowVisible(g_focusOldRoot) || IsIconic(g_focusOldRoot) ||
        !GetVisibleRect(g_focusOldRoot, &current) ||
        !EqualRect(&current, &g_focusOldRect)) {
        FinishFocusFade();
        return;
    }
    AnimateDim(g_outgoingDim, g_transitionMs);
    if (g_outgoingDim.alpha == g_outgoingDim.to) FinishFocusFade();
}

struct FocusRegion {
    HWND oldRoot;
    RECT screen;
    HRGN region;
};

BOOL CALLBACK AddPreviousWindowRegion(HWND hwnd, LPARAM value) {
    auto& data = *reinterpret_cast<FocusRegion*>(value);
    if (GetAncestor(hwnd, GA_ROOTOWNER) != data.oldRoot ||
        !IsWindowVisible(hwnd) || IsIconic(hwnd)) return TRUE;
    BOOL cloaked = FALSE;
    if (SUCCEEDED(DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED,
                                        &cloaked, sizeof(cloaked))) && cloaked)
        return TRUE;
    RECT rect{};
    if (!GetVisibleRect(hwnd, &rect) || rect.right <= rect.left ||
        rect.bottom <= rect.top) return TRUE;
    OffsetRect(&rect, -data.screen.left, -data.screen.top);
    int radius = CornerPixels(hwnd);
    HRGN part = radius ? CreateRoundRectRgn(rect.left, rect.top,
        rect.right + 1, rect.bottom + 1, radius * 2, radius * 2) :
        CreateRectRgnIndirect(&rect);
    if (part) {
        CombineRgn(data.region, data.region, part, RGN_OR);
        DeleteObject(part);
    }
    return TRUE;
}

bool StartFocusFade(const RECT& screen, HRGN background,
                    HWND oldRoot, HWND newRoot) {
    if (!g_screenDim.hwnd || g_screenDim.alpha != g_fullscreenOpacity ||
        !g_transitionMs || !IsWindow(oldRoot) ||
        !IsWindowVisible(oldRoot) || IsIconic(oldRoot) ||
        !IsWindow(newRoot) || !IsWindowVisible(newRoot) ||
        !EnsureDimLayer(g_outgoingDim, screen)) return false;
    // The temporary layer must fit between the two apps in the z-order.
    // Foreground events can arrive before the new app has actually risen.
    bool oldBelowNew = false;
    for (HWND below = GetWindow(newRoot, GW_HWNDNEXT); below;
         below = GetWindow(below, GW_HWNDNEXT)) {
        if (GetAncestor(below, GA_ROOTOWNER) == oldRoot) {
            oldBelowNew = true;
            break;
        }
    }
    if (!oldBelowNew) return false;
    RECT oldRect{};
    if (!GetVisibleRect(oldRoot, &oldRect)) return false;
    HRGN region = CreateRectRgn(0, 0, 0, 0);
    if (!region) return false;
    FocusRegion data{oldRoot, screen, region};
    EnumWindows(AddPreviousWindowRegion, reinterpret_cast<LPARAM>(&data));
    int combined = CombineRgn(region, region, background, RGN_AND);
    if (combined == NULLREGION || combined == ERROR ||
        !SetWindowRgn(g_outgoingDim.hwnd, region, TRUE)) {
        DeleteObject(region);
        return false;
    }
    // The region now belongs to the window. It covers only the previous app.
    g_outgoingDim.alpha = g_outgoingDim.from = 0;
    g_outgoingDim.to = g_fullscreenOpacity;
    g_outgoingDim.start = GetTickCount64();
    SetLayeredWindowAttributes(g_outgoingDim.hwnd, 0, 0, LWA_ALPHA);
    if (!SetWindowPos(g_outgoingDim.hwnd, newRoot,
        screen.left, screen.top, screen.right - screen.left,
        screen.bottom - screen.top, SWP_NOACTIVATE | SWP_SHOWWINDOW |
        SWP_NOSENDCHANGING)) {
        ShowWindow(g_outgoingDim.hwnd, SW_HIDE);
        return false;
    }
    g_focusOldRoot = oldRoot;
    g_focusNewRoot = newRoot;
    g_focusOldRect = oldRect;
    g_focusFading = true;
    return true;
}

void ApplyDimRegion(DimLayer& layer, HRGN region, const RECT& screen) {
    if (!layer.hwnd) { DeleteObject(region); return; }
    RECT current{};
    bool sameBounds = GetWindowRect(layer.hwnd, &current) &&
                      EqualRect(&current, &screen);
    if (!sameBounds) {
        SetWindowPos(layer.hwnd, nullptr, screen.left, screen.top,
            screen.right - screen.left, screen.bottom - screen.top,
            SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
    }
    HRGN previous = CreateRectRgn(0, 0, 0, 0);
    bool sameRegion = previous && sameBounds &&
        GetWindowRgn(layer.hwnd, previous) != ERROR &&
        EqualRgn(previous, region) == TRUE;
    if (previous) DeleteObject(previous);
    if (sameRegion) DeleteObject(region);
    else if (!SetWindowRgn(layer.hwnd, region, TRUE)) DeleteObject(region);
    // Region changes on a layered window trigger a compositor repaint.
    // A stationary desktop needs neither that repaint nor a z-order update.
}

struct RegionWindows {
    RECT screen;
    HRGN screenRegion;
    HRGN taskbarRegion;
    bool taskbarHovered = false;
    POINT cursor{};
};

BOOL CALLBACK AddDimRegions(HWND hwnd, LPARAM arg) {
    auto& data = *reinterpret_cast<RegionWindows*>(arg);
    if (!IsWindowVisible(hwnd) || IsIconic(hwnd)) return TRUE;
    WCHAR name[128]{};
    if (!GetClassNameW(hwnd, name, 128) ||
        (_wcsicmp(name, L"Shell_TrayWnd") != 0 &&
         _wcsicmp(name, L"Shell_SecondaryTrayWnd") != 0)) return TRUE;
    BOOL cloaked = FALSE;
    if (SUCCEEDED(DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED,
                                        &cloaked, sizeof(cloaked))) && cloaked)
        return TRUE;
    RECT rect{};
    if (!GetWindowRect(hwnd, &rect) || rect.right <= rect.left ||
        rect.bottom <= rect.top) return TRUE;
    if (g_undimTaskbarOnHover &&
        PtInRect(&rect, data.cursor)) data.taskbarHovered = true;
    OffsetRect(&rect, -data.screen.left, -data.screen.top);
    HRGN region = CreateRectRgnIndirect(&rect);
    if (!region) return TRUE;
    CombineRgn(data.screenRegion, data.screenRegion, region, RGN_DIFF);
    CombineRgn(data.taskbarRegion, data.taskbarRegion, region, RGN_OR);
    DeleteObject(region);
    return TRUE;
}

bool IsDesktopRoot(HWND hwnd) {
    if (!hwnd || hwnd == GetShellWindow()) return true;
    WCHAR name[128]{};
    return GetClassNameW(hwnd, name, 128) &&
        (_wcsicmp(name, L"Progman") == 0 ||
         _wcsicmp(name, L"WorkerW") == 0);
}

struct FrontApp {
    HWND root = nullptr;
};

BOOL CALLBACK FindFrontApp(HWND hwnd, LPARAM value) {
    auto& app = *reinterpret_cast<FrontApp*>(value);
    if (!IsWindowVisible(hwnd) || IsIconic(hwnd) ||
        GetAncestor(hwnd, GA_ROOT) != hwnd ||
        GetWindowLongPtrW(hwnd, GWL_EXSTYLE) &
            (WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE)) return TRUE;
    HWND root = GetAncestor(hwnd, GA_ROOTOWNER);
    if (IsDesktopRoot(root)) return TRUE;
    WCHAR name[128]{};
    if (!GetClassNameW(hwnd, name, 128) ||
        _wcsicmp(name, L"Shell_TrayWnd") == 0 ||
        _wcsicmp(name, L"Shell_SecondaryTrayWnd") == 0 ||
        _wcsicmp(name, kDimClass) == 0 ||
        _wcsicmp(name, kHelperClass) == 0 ||
        _wcsicmp(name, kControllerClass) == 0) return TRUE;
    BOOL cloaked = FALSE;
    if (SUCCEEDED(DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED,
                                        &cloaked, sizeof(cloaked))) && cloaked)
        return TRUE;
    RECT rect{};
    if (!GetVisibleRect(hwnd, &rect) ||
        rect.right - rect.left < g_minWidth ||
        rect.bottom - rect.top < g_minHeight) return TRUE;
    app.root = root;
    return FALSE;
}

void UpdateDimLayers(HWND foreground, HWND root, bool focusChanged) {
    if ((!g_fullscreenMode && !g_dimTaskbar) || !EnsureOverlayOwner()) return;
    RECT screen = VirtualScreen();
    if (screen.right <= screen.left || screen.bottom <= screen.top) return;
    WCHAR rootClass[128]{};
    if (root) GetClassNameW(root, rootClass, 128);
    bool taskbarForeground = _wcsicmp(rootClass, L"Shell_TrayWnd") == 0 ||
                             _wcsicmp(rootClass, L"Shell_SecondaryTrayWnd") == 0;
    const ULONGLONG now = GetTickCount64();
    if (taskbarForeground) g_lastTaskbarFocus = now;
    // Explorer can report taskbar focus while it is still raising the chosen
    // app. Accept only a stable front window for this intermediate state.
    FrontApp front{};
    if (taskbarForeground) EnumWindows(FindFrontApp,
                                      reinterpret_cast<LPARAM>(&front));
    HWND screenRoot = root;
    if (taskbarForeground) {
        HWND candidate = front.root ? front.root : GetShellWindow();
        if (candidate != g_pendingTaskbarRoot) {
            g_pendingTaskbarRoot = candidate;
            g_pendingTaskbarSince = now;
        }
        const DWORD settleMs = IsDesktopRoot(candidate) ? 250 : 60;
        screenRoot = candidate == g_screenRoot ||
            now - g_pendingTaskbarSince >= settleMs ? candidate : g_screenRoot;
    } else {
        g_pendingTaskbarRoot = nullptr;
    }
    bool screenChanged = screenRoot != g_screenRoot;
    if (g_fullscreenMode && g_fullscreenOpacity &&
        !EnsureDimLayer(g_screenDim, screen)) return;
    if (g_dimTaskbar && g_taskbarOpacity &&
        !EnsureDimLayer(g_taskbarDim, screen,
                        FindWindowW(L"Shell_TrayWnd", nullptr))) return;
    if (!focusChanged && !screenChanged && now - g_lastRegionUpdate < 50) {
        AnimateScreenDim();
        AnimateDim(g_taskbarDim, g_taskbarTransitionMs);
        return;
    }
    g_lastRegionUpdate = now;
    int width = screen.right - screen.left;
    int height = screen.bottom - screen.top;
    HRGN background = CreateRectRgn(0, 0, width, height);
    HRGN taskbar = CreateRectRgn(0, 0, 0, 0);
    if (!background || !taskbar) {
        if (background) DeleteObject(background);
        if (taskbar) DeleteObject(taskbar);
        return;
    }
    RegionWindows windows{screen, background, taskbar};
    if (g_undimTaskbarOnHover) GetCursorPos(&windows.cursor);
    EnumWindows(AddDimRegions, reinterpret_cast<LPARAM>(&windows));
    if (screenChanged) {
        if (g_focusFading) FinishFocusFade();
        if (g_fullscreenMode && foreground && screenRoot && g_screenRoot &&
            !taskbarForeground &&
            (!g_lastTaskbarFocus || now - g_lastTaskbarFocus >= 250) &&
            !IsDesktopRoot(screenRoot) && !IsDesktopRoot(g_screenRoot) &&
            g_fullscreenOpacity &&
            g_screenDim.alpha == g_fullscreenOpacity) {
            StartFocusFade(screen, background, g_screenRoot, screenRoot);
        }
    }
    if ((!foreground || !root) && g_focusFading) FinishFocusFade();
    g_screenRoot = screenRoot;
    auto apply = [&](DimLayer& layer, HRGN region, BYTE opacity,
                     bool isTaskbar) {
        if (!layer.hwnd) { DeleteObject(region); return; }
        ApplyDimRegion(layer, region, screen);
        BYTE target = isTaskbar ?
            (g_undimTaskbarOnHover && windows.taskbarHovered ? 0 : opacity) :
            (foreground && !IsDesktopRoot(screenRoot) ? opacity : 0);
        TransitionDim(layer, target);
        bool reanchor = !isTaskbar && screenChanged && !g_focusFading;
        if ((target || layer.alpha) &&
            (reanchor || !IsWindowVisible(layer.hwnd))) {
            HWND after = HWND_TOPMOST;
            if (!isTaskbar && !IsDesktopRoot(screenRoot) &&
                IsWindowVisible(screenRoot) && !IsIconic(screenRoot))
                after = screenRoot;
            SetWindowPos(layer.hwnd, after, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW |
                (isTaskbar ? SWP_NOOWNERZORDER : 0));
        }
        if (isTaskbar) AnimateDim(layer, g_taskbarTransitionMs);
        else AnimateScreenDim();
        if (!target && !layer.alpha)
            ShowWindow(layer.hwnd, SW_HIDE);
    };
    if (g_fullscreenMode) {
        apply(g_screenDim, background, g_fullscreenOpacity, false);
    } else DeleteObject(background);
    if (g_dimTaskbar)
        apply(g_taskbarDim, taskbar, g_taskbarOpacity, true);
    else DeleteObject(taskbar);
}

void ReleaseDimLayers() {
    if (g_screenDim.hwnd) DestroyWindow(g_screenDim.hwnd);
    if (g_outgoingDim.hwnd) DestroyWindow(g_outgoingDim.hwnd);
    if (g_taskbarDim.hwnd) DestroyWindow(g_taskbarDim.hwnd);
    g_screenDim = {};
    g_outgoingDim = {};
    g_taskbarDim = {};
    g_screenRoot = nullptr;
    g_pendingTaskbarRoot = nullptr;
    g_pendingTaskbarSince = 0;
    g_lastTaskbarFocus = 0;
    g_focusOldRoot = g_focusNewRoot = nullptr;
    g_focusOldRect = {};
    g_focusFading = false;
    if (g_overlayOwner) ReleaseMutex(g_overlayMutex);
    g_overlayOwner = false;
    if (g_overlayMutex) CloseHandle(g_overlayMutex);
    g_overlayMutex = nullptr;
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
    int corner = 2; // A popup doesn't receive automatic rounding.
    if (CornerPixels(target) == 0) corner = 1;
    else {
        int targetPreference = 0;
        if (SUCCEEDED(DwmGetWindowAttribute(target,
                DWMWA_WINDOW_CORNER_PREFERENCE, &targetPreference,
                sizeof(targetPreference))) && targetPreference == 3)
            corner = 3;
    }
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
    if (!applied) {
        DestroyWindow(helper);
        return nullptr;
    }
    RoundHelper(helper, target, rect.right - rect.left,
                rect.bottom - rect.top);
    if (!SetWindowPos(helper, target,
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
    int cornerPreference = 0;
    if (CornerPixels(hwnd) > 0 &&
        SUCCEEDED(DwmGetWindowAttribute(hwnd,
            DWMWA_WINDOW_CORNER_PREFERENCE,
            &cornerPreference, sizeof(cornerPreference))) &&
        cornerPreference == 0) {
        int rounded = 2;
        if (SUCCEEDED(DwmSetWindowAttribute(hwnd,
                DWMWA_WINDOW_CORNER_PREFERENCE,
                &rounded, sizeof(rounded)))) {
            state.originalCornerPreference = cornerPreference;
            state.changedCornerPreference = true;
        }
    }
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
        if (target.right - target.left != helper.right - helper.left ||
            target.bottom - target.top != helper.bottom - helper.top)
            RoundHelper(state.helper, hwnd, target.right - target.left,
                        target.bottom - target.top);
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
    g_lastForeground = foreground;
    if (rescan && !g_fullscreenMode) {
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
    UpdateDimLayers(foreground, root, focusChanged);
}

void CALLBACK ForegroundChanged(HWINEVENTHOOK, DWORD, HWND,
                                LONG, LONG, DWORD, DWORD) {
    HWND controller = g_controller.load(std::memory_order_acquire);
    if (controller && !g_stopping.load(std::memory_order_relaxed))
        PostMessageW(controller, kRescanMessage, 0, 0);
}

bool RefreshTimer(HWND controller) {
    UINT period = g_states.empty() && !g_screenDim.hwnd &&
        !g_outgoingDim.hwnd && !g_taskbarDim.hwnd ? 250 : 16;
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
        ReleaseDimLayers();
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
    wc.lpfnWndProc = DimProc;
    wc.lpszClassName = kDimClass;
    if (!RegisterClassExW(&wc)) {
        Wh_Log(L"Inactive Acrylic: dim class registration failed (%lu)",
               GetLastError());
        UnregisterClassW(kControllerClass, g_module);
        return 0;
    }
    wc.lpfnWndProc = HelperProc;
    wc.lpszClassName = kHelperClass;
    if (!RegisterClassExW(&wc)) {
        Wh_Log(L"Inactive Acrylic: helper class registration failed (%lu)",
               GetLastError());
        UnregisterClassW(kDimClass, g_module);
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
    UnregisterClassW(kDimClass, g_module);
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
    PCWSTR mode = Wh_GetStringSetting(L"dimMode");
    g_fullscreenMode = mode && std::wcscmp(mode, L"fullscreen") == 0;
    if (mode) Wh_FreeStringSetting(mode);
    g_fullscreenOpacity = static_cast<BYTE>(
        std::clamp(Wh_GetIntSetting(L"fullscreenDimOpacity"), 0, 255));
    g_dimTaskbar = Wh_GetIntSetting(L"dimTaskbar") != 0;
    g_undimTaskbarOnHover = Wh_GetIntSetting(L"undimTaskbarOnHover") != 0;
    g_taskbarOpacity = static_cast<BYTE>(
        std::clamp(Wh_GetIntSetting(L"taskbarDimOpacity"), 0, 255));
    g_taskbarTransitionMs =
        std::clamp(Wh_GetIntSetting(L"taskbarTransitionMs"), 0, 1000);
    PCWSTR taskbarColor = Wh_GetStringSetting(L"taskbarDimColorHex");
    g_taskbarColor = ParseTaskbarColor(taskbarColor);
    if (taskbarColor) Wh_FreeStringSetting(taskbarColor);
    g_cornerRadius = std::clamp(Wh_GetIntSetting(L"cornerRadius"), 0, 32);
    WCHAR path[MAX_PATH]{};
    if (GetModuleFileNameW(nullptr, path, MAX_PATH)) {
        PCWSTR basename = std::wcsrchr(path, L'\\');
        g_isExplorer = _wcsicmp(basename ? basename + 1 : path,
                               L"explorer.exe") == 0;
    }
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
