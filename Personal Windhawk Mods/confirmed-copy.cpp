// ==WindhawkMod==
// @id              confirmed-copy
// @name            confirmed-copy
// @description     Soft tri-color copy indicator. Shows that data was placed on the Windows clipboard.
// @version         1.0.5
// @author          bbmaster123/Gemini
// @include         *
// @compilerOptions -luser32 -lgdi32 -lwinmm
// ==/WindhawkMod==

// ==WindhawkModSettings==
/*
- enableVisual: true
  $name: Enable Animation
- color1: "00FF00"
  $name: Outer Halo Color (Hex)
- color2: "00FFFF"
  $name: Middle Color (Hex)
- color3: "FFFFFF"
  $name: Centre Color (Hex)
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
#include <atomic>
#include <algorithm>
#include <cstdint>
#include <wchar.h>

namespace {

constexpr size_t kQueueCapacity = 32;
constexpr UINT_PTR kAnimationTimer = 7006;
constexpr wchar_t kWindowClass[] = L"WindhawkConfirmedCopyPacket105";

struct Settings {
    COLORREF color1, color2, color3;
    int flySpeed, packetSize;
    bool enableVisual, enableSound;
    WCHAR soundPath[MAX_PATH];
};

struct CopyEvent {
    POINT start;
    Settings settings;
};

// SRW locks have no DLL-detach destructor. All handles and window procedures are
// explicitly released on a normal mod unload; process termination needs no wait.
SRWLOCK g_settingsLock = SRWLOCK_INIT;
Settings g_settings{};
SRWLOCK g_queueLock = SRWLOCK_INIT;
SRWLOCK g_workerLock = SRWLOCK_INIT;
CopyEvent g_queue[kQueueCapacity]{};
size_t g_queueHead = 0;
size_t g_queueCount = 0;
HANDLE g_queueEvent = nullptr;
HANDLE g_stopEvent = nullptr;
HANDLE g_workerThread = nullptr;
HMODULE g_module = nullptr;
std::atomic<bool> g_stopping{false};
std::atomic<unsigned> g_activeWork{0};

// Count the whole hook call, including time spent in the original API. The
// normal unload waits for entered hooks before releasing the worker resources.
class HookActivity {
public:
    HookActivity() { g_activeWork.fetch_add(1); }
    ~HookActivity() { g_activeWork.fetch_sub(1); }
    HookActivity(const HookActivity&) = delete;
    HookActivity& operator=(const HookActivity&) = delete;
};

// Only the worker thread accesses these animation/window fields.
struct Animation {
    float curX, curY, targetX, targetY, phase;
    int alpha;
    Settings settings;
};
Animation g_anim{};
HWND g_packetWindow = nullptr;
// Layered-window bitmap and DC are owned exclusively by the worker thread.
HDC g_packetDC = nullptr;
HBITMAP g_packetBitmap = nullptr;
HGDIOBJ g_originalBitmap = nullptr;
std::uint32_t* g_packetPixels = nullptr;
int g_bitmapSize = 0;

COLORREF ParseColor(PCWSTR value, COLORREF fallback) {
    if (!value) return fallback;
    unsigned valueRgb = 0;
    for (int i = 0; i < 6; ++i) {
        wchar_t c = value[i];
        unsigned digit;
        if (c >= L'0' && c <= L'9') digit = c - L'0';
        else if (c >= L'a' && c <= L'f') digit = c - L'a' + 10;
        else if (c >= L'A' && c <= L'F') digit = c - L'A' + 10;
        else return fallback;
        valueRgb = (valueRgb << 4) | digit;
    }
    if (value[6] != 0) return fallback;
    return RGB((valueRgb >> 16) & 0xFF, (valueRgb >> 8) & 0xFF, valueRgb & 0xFF);
}

void LoadSettings() {
    Settings next{};
    next.enableVisual = Wh_GetIntSetting(L"enableVisual") != 0;
    next.enableSound = Wh_GetIntSetting(L"enableSound") != 0;
    next.flySpeed = Wh_GetIntSetting(L"flySpeed");
    next.packetSize = Wh_GetIntSetting(L"packetSize");
    if (next.flySpeed < 1 || next.flySpeed > 100) next.flySpeed = 8;
    if (next.packetSize < 5 || next.packetSize > 500) next.packetSize = 24;

    PCWSTR value = Wh_GetStringSetting(L"color1");
    next.color1 = ParseColor(value, RGB(0, 255, 0));
    Wh_FreeStringSetting(value);
    value = Wh_GetStringSetting(L"color2");
    next.color2 = ParseColor(value, RGB(0, 255, 255));
    Wh_FreeStringSetting(value);
    value = Wh_GetStringSetting(L"color3");
    next.color3 = ParseColor(value, RGB(255, 255, 255));
    Wh_FreeStringSetting(value);

    value = Wh_GetStringSetting(L"customSoundPath");
    if (value) {
        wcsncpy(next.soundPath, value, MAX_PATH - 1);
        next.soundPath[MAX_PATH - 1] = 0;
    }
    Wh_FreeStringSetting(value);

    AcquireSRWLockExclusive(&g_settingsLock);
    g_settings = next;
    ReleaseSRWLockExclusive(&g_settingsLock);
}

void ReleasePacketBitmap() {
    if (g_packetDC) {
        if (g_originalBitmap) SelectObject(g_packetDC, g_originalBitmap);
        if (g_packetBitmap) DeleteObject(g_packetBitmap);
        DeleteDC(g_packetDC);
    }
    g_packetDC = nullptr;
    g_packetBitmap = nullptr;
    g_originalBitmap = nullptr;
    g_packetPixels = nullptr;
    g_bitmapSize = 0;
}

bool EnsurePacketBitmap(int size) {
    if (g_packetDC && g_bitmapSize >= size) return true;

    HDC screen = GetDC(nullptr);
    if (!screen) return false;
    if (!g_packetDC) g_packetDC = CreateCompatibleDC(screen);
    if (!g_packetDC) {
        ReleaseDC(nullptr, screen);
        return false;
    }

    // Reserve room for the pulse so normal frames reuse one bitmap.
    int capacity = std::max(size, g_anim.settings.packetSize * 6 / 5 + 2);
    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(info.bmiHeader);
    info.bmiHeader.biWidth = capacity;
    info.bmiHeader.biHeight = -capacity; // Top-down BGRA.
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    void* pixels = nullptr;
    HBITMAP bitmap = CreateDIBSection(screen, &info, DIB_RGB_COLORS,
                                      &pixels, nullptr, 0);
    ReleaseDC(nullptr, screen);
    if (!bitmap || !pixels) return false;

    HGDIOBJ previous = SelectObject(g_packetDC, bitmap);
    if (!previous || previous == HGDI_ERROR) {
        DeleteObject(bitmap);
        return false;
    }
    if (g_packetBitmap) DeleteObject(g_packetBitmap);
    else g_originalBitmap = previous;
    g_packetBitmap = bitmap;
    g_packetPixels = static_cast<std::uint32_t*>(pixels);
    g_bitmapSize = capacity;
    return true;
}

float Smooth(float low, float high, float value) {
    float t = std::clamp((value - low) / (high - low), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

float Channel(COLORREF color, int shift) {
    return static_cast<float>((color >> shift) & 0xFF);
}

bool DrawPacket(HWND hwnd, int x, int y, int size, BYTE opacity) {
    if (!EnsurePacketBitmap(size)) return false;
    const float centre = size * 0.5f;
    const float radius = std::max(centre, 0.5f);
    const auto& colors = g_anim.settings;
    for (int py = 0; py < size; ++py) {
        for (int px = 0; px < size; ++px) {
            float dx = (px + 0.5f - centre) / radius;
            float dy = (py + 0.5f - centre) / radius;
            float r = sqrtf(dx * dx + dy * dy);

            // Keep the soft edge while adding a little opacity to the coloured
            // body, so the indicator stays legible at its default size.
            float halo = 1.0f - Smooth(0.70f, 1.0f, r);
            float body = 1.0f - Smooth(0.50f, 0.85f, r);
            float core = 1.0f - Smooth(0.04f, 0.28f, r);
            float coverage = std::clamp(0.21f * halo + 0.77f * body +
                                        0.12f * core, 0.0f, 1.0f);
            int alpha = static_cast<int>(coverage * 255.0f + 0.5f);

            float middle = Smooth(0.07f, 0.36f, r);
            float outer = Smooth(0.40f, 0.70f, r);
            auto blended = [&](int shift) {
                float value = Channel(colors.color3, shift) * (1.0f - middle) +
                              Channel(colors.color2, shift) * middle;
                value = value * (1.0f - outer) +
                        Channel(colors.color1, shift) * outer;
                return static_cast<unsigned>(value * alpha / 255.0f + 0.5f);
            };
            unsigned red = blended(0);
            unsigned green = blended(8);
            unsigned blue = blended(16);
            g_packetPixels[py * g_bitmapSize + px] =
                (static_cast<unsigned>(alpha) << 24) |
                (red << 16) | (green << 8) | blue;
        }
    }

    HDC screen = GetDC(nullptr);
    if (!screen) return false;
    POINT destination{x, y};
    POINT origin{0, 0};
    SIZE extent{size, size};
    BLENDFUNCTION blend{AC_SRC_OVER, 0, opacity, AC_SRC_ALPHA};
    BOOL ok = UpdateLayeredWindow(hwnd, screen, &destination, &extent,
                                  g_packetDC, &origin, 0, &blend, ULW_ALPHA);
    ReleaseDC(nullptr, screen);
    return ok != FALSE;
}

LRESULT CALLBACK PacketWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT: {
        PAINTSTRUCT paint;
        BeginPaint(hwnd, &paint);
        EndPaint(hwnd, &paint);
        return 0;
    }
    case WM_TIMER:
        if (wParam == kAnimationTimer) {
            int speed = g_anim.settings.flySpeed;
            g_anim.curX += (g_anim.targetX - g_anim.curX) / speed;
            g_anim.curY += (g_anim.targetY - g_anim.curY) / speed;
            g_anim.alpha -= 5;
            g_anim.phase += 0.3f;
            if (g_anim.alpha <= 0 ||
                (fabsf(g_anim.targetX - g_anim.curX) < 4 &&
                 fabsf(g_anim.targetY - g_anim.curY) < 4)) {
                KillTimer(hwnd, kAnimationTimer);
                DestroyWindow(hwnd);
                return 0;
            }
            float pulse = 1.0f + 0.15f * sinf(g_anim.phase);
            int size = static_cast<int>(g_anim.settings.packetSize *
                                        (g_anim.alpha / 255.0f) * pulse);
            if (size < 1) size = 1;
            DrawPacket(hwnd, static_cast<int>(g_anim.curX),
                       static_cast<int>(g_anim.curY), size,
                       static_cast<BYTE>(g_anim.alpha));
            return 0;
        }
        break;
    case WM_NCDESTROY:
        g_packetWindow = nullptr;
        break;
    }
    return DefWindowProcW(hwnd, message, wParam, lParam);
}

void StartAnimation(const CopyEvent& event, bool canDraw) {
    if (!event.settings.enableVisual || !canDraw) return;
    g_anim.curX = static_cast<float>(event.start.x);
    g_anim.curY = static_cast<float>(event.start.y);
    g_anim.targetX = static_cast<float>(GetSystemMetrics(SM_CXSCREEN) - 40);
    g_anim.targetY = static_cast<float>(GetSystemMetrics(SM_CYSCREEN) - 40);
    g_anim.phase = 0;
    g_anim.alpha = 255;
    g_anim.settings = event.settings;

    if (!g_packetWindow) {
        g_packetWindow = CreateWindowExW(
            WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
            kWindowClass, nullptr, WS_POPUP, event.start.x, event.start.y,
            event.settings.packetSize, event.settings.packetSize,
            nullptr, nullptr, g_module, nullptr);
        if (!g_packetWindow) return;
    }
    if (!DrawPacket(g_packetWindow, event.start.x, event.start.y,
                    event.settings.packetSize, 255)) {
        DestroyWindow(g_packetWindow);
        return;
    }
    ShowWindow(g_packetWindow, SW_SHOWNOACTIVATE);
    if (!SetTimer(g_packetWindow, kAnimationTimer, 16, nullptr)) {
        DestroyWindow(g_packetWindow);
    }
}

DWORD WINAPI WorkerThread(LPVOID) {
    WNDCLASSW wc{};
    wc.lpfnWndProc = PacketWndProc;
    wc.hInstance = g_module;
    wc.lpszClassName = kWindowClass;
    const bool canDraw = RegisterClassW(&wc) != 0;

    HANDLE events[] = {g_stopEvent, g_queueEvent};
    for (;;) {
        DWORD wait = MsgWaitForMultipleObjects(2, events, FALSE, INFINITE, QS_ALLINPUT);
        if (wait == WAIT_OBJECT_0 || wait == WAIT_FAILED) break;
        if (wait == WAIT_OBJECT_0 + 1) {
            for (;;) {
                CopyEvent event;
                AcquireSRWLockExclusive(&g_queueLock);
                bool hasEvent = g_queueCount != 0;
                if (hasEvent) {
                    event = g_queue[g_queueHead];
                    g_queueHead = (g_queueHead + 1) % kQueueCapacity;
                    --g_queueCount;
                }
                ReleaseSRWLockExclusive(&g_queueLock);
                if (!hasEvent || g_stopping.load()) break;
                if (event.settings.enableSound) {
                    if (event.settings.soundPath[0]) {
                        PlaySoundW(event.settings.soundPath, nullptr,
                                   SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
                    } else {
                        MessageBeep(MB_ICONINFORMATION);
                    }
                }
                StartAnimation(event, canDraw);
            }
        }
        MSG msg;
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) goto worker_exit;
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
worker_exit:
    if (g_packetWindow) {
        KillTimer(g_packetWindow, kAnimationTimer);
        DestroyWindow(g_packetWindow);
    }
    ReleasePacketBitmap();
    if (canDraw) UnregisterClassW(kWindowClass, g_module);
    return 0;
}

bool EnsureWorker() {
    AcquireSRWLockExclusive(&g_workerLock);
    if (!g_workerThread && !g_stopping.load()) {
        g_workerThread = CreateThread(nullptr, 0, WorkerThread, nullptr, 0, nullptr);
    }
    bool ready = g_workerThread != nullptr && !g_stopping.load();
    ReleaseSRWLockExclusive(&g_workerLock);
    return ready;
}

using SetClipboardData_t = HANDLE (WINAPI*)(UINT, HANDLE);
SetClipboardData_t SetClipboardData_Orig = nullptr;
using EmptyClipboard_t = BOOL (WINAPI*)();
EmptyClipboard_t EmptyClipboard_Orig = nullptr;
using CloseClipboard_t = BOOL (WINAPI*)();
CloseClipboard_t CloseClipboard_Orig = nullptr;

// Clipboard writes from a normal copy follow EmptyClipboard and occur before
// CloseClipboard on the same thread. Delayed rendering during paste calls
// SetClipboardData without EmptyClipboard, so it must not show a copy packet.
thread_local bool g_copyOperation = false;
thread_local bool g_notifiedForOperation = false;

BOOL WINAPI EmptyClipboard_Hook() {
    HookActivity active;
    g_copyOperation = false;
    g_notifiedForOperation = false;
    BOOL result = EmptyClipboard_Orig();
    DWORD lastError = GetLastError();
    if (result && !g_stopping.load()) g_copyOperation = true;
    SetLastError(lastError);
    return result;
}

BOOL WINAPI CloseClipboard_Hook() {
    HookActivity active;
    BOOL result = CloseClipboard_Orig();
    DWORD lastError = GetLastError();
    g_copyOperation = false;
    g_notifiedForOperation = false;
    SetLastError(lastError);
    return result;
}

HANDLE WINAPI SetClipboardData_Hook(UINT format, HANDLE memory) {
    HookActivity active;
    HANDLE result = SetClipboardData_Orig(format, memory);
    DWORD lastError = GetLastError();
    if (result && g_copyOperation && !g_notifiedForOperation &&
        !g_stopping.load()) {
        // Mark before queueing. A multi-format copy gets one notification even
        // if starting the worker fails; the next copy can still try again.
        g_notifiedForOperation = true;
        if (!g_stopping.load()) {
            CopyEvent event{};
            AcquireSRWLockShared(&g_settingsLock);
            event.settings = g_settings;
            ReleaseSRWLockShared(&g_settingsLock);
            if ((event.settings.enableVisual || event.settings.enableSound) &&
                EnsureWorker()) {
                if (event.settings.enableVisual) GetCursorPos(&event.start);
                AcquireSRWLockExclusive(&g_queueLock);
                if (g_queueCount == kQueueCapacity) {
                    g_queueHead = (g_queueHead + 1) % kQueueCapacity;
                    --g_queueCount;
                }
                g_queue[(g_queueHead + g_queueCount) % kQueueCapacity] = event;
                ++g_queueCount;
                ReleaseSRWLockExclusive(&g_queueLock);
                SetEvent(g_queueEvent);
            }
        }
    }
    SetLastError(lastError);
    return result;
}

} // namespace

BOOL Wh_ModInit() {
    LoadSettings();
    HMODULE user32 = nullptr;
    FARPROC setClipboardData = nullptr;
    FARPROC emptyClipboard = nullptr;
    FARPROC closeClipboard = nullptr;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            reinterpret_cast<PCWSTR>(&PacketWndProc), &g_module)) {
        return FALSE;
    }
    g_stopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    g_queueEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (!g_stopEvent || !g_queueEvent) goto failure;

    user32 = GetModuleHandleW(L"user32.dll");
    setClipboardData = user32 ? GetProcAddress(user32, "SetClipboardData") : nullptr;
    emptyClipboard = user32 ? GetProcAddress(user32, "EmptyClipboard") : nullptr;
    closeClipboard = user32 ? GetProcAddress(user32, "CloseClipboard") : nullptr;
    if (setClipboardData && emptyClipboard && closeClipboard &&
        Wh_SetFunctionHook(reinterpret_cast<void*>(setClipboardData),
                           reinterpret_cast<void*>(SetClipboardData_Hook),
                           reinterpret_cast<void**>(&SetClipboardData_Orig)) &&
        Wh_SetFunctionHook(reinterpret_cast<void*>(emptyClipboard),
                           reinterpret_cast<void*>(EmptyClipboard_Hook),
                           reinterpret_cast<void**>(&EmptyClipboard_Orig)) &&
        Wh_SetFunctionHook(reinterpret_cast<void*>(closeClipboard),
                           reinterpret_cast<void*>(CloseClipboard_Hook),
                           reinterpret_cast<void**>(&CloseClipboard_Orig))) {
        return TRUE;
    }

failure:
    g_stopping.store(true);
    if (g_stopEvent) SetEvent(g_stopEvent);
    if (g_workerThread) {
        WaitForSingleObject(g_workerThread, INFINITE);
        CloseHandle(g_workerThread);
        g_workerThread = nullptr;
    }
    if (g_queueEvent) CloseHandle(g_queueEvent);
    if (g_stopEvent) CloseHandle(g_stopEvent);
    g_queueEvent = g_stopEvent = nullptr;
    return FALSE;
}

void Wh_ModBeforeUninit() {
    g_stopping.store(true);
    if (g_stopEvent) SetEvent(g_stopEvent);
}

void Wh_ModUninit() {
    g_stopping.store(true);
    if (g_stopEvent) SetEvent(g_stopEvent);
    // The hooks have been disabled by Windhawk, but calls that entered earlier
    // may still be inside the original API or finishing their queue work.
    while (g_activeWork.load() != 0) Sleep(1);
    if (g_workerThread) {
        WaitForSingleObject(g_workerThread, INFINITE);
        CloseHandle(g_workerThread);
        g_workerThread = nullptr;
    }
    if (g_queueEvent) CloseHandle(g_queueEvent);
    if (g_stopEvent) CloseHandle(g_stopEvent);
    g_queueEvent = g_stopEvent = nullptr;
}

void Wh_ModSettingsChanged() {
    LoadSettings();
}
