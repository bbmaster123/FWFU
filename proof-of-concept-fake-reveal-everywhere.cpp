// ==WindhawkMod==
// @id              fake-reveal-everywhere
// @name            Taskbar Fluent Border Glow
// @description     add reveal like effect to everthing across explorer
// @version         1.0
// @author          Gemini
// @include         explorer.exe
// @compilerOptions -lgdi32 -luser32 -ldwmapi -lole32 -loleaut32 -luiautomationcore -luuid
// ==/WindhawkMod==

// ==WindhawkModSettings==
/*
- GlowColor: "#FFFFFFFF"
  $name: Color (Hex)
- AnimSpeed100: 30
  $name: Animation Speed (100=Instant)
- GlowOpacity: 120
  $name: Spotlight Intensity
- BorderOpacity: 255
  $name: Border Intensity
- BorderThickness10: 15
  $name: Border Thickness (10=1px)
- MarginTop: 0
  $name: Margin - Top
- MarginBottom: 0
  $name: Margin - Bottom
- MarginLeft: 0
  $name: Margin - Left
- MarginRight: 0
  $name: Margin - Right
- GlowSoftness: 350
  $name: Spotlight Spread
- CornerRadius: 6
  $name: Corner Radius
- TargetList: "Button,ListItem,Pane,Group,Image,Text"
  $name: Target Element Types
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <uiautomation.h>
#include <math.h>
#include <wchar.h>

struct {
    COLORREF color;
    float animSpeed;
    int maxGlow;
    int maxBorder;
    float thickness;
    int mTop, mBottom, mLeft, mRight;
    float softness;
    int radius;
    WCHAR targetBuffer[256];
} g_activeSettings;

HWND g_hGlowWnd = NULL;
IUIAutomation* g_pAutomation = NULL;
RECT g_targetRect = {0};
float g_opacitySmooth = 0.0f;
bool g_isToolProcess = false;

struct GdiCache {
    HDC hdcMem; HBITMAP hBitmap; void* pBits; int w, h;
} g_cache = {0};

// --- Logic ---

void LoadSettings() {
    PCWSTR hex = Wh_GetStringSetting(L"GlowColor");
    int r = 255, g = 255, b = 255;
    if (hex && hex[0] == L'#') swscanf(hex + 1, L"%02x%02x%02x", &r, &g, &b);
    g_activeSettings.color = RGB(r, g, b);
    Wh_FreeStringSetting(hex);

    g_activeSettings.animSpeed = (float)Wh_GetIntSetting(L"AnimSpeed100") / 100.0f;
    g_activeSettings.maxGlow = Wh_GetIntSetting(L"GlowOpacity");
    g_activeSettings.maxBorder = Wh_GetIntSetting(L"BorderOpacity");
    g_activeSettings.softness = (float)Wh_GetIntSetting(L"GlowSoftness");
    g_activeSettings.radius = Wh_GetIntSetting(L"CornerRadius");
    g_activeSettings.thickness = (float)Wh_GetIntSetting(L"BorderThickness10") / 10.0f;
    
    g_activeSettings.mTop = Wh_GetIntSetting(L"MarginTop");
    g_activeSettings.mBottom = Wh_GetIntSetting(L"MarginBottom");
    g_activeSettings.mLeft = Wh_GetIntSetting(L"MarginLeft");
    g_activeSettings.mRight = Wh_GetIntSetting(L"MarginRight");

    PCWSTR targets = Wh_GetStringSetting(L"TargetList");
    wcsncpy(g_activeSettings.targetBuffer, targets ? targets : L"Button", 255);
    Wh_FreeStringSetting(targets);
}

bool IsTypeAllowed(IUIAutomationElement* pEl) {
    if (!pEl) return false;
    CONTROLTYPEID typeId;
    if (FAILED(pEl->get_CurrentControlType(&typeId))) return false;
    
    if (wcsstr(g_activeSettings.targetBuffer, L"Button") && typeId == UIA_ButtonControlTypeId) return true;
    if (wcsstr(g_activeSettings.targetBuffer, L"ListItem") && typeId == UIA_ListItemControlTypeId) return true;
    if (wcsstr(g_activeSettings.targetBuffer, L"Pane") && typeId == UIA_PaneControlTypeId) return true;
    if (wcsstr(g_activeSettings.targetBuffer, L"Group") && typeId == UIA_GroupControlTypeId) return true;
    if (wcsstr(g_activeSettings.targetBuffer, L"Image") && typeId == UIA_ImageControlTypeId) return true;
    if (wcsstr(g_activeSettings.targetBuffer, L"Text") && typeId == UIA_TextControlTypeId) return true;
    return false;
}

float GetDistToRoundedRect(float x, float y, float w, float h, float r) {
    float dx = fmaxf(fmaxf(r - x, x - (w - r)), 0.0f);
    float dy = fmaxf(fmaxf(r - y, y - (h - r)), 0.0f);
    return sqrtf(dx * dx + dy * dy);
}

void RenderShine(int w, int h, float masterOpacity, POINT pt, RECT rect) {
    if (w <= 0 || h <= 0) return;

    // Fix: Recreate cache if size changes to prevent "Black Squares"
    if (w != g_cache.w || h != g_cache.h) {
        if (g_cache.hdcMem) { 
            DeleteObject(g_cache.hBitmap); 
            DeleteDC(g_cache.hdcMem); 
        }
        HDC hdc = GetDC(NULL);
        g_cache.hdcMem = CreateCompatibleDC(hdc);
        
        BITMAPINFO bmi = {0};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = w;
        bmi.bmiHeader.biHeight = -h; // Top-down
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        g_cache.hBitmap = CreateDIBSection(g_cache.hdcMem, &bmi, DIB_RGB_COLORS, &g_cache.pBits, NULL, 0);
        SelectObject(g_cache.hdcMem, g_cache.hBitmap);
        g_cache.w = w; g_cache.h = h;
        ReleaseDC(NULL, hdc);
    }

    // Force zero-clear the buffer
    memset(g_cache.pBits, 0, w * h * 4);
    
    UINT32* pixels = (UINT32*)g_cache.pBits;
    BYTE rt = GetRValue(g_activeSettings.color);
    BYTE gt = GetGValue(g_activeSettings.color);
    BYTE bt = GetBValue(g_activeSettings.color);
    
    float rx = (float)(pt.x - rect.left);
    float ry = (float)(pt.y - rect.top);
    float r = (float)g_activeSettings.radius;

    for (int i = 0; i < w * h; i++) {
        float x = (float)(i % w);
        float y = (float)(i / w);
        
        float d = GetDistToRoundedRect(x, y, (float)w, (float)h, r);
        float distSq = (x - rx) * (x - rx) + (y - ry) * (y - ry);
        
        // Spotlight Glow
        float spot = expf(-distSq / fmaxf(1.0f, g_activeSettings.softness));
        float glowVal = spot * (float)g_activeSettings.maxGlow;
        
        // Border Logic
        float borderVal = (d >= (r - g_activeSettings.thickness) && d <= r) ? (float)g_activeSettings.maxBorder : 0.0f;
        
        // Edge Masking
        float mask = (d <= r) ? 1.0f : fmaxf(0.0f, 1.0f - (d - r));
        
        int a = (int)(fmaxf(glowVal, borderVal) * (masterOpacity / 255.0f) * mask);
        if (a > 255) a = 255;
        if (a < 0) a = 0;

        // Premultiplied Alpha (Required for UpdateLayeredWindow)
        pixels[i] = (a << 24) | (((a * rt) / 255) << 16) | (((a * gt) / 255) << 8) | ((a * bt) / 255);
    }

    POINT ps = {0, 0};
    SIZE sz = {w, h};
    BLENDFUNCTION bl = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    UpdateLayeredWindow(g_hGlowWnd, NULL, NULL, &sz, g_cache.hdcMem, &ps, 0, &bl, ULW_ALPHA);
}

void UpdateLogic() {
    POINT pt;
    GetCursorPos(&pt);
    bool found = false;

    if (g_pAutomation) {
        IUIAutomationElement* pEl = NULL;
        if (SUCCEEDED(g_pAutomation->ElementFromPoint(pt, &pEl)) && pEl) {
            IUIAutomationTreeWalker* pW = NULL;
            g_pAutomation->get_ControlViewWalker(&pW);
            IUIAutomationElement* pChild = NULL;
            if (pW) {
                pW->GetFirstChildElement(pEl, &pChild);
                pW->Release();
            }
            
            IUIAutomationElement* pT = (pChild && IsTypeAllowed(pChild)) ? pChild : pEl;
            if (pT && IsTypeAllowed(pT)) {
                if (SUCCEEDED(pT->get_CurrentBoundingRectangle(&g_targetRect))) {
                    g_targetRect.top += g_activeSettings.mTop;
                    g_targetRect.bottom -= g_activeSettings.mBottom;
                    g_targetRect.left += g_activeSettings.mLeft;
                    g_targetRect.right -= g_activeSettings.mRight;
                    found = true;
                }
            }
            if (pChild) pChild->Release();
            pEl->Release();
        }
    }

    // Smoother opacity transition
    g_opacitySmooth += ((found ? 255.0f : 0.0f) - g_opacitySmooth) * g_activeSettings.animSpeed;

    if (g_opacitySmooth > 2.0f) {
        int w = g_targetRect.right - g_targetRect.left;
        int h = g_targetRect.bottom - g_targetRect.top;
        if (w > 0 && h > 0) {
            RenderShine(w, h, g_opacitySmooth, pt, g_targetRect);
            SetWindowPos(g_hGlowWnd, HWND_TOPMOST, g_targetRect.left, g_targetRect.top, w, h, SWP_NOACTIVATE | SWP_SHOWWINDOW);
        }
    } else {
        ShowWindow(g_hGlowWnd, SW_HIDE);
    }
}

// --- Windows Integration ---
LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (m == WM_TIMER) { UpdateLogic(); return 0; }
    if (m == WM_DESTROY) { PostQuitMessage(0); return 0; }
    return DefWindowProc(h, m, w, l);
}

DWORD WINAPI ThreadProc(LPVOID lp) {
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    CoCreateInstance(CLSID_CUIAutomation, NULL, CLSCTX_INPROC_SERVER, IID_IUIAutomation, (void**)&g_pAutomation);
    
    WNDCLASSEXW wc = {sizeof(wc), 0, WndProc, 0, 0, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"FluentGlowWnd", NULL};
    RegisterClassExW(&wc);
    
    g_hGlowWnd = CreateWindowExW(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, 
                                L"FluentGlowWnd", L"", WS_POPUP, 0, 0, 0, 0, NULL, NULL, wc.hInstance, NULL);
    
    SetTimer(g_hGlowWnd, 1, 16, NULL); // ~60fps logic
    
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    if (g_pAutomation) g_pAutomation->Release();
    CoUninitialize();
    return 0;
}

void WINAPI EntryPoint_Hook() { ExitThread(0); }

BOOL Wh_ModInit() {
    LoadSettings();
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLine(), &argc);
    for (int i = 0; i < argc - 1; i++) {
        if (wcscmp(argv[i], L"-tool-mod") == 0 && wcscmp(argv[i+1], WH_MOD_ID) == 0) g_isToolProcess = true;
    }
    LocalFree(argv);

    if (g_isToolProcess) {
        IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)GetModuleHandle(NULL);
        IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)((BYTE*)dos + dos->e_lfanew);
        Wh_SetFunctionHook((BYTE*)dos + nt->OptionalHeader.AddressOfEntryPoint, (void*)EntryPoint_Hook, NULL);
        CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
    }
    return TRUE;
}

void Wh_ModAfterInit() {
    if (g_isToolProcess) return;
    WCHAR path[MAX_PATH], cmd[MAX_PATH + 128]; 
    GetModuleFileNameW(NULL, path, MAX_PATH);
    swprintf(cmd, MAX_PATH + 128, L"\"%s\" -tool-mod \"%s\"", path, WH_MOD_ID);
    STARTUPINFOW si = {sizeof(si)};
    si.dwFlags = STARTF_FORCEOFFFEEDBACK;
    PROCESS_INFORMATION pi;
    if (CreateProcessW(path, cmd, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}

void Wh_ModSettingsChanged() { LoadSettings(); }
void Wh_ModUninit() { if (g_hGlowWnd) PostMessage(g_hGlowWnd, WM_CLOSE, 0, 0); }
