// ==WindhawkMod==
// @id              startmenu-video-injector-pro
// @name            Start Menu Video Injector
// @description     Injects a video player using VisibilityChanged and LayoutUpdated listeners
// @version         1.8
// @author          Bbmaster123 / AI
// @include         StartMenuExperienceHost.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lruntimeobject -lshcore
// ==/WindhawkMod==

// ==WindhawkModSettings==
/*
- videoUrl: "https://cdn.pixabay.com/video/2025/12/21/323513_tiny.mp4"
  $name: Video URL
- loop: true
  $name: Loop video
- rate: 100
  $name: Playback rate (100 = 1x)
- opacity: 100
  $name: Video opacity (0-100)
- zIndex: -1
  $name: Z-Index
*/
// ==/WindhawkModSettings==

#include <windhawk_utils.h>
#include <roapi.h>
#include <winrt/base.h>
#include <winstring.h>

#undef GetCurrentTime

#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.Media.Core.h>
#include <winrt/Windows.Media.Playback.h>

#include <functional>

using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Media;

const std::wstring c_InjectedGridName = L"StartMenuVideoGrid";

// --- State Management ---
bool g_applyPending = false;
winrt::event_token g_layoutUpdatedToken{};
winrt::event_token g_visibilityChangedToken{};

// --- Helpers ---
FrameworkElement FindChildRecursive(DependencyObject parent, std::function<bool(FrameworkElement)> predicate) {
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

// --- Injection Logic ---
// --- Persistence ---
// We need to keep these alive, otherwise the video stops when the function ends
struct VideoInstance {
    winrt::Windows::Media::Playback::MediaPlayer player{ nullptr };
    MediaPlayerElement element{ nullptr };
};
std::unique_ptr<VideoInstance> g_videoInstance;

// --- Updated Injection Logic ---
void InjectVideo() {
    auto window = Window::Current();
    if (!window) return;
    auto content = window.Content();
    if (!content) return;

    auto target = FindChildRecursive(content, [](FrameworkElement fe) {
        return fe.Name() == L"MainMenu" || fe.Name() == L"RootGrid";
    }).try_as<Grid>();

    if (!target) return;

    try {
        auto children = target.Children();
        for (uint32_t i = 0; i < children.Size(); ++i) {
            if (auto fe = children.GetAt(i).try_as<FrameworkElement>()) {
                if (fe.Name() == c_InjectedGridName) return; 
            }
        }

        g_videoInstance = std::make_unique<VideoInstance>();
        g_videoInstance->player = winrt::Windows::Media::Playback::MediaPlayer();
        
        // STABILITY: Disable real-time sync to reduce tearing during UI animations
        g_videoInstance->player.RealTimePlayback(false);

        auto source = winrt::Windows::Media::Core::MediaSource::CreateFromUri(
            winrt::Windows::Foundation::Uri(Wh_GetStringSetting(L"videoUrl"))
        );
        
        g_videoInstance->player.Source(source);
        g_videoInstance->player.IsLoopingEnabled(Wh_GetIntSetting(L"loop") != 0);
        g_videoInstance->player.IsMuted(true);

        g_videoInstance->element = MediaPlayerElement();
        g_videoInstance->element.SetMediaPlayer(g_videoInstance->player);
        g_videoInstance->element.Stretch(Stretch::UniformToFill);
        g_videoInstance->element.Opacity((double)Wh_GetIntSetting(L"opacity") / 100.0);
        g_videoInstance->element.IsHitTestVisible(false);

        Grid container;
        container.Name(c_InjectedGridName);
        Canvas::SetZIndex(container, Wh_GetIntSetting(L"zIndex"));

        // CORNER RADIUS: Correct implementation
        // Sets all 4 corners to 12px. Adjust as needed.
        container.CornerRadius(winrt::Windows::UI::Xaml::CornerRadius{ 5, 5, 5, 5 });
        
        // STABILITY: Prevent "alpha bleed" flickering by giving it a solid base
        container.Background(SolidColorBrush(winrt::Windows::UI::Colors::Black()));
        
        // This ensures the video content itself is clipped to the CornerRadius
        container.BorderBrush(SolidColorBrush(winrt::Windows::UI::Colors::Transparent()));
        container.BorderThickness(winrt::Windows::UI::Xaml::Thickness{ 0 });

        container.Children().Append(g_videoInstance->element);

        target.Children().InsertAt(0, container);
        g_videoInstance->player.Play();
        
    } catch (...) {
        Wh_Log(L"Injection failed.");
    }
}
// --- Lifecycle (m417z style) ---

void Init() {
    if (g_layoutUpdatedToken) return;

    auto window = Window::Current();
    if (!window) return;

    if (!g_visibilityChangedToken) {
        g_visibilityChangedToken = window.VisibilityChanged([](auto const&, winrt::Windows::UI::Core::VisibilityChangedEventArgs const& args) {
            Wh_Log(L"Window visibility changed: %d", args.Visible());
            if (args.Visible()) {
                g_applyPending = true;
            }
        });
    }

    if (auto contentUI = window.Content()) {
        auto content = contentUI.as<FrameworkElement>();
        g_layoutUpdatedToken = content.LayoutUpdated([](auto const&, auto const&) {
            if (g_applyPending) {
                g_applyPending = false;
                InjectVideo();
            }
        });
    }

    InjectVideo();
}

// --- Window Thread Hooking ---

using RunFromWindowThreadProc_t = void(WINAPI*)(PVOID parameter);
bool RunFromWindowThread(HWND hWnd, RunFromWindowThreadProc_t proc, PVOID procParam) {
    static const UINT runMsg = RegisterWindowMessage(L"WH_RunThread_" WH_MOD_ID);
    DWORD tid = GetWindowThreadProcessId(hWnd, nullptr);
    if (tid == GetCurrentThreadId()) { proc(procParam); return true; }

    HHOOK hook = SetWindowsHookEx(WH_CALLWNDPROC, [](int n, WPARAM w, LPARAM l) -> LRESULT {
        if (n == HC_ACTION) {
            auto cwp = (CWPSTRUCT*)l;
            if (cwp->message == RegisterWindowMessage(L"WH_RunThread_" WH_MOD_ID)) {
                struct P { RunFromWindowThreadProc_t p; PVOID pp; } *m = (P*)cwp->lParam;
                m->p(m->pp);
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

// --- Hooks ---

using RoGetActivationFactory_t = decltype(&RoGetActivationFactory);
RoGetActivationFactory_t RoGetActivationFactory_Original;

HRESULT WINAPI RoGetActivationFactory_Hook(HSTRING cls, REFIID iid, void** f) {
    if (wcscmp(WindowsGetStringRawBuffer(cls, nullptr), L"Windows.UI.Xaml.Hosting.XamlIsland") == 0) {
        HWND h = GetCoreWnd();
        if (h) RunFromWindowThread(h, [](PVOID) { Init(); }, nullptr);
    }
    return RoGetActivationFactory_Original(cls, iid, f);
}

BOOL Wh_ModInit() {
    Wh_Log(L"Mod Initializing");
    HMODULE rt = GetModuleHandle(L"api-ms-win-core-winrt-l1-1-0.dll");
    auto pRo = (RoGetActivationFactory_t)GetProcAddress(rt, "RoGetActivationFactory");
    WindhawkUtils::SetFunctionHook((void*)pRo, (void*)RoGetActivationFactory_Hook, (void**)&RoGetActivationFactory_Original);
    return TRUE;
}

void Wh_ModAfterInit() {
    HWND h = GetCoreWnd();
    if (h) RunFromWindowThread(h, [](PVOID) { Init(); }, nullptr);
}

void Wh_ModSettingsChanged() {
    HWND h = GetCoreWnd();
    if (h) RunFromWindowThread(h, [](PVOID) { InjectVideo(); }, nullptr);
}
