// ==WindhawkMod==
// @id              gdi-start-orb-restorer
// @name            GDI+ Start Orb (Animated)
// @description     Overlays the Start button with smooth alpha-blended transitions. Works, still buggy, mainly with triggering start menu, animation looks good so far though!
// @version         0.7
// @author          Bbmaster123/AI
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lgdiplus -lgdi32 -lshlwapi -luser32
// ==/WindhawkMod==

// ==WindhawkModSettings==
/*
- orbNormal: "C:\\Users\\bbmaster123\\Desktop\\junk\\themes\\orb.png"
  $name: Normal Orb Path
- orbHover: "C:\\Users\\bbmaster123\\Desktop\\junk\\themes\\orb-pointer.png"
  $name: Hover Orb Path
- orbPressed: "C:\\Users\\bbmaster123\\Desktop\\junk\\themes\\orb-pressed.png"
  $name: Pressed Orb Path
- orbSize: 96
  $name: Orb Size (DPI Aware)
- animSpeed: 25
  $name: Animation Fade Speed (Higher = Faster)
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <gdiplus.h>

using namespace Gdiplus;

// --- Globals ---
HWND g_hOrbWnd = NULL;
ULONG_PTR g_gdiToken;
Image *g_imgNormal = nullptr, *g_imgHover = nullptr, *g_imgPressed = nullptr;

int g_state = 0;          // 0: Normal, 1: Hover, 2: Pressed
int g_hoverAlpha = 0;     // 0 to 255 for the fade effect
UINT_PTR g_animTimer = 0;
UINT_PTR g_posTimer = 0;

// --- Rendering Engine ---
void UpdateOrbDisplay(HWND hwnd) {
    int size = Wh_GetIntSetting(L"orbSize");
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    
    BITMAPINFO bmi = {{0}};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = size;
    bmi.bmiHeader.biHeight = size;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* pBits = NULL;
    HBITMAP hBitmap = CreateDIBSection(hdcMem, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);

    Graphics graphics(hdcMem);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);
    graphics.Clear(Color(0, 0, 0, 0));

    // 1. Draw Base (Normal)
    if (g_imgNormal) {
        graphics.DrawImage(g_imgNormal, 0, 0, size, size);
    }

    // 2. Draw Hover Layer with Alpha
    if (g_imgHover && g_hoverAlpha > 0) {
        ImageAttributes attr;
       ColorMatrix matrix = {{
            {1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f, 0.0f, 0.0f},
            {0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
            {0.0f, 0.0f, 0.0f, (float)g_hoverAlpha / 255.0f, 0.0f},
            {0.0f, 0.0f, 0.0f, 0.0f, 1.0f}
        }};
        attr.SetColorMatrix(&matrix, ColorMatrixFlagsDefault, ColorAdjustTypeBitmap);
        graphics.DrawImage(g_imgHover, Rect(0, 0, size, size), 0, 0, g_imgHover->GetWidth(), g_imgHover->GetHeight(), UnitPixel, &attr);
    }

    // 3. Draw Pressed Layer (Overrides Hover during click)
    if (g_state == 2 && g_imgPressed) {
        graphics.DrawImage(g_imgPressed, 0, 0, size, size);
    }

    POINT ptSrc = {0, 0};
    SIZE sizeWnd = {size, size};
    BLENDFUNCTION blend = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    UpdateLayeredWindow(hwnd, hdcScreen, NULL, &sizeWnd, hdcMem, &ptSrc, 0, &blend, ULW_ALPHA);

    SelectObject(hdcMem, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);
}

// --- Animation Timer ---
void CALLBACK AnimTimerProc(HWND hwnd, UINT, UINT_PTR, DWORD) {
    int step = Wh_GetIntSetting(L"animSpeed");
    bool changed = false;

    if (g_state == 1 || g_state == 2) { // Hovering or Clicking
        if (g_hoverAlpha < 255) {
            g_hoverAlpha = fmin(255, g_hoverAlpha + step);
            changed = true;
        }
    } else { // Idle
        if (g_hoverAlpha > 0) {
            g_hoverAlpha = fmax(0, g_hoverAlpha - step);
            changed = true;
        }
    }

    if (changed) UpdateOrbDisplay(g_hOrbWnd);
}

// --- Window Logic ---
LRESULT CALLBACK OrbWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_MOUSEMOVE: {
            if (g_state == 0) {
                g_state = 1;
                TRACKMOUSEEVENT tme = {sizeof(tme), TME_LEAVE, hwnd, 0};
                TrackMouseEvent(&tme);
            }
            break;
        }
        case WM_MOUSELEAVE: g_state = 0; break;
        case WM_LBUTTONDOWN: g_state = 2; UpdateOrbDisplay(hwnd); break;
        case WM_LBUTTONUP:
            g_state = 1;
            keybd_event(VK_LWIN, 0, 0, 0);
            keybd_event(VK_LWIN, 0, KEYEVENTF_KEYUP, 0);
            break;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

// --- Positioning ---
void UpdateOrbPosition() {
    HWND hTask = FindWindow(L"Shell_TrayWnd", NULL);
    HWND hStart = FindWindowEx(hTask, NULL, L"Start", NULL);
    if (!hStart) hStart = FindWindow(NULL, L"Start");

    if (hStart && g_hOrbWnd) {
        RECT rc; GetWindowRect(hStart, &rc);
        if (rc.left != 0 || rc.top != 0) {
            int size = Wh_GetIntSetting(L"orbSize");
            int x = rc.left + ((rc.right - rc.left) - size) / 2;
            int y = rc.top + ((rc.bottom - rc.top) - size) / 2;
            SetWindowPos(g_hOrbWnd, HWND_TOPMOST, x, y, size, size, SWP_SHOWWINDOW | SWP_NOACTIVATE);
        }
    }
}

BOOL Wh_ModInit() {
    GdiplusStartupInput gdiInput;
    GdiplusStartup(&g_gdiToken, &gdiInput, NULL);
    
    g_imgNormal = Image::FromFile(Wh_GetStringSetting(L"orbNormal"));
    g_imgHover = Image::FromFile(Wh_GetStringSetting(L"orbHover"));
    g_imgPressed = Image::FromFile(Wh_GetStringSetting(L"orbPressed"));

    WNDCLASS wc = {0};
    wc.lpfnWndProc = OrbWndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"WindhawkOrbAnimated";
    RegisterClass(&wc);

    int size = Wh_GetIntSetting(L"orbSize");
    g_hOrbWnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, 
                               L"WindhawkOrbAnimated", NULL, WS_POPUP, 0, 0, size, size, NULL, NULL, wc.hInstance, NULL);

    UpdateOrbDisplay(g_hOrbWnd);
    
    // Animation timer (approx 60fps)
    g_animTimer = SetTimer(NULL, 0, 16, AnimTimerProc);
    // Position timer
    g_posTimer = SetTimer(NULL, 1, 100, [](HWND, UINT, UINT_PTR, DWORD) { UpdateOrbPosition(); });

    return TRUE;
}

void Wh_ModUninit() {
    if (g_animTimer) KillTimer(NULL, g_animTimer);
    if (g_posTimer) KillTimer(NULL, g_posTimer);
    if (g_hOrbWnd) DestroyWindow(g_hOrbWnd);
    if (g_imgNormal) delete g_imgNormal;
    if (g_imgHover) delete g_imgHover;
    if (g_imgPressed) delete g_imgPressed;
    GdiplusShutdown(g_gdiToken);
}
