// ==WindhawkMod==
// @id             start-orb-restorer
// @name           Start Button Orb +
// @description    Stable Windows 7 style Start Orb overlay
// @version        0.7
// @author         Bbmaster123/AI
// @include        explorer.exe
// @architecture   x86-64
// @compilerOptions -lgdiplus -lgdi32 -luser32
// ==/WindhawkMod==

// ==WindhawkModSettings==
/*
- orbNormal: "C:\\Users\\bbmaster123\\Desktop\\Junk\\orb\\orb.png"
- orbHover: "C:\\Users\\bbmaster123\\Desktop\\Junk\\orb\\orbHover.png"
- orbPressed: "C:\\Users\\bbmaster123\\Desktop\\Junk\\orb\\orbPressed.png"
- orbSize: 72
- animSpeed: 15
- minOpacity: 255
- maxOpacity: 255
- offsetX: 0
- offsetY: 0
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <gdiplus.h>

using namespace Gdiplus;

struct {
    int size;
    int speed;
    int minOpacity;
    int maxOpacity;
    int offsetX;
    int offsetY;
} g_settings;

HWND g_hOrbWnd = NULL;
HWND g_hStart = NULL;

ULONG_PTR g_gdiToken = 0;

Image *g_imgNormal = nullptr;
Image *g_imgHover = nullptr;
Image *g_imgPressed = nullptr;

float g_fadeAlpha = 0.0f;
int g_state = 0;

bool g_isUnloading = false;

CRITICAL_SECTION g_cs;

// -------------------- IMAGE --------------------

Image* LoadOne(PCWSTR path) {
    if (!path || !path[0]) return nullptr;
    Image* img = Image::FromFile(path);
    if (!img || img->GetLastStatus() != Ok) {
        delete img;
        return nullptr;
    }
    return img;
}

void CleanImages() {
    EnterCriticalSection(&g_cs);
    delete g_imgNormal;
    delete g_imgHover;
    delete g_imgPressed;
    g_imgNormal = g_imgHover = g_imgPressed = nullptr;
    LeaveCriticalSection(&g_cs);
}

void LoadImages() {
    CleanImages();

    EnterCriticalSection(&g_cs);
    g_imgNormal  = LoadOne(Wh_GetStringSetting(L"orbNormal"));
    g_imgHover   = LoadOne(Wh_GetStringSetting(L"orbHover"));
    g_imgPressed = LoadOne(Wh_GetStringSetting(L"orbPressed"));
    LeaveCriticalSection(&g_cs);
}

// -------------------- START BUTTON --------------------

HWND FindStartButton() {
    HWND hTask = FindWindow(L"Shell_TrayWnd", NULL);
    if (!hTask) return NULL;

    HWND child = NULL;
    while ((child = FindWindowEx(hTask, child, NULL, NULL))) {
        wchar_t cls[64];
        GetClassName(child, cls, 64);

        if (wcscmp(cls, L"Start") == 0 ||
            wcscmp(cls, L"Button") == 0) {
            return child;
        }
    }
    return NULL;
}

// -------------------- POSITION --------------------

void PositionOrb() {
    if (!g_hOrbWnd) return;

    if (!g_hStart)
        g_hStart = FindStartButton();

    if (!g_hStart) return;

    RECT rc;
    GetWindowRect(g_hStart, &rc);

    int x = rc.left + ((rc.right - rc.left) - g_settings.size) / 2 + g_settings.offsetX;
    int y = rc.top + ((rc.bottom - rc.top) - g_settings.size) / 2 + g_settings.offsetY;

    SetWindowPos(g_hOrbWnd, HWND_TOPMOST,
        x, y, g_settings.size, g_settings.size,
        SWP_NOACTIVATE | SWP_SHOWWINDOW);

    // Force topmost (prevents taskbar stealing z-order)
    SetWindowPos(g_hOrbWnd, HWND_TOPMOST,
        0,0,0,0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

// -------------------- DRAW --------------------

void UpdateOrbDisplay(HWND hwnd) {
    if (g_isUnloading || !hwnd) return;

    EnterCriticalSection(&g_cs);

    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = g_settings.size;
    bmi.bmiHeader.biHeight = -g_settings.size;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;

    void* pBits = NULL;
    HBITMAP hBitmap = CreateDIBSection(hdcMem, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);

    if (hBitmap) {
        HBITMAP old = (HBITMAP)SelectObject(hdcMem, hBitmap);

        Graphics g(hdcMem);
        g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
        g.Clear(Color(0,0,0,0));

        RectF rect(0,0,(REAL)g_settings.size,(REAL)g_settings.size);

        Image* img = g_imgNormal;

        if (g_state == 2 && g_imgPressed)
            img = g_imgPressed;

        g.DrawImage(img, rect);

        if (g_imgHover && g_fadeAlpha > 0.001f && g_state != 2) {
            ImageAttributes attr;
            ColorMatrix matrix = {
                1,0,0,0,0,
                0,1,0,0,0,
                0,0,1,0,0,
                0,0,0,g_fadeAlpha,0,
                0,0,0,0,1
            };
            attr.SetColorMatrix(&matrix);

            g.DrawImage(g_imgHover, rect, 0,0,
                g_imgHover->GetWidth(),
                g_imgHover->GetHeight(),
                UnitPixel, &attr);
        }

        int alpha = (int)(g_settings.minOpacity +
            (g_settings.maxOpacity - g_settings.minOpacity) * g_fadeAlpha);

        BLENDFUNCTION blend = {AC_SRC_OVER, 0, (BYTE)alpha, AC_SRC_ALPHA};

        POINT pt = {0,0};
        SIZE sz = {g_settings.size, g_settings.size};

        UpdateLayeredWindow(hwnd, hdcScreen, NULL, &sz,
            hdcMem, &pt, 0, &blend, ULW_ALPHA);

        SelectObject(hdcMem, old);
        DeleteObject(hBitmap);
    }

    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);

    LeaveCriticalSection(&g_cs);
}

// -------------------- WINDOW PROC --------------------

LRESULT CALLBACK OrbWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {

    case WM_TIMER:

        if (wp == 1) {
            float target = (g_state >= 1) ? 1.0f : 0.0f;
            float step = g_settings.speed / 1000.0f;

            if (g_fadeAlpha < target) {
                g_fadeAlpha += step;
                if (g_fadeAlpha > target) g_fadeAlpha = target;
            } else if (g_fadeAlpha > target) {
                g_fadeAlpha -= step;
                if (g_fadeAlpha < target) g_fadeAlpha = target;
            }

            UpdateOrbDisplay(hwnd);
        }

        else if (wp == 2) {
            PositionOrb();
        }

        else if (wp == 3) {
            if (!g_hStart)
                g_hStart = FindStartButton();

            if (g_hStart) {
                POINT pt;
                GetCursorPos(&pt);

                RECT rc;
                GetWindowRect(g_hStart, &rc);

                bool hover = PtInRect(&rc, pt);
                bool pressed = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) && hover;

                g_state = pressed ? 2 : (hover ? 1 : 0);
            }
        }

        return 0;

    case WM_SETCURSOR:
        SetCursor(LoadCursor(NULL, IDC_HAND));
        return TRUE;
    }

    return DefWindowProc(hwnd, msg, wp, lp);
}

// -------------------- INIT --------------------

BOOL Wh_ModInit() {
    InitializeCriticalSection(&g_cs);

    GdiplusStartupInput gdiInput;
    GdiplusStartup(&g_gdiToken, &gdiInput, NULL);

    g_settings.size = Wh_GetIntSetting(L"orbSize");
    g_settings.speed = Wh_GetIntSetting(L"animSpeed");
    g_settings.minOpacity = Wh_GetIntSetting(L"minOpacity");
    g_settings.maxOpacity = Wh_GetIntSetting(L"maxOpacity");
    g_settings.offsetX = Wh_GetIntSetting(L"offsetX");
    g_settings.offsetY = Wh_GetIntSetting(L"offsetY");

    LoadImages();

    WNDCLASS wc = {};
    wc.lpfnWndProc = OrbWndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"WindhawkOrbStable";
    RegisterClass(&wc);

    g_hOrbWnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE | WS_EX_TRANSPARENT,
        wc.lpszClassName, NULL,
        WS_POPUP,
        0,0,g_settings.size,g_settings.size,
        NULL,NULL,wc.hInstance,NULL);

    if (g_hOrbWnd) {
        SetTimer(g_hOrbWnd, 1, 16, NULL);  // animation
        SetTimer(g_hOrbWnd, 2, 50, NULL);  // position
        SetTimer(g_hOrbWnd, 3, 16, NULL);  // state tracking

        PositionOrb();
        UpdateOrbDisplay(g_hOrbWnd);
    }

    return TRUE;
}

// -------------------- CLEANUP --------------------

void Wh_ModUninit() {
    g_isUnloading = true;

    if (g_hOrbWnd) {
        KillTimer(g_hOrbWnd, 1);
        KillTimer(g_hOrbWnd, 2);
        KillTimer(g_hOrbWnd, 3);
        DestroyWindow(g_hOrbWnd);
    }

    CleanImages();

    if (g_gdiToken)
        GdiplusShutdown(g_gdiToken);

    DeleteCriticalSection(&g_cs);
}

// -------------------- SETTINGS --------------------

void Wh_ModSettingsChanged() {
    g_settings.size = Wh_GetIntSetting(L"orbSize");
    g_settings.speed = Wh_GetIntSetting(L"animSpeed");
    g_settings.minOpacity = Wh_GetIntSetting(L"minOpacity");
    g_settings.maxOpacity = Wh_GetIntSetting(L"maxOpacity");
    g_settings.offsetX = Wh_GetIntSetting(L"offsetX");
    g_settings.offsetY = Wh_GetIntSetting(L"offsetY");

    LoadImages();

    if (g_hOrbWnd) {
        PositionOrb();
        UpdateOrbDisplay(g_hOrbWnd);
    }
}
