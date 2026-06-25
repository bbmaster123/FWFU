// ==WindhawkMod==
// @id              taskbar-video-injector
// @name            Taskbar Video Injector
// @description     Injects a video player into Taskbar's RootGrid, intended as background video
// @version         0.9
// @author          Bbmaster123 / AI
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -DWINVER=0x0A00 -ldwmapi -lole32 -loleaut32 -lruntimeobject -lshcore -lversion
// ==/WindhawkMod==
// ==WindhawkModReadme==
/*
Based on the code from Lockframe's injector mod, this mod enables video to be played on the taskbar. If no url or filepath is supplied, a royalty-free stock video will be applied as a placeholder. 
Includes experimental injection to start button

- supports local filepath (eg. C:\users\admin\videos\test.mp4)
- looping
- sets video to mute by default
- opacity
- playback speed
- zindex (depth ordering)
- works with taskbar styler and other taskbar mods

limitations:
- start button can only use the same video as taskbar background at the moment.Intended only as proof of concept for now.

*/
// ==/WindhawkModReadme==
// ==WindhawkModSettings==
/*
- videoUrl: "https://cdn.pixabay.com/video/2025/12/21/323513_tiny.mp4"
  $name: Video URL
- loop: true
  $name: Loop Video
- rate: 100
  $name: Playback Speed (100 = 1.0x)
- opacity: 100
  $name: Opacity (0-100)
- zIndex: 1
  $name: Z-Index
- TaskbarHeight: 48
  $name: Reserved Taskbar Height (0 for default)
- injectTaskbar: true
  $name: Inject into Taskbar Background
- injectStartButton: false
  $name: Inject into Start Button
*/
// ==/WindhawkModSettings==

#include <windhawk_utils.h>
#undef GetCurrentTime

#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.UI.Xaml.Media.Animation.h>
#include <winrt/Windows.Media.Core.h>
#include <winrt/Windows.Media.Playback.h>
#include <winrt/base.h>

#include <atomic>
#include <mutex>
#include <string>
#include <vector>

using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Media;

// --- Constants ---
constexpr std::wstring_view c_TargetGridName      = L"RootGrid";
constexpr std::wstring_view c_RootFrameName       = L"Taskbar.TaskbarFrame";
constexpr std::wstring_view c_InjectedControlName = L"CustomVideoInjectedGrid";
constexpr std::wstring_view c_ItemsRepeater       = L"Microsoft.UI.Xaml.Controls.ItemsRepeater";

constexpr std::wstring_view c_StartButtonClass    = L"Taskbar.ExperienceToggleButton";
constexpr std::wstring_view c_StartButtonName     = L"LaunchListButton";

// --- Global State ---
std::atomic<bool> g_taskbarViewDllLoaded = false;
std::atomic<bool> g_injectedOnce{ false };
std::atomic<bool> g_unloading{ false };
std::atomic<bool> g_scanPending = false;
std::atomic<bool> g_applyingSettings{ false };
std::atomic<bool> g_pendingMeasureOverride{ false };
std::atomic<int> g_hookCallCounter{ 0 };

struct TrackedGridRef {
    winrt::weak_ref<Controls::Grid> ref;
    winrt::weak_ref<winrt::Windows::Media::Playback::MediaPlayer> playerRef;
};

std::vector<TrackedGridRef> g_trackedGrids;
std::mutex g_gridMutex;

struct PendingHook {
    winrt::weak_ref<Grid> gridRef;
    winrt::event_token token;
};

std::vector<PendingHook> g_pendingHooks;
std::vector<winrt::weak_ref<FrameworkElement>> g_scannedFrames;
std::mutex g_pendingMutex;

int g_originalTaskbarHeight = 0;
int g_taskbarHeight = 0;
double* double_48_value_Original = nullptr;

// --- Function Pointers ---
using TrayUI__StuckTrayChange_t = void(WINAPI*)(void*);
TrayUI__StuckTrayChange_t TrayUI__StuckTrayChange_Original;

using TrayUI__HandleSettingChange_t = void(WINAPI*)(void*, void*, void*, void*, void*);
TrayUI__HandleSettingChange_t TrayUI__HandleSettingChange_Original;

using TaskbarFrame_MeasureOverride_t = int(WINAPI*)(void*, winrt::Windows::Foundation::Size, winrt::Windows::Foundation::Size*);
TaskbarFrame_MeasureOverride_t TaskbarFrame_MeasureOverride_Original;

using TaskListButton_UpdateVisualStates_t = void(WINAPI*)(void*);
TaskListButton_UpdateVisualStates_t TaskListButton_UpdateVisualStates_Original;

using TaskbarConfiguration_GetFrameSize_t = double(WINAPI*)(int);
TaskbarConfiguration_GetFrameSize_t TaskbarConfiguration_GetFrameSize_Original;

using TaskbarController_UpdateFrameHeight_t = void(WINAPI*)(void*);
TaskbarController_UpdateFrameHeight_t TaskbarController_UpdateFrameHeight_Original;

void* TaskbarController_OnGroupingModeChanged_Original = nullptr;

// --- Helpers ---
void LoadSettings() {
    g_taskbarHeight = Wh_GetIntSetting(L"TaskbarHeight");
}

HWND FindCurrentProcessTaskbarWnd() {
    HWND hTaskbarWnd = nullptr;
    EnumWindows(
        [](HWND hWnd, LPARAM lParam) -> BOOL {
            DWORD dwProcessId;
            WCHAR className[32];
            if (GetWindowThreadProcessId(hWnd, &dwProcessId) &&
                dwProcessId == GetCurrentProcessId() &&
                GetClassName(hWnd, className, ARRAYSIZE(className)) &&
                (_wcsicmp(className, L"Shell_TrayWnd") == 0)) {
                *reinterpret_cast<HWND*>(lParam) = hWnd;
                return FALSE; 
            }
            return TRUE;
        },
        reinterpret_cast<LPARAM>(&hTaskbarWnd));
    return hTaskbarWnd;
}

FrameworkElement GetFrameworkElementFromNative(void* pThis) {
    try {
        void* iUnknownPtr = (void**)pThis + 3;
        winrt::Windows::Foundation::IUnknown iUnknown;
        winrt::copy_from_abi(iUnknown, iUnknownPtr);
        return iUnknown.try_as<FrameworkElement>();
    } catch (...) { return nullptr; }
}

bool ProtectAndMemcpy(DWORD protect, void* dst, const void* src, size_t size) {
    DWORD oldProtect;
    if (!VirtualProtect(dst, size, protect, &oldProtect)) return false;
    memcpy(dst, src, size);
    VirtualProtect(dst, size, oldProtect, &oldProtect);
    return true;
}

void RemoveInjectedFromGrid(Grid grid) {
    if (!grid) return;
    try {
        auto children = grid.Children();
        for (int i = (int)children.Size() - 1; i >= 0; i--) {
            if (auto fe = children.GetAt(i).try_as<FrameworkElement>()) {
                if (fe.Name() == c_InjectedControlName) children.RemoveAt(i);
            }
        }
    } catch (...) {}
}

// --- Video Core ---
void CreateAndInjectVideo(Grid targetGrid, std::wstring_view uniqueName) {
    std::wstring videoUrl = Wh_GetStringSetting(L"videoUrl");
    if (videoUrl.empty()) videoUrl = L"https://cdn.pixabay.com/video/2025/12/21/323513_tiny.mp4";
    if (videoUrl.find(L":\\") != std::wstring::npos) videoUrl = L"file:///" + videoUrl;

    bool loop = Wh_GetIntSetting(L"loop") != 0;
    double rate = static_cast<double>(Wh_GetIntSetting(L"rate")) / 100.0;
    if (rate <= 0.0) rate = 1.0;

    double opacityValue = static_cast<double>(Wh_GetIntSetting(L"opacity")) / 100.0;
    int zIndexValue = Wh_GetIntSetting(L"zIndex");

    Grid videoContainer;
    videoContainer.Name(uniqueName);
    videoContainer.HorizontalAlignment(HorizontalAlignment::Stretch);
    videoContainer.VerticalAlignment(VerticalAlignment::Stretch);
    videoContainer.Opacity(0); // Start invisible for fade-in
    Canvas::SetZIndex(videoContainer, zIndexValue);

    winrt::Windows::Media::Playback::MediaPlayer mediaPlayer;
    mediaPlayer.Source(winrt::Windows::Media::Core::MediaSource::CreateFromUri(winrt::Windows::Foundation::Uri(videoUrl)));
    mediaPlayer.IsLoopingEnabled(loop);
    mediaPlayer.IsMuted(true);
    mediaPlayer.PlaybackRate(rate);

    MediaPlayerElement player;
    player.SetMediaPlayer(mediaPlayer);
    player.Stretch(Stretch::UniformToFill);
    player.IsHitTestVisible(false);

    videoContainer.Children().Append(player);
    targetGrid.Children().Append(videoContainer);
    
    // --- Fade-in Animation ---
    using namespace winrt::Windows::UI::Xaml::Media::Animation;
    DoubleAnimation fadeIn;
    fadeIn.Duration(winrt::Windows::UI::Xaml::Duration{ winrt::Windows::Foundation::TimeSpan{ std::chrono::milliseconds(800) } });
    fadeIn.From(0.0);
    fadeIn.To(opacityValue);
    
    Storyboard storyboard;
    storyboard.Children().Append(fadeIn);
    Storyboard::SetTarget(fadeIn, videoContainer);
    Storyboard::SetTargetProperty(fadeIn, L"Opacity");
    storyboard.Begin();

    mediaPlayer.Play();

    // Store references for tracking/performance
    {
        std::lock_guard<std::mutex> lock(g_gridMutex);
        bool found = false;
        for (auto& t : g_trackedGrids) {
            if (auto g = t.ref.get()) {
                if (winrt::get_abi(g) == winrt::get_abi(targetGrid)) {
                    t.playerRef = winrt::make_weak(mediaPlayer);
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            g_trackedGrids.push_back({ winrt::make_weak(targetGrid), winrt::make_weak(mediaPlayer) });
        }
    }

    // Ensure icons stay above
    int count = VisualTreeHelper::GetChildrenCount(targetGrid);
    for (int i = 0; i < count; i++) {
        if (auto child = VisualTreeHelper::GetChild(targetGrid, i).try_as<FrameworkElement>()) {
            if (winrt::get_class_name(child) == c_ItemsRepeater) {
                Canvas::SetZIndex(child, zIndexValue + 1);
            }
        }
    }
}

void InjectContentIntoGrid(FrameworkElement element, std::wstring_view uniqueName) {
    auto grid = element.try_as<Grid>();
    if (!grid) return;

    RemoveInjectedFromGrid(grid);

    if (element.ActualWidth() > 0 && element.ActualHeight() > 0) {
        CreateAndInjectVideo(grid, uniqueName);
        return;
    }

    auto weakGrid = winrt::make_weak(grid);
    std::wstring name(uniqueName);
    element.SizeChanged([weakGrid, name](auto const&, auto const&) {
        if (auto g = weakGrid.get()) {
            if (g.ActualWidth() > 0 && g.ActualHeight() > 0) {
                CreateAndInjectVideo(g, name);
            }
        }
    });
}

// Walk down from TaskbarFrame to Start button (ExperienceToggleButton#LaunchListButton)
FrameworkElement FindStartButtonInFrame(FrameworkElement frameElem) {
    if (!frameElem) return nullptr;
    int count = VisualTreeHelper::GetChildrenCount(frameElem);
    for (int i = 0; i < count; i++) {
        auto child = VisualTreeHelper::GetChild(frameElem, i).try_as<FrameworkElement>();
        if (!child) continue;
        if (winrt::get_class_name(child) == c_StartButtonClass && child.Name() == c_StartButtonName) return child;
        if (auto found = FindStartButtonInFrame(child)) return found;
    }
    return nullptr;
}

// Find first Grid inside an element (for Start button template)
FrameworkElement FindFirstGridInElement(FrameworkElement root) {
    if (!root) return nullptr;
    if (auto grid = root.try_as<Grid>()) return grid;
    int count = VisualTreeHelper::GetChildrenCount(root);
    for (int i = 0; i < count; i++) {
        auto child = VisualTreeHelper::GetChild(root, i).try_as<FrameworkElement>();
        if (!child) continue;
        if (auto grid = child.try_as<Grid>()) return grid;
        if (auto nested = FindFirstGridInElement(child)) return nested;
    }
    return nullptr;
}

void ScanAndInjectRecursive(FrameworkElement element) {
    if (!element) return;
    
    if (Wh_GetIntSetting(L"injectTaskbar") && element.Name() == c_TargetGridName) {
        InjectContentIntoGrid(element, c_InjectedControlName);
        return;
    }

    int count = VisualTreeHelper::GetChildrenCount(element);
    for (int i = 0; i < count; i++) {
        auto child = VisualTreeHelper::GetChild(element, i).try_as<FrameworkElement>();
        if (child) ScanAndInjectRecursive(child);
    }
}

void ScheduleScanAsync(FrameworkElement startNode) {
    if (!startNode || g_scanPending.exchange(true)) return;
    auto weak = winrt::make_weak(startNode);
    try {
        startNode.Dispatcher().RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::Low, [weak]() {
            g_scanPending = false;
            if (auto node = weak.get()) {
                // Robust injection: walk up to the frame first to ensure we scan the whole taskbar
                FrameworkElement current = node;
                while (current) {
                    if (winrt::get_class_name(current) == c_RootFrameName) {
                        void* abi = winrt::get_abi(current);
                        {
                            std::lock_guard<std::mutex> lock(g_pendingMutex);
                            bool alreadyScanned = false;
                            for (auto& f : g_scannedFrames) {
                                if (auto existing = f.get()) {
                                    if (winrt::get_abi(existing) == abi) {
                                        alreadyScanned = true;
                                        break;
                                    }
                                }
                            }
                            if (alreadyScanned) break;
                            g_scannedFrames.push_back(winrt::make_weak(current));
                        }
                        
                        // Taskbar Background
                        ScanAndInjectRecursive(current);

                        // Start Button
                        if (Wh_GetIntSetting(L"injectStartButton")) {
                            if (auto startBtn = FindStartButtonInFrame(current)) {
                                if (auto grid = FindFirstGridInElement(startBtn)) {
                                    InjectContentIntoGrid(grid, L"StartButtonVideoGrid");
                                }
                            }
                        }
                        return;
                    }
                    auto parent = VisualTreeHelper::GetParent(current);
                    current = parent ? parent.try_as<FrameworkElement>() : nullptr;
                }
                // Fallback: scan from node
                ScanAndInjectRecursive(node);
            }
        });
    } catch (...) { g_scanPending = false; }
}

// --- Height Logic ---
LONG GetTaskbarFrameOffset() {
    static LONG offset = []() -> LONG {
        if (!TaskbarController_OnGroupingModeChanged_Original) return 0;
        const BYTE* p = (const BYTE*)TaskbarController_OnGroupingModeChanged_Original;
#if defined(_M_X64)
        if (p && p[0] == 0x48 && p[1] == 0x83 && p[2] == 0xEC && (p[4] == 0x48 || p[4] == 0x4C) && p[5] == 0x8B && (p[6] & 0xC0) == 0x80) {
            return *(LONG*)(p + 7);
        }
#endif
        return 0;
    }();
    return offset;
}

void WINAPI TrayUI__HandleSettingChange_Hook(void* pThis, void* p1, void* p2, void* p3, void* p4) {
    TrayUI__HandleSettingChange_Original(pThis, p1, p2, p3, p4);
    if (g_applyingSettings && TrayUI__StuckTrayChange_Original) {
        TrayUI__StuckTrayChange_Original(pThis);
    }
}

int WINAPI TaskbarFrame_MeasureOverride_Hook(void* pThis, winrt::Windows::Foundation::Size size, winrt::Windows::Foundation::Size* resultSize) {
    g_hookCallCounter++;
    int ret = TaskbarFrame_MeasureOverride_Original(pThis, size, resultSize);
    g_pendingMeasureOverride = false;
    g_hookCallCounter--;
    return ret;
}

void WINAPI TaskbarController_UpdateFrameHeight_Hook(void* pThis) {
    LONG offset = GetTaskbarFrameOffset();
    if (!offset) {
        TaskbarController_UpdateFrameHeight_Original(pThis);
        return;
    }
    void* taskbarFrame = *(void**)((BYTE*)pThis + offset);
    if (taskbarFrame) {
        FrameworkElement frameElem = nullptr;
        ((IUnknown**)taskbarFrame)[1]->QueryInterface(winrt::guid_of<FrameworkElement>(), winrt::put_abi(frameElem));
        if (frameElem) {
            TaskbarController_UpdateFrameHeight_Original(pThis);
            auto parent = VisualTreeHelper::GetParent(frameElem).try_as<FrameworkElement>();
            if (parent && parent.Height() != frameElem.Height()) parent.Height(frameElem.Height());
            return;
        }
    }
    TaskbarController_UpdateFrameHeight_Original(pThis);
}

void ApplySettings(int taskbarHeight) {
    HWND hTaskbarWnd = FindCurrentProcessTaskbarWnd();
    if (!hTaskbarWnd) {
        g_taskbarHeight = taskbarHeight;
        return;
    }

    // If 0, we don't override, but we still want to refresh the UI to trigger injection
    int targetHeight = taskbarHeight;
    if (targetHeight <= 0) {
        RECT rect{};
        GetWindowRect(hTaskbarWnd, &rect);
        targetHeight = MulDiv(rect.bottom - rect.top, 96, GetDpiForWindow(hTaskbarWnd));
    }

    if (!g_taskbarHeight) {
        g_taskbarHeight = targetHeight;
    }

    g_applyingSettings = true;
    
    // Force refresh by toggling height
    g_pendingMeasureOverride = true;
    g_taskbarHeight = targetHeight - 1;
    if (double_48_value_Original) {
        double val = (double)g_taskbarHeight;
        ProtectAndMemcpy(PAGE_READWRITE, double_48_value_Original, &val, sizeof(double));
    }
    SendMessage(hTaskbarWnd, WM_SETTINGCHANGE, SPI_SETLOGICALDPIOVERRIDE, 0);
    for (int i = 0; i < 100 && g_pendingMeasureOverride; i++) Sleep(100);

    g_pendingMeasureOverride = true;
    g_taskbarHeight = taskbarHeight; // Set to actual setting (can be 0)
    
    // If we are overriding with a specific height, update the constant
    if (double_48_value_Original) {
        double val = (taskbarHeight > 0) ? (double)taskbarHeight : (double)targetHeight;
        ProtectAndMemcpy(PAGE_READWRITE, double_48_value_Original, &val, sizeof(double));
    }

    SendMessage(hTaskbarWnd, WM_SETTINGCHANGE, SPI_SETLOGICALDPIOVERRIDE, 0);
    for (int i = 0; i < 100 && g_pendingMeasureOverride; i++) Sleep(100);

    HWND hReBar = FindWindowEx(hTaskbarWnd, nullptr, L"ReBarWindow32", nullptr);
    if (hReBar) {
        HWND hMSTask = FindWindowEx(hReBar, nullptr, L"MSTaskSwWClass", nullptr);
        if (hMSTask) SendMessage(hMSTask, 0x452, 3, 0);
    }
    g_applyingSettings = false;
}

// --- Performance Logic ---
void CALLBACK PerformanceTimerProc(HWND, UINT, UINT_PTR, DWORD) {
    if (g_unloading) return;

    bool isCovered = false;
    HWND hForeground = GetForegroundWindow();
    if (hForeground) {
        // Simple check: is the foreground window maximized?
        if (IsZoomed(hForeground)) {
            isCovered = true;
        }
    }

    std::lock_guard<std::mutex> lock(g_gridMutex);
    for (auto& t : g_trackedGrids) {
        if (auto grid = t.ref.get()) {
            if (auto player = t.playerRef.get()) {
                grid.Dispatcher().RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::Low, [player, isCovered]() {
                    auto state = player.PlaybackSession().PlaybackState();
                    if (isCovered) {
                        if (state == winrt::Windows::Media::Playback::MediaPlaybackState::Playing) player.Pause();
                    } else {
                        if (state == winrt::Windows::Media::Playback::MediaPlaybackState::Paused) player.Play();
                    }
                });
            }
        }
    }
}

// --- Hooks ---
double WINAPI TaskbarConfiguration_GetFrameSize_Hook(int enumTaskbarSize) {
    if (g_taskbarHeight > 0 && (enumTaskbarSize == 1 || enumTaskbarSize == 2)) return (double)g_taskbarHeight;
    return TaskbarConfiguration_GetFrameSize_Original(enumTaskbarSize);
}

void WINAPI TaskListButton_UpdateVisualStates_Hook(void* pThis) {
    TaskListButton_UpdateVisualStates_Original(pThis);
    if (auto elem = GetFrameworkElementFromNative(pThis)) ScheduleScanAsync(elem);
}

using SHAppBarMessage_t = decltype(&SHAppBarMessage);
SHAppBarMessage_t SHAppBarMessage_Original;
UINT_PTR WINAPI SHAppBarMessage_Hook(DWORD dwMessage, PAPPBARDATA pData) {
    auto ret = SHAppBarMessage_Original(dwMessage, pData);
    if (dwMessage == ABM_QUERYPOS && ret && g_taskbarHeight > 0) {
        pData->rc.top = pData->rc.bottom - MulDiv(g_taskbarHeight, GetDpiForWindow(pData->hWnd), 96);
    }
    return ret;
}

bool HookTaskbarViewDllSymbols(HMODULE module) {
    WindhawkUtils::SYMBOL_HOOK symbolHooks[] = {
        { {LR"(__real@4048000000000000)"}, &double_48_value_Original, nullptr, true },
        { {LR"(public: static double __cdecl winrt::Taskbar::implementation::TaskbarConfiguration::GetFrameSize(enum winrt::WindowsUdk::UI::Shell::TaskbarSize))"}, (void**)&TaskbarConfiguration_GetFrameSize_Original, (void*)TaskbarConfiguration_GetFrameSize_Hook, true },
        { {LR"(private: void __cdecl winrt::Taskbar::implementation::TaskListButton::UpdateVisualStates(void))"}, (void**)&TaskListButton_UpdateVisualStates_Original, (void*)TaskListButton_UpdateVisualStates_Hook },
        { {LR"(public: void __cdecl winrt::Taskbar::implementation::TaskbarController::UpdateFrameHeight(void))"}, (void**)&TaskbarController_UpdateFrameHeight_Original, (void*)TaskbarController_UpdateFrameHeight_Hook, true },
        { {LR"(public: void __cdecl winrt::Taskbar::implementation::TaskbarController::OnGroupingModeChanged(void))"}, &TaskbarController_OnGroupingModeChanged_Original, nullptr, true },
        { {LR"(public: virtual int __cdecl winrt::impl::produce<struct winrt::Taskbar::implementation::TaskbarFrame,struct winrt::Windows::UI::Xaml::IFrameworkElementOverrides>::MeasureOverride(struct winrt::Windows::Foundation::Size,struct winrt::Windows::Foundation::Size *))"}, (void**)&TaskbarFrame_MeasureOverride_Original, (void*)TaskbarFrame_MeasureOverride_Hook }
    };
    return HookSymbols(module, symbolHooks, ARRAYSIZE(symbolHooks));
}

bool HookTaskbarDllSymbols() {
    HMODULE module = LoadLibraryEx(L"taskbar.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!module) return false;

    WindhawkUtils::SYMBOL_HOOK symbolHooks[] = {
        { {LR"(public: void __cdecl TrayUI::_StuckTrayChange(void))"}, (void**)&TrayUI__StuckTrayChange_Original, nullptr },
        { {LR"(public: void __cdecl TrayUI::_HandleSettingChange(struct HWND__ *,unsigned int,unsigned __int64,__int64))"}, (void**)&TrayUI__HandleSettingChange_Original, (void*)TrayUI__HandleSettingChange_Hook }
    };
    return HookSymbols(module, symbolHooks, ARRAYSIZE(symbolHooks));
}

using LoadLibraryExW_t = decltype(&LoadLibraryExW);
LoadLibraryExW_t LoadLibraryExW_Original;
HMODULE WINAPI LoadLibraryExW_Hook(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
    HMODULE module = LoadLibraryExW_Original(lpLibFileName, hFile, dwFlags);
    if (module && !g_taskbarViewDllLoaded && (wcsstr(lpLibFileName, L"Taskbar.View.dll") || wcsstr(lpLibFileName, L"ExplorerExtensions.dll"))) {
        if (!g_taskbarViewDllLoaded.exchange(true)) {
            HookTaskbarViewDllSymbols(module);
            Wh_ApplyHookOperations();
        }
    }
    return module;
}

void Wh_ModSettingsChanged() {
    LoadSettings();
    {
        std::lock_guard<std::mutex> lock(g_gridMutex);
        for (auto& tracked : g_trackedGrids) {
            if (auto grid = tracked.ref.get()) {
                grid.Dispatcher().RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::Normal, [grid]() {
                    RemoveInjectedFromGrid(grid);
                    // Re-inject based on name
                    if (grid.Name() == c_InjectedControlName) {
                        if (Wh_GetIntSetting(L"injectTaskbar")) CreateAndInjectVideo(grid, c_InjectedControlName);
                    } else if (grid.Name() == L"StartButtonVideoGrid") {
                        if (Wh_GetIntSetting(L"injectStartButton")) CreateAndInjectVideo(grid, L"StartButtonVideoGrid");
                    }
                });
            }
        }
    }
    ApplySettings(g_taskbarHeight);
}

BOOL Wh_ModInit() {
    LoadSettings();
    HookTaskbarDllSymbols(); // Hook TrayUI in taskbar.dll
    HMODULE mod = GetModuleHandle(L"Taskbar.View.dll");
    if (!mod) mod = GetModuleHandle(L"ExplorerExtensions.dll");
    if (mod) {
        g_taskbarViewDllLoaded = true;
        HookTaskbarViewDllSymbols(mod);
    } else {
        WindhawkUtils::SetFunctionHook(LoadLibraryExW, LoadLibraryExW_Hook, &LoadLibraryExW_Original);
    }
    WindhawkUtils::SetFunctionHook(SHAppBarMessage, SHAppBarMessage_Hook, &SHAppBarMessage_Original);
    
    // Performance Timer: Check every 1 second
    SetTimer(nullptr, 0, 1000, PerformanceTimerProc);

    return TRUE;
}

void Wh_ModAfterInit() { ApplySettings(g_taskbarHeight); }
void Wh_ModBeforeUninit() { g_unloading = true; }
void Wh_ModUninit() {
    while (g_hookCallCounter > 0) Sleep(100);
    std::vector<TrackedGridRef> local;
    { std::lock_guard<std::mutex> lock(g_gridMutex); local = std::move(g_trackedGrids); }
    for (auto& t : local) if (auto g = t.ref.get()) g.Dispatcher().RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::Normal, [g]() { RemoveInjectedFromGrid(g); });
}
