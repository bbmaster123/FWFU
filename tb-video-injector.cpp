// ==WindhawkMod==
// @id              taskbar-video-injector
// @name            Taskbar Video Injector
// @description     Injects a video player into Taskbar's Grid#RootGrid Element, intended as background video but could also be made into a popup video player
// @version         0.3
// @author          Bbmaster123 (but actually 60% Lockframe, 25% AI, 15% me)
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lruntimeobject
// ==/WindhawkMod==
// ==WindhawkModReadme==
/*
This mod acts as an addon to the [Windows 11 Taskbar Styler mod](https://windhawk.net/mods/windows-11-taskbar-styler), enabling video to be played on the taskbar. The mod works, but has some minor bugs. If no video is set, a royalty-free stock video is used.

- added local file support via filepath (eg. C:\users\admin\videos\test.mp4)
- fixed looping
- fixed applying settings
- set video to mute by default
- removed old and unused code
- added opacity
- added zindex
- no longer relies on taskbar styler

bugs remaining:
- Wont apply until you change and save settings
- if explorer is restarted, mod may need to be toggled
- in my vm, video appears to stutter
*/
// ==/WindhawkModReadme==
// ==WindhawkModSettings==
/*
- videoUrl: "https://cdn.pixabay.com/video/2025/12/21/323513_tiny.mp4"
  $name: Video URL
- loop: true
  $name: Loop Video
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

using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Media;

// --- Global state ---

struct TrackedGridRef {
    winrt::weak_ref<Controls::Grid> ref;
};

std::vector<TrackedGridRef> g_trackedGrids;
std::mutex g_gridMutex;

const std::wstring c_TaskbarFrameClass   = L"Taskbar.TaskbarFrame";
const std::wstring c_RootGridName        = L"RootGrid";
const std::wstring c_InjectedGridName    = L"CustomInjectedGrid";
const std::wstring c_ItemsRepeaterClass1 = L"Microsoft.UI.Xaml.Controls.ItemsRepeater";
const std::wstring c_ItemsRepeaterClass2 = L"Windows.UI.Xaml.Controls.ItemsRepeater";

// --- Function pointers ---

using TaskListButton_UpdateVisualStates_t = void(WINAPI*)(void* pThis);
TaskListButton_UpdateVisualStates_t TaskListButton_UpdateVisualStates_Original = nullptr;

// --- Helpers ---

FrameworkElement GetFrameworkElementFromNative(void* pThis) {
    try {
        void* iUnknownPtr = (void**)pThis + 3;
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
        if (className == c_ItemsRepeaterClass1 || className == c_ItemsRepeaterClass2) {
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
                    goto skip_push;
                }
            }
        }
        g_trackedGrids.push_back({ winrt::make_weak(targetGrid) });
    }
skip_push:

    RemoveInjectedFromGrid(targetGrid);

    std::wstring videoUrl = Wh_GetStringSetting(L"videoUrl");
    if (videoUrl.empty())
        videoUrl = L"https://cdn.pixabay.com/video/2025/12/21/323513_tiny.mp4";
    if (videoUrl.find(L":") != std::wstring::npos && !videoUrl.starts_with(L"http"))
        videoUrl = L"file:///" + videoUrl;

    bool loop = Wh_GetIntSetting(L"loop") != 0;
    double opacityValue = static_cast<double>(Wh_GetIntSetting(L"opacity")) / 100.0;
    int zIndexValue = Wh_GetIntSetting(L"zIndex");

    Controls::Grid injected;
    injected.Name(c_InjectedGridName);
    Controls::Canvas::SetZIndex(injected, zIndexValue);

    winrt::Windows::Media::Playback::MediaPlayer mediaPlayer;
    mediaPlayer.Source(
        winrt::Windows::Media::Core::MediaSource::CreateFromUri(
            winrt::Windows::Foundation::Uri(videoUrl)));
    mediaPlayer.IsLoopingEnabled(loop);
    mediaPlayer.IsMuted(true);

    Controls::MediaPlayerElement player;
    player.SetMediaPlayer(mediaPlayer);
    player.Stretch(Stretch::UniformToFill);
    player.IsHitTestVisible(false);
    player.Opacity(opacityValue);

    injected.Children().Append(player);
    targetGrid.Children().Append(injected);
    mediaPlayer.Play();

    // Ensure icons stay above the video
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

// Walk down from TaskbarFrame to Grid#RootGrid
FrameworkElement FindRootGridInFrame(FrameworkElement frameElem) {
    if (!frameElem)
        return nullptr;

    int count = VisualTreeHelper::GetChildrenCount(frameElem);
    for (int i = 0; i < count; i++) {
        auto child = VisualTreeHelper::GetChild(frameElem, i).try_as<FrameworkElement>();
        if (!child)
            continue;

        if (auto grid = child.try_as<Controls::Grid>()) {
            if (grid.Name() == c_RootGridName) {
                return grid;
            }
        }

        if (auto found = FindRootGridInFrame(child)) {
            return found;
        }
    }

    return nullptr;
}

void TryInjectFromElement(FrameworkElement elem) {
    if (!elem)
        return;

    if (auto frame = FindTaskbarFrameFromElement(elem)) {
        if (auto rootGrid = FindRootGridInFrame(frame)) {
            InjectGridInsideTargetGrid(rootGrid);
        }
    }
}

// --- Hooks ---

void WINAPI TaskListButton_UpdateVisualStates_Hook(void* pThis) {
    TaskListButton_UpdateVisualStates_Original(pThis);

    if (auto elem = GetFrameworkElementFromNative(pThis)) {
        TryInjectFromElement(elem);
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

BOOL Wh_ModInit() {
    HMODULE module = GetModuleHandle(L"Taskbar.View.dll");
    if (!module)
        module = GetModuleHandle(L"ExplorerExtensions.dll");

    if (module) {
        WindhawkUtils::SYMBOL_HOOK hooks[] = {
            {
                { LR"(private: void __cdecl winrt::Taskbar::implementation::TaskListButton::UpdateVisualStates(void))" },
                (void**)&TaskListButton_UpdateVisualStates_Original,
                (void*)TaskListButton_UpdateVisualStates_Hook
            }
        };

        HookSymbols(module, hooks, _countof(hooks));
    }

    return TRUE;
}

void Wh_ModAfterInit() {
    Wh_Log(L">");
    Wh_ModSettingsChanged();
    }

void Wh_ModUninit() {
    std::vector<TrackedGridRef> localGrids;
    {
        std::lock_guard<std::mutex> lock(g_gridMutex);
        localGrids = std::move(g_trackedGrids);
    }

    for (auto& tracked : localGrids) {
        if (auto grid = tracked.ref.get()) {
            grid.Dispatcher().RunAsync(
                winrt::Windows::UI::Core::CoreDispatcherPriority::Normal,
                [grid]() {
                    RemoveInjectedFromGrid(grid);
                }
            );
        }
    }
}
