// ==WindhawkMod==
// @id              drag-fade
// @name            Drag Fade
// @description     Fades window when dragging. Includes blur options and fixes for modern apps.
// @version         0.0.5
// @author          Gemini
// @include         *
// @compilerOptions -ldwmapi -luser32
// ==/WindhawkMod==

// ==WindhawkModSettings==
/*
- targetOpacity: 150
  $name: Drag Opacity (0-255)
- fadeSpeed: 20
  $name: Fade Speed (Higher = Faster)
- blurMode: 0
  $name: Blur Mode
  $description: 0=None (Let other mods handle it), 1=Native Acrylic, 2=Native Mica, 3=Legacy Acrylic
- tintColor: "1A1A1A"
  $name: Tint Color (Hex RRGGBB)
- tintOpacity: 120
  $name: Tint Strength (0-255)
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <dwmapi.h>
#include <windhawk_api.h>

struct ACCENT_POLICY { int State; int Flags; int Color; int AnimationId; };
struct WINDOWCOMPOSITIONATTRIBDATA { int Attrib; PVOID pvData; int cbData; };
typedef BOOL (WINAPI *SetWindowCompositionAttribute_t)(HWND, WINDOWCOMPOSITIONATTRIBDATA*);

#ifndef DWMWA_SYSTEMBACKDROP_TYPE
#define DWMWA_SYSTEMBACKDROP_TYPE 38
#endif

#define MOD_MIN(a,b) (((a)<(b))?(a):(b))
#define MOD_MAX(a,b) (((a)>(b))?(a):(b))

int g_targetOpacity, g_fadeSpeed, g_blurMode, g_tintColorInt, g_tintOpacity;

struct DragState {
    LONG_PTR origExStyle;
    int origBackdrop;
    BYTE origAlpha;
    bool hadLayered;
};

int HexStringToABGR(PCWSTR hexStr, int opacity) {
    if (!hexStr) return (opacity << 24);
    unsigned long color = wcstoul(hexStr, NULL, 16);
    int r = (color >> 16) & 0xFF;
    int g = (color >> 8) & 0xFF;
    int b = color & 0xFF;
    return (opacity << 24) | (b << 16) | (g << 8) | r;
}

void LoadSettings() {
    g_targetOpacity = Wh_GetIntSetting(L"targetOpacity");
    g_fadeSpeed     = Wh_GetIntSetting(L"fadeSpeed");
    g_blurMode      = Wh_GetIntSetting(L"blurMode");
    g_tintOpacity   = Wh_GetIntSetting(L"tintOpacity");

    PCWSTR hexColor = Wh_GetStringSetting(L"tintColor");
    g_tintColorInt = HexStringToABGR(hexColor, g_tintOpacity);
    Wh_FreeStringSetting(hexColor); 
}

void CALLBACK FadeTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
    BYTE alpha = 255;
    bool getAttrSuccess = GetLayeredWindowAttributes(hwnd, NULL, &alpha, NULL);
    
    DragState* state = (DragState*)GetPropW(hwnd, L"DragFadeState");
    
    int target = (idEvent == 101) ? g_targetOpacity : 255;
    if (idEvent == 102 && state && state->hadLayered) {
        target = state->origAlpha; 
    }
    
    // If we are fading in (102) and GetLayeredWindowAttributes fails, 
    // it likely means the style was already removed or the window is opaque.
    if (!getAttrSuccess && idEvent == 102) {
        alpha = (BYTE)target;
    }

    if (alpha == (BYTE)target) {
        KillTimer(hwnd, idEvent);
        
        if (idEvent == 102) { // Fade in complete
            SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
            
            if (state) {
                // Remove WS_EX_LAYERED if it wasn't there originally
                if (!state->hadLayered) {
                    LONG_PTR currentStyle = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
                    SetWindowLongPtrW(hwnd, GWL_EXSTYLE, currentStyle & ~WS_EX_LAYERED);
                }
                
                // CRITICAL FIX: Force DWM to re-evaluate the backdrop.
                // We toggle it to NONE then back to ORIGINAL to "kick" the DWM into re-applying Mica/Acrylic.
                int none = 1; // DWMSBT_NONE
                DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &none, sizeof(int));
                DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &state->origBackdrop, sizeof(int));
                
                if (g_blurMode == 3) {
                    static auto pSetWindowCompositionAttribute = (SetWindowCompositionAttribute_t)GetProcAddress(GetModuleHandle(L"user32.dll"), "SetWindowCompositionAttribute");
                    if (pSetWindowCompositionAttribute) {
                        ACCENT_POLICY accent = { 0, 0, 0, 0 }; 
                        WINDOWCOMPOSITIONATTRIBDATA data = { 19, &accent, sizeof(accent) };
                        pSetWindowCompositionAttribute(hwnd, &data);
                    }
                }

                RemovePropW(hwnd, L"DragFadeState");
                delete state;
            }
            
            // Aggressive redraw to fix any lingering XAML/Mica glitches
            SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_ASYNCWINDOWPOS);
            RedrawWindow(hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN | RDW_UPDATENOW);
        }
        return;
    }

    int next = (target < alpha) ? MOD_MAX(target, (int)alpha - g_fadeSpeed) : MOD_MIN(target, (int)alpha + g_fadeSpeed);
    
    if (next != alpha) {
        SetLayeredWindowAttributes(hwnd, 0, (BYTE)next, LWA_ALPHA);
    }
}

typedef LRESULT (WINAPI *DefWindowProcW_t)(HWND, UINT, WPARAM, LPARAM);
DefWindowProcW_t DefWindowProcW_Orig;

typedef LRESULT (WINAPI *DefFrameProcW_t)(HWND, HWND, UINT, WPARAM, LPARAM);
DefFrameProcW_t DefFrameProcW_Orig;

typedef LRESULT (WINAPI *DefMDIChildProcW_t)(HWND, UINT, WPARAM, LPARAM);
DefMDIChildProcW_t DefMDIChildProcW_Orig;

#ifndef DWMWA_MICA_EFFECT
#define DWMWA_MICA_EFFECT 1029
#endif

LRESULT HandleDragMessages(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    bool isStartMsg = (uMsg == WM_ENTERSIZEMOVE);
    
    // Catch Win32 apps that might bypass WM_ENTERSIZEMOVE by hooking the system command directly
    if (uMsg == WM_SYSCOMMAND) {
        UINT cmd = (UINT)(wParam & 0xFFF0);
        if (cmd == SC_MOVE || cmd == SC_SIZE) {
            isStartMsg = true;
        }
    }
    
    // Raw mouse trigger for stubborn Win32 apps
    if (uMsg == WM_NCLBUTTONDOWN && wParam == HTCAPTION) {
        isStartMsg = true;
    }

    if (isStartMsg) {
        DragState* state = (DragState*)GetPropW(hwnd, L"DragFadeState");
        if (!state) {
            state = new DragState();
            state->origExStyle = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
            state->hadLayered = (state->origExStyle & WS_EX_LAYERED) != 0;
            
            if (state->hadLayered) {
                if (!GetLayeredWindowAttributes(hwnd, NULL, &state->origAlpha, NULL)) {
                    state->origAlpha = 255;
                }
            } else {
                state->origAlpha = 255;
            }

            state->origBackdrop = 0; 
            DwmGetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &state->origBackdrop, sizeof(int));

            SetPropW(hwnd, L"DragFadeState", (HANDLE)state);
        }

        KillTimer(hwnd, 102);
        
        // Apply Blur/Backdrop BEFORE setting layered style
        if (g_blurMode > 0) {
            if (g_blurMode == 1 || g_blurMode == 2) {
                int backdrop = (g_blurMode == 1) ? 3 : 2; // 3=Acrylic, 2=Mica
                DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &backdrop, sizeof(backdrop));
                
                // Fallback for older Win11 versions
                if (g_blurMode == 2) {
                    BOOL enable = TRUE;
                    DwmSetWindowAttribute(hwnd, DWMWA_MICA_EFFECT, &enable, sizeof(BOOL));
                }
            } 
            else if (g_blurMode == 3) {
                // For Legacy Acrylic, we disable the DWM backdrop first to avoid conflicts
                int disableBackdrop = 1; // DWMSBT_NONE
                DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &disableBackdrop, sizeof(disableBackdrop));

                static auto pSetWindowCompositionAttribute = (SetWindowCompositionAttribute_t)GetProcAddress(GetModuleHandle(L"user32.dll"), "SetWindowCompositionAttribute");
                if (pSetWindowCompositionAttribute) {
                    // State 4 = ACCENT_ENABLE_ACRYLICBLURBEHIND
                    // Flags 2 = Draw all borders
                    ACCENT_POLICY accent = { 4, 2, g_tintColorInt, 0 };
                    WINDOWCOMPOSITIONATTRIBDATA data = { 19, &accent, sizeof(accent) };
                    pSetWindowCompositionAttribute(hwnd, &data);
                }
            }
        }

        // Ensure WS_EX_LAYERED is set for the fade effect
        if (!(state->origExStyle & WS_EX_LAYERED)) {
            SetWindowLongPtrW(hwnd, GWL_EXSTYLE, state->origExStyle | WS_EX_LAYERED);
        }

        SetTimer(hwnd, 101, 16, (TIMERPROC)FadeTimerProc);
    } 
    else if (uMsg == WM_EXITSIZEMOVE || (uMsg == WM_CAPTURECHANGED && GetPropW(hwnd, L"DragFadeState"))) {
        // If we lose capture (e.g. MMC closing a dialog), we must ensure we fade back
        KillTimer(hwnd, 101);
        if (g_fadeSpeed >= 255) {
            SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
            FadeTimerProc(hwnd, WM_TIMER, 102, 0);
        } else {
            SetTimer(hwnd, 102, 16, (TIMERPROC)FadeTimerProc);
        }
    }
    else if (uMsg == WM_DESTROY) {
        DragState* state = (DragState*)GetPropW(hwnd, L"DragFadeState");
        if (state) {
            RemovePropW(hwnd, L"DragFadeState");
            delete state;
        }
    }
    return 0;
}

LRESULT WINAPI DefWindowProcW_Hook(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    HandleDragMessages(hwnd, uMsg, wParam, lParam);
    return DefWindowProcW_Orig(hwnd, uMsg, wParam, lParam);
}

LRESULT WINAPI DefFrameProcW_Hook(HWND hwnd, HWND hwndMDI, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    HandleDragMessages(hwnd, uMsg, wParam, lParam);
    return DefFrameProcW_Orig(hwnd, hwndMDI, uMsg, wParam, lParam);
}

LRESULT WINAPI DefMDIChildProcW_Hook(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    HandleDragMessages(hwnd, uMsg, wParam, lParam);
    return DefMDIChildProcW_Orig(hwnd, uMsg, wParam, lParam);
}

BOOL Wh_ModInit() {
    LoadSettings();
    HMODULE hUser32 = GetModuleHandle(L"user32.dll");
    Wh_SetFunctionHook((void*)GetProcAddress(hUser32, "DefWindowProcW"), (void*)DefWindowProcW_Hook, (void**)&DefWindowProcW_Orig);
    Wh_SetFunctionHook((void*)GetProcAddress(hUser32, "DefFrameProcW"), (void*)DefFrameProcW_Hook, (void**)&DefFrameProcW_Orig);
    Wh_SetFunctionHook((void*)GetProcAddress(hUser32, "DefMDIChildProcW"), (void*)DefMDIChildProcW_Hook, (void**)&DefMDIChildProcW_Orig);
    return TRUE;
}

void Wh_ModUninit() {
}

void Wh_ModSettingsChanged() {
    LoadSettings();
}
