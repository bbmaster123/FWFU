// ==WindhawkMod==
// @id              xaml-video-injector
// @name            XAML-Video-Injector
// @description     Injects a video player into taskbar, start button, start menu, File Explorer tabs/command bar, and custom XAML/WinUI3 paths with hierarchical selector support.
// @version         1.0.0
// @author          bbmaster123 / Gemini
// @include         explorer.exe
// @include         StartMenuExperienceHost.exe
// @include         SearchHost.exe
// @include         SearchApp.exe
// @architecture    x86-64
// @compilerOptions -DWINVER=0x0A00 -ladvapi32 -ldwmapi -lgdi32 -lole32 -loleaut32 -lruntimeobject -lshcore -lshell32 -lshlwapi -luuid -lversion
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
Injects Grid element with media player to defined XAML targets on a per element basis. Each target can have its own video and settings.

- Supports both UWP XAML (Windows.UI.Xaml) and WinUI 3 (Microsoft.UI.Xaml) applications
- Supports hierarchical path targeting (e.g. "CommandBarControlRootGrid > Grid", "RootGrid > Grid", "> Grid")
- Taskbar, Start Button, and Start Menu dedicated injection
- File Explorer command bar & WinUI 3 custom injections
- Looping, playback speed, opacity, corner radius, and Z-Index options for each target
- Supports local filepaths (e.g. C:\videos\test.mp4) and web stream URLs
- Mutes videos by default to prevent unwanted system audio
- Performance timer pauses playback when windows are covered or minimized
- Safe asynchronous lifecycle management to prevent host crashes on unload
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- loop: true
  $name: "Loop Video (Global)"

- injectTaskbar: true
  $name: "Taskbar: Enable Injection"
- taskbarVideoUrl: "https://cdn.pixabay.com/video/2025/12/21/323513_tiny.mp4"
  $name: "Taskbar: Video URL"
- taskbarRate: 100
  $name: "Taskbar: Playback Speed (100 = 1.0x)"
- taskbarOpacity: 100
  $name: "Taskbar: Opacity (0-100)"
- taskbarZIndex: 1
  $name: "Taskbar: Z-Index"
- taskbarCornerRadius: 0
  $name: "Taskbar: Corner Radius"
- TaskbarHeight: 48
  $name: "Taskbar: Reserved Height (0 for default)"

- injectStartButton: false
  $name: "Start Button: Enable Injection"
- startButtonVideoUrl: "https://cdn.pixabay.com/video/2025/12/21/323513_tiny.mp4"
  $name: "Start Button: Video URL"
- startButtonRate: 100
  $name: "Start Button: Playback Speed (100 = 1.0x)"
- startButtonOpacity: 100
  $name: "Start Button: Opacity (0-100)"
- startButtonZIndex: 1
  $name: "Start Button: Z-Index"
- startButtonCornerRadius: 0
  $name: "Start Button: Corner Radius"

- injectStartMenu: true
  $name: "Start Menu: Enable Injection"
- startMenuVideoUrl: "https://cdn.pixabay.com/video/2025/12/21/323513_tiny.mp4"
  $name: "Start Menu: Video URL"
- startMenuRate: 100
  $name: "Start Menu: Playback Speed (100 = 1.0x)"
- startMenuOpacity: 100
  $name: "Start Menu: Opacity (0-100)"
- startMenuZIndex: -1
  $name: "Start Menu: Z-Index"
- startMenuCornerRadius: 8
  $name: "Start Menu: Corner Radius"

- customInjections:
  - - processName: "explorer.exe"
      $name: "Process Name (e.g. explorer.exe)"
    - xamlPath: "FileExplorerExtensions.CommandBarControl > Grid"
      $name: "XAML Element Path (e.g. RootGrid or FileExplorerExtensions.CommandBarControl > Grid)"
    - videoUrl: "https://cdn.pixabay.com/video/2025/12/21/323513_tiny.mp4"
      $name: "Video URL"
    - rate: 100
      $name: "Playback Speed (100 = 1.0x)"
    - opacity: 100
      $name: "Opacity (0-100)"
    - zIndex: 0
      $name: "Z-Index"
    - cornerRadius: 0
      $name: "Corner Radius"
  $name: "Custom Injections"
  $description: "Add custom XAML / WinUI 3 injection points here. Supports hierarchical '>' (direct child) and space (descendant) syntax. Note: You must also add the process name to the @include list in mod settings."
*/
// ==/WindhawkModSettings==

#include <windhawk_utils.h>
#include <roapi.h>
#include <winstring.h>
#include <Unknwn.h>
#undef GetCurrentTime

#include <winrt/base.h>
#include <winrt/Windows.Data.Json.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.UI.Xaml.Media.Animation.h>
#include <winrt/Windows.Media.Core.h>
#include <winrt/Windows.Media.Playback.h>

#include <winrt/Microsoft.UI.h>
#include <winrt/Microsoft.UI.Content.h>
#include <winrt/Microsoft.UI.Dispatching.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Automation.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>
#include <winrt/Microsoft.UI.Xaml.Documents.h>
#include <winrt/Microsoft.UI.Xaml.Input.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <winrt/Microsoft.UI.Xaml.Media.Animation.h>

#include <atomic>
#include <mutex>
#include <string>
#include <vector>
#include <algorithm>

// UWP XAML Namespaces
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Media;

// WinUI 3 Namespaces
namespace mud = winrt::Microsoft::UI::Dispatching;
namespace mux = winrt::Microsoft::UI::Xaml;
namespace muxc = winrt::Microsoft::UI::Xaml::Controls;
namespace muxm = winrt::Microsoft::UI::Xaml::Media;
namespace muxa = winrt::Microsoft::UI::Xaml::Media::Animation;

// --- Constants ---
constexpr std::wstring_view c_TargetGridName      = L"RootGrid";
constexpr std::wstring_view c_RootFrameName       = L"Taskbar.TaskbarFrame";
constexpr std::wstring_view c_InjectedControlName = L"CustomVideoInjectedGrid";
constexpr std::wstring_view c_ItemsRepeater       = L"Microsoft.UI.Xaml.Controls.ItemsRepeater";

constexpr std::wstring_view c_StartButtonClass    = L"Taskbar.ExperienceToggleButton";
constexpr std::wstring_view c_StartButtonName     = L"LaunchListButton";

// --- Global State ---
std::atomic<bool> g_taskbarViewDllLoaded = false;
std::atomic<bool> g_fileExplorerExtDllLoaded = false;
std::atomic<bool> g_unloading{ false };
std::atomic<bool> g_scanPending = false;
std::atomic<bool> g_applyingSettings{ false };
std::atomic<bool> g_pendingMeasureOverride{ false };
std::atomic<int> g_hookCallCounter{ 0 };
UINT_PTR g_perfTimerId = 0;

// --- Start Menu Specific State ---
bool g_applyPending = false;
winrt::event_token g_layoutUpdatedToken{};
winrt::event_token g_visibilityChangedToken{};

// --- Tracking Structs ---
struct TrackedGridRef {
    winrt::weak_ref<Controls::Grid> ref;
    winrt::weak_ref<winrt::Windows::Media::Playback::MediaPlayer> playerRef;
    std::wstring uniqueName;
};

struct TrackedGridRefWinUI3 {
    winrt::weak_ref<muxc::Grid> ref;
    winrt::weak_ref<winrt::Windows::Media::Playback::MediaPlayer> playerRef;
    std::wstring uniqueName;
};

struct XamlPathSegment {
    std::wstring name;
    bool directParent{ false };
};

struct InjectionSettings {
    std::wstring videoUrl;
    bool loop{ true };
    double rate{ 1.0 };
    double opacity{ 1.0 };
    int zIndex{ 0 };
    int cornerRadius{ 0 };
};

struct CustomInjectionConfig {
    std::wstring processName;
    std::wstring xamlPath;
    std::vector<XamlPathSegment> parsedSegments;
    std::wstring videoUrl;
    double rate{ 1.0 };
    double opacity{ 1.0 };
    int zIndex{ 0 };
    int cornerRadius{ 0 };
};

struct ModSettings {
    std::mutex mutex;
    bool loop{ true };
    bool injectTaskbar{ true };
    bool injectStartButton{ false };
    bool injectStartMenu{ true };
    int taskbarHeight{ 48 };
    std::vector<CustomInjectionConfig> customInjections;
} g_modSettings;

std::vector<TrackedGridRef> g_trackedGrids;
std::vector<TrackedGridRefWinUI3> g_trackedGridsWinUI3;
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

// Forward declarations
void ScanWinUI3NodeRecursive(mux::DependencyObject const& node);
void ScanXamlRootForCommandBars(mux::UIElement const& element);
void ScheduleXamlRootScan(mux::UIElement const& element);
void ScanCurrentThreadForCommandBars();
void ScheduleCurrentThreadScan();
std::vector<HWND> GetFileExplorerWnds();

// --- Safe Settings Access Helpers ---
std::wstring GetStringSettingSafe(PCWSTR pszKey, PCWSTR pszDefault = L"") {
    PCWSTR raw = Wh_GetStringSetting(pszKey);
    if (!raw) return pszDefault ? pszDefault : L"";
    std::wstring result(raw);
    Wh_FreeStringSetting(raw);
    return result;
}

std::wstring GetStringSettingIndexedSafe(PCWSTR pszFormat, int index, PCWSTR pszDefault = L"") {
    WCHAR key[128];
    swprintf_s(key, ARRAYSIZE(key), pszFormat, index);
    return GetStringSettingSafe(key, pszDefault);
}

std::wstring GetCurrentProcessBaseName() {
    WCHAR processPath[MAX_PATH];
    if (!GetModuleFileNameW(nullptr, processPath, MAX_PATH)) return L"";
    std::wstring path = processPath;
    size_t lastSlash = path.find_last_of(L"\\/");
    if (lastSlash != std::wstring::npos) {
        path = path.substr(lastSlash + 1);
    }
    std::transform(path.begin(), path.end(), path.begin(), ::towlower);
    return path;
}

std::wstring NormalizeVideoUri(std::wstring url) {
    url.erase(0, url.find_first_not_of(L" \t\r\n"));
    url.erase(url.find_last_not_of(L" \t\r\n") + 1);
    if (url.empty()) return L"";

    if ((url.length() >= 2 && url[1] == L':') || url.find(L"\\\\") == 0) {
        std::replace(url.begin(), url.end(), L'\\', L'/');
        if (url.find(L"file:///") != 0 && url.find(L"file://") != 0) {
            if (url.find(L"//") == 0) {
                url = L"file:" + url;
            } else {
                url = L"file:///" + url;
            }
        }
    }
    return url;
}

// --- Hierarchical Path Selector Parser & Matcher ---
std::vector<XamlPathSegment> ParseXamlPath(const std::wstring& path) {
    std::vector<XamlPathSegment> segments;
    size_t i = 0;
    bool nextDirect = false;

    while (i < path.size()) {
        while (i < path.size() && (path[i] == L' ' || path[i] == L'\t' || path[i] == L'\r' || path[i] == L'\n')) {
            i++;
        }
        if (i >= path.size()) break;

        if (path[i] == L'>') {
            nextDirect = true;
            i++;
            continue;
        }

        size_t start = i;
        while (i < path.size() && path[i] != L'>' && path[i] != L' ' && path[i] != L'\t' && path[i] != L'\r' && path[i] != L'\n') {
            i++;
        }

        std::wstring seg = path.substr(start, i - start);
        std::transform(seg.begin(), seg.end(), seg.begin(), ::towlower);
        if (!seg.empty()) {
            segments.push_back({ seg, nextDirect });
            nextDirect = false;
        }
    }
    return segments;
}

bool MatchSingleElementUWP(winrt::Windows::UI::Xaml::DependencyObject const& obj, const std::wstring& seg) {
    if (!obj) return false;
    if (seg.empty() || seg == L"*") return true;

    if (auto fe = obj.try_as<winrt::Windows::UI::Xaml::FrameworkElement>()) {
        std::wstring name(fe.Name());
        std::transform(name.begin(), name.end(), name.begin(), ::towlower);
        if (!name.empty() && name == seg) return true;
    }

    std::wstring className(winrt::get_class_name(obj));
    std::transform(className.begin(), className.end(), className.begin(), ::towlower);

    if (!className.empty()) {
        if (className == seg) return true;
        size_t dot = className.find_last_of(L'.');
        if (dot != std::wstring::npos && className.substr(dot + 1) == seg) return true;
        if (className.find(seg) != std::wstring::npos) return true;
    }

    return false;
}

bool MatchesXamlPathUWP(winrt::Windows::UI::Xaml::DependencyObject const& node, const std::vector<XamlPathSegment>& segments) {
    if (!node || segments.empty()) return false;

    int segIdx = (int)segments.size() - 1;
    if (!MatchSingleElementUWP(node, segments[segIdx].name)) {
        return false;
    }

    winrt::Windows::UI::Xaml::DependencyObject current = node;
    while (segIdx > 0) {
        bool direct = segments[segIdx].directParent;
        segIdx--;

        if (direct) {
            current = winrt::Windows::UI::Xaml::Media::VisualTreeHelper::GetParent(current);
            if (!current || !MatchSingleElementUWP(current, segments[segIdx].name)) {
                return false;
            }
        } else {
            bool matchedAncestor = false;
            while (current) {
                current = winrt::Windows::UI::Xaml::Media::VisualTreeHelper::GetParent(current);
                if (current && MatchSingleElementUWP(current, segments[segIdx].name)) {
                    matchedAncestor = true;
                    break;
                }
            }
            if (!matchedAncestor) return false;
        }
    }

    return (segIdx == 0);
}

bool MatchSingleElementWinUI3(mux::DependencyObject const& obj, const std::wstring& seg) {
    if (!obj) return false;
    if (seg.empty() || seg == L"*") return true;

    if (auto fe = obj.try_as<mux::FrameworkElement>()) {
        std::wstring name(fe.Name());
        std::transform(name.begin(), name.end(), name.begin(), ::towlower);
        if (!name.empty() && name == seg) return true;
    }

    std::wstring className(winrt::get_class_name(obj));
    std::transform(className.begin(), className.end(), className.begin(), ::towlower);

    if (!className.empty()) {
        if (className == seg) return true;
        size_t dot = className.find_last_of(L'.');
        if (dot != std::wstring::npos && className.substr(dot + 1) == seg) return true;
        if (className.find(seg) != std::wstring::npos) return true;
    }

    return false;
}

bool MatchesXamlPathWinUI3(mux::DependencyObject const& node, const std::vector<XamlPathSegment>& segments) {
    if (!node || segments.empty()) return false;

    int segIdx = (int)segments.size() - 1;
    if (!MatchSingleElementWinUI3(node, segments[segIdx].name)) {
        return false;
    }

    mux::DependencyObject current = node;
    while (segIdx > 0) {
        bool direct = segments[segIdx].directParent;
        segIdx--;

        if (direct) {
            current = muxm::VisualTreeHelper::GetParent(current);
            if (!current || !MatchSingleElementWinUI3(current, segments[segIdx].name)) {
                return false;
            }
        } else {
            bool matchedAncestor = false;
            while (current) {
                current = muxm::VisualTreeHelper::GetParent(current);
                if (current && MatchSingleElementWinUI3(current, segments[segIdx].name)) {
                    matchedAncestor = true;
                    break;
                }
            }
            if (!matchedAncestor) return false;
        }
    }

    return (segIdx == 0);
}

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

using RoGetActivationFactory_t = HRESULT(WINAPI*)(HSTRING, REFIID, void**);
RoGetActivationFactory_t RoGetActivationFactory_Original;

void* TaskbarController_OnGroupingModeChanged_Original = nullptr;

// --- WinUI 3 FileExplorerExtensions Function Pointers ---
using CommandBarManager_CommandBar_t = void(WINAPI*)(void*, void*);
CommandBarManager_CommandBar_t CommandBarManager_CommandBar_Original;

using CommandBarControl_OnApplyTemplate_t = void(WINAPI*)(void*);
CommandBarControl_OnApplyTemplate_t CommandBarControl_OnApplyTemplate_Original;
CommandBarControl_OnApplyTemplate_t CommandBarControl_Wave1_OnApplyTemplate_Original;

using CommandBarControl_GotFocus_t = void(WINAPI*)(void*, void*, void*);
CommandBarControl_GotFocus_t CommandBarControl_GotFocus_Original;
CommandBarControl_GotFocus_t CommandBarControl_Wave1_GotFocus_Original;

// --- Settings Resolver ---
InjectionSettings GetSettingsForTarget(std::wstring_view uniqueName) {
    InjectionSettings s;
    s.loop = Wh_GetIntSetting(L"loop") != 0;
    
    std::wstring prefix;
    if (uniqueName == c_InjectedControlName) prefix = L"taskbar";
    else if (uniqueName == L"StartButtonVideoGrid") prefix = L"startButton";
    else if (uniqueName == L"StartMenuVideoGrid") prefix = L"startMenu";

    if (!prefix.empty()) {
        s.videoUrl = GetStringSettingSafe((prefix + L"VideoUrl").c_str(), L"https://cdn.pixabay.com/video/2025/12/21/323513_tiny.mp4");
        s.videoUrl = NormalizeVideoUri(s.videoUrl);

        s.rate = static_cast<double>(Wh_GetIntSetting((prefix + L"Rate").c_str())) / 100.0;
        if (s.rate <= 0.0) s.rate = 1.0;

        s.opacity = static_cast<double>(Wh_GetIntSetting((prefix + L"Opacity").c_str())) / 100.0;
        s.zIndex = Wh_GetIntSetting((prefix + L"ZIndex").c_str());
        s.cornerRadius = Wh_GetIntSetting((prefix + L"CornerRadius").c_str());
    } else if (std::wstring_view(uniqueName).find(L"CustomVideoGrid_") == 0) {
        int index = 0;
        try {
            index = std::stoi(std::wstring(uniqueName.substr(16)));
        } catch (...) { index = 0; }
        
        std::lock_guard<std::mutex> lock(g_modSettings.mutex);
        if (index >= 0 && index < (int)g_modSettings.customInjections.size()) {
            auto const& ci = g_modSettings.customInjections[index];
            s.videoUrl = ci.videoUrl;
            s.rate = ci.rate;
            s.opacity = ci.opacity;
            s.zIndex = ci.zIndex;
            s.cornerRadius = ci.cornerRadius;
        }
    }
    
    return s;
}

void LoadSettings() {
    std::lock_guard<std::mutex> lock(g_modSettings.mutex);
    g_modSettings.loop = Wh_GetIntSetting(L"loop") != 0;
    g_modSettings.injectTaskbar = Wh_GetIntSetting(L"injectTaskbar") != 0;
    g_modSettings.injectStartButton = Wh_GetIntSetting(L"injectStartButton") != 0;
    g_modSettings.injectStartMenu = Wh_GetIntSetting(L"injectStartMenu") != 0;
    g_modSettings.taskbarHeight = Wh_GetIntSetting(L"TaskbarHeight");
    g_taskbarHeight = g_modSettings.taskbarHeight;

    g_modSettings.customInjections.clear();
    for (int i = 0;; i++) {
        std::wstring proc = GetStringSettingIndexedSafe(L"customInjections[%d].processName", i);
        if (proc.empty()) break;
        std::transform(proc.begin(), proc.end(), proc.begin(), ::towlower);

        std::wstring xamlPath = GetStringSettingIndexedSafe(L"customInjections[%d].xamlPath", i);
        std::wstring videoUrl = GetStringSettingIndexedSafe(L"customInjections[%d].videoUrl", i);
        videoUrl = NormalizeVideoUri(videoUrl);

        WCHAR key[128];
        swprintf_s(key, ARRAYSIZE(key), L"customInjections[%d].rate", i);
        int rateVal = Wh_GetIntSetting(key);
        double rate = (rateVal > 0) ? ((double)rateVal / 100.0) : 1.0;

        swprintf_s(key, ARRAYSIZE(key), L"customInjections[%d].opacity", i);
        int opVal = Wh_GetIntSetting(key);
        double opacity = (opVal >= 0) ? ((double)opVal / 100.0) : 1.0;

        swprintf_s(key, ARRAYSIZE(key), L"customInjections[%d].zIndex", i);
        int zIndex = Wh_GetIntSetting(key);

        swprintf_s(key, ARRAYSIZE(key), L"customInjections[%d].cornerRadius", i);
        int cornerRadius = Wh_GetIntSetting(key);

        auto segments = ParseXamlPath(xamlPath);
        g_modSettings.customInjections.push_back({ proc, xamlPath, segments, videoUrl, rate, opacity, zIndex, cornerRadius });
    }
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

void NotifyAllTaskbarWindows(UINT msg, WPARAM wParam, LPARAM lParam) {
    EnumWindows(
        [](HWND hWnd, LPARAM lp) -> BOOL {
            DWORD dwProcessId;
            WCHAR className[32];
            if (GetWindowThreadProcessId(hWnd, &dwProcessId) &&
                dwProcessId == GetCurrentProcessId() &&
                GetClassName(hWnd, className, ARRAYSIZE(className)) &&
                (_wcsicmp(className, L"Shell_TrayWnd") == 0 || _wcsicmp(className, L"Shell_SecondaryTrayWnd") == 0)) {
                SendMessage(hWnd, (UINT)lp, 0, 0);
            }
            return TRUE;
        },
        (LPARAM)msg);
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

// --- UWP Video Operations ---
void CleanupVideoContainer(FrameworkElement const& container) {
    try {
        if (auto grid = container.try_as<Grid>()) {
            for (auto child : grid.Children()) {
                if (auto mpe = child.try_as<MediaPlayerElement>()) {
                    if (auto player = mpe.MediaPlayer()) {
                        try {
                            player.Pause();
                            player.Source(nullptr);
                        } catch (...) {}
                        mpe.SetMediaPlayer(nullptr);
                    }
                }
            }
        }
    } catch (...) {}
}

void RemoveInjectedFromGrid(Grid grid) {
    if (!grid) return;
    try {
        auto children = grid.Children();
        for (int i = (int)children.Size() - 1; i >= 0; i--) {
            if (auto fe = children.GetAt(i).try_as<FrameworkElement>()) {
                std::wstring name(fe.Name());
                if (name == c_InjectedControlName || name == L"StartButtonVideoGrid" || 
                    name == L"StartMenuVideoGrid" || name.rfind(L"CustomVideoGrid_", 0) == 0) {
                    CleanupVideoContainer(fe);
                    children.RemoveAt(i);
                }
            }
        }
    } catch (...) {}
}

void CreateAndInjectVideo(Grid targetGrid, std::wstring_view uniqueName) {
    if (!targetGrid || g_unloading) return;

    InjectionSettings s = GetSettingsForTarget(uniqueName);
    if (s.videoUrl.empty()) return;

    try {
        winrt::Windows::Foundation::Uri videoUri{ nullptr };
        try {
            videoUri = winrt::Windows::Foundation::Uri(s.videoUrl);
        } catch (...) {
            return;
        }

        Grid videoContainer;
        videoContainer.Name(uniqueName);
        videoContainer.HorizontalAlignment(HorizontalAlignment::Stretch);
        videoContainer.VerticalAlignment(VerticalAlignment::Stretch);
        videoContainer.Opacity(0);
        Canvas::SetZIndex(videoContainer, s.zIndex);
        Controls::Grid::SetColumn(videoContainer, 0);
        Controls::Grid::SetColumnSpan(videoContainer, 100);
        Controls::Grid::SetRow(videoContainer, 0);
        Controls::Grid::SetRowSpan(videoContainer, 100);

        if (s.cornerRadius > 0) {
            videoContainer.CornerRadius(winrt::Windows::UI::Xaml::CornerRadius{ 
                (double)s.cornerRadius, (double)s.cornerRadius, (double)s.cornerRadius, (double)s.cornerRadius 
            });
        }

        winrt::Windows::Media::Playback::MediaPlayer mediaPlayer;
        mediaPlayer.Source(winrt::Windows::Media::Core::MediaSource::CreateFromUri(videoUri));
        mediaPlayer.CommandManager().IsEnabled(false);
        mediaPlayer.IsLoopingEnabled(s.loop);
        mediaPlayer.IsMuted(true);
        mediaPlayer.PlaybackRate(s.rate);

        MediaPlayerElement player;
        player.SetMediaPlayer(mediaPlayer);
        player.Stretch(Stretch::UniformToFill);
        player.IsHitTestVisible(false);
        try {
            player.CacheMode(winrt::Windows::UI::Xaml::Media::BitmapCache{});
        } catch (...) {}

        videoContainer.Children().Append(player);
        
        if (uniqueName == L"StartMenuVideoGrid" || uniqueName.rfind(L"CustomVideoGrid_", 0) == 0) {
            targetGrid.Children().InsertAt(0, videoContainer);
        } else {
            targetGrid.Children().Append(videoContainer);
        }
        
        using namespace winrt::Windows::UI::Xaml::Media::Animation;
        DoubleAnimation fadeIn;
        fadeIn.Duration(winrt::Windows::UI::Xaml::Duration{ winrt::Windows::Foundation::TimeSpan{ std::chrono::milliseconds(314) } });
        fadeIn.From(0.0);
        fadeIn.To(s.opacity);
        
        Storyboard storyboard;
        storyboard.Children().Append(fadeIn);
        Storyboard::SetTarget(fadeIn, videoContainer);
        Storyboard::SetTargetProperty(fadeIn, L"Opacity");
        storyboard.Begin();

        mediaPlayer.Play();

        {
            std::lock_guard<std::mutex> lock(g_gridMutex);
            bool found = false;
            for (auto& t : g_trackedGrids) {
                if (auto g = t.ref.get()) {
                    if (winrt::get_abi(g) == winrt::get_abi(targetGrid)) {
                        t.playerRef = winrt::make_weak(mediaPlayer);
                        t.uniqueName = std::wstring(uniqueName);
                        found = true;
                        break;
                    }
                }
            }
            if (!found) {
                g_trackedGrids.push_back({ winrt::make_weak(targetGrid), winrt::make_weak(mediaPlayer), std::wstring(uniqueName) });
            }
        }

        if (uniqueName != L"StartMenuVideoGrid" && uniqueName.rfind(L"CustomVideoGrid_", 0) != 0) {
            int count = VisualTreeHelper::GetChildrenCount(targetGrid);
            for (int i = 0; i < count; i++) {
                if (auto child = VisualTreeHelper::GetChild(targetGrid, i).try_as<FrameworkElement>()) {
                    if (winrt::get_class_name(child) == c_ItemsRepeater) {
                        Canvas::SetZIndex(child, s.zIndex + 1);
                    }
                }
            }
        }
    } catch (...) {}
}

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

void InjectContentIntoGrid(FrameworkElement element, std::wstring_view uniqueName) {
    if (!element || g_unloading) return;
    auto grid = element.try_as<Grid>() ? element.as<Grid>() : FindFirstGridInElement(element).try_as<Grid>();
    if (!grid) return;

    try {
        for (auto child : grid.Children()) {
            if (auto fe = child.try_as<FrameworkElement>()) {
                if (fe.Name() == uniqueName) return;
            }
        }
    } catch (...) {}

    bool enabled = false;
    if (uniqueName == c_InjectedControlName) enabled = Wh_GetIntSetting(L"injectTaskbar") != 0;
    else if (uniqueName == L"StartButtonVideoGrid") enabled = Wh_GetIntSetting(L"injectStartButton") != 0;
    else if (uniqueName == L"StartMenuVideoGrid") enabled = Wh_GetIntSetting(L"injectStartMenu") != 0;
    else if (std::wstring_view(uniqueName).find(L"CustomVideoGrid_") == 0) enabled = true;

    if (!enabled) return;

    if (element.ActualWidth() > 0 && element.ActualHeight() > 0) {
        CreateAndInjectVideo(grid, uniqueName);
        return;
    }

    auto weakGrid = winrt::make_weak(grid);
    std::wstring name(uniqueName);
    element.SizeChanged([weakGrid, name](auto const&, auto const&) {
        if (g_unloading) return;
        if (auto g = weakGrid.get()) {
            if (g.ActualWidth() > 0 && g.ActualHeight() > 0) {
                try {
                    for (auto child : g.Children()) {
                        if (auto fe = child.try_as<FrameworkElement>()) {
                            if (fe.Name() == name) return;
                        }
                    }
                } catch (...) {}
                CreateAndInjectVideo(g, name);
            }
        }
    });
}

// --- WinUI 3 Video Operations ---
muxc::Grid FindFirstGridInElementWinUI3(mux::DependencyObject const& root) {
    if (!root) return nullptr;
    if (auto grid = root.try_as<muxc::Grid>()) return grid;
    int count = muxm::VisualTreeHelper::GetChildrenCount(root);
    for (int i = 0; i < count; i++) {
        auto child = muxm::VisualTreeHelper::GetChild(root, i);
        if (auto grid = child.try_as<muxc::Grid>()) return grid;
        if (auto nested = FindFirstGridInElementWinUI3(child)) return nested;
    }
    return nullptr;
}

void RemoveInjectedFromGridWinUI3(muxc::Grid grid) {
    if (!grid) return;
    try {
        auto children = grid.Children();
        for (int i = (int)children.Size() - 1; i >= 0; i--) {
            if (auto fe = children.GetAt(i).try_as<mux::FrameworkElement>()) {
                std::wstring name(fe.Name());
                if (name == L"ExplorerVideoBackgroundGrid" || name.rfind(L"CustomVideoGrid_", 0) == 0) {
                    if (auto container = fe.try_as<muxc::Grid>()) {
                        for (auto child : container.Children()) {
                            if (auto mpe = child.try_as<muxc::MediaPlayerElement>()) {
                                if (auto player = mpe.MediaPlayer()) {
                                    try {
                                        player.Pause();
                                        player.Source(nullptr);
                                    } catch (...) {}
                                    mpe.SetMediaPlayer(nullptr);
                                }
                            }
                        }
                    }
                    children.RemoveAt(i);
                }
            }
        }
    } catch (...) {}
}

void CreateAndInjectVideoWinUI3(muxc::Grid targetGrid, std::wstring_view uniqueName) {
    if (!targetGrid || g_unloading) return;

    try {
        for (auto child : targetGrid.Children()) {
            if (auto fe = child.try_as<mux::FrameworkElement>()) {
                if (fe.Name() == uniqueName) return;
            }
        }
    } catch (...) {}

    InjectionSettings s = GetSettingsForTarget(uniqueName);
    if (s.videoUrl.empty()) return;

    try {
        winrt::Windows::Foundation::Uri videoUri{ nullptr };
        try {
            videoUri = winrt::Windows::Foundation::Uri(s.videoUrl);
        } catch (...) { return; }

        muxc::Grid videoContainer;
        videoContainer.Name(uniqueName);
        videoContainer.HorizontalAlignment(mux::HorizontalAlignment::Stretch);
        videoContainer.VerticalAlignment(mux::VerticalAlignment::Stretch);
        videoContainer.Opacity(0);
        muxc::Canvas::SetZIndex(videoContainer, s.zIndex);
        muxc::Grid::SetColumn(videoContainer, 0);
        muxc::Grid::SetColumnSpan(videoContainer, 100);
        muxc::Grid::SetRow(videoContainer, 0);
        muxc::Grid::SetRowSpan(videoContainer, 100);

        if (s.cornerRadius > 0) {
            videoContainer.CornerRadius(mux::CornerRadius{ 
                (double)s.cornerRadius, (double)s.cornerRadius, (double)s.cornerRadius, (double)s.cornerRadius 
            });
        }

        winrt::Windows::Media::Playback::MediaPlayer mediaPlayer;
        mediaPlayer.Source(winrt::Windows::Media::Core::MediaSource::CreateFromUri(videoUri));
        mediaPlayer.CommandManager().IsEnabled(false);
        mediaPlayer.IsLoopingEnabled(s.loop);
        mediaPlayer.IsMuted(true);
        mediaPlayer.PlaybackRate(s.rate);

        muxc::MediaPlayerElement player;
        player.SetMediaPlayer(mediaPlayer);
        player.Stretch(muxm::Stretch::UniformToFill);
        player.IsHitTestVisible(false);

        videoContainer.Children().Append(player);
        targetGrid.Children().InsertAt(0, videoContainer);

        muxa::DoubleAnimation fadeIn;
        fadeIn.Duration(mux::Duration{ winrt::Windows::Foundation::TimeSpan{ std::chrono::milliseconds(314) } });
        fadeIn.From(0.0);
        fadeIn.To(s.opacity);

        muxa::Storyboard storyboard;
        storyboard.Children().Append(fadeIn);
        muxa::Storyboard::SetTarget(fadeIn, videoContainer);
        muxa::Storyboard::SetTargetProperty(fadeIn, L"Opacity");
        storyboard.Begin();

        mediaPlayer.Play();

        {
            std::lock_guard<std::mutex> lock(g_gridMutex);
            bool found = false;
            for (auto& t : g_trackedGridsWinUI3) {
                if (auto g = t.ref.get()) {
                    if (winrt::get_abi(g) == winrt::get_abi(targetGrid)) {
                        t.playerRef = winrt::make_weak(mediaPlayer);
                        t.uniqueName = std::wstring(uniqueName);
                        found = true;
                        break;
                    }
                }
            }
            if (!found) {
                g_trackedGridsWinUI3.push_back({ winrt::make_weak(targetGrid), winrt::make_weak(mediaPlayer), std::wstring(uniqueName) });
            }
        }
    } catch (...) {}
}

void InjectContentIntoGridWinUI3(mux::FrameworkElement element, std::wstring_view uniqueName) {
    if (!element || g_unloading) return;
    auto grid = element.try_as<muxc::Grid>() ? element.as<muxc::Grid>() : FindFirstGridInElementWinUI3(element);
    if (!grid) return;

    try {
        for (auto child : grid.Children()) {
            if (auto fe = child.try_as<mux::FrameworkElement>()) {
                if (fe.Name() == uniqueName) return;
            }
        }
    } catch (...) {}

    if (element.ActualWidth() > 0 && element.ActualHeight() > 0) {
        CreateAndInjectVideoWinUI3(grid, uniqueName);
        return;
    }

    auto weakGrid = winrt::make_weak(grid);
    std::wstring name(uniqueName);
    element.SizeChanged([weakGrid, name](auto const&, auto const&) {
        if (g_unloading) return;
        if (auto g = weakGrid.get()) {
            if (g.ActualWidth() > 0 && g.ActualHeight() > 0) {
                try {
                    for (auto child : g.Children()) {
                        if (auto fe = child.try_as<mux::FrameworkElement>()) {
                            if (fe.Name() == name) return;
                        }
                    }
                } catch (...) {}
                CreateAndInjectVideoWinUI3(g, name);
            }
        }
    });
}

void ScanWinUI3NodeRecursive(mux::DependencyObject const& node) {
    if (!node || g_unloading) return;

    if (auto fe = node.try_as<mux::FrameworkElement>()) {
        std::wstring currentProc = GetCurrentProcessBaseName();
        std::vector<size_t> matchIndices;
        {
            std::lock_guard<std::mutex> lock(g_modSettings.mutex);
            for (size_t i = 0; i < g_modSettings.customInjections.size(); i++) {
                auto const& ci = g_modSettings.customInjections[i];
                if (currentProc == ci.processName || ci.processName.empty() || ci.processName == L"*") {
                    if (!ci.parsedSegments.empty()) {
                        if (MatchesXamlPathWinUI3(fe, ci.parsedSegments)) {
                            matchIndices.push_back(i);
                        }
                    }
                }
            }
        }

        for (size_t idx : matchIndices) {
            InjectContentIntoGridWinUI3(fe, L"CustomVideoGrid_" + std::to_wstring(idx));
        }
    }

    int count = muxm::VisualTreeHelper::GetChildrenCount(node);
    for (int i = 0; i < count; i++) {
        auto child = muxm::VisualTreeHelper::GetChild(node, i);
        if (child) {
            ScanWinUI3NodeRecursive(child);
        }
    }
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

void ScanAndInjectRecursive(FrameworkElement element) {
    if (!element || g_unloading) return;
    
    std::wstring name(element.Name());
    if (Wh_GetIntSetting(L"injectTaskbar") && name == c_TargetGridName) {
        InjectContentIntoGrid(element, c_InjectedControlName);
    }

    std::wstring currentProc = GetCurrentProcessBaseName();
    std::vector<size_t> matchIndices;
    {
        std::lock_guard<std::mutex> lock(g_modSettings.mutex);
        for (size_t i = 0; i < g_modSettings.customInjections.size(); i++) {
            auto const& ci = g_modSettings.customInjections[i];
            if (currentProc == ci.processName || ci.processName.empty() || ci.processName == L"*") {
                if (!ci.parsedSegments.empty()) {
                    if (MatchesXamlPathUWP(element, ci.parsedSegments)) {
                        matchIndices.push_back(i);
                    }
                }
            }
        }
    }

    for (size_t idx : matchIndices) {
        InjectContentIntoGrid(element, L"CustomVideoGrid_" + std::to_wstring(idx));
    }

    int count = VisualTreeHelper::GetChildrenCount(element);
    for (int i = 0; i < count; i++) {
        auto child = VisualTreeHelper::GetChild(element, i).try_as<FrameworkElement>();
        if (child) ScanAndInjectRecursive(child);
    }
}

void ScheduleScanAsync(FrameworkElement startNode) {
    if (!startNode || g_unloading || g_scanPending.exchange(true)) return;
    auto weak = winrt::make_weak(startNode);
    try {
        startNode.Dispatcher().RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::Low, [weak]() {
            g_scanPending = false;
            if (g_unloading) return;
            if (auto node = weak.get()) {
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
                        
                        ScanAndInjectRecursive(current);

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
    if (!g_unloading) {
        if (auto elem = GetFrameworkElementFromNative(pThis)) {
            ScheduleScanAsync(elem);
        }
    }
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
    
    g_pendingMeasureOverride = true;
    g_taskbarHeight = targetHeight - 1;
    if (double_48_value_Original) {
        double val = (double)g_taskbarHeight;
        ProtectAndMemcpy(PAGE_READWRITE, double_48_value_Original, &val, sizeof(double));
    }
    NotifyAllTaskbarWindows(WM_SETTINGCHANGE, SPI_SETLOGICALDPIOVERRIDE, 0);
    for (int i = 0; i < 100 && g_pendingMeasureOverride; i++) Sleep(100);

    g_pendingMeasureOverride = true;
    g_taskbarHeight = taskbarHeight;
    
    if (double_48_value_Original) {
        double val = (taskbarHeight > 0) ? (double)taskbarHeight : (double)targetHeight;
        ProtectAndMemcpy(PAGE_READWRITE, double_48_value_Original, &val, sizeof(double));
    }

    NotifyAllTaskbarWindows(WM_SETTINGCHANGE, SPI_SETLOGICALDPIOVERRIDE, 0);
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
    if (hForeground && IsZoomed(hForeground)) {
        isCovered = true;
    }

    HWND hLock = FindWindowW(L"LockScreenClass", nullptr);
    if (hLock && IsWindowVisible(hLock)) {
        isCovered = true;
    }

    // Update UWP media players
    {
        std::lock_guard<std::mutex> lock(g_gridMutex);
        for (auto& t : g_trackedGrids) {
            if (auto grid = t.ref.get()) {
                if (auto player = t.playerRef.get()) {
                    std::wstring uName = t.uniqueName;
                    try {
                        grid.Dispatcher().RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::Low, [grid, player, isCovered, uName]() {
                            if (g_unloading) return;
                            try {
                                bool pauseThis = isCovered;
                                if (uName == L"StartMenuVideoGrid") {
                                    if (grid.Visibility() != Visibility::Visible) {
                                        pauseThis = true;
                                    } else {
                                        HWND hStart = FindWindowW(L"XYWidgetHostWindow", nullptr);
                                        if (!hStart) hStart = FindWindowW(L"Windows.UI.Core.CoreWindow", L"Start");
                                        if (hStart && !IsWindowVisible(hStart)) {
                                            pauseThis = true;
                                        }
                                    }
                                }

                                auto state = player.PlaybackSession().PlaybackState();
                                if (pauseThis) {
                                    if (state == winrt::Windows::Media::Playback::MediaPlaybackState::Playing) player.Pause();
                                } else {
                                    if (state == winrt::Windows::Media::Playback::MediaPlaybackState::Paused) player.Play();
                                }
                            } catch (...) {}
                        });
                    } catch (...) {}
                }
            }
        }

        // Update WinUI 3 media players
        for (auto& t : g_trackedGridsWinUI3) {
            if (auto grid = t.ref.get()) {
                if (auto player = t.playerRef.get()) {
                    try {
                        grid.DispatcherQueue().TryEnqueue(mud::DispatcherQueuePriority::Low, [player, isCovered]() {
                            if (g_unloading) return;
                            try {
                                auto state = player.PlaybackSession().PlaybackState();
                                if (isCovered) {
                                    if (state == winrt::Windows::Media::Playback::MediaPlaybackState::Playing) player.Pause();
                                } else {
                                    if (state == winrt::Windows::Media::Playback::MediaPlaybackState::Paused) player.Play();
                                }
                            } catch (...) {}
                        });
                    } catch (...) {}
                }
            }
        }
    }
}

// --- Start Menu & Custom UWP Logic ---
template <typename Predicate>
FrameworkElement FindChildRecursive(DependencyObject parent, Predicate&& predicate) {
    if (!parent) return nullptr;
    int count = VisualTreeHelper::GetChildrenCount(parent);
    for (int i = 0; i < count; i++) {
        auto child = VisualTreeHelper::GetChild(parent, i).try_as<FrameworkElement>();
        if (!child) continue;
        if (predicate(child)) return child;
        auto found = FindChildRecursive(child, predicate);
        if (found) return found;
    }
    return nullptr;
}

void InjectStartMenuVideo() {
    if (!Wh_GetIntSetting(L"injectStartMenu") || g_unloading) return;

    std::wstring proc = GetCurrentProcessBaseName();
    if (proc.find(L"searchhost.exe") != std::wstring::npos || proc.find(L"searchapp.exe") != std::wstring::npos) {
        return;
    }

    auto window = Window::Current();
    if (!window) return;
    auto content = window.Content();
    if (!content) return;

    auto target = FindChildRecursive(content, [](FrameworkElement const& fe) {
        std::wstring name(fe.Name());
        return name == L"MainMenu" || name == L"RootGrid" || name == L"Root" || name == L"StartMenuRoot" || name == L"FrameGrid";
    }).try_as<Grid>();

    if (target) {
        InjectContentIntoGrid(target, L"StartMenuVideoGrid");
    }
}

void InjectCustomVideo() {
    if (g_unloading) return;
    auto window = Window::Current();
    if (!window) return;
    auto content = window.Content();
    if (!content) return;

    std::wstring currentProc = GetCurrentProcessBaseName();

    std::lock_guard<std::mutex> lock(g_modSettings.mutex);
    for (size_t i = 0; i < g_modSettings.customInjections.size(); i++) {
        auto const& ci = g_modSettings.customInjections[i];
        std::wstring ciProc = ci.processName;
        std::transform(ciProc.begin(), ciProc.end(), ciProc.begin(), ::towlower);

        if (currentProc == ciProc || ciProc.empty() || ciProc == L"*") {
            if (ci.parsedSegments.empty()) continue;

            auto target = FindChildRecursive(content, [&ci](FrameworkElement const& fe) {
                return MatchesXamlPathUWP(fe, ci.parsedSegments);
            }).try_as<Grid>();

            if (target) {
                InjectContentIntoGrid(target, L"CustomVideoGrid_" + std::to_wstring(i));
            }
        }
    }
}

void StartMenuInit() {
    if (g_layoutUpdatedToken || g_unloading) return;

    auto window = Window::Current();
    if (!window) return;

    if (!g_visibilityChangedToken) {
        g_visibilityChangedToken = window.VisibilityChanged([](auto const&, winrt::Windows::UI::Core::VisibilityChangedEventArgs const& args) {
            if (args.Visible() && !g_unloading) {
                g_applyPending = true;
            }
        });
    }

    if (auto contentUI = window.Content()) {
        auto content = contentUI.as<FrameworkElement>();
        g_layoutUpdatedToken = content.LayoutUpdated([](auto const&, auto const&) {
            if (g_applyPending && !g_unloading) {
                g_applyPending = false;
                InjectStartMenuVideo();
                InjectCustomVideo();
            }
        });
    }

    InjectStartMenuVideo();
    InjectCustomVideo();
}

using RunFromWindowThreadProc_t = void(WINAPI*)(PVOID parameter);
bool RunFromWindowThread(HWND hWnd, RunFromWindowThreadProc_t proc, PVOID procParam) {
    static const UINT runMsg = RegisterWindowMessageW(L"WH_RunThread_XamlVideoInjector");
    DWORD tid = GetWindowThreadProcessId(hWnd, nullptr);
    if (tid == GetCurrentThreadId()) { proc(procParam); return true; }

    HHOOK hook = SetWindowsHookExW(WH_CALLWNDPROC, [](int n, WPARAM w, LPARAM l) -> LRESULT {
        if (n == HC_ACTION) {
            auto cwp = (CWPSTRUCT*)l;
            if (cwp->message == RegisterWindowMessageW(L"WH_RunThread_XamlVideoInjector")) {
                struct P { RunFromWindowThreadProc_t p; PVOID pp; } *m = (P*)cwp->lParam;
                if (m && m->p) m->p(m->pp);
            }
        }
        return CallNextHookEx(nullptr, n, w, l);
    }, nullptr, tid);

    struct P { RunFromWindowThreadProc_t p; PVOID pp; } m = { proc, procParam };
    SendMessage(hWnd, runMsg, 0, (LPARAM)&m);
    UnhookWindowsHookEx(hook);
    return true;
}

HWND GetCoreWnd() {
    HWND res = nullptr;
    EnumWindows([](HWND h, LPARAM lp) -> BOOL {
        DWORD pid; GetWindowThreadProcessId(h, &pid);
        if (pid != GetCurrentProcessId()) return TRUE;
        WCHAR cls[64]; GetClassName(h, cls, 64);
        if (wcscmp(cls, L"Windows.UI.Core.CoreWindow") == 0) { *(HWND*)lp = h; return FALSE; }
        return TRUE;
    }, (LPARAM)&res);
    return res;
}

HRESULT WINAPI RoGetActivationFactory_Hook(HSTRING cls, REFIID iid, void** f) {
    if (cls) {
        PCWSTR raw = WindowsGetStringRawBuffer(cls, nullptr);
        if (raw) {
            if (wcscmp(raw, L"Windows.UI.Xaml.Hosting.XamlIsland") == 0 ||
                wcscmp(raw, L"Windows.UI.Xaml.Application") == 0) {
                HWND h = GetCoreWnd();
                if (h) RunFromWindowThread(h, [](PVOID) { StartMenuInit(); }, nullptr);
            }
        }
    }
    return RoGetActivationFactory_Original(cls, iid, f);
}

// --- Hooks ---
double WINAPI TaskbarConfiguration_GetFrameSize_Hook(int enumTaskbarSize) {
    if (g_taskbarHeight > 0 && (enumTaskbarSize == 1 || enumTaskbarSize == 2)) return (double)g_taskbarHeight;
    return TaskbarConfiguration_GetFrameSize_Original(enumTaskbarSize);
}

void WINAPI TaskListButton_UpdateVisualStates_Hook(void* pThis) {
    g_hookCallCounter++;
    TaskListButton_UpdateVisualStates_Original(pThis);
    if (!g_unloading) {
        if (auto elem = GetFrameworkElementFromNative(pThis)) ScheduleScanAsync(elem);
    }
    g_hookCallCounter--;
}

using SHAppBarMessage_t = decltype(&SHAppBarMessage);
SHAppBarMessage_t SHAppBarMessage_Original;
UINT_PTR WINAPI SHAppBarMessage_Hook(DWORD dwMessage, PAPPBARDATA pData) {
    auto ret = SHAppBarMessage_Original(dwMessage, pData);
    if (dwMessage == ABM_QUERYPOS && ret && g_taskbarHeight > 0 && pData) {
        pData->rc.top = pData->rc.bottom - MulDiv(g_taskbarHeight, GetDpiForWindow(pData->hWnd), 96);
    }
    return ret;
}

// --- WinUI 3 File Explorer Scanning & Scheduling Helpers ---
void ScanXamlRootForCommandBars(mux::UIElement const& element) {
    if (g_unloading || !element) return;
    try {
        auto xamlRoot = element.XamlRoot();
        if (!xamlRoot) return;
        auto content = xamlRoot.Content();
        if (!content) return;
        ScanWinUI3NodeRecursive(content);
    } catch (...) {}
}

void ScheduleXamlRootScan(mux::UIElement const& element) {
    if (g_unloading || !element) return;
    try {
        auto dispatcherQueue = mud::DispatcherQueue::GetForCurrentThread();
        if (!dispatcherQueue) {
            ScanXamlRootForCommandBars(element);
            return;
        }
        dispatcherQueue.TryEnqueue([weakElement = winrt::make_weak(element)]() {
            if (auto el = weakElement.get()) {
                ScanXamlRootForCommandBars(el);
            }
        });
    } catch (...) {}
}

void ScanCurrentThreadForCommandBars() {
    if (g_unloading) return;
    try {
        auto focused = mux::Input::FocusManager::GetFocusedElement();
        auto element = focused ? focused.try_as<mux::UIElement>() : nullptr;
        if (element) {
            ScanXamlRootForCommandBars(element);
        }
    } catch (...) {}
}

void ScheduleCurrentThreadScan() {
    if (g_unloading) return;
    try {
        auto dispatcherQueue = mud::DispatcherQueue::GetForCurrentThread();
        if (!dispatcherQueue) {
            ScanCurrentThreadForCommandBars();
            return;
        }
        dispatcherQueue.TryEnqueue([]() {
            ScanCurrentThreadForCommandBars();
        });
    } catch (...) {}
}

std::vector<HWND> GetFileExplorerWnds() {
    std::vector<HWND> hWnds;
    EnumWindows(
        [](HWND hWnd, LPARAM lParam) -> BOOL {
            auto& hWnds = *(std::vector<HWND>*)lParam;

            DWORD dwProcessId = 0;
            if (!GetWindowThreadProcessId(hWnd, &dwProcessId) ||
                dwProcessId != GetCurrentProcessId()) {
                return TRUE;
            }

            WCHAR className[64];
            if (GetClassName(hWnd, className, ARRAYSIZE(className)) &&
                _wcsicmp(className, L"CabinetWClass") == 0) {
                hWnds.push_back(hWnd);
            }

            return TRUE;
        },
        (LPARAM)&hWnds);

    return hWnds;
}

// --- WinUI 3 Hooks (FileExplorerExtensions) ---
void WINAPI CommandBarManager_CommandBar_Hook(void* pThis, void* commandBar) {
    CommandBarManager_CommandBar_Original(pThis, commandBar);
    if (g_unloading || !commandBar) return;
    try {
        auto const& element = *reinterpret_cast<muxc::CommandBar const*>(commandBar);
        if (!element) return;
        ScheduleXamlRootScan(element);
    } catch (...) {}
}

void WINAPI CommandBarControl_OnApplyTemplate_Hook(void* pThis) {
    CommandBarControl_OnApplyTemplate_Original(pThis);
    ScheduleCurrentThreadScan();
}

void WINAPI CommandBarControl_Wave1_OnApplyTemplate_Hook(void* pThis) {
    CommandBarControl_Wave1_OnApplyTemplate_Original(pThis);
    ScheduleCurrentThreadScan();
}

void HandleCommandBarControlGotFocus(void* sender) {
    if (g_unloading || !sender) return;
    try {
        auto const& inspectable = *reinterpret_cast<winrt::Windows::Foundation::IInspectable const*>(sender);
        if (auto element = inspectable ? inspectable.try_as<mux::UIElement>() : nullptr) {
            ScanXamlRootForCommandBars(element);
        }
    } catch (...) {}
}

void WINAPI CommandBarControl_GotFocus_Hook(void* pThis, void* sender, void* args) {
    CommandBarControl_GotFocus_Original(pThis, sender, args);
    HandleCommandBarControlGotFocus(sender);
}

void WINAPI CommandBarControl_Wave1_GotFocus_Hook(void* pThis, void* sender, void* args) {
    CommandBarControl_Wave1_GotFocus_Original(pThis, sender, args);
    HandleCommandBarControlGotFocus(sender);
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

bool HookFileExplorerExtensionsDllSymbols(HMODULE module) {
    WindhawkUtils::SYMBOL_HOOK symbolHooks[] = {
        {
            {
                LR"(public: void __cdecl winrt::FileExplorerExtensions::implementation::CommandBarManager::CommandBar(struct winrt::Microsoft::UI::Xaml::Controls::CommandBar const &))",
                LR"(public: void __cdecl winrt::FileExplorerExtensions::implementation::CommandBarManager::CommandBar(struct winrt::Microsoft::UI::Xaml::Controls::CommandBar const & __ptr64) __ptr64)",
            },
            (void**)&CommandBarManager_CommandBar_Original,
            (void*)CommandBarManager_CommandBar_Hook,
            true,
        },
        {
            {
                LR"(public: void __cdecl winrt::FileExplorerExtensions::implementation::CommandBarControl::OnApplyTemplate(void))",
                LR"(public: void __cdecl winrt::FileExplorerExtensions::implementation::CommandBarControl::OnApplyTemplate(void) __ptr64)",
            },
            (void**)&CommandBarControl_OnApplyTemplate_Original,
            (void*)CommandBarControl_OnApplyTemplate_Hook,
            true,
        },
        {
            {
                LR"(public: void __cdecl winrt::FileExplorerExtensions::implementation::CommandBarControl_Wave1::OnApplyTemplate(void))",
                LR"(public: void __cdecl winrt::FileExplorerExtensions::implementation::CommandBarControl_Wave1::OnApplyTemplate(void) __ptr64)",
            },
            (void**)&CommandBarControl_Wave1_OnApplyTemplate_Original,
            (void*)CommandBarControl_Wave1_OnApplyTemplate_Hook,
            true,
        },
        {
            {
                LR"(public: void __cdecl winrt::FileExplorerExtensions::implementation::CommandBarControl::CommandBarControlGotFocusHandler(struct winrt::Windows::Foundation::IInspectable const &,struct winrt::Microsoft::UI::Xaml::RoutedEventArgs const &))",
                LR"(public: void __cdecl winrt::FileExplorerExtensions::implementation::CommandBarControl::CommandBarControlGotFocusHandler(struct winrt::Windows::Foundation::IInspectable const & __ptr64,struct winrt::Microsoft::UI::Xaml::RoutedEventArgs const & __ptr64) __ptr64)",
            },
            (void**)&CommandBarControl_GotFocus_Original,
            (void*)CommandBarControl_GotFocus_Hook,
            true,
        },
        {
            {
                LR"(public: void __cdecl winrt::FileExplorerExtensions::implementation::CommandBarControl_Wave1::CommandBarControlGotFocusHandler(struct winrt::Windows::Foundation::IInspectable const &,struct winrt::Microsoft::UI::Xaml::RoutedEventArgs const &))",
                LR"(public: void __cdecl winrt::FileExplorerExtensions::implementation::CommandBarControl_Wave1::CommandBarControlGotFocusHandler(struct winrt::Windows::Foundation::IInspectable const & __ptr64,struct winrt::Microsoft::UI::Xaml::RoutedEventArgs const & __ptr64) __ptr64)",
            },
            (void**)&CommandBarControl_Wave1_GotFocus_Original,
            (void*)CommandBarControl_Wave1_GotFocus_Hook,
            true,
        },
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
    if (module && lpLibFileName && !g_unloading) {
        PCWSTR fileName = lpLibFileName;
        for (PCWSTR p = lpLibFileName; *p; p++) {
            if (*p == L'\\' || *p == L'/') {
                fileName = p + 1;
            }
        }

        if (!g_taskbarViewDllLoaded && (_wcsicmp(fileName, L"Taskbar.View.dll") == 0 || _wcsicmp(fileName, L"ExplorerExtensions.dll") == 0)) {
            if (!g_taskbarViewDllLoaded.exchange(true)) {
                HookTaskbarViewDllSymbols(module);
                Wh_ApplyHookOperations();
            }
        }
        if (!g_fileExplorerExtDllLoaded && (_wcsicmp(fileName, L"FileExplorerExtensions.dll") == 0 || _wcsicmp(fileName, L"FileExplorerExtensions") == 0)) {
            if (!g_fileExplorerExtDllLoaded.exchange(true)) {
                HookFileExplorerExtensionsDllSymbols(module);
                Wh_ApplyHookOperations();
            }
        }
    }
    return module;
}

void Wh_ModSettingsChanged() {
    LoadSettings();
    
    {
        std::lock_guard<std::mutex> lock(g_pendingMutex);
        g_scannedFrames.clear();
    }

    HWND hCore = GetCoreWnd();
    if (hCore) {
        RunFromWindowThread(hCore, [](PVOID) { 
            InjectStartMenuVideo(); 
            InjectCustomVideo();
        }, nullptr);
    }

    // Refresh UWP Grids
    {
        std::lock_guard<std::mutex> lock(g_gridMutex);
        for (auto& tracked : g_trackedGrids) {
            if (auto grid = tracked.ref.get()) {
                std::wstring uName = tracked.uniqueName;
                try {
                    grid.Dispatcher().RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::Normal, [grid, uName]() {
                        if (g_unloading) return;
                        RemoveInjectedFromGrid(grid);
                        
                        if (uName == c_InjectedControlName) {
                            if (Wh_GetIntSetting(L"injectTaskbar")) CreateAndInjectVideo(grid, c_InjectedControlName);
                        } else if (uName == L"StartButtonVideoGrid") {
                            if (Wh_GetIntSetting(L"injectStartButton")) CreateAndInjectVideo(grid, L"StartButtonVideoGrid");
                        } else if (uName == L"StartMenuVideoGrid") {
                            if (Wh_GetIntSetting(L"injectStartMenu")) CreateAndInjectVideo(grid, L"StartMenuVideoGrid");
                        } else if (uName.rfind(L"CustomVideoGrid_", 0) == 0) {
                            CreateAndInjectVideo(grid, uName);
                        }
                    });
                } catch (...) {}
            }
        }

        // Refresh WinUI 3 Grids
        for (auto& tracked : g_trackedGridsWinUI3) {
            if (auto grid = tracked.ref.get()) {
                std::wstring uName = tracked.uniqueName;
                try {
                    grid.DispatcherQueue().TryEnqueue(mud::DispatcherQueuePriority::Normal, [grid, uName]() {
                        if (g_unloading) return;
                        RemoveInjectedFromGridWinUI3(grid);
                        CreateAndInjectVideoWinUI3(grid, uName);
                    });
                } catch (...) {}
            }
        }
    }
    ApplySettings(g_taskbarHeight);

    std::wstring proc = GetCurrentProcessBaseName();
    if (proc == L"explorer.exe") {
        for (HWND hWnd : GetFileExplorerWnds()) {
            RunFromWindowThread(hWnd, [](PVOID) {
                ScanCurrentThreadForCommandBars();
            }, nullptr);
        }
    }
}

BOOL Wh_ModInit() {
    LoadSettings();
    
    std::wstring proc = GetCurrentProcessBaseName();
    bool isExplorer = (proc == L"explorer.exe");
    
    bool isCustomTarget = false;
    {
        std::lock_guard<std::mutex> lock(g_modSettings.mutex);
        for (auto const& ci : g_modSettings.customInjections) {
            std::wstring customProc = ci.processName;
            std::transform(customProc.begin(), customProc.end(), customProc.begin(), ::towlower);
            if (proc.find(customProc) != std::wstring::npos) {
                isCustomTarget = true;
                break;
            }
        }
    }

    if (isExplorer) {
        HookTaskbarDllSymbols();
        HMODULE mod = GetModuleHandle(L"Taskbar.View.dll");
        if (!mod) mod = GetModuleHandle(L"ExplorerExtensions.dll");
        if (mod) {
            g_taskbarViewDllLoaded = true;
            HookTaskbarViewDllSymbols(mod);
        }

        HMODULE feeMod = GetModuleHandle(L"FileExplorerExtensions.dll");
        if (feeMod) {
            g_fileExplorerExtDllLoaded = true;
            HookFileExplorerExtensionsDllSymbols(feeMod);
        }

        WindhawkUtils::SetFunctionHook(LoadLibraryExW, LoadLibraryExW_Hook, &LoadLibraryExW_Original);
        WindhawkUtils::SetFunctionHook(SHAppBarMessage, SHAppBarMessage_Hook, &SHAppBarMessage_Original);
    } 
    
    if (isCustomTarget || proc == L"startmenuexperiencehost.exe" || 
        proc == L"searchhost.exe" || proc == L"searchapp.exe") {
        HMODULE rt = GetModuleHandle(L"api-ms-win-core-winrt-l1-1-0.dll");
        if (rt) {
            auto pRo = (RoGetActivationFactory_t)GetProcAddress(rt, "RoGetActivationFactory");
            if (pRo) {
                WindhawkUtils::SetFunctionHook((void*)pRo, (void*)RoGetActivationFactory_Hook, (void**)&RoGetActivationFactory_Original);
            }
        }
    }
    
    g_perfTimerId = SetTimer(nullptr, 0, 1000, PerformanceTimerProc);

    return TRUE;
}

void Wh_ModAfterInit() { 
    std::wstring proc = GetCurrentProcessBaseName();

    if (proc == L"explorer.exe") {
        ApplySettings(g_taskbarHeight); 
        for (HWND hWnd : GetFileExplorerWnds()) {
            RunFromWindowThread(hWnd, [](PVOID) {
                ScanCurrentThreadForCommandBars();
            }, nullptr);
        }
    } 
    
    HWND h = GetCoreWnd();
    if (h) RunFromWindowThread(h, [](PVOID) { StartMenuInit(); }, nullptr);
}

void Wh_ModBeforeUninit() { 
    g_unloading = true; 
    if (g_perfTimerId) {
        KillTimer(nullptr, g_perfTimerId);
        g_perfTimerId = 0;
    }
}

void Wh_ModUninit() {
    g_unloading = true;
    if (g_perfTimerId) {
        KillTimer(nullptr, g_perfTimerId);
        g_perfTimerId = 0;
    }

    while (g_hookCallCounter > 0) Sleep(50);

    if (double_48_value_Original) {
        double defaultHeight = 48.0;
        ProtectAndMemcpy(PAGE_READWRITE, double_48_value_Original, &defaultHeight, sizeof(double));
        NotifyAllTaskbarWindows(WM_SETTINGCHANGE, SPI_SETLOGICALDPIOVERRIDE, 0);
    }

    std::vector<TrackedGridRef> localUwp;
    std::vector<TrackedGridRefWinUI3> localWinUI3;
    { 
        std::lock_guard<std::mutex> lock(g_gridMutex); 
        localUwp = std::move(g_trackedGrids); 
        localWinUI3 = std::move(g_trackedGridsWinUI3); 
    }

    int totalPending = (int)localUwp.size() + (int)localWinUI3.size();
    if (totalPending > 0) {
        HANDLE hCleanupDone = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        std::atomic<int> pendingCleanups{ totalPending };

        for (auto& t : localUwp) {
            if (auto g = t.ref.get()) {
                try {
                    g.Dispatcher().RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::Normal, [g, &pendingCleanups, hCleanupDone]() {
                        try {
                            RemoveInjectedFromGrid(g);
                        } catch (...) {}
                        if (--pendingCleanups <= 0) {
                            SetEvent(hCleanupDone);
                        }
                    });
                } catch (...) {
                    if (--pendingCleanups <= 0) SetEvent(hCleanupDone);
                }
            } else {
                if (--pendingCleanups <= 0) SetEvent(hCleanupDone);
            }
        }

        for (auto& t : localWinUI3) {
            if (auto g = t.ref.get()) {
                try {
                    g.DispatcherQueue().TryEnqueue(mud::DispatcherQueuePriority::Normal, [g, &pendingCleanups, hCleanupDone]() {
                        try {
                            RemoveInjectedFromGridWinUI3(g);
                        } catch (...) {}
                        if (--pendingCleanups <= 0) {
                            SetEvent(hCleanupDone);
                        }
                    });
                } catch (...) {
                    if (--pendingCleanups <= 0) SetEvent(hCleanupDone);
                }
            } else {
                if (--pendingCleanups <= 0) SetEvent(hCleanupDone);
            }
        }

        WaitForSingleObject(hCleanupDone, 1200);
        CloseHandle(hCleanupDone);
    }
}
