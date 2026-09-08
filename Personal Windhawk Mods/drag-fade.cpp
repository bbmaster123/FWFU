// ==WindhawkMod==
// @id              drag-fade
// @name            Drag Fade
// @description     Fades window with live Acrylic, Mica, or Mica Alt during drag and resize
// @version         0.3.4
// @author          Gemini & bbmaster123
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
- blurAmount: heavy
  $name: Blur Amount / Radius
  $description: "Blur intensity for the underlay backdrop (applies to Acrylic mode)."
  $options:
    - heavy: "Heavy Blur (~30px, Fluent Acrylic)"
    - medium: "Medium Blur (~12px, Aero Glass)"
    - none: "No Blur (0px, Pure Clear Glass)"
- targetOpacity: 160
  $name: Drag Opacity (0-255)
  $description: "Opacity of the window content while dragging (0 = transparent, 255 = opaque)."
- underlayOpacity: 255
  $name: Underlay Backdrop Opacity (0-255)
  $description: "Master opacity scale for the underlay backdrop tint (0 = transparent tint, 255 = full tint opacity)."
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
- removeLayeredOnRelease: false
  $name: Remove Layered Style on Drag Release
  $description: "If disabled (recommended), keeps the window layered at 255 alpha to prevent white flash/flicker on Win32 apps. If enabled, removes WS_EX_LAYERED when dragging ends."
- enableOnMove: true
  $name: Enable on Window Move
  $description: "Fade window when dragging or moving by the titlebar."
- enableOnResize: true
  $name: Enable on Window Resize
  $description: "Fade window when resizing window borders or corners."
- targetFps: 0
  $name: Refresh Rate / Target FPS
  $description: "0 = Auto (~120 Hz). Set 60 for 60Hz, 120 for 120Hz, 144 for 144Hz, 240 for 240Hz."
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

enum BlurAmountMode {
    BLUR_HEAVY  = 0, // Fluent Acrylic (~30px)
    BLUR_MEDIUM = 1, // Aero Glass (~12px)
    BLUR_NONE   = 2  // Clear Glass (0px)
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

// Global Configuration
static FadeEffectMode  g_fadeEffect          = EFFECT_UNDERLAY;
static BackdropType    g_backdropType        = BACKDROP_ACRYLIC;
static BlurAmountMode  g_blurAmount          = BLUR_HEAVY;
static CornerStyleMode g_cornerStyle         = CORNER_ROUND;
static int             g_targetOpacity       = 160;
static int             g_underlayOpacity     = 255;
static BYTE            g_tintR               = 32;
static BYTE            g_tintG               = 32;
static BYTE            g_tintB               = 32;
static BYTE            g_tintOpacity         = 60;
static bool            g_acrylicLuminance    = false;
static int             g_fadeSpeed           = 20;
static bool            g_removeLayeredOnRel  = false;
static bool            g_enableOnMove        = true;
static bool            g_enableOnResize      = true;
static int             g_targetFps           = 0;
static int             g_timerIntervalMs     = 8;
static int             g_minWindowWidth      = 120;
static int             g_minWindowHeight     = 80;

static HINSTANCE       g_hInstance           = nullptr;

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

    PCWSTR blurStr = Wh_GetStringSetting(L"blurAmount");
    if (blurStr) {
        if (wcscmp(blurStr, L"medium") == 0) {
            g_blurAmount = BLUR_MEDIUM;
        } else if (wcscmp(blurStr, L"none") == 0) {
            g_blurAmount = BLUR_NONE;
        } else {
            g_blurAmount = BLUR_HEAVY;
        }
        Wh_FreeStringSetting(blurStr);
    } else {
        g_blurAmount = BLUR_HEAVY;
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

    g_targetOpacity      = std::clamp(Wh_GetIntSetting(L"targetOpacity"), 0, 255);
    g_underlayOpacity    = std::clamp(Wh_GetIntSetting(L"underlayOpacity"), 0, 255);
    g_tintOpacity        = (BYTE)std::clamp(Wh_GetIntSetting(L"tintOpacity"), 0, 255);
    g_acrylicLuminance   = Wh_GetIntSetting(L"acrylicLuminance") != 0;
    g_fadeSpeed          = std::max(1, Wh_GetIntSetting(L"fadeSpeed"));
    g_removeLayeredOnRel = Wh_GetIntSetting(L"removeLayeredOnRelease") != 0;
    g_enableOnMove       = Wh_GetIntSetting(L"enableOnMove") != 0;
    g_enableOnResize     = Wh_GetIntSetting(L"enableOnResize") != 0;
    g_targetFps          = Wh_GetIntSetting(L"targetFps");
    g_minWindowWidth     = std::max(20, Wh_GetIntSetting(L"minWindowWidth"));
    g_minWindowHeight    = std::max(20, Wh_GetIntSetting(L"minWindowHeight"));

    PCWSTR hexColor = Wh_GetStringSetting(L"tintColorHex");
    ParseHexColor(hexColor, g_tintR, g_tintG, g_tintB);
    if (hexColor) Wh_FreeStringSetting(hexColor);

    if (g_targetFps <= 0) {
        g_timerIntervalMs = 8; // Auto (~120 FPS)
    } else {
        g_timerIntervalMs = std::clamp(1000 / g_targetFps, 1, 50);
    }
}

// Modern app check
static bool IsModernApp(HWND hwnd) {
    WCHAR className[256];
    if (GetClassNameW(hwnd, className, 256)) {
        if (wcsstr(className, L"WinUIDesktop") != NULL ||
            wcsstr(className, L"TaskManager") != NULL ||
            wcsstr(className, L"ApplicationFrameWindow") != NULL ||
            wcsstr(className, L"Windows.UI.Core.CoreWindow") != NULL ||
            wcsstr(className, L"CASCADIA_HOSTING_WINDOW_CLASS") != NULL) {
            return true;
        }
    }
    return false;
}

static bool IsValidTargetWindow(HWND hwnd) {
    if (!IsWindow(hwnd)) return false;

    LONG_PTR style = GetWindowLongPtrW(hwnd, GWL_STYLE);
    if (style & WS_CHILD) return false;
    if (GetAncestor(hwnd, GA_ROOT) != hwnd) return false;

    RECT rc;
    if (GetWindowRect(hwnd, &rc)) {
        int width = rc.right - rc.left;
        int height = rc.bottom - rc.top;
        if (width < g_minWindowWidth || height < g_minWindowHeight) {
            return false;
        }
    }
    return true;
}

// ---------------------------------------------------------------------------
// Native DWM Backdrop Underlay Window Procedure & Construction
// ---------------------------------------------------------------------------

static LRESULT CALLBACK UnderlayWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_ERASEBKGND) {
        HDC hdc = (HDC)wp;
        RECT rc;
        GetClientRect(hwnd, &rc);
        FillRect(hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
        return 1;
    }
    if (msg == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc;
        GetClientRect(hwnd, &rc);
        FillRect(hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
        EndPaint(hwnd, &ps);
        return 0;
    }
    if (msg == WM_TIMER && wp == 1) {
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
    wcU.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
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
    } else {
        // For Acrylic / Blur / Transparent Glass:
        // Set DWMSBT_NONE so Windows 11 system backdrop does not override ACCENT_POLICY
        int dwmBackdrop = DWMSBT_NONE;
        DwmSetWindowAttribute(hUnderlay, (DWMWINDOWATTRIBUTE)DWMWA_SYSTEMBACKDROP_TYPE, &dwmBackdrop, sizeof(dwmBackdrop));

        if (g_pSetWindowCompositionAttribute) {
            ACCENT_POLICY policy{};
            if (g_blurAmount == BLUR_MEDIUM) {
                policy.AccentState = ACCENT_ENABLE_BLURBEHIND; // Aero Glass light/medium blur (~12px)
                policy.AccentFlags = 0;
            } else if (g_blurAmount == BLUR_NONE) {
                policy.AccentState = ACCENT_ENABLE_TRANSPARENTGRADIENT; // Clear transparent glass (0px)
                policy.AccentFlags = 0;
            } else {
                policy.AccentState = ACCENT_ENABLE_ACRYLICBLURBEHIND; // Fluent Acrylic heavy blur (~30px)
                policy.AccentFlags = g_acrylicLuminance ? 2 : 0;
            }

            // Calculate final tint alpha by scaling tintOpacity with underlayOpacity
            BYTE finalAlpha = (BYTE)std::clamp(((int)g_tintOpacity * (int)g_underlayOpacity) / 255, 0, 255);
            policy.GradientColor = ((DWORD)finalAlpha << 24) | ((DWORD)g_tintB << 16) | ((DWORD)g_tintG << 8) | (DWORD)g_tintR;

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
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);

    // Dedicated timer on underlay window for rock-solid 120Hz+ animation and mouse-release watchdog
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
        // 1. Destroy helper window (which automatically halts its animation watchdog timer)
        if (state->hHelperWnd && IsWindow(state->hHelperWnd)) {
            KillTimer(state->hHelperWnd, 1);
            DestroyWindow(state->hHelperWnd);
            state->hHelperWnd = NULL;
        }

        // 2. Restore target window original alpha first
        SetLayeredWindowAttributes(hwnd, 0, state->origAlpha, LWA_ALPHA);

        // 3. Remove WS_EX_LAYERED only if explicitly enabled in settings AND window did not have it originally.
        // Keeping WS_EX_LAYERED with alpha 255 avoids DWM surface recreation and completely prevents Win32 white flashes.
        if (g_removeLayeredOnRel && !state->hadLayered && !IsModernApp(hwnd)) {
            LONG_PTR currentStyle = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
            if (currentStyle & WS_EX_LAYERED) {
                SetWindowLongPtrW(hwnd, GWL_EXSTYLE, currentStyle & ~WS_EX_LAYERED);
                SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOACTIVATE);
            }
        }

        UnregisterActiveWindow(hwnd);
        RemovePropW(hwnd, L"DragFadeState");
        delete state;
    }
}

static void StepFadeOut(HWND hwnd, DragState* state) {
    if (!state) return;

    BYTE currentAlpha = 255;
    if (!GetLayeredWindowAttributes(hwnd, NULL, &currentAlpha, NULL)) {
        currentAlpha = 255;
    }

    ULONGLONG now = GetTickCount64();
    DWORD elapsed = (DWORD)(now - state->lastStepTime);
    if (elapsed < 1) elapsed = 1;
    state->lastStepTime = now;

    int step = (int)std::round((double)g_fadeSpeed * ((double)elapsed / 16.0));
    if (step < 1) step = 1;

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

    BYTE currentAlpha = 255;
    if (!GetLayeredWindowAttributes(hwnd, NULL, &currentAlpha, NULL)) {
        currentAlpha = 255;
    }

    ULONGLONG now = GetTickCount64();
    DWORD elapsed = (DWORD)(now - state->lastStepTime);
    if (elapsed < 1) elapsed = 1;
    state->lastStepTime = now;

    int step = (int)std::round((double)g_fadeSpeed * ((double)elapsed / 16.0));
    if (step < 1) step = 1;

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

        if (state->hadLayered) {
            BYTE existingAlpha = 255;
            if (GetLayeredWindowAttributes(hwnd, NULL, &existingAlpha, NULL) && existingAlpha > 0) {
                state->origAlpha = existingAlpha;
            } else {
                state->origAlpha = 255;
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

    // Make sure the target window is layered to allow alpha fading
    LONG_PTR currentStyle = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    if (!(currentStyle & WS_EX_LAYERED)) {
        SetWindowLongPtrW(hwnd, GWL_EXSTYLE, currentStyle | WS_EX_LAYERED);
        SetLayeredWindowAttributes(hwnd, 0, state->origAlpha, LWA_ALPHA);
    }

    state->lastStepTime = GetTickCount64();
    if (g_fadeSpeed >= 255) {
        SetLayeredWindowAttributes(hwnd, 0, (BYTE)g_targetOpacity, LWA_ALPHA);
    } else {
        StepFadeOut(hwnd, state);
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
// Window Message Interception
// ---------------------------------------------------------------------------

static LRESULT HandleDragMessages(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_SYSCOMMAND:
        case WM_ENTERSIZEMOVE:
        case WM_MOVING:
        case WM_SIZING:
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

    // Fast reject our own helper window
    WCHAR cls[64];
    if (GetClassNameW(hwnd, cls, 64) && wcscmp(cls, L"DragFadeUnderlayClass") == 0) {
        return 0;
    }

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
            } else if (cmd == SC_SIZE) {
                SetPropW(hwnd, L"DragPendingAction", (HANDLE)(INT_PTR)ACTION_RESIZE);
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
            } else {
                if (state->hHelperWnd && IsWindow(state->hHelperWnd)) {
                    RECT* r = (RECT*)lParam;
                    // Apply frame margin offset to perfectly align with visible window borders
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
            } else {
                if (state->hHelperWnd && IsWindow(state->hHelperWnd)) {
                    RECT* r = (RECT*)lParam;
                    // Apply frame margin offset to perfectly align with visible window borders
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

        case WM_LBUTTONUP:
        case WM_NCLBUTTONUP: {
            RemovePropW(hwnd, L"DragPendingAction");
            if (state) {
                StartFadeIn(hwnd, state);
            }
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
            UnregisterActiveWindow(hwnd);
            RemovePropW(hwnd, L"DragPendingAction");
            if (state) {
                if (state->hHelperWnd && IsWindow(state->hHelperWnd)) {
                    KillTimer(state->hHelperWnd, 1);
                    SetWindowLongPtrW(state->hHelperWnd, GWLP_USERDATA, 0);
                    DestroyWindow(state->hHelperWnd);
                    state->hHelperWnd = NULL;
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
static DefWindowProcW_t DefWindowProcW_Orig;

typedef LRESULT (WINAPI *DefWindowProcA_t)(HWND, UINT, WPARAM, LPARAM);
static DefWindowProcA_t DefWindowProcA_Orig;

static LRESULT WINAPI DefWindowProcW_Hook(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    HandleDragMessages(hwnd, uMsg, wParam, lParam);
    return DefWindowProcW_Orig(hwnd, uMsg, wParam, lParam);
}

static LRESULT WINAPI DefWindowProcA_Hook(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    HandleDragMessages(hwnd, uMsg, wParam, lParam);
    return DefWindowProcA_Orig(hwnd, uMsg, wParam, lParam);
}

// ---------------------------------------------------------------------------
// Mod Lifecycle Entry Points
// ---------------------------------------------------------------------------

BOOL Wh_ModInit() {
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
    timeEndPeriod(1);

    std::lock_guard<std::mutex> lock(g_activeMutex);
    for (HWND hwnd : g_activeWindows) {
        if (IsWindow(hwnd)) {
            DragState* state = (DragState*)GetPropW(hwnd, L"DragFadeState");
            if (state) {
                SetLayeredWindowAttributes(hwnd, 0, state->origAlpha, LWA_ALPHA);

                if (state->hHelperWnd && IsWindow(state->hHelperWnd)) {
                    KillTimer(state->hHelperWnd, 1);
                    SetWindowLongPtrW(state->hHelperWnd, GWLP_USERDATA, 0);
                    DestroyWindow(state->hHelperWnd);
                    state->hHelperWnd = NULL;
                }

                if (!state->hadLayered && !IsModernApp(hwnd)) {
                    LONG_PTR currentStyle = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
                    SetWindowLongPtrW(hwnd, GWL_EXSTYLE, currentStyle & ~WS_EX_LAYERED);
                }

                RemovePropW(hwnd, L"DragFadeState");
                delete state;
            }

            RemovePropW(hwnd, L"DragPendingAction");
            SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOACTIVATE);
        }
    }
    g_activeWindows.clear();

    UnregisterClassW(L"DragFadeUnderlayClass", g_hInstance);
}

void Wh_ModSettingsChanged() {
    LoadSettings();
}
