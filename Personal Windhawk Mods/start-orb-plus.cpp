// ==WindhawkMod==
// @id             start-orb-plus
// @name           Start Orb Plus
// @description    Windows 7 style Start Orb overlay
// @version        1.0.0
// @author         Bbmaster123/Gemini
// @include        explorer.exe
// @architecture   x86-64
// @compilerOptions -lgdiplus -lgdi32 -luser32 -ldwmapi -lcomctl32 -lole32 -loleaut32 -lruntimeobject
// ==/WindhawkMod==
// ==WindhawkModReadme==
/*
![Screenshot](https://raw.githubusercontent.com/bbmaster123/FWFU/refs/heads/main/Assets/start-orb.gif)
![Screenshot](https://raw.githubusercontent.com/bbmaster123/FWFU/refs/heads/main/Assets/start-orb2.gif)

Replaces start button with a custom start button on Windows 10 and 11 using a GDI overlay. Users supply 3 Images:
1. Idle
2. Hover
3. Pressed

## Features
- Animated transition effect similar to windows 7 start orb
- animation speed setting
- X/Y Size and offset adjustments
- Min and Max opacity for animation states
- Option to hide original start button icon. Disable If you would like to use a semi-transparent overlay effect.
- Works in all taskbar orientations  
- Compatible with styler mods and Explorerpatcher 
- Tested on builds ranging from 19045 to 26300
*/
// ==/WindhawkModReadme==
// ==WindhawkModSettings==
/*
- orbNormal: "C:\\Users\\Admin\\Pictures\\orbs\\orbIdle.png"
  $name: Idle
- orbHover: "C:\\Users\\Admin\\Pictures\\orbs\\orbHover.png"
  $name: Hover
- orbPressed: "C:\\Users\\Admin\\Pictures\\orbs\\orbPressed.png"
  $name: Pressed
- orbSizeX: 72
  $name: Width
- orbSizeY: 72
  $name: Height
- offsetX: 0
  $name: Horizontal Offset
- offsetY: 0
  $name: Vertical Offset
- animSpeed: 85
  $name: Animation Speed
- minOpacity: 255
- maxOpacity: 255
- hideDefault: true
  $name: Hide Default Start Button Icon
*/
// ==/WindhawkModSettings==

#include <commctrl.h>
#include <dwmapi.h>
#include <gdiplus.h>
#include <windhawk_utils.h>
#include <windows.h>
#include <atomic>

#undef GetCurrentTime
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Xaml.Automation.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.UI.Xaml.Shapes.h>
#include <winrt/Windows.UI.Xaml.h>

using namespace Gdiplus;
using namespace winrt::Windows::UI::Xaml;

struct {
    int sizeX;
    int sizeY;
    int speed;
    int minOpacity;
    int maxOpacity;
    int offsetX;
    int offsetY;
    bool hideDefault;
} g_settings;

HWND g_hOrbWnd = NULL;
HWND g_hStart = NULL;
HANDLE g_hOrbThread = NULL;

ULONG_PTR g_gdiToken = 0;

Image* g_imgNormal = nullptr;
Image* g_imgHover = nullptr;
Image* g_imgPressed = nullptr;

std::wstring g_tempNormal;
std::wstring g_tempHover;
std::wstring g_tempPressed;
uint64_t g_downloadJobId = 0;

float g_fadeAlpha = 0.0f;
int g_state = 0;

bool g_isUnloading = false;

CRITICAL_SECTION g_cs;

// -------------------- WINDOWS 11 START BUTTON HIDING --------------------
bool g_isWindows11 = false;
std::atomic<bool> g_taskbarViewDllLoaded;
std::atomic<bool> g_unloading_win11;
thread_local bool g_inArrangeOverride;

void* CTaskBand_ITaskListWndSite_vftable;
void* CSecondaryTaskBand_ITaskListWndSite_vftable;

using CTaskBand_GetTaskbarHost_t = void*(WINAPI*)(void* pThis, void** result);
CTaskBand_GetTaskbarHost_t CTaskBand_GetTaskbarHost_Original;
void* TaskbarHost_FrameHeight_Original;

using CSecondaryTaskBand_GetTaskbarHost_t = void*(WINAPI*)(void* pThis,
                                                           void** result);
CSecondaryTaskBand_GetTaskbarHost_t CSecondaryTaskBand_GetTaskbarHost_Original;

using std__Ref_count_base__Decref_t = void(WINAPI*)(void* pThis);
std__Ref_count_base__Decref_t std__Ref_count_base__Decref_Original;

template <typename F>
FrameworkElement EnumChildElements(FrameworkElement element, F callback) {
    int count = Media::VisualTreeHelper::GetChildrenCount(element);
    for (int i = 0; i < count; i++) {
        auto child = Media::VisualTreeHelper::GetChild(element, i)
                         .try_as<FrameworkElement>();
        if (child && callback(child))
            return child;
    }
    return nullptr;
}

FrameworkElement FindChildByName(FrameworkElement element, PCWSTR name) {
    return EnumChildElements(element, [name](FrameworkElement child) {
        return child.Name() == name;
    });
}

void CollapseStartButtonIcon(FrameworkElement element, bool hide) {
    if (!element)
        return;

    int count = Media::VisualTreeHelper::GetChildrenCount(element);
    for (int i = 0; i < count; i++) {
        auto child = Media::VisualTreeHelper::GetChild(element, i)
                         .try_as<FrameworkElement>();
        if (child) {
            child.Opacity(hide ? 0.0 : 1.0);
        }
    }
}

bool ApplyStyle(XamlRoot xamlRoot) {
    FrameworkElement child = xamlRoot.Content().try_as<FrameworkElement>();
    if (!child ||
        !(child = EnumChildElements(child,
                                    [](FrameworkElement c) {
                                        return winrt::get_class_name(c) ==
                                               L"Taskbar.TaskbarFrame";
                                    })) ||
        !(child = FindChildByName(child, L"RootGrid")) ||
        !(child = FindChildByName(child, L"TaskbarFrameRepeater")))
        return false;

    auto startButton = EnumChildElements(child, [](FrameworkElement c) {
        return winrt::get_class_name(c) == L"Taskbar.ExperienceToggleButton" &&
               Automation::AutomationProperties::GetAutomationId(c) ==
                   L"StartButton";
    });

    if (startButton) {
        bool hide;
        EnterCriticalSection(&g_cs);
        hide = g_settings.hideDefault;
        LeaveCriticalSection(&g_cs);

        CollapseStartButtonIcon(startButton, hide && !g_unloading_win11);
    }
    return true;
}

XamlRoot XamlRootFromTaskbarHostSharedPtr(void* taskbarHostSharedPtr[2]) {
    if (!taskbarHostSharedPtr[0] && !taskbarHostSharedPtr[1])
        return nullptr;
    size_t offset = 0x48;
    const BYTE* b = (const BYTE*)TaskbarHost_FrameHeight_Original;
    if (b[0] == 0x48 && b[1] == 0x83 && b[2] == 0xEC && b[4] == 0x48 &&
        b[5] == 0x83 && b[6] == 0xC1 && b[7] <= 0x7F)
        offset = b[7];
    auto* unk = *(IUnknown**)((BYTE*)taskbarHostSharedPtr[0] + offset);
    FrameworkElement fe = nullptr;
    unk->QueryInterface(winrt::guid_of<FrameworkElement>(), winrt::put_abi(fe));
    auto result = fe ? fe.XamlRoot() : nullptr;
    std__Ref_count_base__Decref_Original(taskbarHostSharedPtr[1]);
    return result;
}

XamlRoot GetTaskbarXamlRoot(HWND hWnd, bool isSecondary) {
    HWND hTaskSwWnd =
        isSecondary ? (HWND)FindWindowEx(hWnd, nullptr, L"WorkerW", nullptr)
                    : (HWND)GetProp(hWnd, L"TaskbandHWND");
    if (!hTaskSwWnd)
        return nullptr;
    void* vftable = isSecondary ? CSecondaryTaskBand_ITaskListWndSite_vftable
                                : CTaskBand_ITaskListWndSite_vftable;
    void* p = (void*)GetWindowLongPtr(hTaskSwWnd, 0);
    for (int i = 0; *(void**)p != vftable; i++) {
        if (i == 20)
            return nullptr;
        p = (void**)p + 1;
    }
    void* sharedPtr[2]{};
    if (isSecondary)
        CSecondaryTaskBand_GetTaskbarHost_Original(p, sharedPtr);
    else
        CTaskBand_GetTaskbarHost_Original(p, sharedPtr);
    return XamlRootFromTaskbarHostSharedPtr(sharedPtr);
}

HWND FindCurrentProcessTaskbarWnd() {
    HWND hWnd = nullptr;
    while ((hWnd = FindWindowEx(nullptr, hWnd, L"Shell_TrayWnd", nullptr))) {
        DWORD pid;
        GetWindowThreadProcessId(hWnd, &pid);
        if (pid == GetCurrentProcessId())
            return hWnd;
    }
    return nullptr;
}

void ApplySettingsFromTaskbarThread() {
    EnumThreadWindows(
        GetCurrentThreadId(),
        [](HWND hWnd, LPARAM) -> BOOL {
            WCHAR cls[32];
            if (GetClassName(hWnd, cls, ARRAYSIZE(cls)) == 0)
                return TRUE;
            XamlRoot xamlRoot = nullptr;
            if (_wcsicmp(cls, L"Shell_TrayWnd") == 0)
                xamlRoot = GetTaskbarXamlRoot(hWnd, false);
            else if (_wcsicmp(cls, L"Shell_SecondaryTrayWnd") == 0)
                xamlRoot = GetTaskbarXamlRoot(hWnd, true);
            if (xamlRoot)
                ApplyStyle(xamlRoot);
            return TRUE;
        },
        0);
}

void ApplySettings(HWND hTaskbarWnd) {
    static const UINT msg = RegisterWindowMessage(
        L"Windhawk_RunFromWindowThread_start-orb-restorer");
    DWORD threadId = GetWindowThreadProcessId(hTaskbarWnd, nullptr);
    if (!threadId)
        return;
    if (threadId == GetCurrentThreadId()) {
        ApplySettingsFromTaskbarThread();
        return;
    }
    HHOOK hook = SetWindowsHookEx(
        WH_CALLWNDPROC,
        [](int nCode, WPARAM wParam, LPARAM lParam) -> LRESULT {
            if (nCode == HC_ACTION &&
                ((const CWPSTRUCT*)lParam)->message == msg)
                ApplySettingsFromTaskbarThread();
            return CallNextHookEx(nullptr, nCode, wParam, lParam);
        },
        nullptr, threadId);
    if (!hook)
        return;
    SendMessage(hTaskbarWnd, msg, 0, 0);
    UnhookWindowsHookEx(hook);
}

using IUIElement_Arrange_t =
    HRESULT(WINAPI*)(void* pThis, winrt::Windows::Foundation::Rect rect);
IUIElement_Arrange_t IUIElement_Arrange_Original;

HRESULT WINAPI IUIElement_Arrange_Hook(void* pThis,
                                       winrt::Windows::Foundation::Rect rect) {
    auto original = [=] { return IUIElement_Arrange_Original(pThis, rect); };
    if (!g_inArrangeOverride || g_unloading_win11)
        return original();
    FrameworkElement element = nullptr;
    ((IUnknown*)pThis)
        ->QueryInterface(winrt::guid_of<FrameworkElement>(),
                         winrt::put_abi(element));
    if (!element)
        return original();
    if (winrt::get_class_name(element) == L"Taskbar.ExperienceToggleButton" &&
        Automation::AutomationProperties::GetAutomationId(element) ==
            L"StartButton") {
        bool hide;
        EnterCriticalSection(&g_cs);
        hide = g_settings.hideDefault;
        LeaveCriticalSection(&g_cs);

        CollapseStartButtonIcon(element, hide);
    }
    return IUIElement_Arrange_Original(pThis, rect);
}

using TaskbarCollapsibleLayoutXamlTraits_ArrangeOverride_t =
    HRESULT(WINAPI*)(void* pThis,
                     void* context,
                     winrt::Windows::Foundation::Size size,
                     winrt::Windows::Foundation::Size* resultSize);
TaskbarCollapsibleLayoutXamlTraits_ArrangeOverride_t
    TaskbarCollapsibleLayoutXamlTraits_ArrangeOverride_Original;

HRESULT WINAPI TaskbarCollapsibleLayoutXamlTraits_ArrangeOverride_Hook(
    void* pThis,
    void* context,
    winrt::Windows::Foundation::Size size,
    winrt::Windows::Foundation::Size* resultSize) {
    [[maybe_unused]] static bool hooked = [] {
        Shapes::Rectangle rectangle;
        IUIElement element = rectangle;
        void** vtable = *(void***)winrt::get_abi(element);
        WindhawkUtils::SetFunctionHook((IUIElement_Arrange_t)vtable[92],
                                       IUIElement_Arrange_Hook,
                                       &IUIElement_Arrange_Original);
        Wh_ApplyHookOperations();
        return true;
    }();
    g_inArrangeOverride = true;
    HRESULT ret = TaskbarCollapsibleLayoutXamlTraits_ArrangeOverride_Original(
        pThis, context, size, resultSize);
    g_inArrangeOverride = false;
    return ret;
}

bool HookTaskbarDllSymbols() {
    HMODULE module =
        LoadLibraryEx(L"taskbar.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!module)
        return false;
    WindhawkUtils::SYMBOL_HOOK taskbarDllHooks[] = {
        {{LR"(const CTaskBand::`vftable'{for `ITaskListWndSite'})"},
         &CTaskBand_ITaskListWndSite_vftable},
        {{LR"(const CSecondaryTaskBand::`vftable'{for `ITaskListWndSite'})"},
         &CSecondaryTaskBand_ITaskListWndSite_vftable},
        {{LR"(public: virtual class std::shared_ptr<class TaskbarHost> __cdecl CTaskBand::GetTaskbarHost(void)const )"},
         &CTaskBand_GetTaskbarHost_Original},
        {{LR"(public: int __cdecl TaskbarHost::FrameHeight(void)const )"},
         &TaskbarHost_FrameHeight_Original},
        {{LR"(public: virtual class std::shared_ptr<class TaskbarHost> __cdecl CSecondaryTaskBand::GetTaskbarHost(void)const )"},
         &CSecondaryTaskBand_GetTaskbarHost_Original},
        {{LR"(public: void __cdecl std::_Ref_count_base::_Decref(void))"},
         &std__Ref_count_base__Decref_Original},
    };
    return HookSymbols(module, taskbarDllHooks, ARRAYSIZE(taskbarDllHooks));
}

bool HookTaskbarViewDllSymbols(HMODULE module) {
    WindhawkUtils::SYMBOL_HOOK hooks[] = {
        {{LR"(public: virtual int __cdecl winrt::impl::produce<struct winrt::Taskbar::implementation::TaskbarCollapsibleLayout,struct winrt::Microsoft::UI::Xaml::Controls::IVirtualizingLayoutOverrides>::ArrangeOverride(void *,struct winrt::Windows::Foundation::Size,struct winrt::Windows::Foundation::Size *))"},
         &TaskbarCollapsibleLayoutXamlTraits_ArrangeOverride_Original,
         TaskbarCollapsibleLayoutXamlTraits_ArrangeOverride_Hook},
    };
    return HookSymbols(module, hooks, ARRAYSIZE(hooks));
}

HMODULE GetTaskbarViewModuleHandle() {
    HMODULE m = GetModuleHandle(L"Taskbar.View.dll");
    return m ? m : GetModuleHandle(L"ExplorerExtensions.dll");
}

using LoadLibraryExW_t = decltype(&LoadLibraryExW);
LoadLibraryExW_t LoadLibraryExW_Original;

HMODULE WINAPI LoadLibraryExW_Hook(LPCWSTR lpLibFileName,
                                   HANDLE hFile,
                                   DWORD dwFlags) {
    HMODULE module = LoadLibraryExW_Original(lpLibFileName, hFile, dwFlags);
    if (module && !g_taskbarViewDllLoaded &&
        GetTaskbarViewModuleHandle() == module &&
        !g_taskbarViewDllLoaded.exchange(true)) {
        if (HookTaskbarViewDllSymbols(module))
            Wh_ApplyHookOperations();
    }
    return module;
}

// -------------------- IMAGE --------------------

bool IsURL(PCWSTR path, std::wstring& outNormalizedURL) {
    if (!path || !path[0])
        return false;

    if (_wcsnicmp(path, L"http://", 7) == 0 ||
        _wcsnicmp(path, L"https://", 8) == 0) {
        outNormalizedURL = path;
        return true;
    }

    if (wcsstr(path, L"://") != nullptr) {
        outNormalizedURL = path;
        return true;
    }

    if (wcsncmp(path, L"//", 2) == 0) {
        outNormalizedURL = L"https:" + std::wstring(path);
        return true;
    }

    if (_wcsnicmp(path, L"www.", 4) == 0) {
        outNormalizedURL = L"https://" + std::wstring(path);
        return true;
    }

    if (path[1] != L':' && wcschr(path, L'\\') == nullptr) {
        PCWSTR dot = wcschr(path, L'.');
        PCWSTR slash = wcschr(path, L'/');
        if (dot && slash && dot < slash) {
            outNormalizedURL = L"https://" + std::wstring(path);
            return true;
        } else if (slash != nullptr &&
                   (wcsstr(path, L".com") || wcsstr(path, L".net") ||
                    wcsstr(path, L".org") || wcsstr(path, L".io") ||
                    wcsstr(path, L".co"))) {
            outNormalizedURL = L"https://" + std::wstring(path);
            return true;
        }
    }

    return false;
}

struct DownloadJob {
    uint64_t jobId;
    std::wstring urlNormal;
    std::wstring urlHover;
    std::wstring urlPressed;
};

bool DownloadURLToTempFile(PCWSTR url, std::wstring& outTempFile) {
    HMODULE hUrlmon = LoadLibraryW(L"urlmon.dll");
    if (!hUrlmon)
        return false;

    typedef HRESULT(WINAPI * URLDownloadToFileW_t)(LPUNKNOWN, LPCWSTR, LPCWSTR,
                                                   DWORD, LPBINDSTATUSCALLBACK);
    auto pURLDownloadToFileW =
        (URLDownloadToFileW_t)GetProcAddress(hUrlmon, "URLDownloadToFileW");
    if (!pURLDownloadToFileW) {
        FreeLibrary(hUrlmon);
        return false;
    }

    WCHAR tempPath[MAX_PATH];
    WCHAR tempFile[MAX_PATH];
    if (GetTempPathW(MAX_PATH, tempPath) == 0) {
        FreeLibrary(hUrlmon);
        return false;
    }

    if (GetTempFileNameW(tempPath, L"orb", 0, tempFile) == 0) {
        FreeLibrary(hUrlmon);
        return false;
    }

    DeleteFileW(tempFile);

    HRESULT hr = pURLDownloadToFileW(NULL, url, tempFile, 0, NULL);
    FreeLibrary(hUrlmon);

    if (SUCCEEDED(hr)) {
        outTempFile = tempFile;
        return true;
    }
    return false;
}

Image* LoadOne(PCWSTR path, std::wstring& outTempPath) {
    if (!path || !path[0])
        return nullptr;

    PCWSTR loadPath = path;
    std::wstring normalizedURL;
    if (IsURL(path, normalizedURL)) {
        if (DownloadURLToTempFile(normalizedURL.c_str(), outTempPath)) {
            loadPath = outTempPath.c_str();
        } else {
            return nullptr;
        }
    }

    Image* img = Image::FromFile(loadPath);
    if (!img || img->GetLastStatus() != Ok) {
        delete img;
        if (!outTempPath.empty()) {
            DeleteFileW(outTempPath.c_str());
            outTempPath.clear();
        }
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

    if (!g_tempNormal.empty()) {
        DeleteFileW(g_tempNormal.c_str());
        g_tempNormal.clear();
    }
    if (!g_tempHover.empty()) {
        DeleteFileW(g_tempHover.c_str());
        g_tempHover.clear();
    }
    if (!g_tempPressed.empty()) {
        DeleteFileW(g_tempPressed.c_str());
        g_tempPressed.clear();
    }
    LeaveCriticalSection(&g_cs);
}

DWORD WINAPI DownloadThreadProc(LPVOID lpParam) {
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    DownloadJob* job = (DownloadJob*)lpParam;
    if (!job) {
        CoUninitialize();
        return 0;
    }

    std::wstring tempNormal;
    std::wstring tempHover;
    std::wstring tempPressed;

    Image* imgNormal = LoadOne(job->urlNormal.c_str(), tempNormal);
    Image* imgHover = LoadOne(job->urlHover.c_str(), tempHover);
    Image* imgPressed = LoadOne(job->urlPressed.c_str(), tempPressed);

    EnterCriticalSection(&g_cs);
    if (job->jobId != g_downloadJobId) {
        delete imgNormal;
        delete imgHover;
        delete imgPressed;
        if (!tempNormal.empty()) {
            DeleteFileW(tempNormal.c_str());
        }
        if (!tempHover.empty()) {
            DeleteFileW(tempHover.c_str());
        }
        if (!tempPressed.empty()) {
            DeleteFileW(tempPressed.c_str());
        }
        LeaveCriticalSection(&g_cs);
        delete job;
        CoUninitialize();
        return 0;
    }

    if (g_imgNormal) {
        delete g_imgNormal;
        g_imgNormal = nullptr;
    }
    if (g_imgHover) {
        delete g_imgHover;
        g_imgHover = nullptr;
    }
    if (g_imgPressed) {
        delete g_imgPressed;
        g_imgPressed = nullptr;
    }

    if (!g_tempNormal.empty()) {
        DeleteFileW(g_tempNormal.c_str());
        g_tempNormal.clear();
    }
    if (!g_tempHover.empty()) {
        DeleteFileW(g_tempHover.c_str());
        g_tempHover.clear();
    }
    if (!g_tempPressed.empty()) {
        DeleteFileW(g_tempPressed.c_str());
        g_tempPressed.clear();
    }

    g_imgNormal = imgNormal;
    g_imgHover = imgHover;
    g_imgPressed = imgPressed;

    g_tempNormal = tempNormal;
    g_tempHover = tempHover;
    g_tempPressed = tempPressed;
    LeaveCriticalSection(&g_cs);

    delete job;

    if (g_hOrbWnd) {
        PostMessage(g_hOrbWnd, WM_USER + 1, 0, 0);
    }

    CoUninitialize();
    return 0;
}

void LoadImages() {
    EnterCriticalSection(&g_cs);
    g_downloadJobId++;
    uint64_t currentJobId = g_downloadJobId;
    PCWSTR pathNormal = Wh_GetStringSetting(L"orbNormal");
    PCWSTR pathHover = Wh_GetStringSetting(L"orbHover");
    PCWSTR pathPressed = Wh_GetStringSetting(L"orbPressed");

    DownloadJob* job = new DownloadJob();
    job->jobId = currentJobId;

    job->urlNormal = (pathNormal && pathNormal[0])
                         ? pathNormal
                         : L"https://raw.githubusercontent.com/ramensoftware/"
                           L"windows-11-taskbar-styling-guide/refs/heads/main/"
                           L"Themes/Windows7/ThemeResources/orbNormal.png";

    job->urlHover = (pathHover && pathHover[0])
                        ? pathHover
                        : L"https://raw.githubusercontent.com/ramensoftware/"
                          L"windows-11-taskbar-styling-guide/refs/heads/main/"
                          L"Themes/Windows7/ThemeResources/orbHover.png";

    job->urlPressed = (pathPressed && pathPressed[0])
                          ? pathPressed
                          : L"https://raw.githubusercontent.com/ramensoftware/"
                            L"windows-11-taskbar-styling-guide/refs/heads/main/"
                            L"Themes/Windows7/ThemeResources/orbPressed.png";

    Wh_FreeStringSetting(pathNormal);
    Wh_FreeStringSetting(pathHover);
    Wh_FreeStringSetting(pathPressed);
    LeaveCriticalSection(&g_cs);

    HANDLE hThread = CreateThread(NULL, 0, DownloadThreadProc, job, 0, NULL);
    if (hThread) {
        CloseHandle(hThread);
    } else {
        delete job;
    }
}

// -------------------- START BUTTON --------------------

HWND FindStartButton() {
    HWND hTask = FindWindow(L"Shell_TrayWnd", NULL);
    if (!hTask)
        return NULL;

    HWND child = NULL;
    while ((child = FindWindowEx(hTask, child, NULL, NULL))) {
        wchar_t cls[64];
        GetClassName(child, cls, 64);

        if (wcscmp(cls, L"Start") == 0 || wcscmp(cls, L"Button") == 0) {
            return child;
        }
    }
    return NULL;
}

// -------------------- POSITION --------------------

#ifndef DWMWA_EXCLUDED_FROM_PEEK
#define DWMWA_EXCLUDED_FROM_PEEK 12
#endif

bool IsStartMenuVisible() {
    auto isVisible = [](HWND hwnd) -> bool {
        if (!hwnd || !IsWindowVisible(hwnd))
            return false;
        BOOL cloaked = FALSE;
        if (SUCCEEDED(DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &cloaked,
                                            sizeof(cloaked))) &&
            cloaked) {
            return false;
        }
        return true;
    };

    if (isVisible(FindWindow(L"Windows.UI.Core.CoreWindow", L"Start")))
        return true;
    if (isVisible(FindWindow(L"Windows.UI.Core.CoreWindow",
                             L"StartMenuExperienceHost")))
        return true;
    if (isVisible(FindWindow(L"ClassicShell.CMenuWin", NULL)))
        return true;
    if (isVisible(FindWindow(L"OpenShell.CMenuWin", NULL)))
        return true;

    return false;
}

void SetWindowThemeHelper(HWND hWnd,
                          LPCWSTR pszSubAppName,
                          LPCWSTR pszSubIdList) {
    HMODULE hUxTheme = GetModuleHandle(L"uxtheme.dll");
    if (!hUxTheme) {
        hUxTheme = LoadLibrary(L"uxtheme.dll");
    }
    if (hUxTheme) {
        typedef HRESULT(WINAPI * SetWindowTheme_t)(HWND, LPCWSTR, LPCWSTR);
        auto pSetWindowTheme =
            (SetWindowTheme_t)GetProcAddress(hUxTheme, "SetWindowTheme");
        if (pSetWindowTheme) {
            pSetWindowTheme(hWnd, pszSubAppName, pszSubIdList);
        }
    }
}

void UpdateStartButtonVisibility() {
    if (g_isUnloading)
        return;

    if (g_isWindows11 && g_taskbarViewDllLoaded) {
        HWND hTaskbarWnd = FindCurrentProcessTaskbarWnd();
        if (hTaskbarWnd)
            ApplySettings(hTaskbarWnd);
        return;  // Return early on native Windows 11 to avoid breaking
                 // clickability
    }

    if (!g_hStart || !IsWindow(g_hStart))
        return;

    bool hide;
    EnterCriticalSection(&g_cs);
    hide = g_settings.hideDefault;
    LeaveCriticalSection(&g_cs);

    if (hide) {
        SetWindowThemeHelper(g_hStart, L"", L"");
        EnumChildWindows(
            g_hStart,
            [](HWND hChild, LPARAM) -> BOOL {
                ShowWindow(hChild, SW_HIDE);
                return TRUE;
            },
            0);
    } else {
        SetWindowThemeHelper(g_hStart, NULL, NULL);
        EnumChildWindows(
            g_hStart,
            [](HWND hChild, LPARAM) -> BOOL {
                ShowWindow(hChild, SW_SHOW);
                return TRUE;
            },
            0);
    }

    // Fully redraw the window to apply changes
    RedrawWindow(g_hStart, NULL, NULL,
                 RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
}

LRESULT CALLBACK StartButtonSubclassProc(HWND hWnd,
                                         UINT uMsg,
                                         WPARAM wParam,
                                         LPARAM lParam,
                                         UINT_PTR uIdSubclass,
                                         DWORD_PTR dwRefData) {
    static bool s_eatNextLButtonUp = false;

    switch (uMsg) {
        case WM_MOUSEACTIVATE: {
            if (IsStartMenuVisible()) {
                s_eatNextLButtonUp = true;
                return MA_NOACTIVATEANDEAT;
            }
            break;
        }
        case WM_LBUTTONDOWN: {
            if (IsStartMenuVisible()) {
                s_eatNextLButtonUp = true;
                return 0;
            }
            break;
        }
        case WM_LBUTTONUP: {
            if (s_eatNextLButtonUp) {
                s_eatNextLButtonUp = false;
                return 0;
            }
            break;
        }
        case WM_PAINT: {
            bool hide;
            EnterCriticalSection(&g_cs);
            hide = g_settings.hideDefault;
            LeaveCriticalSection(&g_cs);

            if (hide) {
                PAINTSTRUCT ps;
                BeginPaint(hWnd, &ps);
                EndPaint(hWnd, &ps);
                return 0;
            }
            break;
        }
        case WM_ERASEBKGND: {
            bool hide;
            EnterCriticalSection(&g_cs);
            hide = g_settings.hideDefault;
            LeaveCriticalSection(&g_cs);

            if (hide) {
                return TRUE;  // indicate background erased/painted (drawn
                              // nothing)
            }
            break;
        }
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

void PositionOrb() {
    if (g_isUnloading || !g_hOrbWnd)
        return;

    HWND hTask = GetParent(g_hOrbWnd);
    if (!hTask) {
        hTask = FindWindow(L"Shell_TrayWnd", NULL);
    }
    if (!hTask)
        return;

    if (!g_hStart || !IsWindow(g_hStart)) {
        g_hStart = FindStartButton();
        if (g_hStart) {
            SetWindowSubclass(g_hStart, StartButtonSubclassProc, 1, 0);
            UpdateStartButtonVisibility();
        }
    }

    if (!g_hStart)
        return;

    // Always keep the original start button shown so it remains clickable and
    // hit-testable
    ShowWindow(g_hStart, SW_SHOW);

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

    int sizeX, sizeY, offsetX, offsetY;
    EnterCriticalSection(&g_cs);
    sizeX = g_settings.sizeX;
    sizeY = g_settings.sizeY;
    offsetX = g_settings.offsetX;
    offsetY = g_settings.offsetY;
    LeaveCriticalSection(&g_cs);

    // Convert start button screen coordinates to taskbar client coordinates
    POINT pt = {rc.left, rc.top};
    ScreenToClient(hTask, &pt);

    int x = pt.x + ((rc.right - rc.left) - sizeX) / 2 + offsetX;
    int y = pt.y + ((rc.bottom - rc.top) - sizeY) / 2 + offsetY;

    // Use HWND_TOP to keep the child window at the top of the taskbar child
    // z-order
    SetWindowPos(g_hOrbWnd, HWND_TOP, x, y, sizeX, sizeY,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

// -------------------- DRAW --------------------

void UpdateOrbDisplay(HWND hwnd) {
    if (g_isUnloading || !hwnd)
        return;

    EnterCriticalSection(&g_cs);

    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = g_settings.sizeX;
    bmi.bmiHeader.biHeight = -g_settings.sizeY;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;

    void* pBits = NULL;
    HBITMAP hBitmap =
        CreateDIBSection(hdcMem, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);

    if (hBitmap) {
        HBITMAP old = (HBITMAP)SelectObject(hdcMem, hBitmap);

        Graphics g(hdcMem);
        g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
        g.Clear(Color(0, 0, 0, 0));

        RectF rect(0, 0, (REAL)g_settings.sizeX, (REAL)g_settings.sizeY);

        Image* img = g_imgNormal;

        if (g_state == 2 && g_imgPressed)
            img = g_imgPressed;

        g.DrawImage(img, rect);

        if (g_imgHover && g_fadeAlpha > 0.001f && g_state != 2) {
            ImageAttributes attr;
            ColorMatrix matrix = {1,           0, 0, 0, 0, 0, 1, 0, 0,
                                  0,           0, 0, 1, 0, 0, 0, 0, 0,
                                  g_fadeAlpha, 0, 0, 0, 0, 0, 1};
            attr.SetColorMatrix(&matrix);

            g.DrawImage(g_imgHover, rect, 0, 0, (REAL)g_imgHover->GetWidth(),
                        (REAL)g_imgHover->GetHeight(), UnitPixel, &attr);
        }

        int alpha = (int)(g_settings.minOpacity +
                          (g_settings.maxOpacity - g_settings.minOpacity) *
                              g_fadeAlpha);

        BLENDFUNCTION blend = {AC_SRC_OVER, 0, (BYTE)alpha, AC_SRC_ALPHA};

        POINT pt = {0, 0};
        SIZE sz = {g_settings.sizeX, g_settings.sizeY};

        UpdateLayeredWindow(hwnd, hdcScreen, NULL, &sz, hdcMem, &pt, 0, &blend,
                            ULW_ALPHA);

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

    int sizeX, sizeY;
    EnterCriticalSection(&g_cs);
    sizeX = g_settings.sizeX;
    sizeY = g_settings.sizeY;
    LeaveCriticalSection(&g_cs);

    // Wait up to 5 seconds for Shell_TrayWnd to be available
    HWND hTask = NULL;
    for (int i = 0; i < 100; i++) {
        hTask = FindWindow(L"Shell_TrayWnd", NULL);
        if (hTask)
            break;
        Sleep(50);
    }

    g_hOrbWnd =
        CreateWindowEx(WS_EX_LAYERED | WS_EX_TRANSPARENT, wc.lpszClassName,
                       NULL, WS_CHILD | WS_VISIBLE, 0, 0, sizeX, sizeY, hTask,
                       NULL, wc.hInstance, NULL);

    if (g_hOrbWnd) {
        Wh_Log(
            L"Orb window successfully created as WS_CHILD: %p (parent hTask: "
            L"%p)",
            g_hOrbWnd, hTask);

        BOOL exclude = TRUE;
        DwmSetWindowAttribute(g_hOrbWnd, DWMWA_EXCLUDED_FROM_PEEK, &exclude,
                              sizeof(exclude));

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
                    if (g_fadeAlpha > target)
                        g_fadeAlpha = target;
                } else if (g_fadeAlpha > target) {
                    g_fadeAlpha -= step;
                    if (g_fadeAlpha < target)
                        g_fadeAlpha = target;
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

                // 2. Check if cursor is over the original start button
                // (fallback)
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

        case 0x031B: {  // WM_DWMCLOAKEDCHANGED
            if (!g_isUnloading) {
                BOOL cloaked = FALSE;
#ifndef DWMWA_CLOAKED
#define DWMWA_CLOAKED 14
#endif
                if (SUCCEEDED(DwmGetWindowAttribute(
                        hwnd, DWMWA_CLOAKED, &cloaked, sizeof(cloaked))) &&
                    cloaked) {
                    PostMessage(hwnd, WM_USER + 2, 0, 0);
                }
            }
            break;
        }

        case WM_USER + 1:  // WM_SETTINGS_CHANGED
            PositionOrb();
            if (g_hStart && IsWindow(g_hStart)) {
                InvalidateRect(g_hStart, NULL, TRUE);
                UpdateWindow(g_hStart);
            }
            UpdateOrbDisplay(hwnd);
            return 0;

        case WM_USER + 2:  // Force show / restore
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

    g_settings.sizeX = Wh_GetIntSetting(L"orbSizeX");
    g_settings.sizeY = Wh_GetIntSetting(L"orbSizeY");
    g_settings.speed = Wh_GetIntSetting(L"animSpeed");
    g_settings.minOpacity = Wh_GetIntSetting(L"minOpacity");
    g_settings.maxOpacity = Wh_GetIntSetting(L"maxOpacity");
    g_settings.offsetX = Wh_GetIntSetting(L"offsetX");
    g_settings.offsetY = Wh_GetIntSetting(L"offsetY");
    g_settings.hideDefault = Wh_GetIntSetting(L"hideDefault") != 0;

    LoadImages();

    if (HookTaskbarDllSymbols()) {
        g_isWindows11 = true;
        if (HMODULE m = GetTaskbarViewModuleHandle()) {
            g_taskbarViewDllLoaded = true;
            if (HookTaskbarViewDllSymbols(m)) {
                Wh_ApplyHookOperations();
            }
        } else {
            HMODULE kb = GetModuleHandle(L"kernelbase.dll");
            auto pLoadLib =
                (decltype(&LoadLibraryExW))GetProcAddress(kb, "LoadLibraryExW");
            WindhawkUtils::SetFunctionHook(pLoadLib, LoadLibraryExW_Hook,
                                           &LoadLibraryExW_Original);
        }
    }

    // Create the background thread
    g_hOrbThread = CreateThread(NULL, 0, OrbThreadProc, NULL, 0, NULL);

    return TRUE;
}

// -------------------- AFTER INIT --------------------

void Wh_ModAfterInit() {
    if (g_isWindows11) {
        if (!g_taskbarViewDllLoaded) {
            if (HMODULE m = GetTaskbarViewModuleHandle()) {
                if (!g_taskbarViewDllLoaded.exchange(true)) {
                    if (HookTaskbarViewDllSymbols(m)) {
                        Wh_ApplyHookOperations();
                    }
                }
            }
        }
        HWND hTaskbarWnd = FindCurrentProcessTaskbarWnd();
        if (hTaskbarWnd)
            ApplySettings(hTaskbarWnd);
    }
}

// -------------------- BEFORE UNINIT --------------------

void Wh_ModBeforeUninit() {
    g_isUnloading = true;
    if (g_isWindows11) {
        g_unloading_win11 = true;
        HWND hTaskbarWnd = FindCurrentProcessTaskbarWnd();
        if (hTaskbarWnd)
            ApplySettings(hTaskbarWnd);
    }
}

// -------------------- CLEANUP --------------------

void Wh_ModUninit() {
    g_isUnloading = true;

    if (g_isWindows11) {
        g_unloading_win11 = true;
        HWND hTaskbarWnd = FindCurrentProcessTaskbarWnd();
        if (hTaskbarWnd)
            ApplySettings(hTaskbarWnd);
    }

    if (g_hStart && IsWindow(g_hStart)) {
        RemoveWindowSubclass(g_hStart, StartButtonSubclassProc, 1);

        LONG_PTR exStyle = GetWindowLongPtr(g_hStart, GWL_EXSTYLE);
        if (exStyle & WS_EX_LAYERED) {
            SetWindowLongPtr(g_hStart, GWL_EXSTYLE, exStyle & ~WS_EX_LAYERED);
        }

        SetWindowThemeHelper(g_hStart, NULL, NULL);

        EnumChildWindows(
            g_hStart,
            [](HWND hChild, LPARAM) -> BOOL {
                ShowWindow(hChild, SW_SHOW);
                return TRUE;
            },
            0);

        RedrawWindow(g_hStart, NULL, NULL,
                     RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
    }

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
    g_settings.sizeX = Wh_GetIntSetting(L"orbSizeX");
    g_settings.sizeY = Wh_GetIntSetting(L"orbSizeY");
    g_settings.speed = Wh_GetIntSetting(L"animSpeed");
    g_settings.minOpacity = Wh_GetIntSetting(L"minOpacity");
    g_settings.maxOpacity = Wh_GetIntSetting(L"maxOpacity");
    g_settings.offsetX = Wh_GetIntSetting(L"offsetX");
    g_settings.offsetY = Wh_GetIntSetting(L"offsetY");
    g_settings.hideDefault = Wh_GetIntSetting(L"hideDefault") != 0;
    LeaveCriticalSection(&g_cs);

    LoadImages();

    UpdateStartButtonVisibility();

    if (g_hOrbWnd) {
        PostMessage(g_hOrbWnd, WM_USER + 1, 0, 0);
    }
}
