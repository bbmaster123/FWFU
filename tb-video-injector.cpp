// ==WindhawkMod==
// @id              taskbar-video-injector
// @name            Taskbar Video Injector
// @description     Injects a video player into Taskbar's Grid#RootGrid Element, intended as background video but could also be made into a popup video player
// @version         0.0
// @author          Bbmaster123 (but actually 80% Lockframe, 10% GPT5, 10% me)
// @github          https://github.com/Lockframe
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lruntimeobject
// ==/WindhawkMod==

// ==WindhawkModSettings==
/*
- videoUrl: "https://cdn.pixabay.com/video/2025/12/21/323513_tiny.mp4"
  $name: Video URL
  $description: The URL of the video to play on the taskbar

- volume: 50
  $name: Volume
  $description: Volume level (0-100)

- loopVideo: true
  $name: Loop Video
  $description: Whether the video should loop or not

*/
// ==/WindhawkModSettings==



// ==WindhawkModReadme==
/*
# Taskbar video Injector

This mod acts as an addon to the [Windows 11 Taskbar Styler mod](https://windhawk.net/mods/windows-11-taskbar-styler), enabling video to be played on the taskbar. The mod doesn't fully work yet, but does 
show a video player with cover image and clickable controls, so only a matter of time until it works. 

*/
// ==/WindhawkModReadme==



#include <windhawk_utils.h>

// Fix for conflict between Windows macro and WinRT method names
#undef GetCurrentTime

#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/base.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Media.Core.h>
#include <winrt/Windows.Media.Playback.h>


#include <atomic>
#include <string>
#include <vector>
#include <mutex>

using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Data;
using namespace winrt::Windows::UI::Xaml::Media;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Media::Playback;


// Global state tracking
std::atomic<bool> g_taskbarViewDllLoaded = false;

struct TrackedPanelRef {
    winrt::weak_ref<Controls::Panel> ref;
};

std::vector<TrackedPanelRef> g_trackedPanels;
std::mutex g_panelMutex;

// Cache the TaskbarFrame to allow triggering global scans from local events
winrt::weak_ref<FrameworkElement> g_cachedTaskbarFrame;

const std::wstring c_TargetPanelLabeled = L"Windows.UI.Xaml.Controls.Grid";
const std::wstring c_TargetPanelButton = L"Windows.UI.Xaml.Controls.Grid";
const std::wstring c_RootFrameName = L"Taskbar.TaskbarFrame";
const std::wstring c_InjectedControlName = L"CustomInjectedGrid";

// -------------------------------------------------------------------------
// Original Function Pointers
// -------------------------------------------------------------------------
using TaskListButton_UpdateVisualStates_t = void(WINAPI*)(void* pThis);
TaskListButton_UpdateVisualStates_t TaskListButton_UpdateVisualStates_Original;

using TaskListButton_UpdateButtonPadding_t = void(WINAPI*)(void* pThis);
TaskListButton_UpdateButtonPadding_t TaskListButton_UpdateButtonPadding_Original;

using ExperienceToggleButton_UpdateVisualStates_t = void(WINAPI*)(void* pThis);
ExperienceToggleButton_UpdateVisualStates_t ExperienceToggleButton_UpdateVisualStates_Original;

// -------------------------------------------------------------------------
// Helpers
// -------------------------------------------------------------------------

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



void RegisterPanelForCleanup(Controls::Panel const& panel) {
    if (!panel) return;
    
    void* pAbi = winrt::get_abi(panel);
    
    std::lock_guard<std::mutex> lock(g_panelMutex);

    // OPTIMIZATION: Prune dead references while scanning for duplicates.
    auto it = g_trackedPanels.begin();
    while (it != g_trackedPanels.end()) {
        auto existing = it->ref.get();
        if (!existing) {
            it = g_trackedPanels.erase(it);
        } else {
            if (winrt::get_abi(existing) == pAbi) {
                return; // Already tracked
            }
            ++it;
        }
    }
    
    g_trackedPanels.push_back({ winrt::make_weak(panel) });
}

bool IsAlreadyInjected(Controls::Grid grid) {
    for (auto child : grid.Children()) {
        if (auto elem = child.try_as<FrameworkElement>()) {
            if (elem.Name() == c_InjectedControlName) {
                return true;
            }
        }
    }
    return false;
}

void extracted() {
    return;
}
void InjectGridInsideTargetGrid(FrameworkElement element) {
    // Ensure the element is a Grid
    auto targetGrid = element.try_as<Controls::Grid>();
    if (!targetGrid) return;

    // Prevent duplicates
    for (auto child : targetGrid.Children()) {
        if (auto fe = child.try_as<FrameworkElement>()) {
            if (fe.Name() == c_InjectedControlName)
                return;
        }
    }

    // Create the injected Grid
    Controls::Grid injected;
    injected.Name(c_InjectedControlName);
    injected.HorizontalAlignment(HorizontalAlignment::Stretch);
    injected.VerticalAlignment(VerticalAlignment::Stretch);
    injected.Width(NAN);
    injected.Height(NAN);

    // Fetch settings dynamically from Windhawk
    std::wstring videoUrl = Wh_GetStringSetting(L"videoUrl");
    if (videoUrl.empty()) {
        videoUrl = L"https://cdn.pixabay.com/video/2025/12/21/323513_tiny.mp4"; // Default video URL
    }
    int volumeInt = Wh_GetIntSetting(L"volume"); // Get volume as an integer (0-100)
    if (volumeInt < 0 || volumeInt > 100) {
        volumeInt = 50; // Default volume to 50 if out of range
    }
    float volume = volumeInt / 100.0f;  // Convert to a float between 0.0 and 1.0
    bool loop = Wh_GetIntSetting(L"loop") > 0; // Assume loop is a boolean stored as 0 (false) or 1 (true)

    // Create a MediaPlayer
    Controls::MediaPlayerElement player;
    player.AreTransportControlsEnabled(false);
    player.AutoPlay(true);
    player.Stretch(Stretch::UniformToFill);

    // Set the video URL dynamically from settings
    auto mediaPlayer = winrt::Windows::Media::Playback::MediaPlayer();
    mediaPlayer.Source(
        winrt::Windows::Media::Core::MediaSource::CreateFromUri(
            winrt::Windows::Foundation::Uri(videoUrl)  // Apply the dynamic URL
        )
    );

    // Set volume and loop settings
    mediaPlayer.Volume(volume); // Apply volume (0.0 to 1.0)
    mediaPlayer.IsLoopingEnabled(loop); // Apply looping setting

    // Explicitly restart video to ensure it loops
    if (loop) {
        mediaPlayer.Play();  // Restart the video to make sure it's looping
    }

    player.SetMediaPlayer(mediaPlayer);

    // Add the injected Grid to the target Grid
    targetGrid.Children().Append(injected);

    // Add the MediaElement to the injected Grid
    injected.Children().Append(player);
}





void ScanAndInjectRecursive(FrameworkElement element) {
    if (!element) return;

    auto className = winrt::get_class_name(element);

    if (className == c_TargetPanelLabeled || className == c_TargetPanelButton) {
        InjectGridInsideTargetGrid(element);
        return; 
    }

    int childrenCount = Media::VisualTreeHelper::GetChildrenCount(element);
    for (int i = 0; i < childrenCount; i++) {
        auto childDependencyObject = Media::VisualTreeHelper::GetChild(element, i);
        auto child = childDependencyObject.try_as<FrameworkElement>();
        if (child) {
            ScanAndInjectRecursive(child);
        }
    }
}

void EnsureGlobalScanFromElement(FrameworkElement startNode) {
    if (g_cachedTaskbarFrame.get()) return;

    try {
        FrameworkElement current = startNode;
        while (current) {
            auto className = winrt::get_class_name(current);
            if (className == c_RootFrameName) {
                g_cachedTaskbarFrame = winrt::make_weak(current);
                ScanAndInjectRecursive(current);
                return;
            }
            auto parent = Media::VisualTreeHelper::GetParent(current);
            current = parent.try_as<FrameworkElement>();
        }
    } catch (...) {}

    // If not found, force a global scan immediately.
    ScanAndInjectRecursive(startNode);
}

// -------------------------------------------------------------------------
// Cleanup Helpers
// -------------------------------------------------------------------------

void RemoveInjectedFromPanel(Controls::Panel panel) {
    if (!panel) return;
    try {
        auto children = panel.Children();
        for (int i = children.Size() - 1; i >= 0; i--) {
            auto child = children.GetAt(i);
            if (auto childFe = child.try_as<FrameworkElement>()) {
                if (childFe.Name() == c_InjectedControlName) {
                    children.RemoveAt(i);
                }
            }
        }
    } catch (...) {}
}

// -------------------------------------------------------------------------
// Hooks
// -------------------------------------------------------------------------

// Helper to reduce redundancy in hooks
void InjectForElement(void* pThis) {
    try {
        if (auto elem = GetFrameworkElementFromNative(pThis)) {
            ScanAndInjectRecursive(elem);
            if (!g_cachedTaskbarFrame.get()) {
                EnsureGlobalScanFromElement(elem);
            }
        }
    } catch (...) {}
}

void WINAPI TaskListButton_UpdateVisualStates_Hook(void* pThis) {
    TaskListButton_UpdateVisualStates_Original(pThis);
    InjectForElement(pThis);
}

void WINAPI TaskListButton_UpdateButtonPadding_Hook(void* pThis) {
    TaskListButton_UpdateButtonPadding_Original(pThis);
    InjectForElement(pThis);
}

void WINAPI ExperienceToggleButton_UpdateVisualStates_Hook(void* pThis) {
    ExperienceToggleButton_UpdateVisualStates_Original(pThis);
    InjectForElement(pThis);
}

// -------------------------------------------------------------------------
// Initialization Logic
// -------------------------------------------------------------------------

bool HookTaskbarViewDllSymbols(HMODULE module) {
    // Taskbar.View.dll
    WindhawkUtils::SYMBOL_HOOK taskbarViewHooks[] = {
        {
            {LR"(private: void __cdecl winrt::Taskbar::implementation::TaskListButton::UpdateVisualStates(void))"},
            &TaskListButton_UpdateVisualStates_Original,
            TaskListButton_UpdateVisualStates_Hook,
        },
        {
            {LR"(private: void __cdecl winrt::Taskbar::implementation::TaskListButton::UpdateButtonPadding(void))"},
            &TaskListButton_UpdateButtonPadding_Original,
            TaskListButton_UpdateButtonPadding_Hook,
        },
        {
            {LR"(private: void __cdecl winrt::Taskbar::implementation::ExperienceToggleButton::UpdateVisualStates(void))"},
            &ExperienceToggleButton_UpdateVisualStates_Original,
            ExperienceToggleButton_UpdateVisualStates_Hook,
            true // Optional
        }
    };

    if (!HookSymbols(module, taskbarViewHooks, ARRAYSIZE(taskbarViewHooks))) {
        Wh_Log(L"Failed to hook Taskbar.View.dll symbols");
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

void HandleLoadedModuleIfTaskbarView(HMODULE module, LPCWSTR lpLibFileName) {
    if (!g_taskbarViewDllLoaded && GetTaskbarViewModuleHandle() == module &&
        !g_taskbarViewDllLoaded.exchange(true)) {
        
        Wh_Log(L"Taskbar View DLL loaded: %s", lpLibFileName);
        
        if (HookTaskbarViewDllSymbols(module)) {
            Wh_ApplyHookOperations();
        }
    }
}

using LoadLibraryExW_t = decltype(&LoadLibraryExW);
LoadLibraryExW_t LoadLibraryExW_Original;

HMODULE WINAPI LoadLibraryExW_Hook(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
    HMODULE module = LoadLibraryExW_Original(lpLibFileName, hFile, dwFlags);
    if (module) {
        HandleLoadedModuleIfTaskbarView(module, lpLibFileName);
    }
    return module;
}


void OnSettingsChanged() {
    // This function should be called whenever settings are changed
    std::wstring videoUrl = Wh_GetStringSetting(L"videoUrl");
    int volumeInt = Wh_GetIntSetting(L"volume");
    bool loop = Wh_GetIntSetting(L"loop") > 0;

    // Log settings to ensure they're applied
    Wh_Log(L"Updated Settings - Video URL: %s, Volume: %d, Loop: %d", videoUrl.c_str(), volumeInt, loop);

    // Ensure the player is updated with the new settings
    if (g_cachedTaskbarFrame.get()) {
        auto taskbarFrame = g_cachedTaskbarFrame.get();
        ScanAndInjectRecursive(taskbarFrame);  // Refresh UI with new settings
    }
}

// You should ensure this function is triggered when settings change, such as when the user clicks "Save".


bool Wh_ModInit() {
    Wh_Log(L"Initializing Taskbar Injector Mod v1.6");

    if (HMODULE taskbarViewModule = GetTaskbarViewModuleHandle()) {
        g_taskbarViewDllLoaded = true;
        if (!HookTaskbarViewDllSymbols(taskbarViewModule)) {
            return FALSE;
        }
        
        // Trigger a global scan immediately after the taskbar is initialized
        EnsureGlobalScanFromElement(nullptr);

        // Inject the video immediately if it hasn't already been injected
        if (g_cachedTaskbarFrame.get()) {
            auto taskbarFrame = g_cachedTaskbarFrame.get();
            ScanAndInjectRecursive(taskbarFrame);  // Force the scan and injection
        }
    } else {
        HMODULE kernelBaseModule = GetModuleHandle(L"kernelbase.dll");
        auto pKernelBaseLoadLibraryExW = (decltype(&LoadLibraryExW))GetProcAddress(kernelBaseModule, "LoadLibraryExW");
        WindhawkUtils::Wh_SetFunctionHookT(pKernelBaseLoadLibraryExW, LoadLibraryExW_Hook, &LoadLibraryExW_Original);
    }

    return TRUE;
}



void Wh_ModUninit() {
    Wh_Log(L"Uninitializing Taskbar Injector Mod");

    std::vector<TrackedPanelRef> localPanels;
    {
        std::lock_guard<std::mutex> lock(g_panelMutex);
        localPanels = std::move(g_trackedPanels);
    }
    
    for (auto& tracked : localPanels) {
        if (auto panel = tracked.ref.get()) {
            auto dispatcher = panel.Dispatcher();
            if (dispatcher.HasThreadAccess()) {
                RemoveInjectedFromPanel(panel);
            } else {
                dispatcher.RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::Normal, [panel]() {
                    RemoveInjectedFromPanel(panel);
                }).get();
            }
        }
    }    
    g_cachedTaskbarFrame = nullptr;
}
