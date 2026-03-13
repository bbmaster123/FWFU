// ==WindhawkMod==
// @id             startmenu-video-injector
// @name           Start menu Video Injector
// @description    Injects a video player into the Start menu background
// @version        1.1
// @author         Bbmaster123 / AI
// @include        explorer.exe
// @include        StartMenuExperienceHost.exe
// @architecture   x86-64
// @compilerOptions -DWINVER=0x0A00 -ldwmapi -lole32 -loleaut32 -lruntimeobject -lshcore -lversion -Wl,--export-all-symbols
// ==/WindhawkMod==

// ==WindhawkModSettings==
/*
- videoUrl: "https://cdn.pixabay.com/video/2025/12/21/323513_tiny.mp4"
  $name: Video URL
- opacity: 50
  $name: Video opacity
- loop: true
  $name: Loop video
- rate: 100
  $name: Playback rate
- zIndex: -1
  $name: Z-Index
*/
// ==/WindhawkModSettings==

#include <windhawk_utils.h>
#include <windows.h>

#undef GetCurrentTime
#undef GetAt
#undef Size

#include <mutex>
#include <vector>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.Media.Core.h>
#include <winrt/Windows.Media.Playback.h>
#include <winrt/Windows.UI.Core.h>
// Fix for the 'RunAsync' deduction error
#include <winrt/impl/Windows.UI.Core.2.h>

using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Media;

struct TrackedGridRef {
    winrt::weak_ref<Grid> ref;
};
std::vector<TrackedGridRef> g_trackedGrids;
std::mutex g_gridMutex;
const std::wstring c_InjectedGridName = L"StartMenuVideoGrid";

void InjectVideo(Grid targetGrid) {
    if (!targetGrid) return;
    try {
        auto children = targetGrid.Children();
        for (uint32_t i = 0; i < children.Size(); ++i) {
            auto fe = children.GetAt(i).try_as<FrameworkElement>();
            if (fe && fe.Name() == c_InjectedGridName) {
                children.RemoveAt(i);
                break;
            }
        }

        std::wstring videoUrl = Wh_GetStringSetting(L"videoUrl");
        double opacity = (double)Wh_GetIntSetting(L"opacity") / 100.0;
        float rate = (float)Wh_GetIntSetting(L"rate") / 100.0f;
        bool loop = Wh_GetIntSetting(L"loop") != 0;
        int zIndex = Wh_GetIntSetting(L"zIndex");

        Grid container;
        container.Name(c_InjectedGridName);
        Canvas::SetZIndex(container, zIndex);

        winrt::Windows::Media::Playback::MediaPlayer mp;
        mp.Source(winrt::Windows::Media::Core::MediaSource::CreateFromUri(winrt::Windows::Foundation::Uri(videoUrl)));
        mp.IsLoopingEnabled(loop);
        mp.IsMuted(true);
        mp.PlaybackRate(rate);

        MediaPlayerElement player;
        player.SetMediaPlayer(mp);
        player.Stretch(Stretch::UniformToFill);
        player.Opacity(opacity);
        player.IsHitTestVisible(false);

        container.Children().Append(player);
        targetGrid.Children().InsertAt(0, container);
        mp.Play();

        std::lock_guard<std::mutex> lock(g_gridMutex);
        g_trackedGrids.push_back({ winrt::make_weak(targetGrid) });
    } catch (...) {}
}

using OnApplyTemplate_t = void (WINAPI*)(void*);
OnApplyTemplate_t OnApplyTemplate_Original;

void WINAPI OnApplyTemplate_Hook(void* pThis) {
    OnApplyTemplate_Original(pThis);
    try {
        winrt::Windows::Foundation::IUnknown unknown{ nullptr };
        winrt::copy_from_abi(unknown, pThis);
        auto control = unknown.try_as<Control>();
        if (control) {
            auto root = VisualTreeHelper::GetChild(control, 0).try_as<Grid>();
            if (root) InjectVideo(root);
        }
    } catch (...) {}
}

BOOL Wh_ModInit() {
    // Try Taskbar.View first, fallback to current process
    HMODULE hModule = GetModuleHandle(L"Taskbar.View.dll");
    if (!hModule) hModule = GetModuleHandle(nullptr);

    WindhawkUtils::SYMBOL_HOOK symbolHooks[] = {
        {
            { 
              // Using wildcards to find the StartMenuRoot OnApplyTemplate 
              // regardless of namespace mangling
              L"*StartMenuRoot*OnApplyTemplate*" 
            },
            (void**)&OnApplyTemplate_Original,
            (void*)OnApplyTemplate_Hook
        }
    };

    return WindhawkUtils::HookSymbols(hModule, symbolHooks, 1);
}

void Wh_ModSettingsChanged() {
    std::lock_guard<std::mutex> lock(g_gridMutex);
    for (auto it = g_trackedGrids.begin(); it != g_trackedGrids.end();) {
        if (auto grid = it->ref.get()) {
            grid.Dispatcher().RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::Normal, [grid]() {
                InjectVideo(grid);
            });
            ++it;
        } else {
            it = g_trackedGrids.erase(it);
        }
    }
}

void Wh_ModUninit() {}
