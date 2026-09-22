// ==WindhawkMod==
// @id              drag-fade
// @name            Drag Fade
// @description     Fades window with live Acrylic, Mica, or Mica Alt during drag and resize
// @version         0.5.4
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
    - underlay: "Native DWM Backdrop Underlay (Live Acrylic, Mica, or Mica Alt behind window)"
    - simple: "Simple Transparent Fade (Classic alpha transition)"
- underlayBackdrop: acrylic
  $name: Backdrop Material (Underlay Mode)
  $description: "DWM backdrop material applied behind the window during drag and resize."
  $options:
    - acrylic: "Acrylic (Frosted translucent blur with custom tint)"
    - mica: "Mica (Dynamic wallpaper-tinted surface)"
    - mica_alt: "Mica Alt (Tabbed wallpaper surface with enhanced contrast)"
- targetOpacity: 160
  $name: Drag Opacity (0-255)
  $description: "Opacity of the window content while dragging (0 = transparent, 255 = opaque)."
- tintColorHex: "202020"
  $name: "Acrylic Tint Color (HEX)"
  $description: "Hex color code for the acrylic tint (e.g. 000000 for dark, FFFFFF for frosted light, 202020 for slate)."
- tintOpacity: 60
  $name: "Acrylic Tint Opacity (0-255)"
  $description: "Strength of the color tint overlay (0 = transparent tint, 255 = solid color tint)."
- acrylicLuminance: false
  $name: Acrylic Frosted Noise / Texture
  $description: "Adds the Windows textured frosted noise layer over the blur. When disabled (default), removes the texture layer for a much clearer, high-transparency glass look."
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
#include <unordered_set>
#include <mutex>
#include <atomic>
#include <cmath>
#include <algorithm>
#include <cstdio>

// Windows 11 DWM backdrop and corner constants
#ifndef DWMWA_SYSTEMBACKDROP_TYPE
#define DWMWA_SYSTEMBACKDROP_TYPE 38
#endif

#ifndef DWMWA_MICA_EFFECT
#define DWMWA_MICA_EFFECT 1029
#endif

#ifndef DWMSBT_AUTO
#define DWMSBT_AUTO 0
#endif

#ifndef DWMSBT_NONE
#define DWMSBT_NONE 1
#endif

#ifndef DWMSBT_MAINWINDOW
#define DWMSBT_MAINWINDOW 2 // Mica
#endif

#ifndef DWMSBT_TRANSIENTWINDOW
#define DWMSBT_TRANSIENTWINDOW 3 // Acrylic
#endif

#ifndef DWMSBT_TABBEDWINDOW
#define DWMSBT_TABBEDWINDOW 4 // Mica Alt
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
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_GRADIENT = 1,
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    ACCENT_ENABLE_BLURBEHIND = 3,
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
    ACCENT_ENABLE_HOSTBACKDROP = 5,
    ACCENT_INVALID_STATE = 6
} ACCENT_STATE;

typedef struct _ACCENT_POLICY {
    ACCENT_STATE AccentState;
    DWORD AccentFlags;
    DWORD GradientColor;
    DWORD AnimationId;
} ACCENT_POLICY;

typedef enum _WINDOWCOMPOSITIONATTRIB {
    WCA_UNDEFINED = 0,
    WCA_NCRENDERING_ENABLED = 1,
    WCA_NCRENDERING_POLICY = 2,
    WCA_TRANSITIONS_FORCEDISABLED = 3,
    WCA_ALLOW_FORCE_DISABLED = 4,
    WCA_GHOST_ALLOWED = 5,
    WCA_NONCLIENT_RTL_LAYOUT = 6,
    WCA_STYLEBOOL = 7,
    WCA_FORCE_ICONIC_REPRESENTATION = 8,
    WCA_EXTENDED_FRAME_BOUNDS = 9,
    WCA_HAS_ICONIC_BITMAP = 10,
    WCA_THEME_METRICS = 11,
    WCA_NCSHADOW = 12,
    WCA_AU_WINDOW_COLOR_STYLE = 13,
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

enum BackdropType {
    BACKDROP_ACRYLIC  = 0,
    BACKDROP_MICA     = 1,
    BACKDROP_MICA_ALT = 2
};

enum CornerStyleMode {
    CORNER_ROUND       = 0,
    CORNER_ROUND_SMALL = 1,
    CORNER_SQUARE      = 2
};

enum DragActionType {
    ACTION_UNKNOWN = 0,
    ACTION_MOVE    = 1,
    ACTION_RESIZE  = 2
};

enum UnderlayFollowMode {
    UNDERLAY_FOLLOW = 0,
    UNDERLAY_STAY   = 1
};

// Global Configuration
static FadeEffectMode     g_fadeEffect          = EFFECT_UNDERLAY;
static BackdropType       g_backdropType        = BACKDROP_ACRYLIC;
static CornerStyleMode    g_cornerStyle         = CORNER_ROUND;
static int                g_targetOpacity       = 160;
static BYTE               g_tintR               = 32;
static BYTE               g_tintG               = 32;
static BYTE               g_tintB               = 32;
static BYTE               g_tintOpacity         = 60;
static bool               g_acrylicLuminance    = false;
static int                g_fadeSpeed           = 20;
static bool               g_enableOnMove        = true;
static bool               g_enableOnResize      = true;
static UnderlayFollowMode g_underlayMoveMode    = UNDERLAY_FOLLOW;
static UnderlayFollowMode g_underlayResizeMode  = UNDERLAY_FOLLOW;
static int                g_timerIntervalMs     = 8;
static int                g_minWindowWidth      = 120;
static int                g_minWindowHeight     = 80;

static HINSTANCE          g_hInstance           = nullptr;

// Global unload flag to immediately stop intercepting messages when mod is updating or unloading
static std::atomic<bool>  g_unloading{false};

struct FrameMargin {
    int left;
    int top;
    int right;
    int bottom;
};

struct DragState {
    HWND hTargetWnd;
    BYTE origAlpha;
    LONG_PTR origExStyle;
    bool hadLayered;
    bool isULW;
    DragActionType action;
    ULONGLONG lastStepTime;
    HWND hHelperWnd;
    FrameMargin margin;
    bool isFadingIn;
    bool startedWithMouse;
};

// Forward declarations for animation functions
static void StartFadeIn(HWND hwnd, DragState* state);
static void StepFadeIn(HWND hwnd, DragState* state);
static void StepFadeOut(HWND hwnd, DragState* state);
static void CleanupAndRestoreWindow(HWND hwnd, DragState* state);

// Tracking active windows for safe cleanup
static std::unordered_set<HWND> g_activeWindows;
static std::mutex g_activeMutex;

static void RegisterActiveWindow(HWND hwnd) {
    std::lock_guard<std::mutex> lock(g_activeMutex);
    g_activeWindows.insert(hwnd);
}

static void UnregisterActiveWindow(HWND hwnd) {
    std::lock_guard<std::mutex> lock(g_activeMutex);
    g_activeWindows.erase(hwnd);
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
    if (!hexStr) { r = 32; g = 32; b = 32; return; }
    if (*hexStr == L'#') hexStr++;
    unsigned int val = 0;
    if (swscanf(hexStr, L"%x", &val) == 1) {
        r = (val >> 16) & 0xFF;
        g = (val >> 8) & 0xFF;
        b = val & 0xFF;
    } else {
        r = 32; g = 32; b = 32;
    }
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

    PCWSTR backdropStr = Wh_GetStringSetting(L"underlayBackdrop");
    if (backdropStr) {
        if (wcscmp(backdropStr, L"mica") == 0) {
            g_backdropType = BACKDROP_MICA;
        } else if (wcscmp(backdropStr, L"mica_alt") == 0) {
            g_backdropType = BACKDROP_MICA_ALT;
        } else {
            g_backdropType = BACKDROP_ACRYLIC;
        }
        Wh_FreeStringSetting(backdropStr);
    } else {
        g_backdropType = BACKDROP_ACRYLIC;
    }

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
    g_acrylicLuminance = Wh_GetIntSetting(L"acrylicLuminance") != 0;
    g_fadeSpeed        = std::max(1, Wh_GetIntSetting(L"fadeSpeed"));
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

// Delphi / VCL applications (e.g. WizTree) cannot tolerate persistent WS_EX_LAYERED on window close or paint
static bool IsVclWindow(HWND hwnd) {
    WCHAR className[64];
    if (GetClassNameW(hwnd, className, 64)) {
        if (className[0] == L'T' || wcsstr(className, L"WizTree") != NULL) {
            return true;
        }
    }
    return false;
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
    }
    return true;
}

// Safely destroys helper windows across threads without leaking or jumping to unmapped code
static void SafeDestroyHelperWindow(HWND hHelper) {
    if (hHelper && IsWindow(hHelper)) {
        KillTimer(hHelper, 1);
        SetWindowLongPtrW(hHelper, GWLP_USERDATA, 0);

        DWORD helperTid = GetWindowThreadProcessId(hHelper, NULL);
        if (helperTid == GetCurrentThreadId()) {
            DestroyWindow(hHelper);
        } else {
            // Post WM_CLOSE to the helper window so its owning thread destroys it cleanly
            PostMessageW(hHelper, WM_CLOSE, 0, 0);
        }
    }
}

// ---------------------------------------------------------------------------
// Native DWM Backdrop Underlay Window Procedure & Construction
// ---------------------------------------------------------------------------

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
    // Prevent any mouse sizing anchors or clicks from hitting the underlay window
    if (msg == WM_NCHITTEST) {
        return HTTRANSPARENT;
    }
    if (msg == WM_CLOSE) {
        KillTimer(hwnd, 1);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
        DestroyWindow(hwnd);
        return 0;
    }
    if (msg == WM_ERASEBKGND) {
        return 1; // Prevent GDI background erase from drawing opaque black
    }
    if (msg == WM_PAINT) {
        PAINTSTRUCT ps;
        BeginPaint(hwnd, &ps);
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

        if (!IsWindow(state->hTargetWnd)) {
            KillTimer(hwnd, 1);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
            DestroyWindow(hwnd);
            return 0;
        }

        // Watchdog: If drag was initiated with mouse, verify if mouse button has been released
        if (state->startedWithMouse) {
            bool lDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
            bool rDown = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
            if (!lDown && !rDown && !state->isFadingIn) {
                StartFadeIn(state->hTargetWnd, state);
                return 0;
            }
        }

        // Active Position Tracking: Ensure underlay stays locked right behind the window as you drag/resize
        if (!state->isFadingIn) {
            bool shouldFollow = (state->action == ACTION_MOVE && g_underlayMoveMode == UNDERLAY_FOLLOW) ||
                                (state->action == ACTION_RESIZE && g_underlayResizeMode == UNDERLAY_FOLLOW) ||
                                (state->action == ACTION_UNKNOWN && g_underlayMoveMode == UNDERLAY_FOLLOW);
            if (shouldFollow) {
                RECT rcTarget{};
                if (GetTargetVisibleRect(state->hTargetWnd, state->margin, &rcTarget)) {
                    RECT rcUnderlay{};
                    GetWindowRect(hwnd, &rcUnderlay);
                    int targetW = rcTarget.right - rcTarget.left;
                    int targetH = rcTarget.bottom - rcTarget.top;
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
        } else {
            StepFadeOut(state->hTargetWnd, state);
        }
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

static void EnsureHelperClassesRegistered() {
    if (!g_hInstance) {
        g_hInstance = GetModuleHandleW(NULL);
    }

    WNDCLASSEXW existing{};
    if (GetClassInfoExW(g_hInstance, L"DragFadeUnderlayClass", &existing)) {
        return;
    }

    WNDCLASSEXW wcU{};
    wcU.cbSize        = sizeof(WNDCLASSEXW);
    wcU.lpfnWndProc   = UnderlayWndProc;
    wcU.hInstance     = g_hInstance;
    wcU.lpszClassName = L"DragFadeUnderlayClass";
    wcU.hbrBackground = NULL; // No background brush to prevent GDI erase flashing
    RegisterClassExW(&wcU);
}

// Creates the Native Backdrop Underlay helper window placed directly underneath the target window
static HWND CreateUnderlayHelper(HWND hTarget, DragState* state, const RECT& rc) {
    EnsureHelperClassesRegistered();

    HWND hUnderlay = CreateWindowExW(
        WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        L"DragFadeUnderlayClass",
        NULL,
        WS_POPUP,
        rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
        NULL, NULL, g_hInstance, NULL
    );

    if (!hUnderlay) return NULL;

    SetWindowLongPtrW(hUnderlay, GWLP_USERDATA, (LONG_PTR)state);

    // Extend DWM frame into entire client area so backdrop shader covers the window
    MARGINS margins = {-1, -1, -1, -1};
    DwmExtendFrameIntoClientArea(hUnderlay, &margins);

    if (g_backdropType == BACKDROP_MICA) {
        int dwmBackdrop = DWMSBT_MAINWINDOW;
        DwmSetWindowAttribute(hUnderlay, (DWMWINDOWATTRIBUTE)DWMWA_SYSTEMBACKDROP_TYPE, &dwmBackdrop, sizeof(dwmBackdrop));
        BOOL mica = TRUE;
        DwmSetWindowAttribute(hUnderlay, (DWMWINDOWATTRIBUTE)DWMWA_MICA_EFFECT, &mica, sizeof(mica));
    } else if (g_backdropType == BACKDROP_MICA_ALT) {
        int dwmBackdrop = DWMSBT_TABBEDWINDOW;
        DwmSetWindowAttribute(hUnderlay, (DWMWINDOWATTRIBUTE)DWMWA_SYSTEMBACKDROP_TYPE, &dwmBackdrop, sizeof(dwmBackdrop));
        BOOL mica = TRUE;
        DwmSetWindowAttribute(hUnderlay, (DWMWINDOWATTRIBUTE)DWMWA_MICA_EFFECT, &mica, sizeof(mica));
    } else if (g_backdropType == BACKDROP_ACRYLIC && !g_acrylicLuminance) {
        // Windows 11 Native Acrylic System Backdrop (Hardware-accelerated, zero-flicker at full screen refresh rate)
        int dwmBackdrop = DWMSBT_TRANSIENTWINDOW;
        HRESULT hr = DwmSetWindowAttribute(hUnderlay, (DWMWINDOWATTRIBUTE)DWMWA_SYSTEMBACKDROP_TYPE, &dwmBackdrop, sizeof(dwmBackdrop));
        if (SUCCEEDED(hr)) {
            // Synchronize dark/light mode with target window
            BOOL isDark = FALSE;
            if (DwmGetWindowAttribute(hTarget, 20 /* DWMWA_USE_IMMERSIVE_DARK_MODE */, &isDark, sizeof(isDark)) == S_OK) {
                DwmSetWindowAttribute(hUnderlay, 20, &isDark, sizeof(isDark));
            }

            // If tint is configured, apply transparent gradient tint overlay
            if (g_tintOpacity > 0 && g_pSetWindowCompositionAttribute) {
                ACCENT_POLICY policy{};
                policy.AccentState = ACCENT_ENABLE_TRANSPARENTGRADIENT;
                policy.AccentFlags = 2;
                policy.GradientColor = ((DWORD)g_tintOpacity << 24) | ((DWORD)g_tintB << 16) | ((DWORD)g_tintG << 8) | (DWORD)g_tintR;

                WINDOWCOMPOSITIONATTRIBDATA data{};
                data.Attrib = WCA_ACCENT_POLICY;
                data.pvData = &policy;
                data.cbData = sizeof(policy);
                g_pSetWindowCompositionAttribute(hUnderlay, &data);
            }
        } else {
            // Windows 10 fallback: legacy SetWindowCompositionAttribute
            int dwmNone = DWMSBT_NONE;
            DwmSetWindowAttribute(hUnderlay, (DWMWINDOWATTRIBUTE)DWMWA_SYSTEMBACKDROP_TYPE, &dwmNone, sizeof(dwmNone));

            if (g_pSetWindowCompositionAttribute) {
                ACCENT_POLICY policy{};
                policy.AccentState = ACCENT_ENABLE_ACRYLICBLURBEHIND;
                policy.AccentFlags = 0;
                policy.GradientColor = ((DWORD)g_tintOpacity << 24) | ((DWORD)g_tintB << 16) | ((DWORD)g_tintG << 8) | (DWORD)g_tintR;

                WINDOWCOMPOSITIONATTRIBDATA data{};
                data.Attrib = WCA_ACCENT_POLICY;
                data.pvData = &policy;
                data.cbData = sizeof(policy);
                g_pSetWindowCompositionAttribute(hUnderlay, &data);
            }
        }
    } else {
        // Acrylic with luminance noise texture enabled
        int dwmBackdrop = DWMSBT_NONE;
        DwmSetWindowAttribute(hUnderlay, (DWMWINDOWATTRIBUTE)DWMWA_SYSTEMBACKDROP_TYPE, &dwmBackdrop, sizeof(dwmBackdrop));

        if (g_pSetWindowCompositionAttribute) {
            ACCENT_POLICY policy{};
            policy.AccentState = ACCENT_ENABLE_ACRYLICBLURBEHIND;
            policy.AccentFlags = g_acrylicLuminance ? 2 : 0;
            policy.GradientColor = ((DWORD)g_tintOpacity << 24) | ((DWORD)g_tintB << 16) | ((DWORD)g_tintG << 8) | (DWORD)g_tintR;

            WINDOWCOMPOSITIONATTRIBDATA data{};
            data.Attrib = WCA_ACCENT_POLICY;
            data.pvData = &policy;
            data.cbData = sizeof(policy);
            g_pSetWindowCompositionAttribute(hUnderlay, &data);
        }
    }

    // Set corner preference explicitly: Windows 11 defaults WS_POPUP to square if left at default,
    // so we explicitly enforce DWMWCP_ROUND (or user preference) to guarantee smooth rounded corners.
    int corner = DWMWCP_ROUND;
    if (g_cornerStyle == CORNER_ROUND_SMALL) {
        corner = DWMWCP_ROUNDSMALL;
    } else if (g_cornerStyle == CORNER_SQUARE) {
        corner = DWMWCP_DONOTROUND;
    } else {
        corner = DWMWCP_ROUND;
    }
    DwmSetWindowAttribute(hUnderlay, (DWMWINDOWATTRIBUTE)DWMWA_WINDOW_CORNER_PREFERENCE, &corner, sizeof(corner));

    // Remove border color / shadow from underlay so it doesn't bleed out beyond window
    COLORREF noneColor = 0xFFFFFFFE; // DWMWA_COLOR_NONE
    DwmSetWindowAttribute(hUnderlay, (DWMWINDOWATTRIBUTE)DWMWA_BORDER_COLOR, &noneColor, sizeof(noneColor));

    // Position directly behind the target window in Z-order
    SetWindowPos(hUnderlay, hTarget, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOSENDCHANGING);

    // Dedicated timer on underlay window for native refresh rate animation and mouse-release watchdog
    SetTimer(hUnderlay, 1, g_timerIntervalMs, NULL);

    return hUnderlay;
}

// Creates an invisible helper window to drive the timer watchdog when in Simple Fade mode
static HWND CreateSimpleHelper(HWND hTarget, DragState* state) {
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
        SetTimer(hHelper, 1, g_timerIntervalMs, NULL);
    }

    return hHelper;
}

// ---------------------------------------------------------------------------
// Animation & Fade State Management
// ---------------------------------------------------------------------------

static void CleanupAndRestoreWindow(HWND hwnd, DragState* state) {
    if (state) {
        // 1. Destroy helper window (halts its animation watchdog timer)
        if (state->hHelperWnd) {
            SafeDestroyHelperWindow(state->hHelperWnd);
            state->hHelperWnd = NULL;
        }

        // 2. Restore target window original alpha
        if (IsWindow(hwnd)) {
            if (!state->isULW) {
                SetLayeredWindowAttributes(hwnd, 0, state->origAlpha, LWA_ALPHA);
            }

            // 3. Remove WS_EX_LAYERED if the window was not originally layered
            // (or if it's a VCL window like WizTree) to prevent crashes or drawing issues
            if (!state->hadLayered || IsVclWindow(hwnd)) {
                LONG_PTR currentStyle = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
                if (currentStyle & WS_EX_LAYERED) {
                    SetWindowLongPtrW(hwnd, GWL_EXSTYLE, currentStyle & ~WS_EX_LAYERED);
                    SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOACTIVATE);
                }
            }
        }

        UnregisterActiveWindow(hwnd);
        RemovePropW(hwnd, L"DragFadeState");
        delete state;
    }
}

static void StepFadeOut(HWND hwnd, DragState* state) {
    if (!state || state->isULW) return;

    BYTE currentAlpha = 255;
    if (!GetLayeredWindowAttributes(hwnd, NULL, &currentAlpha, NULL)) {
        currentAlpha = 255;
    }

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

    if (currentAlpha > (BYTE)g_targetOpacity) {
        int next = (int)currentAlpha - step;
        if (next <= g_targetOpacity) {
            next = g_targetOpacity;
        }
        SetLayeredWindowAttributes(hwnd, 0, (BYTE)next, LWA_ALPHA);
    }
}

static void StepFadeIn(HWND hwnd, DragState* state) {
    if (!state) return;
    if (state->isULW) {
        CleanupAndRestoreWindow(hwnd, state);
        return;
    }

    BYTE currentAlpha = 255;
    if (!GetLayeredWindowAttributes(hwnd, NULL, &currentAlpha, NULL)) {
        currentAlpha = 255;
    }

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
        SetLayeredWindowAttributes(hwnd, 0, (BYTE)next, LWA_ALPHA);
    } else {
        CleanupAndRestoreWindow(hwnd, state);
    }
}

static void StartFadeOut(HWND hwnd, DragActionType action) {
    DragState* state = (DragState*)GetPropW(hwnd, L"DragFadeState");

    if (!state) {
        state = new DragState();
        state->hTargetWnd = hwnd;
        state->action = action;
        state->lastStepTime = GetTickCount64();
        state->hHelperWnd = NULL;
        state->isFadingIn = false;
        state->startedWithMouse = ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0) ||
                                  ((GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0);

        state->origExStyle = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
        state->hadLayered = (state->origExStyle & WS_EX_LAYERED) != 0;
        state->isULW = false;

        if (state->hadLayered) {
            BYTE existingAlpha = 255;
            COLORREF crKey = 0;
            DWORD dwFlags = 0;
            if (GetLayeredWindowAttributes(hwnd, &crKey, &existingAlpha, &dwFlags)) {
                state->origAlpha = (existingAlpha > 0) ? existingAlpha : 255;
            } else {
                // Window has WS_EX_LAYERED but uses per-pixel UpdateLayeredWindow
                state->origAlpha = 255;
                state->isULW = true;
            }
        } else {
            state->origAlpha = 255;
        }

        // Accurately calculate the invisible DWM drop shadow / resize margin
        RECT rcWin{};
        RECT rcExt{};
        GetWindowRect(hwnd, &rcWin);
        if (DwmGetWindowAttribute(hwnd, (DWMWINDOWATTRIBUTE)DWMWA_EXTENDED_FRAME_BOUNDS, &rcExt, sizeof(rcExt)) == S_OK) {
            state->margin.left = rcExt.left - rcWin.left;
            state->margin.top = rcExt.top - rcWin.top;
            state->margin.right = rcWin.right - rcExt.right;
            state->margin.bottom = rcWin.bottom - rcExt.bottom;
        } else {
            state->margin = {0, 0, 0, 0};
            rcExt = rcWin;
        }

        SetPropW(hwnd, L"DragFadeState", (HANDLE)state);
        RegisterActiveWindow(hwnd);
    } else {
        state->action = action;
        state->isFadingIn = false;
    }

    RECT rcExt{};
    if (DwmGetWindowAttribute(hwnd, (DWMWINDOWATTRIBUTE)DWMWA_EXTENDED_FRAME_BOUNDS, &rcExt, sizeof(rcExt)) != S_OK) {
        GetWindowRect(hwnd, &rcExt);
    }

    // Effect Specific Initializations
    if (g_fadeEffect == EFFECT_UNDERLAY) {
        if (!state->hHelperWnd || !IsWindow(state->hHelperWnd)) {
            state->hHelperWnd = CreateUnderlayHelper(hwnd, state, rcExt);
        }
    } else {
        if (!state->hHelperWnd || !IsWindow(state->hHelperWnd)) {
            state->hHelperWnd = CreateSimpleHelper(hwnd, state);
        }
    }

    // Make sure the target window is layered to allow alpha fading (unless it is already an UpdateLayeredWindow surface)
    if (!state->isULW) {
        LONG_PTR currentStyle = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
        if (!(currentStyle & WS_EX_LAYERED)) {
            SetWindowLongPtrW(hwnd, GWL_EXSTYLE, currentStyle | WS_EX_LAYERED);
            // No SWP_FRAMECHANGED here - avoids non-client recalculation and cursor flicker!
            SetLayeredWindowAttributes(hwnd, 0, state->origAlpha, LWA_ALPHA);
        }

        // Re-timestamp after helper window creation so helper initialization doesn't count against fade step
        state->lastStepTime = GetTickCount64();
        if (g_fadeSpeed >= 255) {
            SetLayeredWindowAttributes(hwnd, 0, (BYTE)g_targetOpacity, LWA_ALPHA);
        } else {
            StepFadeOut(hwnd, state);
        }
    }
}

static void StartFadeIn(HWND hwnd, DragState* state) {
    if (!state || state->isFadingIn) return;
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

    DragState* state = (DragState*)GetPropW(hwnd, L"DragFadeState");

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

        case WM_MOVING: {
            if (!g_enableOnMove) return 0;
            if (!state) {
                StartFadeOut(hwnd, ACTION_MOVE);
                state = (DragState*)GetPropW(hwnd, L"DragFadeState");
            }
            if (state) {
                if (g_underlayMoveMode == UNDERLAY_FOLLOW && state->hHelperWnd && IsWindow(state->hHelperWnd)) {
                    RECT* r = (RECT*)lParam;
                    int x = r->left + state->margin.left;
                    int y = r->top + state->margin.top;
                    int w = (r->right - r->left) - state->margin.left - state->margin.right;
                    int h = (r->bottom - r->top) - state->margin.top - state->margin.bottom;
                    if (w > 0 && h > 0) {
                        SetWindowPos(state->hHelperWnd, hwnd, x, y, w, h,
                                     SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOSENDCHANGING);
                    }
                }
                StepFadeOut(hwnd, state);
            }
            break;
        }

        case WM_SIZING: {
            if (!g_enableOnResize) return 0;
            if (!state) {
                StartFadeOut(hwnd, ACTION_RESIZE);
                state = (DragState*)GetPropW(hwnd, L"DragFadeState");
            }
            if (state) {
                if (g_underlayResizeMode == UNDERLAY_FOLLOW && state->hHelperWnd && IsWindow(state->hHelperWnd)) {
                    RECT* r = (RECT*)lParam;
                    int x = r->left + state->margin.left;
                    int y = r->top + state->margin.top;
                    int w = (r->right - r->left) - state->margin.left - state->margin.right;
                    int h = (r->bottom - r->top) - state->margin.top - state->margin.bottom;
                    if (w > 0 && h > 0) {
                        SetWindowPos(state->hHelperWnd, hwnd, x, y, w, h,
                                     SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOSENDCHANGING);
                    }
                }
                StepFadeOut(hwnd, state);
            }
            break;
        }

        case WM_WINDOWPOSCHANGED: {
            if (state && state->hHelperWnd && IsWindow(state->hHelperWnd) && !state->isFadingIn) {
                WINDOWPOS* wp = (WINDOWPOS*)lParam;
                if (!(wp->flags & SWP_NOMOVE) || !(wp->flags & SWP_NOSIZE)) {
                    bool shouldFollow = (state->action == ACTION_MOVE && g_underlayMoveMode == UNDERLAY_FOLLOW) ||
                                        (state->action == ACTION_RESIZE && g_underlayResizeMode == UNDERLAY_FOLLOW) ||
                                        (state->action == ACTION_UNKNOWN && g_underlayMoveMode == UNDERLAY_FOLLOW);
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
                    StepFadeOut(hwnd, state);
                }
            }
            break;
        }

        case WM_LBUTTONUP:
        case WM_NCLBUTTONUP:
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
            UnregisterActiveWindow(hwnd);
            RemovePropW(hwnd, L"DragPendingAction");
            if (state) {
                if (state->hHelperWnd) {
                    SafeDestroyHelperWindow(state->hHelperWnd);
                    state->hHelperWnd = NULL;
                }
                // Clean up WS_EX_LAYERED on window close to protect Delphi/VCL apps and custom controls
                if (!state->hadLayered) {
                    LONG_PTR currentStyle = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
                    if (currentStyle & WS_EX_LAYERED) {
                        SetWindowLongPtrW(hwnd, GWL_EXSTYLE, currentStyle & ~WS_EX_LAYERED);
                    }
                }
                RemovePropW(hwnd, L"DragFadeState");
                delete state;
            }
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
    if (!g_unloading.load(std::memory_order_relaxed)) {
        HandleDragMessages(hwnd, uMsg, wParam, lParam);
    }
    return DefWindowProcW_Orig(hwnd, uMsg, wParam, lParam);
}

static LRESULT WINAPI DefWindowProcA_Hook(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
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
    g_hInstance = GetModuleHandleW(NULL);
    LoadSettings();

    // Dynamically resolve SetWindowCompositionAttribute
    HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
    if (hUser32) {
        g_pSetWindowCompositionAttribute = (pfnSetWindowCompositionAttribute)GetProcAddress(hUser32, "SetWindowCompositionAttribute");
    }

    // Enable 1ms timer precision
    timeBeginPeriod(1);

    Wh_SetFunctionHook((void*)GetProcAddress(hUser32, "DefWindowProcW"), (void*)DefWindowProcW_Hook, (void**)&DefWindowProcW_Orig);
    Wh_SetFunctionHook((void*)GetProcAddress(hUser32, "DefWindowProcA"), (void*)DefWindowProcA_Hook, (void**)&DefWindowProcA_Orig);
    return TRUE;
}

void Wh_ModUninit() {
    // 1. Immediately set unloading flag so all threads bypass hook message processing
    g_unloading.store(true);
    timeEndPeriod(1);

    // 2. Clean up all active drag states and restore target windows
    {
        std::lock_guard<std::mutex> lock(g_activeMutex);
        for (HWND hwnd : g_activeWindows) {
            if (IsWindow(hwnd)) {
                DragState* state = (DragState*)GetPropW(hwnd, L"DragFadeState");
                if (state) {
                    if (!state->isULW) {
                        SetLayeredWindowAttributes(hwnd, 0, state->origAlpha, LWA_ALPHA);
                    }

                    if (state->hHelperWnd) {
                        SafeDestroyHelperWindow(state->hHelperWnd);
                        state->hHelperWnd = NULL;
                    }

                    if (!state->hadLayered) {
                        LONG_PTR currentStyle = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
                        if (currentStyle & WS_EX_LAYERED) {
                            SetWindowLongPtrW(hwnd, GWL_EXSTYLE, currentStyle & ~WS_EX_LAYERED);
                            SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOACTIVATE);
                        }
                    }

                    RemovePropW(hwnd, L"DragFadeState");
                    delete state;
                }

                RemovePropW(hwnd, L"DragPendingAction");
            }
        }
        g_activeWindows.clear();
    }

    UnregisterClassW(L"DragFadeUnderlayClass", g_hInstance);
}

void Wh_ModSettingsChanged() {
    LoadSettings();
}
