// ==WindhawkMod==
// @id              confirmed-copy
// @name            confirmed-copy
// @description     Tri-color copy success indicator. shows something has been copied to the windows clipboard.
// @version         1.0.0
// @author          bbmaster123/Gemini
// @include         *
// @compilerOptions -luser32 -lgdi32 -lmsimg32 -lwinmm
// ==/WindhawkMod==

// ==WindhawkModSettings==
/*
- enableVisual: true
  $name: Enable Animation
- color1: "00FF00"
  $name: Outer Glow Color (Hex)
- color2: "00FFFF"
  $name: Mid-Tone Color (Hex)
- color3: "FFFFFF"
  $name: Inner Core Color (Hex)
- packetSize: 24
  $name: Base Size (Pixels)
- flySpeed: 8
  $name: Flight Smoothness (5-30)
- enableSound: true
  $name: Enable Sound
- customSoundPath: ""
  $name: Custom WAV Path
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <windhawk_api.h>
#include <mmsystem.h>
#include <math.h>

#define MOD_MIN(a,b) (((a)<(b))?(a):(b))
#define MOD_MAX(a,b) (((a)>(b))?(a):(b))

COLORREF g_c1, g_c2, g_c3;
int g_flySpeed, g_packetSize;
bool g_enableVisual, g_enableSound;
WCHAR g_soundPath[MAX_PATH];

struct PacketAnim {
    float curX, curY;
    float targetX, targetY;
    float phase; 
    int alpha;
};

COLORREF HexToColor(PCWSTR hexStr, COLORREF defaultCol) {
    if (!hexStr || !*hexStr) return defaultCol;
    unsigned long color = wcstoul(hexStr, NULL, 16);
    return RGB((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
}

void LoadSettings() {
    g_enableVisual = Wh_GetIntSetting(L"enableVisual");
    g_enableSound = Wh_GetIntSetting(L"enableSound");
    g_flySpeed = Wh_GetIntSetting(L"flySpeed");
    g_packetSize = Wh_GetIntSetting(L"packetSize");
    
    PCWSTR hex1 = Wh_GetStringSetting(L"color1");
    g_c1 = HexToColor(hex1, RGB(0, 255, 0));
    Wh_FreeStringSetting(hex1);

    PCWSTR hex2 = Wh_GetStringSetting(L"color2");
    g_c2 = HexToColor(hex2, RGB(0, 255, 255));
    Wh_FreeStringSetting(hex2);

    PCWSTR hex3 = Wh_GetStringSetting(L"color3");
    g_c3 = HexToColor(hex3, RGB(255, 255, 255));
    Wh_FreeStringSetting(hex3);

    PCWSTR path = Wh_GetStringSetting(L"customSoundPath");
    wcsncpy(g_soundPath, path ? path : L"", MAX_PATH);
    Wh_FreeStringSetting(path);
}

LRESULT CALLBACK PacketWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        
        RECT client;
        GetClientRect(hwnd, &client);
        int w = client.right;
        int h = client.bottom;

        HPEN nullPen = CreatePen(PS_NULL, 0, 0);
        SelectObject(hdc, nullPen);

        // 1. Layer: Outer Glow (Color 1)
        HBRUSH b1 = CreateSolidBrush(g_c1);
        SelectObject(hdc, b1);
        Ellipse(hdc, 0, 0, w, h);
        DeleteObject(b1);

        // 2. Layer: Mid-Tone (Color 2)
        HBRUSH b2 = CreateSolidBrush(g_c2);
        SelectObject(hdc, b2);
        int m2 = w / 6;
        Ellipse(hdc, m2, m2, w - m2, h - m2);
        DeleteObject(b2);

        // 3. Layer: Core Hotspot (Color 3)
        HBRUSH b3 = CreateSolidBrush(g_c3);
        SelectObject(hdc, b3);
        int m3 = w / 2.5;
        Ellipse(hdc, m3, m3, w - m3, h - m3);
        DeleteObject(b3);
        
        DeleteObject(nullPen);
        EndPaint(hwnd, &ps);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void CALLBACK PacketTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
    PacketAnim* anim = (PacketAnim*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (!anim) return;

    anim->curX += (anim->targetX - anim->curX) / (float)g_flySpeed;
    anim->curY += (anim->targetY - anim->curY) / (float)g_flySpeed;
    anim->alpha -= 5;
    anim->phase += 0.3f;

    // High-energy pulse
    float pulse = 1.0f + (0.15f * (float)sin(anim->phase));
    int displaySize = (int)(g_packetSize * (anim->alpha / 255.0f) * pulse);

    if (anim->alpha <= 0 || (MOD_MAX(anim->targetX, anim->curX) - MOD_MIN(anim->targetX, anim->curX) < 4)) {
        KillTimer(hwnd, idEvent);
        DestroyWindow(hwnd);
        HeapFree(GetProcessHeap(), 0, anim);
        return;
    }

    SetWindowPos(hwnd, NULL, (int)anim->curX, (int)anim->curY, displaySize, displaySize, SWP_NOZORDER | SWP_NOACTIVATE);
    SetLayeredWindowAttributes(hwnd, 0, (BYTE)MOD_MAX(0, anim->alpha), LWA_ALPHA);
}

void SpawnPacket(POINT startPt) {
    if (!g_enableVisual) return;

    WNDCLASSW wc = {0};
    wc.lpfnWndProc = PacketWndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"EchoPlasmaTriClass";
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        L"EchoPlasmaTriClass", NULL, WS_POPUP,
        startPt.x, startPt.y, g_packetSize, g_packetSize,
        NULL, NULL, NULL, NULL
    );

    PacketAnim* anim = (PacketAnim*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(PacketAnim));
    anim->curX = (float)startPt.x;
    anim->curY = (float)startPt.y;
    anim->targetX = (float)GetSystemMetrics(SM_CXSCREEN) - 40;
    anim->targetY = (float)GetSystemMetrics(SM_CYSCREEN) - 40;
    anim->alpha = 255;
    anim->phase = 0;

    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)anim);
    SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
    ShowWindow(hwnd, SW_SHOWNOACTIVATE);

    SetTimer(hwnd, 7006, 16, (TIMERPROC)PacketTimerProc);
}

typedef HANDLE (WINAPI *SetClipboardData_t)(UINT, HANDLE);
SetClipboardData_t SetClipboardData_Orig;

HANDLE WINAPI SetClipboardData_Hook(UINT uFormat, HANDLE hMem) {
    HANDLE result = SetClipboardData_Orig(uFormat, hMem);
    if (result) {
        if (g_enableSound) {
            if (g_soundPath[0] != L'\0') PlaySoundW(g_soundPath, NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
            else MessageBeep(MB_ICONINFORMATION);
        }
        if (g_enableVisual) {
            POINT pt; GetCursorPos(&pt);
            SpawnPacket(pt);
        }
    }
    return result;
}

BOOL Wh_ModInit() {
    LoadSettings();
    Wh_SetFunctionHook((void*)GetProcAddress(GetModuleHandle(L"user32.dll"), "SetClipboardData"),
                       (void*)SetClipboardData_Hook,
                       (void**)&SetClipboardData_Orig);
    return TRUE;
}

void Wh_ModSettingsChanged() {
    LoadSettings();
}
