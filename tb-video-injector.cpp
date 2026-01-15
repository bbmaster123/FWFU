// ==WindhawkMod==
// @id              taskbar-video-injector
// @name            Taskbar Video Injector
// @description     Injects a video player into Taskbar's Grid#RootGrid Element, intended as background video but could also be made into a popup video player
// @version         0.2
// @author          Bbmaster123 (but actually 60% Lockframe, 25% GPT5, 15% me)
// @github          https://github.com/bbmaster123
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lruntimeobject
// ==/WindhawkMod==

// ==WindhawkModSettings==
/*
- videoUrl: "https://cdn.pixabay.com/video/2025/12/21/323513_tiny.mp4"
  $name: Video URL
  $description: The URL of the video to play on the taskbar

- loop: true
  $name: Loop Video
  $description: Whether the video should loop or not
*/
// ==/WindhawkModSettings==

// ==WindhawkModReadme==
/*
# Taskbar video Injector

This mod acts as an addon to the [Windows 11 Taskbar Styler mod](https://windhawk.net/mods/windows-11-taskbar-styler), enabling video to be played on the taskbar. The mod works, but does 
have some bugs. Also requires styles from video-tb.json to be applied in taskbar styler for now.

-added local file support via filepath (eg. C:\users\admin\videos\test.mp4)
-fixed looping
-set video to mute by default
-removed old and unused code

bugs remaining:
- does not apply, as well as setting wont apply until you toggle mod off then on, and click tasbar
- may need to restart explorer.exe 
*/
// ==/WindhawkModReadme==

#include <windhawk_utils.h> 

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

#include <Unistd.h>
#include <windows.h>
#include <shlobj.h>

using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Data;
using namespace winrt::Windows::UI::Xaml::Media;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Media::Playback;

// Global state tracking
std::atomic<bool> g_taskbarViewDllLoaded = false;

struct TrackedGridRef {
    winrt::weak_ref<Controls::Grid> ref;
};

std::vector<TrackedGridRef> g_trackedGrids;
std::mutex g_gridMutex;

// Cache the TaskbarFrame to allow triggering global scans from local events
winrt::weak_ref<FrameworkElement> g_cachedTaskbarFrame;

const std::wstring c_TargetGridLabeled = L"Windows.UI.Xaml.Controls.Grid";
const std::wstring c_RootFrameName = L"Taskbar.TaskbarFrame";
const std::wstring c_InjectedGridName = L"CustomInjectedGrid";
const std::wstring c_InjectedMediaName = L"CustomInjectedMedia";

// -------------------------------------------------------------------------
// Original Function Pointers
// -------------------------------------------------------------------------
using TaskListButton_UpdateVisualStates_t = void(WINAPI*)(void* pThis);
using TaskbarFrame_TaskbarFrame_t = void*(WINAPI*)(void* pThis);
TaskbarFrame_TaskbarFrame_t TaskbarFrame_TaskbarFrame_Original;

void WINAPI TaskbarFrame_TaskbarFrame_Hook(void* pThis) {
      Wh_Log(L"TaskbarFrame Hooked!");    
    // Call the original function to maintain functionality
    TaskbarFrame_TaskbarFrame_Original(pThis);   
}
TaskListButton_UpdateVisualStates_t TaskListButton_UpdateVisualStates_Original;
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

void RegisterGridForCleanup(Controls::Grid const& grid) {
    if (!grid) return;
    
    void* pAbi = winrt::get_abi(grid);
    
    std::lock_guard<std::mutex> lock(g_gridMutex);

    //Prune dead references while scanning for duplicates.
    auto it = g_trackedGrids.begin();
    while (it != g_trackedGrids.end()) {
        auto existing = it->ref.get();
        {
            if (winrt::get_abi(existing) == pAbi) {
                return; // Already tracked
            }
            ++it;
        }
    }
    
    g_trackedGrids.push_back({ winrt::make_weak(grid) });
}

bool IsAlreadyInjected(Controls::Grid const& grid) {
    for (auto child : grid.Children()) {
        if (auto elem = child.try_as<FrameworkElement>()) {
            if (elem.Name() == c_InjectedGridName) {
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
    auto targetGrid = element.try_as<Controls::Grid>();
    if (!targetGrid) return;
    RegisterGridForCleanup(targetGrid);

    // Prevent duplicates
    for (auto child : targetGrid.Children()) {
        if (auto fe = child.try_as<FrameworkElement>()) {
            if (fe.Name() == c_InjectedGridName)
                return;
        }
    }

    // Create the injected Grid
    Controls::Grid injected;
    injected.Name(c_InjectedGridName);
    injected.HorizontalAlignment(HorizontalAlignment::Stretch);
    injected.VerticalAlignment(VerticalAlignment::Stretch);
    injected.Width(NAN);
    injected.Height(NAN);

    // Fetch settings dynamically from Windhawk
    std::string ResolveVideoUrl(std::string url); 
    std::wstring videoUrl = Wh_GetStringSetting(L"videoUrl");
    if (videoUrl.empty()) {
        videoUrl = L"https://cdn.pixabay.com/video/2025/12/21/323513_tiny.mp4"; // Default video URL        
    }
   if (videoUrl.starts_with(L"file://")) {
        return;  // Already a local path
    } else {
       L"file:///" + videoUrl;  // Convert to local file URI format
    }    

    bool loop = Wh_GetIntSetting(L"loop") == 1; // Assume loop is a boolean stored as 0 (false) or 1 (true)
    Wh_Log(L"Video URL: %s, Loop Setting: %d", videoUrl.c_str(), loop);

    // Create a MediaPlayer for advanced playback controls
    winrt::Windows::Media::Playback::MediaPlayer mediaPlayer;

    // Set the media source
    mediaPlayer.Source(
        winrt::Windows::Media::Core::MediaSource::CreateFromUri(
            winrt::Windows::Foundation::Uri(videoUrl)
        )
    );

    // Set loop settings   
    mediaPlayer.IsLoopingEnabled(loop);  // Apply looping
    mediaPlayer.IsMuted(true);
    Wh_Log(L"MediaPlayer LoopingEnabled: %d", mediaPlayer.IsLoopingEnabled());

    // Create a MediaPlayerElement and link it to the MediaPlayer
    Controls::MediaPlayerElement player;
    player.SetMediaPlayer(mediaPlayer);
    player.AreTransportControlsEnabled(false);  // Hide transport controls (you can modify this)
    player.Stretch(Stretch::UniformToFill);    

    // Check if this update is happening on the UI thread
    auto dispatcher = element.Dispatcher();
    if (!dispatcher.HasThreadAccess()) {
        Wh_Log(L"Not on UI thread, posting to UI thread.");
        // If not on UI thread, run asynchronously on the UI thread
        dispatcher.RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::Normal, [targetGrid, injected, player]() {
           
        // Add the MediaPlayerElement to the injected Grid
        injected.Children().Append(player);

        // Play the media
        player.MediaPlayer().Play();
        }).get();
    } else {
        // If on the UI thread, proceed directly
        targetGrid.Children().Append(injected);

        // Add the MediaPlayerElement to the injected Grid
        injected.Children().Append(player);

        // Play the media
        mediaPlayer.Play();
    }
}

void ScanAndInject(FrameworkElement element) {
    if (!element) return;
    auto className = winrt::get_class_name(element);
    if (className == c_TargetGridLabeled ) {
        InjectGridInsideTargetGrid(element);
        return; 
    }
    int childrenCount = Media::VisualTreeHelper::GetChildrenCount(element);
    for (int i = 0; i < childrenCount; i++) {
        auto childDependencyObject = Media::VisualTreeHelper::GetChild(element, i);
        auto child = childDependencyObject.try_as<FrameworkElement>();
        if (child) {
            ScanAndInject(child);           
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
                ScanAndInject(current);
                return;
            }
            auto parent = Media::VisualTreeHelper::GetParent(current);
            current = parent.try_as<FrameworkElement>();
        }
    } catch (...) {}
    // If not found, force a global scan immediately.
    ScanAndInject(startNode);
}
// -------------------------------------------------------------------------
// Cleanup Helpers
// -------------------------------------------------------------------------
void RemoveInjectedFromGrid(Controls::Grid grid) {
    if (!grid) return;
    try {
        auto children = grid.Children();
        for (int i = children.Size() - 1; i >= 0; i--) {
            auto child = children.GetAt(i);
            if (auto childFe = child.try_as<FrameworkElement>()) {
                if (childFe.Name() == c_InjectedGridName) {
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
            ScanAndInject(elem);
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
// -------------------------------------------------------------------------
// Initialization Logic
// -------------------------------------------------------------------------
bool HookTaskbarViewDllSymbols(HMODULE module) {
    WindhawkUtils::SYMBOL_HOOK hooks[] = {
        {
            {LR"(private: void __cdecl winrt::Taskbar::implementation::TaskListButton::UpdateVisualStates(void))"},
            (void**)&TaskListButton_UpdateVisualStates_Original,
            (void*)TaskListButton_UpdateVisualStates_Hook
        },
        {
            {LR"(public: __cdecl winrt::Taskbar::implementation::TaskbarFrame::TaskbarFrame(void))"},
            (void**)&TaskbarFrame_TaskbarFrame_Original,
            (void*)TaskbarFrame_TaskbarFrame_Hook
        }
    };
    return HookSymbols(module, hooks, ARRAYSIZE(hooks));
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
    Wh_Log(L"Settings changed. Refreshing Video...");
    
    //use the cached frame to trigger a re-scan
    auto frame = g_cachedTaskbarFrame.get();
    if (!frame) {
        Wh_Log(L"No cached frame found. Flickering taskbar to force update...");
        HWND hwnd = FindWindowW(L"Shell_TrayWnd", NULL);
        if (hwnd) PostMessage(hwnd, WM_SETTINGCHANGE, 0, 0);
        return;
    }

    auto dispatcher = frame.Dispatcher();
    dispatcher.RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::Normal, [frame]() {
        // 1. Remove old injected grids
        std::lock_guard<std::mutex> lock(g_gridMutex);
        for (auto& tracked : g_trackedGrids) {
            if (auto grid = tracked.ref.get()) {
                RemoveInjectedFromGrid(grid);
            }
        }
        g_trackedGrids.clear();
        
        //Rerun scan with new settings
        ScanAndInject(frame);
    });
}

BOOL Wh_ModInit() {
    Wh_Log(L"Initializing Taskbar Video");
    HMODULE module = GetTaskbarViewModuleHandle();
    if (module) {
        g_taskbarViewDllLoaded = true;
        if (HookTaskbarViewDllSymbols(module)) {
            Wh_ApplyHookOperations();
            Wh_Log(L"Hooks applied successfully");
        }
    } else {
        // Fallback: Hook LoadLibrary if the DLL isn't there yet
        WindhawkUtils::Wh_SetFunctionHookT(LoadLibraryExW, LoadLibraryExW_Hook, &LoadLibraryExW_Original);
        Wh_Log(L"Waiting for Taskbar.View.dll to load...");
    }
    return TRUE;    
}

void Wh_ModUninit() {
    Wh_Log(L"Uninitializing Taskbar video");
    std::vector<TrackedGridRef> localGrids;
    {
        std::lock_guard<std::mutex> lock(g_gridMutex);
        localGrids = std::move(g_trackedGrids);
    }    
    for (auto& tracked : localGrids) {        
        if (auto grid = tracked.ref.get()) {           
            auto dispatcher = grid.Dispatcher();           
            if (dispatcher.HasThreadAccess()) {              
                RemoveInjectedFromGrid(grid);                
            } else {              
                try {
                    dispatcher.RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::Normal, [grid]() {   
                        RemoveInjectedFromGrid(grid);                        
                    }).get();
                } catch (...) {Wh_Log(L"not removed correctly");}
            }
        }
    }    
    g_cachedTaskbarFrame = nullptr;
}
