// ==WindhawkMod==
// @id              drag-fade
// @name            Drag Fade
// @description     fades window when dragging. messes up file explorer
// @version         0.0.1
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
- backdropType: 3
  $name: Windows 11 Backdrop (1=Auto, 2=Mica, 3=Acrylic, 4=Tabbed)
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

// Re-defining min/max macros to avoid header conflicts
#define MOD_MIN(a,b) (((a)<(b))?(a):(b))
#define MOD_MAX(a,b) (((a)>(b))?(a):(b))

int g_targetOpacity, g_fadeSpeed, g_backdropType, g_tintColorInt, g_tintOpacity;

int HexStringToABGR(PCWSTR hexStr, int opacity) {
    if (!hexStr) return (opacity << 24);
    unsigned long color = wcstoul(hexStr, NULL, 16);
    int r = (color >> 16) & 0xFF;
    int g = (color >> 8) & 0xFF;
    int b = color & 0xFF;
    // Format: 0xAABBGGRR
    return (opacity << 24) | (b << 16) | (g << 8) | r;
}

void LoadSettings() {
    g_targetOpacity = Wh_GetIntSetting(L"targetOpacity");
    g_fadeSpeed     = Wh_GetIntSetting(L"fadeSpeed");
    g_backdropType  = Wh_GetIntSetting(L"backdropType");
    g_tintOpacity   = Wh_GetIntSetting(L"tintOpacity");

    PCWSTR hexColor = Wh_GetStringSetting(L"tintColor");
    g_tintColorInt = HexStringToABGR(hexColor, g_tintOpacity);
    Wh_FreeStringSetting(hexColor); 
}

void ForceComposition(HWND hwnd, bool enabled) {
    static auto pSetWindowCompositionAttribute = (SetWindowCompositionAttribute_t)GetProcAddress(GetModuleHandle(L"user32.dll"), "SetWindowCompositionAttribute");
    
    if (enabled) {
        // Windows 11 Backdrop
        DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &g_backdropType, sizeof(g_backdropType));

        if (pSetWindowCompositionAttribute) {
            // State 4 = Acrylic, Flag 2 = Enable Color/Alpha
            ACCENT_POLICY accent = { 4, 2, g_tintColorInt, 0 };
            WINDOWCOMPOSITIONATTRIBDATA data = { 19, &accent, sizeof(accent) };
            pSetWindowCompositionAttribute(hwnd, &data);
        }
    } else {
        int disable = 0;
        DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &disable, sizeof(disable));
        if (pSetWindowCompositionAttribute) {
            ACCENT_POLICY accent = { 0, 0, 0, 0 };
            WINDOWCOMPOSITIONATTRIBDATA data = { 19, &accent, sizeof(accent) };
            pSetWindowCompositionAttribute(hwnd, &data);
        }
    }
}

void CALLBACK FadeTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
    BYTE alpha;
    if (!GetLayeredWindowAttributes(hwnd, NULL, &alpha, NULL)) alpha = 255;
    
    int target = (idEvent == 101) ? g_targetOpacity : 255;
    
    if (alpha == (BYTE)target) {
        KillTimer(hwnd, idEvent);
        if (target == 255) {
            ForceComposition(hwnd, false);
            SetWindowPos(hwnd, NULL, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_FRAMECHANGED);
        }
        return;
    }

    int next = (target < alpha) ? MOD_MAX(target, (int)alpha - g_fadeSpeed) : MOD_MIN(target, (int)alpha + g_fadeSpeed);
    SetLayeredWindowAttributes(hwnd, 0, (BYTE)next, LWA_ALPHA);
}

typedef LRESULT (WINAPI *DefWindowProcW_t)(HWND, UINT, WPARAM, LPARAM);
DefWindowProcW_t DefWindowProcW_Orig;

LRESULT WINAPI DefWindowProcW_Hook(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_ENTERSIZEMOVE) {
        KillTimer(hwnd, 102);
        
        LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
        if (!(exStyle & WS_EX_LAYERED)) SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);
        
        ForceComposition(hwnd, true);
        SetTimer(hwnd, 101, 10, (TIMERPROC)FadeTimerProc);
    } 
    else if (uMsg == WM_EXITSIZEMOVE) {
        KillTimer(hwnd, 101);
        SetTimer(hwnd, 102, 10, (TIMERPROC)FadeTimerProc);
    }
    return DefWindowProcW_Orig(hwnd, uMsg, wParam, lParam);
}

BOOL Wh_ModInit() {
    LoadSettings();
    Wh_SetFunctionHook((void*)GetProcAddress(GetModuleHandle(L"user32.dll"), "DefWindowProcW"), 
                       (void*)DefWindowProcW_Hook, 
                       (void**)&DefWindowProcW_Orig);
    return TRUE;
}

void Wh_ModSettingsChanged() {
    LoadSettings();
}
