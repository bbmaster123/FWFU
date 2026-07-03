// ==WindhawkMod==
// @id             start-orb-restorer
// @name           Start Orb Plus
// @description    Stable Windows 7 style Start Orb overlay
// @version        0.6
// @author         Bbmaster123/AI
// @include        explorer.exe
// @architecture   x86-64
// @compilerOptions -lgdiplus -lgdi32 -luser32 -ldwmapi
// ==/WindhawkMod==

// ==WindhawkModSettings==
/*
- orbNormal: "C:\\Users\\Brandon\\Desktop\\Junk\\orb\\orb.png"
- orbHover: "C:\\Users\\Brandon\\Desktop\\Junk\\orb\\orbHover.png"
- orbPressed: "C:\\Users\\Brandon\\Desktop\\Junk\\orb\\orbPressed.png"
- orbSize: 72
- animSpeed: 15
- minOpacity: 255
- maxOpacity: 255
- offsetX: 0
- offsetY: 0
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <dwmapi.h>
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
HANDLE g_hOrbThread = NULL;

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
    PCWSTR pathNormal = Wh_GetStringSetting(L"orbNormal");
    PCWSTR pathHover = Wh_GetStringSetting(L"orbHover");
    PCWSTR pathPressed = Wh_GetStringSetting(L"orbPressed");

    g_imgNormal  = LoadOne(pathNormal);
    g_imgHover   = LoadOne(pathHover);
    g_imgPressed = LoadOne(pathPressed);

    Wh_FreeStringSetting(pathNormal);
    Wh_FreeStringSetting(pathHover);
    Wh_FreeStringSetting(pathPressed);
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

#ifndef DWMWA_EXCLUDED_FROM_PEEK
#define DWMWA_EXCLUDED_FROM_PEEK 12
#endif

void PositionOrb() {
    if (!g_hOrbWnd) return;

    HWND hTask = GetParent(g_hOrbWnd);
    if (!hTask) {
        hTask = FindWindow(L"Shell_TrayWnd", NULL);
    }
    if (!hTask) return;

    if (!g_hStart || !IsWindow(g_hStart))
        g_hStart = FindStartButton();

    if (!g_hStart) return;

    // If minimized, restore it
    if (IsIconic(g_hOrbWnd)) {
        ShowWindow(g_hOrbWnd, SW_RESTORE);
    }

    // If hidden, show it
    if (!IsWindowVisible(g_hOrbWnd)) {
        ShowWindow(g_hOrbWnd, SW_SHOW);
    }

    RECT rc;
    GetWindowRect(g_hStart, &rc);

    int size, offsetX, offsetY;
    EnterCriticalSection(&g_cs);
    size = g_settings.size;
    offsetX = g_settings.offsetX;
    offsetY = g_settings.offsetY;
    LeaveCriticalSection(&g_cs);

    // Convert start button screen coordinates to taskbar client coordinates
    POINT pt = { rc.left, rc.top };
    ScreenToClient(hTask, &pt);

    int x = pt.x + ((rc.right - rc.left) - size) / 2 + offsetX;
    int y = pt.y + ((rc.bottom - rc.top) - size) / 2 + offsetY;

    // Use HWND_TOP to keep the child window at the top of the taskbar child z-order
    SetWindowPos(g_hOrbWnd, HWND_TOP,
        x, y, size, size,
        SWP_NOACTIVATE | SWP_SHOWWINDOW);
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

// Forward declaration
LRESULT CALLBACK OrbWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

// -------------------- BACKGROUND THREAD --------------------

DWORD WINAPI OrbThreadProc(LPVOID lpParam) {
    WNDCLASS wc = {};
    wc.lpfnWndProc = OrbWndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"WindhawkOrbStable";
    RegisterClass(&wc);

    int size;
    EnterCriticalSection(&g_cs);
    size = g_settings.size;
    LeaveCriticalSection(&g_cs);

    // Wait up to 5 seconds for Shell_TrayWnd to be available
    HWND hTask = NULL;
    for (int i = 0; i < 100; i++) {
        hTask = FindWindow(L"Shell_TrayWnd", NULL);
        if (hTask) break;
        Sleep(50);
    }

    g_hOrbWnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TRANSPARENT,
        wc.lpszClassName, NULL,
        WS_CHILD | WS_VISIBLE,
        0,0,size,size,
        hTask, NULL, wc.hInstance, NULL);

    if (g_hOrbWnd) {
        Wh_Log(L"Orb window successfully created as WS_CHILD: %p (parent hTask: %p)", g_hOrbWnd, hTask);
        
        BOOL exclude = TRUE;
        DwmSetWindowAttribute(g_hOrbWnd, DWMWA_EXCLUDED_FROM_PEEK, &exclude, sizeof(exclude));

        SetTimer(g_hOrbWnd, 1, 16, NULL);  // animation
        SetTimer(g_hOrbWnd, 2, 50, NULL);  // position
        SetTimer(g_hOrbWnd, 3, 16, NULL);  // state tracking

        PositionOrb();
        UpdateOrbDisplay(g_hOrbWnd);

        // Message pump
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return 0;
}

// -------------------- WINDOW PROC --------------------

LRESULT CALLBACK OrbWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {

    case WM_TIMER:

        if (wp == 1) {
            float target = (g_state >= 1) ? 1.0f : 0.0f;
            
            float speed;
            EnterCriticalSection(&g_cs);
            speed = (float)g_settings.speed;
            LeaveCriticalSection(&g_cs);
            
            float step = speed / 1000.0f;

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
            POINT pt;
            GetCursorPos(&pt);

            bool hover = false;

            // 1. Check if cursor is over our orb window
            if (g_hOrbWnd) {
                RECT rcOrb;
                GetWindowRect(g_hOrbWnd, &rcOrb);
                if (PtInRect(&rcOrb, pt)) {
                    hover = true;
                }
            }

            // 2. Check if cursor is over the original start button (fallback)
            if (!hover) {
                if (!g_hStart || !IsWindow(g_hStart))
                    g_hStart = FindStartButton();

                if (g_hStart) {
                    RECT rcStart;
                    GetWindowRect(g_hStart, &rcStart);
                    if (PtInRect(&rcStart, pt)) {
                        hover = true;
                    }
                }
            }

            bool pressed = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) && hover;
            g_state = pressed ? 2 : (hover ? 1 : 0);
        }

        return 0;

    case WM_WINDOWPOSCHANGING: {
        if (!g_isUnloading) {
            WINDOWPOS* lpwpos = (WINDOWPOS*)lp;
            if (lpwpos->flags & SWP_HIDEWINDOW) {
                lpwpos->flags &= ~SWP_HIDEWINDOW;
            }
            lpwpos->flags |= SWP_SHOWWINDOW;
        }
        break;
    }

    case WM_WINDOWPOSCHANGED: {
        if (!g_isUnloading) {
            WINDOWPOS* lpwpos = (WINDOWPOS*)lp;
            if ((lpwpos->flags & SWP_HIDEWINDOW) || IsIconic(hwnd)) {
                PostMessage(hwnd, WM_USER + 2, 0, 0);
            }
        }
        break;
    }

    case WM_SHOWWINDOW: {
        if (!g_isUnloading && wp == FALSE) {
            PostMessage(hwnd, WM_USER + 2, 0, 0);
        }
        break;
    }

    case WM_SYSCOMMAND: {
        if (!g_isUnloading && (wp & 0xFFF0) == SC_MINIMIZE) {
            PostMessage(hwnd, WM_USER + 2, 0, 0);
            return 0;
        }
        break;
    }

    case WM_SIZE: {
        if (!g_isUnloading && wp == SIZE_MINIMIZED) {
            PostMessage(hwnd, WM_USER + 2, 0, 0);
            return 0;
        }
        break;
    }

    case 0x031B: { // WM_DWMCLOAKEDCHANGED
        if (!g_isUnloading) {
            BOOL cloaked = FALSE;
            #ifndef DWMWA_CLOAKED
            #define DWMWA_CLOAKED 14
            #endif
            if (SUCCEEDED(DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &cloaked, sizeof(cloaked))) && cloaked) {
                PostMessage(hwnd, WM_USER + 2, 0, 0);
            }
        }
        break;
    }

    case WM_USER + 1: // WM_SETTINGS_CHANGED
        PositionOrb();
        UpdateOrbDisplay(hwnd);
        return 0;

    case WM_USER + 2: // Force show / restore
        if (!g_isUnloading) {
            if (IsIconic(hwnd)) {
                ShowWindow(hwnd, SW_RESTORE);
            }
            ShowWindow(hwnd, SW_SHOW);
            PositionOrb();
            UpdateOrbDisplay(hwnd);
        }
        return 0;

    case WM_SETCURSOR:
        SetCursor(LoadCursor(NULL, IDC_HAND));
        return TRUE;

    case WM_CLOSE:
        KillTimer(hwnd, 1);
        KillTimer(hwnd, 2);
        KillTimer(hwnd, 3);
        DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        g_hOrbWnd = NULL;
        PostQuitMessage(0);
        return 0;
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

    // Create the background thread
    g_hOrbThread = CreateThread(NULL, 0, OrbThreadProc, NULL, 0, NULL);

    return TRUE;
}

// -------------------- CLEANUP --------------------

void Wh_ModUninit() {
    g_isUnloading = true;

    if (g_hOrbWnd) {
        PostMessage(g_hOrbWnd, WM_CLOSE, 0, 0);
    }

    if (g_hOrbThread) {
        WaitForSingleObject(g_hOrbThread, 2000);
        CloseHandle(g_hOrbThread);
        g_hOrbThread = NULL;
    }

    UnregisterClass(L"WindhawkOrbStable", GetModuleHandle(NULL));

    CleanImages();

    if (g_gdiToken)
        GdiplusShutdown(g_gdiToken);

    DeleteCriticalSection(&g_cs);
}

// -------------------- SETTINGS --------------------

void Wh_ModSettingsChanged() {
    EnterCriticalSection(&g_cs);
    g_settings.size = Wh_GetIntSetting(L"orbSize");
    g_settings.speed = Wh_GetIntSetting(L"animSpeed");
    g_settings.minOpacity = Wh_GetIntSetting(L"minOpacity");
    g_settings.maxOpacity = Wh_GetIntSetting(L"maxOpacity");
    g_settings.offsetX = Wh_GetIntSetting(L"offsetX");
    g_settings.offsetY = Wh_GetIntSetting(L"offsetY");
    LeaveCriticalSection(&g_cs);

    LoadImages();

    if (g_hOrbWnd) {
        PostMessage(g_hOrbWnd, WM_USER + 1, 0, 0);
    }
}
