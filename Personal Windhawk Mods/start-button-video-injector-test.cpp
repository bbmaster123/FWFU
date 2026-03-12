// ==WindhawkMod==
// @id              startbutton-video-injector
// @name            Start button Video Injector
// @description     Injects a video player into the Start button (LaunchListButton) only
// @version         0.2
// @author          Bbmaster123 / AI
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -DWINVER=0x0A00 -ldwmapi -lole32 -loleaut32 -lruntimeobject -lshcore -lversion
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
Variant of Taskbar Video Injector that targets ONLY the Start button
(Taskbar.ExperienceToggleButton#LaunchListButton) instead of Taskbar's RootGrid.

- Same video pipeline (local path or URL, loop, mute, opacity, z-index)
- Uses the same Taskbar.View.dll hook (TaskListButton::UpdateVisualStates)
- Does NOT touch taskbar height or DPI
- Leaves the rest of the taskbar background untouched
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- videoUrl: "https://cdn.pixabay.com/video/2025/12/21/323513_tiny.mp4"
  $name: Video URL
- loop: true
  $name: Loop Video
- rate: 0.1
  $name: Playback Speed (50 = half speed)
- opacity: 100
  $name: Opacity (0-100)
- zIndex: 1
  $name: Z-Index
*/
// ==/WindhawkModSettings==

#include <windhawk_utils.h>
#undef GetCurrentTime

#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.Media.Core.h>
#include <winrt/Windows.Media.Playback.h>

#include <windows.h>
#include <string>
#include <vector>
#include <mutex>
#include <atomic>

using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Media;

// --- Global state ---

std::atomic<bool> g_injectedOnce{ false };

struct TrackedGridRef {
    winrt::weak_ref<Controls::Grid> ref;
};

std::vector<TrackedGridRef> g_trackedGrids;
std::mutex g_gridMutex;
winrt::weak_ref<FrameworkElement> g_taskbarFrameWeak;

const std::wstring c_TaskbarFrameClass   = L"Taskbar.TaskbarFrame";
const std::wstring c_InjectedGridName    = L"StartButtonVideoGrid";
const std::wstring c_ItemsRepeater       = L"Microsoft.UI.Xaml.Controls.ItemsRepeater";

const std::wstring c_StartButtonClass    = L"Taskbar.ExperienceToggleButton";
const std::wstring c_StartButtonName     = L"LaunchListButton";

std::atomic<bool> g_taskbarViewDllLoaded{ false };

// --- Function pointers ---

using TaskListButton_UpdateVisualStates_t = void(WINAPI*)(void* pThis);
TaskListButton_UpdateVisualStates_t TaskListButton_UpdateVisualStates_Original = nullptr;

// --- Helpers ---

FrameworkElement GetFrameworkElementFromNative(void* pThis) {
    try {
        void* iUnknownPtr = 3 + (void**)pThis;
        winrt::Windows::Foundation::IUnknown iUnknown;
        winrt::copy_from_abi(iUnknown, iUnknownPtr);
        return iUnknown.try_as<FrameworkElement>();
    } catch (...) {
        return nullptr;
    }
}

void RemoveInjectedFromGrid(Controls::Grid grid) {
    if (!grid)
        return;

    try {
        auto children = grid.Children();
        for (int i = static_cast<int>(children.Size()) - 1; i >= 0; i--) {
            if (auto fe = children.GetAt(i).try_as<FrameworkElement>()) {
                if (fe.Name() == c_InjectedGridName) {
                    children.RemoveAt(i);
                }
            }
        }
    } catch (...) {
    }
}

void AdjustItemsRepeaterZIndex(FrameworkElement root) {
    if (!root)
        return;

    int count = VisualTreeHelper::GetChildrenCount(root);
    for (int i = 0; i < count; i++) {
        auto child = VisualTreeHelper::GetChild(root, i).try_as<FrameworkElement>();
        if (!child)
            continue;

        auto className = winrt::get_class_name(child);
        if (className == c_ItemsRepeater) {
            Controls::Canvas::SetZIndex(child, 2);
        }
        AdjustItemsRepeaterZIndex(child);
    }
}

void InjectGridInsideTargetGrid(FrameworkElement element) {
    auto targetGrid = element.try_as<Controls::Grid>();
    if (!targetGrid)
        return;

    {
        std::lock_guard<std::mutex> lock(g_gridMutex);

        for (auto& tracked : g_trackedGrids) {
            if (auto existing = tracked.ref.get()) {
                if (existing == targetGrid) {
                    RemoveInjectedFromGrid(targetGrid);
                }
            }
        }
        g_trackedGrids.push_back({ winrt::make_weak(targetGrid) });
    }

    RemoveInjectedFromGrid(targetGrid);

    std::wstring videoUrl = Wh_GetStringSetting(L"videoUrl");
    if (videoUrl.empty())
        videoUrl = L"https://cdn.pixabay.com/video/2025/12/21/323513_tiny.mp4";

    if (videoUrl.find(L":") != std::wstring::npos &&
        !videoUrl.starts_with(L"http")) {
        videoUrl = L"file:///" + videoUrl;
    }

    bool loop = Wh_GetIntSetting(L"loop") != 0;
    float rate = static_cast<float>(Wh_GetIntSetting(L"rate")) / 100.0f;
    double opacityValue = static_cast<double>(Wh_GetIntSetting(L"opacity")) / 100.0;
    int zIndexValue = Wh_GetIntSetting(L"zIndex");

    Controls::Grid injected;
    injected.Name(c_InjectedGridName);
    injected.AllowFocusOnInteraction(false);
    Controls::Canvas::SetZIndex(injected, zIndexValue);

    winrt::Windows::Media::Playback::MediaPlayer mediaPlayer;
    mediaPlayer.Source(
        winrt::Windows::Media::Core::MediaSource::CreateFromUri(
            winrt::Windows::Foundation::Uri(videoUrl)));
    mediaPlayer.IsLoopingEnabled(loop);
    mediaPlayer.IsMuted(true);
    mediaPlayer.AutoPlay(true);
    mediaPlayer.PlaybackRate(rate);

    Controls::MediaPlayerElement player;
    player.SetMediaPlayer(mediaPlayer);
    player.Stretch(Stretch::UniformToFill);
    player.AllowFocusOnInteraction(false);
    player.AutoPlay(true);
    player.AreTransportControlsEnabled(false);
    player.IsEnabled(false);
    player.IsFullWindow(false);
    player.IsHitTestVisible(false);
    player.Opacity(opacityValue);

    targetGrid.Children().Append(injected);
    injected.Children().Append(player);

    mediaPlayer.Play();

    AdjustItemsRepeaterZIndex(targetGrid);
}

// Walk up from any element to TaskbarFrame
FrameworkElement FindTaskbarFrameFromElement(FrameworkElement elem) {
    FrameworkElement current = elem;
    while (current) {
        if (winrt::get_class_name(current) == c_TaskbarFrameClass) {
            return current;
        }
        current = VisualTreeHelper::GetParent(current).try_as<FrameworkElement>();
    }
    return nullptr;
}

// Walk down from TaskbarFrame to Start button (ExperienceToggleButton#LaunchListButton)
FrameworkElement FindStartButtonInFrame(FrameworkElement frameElem) {
    if (!frameElem)
        return nullptr;

    int count = VisualTreeHelper::GetChildrenCount(frameElem);
    for (int i = 0; i < count; i++) {
        auto child = VisualTreeHelper::GetChild(frameElem, i).try_as<FrameworkElement>();
        if (!child)
            continue;

        auto className = winrt::get_class_name(child);
        if (className == c_StartButtonClass && child.Name() == c_StartButtonName) {
            return child;
        }

        if (auto found = FindStartButtonInFrame(child)) {
            return found;
        }
    }
    return nullptr;
}

// Find first Grid inside an element (for Start button template)
FrameworkElement FindFirstGridInElement(FrameworkElement root) {
    if (!root)
        return nullptr;

    if (auto grid = root.try_as<Controls::Grid>())
        return grid;

    int count = VisualTreeHelper::GetChildrenCount(root);
    for (int i = 0; i < count; i++) {
        auto child = VisualTreeHelper::GetChild(root, i).try_as<FrameworkElement>();
        if (!child)
            continue;

        if (auto grid = child.try_as<Controls::Grid>())
            return grid;

        if (auto nested = FindFirstGridInElement(child))
            return nested;
    }

    return nullptr;
}

// Inject into Start button only
void TryInjectFromElement(FrameworkElement elem) {
    if (!elem)
        return;

    if (auto frame = FindTaskbarFrameFromElement(elem)) {
        if (auto startButton = FindStartButtonInFrame(frame)) {
            if (auto grid = FindFirstGridInElement(startButton)) {
                InjectGridInsideTargetGrid(grid);
            }
        }
    }
}

// --- Hooks ---

void WINAPI TaskListButton_UpdateVisualStates_Hook(void* pThis) {
    TaskListButton_UpdateVisualStates_Original(pThis);

    if (g_injectedOnce.load())
        return;

    if (auto elem = GetFrameworkElementFromNative(pThis)) {
        if (!g_taskbarFrameWeak.get()) {
            if (auto frame = FindTaskbarFrameFromElement(elem)) {
                g_taskbarFrameWeak = frame;
            }
        }

        TryInjectFromElement(elem);
        g_injectedOnce.store(true);
    }
}

// Called by Windhawk when settings are changed and applied
void Wh_ModSettingsChanged() {
    std::vector<Controls::Grid> grids;
    {
        std::lock_guard<std::mutex> lock(g_gridMutex);
        for (auto& tracked : g_trackedGrids) {
            if (auto grid = tracked.ref.get()) {
                grids.push_back(grid);
            }
        }
    }

    for (auto& grid : grids) {
        auto dispatcher = grid.Dispatcher();
        if (dispatcher) {
            dispatcher.RunAsync(
                winrt::Windows::UI::Core::CoreDispatcherPriority::Normal,
                [grid]() {
                    InjectGridInsideTargetGrid(grid);
                }
            );
        }
    }
}

// --- Symbol hooking ---

bool HookTaskbarViewDllSymbols(HMODULE module) {
    WindhawkUtils::SYMBOL_HOOK symbolHooks[] =
        {
            {
                { LR"(private: void __cdecl winrt::Taskbar::implementation::TaskListButton::UpdateVisualStates(void))" },
                (void**)&TaskListButton_UpdateVisualStates_Original,
                (void*)TaskListButton_UpdateVisualStates_Hook
            }
        };

    if (!HookSymbols(module, symbolHooks, ARRAYSIZE(symbolHooks))) {
        Wh_Log(L"HookSymbols failed");
        return false;
    }
    return true;
}

HMODULE GetTaskbarViewModuleHandle() {
    HMODULE module = GetModuleHandle(L"Taskbar.View.dll");
    if (!module) {
        module = GetModuleHandle(L"ExplorerExtensions.dll");
    }

    return module;
}

using LoadLibraryExW_t = decltype(&LoadLibraryExW);
LoadLibraryExW_t LoadLibraryExW_Original;

HMODULE WINAPI LoadLibraryExW_Hook(LPCWSTR lpLibFileName,
                                   HANDLE hFile,
                                   DWORD dwFlags) {
    HMODULE module = LoadLibraryExW_Original(lpLibFileName, hFile, dwFlags);
    if (!module) {
        return module;
    }

    if (!g_taskbarViewDllLoaded && GetTaskbarViewModuleHandle() == module &&
        !g_taskbarViewDllLoaded.exchange(true)) {
        Wh_Log(L"Loaded %s", lpLibFileName);
        HookTaskbarViewDllSymbols(module);
    }

    return module;
}

// --- Windhawk entry points ---

BOOL Wh_ModInit() {
    Wh_Log(L"> Start button video injector init");

    bool delayLoadingNeeded = false;
    if (HMODULE taskbarViewModule = GetTaskbarViewModuleHandle()) {
        g_taskbarViewDllLoaded = true;
        if (!HookTaskbarViewDllSymbols(taskbarViewModule)) {
            return FALSE;
        }
    } else {
        Wh_Log(L"Taskbar view module not loaded yet");
        delayLoadingNeeded = true;
    }

    if (delayLoadingNeeded) {
        HMODULE kernelBaseModule = GetModuleHandle(L"kernelbase.dll");
        auto pKernelBaseLoadLibraryExW =
            (decltype(&LoadLibraryExW))GetProcAddress(kernelBaseModule,
                                                      "LoadLibraryExW");
        WindhawkUtils::Wh_SetFunctionHookT(pKernelBaseLoadLibraryExW,
                                           LoadLibraryExW_Hook,
                                           &LoadLibraryExW_Original);
    }

    return TRUE;
}

void Wh_ModAfterInit() {
    Wh_Log(L"Start button video injector after init");

    if (!g_injectedOnce.load()) {
        if (auto frame = g_taskbarFrameWeak.get()) {
            if (auto startButton = FindStartButtonInFrame(frame)) {
                if (auto grid = FindFirstGridInElement(startButton)) {
                    InjectGridInsideTargetGrid(grid);
                    g_injectedOnce.store(true);
                }
            }
        }
    }
}


void Wh_ModBeforeUninit() {
    Wh_Log(L"Start button video injector before uninit");
}

void Wh_ModUninit() {
    Wh_Log(L"Start button video injector uninit");

    std::vector<TrackedGridRef> localGrids;
    {
        std::lock_guard<std::mutex> lock(g_gridMutex);
        localGrids = std::move(g_trackedGrids);
    }

    for (auto& tracked : localGrids) {
        if (auto grid = tracked.ref.get()) {
            grid.Dispatcher().RunAsync(
                winrt::Windows::UI::Core::CoreDispatcherPriority::High,
                [grid]() {
                    RemoveInjectedFromGrid(grid);
                }
            );
        }
    }
}
