// ==WindhawkMod==
// @id              taskbar-video-injector
// @name            Taskbar Video Injector
// @description     Injects a video player into Taskbar's RootGrid, intended as background video
// @version         0.7
// @author          Bbmaster123 / AI
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -DWINVER=0x0A00 -ldwmapi -lole32 -loleaut32 -lruntimeobject -lshcore -lversion
// ==/WindhawkMod==
// ==WindhawkModReadme==
/*
Based on the code from Lockframe's injector mod, this mod enables video to be played on the taskbar. The mod works, but has some minor bugs.
If no url or filepath is supplied, a royalty-free stock video will be applied as a placeholder

- supports local filepath (eg. C:\users\admin\videos\test.mp4)
- looping
- set video to mute by default
- opacity
- zindex (depth ordering)
- works with taskbar styler and other taskbar mods

bugs remaining:
- Wont apply until taskbar is interacted with (click, open app, etc)
- if explorer is restarted, mod may need to be toggled (including on reboot)

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
const std::wstring c_RootGridName        = L"RootGrid";
const std::wstring c_InjectedGridName    = L"CustomInjectedGrid";
const std::wstring c_ItemsRepeater = L"Microsoft.UI.Xaml.Controls.ItemsRepeater";


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

    std::wstring videoUrl;
    videoUrl = Wh_GetStringSetting(L"videoUrl");
    if (videoUrl.empty())
        videoUrl = L"https://cdn.pixabay.com/video/2025/12/21/323513_tiny.mp4";
    if (videoUrl.find(L":") != std::wstring::npos && !videoUrl.starts_with(L"http"))
        videoUrl = L"file:///" + videoUrl;

    bool loop = Wh_GetIntSetting(L"loop") != 0;
    float rate = static_cast<float>(Wh_GetIntSetting(L"rate")) /100.0;
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

    if (g_injectedOnce.load())
        return;

    if (auto elem = GetFrameworkElementFromNative(pThis)) {
        // Save the frame reference if we don't have it
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

struct { 
    bool loop;
}
 g_settings;

std::atomic<bool> g_taskbarViewDllLoaded;
std::atomic<bool> g_applyingSettings;
std::atomic<bool> g_pendingMeasureOverride;
std::atomic<bool> g_unloading;
std::atomic<int> g_hookCallCounter;

bool loop;
bool g_inSystemTrayController_UpdateFrameSize;

 void LoadSettings() {
    g_settings.loop = Wh_GetIntSetting(L"loop");
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
                _wcsicmp(className, L"Shell_TrayWnd") == 0) {
                *reinterpret_cast<HWND*>(lParam) = hWnd;
                return FALSE;
            }
            return TRUE;
        },
        reinterpret_cast<LPARAM>(&hTaskbarWnd));

    return hTaskbarWnd;
}

bool ProtectAndMemcpy(DWORD protect, void* dst, const void* src, size_t size) {
    DWORD oldProtect;
    if (!VirtualProtect(dst, size, protect, &oldProtect)) {
        return false;
    }

    memcpy(dst, src, size);
    VirtualProtect(dst, size, oldProtect, &oldProtect);
    return true;
}


void ApplySettings(int taskbarHeight) {
  

    HWND hTaskbarWnd = FindCurrentProcessTaskbarWnd();
    if (!hTaskbarWnd) {
        Wh_Log(L"No taskbar found");     
        return;
    }

      Wh_ModSettingsChanged();   

    Wh_Log(L"Applying settings for taskbar %08X",
           (DWORD)(DWORD_PTR)hTaskbarWnd);
    
        g_pendingMeasureOverride = true;

        // Trigger TrayUI::_HandleSettingChange.
        SendMessage(hTaskbarWnd, WM_SETTINGCHANGE, SPI_SETLOGICALDPIOVERRIDE,
                    0);

        // Wait for the change to apply.
        for (int i = 0; i < 100; i++) {
            if (!g_pendingMeasureOverride) {
                break;
            }

            Sleep(100);
    
    }
    g_pendingMeasureOverride = true;

    // Trigger TrayUI::_HandleSettingChange.
    SendMessage(hTaskbarWnd, WM_SETTINGCHANGE, SPI_SETLOGICALDPIOVERRIDE, 0);

    
        // Wait for the change to apply.
        for (int i = 0; i < 100; i++) {
            if (!g_pendingMeasureOverride) {
                break;
            }
            Sleep(100);
        } 
        g_pendingMeasureOverride = false;


    HWND hReBarWindow32 = FindWindowEx(hTaskbarWnd, nullptr, L"ReBarWindow32", nullptr);
    if (hReBarWindow32) {
        HWND hMSTaskSwWClass = FindWindowEx(hReBarWindow32, nullptr, L"MSTaskSwWClass", nullptr);
        if (hMSTaskSwWClass) {
            // Trigger CTaskBand::_HandleSyncDisplayChange.
            SendMessage(hMSTaskSwWClass, 0x452, 3, 0);
        }
    }

    g_applyingSettings = false;
}

using TaskbarFrame_TaskbarFrame_t = void*(WINAPI*)(void* pThis);
TaskbarFrame_TaskbarFrame_t TaskbarFrame_TaskbarFrame_Original;
void* WINAPI TaskbarFrame_TaskbarFrame_Hook(void* pThis) {
    Wh_Log(L">");

    void* ret = TaskbarFrame_TaskbarFrame_Original(pThis);

    FrameworkElement taskbarFrame = nullptr;
    ((IUnknown**)pThis)[1]->QueryInterface(
        winrt::guid_of<FrameworkElement>(),
        winrt::put_abi(taskbarFrame)
    );
    if (!taskbarFrame) {
        return ret;
    }

    auto className = winrt::get_class_name(taskbarFrame);
    Wh_Log(L"className: %s", className.c_str());

    // store it globally for later
    g_taskbarFrameWeak = taskbarFrame;

    return ret;
}

void ForceInitialInjection() {
    Wh_Log(L"Proactive injection attempt started.");
    // Let's try to see if we have ANY tracked grids first.
    if (auto frame = g_taskbarFrameWeak.get()) {
        if (auto rootGrid = FindRootGridInFrame(frame)) {
            auto dispatcher = rootGrid.Dispatcher();
            if (dispatcher) {
                dispatcher.RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::Normal, [rootGrid]() {
                    Wh_Log(L"Injecting into existing frame via dispatcher.");
                    InjectGridInsideTargetGrid(rootGrid);
                    g_injectedOnce.store(true);
                });
                return;
            }
        }
    }
    Wh_Log(L"No frame found yet. Injection will occur on next taskbar interaction.");
}


bool HookTaskbarViewDllSymbols(HMODULE module) {
    // Taskbar.View.dll, ExplorerExtensions.dll
    WindhawkUtils::SYMBOL_HOOK symbolHooks[] =  //
        {
          
            {
                {LR"(private: void __cdecl winrt::Taskbar::implementation::TaskListButton::UpdateVisualStates(void))"},
                &TaskListButton_UpdateVisualStates_Original,
                TaskListButton_UpdateVisualStates_Hook,
            }
        };

    if (!HookSymbols(module, symbolHooks, ARRAYSIZE(symbolHooks))) {
        Wh_Log(L"HookSymbols failed");
        return false;
    }
    return HookSymbols(module, symbolHooks, ARRAYSIZE(symbolHooks));
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

        if (HookTaskbarViewDllSymbols(module)) {
           
        }
    }   
        Wh_Log(L"Loaded %s", lpLibFileName);  
          

    return module;
}


using SHAppBarMessage_t = decltype(&SHAppBarMessage);
SHAppBarMessage_t SHAppBarMessage_Original;
auto WINAPI SHAppBarMessage_Hook(DWORD dwMessage, PAPPBARDATA pData) {
    auto ret = SHAppBarMessage_Original(dwMessage, pData);

    return ret;
}


using SendMessageTimeoutW_t = decltype(&SendMessageTimeoutW);
SendMessageTimeoutW_t SendMessageTimeoutW_Original;
LRESULT WINAPI SendMessageTimeoutW_Hook(HWND hWnd,
                                        UINT Msg,
                                        WPARAM wParam,
                                        LPARAM lParam,
                                        UINT fuFlags,
                                        UINT uTimeout,
                                        PDWORD_PTR lpdwResult) {
    if (!g_unloading) {
        Wh_Log(L">");      
    }

    LRESULT ret = SendMessageTimeoutW_Original(hWnd, Msg, wParam, lParam,
                                               fuFlags, uTimeout, lpdwResult);

    return ret;
}

BOOL Wh_ModInit() {
    Wh_Log(L"Wh_ModInit");

    // Hook standard Windows functions
    WindhawkUtils::Wh_SetFunctionHookT(SHAppBarMessage, SHAppBarMessage_Hook, &SHAppBarMessage_Original);
    WindhawkUtils::Wh_SetFunctionHookT(SendMessageTimeoutW, SendMessageTimeoutW_Hook, &SendMessageTimeoutW_Original);

    HMODULE module = GetTaskbarViewModuleHandle();
    if (module) {
        g_taskbarViewDllLoaded = true;
        // This function now handles ALL symbol hooking for this DLL
        if (!HookTaskbarViewDllSymbols(module)) {
            return FALSE;
        }
    } else {
        // Fallback for delayed loading
        HMODULE kernelBaseModule = GetModuleHandle(L"kernelbase.dll");
        auto pKernelBaseLoadLibraryExW = (decltype(&LoadLibraryExW))GetProcAddress(kernelBaseModule, "LoadLibraryExW");
        WindhawkUtils::Wh_SetFunctionHookT(pKernelBaseLoadLibraryExW, LoadLibraryExW_Hook, &LoadLibraryExW_Original);
    }

    return TRUE;
}

void Wh_ModAfterInit() {
    HWND hTaskbarWnd = nullptr;
    HWND hReBarWindow32 = FindWindowEx(hTaskbarWnd, nullptr, L"ReBarWindow32", nullptr);
    if (hReBarWindow32) {
        HWND hMSTaskSwWClass = FindWindowEx(hReBarWindow32, nullptr, L"MSTaskSwWClass", nullptr);
        if (hMSTaskSwWClass) {
            // Trigger CTaskBand::_HandleSyncDisplayChange.
            SendMessage(hMSTaskSwWClass, 0x452, 3, 0);
        }
    }

    g_applyingSettings = false;
}

void Wh_ModBeforeUninit() {
    Wh_Log(L">");
    g_unloading = true;
}

void Wh_ModUninit() {
         while (g_hookCallCounter > 0) {
        Sleep(100);
    }
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
