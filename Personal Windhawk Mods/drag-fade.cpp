// ==WindhawkMod==
// @id              drag-fade
// @name            Drag Fade
// @description     Fades windows during drag and resize with Acrylic or simple transparency
// @version         0.5.15
// @author          bbmaster123
// @include         *
// @compilerOptions -ldwmapi -lgdi32 -luser32 -lwinmm
// ==/WindhawkMod==

// ==WindhawkModSettings==
/*
- fadeEffect: underlay
  $name: Fade Effect Style
  $description: "Visual effect applied when dragging or resizing windows."
  $options:
    - underlay: "Acrylic backdrop underlay behind the window"
    - simple: "Simple Transparent Fade (Classic alpha transition)"
- targetOpacity: 160
  $name: Drag Opacity (0-255)
  $description: "Opacity of the window content while dragging (0 = transparent, 255 = opaque)."
- acrylicRenderer: legacy
  $name: Acrylic Renderer
  $description: "System Acrylic needs Windows 11 build 22621 or later. On Windows 10 it falls back to Tinted Acrylic, so both choices look the same. If either renderer is unavailable, the other is tried."
  $options:
    - legacy: "Tinted Acrylic (default)"
    - system: "Windows 11 System Acrylic (tinted fallback on Windows 10)"
- tintColorHex: "202020"
  $name: "Tint Color (HEX)"
  $description: "Hex color for tinted Acrylic and the optional Simple Fade tint (e.g. 202020)."
- tintOpacity: 60
  $name: "Tint Opacity (0-255)"
  $description: "Approximate tint strength for tinted Acrylic and, if enabled, Simple Fade tint (0 = no Simple Fade tint, 255 = strong tint)."
- simpleTintEnabled: false
  $name: "Tint Simple Transparent Fade"
  $description: "Show a translucent sheet in the selected tint color behind the faded window. Off keeps the original plain transparent effect."
- cornerStyle: round
  $name: Window Corner Style
  $description: "Corner radius style for the underlay backdrop."
  $options:
    - round: "Rounded Corners (Default Windows 11)"
    - round_small: "Small Rounded Corners"
    - square: "Square / Rectangular Corners"
- fadeSpeed: 20
  $name: Fade Speed (Higher = Faster)
  $description: "Speed of transition. Values 255 or higher are instant."
- holdDelay: 250
  $name: Hold-to-Fade Delay (ms)
  $description: "Delay while holding without dragging. An actual move or resize starts fading on the next timer tick (0 = fade immediately)."
- enableOnMove: true
  $name: Enable on Window Move
  $description: "Fade window when dragging or moving by the titlebar."
- underlayBehaviorOnMove: follow
  $name: Underlay Behavior on Move
  $description: "Controls whether the backdrop underlay follows the window or remains at the starting position while dragging."
  $options:
    - follow: "Follow Window (Moves with the window)"
    - stay: "Leave in Place (Stays at the starting position)"
- enableOnResize: true
  $name: Enable on Window Resize
  $description: "Fade window when resizing window borders or corners."
- underlayBehaviorOnResize: follow
  $name: Underlay Behavior on Resize
  $description: "Controls whether the backdrop underlay resizes with the window or remains at the starting size while resizing."
  $options:
    - follow: "Follow Window (Resizes with the window)"
    - stay: "Leave in Place (Stays at the starting size)"
- minWindowWidth: 120
  $name: Minimum Window Width
  $description: "Ignore small tooltips and popups narrower than this width."
- minWindowHeight: 80
  $name: Minimum Window Height
  $description: "Ignore small tooltips and popups shorter than this height."
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <dwmapi.h>
#include <mmsystem.h>
#include <windhawk_api.h>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <atomic>
#include <cmath>
#include <algorithm>

// Windows 11 DWM backdrop and corner constants
#ifndef DWMWA_SYSTEMBACKDROP_TYPE
#define DWMWA_SYSTEMBACKDROP_TYPE 38
#endif

#ifndef DWMSBT_TRANSIENTWINDOW
#define DWMSBT_TRANSIENTWINDOW 3 // Acrylic
#endif

#ifndef DWMWA_BORDER_COLOR
#define DWMWA_BORDER_COLOR 34
#endif

#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
#define DWMWA_WINDOW_CORNER_PREFERENCE 33
#endif

#ifndef DWMWCP_DEFAULT
#define DWMWCP_DEFAULT 0
#endif

#ifndef DWMWCP_DONOTROUND
#define DWMWCP_DONOTROUND 1
#endif

#ifndef DWMWCP_ROUND
#define DWMWCP_ROUND 2
#endif

#ifndef DWMWCP_ROUNDSMALL
#define DWMWCP_ROUNDSMALL 3
#endif

// Undocumented SetWindowCompositionAttribute definitions
typedef enum _ACCENT_STATE {
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4
} ACCENT_STATE;

typedef struct _ACCENT_POLICY {
    ACCENT_STATE AccentState;
    DWORD AccentFlags;
    DWORD GradientColor;
    DWORD AnimationId;
} ACCENT_POLICY;

typedef enum _WINDOWCOMPOSITIONATTRIB {
    WCA_NCSHADOW = 12,
    WCA_ACCENT_POLICY = 19
} WINDOWCOMPOSITIONATTRIB;

typedef struct _WINDOWCOMPOSITIONATTRIBDATA {
    WINDOWCOMPOSITIONATTRIB Attrib;
    PVOID pvData;
    SIZE_T cbData;
} WINDOWCOMPOSITIONATTRIBDATA;

typedef BOOL (WINAPI *pfnSetWindowCompositionAttribute)(HWND, WINDOWCOMPOSITIONATTRIBDATA*);
static pfnSetWindowCompositionAttribute g_pSetWindowCompositionAttribute = nullptr;

// Settings enumerations
enum FadeEffectMode {
    EFFECT_UNDERLAY = 0,
    EFFECT_SIMPLE   = 1
};

enum AcrylicRenderer {
    ACRYLIC_LEGACY = 0,
    ACRYLIC_SYSTEM = 1
};

enum CornerStyleMode {
    CORNER_ROUND       = 0,
    CORNER_ROUND_SMALL = 1,
    CORNER_SQUARE      = 2
};

enum DragActionType {
    ACTION_MOVE    = 1,
    ACTION_RESIZE  = 2
};

enum UnderlayFollowMode {
    UNDERLAY_FOLLOW = 0,
    UNDERLAY_STAY   = 1
};

// Global Configuration
static FadeEffectMode     g_fadeEffect          = EFFECT_UNDERLAY;
static AcrylicRenderer    g_acrylicRenderer     = ACRYLIC_LEGACY;
static CornerStyleMode    g_cornerStyle         = CORNER_ROUND;
static int                g_targetOpacity       = 160;
static BYTE               g_tintR               = 32;
static BYTE               g_tintG               = 32;
static BYTE               g_tintB               = 32;
static BYTE               g_tintOpacity         = 60;
static bool               g_simpleTintEnabled   = false;
static int                g_fadeSpeed           = 20;
static int                g_holdDelay           = 250;
static bool               g_enableOnMove        = true;
static bool               g_enableOnResize      = true;
static UnderlayFollowMode g_underlayMoveMode    = UNDERLAY_FOLLOW;
static UnderlayFollowMode g_underlayResizeMode  = UNDERLAY_FOLLOW;
static int                g_timerIntervalMs     = 8;
static int                g_minWindowWidth      = 120;
static int                g_minWindowHeight     = 80;

static HINSTANCE          g_hInstance           = nullptr;
static bool               g_timerResolutionEnabled = false;

// Global unload flag to immediately stop intercepting messages when mod is updating or unloading
static std::atomic<bool>  g_unloading{false};
static std::atomic<unsigned> g_activeCallbacks{0};

struct CallbackUse {
    CallbackUse() { g_activeCallbacks.fetch_add(1); }
    ~CallbackUse() { g_activeCallbacks.fetch_sub(1); }
    CallbackUse(const CallbackUse&) = delete;
    CallbackUse& operator=(const CallbackUse&) = delete;
};

struct FrameMargin {
    int left;
    int top;
    int right;
    int bottom;
};

struct DragState {
    HWND hTargetWnd;
    BYTE origAlpha;
    COLORREF origColorKey;
    DWORD origLayerFlags;
    bool hadLayered;
    bool isULW;
    DragActionType action;
    ULONGLONG lastStepTime;
    HWND hHelperWnd;
    FrameMargin margin;
    bool isFadingIn;
    bool startedWithMouse;
    
    // Hold delay configuration
    ULONGLONG triggerTime;
    bool fadeStarted;
    bool ownsLayeredStyle;
    POINT startCursorPos;
    RECT initialVisibleRect;
    RECT initialWindowRect;
    bool observedDragEvent;
    unsigned callbackRefs;
    bool cleanupRequested;
};

// Reentrant window messages can destroy a target while a fade call is still
// using its state. Delay the delete until that outer call has returned.
struct StateUse {
    DragState* state;
    explicit StateUse(DragState* value) : state(value) {
        if (state) ++state->callbackRefs;
    }
    ~StateUse() {
        if (state && --state->callbackRefs == 0 && state->cleanupRequested)
            delete state;
    }
    StateUse(const StateUse&) = delete;
    StateUse& operator=(const StateUse&) = delete;
};

// Forward declarations for animation functions
static void StartFadeIn(HWND hwnd, DragState* state);
static void StepFadeIn(HWND hwnd, DragState* state);
static void StepFadeOut(HWND hwnd, DragState* state);
static void ActivateFade(HWND hwnd, DragState* state);
static void CleanupAndRestoreWindow(HWND hwnd, DragState* state);
static void RestoreWindowAppearance(HWND hwnd, DragState* state);

// Tracking active windows for safe cleanup
// Only the target's UI thread dereferences DragState. Unload reads this
// handle-only registry instead of fetching a potentially freed window property.
static std::unordered_map<HWND, HWND> g_activeWindows;
static std::mutex g_activeMutex;

static void RegisterActiveWindow(HWND hwnd) {
    std::lock_guard<std::mutex> lock(g_activeMutex);
    if (g_activeWindows.emplace(hwnd, nullptr).second && g_activeWindows.size() == 1) {
        g_timerResolutionEnabled = timeBeginPeriod(1) == TIMERR_NOERROR;
    }
}

static void UpdateActiveHelper(HWND hwnd, HWND helper) {
    std::lock_guard<std::mutex> lock(g_activeMutex);
    auto it = g_activeWindows.find(hwnd);
    if (it != g_activeWindows.end()) it->second = helper;
}

static void UnregisterActiveWindow(HWND hwnd) {
    std::lock_guard<std::mutex> lock(g_activeMutex);
    if (g_activeWindows.erase(hwnd) && g_activeWindows.empty() &&
        g_timerResolutionEnabled) {
        timeEndPeriod(1);
        g_timerResolutionEnabled = false;
    }
}

static int GetScreenRefreshRate() {
    DEVMODEW dm{};
    dm.dmSize = sizeof(dm);
    if (EnumDisplaySettingsW(NULL, ENUM_CURRENT_SETTINGS, &dm) && dm.dmDisplayFrequency > 1) {
        return (int)dm.dmDisplayFrequency;
    }
    return 60;
}

static void ParseHexColor(PCWSTR hexStr, BYTE& r, BYTE& g, BYTE& b) {
    if (hexStr && *hexStr == L'#') ++hexStr;
    unsigned int value = 0;
    if (hexStr) {
        for (int i = 0; i < 6; ++i) {
            WCHAR c = hexStr[i];
            if (!c) break;
            unsigned int digit;
            if (c >= L'0' && c <= L'9') digit = c - L'0';
            else if (c >= L'a' && c <= L'f') digit = c - L'a' + 10;
            else if (c >= L'A' && c <= L'F') digit = c - L'A' + 10;
            else break;
            value = (value << 4) | digit;
            if (i == 5 && hexStr[6] == 0) {
                r = (BYTE)(value >> 16);
                g = (BYTE)(value >> 8);
                b = (BYTE)value;
                return;
            }
        }
    }
    r = g = b = 32;
}

static void LoadSettings() {
    PCWSTR effectStr = Wh_GetStringSetting(L"fadeEffect");
    if (effectStr) {
        if (wcscmp(effectStr, L"simple") == 0) {
            g_fadeEffect = EFFECT_SIMPLE;
        } else {
            g_fadeEffect = EFFECT_UNDERLAY;
        }
        Wh_FreeStringSetting(effectStr);
    } else {
        g_fadeEffect = EFFECT_UNDERLAY;
    }

    PCWSTR rendererStr = Wh_GetStringSetting(L"acrylicRenderer");
    g_acrylicRenderer = rendererStr && wcscmp(rendererStr, L"system") == 0
                            ? ACRYLIC_SYSTEM : ACRYLIC_LEGACY;
    if (rendererStr) Wh_FreeStringSetting(rendererStr);

    PCWSTR cornerStr = Wh_GetStringSetting(L"cornerStyle");
    if (cornerStr) {
        if (wcscmp(cornerStr, L"round_small") == 0) {
            g_cornerStyle = CORNER_ROUND_SMALL;
        } else if (wcscmp(cornerStr, L"square") == 0) {
            g_cornerStyle = CORNER_SQUARE;
        } else {
            g_cornerStyle = CORNER_ROUND;
        }
        Wh_FreeStringSetting(cornerStr);
    } else {
        g_cornerStyle = CORNER_ROUND;
    }

    g_targetOpacity    = std::clamp(Wh_GetIntSetting(L"targetOpacity"), 0, 255);
    g_tintOpacity      = (BYTE)std::clamp(Wh_GetIntSetting(L"tintOpacity"), 0, 255);
    g_simpleTintEnabled = Wh_GetIntSetting(L"simpleTintEnabled") != 0;
    g_fadeSpeed        = std::clamp(Wh_GetIntSetting(L"fadeSpeed"), 1, 255);
    g_holdDelay        = std::clamp(Wh_GetIntSetting(L"holdDelay"), 0, 1000);
    g_enableOnMove     = Wh_GetIntSetting(L"enableOnMove") != 0;
    g_enableOnResize   = Wh_GetIntSetting(L"enableOnResize") != 0;
    g_minWindowWidth   = std::max(20, Wh_GetIntSetting(L"minWindowWidth"));
    g_minWindowHeight  = std::max(20, Wh_GetIntSetting(L"minWindowHeight"));

    PCWSTR moveBehaviorStr = Wh_GetStringSetting(L"underlayBehaviorOnMove");
    if (moveBehaviorStr) {
        if (wcscmp(moveBehaviorStr, L"stay") == 0) {
            g_underlayMoveMode = UNDERLAY_STAY;
        } else {
            g_underlayMoveMode = UNDERLAY_FOLLOW;
        }
        Wh_FreeStringSetting(moveBehaviorStr);
    } else {
        g_underlayMoveMode = UNDERLAY_FOLLOW;
    }

    PCWSTR resizeBehaviorStr = Wh_GetStringSetting(L"underlayBehaviorOnResize");
    if (resizeBehaviorStr) {
        if (wcscmp(resizeBehaviorStr, L"stay") == 0) {
            g_underlayResizeMode = UNDERLAY_STAY;
        } else {
            g_underlayResizeMode = UNDERLAY_FOLLOW;
        }
        Wh_FreeStringSetting(resizeBehaviorStr);
    } else {
        g_underlayResizeMode = UNDERLAY_FOLLOW;
    }

    PCWSTR hexColor = Wh_GetStringSetting(L"tintColorHex");
    ParseHexColor(hexColor, g_tintR, g_tintG, g_tintB);
    if (hexColor) Wh_FreeStringSetting(hexColor);

    // Follow screen refresh rate automatically for silky-smooth 60Hz/120Hz/144Hz/240Hz animation
    int refreshHz = GetScreenRefreshRate();
    g_timerIntervalMs = std::clamp((int)std::round(1000.0 / (double)refreshHz), 1, 33);
}

static bool IsInternalModClass(HWND hwnd) {
    WCHAR className[64];
    if (GetClassNameW(hwnd, className, 64)) {
        if (wcscmp(className, L"DragFadeUnderlayClass") == 0) {
            return true;
        }
    }
    return false;
}

static bool IsValidTargetWindow(HWND hwnd) {
    if (!hwnd || !IsWindow(hwnd)) return false;
    if (IsInternalModClass(hwnd)) return false;
    if (IsHungAppWindow(hwnd)) return false;

    LONG_PTR style = GetWindowLongPtrW(hwnd, GWL_STYLE);
    if (style & WS_CHILD) return false;
    if (GetAncestor(hwnd, GA_ROOT) != hwnd) return false;

    // Filter out shell desktop and taskbars, but allow consoles, tool windows, popouts, and standard apps
    WCHAR className[64];
    if (GetClassNameW(hwnd, className, ARRAYSIZE(className))) {
        if (wcscmp(className, L"Progman") == 0 ||
            wcscmp(className, L"WorkerW") == 0 ||
            wcscmp(className, L"Shell_TrayWnd") == 0 ||
            wcscmp(className, L"Shell_SecondaryTrayWnd") == 0 ||
            wcscmp(className, L"tooltips_class32") == 0 ||
            wcscmp(className, L"#32768") == 0 ||
            wcscmp(className, L"SysShadow") == 0) {
            return false;
        }
    }

    RECT rc;
    if (GetWindowRect(hwnd, &rc)) {
        int width = rc.right - rc.left;
        int height = rc.bottom - rc.top;

        // Tool windows and compact popouts (like Opera video popout) can have smaller dimensions
        LONG_PTR exStyle = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
        int minW = (exStyle & WS_EX_TOOLWINDOW) ? 20 : g_minWindowWidth;
        int minH = (exStyle & WS_EX_TOOLWINDOW) ? 20 : g_minWindowHeight;

        if (width < minW || height < minH) {
            return false;
        }
    } else {
        return false;
    }
    return true;
}

// Request helper destruction on its owning thread.
static void SafeDestroyHelperWindow(HWND hHelper) {
    if (hHelper && IsWindow(hHelper)) {
        DWORD helperTid = GetWindowThreadProcessId(hHelper, NULL);
        if (helperTid == GetCurrentThreadId()) {
            DestroyWindow(hHelper);
        } else {
            // The owning thread clears its own timer and state in WM_CLOSE.
            PostMessageW(hHelper, WM_CLOSE, 0, 0);
        }
    }
}

// ---------------------------------------------------------------------------
// Native DWM Backdrop Underlay Window Procedure & Construction
// ---------------------------------------------------------------------------

// Helper to get target window's visible bounds
static bool GetTargetVisibleRect(HWND hTarget, const FrameMargin& margin, RECT* outRect) {
    if (!hTarget || !IsWindow(hTarget) || !outRect) return false;
    RECT rcExt{};
    if (DwmGetWindowAttribute(hTarget, (DWMWINDOWATTRIBUTE)DWMWA_EXTENDED_FRAME_BOUNDS, &rcExt, sizeof(rcExt)) == S_OK) {
        *outRect = rcExt;
        return true;
    }
    RECT rcWin{};
    if (GetWindowRect(hTarget, &rcWin)) {
        outRect->left = rcWin.left + margin.left;
        outRect->top = rcWin.top + margin.top;
        outRect->right = rcWin.right - margin.right;
        outRect->bottom = rcWin.bottom - margin.bottom;
        return true;
    }
    return false;
}

static LRESULT CALLBACK UnderlayWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    CallbackUse callback;
    if (msg == WM_APP + 31) {
        DragState* state = (DragState*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
        if (state) CleanupAndRestoreWindow(state->hTargetWnd, state);
        return 0;
    }
    // Prevent any mouse sizing anchors or clicks from hitting the underlay window
    if (msg == WM_NCHITTEST) {
        return HTTRANSPARENT;
    }
    // Prevent classic window borders from being calculated by User32
    if (msg == WM_NCCALCSIZE) {
        return 0;
    }
    if (msg == WM_CLOSE) {
        DragState* state = (DragState*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
        if (state && state->hHelperWnd == hwnd) {
            CleanupAndRestoreWindow(state->hTargetWnd, state);
            return 0;
        }
        KillTimer(hwnd, 1);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
        DestroyWindow(hwnd);
        return 0;
    }
    if (msg == WM_NCDESTROY) {
        DragState* state = (DragState*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
        if (state && state->hHelperWnd == hwnd) {
            state->hHelperWnd = nullptr;
            CleanupAndRestoreWindow(state->hTargetWnd, state);
        }
    }
    if (msg == WM_ERASEBKGND) {
        return 1; // Prevent GDI background erase from drawing opaque black
    }
    if (msg == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(hwnd, &ps);
        // The Simple Fade tint helper is the only layered window of this
        // class. The acrylic helper retains its own DWM-drawn background.
        if (dc && (GetWindowLongPtrW(hwnd, GWL_EXSTYLE) & WS_EX_LAYERED)) {
            RECT client{};
            GetClientRect(hwnd, &client);
            SetDCBrushColor(dc, RGB(g_tintR, g_tintG, g_tintB));
            FillRect(dc, &client, (HBRUSH)GetStockObject(DC_BRUSH));
        }
        EndPaint(hwnd, &ps);
        return 0; // Validate update region without GDI black fill
    }
    if (msg == WM_TIMER && wp == 1) {
        if (g_unloading.load(std::memory_order_relaxed)) {
            KillTimer(hwnd, 1);
            return 0;
        }

        DragState* state = (DragState*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
        if (!state) {
            KillTimer(hwnd, 1);
            return 0;
        }
        StateUse keepAlive(state);

        if (!IsWindow(state->hTargetWnd) ||
            GetPropW(state->hTargetWnd, L"DragFadeState") != state) {
            CleanupAndRestoreWindow(state->hTargetWnd, state);
            return 0;
        }

        // Watchdog: If drag was initiated with mouse, verify if mouse button has been released
        if (state->startedWithMouse) {
            bool lDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
            bool rDown = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
            if (!lDown && !rDown) {
                if (!state->fadeStarted) {
                    // Released before hold delay expired: cancel cleanly without modifying window
                    CleanupAndRestoreWindow(state->hTargetWnd, state);
                    return 0;
                } else if (!state->isFadingIn) {
                    StartFadeIn(state->hTargetWnd, state);
                    return 0;
                }
            }
        }

        // Check hold delay & movement threshold
        if (!state->fadeStarted && !state->isFadingIn) {
            ULONGLONG now = GetTickCount64();
            POINT curPt{};
            GetCursorPos(&curPt);
            int dx = std::abs(curPt.x - state->startCursorPos.x);
            int dy = std::abs(curPt.y - state->startCursorPos.y);
            bool movedBeyondThreshold = (dx >= 4 || dy >= 4);
            // Many applications process WM_SIZING themselves, so DefWindowProc
            // never sees it. Read committed bounds on the existing UI-thread
            // timer to recognize the first actual resize without the hold delay.
            RECT currentRect{};
            bool resized = g_enableOnResize &&
                GetWindowRect(state->hTargetWnd, &currentRect) &&
                (currentRect.right - currentRect.left !=
                     state->initialWindowRect.right - state->initialWindowRect.left ||
                 currentRect.bottom - currentRect.top !=
                     state->initialWindowRect.bottom - state->initialWindowRect.top);
            if (resized) state->action = ACTION_RESIZE;

            if (state->observedDragEvent || resized || movedBeyondThreshold ||
                now >= state->triggerTime + (ULONGLONG)g_holdDelay) {
                ActivateFade(state->hTargetWnd, state);
                return 0; // ActivateFade can replace and destroy this timer window.
            } else {
                return 0; // Still waiting for hold delay
            }
        }

        // Active Position Tracking: Ensure underlay stays locked right behind the window as you drag/resize
        if (!state->isFadingIn && state->fadeStarted) {
            bool shouldFollow = (GetWindowLongPtrW(hwnd, GWL_EXSTYLE) & WS_EX_LAYERED) ||
                                (state->action == ACTION_MOVE && g_underlayMoveMode == UNDERLAY_FOLLOW) ||
                                (state->action == ACTION_RESIZE && g_underlayResizeMode == UNDERLAY_FOLLOW);
            if (shouldFollow) {
                RECT rcTarget{};
                if (GetTargetVisibleRect(state->hTargetWnd, state->margin, &rcTarget)) {
                    int targetW = rcTarget.right - rcTarget.left;
                    int targetH = rcTarget.bottom - rcTarget.top;

                    RECT rcUnderlay{};
                    GetWindowRect(hwnd, &rcUnderlay);
                    int curW = rcUnderlay.right - rcUnderlay.left;
                    int curH = rcUnderlay.bottom - rcUnderlay.top;

                    if (rcUnderlay.left != rcTarget.left || rcUnderlay.top != rcTarget.top ||
                        curW != targetW || curH != targetH) {
                        SetWindowPos(hwnd, state->hTargetWnd, rcTarget.left, rcTarget.top, targetW, targetH,
                                     SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOSENDCHANGING);
                    }
                }
            }
        }

        if (state->isFadingIn) {
            StepFadeIn(state->hTargetWnd, state);
        } else if (state->fadeStarted) {
            StepFadeOut(state->hTargetWnd, state);
        }
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

static void EnsureHelperClassesRegistered() {
    WNDCLASSEXW existing{};
    if (GetClassInfoExW(g_hInstance, L"DragFadeUnderlayClass", &existing)) {
        return;
    }

    WNDCLASSEXW wcU{};
    wcU.cbSize        = sizeof(WNDCLASSEXW);
    wcU.style         = 0; // No CS_DROPSHADOW
    wcU.lpfnWndProc   = UnderlayWndProc;
    wcU.hInstance     = g_hInstance;
    wcU.lpszClassName = L"DragFadeUnderlayClass";
    wcU.hbrBackground = NULL; // No background brush to prevent GDI erase flashing
    RegisterClassExW(&wcU);
}

// Creates the Native Backdrop Underlay helper window placed directly underneath the target window
static HWND CreateUnderlayHelper(HWND hTarget, DragState* state, const RECT& rc) {
    EnsureHelperClassesRegistered();

    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;
    if (w < 1) w = 100;
    if (h < 1) h = 100;

    HWND hUnderlay = CreateWindowExW(
        WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        L"DragFadeUnderlayClass",
        NULL,
        WS_POPUP,
        rc.left, rc.top, w, h,
        NULL, NULL, g_hInstance, NULL
    );

    if (!hUnderlay) return NULL;

    SetWindowLongPtrW(hUnderlay, GWLP_USERDATA, (LONG_PTR)state);

    // Disable window transitions on underlay helper to prevent animation latency
    BOOL disableTransitions = TRUE;
    DwmSetWindowAttribute(hUnderlay, (DWMWINDOWATTRIBUTE)3 /* DWMWA_TRANSITIONS_FORCEDISABLED */, &disableTransitions, sizeof(disableTransitions));

    // Synchronize dark/light mode with target window
    BOOL isDark = FALSE;
    if (DwmGetWindowAttribute(hTarget, 20 /* DWMWA_USE_IMMERSIVE_DARK_MODE */, &isDark, sizeof(isDark)) == S_OK) {
        DwmSetWindowAttribute(hUnderlay, 20, &isDark, sizeof(isDark));
    }

    // Extend DWM frame into entire client area so backdrop shader covers the window
    MARGINS margins = {-1, -1, -1, -1};
    DwmExtendFrameIntoClientArea(hUnderlay, &margins);

    // The helper stays inactive behind the target. Mica falls back to gray for
    // inactive windows, so use Acrylic only. Avoid combining backdrops: an
    // accent gradient can replace native blur.
    bool acrylicApplied = false;
    if (g_acrylicRenderer == ACRYLIC_SYSTEM) {
        int dwmBackdrop = DWMSBT_TRANSIENTWINDOW;
        acrylicApplied = SUCCEEDED(DwmSetWindowAttribute(
            hUnderlay, (DWMWINDOWATTRIBUTE)DWMWA_SYSTEMBACKDROP_TYPE,
            &dwmBackdrop, sizeof(dwmBackdrop)));
    }
    if (!acrylicApplied && g_pSetWindowCompositionAttribute) {
        ACCENT_POLICY policy{};
        policy.AccentState = ACCENT_ENABLE_ACRYLICBLURBEHIND;
        policy.AccentFlags = 2;
        policy.GradientColor = ((DWORD)std::max(1, (int)g_tintOpacity) << 24) |
                               ((DWORD)g_tintB << 16) |
                               ((DWORD)g_tintG << 8) | (DWORD)g_tintR;

        WINDOWCOMPOSITIONATTRIBDATA data{};
        data.Attrib = WCA_ACCENT_POLICY;
        data.pvData = &policy;
        data.cbData = sizeof(policy);
        acrylicApplied = g_pSetWindowCompositionAttribute(hUnderlay, &data) != FALSE;
    }
    if (!acrylicApplied && g_acrylicRenderer == ACRYLIC_LEGACY) {
        // Fallback for Windows 11 when the legacy renderer is unavailable.
        int dwmBackdrop = DWMSBT_TRANSIENTWINDOW;
        HRESULT hr = DwmSetWindowAttribute(hUnderlay,
                                           (DWMWINDOWATTRIBUTE)DWMWA_SYSTEMBACKDROP_TYPE,
                                           &dwmBackdrop, sizeof(dwmBackdrop));
        acrylicApplied = SUCCEEDED(hr);
    }
    if (!acrylicApplied) {
        Wh_Log(L"Drag Fade: Acrylic backdrop unavailable for helper %p", hUnderlay);
        DestroyWindow(hUnderlay);
        return NULL;
    }

    // Explicit user choices override the target; the default follows its preference.
    int corner = DWMWCP_ROUND;
    if (g_cornerStyle == CORNER_ROUND_SMALL) {
        corner = DWMWCP_ROUNDSMALL;
    } else if (g_cornerStyle == CORNER_SQUARE) {
        corner = DWMWCP_DONOTROUND;
    } else {
        int targetCorner = DWMWCP_DEFAULT;
        if (DwmGetWindowAttribute(hTarget,
                (DWMWINDOWATTRIBUTE)DWMWA_WINDOW_CORNER_PREFERENCE,
                &targetCorner, sizeof(targetCorner)) == S_OK &&
            targetCorner != DWMWCP_DEFAULT) {
            corner = targetCorner;
        }
    }
    DwmSetWindowAttribute(hUnderlay,
                          (DWMWINDOWATTRIBUTE)DWMWA_WINDOW_CORNER_PREFERENCE,
                          &corner, sizeof(corner));

    // Match the target window's border color (or system default) so no black outline is produced
    COLORREF targetBorderColor = 0xFFFFFFFF; // DWMWA_COLOR_DEFAULT
    if (DwmGetWindowAttribute(hTarget, (DWMWINDOWATTRIBUTE)DWMWA_BORDER_COLOR, &targetBorderColor, sizeof(targetBorderColor)) == S_OK) {
        DwmSetWindowAttribute(hUnderlay, (DWMWINDOWATTRIBUTE)DWMWA_BORDER_COLOR, &targetBorderColor, sizeof(targetBorderColor));
    } else {
        COLORREF defaultBorder = 0xFFFFFFFF;
        DwmSetWindowAttribute(hUnderlay, (DWMWINDOWATTRIBUTE)DWMWA_BORDER_COLOR, &defaultBorder, sizeof(defaultBorder));
    }

    if (g_pSetWindowCompositionAttribute) {
        DWORD ncShadow = 0; // Disable NC drop shadow
        WINDOWCOMPOSITIONATTRIBDATA shadowData{};
        shadowData.Attrib = WCA_NCSHADOW;
        shadowData.pvData = &ncShadow;
        shadowData.cbData = sizeof(ncShadow);
        g_pSetWindowCompositionAttribute(hUnderlay, &shadowData);
    }

    // Dedicated timer for animation and release detection.
    if (!SetTimer(hUnderlay, 1, g_timerIntervalMs, NULL)) {
        DestroyWindow(hUnderlay);
        return NULL;
    }

    // Position the underlay behind the target before changing the target alpha.
    // Delayed SWP_SHOWWINDOW during WM_MOVING can disturb drag tracking.
    if (!SetWindowPos(hUnderlay, hTarget, rc.left, rc.top, w, h,
                      SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOSENDCHANGING)) {
        DestroyWindow(hUnderlay);
        return NULL;
    }

    return hUnderlay;
}

// Creates an invisible helper window to drive the timer watchdog when in Simple Fade mode
static HWND CreateSimpleHelper(DragState* state) {
    EnsureHelperClassesRegistered();

    HWND hHelper = CreateWindowExW(
        WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        L"DragFadeUnderlayClass",
        NULL,
        WS_POPUP,
        0, 0, 0, 0,
        NULL, NULL, g_hInstance, NULL
    );

    if (hHelper) {
        SetWindowLongPtrW(hHelper, GWLP_USERDATA, (LONG_PTR)state);
        if (!SetTimer(hHelper, 1, g_timerIntervalMs, NULL)) {
            DestroyWindow(hHelper);
            return NULL;
        }
    }

    return hHelper;
}

// Optional translucent tint for Simple Fade. The existing timer-only helper
// remains in use until fading actually starts, avoiding a visible press flash.
static HWND CreateTintedSimpleHelper(HWND target, DragState* state, const RECT& rc) {
    EnsureHelperClassesRegistered();
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;
    if (width <= 0 || height <= 0) return NULL;

    HWND helper = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        L"DragFadeUnderlayClass", NULL, WS_POPUP,
        rc.left, rc.top, width, height,
        NULL, NULL, g_hInstance, NULL);
    if (!helper) return NULL;
    SetWindowLongPtrW(helper, GWLP_USERDATA, (LONG_PTR)state);
    if (!SetLayeredWindowAttributes(helper, 0, g_tintOpacity, LWA_ALPHA) ||
        !SetTimer(helper, 1, g_timerIntervalMs, NULL)) {
        DestroyWindow(helper);
        return NULL;
    }
    // Paint while hidden so its first compositor frame already has the tint.
    InvalidateRect(helper, NULL, FALSE);
    UpdateWindow(helper);
    if (!SetWindowPos(helper, target, rc.left, rc.top, width, height,
                      SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOSENDCHANGING)) {
        DestroyWindow(helper);
        return NULL;
    }
    return helper;
}

// ---------------------------------------------------------------------------
// Animation & Fade State Management
// ---------------------------------------------------------------------------

static void CleanupAndRestoreWindow(HWND hwnd, DragState* state) {
    if (state && !state->cleanupRequested) {
        state->cleanupRequested = true;
        // The property is the ownership token. Detach it first so reentrant
        // window messages cannot run a second animation on this state.
        bool stillOwned = IsWindow(hwnd) &&
            GetPropW(hwnd, L"DragFadeState") == state;
        if (stillOwned) RemovePropW(hwnd, L"DragFadeState");
        // 1. Destroy helper window (halts its animation watchdog timer)
        HWND helper = state->hHelperWnd;
        state->hHelperWnd = nullptr;
        if (helper) SafeDestroyHelperWindow(helper);

        if (stillOwned) RestoreWindowAppearance(hwnd, state);

        UnregisterActiveWindow(hwnd);
        if (!state->callbackRefs) delete state;
    }
}

static void RestoreWindowAppearance(HWND hwnd, DragState* state) {
    if (!IsWindow(hwnd)) return;
    if (state->hadLayered && !state->isULW && state->fadeStarted) {
        SetLayeredWindowAttributes(hwnd, state->origColorKey, state->origAlpha,
                                   state->origLayerFlags);
    } else if (state->ownsLayeredStyle) {
        LONG_PTR currentStyle = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
        if (currentStyle & WS_EX_LAYERED) {
            SetWindowLongPtrW(hwnd, GWL_EXSTYLE, currentStyle & ~WS_EX_LAYERED);
            SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
                         SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                         SWP_FRAMECHANGED | SWP_NOACTIVATE);
        }
    }
}

static void ActivateFade(HWND hwnd, DragState* state) {
    if (!state || state->cleanupRequested || state->fadeStarted || state->isFadingIn) return;

    state->fadeStarted = true;
    state->lastStepTime = GetTickCount64();

    RECT rcExt{};
    if (DwmGetWindowAttribute(hwnd, (DWMWINDOWATTRIBUTE)DWMWA_EXTENDED_FRAME_BOUNDS, &rcExt, sizeof(rcExt)) != S_OK) {
        GetWindowRect(hwnd, &rcExt);
    }
    if (state->cleanupRequested) return;

    // Effect Specific Initializations
    if (g_fadeEffect == EFFECT_UNDERLAY) {
        bool stay = (state->action == ACTION_MOVE && g_underlayMoveMode == UNDERLAY_STAY) ||
                    (state->action == ACTION_RESIZE && g_underlayResizeMode == UNDERLAY_STAY);
        HWND previousHelper = state->hHelperWnd;
        HWND underlay = CreateUnderlayHelper(
            hwnd, state, stay ? state->initialVisibleRect : rcExt);
        if (state->cleanupRequested) {
            if (underlay) SafeDestroyHelperWindow(underlay);
            return;
        }
        if (underlay) {
            state->hHelperWnd = underlay;
            if (previousHelper) SafeDestroyHelperWindow(previousHelper);
        } else {
            // Keep the existing watchdog and fall back to a plain fade.
            if (!previousHelper) state->hHelperWnd = CreateSimpleHelper(state);
            Wh_Log(L"Drag Fade: underlay creation failed; using simple fade");
        }
    } else {
        if (g_simpleTintEnabled && g_tintOpacity != 0) {
            HWND previousHelper = state->hHelperWnd;
            HWND tinted = CreateTintedSimpleHelper(hwnd, state, rcExt);
            if (state->cleanupRequested) {
                if (tinted) SafeDestroyHelperWindow(tinted);
                return;
            }
            if (tinted) {
                state->hHelperWnd = tinted;
                if (previousHelper) SafeDestroyHelperWindow(previousHelper);
            } else if (!previousHelper) {
                state->hHelperWnd = CreateSimpleHelper(state);
            }
        } else if (!state->hHelperWnd || !IsWindow(state->hHelperWnd)) {
            state->hHelperWnd = CreateSimpleHelper(state);
        }
    }
    if (!state->hHelperWnd) {
        state->fadeStarted = false;
        CleanupAndRestoreWindow(hwnd, state); // No watchdog means no safe fade.
        return;
    }
    UpdateActiveHelper(hwnd, state->hHelperWnd);

    // Make sure the target window is layered to allow alpha fading
    if (!state->isULW) {
        LONG_PTR currentStyle = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
        if (!(currentStyle & WS_EX_LAYERED)) {
            SetLastError(ERROR_SUCCESS);
            LONG_PTR previous = SetWindowLongPtrW(hwnd, GWL_EXSTYLE,
                                                   currentStyle | WS_EX_LAYERED);
            if (state->cleanupRequested) return;
            if (previous == 0 && GetLastError() != ERROR_SUCCESS) {
                CleanupAndRestoreWindow(hwnd, state);
                return;
            }
            state->ownsLayeredStyle = true;
        }

        const DWORD fadeFlags = state->origLayerFlags | LWA_ALPHA;
        if (!SetLayeredWindowAttributes(hwnd, state->origColorKey, state->origAlpha,
                                        fadeFlags)) {
            CleanupAndRestoreWindow(hwnd, state);
            return;
        }
        if (state->cleanupRequested) return;
        if (g_fadeSpeed >= 255) {
            SetLayeredWindowAttributes(hwnd, state->origColorKey,
                                       (BYTE)std::min(g_targetOpacity, (int)state->origAlpha), fadeFlags);
        } else {
            StepFadeOut(hwnd, state);
        }
    }
}

static void StepFadeOut(HWND hwnd, DragState* state) {
    if (!state || state->cleanupRequested || state->isULW || !state->fadeStarted) return;

    BYTE currentAlpha = 255;
    if (!GetLayeredWindowAttributes(hwnd, NULL, &currentAlpha, NULL)) {
        currentAlpha = 255;
    }
    if (state->cleanupRequested) return;

    ULONGLONG now = GetTickCount64();
    DWORD elapsed = (DWORD)(now - state->lastStepTime);
    if (elapsed < 1) elapsed = 1;
    if (elapsed > 32) elapsed = 32; // Prevent any hitch or modal lag from jumping opacity in one frame
    state->lastStepTime = now;

    int step = (int)std::round((double)g_fadeSpeed * ((double)elapsed / 16.0));
    if (step < 1) step = 1;

    // Cap step so transition always has multiple visual frames regardless of speed
    if (g_fadeSpeed < 255) {
        int totalDelta = (int)state->origAlpha - g_targetOpacity;
        int maxStep = (totalDelta > 0) ? std::max(4, totalDelta / 4) : 25;
        if (step > maxStep) step = maxStep;
    }

    int target = std::min(g_targetOpacity, (int)state->origAlpha);
    if (currentAlpha > target) {
        int next = (int)currentAlpha - step;
        if (next <= target) {
            next = target;
        }
        SetLayeredWindowAttributes(hwnd, state->origColorKey, (BYTE)next,
                                   state->origLayerFlags | LWA_ALPHA);
    }
}

static void StepFadeIn(HWND hwnd, DragState* state) {
    if (!state || state->cleanupRequested) return;
    if (state->isULW || !state->fadeStarted) {
        CleanupAndRestoreWindow(hwnd, state);
        return;
    }

    BYTE currentAlpha = 255;
    if (!GetLayeredWindowAttributes(hwnd, NULL, &currentAlpha, NULL)) {
        currentAlpha = 255;
    }
    if (state->cleanupRequested) return;

    ULONGLONG now = GetTickCount64();
    DWORD elapsed = (DWORD)(now - state->lastStepTime);
    if (elapsed < 1) elapsed = 1;
    if (elapsed > 32) elapsed = 32;
    state->lastStepTime = now;

    int step = (int)std::round((double)g_fadeSpeed * ((double)elapsed / 16.0));
    if (step < 1) step = 1;

    if (g_fadeSpeed < 255) {
        int totalDelta = (int)state->origAlpha - g_targetOpacity;
        int maxStep = (totalDelta > 0) ? std::max(4, totalDelta / 4) : 25;
        if (step > maxStep) step = maxStep;
    }

    if (currentAlpha < state->origAlpha) {
        int next = (int)currentAlpha + step;
        if (next >= (int)state->origAlpha) {
            CleanupAndRestoreWindow(hwnd, state);
            return;
        }
        SetLayeredWindowAttributes(hwnd, state->origColorKey, (BYTE)next,
                                   state->origLayerFlags | LWA_ALPHA);
    } else {
        CleanupAndRestoreWindow(hwnd, state);
    }
}

static void StartFadeOut(HWND hwnd, DragActionType action) {
    if (g_unloading.load(std::memory_order_relaxed)) return;
    DragState* state = (DragState*)GetPropW(hwnd, L"DragFadeState");

    if (!state) {
        state = new DragState{};
        StateUse keepAlive(state);
        state->hTargetWnd = hwnd;
        state->action = action;
        state->triggerTime = GetTickCount64();
        state->observedDragEvent = false;
        state->fadeStarted = false;
        state->lastStepTime = GetTickCount64();
        state->hHelperWnd = NULL;
        state->isFadingIn = false;
        GetCursorPos(&state->startCursorPos);
        state->startedWithMouse = ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0) ||
                                  ((GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0);

        state->hadLayered = (GetWindowLongPtrW(hwnd, GWL_EXSTYLE) & WS_EX_LAYERED) != 0;
        state->isULW = false;
        state->origColorKey = 0;
        state->origLayerFlags = 0;

        if (state->hadLayered) {
            BYTE existingAlpha = 255;
            if (GetLayeredWindowAttributes(hwnd, &state->origColorKey,
                                            &existingAlpha, &state->origLayerFlags)) {
                state->origAlpha = (state->origLayerFlags & LWA_ALPHA) ? existingAlpha : 255;
            } else {
                // Window has WS_EX_LAYERED but uses per-pixel UpdateLayeredWindow
                state->origAlpha = 255;
                state->isULW = true;
            }
        } else {
            state->origAlpha = 255;
        }
        if (state->isULW) {
            state->cleanupRequested = true; // No property or helper was installed.
            return;
        }

        // Accurately calculate the invisible DWM drop shadow / resize margin
        RECT rcWin{};
        RECT rcExt{};
        if (!GetWindowRect(hwnd, &rcWin)) {
            state->cleanupRequested = true;
            return;
        }
        state->initialWindowRect = rcWin;
        if (DwmGetWindowAttribute(hwnd, (DWMWINDOWATTRIBUTE)DWMWA_EXTENDED_FRAME_BOUNDS, &rcExt, sizeof(rcExt)) == S_OK) {
            state->margin.left = rcExt.left - rcWin.left;
            state->margin.top = rcExt.top - rcWin.top;
            state->margin.right = rcWin.right - rcExt.right;
            state->margin.bottom = rcWin.bottom - rcExt.bottom;
        } else {
            state->margin = {0, 0, 0, 0};
            rcExt = rcWin;
        }
        state->initialVisibleRect = rcExt;

        if (!SetPropW(hwnd, L"DragFadeState", (HANDLE)state)) {
            state->cleanupRequested = true;
            return;
        }
        RegisterActiveWindow(hwnd);

        // If hold delay is configured, only create a lightweight invisible timer helper initially
        // The real underlay backdrop helper is only created when the fade actually activates
        if (g_holdDelay > 0) {
            state->hHelperWnd = CreateSimpleHelper(state);
            if (!state->hHelperWnd) CleanupAndRestoreWindow(hwnd, state);
            else UpdateActiveHelper(hwnd, state->hHelperWnd);
        } else {
            ActivateFade(hwnd, state);
        }
    } else {
        state->action = action;
        state->isFadingIn = false;
        if (g_holdDelay == 0 && !state->fadeStarted) {
            ActivateFade(hwnd, state);
        }
    }
}

static void StartFadeIn(HWND hwnd, DragState* state) {
    if (!state || state->isFadingIn) return;

    if (!state->fadeStarted) {
        CleanupAndRestoreWindow(hwnd, state);
        return;
    }

    state->isFadingIn = true;
    state->lastStepTime = GetTickCount64();

    if (g_fadeSpeed >= 255) {
        CleanupAndRestoreWindow(hwnd, state);
    } else {
        StepFadeIn(hwnd, state);
    }
}

// ---------------------------------------------------------------------------
// Universal Win32 Window Message Interception
// ---------------------------------------------------------------------------

static LRESULT HandleDragMessages(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (g_unloading.load(std::memory_order_relaxed)) return 0;

    switch (uMsg) {
        case WM_SYSCOMMAND:
        case WM_ENTERSIZEMOVE:
        case WM_MOVING:
        case WM_SIZING:
        case WM_WINDOWPOSCHANGED:
        case WM_LBUTTONUP:
        case WM_NCLBUTTONUP:
        case WM_EXITSIZEMOVE:
        case WM_CAPTURECHANGED:
        case WM_DESTROY:
            break;
        default:
            return 0;
    }

    if (!hwnd || !IsWindow(hwnd)) return 0;
    if (IsInternalModClass(hwnd)) return 0;
    // Both the target property and its helper belong to this UI thread.
    if (GetWindowThreadProcessId(hwnd, nullptr) != GetCurrentThreadId()) return 0;

    DragState* state = (DragState*)GetPropW(hwnd, L"DragFadeState");
    StateUse keepAlive(state);

    if (!state) {
        if (uMsg != WM_SYSCOMMAND && uMsg != WM_ENTERSIZEMOVE && uMsg != WM_MOVING && uMsg != WM_SIZING) {
            return 0;
        }
        if (!IsValidTargetWindow(hwnd)) {
            return 0;
        }
    }

    switch (uMsg) {
        case WM_SYSCOMMAND: {
            UINT cmd = (UINT)(wParam & 0xFFF0);
            if (cmd == SC_MOVE) {
                SetPropW(hwnd, L"DragPendingAction", (HANDLE)(INT_PTR)ACTION_MOVE);
                if (g_enableOnMove && !state && IsValidTargetWindow(hwnd)) {
                    StartFadeOut(hwnd, ACTION_MOVE);
                }
            } else if (cmd == SC_SIZE) {
                SetPropW(hwnd, L"DragPendingAction", (HANDLE)(INT_PTR)ACTION_RESIZE);
                if (g_enableOnResize && !state && IsValidTargetWindow(hwnd)) {
                    StartFadeOut(hwnd, ACTION_RESIZE);
                }
            }
            break;
        }

        case WM_ENTERSIZEMOVE: {
            INT_PTR pending = (INT_PTR)GetPropW(hwnd, L"DragPendingAction");
            RemovePropW(hwnd, L"DragPendingAction");

            DragActionType action = (pending == ACTION_RESIZE) ? ACTION_RESIZE : ACTION_MOVE;
            if (action == ACTION_MOVE && !g_enableOnMove) return 0;
            if (action == ACTION_RESIZE && !g_enableOnResize) return 0;

            StartFadeOut(hwnd, action);
            break;
        }

        case WM_MOVING:
        case WM_SIZING: {
            DragActionType action = uMsg == WM_MOVING ? ACTION_MOVE : ACTION_RESIZE;
            if ((action == ACTION_MOVE && !g_enableOnMove) ||
                (action == ACTION_RESIZE && !g_enableOnResize)) return 0;
            if (!state) {
                StartFadeOut(hwnd, action);
                state = (DragState*)GetPropW(hwnd, L"DragFadeState");
            }
            if (state) {
                // WM_MOVING/WM_SIZING carries a proposed rectangle. Leave the
                // modal drag loop alone; the helper timer starts the fade after
                // this message, and WM_WINDOWPOSCHANGED follows committed bounds.
                state->action = action;
                state->observedDragEvent = true;
            }
            break;
        }

        case WM_WINDOWPOSCHANGED: {
            if (state && !state->fadeStarted && lParam && g_enableOnResize) {
                WINDOWPOS* pos = (WINDOWPOS*)lParam;
                if (!(pos->flags & SWP_NOSIZE) &&
                    (pos->cx != state->initialWindowRect.right - state->initialWindowRect.left ||
                     pos->cy != state->initialWindowRect.bottom - state->initialWindowRect.top)) {
                    state->action = ACTION_RESIZE;
                    state->observedDragEvent = true;
                }
            }
            if (state && g_fadeEffect == EFFECT_UNDERLAY && state->hHelperWnd &&
                IsWindow(state->hHelperWnd) && !state->isFadingIn && state->fadeStarted && lParam) {
                WINDOWPOS* wp = (WINDOWPOS*)lParam;
                if (!(wp->flags & SWP_NOMOVE) || !(wp->flags & SWP_NOSIZE)) {
                    bool shouldFollow = (state->action == ACTION_MOVE && g_underlayMoveMode == UNDERLAY_FOLLOW) ||
                                        (state->action == ACTION_RESIZE && g_underlayResizeMode == UNDERLAY_FOLLOW);
                    if (shouldFollow) {
                        RECT rcTarget{};
                        if (GetTargetVisibleRect(hwnd, state->margin, &rcTarget)) {
                            int w = rcTarget.right - rcTarget.left;
                            int h = rcTarget.bottom - rcTarget.top;
                            if (w > 0 && h > 0) {
                                SetWindowPos(state->hHelperWnd, hwnd, rcTarget.left, rcTarget.top, w, h,
                                             SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOSENDCHANGING);
                            }
                        }
                    }
                    if (!state->cleanupRequested) StepFadeOut(hwnd, state);
                }
            }
            break;
        }

        case WM_LBUTTONUP:
        case WM_NCLBUTTONUP: {
            if (state && state->startedWithMouse) StartFadeIn(hwnd, state);
            RemovePropW(hwnd, L"DragPendingAction");
            break;
        }
        case WM_EXITSIZEMOVE: {
            RemovePropW(hwnd, L"DragPendingAction");
            if (state) {
                StartFadeIn(hwnd, state);
            }
            break;
        }

        case WM_CAPTURECHANGED: {
            bool lDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
            bool rDown = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
            if (!lDown && !rDown) {
                RemovePropW(hwnd, L"DragPendingAction");
                if (state) {
                    StartFadeIn(hwnd, state);
                }
            }
            break;
        }

        case WM_DESTROY: {
            RemovePropW(hwnd, L"DragPendingAction");
            if (state) CleanupAndRestoreWindow(hwnd, state);
            break;
        }
    }

    return 0;
}

typedef LRESULT (WINAPI *DefWindowProcW_t)(HWND, UINT, WPARAM, LPARAM);
static DefWindowProcW_t DefWindowProcW_Orig = nullptr;

typedef LRESULT (WINAPI *DefWindowProcA_t)(HWND, UINT, WPARAM, LPARAM);
static DefWindowProcA_t DefWindowProcA_Orig = nullptr;

static LRESULT WINAPI DefWindowProcW_Hook(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    CallbackUse callback;
    if (!g_unloading.load(std::memory_order_relaxed)) {
        HandleDragMessages(hwnd, uMsg, wParam, lParam);
    }
    return DefWindowProcW_Orig(hwnd, uMsg, wParam, lParam);
}

static LRESULT WINAPI DefWindowProcA_Hook(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    CallbackUse callback;
    if (!g_unloading.load(std::memory_order_relaxed)) {
        HandleDragMessages(hwnd, uMsg, wParam, lParam);
    }
    return DefWindowProcA_Orig(hwnd, uMsg, wParam, lParam);
}

// ---------------------------------------------------------------------------
// Mod Lifecycle Entry Points
// ---------------------------------------------------------------------------

BOOL Wh_ModInit() {
    g_unloading.store(false);
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            reinterpret_cast<LPCWSTR>(&UnderlayWndProc), &g_hInstance)) {
        return FALSE;
    }
    LoadSettings();

    // Dynamically resolve SetWindowCompositionAttribute
    HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
    if (hUser32) {
        g_pSetWindowCompositionAttribute = (pfnSetWindowCompositionAttribute)GetProcAddress(hUser32, "SetWindowCompositionAttribute");
    }

    if (!hUser32 ||
        !Wh_SetFunctionHook((void*)GetProcAddress(hUser32, "DefWindowProcW"),
                            (void*)DefWindowProcW_Hook, (void**)&DefWindowProcW_Orig) ||
        !Wh_SetFunctionHook((void*)GetProcAddress(hUser32, "DefWindowProcA"),
                            (void*)DefWindowProcA_Hook, (void**)&DefWindowProcA_Orig)) {
        return FALSE;
    }
    return TRUE;
}

void Wh_ModBeforeUninit() {
    // Stop creating states, then ask the owner of each helper to restore its target.
    g_unloading.store(true);
    std::vector<HWND> helpers;
    {
        std::lock_guard<std::mutex> lock(g_activeMutex);
        for (const auto& entry : g_activeWindows) {
            if (entry.second) helpers.push_back(entry.second);
        }
    }
    for (HWND helper : helpers) {
        DWORD pid = 0;
        GetWindowThreadProcessId(helper, &pid);
        if (pid == GetCurrentProcessId() && IsInternalModClass(helper)) {
            DWORD_PTR ignored = 0;
            if (!SendMessageTimeoutW(helper, WM_APP + 31, 0, 0,
                                     SMTO_ABORTIFHUNG | SMTO_BLOCK, 2000, &ignored)) {
                Wh_Log(L"Drag Fade: cleanup timed out for helper %p", helper);
            }
        }
    }
}

void Wh_ModUninit() {
    std::vector<HWND> helpers;
    bool activeStates = false;
    {
        std::lock_guard<std::mutex> lock(g_activeMutex);
        activeStates = !g_activeWindows.empty();
        for (const auto& entry : g_activeWindows) {
            if (entry.second) helpers.push_back(entry.second);
        }
        if (g_timerResolutionEnabled) {
            timeEndPeriod(1);
            g_timerResolutionEnabled = false;
        }
    }
    if (!activeStates && g_activeCallbacks.load() == 0) {
        UnregisterClassW(L"DragFadeUnderlayClass", g_hInstance);
        return;
    }
    {
        // A busy owner or a reentrant callback can outlive the unload request.
        // Keep its code and state mapped until the process exits.
        HMODULE pinned = nullptr;
        if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                GET_MODULE_HANDLE_EX_FLAG_PIN,
                                reinterpret_cast<LPCWSTR>(&UnderlayWndProc), &pinned)) {
            Wh_Log(L"Drag Fade: could not pin module with active callbacks");
        }
        for (HWND helper : helpers) {
            DWORD pid = 0;
            GetWindowThreadProcessId(helper, &pid);
            if (pid == GetCurrentProcessId() && IsInternalModClass(helper)) {
                PostMessageW(helper, WM_APP + 31, 0, 0);
            }
        }
    }
}

BOOL Wh_ModSettingsChanged(BOOL* bReload) {
    *bReload = TRUE; // Reinitialize after all current drags are restored.
    return TRUE;
}
