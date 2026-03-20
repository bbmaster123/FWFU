// ==WindhawkMod==
// @id              gdi-start-orb-restorer
// @name            GDI+ Start Orb (Animated)
// @description     Overlays the Start button with smooth alpha-blended transitions.
// @version         1.2
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
  $name: Orb Size (Pixel Height x DPI Scaling)
- animSpeed: 25
  $name: Animation Fade Speed (Higher = Faster)
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <gdiplus.h>
#include <math.h>

using namespace Gdiplus;

// --- Globals ---
HWND g_hOrbWnd = NULL;
ULONG_PTR g_gdiToken = 0;
Image *g_imgNormal = nullptr, *g_imgHover = nullptr, *g_imgPressed = nullptr;
const wchar_t g_szClassName[] = L"WindhawkOrbAnimated";

int g_state = 0;          
int g_hoverAlpha = 0;     
UINT_PTR g_animTimer = 0;
UINT_PTR g_posTimer = 0;
bool g_isDrawing = false;

// --- Rendering Engine ---
void UpdateOrbDisplay(HWND hwnd) {
    if (!hwnd || !IsWindow(hwnd) || g_isDrawing) return;
    g_isDrawing = true;

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

    {
        Graphics graphics(hdcMem);
        graphics.SetSmoothingMode(SmoothingModeAntiAlias);
        graphics.Clear(Color(0, 0, 0, 0));

        // Safety check: only draw if status is OK
        if (g_imgNormal && g_imgNormal->GetLastStatus() == Ok) {
            graphics.DrawImage(g_imgNormal, 0, 0, size, size);
        }

        if (g_imgHover && g_imgHover->GetLastStatus() == Ok && g_hoverAlpha > 0) {
            float a = fmin(1.0f, fmax(0.0f, (float)g_hoverAlpha / 255.0f));
            ColorMatrix matrix = {{
                {1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
                {0.0f, 1.0f, 0.0f, 0.0f, 0.0f},
                {0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
                {0.0f, 0.0f, 0.0f, a, 0.0f},
                {0.0f, 0.0f, 0.0f, 0.0f, 1.0f}
            }};
            ImageAttributes attr;
            attr.SetColorMatrix(&matrix, ColorMatrixFlagsDefault, ColorAdjustTypeBitmap);
            graphics.DrawImage(g_imgHover, Rect(0, 0, size, size), 0, 0, 
                               g_imgHover->GetWidth(), g_imgHover->GetHeight(), UnitPixel, &attr);
        }

        if (g_state == 2 && g_imgPressed && g_imgPressed->GetLastStatus() == Ok) {
            graphics.DrawImage(g_imgPressed, 0, 0, size, size);
        }
    }

    POINT ptSrc = {0, 0};
    SIZE sizeWnd = {size, size};
    BLENDFUNCTION blend = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    
    UpdateLayeredWindow(hwnd, hdcScreen, NULL, &sizeWnd, hdcMem, &ptSrc, 0, &blend, ULW_ALPHA);

    SelectObject(hdcMem, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);
    
    g_isDrawing = false;
}

// --- Positioning ---
void UpdateOrbPosition() {
    if (!g_hOrbWnd || !IsWindow(g_hOrbWnd)) return;

    HWND hTask = FindWindow(L"Shell_TrayWnd", NULL);
    HWND hStart = FindWindowEx(hTask, NULL, L"Start", NULL);
    if (!hStart) hStart = FindWindowEx(hTask, NULL, L"Button", NULL);

    if (hStart) {
        RECT rc; GetWindowRect(hStart, &rc);
        if (rc.left == 0 && rc.top == 0 && rc.right == 0) return;

        int size = Wh_GetIntSetting(L"orbSize");
        int x = rc.left + ((rc.right - rc.left) - size) / 2;
        int y = rc.top + ((rc.bottom - rc.top) - size) / 2;

        SetWindowPos(g_hOrbWnd, HWND_TOPMOST, x, y, size, size, 
                     SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_ASYNCWINDOWPOS);
    }
}

// --- Animation ---
void CALLBACK AnimTimerProc(HWND hwnd, UINT, UINT_PTR, DWORD) {
    int step = Wh_GetIntSetting(L"animSpeed");
    bool changed = false;

    if (g_state >= 1) { 
        if (g_hoverAlpha < 255) {
            g_hoverAlpha = (int)fmin(255.0, (double)g_hoverAlpha + step);
            changed = true;
        }
    } else { 
        if (g_hoverAlpha > 0) {
            g_hoverAlpha = (int)fmax(0.0, (double)g_hoverAlpha - step);
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
                UpdateOrbDisplay(hwnd);
            }
            break;
        }
        case WM_MOUSELEAVE: g_state = 0; break;
        case WM_LBUTTONDOWN: g_state = 2; UpdateOrbDisplay(hwnd); break;
        case WM_LBUTTONUP:
            if (g_state == 2) {
                g_state = 1;
                UpdateOrbDisplay(hwnd);
                HWND hTask = FindWindow(L"Shell_TrayWnd", NULL);
                PostMessage(hTask, WM_SYSCOMMAND, SC_TASKLIST, 0);
            }
            break;
        case WM_SETCURSOR: SetCursor(LoadCursor(NULL, IDC_HAND)); return TRUE;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

BOOL Wh_ModInit() {
    GdiplusStartupInput gdiInput;
    if (GdiplusStartup(&g_gdiToken, &gdiInput, NULL) != Ok) return FALSE;
    
    g_imgNormal = Image::FromFile(Wh_GetStringSetting(L"orbNormal"));
    g_imgHover = Image::FromFile(Wh_GetStringSetting(L"orbHover"));
    g_imgPressed = Image::FromFile(Wh_GetStringSetting(L"orbPressed"));

    WNDCLASS wc = {0};
    wc.lpfnWndProc = OrbWndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = g_szClassName;
    RegisterClass(&wc);

    HWND hTask = FindWindow(L"Shell_TrayWnd", NULL);
    int size = Wh_GetIntSetting(L"orbSize");
    
    g_hOrbWnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, 
                               g_szClassName, NULL, WS_POPUP, 0, 0, size, size, hTask, NULL, wc.hInstance, NULL);

    UpdateOrbPosition();
    UpdateOrbDisplay(g_hOrbWnd);
    
    g_animTimer = SetTimer(NULL, 0, 16, AnimTimerProc);
    g_posTimer = SetTimer(NULL, 1, 240, [](HWND, UINT, UINT_PTR, DWORD) { UpdateOrbPosition(); });

    return TRUE;
}

void Wh_ModUninit() {
    if (g_animTimer) KillTimer(NULL, g_animTimer);
    if (g_posTimer) KillTimer(NULL, g_posTimer);
    
    if (g_hOrbWnd) {
        ShowWindow(g_hOrbWnd, SW_HIDE);
        DestroyWindow(g_hOrbWnd);
        g_hOrbWnd = NULL;
    }

    UnregisterClass(g_szClassName, GetModuleHandle(NULL));

    if (g_imgNormal) { delete g_imgNormal; g_imgNormal = nullptr; }
    if (g_imgHover) { delete g_imgHover; g_imgHover = nullptr; }
    if (g_imgPressed) { delete g_imgPressed; g_imgPressed = nullptr; }

    if (g_gdiToken) GdiplusShutdown(g_gdiToken);
}
