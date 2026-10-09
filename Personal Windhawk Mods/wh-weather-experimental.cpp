// ==WindhawkMod==
// @id              wh-weather
// @name            Windhawk Weather
// @description     A weather widget and flyout that injects directly into the Windows Taskbar. Inspired by ExplorerPatcher's weather widget :)
// @version         1.0.0
// @author          bbmaster123/Gemini
// @github          https://github.com/bbmaster123
// @include         explorer.exe
// @compilerOptions -DWINVER=0x0A00 -lgdi32 -luser32 -luxtheme -lwinhttp -lshlwapi -lole32 -luuid -lshell32 -loleaut32 -ldwmapi -lruntimeobject -lshcore -lversion -lcomctl32 -ld2d1 -ldwrite
// ==/WindhawkMod==
// ==WindhawkModReadme==
/*
- Auto or custom location provided by Open-Meteo
- Toggle alert banner on or off. Click to see multiple alerts.
- Celsius/Fareinheit
- Emoji/Glyph icon sets
- Temperature, Precipitation, humidity, Wind speed/direction
- Interactive global precipitation forecast map with a 24-hour timeline and zoom controls (ECCC GeoMet)
- Precipitation forecast data source: Environment and Climate Change Canada, under its Data Server End-use Licence
- Forecast base map imagery: Esri
- Scrollable graph with clickable hours
- Adjustable taskbar position offset
- Font adjustments
- Acrylic toggle and Opacity
- Background Image/Video
- Up to 16 days forcast (scroll for more)
- Hotkey activation option
- Inject to Tray or existing Windows Weather Panel button (Windows 11)
- Panel opens to current time and day rounded down to the nearest hour
- Tested on Builds ranging from 19045 to 26300
# Windows 10
![Screenshot](https://raw.githubusercontent.com/bbmaster123/FWFU/refs/heads/main/Assets/win10weather.png)
# Windows 11
![Screenshot](https://raw.githubusercontent.com/bbmaster123/FWFU/refs/heads/main/Assets/weather.png)
# Taskbar Button
![Screenshot](https://raw.githubusercontent.com/bbmaster123/FWFU/refs/heads/main/Assets/weatherbutton.png)
*/
// ==/WindhawkModReadme==
// ==WindhawkModSettings==
/*
- debugLogs: false
  $name: Enable Debug Logging
  $description: Check to log detailed diagnostic messages to the Windhawk console

- location: "auto"
  $name: Location (City Name or "auto" for IP-based Geolocation)
- showWeatherAlerts: true
  $name: Show Weather Alerts Banner
- useCelsius: true
  $name: Use Metric System (°C, km/h)
  $description: Uncheck to use Imperial System (°F, mph)
- showTempDecimals: true
  $name: Show Decimal in Temperature
  $description: Uncheck to round temperature values to nearest whole degree (e.g. 72° instead of 72.4°)
- updateInterval: 15
  $name: Update Interval (between 5 and 120 minutes)
- weatherStyle: "0"
  $name: Weather Icon Style
  $description: Choose the visual style for taskbar and forecast icons
  $options:
    - "0": "Modern Color Emoji"
    - "1": "Segoe Vector Glyphs"
    - "2": "MSN Weather Native Taskbar SVGs"
    - "3": "Fluent 3D Color Emojis"
    - "4": Custom Icon Pack Folder or URL
- customIconPackPath: ""
  $name: Custom Icon Pack Folder or URL
  $description: "For Style 4: Local folder (e.g. C:\\WeatherIcons) or URL containing condition PNG/SVG icons (sunny.png, cloudy.png, etc.)"
- showConditionName: true
  $name: Show Condition Name (e.g. "Sunny", "Cloudy")
- itemsRepeaterOffset: 110
  $name: Taskbar Items Shift (Windows 11 modern taskbar)
  $description: Pixels to push app icons away from the injected weather on the modern taskbar
- textOffset: 10
  $name: Horizontal Offset (Pixels)
- textColor: "#FFFFFF"
  $name: Text Color
- iconFontSize: 16
  $name: Weather Icon Font Size
- line1FontFamily: "Segoe UI"
  $name: Line 1 Font Family
  $description: Font family for first line of text
- line1FontSize: 13
  $name: Line 1 Font Size
- line1Bold: true
  $name: Line 1 Bold Toggle
  $description: Enable bold for line 1 text
- line2FontFamily: "Segoe UI"
  $name: Line 2 Font Family
  $description: Font family for second line of text (Condition Name)
- line2FontSize: 11
  $name: Line 2 Font Size
- line2Bold: false
  $name: Line 2 Bold Toggle
  $description: Enable bold for line 2 text
- useAcrylic: true
  $name: Enable Acrylic Effect in Forecast Panel
- acrylicOpacity: 50
  $name: Acrylic Opacity (0-100)
- bgImageVideoUrl: ""
  $name: Background Image or Video URL
  $description: URL to an image (.jpg, .png) or video (.mp4) to display over the acrylic background
- bgImageVideoOpacity: 30
  $name: Background Image/Video Opacity (0-100)
  $description: Opacity of the background image or video overlay
- bgImageVideoStretch: "3"
  $name: Background Image/Video Fit
  $description: Choose how background images and videos fit inside the forecast panel
  $options:
    - "0": "No Scaling"
    - "1": "Stretch to Fill"
    - "2": "Fit (Uniform)"
    - "3": "Fill / Crop (Uniform to Fill)"
- forecastDaysFetch: 14
  $name: Forecast API query length (Days, max 16)
- injectToSysTray: false
  $name: Inject to System Tray (Windows 11)
  $description: Check to inject next to the clock/system tray instead of replacing the Widgets button
- win11FlyoutAnimation: true
  $name: Windows 11 Smooth Flyout Animation (Start Menu / Quick Access Style)
  $description: Enable smooth Windows 11 flyout entrance/exit animation (Windows 11 only)
- animateIcons: true
  $name: Weather Icon Animations & Hover Effects
  $description: Smooth subtle breathing & hover scaling on taskbar weather icon, hero forecast icon, and day cards
- hotkeyModifiers: "Win+Alt"
  $name: Hotkey Modifiers
  $description: Combination of Win, Alt, Ctrl, Shift separated by '+' (e.g. "Win+Alt", "Ctrl+Shift", or "None")
- hotkeyKey: "W"
  $name: Hotkey Key
  $description: A single letter or number (e.g. "W", "I", "F8")
- radarOpacity: 85
  $name: Forecast Base Map Opacity (0-100)
  $description: Opacity of the Esri base map only; precipitation has its own setting
- precipitationOpacity: 65
  $name: Precipitation Overlay Opacity (0-100)
  $description: Opacity of the forecast precipitation only; 100 is fully opaque. Does not affect the base map or location marker
*/
// ==/WindhawkModSettings==
#include <roapi.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <windhawk_utils.h>
#include <windows.h>
#include <dwmapi.h>
#include <commctrl.h>
#include <uxtheme.h>
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
#define DWMWA_WINDOW_CORNER_PREFERENCE 33
#endif
#include <winhttp.h>
#include <winstring.h>
#include <string>
#include <vector>
#include <d2d1.h>
#include <d2d1_1.h>
#include <d2d1_3.h>
#include <dwrite.h>
#undef GetCurrentTime
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Data.Json.h>
#include <winrt/Windows.UI.Xaml.Automation.Peers.h>
#include <winrt/Windows.UI.Xaml.Automation.Provider.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Devices.Geolocation.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Text.h>
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.Imaging.h>
#include <winrt/Windows.UI.Xaml.Input.h>
#include <winrt/Windows.UI.Input.h>
#include <winrt/Windows.UI.Xaml.Interop.h>
#include <winrt/Windows.UI.Xaml.Media.Animation.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.UI.Xaml.Shapes.h>
#include <winrt/Windows.UI.Xaml.Hosting.h>
#include <winrt/base.h>
#include <algorithm>
#include <functional>
#include <cmath>
#include <cstdlib>
#include <chrono>
#include <ctime>
#include <atomic>
#include <mutex>
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Media;
using namespace winrt::Windows::UI::Xaml::Media::Animation;
using namespace winrt::Windows::UI::Xaml::Shapes;
// Global settings
std::wstring g_location;
std::mutex g_locationMutex;
std::atomic<bool> g_useCelsius{false};
std::atomic<bool> g_showTempDecimals{true};
std::atomic<int> g_updateInterval{15};
std::atomic<int> g_weatherStyle{0};  // 0 = Emoji, 1 = Segoe MDL2, 2 = MSN Weather Native Taskbar SVGs, 3 = Fluent 3D, 4 = Custom Icon Pack
std::wstring g_customIconPackPath = L"";
bool g_showConditionName = true;
bool g_useAcrylic = true;
int g_acrylicOpacity = 65;
std::wstring g_bgImageVideoUrl = L"";
int g_bgImageVideoOpacity = 30;
int g_bgImageVideoStretch = 3;
std::atomic<int> g_forecastDaysFetch{14};
int g_textOffset = 10;
int g_itemsRepeaterOffset = 110;
COLORREF g_textColor = RGB(255, 255, 255);
int g_fontSize = 13;
int g_iconFontSize = 16;
bool g_debugLogs = false;
bool g_injectToSysTray = true;
bool g_win11FlyoutAnimation = true;
bool g_animateIcons = true;
bool g_isInitScan = true;
std::atomic<bool> g_taskbarViewDllLoaded = false;
std::atomic<bool> g_modUnloaded{false};
std::atomic<int> g_pendingRunAsync{0};
bool g_hkWin = false;
bool g_hkAlt = false;
bool g_hkCtrl = false;
bool g_hkShift = false;
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
UINT g_hkVk = 0;
HHOOK g_hKeyboardHook = NULL;
struct HotkeyHookState {
    bool winDown = false, altDown = false, ctrlDown = false, shiftDown = false;
    bool hotkeyHeld = false;
};
HotkeyHookState g_hotkeyHookState;
 
 
int g_currentAddedWeatherWidth = 0;
int g_cleanCx = 0;
int g_cleanCy = 0;
int g_cleanX = 0;
int g_cleanY = 0;
HHOOK g_hMouseHook = NULL;
// The foreground hook is created and removed by one dedicated message thread.
HANDLE g_hForegroundEventThread = nullptr;
HANDLE g_hForegroundEventStop = nullptr;
std::atomic<HWND> g_foregroundMessageTarget{nullptr};
constexpr UINT kForegroundChangedMessage = WM_APP + 0x4A7;
void HandleForegroundChange(HWND hwnd);
void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
void ForceTaskbarUpdateOriginal();
std::wstring g_line1FontFamily = L"Segoe UI";
int g_line1FontSize = 13;
bool g_line1Bold = true;
std::wstring g_line2FontFamily = L"Segoe UI";
int g_line2FontSize = 11;
bool g_line2Bold = false;
// Weather state Cached values
std::wstring g_cachedTemp = L"--°F";
std::wstring g_cachedCondition = L"Loading";
std::wstring g_cachedIcon = L"⏳";
std::wstring g_cachedIconUrl = L"";
int g_cachedWeatherCode = 0;
int g_cachedIsDay = 1;
std::wstring g_lastUpdatedTime = L"";
std::wstring g_cachedSunrise = L"";
std::wstring g_cachedSunset = L"";
struct WeatherAlert {
    std::wstring eventDesc;
    std::wstring headlineText;
    std::wstring areaName;
    std::wstring severity;
    std::wstring details;
    std::wstring customIcon = L"⚠️";
    std::wstring significance;
    std::wstring source;
    double expiryUtc = 0;
};
enum class AlertTone { Neutral, Information, Watch, Warning };
AlertTone GetAlertTone(const WeatherAlert& alert) {
    std::wstring severity = alert.severity;
    for (auto& ch : severity) {
        if (ch >= L'A' && ch <= L'Z') ch += L'a' - L'A';
    }
    // Environment Canada also supplies colour-qualified warning/watch codes
    // such as WR and AO; keep the issuing agency's classification intact.
    if ((!alert.significance.empty() && alert.significance.front() == L'W') ||
        alert.significance == L"E" || severity == L"extreme" || severity == L"severe")
        return AlertTone::Warning;
    if ((!alert.significance.empty() && alert.significance.front() == L'A') ||
        severity == L"moderate")
        return AlertTone::Watch;
    if (alert.significance == L"Y" || alert.significance == L"S" || severity == L"minor")
        return AlertTone::Information;
    return AlertTone::Neutral;
}
winrt::Windows::UI::Color GetAlertColour(AlertTone tone, BYTE alpha, bool text, bool dark) {
    using winrt::Windows::UI::ColorHelper;
    if (!text) {
        switch (tone) {
        case AlertTone::Warning: return ColorHelper::FromArgb(alpha, 239, 68, 68);
        case AlertTone::Watch: return ColorHelper::FromArgb(alpha, 245, 158, 11);
        case AlertTone::Information: return ColorHelper::FromArgb(alpha, 59, 130, 246);
        default: return ColorHelper::FromArgb(alpha, 128, 128, 128);
        }
    }
    switch (tone) {
    case AlertTone::Warning: return ColorHelper::FromArgb(alpha, dark ? 248 : 185, dark ? 113 : 28, dark ? 113 : 28);
    case AlertTone::Watch: return ColorHelper::FromArgb(alpha, dark ? 252 : 146, dark ? 211 : 64, dark ? 77 : 14);
    case AlertTone::Information: return ColorHelper::FromArgb(alpha, dark ? 147 : 29, dark ? 197 : 78, dark ? 253 : 216);
    default: return ColorHelper::FromArgb(alpha, dark ? 210 : 65, dark ? 210 : 65, dark ? 210 : 65);
    }
}
std::wstring g_activeWarning = L"";
std::atomic<bool> g_showWeatherAlerts{true};
std::vector<WeatherAlert> g_activeWarnings;
int g_currentWarningIndex = 0;
bool g_isAlertExpanded = false;
std::mutex g_alertMutex;
enum class AlertFetchStatus { Unknown, NoAlerts, Active, Unavailable };
AlertFetchStatus g_alertFetchStatus = AlertFetchStatus::Unknown;
struct AlertSnapshot {
    std::vector<WeatherAlert> warnings;
    int currentIndex = 0;
    bool expanded = false;
    std::wstring activeWarning;
    AlertFetchStatus fetchStatus = AlertFetchStatus::Unknown;
};
AlertSnapshot GetAlertSnapshot() {
    std::lock_guard<std::mutex> lock(g_alertMutex);
    return { g_activeWarnings, g_currentWarningIndex, g_isAlertExpanded,
             g_activeWarning, g_alertFetchStatus };
}
int g_selectedGraphTab = 0;  // 0=Temp, 1=Precip, 2=Wind, 3=Radar
constexpr double kBasePanelHeight = 350.0;
inline double CalculatePanelHeight() {
    double baseHeight = kBasePanelHeight;
    if (g_selectedGraphTab == 3) {
        baseHeight += 110.0;
    }
    auto alerts = GetAlertSnapshot();
    std::wstring displayWarning = g_showWeatherAlerts ? alerts.activeWarning : L"";
    if (!displayWarning.empty()) {
        baseHeight += 34.0;
        if (alerts.expanded) {
            baseHeight += 60.0;
        }
    }
    return baseHeight;
}
void FilterExpiredAlertsLocked() {
    const double nowUtc = static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
    auto it = g_activeWarnings.begin();
    while (it != g_activeWarnings.end()) {
        bool expired = it->expiryUtc > 0 && nowUtc >= it->expiryUtc;
        if (expired) it = g_activeWarnings.erase(it);
        else ++it;
    }
}
void UpdateActiveWarningTextLocked() {
    FilterExpiredAlertsLocked();
    if (g_activeWarnings.empty() || !g_showWeatherAlerts) {
        g_activeWarning = L"";
        return;
    }
    if (g_currentWarningIndex < 0 || g_currentWarningIndex >= (int)g_activeWarnings.size()) {
        g_currentWarningIndex = 0;
    }
    std::wstring text = g_activeWarnings[g_currentWarningIndex].eventDesc;
    if (g_activeWarnings.size() > 1) {
        text += L" (" + std::to_wstring(g_currentWarningIndex + 1) + L"/" + std::to_wstring(g_activeWarnings.size()) + L")";
    }
    g_activeWarning = text;
}
std::wstring g_displayCity = L"Detecting...";
double g_cachedLatitude = 0.0;
double g_cachedLongitude = 0.0;
bool g_winrtPreviouslySucceeded = false;
bool g_weatherAcquired = false;

// Current-condition/location values are produced by the weather worker and
// consumed by both the taskbar and XAML UI threads. Keep string/object state
// behind one mutex and publish/read it as a coherent snapshot.
std::mutex g_currentWeatherMutex;
struct CurrentWeatherSnapshot {
    std::wstring temp;
    std::wstring condition;
    std::wstring icon;
    std::wstring iconUrl;
    int weatherCode = 0;
    int isDay = 1;
    std::wstring lastUpdatedTime;
    std::wstring sunrise;
    std::wstring sunset;
    std::wstring displayCity;
    double latitude = 0.0;
    double longitude = 0.0;
    bool acquired = false;
};
CurrentWeatherSnapshot GetCurrentWeatherSnapshot() {
    std::lock_guard<std::mutex> lock(g_currentWeatherMutex);
    return {
        g_cachedTemp, g_cachedCondition, g_cachedIcon, g_cachedIconUrl,
        g_cachedWeatherCode, g_cachedIsDay, g_lastUpdatedTime,
        g_cachedSunrise, g_cachedSunset, g_displayCity,
        g_cachedLatitude, g_cachedLongitude, g_weatherAcquired
    };
}

// Background Thread values
HANDLE g_hQueryThread = NULL;
HANDLE g_hWatchdogThread = NULL;
std::atomic<bool> g_bThreadShouldTerm{false};
HANDLE g_hForceUpdateEvent = NULL;
bool g_isShellProcess = true;
// Forecast Definitions and Structures
struct DailyForecast {
    std::wstring dayName;
    std::wstring icon;
    std::wstring condition;
    std::wstring tempMax;
    std::wstring tempMin;
    std::wstring rawDate;
    std::wstring sunrise;
    std::wstring sunset;
    int weatherCode = 0;
    std::wstring iconUrl = L"";
};
struct ThreadXamlState {
    winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager manager = nullptr;
    winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource source = nullptr;
    HWND popupHwnd = nullptr;
    HWND sourceHwnd = nullptr;
};
[[clang::no_destroy]] ThreadXamlState g_globalXamlState;
ThreadXamlState& GetThreadXamlState() {
    return g_globalXamlState;
}
// The Win10 XAML objects belong exclusively to the popup STA. Other threads
// use HWND/visibility mirrors and communicate with that STA through messages.
std::atomic<HWND> g_win10PopupHwnd{nullptr};
std::atomic<HWND> g_win10SourceHwnd{nullptr};
std::atomic<bool> g_win10PopupVisible{false};
enum class Win10PopupInitState : int {
    NotStarted = 0,
    Starting,
    WaitingForXaml,
    Ready,
    Stopping,
    Failed
};
std::atomic<Win10PopupInitState> g_win10PopupInitState{Win10PopupInitState::NotStarted};
std::atomic<HWND> g_pendingWin10OpenClock{nullptr};
HANDLE g_hPopupRetryEvent = NULL;
std::mutex g_win10PopupThreadMutex;
HANDLE g_hPopupThread = NULL;
DWORD g_dwPopupThreadId = 0;
struct HourlyForecast {
    std::wstring timeString;
    int hour24;
    std::wstring icon;
    std::wstring conditionName;
    std::wstring temp;
    std::wstring precipProb;
    std::wstring rawDate;
    double tempRaw;
    double precipRaw;
    double windkphRaw;
    std::wstring humidity;
    std::wstring windSpeed;
    int weatherCode = 0;
    int isDay = 1;
    std::wstring iconUrl = L"";
};
std::vector<DailyForecast> g_forecastDaily;
std::vector<HourlyForecast> g_forecastHourly;
std::wstring g_cachedWindSpeed = L"--";
std::wstring g_cachedPrecipProb = L"0%";
std::wstring g_cachedHumidity = L"0%";
CRITICAL_SECTION g_forecastLock;
CRITICAL_SECTION g_subclassLock;
bool g_forecastAcquired = false;
ID2D1Factory* g_pD2DFactory = nullptr;
IDWriteFactory* g_pDWriteFactory = nullptr;
std::mutex g_win10TaskbarSvgMutex;
HBITMAP g_win10TaskbarSvgBitmap = nullptr;
std::wstring g_win10TaskbarSvgUrl;
extern HWND g_hSubclassedWnd;
void CleanupXamlMedia(winrt::Windows::UI::Xaml::UIElement const& element);

struct ImageHookRecord {
    void* abiIdentity = nullptr;
    winrt::weak_ref<winrt::Windows::UI::Xaml::Controls::Image> weakImage;
    winrt::event_token opened{};
    winrt::event_token failed{};
};
std::mutex g_imageHookMutex;
[[clang::no_destroy]] std::vector<ImageHookRecord> g_imageHooks;

void RevokeImageHooksForImage(
    winrt::Windows::UI::Xaml::Controls::Image const& image) {
    if (!image) return;
    void* identity = winrt::get_abi(image);
    std::vector<ImageHookRecord> toRevoke;
    {
        std::lock_guard<std::mutex> lock(g_imageHookMutex);
        auto it = g_imageHooks.begin();
        while (it != g_imageHooks.end()) {
            if (it->abiIdentity == identity) {
                toRevoke.push_back(*it);
                it = g_imageHooks.erase(it);
            } else {
                ++it;
            }
        }
    }
    for (auto const& hook : toRevoke) {
        try {
            if (auto img = hook.weakImage.get()) {
                img.ImageOpened(hook.opened);
                img.ImageFailed(hook.failed);
            }
        } catch (...) {}
    }
}

void RevokeAllImageHooks() {
    std::vector<ImageHookRecord> hooks;
    {
        std::lock_guard<std::mutex> lock(g_imageHookMutex);
        hooks.swap(g_imageHooks);
    }
    for (auto const& hook : hooks) {
        try {
            if (auto img = hook.weakImage.get()) {
                img.ImageOpened(hook.opened);
                img.ImageFailed(hook.failed);
            }
        } catch (...) {}
    }
}

void InvalidateClockParentRegion(HWND hWnd);
void ResizeWin10PopupToCurrentContent();
bool IsWindows11();
void AlignOverlayWindow();
void RestoreDefaultWindowSize(HWND hwndToClean);
int GetRequiredWeatherWidth(HWND hWnd);
HWND FindXamlIslandWindow(HWND hParent);
std::pair<std::wstring, std::wstring> GetCodeMapping(int code);
// Injected Grid reference
std::vector<winrt::weak_ref<Grid>> g_injectedWeatherGrids;
std::mutex g_weatherGridMutex;
#include <set>
struct HookRecord {
    winrt::weak_ref<winrt::Windows::UI::Xaml::FrameworkElement> weakElement;
    winrt::event_token token;
};
extern std::vector<HookRecord> g_sizeChangedHooks;
extern std::mutex g_hooksMutex;
extern std::set<void*> g_hookedElements;
extern std::mutex g_hookedMutex;
std::vector<HookRecord> g_sizeChangedHooks;
std::mutex g_hooksMutex;
std::set<void*> g_hookedElements;
std::mutex g_hookedMutex;
void QueueWeatherUpdateOnUIThread();
std::wstring RequestHttpData(const std::wstring& host, const std::wstring& path, bool secure, DWORD* httpStatusCode = nullptr, DWORD* winHttpError = nullptr, bool* responseComplete = nullptr);
std::wstring ExtractJSONValue(const std::wstring& json, const std::wstring& key);
struct ClockSearchData {
    HWND hClock;
};
static BOOL CALLBACK FindClockEnumChildProc(HWND hwnd, LPARAM lParam) {
    WCHAR className[256];
    GetClassNameW(hwnd, className, 256);
    if (wcscmp(className, L"TrayClockWClass") == 0) {
        ((ClockSearchData*)lParam)->hClock = hwnd;
        return FALSE;
    }
    return TRUE;
}
// Find Taskbar clock window
HWND FindSystemClockWnd() {
    HWND hShell = FindWindowW(L"Shell_TrayWnd", NULL);
    if (!hShell)
        return NULL;
    HWND hTray = FindWindowExW(hShell, NULL, L"TrayNotifyWnd", NULL);
    if (hTray) {
        HWND hClock = FindWindowExW(hTray, NULL, L"TrayClockWClass", NULL);
        if (hClock)
            return hClock;
    }
    ClockSearchData data = { NULL };
    EnumChildWindows(hShell, FindClockEnumChildProc, (LPARAM)&data);
    return data.hClock;
}
HWND FindTrayNotifyWnd() {
    HWND hShell = FindWindowW(L"Shell_TrayWnd", NULL);
    if (!hShell)
        return NULL;
    return FindWindowExW(hShell, NULL, L"TrayNotifyWnd", NULL);
}
struct AnchorSearchData {
    HWND hAnchor;
};
static BOOL CALLBACK FindAnchorEnumChildProc(HWND hwnd, LPARAM lParam) {
    WCHAR className[256];
    GetClassNameW(hwnd, className, 256);
    if (wcscmp(className, L"TrayNotifyWnd") == 0) {
        ((AnchorSearchData*)lParam)->hAnchor = hwnd;
        return FALSE; // Stop searching immediately because we found the tray!
    }
    if (wcscmp(className, L"TrayClockWClass") == 0) {
        if (!((AnchorSearchData*)lParam)->hAnchor) {
            ((AnchorSearchData*)lParam)->hAnchor = hwnd; // Fallback to clock if no tray found yet
        }
    }
    return TRUE;
}
// Find the System Tray area or fall back to system clock window as alignment
// anchor
HWND FindSystemAnchorWnd() {
    HWND hShell = FindWindowW(L"Shell_TrayWnd", NULL);
    if (!hShell)
        return NULL;
    HWND hTray = FindWindowExW(hShell, NULL, L"TrayNotifyWnd", NULL);
    if (hTray && IsWindowVisible(hTray)) {
        return hTray;
    }
    // Fallback search
    AnchorSearchData data = { NULL };
    EnumChildWindows(hShell, FindAnchorEnumChildProc, (LPARAM)&data);
    return data.hAnchor ? data.hAnchor : hShell;
}
// Update margin of injected XAML Grid dynamically based on real-time taskbar
// layout state
bool ShouldUseXamlTaskbar();
struct ReservedRepeaterMargin {
    winrt::weak_ref<FrameworkElement> element;
    Thickness original;
    DWORD uiThread;
};
std::mutex g_reservedRepeaterMarginsMutex;
std::vector<ReservedRepeaterMargin> g_reservedRepeaterMargins;

// Each XAML element is read and changed only on the thread that owns it.
void ReserveRepeaterMargin(FrameworkElement element, double right, double bottom) {
    Thickness original{};
    const DWORD currentThread = GetCurrentThreadId();
    {
        std::lock_guard<std::mutex> lock(g_reservedRepeaterMarginsMutex);
        auto it = g_reservedRepeaterMargins.begin();
        for (; it != g_reservedRepeaterMargins.end(); ++it) {
            if (it->uiThread == currentThread && it->element.get() == element) break;
        }
        if (it == g_reservedRepeaterMargins.end()) {
            original = element.Margin();
            g_reservedRepeaterMargins.push_back({winrt::make_weak(element), original, currentThread});
        } else {
            original = it->original;
        }
    }
    Thickness wanted{original.Left, original.Top,
                     original.Right + right, original.Bottom + bottom};
    auto current = element.Margin();
    if (current.Left != wanted.Left || current.Top != wanted.Top ||
        current.Right != wanted.Right || current.Bottom != wanted.Bottom) {
        element.Margin(wanted);
    }
}

void RestoreRepeaterMarginsOnCurrentUIThread() {
    std::vector<ReservedRepeaterMargin> owned;
    {
        std::lock_guard<std::mutex> lock(g_reservedRepeaterMarginsMutex);
        auto it = g_reservedRepeaterMargins.begin();
        while (it != g_reservedRepeaterMargins.end()) {
            if (it->uiThread == GetCurrentThreadId()) {
                owned.push_back(*it);
                it = g_reservedRepeaterMargins.erase(it);
            } else {
                ++it;
            }
        }
    }
    for (auto const& entry : owned) {
        try { if (auto element = entry.element.get()) element.Margin(entry.original); }
        catch (...) {}
    }
}

void UpdateInjectedWeatherLayout(Grid weatherGrid) {
    if (!weatherGrid)
        return;
    
    auto safeSetMargin = [](FrameworkElement el, Thickness margin) {
        auto current = el.Margin();
        if (current.Left != margin.Left || current.Top != margin.Top || current.Right != margin.Right || current.Bottom != margin.Bottom) {
            el.Margin(margin);
        }
    };
    
    try {
        if (!g_injectToSysTray) RestoreRepeaterMarginsOnCurrentUIThread();
        HWND hAnchor = FindSystemAnchorWnd();
        HWND hParentTaskbar = hAnchor ? GetAncestor(hAnchor, GA_ROOT) : NULL;
        if (!hParentTaskbar)
            hParentTaskbar = FindWindowW(L"Shell_TrayWnd", NULL);
        if (hParentTaskbar) {
            RECT trayRect;
            GetWindowRect(hParentTaskbar, &trayRect);
            bool isHorizontal = (trayRect.right - trayRect.left) >
                (trayRect.bottom - trayRect.top);

            if (!g_injectToSysTray) {
                if (!isHorizontal) {
                    // Give our host a real measure width even in a native panel
                    // whose sizing is driven by its now-collapsed original children.
                    weatherGrid.Width(40.0);
                    weatherGrid.MinWidth(40.0);
                    weatherGrid.Height(40.0);
                    weatherGrid.MinHeight(40.0);
                    weatherGrid.HorizontalAlignment(HorizontalAlignment::Left);
                    weatherGrid.VerticalAlignment(VerticalAlignment::Center);
                    safeSetMargin(weatherGrid, Thickness{ 0, 0, 0, 0 });
                    return;
                }
                // Clear only properties belonging to our injected grid on rotation.
                weatherGrid.ClearValue(FrameworkElement::WidthProperty());
                weatherGrid.ClearValue(FrameworkElement::MinWidthProperty());
            }
                
            HWND hDpiWnd = FindXamlIslandWindow(hParentTaskbar);
            if (!hDpiWnd) hDpiWnd = hParentTaskbar;
            UINT dpi = GetDpiForWindow(hDpiWnd);
            if (dpi == 0) dpi = 96;
            double scale = (double)dpi / 96.0;

            if (isHorizontal) {
                if (g_injectToSysTray && hAnchor) {
                    RECT anchorRect;
                    GetWindowRect(hAnchor, &anchorRect);
                    double marginFromRight = (trayRect.right - anchorRect.left) / scale - g_textOffset;
                    if (marginFromRight < 0.0) marginFromRight = 0.0;
                    try {
                        if (std::isnan(weatherGrid.Height()) || weatherGrid.Height() != 40.0) weatherGrid.Height(40.0);
                        if (std::isnan(weatherGrid.MinHeight()) || weatherGrid.MinHeight() != 40.0) weatherGrid.MinHeight(40.0);
                    } catch (...) {}
                    weatherGrid.HorizontalAlignment(HorizontalAlignment::Right);
                    weatherGrid.VerticalAlignment(VerticalAlignment::Center);
                    safeSetMargin(weatherGrid, Thickness{ 0, 0, marginFromRight, 0 });
                    double reservedButtonWidth = (double)g_itemsRepeaterOffset;
                    try {
                        auto children = weatherGrid.Children();
                        if (children.Size() != 0) {
                            if (auto button = children.GetAt(0).try_as<FrameworkElement>()) {
                                reservedButtonWidth = (std::max)(reservedButtonWidth,
                                    button.ActualWidth() + 8.0);
                            }
                        }
                    } catch (...) {}
                    // Reserve space next to the sibling TaskbarFrameRepeater/ItemsRepeater
                    try {
                        if (auto parentGrid = weatherGrid.Parent().try_as<Grid>()) {
                            auto siblings = parentGrid.Children();
                            uint32_t count = 0;
                            try { count = siblings.Size(); } catch (...) {}
                            for (uint32_t i = 0; i < count; i++) {
                                try {
                                    if (auto sibling = siblings.GetAt(i).try_as<FrameworkElement>()) {
                                        std::wstring sibClass(winrt::get_class_name(sibling).c_str());
                                        if (sibClass.find(L"Repeater") != std::wstring::npos || sibClass.find(L"TaskbarFrameRepeater") != std::wstring::npos) {
                                            // The grid's own right margin already positions it before the tray.
                                            // Only its occupied width needs to be reserved in the native repeater.
                                            ReserveRepeaterMargin(sibling, reservedButtonWidth, 0);
                                        }
                                    }
                                }
                                catch (...) {}
                            }
                        }
                    }
                    catch (...) {}
                }
                else {
                    try {
                        if (!std::isnan(weatherGrid.Height())) weatherGrid.Height(std::numeric_limits<double>::quiet_NaN());
                        if (!std::isnan(weatherGrid.MinHeight())) weatherGrid.MinHeight(0.0);
                        try { weatherGrid.MaxHeight(std::numeric_limits<double>::infinity()); } catch (...) {}
                    } catch (...) {}
                    weatherGrid.HorizontalAlignment(HorizontalAlignment::Stretch);
                    weatherGrid.VerticalAlignment(VerticalAlignment::Stretch);
                    safeSetMargin(weatherGrid, Thickness{ 0, 0, 0, 0 });
                }
            }
            else {
                try {
                    if (!std::isnan(weatherGrid.Height())) weatherGrid.Height(std::numeric_limits<double>::quiet_NaN());
                    if (!std::isnan(weatherGrid.MinHeight())) weatherGrid.MinHeight(0.0);
                    try { weatherGrid.MaxHeight(std::numeric_limits<double>::infinity()); } catch (...) {}
                } catch (...) {}
                weatherGrid.HorizontalAlignment(HorizontalAlignment::Stretch);
                weatherGrid.VerticalAlignment(g_injectToSysTray ? VerticalAlignment::Bottom : VerticalAlignment::Center);
                if (g_injectToSysTray && hAnchor) {
                    RECT anchorRect;
                    GetWindowRect(hAnchor, &anchorRect);
                    double bottomMargin = (trayRect.bottom > anchorRect.top) ? ((trayRect.bottom - anchorRect.top) / scale - g_textOffset) : 0.0;
                    if (bottomMargin < 0.0) bottomMargin = 0.0;
                    safeSetMargin(weatherGrid, Thickness{ 0, 0, 0, bottomMargin });
                    // Reserve roughly 64px for the expanded vertical button
                    try {
                        if (auto parentGrid = weatherGrid.Parent().try_as<Grid>()) {
                            auto siblings = parentGrid.Children();
                            uint32_t count = 0;
                            try { count = siblings.Size(); } catch (...) {}
                            for (uint32_t i = 0; i < count; i++) {
                                try {
                                    if (auto sibling = siblings.GetAt(i).try_as<FrameworkElement>()) {
                                        std::wstring sibClass(winrt::get_class_name(sibling).c_str());
                                        if (sibClass.find(L"Repeater") != std::wstring::npos || sibClass.find(L"TaskbarFrameRepeater") != std::wstring::npos) {
                                            double extraSpace = isHorizontal ? 0.0 : 64.0;
                                            ReserveRepeaterMargin(sibling, 0, bottomMargin + extraSpace + ((double)g_itemsRepeaterOffset / scale));
                                        }
                                    }
                                }
                                catch (...) {}
                            }
                        }
                    }
                    catch (...) {}
                }
                else {
                    safeSetMargin(weatherGrid, Thickness{ 0, 0, 0, 0 });
                }
            }
        }
        else {
            weatherGrid.HorizontalAlignment(HorizontalAlignment::Right);
            weatherGrid.VerticalAlignment(VerticalAlignment::Center);
            safeSetMargin(weatherGrid, Thickness{ 0, 0, 100, 0 });
        }
    }
    catch (...) {
    }
}
// State for interactive flyout
winrt::weak_ref<winrt::Windows::UI::Xaml::FrameworkElement> g_weakXamlWeatherButton;
[[clang::no_destroy]] std::function<void()> g_showWin11Flyout = nullptr;
bool g_win11FlyoutIsOpen = false;
bool g_flyoutWasOpenOnPointerPressed = false;
[[clang::no_destroy]] winrt::Windows::UI::Xaml::Controls::Flyout g_activeFlyout = nullptr;
HWND g_hPreFlyoutActiveWnd = NULL;
ULONGLONG g_lastClosedTickCount = 0;
std::wstring g_selectedDate = L"";
std::wstring g_selectedHour = L"";
double g_graphScrollOffset = -1.0;
double g_dailyScrollOffset = 0.0;
struct RadarFrame {
    int64_t time = 0;
    std::wstring isoTime;
    std::wstring referenceTime;
};
std::vector<RadarFrame> g_radarFrames;
std::atomic<bool> g_radarMetadataError{false};
int g_radarZoom = 6;
std::atomic<int> g_radarCurrentFrame{-1};
bool g_radarIsPlaying = false;
int g_radarPlaySpeedIdx = 1;
int g_radarOpacity = 85;
int g_precipitationOpacity = 65;
int RadarPlaybackIntervalMs() {
    switch (g_radarPlaySpeedIdx) {
        case 0: return 1800; // 0.5x
        case 2: return 650;  // 1.5x
        case 3: return 500;  // 2x
        default: return 950; // 1x
    }
}
[[clang::no_destroy]] std::map<std::wstring, winrt::Windows::UI::Xaml::Media::Imaging::BitmapImage> g_radarTileBitmapCache;
void FetchRadarMetadata();
bool IsSystemDarkMode() {
    DWORD data = 1;
    DWORD dataSize = sizeof(data);
    LSTATUS status = RegGetValueW(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        L"SystemUsesLightTheme", RRF_RT_REG_DWORD, nullptr, &data, &dataSize);
    if (status == ERROR_SUCCESS) {
        return data == 0;
    }
    return true;  // Default to dark
}
static bool g_btnIsHovered = false;
static bool g_btnIsPressed = false;
void UpdateWeatherButtonVisuals(
    winrt::Windows::UI::Xaml::Controls::Button btn,
    bool isHovered,
    bool isPressed,
    bool isFlyoutOpen) {
    if (!btn) return;
    
    bool isDark = IsSystemDarkMode();
    winrt::Windows::UI::Color bg;
    winrt::Windows::UI::Color border;
    
    if (isDark) {
        if (isFlyoutOpen) {
            bg = winrt::Windows::UI::ColorHelper::FromArgb(35, 255, 255, 255);
            border = winrt::Windows::UI::ColorHelper::FromArgb(25, 255, 255, 255);
        } else if (isPressed) {
            bg = winrt::Windows::UI::ColorHelper::FromArgb(20, 255, 255, 255);
            border = winrt::Windows::UI::ColorHelper::FromArgb(15, 255, 255, 255);
        } else if (isHovered) {
            bg = winrt::Windows::UI::ColorHelper::FromArgb(30, 255, 255, 255);
            border = winrt::Windows::UI::ColorHelper::FromArgb(20, 255, 255, 255);
        } else {
            bg = winrt::Windows::UI::ColorHelper::FromArgb(0, 0, 0, 0);
            border = winrt::Windows::UI::ColorHelper::FromArgb(0, 0, 0, 0);
        }
    } else {
        if (isFlyoutOpen) {
            bg = winrt::Windows::UI::ColorHelper::FromArgb(30, 0, 0, 0);
            border = winrt::Windows::UI::ColorHelper::FromArgb(20, 0, 0, 0);
        } else if (isPressed) {
            bg = winrt::Windows::UI::ColorHelper::FromArgb(15, 0, 0, 0);
            border = winrt::Windows::UI::ColorHelper::FromArgb(10, 0, 0, 0);
        } else if (isHovered) {
            bg = winrt::Windows::UI::ColorHelper::FromArgb(22, 0, 0, 0);
            border = winrt::Windows::UI::ColorHelper::FromArgb(15, 0, 0, 0);
        } else {
            bg = winrt::Windows::UI::ColorHelper::FromArgb(0, 0, 0, 0);
            border = winrt::Windows::UI::ColorHelper::FromArgb(0, 0, 0, 0);
        }
    }
    try {
        btn.Background(winrt::Windows::UI::Xaml::Media::SolidColorBrush{ bg });
        btn.BorderBrush(winrt::Windows::UI::Xaml::Media::SolidColorBrush{ border });
        btn.BorderThickness(winrt::Windows::UI::Xaml::Thickness{ 1.0, 1.0, 1.0, 1.0 });
        try {
            btn.CornerRadius(winrt::Windows::UI::Xaml::CornerRadius{ 4.0, 4.0, 4.0, 4.0 });
        } catch (...) {}
        // Make inner template child background transparent to prevent double-layered native backgrounds
        if (int count = winrt::Windows::UI::Xaml::Media::VisualTreeHelper::GetChildrenCount(btn)) {
            if (count > 0) {
                auto child = winrt::Windows::UI::Xaml::Media::VisualTreeHelper::GetChild(btn, 0);
                if (auto innerBorder = child.try_as<winrt::Windows::UI::Xaml::Controls::Border>()) {
                    innerBorder.Background(winrt::Windows::UI::Xaml::Media::SolidColorBrush{ winrt::Windows::UI::Colors::Transparent() });
                    innerBorder.BorderBrush(winrt::Windows::UI::Xaml::Media::SolidColorBrush{ winrt::Windows::UI::Colors::Transparent() });
                    innerBorder.BorderThickness(winrt::Windows::UI::Xaml::Thickness{ 0 });
                    innerBorder.CornerRadius(winrt::Windows::UI::Xaml::CornerRadius{ 4.0, 4.0, 4.0, 4.0 });
                } else if (auto innerGrid = child.try_as<winrt::Windows::UI::Xaml::Controls::Grid>()) {
                    innerGrid.Background(winrt::Windows::UI::Xaml::Media::SolidColorBrush{ winrt::Windows::UI::Colors::Transparent() });
                    innerGrid.CornerRadius(winrt::Windows::UI::Xaml::CornerRadius{ 4.0, 4.0, 4.0, 4.0 });
                }
            }
        }
    } catch (...) {}
}
void UpdateWeatherButtonActiveState(bool isOpen) {
    if (auto fe = g_weakXamlWeatherButton.get()) {
        if (auto btn = fe.try_as<winrt::Windows::UI::Xaml::Controls::Button>()) {
            UpdateWeatherButtonVisuals(btn, g_btnIsHovered, g_btnIsPressed, isOpen);
        }
    }
}
// One-shot animation delegates are owned by the UI thread that registered them.
// Keep revocation handles until completion or explicit flyout teardown. In
// particular, stopping a DispatcherTimer alone does not release its delegate.
struct TrackedAnimationEvent {
    unsigned long long id;
    DWORD ownerThread;
    std::function<void(bool)> revoke;
};
std::mutex g_animationEventsMutex;
std::vector<TrackedAnimationEvent> g_animationEvents;
std::atomic<unsigned long long> g_nextAnimationEventId{0};

bool ReleaseAnimationEvent(unsigned long long id, bool cancel) {
    std::function<void(bool)> revoke;
    {
        std::lock_guard<std::mutex> lock(g_animationEventsMutex);
        auto it = std::find_if(g_animationEvents.begin(), g_animationEvents.end(),
            [id](auto const& entry) {
                return entry.id == id && entry.ownerThread == GetCurrentThreadId();
            });
        if (it == g_animationEvents.end()) return false;
        revoke = std::move(it->revoke);
        g_animationEvents.erase(it);
    }
    // No registry lock across XAML calls: Stop/removal can reenter UI code.
    revoke(cancel);
    return true;
}

void CancelAnimationEventsOnCurrentThread() {
    unsigned int canceled = 0;
    for (;;) {
        unsigned long long id = 0;
        {
            std::lock_guard<std::mutex> lock(g_animationEventsMutex);
            auto it = std::find_if(g_animationEvents.begin(), g_animationEvents.end(),
                [](auto const& entry) { return entry.ownerThread == GetCurrentThreadId(); });
            if (it == g_animationEvents.end()) break;
            id = it->id;
        }
        if (ReleaseAnimationEvent(id, true)) ++canceled;
    }
    if (g_debugLogs && canceled)
        Wh_Log(L"[WeatherLifetime] Revoked %u animation/timer events on owner thread %lu",
               canceled, GetCurrentThreadId());
}

template<typename Handler>
void TrackStoryboardCompleted(Storyboard const& storyboard, Handler handler) {
    if (g_modUnloaded) return;
    auto id = ++g_nextAnimationEventId;
    auto token = storyboard.Completed([id, handler = std::move(handler)]
        (auto const& sender, auto const& args) mutable {
            // Removing the event can release its delegate; keep the user action
            // on this stack before revoking the event's ownership of it.
            auto action = handler;
            if (!ReleaseAnimationEvent(id, false) || g_modUnloaded) return;
            action(sender, args);
        });
    try {
        std::lock_guard<std::mutex> lock(g_animationEventsMutex);
        g_animationEvents.push_back({id, GetCurrentThreadId(),
            [storyboard, token](bool cancel) {
                try { storyboard.Completed(token); }
                catch (...) { Wh_Log(L"[WeatherLifetime] Storyboard event revocation failed"); }
                if (cancel) {
                    try { storyboard.Stop(); } catch (...) {}
                }
            }});
    } catch (...) {
        // Do not leave an untracked delegate if registry allocation fails.
        storyboard.Completed(token);
        throw;
    }
}

template<typename Handler>
void TrackOneShotTimer(winrt::Windows::UI::Xaml::DispatcherTimer const& timer, Handler handler) {
    if (g_modUnloaded) return;
    auto id = ++g_nextAnimationEventId;
    auto token = timer.Tick([id, handler = std::move(handler)]
        (auto const& sender, auto const& args) mutable {
            // Removing the event can release its delegate; keep the user action
            // on this stack before revoking the event's ownership of it.
            auto action = handler;
            if (!ReleaseAnimationEvent(id, false) || g_modUnloaded) return;
            action(sender, args);
        });
    try {
        std::lock_guard<std::mutex> lock(g_animationEventsMutex);
        g_animationEvents.push_back({id, GetCurrentThreadId(),
            [timer, token](bool) {
                try { timer.Tick(token); }
                catch (...) { Wh_Log(L"[WeatherLifetime] Timer event revocation failed"); }
                try { timer.Stop(); } catch (...) {}
            }});
    } catch (...) {
        timer.Tick(token);
        throw;
    }
}

[[clang::no_destroy]] static winrt::Windows::UI::Xaml::DispatcherTimer s_radarFrameTimer{ nullptr };
static winrt::event_token s_radarFrameTickToken{};
static std::mutex s_activeRadarMapsMutex;
[[clang::no_destroy]] static std::map<void*,
    winrt::weak_ref<winrt::Windows::UI::Xaml::Controls::Canvas>> s_activeRadarMaps;
void StopRadarTimer() {
    g_radarIsPlaying = false;
    if (s_radarFrameTimer) {
        try {
            s_radarFrameTimer.Stop();
        } catch (...) {}
        try { s_radarFrameTimer.Tick(s_radarFrameTickToken); } catch (...) {}
        s_radarFrameTickToken = {};
        s_radarFrameTimer = nullptr;
    }
}
std::wstring GetWeatherIconPathOrUrl(int code, bool is_day);
void PopulateForecastUI(winrt::Windows::UI::Xaml::Controls::Grid rootGrid,
    std::wstring condition,
    std::wstring currentIcon,
    std::wstring currentTemp,
    bool animate = true,
    bool animateGraph = true,
    std::wstring customPrecip = L"",
    std::wstring customHumidity = L"",
    std::wstring customWind = L"",
    std::wstring customIconUrl = L"",
    int zoomDelta = 0);
winrt::Windows::UI::Xaml::Controls::Flyout CreateForecastFlyout(
    std::wstring condition,
    std::wstring currentIcon,
    std::wstring currentTemp) {
    using namespace winrt::Windows::UI::Xaml;
    using namespace winrt::Windows::UI::Xaml::Controls;
    using namespace winrt::Windows::UI::Xaml::Media;
    Flyout flyout;
    Grid rootGrid;
    // Clear selection state on new open
    g_selectedDate = L"";
    g_selectedHour = L"";
    g_selectedGraphTab = 0;
    g_graphScrollOffset = -1.0;
    g_dailyScrollOffset = 0.0;
    // Radar metadata is owned by the main weather worker. Never create detached
    // threads in explorer.exe: wake the tracked worker for an immediate retry.
    bool hasRadarFrames = false;
    EnterCriticalSection(&g_forecastLock);
    hasRadarFrames = !g_radarFrames.empty();
    LeaveCriticalSection(&g_forecastLock);
    if (!hasRadarFrames && !g_radarMetadataError && g_hForceUpdateEvent) {
        SetEvent(g_hForceUpdateEvent);
    }
    PopulateForecastUI(rootGrid, condition, currentIcon, currentTemp);
    bool isBottom = true;
    bool isTop = false;
    bool isLeft = false;
    bool isRight = false;
    HWND hAnchor = FindSystemAnchorWnd();
    if (hAnchor) {
        HWND hParentTaskbar = GetAncestor(hAnchor, GA_ROOT);
        if (!hParentTaskbar) hParentTaskbar = FindWindowW(L"Shell_TrayWnd", NULL);
        if (hParentTaskbar) {
            RECT trayRect;
            GetWindowRect(hParentTaskbar, &trayRect);
            HMONITOR hMon = MonitorFromRect(&trayRect, MONITOR_DEFAULTTONEAREST);
            MONITORINFO mi = { sizeof(mi) };
            if (GetMonitorInfoW(hMon, &mi)) {
                int trayWidth = trayRect.right - trayRect.left;
                int trayHeight = trayRect.bottom - trayRect.top;
                bool isHorizontal = trayWidth > trayHeight;
                if (isHorizontal) {
                    if (trayRect.top <= mi.rcMonitor.top + (mi.rcMonitor.bottom - mi.rcMonitor.top) / 2) {
                        isTop = true;
                        isBottom = false;
                    } else {
                        isBottom = true;
                    }
                } else {
                    isBottom = false;
                    if (trayRect.left <= mi.rcMonitor.left + (mi.rcMonitor.right - mi.rcMonitor.left) / 2) {
                        isLeft = true;
                    } else {
                        isRight = true;
                    }
                }
            }
        }
    }
    // Remove FlyoutPresenter Transform so the popup window touches the taskbar.
    // We will use an outer grid to contain the margins and clip the animation.
    
    // Make the flyout background the actual acrylic backdrop, so it remains static and doesn't fall back during content slide animation.
    winrt::Windows::UI::Xaml::Style flyoutStyle(
        winrt::xaml_typename<
        winrt::Windows::UI::Xaml::Controls::FlyoutPresenter>());
    winrt::Windows::UI::Color solidBg = IsSystemDarkMode() ?
        winrt::Windows::UI::ColorHelper::FromArgb(255, 30, 30, 30) :
        winrt::Windows::UI::ColorHelper::FromArgb(255, 240, 240, 240);
    winrt::Windows::UI::Xaml::Setter bgSetter(
        winrt::Windows::UI::Xaml::Controls::Control::BackgroundProperty(),
        winrt::box_value(winrt::Windows::UI::Xaml::Media::SolidColorBrush{ winrt::Windows::UI::Colors::Transparent() }));
    winrt::Windows::UI::Xaml::Setter marginSetter(
        winrt::Windows::UI::Xaml::FrameworkElement::MarginProperty(),
        winrt::box_value(Thickness{ 0, 0, 0, 0 }));
    winrt::Windows::UI::Xaml::Setter padSetter(
        winrt::Windows::UI::Xaml::Controls::Control::PaddingProperty(),
        winrt::box_value(Thickness{ 0, 0, 0, 0 }));
    winrt::Windows::UI::Xaml::Setter borSetter(
        winrt::Windows::UI::Xaml::Controls::Control::BorderThicknessProperty(),
        winrt::box_value(Thickness{ 0, 0, 0, 0 }));
    
    winrt::Windows::UI::Color borderColor;
    borderColor.A = 64; 
    borderColor.R = 128;
    borderColor.G = 128;
    borderColor.B = 128;
    winrt::Windows::UI::Xaml::Setter borBrushSetter(
        winrt::Windows::UI::Xaml::Controls::Control::BorderBrushProperty(),
        winrt::box_value(winrt::Windows::UI::Xaml::Media::SolidColorBrush{ winrt::Windows::UI::Colors::Transparent() }));
        
    winrt::Windows::UI::Xaml::Setter cornerSetter(
        winrt::Windows::UI::Xaml::Controls::Control::CornerRadiusProperty(),
        winrt::box_value(IsWindows11() ? winrt::Windows::UI::Xaml::CornerRadius{ 8.0, 8.0, 8.0, 8.0 } : winrt::Windows::UI::Xaml::CornerRadius{ 0, 0, 0, 0 }));
    // Override MaxWidth to allow wider flyout panels
    winrt::Windows::UI::Xaml::Setter maxWidthSetter(
        winrt::Windows::UI::Xaml::FrameworkElement::MaxWidthProperty(),
        winrt::box_value((double)9999.0));
    winrt::Windows::UI::Xaml::Setter shadowSetter(
        winrt::Windows::UI::Xaml::Controls::FlyoutPresenter::IsDefaultShadowEnabledProperty(),
        winrt::box_value(false));
    winrt::Windows::UI::Xaml::Setter transitionsSetter(
        winrt::Windows::UI::Xaml::UIElement::TransitionsProperty(),
        winrt::Windows::UI::Xaml::Media::Animation::TransitionCollection{});
    winrt::Windows::UI::Xaml::Setter isTabStopSetter(
        winrt::Windows::UI::Xaml::Controls::Control::IsTabStopProperty(),
        winrt::box_value(false));
    winrt::Windows::UI::Xaml::Setter allowFocusSetter(
        winrt::Windows::UI::Xaml::FrameworkElement::AllowFocusOnInteractionProperty(),
        winrt::box_value(false));
    flyoutStyle.Setters().Append(bgSetter);
    flyoutStyle.Setters().Append(marginSetter);
    flyoutStyle.Setters().Append(maxWidthSetter);
    flyoutStyle.Setters().Append(padSetter);
    flyoutStyle.Setters().Append(borSetter);
    flyoutStyle.Setters().Append(borBrushSetter);
    flyoutStyle.Setters().Append(cornerSetter);
    flyoutStyle.Setters().Append(transitionsSetter);
    flyoutStyle.Setters().Append(shadowSetter);
    flyoutStyle.Setters().Append(isTabStopSetter);
    flyoutStyle.Setters().Append(allowFocusSetter);
    rootGrid.Margin(winrt::Windows::UI::Xaml::Thickness{ 0.0, 0.0, 0.0, 0.0 });
    winrt::Windows::UI::Xaml::Controls::Grid outerGrid;
    outerGrid.Margin(winrt::Windows::UI::Xaml::Thickness{ 0, 0, 0, 0 });
    winrt::Windows::UI::Xaml::Media::Brush flyoutBgBrush = nullptr;
    if (g_useAcrylic) {
        if (IsWindows11()) {
            try {
                AcrylicBrush acrylic;
                if (!g_injectToSysTray) {
                    acrylic.BackgroundSource(AcrylicBackgroundSource::Backdrop);
                } else {
                    acrylic.BackgroundSource(AcrylicBackgroundSource::HostBackdrop);
                }
                acrylic.TintColor(solidBg);
                acrylic.TintOpacity(g_acrylicOpacity / 100.0);
                acrylic.FallbackColor(solidBg);
                flyoutBgBrush = acrylic;
            } catch (...) {
                flyoutBgBrush = SolidColorBrush{ solidBg };
            }
        } else {
            // Windows 10: use the XAML Island HostBackdrop acrylic directly.
            // The older working implementation used this path; layered/no-redirection
            // HWND styles interfere with the backdrop and can make it render solid black.
            try {
                AcrylicBrush acrylic;
                acrylic.BackgroundSource(AcrylicBackgroundSource::HostBackdrop);
                acrylic.TintColor(solidBg);
                acrylic.TintOpacity(g_acrylicOpacity / 100.0);
                acrylic.FallbackColor(solidBg);
                flyoutBgBrush = acrylic;
            } catch (...) {
                flyoutBgBrush = SolidColorBrush{ solidBg };
            }
        }
    } else {
        flyoutBgBrush = SolidColorBrush{ solidBg };
    }
    if (flyoutBgBrush) {
        outerGrid.Background(flyoutBgBrush);
    } else {
        outerGrid.Background(SolidColorBrush{ solidBg });
    }
    outerGrid.BorderThickness(winrt::Windows::UI::Xaml::Thickness{ 1, 1, 1, 1 });
    outerGrid.BorderBrush(SolidColorBrush{ borderColor });
    if (IsWindows11()) {
        outerGrid.CornerRadius(winrt::Windows::UI::Xaml::CornerRadius{ 8.0, 8.0, 8.0, 8.0 });
    } else {
        outerGrid.CornerRadius(winrt::Windows::UI::Xaml::CornerRadius{ 0, 0, 0, 0 });
    }
    outerGrid.Children().Append(rootGrid);
    try {
        outerGrid.Transitions(nullptr);
    } catch (...) {}
    winrt::Windows::UI::Xaml::Media::CompositeTransform outerTransform;
    outerGrid.RenderTransform(outerTransform);
    flyout.FlyoutPresenterStyle(flyoutStyle);
    try {
        flyout.AllowFocusOnInteraction(false);
    } catch (...) {}
    flyout.Content(outerGrid);
    auto isClosingAnimating = std::make_shared<bool>(false);
    auto isCloseAnimationDone = std::make_shared<bool>(false);
    flyout.Closing([isClosingAnimating, isCloseAnimationDone, isTop, isLeft, isRight, isBottom](
        winrt::Windows::UI::Xaml::Controls::Primitives::FlyoutBase const& sender,
        winrt::Windows::UI::Xaml::Controls::Primitives::FlyoutBaseClosingEventArgs const& args) {
        
        if (g_modUnloaded || !g_win11FlyoutAnimation) return;
        if (*isCloseAnimationDone) {
            return;
        }
        if (*isClosingAnimating) {
            args.Cancel(true);
            return;
        }
        args.Cancel(true);
        *isClosingAnimating = true;
        try {
            using namespace winrt::Windows::UI::Xaml::Media::Animation;
            using namespace winrt::Windows::UI::Xaml::Media;
            auto actualFlyout = sender.try_as<winrt::Windows::UI::Xaml::Controls::Flyout>();
            auto outerG = actualFlyout ? actualFlyout.Content().try_as<winrt::Windows::UI::Xaml::Controls::Grid>() : nullptr;
            
            if (outerG) {
                try {
                    outerG.Transitions(nullptr);
                } catch (...) {}
                CompositeTransform trans;
                if (auto existingTrans = outerG.RenderTransform().try_as<CompositeTransform>()) {
                    trans = existingTrans;
                } else {
                    trans = CompositeTransform{};
                    outerG.RenderTransform(trans);
                }
                Storyboard storyboard;
                DoubleAnimation slideAnim;
                Storyboard::SetTarget(slideAnim, trans);
                double targetOffset = 420.0; // large slide to mimic native Windows 11 behavior
                if (isTop) {
                    Storyboard::SetTargetProperty(slideAnim, L"TranslateY");
                    targetOffset = -420.0;
                } else if (isLeft) {
                    Storyboard::SetTargetProperty(slideAnim, L"TranslateX");
                    targetOffset = -420.0;
                } else if (isRight) {
                    Storyboard::SetTargetProperty(slideAnim, L"TranslateX");
                    targetOffset = 420.0;
                } else if (isBottom) {
                    Storyboard::SetTargetProperty(slideAnim, L"TranslateY");
                    targetOffset = 420.0;
                }
                slideAnim.EnableDependentAnimation(true);
                slideAnim.From(0.0);
                slideAnim.To(targetOffset);
                slideAnim.Duration(winrt::Windows::UI::Xaml::DurationHelper::FromTimeSpan(std::chrono::milliseconds(180)));
                CubicEase cubicEase;
                cubicEase.EasingMode(EasingMode::EaseIn);
                slideAnim.EasingFunction(cubicEase);
                storyboard.Children().Append(slideAnim);
                
                auto finishClose = [flyout = sender, isClosingAnimating, isCloseAnimationDone]() {
                    if (*isCloseAnimationDone) return;
                    *isCloseAnimationDone = true;
                    *isClosingAnimating = false;
                    if (g_modUnloaded) return;
                    try {
                        flyout.Hide();
                    } catch (...) {}
                };
                TrackStoryboardCompleted(storyboard, [finishClose](auto const&, auto const&) {
                    if (g_modUnloaded) return;
                    finishClose();
                });
                storyboard.Begin();
                winrt::Windows::UI::Xaml::DispatcherTimer timer;
                timer.Interval(std::chrono::milliseconds(220));
                TrackOneShotTimer(timer, [finishClose](auto const&, auto const&) {
                    if (g_modUnloaded) return;
                    finishClose();
                });
                timer.Start();
            } else {
                *isCloseAnimationDone = true;
                *isClosingAnimating = false;
                sender.Hide();
            }
        } catch (...) {
            *isCloseAnimationDone = true;
            *isClosingAnimating = false;
            sender.Hide();
        }
    });
    flyout.Opened([isClosingAnimating, isCloseAnimationDone, outerGrid, rootGrid, isTop, isLeft, isRight, isBottom](auto const&, auto const&) {
        *isClosingAnimating = false;
        *isCloseAnimationDone = false;
        g_win11FlyoutIsOpen = true;
        UpdateWeatherButtonActiveState(true);
        if (g_win11FlyoutAnimation) {
            try {
                using namespace winrt::Windows::UI::Xaml::Media::Animation;
                using namespace winrt::Windows::UI::Xaml::Media;
                outerGrid.Transitions(nullptr);
                outerGrid.Opacity(1.0);
                CompositeTransform trans;
                if (auto existingTrans = outerGrid.RenderTransform().try_as<CompositeTransform>()) {
                    trans = existingTrans;
                } else {
                    trans = CompositeTransform{};
                    outerGrid.RenderTransform(trans);
                }
                Storyboard storyboard;
                DoubleAnimation slideAnim;
                Storyboard::SetTarget(slideAnim, trans);
                double startOffset = 420.0; // large slide to mimic native Windows 11 behavior
                if (isTop) {
                    Storyboard::SetTargetProperty(slideAnim, L"TranslateY");
                    startOffset = -420.0;
                } else if (isLeft) {
                    Storyboard::SetTargetProperty(slideAnim, L"TranslateX");
                    startOffset = -420.0;
                } else if (isRight) {
                    Storyboard::SetTargetProperty(slideAnim, L"TranslateX");
                    startOffset = 420.0;
                } else if (isBottom) {
                    Storyboard::SetTargetProperty(slideAnim, L"TranslateY");
                    startOffset = 420.0;
                }
                slideAnim.EnableDependentAnimation(true);
                slideAnim.From(startOffset);
                slideAnim.To(0.0);
                slideAnim.Duration(winrt::Windows::UI::Xaml::DurationHelper::FromTimeSpan(std::chrono::milliseconds(200)));
                CubicEase cubicEase;
                cubicEase.EasingMode(EasingMode::EaseOut);
                slideAnim.EasingFunction(cubicEase);
                storyboard.Children().Append(slideAnim);
                
                storyboard.Begin();
            } catch (...) {}
        } else {
            try {
                outerGrid.Transitions(nullptr);
                outerGrid.Opacity(1.0);
                if (auto trans = outerGrid.RenderTransform().try_as<winrt::Windows::UI::Xaml::Media::CompositeTransform>()) {
                    trans.TranslateX(0.0);
                    trans.TranslateY(0.0);
                }
            } catch (...) {}
        }
    });
    flyout.Closed([isClosingAnimating, isCloseAnimationDone](auto const&, auto const&) {
        CancelAnimationEventsOnCurrentThread();
        if (g_modUnloaded) return;
        StopRadarTimer();
        try { g_radarTileBitmapCache.clear(); } catch (...) {}
        *isClosingAnimating = false;
        *isCloseAnimationDone = false;
        g_win11FlyoutIsOpen = false;
        g_activeFlyout = nullptr;
        g_flyoutWasOpenOnPointerPressed = false;
        g_lastClosedTickCount = GetTickCount64();
        UpdateWeatherButtonActiveState(false);
        if (g_hMouseHook) {
            UnhookWindowsHookEx(g_hMouseHook);
            g_hMouseHook = NULL;
        }

        // Restore focus/foreground window when dismissed so focus does NOT go to Start button
        HWND hCurForeground = GetForegroundWindow();
        HWND hTaskbar = FindWindowW(L"Shell_TrayWnd", NULL);
        if (hCurForeground == hTaskbar || GetAncestor(hCurForeground, GA_ROOT) == hTaskbar || hCurForeground == NULL) {
            HWND hTarget = g_hPreFlyoutActiveWnd;
            if (!hTarget || !IsWindow(hTarget) || hTarget == hTaskbar || (GetAncestor(hTarget, GA_ROOT) != NULL && GetAncestor(hTarget, GA_ROOT) == hTaskbar)) {
                hTarget = FindWindowW(L"Progman", NULL);
            }
            if (hTarget && IsWindow(hTarget)) {
                DWORD dwCurrThread = GetCurrentThreadId();
                DWORD dwTargetThread = GetWindowThreadProcessId(hTarget, NULL);
                if (dwCurrThread != dwTargetThread && dwTargetThread != 0) {
                    AttachThreadInput(dwCurrThread, dwTargetThread, TRUE);
                    SetForegroundWindow(hTarget);
                    BringWindowToTop(hTarget);
                    SetFocus(hTarget);
                    AttachThreadInput(dwCurrThread, dwTargetThread, FALSE);
                } else {
                    SetForegroundWindow(hTarget);
                    BringWindowToTop(hTarget);
                    SetFocus(hTarget);
                }
            }
        }
        g_hPreFlyoutActiveWnd = NULL;
    });
    try {
        flyout.ShouldConstrainToRootBounds(false);
        if (isTop) {
            flyout.Placement(winrt::Windows::UI::Xaml::Controls::Primitives::FlyoutPlacementMode::Bottom);
        } else if (isLeft) {
            flyout.Placement(winrt::Windows::UI::Xaml::Controls::Primitives::FlyoutPlacementMode::RightEdgeAlignedTop);
        } else if (isRight) {
            flyout.Placement(winrt::Windows::UI::Xaml::Controls::Primitives::FlyoutPlacementMode::LeftEdgeAlignedTop);
        } else if (isBottom) {
            flyout.Placement(winrt::Windows::UI::Xaml::Controls::Primitives::FlyoutPlacementMode::Top);
        }
        flyout.AllowFocusOnInteraction(true);  // Allow interactions
        flyout.OverlayInputPassThroughElement(
            winrt::Windows::UI::Xaml::Controls::Primitives::FlyoutBase::
            GetAttachedFlyout(rootGrid));
    }
    catch (...) {
    }
    return flyout;
}
COLORREF GetIconColor(const std::wstring& condition);
winrt::Windows::UI::Color GetXamlIconColor(const std::wstring& condition) {
    COLORREF c = GetIconColor(condition);
    return winrt::Windows::UI::ColorHelper::FromArgb(255, GetRValue(c), GetGValue(c), GetBValue(c));
}
std::wstring EnforceOneDecimalTemp(const std::wstring& tempStr) {
    if (tempStr.empty() || tempStr == L"--" || tempStr.find(L"--") != std::wstring::npos) {
        return tempStr;
    }
    size_t degPos = tempStr.find(L"°");
    if (degPos == std::wstring::npos) {
        size_t dotPos = tempStr.find(L".");
        if (dotPos == std::wstring::npos) {
            wchar_t* endp = nullptr;
            double val = wcstod(tempStr.c_str(), &endp);
            if (endp != tempStr.c_str()) {
                wchar_t buf[64];
                swprintf(buf, L"%.1f%s", val, endp);
                return buf;
            }
        }
        return tempStr;
    }
    std::wstring beforeDeg = tempStr.substr(0, degPos);
    std::wstring afterDeg = tempStr.substr(degPos);
    if (beforeDeg.find(L".") == std::wstring::npos) {
        wchar_t* endp = nullptr;
        double val = wcstod(beforeDeg.c_str(), &endp);
        if (endp != beforeDeg.c_str()) {
            wchar_t buf[64];
            swprintf(buf, L"%.1f%s%s", val, endp, afterDeg.c_str());
            return buf;
        }
    }
    return tempStr;
}
std::wstring FormatNoDecimalTemp(const std::wstring& tempStr) {
    if (tempStr.empty() || tempStr == L"--" || tempStr.find(L"--") != std::wstring::npos) {
        return tempStr;
    }
    size_t degPos = tempStr.find(L"°");
    std::wstring beforeDeg = (degPos != std::wstring::npos) ? tempStr.substr(0, degPos) : tempStr;
    std::wstring afterDeg = (degPos != std::wstring::npos) ? tempStr.substr(degPos) : L"";
    wchar_t* endp = nullptr;
    double val = wcstod(beforeDeg.c_str(), &endp);
    if (endp != beforeDeg.c_str()) {
        wchar_t buf[64];
        swprintf(buf, L"%.0f%s%s", val, endp, afterDeg.c_str());
        return buf;
    }
    return tempStr;
}
std::wstring FormatTempDisplay(const std::wstring& tempStr) {
    if (tempStr.empty() || tempStr == L"--" || tempStr.find(L"--") != std::wstring::npos) {
        return tempStr;
    }
    if (!g_showTempDecimals) {
        return FormatNoDecimalTemp(tempStr);
    }
    return EnforceOneDecimalTemp(tempStr);
}
std::wstring MakeValidUri(const std::wstring& rawPathOrUrl) {
    if (rawPathOrUrl.empty()) return L"";
    if (rawPathOrUrl.find(L"://") != std::wstring::npos && rawPathOrUrl.find(L"file://") != 0) {
        return rawPathOrUrl; // Already HTTP or HTTPS URL
    }
    std::wstring path = rawPathOrUrl;
    if (path.find(L"file:///") == 0) {
        path = path.substr(8);
    } else if (path.find(L"file://") == 0) {
        path = path.substr(7);
    }
    for (auto& c : path) {
        if (c == L'\\') c = L'/';
    }
    std::wstring encoded = L"file:///";
    for (wchar_t c : path) {
        if (c == L' ') {
            encoded += L"%20";
        } else if (c == L'#') {
            encoded += L"%23";
        } else if (c == L'%') {
            encoded += L"%25";
        } else {
            encoded += c;
        }
    }
    return encoded;
}
static std::unordered_map<int, std::wstring> s_customIconCacheDay;
static std::unordered_map<int, std::wstring> s_customIconCacheNight;
static std::wstring s_lastCustomPath = L"";
static std::vector<std::pair<std::wstring, std::wstring>> s_indexedCustomFiles;
static std::mutex s_customIconCacheMutex;
[[clang::no_destroy]] std::map<std::wstring, winrt::Windows::UI::Xaml::Media::ImageSource> g_iconCache;
std::atomic<bool> g_needsIconCacheClear{false};
void ClearCustomIconCache() {
    std::lock_guard<std::mutex> lock(s_customIconCacheMutex);
    s_customIconCacheDay.clear();
    s_customIconCacheNight.clear();
    s_lastCustomPath = L"";
    s_indexedCustomFiles.clear();
    g_needsIconCacheClear = true;
}
void SetIconConfiguration(int style, std::wstring path) {
    std::lock_guard<std::mutex> lock(s_customIconCacheMutex);
    g_weatherStyle = style;
    g_customIconPackPath = std::move(path);
    s_customIconCacheDay.clear();
    s_customIconCacheNight.clear();
    s_lastCustomPath.clear();
    s_indexedCustomFiles.clear();
    g_needsIconCacheClear = true;
}
std::wstring GetWeatherIconPathOrUrl(int code, bool is_day) {
    std::lock_guard<std::mutex> lock(s_customIconCacheMutex);
    if (g_weatherStyle == 0 || g_weatherStyle == 1) {
        return L"";
    }
    if (g_weatherStyle == 2) {
        std::wstring msnSvgName = L"CloudyV3.svg"; // default fallback
        if (code == 0) {
            msnSvgName = is_day ? L"SunnyDayV3.svg" : L"ClearNightV3.svg";
        }
        else if (code == 1) {
            msnSvgName = is_day ? L"MostlySunnyDay.svg" : L"ClearNightV3.svg";
        }
        else if (code == 2) {
            msnSvgName = is_day ? L"PartlyCloudyDayV3.svg" : L"PartlyCloudyNightV2.svg";
        }
        else if (code == 3) {
            msnSvgName = is_day ? L"MostlyCloudyDayV2.svg" : L"MostlyCloudyNightV2.svg";
        }
        else if (code == 45 || code == 48) {
            msnSvgName = L"FogV2.svg";
        }
        else if (code >= 51 && code <= 55) {
            msnSvgName = L"LightRainV3.svg";
        }
        else if (code == 56 || code == 57) {
            msnSvgName = L"FreezingRainV2.svg";
        }
        else if (code >= 61 && code <= 65) {
            if (code == 61) {
                msnSvgName = L"LightRainV3.svg";
            } else {
                msnSvgName = is_day ? L"RainShowersDayV2.svg" : L"RainShowersNightV2.svg";
            }
        }
        else if (code == 66 || code == 67) {
            msnSvgName = L"FreezingRainV2.svg";
        }
        else if (code >= 71 && code <= 77) {
            if (code <= 72) {
                msnSvgName = L"LightSnowV2.svg";
            } else if (code == 73 || code == 75) {
                msnSvgName = L"Snow.svg";
            } else {
                msnSvgName = L"HeavySnowV2.svg";
            }
        }
        else if (code >= 80 && code <= 82) {
            msnSvgName = is_day ? L"RainShowersDayV2.svg" : L"RainShowersNightV2.svg";
        }
        else if (code == 85 || code == 86) {
            msnSvgName = is_day ? L"SnowShowersDayV2.svg" : L"SnowShowersNightV2.svg";
        }
        else if (code >= 95) {
            msnSvgName = L"ThunderstormsV2.svg";
        }
        return L"https://assets.msn.com/weathermapdata/1/static/weather/Icons/taskbar_v10/Condition_Card/" + msnSvgName;
    }
    if (g_weatherStyle == 4 && !g_customIconPackPath.empty()) {
        if (is_day) {
            auto it = s_customIconCacheDay.find(code);
            if (it != s_customIconCacheDay.end()) return it->second;
        } else {
            auto it = s_customIconCacheNight.find(code);
            if (it != s_customIconCacheNight.end()) return it->second;
        }
    }
    // Official Microsoft Fluent 3D Weather Icons from fluentui-emoji (crisp 256x256 high-resolution assets)
    std::wstring fluent3dName = L"Cloud/3D/cloud_3d.png";
    std::wstring standardName = L"cloudy.png";
    std::wstring baseName = L"Cloudy";
    std::vector<int> bingCodes = { 5, 4, 7, 8 }; // default cloudy
    if (code == 0) {
        baseName = is_day ? L"Sunny" : L"ClearNight";
        fluent3dName = is_day ? L"Sun/3D/sun_3d.png" : L"Crescent%20moon/3D/crescent_moon_3d.png";
        standardName = is_day ? L"sunny.png" : L"clear_night.png";
        bingCodes = is_day ? std::vector<int>{ 0, 1, 2 } : std::vector<int>{ 30, 31, 32, 1 };
    }
    else if (code == 1) {
        baseName = is_day ? L"MostlyCloudyDay" : L"MostlyCloudyNight";
        fluent3dName = is_day ? L"Sun%20behind%20small%20cloud/3D/sun_behind_small_cloud_3d.png" : L"Crescent%20moon/3D/crescent_moon_3d.png";
        standardName = is_day ? L"mainly_clear_day.png" : L"mainly_clear_night.png";
        bingCodes = is_day ? std::vector<int>{ 2, 3, 1 } : std::vector<int>{ 31, 32, 33 };
    }
    else if (code == 2) {
        baseName = is_day ? L"PartlyCloudyDay" : L"PartlyCloudyNight";
        fluent3dName = is_day ? L"Sun%20behind%20cloud/3D/sun_behind_cloud_3d.png" : L"Crescent%20moon/3D/crescent_moon_3d.png";
        standardName = is_day ? L"partly_cloudy_day.png" : L"partly_cloudy_night.png";
        bingCodes = is_day ? std::vector<int>{ 3, 4, 2 } : std::vector<int>{ 32, 33, 35 };
    }
    else if (code == 3) {
        baseName = L"Cloudy";
        fluent3dName = L"Cloud/3D/cloud_3d.png";
        standardName = L"cloudy.png";
        bingCodes = is_day ? std::vector<int>{ 6, 7, 8, 4, 5, 3 } : std::vector<int>{ 35, 7, 8, 4, 5 };
    }
    else if (code == 45 || code == 48) {
        baseName = L"Fog";
        fluent3dName = L"Fog/3D/fog_3d.png";
        standardName = L"fog.png";
        bingCodes = { 9, 10, 5, 6 };
    }
    else if (code >= 51 && code <= 55) {
        baseName = L"LightRain";
        fluent3dName = L"Cloud%20with%20rain/3D/cloud_with_rain_3d.png";
        standardName = L"drizzle.png";
        bingCodes = { 11, 13, 14, 8 };
    }
    else if (code == 56 || code == 57) {
        baseName = L"FreezingRain";
        fluent3dName = L"Snowflake/3D/snowflake_3d.png";
        standardName = L"freezing_rain.png";
        bingCodes = { 24, 25, 20, 21 };
    }
    else if (code >= 61 && code <= 65) {
        baseName = (code >= 64) ? L"HeavyRain" : L"Rain";
        fluent3dName = L"Cloud%20with%20rain/3D/cloud_with_rain_3d.png";
        standardName = (code >= 64) ? L"heavy_rain.png" : L"rain.png";
        bingCodes = (code >= 64) ? std::vector<int>{ 18, 12, 11 } : std::vector<int>{ 11, 18, 12, 9, 10 };
    }
    else if (code == 66 || code == 67) {
        baseName = L"FreezingRain";
        fluent3dName = L"Snowflake/3D/snowflake_3d.png";
        standardName = L"freezing_rain.png";
        bingCodes = { 25, 24, 20, 21 };
    }
    else if (code >= 71 && code <= 77) {
        baseName = (code <= 72) ? L"LightSnow" : L"Snow";
        fluent3dName = L"Cloud%20with%20snow/3D/cloud_with_snow_3d.png";
        standardName = L"snow.png";
        bingCodes = { 22, 19, 23, 20, 21, 14, 15, 16 };
    }
    else if (code >= 80 && code <= 82) {
        baseName = is_day ? L"RainShowersDay" : L"RainShowersNight";
        fluent3dName = is_day ? L"Sun%20behind%20rain%20cloud/3D/sun_behind_rain_cloud_3d.png" : L"Cloud%20with%20rain/3D/cloud_with_rain_3d.png";
        standardName = is_day ? L"showers_day.png" : L"showers_night.png";
        bingCodes = is_day ? std::vector<int>{ 11, 43, 12, 13, 14 } : std::vector<int>{ 36, 37, 43, 11 };
    }
    else if (code == 85 || code == 86) {
        baseName = is_day ? L"SnowShowersDay" : L"SnowShowersNight";
        fluent3dName = L"Cloud%20with%20snow/3D/cloud_with_snow_3d.png";
        standardName = L"snow_showers.png";
        bingCodes = is_day ? std::vector<int>{ 44, 23, 19, 21 } : std::vector<int>{ 40, 41, 44, 23 };
    }
    else if (code >= 95) {
        baseName = L"Thunderstorms";
        fluent3dName = L"Cloud%20with%20lightning%20and%20rain/3D/cloud_with_lightning_and_rain_3d.png";
        standardName = L"thunderstorm.png";
        bingCodes = is_day ? std::vector<int>{ 15, 16, 17, 42, 27, 28 } : std::vector<int>{ 38, 39, 42, 15 };
    }
    if (g_weatherStyle == 4 && !g_customIconPackPath.empty()) {
        std::wstring customPath = g_customIconPackPath;
        while (!customPath.empty() && (customPath.front() == L' ' || customPath.front() == L'\"')) customPath.erase(customPath.begin());
        while (!customPath.empty() && (customPath.back() == L' ' || customPath.back() == L'\"')) customPath.pop_back();
        if (customPath.find(L"://") != std::wstring::npos) {
            if (customPath.back() != L'/' && customPath.back() != L'\\') {
                customPath += L"/";
            }
            std::wstring res = customPath + standardName;
            if (is_day) s_customIconCacheDay[code] = res;
            else s_customIconCacheNight[code] = res;
            return res;
        }
        else {
            if (customPath.back() != L'/' && customPath.back() != L'\\') {
                customPath += L"\\";
            }
            DWORD attr = GetFileAttributesW(customPath.c_str());
            if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY)) {
                // Index folder files once in memory
                if (s_lastCustomPath != customPath) {
                    s_lastCustomPath = customPath;
                    s_indexedCustomFiles.clear();
                    auto indexFolder = [](const std::wstring& folderPath, const std::wstring& prefix) {
                        WIN32_FIND_DATAW ffd;
                        std::wstring pattern = folderPath + L"*.*";
                        HANDLE hFind = FindFirstFileW(pattern.c_str(), &ffd);
                        if (hFind != INVALID_HANDLE_VALUE) {
                            do {
                                if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                                    std::wstring fn = ffd.cFileName;
                                    std::wstring fnLower = fn;
                                    std::transform(fnLower.begin(), fnLower.end(), fnLower.begin(), ::towlower);
                                    s_indexedCustomFiles.push_back({ prefix + fnLower, folderPath + fn });
                                }
                            } while (FindNextFileW(hFind, &ffd));
                            FindClose(hFind);
                        }
                    };
                    indexFolder(customPath, L"");
                    std::vector<std::wstring> topSubfolders = {
                        L"106\\", L"106x106\\", L"WeatherIcons\\", L"WeatherIcons\\106\\",
                        L"WeatherIcons\\106x106\\", L"Assets\\", L"Assets\\AppTiles\\WeatherIcons\\106\\",
                        L"Assets\\AppTiles\\WeatherIcons\\106x106\\", L"AppTiles\\", L"AppTiles\\WeatherIcons\\106\\"
                    };
                    for (const auto& sub : topSubfolders) {
                        std::wstring subPath = customPath + sub;
                        DWORD subAttr = GetFileAttributesW(subPath.c_str());
                        if (subAttr != INVALID_FILE_ATTRIBUTES && (subAttr & FILE_ATTRIBUTE_DIRECTORY)) {
                            std::wstring subLower = sub;
                            std::transform(subLower.begin(), subLower.end(), subLower.begin(), ::towlower);
                            indexFolder(subPath, subLower);
                        }
                    }
                }
                // Check candidates against indexed files in memory
                std::vector<std::wstring> candidates;
                candidates.push_back(standardName);
                candidates.push_back(baseName + L".png");
                candidates.push_back(baseName + L".svg");
                candidates.push_back(L"106_" + baseName + L".png");
                candidates.push_back(L"106x106_" + baseName + L".png");
                candidates.push_back(L"AppTile_" + baseName + L".png");
                candidates.push_back(L"AppTile_" + baseName + L".scale-100.png");
                candidates.push_back(L"AppTile_" + baseName + L".scale-200.png");
                candidates.push_back(L"Weather_" + baseName + L".png");
                candidates.push_back(L"WeatherIcon_" + baseName + L".png");
                std::vector<int> allNumCodes = { code };
                for (int b : bingCodes) {
                    if (std::find(allNumCodes.begin(), allNumCodes.end(), b) == allNumCodes.end()) {
                        allNumCodes.push_back(b);
                    }
                }
                for (int n : allNumCodes) {
                    std::wstring numStr = std::to_wstring(n);
                    candidates.push_back(L"106_" + numStr + L".png");
                    candidates.push_back(L"106x106_" + numStr + L".png");
                    candidates.push_back(L"AppTile_106_" + numStr + L".png");
                    candidates.push_back(L"AppTile_106x106_" + numStr + L".png");
                    candidates.push_back(L"Weather_106_" + numStr + L".png");
                    candidates.push_back(L"WeatherIcon_106_" + numStr + L".png");
                    candidates.push_back(L"Weather_" + numStr + L".png");
                    candidates.push_back(L"WeatherIcon_" + numStr + L".png");
                    candidates.push_back(L"AppTile_" + numStr + L".png");
                    candidates.push_back(numStr + (is_day ? L"_day.png" : L"_night.png"));
                    candidates.push_back(numStr + (is_day ? L"d.png" : L"n.png"));
                    candidates.push_back(numStr + L".png");
                    candidates.push_back(numStr + L".svg");
                }
                // In-memory exact/suffix match
                for (const auto& candidate : candidates) {
                    std::wstring candLower = candidate;
                    std::transform(candLower.begin(), candLower.end(), candLower.begin(), ::towlower);
                    for (const auto& item : s_indexedCustomFiles) {
                        if (item.first == candLower || (item.first.length() >= candLower.length() && item.first.rfind(candLower) == item.first.length() - candLower.length())) {
                            std::wstring res = MakeValidUri(item.second);
                            if (is_day) s_customIconCacheDay[code] = res;
                            else s_customIconCacheNight[code] = res;
                            return res;
                        }
                    }
                }
                // In-memory substring fallback
                std::wstring baseLower = baseName;
                std::transform(baseLower.begin(), baseLower.end(), baseLower.begin(), ::towlower);
                for (const auto& item : s_indexedCustomFiles) {
                    if (item.first.find(baseLower) != std::wstring::npos) {
                        std::wstring res = MakeValidUri(item.second);
                        if (is_day) s_customIconCacheDay[code] = res;
                        else s_customIconCacheNight[code] = res;
                        return res;
                    }
                }
            }
        }
    }
    // Microsoft Fluent 3D Condition Cards (crisp 256x256 high-resolution assets)
    std::wstring defaultRes = L"https://cdn.jsdelivr.net/gh/microsoft/fluentui-emoji@main/assets/" + fluent3dName;
    if (g_weatherStyle == 4) {
        if (is_day) s_customIconCacheDay[code] = defaultRes;
        else s_customIconCacheNight[code] = defaultRes;
    }
    return defaultRes;
}
void SetImageOrFallback(
    winrt::Windows::UI::Xaml::Controls::Image img,
    winrt::Windows::UI::Xaml::Controls::FontIcon fb,
    const std::wstring& uriStr) {
    if (!img) return;
    if (g_needsIconCacheClear) {
        try { g_iconCache.clear(); } catch (...) {}
        g_needsIconCacheClear = false;
    }
    if (uriStr.empty()) {
        if (fb) fb.Visibility(winrt::Windows::UI::Xaml::Visibility::Visible);
        img.Visibility(winrt::Windows::UI::Xaml::Visibility::Collapsed);
        return;
    }
    if (fb) fb.Visibility(winrt::Windows::UI::Xaml::Visibility::Collapsed);
    img.Visibility(winrt::Windows::UI::Xaml::Visibility::Visible);
    img.Stretch(winrt::Windows::UI::Xaml::Media::Stretch::Uniform);
    img.HorizontalAlignment(winrt::Windows::UI::Xaml::HorizontalAlignment::Center);
    img.VerticalAlignment(winrt::Windows::UI::Xaml::VerticalAlignment::Center);
    RevokeImageHooksForImage(img);
    auto weakFb = winrt::make_weak(fb);
    auto weakImg = winrt::make_weak(img);
    auto failedToken = img.ImageFailed([weakFb, weakImg](auto const&, auto const&) {
        if (g_modUnloaded) return;
        if (auto f = weakFb.get()) {
            f.Visibility(winrt::Windows::UI::Xaml::Visibility::Visible);
        }
        if (auto i = weakImg.get()) {
            i.Visibility(winrt::Windows::UI::Xaml::Visibility::Collapsed);
        }
    });
    auto openedToken = img.ImageOpened([weakFb, weakImg](auto const&, auto const&) {
        if (g_modUnloaded) return;
        if (auto f = weakFb.get()) {
            f.Visibility(winrt::Windows::UI::Xaml::Visibility::Collapsed);
        }
        if (auto i = weakImg.get()) {
            i.Visibility(winrt::Windows::UI::Xaml::Visibility::Visible);
        }
    });
    {
        std::lock_guard<std::mutex> lock(g_imageHookMutex);
        g_imageHooks.push_back({
            winrt::get_abi(img), weakImg, openedToken, failedToken
        });
    }
    try {
        std::wstring formattedUri = MakeValidUri(uriStr);
        std::wstring lower = formattedUri;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::towlower);
        
        auto cacheIt = g_iconCache.find(formattedUri);
        if (cacheIt != g_iconCache.end()) {
            img.Source(cacheIt->second);
            if (fb) fb.Visibility(winrt::Windows::UI::Xaml::Visibility::Collapsed);
            img.Visibility(winrt::Windows::UI::Xaml::Visibility::Visible);
            return;
        }
        winrt::Windows::Foundation::Uri uri(formattedUri);
        if (lower.rfind(L".svg") != std::wstring::npos) {
            winrt::Windows::UI::Xaml::Media::Imaging::SvgImageSource svg;
            if (g_weatherStyle == 2 || lower.find(L"assets.msn.com") != std::wstring::npos) {
                // Several MSN weather assets are taller than 72px (for example,
                // RainShowersDayV2 is 72x78). Specifying both dimensions forced
                // those non-square SVGs into a 72x72 raster surface and could
                // clip the lowest raindrop. Let XAML derive the height from the
                // SVG viewBox so the source aspect ratio is preserved.
                svg.RasterizePixelWidth(72);
            }
            svg.UriSource(uri);
            img.Source(svg);
            g_iconCache.insert_or_assign(formattedUri, svg);
        } else {
            winrt::Windows::UI::Xaml::Media::Imaging::BitmapImage bmp;
            bmp.UriSource(uri);
            img.Source(bmp);
            g_iconCache.insert_or_assign(formattedUri, bmp);
        }
    }
    catch (...) {
        if (fb) fb.Visibility(winrt::Windows::UI::Xaml::Visibility::Visible);
        img.Visibility(winrt::Windows::UI::Xaml::Visibility::Collapsed);
    }
}
void PopulateForecastUI(winrt::Windows::UI::Xaml::Controls::Grid rootGrid,
    std::wstring condition,
    std::wstring currentIcon,
    std::wstring currentTemp,
    bool animate,
    bool animateGraph,
    std::wstring customPrecip,
    std::wstring customHumidity,
    std::wstring customWind,
    std::wstring customIconUrl,
    int zoomDelta) {
    using namespace winrt::Windows::UI::Xaml;
    using namespace winrt::Windows::UI::Xaml::Controls;
    using namespace winrt::Windows::UI::Xaml::Media;
    using namespace winrt::Windows::UI::Xaml::Media::Animation;
    using namespace winrt::Windows::UI::Xaml::Shapes;
    auto currentWeather = GetCurrentWeatherSnapshot();
    bool resumeRadarPlayback = g_selectedGraphTab == 3 && g_radarIsPlaying;
    StopRadarTimer();
    g_radarIsPlaying = resumeRadarPlayback;
    Canvas previousRadarMap{nullptr};
    void* radarOwner = winrt::get_abi(rootGrid);
    if (zoomDelta != 0 && g_selectedGraphTab == 3) {
        winrt::weak_ref<Canvas> oldMap;
        {
            std::lock_guard<std::mutex> lock(s_activeRadarMapsMutex);
            auto it = s_activeRadarMaps.find(radarOwner);
            if (it != s_activeRadarMaps.end()) oldMap = it->second;
        }
        if (auto map = oldMap.get()) {
            if (auto parent = map.Parent().try_as<Grid>()) {
                if (parent.Children().Size() > 0 && parent.Children().GetAt(0) == map) {
                    parent.Children().RemoveAt(0);
                    previousRadarMap = map;
                }
            }
        }
    }
    if (g_selectedGraphTab != 3) {
        std::lock_guard<std::mutex> lock(s_activeRadarMapsMutex);
        s_activeRadarMaps.erase(radarOwner);
    }
    bool localAcquired;
    std::vector<DailyForecast> localDaily;
    std::vector<HourlyForecast> localHourly;
    std::vector<RadarFrame> localRadarFrames;
    std::wstring localWindSpeed, localPrecipProb, localHumidity;
    EnterCriticalSection(&g_forecastLock);
    localDaily = g_forecastDaily;
    localHourly = g_forecastHourly;
    localRadarFrames = g_radarFrames;
    localWindSpeed = g_cachedWindSpeed;
    localPrecipProb = g_cachedPrecipProb;
    localHumidity = g_cachedHumidity;
    localAcquired = g_forecastAcquired;
    LeaveCriticalSection(&g_forecastLock);
    int panelWidth = 336;  // 4 cards: 4 * 70 + 3 * 8 + 32 = 336
    if (g_forecastDaysFetch >= 14)
        panelWidth = 570;  // 7 cards: 7 * 70 + 6 * 8 + 32 = 570
    else if (g_forecastDaysFetch >= 10)
        panelWidth = 492;  // 6 cards: 6 * 70 + 5 * 8 + 32 = 492
    if (!localAcquired) {
        rootGrid.Children().Clear();
        rootGrid.Width(panelWidth);
        rootGrid.Height(CalculatePanelHeight());
        rootGrid.MaxHeight(CalculatePanelHeight());
        StackPanel stack;
        stack.HorizontalAlignment(HorizontalAlignment::Center);
        stack.VerticalAlignment(VerticalAlignment::Center);
        stack.Spacing(12);
        ProgressRing ring;
        ring.IsActive(true);
        ring.Width(36);
        ring.Height(36);
        ring.HorizontalAlignment(HorizontalAlignment::Center);
        TextBlock text;
        text.Text(L"Fetching latest weather forecast...");
        text.FontSize(14);
        text.Foreground(SolidColorBrush{ IsSystemDarkMode() ? winrt::Windows::UI::Colors::White() : winrt::Windows::UI::Colors::Black() });
        text.HorizontalAlignment(HorizontalAlignment::Center);
        stack.Children().Append(ring);
        stack.Children().Append(text);
        rootGrid.Children().Append(stack);
        rootGrid.Background(SolidColorBrush{ winrt::Windows::UI::Colors::Transparent() });
        return;
    }
    if (g_selectedDate.empty() && !localDaily.empty()) {
        SYSTEMTIME st; GetLocalTime(&st);
        wchar_t dateBuf[32]; swprintf(dateBuf, L"%04d-%02d-%02d", st.wYear, st.wMonth, st.wDay);
        std::wstring sysDate(dateBuf);
        bool foundToday = false;
        for (const auto& d : localDaily) {
            if (d.rawDate == sysDate) {
                foundToday = true;
                break;
            }
        }
        g_selectedDate = foundToday ? sysDate : localDaily[0].rawDate;
        // Default hour
        if (g_selectedHour.empty()) {
            int sysHour = st.wHour;
            for (const auto& h : localHourly) {
                if (h.rawDate == g_selectedDate && h.hour24 == sysHour) {
                    g_selectedHour = h.timeString;
                    break;
                }
            }
        }
    }
    bool isDark = IsSystemDarkMode();
    auto primaryColor = isDark ? winrt::Windows::UI::Colors::White()
        : winrt::Windows::UI::Colors::Black();
    auto secondaryColor =
        isDark ? winrt::Windows::UI::ColorHelper::FromArgb(255, 200, 200, 200)
        : winrt::Windows::UI::ColorHelper::FromArgb(255, 90, 90, 90);
    auto tertiaryColor =
        isDark ? winrt::Windows::UI::ColorHelper::FromArgb(255, 150, 150, 150)
        : winrt::Windows::UI::ColorHelper::FromArgb(255, 120, 120, 120);
    auto dividerColor =
        isDark ? winrt::Windows::UI::ColorHelper::FromArgb(40, 255, 255, 255)
        : winrt::Windows::UI::ColorHelper::FromArgb(20, 0, 0, 0);
    auto cardBgColor =
        isDark ? winrt::Windows::UI::ColorHelper::FromArgb(30, 255, 255, 255)
        : winrt::Windows::UI::ColorHelper::FromArgb(15, 0, 0, 0);
    auto activeTabColor =
        isDark ? winrt::Windows::UI::ColorHelper::FromArgb(255, 129, 212, 250)
        : winrt::Windows::UI::ColorHelper::FromArgb(255, 0, 120, 215);
    bool isVideo = false;
    if (!g_bgImageVideoUrl.empty()) {
        std::wstring urlLower = g_bgImageVideoUrl;
        std::transform(urlLower.begin(), urlLower.end(), urlLower.begin(), ::towlower);
        isVideo = (urlLower.find(L".mp4") != std::wstring::npos ||
            urlLower.find(L".mov") != std::wstring::npos ||
            urlLower.find(L".avi") != std::wstring::npos ||
            urlLower.find(L".webm") != std::wstring::npos ||
            urlLower.find(L".mkv") != std::wstring::npos);
    }
    bool hasBg = false;
    try {
        if (!g_bgImageVideoUrl.empty() && rootGrid.Children().Size() > 0) {
            auto first = rootGrid.Children().GetAt(0);
            if (isVideo) {
                if (first.try_as<MediaElement>()) {
                    hasBg = true;
                }
            }
            else {
                if (first.try_as<Border>()) {
                    hasBg = true;
                }
            }
        }
    }
    catch (...) {}
    if (hasBg) {
        // Remove all children except the first one (the background element)
        while (rootGrid.Children().Size() > 1) {
            try {
                auto child = rootGrid.Children().GetAt(1);
                CleanupXamlMedia(child);
            }
            catch (...) {}
            rootGrid.Children().RemoveAt(1);
        }
    }
    else {
        try {
            uint32_t size = rootGrid.Children().Size();
            for (uint32_t i = 0; i < size; ++i) {
                CleanupXamlMedia(rootGrid.Children().GetAt(i));
            }
        }
        catch (...) {}
        rootGrid.Children().Clear();
    }
    if (!hasBg && !g_bgImageVideoUrl.empty()) {
        try {
            winrt::Windows::UI::Xaml::Media::Stretch stretchMode = winrt::Windows::UI::Xaml::Media::Stretch::UniformToFill;
            if (g_bgImageVideoStretch == 0) stretchMode = winrt::Windows::UI::Xaml::Media::Stretch::None;
            else if (g_bgImageVideoStretch == 1) stretchMode = winrt::Windows::UI::Xaml::Media::Stretch::Fill;
            else if (g_bgImageVideoStretch == 2) stretchMode = winrt::Windows::UI::Xaml::Media::Stretch::Uniform;
            else if (g_bgImageVideoStretch == 3) stretchMode = winrt::Windows::UI::Xaml::Media::Stretch::UniformToFill;
            if (isVideo) {
                MediaElement media;
                winrt::Windows::Foundation::Uri uri{ g_bgImageVideoUrl };
                media.Source(uri);
                media.IsLooping(true);
                media.IsMuted(true);
                media.AutoPlay(true);
                media.Stretch(stretchMode);
                media.Opacity(g_bgImageVideoOpacity / 100.0);
                media.HorizontalAlignment(HorizontalAlignment::Stretch);
                media.VerticalAlignment(VerticalAlignment::Stretch);
                // Hack: apply a microscopic transform to force XAML composition
                // instead of a hardware overlay, which prevents the transparent
                // hole-punch issue in XAML islands.
                winrt::Windows::UI::Xaml::Media::CompositeTransform trans;
                trans.Rotation(0.001);
                media.RenderTransform(trans);
                rootGrid.Children().Append(media);
            }
            else {
                Border imgBorder;
                imgBorder.HorizontalAlignment(HorizontalAlignment::Stretch);
                imgBorder.VerticalAlignment(VerticalAlignment::Stretch);
                if (IsWindows11()) {
                    imgBorder.CornerRadius(CornerRadius{ 8.0, 8.0, 8.0, 8.0 });
                }
                else {
                    imgBorder.CornerRadius(CornerRadius{ 0.0, 0.0, 0.0, 0.0 });
                }
                ImageBrush imgBrush;
                winrt::Windows::Foundation::Uri uri{ g_bgImageVideoUrl };
                winrt::Windows::UI::Xaml::Media::Imaging::BitmapImage bitmap{ uri };
                imgBrush.ImageSource(bitmap);
                imgBrush.Stretch(stretchMode);
                imgBrush.Opacity(g_bgImageVideoOpacity / 100.0);
                imgBorder.Background(imgBrush);
                rootGrid.Children().Append(imgBorder);
            }
        }
        catch (...) {
            // Gracefully ignore error loading background
        }
    }
    double panelHeight = CalculatePanelHeight();
    double prevHeight = rootGrid.Height();
    bool needHeightAnimation = IsWindows11() && (prevHeight > 50.0 && fabs(prevHeight - panelHeight) > 1.0);
    rootGrid.Width(panelWidth);
    rootGrid.MaxWidth(9999.0);
    if (needHeightAnimation) {
        rootGrid.Height(prevHeight);
        rootGrid.MaxHeight(std::max(prevHeight, panelHeight));
        Storyboard heightSb;
        DoubleAnimation hAnim;
        hAnim.EnableDependentAnimation(true);
        Storyboard::SetTarget(hAnim, rootGrid);
        Storyboard::SetTargetProperty(hAnim, L"(FrameworkElement.Height)");
        hAnim.From(prevHeight);
        hAnim.To(panelHeight);
        hAnim.Duration(winrt::Windows::UI::Xaml::DurationHelper::FromTimeSpan(std::chrono::milliseconds(220)));
        CubicEase ease;
        ease.EasingMode(EasingMode::EaseInOut);
        hAnim.EasingFunction(ease);
        DoubleAnimation maxHAnim;
        maxHAnim.EnableDependentAnimation(true);
        Storyboard::SetTarget(maxHAnim, rootGrid);
        Storyboard::SetTargetProperty(maxHAnim, L"(FrameworkElement.MaxHeight)");
        maxHAnim.From(std::max(prevHeight, panelHeight));
        maxHAnim.To(panelHeight);
        maxHAnim.Duration(winrt::Windows::UI::Xaml::DurationHelper::FromTimeSpan(std::chrono::milliseconds(220)));
        maxHAnim.EasingFunction(ease);
        heightSb.Children().Append(hAnim);
        heightSb.Children().Append(maxHAnim);
        auto weakR = winrt::make_weak(rootGrid);
        TrackStoryboardCompleted(heightSb, [weakR, panelHeight](auto const&, auto const&) {
            if (auto r = weakR.get()) {
                r.Height(panelHeight);
                r.MaxHeight(panelHeight);
            }
        });
        heightSb.Begin();
    } else {
        rootGrid.Height(panelHeight);
        rootGrid.MaxHeight(panelHeight);
    }
    if (IsWindows11()) {
        rootGrid.CornerRadius(CornerRadius{ 8.0, 8.0, 8.0, 8.0 });
        try {
            rootGrid.Clip(nullptr);
        }
        catch (...) {}
    }
    else {
        rootGrid.CornerRadius(CornerRadius{ 0.0, 0.0, 0.0, 0.0 });
        try {
            rootGrid.Clip(nullptr);
        }
        catch (...) {}
    }
    // Windows 11 uses the Flyout's outerGrid as the single acrylic backdrop.
    // Windows 10 hosts rootGrid directly in DesktopWindowXamlSource, so the
    // HostBackdrop AcrylicBrush must live on rootGrid itself (as in v0.8).
    if (IsWindows11()) {
        rootGrid.Background(SolidColorBrush{ winrt::Windows::UI::Colors::Transparent() });
    } else {
        winrt::Windows::UI::Color win10Bg = IsSystemDarkMode()
            ? winrt::Windows::UI::ColorHelper::FromArgb(255, 30, 30, 30)
            : winrt::Windows::UI::ColorHelper::FromArgb(255, 240, 240, 240);
        if (g_useAcrylic) {
            try {
                AcrylicBrush acrylic;
                acrylic.BackgroundSource(AcrylicBackgroundSource::HostBackdrop);
                acrylic.TintColor(win10Bg);
                acrylic.TintOpacity(g_acrylicOpacity / 100.0);
                acrylic.FallbackColor(win10Bg);
                rootGrid.Background(acrylic);
            } catch (...) {
                rootGrid.Background(SolidColorBrush{ win10Bg });
            }
        } else {
            rootGrid.Background(SolidColorBrush{ win10Bg });
        }
    }
    winrt::Windows::UI::Color borderColor;
    borderColor.A = 64;
    borderColor.R = 128;
    borderColor.G = 128;
    borderColor.B = 128;
    rootGrid.BorderBrush(SolidColorBrush{ winrt::Windows::UI::Colors::Transparent() });
    rootGrid.BorderThickness(Thickness{ 0, 0, 0, 0 });
    rootGrid.UseLayoutRounding(true);
    Grid contentGrid;
    contentGrid.UseLayoutRounding(true);
    RowDefinition alertRowDef;
    alertRowDef.Height(GridLength{ 1.0, GridUnitType::Auto });
    RowDefinition weatherRowDef;
    weatherRowDef.Height(GridLength{ 1.0, GridUnitType::Star });
    contentGrid.RowDefinitions().Append(alertRowDef);
    contentGrid.RowDefinitions().Append(weatherRowDef);
    StackPanel mainStack;
    mainStack.Orientation(Orientation::Vertical);
    mainStack.VerticalAlignment(VerticalAlignment::Top);
    // Optimized static padding and spacing for extreme compactness
    double sidePad = 8.0;
    mainStack.Padding(Thickness{ sidePad, 4.0, sidePad, 4.0 });
    double mSpacing = 1.5;
    mainStack.Spacing(mSpacing);
    if (animate) {
        winrt::Windows::UI::Xaml::Media::Animation::TransitionCollection mainTrans;
        winrt::Windows::UI::Xaml::Media::Animation::EntranceThemeTransition mainEnt;
        mainEnt.IsStaggeringEnabled(true);
        mainEnt.FromVerticalOffset(15.0);
        mainTrans.Append(mainEnt);
        mainStack.ChildrenTransitions(mainTrans);
    }
    auto alertSnapshot = GetAlertSnapshot();
    bool hasWarning = !alertSnapshot.warnings.empty() && g_showWeatherAlerts;
    if (hasWarning) {
        int alertIndex = alertSnapshot.currentIndex;
        if (alertIndex < 0 || alertIndex >= (int)alertSnapshot.warnings.size()) {
            alertIndex = 0;
        }
        WeatherAlert curAlert = alertSnapshot.warnings[alertIndex];
        Border warningContainer;
        warningContainer.CornerRadius(CornerRadius{ 6, 6, 6, 6 });
        warningContainer.Margin(Thickness{ 0, 2, 0, 4 });
        bool isDark = IsSystemDarkMode();
        const AlertTone alertTone = GetAlertTone(curAlert);
        
        winrt::Windows::UI::Color warningBgColor = GetAlertColour(alertTone, isDark ? 35 : 25, false, isDark);
        winrt::Windows::UI::Color warningBorderColor = GetAlertColour(alertTone, isDark ? 100 : 80, false, isDark);
        winrt::Windows::UI::Color warningTextColor = GetAlertColour(alertTone, 255, true, isDark);
        warningContainer.Background(SolidColorBrush{ warningBgColor });
        warningContainer.BorderBrush(SolidColorBrush{ warningBorderColor });
        warningContainer.BorderThickness(Thickness{ 1, 1, 1, 1 });
        StackPanel mainAlertStack;
        mainAlertStack.Orientation(Orientation::Vertical);
        mainAlertStack.Padding(Thickness{ 8, 5, 8, 5 });
        mainAlertStack.Spacing(4);
        // Header Grid
        Grid alertHeaderGrid;
        ColumnDefinition col1, col2, col3;
        col1.Width(GridLength{ 1.0, GridUnitType::Star });
        col2.Width(GridLength{ 1.0, GridUnitType::Auto });
        col3.Width(GridLength{ 1.0, GridUnitType::Auto });
        alertHeaderGrid.ColumnDefinitions().Append(col1);
        alertHeaderGrid.ColumnDefinitions().Append(col2);
        alertHeaderGrid.ColumnDefinitions().Append(col3);
        // Col 0: Title & Icon
        StackPanel titleStack;
        titleStack.Orientation(Orientation::Horizontal);
        titleStack.Spacing(6);
        titleStack.VerticalAlignment(VerticalAlignment::Center);
        TextBlock iconTB;
        iconTB.Text(curAlert.customIcon.empty() ? L"⚠️" : curAlert.customIcon);
        iconTB.FontSize(12);
        iconTB.VerticalAlignment(VerticalAlignment::Center);
        TextBlock titleTB;
        titleTB.Text(curAlert.eventDesc);
        titleTB.FontSize(11.5);
        titleTB.FontWeight(winrt::Windows::UI::Text::FontWeights::Bold());
        titleTB.Foreground(SolidColorBrush{ warningTextColor });
        titleTB.VerticalAlignment(VerticalAlignment::Center);
        titleStack.Children().Append(iconTB);
        titleStack.Children().Append(titleTB);
        Grid::SetColumn(titleStack, 0);
        // Col 1: Pagination / Next Button
        StackPanel navStack;
        navStack.Orientation(Orientation::Horizontal);
        navStack.Spacing(4);
        navStack.VerticalAlignment(VerticalAlignment::Center);
        navStack.Margin(Thickness{ 4, 0, 6, 0 });
        if (alertSnapshot.warnings.size() > 1) {
            TextBlock pageCountTB;
            wchar_t pageBuf[32];
            swprintf(pageBuf, L"%d/%d", alertIndex + 1, (int)alertSnapshot.warnings.size());
            pageCountTB.Text(pageBuf);
            pageCountTB.FontSize(10.5);
            pageCountTB.Foreground(SolidColorBrush{ warningTextColor });
            pageCountTB.Opacity(0.85);
            pageCountTB.VerticalAlignment(VerticalAlignment::Center);
            Border nextBtn;
            nextBtn.CornerRadius(CornerRadius{ 3, 3, 3, 3 });
            nextBtn.Padding(Thickness{ 5, 2, 5, 2 });
            winrt::Windows::UI::Color btnBg = GetAlertColour(alertTone, isDark ? 45 : 30, false, isDark);
            nextBtn.Background(SolidColorBrush{ btnBg });
            TextBlock nextTB;
            nextTB.Text(L"Next ❯");
            nextTB.FontSize(10);
            nextTB.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());
            nextTB.Foreground(SolidColorBrush{ warningTextColor });
            nextBtn.Child(nextTB);
            auto weakRootNav = winrt::make_weak(rootGrid);
            nextBtn.Tapped([weakRootNav, condition, currentIcon, currentTemp, customPrecip, customHumidity, customWind](winrt::Windows::Foundation::IInspectable const&, winrt::Windows::UI::Xaml::Input::TappedRoutedEventArgs const& e) {
                e.Handled(true);
                {
                    std::lock_guard<std::mutex> alertLock(g_alertMutex);
                    if (!g_activeWarnings.empty()) {
                        g_currentWarningIndex = (g_currentWarningIndex + 1) % g_activeWarnings.size();
                        UpdateActiveWarningTextLocked();
                    }
                }
                if (auto root = weakRootNav.get()) {
                    PopulateForecastUI(root, condition, currentIcon, currentTemp, false, false, customPrecip, customHumidity, customWind);
                }
            });
            navStack.Children().Append(pageCountTB);
            navStack.Children().Append(nextBtn);
        }
        Grid::SetColumn(navStack, 1);
        // Col 2: Accordion Caret Button
        Border caretBtn;
        caretBtn.CornerRadius(CornerRadius{ 3, 3, 3, 3 });
        caretBtn.Padding(Thickness{ 6, 2, 6, 2 });
        winrt::Windows::UI::Color caretBg = GetAlertColour(alertTone, isDark ? 35 : 20, false, isDark);
        caretBtn.Background(SolidColorBrush{ caretBg });
        TextBlock caretTB;
        caretTB.Text(L"▼");
        caretTB.FontSize(10);
        caretTB.Foreground(SolidColorBrush{ warningTextColor });
        RotateTransform caretRotate;
        caretRotate.Angle(alertSnapshot.expanded ? 180.0 : 0.0);
        caretTB.RenderTransform(caretRotate);
        caretTB.RenderTransformOrigin(winrt::Windows::Foundation::Point{ 0.5, 0.5 });
        caretBtn.Child(caretTB);
        Grid::SetColumn(caretBtn, 2);
        alertHeaderGrid.Children().Append(titleStack);
        alertHeaderGrid.Children().Append(navStack);
        alertHeaderGrid.Children().Append(caretBtn);
        // Tapping header toggles accordion expand/collapse
        StackPanel expandedContainer;
        expandedContainer.Orientation(Orientation::Vertical);
        expandedContainer.MaxHeight(alertSnapshot.expanded ? 60.0 : 0.0);
        expandedContainer.Opacity(alertSnapshot.expanded ? 1.0 : 0.0);
        expandedContainer.Visibility(alertSnapshot.expanded ? Visibility::Visible : Visibility::Collapsed);
        
        auto weakExpandedContainer = winrt::make_weak(expandedContainer);
        auto weakCaretTB = winrt::make_weak(caretTB);
        auto weakCaretRotate = winrt::make_weak(caretRotate);
        auto weakRootGrid = winrt::make_weak(rootGrid);
        
        alertHeaderGrid.Tapped([weakExpandedContainer, weakCaretTB, weakCaretRotate, weakRootGrid](winrt::Windows::Foundation::IInspectable const&, winrt::Windows::UI::Xaml::Input::TappedRoutedEventArgs const&) {
            using namespace winrt::Windows::UI::Xaml::Media::Animation;
            bool nowExpanded = false;
            {
                std::lock_guard<std::mutex> alertLock(g_alertMutex);
                g_isAlertExpanded = !g_isAlertExpanded;
                nowExpanded = g_isAlertExpanded;
            }
            if (!ShouldUseXamlTaskbar()) {
                ResizeWin10PopupToCurrentContent();
            }
            double collapsedH = kBasePanelHeight + (g_selectedGraphTab == 3 ? 110.0 : 0.0) + 34.0;
            double expandedH = collapsedH + 60.0;
            if (auto rotate = weakCaretRotate.get()) {
                Storyboard caretStoryboard;
                DoubleAnimation caretAnim;
                caretAnim.EnableDependentAnimation(true);
                Storyboard::SetTarget(caretAnim, rotate);
                Storyboard::SetTargetProperty(caretAnim, L"Angle");
                caretAnim.From(nowExpanded ? 0.0 : 180.0);
                caretAnim.To(nowExpanded ? 180.0 : 0.0);
                caretAnim.Duration(winrt::Windows::UI::Xaml::DurationHelper::FromTimeSpan(std::chrono::milliseconds(200)));
                CubicEase ease;
                ease.EasingMode(EasingMode::EaseInOut);
                caretAnim.EasingFunction(ease);
                caretStoryboard.Children().Append(caretAnim);
                TrackStoryboardCompleted(caretStoryboard, [rotate, nowExpanded](auto const&, auto const&) {
                    rotate.Angle(nowExpanded ? 180.0 : 0.0);
                });
                caretStoryboard.Begin();
            } else if (auto caret = weakCaretTB.get()) {
                caret.Text(nowExpanded ? L"▲" : L"▼");
            }
            if (auto container = weakExpandedContainer.get()) {
                if (nowExpanded) {
                    container.Visibility(Visibility::Visible);
                    container.MaxHeight(60.0);
                    Storyboard storyboard;
                    DoubleAnimation heightAnim;
                    heightAnim.EnableDependentAnimation(true);
                    Storyboard::SetTarget(heightAnim, container);
                    Storyboard::SetTargetProperty(heightAnim, L"(FrameworkElement.MaxHeight)");
                    heightAnim.From(0.0);
                    heightAnim.To(60.0);
                    heightAnim.Duration(winrt::Windows::UI::Xaml::DurationHelper::FromTimeSpan(std::chrono::milliseconds(220)));
                    DoubleAnimation opacityAnim;
                    opacityAnim.EnableDependentAnimation(true);
                    Storyboard::SetTarget(opacityAnim, container);
                    Storyboard::SetTargetProperty(opacityAnim, L"Opacity");
                    opacityAnim.From(0.0);
                    opacityAnim.To(1.0);
                    opacityAnim.Duration(winrt::Windows::UI::Xaml::DurationHelper::FromTimeSpan(std::chrono::milliseconds(200)));
                    CubicEase cubicEase;
                    cubicEase.EasingMode(EasingMode::EaseOut);
                    heightAnim.EasingFunction(cubicEase);
                    opacityAnim.EasingFunction(cubicEase);
                    storyboard.Children().Append(heightAnim);
                    storyboard.Children().Append(opacityAnim);
                    if (auto root = weakRootGrid.get()) {
                        DoubleAnimation rootHAnim;
                        rootHAnim.EnableDependentAnimation(true);
                        Storyboard::SetTarget(rootHAnim, root);
                        Storyboard::SetTargetProperty(rootHAnim, L"(FrameworkElement.Height)");
                        rootHAnim.From(collapsedH);
                        rootHAnim.To(expandedH);
                        rootHAnim.Duration(winrt::Windows::UI::Xaml::DurationHelper::FromTimeSpan(std::chrono::milliseconds(220)));
                        rootHAnim.EasingFunction(cubicEase);
                        DoubleAnimation rootMaxHAnim;
                        rootMaxHAnim.EnableDependentAnimation(true);
                        Storyboard::SetTarget(rootMaxHAnim, root);
                        Storyboard::SetTargetProperty(rootMaxHAnim, L"(FrameworkElement.MaxHeight)");
                        rootMaxHAnim.From(collapsedH);
                        rootMaxHAnim.To(expandedH);
                        rootMaxHAnim.Duration(winrt::Windows::UI::Xaml::DurationHelper::FromTimeSpan(std::chrono::milliseconds(220)));
                        rootMaxHAnim.EasingFunction(cubicEase);
                        storyboard.Children().Append(rootHAnim);
                        storyboard.Children().Append(rootMaxHAnim);
                    }
                    auto weakContainer = winrt::make_weak(container);
                    auto weakRoot = weakRootGrid;
                    TrackStoryboardCompleted(storyboard, [weakContainer, weakRoot, expandedH, nowExpanded](auto const&, auto const&) {
                        if (auto c = weakContainer.get()) {
                            if (nowExpanded) {
                                c.MaxHeight(60.0);
                                c.Opacity(1.0);
                            }
                        }
                        if (auto r = weakRoot.get()) {
                            if (nowExpanded) {
                                r.Height(expandedH);
                                r.MaxHeight(expandedH);
                            }
                        }
                    });
                    storyboard.Begin();
                } else {
                    Storyboard storyboard;
                    DoubleAnimation heightAnim;
                    heightAnim.EnableDependentAnimation(true);
                    Storyboard::SetTarget(heightAnim, container);
                    Storyboard::SetTargetProperty(heightAnim, L"(FrameworkElement.MaxHeight)");
                    heightAnim.From(container.MaxHeight() > 0 ? container.MaxHeight() : 60.0);
                    heightAnim.To(0.0);
                    heightAnim.Duration(winrt::Windows::UI::Xaml::DurationHelper::FromTimeSpan(std::chrono::milliseconds(200)));
                    DoubleAnimation opacityAnim;
                    opacityAnim.EnableDependentAnimation(true);
                    Storyboard::SetTarget(opacityAnim, container);
                    Storyboard::SetTargetProperty(opacityAnim, L"Opacity");
                    opacityAnim.From(container.Opacity());
                    opacityAnim.To(0.0);
                    opacityAnim.Duration(winrt::Windows::UI::Xaml::DurationHelper::FromTimeSpan(std::chrono::milliseconds(160)));
                    CubicEase cubicEase;
                    cubicEase.EasingMode(EasingMode::EaseIn);
                    heightAnim.EasingFunction(cubicEase);
                    opacityAnim.EasingFunction(cubicEase);
                    storyboard.Children().Append(heightAnim);
                    storyboard.Children().Append(opacityAnim);
                    if (auto root = weakRootGrid.get()) {
                        double curRootH = root.Height() > 0 ? root.Height() : expandedH;
                        DoubleAnimation rootHAnim;
                        rootHAnim.EnableDependentAnimation(true);
                        Storyboard::SetTarget(rootHAnim, root);
                        Storyboard::SetTargetProperty(rootHAnim, L"(FrameworkElement.Height)");
                        rootHAnim.From(curRootH);
                        rootHAnim.To(collapsedH);
                        rootHAnim.Duration(winrt::Windows::UI::Xaml::DurationHelper::FromTimeSpan(std::chrono::milliseconds(200)));
                        rootHAnim.EasingFunction(cubicEase);
                        DoubleAnimation rootMaxHAnim;
                        rootMaxHAnim.EnableDependentAnimation(true);
                        Storyboard::SetTarget(rootMaxHAnim, root);
                        Storyboard::SetTargetProperty(rootMaxHAnim, L"(FrameworkElement.MaxHeight)");
                        rootMaxHAnim.From(curRootH);
                        rootMaxHAnim.To(collapsedH);
                        rootMaxHAnim.Duration(winrt::Windows::UI::Xaml::DurationHelper::FromTimeSpan(std::chrono::milliseconds(200)));
                        rootMaxHAnim.EasingFunction(cubicEase);
                        storyboard.Children().Append(rootHAnim);
                        storyboard.Children().Append(rootMaxHAnim);
                    }
                    auto weakContainer = winrt::make_weak(container);
                    auto weakRoot = weakRootGrid;
                    TrackStoryboardCompleted(storyboard, [weakContainer, weakRoot, collapsedH, nowExpanded](auto const&, auto const&) {
                        if (auto c = weakContainer.get()) {
                            if (!nowExpanded) {
                                c.MaxHeight(0.0);
                                c.Opacity(0.0);
                                c.Visibility(Visibility::Collapsed);
                            }
                        }
                        if (auto r = weakRoot.get()) {
                            if (!nowExpanded) {
                                r.Height(collapsedH);
                                r.MaxHeight(collapsedH);
                            }
                        }
                    });
                    storyboard.Begin();
                }
            }
        });
        mainAlertStack.Children().Append(alertHeaderGrid);
        // Expanded Accordion View with ScrollViewer
        winrt::Windows::UI::Xaml::Shapes::Rectangle div;
        div.Height(1);
        winrt::Windows::UI::Color divColor = GetAlertColour(alertTone, isDark ? 50 : 35, false, isDark);
        div.Fill(SolidColorBrush{ divColor });
        expandedContainer.Children().Append(div);
        ScrollViewer alertScroll;
        alertScroll.MaxHeight(60.0);
            alertScroll.VerticalScrollBarVisibility(ScrollBarVisibility::Auto);
            alertScroll.HorizontalScrollBarVisibility(ScrollBarVisibility::Disabled);
            alertScroll.VerticalScrollMode(ScrollMode::Enabled);
            alertScroll.HorizontalScrollMode(ScrollMode::Disabled);
            StackPanel detailStack;
            detailStack.Orientation(Orientation::Vertical);
            detailStack.Spacing(4);
            detailStack.Padding(Thickness{ 0, 2, 4, 2 });
            if (!curAlert.headlineText.empty() && curAlert.headlineText != curAlert.eventDesc) {
                TextBlock headlineTB;
                headlineTB.Text(curAlert.headlineText);
                headlineTB.FontSize(11);
                headlineTB.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());
                headlineTB.Foreground(SolidColorBrush{ warningTextColor });
                headlineTB.TextWrapping(TextWrapping::Wrap);
                detailStack.Children().Append(headlineTB);
            }
            if (!curAlert.areaName.empty() || !curAlert.severity.empty() || !curAlert.source.empty()) {
                std::wstring metaText = L"";
                if (!curAlert.areaName.empty()) metaText += L"📍 Area: " + curAlert.areaName;
                if (!curAlert.severity.empty()) {
                    if (!metaText.empty()) metaText += L"  •  ";
                    metaText += L"Severity: " + curAlert.severity;
                }
                if (!curAlert.source.empty()) {
                    if (!metaText.empty()) metaText += L"  •  ";
                    metaText += L"Source: " + curAlert.source;
                }
                TextBlock metaTB;
                metaTB.Text(metaText);
                metaTB.FontSize(10);
                metaTB.Foreground(SolidColorBrush{ warningTextColor });
                metaTB.Opacity(0.85);
                metaTB.TextWrapping(TextWrapping::Wrap);
                detailStack.Children().Append(metaTB);
            }
            if (!curAlert.details.empty() && curAlert.details != curAlert.headlineText && curAlert.details != curAlert.eventDesc) {
                TextBlock detailsTB;
                detailsTB.Text(curAlert.details);
                detailsTB.FontSize(10.5);
                detailsTB.Foreground(SolidColorBrush{ warningTextColor });
                detailsTB.Opacity(0.95);
                detailsTB.TextWrapping(TextWrapping::Wrap);
                detailStack.Children().Append(detailsTB);
            }
            alertScroll.Content(detailStack);
            expandedContainer.Children().Append(alertScroll);
            
        mainAlertStack.Children().Append(expandedContainer);
        warningContainer.Child(mainAlertStack);
        Grid::SetRow(warningContainer, 0);
        warningContainer.VerticalAlignment(VerticalAlignment::Top);
        warningContainer.Margin(Thickness{ sidePad, 4.0, sidePad, 0.0 });
        contentGrid.Children().Append(warningContainer);
    }
    // --- Header Row (Location & Details aligned level)
    Grid headerGrid;
    GridLength h1Len = { 1.0, GridUnitType::Star };
    GridLength h2Len = { 1.0, GridUnitType::Auto };
    ColumnDefinition hCol1;
    hCol1.Width(h1Len);
    ColumnDefinition hCol2;
    hCol2.Width(h2Len);
    headerGrid.ColumnDefinitions().Append(hCol1);
    headerGrid.ColumnDefinitions().Append(hCol2);
    StackPanel locationContainer;
    locationContainer.Orientation(Orientation::Vertical);
    locationContainer.Spacing(1);
    locationContainer.VerticalAlignment(VerticalAlignment::Top);
    locationContainer.Margin(Thickness{ 6, 2, 0, 0 });
    Grid::SetColumn(locationContainer, 0);
    StackPanel titleStack;
    titleStack.Orientation(Orientation::Horizontal);
    titleStack.Spacing(8);
    titleStack.VerticalAlignment(VerticalAlignment::Center);
    TextBlock cityTitle;
    cityTitle.Text(currentWeather.displayCity);
    cityTitle.FontSize(18);
    cityTitle.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());
    cityTitle.Foreground(SolidColorBrush{ primaryColor });
    cityTitle.VerticalAlignment(VerticalAlignment::Center);
    cityTitle.Margin(Thickness{ 0, 0, 0, 0 });
    titleStack.Children().Append(cityTitle);
    if (g_showWeatherAlerts &&
        alertSnapshot.fetchStatus == AlertFetchStatus::Unavailable) {
        TextBlock alertStatus;
        alertStatus.Text(L"ⓘ");
        alertStatus.FontSize(12);
        alertStatus.Foreground(SolidColorBrush{ tertiaryColor });
        alertStatus.VerticalAlignment(VerticalAlignment::Center);
        ToolTipService::SetToolTip(alertStatus,
            winrt::box_value(L"Weather alerts could not be checked. Refresh to try again."));
        titleStack.Children().Append(alertStatus);
    }
    Button refreshBtn;
    refreshBtn.Content(winrt::box_value(L"\uE72C")); // Refresh Segoe MDL2 icon
    refreshBtn.FontFamily(winrt::Windows::UI::Xaml::Media::FontFamily(L"Segoe MDL2 Assets"));
    refreshBtn.FontSize(12);
    refreshBtn.Foreground(SolidColorBrush{ secondaryColor });
    refreshBtn.Background(SolidColorBrush{ winrt::Windows::UI::Colors::Transparent() });
    refreshBtn.BorderThickness(Thickness{ 0, 0, 0, 0 });
    refreshBtn.Padding(Thickness{ 4, 4, 4, 4 });
    refreshBtn.VerticalAlignment(VerticalAlignment::Center);
    refreshBtn.Margin(Thickness{ 0, 0, 0, 0 });
    ToolTipService::SetToolTip(refreshBtn, winrt::box_value(L"Refresh weather data"));
    try {
        refreshBtn.UseSystemFocusVisuals(false);
        refreshBtn.IsTabStop(false);
    }
    catch (...) {}
    refreshBtn.Click([](auto const& sender, auto const&) {
        if (g_hForceUpdateEvent) {
            SetEvent(g_hForceUpdateEvent);
        }
        try {
            if (auto btn = sender.template try_as<Button>()) {
                btn.IsEnabled(false);
                btn.Content(winrt::box_value(L"Refreshing..."));
                btn.FontFamily(winrt::Windows::UI::Xaml::Media::FontFamily(L"Segoe UI"));
                btn.FontSize(10);
            }
        }
        catch (...) {}
        });
    titleStack.Children().Append(refreshBtn);
    locationContainer.Children().Append(titleStack);
    if (!g_lastUpdatedTime.empty()) {
        TextBlock lastUpdatedTB;
        lastUpdatedTB.Text(L"Updated " + g_lastUpdatedTime);
        lastUpdatedTB.FontSize(10);
        lastUpdatedTB.Foreground(SolidColorBrush{ secondaryColor });
        lastUpdatedTB.Opacity(0.85);
        ToolTipService::SetToolTip(lastUpdatedTB, winrt::box_value(L"Time of last weather data fetch"));
        locationContainer.Children().Append(lastUpdatedTB);
    }
    headerGrid.Children().Append(locationContainer);
    StackPanel rightCurrent;
    rightCurrent.Orientation(Orientation::Vertical);
    rightCurrent.HorizontalAlignment(HorizontalAlignment::Right);
    rightCurrent.VerticalAlignment(VerticalAlignment::Top);
    rightCurrent.Spacing(1);
    rightCurrent.Margin(Thickness{ 0, 2, 0, 0 });
    auto MakeSmallText = [secondaryColor](std::wstring text, std::wstring tooltipText = L"") {
        TextBlock tb;
        tb.Text(text);
        tb.FontSize(11.5);
        tb.HorizontalAlignment(HorizontalAlignment::Right);
        tb.Foreground(SolidColorBrush{ secondaryColor });
        if (!tooltipText.empty()) {
            ToolTipService::SetToolTip(tb, winrt::box_value(tooltipText));
        }
        return tb;
        };
    std::wstring repPrecip = customPrecip;
    std::wstring repHumidity = customHumidity;
    std::wstring repWind = customWind;
    if (repPrecip.empty() || repHumidity.empty() || repWind.empty()) {
        std::wstring targetHourStr = g_selectedHour;
        bool foundRep = false;
        for (const auto& h : localHourly) {
            if (h.rawDate == g_selectedDate && h.timeString == targetHourStr) {
                if (repPrecip.empty()) repPrecip = h.precipProb;
                if (repHumidity.empty()) repHumidity = h.humidity;
                if (repWind.empty()) repWind = h.windSpeed;
                foundRep = true;
                break;
            }
        }
        if (!foundRep) {
            for (const auto& h : localHourly) {
                if (h.rawDate == g_selectedDate) {
                    if (repPrecip.empty()) repPrecip = h.precipProb;
                    if (repHumidity.empty()) repHumidity = h.humidity;
                    if (repWind.empty()) repWind = h.windSpeed;
                    break;
                }
            }
        }
    }
    std::wstring dispPrecip = repPrecip.empty() ? localPrecipProb : repPrecip;
    std::wstring dispHumidity = repHumidity.empty() ? localHumidity : repHumidity;
    std::wstring dispWind = repWind.empty() ? localWindSpeed : repWind;
    // Weather status text in the upper right, above Precipitation
    TextBlock condText;
    condText.Text(condition);
    condText.FontSize(12.5);
    condText.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());
    condText.Foreground(SolidColorBrush{ primaryColor });
    condText.HorizontalAlignment(HorizontalAlignment::Right);
    condText.Margin(Thickness{ 0, 0, 0, 1 });
    ToolTipService::SetToolTip(condText, winrt::box_value(L"Current condition: " + condition));
    rightCurrent.Children().Append(condText);
    rightCurrent.Children().Append(
        MakeSmallText(L"Precipitation: " + dispPrecip, L"Probability of precipitation"));
    rightCurrent.Children().Append(
        MakeSmallText(L"Humidity: " + dispHumidity, L"Relative humidity"));
    // Split wind speed (e.g. "12.0 mph NNW") so wind speed is on one line and the direction (NNW) is on a new line below.
    std::wstring windSpeedOnly = dispWind;
    std::wstring windDirOnly = L"";
    size_t lastSpace = dispWind.find_last_of(L' ');
    if (lastSpace != std::wstring::npos) {
        windSpeedOnly = dispWind.substr(0, lastSpace);
        windDirOnly = dispWind.substr(lastSpace + 1);
    }
    rightCurrent.Children().Append(MakeSmallText(L"Wind: " + windSpeedOnly, L"Wind speed"));
    if (!windDirOnly.empty()) {
        TextBlock dirTb = MakeSmallText(L"Direction: " + windDirOnly, L"Wind direction");
        dirTb.HorizontalAlignment(HorizontalAlignment::Right);
        dirTb.Margin(Thickness{ 0, -1, 0, 0 });
        rightCurrent.Children().Append(dirTb);
    }
    if (!currentWeather.sunset.empty()) {
        std::wstring sunTip = L"Sunset: " + currentWeather.sunset;
        if (!currentWeather.sunrise.empty()) {
            sunTip += L"  |  Sunrise: " + currentWeather.sunrise;
        }
        rightCurrent.Children().Append(MakeSmallText(L"Sunset: " + currentWeather.sunset, sunTip));
    }
    Grid::SetColumn(rightCurrent, 1);
    headerGrid.Children().Append(rightCurrent);
    mainStack.Children().Append(headerGrid);
    // --- Weather Temp & Icon details (now in leftCurrent horizontal layout)
    StackPanel leftCurrent;
    leftCurrent.Orientation(Orientation::Vertical);
    leftCurrent.Spacing(1);
    leftCurrent.Margin(Thickness{ 4, -54, 0, -12 }); // Move up by ~8px (from -46 to -54)
    // A row for the Icon and the Temperature side-by-side
    StackPanel topWeatherRow;
    topWeatherRow.Orientation(Orientation::Horizontal);
    topWeatherRow.Spacing(10);
    topWeatherRow.VerticalAlignment(VerticalAlignment::Center);
    Grid heroBox;
    heroBox.Width(56);
    heroBox.Height(56);
    heroBox.VerticalAlignment(VerticalAlignment::Center);
    heroBox.HorizontalAlignment(HorizontalAlignment::Center);
    heroBox.Margin(Thickness{ 0, -5, 0, 0 });
    CompositeTransform heroTrans;
    heroTrans.ScaleX(1.0 / 1.08);
    heroTrans.ScaleY(1.0 / 1.08);
    heroBox.RenderTransform(heroTrans);
    heroBox.RenderTransformOrigin(winrt::Windows::Foundation::Point{ 0.5, 0.5 });
    FontIcon heroFb{ nullptr };
    Image heroImg{ nullptr };
    FontIcon heroIcon{ nullptr };
    TextBlock heroEmoji{ nullptr };
    if (g_weatherStyle == 2 || g_weatherStyle == 3 || g_weatherStyle == 4) {
        heroFb = FontIcon{};
        heroFb.FontFamily(winrt::Windows::UI::Xaml::Media::FontFamily(L"Segoe UI Symbol"));
        heroFb.Glyph(currentIcon);
        heroFb.FontSize(44.0 * 1.08);
        heroFb.Foreground(SolidColorBrush{ GetXamlIconColor(condition) });
        heroFb.VerticalAlignment(VerticalAlignment::Center);
        heroFb.HorizontalAlignment(HorizontalAlignment::Center);
        heroBox.Children().Append(heroFb);
        heroImg = Image{};
        heroImg.Width(44.0 * 1.08);
        heroImg.Height(44.0 * 1.08);
        heroImg.VerticalAlignment(VerticalAlignment::Center);
        heroImg.HorizontalAlignment(HorizontalAlignment::Center);
        heroImg.Stretch(winrt::Windows::UI::Xaml::Media::Stretch::Uniform);
        std::wstring heroUri = !customIconUrl.empty() ? customIconUrl : GetWeatherIconPathOrUrl(currentWeather.weatherCode, currentWeather.isDay != 0);
        SetImageOrFallback(heroImg, heroFb, heroUri);
        heroBox.Children().Append(heroImg);
        topWeatherRow.Children().Append(heroBox);
    }
    else if (g_weatherStyle == 1) {
        heroIcon = FontIcon{};
        heroIcon.FontFamily(winrt::Windows::UI::Xaml::Media::FontFamily(L"Segoe UI Symbol"));
        heroIcon.Glyph(currentIcon);
        heroIcon.FontSize(44.0 * 1.08);
        heroIcon.Foreground(SolidColorBrush{ GetXamlIconColor(condition) });
        heroIcon.VerticalAlignment(VerticalAlignment::Center);
        heroIcon.HorizontalAlignment(HorizontalAlignment::Center);
        heroBox.Children().Append(heroIcon);
        topWeatherRow.Children().Append(heroBox);
    }
    else {
        heroEmoji = TextBlock{};
        heroEmoji.Text(currentIcon);
        heroEmoji.FontSize(44.0 * 1.08);
        heroEmoji.VerticalAlignment(VerticalAlignment::Center);
        heroEmoji.HorizontalAlignment(HorizontalAlignment::Center);
        heroBox.Children().Append(heroEmoji);
        topWeatherRow.Children().Append(heroBox);
    }
    if (g_animateIcons) {
        Storyboard heroSb;
        heroSb.RepeatBehavior(winrt::Windows::UI::Xaml::Media::Animation::RepeatBehaviorHelper::Forever());
        heroSb.AutoReverse(true);
        SineEase ease;
        ease.EasingMode(EasingMode::EaseInOut);
        if (heroTrans) {
            DoubleAnimation sXAnim;
            sXAnim.EnableDependentAnimation(true);
            Storyboard::SetTarget(sXAnim, heroTrans);
            Storyboard::SetTargetProperty(sXAnim, L"ScaleX");
            sXAnim.From(0.96 / 1.08);
            sXAnim.To(1.0);
            sXAnim.Duration(winrt::Windows::UI::Xaml::DurationHelper::FromTimeSpan(std::chrono::milliseconds(1800)));
            sXAnim.EasingFunction(ease);
            heroSb.Children().Append(sXAnim);
            DoubleAnimation sYAnim;
            sYAnim.EnableDependentAnimation(true);
            Storyboard::SetTarget(sYAnim, heroTrans);
            Storyboard::SetTargetProperty(sYAnim, L"ScaleY");
            sYAnim.From(0.96 / 1.08);
            sYAnim.To(1.0);
            sYAnim.Duration(winrt::Windows::UI::Xaml::DurationHelper::FromTimeSpan(std::chrono::milliseconds(1800)));
            sYAnim.EasingFunction(ease);
            heroSb.Children().Append(sYAnim);
        }
        topWeatherRow.Loaded([heroSb](auto const&, auto const&) {
            try {
                heroSb.Begin();
            } catch (...) {}
        });
        try {
            heroSb.Begin();
        } catch (...) {}
    }
    TextBlock bigTemp;
    std::wstring displayTemp = currentTemp;
    if (!displayTemp.empty() && displayTemp.back() == L'°') {
        displayTemp += (g_useCelsius ? L"C" : L"F");
    }
    bigTemp.Text(FormatTempDisplay(displayTemp));
    bigTemp.FontSize(30);
    bigTemp.FontWeight(winrt::Windows::UI::Text::FontWeights::Bold());
    bigTemp.Foreground(SolidColorBrush{ primaryColor });
    bigTemp.VerticalAlignment(VerticalAlignment::Center);
    bigTemp.Margin(Thickness{ 0, -2, 0, 0 }); // align centered with heroBox
    topWeatherRow.Children().Append(bigTemp);
    std::wstring topTip = condition + L" (" + FormatTempDisplay(displayTemp) + L")";
    if (!g_selectedDate.empty()) {
        topTip += L"\nSelected Date: " + g_selectedDate;
    }
    if (!currentWeather.sunrise.empty() && !currentWeather.sunset.empty()) {
        topTip += L"\nSunrise: " + currentWeather.sunrise + L"  |  Sunset: " + currentWeather.sunset;
    }
    ToolTipService::SetToolTip(topWeatherRow, winrt::box_value(topTip));
    leftCurrent.Children().Append(topWeatherRow);
    try {
        bigTemp.Transitions(winrt::Windows::UI::Xaml::Media::Animation::TransitionCollection{});
        topWeatherRow.Transitions(winrt::Windows::UI::Xaml::Media::Animation::TransitionCollection{});
        leftCurrent.Transitions(winrt::Windows::UI::Xaml::Media::Animation::TransitionCollection{});
    }
    catch (...) {}
    mainStack.Children().Append(leftCurrent);
    auto MakeTabBtn = [rootGrid, condition, currentIcon, currentTemp,
        primaryColor, tertiaryColor, activeTabColor, animate](
            std::wstring text, int tabIndex, bool isSelected) {
                Grid tabGrid;
                tabGrid.Padding(Thickness{ 0, 0, 0, 0 });
                tabGrid.Background(SolidColorBrush{ winrt::Windows::UI::Colors::Transparent() });
                try {
                    if (animate) {
                        winrt::Windows::UI::Xaml::Media::Animation::TransitionCollection tabGridTransitions;
                        winrt::Windows::UI::Xaml::Media::Animation::AddDeleteThemeTransition addDelete;
                        tabGridTransitions.Append(addDelete);
                        tabGrid.ChildrenTransitions(tabGridTransitions);
                    }
                }
                catch (...) {
                }
                TextBlock tb;
                tb.Text(text);
                tb.FontSize(13);
                tb.Margin(Thickness{ 0, 0, 0, 8.0 });
                tb.FontWeight(isSelected
                    ? winrt::Windows::UI::Text::FontWeights::SemiBold()
                    : winrt::Windows::UI::Text::FontWeights::Normal());
                tb.Foreground(
                    SolidColorBrush{ isSelected ? primaryColor : tertiaryColor });
                tb.HorizontalAlignment(HorizontalAlignment::Center);
                tabGrid.Children().Append(tb);
                if (isSelected) {
                    winrt::Windows::UI::Xaml::Shapes::Rectangle underline;
                    underline.Height(1.0);
                    underline.Fill(SolidColorBrush{ activeTabColor });
                    underline.VerticalAlignment(VerticalAlignment::Bottom);
                    underline.Margin(Thickness{ 0, 0, 0, 5.0 }); // Raised up from bottom
                    tabGrid.Children().Append(underline);
                }
                tabGrid.PointerEntered([tb, isSelected, activeTabColor](auto const&, auto const&) {
                    if (!isSelected) {
                        tb.Foreground(SolidColorBrush{ activeTabColor });
                    }
                    });
                tabGrid.PointerExited([tb, isSelected, tertiaryColor](auto const&, auto const&) {
                    if (!isSelected) {
                        tb.Foreground(SolidColorBrush{ tertiaryColor });
                    }
                    });
                auto weakRoot = winrt::make_weak(rootGrid);
                tabGrid.Tapped([weakRoot, tabIndex, condition, currentIcon, currentTemp](auto const&, auto const&) {
                    if (g_selectedGraphTab != tabIndex) {
                        g_selectedGraphTab = tabIndex;
                        if (tabIndex == 3) g_radarIsPlaying = true;
                        if (auto root = weakRoot.get()) {
                            PopulateForecastUI(root, condition, currentIcon,
                                currentTemp, false);
                            if (!ShouldUseXamlTaskbar()) {
                                ResizeWin10PopupToCurrentContent();
                            }
                        }
                    }
                    });
                return tabGrid;
        };
    StackPanel graphSection;
    graphSection.Orientation(Orientation::Vertical);
    graphSection.Spacing(0);
    Grid tabHeaderGrid;
    tabHeaderGrid.Margin(Thickness{ 0, 0, 0, 1 });
    winrt::Windows::UI::Xaml::Shapes::Rectangle div1;
    div1.Height(1);
    div1.Fill(SolidColorBrush{ dividerColor });
    div1.VerticalAlignment(VerticalAlignment::Bottom);
    div1.Margin(Thickness{ 0, 0, 0, 2 });
    tabHeaderGrid.Children().Append(div1);
    StackPanel tabs;
    tabs.Orientation(Orientation::Horizontal);
    tabs.Spacing(18);
    tabs.Margin(Thickness{ 8.0, 0, 0, 4.0 });
    tabs.VerticalAlignment(VerticalAlignment::Bottom);
    tabs.Children().Append(
        MakeTabBtn(L"Temperature", 0, g_selectedGraphTab == 0));
    tabs.Children().Append(
        MakeTabBtn(L"Precipitation", 1, g_selectedGraphTab == 1));
    tabs.Children().Append(MakeTabBtn(L"Wind", 2, g_selectedGraphTab == 2));
    tabs.Children().Append(MakeTabBtn(L"Radar", 3, g_selectedGraphTab == 3));
    tabHeaderGrid.Children().Append(tabs);
    graphSection.Children().Append(tabHeaderGrid);
    if (g_selectedGraphTab == 3) {
        // --- GeoMet precipitation forecast map
        if (localRadarFrames.empty() && !g_radarMetadataError && g_hForceUpdateEvent) {
            // The tracked weather worker owns radar network I/O.
            SetEvent(g_hForceUpdateEvent);
        }
        const double graphHeight = 174.0;
        const double graphOuterWidth = std::max(280.0, rootGrid.Width() - 32.0);
        double radarWidth = graphOuterWidth;
        double radarHeight = graphHeight + 141.0; // Adjusted to fill space previously used by daily cards // 208.0 px expanded radar height
        Grid radarRootGrid;
        radarRootGrid.Width(radarWidth);
        radarRootGrid.Height(radarHeight);
        radarRootGrid.CornerRadius(CornerRadius{ 6, 6, 6, 6 });
        radarRootGrid.BorderBrush(SolidColorBrush{ dividerColor });
        radarRootGrid.BorderThickness(Thickness{ 1, 1, 1, 1 });
        radarRootGrid.Background(SolidColorBrush{ isDark ? winrt::Windows::UI::ColorHelper::FromArgb(240, 18, 18, 22) : winrt::Windows::UI::ColorHelper::FromArgb(240, 235, 238, 242) });
        Canvas radarCanvas;
        radarCanvas.Width(radarWidth);
        radarCanvas.Height(radarHeight);
        winrt::Windows::UI::Xaml::Media::RectangleGeometry clipGeo;
        clipGeo.Rect(winrt::Windows::Foundation::Rect{ 0, 0, (float)radarWidth, (float)radarHeight });
        radarCanvas.Clip(clipGeo);
        Canvas baseMapCanvas;
        baseMapCanvas.Width(radarWidth);
        baseMapCanvas.Height(radarHeight);
        baseMapCanvas.Opacity((double)std::clamp(g_radarOpacity, 0, 100) / 100.0);
        struct ZoomTransitionState {
            Canvas previous{nullptr};
            int pendingBase = 0;
            bool precipitationReady = false;
            bool armed = false;
            bool finished = false;
        };
        auto zoomState = previousRadarMap
            ? std::make_shared<ZoomTransitionState>() : nullptr;
        if (zoomState) zoomState->previous = previousRadarMap;
        auto finishZoom = [zoomState]() {
            if (!zoomState || !zoomState->armed || zoomState->finished ||
                zoomState->pendingBase > 0 || !zoomState->precipitationReady) return;
            zoomState->finished = true;
            auto oldMap = zoomState->previous;
            if (!oldMap) return;
            auto releaseOld = [zoomState](auto const&, auto const&) {
                auto old = zoomState->previous;
                zoomState->previous = nullptr;
                if (!old) return;
                try {
                    if (auto parent = old.Parent().try_as<Grid>()) {
                        auto children = parent.Children();
                        for (uint32_t i = 0; i < children.Size(); ++i) {
                            if (children.GetAt(i) == old) {
                                children.RemoveAt(i);
                                break;
                            }
                        }
                    }
                    CleanupXamlMedia(old);
                } catch (...) {}
            };
            try {
                Storyboard fade;
                DoubleAnimation opacity;
                Storyboard::SetTarget(opacity, oldMap);
                Storyboard::SetTargetProperty(opacity, L"Opacity");
                opacity.From(1.0);
                opacity.To(0.0);
                opacity.Duration(winrt::Windows::UI::Xaml::DurationHelper::FromTimeSpan(
                    std::chrono::milliseconds(160)));
                fade.Children().Append(opacity);
                TrackStoryboardCompleted(fade, releaseOld);
                fade.Begin();
            } catch (...) {
                releaseOld(nullptr, nullptr);
            }
        };
        double lat = currentWeather.latitude;
        double lon = currentWeather.longitude;
        if (lat == 0.0 && lon == 0.0) {
            lat = 40.7128; lon = -74.0060;
        }
        lat = std::clamp(lat, -85.0, 85.0);
        double n = pow(2.0, (double)g_radarZoom);
        double xExact = (lon + 180.0) / 360.0 * n;
        double latRad = lat * 3.14159265358979323846 / 180.0;
        double yExact = (1.0 - asinh(tan(latRad)) / 3.14159265358979323846) / 2.0 * n;
        int tileX = (int)floor(xExact);
        int tileY = (int)floor(yExact);
        double fracX = xExact - (double)tileX;
        double fracY = yExact - (double)tileY;
        double cx = radarWidth / 2.0;
        double cy = radarHeight / 2.0;
        double originX = cx - (fracX * 256.0);
        double originY = cy - (fracY * 256.0);
        int minDx = -1, maxDx = 1;
        if (radarWidth > 380.0) { minDx = -2; maxDx = 2; }
        int minDy = -1, maxDy = 1;
        if (radarHeight > 240.0) { minDy = -2; maxDy = 2; }
        // 1. Base Map Tiles (ArcGIS World Gray Canvas cached for zoom level g_radarZoom - includes cities, borders, and roads with zero API keys required)
        for (int dy = minDy; dy <= maxDy; dy++) {
            for (int dx = minDx; dx <= maxDx; dx++) {
                int tx = tileX + dx;
                int ty = tileY + dy;
                if (ty < 0 || ty >= static_cast<int>(n)) continue;
                int wrappedX = (tx % static_cast<int>(n) + static_cast<int>(n)) % static_cast<int>(n);
                double posX = originX + (dx * 256.0);
                double posY = originY + (dy * 256.0);
                if (posX >= radarWidth || posY >= radarHeight ||
                    posX + 256.0 <= 0 || posY + 256.0 <= 0) continue;
                winrt::Windows::UI::Xaml::Controls::Image baseImg;
                auto settleBase = [zoomState, finishZoom,
                                   settled = std::make_shared<bool>(false)]() {
                    if (!zoomState || *settled || g_modUnloaded) return;
                    *settled = true;
                    --zoomState->pendingBase;
                    finishZoom();
                };
                if (zoomState) {
                    ++zoomState->pendingBase;
                    baseImg.ImageOpened([settleBase](auto const&, auto const&) { settleBase(); });
                    baseImg.ImageFailed([settleBase](auto const&, auto const&) { settleBase(); });
                }
                bool useDarkMap = (currentWeather.isDay == 0) || (currentWeather.isDay == -1 && isDark);
                std::wstring baseMapUrl = useDarkMap
                    ? (std::wstring(L"https://server.arcgisonline.com/ArcGIS/rest/services/Canvas/World_Dark_Gray_Base/MapServer/tile/") + std::to_wstring(g_radarZoom) + L"/" + std::to_wstring(ty) + L"/" + std::to_wstring(wrappedX))
                    : (std::wstring(L"https://server.arcgisonline.com/ArcGIS/rest/services/World_Street_Map/MapServer/tile/") + std::to_wstring(g_radarZoom) + L"/" + std::to_wstring(ty) + L"/" + std::to_wstring(wrappedX));
                try {
                    winrt::Windows::UI::Xaml::Media::Imaging::BitmapImage baseBmp = nullptr;
                    auto it = g_radarTileBitmapCache.find(baseMapUrl);
                    if (it != g_radarTileBitmapCache.end()) {
                        baseBmp = it->second;
                    } else {
                        baseBmp = winrt::Windows::UI::Xaml::Media::Imaging::BitmapImage{ winrt::Windows::Foundation::Uri{ baseMapUrl } };
                        g_radarTileBitmapCache[baseMapUrl] = baseBmp;
                    }
                    baseImg.Source(baseBmp);
                } catch (...) { settleBase(); }
                baseImg.Width(256.0);
                baseImg.Height(256.0);
                Canvas::SetLeft(baseImg, posX);
                Canvas::SetTop(baseImg, posY);
                baseMapCanvas.Children().Append(baseImg);
            }
        }
        radarCanvas.Children().Append(baseMapCanvas);
        // Request one server-rendered WMS image per selected frame and viewport.
        // At the date line, split the viewport into two valid Web Mercator bounds.
        constexpr double halfWorld = 20037508.342789244;
        constexpr double worldWidth = halfWorld * 2.0;
        const double metersPerTile = worldWidth / n;
        const double xLeft = (xExact - radarWidth / 512.0) * metersPerTile - halfWorld;
        const double yTop = halfWorld - (yExact - radarHeight / 512.0) * metersPerTile;
        const double yBottom = yTop - radarHeight / 256.0 * metersPerTile;
        struct ForecastSegment { double left; double width; double minX; double maxX; };
        std::vector<ForecastSegment> segments;
        double consumed = 0.0;
        while (consumed < radarWidth - 0.01 && segments.size() < 2) {
            double x = xLeft + consumed / 256.0 * metersPerTile;
            double wrapped = x - worldWidth * std::floor((x + halfWorld) / worldWidth);
            double width = std::min(radarWidth - consumed,
                (halfWorld - wrapped) * 256.0 / metersPerTile);
            if (width < 0.01) break;
            segments.push_back({consumed, width, wrapped,
                wrapped + width / 256.0 * metersPerTile});
            consumed += width;
        }
        Canvas precipitationCanvas;
        precipitationCanvas.Width(radarWidth);
        precipitationCanvas.Height(radarHeight);
        precipitationCanvas.Opacity((double)std::clamp(g_precipitationOpacity, 0, 100) / 100.0);
        auto frameImages = std::make_shared<std::vector<std::vector<Image>>>();
        auto frameLoaded = std::make_shared<std::vector<std::vector<bool>>>();
        auto frameFailed = std::make_shared<std::vector<bool>>();
        auto weakFrames = std::make_shared<std::vector<winrt::weak_ref<Canvas>>>();
        int activeFrameIdx = (g_radarCurrentFrame.load() >= 0 && g_radarCurrentFrame.load() < (int)localRadarFrames.size()) ? g_radarCurrentFrame.load() : 0;
        if (activeFrameIdx < 0) activeFrameIdx = 0;
        g_radarCurrentFrame = activeFrameIdx;
        for (size_t fi = 0; fi < localRadarFrames.size(); fi++) {
            Canvas frameC;
            frameC.Width(radarWidth);
            frameC.Height(radarHeight);
            // Keep the Image in the visual tree while it downloads; a collapsed
            // parent can delay ImageOpened until after the frame is selected.
            frameC.Opacity(0.0);
            std::vector<Image> images;
            for (const auto& segment : segments) {
                Image img;
                img.Width(segment.width);
                img.Height(radarHeight);
                img.Stretch(Stretch::Fill);
                Canvas::SetLeft(img, segment.left);
                Canvas::SetTop(img, 0.0);
                frameC.Children().Append(img);
                images.push_back(img);
            }
            precipitationCanvas.Children().Append(frameC);
            weakFrames->push_back(winrt::make_weak(frameC));
            frameImages->push_back(std::move(images));
            frameLoaded->emplace_back(segments.size(), false);
            frameFailed->push_back(false);
        }
        radarCanvas.Children().Append(precipitationCanvas);
        // Center Location Pin (Ripple ring + Dot)
        winrt::Windows::UI::Xaml::Shapes::Ellipse pulseRing;
        pulseRing.Width(18);
        pulseRing.Height(18);
        pulseRing.Stroke(SolidColorBrush{ winrt::Windows::UI::ColorHelper::FromArgb(220, 0, 174, 255) });
        pulseRing.StrokeThickness(1.5);
        Canvas::SetLeft(pulseRing, cx - 9.0);
        Canvas::SetTop(pulseRing, cy - 9.0);
        radarCanvas.Children().Append(pulseRing);
        winrt::Windows::UI::Xaml::Shapes::Ellipse pinDot;
        pinDot.Width(7);
        pinDot.Height(7);
        pinDot.Fill(SolidColorBrush{ winrt::Windows::UI::ColorHelper::FromArgb(255, 0, 174, 255) });
        pinDot.Stroke(SolidColorBrush{ winrt::Windows::UI::Colors::White() });
        pinDot.StrokeThickness(1.2);
        Canvas::SetLeft(pinDot, cx - 3.5);
        Canvas::SetTop(pinDot, cy - 3.5);
        radarCanvas.Children().Append(pinDot);
        radarRootGrid.Children().Append(radarCanvas);
        {
            std::lock_guard<std::mutex> lock(s_activeRadarMapsMutex);
            s_activeRadarMaps[radarOwner] = winrt::make_weak(radarCanvas);
        }
        if (zoomState) {
            ScaleTransform scaledOld;
            scaledOld.CenterX(cx);
            scaledOld.CenterY(cy);
            scaledOld.ScaleX(zoomDelta > 0 ? 2.0 : 0.5);
            scaledOld.ScaleY(zoomDelta > 0 ? 2.0 : 0.5);
            previousRadarMap.RenderTransform(scaledOld);
            previousRadarMap.IsHitTestVisible(false);
            radarRootGrid.Children().Append(previousRadarMap);
            zoomState->armed = true;
            winrt::Windows::UI::Xaml::DispatcherTimer timeout;
            timeout.Interval(std::chrono::milliseconds(2500));
            TrackOneShotTimer(timeout, [zoomState, finishZoom](auto const&, auto const&) {
                zoomState->pendingBase = 0;
                zoomState->precipitationReady = true;
                finishZoom();
            });
            timeout.Start();
            finishZoom();
        }
        // Overlay HUD Grid
        Grid hudGrid;
        hudGrid.Width(radarWidth);
        hudGrid.Height(radarHeight);
        // Loading precipitation overlay if frames not yet acquired
        if (localRadarFrames.empty()) {
            StackPanel loadingRadarPanel;
            loadingRadarPanel.HorizontalAlignment(HorizontalAlignment::Center);
            loadingRadarPanel.VerticalAlignment(VerticalAlignment::Center);
            loadingRadarPanel.Spacing(6);
            Border loadingBox;
            loadingBox.CornerRadius(CornerRadius{ 6, 6, 6, 6 });
            loadingBox.Background(SolidColorBrush{ winrt::Windows::UI::ColorHelper::FromArgb(220, 20, 20, 26) });
            loadingBox.BorderBrush(SolidColorBrush{ winrt::Windows::UI::ColorHelper::FromArgb(80, 255, 255, 255) });
            loadingBox.BorderThickness(Thickness{ 1, 1, 1, 1 });
            loadingBox.Padding(Thickness{ 12, 8, 12, 8 });
            StackPanel boxContent;
            boxContent.Orientation(Orientation::Horizontal);
            boxContent.Spacing(8);
            boxContent.VerticalAlignment(VerticalAlignment::Center);
            ProgressRing radarRing;
            radarRing.IsActive(true);
            radarRing.Width(16);
            radarRing.Height(16);
            radarRing.Foreground(SolidColorBrush{ winrt::Windows::UI::Colors::White() });
            radarRing.VerticalAlignment(VerticalAlignment::Center);
            TextBlock radarLoadingTb;
            radarLoadingTb.Text(g_radarMetadataError
                ? L"Forecast unavailable; retry on next update"
                : L"Loading precipitation forecast...");
            radarRing.IsActive(!g_radarMetadataError);
            radarLoadingTb.FontSize(11.0);
            radarLoadingTb.Foreground(SolidColorBrush{ winrt::Windows::UI::Colors::White() });
            radarLoadingTb.VerticalAlignment(VerticalAlignment::Center);
            boxContent.Children().Append(radarRing);
            boxContent.Children().Append(radarLoadingTb);
            loadingBox.Child(boxContent);
            loadingRadarPanel.Children().Append(loadingBox);
            hudGrid.Children().Append(loadingRadarPanel);
        }
        // Keep the short Radar tab label, but identify these as model forecasts.
        StackPanel badgePanel;
        badgePanel.Orientation(Orientation::Horizontal);
        badgePanel.Spacing(4);
        badgePanel.HorizontalAlignment(HorizontalAlignment::Left);
        badgePanel.VerticalAlignment(VerticalAlignment::Top);
        badgePanel.Margin(Thickness{ 6, 6, 0, 0 });
        Border liveBadge;
        liveBadge.CornerRadius(CornerRadius{ 4, 4, 4, 4 });
        liveBadge.Background(SolidColorBrush{ winrt::Windows::UI::ColorHelper::FromArgb(200, 15, 15, 20) });
        liveBadge.BorderBrush(SolidColorBrush{ winrt::Windows::UI::ColorHelper::FromArgb(80, 255, 255, 255) });
        liveBadge.BorderThickness(Thickness{ 1, 1, 1, 1 });
        liveBadge.Padding(Thickness{ 6, 2, 6, 2 });
        StackPanel liveBadgeContent;
        liveBadgeContent.Orientation(Orientation::Horizontal);
        liveBadgeContent.Spacing(4);
        liveBadgeContent.VerticalAlignment(VerticalAlignment::Center);
        winrt::Windows::UI::Xaml::Shapes::Ellipse liveDot;
        liveDot.Width(6);
        liveDot.Height(6);
        liveDot.Fill(SolidColorBrush{ winrt::Windows::UI::ColorHelper::FromArgb(255, 65, 170, 255) });
        liveDot.VerticalAlignment(VerticalAlignment::Center);
        liveBadgeContent.Children().Append(liveDot);
        TextBlock liveTb;
        liveTb.Text(L"FORECAST · ECCC");
        liveTb.FontSize(9.5);
        liveTb.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());
        liveTb.Foreground(SolidColorBrush{ winrt::Windows::UI::Colors::White() });
        liveTb.VerticalAlignment(VerticalAlignment::Center);
        liveBadgeContent.Children().Append(liveTb);
        liveBadge.Child(liveBadgeContent);
        ToolTipService::SetToolTip(liveBadge,
            winrt::box_value(L"Precipitation forecast: Environment and Climate Change Canada (GeoMet)"));
        badgePanel.Children().Append(liveBadge);
        Border timeBadge;
        timeBadge.CornerRadius(CornerRadius{ 4, 4, 4, 4 });
        timeBadge.Background(SolidColorBrush{ winrt::Windows::UI::ColorHelper::FromArgb(190, 15, 15, 20) });
        timeBadge.BorderBrush(SolidColorBrush{ winrt::Windows::UI::ColorHelper::FromArgb(60, 255, 255, 255) });
        timeBadge.BorderThickness(Thickness{ 1, 1, 1, 1 });
        timeBadge.Padding(Thickness{ 6, 2, 6, 2 });
        timeBadge.Width(96);
        TextBlock frameTimeTb;
        frameTimeTb.FontSize(9.5);
        frameTimeTb.Foreground(SolidColorBrush{ winrt::Windows::UI::Colors::White() });
        frameTimeTb.VerticalAlignment(VerticalAlignment::Center);
        frameTimeTb.HorizontalAlignment(HorizontalAlignment::Center);
        if (!localRadarFrames.empty()) {
            frameTimeTb.Text(L"Loading...");
        }
        timeBadge.Child(frameTimeTb);
        if (localRadarFrames.size() <= 1) {
            badgePanel.Children().Append(timeBadge);
        }
        hudGrid.Children().Append(badgePanel);
        // Top-right zoom controls. Playback belongs beside the scrubber below.
        StackPanel ctrlPanel;
        ctrlPanel.Orientation(Orientation::Horizontal);
        ctrlPanel.Spacing(4);
        ctrlPanel.HorizontalAlignment(HorizontalAlignment::Right);
        ctrlPanel.VerticalAlignment(VerticalAlignment::Top);
        ctrlPanel.Margin(Thickness{ 0, 6, 6, 0 });
        auto makeCtrlBtn = [](const std::wstring& iconText, double fontSize = 11.0) {
            Border btn;
            btn.CornerRadius(CornerRadius{ 4, 4, 4, 4 });
            btn.Background(SolidColorBrush{ winrt::Windows::UI::ColorHelper::FromArgb(200, 20, 20, 25) });
            btn.BorderBrush(SolidColorBrush{ winrt::Windows::UI::ColorHelper::FromArgb(80, 255, 255, 255) });
            btn.BorderThickness(Thickness{ 1, 1, 1, 1 });
            btn.Width(26);
            btn.Height(24);
            TextBlock tb;
            tb.Text(iconText);
            tb.FontSize(fontSize);
            tb.Foreground(SolidColorBrush{ winrt::Windows::UI::Colors::White() });
            tb.HorizontalAlignment(HorizontalAlignment::Center);
            tb.VerticalAlignment(VerticalAlignment::Center);
            btn.Child(tb);
            btn.PointerEntered([](auto const& sender, auto const&) {
                if (auto b = sender.template try_as<winrt::Windows::UI::Xaml::Controls::Border>()) {
                    b.Background(SolidColorBrush{ winrt::Windows::UI::ColorHelper::FromArgb(230, 0, 120, 215) });
                }
            });
            btn.PointerExited([](auto const& sender, auto const&) {
                if (auto b = sender.template try_as<winrt::Windows::UI::Xaml::Controls::Border>()) {
                    b.Background(SolidColorBrush{ winrt::Windows::UI::ColorHelper::FromArgb(200, 20, 20, 25) });
                }
            });
            return btn;
        };
        auto weakRoot = winrt::make_weak(rootGrid);
        // Zoom out [-]
        auto zoomOutBtn = makeCtrlBtn(L"−", 13.0);
        zoomOutBtn.Tapped([weakRoot, condition, currentIcon, currentTemp](auto const&, auto const&) {
            if (g_radarZoom > 5) {
                g_radarZoom--;
                if (auto root = weakRoot.get()) {
                    PopulateForecastUI(root, condition, currentIcon, currentTemp, false,
                                       true, L"", L"", L"", L"", -1);
                }
            }
        });
        ctrlPanel.Children().Append(zoomOutBtn);
        // Zoom in [+]
        auto zoomInBtn = makeCtrlBtn(L"+", 12.0);
        zoomInBtn.Tapped([weakRoot, condition, currentIcon, currentTemp](auto const&, auto const&) {
            if (g_radarZoom < 12) {
                g_radarZoom++;
                if (auto root = weakRoot.get()) {
                    PopulateForecastUI(root, condition, currentIcon, currentTemp, false,
                                       true, L"", L"", L"", L"", 1);
                }
            }
        });
        ctrlPanel.Children().Append(zoomInBtn);
        // Play / Pause button and timer definition
        s_radarFrameTimer = winrt::Windows::UI::Xaml::DispatcherTimer{};
        auto activeTimer = s_radarFrameTimer;
        auto playBtn = makeCtrlBtn(g_radarIsPlaying ? L"⏸" : L"▶", 9.0);
        playBtn.Tapped([activeTimer](auto const& sender, auto const&) {
            g_radarIsPlaying = !g_radarIsPlaying;
            if (auto p = sender.template try_as<winrt::Windows::UI::Xaml::Controls::Border>()) {
                if (auto tb = p.Child().template try_as<winrt::Windows::UI::Xaml::Controls::TextBlock>()) {
                    tb.Text(g_radarIsPlaying ? L"⏸" : L"▶");
                }
            }
            if (activeTimer) {
                if (g_radarIsPlaying) {
                    activeTimer.Interval(std::chrono::milliseconds(RadarPlaybackIntervalMs()));
                    activeTimer.Start();
                } else {
                    activeTimer.Stop();
                }
            }
        });
        auto speedBtn = makeCtrlBtn(L"1x", 8.5);
        if (auto tb = speedBtn.Child().try_as<winrt::Windows::UI::Xaml::Controls::TextBlock>()) {
            if (g_radarPlaySpeedIdx == 0) tb.Text(L"0.5x");
            else if (g_radarPlaySpeedIdx == 1) tb.Text(L"1x");
            else if (g_radarPlaySpeedIdx == 2) tb.Text(L"1.5x");
            else if (g_radarPlaySpeedIdx == 3) tb.Text(L"2x");
        }
        speedBtn.Width(30);
        speedBtn.Tapped([activeTimer](auto const& sender, auto const&) {
            g_radarPlaySpeedIdx = (g_radarPlaySpeedIdx + 1) % 4;
            if (activeTimer) activeTimer.Interval(std::chrono::milliseconds(RadarPlaybackIntervalMs()));
            if (auto p = sender.template try_as<winrt::Windows::UI::Xaml::Controls::Border>()) {
                if (auto tb = p.Child().template try_as<winrt::Windows::UI::Xaml::Controls::TextBlock>()) {
                    if (g_radarPlaySpeedIdx == 0) tb.Text(L"0.5x");
                    else if (g_radarPlaySpeedIdx == 1) tb.Text(L"1x");
                    else if (g_radarPlaySpeedIdx == 2) tb.Text(L"1.5x");
                    else tb.Text(L"2x");
                }
            }
        });
        ToolTipService::SetToolTip(speedBtn, winrt::box_value(L"Playback Speed"));
        hudGrid.Children().Append(ctrlPanel);
        // Radar timeline: the slider and playback timer share one frame switch.
        Slider frameSlider;
        auto weakFrameSlider = winrt::make_weak(frameSlider);
        auto weakActiveTimer = winrt::make_weak(activeTimer);
        auto weakPlayBtn = winrt::make_weak(playBtn);
        auto updatingSliderFromPlayback = std::make_shared<bool>(false);
        auto showFrame = [weakFrames, frameImages, frameLoaded, frameFailed, segments,
                          yBottom, yTop, radarHeight, weakFrameSlider,
                          updatingSliderFromPlayback, frameTimeTb,
                          localRadarFrames, weakActiveTimer, weakPlayBtn,
                          zoomState, finishZoom](int targetIdx) mutable {
            if (localRadarFrames.empty() || weakFrames->empty()) return;
            int totalFrames = (int)localRadarFrames.size();
            g_radarCurrentFrame = (targetIdx >= 0 && targetIdx < totalFrames) ? targetIdx : 0;
            int newIdx = g_radarCurrentFrame.load();
            const auto showLoaded = [weakFrames, frameLoaded, frameFailed,
                                     zoomState, finishZoom](int index) {
                if (g_modUnloaded || index != g_radarCurrentFrame.load() ||
                    index < 0 || index >= static_cast<int>(weakFrames->size()) ||
                    (*frameFailed)[index]) return;
                for (bool ready : (*frameLoaded)[index]) if (!ready) return;
                for (size_t i = 0; i < weakFrames->size(); ++i) {
                    if (auto canvas = (*weakFrames)[i].get()) {
                        canvas.Opacity(i == static_cast<size_t>(index) ? 1.0 : 0.0);
                    }
                }
                if (zoomState) {
                    zoomState->precipitationReady = true;
                    finishZoom();
                }
            };
            time_t tt = static_cast<time_t>(localRadarFrames[newIdx].time);
            struct tm localTm{};
            wchar_t timeBuf[32]{};
            if (localtime_s(&localTm, &tt) == 0 &&
                wcsftime(timeBuf, 32, L"%a %I:%M %p", &localTm)) {
                frameTimeTb.Text(timeBuf);
                if (auto slider = weakFrameSlider.get())
                    ToolTipService::SetToolTip(slider,
                        winrt::box_value(winrt::hstring{timeBuf}));
            }
            const auto showFailure = [weakFrames, frameTimeTb, weakActiveTimer,
                weakPlayBtn, frameFailed](int index) {
                (*frameFailed)[index] = true;
                if (g_modUnloaded || !(*weakFrames)[index].get() ||
                    index != g_radarCurrentFrame.load()) return;
                g_radarIsPlaying = false;
                if (auto timer = weakActiveTimer.get()) timer.Stop();
                if (auto button = weakPlayBtn.get()) {
                    if (auto glyph = button.Child().try_as<TextBlock>()) glyph.Text(L"▶");
                }
                for (auto const& frame : *weakFrames) {
                    if (auto canvas = frame.get()) canvas.Opacity(0.0);
                }
                frameTimeTb.Text(L"Unavailable");
            };
            if ((*frameFailed)[newIdx]) {
                showFailure(newIdx);
                if (auto slider = weakFrameSlider.get()) {
                    *updatingSliderFromPlayback = true;
                    slider.Value(static_cast<double>(newIdx));
                    *updatingSliderFromPlayback = false;
                }
                return;
            }
            // A visible frame ensures that XAML starts loading its image even
            // on hosts which defer decoding images under a transparent parent.
            for (size_t i = 0; i < weakFrames->size(); ++i) {
                if (auto canvas = (*weakFrames)[i].get())
                    canvas.Opacity(i == static_cast<size_t>(newIdx) ? 1.0 : 0.0);
            }
            auto loadFrameImages = [weakFrames, frameImages, frameLoaded, frameFailed,
                                    segments, yBottom, yTop, radarHeight,
                                    localRadarFrames, showLoaded, showFailure](int index) {
                if (index < 0 || index >= static_cast<int>(frameImages->size()) ||
                    (*frameFailed)[index] ||
                    (*frameImages)[index].size() != segments.size()) return;
                for (size_t i = 0; i < segments.size(); ++i) {
                    auto img = (*frameImages)[index][i];
                    if (img.Source()) continue;
                    const auto& segment = segments[i];
                    wchar_t bounds[160]{};
                    swprintf(bounds, 160, L"%.2f,%.2f,%.2f,%.2f",
                        segment.minX, yBottom, segment.maxX, yTop);
                    std::wstring uri = L"https://geo.weather.gc.ca/geomet?SERVICE=WMS&VERSION=1.3.0&REQUEST=GetMap"
                        L"&LAYERS=GDPS_15km_Precip-Accum1h&STYLES=PRECIPMM-LINEAR&CRS=EPSG:3857"
                        L"&FORMAT=image%2Fpng&TRANSPARENT=TRUE&INTERPOLATION=TRUE&BBOX=";
                    uri += bounds;
                    // Render the same geographic extent at twice the pixel density.
                    // XAML scales the provider image to the logical map dimensions,
                    // reducing its hard one-pixel edges and preserving detail at 200% DPI.
                    uri += L"&WIDTH=" + std::to_wstring(static_cast<int>(std::ceil(segment.width * 2.0)));
                    uri += L"&HEIGHT=" + std::to_wstring(static_cast<int>(std::ceil(radarHeight * 2.0)));
                    uri += L"&TIME=" + localRadarFrames[index].isoTime;
                    if (!localRadarFrames[index].referenceTime.empty())
                        uri += L"&DIM_REFERENCE_TIME=" + localRadarFrames[index].referenceTime;

                    img.ImageOpened([weakFrames, frameLoaded, showLoaded,
                            index, i](auto const&, auto const&) {
                        if (g_modUnloaded || !(*weakFrames)[index].get()) return;
                        if (g_debugLogs) Wh_Log(L"[EP_WeatherHost] GeoMet image loaded: frame %d, segment %u",
                            index, static_cast<unsigned>(i));
                        (*frameLoaded)[index][i] = true;
                        showLoaded(index);
                    });
                    img.ImageFailed([showFailure, index, uri](auto const&, auto const&) {
                        if (g_debugLogs) Wh_Log(L"[EP_WeatherHost] GeoMet image failed: %s", uri.c_str());
                        showFailure(index);
                    });
                    try {
                        auto it = g_radarTileBitmapCache.find(uri);
                        winrt::Windows::UI::Xaml::Media::Imaging::BitmapImage bmp = nullptr;
                        if (it == g_radarTileBitmapCache.end()) {
                            bmp = winrt::Windows::UI::Xaml::Media::Imaging::BitmapImage{
                                winrt::Windows::Foundation::Uri{uri}};
                            g_radarTileBitmapCache.emplace(uri, bmp);
                        } else {
                            bmp = it->second;
                        }
                        img.Source(bmp);
                    } catch (...) {
                        if (g_debugLogs) Wh_Log(L"[EP_WeatherHost] GeoMet image URL rejected: %s", uri.c_str());
                        showFailure(index);
                    }
                }
            };
            loadFrameImages(newIdx);
            // Keep only the next hour warm during playback. A tiny opacity
            // allows XAML to decode it without visibly changing the map.
            if (g_radarIsPlaying && localRadarFrames.size() > 1) {
                int nextIdx = (newIdx + 1) % static_cast<int>(localRadarFrames.size());
                if (auto next = (*weakFrames)[nextIdx].get()) next.Opacity(0.001);
                loadFrameImages(nextIdx);
            }
            showLoaded(newIdx);
            if (auto slider = weakFrameSlider.get()) {
                *updatingSliderFromPlayback = true;
                slider.Value(static_cast<double>(newIdx));
                *updatingSliderFromPlayback = false;
            }
        };

        if (localRadarFrames.size() > 1) {
            Border timePill;
            timePill.CornerRadius(CornerRadius{ 10, 10, 10, 10 });
            timePill.Background(SolidColorBrush{ winrt::Windows::UI::ColorHelper::FromArgb(190, 15, 15, 20) });
            timePill.BorderBrush(SolidColorBrush{ winrt::Windows::UI::ColorHelper::FromArgb(70, 255, 255, 255) });
            timePill.BorderThickness(Thickness{ 1, 1, 1, 1 });
            timePill.Padding(Thickness{ 6, 3, 6, 3 });
            timePill.HorizontalAlignment(HorizontalAlignment::Center);
            timePill.VerticalAlignment(VerticalAlignment::Bottom);
            timePill.Margin(Thickness{ 0, 0, 0, 8 });
            timePill.Width(radarWidth - 12.0);

            Grid timeline;
            for (double width : {26.0, 34.0, -1.0, 104.0}) {
                ColumnDefinition column;
                column.Width(width < 0.0 ? GridLength{1.0, GridUnitType::Star}
                                         : GridLength{width, GridUnitType::Pixel});
                timeline.ColumnDefinitions().Append(column);
            }
            timeline.VerticalAlignment(VerticalAlignment::Center);
            Grid::SetColumn(playBtn, 0);
            Grid::SetColumn(speedBtn, 1);
            speedBtn.Margin(Thickness{4, 0, 0, 0});
            timeline.Children().Append(playBtn);
            timeline.Children().Append(speedBtn);

            frameSlider.Minimum(0.0);
            frameSlider.Maximum(static_cast<double>(localRadarFrames.size() - 1));
            frameSlider.StepFrequency(1.0);
            frameSlider.TickFrequency(1.0);
            frameSlider.IsThumbToolTipEnabled(false);
            Grid::SetColumn(frameSlider, 2);
            frameSlider.HorizontalAlignment(HorizontalAlignment::Stretch);
            frameSlider.Margin(Thickness{4, 0, 6, 0});
            frameSlider.Height(28);
            frameSlider.VerticalAlignment(VerticalAlignment::Center);
            int currentF = (g_radarCurrentFrame.load() >= 0 && g_radarCurrentFrame.load() < (int)localRadarFrames.size()) ? g_radarCurrentFrame.load() : ((int)localRadarFrames.size() - 1);
            frameSlider.Value(static_cast<double>(currentF));
            ToolTipService::SetToolTip(frameSlider, winrt::box_value(L"Drag to scrub forecast hours"));
            frameSlider.ValueChanged([showFrame, updatingSliderFromPlayback,
                                      activeTimer, playBtn](auto const&, auto const& args) mutable {
                if (*updatingSliderFromPlayback) return;
                const int frameIdx = static_cast<int>(std::round(args.NewValue()));
                if (frameIdx == g_radarCurrentFrame.load()) return;
                g_radarIsPlaying = false;
                if (activeTimer) activeTimer.Stop();
                if (auto glyph = playBtn.Child().try_as<TextBlock>()) {
                    glyph.Text(L"▶");
                }
                showFrame(frameIdx);
            });
            timeline.Children().Append(frameSlider);

            Grid::SetColumn(timeBadge, 3);
            timeBadge.Height(24);
            timeBadge.VerticalAlignment(VerticalAlignment::Center);
            timeBadge.HorizontalAlignment(HorizontalAlignment::Right);
            timeBadge.Margin(Thickness{4, 0, 2, 0});
            timeline.Children().Append(timeBadge);
            timePill.Child(timeline);
            hudGrid.Children().Append(timePill);
        }
        // Mouse wheel zoom support on radar grid
        radarRootGrid.PointerWheelChanged([weakRoot, condition, currentIcon, currentTemp](auto const&, winrt::Windows::UI::Xaml::Input::PointerRoutedEventArgs const& e) {
            int delta = e.GetCurrentPoint(nullptr).Properties().MouseWheelDelta();
            if (delta > 0 && g_radarZoom < 12) {
                g_radarZoom++;
                if (auto root = weakRoot.get()) {
                    PopulateForecastUI(root, condition, currentIcon, currentTemp, false,
                                       true, L"", L"", L"", L"", 1);
                }
                e.Handled(true);
            } else if (delta < 0 && g_radarZoom > 5) {
                g_radarZoom--;
                if (auto root = weakRoot.get()) {
                    PopulateForecastUI(root, condition, currentIcon, currentTemp, false,
                                       true, L"", L"", L"", L"", -1);
                }
                e.Handled(true);
            }
        });
        // Start playback only when requested; each new hour downloads one WMS map.
        activeTimer.Interval(std::chrono::milliseconds(RadarPlaybackIntervalMs()));
        s_radarFrameTickToken = activeTimer.Tick([showFrame, localRadarFrames,
                frameLoaded, frameFailed](auto const& sender, auto const&) mutable {
            auto timer = sender.template try_as<winrt::Windows::UI::Xaml::DispatcherTimer>();
            if (!timer) return;
            if (!g_radarIsPlaying || localRadarFrames.empty() || s_radarFrameTimer != timer || g_modUnloaded) {
                try { timer.Stop(); } catch (...) {}
                return;
            }
            int current = g_radarCurrentFrame.load();
            if (current >= 0 && current < static_cast<int>(frameLoaded->size())) {
                if ((*frameFailed)[current]) { timer.Stop(); return; }
                for (bool ready : (*frameLoaded)[current]) if (!ready) return;
            }
            int nextIdx = (current + 1) % (int)localRadarFrames.size();
            showFrame(nextIdx);
            int ms = RadarPlaybackIntervalMs();
            if (nextIdx == (int)localRadarFrames.size() - 1) {
                timer.Interval(std::chrono::milliseconds(ms + ms / 2));
            } else {
                timer.Interval(std::chrono::milliseconds(ms));
            }
        });
        if (g_radarIsPlaying && localRadarFrames.size() > 1) {
            activeTimer.Start();
        }
        if (!localRadarFrames.empty()) showFrame(activeFrameIdx);
        radarRootGrid.Children().Append(hudGrid);
        graphSection.Children().Append(radarRootGrid);
        mainStack.Children().Append(graphSection);
    } else {
        // Filter Hourly Data for Selected Date
        std::vector<HourlyForecast> currentDayHourly;
        for (const auto& h : localHourly) {
            if (h.rawDate == g_selectedDate) {
                currentDayHourly.push_back(h);
            }
        }
        // --- Line Graph Canvas
        if (!currentDayHourly.empty()) {
        double maxVal = -9999.0;
        double minVal = 9999.0;
        for (const auto& h : currentDayHourly) {
            double v = (g_selectedGraphTab == 0) ? h.tempRaw
                : (g_selectedGraphTab == 1) ? h.precipRaw
                : h.windkphRaw;
            if (g_selectedGraphTab == 0 && !g_useCelsius) {
                v = (v * 9.0 / 5.0) + 32.0;
            }
            if (g_selectedGraphTab == 2 && !g_useCelsius) {
                v = v * 0.621371;
            }
            if (v > maxVal)
                maxVal = v;
            if (v < minVal)
                minVal = v;
        }
        if (g_selectedGraphTab == 1) {
            // Fixed 0% to 100% scale for precipitation probability
            minVal = 0.0;
            maxVal = 100.0;
        } else if (g_selectedGraphTab == 2) {
            minVal = 0.0;
            if (maxVal <= 0.0) {
                maxVal = 10.0;
            } else {
                maxVal = maxVal * 1.2; // Add padding so it doesn't hit the very top
            }
        } else {
            if (maxVal == minVal) {
                maxVal += 1;
                minVal -= 1;
            }
        }
        double range = maxVal - minVal;
        if (range <= 0.0001) range = 1.0;
        const double graphHeight = 64.0;
        const double graphOuterWidth = std::max(280.0, rootGrid.Width() - 32.0);
        double paddingTB = 2.0;
        // We make columns 55.0px wide, and display ALL points seamlessly!
        double colWidth = 55.0;
        double canvasWidth = std::max(graphOuterWidth, colWidth * (double)std::max((size_t)0, currentDayHourly.size() - 1) + 40.0);
        Grid graphContainer;
        graphContainer.Background(SolidColorBrush{ cardBgColor });
        graphContainer.CornerRadius(CornerRadius{ 6.0, 6.0, 6.0, 6.0 });
        graphContainer.Padding(
            Thickness{ 0.0, paddingTB, 0.0, paddingTB });
        // Setup smooth tab/day click fade animation transitions
        if (animateGraph) {
            winrt::Windows::UI::Xaml::Media::Animation::TransitionCollection
                graphTransitions;
            winrt::Windows::UI::Xaml::Media::Animation::ContentThemeTransition
                graphTransition;
            try {
                graphTransitions.Append(graphTransition);
                graphContainer.ChildrenTransitions(graphTransitions);
            }
            catch (...) {
            }
        }
        Canvas canvas;
        canvas.Height(graphHeight + 34.0);
        canvas.Width(canvasWidth);
        // Draw 3 horizontal dashed guidelines to provide separating visual context
        double gridYLines[] = { 18.0, graphHeight / 2.0 + 18.0,
                               graphHeight + 18.0 };
        for (double ly : gridYLines) {
            winrt::Windows::UI::Xaml::Shapes::Line gl;
            gl.X1(0.0);
            gl.Y1(ly);
            gl.X2(canvasWidth);
            gl.Y2(ly);
            gl.Stroke(SolidColorBrush{ dividerColor });
            gl.StrokeThickness(0.75);
            try {
                winrt::Windows::UI::Xaml::Media::DoubleCollection strokeDash;
                strokeDash.Append(4.0);
                strokeDash.Append(4.0);
                gl.StrokeDashArray(strokeDash);
            }
            catch (...) {
            }
            canvas.Children().Append(gl);
        }
        winrt::Windows::UI::Xaml::Shapes::Polyline poly;
        poly.Stroke(SolidColorBrush{ winrt::Windows::UI::ColorHelper::FromArgb(
            242, 250, 214, 53) });  // Yellow
        if (g_selectedGraphTab == 1)
            poly.Stroke(SolidColorBrush{ activeTabColor });  // Blue for precip
        if (g_selectedGraphTab == 2)
            poly.Stroke(
                SolidColorBrush{ winrt::Windows::UI::ColorHelper::FromArgb(
                    42, 200, 200, 200) });  // Gray for wind
        poly.StrokeThickness(2.0);
        winrt::Windows::UI::Xaml::Media::PointCollection points;
        const double graphPaddingLeft = 28.0;
        const double graphPaddingRight = 28.0;
        double activeWidth = canvasWidth - graphPaddingLeft - graphPaddingRight;
        double xStep = (currentDayHourly.size() <= 1) ? 0.0 : activeWidth / (double)(currentDayHourly.size() - 1);
        std::vector<TextBlock> valTbs;
        std::vector<TextBlock> timeTbs;
        for (size_t i = 0; i < currentDayHourly.size(); i++) {
            auto h = currentDayHourly[i];
            double v = (g_selectedGraphTab == 0) ? h.tempRaw
                : (g_selectedGraphTab == 1) ? h.precipRaw
                : h.windkphRaw;
            if (g_selectedGraphTab == 0 && !g_useCelsius) {
                v = (v * 9.0 / 5.0) + 32.0;
            }
            if (g_selectedGraphTab == 2 && !g_useCelsius) {
                v = v * 0.621371;
            }
            double normY = (v - minVal) / range;  // 0 to 1
            double x = graphPaddingLeft + i * xStep;
            double y = graphHeight - (normY * graphHeight);
            points.Append(
                winrt::Windows::Foundation::Point{ (float)x, (float)y + 18.0f });
            // Value Label
            TextBlock valTb;
            wchar_t vBuf[64];
            if (g_selectedGraphTab == 0) {
                swprintf(vBuf, L"%.0f°", v);
            }
            else if (g_selectedGraphTab == 1) {
                swprintf(vBuf, L"%.0f%%", v);
            }
            else {
                if (g_useCelsius) {
                    swprintf(vBuf, L"%.1f\nkm/h", v);
                }
                else {
                    swprintf(vBuf, L"%.1f\nmph", v);
                }
            }
            valTb.Text(vBuf);
            valTb.FontSize(11);
            valTb.Foreground(SolidColorBrush{ secondaryColor });
            valTb.Width(50.0);
            valTb.TextAlignment(winrt::Windows::UI::Xaml::TextAlignment::Center);
            // Center the label horizontally relative to x with clamping to prevent clipping on left and right edges
            double valTbLeft = x - 25.0;
            if (valTbLeft < 0.0) {
                valTbLeft = 0.0;
            }
            if (valTbLeft + 50.0 > canvasWidth) {
                valTbLeft = canvasWidth - 50.0;
            }
            Canvas::SetLeft(valTb, valTbLeft);
            // Move temperature numbers up to sit gracefully above the line
            double valYOffset = (g_selectedGraphTab == 2) ? 24.0 : 14.0;
            Canvas::SetTop(valTb, y + 17.0 - valYOffset);
            canvas.Children().Append(valTb);
            valTbs.push_back(valTb);
            // Time Label
            TextBlock timeTb;
            timeTb.Text(h.timeString);
            timeTb.FontSize(10);
            timeTb.Foreground(SolidColorBrush{ tertiaryColor });
            timeTb.Width(40.0);
            timeTb.TextAlignment(winrt::Windows::UI::Xaml::TextAlignment::Center);
            double timeTbLeft = x - 20.0;
            if (timeTbLeft < 0.0) {
                timeTbLeft = 0.0;
            }
            if (timeTbLeft + 40.0 > canvasWidth) {
                timeTbLeft = canvasWidth - 40.0;
            }
            Canvas::SetLeft(timeTb, timeTbLeft);
            Canvas::SetTop(timeTb, graphHeight + 20.0);
            canvas.Children().Append(timeTb);
            timeTbs.push_back(timeTb);
        }
        poly.Points(points);
        // Underlay semi-transparent shaded gradient region
        winrt::Windows::UI::Xaml::Shapes::Polygon shadedArea;
        winrt::Windows::UI::Xaml::Media::PointCollection polygonPoints;
        for (uint32_t j = 0; j < points.Size(); j++) {
            polygonPoints.Append(points.GetAt(j));
        }
        if (points.Size() > 0) {
            auto lastPt = points.GetAt(points.Size() - 1);
            auto firstPt = points.GetAt(0);
            polygonPoints.Append(winrt::Windows::Foundation::Point{ lastPt.X, (float)graphHeight + 18.0f });
            polygonPoints.Append(winrt::Windows::Foundation::Point{ firstPt.X, (float)graphHeight + 18.0f });
        }
        shadedArea.Points(polygonPoints);
        winrt::Windows::UI::Xaml::Media::LinearGradientBrush gradient;
        gradient.StartPoint(winrt::Windows::Foundation::Point{ 0.0f, 0.0f });
        gradient.EndPoint(winrt::Windows::Foundation::Point{ 0.0f, 1.0f });
        winrt::Windows::UI::Xaml::Media::GradientStop stop1;
        stop1.Offset(0.0);
        winrt::Windows::UI::Color c1;
        if (g_selectedGraphTab == 0) c1 = winrt::Windows::UI::ColorHelper::FromArgb(160, 250, 214, 53); // Yellow
        else if (g_selectedGraphTab == 1) c1 = winrt::Windows::UI::ColorHelper::FromArgb(160, activeTabColor.R, activeTabColor.G, activeTabColor.B); // Blue
        else c1 = winrt::Windows::UI::ColorHelper::FromArgb(160, 200, 200, 200); // Gray
        stop1.Color(c1);
        winrt::Windows::UI::Xaml::Media::GradientStop stop2;
        stop2.Offset(1.0);
        stop2.Color(winrt::Windows::UI::ColorHelper::FromArgb(15, c1.R, c1.G, c1.B)); // subtle glow at bottom
        gradient.GradientStops().Append(stop1);
        gradient.GradientStops().Append(stop2);
        shadedArea.Fill(gradient);
        canvas.Children().Append(shadedArea);
        // Add the poly line on top of the shaded region
        canvas.Children().Append(poly);
        double selectedXOffset = 0.0;
        // Add interactive overlay columns on top of graph
        for (size_t i = 0; i < currentDayHourly.size(); i++) {
            auto h = currentDayHourly[i];
            double x = graphPaddingLeft + i * xStep;
            bool isSelectedHour = (h.timeString == g_selectedHour);
            if (isSelectedHour) {
                selectedXOffset = x - (graphOuterWidth / 2.0);
            }
            Border hoverCol;
            hoverCol.Width(std::max(20.0, xStep - 6.0));
            hoverCol.Height(graphHeight + 34.0);
            if (isSelectedHour) {
                hoverCol.Background(SolidColorBrush{ winrt::Windows::UI::ColorHelper::FromArgb(30, activeTabColor.R, activeTabColor.G, activeTabColor.B) });
                hoverCol.BorderBrush(SolidColorBrush{ winrt::Windows::UI::ColorHelper::FromArgb(80, activeTabColor.R, activeTabColor.G, activeTabColor.B) });
                hoverCol.BorderThickness(Thickness{ 1, 1, 1, 1 });
            }
            else {
                hoverCol.Background(SolidColorBrush{ winrt::Windows::UI::Colors::Transparent() });
                hoverCol.BorderThickness(Thickness{ 0, 0, 0, 0 });
            }
            hoverCol.CornerRadius(winrt::Windows::UI::Xaml::CornerRadius{ 4.0, 4.0, 4.0, 4.0 });
            Canvas::SetLeft(hoverCol, x - (hoverCol.Width() / 2.0));
            Canvas::SetTop(hoverCol, 0);
            TextBlock valTb = valTbs[i];
            TextBlock timeTb = timeTbs[i];
            if (isSelectedHour) {
                valTb.Foreground(SolidColorBrush{ primaryColor });
                timeTb.Foreground(SolidColorBrush{ primaryColor });
                valTb.FontWeight(winrt::Windows::UI::Text::FontWeights::Bold());
                timeTb.FontWeight(winrt::Windows::UI::Text::FontWeights::Bold());
            }
            hoverCol.PointerEntered([valTb = winrt::make_weak(valTb), timeTb = winrt::make_weak(timeTb), isSelectedHour, activeTabColor](auto const& sender, auto const&) {
                if (auto v = valTb.get()) v.Foreground(SolidColorBrush{ activeTabColor });
                if (auto t = timeTb.get()) t.Foreground(SolidColorBrush{ activeTabColor });
                if (!isSelectedHour) {
                    if (auto col = sender.template try_as<Grid>()) {
                        col.Background(SolidColorBrush{ winrt::Windows::UI::ColorHelper::FromArgb(20, activeTabColor.R, activeTabColor.G, activeTabColor.B) });
                        col.BorderBrush(SolidColorBrush{ winrt::Windows::UI::ColorHelper::FromArgb(40, activeTabColor.R, activeTabColor.G, activeTabColor.B) });
                        col.BorderThickness(Thickness{ 1, 1, 1, 1 });
                    }
                }
                });
            hoverCol.PointerExited([valTb = winrt::make_weak(valTb), timeTb = winrt::make_weak(timeTb), isSelectedHour, primaryColor, secondaryColor, tertiaryColor, activeTabColor](auto const& sender, auto const&) {
                if (isSelectedHour) {
                    if (auto v = valTb.get()) v.Foreground(SolidColorBrush{ primaryColor });
                    if (auto t = timeTb.get()) t.Foreground(SolidColorBrush{ primaryColor });
                    if (auto v = valTb.get()) v.FontWeight(winrt::Windows::UI::Text::FontWeights::Bold());
                    if (auto t = timeTb.get()) t.FontWeight(winrt::Windows::UI::Text::FontWeights::Bold());
                    if (auto col = sender.template try_as<Grid>()) {
                        col.Background(SolidColorBrush{ winrt::Windows::UI::ColorHelper::FromArgb(30, activeTabColor.R, activeTabColor.G, activeTabColor.B) });
                        col.BorderBrush(SolidColorBrush{ winrt::Windows::UI::ColorHelper::FromArgb(80, activeTabColor.R, activeTabColor.G, activeTabColor.B) });
                        col.BorderThickness(Thickness{ 1, 1, 1, 1 });
                    }
                }
                else {
                    if (auto v = valTb.get()) v.Foreground(SolidColorBrush{ secondaryColor });
                    if (auto t = timeTb.get()) t.Foreground(SolidColorBrush{ tertiaryColor });
                    if (auto col = sender.template try_as<Grid>()) {
                        col.Background(SolidColorBrush{ winrt::Windows::UI::Colors::Transparent() });
                        col.BorderThickness(Thickness{ 0, 0, 0, 0 });
                    }
                }
                });
            auto weakRoot = winrt::make_weak(rootGrid);
            std::wstring h_timeString = h.timeString;
            std::wstring h_conditionName = h.conditionName;
            std::wstring h_icon = h.icon;
            std::wstring h_temp = h.temp;
            std::wstring h_precipProb = h.precipProb;
            std::wstring h_humidity = h.humidity;
            std::wstring h_windSpeed = h.windSpeed;
            std::wstring h_iconUrl = h.iconUrl;
            hoverCol.Tapped([weakRoot, h_timeString, h_conditionName, h_icon, h_temp, h_precipProb, h_humidity, h_windSpeed, h_iconUrl](auto const&, auto const&) {
                g_selectedHour = h_timeString;
                if (auto root = weakRoot.get()) {
                    PopulateForecastUI(root, h_conditionName, h_icon, h_temp, false, false, h_precipProb, h_humidity, h_windSpeed, h_iconUrl);
                }
                });
            canvas.Children().Append(hoverCol);
        }
        ScrollViewer graphScroll;
        graphScroll.HorizontalScrollBarVisibility(ScrollBarVisibility::Hidden);
        graphScroll.VerticalScrollBarVisibility(ScrollBarVisibility::Disabled);
        graphScroll.HorizontalScrollMode(ScrollMode::Enabled);
        graphScroll.VerticalScrollMode(ScrollMode::Disabled);
        graphScroll.PointerWheelChanged([](auto const& sender, winrt::Windows::UI::Xaml::Input::PointerRoutedEventArgs const& e) {
            auto sv = sender.template try_as<ScrollViewer>();
            if (sv) {
                int delta = e.GetCurrentPoint(sv).Properties().MouseWheelDelta();
                double newOffset = sv.HorizontalOffset() - delta;
                sv.ChangeView(newOffset, nullptr, nullptr, false);
                e.Handled(true);
            }
            });
        graphScroll.Content(canvas);
        graphContainer.Children().Append(graphScroll);
        graphSection.Children().Append(graphContainer);
        mainStack.Children().Append(graphSection);
        double targetScrollOffset = (g_graphScrollOffset < 0.0) ? selectedXOffset : g_graphScrollOffset;
        if (targetScrollOffset > 0.0) {
            graphScroll.ChangeView(targetScrollOffset, nullptr, nullptr, true);
            graphScroll.Loaded([targetScrollOffset](auto const& sender, auto const&) {
                if (auto sv = sender.template try_as<ScrollViewer>()) {
                    sv.ChangeView(targetScrollOffset, nullptr, nullptr, true);
                }
                });
        }
        graphScroll.ViewChanged([](auto const& sender, auto const&) {
            if (auto sv = sender.template try_as<ScrollViewer>()) {
                g_graphScrollOffset = sv.HorizontalOffset();
            }
            });
    }
    }
    if (g_selectedGraphTab != 3) { winrt::Windows::UI::Xaml::Shapes::Rectangle div2;
    div2.Height(1);
    div2.Fill(SolidColorBrush{ dividerColor });
    div2.Margin(Thickness{ 0, 2, 0, 0 });
    mainStack.Children().Append(div2); }
    // --- Daily Forecast (Horizontal Scroll or Grid)
    if (!localDaily.empty() && g_selectedGraphTab != 3) {
        ScrollViewer dailyScroll;
        dailyScroll.HorizontalScrollBarVisibility(ScrollBarVisibility::Hidden);
        dailyScroll.VerticalScrollBarVisibility(ScrollBarVisibility::Disabled);
        dailyScroll.HorizontalScrollMode(ScrollMode::Enabled);
        dailyScroll.VerticalScrollMode(ScrollMode::Disabled);
        dailyScroll.VerticalAlignment(VerticalAlignment::Center);
        dailyScroll.Margin(Thickness{ 3, 0, 0, 0 });
        if (g_dailyScrollOffset > 0.0) {
            dailyScroll.ChangeView(g_dailyScrollOffset, nullptr, nullptr, true);
            dailyScroll.Loaded([](auto const& sender, auto const&) {
                if (auto s = sender.template try_as<winrt::Windows::UI::Xaml::Controls::ScrollViewer>()) s.ChangeView(g_dailyScrollOffset, nullptr, nullptr, true);
                });
        }
        dailyScroll.ViewChanged([](auto const& sender, auto const&) {
            auto sv = sender.template try_as<ScrollViewer>();
            if (sv) {
                g_dailyScrollOffset = sv.HorizontalOffset();
            }
            });
        StackPanel dailyStack;
        dailyStack.Orientation(Orientation::Horizontal);
        dailyStack.Spacing(8);
        dailyStack.VerticalAlignment(VerticalAlignment::Center);
        size_t maxDailyToShow = localDaily.size();
        if (localDaily.size() <= 7) {
            dailyStack.HorizontalAlignment(HorizontalAlignment::Center);
            dailyStack.Margin(Thickness{ 0, 0, 0, 0 });
        }
        else {
            dailyStack.HorizontalAlignment(HorizontalAlignment::Left);
            dailyStack.Margin(Thickness{ 3, 3, 0, 0 });
        }
        winrt::Windows::UI::Xaml::Media::Animation::TransitionCollection dailyTrans;
        if (animate) {
            winrt::Windows::UI::Xaml::Media::Animation::EntranceThemeTransition entrance;
            entrance.IsStaggeringEnabled(true);
            entrance.FromVerticalOffset(24.0);
            dailyTrans.Append(entrance);
        }
        winrt::Windows::UI::Xaml::Media::Animation::RepositionThemeTransition repo;
        dailyTrans.Append(repo);
        dailyStack.ChildrenTransitions(dailyTrans);
        for (size_t i = 0; i < maxDailyToShow; i++) {
            auto d = localDaily[i];
            bool isSel = (d.rawDate == g_selectedDate);
            Grid dayCard;
            dayCard.CornerRadius(CornerRadius{ 8, 8, 8, 8 });
            dayCard.Padding(Thickness{ 4, 1, 4, 0 });
            dayCard.Width(70.0); /* Force exact width for predictable layouts */
            dayCard.Height(96.0); /* Form-fitted height for balanced margins and no dead space */
            dayCard.VerticalAlignment(VerticalAlignment::Center);
            dayCard.Background(SolidColorBrush{
                isSel ? cardBgColor
                      : winrt::Windows::UI::Colors::Transparent() });
            std::wstring dayNum;
            if (d.rawDate.length() >= 10) {
                dayNum = d.rawDate.substr(8, 2);
                if (dayNum.length() > 0 && dayNum[0] == L'0') dayNum = dayNum.substr(1);
            }
            Grid dayHeader;
            dayHeader.HorizontalAlignment(HorizontalAlignment::Stretch);
            dayHeader.VerticalAlignment(VerticalAlignment::Center);
            dayHeader.Margin(Thickness{ 0, 0, 0, 0 });
            TextBlock tDay;
            tDay.Text(i == 0 ? L"Today" : d.dayName);
            tDay.FontSize(10.5);
            tDay.Foreground(SolidColorBrush{ primaryColor });
            tDay.HorizontalAlignment(HorizontalAlignment::Left);
            tDay.VerticalAlignment(VerticalAlignment::Center);
            tDay.Margin(Thickness{ 0, 0, 0, 0 });
            dayHeader.Children().Append(tDay);
            if (!dayNum.empty()) {
                Border dateBorder;
                dateBorder.CornerRadius(CornerRadius{ 3, 3, 3, 3 });
                winrt::Windows::UI::Color dateBg = isDark
                    ? winrt::Windows::UI::ColorHelper::FromArgb(80, 255, 255, 255)
                    : winrt::Windows::UI::ColorHelper::FromArgb(45, 0, 0, 0);
                dateBorder.Background(SolidColorBrush{ dateBg });
                dateBorder.Padding(Thickness{ 3, 0.5, 3, 0.5 });
                dateBorder.HorizontalAlignment(HorizontalAlignment::Right);
                dateBorder.VerticalAlignment(VerticalAlignment::Center);
                dateBorder.Margin(Thickness{ 0, 0, 0, 0 });
                TextBlock tDate;
                tDate.Text(dayNum);
                tDate.FontSize(10);
                tDate.Foreground(SolidColorBrush{ primaryColor });
                tDate.FontWeight(winrt::Windows::UI::Text::FontWeights::Bold());
                tDate.VerticalAlignment(VerticalAlignment::Center);
                tDate.Margin(Thickness{ 0, 0, 0, 0 });
                dateBorder.Child(tDate);
                dayHeader.Children().Append(dateBorder);
            }
            Border headerBorder;
            headerBorder.Width(62.0);
            headerBorder.Height(20.0);
            headerBorder.CornerRadius(CornerRadius{ 4, 4, 4, 4 });
            winrt::Windows::UI::Color bgCol = isDark ? winrt::Windows::UI::ColorHelper::FromArgb(92, 255, 255, 255) : winrt::Windows::UI::ColorHelper::FromArgb(42, 0, 0, 0);
            winrt::Windows::UI::Color borderCol = isDark ? winrt::Windows::UI::ColorHelper::FromArgb(114, 255, 255, 255) : winrt::Windows::UI::ColorHelper::FromArgb(64, 0, 0, 0);
            headerBorder.Background(SolidColorBrush{ bgCol });
            headerBorder.BorderBrush(SolidColorBrush{ borderCol });
            headerBorder.BorderThickness(Thickness{ 1, 1, 1, 1 });
            headerBorder.Padding(Thickness{ 4, 0, 3, 0 });
            headerBorder.HorizontalAlignment(HorizontalAlignment::Center);
            headerBorder.VerticalAlignment(VerticalAlignment::Center);
            headerBorder.Child(dayHeader);
            StackPanel dayCol;
            dayCol.Orientation(Orientation::Vertical);
            dayCol.Spacing(4);
            dayCol.HorizontalAlignment(HorizontalAlignment::Center);
            dayCol.VerticalAlignment(VerticalAlignment::Center);
            dayCol.Margin(Thickness{ 0, 0, 0, 0 });
            dayCol.Children().Append(headerBorder);
            Grid iconBox;
            iconBox.Width(42);
            iconBox.Height(42);
            iconBox.HorizontalAlignment(HorizontalAlignment::Center);
            iconBox.VerticalAlignment(VerticalAlignment::Center);
            CompositeTransform iconBoxTrans;
            iconBoxTrans.ScaleX(1.0 / 1.18);
            iconBoxTrans.ScaleY(1.0 / 1.18);
            iconBox.RenderTransform(iconBoxTrans);
            iconBox.RenderTransformOrigin(winrt::Windows::Foundation::Point{ 0.5, 0.5 });
            std::wstring dUri = GetWeatherIconPathOrUrl(d.weatherCode, true);
            FontIcon dayFb{ nullptr };
            Image dayImg{ nullptr };
            FontIcon dayIcon{ nullptr };
            TextBlock dayEmoji{ nullptr };
            if (g_weatherStyle == 2 || g_weatherStyle == 3 || g_weatherStyle == 4) {
                dayFb = FontIcon{};
                dayFb.FontFamily(winrt::Windows::UI::Xaml::Media::FontFamily(L"Segoe UI Symbol"));
                dayFb.Glyph(d.icon);
                dayFb.FontSize(26.0 * 1.18);
                dayFb.Foreground(SolidColorBrush{ GetXamlIconColor(d.condition) });
                dayFb.HorizontalAlignment(HorizontalAlignment::Center);
                dayFb.VerticalAlignment(VerticalAlignment::Center);
                iconBox.Children().Append(dayFb);
                dayImg = Image{};
                dayImg.Width(28.0 * 1.18);
                dayImg.Height(28.0 * 1.18);
                dayImg.HorizontalAlignment(HorizontalAlignment::Center);
                dayImg.VerticalAlignment(VerticalAlignment::Center);
                dayImg.Stretch(winrt::Windows::UI::Xaml::Media::Stretch::Uniform);
                SetImageOrFallback(dayImg, dayFb, dUri);
                iconBox.Children().Append(dayImg);
            }
            else if (g_weatherStyle == 1) {
                dayIcon = FontIcon{};
                dayIcon.FontFamily(winrt::Windows::UI::Xaml::Media::FontFamily(L"Segoe UI Symbol"));
                dayIcon.Glyph(d.icon);
                dayIcon.FontSize(22.0);
                dayIcon.Foreground(SolidColorBrush{ GetXamlIconColor(d.condition) });
                dayIcon.HorizontalAlignment(HorizontalAlignment::Center);
                dayIcon.VerticalAlignment(VerticalAlignment::Center);
                dayIcon.Margin(Thickness{ 0, -2.0, 0, 0 });
                iconBox.Children().Append(dayIcon);
            }
            else {
                dayEmoji = TextBlock{};
                dayEmoji.Text(d.icon);
                dayEmoji.FontSize(26.0 * 1.18);
                dayEmoji.HorizontalAlignment(HorizontalAlignment::Center);
                dayEmoji.VerticalAlignment(VerticalAlignment::Center);
                dayEmoji.TextAlignment(winrt::Windows::UI::Xaml::TextAlignment::Center);
                iconBox.Children().Append(dayEmoji);
            }
            dayCol.Children().Append(iconBox);
            StackPanel minmax;
            minmax.Orientation(Orientation::Horizontal);
            minmax.Spacing(4);
            minmax.HorizontalAlignment(HorizontalAlignment::Center);
            minmax.Margin(Thickness{ 0, 4, 0, 0 });
            std::wstring cleanMax = FormatNoDecimalTemp(d.tempMax);
            TextBlock tMax;
            tMax.Text(cleanMax);
            tMax.FontSize(11.5);
            tMax.Foreground(SolidColorBrush{ primaryColor });
            tMax.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());
            std::wstring cleanMin = FormatNoDecimalTemp(d.tempMin);
            TextBlock tMin;
            tMin.Text(cleanMin);
            tMin.FontSize(11.5);
            tMin.Foreground(SolidColorBrush{ secondaryColor });
            minmax.Children().Append(tMax);
            minmax.Children().Append(tMin);
            dayCol.Children().Append(minmax);
            dayCard.Children().Append(dayCol);
            std::wstring dDate = d.rawDate;
            std::wstring dCond = d.condition;
            std::wstring dIcon = d.icon;
            std::wstring dTemp = d.tempMax;
            std::wstring dIconUrl = dUri;
            // Find matching hourly info for dDate to pass directly
            std::wstring dPrecip = L"";
            std::wstring dHumidity = L"";
            std::wstring dWind = L"";
            for (const auto& h : localHourly) {
                if (h.rawDate == dDate) {
                    dPrecip = h.precipProb;
                    dHumidity = h.humidity;
                    dWind = h.windSpeed;
                    break;
                }
            }
            auto weakRoot = winrt::make_weak(rootGrid);
            dayCard.Tapped([weakRoot, dDate, dCond, dIcon, dTemp, dPrecip, dHumidity, dWind, dIconUrl](auto const& sender, auto const&) {
                if (g_selectedDate != dDate) {
                    g_selectedDate = dDate;
                    g_graphScrollOffset = -1.0;
                    if (auto root = weakRoot.get()) {
                        PopulateForecastUI(root, dCond, dIcon,
                            dTemp, false, false,
                            dPrecip, dHumidity, dWind, dIconUrl);
                    }
                }
                });
            dayCard.PointerEntered([cardBgColor, iconBoxTrans](auto const& sender, auto const&) {
                if (auto card = sender.template try_as<winrt::Windows::UI::Xaml::Controls::Grid>()) {
                    card.Background(SolidColorBrush{ cardBgColor });
                }
                if (g_animateIcons && iconBoxTrans) {
                    Storyboard enterSb;
                    CubicEase ease;
                    ease.EasingMode(EasingMode::EaseOut);
                    DoubleAnimation sXAnim;
                    sXAnim.EnableDependentAnimation(true);
                    Storyboard::SetTarget(sXAnim, iconBoxTrans);
                    Storyboard::SetTargetProperty(sXAnim, L"ScaleX");
                    sXAnim.To(1.0);
                    sXAnim.Duration(winrt::Windows::UI::Xaml::DurationHelper::FromTimeSpan(std::chrono::milliseconds(140)));
                    sXAnim.EasingFunction(ease);
                    enterSb.Children().Append(sXAnim);
                    DoubleAnimation sYAnim;
                    sYAnim.EnableDependentAnimation(true);
                    Storyboard::SetTarget(sYAnim, iconBoxTrans);
                    Storyboard::SetTargetProperty(sYAnim, L"ScaleY");
                    sYAnim.To(1.0);
                    sYAnim.Duration(winrt::Windows::UI::Xaml::DurationHelper::FromTimeSpan(std::chrono::milliseconds(140)));
                    sYAnim.EasingFunction(ease);
                    enterSb.Children().Append(sYAnim);
                    enterSb.Begin();
                }
            });
            dayCard.PointerExited([dDate, iconBoxTrans](auto const& sender, auto const&) {
                if (auto card = sender.template try_as<winrt::Windows::UI::Xaml::Controls::Grid>()) {
                    if (g_selectedDate != dDate) {
                        card.Background(SolidColorBrush{ winrt::Windows::UI::Colors::Transparent() });
                    }
                }
                if (g_animateIcons && iconBoxTrans) {
                    Storyboard exitSb;
                    CubicEase ease;
                    ease.EasingMode(EasingMode::EaseOut);
                    DoubleAnimation sXAnim;
                    sXAnim.EnableDependentAnimation(true);
                    Storyboard::SetTarget(sXAnim, iconBoxTrans);
                    Storyboard::SetTargetProperty(sXAnim, L"ScaleX");
                    sXAnim.To(1.0 / 1.18);
                    sXAnim.Duration(winrt::Windows::UI::Xaml::DurationHelper::FromTimeSpan(std::chrono::milliseconds(180)));
                    sXAnim.EasingFunction(ease);
                    exitSb.Children().Append(sXAnim);
                    DoubleAnimation sYAnim;
                    sYAnim.EnableDependentAnimation(true);
                    Storyboard::SetTarget(sYAnim, iconBoxTrans);
                    Storyboard::SetTargetProperty(sYAnim, L"ScaleY");
                    sYAnim.To(1.0 / 1.18);
                    sYAnim.Duration(winrt::Windows::UI::Xaml::DurationHelper::FromTimeSpan(std::chrono::milliseconds(180)));
                    sYAnim.EasingFunction(ease);
                    exitSb.Children().Append(sYAnim);
                    exitSb.Begin();
                }
            });
            dailyStack.Children().Append(dayCard);
        }
        dailyScroll.PointerWheelChanged([](auto const& sender, winrt::Windows::UI::Xaml::Input::PointerRoutedEventArgs const& e) {
            auto sv = sender.template try_as<ScrollViewer>();
            if (sv) {
                int delta = e.GetCurrentPoint(sv).Properties().MouseWheelDelta();
                double newOffset = sv.HorizontalOffset() - delta;
                sv.ChangeView(newOffset, nullptr, nullptr, false);
                e.Handled(true);
            }
            });
        dailyScroll.Content(dailyStack);
        mainStack.Children().Append(dailyScroll);
    }
    Grid::SetRow(mainStack, 1);
    contentGrid.Children().Append(mainStack);
    rootGrid.Children().Append(contentGrid);
}
// Programmatically modify the XAML visual children in place
void UpdateWeatherXamlElements(Grid weatherGrid,
    std::wstring temp,
    std::wstring icon,
    std::wstring condition,
    bool acquired) {
    if (!weatherGrid)
        return;
    auto currentWeather = GetCurrentWeatherSnapshot();
    try {
        bool isHorizontal = true;
        HWND hAnchor = FindSystemAnchorWnd();
        HWND hParentTaskbar = hAnchor ? GetAncestor(hAnchor, GA_ROOT) : NULL;
        if (!hParentTaskbar)
            hParentTaskbar = FindWindowW(L"Shell_TrayWnd", NULL);
        if (hParentTaskbar) {
            RECT trayRect;
            if (GetWindowRect(hParentTaskbar, &trayRect)) {
                isHorizontal = (trayRect.right - trayRect.left) > (trayRect.bottom - trayRect.top);
            }
        } else {
            APPBARDATA abd = { sizeof(abd) };
            if (SHAppBarMessage(ABM_GETTASKBARPOS, &abd)) {
                isHorizontal = (abd.uEdge == ABE_TOP || abd.uEdge == ABE_BOTTOM);
            }
        }
        const bool compactWidgets = !isHorizontal && !g_injectToSysTray;
        UpdateInjectedWeatherLayout(weatherGrid);
        weatherGrid.Children().Clear();
        Button buttonElement;
        
        // Remove standard Windows borders and establish padding
        winrt::Windows::UI::Color transColor;
        transColor.A = 0;
        transColor.R = 0;
        transColor.G = 0;
        transColor.B = 0;
        buttonElement.Background(SolidColorBrush{ transColor });
        buttonElement.BorderBrush(nullptr);
        buttonElement.BorderThickness(Thickness{ 0 });
        
        try {
            auto resources = winrt::Windows::UI::Xaml::Application::Current().Resources();
            winrt::Windows::UI::Xaml::Style targetStyle{ nullptr };
            std::vector<std::wstring> styleKeys = {
                L"TaskbarFrameRepeaterButtonStyle",
                L"Taskbar.TaskbarFrameRepeaterButtonStyle",
                L"TaskbarTopLevelButtonStyle",
                L"DefaultButtonStyle"
            };
            
            for (const auto& key : styleKeys) {
                if (resources.HasKey(winrt::box_value(key))) {
                    targetStyle = resources.Lookup(winrt::box_value(key)).try_as<winrt::Windows::UI::Xaml::Style>();
                    if (targetStyle) {
                        buttonElement.Style(targetStyle);
                        break;
                    }
                }
            }
        } catch (...) {}
        
        try {
            buttonElement.CornerRadius(CornerRadius{ 4.0, 4.0, 4.0, 4.0 });
        } catch (...) {}
        try {
            buttonElement.IsTabStop(false);
            buttonElement.AllowFocusOnInteraction(false);
        } catch (...) {}
        try {
            buttonElement.BackgroundSizing(winrt::Windows::UI::Xaml::Controls::BackgroundSizing::InnerBorderEdge);
        } catch (...) {}
        
        // Force CornerRadius on inner elements after loading
        buttonElement.Loaded([](auto const& sender, auto const&) {
            try {
                auto btn = sender.template try_as<winrt::Windows::UI::Xaml::Controls::Button>();
                if (!btn) return;
                if (g_debugLogs && !g_modUnloaded) {
                    try {
                        if (auto content = btn.Content().template try_as<FrameworkElement>()) {
                            Wh_Log(L"[Wh_WeatherHost] Button content: %s actual=%.1fx%.1f desired=%.1fx%.1f",
                                   winrt::get_class_name(content).c_str(), content.ActualWidth(), content.ActualHeight(),
                                   content.DesiredSize().Width, content.DesiredSize().Height);
                        }
                        FrameworkElement current = btn;
                        for (int depth = 0; current && depth < 5; ++depth) {
                            Wh_Log(L"[Wh_WeatherHost] Button layout[%d]: %s name=%s actual=%.1fx%.1f desired=%.1fx%.1f visibility=%d opacity=%.2f",
                                   depth, winrt::get_class_name(current).c_str(), current.Name().c_str(),
                                   current.ActualWidth(), current.ActualHeight(),
                                   current.DesiredSize().Width, current.DesiredSize().Height,
                                   (int)current.Visibility(), current.Opacity());
                            auto parent = VisualTreeHelper::GetParent(current);
                            current = parent ? parent.try_as<FrameworkElement>() : nullptr;
                        }
                    } catch (...) {}
                }
                int count = winrt::Windows::UI::Xaml::Media::VisualTreeHelper::GetChildrenCount(btn);
                if (count > 0) {
                    auto child = winrt::Windows::UI::Xaml::Media::VisualTreeHelper::GetChild(btn, 0);
                    // Usually it's a Grid or Border
                    if (auto border = child.template try_as<winrt::Windows::UI::Xaml::Controls::Border>()) {
                        border.CornerRadius(winrt::Windows::UI::Xaml::CornerRadius{ 4.0, 4.0, 4.0, 4.0 });
                    }
                    else if (auto grid = child.template try_as<winrt::Windows::UI::Xaml::Controls::Grid>()) {
                        grid.CornerRadius(winrt::Windows::UI::Xaml::CornerRadius{ 4.0, 4.0, 4.0, 4.0 });
                    }
                }
            } catch (...) {}
        });
        // The button's measured width can exceed the fixed shift setting.
        // Recalculate the repeater reservation once its XAML template is ready.
        buttonElement.Loaded([weakGrid = winrt::make_weak(weatherGrid)](auto const&, auto const&) {
            if (g_modUnloaded) return;
            try {
                if (auto grid = weakGrid.get()) {
                    if (grid.Parent()) UpdateInjectedWeatherLayout(grid);
                }
            } catch (...) {}
        });
        double padLeftRight = 8.0;
        double padTopBottom = 1.0;
        
        buttonElement.Padding(compactWidgets ? Thickness{ 0, 0, 0, 0 } :
            (isHorizontal ? Thickness{ padLeftRight, padTopBottom, padLeftRight, padTopBottom } : Thickness{ 0.0, 2.0, 0.0, 2.0 }));
        buttonElement.VerticalAlignment(VerticalAlignment::Stretch);
        buttonElement.HorizontalAlignment(HorizontalAlignment::Stretch);
        if (compactWidgets) {
            buttonElement.Width(40.0);
            buttonElement.MinHeight(0.0);
            buttonElement.MaxHeight(40.0);
            buttonElement.Height(40.0);
            buttonElement.MinWidth(0.0);
            buttonElement.HorizontalContentAlignment(HorizontalAlignment::Stretch);
            buttonElement.VerticalContentAlignment(VerticalAlignment::Center);
        } else if (isHorizontal) {
            if (std::isnan(buttonElement.Height()) || buttonElement.Height() != 40.0) buttonElement.Height(40.0);
            try { buttonElement.MinHeight(0.0); } catch (...) {}
            try { buttonElement.MaxHeight(std::numeric_limits<double>::infinity()); } catch (...) {}
        } else {
            if (!std::isnan(buttonElement.Height())) buttonElement.Height(std::numeric_limits<double>::quiet_NaN());
            if (!std::isnan(buttonElement.Width())) buttonElement.Width(std::numeric_limits<double>::quiet_NaN());
            try { 
                buttonElement.MinHeight(64.0); 
                buttonElement.MaxHeight(std::numeric_limits<double>::infinity());
            } catch (...) {} // Request more space on vertical taskbars to prevent clipping
        }
                
                winrt::Windows::UI::Xaml::Controls::Panel rootPanel = nullptr;
        StackPanel stackPanel = nullptr;
        
        stackPanel = StackPanel{};
        stackPanel.IsHitTestVisible(false);
        stackPanel.Orientation(isHorizontal ? Orientation::Horizontal : Orientation::Vertical);
        stackPanel.Spacing(compactWidgets ? 0.0 : (isHorizontal ? 6.0 : -2.0));
        stackPanel.VerticalAlignment(VerticalAlignment::Center);
        stackPanel.HorizontalAlignment(HorizontalAlignment::Center);
        rootPanel = stackPanel;

        winrt::Windows::UI::Color xamlColor;
        xamlColor.A = 255;
        xamlColor.R = GetRValue(g_textColor);
        xamlColor.G = GetGValue(g_textColor);
        xamlColor.B = GetBValue(g_textColor);
        SolidColorBrush foregroundBrush{ xamlColor };

        double actualIconSize = (double)g_iconFontSize;
        if (!isHorizontal) {
            actualIconSize = (double)g_iconFontSize; // Use original size for vertical taskbar
        }

        // Dedicated fixed-size icon wrapper so layout dimensions remain stable
        Grid iconWrapper;
        double wrapperSize = compactWidgets ? 22.0 : actualIconSize * 1.45;
        iconWrapper.Width(wrapperSize);
        iconWrapper.Height(wrapperSize);
        iconWrapper.VerticalAlignment(VerticalAlignment::Center);
        iconWrapper.HorizontalAlignment(HorizontalAlignment::Center);

        CompositeTransform iconWrapperTrans;
        iconWrapperTrans.ScaleX(1.0);
        iconWrapperTrans.ScaleY(1.0);
        iconWrapper.RenderTransform(iconWrapperTrans);
        iconWrapper.RenderTransformOrigin(winrt::Windows::Foundation::Point{ 0.5, 0.5 });

        FontIcon fbBlock{ nullptr };
        Image imgBlock{ nullptr };
        FontIcon iconBlock{ nullptr };
        TextBlock emojiBlock{ nullptr };

        if (compactWidgets) actualIconSize = (std::min)(actualIconSize, 20.0);
        double maxIconSize = compactWidgets ? (std::min)(actualIconSize * 1.15, 22.0) : actualIconSize * 1.15;
        std::wstring displayIcon = acquired ? icon : L"⏳";

        if (acquired && (g_weatherStyle == 2 || g_weatherStyle == 3 || g_weatherStyle == 4)) {
            fbBlock = FontIcon{};
            fbBlock.FontFamily(winrt::Windows::UI::Xaml::Media::FontFamily(L"Segoe UI Symbol"));
            fbBlock.Glyph(displayIcon);
            fbBlock.FontSize(maxIconSize);
            fbBlock.Foreground(SolidColorBrush{ acquired ? GetXamlIconColor(condition) : xamlColor });
            fbBlock.VerticalAlignment(VerticalAlignment::Center);
            fbBlock.HorizontalAlignment(HorizontalAlignment::Center);
            iconWrapper.Children().Append(fbBlock);

            if (acquired) {
                imgBlock = Image{};
                imgBlock.Width(maxIconSize);
                imgBlock.Height(maxIconSize);
                imgBlock.Stretch(winrt::Windows::UI::Xaml::Media::Stretch::Uniform);
                imgBlock.VerticalAlignment(VerticalAlignment::Center);
                imgBlock.HorizontalAlignment(HorizontalAlignment::Center);
                std::wstring tbUri = !currentWeather.iconUrl.empty() ? currentWeather.iconUrl : GetWeatherIconPathOrUrl(currentWeather.weatherCode, currentWeather.isDay != 0);
                SetImageOrFallback(imgBlock, fbBlock, tbUri);
                iconWrapper.Children().Append(imgBlock);
            }
        }
        else if (acquired && g_weatherStyle == 1) {  // Segoe PUA or Segoe MDL2 Assets
            iconBlock = FontIcon{};
            iconBlock.FontFamily(winrt::Windows::UI::Xaml::Media::FontFamily(L"Segoe UI Symbol"));
            iconBlock.Glyph(displayIcon);
            iconBlock.FontSize(actualIconSize * 1.0);
            iconBlock.Foreground(SolidColorBrush{ acquired ? GetXamlIconColor(condition) : xamlColor });
            iconBlock.VerticalAlignment(VerticalAlignment::Center);
            iconBlock.HorizontalAlignment(HorizontalAlignment::Center);
            iconBlock.Margin(Thickness{ 0, -2.0, 0, 0 });
            iconWrapper.Children().Append(iconBlock);
        }
        else {  // Emojis
            emojiBlock = TextBlock{};
            emojiBlock.Text(displayIcon);
            emojiBlock.FontSize(maxIconSize);
            emojiBlock.VerticalAlignment(VerticalAlignment::Center);
            emojiBlock.HorizontalAlignment(HorizontalAlignment::Center);
            emojiBlock.TextAlignment(winrt::Windows::UI::Xaml::TextAlignment::Center);
            iconWrapper.Children().Append(emojiBlock);
        }

        if (isHorizontal) {
            stackPanel.Children().Append(iconWrapper);

            StackPanel textContainer;
            textContainer.Orientation(Orientation::Vertical);
            textContainer.VerticalAlignment(VerticalAlignment::Center);
            textContainer.HorizontalAlignment(HorizontalAlignment::Center);
            textContainer.Spacing(0.0);

            TextBlock tempBlock;
            if (!g_line1FontFamily.empty()) {
                tempBlock.FontFamily(winrt::Windows::UI::Xaml::Media::FontFamily(g_line1FontFamily));
            }
            tempBlock.FontSize((double)g_line1FontSize);
            tempBlock.FontWeight(g_line1Bold ? winrt::Windows::UI::Text::FontWeights::Bold() : winrt::Windows::UI::Text::FontWeights::Normal());
            tempBlock.Foreground(foregroundBrush);

            TextBlock condBlock;
            if (!g_line2FontFamily.empty()) {
                condBlock.FontFamily(winrt::Windows::UI::Xaml::Media::FontFamily(g_line2FontFamily));
            }
            condBlock.FontSize((double)g_line2FontSize);
            condBlock.FontWeight(g_line2Bold ? winrt::Windows::UI::Text::FontWeights::Bold() : winrt::Windows::UI::Text::FontWeights::Normal());
            try {
                condBlock.Opacity(0.85);
            } catch (...) {}
            condBlock.Foreground(foregroundBrush);

            if (acquired) {
                std::wstring displayTemp = temp;
                if (!displayTemp.empty() && displayTemp.back() == L'°') {
                    displayTemp += (g_useCelsius ? L"C" : L"F");
                }
                tempBlock.Text(FormatTempDisplay(displayTemp));
                condBlock.Text(g_showConditionName ? condition : L"");
            } else {
                tempBlock.Text(L"Weather");
                condBlock.Text(condition == L"Location unavailable"
                    ? condition : L"Loading...");
            }

            textContainer.Children().Append(tempBlock);
            if (g_showConditionName || !acquired) {
                textContainer.Children().Append(condBlock);
            }
            stackPanel.Children().Append(textContainer);
        } else {
            // Vertical taskbar: minimal stacked layout to prevent clipping (icon and temp only)
            stackPanel.Children().Append(iconWrapper);

            TextBlock tempBlock;
            if (!g_line1FontFamily.empty()) {
                tempBlock.FontFamily(winrt::Windows::UI::Xaml::Media::FontFamily(g_line1FontFamily));
            }
            tempBlock.FontSize(compactWidgets ? (std::min)((double)g_line1FontSize, 13.0) : (double)g_line1FontSize * 0.85);
            if (compactWidgets) {
                // A 22px icon row plus a 16px text line leaves 2px of slack.
                tempBlock.LineStackingStrategy(LineStackingStrategy::BlockLineHeight);
                tempBlock.LineHeight(16.0);
                tempBlock.Height(16.0);
                tempBlock.TextWrapping(TextWrapping::NoWrap);
            }
            tempBlock.FontWeight(g_line1Bold ? winrt::Windows::UI::Text::FontWeights::Bold() : winrt::Windows::UI::Text::FontWeights::Normal());
            tempBlock.Foreground(foregroundBrush);
            tempBlock.TextAlignment(winrt::Windows::UI::Xaml::TextAlignment::Center);
            tempBlock.HorizontalAlignment(HorizontalAlignment::Center);
            
            // Adjust margin to pull it slightly closer to the icon
            tempBlock.Margin(compactWidgets ? Thickness{ 0, 0, 0, 0 } : Thickness{ 0, -4, 0, 0 });

            if (acquired) {
                std::wstring displayTemp = temp;
                if (!displayTemp.empty() && displayTemp.back() == L'°') {
                    displayTemp += (g_useCelsius ? L"C" : L"F");
                }
                tempBlock.Text(FormatTempDisplay(displayTemp));
            } else {
                tempBlock.Text(condition == L"Location unavailable"
                    ? L"N/A" : L"...");
            }

            stackPanel.Children().Append(tempBlock);
        }

        ToolTip toolTip;
        toolTip.Content(winrt::box_value(currentWeather.displayCity + L"\n" + condition + L", " + temp + L"\nClick for full forecast"));

        if (compactWidgets) {
            // Fit wider temperature strings without pushing outside the native host.
            Viewbox compactContent;
            compactContent.Width(40.0);
            compactContent.Height(38.0);
            compactContent.MaxHeight(38.0);
            compactContent.Stretch(winrt::Windows::UI::Xaml::Media::Stretch::Uniform);
            compactContent.StretchDirection(StretchDirection::DownOnly);
            compactContent.Child(rootPanel);
            buttonElement.Content(compactContent);
        } else {
            buttonElement.Content(rootPanel);
        }
        UpdateWeatherButtonVisuals(buttonElement, false, false, g_win11FlyoutIsOpen);

        winrt::Windows::UI::Xaml::FrameworkElement fe = buttonElement;
        g_weakXamlWeatherButton = winrt::make_weak(fe);
        ToolTipService::SetToolTip(buttonElement, toolTip);

        // PopulateForecastUI already has a loading state. Keep the flyout
        // available before the first forecast arrives, as on Windows 10.
        {
            Flyout flyout = CreateForecastFlyout(condition, icon, temp);
            auto showFlyoutLocal = [flyout, weakBtn = winrt::make_weak(buttonElement), weakStack = winrt::make_weak(rootPanel), condition, icon, temp]() mutable {
                auto buttonElement = weakBtn.get();
                auto stackPanel = weakStack.get();
                if (!buttonElement || !stackPanel) return;
                try {
                    if (g_flyoutWasOpenOnPointerPressed) {
                        g_flyoutWasOpenOnPointerPressed = false;
                        if (g_win11FlyoutIsOpen) {
                            if (g_activeFlyout) {
                                g_activeFlyout.Hide();
                            }
                            else {
                                flyout.Hide();
                            }
                        }
                        return;
                    }
                    if (g_win11FlyoutIsOpen) {
                        if (g_activeFlyout) {
                            g_activeFlyout.Hide();
                        }
                        else {
                            flyout.Hide();
                        }
                        return;
                    }
                    if (GetTickCount64() - g_lastClosedTickCount < 200) {
                        return;
                    }
                    g_selectedDate = L"";
                    g_selectedHour = L"";
                    g_selectedGraphTab = 0;
                    g_graphScrollOffset = -1.0;
                    g_dailyScrollOffset = 0.0;
                    auto outerGrid = flyout.Content().try_as<winrt::Windows::UI::Xaml::Controls::Grid>();
                    if (outerGrid && outerGrid.Children().Size() > 0) {
                        if (auto innerGrid = outerGrid.Children().GetAt(0).try_as<winrt::Windows::UI::Xaml::Controls::Grid>()) {
                            PopulateForecastUI(innerGrid, condition, icon, temp);
                        } else {
                            PopulateForecastUI(outerGrid, condition, icon, temp);
                        }
                    } else if (outerGrid) {
                        PopulateForecastUI(outerGrid, condition, icon, temp);
                    }
                    g_activeFlyout = flyout;
                    if (!g_hMouseHook) {
                        HMODULE hMod = NULL;
                        GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR)LowLevelMouseProc, &hMod);
                        g_hMouseHook = SetWindowsHookExW(WH_MOUSE_LL, LowLevelMouseProc, hMod, 0);
                    }
                }
                catch (...) {}
                try {
                    winrt::Windows::UI::Xaml::Controls::Primitives::FlyoutShowOptions showOptions;
                    showOptions.ShowMode(winrt::Windows::UI::Xaml::Controls::Primitives::FlyoutShowMode::Standard);
                    auto placement = winrt::Windows::UI::Xaml::Controls::Primitives::FlyoutPlacementMode::Top;
                    float xOffset = 0.0f;
                    float yOffset = 0.0f;
                    double contentWidth = stackPanel.ActualWidth();
                    if (contentWidth <= 0.0) {
                        contentWidth = stackPanel.RenderSize().Width;
                    }
                    if (contentWidth <= 0.0) {
                        contentWidth = buttonElement.ActualWidth();
                    }
                    if (contentWidth <= 0.0) {
                        contentWidth = 120.0;
                    }
                    double contentHeight = 40.0;
                    contentHeight = buttonElement.ActualHeight();
                    if (contentHeight <= 0.0) {
                        contentHeight = buttonElement.RenderSize().Height;
                    }
                    if (contentHeight <= 0.0) {
                        contentHeight = 40.0;
                    }
                    xOffset = (float)(contentWidth / 2.0);
                    HWND hAnchor = FindSystemAnchorWnd();
                    if (hAnchor) {
                        HWND hParentTaskbar = GetAncestor(hAnchor, GA_ROOT);
                        if (!hParentTaskbar) hParentTaskbar = FindWindowW(L"Shell_TrayWnd", NULL);
                        if (hParentTaskbar) {
                            RECT trayRect;
                            GetWindowRect(hParentTaskbar, &trayRect);
                            HMONITOR hMon = MonitorFromRect(&trayRect, MONITOR_DEFAULTTONEAREST);
                            MONITORINFO mi = { sizeof(mi) };
                            if (GetMonitorInfoW(hMon, &mi)) {
                                int trayWidth = trayRect.right - trayRect.left;
                                int trayHeight = trayRect.bottom - trayRect.top;
                                bool isHorizontal = trayWidth > trayHeight;
                                double panelWidth = (g_forecastDaysFetch >= 7) ? 570.0 : ((g_forecastDaysFetch == 6) ? 492.0 : 336.0);
                                double panelHeight = CalculatePanelHeight();
                                // Transform button position to screen coordinates via XAML island reference
                                winrt::Windows::Foundation::Point rootPt{ 0, 0 };
                                try {
                                    auto transform = buttonElement.TransformToVisual(nullptr);
                                    if (transform) {
                                        rootPt = transform.TransformPoint(winrt::Windows::Foundation::Point{ 0, 0 });
                                    }
                                }
                                catch (...) {}
                                HWND hIsland = FindXamlIslandWindow(hParentTaskbar);
                                RECT islandRect = { 0 };
                                if (hIsland && GetWindowRect(hIsland, &islandRect)) {
                                    // Successfully retrieved island window rect
                                }
                                else {
                                    islandRect = trayRect; // Fallback to parent taskbar rect
                                }
                                // Get DPI scaling factor
                                HWND hDpiWnd = hIsland ? hIsland : hParentTaskbar;
                                UINT dpi = 96;
                                if (hDpiWnd) {
                                    dpi = GetDpiForWindow(hDpiWnd);
                                }
                                if (dpi == 0) dpi = 96;
                                double scale = (double)dpi / 96.0;
                                // Convert all physical coordinates to logical coordinates (effective pixels)
                                double workLeft = mi.rcWork.left / scale;
                                double workRight = mi.rcWork.right / scale;
                                double workTop = mi.rcWork.top / scale;
                                double workBottom = mi.rcWork.bottom / scale;
                                double islandLeft = islandRect.left / scale;
                                double islandTop = islandRect.top / scale;
                                double buttonScreenX = islandLeft + rootPt.X;
                                double buttonScreenY = islandTop + rootPt.Y;
                                double buttonCenterX = buttonScreenX + (contentWidth / 2.0);
                                if (isHorizontal) {
                                    if (trayRect.top <= mi.rcMonitor.top + (mi.rcMonitor.bottom - mi.rcMonitor.top) / 2) {
                                        placement = winrt::Windows::UI::Xaml::Controls::Primitives::FlyoutPlacementMode::Bottom;
                                        yOffset = (float)(workTop - buttonScreenY + 12.0);
                                    }
                                    else {
                                        placement = winrt::Windows::UI::Xaml::Controls::Primitives::FlyoutPlacementMode::Top;
                                        yOffset = (float)(workBottom - buttonScreenY - 12.0);
                                    }
                                    // Horizontally center the flyout on the button, then constrain to screen bounds
                                    double leftOverlap = (workLeft + 16.0) - (buttonCenterX - (panelWidth / 2.0));
                                    double rightOverlap = (buttonCenterX + (panelWidth / 2.0)) - (workRight - 16.0);
                                    double deltaX = 0.0;
                                    if (leftOverlap > 0.0) {
                                        deltaX = leftOverlap;
                                    }
                                    else if (rightOverlap > 0.0) {
                                        deltaX = -rightOverlap;
                                    }
                                    xOffset = (float)((contentWidth / 2.0) + deltaX);
                                }
                                else {
                                    if (trayRect.left <= mi.rcMonitor.left + (mi.rcMonitor.right - mi.rcMonitor.left) / 2) {
                                        placement = winrt::Windows::UI::Xaml::Controls::Primitives::FlyoutPlacementMode::RightEdgeAlignedTop;
                                        xOffset = (float)(workLeft - buttonScreenX + 12.0);
                                    }
                                    else {
                                        placement = winrt::Windows::UI::Xaml::Controls::Primitives::FlyoutPlacementMode::LeftEdgeAlignedTop;
                                        xOffset = (float)(workRight - buttonScreenX - 12.0);
                                    }
                                    // Align top of flyout with top of button (expand downwards)
                                    // With *EdgeAlignedTop, the yOffset is applied relative to the top edge alignment.
                                    // We start with 0 offset.
                                    double desiredYOffset = 0.0;
                                    
                                    // Constrain to screen bounds
                                    double flyoutTopScreen = buttonScreenY + desiredYOffset;
                                    double flyoutBottomScreen = flyoutTopScreen + panelHeight;
                                    
                                    double deltaY = 0.0;
                                    if (flyoutTopScreen < workTop + 16.0) {
                                        deltaY = (workTop + 16.0) - flyoutTopScreen;
                                    } else if (flyoutBottomScreen > workBottom - 16.0) {
                                        deltaY = (workBottom - 16.0) - flyoutBottomScreen;
                                    }
                                    
                                    yOffset = (float)(desiredYOffset + deltaY);
                                }
                            }
                        }
                    }
                    flyout.Placement(placement);
                    showOptions.Placement(placement);
                    showOptions.Position(winrt::Windows::Foundation::Point{ xOffset, yOffset });
                    flyout.ShowAt(buttonElement, showOptions);
                }
                catch (...) {
                    flyout.ShowAt(buttonElement);
                }
            };
            buttonElement.Click([showFlyoutLocal](auto const&, auto const&) mutable {
                showFlyoutLocal();
            });
            try {
                buttonElement.PointerEntered([iconWrapperTrans](auto const& sender, auto const&) {
                    g_btnIsHovered = true;
                    if (auto btn = sender.template try_as<winrt::Windows::UI::Xaml::Controls::Button>()) {
                        UpdateWeatherButtonVisuals(btn, g_btnIsHovered, g_btnIsPressed, g_win11FlyoutIsOpen);
                    }
                    if (g_animateIcons && iconWrapperTrans) {
                        Storyboard enterSb;
                        CubicEase ease;
                        ease.EasingMode(EasingMode::EaseOut);
                        DoubleAnimation sXAnim;
                        sXAnim.EnableDependentAnimation(true);
                        Storyboard::SetTarget(sXAnim, iconWrapperTrans);
                        Storyboard::SetTargetProperty(sXAnim, L"ScaleX");
                        sXAnim.To(1.0);
                        sXAnim.Duration(winrt::Windows::UI::Xaml::DurationHelper::FromTimeSpan(std::chrono::milliseconds(140)));
                        sXAnim.EasingFunction(ease);
                        enterSb.Children().Append(sXAnim);
                        DoubleAnimation sYAnim;
                        sYAnim.EnableDependentAnimation(true);
                        Storyboard::SetTarget(sYAnim, iconWrapperTrans);
                        Storyboard::SetTargetProperty(sYAnim, L"ScaleY");
                        sYAnim.To(1.0);
                        sYAnim.Duration(winrt::Windows::UI::Xaml::DurationHelper::FromTimeSpan(std::chrono::milliseconds(140)));
                        sYAnim.EasingFunction(ease);
                        enterSb.Children().Append(sYAnim);
                        enterSb.Begin();
                    }
                });
                buttonElement.PointerExited([iconWrapperTrans](auto const& sender, auto const&) {
                    g_btnIsHovered = false;
                    if (auto btn = sender.template try_as<winrt::Windows::UI::Xaml::Controls::Button>()) {
                        UpdateWeatherButtonVisuals(btn, g_btnIsHovered, g_btnIsPressed, g_win11FlyoutIsOpen);
                    }
                    if (g_animateIcons && iconWrapperTrans) {
                        Storyboard exitSb;
                        CubicEase ease;
                        ease.EasingMode(EasingMode::EaseOut);
                        DoubleAnimation sXAnim;
                        sXAnim.EnableDependentAnimation(true);
                        Storyboard::SetTarget(sXAnim, iconWrapperTrans);
                        Storyboard::SetTargetProperty(sXAnim, L"ScaleX");
                        sXAnim.To(1.0 / 1.18);
                        sXAnim.Duration(winrt::Windows::UI::Xaml::DurationHelper::FromTimeSpan(std::chrono::milliseconds(180)));
                        sXAnim.EasingFunction(ease);
                        exitSb.Children().Append(sXAnim);
                        DoubleAnimation sYAnim;
                        sYAnim.EnableDependentAnimation(true);
                        Storyboard::SetTarget(sYAnim, iconWrapperTrans);
                        Storyboard::SetTargetProperty(sYAnim, L"ScaleY");
                        sYAnim.To(1.0 / 1.18);
                        sYAnim.Duration(winrt::Windows::UI::Xaml::DurationHelper::FromTimeSpan(std::chrono::milliseconds(180)));
                        sYAnim.EasingFunction(ease);
                        exitSb.Children().Append(sYAnim);
                        exitSb.Begin();
                    }
                });
                buttonElement.PointerPressed([](auto const& sender, winrt::Windows::UI::Xaml::Input::PointerRoutedEventArgs const&) {
                    if (g_modUnloaded) return;
                    g_flyoutWasOpenOnPointerPressed = g_win11FlyoutIsOpen;
                    g_btnIsPressed = true;
                    if (auto btn = sender.template try_as<winrt::Windows::UI::Xaml::Controls::Button>()) {
                        UpdateWeatherButtonVisuals(btn, g_btnIsHovered, g_btnIsPressed, g_win11FlyoutIsOpen);
                    }
                });
                buttonElement.PointerReleased([](auto const& sender, auto const&) {
                    g_btnIsPressed = false;
                    if (auto btn = sender.template try_as<winrt::Windows::UI::Xaml::Controls::Button>()) {
                        UpdateWeatherButtonVisuals(btn, g_btnIsHovered, g_btnIsPressed, g_win11FlyoutIsOpen);
                    }
                });
                buttonElement.PointerCanceled([](auto const& sender, auto const&) {
                    g_btnIsPressed = false;
                    if (auto btn = sender.template try_as<winrt::Windows::UI::Xaml::Controls::Button>()) {
                        UpdateWeatherButtonVisuals(btn, g_btnIsHovered, g_btnIsPressed, g_win11FlyoutIsOpen);
                    }
                });
            } catch (...) {}
            
            g_showWin11Flyout = showFlyoutLocal;
            // Intercept pointer/click events on weatherGrid so they do not bubble up
            // to the widgets button or other native parent containers.
            try {
                weatherGrid.PointerPressed([](auto const&, winrt::Windows::UI::Xaml::Input::PointerRoutedEventArgs const& e) {
                    try { e.Handled(true); } catch (...) {}
                });
                weatherGrid.PointerReleased([](auto const&, winrt::Windows::UI::Xaml::Input::PointerRoutedEventArgs const& e) {
                    try { e.Handled(true); } catch (...) {}
                });
                weatherGrid.Tapped([](auto const&, winrt::Windows::UI::Xaml::Input::TappedRoutedEventArgs const& e) {
                    try { e.Handled(true); } catch (...) {}
                });
            }
            catch (...) {}
        }
        weatherGrid.Children().Append(buttonElement);
        weatherGrid.Tag(winrt::box_value(isHorizontal));
    }
    catch (winrt::hresult_error const& e) {
        Wh_Log(L"[Wh_WeatherHost] Weather button construction failed: 0x%08X %s",
               (unsigned)e.code().value, e.message().c_str());
    }
    catch (...) {
        Wh_Log(L"[Wh_WeatherHost] Weather button construction failed: unknown exception");
    }
}
// Size notifications can arrive repeatedly while Explorer changes orientation.
// Rebuild only when the orientation changes, and never update a detached host.
void RefreshInjectedWeatherAfterResize(Grid weatherGrid) {
    if (!weatherGrid || g_modUnloaded) return;
    try {
        if (!weatherGrid.Parent()) return;
        UpdateInjectedWeatherLayout(weatherGrid);
        HWND anchor = FindSystemAnchorWnd();
        HWND taskbar = anchor ? GetAncestor(anchor, GA_ROOT) : nullptr;
        if (!taskbar) taskbar = FindWindowW(L"Shell_TrayWnd", nullptr);
        RECT rect{};
        if (!taskbar || !GetWindowRect(taskbar, &rect)) return;
        bool horizontal = (rect.right - rect.left) > (rect.bottom - rect.top);
        auto previous = weatherGrid.Tag();
        if (previous && winrt::unbox_value<bool>(previous) == horizontal) return;
        auto weather = GetCurrentWeatherSnapshot();
        UpdateWeatherXamlElements(weatherGrid, weather.temp, weather.icon,
                                  weather.condition, weather.acquired);
    } catch (...) {}
}

// Dynamic WinHttp pull system
inline bool BackgroundWorkShouldStop() {
    return g_modUnloaded.load(std::memory_order_relaxed) ||
        g_bThreadShouldTerm.load(std::memory_order_relaxed);
}

std::wstring RequestHttpData(const std::wstring& host,
    const std::wstring& path,
    bool secure,
    DWORD* httpStatusCode,
    DWORD* winHttpError,
    bool* responseComplete) {
    if (httpStatusCode) *httpStatusCode = 0;
    if (winHttpError) *winHttpError = ERROR_SUCCESS;
    if (responseComplete) *responseComplete = false;
    if (BackgroundWorkShouldStop()) return L"";

    HINTERNET hSession = nullptr;
    HINTERNET hConnect = nullptr;
    HINTERNET hRequest = nullptr;
    auto cleanup = [&]() {
        if (hRequest) { WinHttpCloseHandle(hRequest); hRequest = nullptr; }
        if (hConnect) { WinHttpCloseHandle(hConnect); hConnect = nullptr; }
        if (hSession) { WinHttpCloseHandle(hSession); hSession = nullptr; }
    };

    hSession = WinHttpOpen(L"EP_WeatherHost/2.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0);
    if (!hSession) {
        if (winHttpError) *winHttpError = GetLastError();
        return L"";
    }

    // Keep every blocking operation bounded so unload can safely join the worker.
    WinHttpSetTimeouts(hSession, 6000, 6000, 10000, 10000);
    if (BackgroundWorkShouldStop()) { cleanup(); return L""; }

    INTERNET_PORT port = secure ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;
    hConnect = WinHttpConnect(hSession, host.c_str(), port, 0);
    if (!hConnect || BackgroundWorkShouldStop()) {
        if (!hConnect && winHttpError) *winHttpError = GetLastError();
        cleanup();
        return L"";
    }

    DWORD flags = secure ? WINHTTP_FLAG_SECURE : 0;
    hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(), nullptr,
        WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    if (!hRequest || BackgroundWorkShouldStop()) {
        if (!hRequest && winHttpError) *winHttpError = GetLastError();
        cleanup();
        return L"";
    }

    BOOL ok = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS,
        0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    if (!ok || BackgroundWorkShouldStop()) {
        if (!ok && winHttpError) *winHttpError = GetLastError();
        cleanup();
        return L"";
    }

    ok = WinHttpReceiveResponse(hRequest, nullptr);
    if (!ok || BackgroundWorkShouldStop()) {
        if (!ok && winHttpError) *winHttpError = GetLastError();
        cleanup();
        return L"";
    }

    DWORD statusCode = 0;
    DWORD statusSize = sizeof(statusCode);
    if (WinHttpQueryHeaders(hRequest,
            WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusSize,
            WINHTTP_NO_HEADER_INDEX)) {
        if (httpStatusCode) *httpStatusCode = statusCode;
    } else if (winHttpError) {
        *winHttpError = GetLastError();
    }

    // Accumulate raw bytes first. Decoding each WinHTTP chunk separately can split
    // a multi-byte UTF-8 character across chunk boundaries and corrupt JSON text.
    std::string responseBytes;
    bool downloadComplete = false;
    for (;;) {
        if (BackgroundWorkShouldStop()) { cleanup(); return L""; }

        DWORD available = 0;
        if (!WinHttpQueryDataAvailable(hRequest, &available)) {
            if (winHttpError) *winHttpError = GetLastError();
            break;
        }
        if (available == 0) {
            downloadComplete = true;
            break;
        }

        size_t oldSize = responseBytes.size();
        responseBytes.resize(oldSize + available);
        DWORD downloaded = 0;
        if (!WinHttpReadData(hRequest, responseBytes.data() + oldSize,
                available, &downloaded)) {
            if (winHttpError) *winHttpError = GetLastError();
            responseBytes.resize(oldSize);
            break;
        }
        responseBytes.resize(oldSize + downloaded);
        if (downloaded == 0) break;
    }

    cleanup();
    if (responseComplete) *responseComplete = downloadComplete;
    if (BackgroundWorkShouldStop() || responseBytes.empty()) return L"";

    int wlen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
        responseBytes.data(), static_cast<int>(responseBytes.size()), nullptr, 0);
    bool validUtf8 = wlen > 0;
    if (wlen <= 0) {
        // Some endpoints are permissive about malformed UTF-8. Fall back to the
        // replacement-character behavior rather than dropping the whole response.
        wlen = MultiByteToWideChar(CP_UTF8, 0, responseBytes.data(),
            static_cast<int>(responseBytes.size()), nullptr, 0);
    }
    if (wlen <= 0) return L"";

    std::wstring response(static_cast<size_t>(wlen), L'\0');
    if (MultiByteToWideChar(CP_UTF8, 0, responseBytes.data(),
            static_cast<int>(responseBytes.size()), response.data(), wlen) <= 0) {
        if (responseComplete) *responseComplete = false;
        return L"";
    }
    if (responseComplete) *responseComplete = downloadComplete && validUtf8;
    return response;
}

HBITMAP RenderWin10TaskbarSvgBitmap(
    const std::wstring& svgText,
    UINT width,
    UINT height) {
    if (svgText.empty() || width == 0 || height == 0) return nullptr;

    int byteCount = WideCharToMultiByte(
        CP_UTF8, 0, svgText.data(), static_cast<int>(svgText.size()),
        nullptr, 0, nullptr, nullptr);
    if (byteCount <= 0) return nullptr;

    std::string svgUtf8(static_cast<size_t>(byteCount), '\0');
    if (WideCharToMultiByte(
            CP_UTF8, 0, svgText.data(), static_cast<int>(svgText.size()),
            svgUtf8.data(), byteCount, nullptr, nullptr) <= 0) {
        return nullptr;
    }

    winrt::com_ptr<IStream> svgStream;
    svgStream.attach(SHCreateMemStream(
        reinterpret_cast<const BYTE*>(svgUtf8.data()),
        static_cast<UINT>(svgUtf8.size())));
    if (!svgStream) return nullptr;

    BITMAPINFO bitmapInfo{};
    bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmapInfo.bmiHeader.biWidth = static_cast<LONG>(width);
    bitmapInfo.bmiHeader.biHeight = -static_cast<LONG>(height);
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biBitCount = 32;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    HBITMAP bitmap = CreateDIBSection(
        nullptr, &bitmapInfo, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!bitmap || !bits) {
        if (bitmap) DeleteObject(bitmap);
        return nullptr;
    }

    HDC memoryDc = CreateCompatibleDC(nullptr);
    if (!memoryDc) {
        DeleteObject(bitmap);
        return nullptr;
    }
    HGDIOBJ oldBitmap = SelectObject(memoryDc, bitmap);

    winrt::com_ptr<ID2D1Factory> factory;
    HRESULT hr = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED, factory.put());
    winrt::com_ptr<ID2D1DCRenderTarget> target;
    if (SUCCEEDED(hr)) {
        D2D1_RENDER_TARGET_PROPERTIES properties = D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
            D2D1::PixelFormat(
                DXGI_FORMAT_B8G8R8A8_UNORM,
                D2D1_ALPHA_MODE_PREMULTIPLIED));
        hr = factory->CreateDCRenderTarget(&properties, target.put());
    }

    RECT targetRect{ 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
    if (SUCCEEDED(hr)) hr = target->BindDC(memoryDc, &targetRect);

    winrt::com_ptr<ID2D1DeviceContext5> deviceContext;
    if (SUCCEEDED(hr)) {
        hr = target->QueryInterface(
            __uuidof(ID2D1DeviceContext5), deviceContext.put_void());
    }

    winrt::com_ptr<ID2D1SvgDocument> svgDocument;
    if (SUCCEEDED(hr)) {
        hr = deviceContext->CreateSvgDocument(
            svgStream.get(),
            D2D1::SizeF(static_cast<float>(width), static_cast<float>(height)),
            svgDocument.put());
    }

    if (SUCCEEDED(hr)) {
        target->BeginDraw();
        target->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));
        deviceContext->DrawSvgDocument(svgDocument.get());
        hr = target->EndDraw();
    }

    SelectObject(memoryDc, oldBitmap);
    DeleteDC(memoryDc);
    if (FAILED(hr)) {
        DeleteObject(bitmap);
        return nullptr;
    }
    return bitmap;
}

void ClearWin10TaskbarSvgCache() {
    std::lock_guard<std::mutex> lock(g_win10TaskbarSvgMutex);
    if (g_win10TaskbarSvgBitmap) {
        DeleteObject(g_win10TaskbarSvgBitmap);
        g_win10TaskbarSvgBitmap = nullptr;
    }
    g_win10TaskbarSvgUrl.clear();
}

void RefreshWin10TaskbarSvgCache(const std::wstring& iconUrl) {
    if (ShouldUseXamlTaskbar() || g_weatherStyle != 2 || iconUrl.empty() ||
        BackgroundWorkShouldStop()) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(g_win10TaskbarSvgMutex);
        if (g_win10TaskbarSvgBitmap && g_win10TaskbarSvgUrl == iconUrl) return;
    }

    constexpr wchar_t kMsnAssetsPrefix[] = L"https://assets.msn.com";
    constexpr size_t kMsnAssetsPrefixLength =
        (sizeof(kMsnAssetsPrefix) / sizeof(kMsnAssetsPrefix[0])) - 1;
    if (iconUrl.rfind(kMsnAssetsPrefix, 0) != 0) return;

    DWORD statusCode = 0;
    DWORD winHttpError = ERROR_SUCCESS;
    std::wstring svgText = RequestHttpData(
        L"assets.msn.com", iconUrl.substr(kMsnAssetsPrefixLength), true,
        &statusCode, &winHttpError);
    if (BackgroundWorkShouldStop()) return;
    if (statusCode != 200 || svgText.find(L"<svg") == std::wstring::npos) {
        if (g_debugLogs) {
            Wh_Log(
                L"[Wh_WeatherHost] Win10 taskbar SVG fetch failed: HTTP=%lu, WinHTTP=%lu",
                statusCode, winHttpError);
        }
        return;
    }

    HBITMAP newBitmap = RenderWin10TaskbarSvgBitmap(svgText, 72, 72);
    if (!newBitmap) {
        if (g_debugLogs) {
            Wh_Log(L"[Wh_WeatherHost] Win10 taskbar SVG rasterization failed");
        }
        return;
    }

    auto currentWeather = GetCurrentWeatherSnapshot();
    if (g_weatherStyle != 2 || currentWeather.iconUrl != iconUrl ||
        BackgroundWorkShouldStop()) {
        DeleteObject(newBitmap);
        return;
    }

    {
        std::lock_guard<std::mutex> lock(g_win10TaskbarSvgMutex);
        if (g_win10TaskbarSvgBitmap) DeleteObject(g_win10TaskbarSvgBitmap);
        g_win10TaskbarSvgBitmap = newBitmap;
        g_win10TaskbarSvgUrl = iconUrl;
    }
    if (g_debugLogs) {
        Wh_Log(L"[Wh_WeatherHost] Win10 taskbar SVG cached for current condition");
    }
    HWND weatherWindow = g_hSubclassedWnd;
    if (weatherWindow && IsWindow(weatherWindow)) {
        InvalidateRect(weatherWindow, nullptr, FALSE);
    }
}
// JSON helpers backed by Windows.Data.Json (available since Windows 10 10240).
// The recursive lookup preserves the old helper behavior for responses where the
// requested property is nested (for example Open-Meteo geocoding "results").
static bool FindJsonValueRecursive(
    winrt::Windows::Data::Json::IJsonValue const& value,
    winrt::hstring const& key,
    winrt::Windows::Data::Json::IJsonValue& found) {
    using namespace winrt::Windows::Data::Json;
    if (!value) return false;

    try {
        switch (value.ValueType()) {
        case JsonValueType::Object: {
            auto object = value.GetObject();
            if (object.HasKey(key)) {
                found = object.GetNamedValue(key);
                return true;
            }
            for (auto const& pair : object) {
                if (FindJsonValueRecursive(pair.Value(), key, found)) {
                    return true;
                }
            }
            break;
        }
        case JsonValueType::Array: {
            auto array = value.GetArray();
            for (auto const& item : array) {
                if (FindJsonValueRecursive(item, key, found)) {
                    return true;
                }
            }
            break;
        }
        default:
            break;
        }
    } catch (...) {}
    return false;
}

static std::wstring JsonScalarToString(
    winrt::Windows::Data::Json::IJsonValue const& value) {
    using namespace winrt::Windows::Data::Json;
    if (!value) return L"";
    try {
        switch (value.ValueType()) {
        case JsonValueType::String:
            return std::wstring(value.GetString());
        case JsonValueType::Number:
        case JsonValueType::Boolean:
            return std::wstring(value.Stringify());
        case JsonValueType::Null:
            return L"";
        default:
            return std::wstring(value.Stringify());
        }
    } catch (...) {
        return L"";
    }
}

std::wstring ExtractJSONObject(const std::wstring& json,
    const std::wstring& key) {
    using namespace winrt::Windows::Data::Json;
    try {
        IJsonValue root = JsonValue::Parse(json);
        IJsonValue found = nullptr;
        if (FindJsonValueRecursive(root, winrt::hstring(key), found) &&
            found && found.ValueType() == JsonValueType::Object) {
            return std::wstring(found.Stringify());
        }
    } catch (...) {}
    return L"";
}

std::wstring ExtractJSONValue(const std::wstring& json,
    const std::wstring& key) {
    using namespace winrt::Windows::Data::Json;
    try {
        IJsonValue root = JsonValue::Parse(json);
        IJsonValue found = nullptr;
        if (FindJsonValueRecursive(root, winrt::hstring(key), found)) {
            return JsonScalarToString(found);
        }
    } catch (...) {}
    return L"";
}

std::wstring ExtractJSONArray(const std::wstring& json,
    const std::wstring& key) {
    using namespace winrt::Windows::Data::Json;
    try {
        IJsonValue root = JsonValue::Parse(json);
        IJsonValue found = nullptr;
        if (FindJsonValueRecursive(root, winrt::hstring(key), found) &&
            found && found.ValueType() == JsonValueType::Array) {
            return std::wstring(found.Stringify());
        }
    } catch (...) {}
    return L"";
}

std::vector<std::wstring> ParseJSONArray(const std::wstring& arrayStr) {
    using namespace winrt::Windows::Data::Json;
    std::vector<std::wstring> results;
    try {
        IJsonValue root = JsonValue::Parse(arrayStr);
        if (!root || root.ValueType() != JsonValueType::Array) return results;
        for (auto const& item : root.GetArray()) {
            results.push_back(JsonScalarToString(item));
        }
    } catch (...) {}
    return results;
}

bool ParseGeoMetUtc(const std::wstring& iso, int64_t& seconds) {
    int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
    if (iso.size() != 20 || iso[19] != L'Z' ||
        swscanf(iso.c_str(), L"%4d-%2d-%2dT%2d:%2d:%2dZ",
            &year, &month, &day, &hour, &minute, &second) != 6 ||
        year < 1970 || year > 2100 || month < 1 || month > 12 ||
        day < 1 || day > 31 || hour > 23 || minute > 59 || second > 59) return false;
    SYSTEMTIME systemTime{};
    systemTime.wYear = static_cast<WORD>(year);
    systemTime.wMonth = static_cast<WORD>(month);
    systemTime.wDay = static_cast<WORD>(day);
    systemTime.wHour = static_cast<WORD>(hour);
    systemTime.wMinute = static_cast<WORD>(minute);
    systemTime.wSecond = static_cast<WORD>(second);
    FILETIME fileTime{};
    if (!SystemTimeToFileTime(&systemTime, &fileTime)) return false;
    ULARGE_INTEGER ticks{};
    ticks.LowPart = fileTime.dwLowDateTime;
    ticks.HighPart = fileTime.dwHighDateTime;
    seconds = static_cast<int64_t>((ticks.QuadPart - 116444736000000000ULL) / 10000000ULL);
    return true;
}

std::wstring GeoMetDimension(const std::wstring& xml, const wchar_t* name,
    std::wstring* defaultValue = nullptr) {
    size_t position = 0;
    while ((position = xml.find(L"<Dimension", position)) != std::wstring::npos) {
        size_t tagEnd = xml.find(L'>', position);
        if (tagEnd == std::wstring::npos) break;
        std::wstring tag = xml.substr(position, tagEnd - position + 1);
        std::wstring attribute = std::wstring(L"name=\"") + name + L"\"";
        if (tag.find(attribute) != std::wstring::npos) {
            if (defaultValue) {
                size_t attr = tag.find(L"default=\"");
                if (attr != std::wstring::npos) {
                    attr += 9;
                    size_t close = tag.find(L'"', attr);
                    if (close != std::wstring::npos)
                        *defaultValue = tag.substr(attr, close - attr);
                }
            }
            size_t end = xml.find(L"</Dimension>", tagEnd + 1);
            if (end != std::wstring::npos) return xml.substr(tagEnd + 1, end - tagEnd - 1);
        }
        position = tagEnd + 1;
    }
    return L"";
}

std::vector<RadarFrame> ParseGeoMetForecastTimes(
    const std::wstring& timeDimension, const std::wstring& reference,
    int64_t now) {
    std::vector<RadarFrame> frames;
    size_t start = 0;
    while (start < timeDimension.size() && frames.size() < 25) {
        size_t end = timeDimension.find(L',', start);
        std::wstring item = timeDimension.substr(start,
            end == std::wstring::npos ? end : end - start);
        size_t first = item.find(L"20");
        if (first != std::wstring::npos) item.erase(0, first);
        if (item.size() >= 20) {
            size_t slash = item.find(L'/');
            int64_t firstTime = 0;
            if (ParseGeoMetUtc(item.substr(0, 20), firstTime)) {
                if (slash == std::wstring::npos) {
                    if (firstTime >= now && firstTime <= now + 24 * 3600)
                        frames.push_back({firstTime, item.substr(0, 20), reference});
                } else if (slash + 21 < item.size()) {
                    int64_t lastTime = 0;
                    size_t secondSlash = item.find(L'/', slash + 1);
                    if (secondSlash != std::wstring::npos &&
                        ParseGeoMetUtc(item.substr(slash + 1, 20), lastTime)) {
                        std::wstring step = item.substr(secondSlash + 1);
                        int64_t interval = 0;
                        if (step.size() > 3 && step.substr(0, 2) == L"PT") {
                            wchar_t* rest = nullptr;
                            long value = wcstol(step.c_str() + 2, &rest, 10);
                            if (value > 0 && value <= 24 && rest && *rest == L'H')
                                interval = value * 3600;
                            else if (value > 0 && value <= 60 && rest && *rest == L'M')
                                interval = value * 60;
                        }
                        if (interval) {
                            int64_t t = firstTime;
                            if (t < now) t += ((now - t + interval - 1) / interval) * interval;
                            for (; t <= lastTime && t <= now + 24 * 3600 &&
                                    frames.size() < 25; t += interval) {
                                time_t unixTime = static_cast<time_t>(t);
                                struct tm utc{};
                                gmtime_s(&utc, &unixTime);
                                wchar_t iso[32]{};
                                wcsftime(iso, 32, L"%Y-%m-%dT%H:%M:%SZ", &utc);
                                frames.push_back({t, iso, reference});
                            }
                        }
                    }
                }
            }
        }
        if (end == std::wstring::npos) break;
        start = end + 1;
    }
    std::sort(frames.begin(), frames.end(), [](auto const& a, auto const& b) {
        return a.time < b.time;
    });
    frames.erase(std::unique(frames.begin(), frames.end(), [](auto const& a, auto const& b) {
        return a.time == b.time;
    }), frames.end());
    return frames;
}

void FetchRadarMetadata() {
    if (BackgroundWorkShouldStop()) return;
    DWORD status = 0;
    bool complete = false;
    std::wstring xml = RequestHttpData(L"geo.weather.gc.ca",
        L"/geomet?service=WMS&version=1.3.0&request=GetCapabilities&layer=GDPS_15km_Precip-Accum1h",
        true, &status, nullptr, &complete);
    if (BackgroundWorkShouldStop()) return;
    if (status != 200 || !complete || xml.empty()) {
        g_radarMetadataError = true;
        if (g_debugLogs) Wh_Log(L"[EP_WeatherHost] GeoMet capabilities unavailable: HTTP %lu, complete=%d",
            status, complete ? 1 : 0);
        return;
    }
    try {
        const size_t layerName = xml.find(L"<Name>GDPS_15km_Precip-Accum1h</Name>");
        const size_t layerStart = xml.rfind(L"<Layer", layerName);
        const size_t layerEnd = xml.find(L"</Layer>", layerName);
        if (layerName == std::wstring::npos || layerStart == std::wstring::npos ||
            layerEnd == std::wstring::npos) {
            g_radarMetadataError = true;
            if (g_debugLogs) Wh_Log(L"[EP_WeatherHost] GeoMet response did not contain the precipitation layer");
            return;
        }
        const std::wstring layerXml = xml.substr(layerStart, layerEnd - layerStart);
        const std::wstring times = GeoMetDimension(layerXml, L"time");
        std::wstring modelRun;
        GeoMetDimension(layerXml, L"reference_time", &modelRun);
        int64_t runTime = 0;
        if (!ParseGeoMetUtc(modelRun, runTime)) modelRun.clear();
        auto frames = ParseGeoMetForecastTimes(times, modelRun, static_cast<int64_t>(std::time(nullptr)));
        if (BackgroundWorkShouldStop()) return;
        if (frames.empty()) {
            g_radarMetadataError = true;
            if (g_debugLogs) Wh_Log(L"[EP_WeatherHost] GeoMet has no forecast hours in the next 24 hours");
            return;
        }

        EnterCriticalSection(&g_forecastLock);
        int oldIndex = g_radarCurrentFrame.load();
        int64_t oldTime = 0;
        if (oldIndex >= 0 && oldIndex < static_cast<int>(g_radarFrames.size()))
            oldTime = g_radarFrames[oldIndex].time;
        g_radarFrames = std::move(frames);
        int nearest = 0;
        if (oldTime > 0) {
            for (int i = 1; i < static_cast<int>(g_radarFrames.size()); ++i) {
                if (std::llabs(g_radarFrames[i].time - oldTime) <
                    std::llabs(g_radarFrames[nearest].time - oldTime)) nearest = i;
            }
        }
        g_radarCurrentFrame = nearest;
        LeaveCriticalSection(&g_forecastLock);
        g_radarMetadataError = false;
    } catch (...) {
        g_radarMetadataError = true;
        if (g_debugLogs) Wh_Log(L"[EP_WeatherHost] GeoMet capabilities parse failed");
    }
}
std::wstring FormatISOTimeTo12H(const std::wstring& isoStr) {
    size_t tPos = isoStr.find(L"T");
    std::wstring timePart = (tPos != std::wstring::npos) ? isoStr.substr(tPos + 1) : isoStr;
    int hour = 0, minute = 0;
    if (swscanf(timePart.c_str(), L"%d:%d", &hour, &minute) == 2) {
        int displayHour = hour % 12;
        if (displayHour == 0) displayHour = 12;
        wchar_t buf[32];
        swprintf(buf, 32, L"%d:%02d %s", displayHour, minute, (hour >= 12 ? L"PM" : L"AM"));
        return buf;
    }
    return isoStr;
}
std::wstring GetWindDirectionString(double degrees) {
    const wchar_t* directions[] = { L"N", L"NNE", L"NE", L"ENE", L"E", L"ESE", L"SE", L"SSE", L"S", L"SSW", L"SW", L"WSW", L"W", L"WNW", L"NW", L"NNW" };
    int index = (int)((degrees + 11.25) / 22.5) % 16;
    return directions[index];
}
// Sakamoto calendar calculator
std::wstring GetDayOfWeek(int year, int month, int day) {
    static int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
    if (month < 3)
        year -= 1;
    int dow =
        (year + year / 4 - year / 100 + year / 400 + t[month - 1] + day) % 7;
    const wchar_t* days[] = { L"Sun", L"Mon", L"Tue", L"Wed",
                             L"Thu", L"Fri", L"Sat" };
    return days[dow];
}
// Resolve weather codes
std::pair<std::wstring, std::wstring> GetCodeMapping(int code) {
    std::wstring icon = L"❓";
    std::wstring cond = L"Cloudy";
    if (g_weatherStyle == 0 || g_weatherStyle == 2 || g_weatherStyle == 3 || g_weatherStyle == 4) {  // Emoji or MSN/Fluent/Custom (Emoji fallback)
        if (code == 0) {
            icon = L"☀️";
            cond = L"Clear";
        }
        else if (code >= 1 && code <= 3) {
            icon = code == 1 ? L"🌤️" : (code == 2 ? L"⛅" : L"☁️");
            cond = code == 1 ? L"Mainly Clear"
                : (code == 2 ? L"Partly Cloudy" : L"Cloudy");
        }
        else if (code == 45 || code == 48) {
            icon = L"🌫️";
            cond = L"Foggy";
        }
        else if (code >= 51 && code <= 55) {
            icon = L"🌧️";
            cond = L"Drizzle";
        }
        else if (code == 56 || code == 57) {
            icon = L"🌧️";
            cond = L"Freezing Drizzle";
        }
        else if (code >= 61 && code <= 65) {
            icon = L"🌧️";
            cond = L"Rain";
        }
        else if (code == 66 || code == 67) {
            icon = L"🌧️";
            cond = L"Freezing Rain";
        }
        else if (code >= 71 && code <= 77) {
            icon = L"❄️";
            cond = L"Snow";
        }
        else if (code >= 80 && code <= 82) {
            icon = L"🌧️";
            cond = L"Showers";
        }
        else if (code == 85 || code == 86) {
            icon = L"❄️";
            cond = L"Snow Showers";
        }
        else if (code >= 95) {
            icon = L"⛈️";
            cond = L"Thunderstorm";
        }
        else {
            icon = L"⛅";
            cond = L"Cloudy";
        }
    }
    else { // Segoe MDL2 icon style
        if (code == 0) {
            icon = L"\u2600";  // Sunny
            cond = L"Clear";
        }
        else if (code >= 1 && code <= 3) {
            icon = code == 1 ? L"\u26C5" : (code == 2 ? L"\u26C5" : L"\u2601");
            cond = code == 1 ? L"Mainly Clear"
                : (code == 2 ? L"Partly Cloudy" : L"Cloudy");
        }
        else if (code == 45 || code == 48) {
            icon = L"\u2601";  // Fog
            cond = L"Foggy";
        }
        else if (code >= 51 && code <= 55) {
            icon = L"\u2614";  // Rain Umbrella
            cond = L"Drizzle";
        }
        else if (code == 56 || code == 57) {
            icon = L"\u2614";
            cond = L"Freezing Drizzle";
        }
        else if (code >= 61 && code <= 65) {
            icon = L"\u2614";
            cond = L"Rain";
        }
        else if (code == 66 || code == 67) {
            icon = L"\u2614";
            cond = L"Freezing Rain";
        }
        else if (code >= 71 && code <= 77) {
            icon = L"\u2744";  // Snowflake
            cond = L"Snow";
        }
        else if (code >= 80 && code <= 82) {
            icon = L"\u2614";  // Showers
            cond = L"Showers";
        }
        else if (code == 85 || code == 86) {
            icon = L"\u2744";
            cond = L"Flurries";
        }
        else if (code >= 95) {
            icon = L"\u26A1";  // Lightning
            cond = L"Storm";
        }
        else {
            icon = L"\u2601";  // Cloudy default
            cond = L"Cloudy";
        }
    }
    return { icon, cond };
}
void MapOpenMeteoCode(int code, int is_day) {
    std::lock_guard<std::mutex> stateLock(g_currentWeatherMutex);
    g_cachedWeatherCode = code;
    g_cachedIsDay = is_day;
    g_cachedIconUrl = GetWeatherIconPathOrUrl(code, is_day != 0);
    if (g_weatherStyle == 0 || g_weatherStyle == 2 || g_weatherStyle == 3 || g_weatherStyle == 4) {  // Emoji Style or MSN/Fluent/Custom
        if (code == 0) {
            g_cachedIcon = is_day ? L"☀️" : L"🌙";
            g_cachedCondition = L"Clear";
        }
        else if (code >= 1 && code <= 3) {
            g_cachedIcon = code == 1 ? L"🌤️" : (code == 2 ? L"⛅" : L"☁️");
            g_cachedCondition =
                code == 1 ? L"Mainly Clear"
                : (code == 2 ? L"Partly Cloudy" : L"Cloudy");
        }
        else if (code == 45 || code == 48) {
            g_cachedIcon = L"🌫️";
            g_cachedCondition = L"Foggy";
        }
        else if (code >= 51 && code <= 55) {
            g_cachedIcon = L"🌧️";
            g_cachedCondition = L"Drizzle";
        }
        else if (code == 56 || code == 57) {
            g_cachedIcon = L"🌧️";
            g_cachedCondition = L"Freezing Drizzle";
        }
        else if (code >= 61 && code <= 65) {
            g_cachedIcon = L"🌧️";
            g_cachedCondition = L"Rain";
        }
        else if (code == 66 || code == 67) {
            g_cachedIcon = L"🌧️";
            g_cachedCondition = L"Freezing Rain";
        }
        else if (code >= 71 && code <= 77) {
            g_cachedIcon = L"❄️";
            g_cachedCondition = L"Snow";
        }
        else if (code >= 80 && code <= 82) {
            g_cachedIcon = L"🌧️";
            g_cachedCondition = L"Showers";
        }
        else if (code == 85 || code == 86) {
            g_cachedIcon = L"❄️";
            g_cachedCondition = L"Snow Showers";
        }
        else if (code >= 95) {
            g_cachedIcon = L"⛈️";
            g_cachedCondition = L"Thunderstorm";
        }
        else {
            g_cachedIcon = L"⛅";
            g_cachedCondition = L"Cloudy";
        }
    }
    else { // Segoe MDL2 icon style
        if (code == 0) {
            g_cachedIcon = is_day ? L"\u2600" : L"\u263D";
            g_cachedCondition = L"Clear";
        }
        else if (code >= 1 && code <= 3) {
            g_cachedIcon = code == 1 ? L"\u26C5" : (code == 2 ? L"\u26C5" : L"\u2601");
            g_cachedCondition =
                code == 1 ? L"Mainly Clear"
                : (code == 2 ? L"Partly Cloudy" : L"Cloudy");
        }
        else if (code == 45 || code == 48) {
            g_cachedIcon = L"\u2601";
            g_cachedCondition = L"Foggy";
        }
        else if (code >= 51 && code <= 55) {
            g_cachedIcon = L"\u2614";
            g_cachedCondition = L"Drizzle";
        }
        else if (code == 56 || code == 57) {
            g_cachedIcon = L"\u2614";
            g_cachedCondition = L"Freezing Drizzle";
        }
        else if (code >= 61 && code <= 65) {
            g_cachedIcon = L"\u2614";
            g_cachedCondition = L"Rain";
        }
        else if (code == 66 || code == 67) {
            g_cachedIcon = L"\u2614";
            g_cachedCondition = L"Freezing Rain";
        }
        else if (code >= 71 && code <= 77) {
            g_cachedIcon = L"\u2744";
            g_cachedCondition = L"Snow";
        }
        else if (code >= 80 && code <= 82) {
            g_cachedIcon = L"\u2614";
            g_cachedCondition = L"Showers";
        }
        else if (code == 85 || code == 86) {
            g_cachedIcon = L"\u2744";
            g_cachedCondition = L"Snow Showers";
        }
        else if (code >= 95) {
            g_cachedIcon = L"\u26A1";
            g_cachedCondition = L"Thunderstorm";
        }
        else {
            g_cachedIcon = L"\u2601";
            g_cachedCondition = L"Cloudy";
        }
    }
}
bool IsWindows11() {
    static int build = 0;
    if (build == 0) {
        OSVERSIONINFOEXW osvi = { sizeof(osvi) };
        osvi.dwBuildNumber = 0;
        using RtlGetVersion_t = LONG(WINAPI*)(PRTL_OSVERSIONINFOEXW);
        HMODULE hNt = GetModuleHandleW(L"ntdll.dll");
        if (hNt) {
            auto pfn = (RtlGetVersion_t)GetProcAddress(hNt, "RtlGetVersion");
            if (pfn) {
                pfn(&osvi);
            }
        }
        build = osvi.dwBuildNumber;
    }
    return build >= 22000;
}
bool ShouldUseXamlTaskbar() {
    if (!IsWindows11()) {
        return false;
    }
    // Check if the classic tray clock window is present (implying ExplorerPatcher classic Taskbar is active)
    HWND hShell = FindWindowW(L"Shell_TrayWnd", NULL);
    if (hShell) {
        HWND hTray = FindWindowExW(hShell, NULL, L"TrayNotifyWnd", NULL);
        if (hTray) {
            HWND hClock = FindWindowExW(hTray, NULL, L"TrayClockWClass", NULL);
            if (hClock) {
                return false; // Found classic clock window, use GDI taskbar mode!
            }
        }
    }
    return true; // Modern Windows 11 XAML taskbar
}
bool IsShellProcess() {
    DWORD currentPid = GetCurrentProcessId();
    DWORD shellPid = 0;
    HWND hShell = GetShellWindow();
    if (hShell) {
        GetWindowThreadProcessId(hShell, &shellPid);
        if (shellPid == currentPid)
            return true;
    }
    HWND hTray = FindWindowW(L"Shell_TrayWnd", NULL);
    if (hTray) {
        DWORD trayPid = 0;
        GetWindowThreadProcessId(hTray, &trayPid);
        if (trayPid == currentPid)
            return true;
    }
    LPWSTR lpCmdLine = GetCommandLineW();
    if (lpCmdLine) {
        if (wcsstr(lpCmdLine, L"-separate") || wcsstr(lpCmdLine, L"/factory")) {
            return false;
        }
    }
    if (hShell || hTray) {
        return false;
    }
    return true;
}
// Queue XAML widget re-fill on UI thread safely from background weather update
// loops
void QueueWeatherUpdateOnUIThread() {
    if (g_modUnloaded) return;
    auto currentWeather = GetCurrentWeatherSnapshot();
    std::wstring temp = currentWeather.temp;
    std::wstring icon = currentWeather.icon;
    std::wstring condition = currentWeather.condition;
    bool acquired = currentWeather.acquired;
    if (!IsWindows11()) {
        HWND hClock = FindSystemClockWnd();
        if (hClock) {
            InvalidateRect(hClock, NULL, TRUE);
            SetWindowPos(
                hClock, NULL, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
        }
        if (g_hSubclassedWnd && IsWindow(g_hSubclassedWnd)) {
            InvalidateClockParentRegion(g_hSubclassedWnd);
            SetWindowPos(g_hSubclassedWnd, NULL, 0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
            RedrawWindow(g_hSubclassedWnd, NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW);
            // If the forecast popup is currently visible, trigger a layout re-population asynchronously!
            if (g_win10PopupVisible.load(std::memory_order_acquire)) {
                PostMessageW(g_hSubclassedWnd, WM_USER + 4244, 0, 0);
            }
        }
    }
    Grid targetGrid = nullptr;
    {
        std::lock_guard<std::mutex> lock(g_weatherGridMutex);
        for (auto& weakG : g_injectedWeatherGrids) {
            if (auto g = weakG.get()) {
                targetGrid = g;
                break;
            }
        }
    }
    if (targetGrid) {
        try {
            auto dispatcher = targetGrid.Dispatcher();
            if (dispatcher && !g_modUnloaded) {
                auto weakGrid = winrt::make_weak(targetGrid);
                g_pendingRunAsync++;
                try {
                    dispatcher.RunAsync(
                    winrt::Windows::UI::Core::CoreDispatcherPriority::Normal,
                    [weakGrid, temp, icon, condition, acquired]() {
struct AutoDecrement { ~AutoDecrement() { g_pendingRunAsync--; } } autoDec;


                        if (g_modUnloaded) return;
                        try {
                            if (auto grid = weakGrid.get()) {
                                UpdateWeatherXamlElements(grid, temp, icon, condition, acquired);
                            }
                        }
                        catch (...) {}
                    });
                } catch (...) {
                    g_pendingRunAsync--;
                    throw;
                }
            }
        }
        catch (...) {}
    }
}
// Background meteorological synchronizer thread
DWORD WINAPI QueryWeatherPipeline(LPVOID lpParam) {
    winrt::init_apartment();
    if (g_debugLogs)
        Wh_Log(L"[EP_WeatherHost] Background thread PIPELINE started");
    if (g_hForceUpdateEvent) {
        ResetEvent(g_hForceUpdateEvent);
    }
    while (!g_bThreadShouldTerm) {
        bool updateSuccess = false;
        std::wstring loc;
        {
            std::lock_guard<std::mutex> lock(g_locationMutex);
            loc = g_location;
        }
        // Trim leading and trailing spaces / quotes
        while (!loc.empty() && (loc.front() == L' ' || loc.front() == L'\t' ||
            loc.front() == L'\r' || loc.front() == L'\n' ||
            loc.front() == L'"')) {
            loc.erase(loc.begin());
        }
        while (!loc.empty() && (loc.back() == L' ' || loc.back() == L'\t' ||
            loc.back() == L'\r' || loc.back() == L'\n' ||
            loc.back() == L'"')) {
            loc.pop_back();
        }
        std::wstring locLower = loc;
        std::transform(locLower.begin(), locLower.end(), locLower.begin(),
            ::towlower);
        if (locLower == L"auto" || locLower.empty()) {
            bool preciseSuccess = false;
            try {
                if (g_debugLogs)
                    Wh_Log(L"[EP_WeatherHost] Attempting WinRT Geolocator precision search...");
                using namespace winrt::Windows::Devices::Geolocation;
                Geolocator geolocator;
                geolocator.DesiredAccuracy(PositionAccuracy::Default);
                auto op = geolocator.GetGeopositionAsync();
                Geoposition pos = nullptr;
                auto status = winrt::Windows::Foundation::AsyncStatus::Started;
                constexpr ULONGLONG kGeolocationTimeoutMs = 10000;
                constexpr DWORD kGeolocationPollMs = 50;
                const ULONGLONG waitStarted = GetTickCount64();
                while (!BackgroundWorkShouldStop()) {
                    status = op.Status();
                    if (status != winrt::Windows::Foundation::AsyncStatus::Started) {
                        break;
                    }

                    const ULONGLONG elapsed = GetTickCount64() - waitStarted;
                    if (elapsed >= kGeolocationTimeoutMs) {
                        break;
                    }

                    const ULONGLONG remaining = kGeolocationTimeoutMs - elapsed;
                    Sleep(static_cast<DWORD>(
                        remaining < kGeolocationPollMs ? remaining : kGeolocationPollMs));
                }
                const ULONGLONG geolocationWaitMs =
                    GetTickCount64() - waitStarted;
                if (g_debugLogs) {
                    Wh_Log(
                        L"[EP_WeatherHost] WinRT Geolocator operation status=%d after %llu ms",
                        static_cast<int>(status),
                        static_cast<unsigned long long>(geolocationWaitMs));
                }
                if (BackgroundWorkShouldStop()) {
                    try { op.Cancel(); } catch (...) {}
                }
                else if (status == winrt::Windows::Foundation::AsyncStatus::Completed) {
                    pos = op.GetResults();
                }
                else if (status == winrt::Windows::Foundation::AsyncStatus::Error) {
                    // GetResults preserves the operation's HRESULT for the outer
                    // geolocation fallback handler and debug log.
                    pos = op.GetResults();
                }
                else if (status == winrt::Windows::Foundation::AsyncStatus::Canceled) {
                    if (g_debugLogs) {
                        Wh_Log(L"[EP_WeatherHost] WinRT Geolocator request was cancelled. Falling back to IP geolocation.");
                    }
                }
                else {
                    if (g_debugLogs) {
                        Wh_Log(L"[EP_WeatherHost] WinRT Geolocator timed out after 10 seconds. Cancelling.");
                    }
                    try { op.Cancel(); } catch (...) {}
                }
                if (pos) {
                    auto coordinate = pos.Coordinate();
                    if (coordinate) {
                        auto point = coordinate.Point();
                        if (point) {
                            auto position = point.Position();
                            {
                                std::lock_guard<std::mutex> stateLock(g_currentWeatherMutex);
                                g_cachedLatitude = position.Latitude;
                                g_cachedLongitude = position.Longitude;
                            }
                            auto preciseLocation = GetCurrentWeatherSnapshot();
                            std::wstring path = L"/data/reverse-geocode-client?latitude=" + std::to_wstring(preciseLocation.latitude) +
                                L"&longitude=" + std::to_wstring(preciseLocation.longitude) + L"&localityLanguage=en";
                            std::wstring reverseGeo = RequestHttpData(L"api.bigdatacloud.net", path, true);
                            if (!reverseGeo.empty()) {
                                std::wstring city = ExtractJSONValue(reverseGeo, L"city");
                                if (city.empty()) city = ExtractJSONValue(reverseGeo, L"locality");
                                std::wstring region = ExtractJSONValue(reverseGeo, L"principalSubdivision");
                                std::wstring country = ExtractJSONValue(reverseGeo, L"countryCode");
                                if (country.empty()) country = ExtractJSONValue(reverseGeo, L"countryName");
                                std::wstring fullCity = city;
                                if (!fullCity.empty() && !region.empty()) fullCity += L", " + region;
                                if (!fullCity.empty() && !country.empty()) fullCity += L", " + country;
                                std::lock_guard<std::mutex> stateLock(g_currentWeatherMutex);
                                g_displayCity = !fullCity.empty() ? fullCity : L"Local Area";
                            }
                            else {
                                std::lock_guard<std::mutex> stateLock(g_currentWeatherMutex);
                                g_displayCity = L"My Location";
                            }
                            if (g_debugLogs) {
                                auto locationState = GetCurrentWeatherSnapshot();
                                Wh_Log(L"[EP_WeatherHost] WinRT Geolocator success: %s (Lat=%f, Lon=%f)",
                                    locationState.displayCity.c_str(), locationState.latitude, locationState.longitude);
                            }
                            g_winrtPreviouslySucceeded = true;
                            preciseSuccess = true;
                            updateSuccess = true;
                        }
                    }
                }
            }
            catch (winrt::hresult_error const& e) {
                if (g_debugLogs) {
                    Wh_Log(
                        L"[EP_WeatherHost] WinRT Geolocator failed, hr=0x%08X: %s. Falling back to IP geolocation.",
                        static_cast<unsigned int>(e.code().value),
                        e.message().c_str());
                }
            }
            catch (...) {
                if (g_debugLogs) {
                    Wh_Log(L"[EP_WeatherHost] WinRT Geolocator failed with an unknown exception. Falling back to IP geolocation.");
                }
            }
            if (BackgroundWorkShouldStop()) break;
            if (!preciseSuccess) {
                if (g_winrtPreviouslySucceeded) {
                    if (g_debugLogs)
                        Wh_Log(L"[EP_WeatherHost] WinRT Geolocator unavailable, but previously succeeded. Reusing last known precise coordinates.");
                    updateSuccess = true;
                } else {
                if (g_debugLogs)
                    Wh_Log(L"[EP_WeatherHost] Falling back to BigDataCloud IP geolocation...");
                std::wstring geoJson = RequestHttpData(
                    L"api.bigdatacloud.net",
                    L"/data/reverse-geocode-client?localityLanguage=en",
                    true);
                if (!geoJson.empty()) {
                    std::wstring latStr = ExtractJSONValue(geoJson, L"latitude");
                    std::wstring lonStr = ExtractJSONValue(geoJson, L"longitude");
                    std::wstring cityStr = ExtractJSONValue(geoJson, L"city");
                    if (cityStr.empty()) cityStr = ExtractJSONValue(geoJson, L"locality");
                    std::wstring regionStr = ExtractJSONValue(geoJson, L"principalSubdivision");
                    std::wstring countryStr = ExtractJSONValue(geoJson, L"countryCode");
                    if (countryStr.empty()) countryStr = ExtractJSONValue(geoJson, L"countryName");
                    if (!latStr.empty() && !lonStr.empty()) {
                        {
                            std::lock_guard<std::mutex> stateLock(g_currentWeatherMutex);
                            g_cachedLatitude = wcstod(latStr.c_str(), NULL);
                            g_cachedLongitude = wcstod(lonStr.c_str(), NULL);
                        }
                        std::wstring fullCity = cityStr;
                        if (!fullCity.empty() && !regionStr.empty())
                            fullCity += L", " + regionStr;
                        if (!fullCity.empty() && !countryStr.empty())
                            fullCity += L", " + countryStr;
                        {
                            std::lock_guard<std::mutex> stateLock(g_currentWeatherMutex);
                            g_displayCity =
                                !fullCity.empty() ? fullCity : L"Local Weather";
                        }
                        if (g_debugLogs) {
                            auto locationState = GetCurrentWeatherSnapshot();
                            Wh_Log(L"[EP_WeatherHost] Geolocated by IP to %s (%f, %f)",
                                locationState.displayCity.c_str(), locationState.latitude,
                                locationState.longitude);
                        }
                        updateSuccess = true;
                    }
                }
                } // End of else !g_winrtPreviouslySucceeded
            }
            if (!preciseSuccess && !updateSuccess) {
                // Keep a previously resolved location and its last valid
                // forecast. Without one, show an honest error and retry soon;
                // never present New York as the user's current location.
                bool hasPreviousLocation = false;
                {
                    std::lock_guard<std::mutex> stateLock(g_currentWeatherMutex);
                    hasPreviousLocation =
                        g_cachedLatitude != 0.0 || g_cachedLongitude != 0.0;
                    if (!hasPreviousLocation) {
                        g_displayCity = L"Location unavailable";
                        g_cachedCondition = L"Location unavailable";
                        g_weatherAcquired = false;
                    }
                }
                if (g_debugLogs) {
                    if (hasPreviousLocation) {
                        Wh_Log(L"[EP_WeatherHost] Geolocation failed; reusing last resolved location.");
                    } else {
                        Wh_Log(L"[EP_WeatherHost] Geolocation failed; no location available. Retrying.");
                    }
                }
                if (!hasPreviousLocation) {
                    QueueWeatherUpdateOnUIThread();
                }
            }
        }
        else {
            if (g_debugLogs)
                Wh_Log(L"[EP_WeatherHost] Resolving coordinates for city: %s",
                    loc.c_str());
            // Clean up city name input: replace separators with spaces
            std::wstring cleanLoc = L"";
            for (wchar_t c : loc) {
                if (c == L',' || c == L';' || c == L'.') {
                    cleanLoc += L" ";
                }
                else {
                    cleanLoc += c;
                }
            }
            std::wstring escapedLoc = cleanLoc;
            size_t sPos;
            while ((sPos = escapedLoc.find(L" ")) != std::wstring::npos) {
                escapedLoc.replace(sPos, 1, L"%20");
            }
            std::wstring path = L"/v1/search?name=" + escapedLoc +
                L"&count=1&language=en&format=json";
            std::wstring geoJson = RequestHttpData(
                L"geocoding-api.open-meteo.com", path.c_str(), true);
            bool gotCoords = false;
            std::wstring latStr = L"", lonStr = L"", nameStr = L"",
                adminStr = L"", countryStr = L"";
            if (!geoJson.empty()) {
                latStr = ExtractJSONValue(geoJson, L"latitude");
                lonStr = ExtractJSONValue(geoJson, L"longitude");
                nameStr = ExtractJSONValue(geoJson, L"name");
                adminStr = ExtractJSONValue(geoJson, L"admin1");
                countryStr = ExtractJSONValue(geoJson, L"country_code");
                if (countryStr.empty())
                    countryStr = ExtractJSONValue(geoJson, L"country");
                if (!latStr.empty() && !lonStr.empty()) {
                    gotCoords = true;
                }
            }
            // Fallback retry: If full name geocoding failed, try only the first
            // alphanumeric token (word)
            if (!gotCoords) {
                size_t firstSpaceIdx = cleanLoc.find_first_not_of(L" ");
                if (firstSpaceIdx != std::wstring::npos) {
                    size_t nextSpaceIdx =
                        cleanLoc.find_first_of(L" \t", firstSpaceIdx);
                    std::wstring firstWord =
                        (nextSpaceIdx == std::wstring::npos)
                        ? cleanLoc.substr(firstSpaceIdx)
                        : cleanLoc.substr(firstSpaceIdx,
                            nextSpaceIdx - firstSpaceIdx);
                    if (firstWord != loc && !firstWord.empty()) {
                        if (g_debugLogs)
                            Wh_Log(
                                L"[EP_WeatherHost] Geocoding failed, retrying "
                                L"with first segment: %s",
                                firstWord.c_str());
                        escapedLoc = firstWord;
                        while ((sPos = escapedLoc.find(L" ")) !=
                            std::wstring::npos) {
                            escapedLoc.replace(sPos, 1, L"%20");
                        }
                        path = L"/v1/search?name=" + escapedLoc +
                            L"&count=1&language=en&format=json";
                        geoJson =
                            RequestHttpData(L"geocoding-api.open-meteo.com",
                                path.c_str(), true);
                        if (!geoJson.empty()) {
                            latStr = ExtractJSONValue(geoJson, L"latitude");
                            lonStr = ExtractJSONValue(geoJson, L"longitude");
                            nameStr = ExtractJSONValue(geoJson, L"name");
                            adminStr = ExtractJSONValue(geoJson, L"admin1");
                            countryStr =
                                ExtractJSONValue(geoJson, L"country_code");
                            if (countryStr.empty())
                                countryStr =
                                ExtractJSONValue(geoJson, L"country");
                            if (!latStr.empty() && !lonStr.empty()) {
                                gotCoords = true;
                            }
                        }
                    }
                }
            }
            if (gotCoords) {
                double resolvedLatitude = wcstod(latStr.c_str(), NULL);
                double resolvedLongitude = wcstod(lonStr.c_str(), NULL);
                std::wstring fullCity = nameStr;
                if (!fullCity.empty() && !adminStr.empty())
                    fullCity += L", " + adminStr;
                if (!fullCity.empty() && !countryStr.empty())
                    fullCity += L", " + countryStr;
                std::wstring resolvedDisplayCity = !fullCity.empty() ? fullCity : loc;
                {
                    std::lock_guard<std::mutex> stateLock(g_currentWeatherMutex);
                    g_cachedLatitude = resolvedLatitude;
                    g_cachedLongitude = resolvedLongitude;
                    g_displayCity = resolvedDisplayCity;
                }
                if (g_debugLogs)
                    Wh_Log(
                        L"[EP_WeatherHost] Resolved Coordinates: %s (%f, %f)",
                        resolvedDisplayCity.c_str(), resolvedLatitude,
                        resolvedLongitude);
                updateSuccess = true;
            }
            else {
                // Clear out stale cached coordinates to avoid showing incorrect
                // cities (like Sarnia)
                {
                    std::lock_guard<std::mutex> stateLock(g_currentWeatherMutex);
                    g_cachedLatitude = 0.0;
                    g_cachedLongitude = 0.0;
                    g_displayCity = L"Location not found";
                    g_cachedTemp = L"--";
                    g_cachedCondition = L"Check settings";
                }
                {
                    std::lock_guard<std::mutex> alertLock(g_alertMutex);
                    g_activeWarning = L"";
                    g_activeWarnings.clear();
                    g_currentWarningIndex = 0;
                    g_alertFetchStatus = AlertFetchStatus::Unknown;
                }
                if (g_debugLogs)
                    Wh_Log(
                        L"[EP_WeatherHost] Geocoding failure: location (%s) "
                        L"not found on Open-Meteo",
                        loc.c_str());
            }
        }
        auto resolvedWeatherState = GetCurrentWeatherSnapshot();
        if (resolvedWeatherState.latitude != 0.0 || resolvedWeatherState.longitude != 0.0) {
            // A resolved location is not a completed weather refresh. Only a
            // validated forecast below may restore the normal update interval.
            updateSuccess = false;
            wchar_t pathBuf[512];
            swprintf(
                pathBuf,
                L"/v1/"
                L"forecast?latitude=%.4f&longitude=%.4f&current=temperature_2m,"
                L"relative_humidity_2m,is_day,weather_code,wind_speed_10m,wind_direction_10m&"
                L"hourly=temperature_2m,weathercode,precipitation_probability,"
                L"wind_speed_10m,wind_direction_10m,relative_humidity_2m&daily=weathercode,temperature_2m_max,"
                L"temperature_2m_min,sunrise,sunset&timezone=auto&forecast_days=%d",
                resolvedWeatherState.latitude, resolvedWeatherState.longitude,
                g_forecastDaysFetch.load());
            DWORD forecastHttpStatus = 0;
            DWORD forecastWinHttpError = ERROR_SUCCESS;
            bool forecastResponseComplete = false;
            std::wstring forecastJson = RequestHttpData(
                L"api.open-meteo.com", pathBuf, true,
                &forecastHttpStatus, &forecastWinHttpError,
                &forecastResponseComplete);
            const bool forecastTransportValid =
                forecastHttpStatus == 200 &&
                forecastWinHttpError == ERROR_SUCCESS &&
                forecastResponseComplete && !forecastJson.empty();
            if (!forecastTransportValid) {
                if (g_debugLogs) {
                    Wh_Log(
                        L"[WeatherData] Forecast refresh rejected; preserving last valid data. HTTP=%lu, WinHTTP=%lu, complete=%d, chars=%d",
                        forecastHttpStatus, forecastWinHttpError,
                        forecastResponseComplete ? 1 : 0,
                        static_cast<int>(forecastJson.length()));
                }
            }
            else {
                try {
                    wchar_t alertPath[512];
                    swprintf(alertPath, L"/v3/alerts/headlines?geocode=%.4f,%.4f&format=json&language=en-US&apiKey=e1f10a1e78da46f5b10a1e78da96f525", resolvedWeatherState.latitude, resolvedWeatherState.longitude);
                    DWORD alertHttpStatus = 0;
                    DWORD alertWinHttpError = ERROR_SUCCESS;
                    std::wstring alertsJson = RequestHttpData(
                        L"api.weather.com", alertPath, true,
                        &alertHttpStatus, &alertWinHttpError);
                    if (g_debugLogs) {
                        Wh_Log(
                            L"[EP_WeatherHost] Weather.com Alerts API: HTTP=%lu, WinHTTP=%lu, chars=%d, body=%.100s",
                            alertHttpStatus, alertWinHttpError,
                            (int)alertsJson.length(), alertsJson.c_str());
                    }
                    // A 204 is a normal empty result, not a parsing failure. Never
                    // interpret an auth/transport/schema error as a successful query.
                    if (alertWinHttpError != ERROR_SUCCESS) {
                        if (g_debugLogs) Wh_Log(L"[WeatherAlerts] Transport/header failure; response is unusable.");
                        throw winrt::hresult_error(E_FAIL);
                    }
                    if (alertHttpStatus != 200 && alertHttpStatus != 204) {
                        if (g_debugLogs) Wh_Log(L"[WeatherAlerts] HTTP %lu: %s", alertHttpStatus,
                            (alertHttpStatus == 401 || alertHttpStatus == 403)
                                ? L"authentication/access rejected" : L"unexpected response");
                        throw winrt::hresult_error(E_FAIL);
                    }
                    std::vector<WeatherAlert> parsedAlerts;
                    if (alertHttpStatus == 200) {
                        using namespace winrt::Windows::Data::Json;
                        JsonObject alertRoot = JsonObject::Parse(winrt::hstring(alertsJson));
                        auto alertsValue = alertRoot.GetNamedValue(L"alerts", JsonValue::CreateNullValue());
                        if (alertsValue.ValueType() != JsonValueType::Array) {
                            if (g_debugLogs) Wh_Log(L"[WeatherAlerts] HTTP 200 but alerts array is absent or invalid.");
                            throw winrt::hresult_error(E_INVALIDARG);
                        }
                        for (auto const& value : alertsValue.GetArray()) {
                            if (value.ValueType() != JsonValueType::Object) throw winrt::hresult_error(E_INVALIDARG);
                            JsonObject obj = value.GetObject();
                            auto readString = [&obj](const wchar_t* key) -> std::wstring {
                                auto field = obj.GetNamedValue(key, JsonValue::CreateNullValue());
                                return field.ValueType() == JsonValueType::String
                                    ? std::wstring(field.GetString().c_str()) : std::wstring{};
                            };
                            WeatherAlert alert;
                            alert.eventDesc = readString(L"eventDescription");
                            alert.headlineText = readString(L"headlineText");
                            if (alert.eventDesc.empty()) alert.eventDesc = alert.headlineText;
                            if (alert.eventDesc.empty()) throw winrt::hresult_error(E_INVALIDARG);
                            alert.areaName = readString(L"areaName");
                            alert.severity = readString(L"severity");
                            alert.significance = readString(L"significance");
                            alert.source = readString(L"source");
                            auto expiry = obj.GetNamedValue(L"expireTimeUTC", JsonValue::CreateNullValue());
                            if (expiry.ValueType() == JsonValueType::Number) alert.expiryUtc = expiry.GetNumber();
                            alert.details = readString(L"headlineSummary");
                            if (alert.details.empty()) alert.details = readString(L"description");
                            if (alert.details.empty()) alert.details = readString(L"overview");
                            if (alert.details.empty()) alert.details = readString(L"instruction");
                            // Full narratives still require a separate detail endpoint.
                            parsedAlerts.push_back(std::move(alert));
                        }
                    }
                    {
                        std::lock_guard<std::mutex> alertLock(g_alertMutex);
                        g_activeWarnings = std::move(parsedAlerts);
                        g_currentWarningIndex = 0;
                        g_alertFetchStatus = g_activeWarnings.empty()
                            ? AlertFetchStatus::NoAlerts : AlertFetchStatus::Active;
                        UpdateActiveWarningTextLocked();
                    }
                    if (g_debugLogs) {
                        Wh_Log(L"[WeatherAlerts] HTTP=%lu; %d issued alerts for %.4f,%.4f (%s).%s",
                            alertHttpStatus, static_cast<int>(GetAlertSnapshot().warnings.size()),
                            resolvedWeatherState.latitude, resolvedWeatherState.longitude,
                            resolvedWeatherState.displayCity.c_str(),
                            alertHttpStatus == 204 ? L" Provider returned no content." : L"");
                    }
                }
                catch (winrt::hresult_error const& e) {
                    // Do not leave an old-location or unverified alert presented as current.
                    std::lock_guard<std::mutex> alertLock(g_alertMutex);
                    g_activeWarnings.clear();
                    g_currentWarningIndex = 0;
                    g_alertFetchStatus = AlertFetchStatus::Unavailable;
                    UpdateActiveWarningTextLocked();
                    if (g_debugLogs) Wh_Log(L"[WeatherAlerts] Retrieval unavailable (not confirmed no-alerts), hr=0x%08X: %s",
                        static_cast<unsigned int>(e.code().value), e.message().c_str());
                }
                catch (...) {
                    std::lock_guard<std::mutex> alertLock(g_alertMutex);
                    g_activeWarnings.clear();
                    g_currentWarningIndex = 0;
                    g_alertFetchStatus = AlertFetchStatus::Unavailable;
                    UpdateActiveWarningTextLocked();
                    if (g_debugLogs) Wh_Log(L"[WeatherAlerts] Retrieval unavailable (unexpected exception).");
                }
                std::wstring currentBlock =
                    ExtractJSONObject(forecastJson, L"current");
                std::wstring dailyBlock =
                    ExtractJSONObject(forecastJson, L"daily");
                std::wstring hourlyBlock =
                    ExtractJSONObject(forecastJson, L"hourly");
                std::wstring tempStr = L"";
                std::wstring codeStr = L"";
                std::wstring dayStr = L"";
                std::wstring windSpeedStr = L"";
                std::wstring windDirStr = L"";
                std::wstring humidityStr = L"";
                if (!currentBlock.empty()) {
                    tempStr = ExtractJSONValue(currentBlock, L"temperature_2m");
                    codeStr = ExtractJSONValue(currentBlock, L"weather_code");
                    dayStr = ExtractJSONValue(currentBlock, L"is_day");
                    windSpeedStr =
                        ExtractJSONValue(currentBlock, L"wind_speed_10m");
                    windDirStr =
                        ExtractJSONValue(currentBlock, L"wind_direction_10m");
                    humidityStr =
                        ExtractJSONValue(currentBlock, L"relative_humidity_2m");
                }
                std::wstring formattedWindSpeed = L"--";
                if (!windSpeedStr.empty()) {
                    double ws = wcstod(windSpeedStr.c_str(), NULL);
                    double wd = windDirStr.empty() ? 0.0 : wcstod(windDirStr.c_str(), NULL);
                    std::wstring dirStr = GetWindDirectionString(wd);
                    wchar_t wsBuf[128];
                    if (g_useCelsius) {
                        swprintf(wsBuf, L"%.1f km/h %s", ws, dirStr.c_str());
                    }
                    else {
                        double mph = ws * 0.621371;
                        swprintf(wsBuf, L"%.1f mph %s", mph, dirStr.c_str());
                    }
                    formattedWindSpeed = wsBuf;
                }
                std::wstring timeArrStr = L"";
                std::wstring codeArrStr = L"";
                std::wstring maxArrStr = L"";
                std::wstring minArrStr = L"";
                std::wstring sunriseArrStr = L"";
                std::wstring sunsetArrStr = L"";
                if (!dailyBlock.empty()) {
                    timeArrStr = ExtractJSONArray(dailyBlock, L"time");
                    codeArrStr = ExtractJSONArray(dailyBlock, L"weathercode");
                    maxArrStr =
                        ExtractJSONArray(dailyBlock, L"temperature_2m_max");
                    minArrStr =
                        ExtractJSONArray(dailyBlock, L"temperature_2m_min");
                    sunriseArrStr = ExtractJSONArray(dailyBlock, L"sunrise");
                    sunsetArrStr = ExtractJSONArray(dailyBlock, L"sunset");
                }
                std::wstring hTimeArrStr = L"";
                std::wstring hTempArrStr = L"";
                std::wstring hCodeArrStr = L"";
                std::wstring hPrecipArrStr = L"";
                std::wstring hWindArrStr = L"";
                std::wstring hWindDirArrStr = L"";
                std::wstring hHumidArrStr = L"";
                if (!hourlyBlock.empty()) {
                    hTimeArrStr = ExtractJSONArray(hourlyBlock, L"time");
                    hTempArrStr =
                        ExtractJSONArray(hourlyBlock, L"temperature_2m");
                    hCodeArrStr = ExtractJSONArray(hourlyBlock, L"weathercode");
                    hPrecipArrStr = ExtractJSONArray(
                        hourlyBlock, L"precipitation_probability");
                    hWindArrStr =
                        ExtractJSONArray(hourlyBlock, L"wind_speed_10m");
                    hWindDirArrStr =
                        ExtractJSONArray(hourlyBlock, L"wind_direction_10m");
                    hHumidArrStr =
                        ExtractJSONArray(hourlyBlock, L"relative_humidity_2m");
                }
                std::vector<std::wstring> timeVec = ParseJSONArray(timeArrStr);
                std::vector<std::wstring> codeVec = ParseJSONArray(codeArrStr);
                std::vector<std::wstring> maxVec = ParseJSONArray(maxArrStr);
                std::vector<std::wstring> minVec = ParseJSONArray(minArrStr);
                std::vector<std::wstring> sunriseVec = ParseJSONArray(sunriseArrStr);
                std::vector<std::wstring> sunsetVec = ParseJSONArray(sunsetArrStr);
                std::vector<std::wstring> hTimeVec =
                    ParseJSONArray(hTimeArrStr);
                std::vector<std::wstring> hTempVec =
                    ParseJSONArray(hTempArrStr);
                std::vector<std::wstring> hCodeVec =
                    ParseJSONArray(hCodeArrStr);
                std::vector<std::wstring> hPrecipVec =
                    ParseJSONArray(hPrecipArrStr);
                std::vector<std::wstring> hWindVec =
                    ParseJSONArray(hWindArrStr);
                std::vector<std::wstring> hWindDirVec =
                    ParseJSONArray(hWindDirArrStr);
                std::vector<std::wstring> hHumidVec =
                    ParseJSONArray(hHumidArrStr);

                auto isFiniteNumber = [](const std::wstring& value) {
                    if (value.empty()) return false;
                    wchar_t* end = nullptr;
                    double parsed = wcstod(value.c_str(), &end);
                    return end != value.c_str() && end && *end == L'\0' &&
                        std::isfinite(parsed);
                };
                auto allFiniteNumbers = [&isFiniteNumber](
                    const std::vector<std::wstring>& values) {
                    return std::all_of(
                        values.begin(), values.end(), isFiniteNumber);
                };
                auto allNonEmpty = [](const std::vector<std::wstring>& values) {
                    return std::all_of(
                        values.begin(), values.end(),
                        [](const std::wstring& value) { return !value.empty(); });
                };

                const size_t numDays = timeVec.size();
                const size_t numHours = hTimeVec.size();
                const bool dailyShapeValid = numDays > 0 &&
                    codeVec.size() == numDays && maxVec.size() == numDays &&
                    minVec.size() == numDays && sunriseVec.size() == numDays &&
                    sunsetVec.size() == numDays && allNonEmpty(timeVec) &&
                    allFiniteNumbers(codeVec) && allFiniteNumbers(maxVec) &&
                    allFiniteNumbers(minVec);
                const bool hourlyShapeValid = numHours > 0 &&
                    hTempVec.size() == numHours && hCodeVec.size() == numHours &&
                    hPrecipVec.size() == numHours && hWindVec.size() == numHours &&
                    hWindDirVec.size() == numHours && hHumidVec.size() == numHours &&
                    allNonEmpty(hTimeVec) && allFiniteNumbers(hTempVec) &&
                    allFiniteNumbers(hCodeVec) && allFiniteNumbers(hPrecipVec) &&
                    allFiniteNumbers(hWindVec) && allFiniteNumbers(hWindDirVec) &&
                    allFiniteNumbers(hHumidVec);
                const bool currentShapeValid = !currentBlock.empty() &&
                    isFiniteNumber(tempStr) && isFiniteNumber(codeStr) &&
                    isFiniteNumber(dayStr) && isFiniteNumber(windSpeedStr) &&
                    isFiniteNumber(windDirStr) && isFiniteNumber(humidityStr);
                const bool forecastSchemaValid =
                    currentShapeValid && dailyShapeValid && hourlyShapeValid;
                if (!forecastSchemaValid) {
                    if (g_debugLogs) {
                        Wh_Log(
                            L"[WeatherData] Forecast schema rejected; preserving last valid data. current=%d, daily=%d (%llu rows), hourly=%d (%llu rows)",
                            currentShapeValid ? 1 : 0,
                            dailyShapeValid ? 1 : 0,
                            static_cast<unsigned long long>(numDays),
                            hourlyShapeValid ? 1 : 0,
                            static_cast<unsigned long long>(numHours));
                    }
                }
                else {
                std::wstring currentPrecipProb = L"0%";
                size_t startHourIdx = 0;
                SYSTEMTIME stNow;
                GetLocalTime(&stNow);
                wchar_t sysHourBuf[32];
                swprintf(sysHourBuf, 32, L"%04d-%02d-%02dT%02d:00", stNow.wYear, stNow.wMonth, stNow.wDay, stNow.wHour);
                std::wstring currentHourIso = sysHourBuf;
                if (!currentBlock.empty()) {
                    std::wstring currentTimeStr =
                        ExtractJSONValue(currentBlock, L"time");
                    for (size_t i = 0; i < hTimeVec.size(); i++) {
                        if (hTimeVec[i] == currentTimeStr) {
                            startHourIdx = i;
                            break;
                        }
                    }
                }
                if (startHourIdx == 0 && !hTimeVec.empty()) {
                    for (size_t i = 0; i < hTimeVec.size(); i++) {
                        if (hTimeVec[i] >= currentHourIso) {
                            startHourIdx = i;
                            break;
                        }
                    }
                }
                if (startHourIdx < hPrecipVec.size()) {
                    currentPrecipProb = hPrecipVec[startHourIdx] + L"%";
                }
                std::vector<DailyForecast> parsedBriefForecasts;
                for (size_t i = 0; i < numDays; i++) {
                    DailyForecast dayData;
                    std::wstring rawDate = timeVec[i];
                    dayData.rawDate = rawDate;
                    int year = 0, month = 0, dayInt = 0;
                    if (swscanf(rawDate.c_str(), L"%d-%d-%d", &year, &month,
                        &dayInt) == 3) {
                        dayData.dayName = GetDayOfWeek(year, month, dayInt);
                    }
                    else {
                        dayData.dayName = rawDate;
                    }
                    double tMax = wcstod(maxVec[i].c_str(), NULL);
                    double tMin = wcstod(minVec[i].c_str(), NULL);
                    if (!g_useCelsius) {
                        tMax = (tMax * 9.0 / 5.0) + 32.0;
                        tMin = (tMin * 9.0 / 5.0) + 32.0;
                    }
                    wchar_t bufMax[32], bufMin[32];
                    swprintf(bufMax, L"%.0f°", tMax);
                    swprintf(bufMin, L"%.0f°", tMin);
                    dayData.tempMax = bufMax;
                    dayData.tempMin = bufMin;
                    int codeVal = _wtoi(codeVec[i].c_str());
                    auto mapping = GetCodeMapping(codeVal);
                    dayData.icon = mapping.first;
                    dayData.condition = mapping.second;
                    dayData.weatherCode = codeVal;
                    dayData.iconUrl = GetWeatherIconPathOrUrl(codeVal, true);
                    if (i < sunriseVec.size()) {
                        dayData.sunrise = FormatISOTimeTo12H(sunriseVec[i]);
                    }
                    if (i < sunsetVec.size()) {
                        dayData.sunset = FormatISOTimeTo12H(sunsetVec[i]);
                    }
                    parsedBriefForecasts.push_back(dayData);
                }
                std::wstring newSunrise = !sunriseVec.empty() ? FormatISOTimeTo12H(sunriseVec[0]) : L"";
                std::wstring newSunset = !sunsetVec.empty() ? FormatISOTimeTo12H(sunsetVec[0]) : L"";
                SYSTEMTIME st;
                GetLocalTime(&st);
                wchar_t updateBuf[32];
                int displayH = st.wHour % 12;
                if (displayH == 0) displayH = 12;
                swprintf(updateBuf, 32, L"%d:%02d %s", displayH, st.wMinute, (st.wHour >= 12 ? L"PM" : L"AM"));
                {
                    std::lock_guard<std::mutex> stateLock(g_currentWeatherMutex);
                    g_cachedSunrise = newSunrise;
                    g_cachedSunset = newSunset;
                    g_lastUpdatedTime = updateBuf;
                }
                std::vector<HourlyForecast> parsedHourlyForecasts;
                size_t limitHours =
                    g_forecastDaysFetch *
                    24;  // Parse all requested days of hourly data
                for (size_t i = 0;
                    i < hTimeVec.size() && i < limitHours;
                    i++) {
                    if (i >= hTempVec.size() || i >= hCodeVec.size() ||
                        i >= hPrecipVec.size() || i >= hWindVec.size())
                        break;
                    HourlyForecast hourData;
                    std::wstring rawTime = hTimeVec[i];
                    size_t tIndex = rawTime.find(L"T");
                    if (tIndex != std::wstring::npos) {
                        hourData.rawDate = rawTime.substr(0, tIndex);
                    }
                    int militaryHour = 0;
                    if (tIndex != std::wstring::npos &&
                        tIndex + 2 < rawTime.length()) {
                        std::wstring hourSub = rawTime.substr(tIndex + 1, 2);
                        militaryHour = _wtoi(hourSub.c_str());
                    }
                    int displayHour = militaryHour % 12;
                    if (displayHour == 0)
                        displayHour = 12;
                    wchar_t timeBuf[32];
                    swprintf(timeBuf, L"%d %s", displayHour,
                        (militaryHour >= 12) ? L"PM" : L"AM");
                    hourData.timeString = timeBuf;
                    hourData.hour24 = militaryHour;
                    hourData.tempRaw = wcstod(hTempVec[i].c_str(), NULL);
                    hourData.precipRaw = wcstod(hPrecipVec[i].c_str(), NULL);
                    hourData.windkphRaw = wcstod(hWindVec[i].c_str(), NULL);
                    double hTemp = hourData.tempRaw;
                    if (!g_useCelsius) {
                        hTemp = (hTemp * 9.0 / 5.0) + 32.0;
                    }
                    wchar_t tempBuf[32];
                    swprintf(tempBuf, L"%.0f°", hTemp);
                    hourData.temp = tempBuf;
                    int codeVal = _wtoi(hCodeVec[i].c_str());
                    auto mapping = GetCodeMapping(codeVal);
                    hourData.icon = mapping.first;
                    hourData.conditionName = mapping.second;
                    hourData.weatherCode = codeVal;
                    int isDayVal = (militaryHour >= 6 && militaryHour < 20) ? 1 : 0;
                    hourData.isDay = isDayVal;
                    hourData.iconUrl = GetWeatherIconPathOrUrl(codeVal, isDayVal != 0);
                    hourData.precipProb = hPrecipVec[i] + L"%";
                    if (i < hHumidVec.size()) {
                        hourData.humidity = hHumidVec[i] + L"%";
                    }
                    else {
                        hourData.humidity = L"0%";
                    }
                    {
                        double hWindVal = hourData.windkphRaw;
                        double hWindDirVal = 0.0;
                        if (i < hWindDirVec.size()) {
                            hWindDirVal = wcstod(hWindDirVec[i].c_str(), NULL);
                        }
                        std::wstring hDirStr = GetWindDirectionString(hWindDirVal);
                        wchar_t hWindBuf[128];
                        if (g_useCelsius) {
                            swprintf(hWindBuf, L"%.1f km/h %s", hWindVal, hDirStr.c_str());
                        }
                        else {
                            double mph = hWindVal * 0.621371;
                            swprintf(hWindBuf, L"%.1f mph %s", mph, hDirStr.c_str());
                        }
                        hourData.windSpeed = hWindBuf;
                    }
                    parsedHourlyForecasts.push_back(hourData);
                }
                EnterCriticalSection(&g_forecastLock);
                g_forecastDaily = parsedBriefForecasts;
                g_forecastHourly = parsedHourlyForecasts;
                g_cachedWindSpeed = formattedWindSpeed;
                g_cachedPrecipProb = currentPrecipProb;
                g_cachedHumidity = humidityStr + L"%";
                g_forecastAcquired = true;
                LeaveCriticalSection(&g_forecastLock);
                if (!tempStr.empty()) {
                    double tempVal = wcstod(tempStr.c_str(), NULL);
                    if (!g_useCelsius) {
                        tempVal = (tempVal * 9.0 / 5.0) + 32.0;
                    }
                    wchar_t formattedTemp[32];
                    swprintf(formattedTemp, L"%.1f°%c", tempVal,
                        g_useCelsius ? 'C' : 'F');
                    {
                        std::lock_guard<std::mutex> stateLock(g_currentWeatherMutex);
                        g_cachedTemp = formattedTemp;
                    }
                    int code = !codeStr.empty() ? _wtoi(codeStr.c_str()) : 0;
                    int is_day = !dayStr.empty() ? _wtoi(dayStr.c_str()) : 1;
                    MapOpenMeteoCode(code, is_day);
                    // On the dedicated/classic taskbar backend, prepare the
                    // selected MSN SVG before
                    // publishing the acquired weather state. This keeps the
                    // taskbar on its loading glyph until the real icon is
                    // ready instead of briefly displaying the emoji fallback.
                    if (!ShouldUseXamlTaskbar() && g_weatherStyle == 2) {
                        RefreshWin10TaskbarSvgCache(
                            GetCurrentWeatherSnapshot().iconUrl);
                    }
                    {
                        std::lock_guard<std::mutex> stateLock(g_currentWeatherMutex);
                        g_weatherAcquired = true;
                    }
                    updateSuccess = true;
                    if (g_debugLogs) {
                        auto updatedWeather = GetCurrentWeatherSnapshot();
                        Wh_Log(
                            L"[EP_WeatherHost] Weather Updated! Temp=%s, "
                            L"Code=%d, Condition=%s, City=%s",
                            updatedWeather.temp.c_str(), code,
                            updatedWeather.condition.c_str(), updatedWeather.displayCity.c_str());
                    }
                    QueueWeatherUpdateOnUIThread();
                    // Publish normal weather first. GeoMet may take several
                    // seconds, but its delay must not hold the whole flyout
                    // in the initial loading state.
                    FetchRadarMetadata();
                    HWND uiTarget = g_foregroundMessageTarget.load(std::memory_order_acquire);
                    if (!BackgroundWorkShouldStop() && uiTarget && IsWindow(uiTarget))
                        PostMessageW(uiTarget, WM_USER + 4247, 0, 0);
                }
                }
            }
        }
        DWORD sleepMs =
            updateSuccess ? (g_updateInterval * 60 * 1000) : (60 * 1000);
        DWORD elapsedMs = 0;
        while (elapsedMs < sleepMs && !g_bThreadShouldTerm) {
            DWORD waitStatus = WaitForSingleObject(g_hForceUpdateEvent, 1000);
            if (waitStatus == WAIT_OBJECT_0) {
                ResetEvent(g_hForceUpdateEvent);
                break;
            }
            elapsedMs += 1000;
        }
    }
    winrt::uninit_apartment();
    return 0;
}
// Remove previously injected containers
HWND FindXamlIslandWindow(HWND hParent) {
    if (!hParent) return nullptr;
    HWND hChild = FindWindowExW(hParent, nullptr, L"Windows.UI.Input.InputSite.WindowClass", nullptr);
    if (!hChild) {
        hChild = FindWindowExW(hParent, nullptr, nullptr, nullptr);
        while (hChild) {
            WCHAR className[256];
            if (GetClassNameW(hChild, className, 256)) {
                if (wcscmp(className, L"Windows.UI.Input.InputSite.WindowClass") == 0) {
                    return hChild;
                }
            }
            HWND hSubChild = FindXamlIslandWindow(hChild);
            if (hSubChild) return hSubChild;
            hChild = FindWindowExW(hParent, hChild, nullptr, nullptr);
        }
    }
    return hChild;
}
void RemoveInjectedFromPanel(winrt::Windows::UI::Xaml::Controls::Panel panel, bool restoreSiblings = true) {
    if (!panel)
        return;
    try {
        auto children = panel.Children();
        for (int i = (int)children.Size() - 1; i >= 0; i--) {
            if (auto fe = children.GetAt(i).try_as<FrameworkElement>()) {
                std::wstring name(fe.Name());
                if (name == L"WhWeatherHostGrid") {
                    children.RemoveAt(i);
                }
                else if (restoreSiblings) {
                    // Only restore elements that were explicitly collapsed/hidden by our mod
                    try {
                        if (fe.Opacity() == 0.0) {
                            fe.Visibility(Visibility::Visible);
                            fe.Opacity(1.0);
                            fe.IsHitTestVisible(true);
                            if (auto ctrl = fe.try_as<winrt::Windows::UI::Xaml::Controls::Control>()) {
                                ctrl.IsTabStop(true);
                                ctrl.AllowFocusOnInteraction(true);
                            }
                        }
                    }
                    catch (...) {}
                }
            }
        }
    }
    catch (...) {
    }
}
void RemoveInjectedFromGrid(Grid grid, bool restoreSiblings = true) {
    RemoveInjectedFromPanel(grid, restoreSiblings);
}
// Native direct XAML injection of Weather Grid
void InjectContentIntoGrid(FrameworkElement element,
    std::wstring_view uniqueName) {
    auto grid = element.try_as<Grid>();
    if (!grid)
        return;
    for (uint32_t i = 0; i < grid.Children().Size(); i++) {
        if (auto existingChild = grid.Children().GetAt(i).try_as<FrameworkElement>()) {
            if (existingChild.Name() == uniqueName) {
                // Already injected! Just update layout/elements and return.
                if (auto existingGrid = existingChild.try_as<Grid>()) {
                    UpdateInjectedWeatherLayout(existingGrid);
                    auto currentWeather = GetCurrentWeatherSnapshot();
                    UpdateWeatherXamlElements(existingGrid, currentWeather.temp, currentWeather.icon, currentWeather.condition, currentWeather.acquired);
                }
                return;
            }
        }
    }
    RemoveInjectedFromGrid(grid, false);
    Grid weatherGrid;
    weatherGrid.Name(uniqueName);
    // Make sure we span the entire Grid, bypassing any column definitions,
    // columns, or layout cells so that margin aligns globally
    try {
        Grid::SetColumn(weatherGrid, 0);
        Grid::SetColumnSpan(weatherGrid, 99);
        Grid::SetRow(weatherGrid, 0);
        Grid::SetRowSpan(weatherGrid, 99);
    }
    catch (...) {
    }
    UpdateInjectedWeatherLayout(weatherGrid);
    // Transparent solid color brush background
    weatherGrid.Background(winrt::Windows::UI::Xaml::Media::SolidColorBrush(
        winrt::Windows::UI::Colors::Transparent()));
    {
        auto currentWeather = GetCurrentWeatherSnapshot();
        UpdateWeatherXamlElements(weatherGrid, currentWeather.temp, currentWeather.icon,
            currentWeather.condition, currentWeather.acquired);
    }
    grid.Children().Append(weatherGrid);
    auto weakWeatherGrid = winrt::make_weak(weatherGrid);
    winrt::event_token token = element.SizeChanged([weakWeatherGrid](auto const&, auto const&) {
        if (g_modUnloaded) return;
        if (auto wg = weakWeatherGrid.get()) {
            if (!wg.Parent()) return;
            try {
                auto dispatcher = wg.Dispatcher();
                if (dispatcher && !g_modUnloaded) {
                    g_pendingRunAsync++;
                    try {
                        dispatcher.RunAsync(
                        winrt::Windows::UI::Core::CoreDispatcherPriority::Normal,
                        [weakWeatherGrid]() {
struct AutoDecrement { ~AutoDecrement() { g_pendingRunAsync--; } } autoDec;


                            if (g_modUnloaded) return;
                            try {
                                if (auto wg2 = weakWeatherGrid.get()) {
                                    if (!wg2.Parent()) return;
                                    RefreshInjectedWeatherAfterResize(wg2);
                                }
                            }
                            catch (...) {}
                        });
                    } catch (...) {
                        g_pendingRunAsync--;
                        throw;
                    }
                }
            }
            catch (...) {}
        }
        });
    {
        std::lock_guard<std::mutex> lock2(g_hooksMutex);
        g_sizeChangedHooks.push_back({winrt::make_weak(element), token});
    }
    {
        std::lock_guard<std::mutex> lock(g_weatherGridMutex);
        g_injectedWeatherGrids.push_back(winrt::make_weak(weatherGrid));
    }
}
// Helper to search visual tree safely and robustly for a target element
FrameworkElement FindElementInTree(FrameworkElement root, const std::wstring& targetName, const std::wstring& targetClassName) {
    if (!root) return nullptr;
    try {
        bool matches = false;
        // 1. Try checking by Name with localized try-catch
        try {
            std::wstring name(root.Name());
            if (!targetName.empty() && name == targetName) {
                matches = true;
            }
        } catch (...) {}
        // 2. Try checking by class name with localized try-catch
        if (!matches && !targetClassName.empty()) {
            try {
                std::wstring className(winrt::get_class_name(root).c_str());
                if (className.find(targetClassName) != std::wstring::npos) {
                    matches = true;
                }
            } catch (...) {}
        }
        if (matches) {
            if (g_debugLogs && targetName != L"WhWeatherHostGrid") {
                try {
                    std::wstring name(root.Name());
                    std::wstring className(winrt::get_class_name(root).c_str());
                    Wh_Log(L"[Wh_WeatherHost] FindElementInTree: Found target! Name='%s', ClassName='%s'", name.c_str(), className.c_str());
                } catch (...) {}
            }
            return root;
        }
        // 3. Recursively search children with localized try-catch so one failing child doesn't abort the rest
        int count = 0;
        try {
            count = VisualTreeHelper::GetChildrenCount(root);
        } catch (...) {}
        for (int i = 0; i < count; i++) {
            FrameworkElement child = nullptr;
            try {
                child = VisualTreeHelper::GetChild(root, i).try_as<FrameworkElement>();
            } catch (...) {}
            if (child) {
                auto found = FindElementInTree(child, targetName, targetClassName);
                if (found) {
                    return found;
                }
            }
        }
    } catch (...) {}
    return nullptr;
}
// Helper to find widgets button
bool FindAndInjectWidgetsButton(FrameworkElement element) {
    if (!element) return false;
    std::wstring name;
    std::wstring className;
    try {
        name = element.Name();
        className = winrt::get_class_name(element).c_str();
    } catch (...) { return false; }
    if (className.find(L"AugmentedEntryPoint") != std::wstring::npos ||
        name == L"WidgetsButton" || name == L"Widgets") {
            if (g_debugLogs) {
                try {
                    Wh_Log(L"[Wh_WeatherHost] Widgets target: visibility=%d actual=%.1fx%.1f",
                           (int)element.Visibility(), element.ActualWidth(), element.ActualHeight());
                } catch (...) {}
            }
            try {
                if (auto ctrl = element.try_as<winrt::Windows::UI::Xaml::Controls::Control>()) {
                    ctrl.IsTabStop(false);
                    ctrl.AllowFocusOnInteraction(false);
                    try {
                        ctrl.CornerRadius(winrt::Windows::UI::Xaml::CornerRadius{ 5.0, 5.0, 5.0, 5.0 });
                        ctrl.Background(winrt::Windows::UI::Xaml::Media::SolidColorBrush(winrt::Windows::UI::Colors::Transparent()));
                    } catch (...) {}
                }
                winrt::Windows::UI::Xaml::Automation::AutomationProperties::SetAccessibilityView(element, winrt::Windows::UI::Xaml::Automation::Peers::AccessibilityView::Raw);
                winrt::Windows::UI::Xaml::Automation::AutomationProperties::SetIsRequiredForForm(element, false);
            } catch (...) {}
        // Keep the shell's native host geometry. The weather content must fit
        // inside it; changing its template panel during layout can cause reentry.
        winrt::Windows::UI::Xaml::Controls::Panel innerPanel = element.try_as<winrt::Windows::UI::Xaml::Controls::Panel>();
        if (!innerPanel) {
            int childCount = VisualTreeHelper::GetChildrenCount(element);
            for (int i = 0; i < childCount; i++) {
                auto child = VisualTreeHelper::GetChild(element, i).try_as<FrameworkElement>();
                if (child) {
                    if (auto p = child.try_as<winrt::Windows::UI::Xaml::Controls::Panel>()) {
                        innerPanel = p;
                        break;
                    }
                    int subCount = VisualTreeHelper::GetChildrenCount(child);
                    for (int j = 0; j < subCount; j++) {
                        auto subChild = VisualTreeHelper::GetChild(child, j).try_as<FrameworkElement>();
                        if (subChild) {
                            if (auto p = subChild.try_as<winrt::Windows::UI::Xaml::Controls::Panel>()) {
                                innerPanel = p;
                                break;
                            }
                        }
                    }
                }
                if (innerPanel) break;
            }
        }
        if (innerPanel) {
            for (uint32_t i = 0; i < innerPanel.Children().Size(); i++) {
                if (auto existingChild = innerPanel.Children().GetAt(i).try_as<FrameworkElement>()) {
                    if (existingChild.Name() == L"WhWeatherHostGrid") {
                        // Already injected! Just update layout/elements and return.
                        if (auto existingGrid = existingChild.try_as<Grid>()) {
                            UpdateInjectedWeatherLayout(existingGrid);
                            auto currentWeather = GetCurrentWeatherSnapshot();
                            UpdateWeatherXamlElements(existingGrid, currentWeather.temp, currentWeather.icon, currentWeather.condition, currentWeather.acquired);
                        }
                        return true;
                    }
                }
            }
            for (uint32_t i = 0; i < innerPanel.Children().Size(); i++) {
                if (auto existingChild = innerPanel.Children().GetAt(i).try_as<FrameworkElement>()) {
                    std::wstring childName(existingChild.Name());
                    if (childName != L"WhWeatherHostGrid") {
                        try {
                            existingChild.Opacity(0.0);
                        } catch (...) {}
                        try {
                            existingChild.Visibility(Visibility::Collapsed);
                        } catch (...) {}
                        try {
                            existingChild.IsHitTestVisible(false);
                        } catch (...) {}
                        try {
                            if (auto ctrl = existingChild.try_as<winrt::Windows::UI::Xaml::Controls::Control>()) {
                                ctrl.IsTabStop(false);
                                ctrl.AllowFocusOnInteraction(false);
                            }
                        } catch (...) {}
                    }
                }
            }
            if (auto grid = innerPanel.try_as<Grid>()) {
                InjectContentIntoGrid(grid, L"WhWeatherHostGrid");
                return true;
            }
            else {
                RemoveInjectedFromPanel(innerPanel, false);
                Grid weatherGrid;
                weatherGrid.Name(L"WhWeatherHostGrid");
                UpdateInjectedWeatherLayout(weatherGrid);
                weatherGrid.Background(winrt::Windows::UI::Xaml::Media::SolidColorBrush(winrt::Windows::UI::Colors::Transparent()));
                {
                    auto currentWeather = GetCurrentWeatherSnapshot();
                    UpdateWeatherXamlElements(weatherGrid, currentWeather.temp, currentWeather.icon, currentWeather.condition, currentWeather.acquired);
                }
                innerPanel.Children().Append(weatherGrid);
                auto weakWg = winrt::make_weak(weatherGrid);
                winrt::event_token token = element.SizeChanged([weakWg](auto const&, auto const&) {
                    if (g_modUnloaded) return;
                    if (auto wg = weakWg.get()) {
                        if (!wg.Parent()) return;
                        try {
                            auto dispatcher = wg.Dispatcher();
                            if (dispatcher) {
                                g_pendingRunAsync++;
                                try {
                                    dispatcher.RunAsync(
                                    winrt::Windows::UI::Core::CoreDispatcherPriority::Normal,
                                    [weakWg]() {
struct AutoDecrement { ~AutoDecrement() { g_pendingRunAsync--; } } autoDec;


                                        if (g_modUnloaded) return;
                                        try {
                                            if (auto wg2 = weakWg.get()) {
                                                if (!wg2.Parent()) return;
                                                RefreshInjectedWeatherAfterResize(wg2);
                                            }
                                        }
                                        catch (...) {}
                                    });
                                } catch (...) {
                                    g_pendingRunAsync--;
                                    throw;
                                }
                            }
                        }
                        catch (...) {}
                    }
                    });
                {
                    std::lock_guard<std::mutex> lock2(g_hooksMutex);
                    g_sizeChangedHooks.push_back({winrt::make_weak(element), token});
                }
                {
                    std::lock_guard<std::mutex> lock(g_weatherGridMutex);
                    g_injectedWeatherGrids.push_back(winrt::make_weak(weatherGrid));
                }
                return true;
            }
        }
    }
    int count = VisualTreeHelper::GetChildrenCount(element);
    for (int i = 0; i < count; i++) {
        auto child = VisualTreeHelper::GetChild(element, i).try_as<FrameworkElement>();
        if (child && FindAndInjectWidgetsButton(child)) {
            return true;
        }
    }
    return false;
}
// Fallback helper
bool FindAndInjectRepeater(FrameworkElement element) {
    if (!element) return false;
    std::wstring name(element.Name());
    std::wstring className(winrt::get_class_name(element).c_str());
    if (name == L"TaskbarFrameRepeater" ||
        className == L"Taskbar.TaskbarFrameRepeater" ||
        className == L"TaskbarFrameRepeater" ||
        className == L"Microsoft.UI.Xaml.Controls.ItemsRepeater" ||
        className == L"ItemsRepeater") {
        auto parent = VisualTreeHelper::GetParent(element);
        if (parent) {
            if (auto parentGrid = parent.try_as<Grid>()) {
                InjectContentIntoGrid(parentGrid, L"WhWeatherHostGrid");
                return true;
            }
        }
    }
    int count = VisualTreeHelper::GetChildrenCount(element);
    for (int i = 0; i < count; i++) {
        auto child = VisualTreeHelper::GetChild(element, i).try_as<FrameworkElement>();
        if (child && FindAndInjectRepeater(child)) {
            return true;
        }
    }
    return false;
}
// Scan on the XAML thread. A missed/late Widgets template must remain retryable.
bool ScanAndInjectRecursive(FrameworkElement element) {
    if (!element || g_modUnloaded) return false;
    if (g_injectToSysTray) return FindAndInjectRepeater(element);
    bool injected = FindAndInjectWidgetsButton(element);
    if (injected) g_isInitScan = false;
    return injected;
}
std::atomic<bool> g_scanPending = false;
// Observed frames are retained weakly for settings changes, not as proof that
// injection succeeded. The host can disappear when the shell rebuilds a template.
std::vector<winrt::weak_ref<FrameworkElement>> g_scannedFrames;
std::mutex g_pendingMutex;
void ScheduleScanAsync(FrameworkElement startNode) {
    if (!startNode || g_modUnloaded || g_scanPending.exchange(true)) return;
    bool counted = false;
    try {
        auto weak = winrt::make_weak(startNode);
        auto dispatcher = startNode.Dispatcher();
        if (!dispatcher) { g_scanPending = false; return; }
        g_pendingRunAsync++;
        counted = true;
        dispatcher.RunAsync(
            winrt::Windows::UI::Core::CoreDispatcherPriority::Low, [weak]() {
                struct ScanComplete {
                    ~ScanComplete() { g_scanPending = false; g_pendingRunAsync--; }
                } complete;
                if (g_modUnloaded) return;
                try {
                    auto node = weak.get();
                    if (!node) return;
                    FrameworkElement current = node;
                    while (current) {
                        if (winrt::get_class_name(current) == L"Taskbar.TaskbarFrame") {
                            {
                                std::lock_guard<std::mutex> lock(g_pendingMutex);
                                bool known = false;
                                auto it = g_scannedFrames.begin();
                                while (it != g_scannedFrames.end()) {
                                    if (auto existing = it->get()) {
                                        if (winrt::get_abi(existing) == winrt::get_abi(current)) known = true;
                                        ++it;
                                    } else {
                                        it = g_scannedFrames.erase(it);
                                    }
                                }
                                if (!known) g_scannedFrames.push_back(winrt::make_weak(current));
                            }
                            // Avoid rebuilding weather UI on every native hover update.
                            // If the shell replaced the host, scan the whole frame again.
                            if (FindElementInTree(current, L"WhWeatherHostGrid", L"")) return;
                            if (ScanAndInjectRecursive(current) && g_debugLogs)
                                Wh_Log(L"[Wh_WeatherHost] Injected weather into taskbar frame (tray=%d)", g_injectToSysTray);
                            return;
                        }
                        auto parent = VisualTreeHelper::GetParent(current);
                        current = parent ? parent.try_as<FrameworkElement>() : nullptr;
                    }
                    if (node.Parent()) ScanAndInjectRecursive(node);
                } catch (winrt::hresult_error const& e) {
                    if (g_debugLogs) Wh_Log(L"[Wh_WeatherHost] Deferred scan failed: 0x%08X", (unsigned)e.code().value);
                } catch (...) {
                    if (g_debugLogs) Wh_Log(L"[Wh_WeatherHost] Deferred scan failed");
                }
            });
    } catch (...) {
        if (counted) g_pendingRunAsync--;
        g_scanPending = false;
    }
}
FrameworkElement GetFrameworkElementFromNative(void* pThis) {
    try {
        void* iUnknownPtr = (void**)pThis + 3;
        winrt::Windows::Foundation::IUnknown iUnknown;
        winrt::copy_from_abi(iUnknown, iUnknownPtr);
        return iUnknown.try_as<FrameworkElement>();
    }
    catch (...) {
        return nullptr;
    }
}
// --- Hooks ---
using TaskListButton_UpdateVisualStates_t = void(WINAPI*)(void*);
TaskListButton_UpdateVisualStates_t TaskListButton_UpdateVisualStates_Original;
void WINAPI TaskListButton_UpdateVisualStates_Hook(void* pThis) {
    TaskListButton_UpdateVisualStates_Original(pThis);
    if (auto elem = GetFrameworkElementFromNative(pThis)) {
        ScheduleScanAsync(elem);
    }
}
bool HookTaskbarViewDllSymbols(HMODULE module) {
    WindhawkUtils::SYMBOL_HOOK symbolHooks[] = {
        {{LR"(private: void __cdecl winrt::Taskbar::implementation::TaskListButton::UpdateVisualStates(void))"},
         (void**)&TaskListButton_UpdateVisualStates_Original,
         (void*)TaskListButton_UpdateVisualStates_Hook} };
    return HookSymbols(module, symbolHooks, ARRAYSIZE(symbolHooks));
}
typedef struct _UNICODE_STRING_MINI {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING_MINI, * PUNICODE_STRING_MINI;
using LdrLoadDll_t = NTSTATUS(NTAPI*)(
    PWSTR PathToFile,
    ULONG* Flags,
    PUNICODE_STRING_MINI ModuleFileName,
    PHANDLE ModuleHandle
    );
LdrLoadDll_t LdrLoadDll_Original = nullptr;
NTSTATUS NTAPI LdrLoadDll_Hook(
    PWSTR PathToFile,
    ULONG* Flags,
    PUNICODE_STRING_MINI ModuleFileName,
    PHANDLE ModuleHandle
) {
    NTSTATUS status = LdrLoadDll_Original(PathToFile, Flags, ModuleFileName, ModuleHandle);
    if (status >= 0 && ModuleHandle && *ModuleHandle && ModuleFileName && ModuleFileName->Buffer) {
        if (!g_taskbarViewDllLoaded) {
            std::wstring name(ModuleFileName->Buffer, ModuleFileName->Length / sizeof(wchar_t));
            for (auto& c : name) {
                c = std::towlower(c);
            }
            if (name.find(L"taskbar.view.dll") != std::wstring::npos ||
                name.find(L"explorerextensions.dll") != std::wstring::npos) {
                if (!g_taskbarViewDllLoaded.exchange(true)) {
                    HookTaskbarViewDllSymbols((HMODULE)*ModuleHandle);
                    Wh_ApplyHookOperations();
                }
            }
        }
    }
    return status;
}
using LoadLibraryExW_t = decltype(&LoadLibraryExW);
LoadLibraryExW_t LoadLibraryExW_Original = nullptr;
HMODULE WINAPI LoadLibraryExW_Hook(LPCWSTR lpLibFileName,
    HANDLE hFile,
    DWORD dwFlags) {
    HMODULE module = LoadLibraryExW_Original(lpLibFileName, hFile, dwFlags);
    if (module && !g_taskbarViewDllLoaded && lpLibFileName) {
        std::wstring name(lpLibFileName);
        for (auto& c : name) {
            c = std::towlower(c);
        }
        if (name.find(L"taskbar.view.dll") != std::wstring::npos ||
            name.find(L"explorerextensions.dll") != std::wstring::npos) {
            if (!g_taskbarViewDllLoaded.exchange(true)) {
                HookTaskbarViewDllSymbols(module);
                Wh_ApplyHookOperations();
            }
        }
    }
    return module;
}
HWND g_hSubclassedWnd = NULL;
HWND g_hSubclassedTaskListWnd = NULL;
UINT g_msgHotkeyControl = 0;
bool g_win10WeatherHovered = false;
bool g_win10WeatherPressed = false;
bool g_win10TrackingMouse = false;
typedef BOOL(WINAPI* PFN_ALPHABLEND)(HDC, int, int, int, int, HDC, int, int, int, int, BLENDFUNCTION);
bool DrawCachedWin10TaskbarSvg(
    HDC destination,
    const RECT& bounds,
    int iconSize,
    const std::wstring& expectedUrl) {
    if (!destination || iconSize <= 0) return false;

    std::lock_guard<std::mutex> lock(g_win10TaskbarSvgMutex);
    if (!g_win10TaskbarSvgBitmap || g_win10TaskbarSvgUrl != expectedUrl) {
        return false;
    }

    BITMAP bitmapInfo{};
    if (!GetObjectW(
            g_win10TaskbarSvgBitmap, sizeof(bitmapInfo), &bitmapInfo) ||
        bitmapInfo.bmWidth <= 0 || bitmapInfo.bmHeight <= 0) {
        return false;
    }

    int availableWidth = bounds.right - bounds.left;
    int availableHeight = bounds.bottom - bounds.top;
    int drawSize = (std::min)(iconSize, (std::min)(availableWidth, availableHeight));
    if (drawSize <= 0) return false;
    int drawX = bounds.left + (availableWidth - drawSize) / 2;
    int drawY = bounds.top + (availableHeight - drawSize) / 2;

    HMODULE msimg32 = GetModuleHandleW(L"msimg32.dll");
    if (!msimg32) msimg32 = LoadLibraryW(L"msimg32.dll");
    auto alphaBlend = msimg32
        ? reinterpret_cast<PFN_ALPHABLEND>(
              GetProcAddress(msimg32, "AlphaBlend"))
        : nullptr;
    if (!alphaBlend) return false;

    HDC sourceDc = CreateCompatibleDC(destination);
    if (!sourceDc) return false;
    HGDIOBJ oldBitmap = SelectObject(sourceDc, g_win10TaskbarSvgBitmap);
    BLENDFUNCTION blend{};
    blend.BlendOp = AC_SRC_OVER;
    blend.SourceConstantAlpha = 255;
    blend.AlphaFormat = AC_SRC_ALPHA;
    BOOL drawn = alphaBlend(
        destination, drawX, drawY, drawSize, drawSize,
        sourceDc, 0, 0, bitmapInfo.bmWidth, bitmapInfo.bmHeight, blend);
    SelectObject(sourceDc, oldBitmap);
    DeleteDC(sourceDc);
    return drawn != FALSE;
}

void AlphaBlendSolidColor(HDC hdcDest, int x, int y, int w, int h, COLORREF color, BYTE alpha) {
    HMODULE hMsimg32 = GetModuleHandleW(L"msimg32.dll");
    if (!hMsimg32) {
        hMsimg32 = LoadLibraryW(L"msimg32.dll");
    }
    if (hMsimg32) {
        PFN_ALPHABLEND pfnAlphaBlend = (PFN_ALPHABLEND)GetProcAddress(hMsimg32, "AlphaBlend");
        if (pfnAlphaBlend) {
            HDC hdcMem = CreateCompatibleDC(hdcDest);
            if (hdcMem) {
                BITMAPINFO bmi = { {0} };
                bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                bmi.bmiHeader.biWidth = 1;
                bmi.bmiHeader.biHeight = 1;
                bmi.bmiHeader.biPlanes = 1;
                bmi.bmiHeader.biBitCount = 32;
                bmi.bmiHeader.biCompression = BI_RGB;
                void* pBits = nullptr;
                HBITMAP hBmp = CreateDIBSection(hdcDest, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
                if (hBmp) {
                    HBITMAP hOldBmp = (HBITMAP)SelectObject(hdcMem, hBmp);
                    DWORD* pPixel = (DWORD*)pBits;
                    BYTE r = GetRValue(color) * alpha / 255;
                    BYTE g = GetGValue(color) * alpha / 255;
                    BYTE b = GetBValue(color) * alpha / 255;
                    *pPixel = (alpha << 24) | (r << 16) | (g << 8) | b;
                    BLENDFUNCTION bf = { 0 };
                    bf.BlendOp = AC_SRC_OVER;
                    bf.SourceConstantAlpha = 255;
                    bf.AlphaFormat = AC_SRC_ALPHA;
                    pfnAlphaBlend(hdcDest, x, y, w, h, hdcMem, 0, 0, 1, 1, bf);
                    SelectObject(hdcMem, hOldBmp);
                    DeleteObject(hBmp);
                }
                DeleteDC(hdcMem);
            }
        }
    }
}
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
int GetRequiredWeatherWidth(HWND hWnd) {
    if (g_modUnloaded) {
        return 0;
    }
    auto currentWeather = GetCurrentWeatherSnapshot();
    std::wstring icon = currentWeather.icon;
    std::wstring temp = currentWeather.acquired ? FormatTempDisplay(currentWeather.temp) : L"Weather";
    std::wstring cond = currentWeather.condition;
    if (!currentWeather.acquired) {
        icon = L"⏳";
        temp = L"Weather";
        cond = currentWeather.condition == L"Location unavailable"
            ? currentWeather.condition : L"Loading...";
    }
    int minimumWidth = g_showConditionName ? 115 : 55;
    HDC hdc = hWnd ? GetDC(hWnd) : GetDC(NULL);
    if (!hdc) return minimumWidth;
    std::wstring l1Family = g_line1FontFamily.empty() ? L"Segoe UI" : g_line1FontFamily;
    std::wstring l2Family = g_line2FontFamily.empty() ? L"Segoe UI" : g_line2FontFamily;
    HFONT hFontTemp = CreateFontW(
        -g_line1FontSize, 0, 0, 0, g_line1Bold ? FW_BOLD : FW_NORMAL, FALSE,
        FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, l1Family.c_str());
    HFONT hFontCond = CreateFontW(
        -g_line2FontSize, 0, 0, 0, g_line2Bold ? FW_BOLD : FW_NORMAL, FALSE,
        FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, l2Family.c_str());
    int baseIconSize = (g_weatherStyle == 1) ? g_iconFontSize + 1 : g_iconFontSize;
    HFONT hFontIcon = CreateFontW(
        -(baseIconSize), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        (g_weatherStyle == 1) ? L"Segoe UI Symbol" : L"Segoe UI Emoji");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFontIcon);
    SIZE szIcon = { 0 };
    GetTextExtentPoint32W(hdc, icon.c_str(), icon.length(), &szIcon);
    if (g_weatherStyle == 2 && currentWeather.acquired) {
        RECT windowRect{};
        bool verticalTaskbar = hWnd && GetWindowRect(hWnd, &windowRect) &&
            (windowRect.bottom - windowRect.top) >
                (windowRect.right - windowRect.left);
        if (verticalTaskbar) {
            szIcon.cx = baseIconSize;
            szIcon.cy = baseIconSize;
        } else {
            // The classic horizontal taskbar historically used the measured
            // fallback glyph width as the SVG slot. Preserve that geometry;
            // forcing a smaller square slot shifts the group right and clips
            // the downloaded SVG on ExplorerPatcher.
            if (szIcon.cx < static_cast<LONG>(baseIconSize)) {
                szIcon.cx = static_cast<LONG>(baseIconSize);
            }
        }
    }
    int maxTextWidth = 0;
    SIZE szTemp = { 0 }, szCond = { 0 };
    if (g_showConditionName) {
        SelectObject(hdc, hFontTemp);
        GetTextExtentPoint32W(hdc, temp.c_str(), temp.length(), &szTemp);
        SelectObject(hdc, hFontCond);
        GetTextExtentPoint32W(hdc, cond.c_str(), cond.length(), &szCond);
        maxTextWidth = szTemp.cx > szCond.cx ? szTemp.cx : szCond.cx;
    }
    else {
        SelectObject(hdc, hFontTemp);
        GetTextExtentPoint32W(hdc, temp.c_str(), temp.length(), &szTemp);
        maxTextWidth = szTemp.cx;
    }
    SelectObject(hdc, hOldFont);
    DeleteObject(hFontTemp);
    DeleteObject(hFontCond);
    DeleteObject(hFontIcon);
    if (hWnd) ReleaseDC(hWnd, hdc);
    else ReleaseDC(NULL, hdc);
    // Dynamic width calculation
    int spacing = (g_weatherStyle == 0) ? 2 : 8;
    // Allow breathing room on both sides of the measured text. Four pixels
    // was too tight against the adjacent notification icon at some scales.
    int requiredWidth = szIcon.cx + spacing + maxTextWidth + 24;
    if (requiredWidth < minimumWidth) {
        requiredWidth = minimumWidth;
    }
    // Set a sensible maximum width to avoid taking over the whole taskbar under weird inputs (e.g. 250px)
    if (requiredWidth > 250) {
        requiredWidth = 250;
    }
    return requiredWidth;
}
COLORREF GetIconColor(const std::wstring& condition) {
    if (condition == L"Clear" || condition == L"Sunny" || condition.find(L"Sun") != std::wstring::npos || condition.find(L"Sunny") != std::wstring::npos) return RGB(250, 214, 53); // Google-like Yellow
    if (condition == L"Partly Cloudy") return RGB(200, 200, 200); // Light Gray
    if (condition == L"Cloudy") return RGB(150, 150, 150); // Gray
    if (condition.find(L"Rain") != std::wstring::npos || condition.find(L"Showers") != std::wstring::npos || condition.find(L"Drizzle") != std::wstring::npos) return RGB(0, 162, 232); // Blue
    if (condition.find(L"Storm") != std::wstring::npos || condition.find(L"Thunderstorm") != std::wstring::npos) return RGB(163, 73, 164); // Purple
    if (condition.find(L"Snow") != std::wstring::npos) return IsSystemDarkMode() ? RGB(255, 255, 255) : RGB(120, 120, 120); // Snow is gray on light theme
    if (condition == L"Foggy") return RGB(127, 127, 127); // Deep Gray
    return g_textColor; // Fallback to taskbar text color
}
void DrawColorEmojiDirect2D(HDC hdc, const std::wstring& text, RECT rc, int fontSize) {
    if (!g_pD2DFactory) {
        D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &g_pD2DFactory);
    }
    if (!g_pD2DFactory) return;
    if (!g_pDWriteFactory) {
        DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&g_pDWriteFactory));
    }
    if (!g_pDWriteFactory) return;
    ID2D1DCRenderTarget* pDCRenderTarget = nullptr;
    D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
    );
    HRESULT hr = g_pD2DFactory->CreateDCRenderTarget(&props, &pDCRenderTarget);
    if (SUCCEEDED(hr) && pDCRenderTarget) {
        RECT bindRect = rc;
        hr = pDCRenderTarget->BindDC(hdc, &bindRect);
        if (SUCCEEDED(hr)) {
            pDCRenderTarget->BeginDraw();
            IDWriteTextFormat* pTextFormat = nullptr;
            hr = g_pDWriteFactory->CreateTextFormat(
                L"Segoe UI Emoji",
                nullptr,
                DWRITE_FONT_WEIGHT_NORMAL,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                (float)fontSize,
                L"",
                &pTextFormat
            );
            if (SUCCEEDED(hr) && pTextFormat) {
                pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
                pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
                IDWriteTextLayout* pTextLayout = nullptr;
                hr = g_pDWriteFactory->CreateTextLayout(
                    text.c_str(),
                    text.length(),
                    pTextFormat,
                    (float)(rc.right - rc.left),
                    (float)(rc.bottom - rc.top),
                    &pTextLayout
                );
                if (SUCCEEDED(hr) && pTextLayout) {
                    ID2D1DeviceContext* pDeviceContext = nullptr;
                    hr = pDCRenderTarget->QueryInterface(__uuidof(ID2D1DeviceContext), reinterpret_cast<void**>(&pDeviceContext));
                    if (SUCCEEDED(hr) && pDeviceContext) {
                        ID2D1SolidColorBrush* pBrush = nullptr;
                        pDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &pBrush);
                        pDeviceContext->DrawTextLayout(
                            D2D1::Point2F(0.f, 0.f),
                            pTextLayout,
                            pBrush,
                            D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT
                        );
                        if (pBrush) pBrush->Release();
                        pDeviceContext->Release();
                    }
                    else {
                        ID2D1SolidColorBrush* pBrush = nullptr;
                        pDCRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &pBrush);
                        pDCRenderTarget->DrawTextLayout(
                            D2D1::Point2F(0.f, 0.f),
                            pTextLayout,
                            pBrush
                        );
                        if (pBrush) pBrush->Release();
                    }
                    pTextLayout->Release();
                }
                pTextFormat->Release();
            }
            pDCRenderTarget->EndDraw();
        }
        pDCRenderTarget->Release();
    }
}
void PaintWin10Weather(HWND hWnd, HDC hdc) {
    if (g_modUnloaded)
        return;
    auto currentWeather = GetCurrentWeatherSnapshot();
    std::wstring icon = currentWeather.icon;
    std::wstring temp = currentWeather.acquired ? FormatTempDisplay(currentWeather.temp) : L"Weather";
    std::wstring cond = currentWeather.condition;
    if (!currentWeather.acquired) {
        icon = L"⏳";
        temp = L"Weather";
        cond = currentWeather.condition == L"Location unavailable"
            ? currentWeather.condition : L"Loading...";
    }
    RECT rcWin;
    GetWindowRect(hWnd, &rcWin);
    int winWidth = rcWin.right - rcWin.left;
    int winHeight = rcWin.bottom - rcWin.top;
    bool isHorizontal = winWidth > winHeight;
    int weatherSpace = GetRequiredWeatherWidth(hWnd);
    int weatherWidth = isHorizontal ? weatherSpace : winWidth;
    int height = isHorizontal ? winHeight : weatherSpace;
    // Fix for Issue 2: GDI text rendering on DWM composited surfaces sets alpha to 0. 
    // We must use BufferedPaint and DrawThemeTextEx to fix faint/transparent text in HDR.
    RECT rcParentBg = { 0, 0, weatherWidth, height };
    // This paints the clock's non-client area. Clear the back buffer before
    // drawing the taskbar background so old pixels cannot survive a repaint.
    BP_PAINTPARAMS params = {
        sizeof(BP_PAINTPARAMS), BPPF_ERASE | BPPF_NONCLIENT, NULL, NULL };
    HDC hdcPaint = NULL;
    HPAINTBUFFER hBufferedPaint = BeginBufferedPaint(hdc, &rcParentBg, BPBF_TOPDOWNDIB, &params, &hdcPaint);
    HDC drawDc = hBufferedPaint ? hdcPaint : hdc;
    SetBkMode(drawDc, TRANSPARENT);
    SetTextColor(drawDc, g_textColor);
    // Draw the clock window's parent background first so we clear any ghost hover colors
    DrawThemeParentBackground(hWnd, drawDc, &rcParentBg);
    // Erase the 1px taskbar separator line that DrawThemeParentBackground copies by duplicating a clean column/row
    if (isHorizontal) {
        if (weatherWidth > 6) {
            for (int i = 0; i < 3; ++i) {
                BitBlt(drawDc, i, 0, 1, height, drawDc, 5, 0, SRCCOPY);
            }
        }
    }
    else {
        if (height > 6) {
            for (int i = 0; i < 3; ++i) {
                BitBlt(drawDc, 0, i, weatherWidth, 1, drawDc, 0, 5, SRCCOPY);
            }
        }
    }
    // Draw hover/pressed background first
    if (g_win10WeatherHovered || g_win10WeatherPressed) {
        COLORREF hoverBgColor = RGB(255, 255, 255);
        if ((GetRValue(g_textColor) + GetGValue(g_textColor) + GetBValue(g_textColor)) / 3 < 128) {
            hoverBgColor = RGB(0, 0, 0); // dark text, light taskbar -> black hover
        }
        BYTE alpha = g_win10WeatherPressed ? 51 : 26; // 20% pressed, 10% hovered
        AlphaBlendSolidColor(drawDc, 0, 0, weatherWidth, height, hoverBgColor, alpha);
    }
    HTHEME hTheme = OpenThemeData(hWnd, L"Taskbar");
    DTTOPTS dttOpts = { sizeof(DTTOPTS) };
    dttOpts.dwFlags = DTT_TEXTCOLOR | DTT_COMPOSITED;
    dttOpts.crText = g_textColor;
    auto DrawTextAlpha = [&](HFONT font, const std::wstring& str, RECT& rc, UINT flags, bool useColor = false, COLORREF customColor = 0) {
        SelectObject(drawDc, font);
        if (hTheme && hBufferedPaint && !useColor) {
            DTTOPTS opts = dttOpts;
            opts.crText = (customColor != 0) ? customColor : g_textColor;
            DrawThemeTextEx(hTheme, drawDc, 0, 0, str.c_str(), -1, flags, &rc, &opts);
        }
        else {
            COLORREF oldCol = GetTextColor(drawDc);
            if (!useColor) {
                SetTextColor(drawDc, customColor != 0 ? customColor : g_textColor);
            }
            DrawTextW(drawDc, str.c_str(), -1, &rc, flags);
            if (!useColor) {
                SetTextColor(drawDc, oldCol);
            }
        }
        };
    std::wstring l1Family = g_line1FontFamily.empty() ? L"Segoe UI" : g_line1FontFamily;
    std::wstring l2Family = g_line2FontFamily.empty() ? L"Segoe UI" : g_line2FontFamily;
    HFONT hFontTemp = CreateFontW(
        -g_line1FontSize, 0, 0, 0, g_line1Bold ? FW_BOLD : FW_NORMAL, FALSE,
        FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, l1Family.c_str());
    HFONT hFontCond = CreateFontW(
        -g_line2FontSize, 0, 0, 0, g_line2Bold ? FW_BOLD : FW_NORMAL, FALSE,
        FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, l2Family.c_str());
    int baseIconSize = (g_weatherStyle == 1) ? g_iconFontSize + 1 : g_iconFontSize;
    HFONT hFontIcon = CreateFontW(
        -(baseIconSize), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        (g_weatherStyle == 1) ? L"Segoe UI Symbol" : L"Segoe UI Emoji");
    HFONT hOldFont = (HFONT)SelectObject(drawDc, hFontIcon);
    // Calculate precise centering
    SIZE szIcon = { 0 };
    GetTextExtentPoint32W(drawDc, icon.c_str(), icon.length(), &szIcon);
    if (g_weatherStyle == 2 && currentWeather.acquired) {
        if (!isHorizontal) {
            szIcon.cx = baseIconSize;
            szIcon.cy = baseIconSize;
        } else {
            if (szIcon.cx < static_cast<LONG>(baseIconSize)) {
                szIcon.cx = static_cast<LONG>(baseIconSize);
            }
        }
    }
    if (szIcon.cx <= 0) {
        szIcon.cx = baseIconSize;
    }
    int maxTextWidth = 0;
    SIZE szTemp = { 0 }, szCond = { 0 };
    if (g_showConditionName) {
        SelectObject(drawDc, hFontTemp);
        GetTextExtentPoint32W(drawDc, temp.c_str(), temp.length(), &szTemp);
        SelectObject(drawDc, hFontCond);
        GetTextExtentPoint32W(drawDc, cond.c_str(), cond.length(), &szCond);
        maxTextWidth = szTemp.cx > szCond.cx ? szTemp.cx : szCond.cx;
    }
    else {
        SelectObject(drawDc, hFontTemp);
        GetTextExtentPoint32W(drawDc, temp.c_str(), temp.length(), &szTemp);
        maxTextWidth = szTemp.cx;
    }
    bool useColorEmoji = !currentWeather.acquired || g_weatherStyle == 0 ||
        g_weatherStyle == 2 || g_weatherStyle == 3 || g_weatherStyle == 4;
    COLORREF iconCol = GetIconColor(cond);
    if (!isHorizontal) {
        // Vertical stacked layout: Icon on top, Temp below it, centered horizontally
        int iconH = szIcon.cy > 0 ? szIcon.cy : baseIconSize;
        int tempH = szTemp.cy > 0 ? szTemp.cy : 16;
        int stackSpacing = 1;
        int totalHeight = iconH + stackSpacing + tempH;
        int startY = (height - totalHeight) / 2;
        if (startY < 1) startY = 1;
        int iconX = (weatherWidth - szIcon.cx) / 2;
        if (iconX < 0) iconX = 0;
        RECT rcIcon = { iconX, startY, iconX + szIcon.cx, startY + iconH };
        bool drewSvg = g_weatherStyle == 2 && currentWeather.acquired &&
            DrawCachedWin10TaskbarSvg(
                drawDc, rcIcon, baseIconSize, currentWeather.iconUrl);
        if (!drewSvg && useColorEmoji) {
            RECT rcEmoji = rcIcon;
            rcEmoji.top -= 1;
            rcEmoji.bottom -= 1;
            DrawColorEmojiDirect2D(drawDc, icon, rcEmoji, baseIconSize);
        }
        else if (!drewSvg) {
            RECT rcVector = rcIcon;
            rcVector.top -= 1;
            rcVector.bottom += 4;
            DrawTextAlpha(hFontIcon, icon, rcVector, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX | DT_NOCLIP, false, iconCol);
        }
        int tempY = startY + iconH + stackSpacing;
        RECT rcTemp = { 0, tempY, weatherWidth, tempY + tempH };
        DrawTextAlpha(hFontTemp, temp, rcTemp, DT_SINGLELINE | DT_CENTER | DT_TOP | DT_NOPREFIX | DT_NOCLIP);
    }
    else {
        // Horizontal layout (original side-by-side logic)
        int spacing = (g_weatherStyle == 0) ? 2 : 8;
        int totalWidth = szIcon.cx + spacing + maxTextWidth;
        int startX = (weatherWidth - totalWidth) / 2;
        if (g_weatherStyle == 0) {
            startX -= 6; // Keep emoji icons exactly where they are
        }
        else {
            startX += 0; // Move assets icons right by 6px relative to emoji icons (i.e. -6 + 6 = 0)
        }
        if (startX < 4) startX = 4;
        RECT rcIcon = { startX, 1, startX + szIcon.cx, height + 1 };
        bool drewSvg = g_weatherStyle == 2 && currentWeather.acquired &&
            DrawCachedWin10TaskbarSvg(
                drawDc, rcIcon, baseIconSize, currentWeather.iconUrl);
        if (!drewSvg && useColorEmoji) {
            RECT rcEmoji = rcIcon;
            rcEmoji.top -= 1; // minor optical adjustment to move emoji up slightly
            rcEmoji.bottom -= 1;
            DrawColorEmojiDirect2D(drawDc, icon, rcEmoji, baseIconSize);
        }
        else if (!drewSvg) {
            RECT rcVector = rcIcon;
            rcVector.top -= 2;
            rcVector.bottom -= 1;
            DrawTextAlpha(hFontIcon, icon, rcVector, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_NOPREFIX | DT_NOCLIP, false, iconCol);
        }
        int textLeft = startX + szIcon.cx + spacing;
        int textWidth = weatherWidth - textLeft;
        if (g_showConditionName) {
            RECT rcTemp = { textLeft, 0, textLeft + textWidth, height / 2 + 1 };
            DrawTextAlpha(hFontTemp, temp, rcTemp, DT_SINGLELINE | DT_BOTTOM | DT_LEFT | DT_NOPREFIX | DT_NOCLIP);
            RECT rcCond = { textLeft, height / 2 - 1, textLeft + textWidth, height };
            DrawTextAlpha(hFontCond, cond, rcCond, DT_SINGLELINE | DT_TOP | DT_LEFT | DT_NOPREFIX | DT_NOCLIP);
        }
        else {
            RECT rcTemp = { textLeft, 0, textLeft + textWidth, height };
            DrawTextAlpha(hFontTemp, temp, rcTemp, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_NOPREFIX | DT_NOCLIP);
        }
    }
    if (hTheme) CloseThemeData(hTheme);
    if (hBufferedPaint) EndBufferedPaint(hBufferedPaint, TRUE);
    SelectObject(drawDc, hOldFont);
    DeleteObject(hFontTemp);
    DeleteObject(hFontCond);
    DeleteObject(hFontIcon);
}
static const GUID IID_IDesktopWindowXamlSourceNative = { 0x3cbcf1bf, 0x2f76, 0x4e9c, { 0x96, 0xab, 0xe8, 0x4b, 0x37, 0x97, 0x25, 0x54 } };
MIDL_INTERFACE("3cbcf1bf-2f76-4e9c-96ab-e84b37972554")
IDesktopWindowXamlSourceNative : public IUnknown{
public:
    virtual HRESULT STDMETHODCALLTYPE AttachToWindow(HWND parent) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_WindowHandle(HWND* value) = 0;
};

// Windows 10 XAML Islands are opaque by default. This ABI interface is
// documented by Microsoft for making every DesktopWindowXamlSource on the
// current XAML thread transparent, allowing HostBackdrop acrylic to compose
// cleanly from the first frame and during native-window resizes.
static const GUID IID_IXamlSourceTransparency = {
    0x06636c29, 0x5a17, 0x458d,
    { 0x8e, 0xa2, 0x24, 0x22, 0xd9, 0x97, 0xa9, 0x22 }
};
MIDL_INTERFACE("06636c29-5a17-458d-8ea2-2422d997a922")
IXamlSourceTransparency : public IInspectable {
public:
    virtual HRESULT STDMETHODCALLTYPE get_IsBackgroundTransparent(unsigned char* value) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_IsBackgroundTransparent(unsigned char value) = 0;
};

void EnableWin10XamlIslandTransparencyForCurrentThread() {
    try {
        auto xamlWindow = winrt::Windows::UI::Xaml::Window::Current();
        if (!xamlWindow) return;

        IXamlSourceTransparency* transparency = nullptr;
        HRESULT hr = ((::IUnknown*)winrt::get_unknown(xamlWindow))->QueryInterface(
            IID_IXamlSourceTransparency,
            reinterpret_cast<void**>(&transparency));
        if (SUCCEEDED(hr) && transparency) {
            hr = transparency->put_IsBackgroundTransparent(1);
            if (g_debugLogs && FAILED(hr)) {
                Wh_Log(L"[Wh_WeatherHost] IXamlSourceTransparency failed: 0x%08X", hr);
            }
            transparency->Release();
        } else if (g_debugLogs) {
            Wh_Log(L"[Wh_WeatherHost] IXamlSourceTransparency unavailable: 0x%08X", hr);
        }
    } catch (...) {
        if (g_debugLogs) {
            Wh_Log(L"[Wh_WeatherHost] Failed to enable XAML Island transparency");
        }
    }
}
// Global state removed - replaced with GetThreadXamlState()
std::atomic<DWORD> g_lastHideTickCount{0};
bool g_popupWasVisibleOnLButtonDown = false;
void CleanupXamlMedia(winrt::Windows::UI::Xaml::UIElement const& element) {
    if (!element) return;
    try {
        if (auto image = element.try_as<winrt::Windows::UI::Xaml::Controls::Image>()) {
            try {
                RevokeImageHooksForImage(image);
                image.Source(nullptr);
            }
            catch (...) {}
        }
        else if (auto media = element.try_as<winrt::Windows::UI::Xaml::Controls::MediaElement>()) {
            try {
                media.Stop();
                media.Source(nullptr);
            }
            catch (...) {}
        }
        else if (auto panel = element.try_as<winrt::Windows::UI::Xaml::Controls::Panel>()) {
            try {
                uint32_t size = panel.Children().Size();
                for (uint32_t i = 0; i < size; ++i) {
                    CleanupXamlMedia(panel.Children().GetAt(i));
                }
                panel.Children().Clear();
            }
            catch (...) {}
        }
        else if (auto border = element.try_as<winrt::Windows::UI::Xaml::Controls::Border>()) {
            try {
                if (auto imageBrush = border.Background().try_as<winrt::Windows::UI::Xaml::Media::ImageBrush>()) {
                    imageBrush.ImageSource(nullptr);
                }
                auto child = border.Child();
                if (child) {
                    CleanupXamlMedia(child);
                    border.Child(nullptr);
                }
            }
            catch (...) {}
        }
        else if (auto contentControl = element.try_as<winrt::Windows::UI::Xaml::Controls::ContentControl>()) {
            try {
                if (auto contentElem = contentControl.Content().try_as<winrt::Windows::UI::Xaml::UIElement>()) {
                    CleanupXamlMedia(contentElem);
                }
                contentControl.Content(nullptr);
            }
            catch (...) {}
        }
    }
    catch (...) {}
}
constexpr UINT_PTR kWin10FirstAcrylicTimer = 0x5741;
void HideWin10PopupInternal(HWND hWnd) {
    CancelAnimationEventsOnCurrentThread();
    StopRadarTimer();
    if (hWnd && IsWindow(hWnd)) {
        KillTimer(hWnd, kWin10FirstAcrylicTimer);
        ShowWindow(hWnd, SW_HIDE);
        g_win10PopupVisible.store(false, std::memory_order_release);
        ThreadXamlState& state = GetThreadXamlState();
        if (state.source) {
            try {
                auto oldContent = state.source.Content();
                if (oldContent) {
                    CleanupXamlMedia(oldContent);
                    if (auto grid = oldContent.try_as<winrt::Windows::UI::Xaml::Controls::Grid>()) {
                        grid.Children().Clear();
                    }
                }

                state.source.Content(nullptr);
            }
            catch (...) {}
        }
    }
}
void HideWin10Popup(HWND hWnd) {
    (void)hWnd;
    HWND popup = g_win10PopupHwnd.load(std::memory_order_acquire);
    if (popup && IsWindow(popup)) {
        PostMessageW(popup, WM_USER + 5002, 0, 0);
    }
}
void ApplyWin10Acrylic(HWND hWnd);
void ShowWin10ForecastPopupInternal(HWND hClock);
void UpdateWin10PopupInternal(HWND hWnd);
void EnsureWin10PopupThreadStarted();
void DispatchPendingWin10PopupOpenIfReady();
LRESULT CALLBACK Win10PopupWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == kForegroundChangedMessage) {
        HandleForegroundChange((HWND)wParam);
        return 0;
    }
    ThreadXamlState& state = GetThreadXamlState();
    if ((g_modUnloaded.load() || g_win10PopupInitState.load() == Win10PopupInitState::Stopping) &&
        (uMsg == WM_USER + 5001 || uMsg == WM_USER + 5003)) return 0;
    if (uMsg == WM_TIMER && wParam == kWin10FirstAcrylicTimer) {
        KillTimer(hWnd, kWin10FirstAcrylicTimer);
        if (g_modUnloaded.load() ||
            g_win10PopupInitState.load() == Win10PopupInitState::Stopping ||
            !IsWindowVisible(hWnd) || IsWindows11() || !g_useAcrylic || !state.source)
            return 0;
        try {
            if (auto root = state.source.Content().try_as<Grid>()) {
                auto color = IsSystemDarkMode()
                    ? winrt::Windows::UI::ColorHelper::FromArgb(255, 30, 30, 30)
                    : winrt::Windows::UI::ColorHelper::FromArgb(255, 240, 240, 240);
                AcrylicBrush brush;
                brush.BackgroundSource(AcrylicBackgroundSource::HostBackdrop);
                brush.TintColor(color);
                brush.TintOpacity(g_acrylicOpacity / 100.0);
                brush.FallbackColor(color);
                root.Background(brush);
                if (g_debugLogs) Wh_Log(L"[WeatherPopup] Deferred first acrylic reconnect; active=%d",
                    GetActiveWindow() == hWnd ? 1 : 0);
            }
        } catch (...) {
            if (g_debugLogs) Wh_Log(L"[WeatherPopup] Deferred first acrylic reconnect failed");
        }
        return 0;
    }
    if (uMsg == WM_ACTIVATE) {
        if (LOWORD(wParam) == WA_INACTIVE) {
            HWND hActive = (HWND)lParam;
            // If lParam is NULL, peek at the actual foreground window to verify click-away
            if (hActive == NULL) hActive = GetForegroundWindow();
            if (hActive != hWnd && hActive != state.sourceHwnd && !IsChild(hWnd, hActive)) {
                g_lastHideTickCount.store(GetTickCount(), std::memory_order_release);
                PostMessageW(hWnd, WM_USER + 5002, 0, 0); // Hide popup
                return 0;
            }
        }
    }
    else if (uMsg == (WM_USER + 5001)) {
        HWND hClock = (HWND)wParam;
        ShowWin10ForecastPopupInternal(hClock);
        return 0;
    }
    else if (uMsg == (WM_USER + 5002)) {
        HideWin10PopupInternal(hWnd);
        return 0;
    }
    else if (uMsg == (WM_USER + 5003)) {
        UpdateWin10PopupInternal(hWnd);
        return 0;
    }
    else if (uMsg == (WM_USER + 5004) || uMsg == (WM_USER + 4246)) {
        KillTimer(hWnd, kWin10FirstAcrylicTimer);
        g_win10PopupInitState.store(Win10PopupInitState::Stopping, std::memory_order_release);
        g_pendingWin10OpenClock.store(nullptr, std::memory_order_release);
        ShowWindow(hWnd, SW_HIDE);
        PostQuitMessage(0);
        return 0;
    }
    else if (uMsg == WM_SHOWWINDOW) {
        g_win10PopupVisible.store(wParam != FALSE, std::memory_order_release);
        if (wParam != FALSE) {
            MARGINS margins = { -1, -1, -1, -1 };
            DwmExtendFrameIntoClientArea(hWnd, &margins);
            if (!IsWindows11() && g_useAcrylic) {
                ApplyWin10Acrylic(hWnd);
            }
        }
    }
    else if (uMsg == WM_SIZE) {
        HWND hXAML = (HWND)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
        if (hXAML) {
            SetWindowPos(hXAML, NULL, 0, 0, LOWORD(lParam), HIWORD(lParam), SWP_NOZORDER | SWP_NOACTIVATE);
        }
    }
    else if (uMsg == WM_ERASEBKGND) {
        return 1;
    }
    else if (uMsg == WM_DESTROY) {
        KillTimer(hWnd, kWin10FirstAcrylicTimer);
        if (g_win10PopupHwnd.load(std::memory_order_acquire) == hWnd) {
            g_win10PopupVisible.store(false, std::memory_order_release);
            g_win10SourceHwnd.store(nullptr, std::memory_order_release);
            g_win10PopupHwnd.store(nullptr, std::memory_order_release);
        }
        if (state.source) {
            try {
                state.source.Close();
            }
            catch (...) {}
            state.source = nullptr;
        }
        state.sourceHwnd = NULL;
        state.popupHwnd = NULL;
    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
void RegisterWin10PopupClass() {
    static bool registered = false;
    if (registered) return;
    HINSTANCE hInstance = GetModuleHandleW(NULL);
    UnregisterClassW(L"WhWin10ForecastPopupClass", hInstance);
    WNDCLASSW wc = { 0 };
    wc.style = CS_DROPSHADOW;
    wc.lpfnWndProc = Win10PopupWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"WhWin10ForecastPopupClass";
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    RegisterClassW(&wc);
    registered = true;
}
BOOL CALLBACK HideCoreWindowCallback(HWND hwnd, LPARAM lParam) {
    wchar_t className[256];
    if (GetClassNameW(hwnd, className, 256)) {
        if (wcscmp(className, L"Windows.UI.Core.CoreWindow") == 0) {
            wchar_t title[256];
            if (GetWindowTextW(hwnd, title, 256)) {
                if (wcscmp(title, L"DesktopWindowXamlSource") == 0) {
                    ShowWindow(hwnd, SW_HIDE);
                    LONG exStyle = GetWindowLongW(hwnd, GWL_EXSTYLE);
                    SetWindowLongW(hwnd, GWL_EXSTYLE, exStyle | WS_EX_TOOLWINDOW);
                }
            }
        }
    }
    return TRUE;
}
void CloseWin10XamlManagerAndDrain() {
    auto& state = GetThreadXamlState();
    if (state.manager) {
        try { state.manager.Close(); }
        catch (winrt::hresult_error const& e) {
            Wh_Log(L"[WeatherPopup] WindowsXamlManager.Close failed: 0x%08X", static_cast<unsigned int>(e.code().value));
        }
        state.manager = nullptr;
    }
    MSG message{};
    unsigned int drained = 0;
    while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
        if (message.message == WM_QUIT) continue;
        TranslateMessage(&message);
        DispatchMessageW(&message);
        ++drained;
    }
    if (g_debugLogs) Wh_Log(L"[WeatherPopup] XAML shutdown queue drained (%u messages), tid=%lu", drained, GetCurrentThreadId());
}
DWORD WINAPI PopupThreadProc(LPVOID lpParam) {
    (void)lpParam;
    g_win10PopupInitState.store(Win10PopupInitState::Starting, std::memory_order_release);

    try {
        winrt::init_apartment(winrt::apartment_type::single_threaded);
    }
    catch (winrt::hresult_error const& e) {
        g_win10PopupInitState.store(Win10PopupInitState::Failed, std::memory_order_release);
        if (g_debugLogs) {
            Wh_Log(L"[Wh_WeatherHost] PopupThreadProc: init_apartment failed, hr=0x%08X: %s",
                   (unsigned int)e.code().value, e.message().c_str());
        }
        return 0;
    }
    catch (...) {
        g_win10PopupInitState.store(Win10PopupInitState::Failed, std::memory_order_release);
        if (g_debugLogs) Wh_Log(L"[Wh_WeatherHost] PopupThreadProc: init_apartment failed!");
        return 0;
    }

    ThreadXamlState& state = GetThreadXamlState();
    const ULONGLONG initStarted = GetTickCount64();
    if (g_debugLogs) Wh_Log(L"[WeatherPopup] Starting XAML initialization, pid=%lu tid=%lu", GetCurrentProcessId(), GetCurrentThreadId());
    DWORD retryDelayMs = 100;
    unsigned int attempt = 0;

    // Explorer can start us before desktop XAML Islands are ready. Keep this STA
    // alive and retry instead of permanently stranding the flyout until the mod
    // is toggled. A user click/hotkey signals g_hPopupRetryEvent and wakes an
    // in-progress backoff immediately.
    while (!g_modUnloaded.load(std::memory_order_relaxed) &&
           !g_bThreadShouldTerm.load(std::memory_order_relaxed)) {
        ++attempt;
        try {
            state.manager = winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager::InitializeForCurrentThread();
            break;
        }
        catch (winrt::hresult_error const& e) {
            state.manager = nullptr;
            g_win10PopupInitState.store(Win10PopupInitState::WaitingForXaml, std::memory_order_release);
            if (g_debugLogs) {
                Wh_Log(
                    L"[Wh_WeatherHost] PopupThreadProc: InitializeForCurrentThread attempt %u failed, hr=0x%08X: %s; retrying in %lu ms",
                    attempt, (unsigned int)e.code().value, e.message().c_str(), retryDelayMs);
            }
        }
        catch (...) {
            state.manager = nullptr;
            g_win10PopupInitState.store(Win10PopupInitState::WaitingForXaml, std::memory_order_release);
            if (g_debugLogs) {
                Wh_Log(
                    L"[Wh_WeatherHost] PopupThreadProc: InitializeForCurrentThread attempt %u failed; retrying in %lu ms",
                    attempt, retryDelayMs);
            }
        }

        if (g_modUnloaded.load(std::memory_order_relaxed) ||
            g_bThreadShouldTerm.load(std::memory_order_relaxed)) {
            break;
        }

        HANDLE retryEvent = g_hPopupRetryEvent;
        if (retryEvent) {
            WaitForSingleObject(retryEvent, retryDelayMs);
        } else {
            Sleep(retryDelayMs);
        }
        retryDelayMs = retryDelayMs >= 1000 ? 2000 : retryDelayMs * 2;
    }

    if (!state.manager || g_modUnloaded.load(std::memory_order_relaxed) ||
        g_bThreadShouldTerm.load(std::memory_order_relaxed)) {
        g_win10PopupInitState.store(Win10PopupInitState::Stopping, std::memory_order_release);
        CloseWin10XamlManagerAndDrain();
        try { winrt::uninit_apartment(); } catch (...) {}
        g_win10PopupInitState.store(Win10PopupInitState::NotStarted, std::memory_order_release);
        return 0;
    }

    EnableWin10XamlIslandTransparencyForCurrentThread();
    EnumThreadWindows(GetCurrentThreadId(), HideCoreWindowCallback, 0);
    RegisterWin10PopupClass();
    DWORD dwExStyle = WS_EX_TOPMOST | WS_EX_TOOLWINDOW;
    state.popupHwnd = CreateWindowExW(
        dwExStyle,
        L"WhWin10ForecastPopupClass", L"",
        WS_POPUP,
        0, 0, 100, 100,
        NULL, NULL, GetModuleHandleW(NULL), NULL
    );
    if (!state.popupHwnd) {
        g_win10PopupInitState.store(Win10PopupInitState::Failed, std::memory_order_release);
        if (g_debugLogs) Wh_Log(L"[Wh_WeatherHost] PopupThreadProc: CreateWindowExW failed, error %d", GetLastError());
        CloseWin10XamlManagerAndDrain();
        try { winrt::uninit_apartment(); } catch (...) {}
        return 0;
    }

    g_win10PopupHwnd.store(state.popupHwnd, std::memory_order_release);
    g_win10PopupVisible.store(false, std::memory_order_release);
    g_win10PopupInitState.store(Win10PopupInitState::Ready, std::memory_order_release);
    if (g_debugLogs) {
        Wh_Log(L"[Wh_WeatherHost] PopupThreadProc: XAML popup host ready after %u attempt(s), %llu ms, tid=%lu",
            attempt, static_cast<unsigned long long>(GetTickCount64() - initStarted), GetCurrentThreadId());
    }
    DispatchPendingWin10PopupOpenIfReady();

    MSG msg;
    BOOL messageResult;
    while ((messageResult = GetMessageW(&msg, NULL, 0, 0)) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    if (messageResult == -1) Wh_Log(L"[WeatherPopup] GetMessage failed: %lu", GetLastError());
    if (g_debugLogs) Wh_Log(L"[Wh_WeatherHost] PopupThreadProc: message loop exited");
    g_win10PopupInitState.store(Win10PopupInitState::Stopping, std::memory_order_release);

    // All XAML sources/timers cached by the Win10 flyout were created on this
    // STA. Release them here, before closing DesktopWindowXamlSource/XAML manager.
    CancelAnimationEventsOnCurrentThread();
    RevokeAllImageHooks();
    StopRadarTimer();
    try { g_radarTileBitmapCache.clear(); } catch (...) {}
    try { g_iconCache.clear(); } catch (...) {}

    if (state.source) {
        try {
            auto oldContent = state.source.Content();
            if (oldContent) {
                CleanupXamlMedia(oldContent);
                if (auto grid = oldContent.try_as<winrt::Windows::UI::Xaml::Controls::Grid>()) {
                    grid.Children().Clear();
                }
            }
            oldContent = nullptr;
            state.source.Content(nullptr);
        }
        catch (...) {}
        try { state.source.Close(); } catch (...) {}
        state.source = nullptr;
    }
    if (state.popupHwnd && IsWindow(state.popupHwnd)) DestroyWindow(state.popupHwnd);
    CloseWin10XamlManagerAndDrain();
    g_win10PopupVisible.store(false, std::memory_order_release);
    g_win10SourceHwnd.store(nullptr, std::memory_order_release);
    g_win10PopupHwnd.store(nullptr, std::memory_order_release);
    g_pendingWin10OpenClock.store(nullptr, std::memory_order_release);
    state.popupHwnd = nullptr;
    state.sourceHwnd = nullptr;
    try {
        winrt::uninit_apartment();
    }
    catch (...) {}
    g_win10PopupInitState.store(Win10PopupInitState::NotStarted, std::memory_order_release);
    return 0;
}

void DispatchPendingWin10PopupOpenIfReady() {
    if (g_modUnloaded.load() || g_bThreadShouldTerm.load()) return;
    HWND popup = g_win10PopupHwnd.load(std::memory_order_acquire);
    if (!popup || !IsWindow(popup) ||
        g_win10PopupInitState.load(std::memory_order_acquire) != Win10PopupInitState::Ready) {
        return;
    }

    HWND pendingClock = g_pendingWin10OpenClock.exchange(nullptr, std::memory_order_acq_rel);
    if (pendingClock && IsWindow(pendingClock)) {
        PostMessageW(popup, WM_USER + 5001, (WPARAM)pendingClock, 0);
    }
}

void EnsureWin10PopupThreadStarted() {
    if (ShouldUseXamlTaskbar() || g_modUnloaded.load(std::memory_order_relaxed) ||
        g_bThreadShouldTerm.load(std::memory_order_relaxed)) {
        return;
    }

    std::lock_guard<std::mutex> lock(g_win10PopupThreadMutex);
    if (g_modUnloaded.load() || g_bThreadShouldTerm.load()) return;

    if (g_hPopupThread) {
        DWORD waitResult = WaitForSingleObject(g_hPopupThread, 0);
        if (waitResult == WAIT_TIMEOUT) {
            if (g_hPopupRetryEvent) SetEvent(g_hPopupRetryEvent);
            return;
        }

        CloseHandle(g_hPopupThread);
        g_hPopupThread = nullptr;
        g_dwPopupThreadId = 0;
        g_win10PopupInitState.store(Win10PopupInitState::NotStarted, std::memory_order_release);
    }

    if (!g_hPopupRetryEvent) {
        g_hPopupRetryEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
        if (!g_hPopupRetryEvent) {
            if (g_debugLogs) {
                Wh_Log(L"[Wh_WeatherHost] EnsureWin10PopupThreadStarted: CreateEvent failed, error %d", GetLastError());
            }
            return;
        }
    }

    g_win10PopupInitState.store(Win10PopupInitState::Starting, std::memory_order_release);
    g_hPopupThread = CreateThread(NULL, 0, PopupThreadProc, NULL, 0, &g_dwPopupThreadId);
    if (!g_hPopupThread) {
        g_dwPopupThreadId = 0;
        g_win10PopupInitState.store(Win10PopupInitState::Failed, std::memory_order_release);
        if (g_debugLogs) {
            Wh_Log(L"[Wh_WeatherHost] EnsureWin10PopupThreadStarted: CreateThread failed, error %d", GetLastError());
        }
    }
}

double GetDpiScaleForWindow(HWND hWnd) {
    typedef UINT(WINAPI* GetDpiForWindow_t)(HWND);
    static GetDpiForWindow_t pfnGetDpiForWindow = nullptr;
    static bool resolved = false;
    if (!resolved) {
        HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
        if (hUser32) {
            pfnGetDpiForWindow = (GetDpiForWindow_t)GetProcAddress(hUser32, "GetDpiForWindow");
        }
        resolved = true;
    }
    if (pfnGetDpiForWindow && hWnd) {
        return (double)pfnGetDpiForWindow(hWnd) / 96.0;
    }
    HDC hdc = GetDC(NULL);
    int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
    ReleaseDC(NULL, hdc);
    return (double)dpiX / 96.0;
}
void ResizeWin10PopupToCurrentContent() {
    if (ShouldUseXamlTaskbar()) return;

    ThreadXamlState& state = GetThreadXamlState();
    if (!state.popupHwnd || !IsWindow(state.popupHwnd)) return;

    HWND hClock = g_hSubclassedWnd;
    if (!hClock || !IsWindow(hClock)) hClock = FindSystemClockWnd();
    if (!hClock || !IsWindow(hClock)) return;

    RECT clockRect{};
    if (!GetWindowRect(hClock, &clockRect)) return;

    double dpiScale = GetDpiScaleForWindow(hClock);
    int physicalHeight = (int)ceil(CalculatePanelHeight() * dpiScale);
    RECT popupRect{};
    GetWindowRect(state.popupHwnd, &popupRect);
    int physicalWidth = popupRect.right - popupRect.left;

    int edge = ABE_BOTTOM;
    RECT trayRect{};
    HWND hTaskbar = GetAncestor(hClock, GA_ROOT);
    if (!hTaskbar) hTaskbar = FindWindowW(L"Shell_TrayWnd", NULL);
    HMONITOR hMonitor = MonitorFromRect(&clockRect, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi{ sizeof(mi) };
    if (hTaskbar && GetWindowRect(hTaskbar, &trayRect) && GetMonitorInfoW(hMonitor, &mi)) {
        int trayWidth = trayRect.right - trayRect.left;
        int trayHeight = trayRect.bottom - trayRect.top;
        if (trayWidth > trayHeight) {
            edge = trayRect.top <= mi.rcMonitor.top + (mi.rcMonitor.bottom - mi.rcMonitor.top) / 2
                ? ABE_TOP : ABE_BOTTOM;
        } else {
            edge = trayRect.left <= mi.rcMonitor.left + (mi.rcMonitor.right - mi.rcMonitor.left) / 2
                ? ABE_LEFT : ABE_RIGHT;
        }
    }

    int weatherWidth = GetRequiredWeatherWidth(hClock);
    int x = (int)floor(clockRect.left + weatherWidth / 2.0 - physicalWidth / 2.0);
    int y = clockRect.top - physicalHeight - 10;
    if (edge == ABE_TOP) {
        y = clockRect.bottom + 10;
    } else if (edge == ABE_LEFT) {
        x = clockRect.right + 10;
        y = (int)floor(clockRect.top + weatherWidth / 2.0 - physicalHeight / 2.0);
    } else if (edge == ABE_RIGHT) {
        x = clockRect.left - physicalWidth - 10;
        y = (int)floor(clockRect.top + weatherWidth / 2.0 - physicalHeight / 2.0);
    }

    if (GetMonitorInfoW(hMonitor, &mi)) {
        RECT rcWork = mi.rcWork;
        if (x < rcWork.left + 15) x = rcWork.left + 15;
        if (y < rcWork.top + 15) y = rcWork.top + 15;
        if (x + physicalWidth > rcWork.right - 15) x = rcWork.right - physicalWidth - 15;
        if (y + physicalHeight > rcWork.bottom - 15) y = rcWork.bottom - physicalHeight - 15;
    }

    SetWindowPos(state.popupHwnd, NULL, x, y, physicalWidth, physicalHeight,
                 SWP_NOZORDER | SWP_NOACTIVATE);
}

void UpdateWin10PopupInternal(HWND hWnd) {
    ThreadXamlState& state = GetThreadXamlState();
    if (state.popupHwnd && IsWindowVisible(state.popupHwnd) && state.source) {
        try {
            winrt::Windows::UI::Xaml::Controls::Grid rootGrid;
            g_selectedDate = L"";
            g_selectedHour = L"";
            g_graphScrollOffset = -1.0;
            g_dailyScrollOffset = 0.0;
            auto currentWeather = GetCurrentWeatherSnapshot();
            PopulateForecastUI(rootGrid, currentWeather.condition, currentWeather.icon, currentWeather.temp);
            double panelWidth = 336.0;
            if (g_forecastDaysFetch >= 14)
                panelWidth = 570.0;
            else if (g_forecastDaysFetch >= 10)
                panelWidth = 492.0;
            rootGrid.Width(panelWidth);
            try {
                auto oldContent = state.source.Content();
                if (oldContent) {
                    CleanupXamlMedia(oldContent);
                }
            }
            catch (...) {}
            state.source.Content(rootGrid);
            ResizeWin10PopupToCurrentContent();
            RedrawWindow(state.popupHwnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
        }
        catch (...) {}
    }
}
struct ACCENT_POLICY_WEATHER {
    int State;
    int Flags;
    DWORD Color;
    int AnimationId;
};
struct WINDOWCOMPOSITIONATTRIBDATA_WEATHER {
    int Attrib;
    PVOID pvData;
    SIZE_T cbData;
};
typedef BOOL(WINAPI* SetWindowCompositionAttribute_t)(HWND, WINDOWCOMPOSITIONATTRIBDATA_WEATHER*);
void ApplyWin10Acrylic(HWND hWnd) {
    // Win10 acrylic is provided by the XAML HostBackdrop AcrylicBrush.
    // Keep this helper as a compatibility no-op for existing call sites.
    (void)hWnd;
}
void ShowWin10ForecastPopup(HWND hClock) {
    if (g_modUnloaded.load() || g_bThreadShouldTerm.load()) return;
    HWND popup = g_win10PopupHwnd.load(std::memory_order_acquire);
    if (popup && IsWindow(popup) &&
        g_win10PopupInitState.load(std::memory_order_acquire) == Win10PopupInitState::Ready) {
        PostMessageW(popup, WM_USER + 5001, (WPARAM)hClock, 0);
        return;
    }

    // Remember one requested open while Explorer's XAML Island infrastructure is
    // still becoming available. The popup STA consumes it once initialization
    // succeeds. Multiple early clicks intentionally coalesce into one open.
    HWND previousRequest = g_pendingWin10OpenClock.exchange(hClock, std::memory_order_acq_rel);
    if (!previousRequest && g_debugLogs) Wh_Log(L"[WeatherPopup] Open queued while XAML initializes; clock=%p", hClock);
    EnsureWin10PopupThreadStarted();
    DispatchPendingWin10PopupOpenIfReady();
}
void ShowWin10ForecastPopupInternal(HWND hClock) {
    ThreadXamlState& state = GetThreadXamlState();
    try {
        // This function is only invoked by the dedicated popup STA after its
        // WindowsXamlManager initialization has succeeded. Never initialize XAML
        // from the taskbar/click path as a fallback.
        if (!state.manager ||
            g_win10PopupInitState.load(std::memory_order_acquire) != Win10PopupInitState::Ready) {
            if (g_debugLogs) {
                Wh_Log(L"[Wh_WeatherHost] ShowWin10ForecastPopupInternal: popup XAML host is not ready");
            }
            return;
        }
        RegisterWin10PopupClass();
        // A fresh Win10 open always starts on Temperature, so reset state before
        // calculating the native host dimensions.
        g_selectedDate = L"";
        g_selectedHour = L"";
        g_selectedGraphTab = 0;
        g_graphScrollOffset = -1.0;
        g_dailyScrollOffset = 0.0;

        double panelWidth = 336.0;
        if (g_forecastDaysFetch >= 14)
            panelWidth = 570.0;
        else if (g_forecastDaysFetch >= 10)
            panelWidth = 492.0;
        double panelHeight = CalculatePanelHeight();
        double dpiScale = GetDpiScaleForWindow(hClock);
        int physicalWidth = (int)ceil(panelWidth * dpiScale);
        int physicalHeight = (int)ceil(panelHeight * dpiScale);
        RECT clockRect;
        GetWindowRect(hClock, &clockRect);
        // Dynamically and accurately calculate the taskbar edge for primary and secondary monitors
        int edge = ABE_BOTTOM;
        RECT trayRect;
        HWND hParentTaskbar = GetAncestor(hClock, GA_ROOT);
        if (!hParentTaskbar) {
            hParentTaskbar = FindWindowW(L"Shell_TrayWnd", NULL);
        }
        if (hParentTaskbar && GetWindowRect(hParentTaskbar, &trayRect)) {
            int trayWidth = trayRect.right - trayRect.left;
            int trayHeight = trayRect.bottom - trayRect.top;
            HMONITOR hMon = MonitorFromRect(&clockRect, MONITOR_DEFAULTTONEAREST);
            MONITORINFO mi = { sizeof(mi) };
            if (GetMonitorInfoW(hMon, &mi)) {
                if (trayWidth > trayHeight) {
                    if (trayRect.top <= mi.rcMonitor.top + (mi.rcMonitor.bottom - mi.rcMonitor.top) / 2) {
                        edge = ABE_TOP;
                    }
                    else {
                        edge = ABE_BOTTOM;
                    }
                }
                else {
                    if (trayRect.left <= mi.rcMonitor.left + (mi.rcMonitor.right - mi.rcMonitor.left) / 2) {
                        edge = ABE_LEFT;
                    }
                    else {
                        edge = ABE_RIGHT;
                    }
                }
            }
        }
        int weatherWidth = GetRequiredWeatherWidth(hClock);
        double offsetX = clockRect.left + (weatherWidth / 2.0) - (physicalWidth / 2.0);
        double offsetY = clockRect.top - physicalHeight - 10;
        if (edge == ABE_TOP) {
            offsetY = clockRect.bottom + 10;
        }
        else if (edge == ABE_LEFT) {
            offsetX = clockRect.right + 10;
            offsetY = clockRect.top;
        }
        else if (edge == ABE_RIGHT) {
            offsetX = clockRect.left - physicalWidth - 10;
            offsetY = clockRect.top;
        }
        // Keep it on the correct monitor and work area
        HMONITOR hMonitor = MonitorFromRect(&clockRect, MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi = { sizeof(mi) };
        if (GetMonitorInfoW(hMonitor, &mi)) {
            RECT rcWork = mi.rcWork;
            if (offsetX < rcWork.left + 15) offsetX = rcWork.left + 15;
            if (offsetY < rcWork.top + 15) offsetY = rcWork.top + 15;
            if (offsetX + physicalWidth > rcWork.right - 15) offsetX = rcWork.right - physicalWidth - 15;
            if (offsetY + physicalHeight > rcWork.bottom - 15) offsetY = rcWork.bottom - physicalHeight - 15;
        }
        else {
            int screenWidth = GetSystemMetrics(SM_CXSCREEN);
            int screenHeight = GetSystemMetrics(SM_CYSCREEN);
            if (offsetX < 15) offsetX = 15;
            if (offsetY < 15) offsetY = 15;
            if (offsetX + physicalWidth > screenWidth - 15) offsetX = screenWidth - physicalWidth - 15;
            if (offsetY + physicalHeight > screenHeight - 15) offsetY = screenHeight - physicalHeight - 15;
        }
        bool needsRecreate = !state.popupHwnd || !IsWindow(state.popupHwnd) || !state.source || !state.sourceHwnd || !IsWindow(state.sourceHwnd);
        bool createdXamlSourceThisOpen = false;
        if (!needsRecreate) {
            try {
                (void)state.source.Content();
            }
            catch (...) {
                needsRecreate = true;
            }
        }
        if (needsRecreate) {
            if (state.source) { try { state.source.Close(); } catch (...) {} state.source = nullptr; }
            g_win10PopupVisible.store(false, std::memory_order_release);
            g_win10SourceHwnd.store(nullptr, std::memory_order_release);
            g_win10PopupHwnd.store(nullptr, std::memory_order_release);
            if (state.popupHwnd && IsWindow(state.popupHwnd)) DestroyWindow(state.popupHwnd);
            state.popupHwnd = nullptr;
            state.sourceHwnd = nullptr;
            // Normal composited tool window; XAML HostBackdrop owns Win10 acrylic.
            DWORD dwExStyle = WS_EX_TOPMOST | WS_EX_TOOLWINDOW;
            HWND hOwner = NULL; // Do not set cross-thread owner for XAML Island stability
            state.popupHwnd = CreateWindowExW(
                dwExStyle,
                L"WhWin10ForecastPopupClass", L"",
                WS_POPUP,
                (int)offsetX, (int)offsetY, physicalWidth, physicalHeight,
                hOwner, NULL, GetModuleHandleW(NULL), NULL
            );
            if (state.popupHwnd) {
                g_win10PopupHwnd.store(state.popupHwnd, std::memory_order_release);
                g_win10PopupVisible.store(false, std::memory_order_release);
                // Apply transparency and acrylic-supporting attributes IMMEDIATELY while hidden
                MARGINS margins = { -1, -1, -1, -1 };
                DwmExtendFrameIntoClientArea(state.popupHwnd, &margins);
                if (g_useAcrylic) {
                    if (IsWindows11()) {
                        DWORD backdropType = 4; // DWMSBT_TABBEDWINDOW (Acrylic)
                        DwmSetWindowAttribute(state.popupHwnd, DWMWA_SYSTEMBACKDROP_TYPE, &backdropType, sizeof(backdropType));
                    }
                }
                BOOL useDarkMode = IsSystemDarkMode() ? TRUE : FALSE;
                DwmSetWindowAttribute(state.popupHwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &useDarkMode, sizeof(useDarkMode));
                if (IsWindows11()) {
                    DWORD cornerPreference = 2; // DWMWCP_ROUND (rounded corners)
                    DwmSetWindowAttribute(state.popupHwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &cornerPreference, sizeof(cornerPreference));
                }
                try {
                    state.source = winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource();
                    IDesktopWindowXamlSourceNative* nativeSource = nullptr;
                    HRESULT hr = ((::IUnknown*)winrt::get_unknown(state.source))->QueryInterface(
                        IID_IDesktopWindowXamlSourceNative,
                        (void**)&nativeSource
                    );
                    if (SUCCEEDED(hr)) {
                        hr = nativeSource->AttachToWindow(state.popupHwnd);
                        if (FAILED(hr)) {
                            nativeSource->Release();
                            throw winrt::hresult_error(hr);
                        }
                        nativeSource->get_WindowHandle(&state.sourceHwnd);
                        g_win10SourceHwnd.store(state.sourceHwnd, std::memory_order_release);
                        createdXamlSourceThisOpen = true;
                        nativeSource->Release();
                        SetWindowPos(state.popupHwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
                        InvalidateRect(state.popupHwnd, NULL, TRUE);
                        UpdateWindow(state.popupHwnd);
                    }
                    else {
                        throw winrt::hresult_error(hr);
                    }
                }
                catch (...) {
                    g_win10PopupVisible.store(false, std::memory_order_release);
                    g_win10SourceHwnd.store(nullptr, std::memory_order_release);
                    g_win10PopupHwnd.store(nullptr, std::memory_order_release);
                    if (state.popupHwnd) DestroyWindow(state.popupHwnd);
                    state.popupHwnd = nullptr;
                    state.source = nullptr;
                    state.sourceHwnd = nullptr;
                    throw;
                }
                SetWindowLongPtrW(state.popupHwnd, GWLP_USERDATA, (LONG_PTR)state.sourceHwnd);
                if (state.sourceHwnd) {
                    SetWindowPos(state.sourceHwnd, NULL, 0, 0, physicalWidth, physicalHeight, SWP_NOZORDER | SWP_NOACTIVATE);
                }
            }
        }
        else {
            MoveWindow(state.popupHwnd, (int)offsetX, (int)offsetY, physicalWidth, physicalHeight, TRUE);
            MARGINS margins = { -1, -1, -1, -1 };
            DwmExtendFrameIntoClientArea(state.popupHwnd, &margins);
            if (g_useAcrylic) {
                if (IsWindows11()) {
                    DWORD backdropType = 4; // DWMSBT_TABBEDWINDOW (Acrylic)
                    DwmSetWindowAttribute(state.popupHwnd, DWMWA_SYSTEMBACKDROP_TYPE, &backdropType, sizeof(backdropType));
                }
            }
            BOOL useDarkMode = IsSystemDarkMode() ? TRUE : FALSE;
            DwmSetWindowAttribute(state.popupHwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &useDarkMode, sizeof(useDarkMode));
        }
        if (state.popupHwnd && state.source) {
            if (g_debugLogs) Wh_Log(L"[Wh_WeatherHost] Reusing existing popup HWND and XamlSource");
            Grid rootGrid;
            if (g_debugLogs) Wh_Log(L"[Wh_WeatherHost] Populating UI");
            auto currentWeather = GetCurrentWeatherSnapshot();
            PopulateForecastUI(rootGrid, currentWeather.condition, currentWeather.icon, currentWeather.temp);
            rootGrid.Width(panelWidth);
            if (g_debugLogs) Wh_Log(L"[Wh_WeatherHost] Setting content");
            try {
                auto oldContent = state.source.Content();
                if (oldContent) {
                    CleanupXamlMedia(oldContent);
                }
            }
            catch (...) {}
            state.source.Content(rootGrid);
            // On the first Windows 10 showing, keep the XAML content transparent
            // until the host window is active. HostBackdrop can otherwise connect
            // while the new top-level HWND is inactive and remain on its fallback
            // color until the popup is opened a second time.
            if (createdXamlSourceThisOpen && !IsWindows11() &&
                g_useAcrylic) {
                rootGrid.Opacity(0.0);
            }
            if (state.sourceHwnd) {
                ShowWindow(state.sourceHwnd, SW_SHOW);
            }
            if (g_debugLogs) Wh_Log(L"[Wh_WeatherHost] Showing window");
            SetWindowPos(state.popupHwnd, NULL, (int)offsetX, (int)offsetY, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
            ShowWindow(state.popupHwnd, SW_SHOW);
            g_win10PopupVisible.store(true, std::memory_order_release);
            MARGINS showMargins = { -1, -1, -1, -1 };
            DwmExtendFrameIntoClientArea(state.popupHwnd, &showMargins);
            if (!IsWindows11() && g_useAcrylic) {
                ApplyWin10Acrylic(state.popupHwnd);
            }
            // Force activation/focus so dismissal on click-away (WA_INACTIVE) triggers reliably
            BOOL foregroundSet = SetForegroundWindow(state.popupHwnd);
            SetActiveWindow(state.popupHwnd);
            SetFocus(state.popupHwnd);
            if (createdXamlSourceThisOpen && !IsWindows11() && g_useAcrylic) {
                rootGrid.Opacity(1.0);
                // Reconnect once after activation messages have been processed.
                // This HWND-owned timer carries no XAML pointer or callback and
                // is cancelled on hide and destruction.
                UINT_PTR timer = SetTimer(state.popupHwnd, kWin10FirstAcrylicTimer, 50, nullptr);
                if (g_debugLogs) Wh_Log(L"[WeatherPopup] Queued first acrylic reconnect: timer=%d foreground=%d",
                    timer != 0 ? 1 : 0, foregroundSet ? 1 : 0);
            }
            RedrawWindow(state.popupHwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
            if (g_debugLogs) {
                Wh_Log(L"[Wh_WeatherHost] Opened Win10 forecast popup at offset (%f, %f) using XAML Island HWND=%p", offsetX, offsetY, state.popupHwnd);
            }
        }
    }
    catch (const winrt::hresult_error& e) {
        if (g_debugLogs) {
            Wh_Log(L"[Wh_WeatherHost] Error creating/showing Win10 forecast popup. WinRT Exception: %s (0x%08X).", e.message().c_str(), e.code().value);
        }
        if (state.source) { try { state.source.Close(); } catch (...) {} state.source = nullptr; }
        g_win10PopupVisible.store(false, std::memory_order_release);
        g_win10SourceHwnd.store(nullptr, std::memory_order_release);
        g_win10PopupHwnd.store(nullptr, std::memory_order_release);
        if (state.popupHwnd && IsWindow(state.popupHwnd)) DestroyWindow(state.popupHwnd);
        state.popupHwnd = nullptr;
        state.sourceHwnd = nullptr;
    }
    catch (const std::exception& e) {
        if (g_debugLogs) {
            Wh_Log(L"[Wh_WeatherHost] Error creating/showing Win10 forecast popup. Std Exception: %S.", e.what());
        }
        if (state.source) { try { state.source.Close(); } catch (...) {} state.source = nullptr; }
        g_win10PopupVisible.store(false, std::memory_order_release);
        g_win10SourceHwnd.store(nullptr, std::memory_order_release);
        g_win10PopupHwnd.store(nullptr, std::memory_order_release);
        if (state.popupHwnd && IsWindow(state.popupHwnd)) DestroyWindow(state.popupHwnd);
        state.popupHwnd = nullptr;
        state.sourceHwnd = nullptr;
    }
    catch (...) {
        if (g_debugLogs) {
            Wh_Log(L"[Wh_WeatherHost] Error creating/showing Win10 forecast popup. Unknown Exception.");
        }
        if (state.source) { try { state.source.Close(); } catch (...) {} state.source = nullptr; }
        g_win10PopupVisible.store(false, std::memory_order_release);
        g_win10SourceHwnd.store(nullptr, std::memory_order_release);
        g_win10PopupHwnd.store(nullptr, std::memory_order_release);
        if (state.popupHwnd && IsWindow(state.popupHwnd)) DestroyWindow(state.popupHwnd);
        state.popupHwnd = nullptr;
        state.sourceHwnd = nullptr;
    }
}
void InvalidateClockParentRegion(HWND hWnd) {
    if (!hWnd || !IsWindow(hWnd)) return;

    // Invalidate without forcing synchronous RDW_UPDATENOW paints through the
    // entire taskbar ancestor chain. Explorer will repaint on its normal cycle,
    // avoiding the hover/click redraw storm that caused visible flicker.
    RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_FRAME);

    HWND hParent = GetParent(hWnd);
    if (hParent) {
        RECT rc;
        GetWindowRect(hWnd, &rc);
        MapWindowPoints(NULL, hParent, (LPPOINT)&rc, 2);
        RedrawWindow(hParent, &rc, NULL, RDW_INVALIDATE | RDW_ERASE);
    }
}
void RestoreDefaultWindowSize(HWND hwndToClean) {
    if (g_currentAddedWeatherWidth > 0) {
        RECT rc = { 0 };
        GetWindowRect(hwndToClean, &rc);
        HWND hParent = GetParent(hwndToClean);
        if (hParent) {
            MapWindowPoints(NULL, hParent, (LPPOINT)&rc, 2);
        }
        int x = rc.left;
        int y = rc.top;
        int cx = rc.right - rc.left;
        int cy = rc.bottom - rc.top;
        bool isHorizontal = cx >= cy;
        if (isHorizontal) {
            x += g_currentAddedWeatherWidth;
            cx -= g_currentAddedWeatherWidth;
        }
        else {
            y += g_currentAddedWeatherWidth;
            cy -= g_currentAddedWeatherWidth;
        }
        SetWindowPos(hwndToClean, NULL, x, y, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
        g_currentAddedWeatherWidth = 0;
    }
    g_cleanCx = 0;
    g_cleanCy = 0;
    g_cleanX = 0;
    g_cleanY = 0;
}
HWND FindTaskListWndForTaskbar(HWND hTaskbar) {
    if (!hTaskbar) return NULL;
    HWND hReBar = FindWindowExW(hTaskbar, nullptr, L"ReBarWindow32", nullptr);
    if (hReBar) {
        return FindWindowExW(hReBar, nullptr, L"MSTaskSwWClass", nullptr);
    }
    return FindWindowExW(hTaskbar, nullptr, L"MSTaskSwWClass", nullptr);
}
LRESULT CALLBACK TaskListSubclassWndProc(HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    DWORD_PTR dwRefData) {
    
    if (g_modUnloaded) {
        return DefSubclassProc(hWnd, uMsg, wParam, lParam);
    }
    if (uMsg == WM_WINDOWPOSCHANGING) {
        WINDOWPOS* lpwp = (WINDOWPOS*)lParam;
        if (lpwp && !(lpwp->flags & SWP_NOSIZE)) {
            HWND hParentTaskbar = GetAncestor(hWnd, GA_ROOT);
            RECT trayRect{}, weatherRect{}, listRect{};
            HWND taskListParent = GetParent(hWnd);
            if (hParentTaskbar && taskListParent && g_hSubclassedWnd &&
                IsWindow(g_hSubclassedWnd) &&
                GetAncestor(g_hSubclassedWnd, GA_ROOT) == hParentTaskbar &&
                GetWindowRect(hParentTaskbar, &trayRect) &&
                GetWindowRect(g_hSubclassedWnd, &weatherRect) &&
                GetWindowRect(hWnd, &listRect)) {
                // Clamp to the weather window's actual edge. Subtracting the
                // reserved width from every proposed size accumulates when
                // Explorer reuses the already shortened task-list rectangle.
                MapWindowPoints(nullptr, taskListParent,
                                reinterpret_cast<POINT*>(&weatherRect), 2);
                MapWindowPoints(nullptr, taskListParent,
                                reinterpret_cast<POINT*>(&listRect), 2);
                // Keep a small visual gap; the configurable repeater offset
                // belongs to the XAML injection path, not this Win32 task list.
                constexpr int gap = 8;
                const bool horizontal = (trayRect.right - trayRect.left) >
                                        (trayRect.bottom - trayRect.top);
                const int available = horizontal
                    ? weatherRect.left - ((lpwp->flags & SWP_NOMOVE)
                        ? listRect.left : lpwp->x) - gap
                    : weatherRect.top - ((lpwp->flags & SWP_NOMOVE)
                        ? listRect.top : lpwp->y) - gap;
                if (available >= 0) {
                    if (horizontal) lpwp->cx = (std::min)(lpwp->cx, available);
                    else lpwp->cy = (std::min)(lpwp->cy, available);
                }
            }
        }
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}
void PerformXamlCleanupOnUIThread() {
    if (!ShouldUseXamlTaskbar()) return; // Dedicated popup owners are cleaned by PopupThreadProc.
    RestoreRepeaterMarginsOnCurrentUIThread();
    CancelAnimationEventsOnCurrentThread();
    RevokeAllImageHooks();
    try {
        if (g_activeFlyout) {
            try { g_activeFlyout.Hide(); } catch (...) {}
            try { g_activeFlyout.Content(nullptr); } catch (...) {}
            g_activeFlyout = nullptr;
        }
    } catch (...) {}
    g_win11FlyoutIsOpen = false;
    g_showWin11Flyout = nullptr;

    // On Windows 11 these XAML objects belong to the taskbar UI thread.
    // On Windows 10 they belong to the dedicated popup STA and are released there.
    if (IsWindows11()) {
        StopRadarTimer();
        try { g_radarTileBitmapCache.clear(); } catch (...) {}
        try { g_iconCache.clear(); } catch (...) {}
    }

    {
        std::lock_guard<std::mutex> lock(g_hooksMutex);
        for (auto& hook : g_sizeChangedHooks) {
            try {
                if (auto el = hook.weakElement.get()) {
                    el.SizeChanged(hook.token);
                }
            } catch (...) {}
        }
        g_sizeChangedHooks.clear();
    }
    {
        std::lock_guard<std::mutex> lock(g_hookedMutex);
        g_hookedElements.clear();
    }
    {
        std::lock_guard<std::mutex> lock(g_pendingMutex);
        g_scannedFrames.clear();
    }

    std::vector<Grid> targetGrids;
    {
        std::lock_guard<std::mutex> lock(g_weatherGridMutex);
        for (auto& weakG : g_injectedWeatherGrids) {
            if (auto g = weakG.get()) targetGrids.push_back(g);
        }
        g_injectedWeatherGrids.clear();
    }
    for (auto targetGrid : targetGrids) {
        try {
            if (auto parent = targetGrid.Parent().try_as<winrt::Windows::UI::Xaml::Controls::Panel>()) {
                RemoveInjectedFromPanel(parent);
            }
            else if (auto grid = targetGrid.Parent().try_as<Grid>()) {
                RemoveInjectedFromGrid(grid);
            }
        } catch (...) {}
    }

    g_weakXamlWeatherButton = nullptr;
}
LRESULT CALLBACK ClockSubclassWndProc(HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    DWORD_PTR dwRefData) {
    if (uMsg == kForegroundChangedMessage) {
        HandleForegroundChange((HWND)wParam);
        return 0;
    }
        if (g_msgHotkeyControl && uMsg == g_msgHotkeyControl) {
        if (g_hKeyboardHook) { UnhookWindowsHookEx(g_hKeyboardHook); g_hKeyboardHook = NULL; }
        g_hotkeyHookState = {};
        if (g_hMouseHook) { UnhookWindowsHookEx(g_hMouseHook); g_hMouseHook = NULL; }
        if (wParam == 1 && g_hkVk != 0) {
            HMODULE hMod = NULL;
            GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR)LowLevelKeyboardProc, &hMod);
            g_hKeyboardHook = SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardProc, hMod, 0);
            if (g_debugLogs) Wh_Log(L"[Wh_WeatherHost] Registered global hotkey via low-level keyboard hook");
        }
        return 0;
    }
    if (uMsg == (WM_USER + 4242)) {
        try {
            if (ShouldUseXamlTaskbar()) {
                if (auto button = g_weakXamlWeatherButton.get()) {
                    if (g_activeFlyout && g_win11FlyoutIsOpen) {
                        try { g_activeFlyout.Hide(); } catch(...) {}
                        return 0;
                    }
                    if (!g_win11FlyoutIsOpen) {
                        HWND hActive = GetForegroundWindow();
                        HWND hTaskbar = FindWindowW(L"Shell_TrayWnd", NULL);
                        if (hActive && hActive != hTaskbar && GetAncestor(hActive, GA_ROOT) != hTaskbar) {
                            g_hPreFlyoutActiveWnd = hActive;
                        }
                        if (g_hSubclassedWnd) {
                            HWND hRoot = GetAncestor(g_hSubclassedWnd, GA_ROOT);
                            if (!hRoot) hRoot = g_hSubclassedWnd;
                            if (hRoot) {
                                // Simulate Alt key press to bypass SetForegroundWindow restrictions
                                ::keybd_event(VK_MENU, 0, 0, 0);
                                ::keybd_event(VK_MENU, 0, KEYEVENTF_KEYUP, 0);
                                HWND hActiveWnd = GetForegroundWindow();
                                if (hActiveWnd && hActiveWnd != hRoot) {
                                    DWORD activeThreadId = GetWindowThreadProcessId(hActiveWnd, NULL);
                                    DWORD currentThreadId = GetCurrentThreadId();
                                    if (activeThreadId != currentThreadId) {
                                        AttachThreadInput(activeThreadId, currentThreadId, TRUE);
                                        SetForegroundWindow(hRoot);
                                        SetActiveWindow(hRoot);
                                        SetFocus(hRoot);
                                        AttachThreadInput(activeThreadId, currentThreadId, FALSE);
                                    }
                                    else {
                                        SetForegroundWindow(hRoot);
                                        SetActiveWindow(hRoot);
                                        SetFocus(hRoot);
                                    }
                                }
                                else {
                                    SetForegroundWindow(hRoot);
                                    SetActiveWindow(hRoot);
                                    SetFocus(hRoot);
                                }
                            }
                        }
                    }
                    if (g_showWin11Flyout) {
                        g_showWin11Flyout();
                    }
                }
                return 0;
            }
            if (wParam == 1) {
                HWND popup = g_win10PopupHwnd.load(std::memory_order_acquire);
                if (popup && IsWindow(popup)) {
                    g_lastHideTickCount.store(GetTickCount(), std::memory_order_release);
                    HideWin10Popup(popup);
                }
            }
            else {
                HWND popup = g_win10PopupHwnd.load(std::memory_order_acquire);
                if (popup && IsWindow(popup) && g_win10PopupVisible.load(std::memory_order_acquire)) {
                    g_lastHideTickCount.store(GetTickCount(), std::memory_order_release);
                    HideWin10Popup(popup);
                }
                else if (GetTickCount() - g_lastHideTickCount.load(std::memory_order_acquire) > 250) {
                    ShowWin10ForecastPopup(hWnd);
                }
            }
        }
        catch (...) {}
        return 0;
    }
    if (uMsg == (WM_USER + 4244)) {
        HWND popup = g_win10PopupHwnd.load(std::memory_order_acquire);
        if (popup && IsWindow(popup)) {
            PostMessageW(popup, WM_USER + 5003, 0, 0);
        }
        return 0;
    }
    if (uMsg == (WM_USER + 4247)) {
        if (g_selectedGraphTab != 3 || g_modUnloaded) return 0;
        try {
            if (ShouldUseXamlTaskbar()) {
                if (g_win11FlyoutIsOpen && g_activeFlyout) {
                    auto outer = g_activeFlyout.Content().try_as<Grid>();
                    if (outer && outer.Children().Size()) {
                        if (auto root = outer.Children().GetAt(0).try_as<Grid>()) {
                            auto weather = GetCurrentWeatherSnapshot();
                            PopulateForecastUI(root, weather.condition,
                                weather.icon, weather.temp, false);
                        }
                    }
                }
            } else if (g_win10PopupVisible.load(std::memory_order_acquire)) {
                HWND popup = g_win10PopupHwnd.load(std::memory_order_acquire);
                if (popup && IsWindow(popup))
                    PostMessageW(popup, WM_USER + 5003, 0, 0);
            }
        } catch (...) {}
        return 0;
    }
    if (uMsg == (WM_USER + 4245)) {
        PerformXamlCleanupOnUIThread();
        return 0;
    }
    if (!ShouldUseXamlTaskbar()) {
        if (uMsg == WM_WINDOWPOSCHANGING) {
            WINDOWPOS* lpwp = (WINDOWPOS*)lParam;
            if (!lpwp) return DefSubclassProc(hWnd, uMsg, wParam, lParam);
            int weatherWidth = GetRequiredWeatherWidth(hWnd);
            if (g_modUnloaded) weatherWidth = 0;
            RECT rc;
            GetWindowRect(hWnd, &rc);
            HWND hParent = GetParent(hWnd);
            if (hParent) {
                MapWindowPoints(NULL, hParent, (LPPOINT)&rc, 2);
            }
            int curX = rc.left;
            int curY = rc.top;
            int curCx = rc.right - rc.left;
            int curCy = rc.bottom - rc.top;
            int orientationCx = (lpwp->flags & SWP_NOSIZE) ? curCx : lpwp->cx;
            int orientationCy = (lpwp->flags & SWP_NOSIZE) ? curCy : lpwp->cy;
            bool isHorizontal = orientationCx > orientationCy;
            // Explorer may propose the window's current, already expanded
            // rectangle during a taskbar relayout. Undo our previous addition
            // before applying the requested weather width again.
            const bool proposalIsExpanded = g_currentAddedWeatherWidth > 0 &&
                (curCx > curCy) == isHorizontal &&
                !(lpwp->flags & SWP_NOSIZE) &&
                (isHorizontal ? lpwp->cx == curCx : lpwp->cy == curCy);
            const bool proposalUsesCurrentPosition =
                g_currentAddedWeatherWidth > 0 &&
                (curCx > curCy) == isHorizontal &&
                !(lpwp->flags & SWP_NOMOVE) &&
                (isHorizontal ? lpwp->x == curX : lpwp->y == curY);
            if (g_debugLogs) {
                Wh_Log(L"[Wh_WeatherHost] WM_WINDOWPOSCHANGING entered: lpwp->flags=0x%08X, proposed cx=%d, cy=%d, x=%d, y=%d, curAddedWidth=%d, targetAddedWidth=%d, isHorizontal=%d",
                    lpwp->flags, lpwp->cx, lpwp->cy, lpwp->x, lpwp->y, g_currentAddedWeatherWidth, weatherWidth, isHorizontal);
            }
            int cleanCx = 0;
            int cleanCy = 0;
            if (!(lpwp->flags & SWP_NOSIZE)) {
                cleanCx = lpwp->cx - (proposalIsExpanded && isHorizontal
                    ? g_currentAddedWeatherWidth : 0);
                cleanCy = lpwp->cy - (proposalIsExpanded && !isHorizontal
                    ? g_currentAddedWeatherWidth : 0);
            }
            else {
                if (isHorizontal) {
                    cleanCx = curCx - g_currentAddedWeatherWidth;
                    cleanCy = curCy;
                }
                else {
                    cleanCx = curCx;
                    cleanCy = curCy - g_currentAddedWeatherWidth;
                }
            }
            int cleanX = 0;
            int cleanY = 0;
            if (!(lpwp->flags & SWP_NOMOVE)) {
                cleanX = lpwp->x + ((proposalIsExpanded || proposalUsesCurrentPosition) && isHorizontal
                    ? g_currentAddedWeatherWidth : 0);
                cleanY = lpwp->y + ((proposalIsExpanded || proposalUsesCurrentPosition) && !isHorizontal
                    ? g_currentAddedWeatherWidth : 0);
            }
            else {
                if (isHorizontal) {
                    cleanX = curX + g_currentAddedWeatherWidth;
                    cleanY = curY;
                }
                else {
                    cleanX = curX;
                    cleanY = curY + g_currentAddedWeatherWidth;
                }
            }
            bool sizeChanged = (weatherWidth != g_currentAddedWeatherWidth);
            if (sizeChanged || !(lpwp->flags & SWP_NOSIZE) || !(lpwp->flags & SWP_NOMOVE)) {
                if (isHorizontal) {
                    lpwp->cx = cleanCx + weatherWidth;
                    lpwp->cy = cleanCy;
                    lpwp->x = cleanX - weatherWidth;
                    lpwp->y = cleanY;
                }
                else {
                    lpwp->cx = cleanCx;
                    lpwp->cy = cleanCy + weatherWidth;
                    lpwp->x = cleanX;
                    lpwp->y = cleanY - weatherWidth;
                }
                lpwp->flags &= ~(SWP_NOSIZE | SWP_NOMOVE);
                // Fix for Issue 1: Windows ignores manual x/y coordinate modifications in 
                // WM_WINDOWPOSCHANGING if the SWP_NOMOVE flag was sent by the caller.
            }
            g_currentAddedWeatherWidth = weatherWidth;
            if (sizeChanged && g_hSubclassedTaskListWnd && IsWindow(g_hSubclassedTaskListWnd)) {
                SetWindowPos(g_hSubclassedTaskListWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
            }
            lpwp->flags |= SWP_NOCOPYBITS;
            if (g_debugLogs) {
                Wh_Log(L"[Wh_WeatherHost] WM_WINDOWPOSCHANGING adjusted: flags=0x%08X, adjusted cx=%d, cy=%d, x=%d, y=%d",
                    lpwp->flags, lpwp->cx, lpwp->cy, lpwp->x, lpwp->y);
            }
            return DefSubclassProc(hWnd, uMsg, wParam, lParam);
        }
        else if (uMsg == WM_TIMER && wParam == 1234) {
            KillTimer(hWnd, 1234);
            HDC hdc = GetDCEx(hWnd, NULL, DCX_WINDOW | DCX_CACHE);
            if (hdc) {
                PaintWin10Weather(hWnd, hdc);
                ReleaseDC(hWnd, hdc);
            }
            return 0;
        }
        else if (uMsg == WM_WINDOWPOSCHANGED) {
            LRESULT res = DefSubclassProc(hWnd, uMsg, wParam, lParam);
            InvalidateClockParentRegion(hWnd);
            RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
            HDC hdc = GetDCEx(hWnd, NULL, DCX_WINDOW | DCX_CACHE);
            if (hdc) {
                PaintWin10Weather(hWnd, hdc);
                ReleaseDC(hWnd, hdc);
            }
            if (g_win10PopupVisible.load(std::memory_order_acquire)) {
                try {
                    ShowWin10ForecastPopup(hWnd);
                }
                catch (...) {}
            }
            return res;
        }
        else if (uMsg == WM_NCCALCSIZE) {
            LRESULT res = DefSubclassProc(hWnd, uMsg, wParam, lParam);
            int weatherWidth = GetRequiredWeatherWidth(hWnd);
            if (g_modUnloaded) weatherWidth = 0;
            RECT* clientRect = nullptr;
            if (wParam == TRUE) {
                NCCALCSIZE_PARAMS* pnc = (NCCALCSIZE_PARAMS*)lParam;
                clientRect = &pnc->rgrc[0];
            }
            else {
                clientRect = (RECT*)lParam;
            }
            if (clientRect) {
                bool isHorizontal = (clientRect->right - clientRect->left) > (clientRect->bottom - clientRect->top);
                if (isHorizontal) {
                    clientRect->left += weatherWidth;
                }
                else {
                    clientRect->top += weatherWidth;
                }
            }
            return res;
        }
        else if (uMsg == WM_SIZE || uMsg == WM_MOVE || uMsg == WM_NOTIFY || uMsg == WM_COMMAND) {
            LRESULT res = DefSubclassProc(hWnd, uMsg, wParam, lParam);
            InvalidateClockParentRegion(hWnd);
            return res;
        }
        else if (uMsg == WM_NCPAINT || uMsg == WM_PAINT || uMsg == 0x0318 /*WM_PRINTCLIENT*/ || uMsg == WM_PRINT) {
            LRESULT res = DefSubclassProc(hWnd, uMsg, wParam, lParam);
            HDC hdc = GetDCEx(hWnd, NULL, DCX_WINDOW | DCX_CACHE);
            if (hdc) {
                PaintWin10Weather(hWnd, hdc);
                ReleaseDC(hWnd, hdc);
            }
            return res;
        }
        else if (uMsg == WM_ERASEBKGND) {
            return TRUE;
        }
        else if (uMsg == WM_NCHITTEST) {
            POINT pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            ScreenToClient(hWnd, &pt);
            RECT rc;
            GetClientRect(hWnd, &rc);
            int height = rc.bottom - rc.top;
            int width = rc.right - rc.left;
            bool isHorizontal = width >= height;
            int weatherWidth = g_currentAddedWeatherWidth;
            if (g_modUnloaded) weatherWidth = 0;
            if (isHorizontal) {
                if (pt.x >= -weatherWidth && pt.x < 0) {
                    return HTCLIENT;
                }
            }
            else {
                if (pt.y >= -weatherWidth && pt.y < 0) {
                    return HTCLIENT;
                }
            }
            return DefSubclassProc(hWnd, uMsg, wParam, lParam);
        }
        else if (uMsg == WM_MOUSEMOVE) {
            RECT rc;
            GetClientRect(hWnd, &rc);
            int height = rc.bottom - rc.top;
            int width = rc.right - rc.left;
            bool isHorizontal = width >= height;
            int pos = isHorizontal ? GET_X_LPARAM(lParam) : GET_Y_LPARAM(lParam);
            int weatherWidth = g_currentAddedWeatherWidth;
            bool overBtn = (pos >= -weatherWidth && pos < 0);
            if (!g_win10TrackingMouse && overBtn) {
                TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, hWnd, 0 };
                TrackMouseEvent(&tme);
                g_win10TrackingMouse = true;
            }
            if (overBtn != g_win10WeatherHovered) {
                g_win10WeatherHovered = overBtn;
                InvalidateClockParentRegion(hWnd);
            }
            return DefSubclassProc(hWnd, uMsg, wParam, lParam);
        }
        else if (uMsg == WM_MOUSELEAVE) {
            g_win10WeatherHovered = false;
            g_win10WeatherPressed = false;
            g_win10TrackingMouse = false;
            InvalidateClockParentRegion(hWnd);
            return DefSubclassProc(hWnd, uMsg, wParam, lParam);
        }
        else if (uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONDBLCLK) {
            RECT rc;
            GetClientRect(hWnd, &rc);
            bool isHorizontal = (rc.right - rc.left) >= (rc.bottom - rc.top);
            int pos = isHorizontal ? GET_X_LPARAM(lParam) : GET_Y_LPARAM(lParam);
            int weatherWidth = g_currentAddedWeatherWidth;
            if (pos >= -weatherWidth && pos < 0) {
                HWND popup = g_win10PopupHwnd.load(std::memory_order_acquire);
                g_popupWasVisibleOnLButtonDown =
                    popup && IsWindow(popup) && g_win10PopupVisible.load(std::memory_order_acquire);
                g_win10WeatherPressed = true;
                InvalidateClockParentRegion(hWnd);
                SetCapture(hWnd);
                return 0;
            }
            return DefSubclassProc(hWnd, uMsg, wParam, lParam);
        }
        else if (uMsg == WM_LBUTTONUP) {
            if (GetCapture() == hWnd) ReleaseCapture();
            bool wasPressed = g_win10WeatherPressed;
            g_win10WeatherPressed = false;
            RECT rc;
            GetClientRect(hWnd, &rc);
            bool isHorizontal = (rc.right - rc.left) >= (rc.bottom - rc.top);
            int pos = isHorizontal ? GET_X_LPARAM(lParam) : GET_Y_LPARAM(lParam);
            int weatherWidth = g_currentAddedWeatherWidth;
            if (wasPressed && pos >= -weatherWidth && pos < 0) {
                InvalidateClockParentRegion(hWnd);
                PostMessageW(hWnd, WM_USER + 4242, g_popupWasVisibleOnLButtonDown ? 1 : 0, 0);
                return 0;
            }
            return DefSubclassProc(hWnd, uMsg, wParam, lParam);
        }
        else if (uMsg == (WM_USER + 4243)) {
            HideWin10Popup(hWnd);
            return 0;
        }
    }
    else {
        if (uMsg == WM_WINDOWPOSCHANGED || uMsg == WM_SIZE ||
            uMsg == WM_SHOWWINDOW) {
            std::vector<Grid> targetGrids;
            {
                std::lock_guard<std::mutex> lock(g_weatherGridMutex);
                auto it = g_injectedWeatherGrids.begin();
                while (it != g_injectedWeatherGrids.end()) {
                    if (auto grid = it->get()) {
                        targetGrids.push_back(grid);
                        ++it;
                    } else {
                        it = g_injectedWeatherGrids.erase(it);
                    }
                }
            }

            for (auto const& targetGrid : targetGrids) {
                try {
                    auto dispatcher = targetGrid.Dispatcher();
                    if (!dispatcher || g_modUnloaded) continue;

                    auto weakGrid = winrt::make_weak(targetGrid);
                    g_pendingRunAsync++;
                    try {
                        dispatcher.RunAsync(
                            winrt::Windows::UI::Core::CoreDispatcherPriority::Normal,
                            [weakGrid]() {
                                struct AutoDecrement {
                                    ~AutoDecrement() { g_pendingRunAsync--; }
                                } autoDec;

                                if (g_modUnloaded) return;
                                try {
                                    if (auto grid = weakGrid.get()) {
                                        RefreshInjectedWeatherAfterResize(grid);
                                    }
                                } catch (...) {}
                            });
                    } catch (...) {
                        g_pendingRunAsync--;
                    }
                } catch (...) {}
            }
        }
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}
void AlignOverlayWindow() {
    EnterCriticalSection(&g_subclassLock);
    HWND hTarget = ShouldUseXamlTaskbar() ? FindSystemAnchorWnd() : FindTrayNotifyWnd();
    if (hTarget && (g_hSubclassedWnd != hTarget || !IsWindow(g_hSubclassedWnd))) {
        if (g_hSubclassedWnd && IsWindow(g_hSubclassedWnd)) {
            PostMessageW(g_hSubclassedWnd, g_msgHotkeyControl, 0, 0); // Unregister hotkey
            PostMessageW(g_hSubclassedWnd, WM_USER + 4243, 0, 0); // Cleanup XAML
            KillTimer(g_hSubclassedWnd, 1234);
            WindhawkUtils::RemoveWindowSubclassFromAnyThread(g_hSubclassedWnd, ClockSubclassWndProc);
            RestoreDefaultWindowSize(g_hSubclassedWnd);
        }
        if (g_hSubclassedTaskListWnd && IsWindow(g_hSubclassedTaskListWnd)) {
            WindhawkUtils::RemoveWindowSubclassFromAnyThread(g_hSubclassedTaskListWnd, TaskListSubclassWndProc);
            SetWindowPos(g_hSubclassedTaskListWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
            g_hSubclassedTaskListWnd = NULL;
        }
        g_hSubclassedWnd = hTarget;
        g_currentAddedWeatherWidth = 0;
        g_cleanCx = 0;
        g_cleanCy = 0;
        g_cleanX = 0;
        g_cleanY = 0;
        if (WindhawkUtils::SetWindowSubclassFromAnyThread(hTarget, ClockSubclassWndProc, NULL)) {
            g_foregroundMessageTarget.store(hTarget, std::memory_order_release);
            if (g_debugLogs) {
                Wh_Log(
                    L"[Wh_WeatherHost] Successfully subclassed Target HWND=%p "
                    L"(XamlTaskbar=%d)",
                    hTarget, ShouldUseXamlTaskbar());
            }
            if (!ShouldUseXamlTaskbar()) {
                HWND hParentTaskbar = GetAncestor(hTarget, GA_ROOT);
                HWND hTaskList = FindTaskListWndForTaskbar(hParentTaskbar);
                if (hTaskList) {
                    g_hSubclassedTaskListWnd = hTaskList;
                    WindhawkUtils::SetWindowSubclassFromAnyThread(hTaskList, TaskListSubclassWndProc, NULL);
                    SetWindowPos(hTaskList, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
                }
            }
            PostMessageW(hTarget, g_msgHotkeyControl, 1, 0); // Register hotkey on UI thread
            // Trigger a layout refresh on the tray area and taskbar
            InvalidateClockParentRegion(hTarget);
            SetWindowPos(hTarget, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
            HWND hTaskbar = FindWindowW(L"Shell_TrayWnd", NULL);
            if (hTaskbar) {
                PostMessageW(hTaskbar, WM_SIZE, 0, 0);
            }
            RedrawWindow(hTarget, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
        }
    }
    LeaveCriticalSection(&g_subclassLock);
}
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && g_hkVk != 0) {
        auto pKbd = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        // GetAsyncKeyState can lag the event currently being dispatched, and
        // may be unavailable for another process' foreground thread. Track
        // modifier events in this hook as well as consulting the OS state.
        auto& state = g_hotkeyHookState;
        const bool down = wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN;
        const bool up = wParam == WM_KEYUP || wParam == WM_SYSKEYUP;
        if (!(pKbd->flags & LLKHF_INJECTED) && (down || up)) {
            bool* modifier = nullptr;
            switch (pKbd->vkCode) {
            case VK_LWIN: case VK_RWIN: modifier = &state.winDown; break;
            case VK_MENU: case VK_LMENU: case VK_RMENU: modifier = &state.altDown; break;
            case VK_CONTROL: case VK_LCONTROL: case VK_RCONTROL: modifier = &state.ctrlDown; break;
            case VK_SHIFT: case VK_LSHIFT: case VK_RSHIFT: modifier = &state.shiftDown; break;
            }
            if (modifier) *modifier = down;
        }
        if (pKbd->vkCode == g_hkVk) {
            // Suppress the matching key's release even if Alt/Win was already
            // released. Letting that release through can invoke Win+W Widgets.
            if (up && state.hotkeyHeld) { state.hotkeyHeld = false; return 1; }
            if (down && state.hotkeyHeld) return 1;
            if (!down || (pKbd->flags & LLKHF_INJECTED))
                return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
            bool win = state.winDown || (GetAsyncKeyState(VK_LWIN) & 0x8000) || (GetAsyncKeyState(VK_RWIN) & 0x8000);
            bool alt = state.altDown || (GetAsyncKeyState(VK_LMENU) & 0x8000) || (GetAsyncKeyState(VK_RMENU) & 0x8000);
            bool ctrl = state.ctrlDown || (GetAsyncKeyState(VK_LCONTROL) & 0x8000) || (GetAsyncKeyState(VK_RCONTROL) & 0x8000);
            bool shift = state.shiftDown || (GetAsyncKeyState(VK_LSHIFT) & 0x8000) || (GetAsyncKeyState(VK_RSHIFT) & 0x8000);
            if (win == g_hkWin && alt == g_hkAlt && ctrl == g_hkCtrl && shift == g_hkShift) {
                if (!state.hotkeyHeld) {
                    state.hotkeyHeld = true;
                    if (g_hkWin) {
                        // Suppress Windows Start menu / native shortcut activation when releasing Win key
                        ::keybd_event(VK_CONTROL, 0, 0, 0);
                        ::keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
                    }
                    if (g_hSubclassedWnd) {
                        PostMessageW(g_hSubclassedWnd, WM_USER + 4242, 0, 0);
                    }
                }
                return 1;
            }
        }
    }
    return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
}
LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (g_modUnloaded) {
        return CallNextHookEx(g_hMouseHook, nCode, wParam, lParam);
    }
    if (nCode == HC_ACTION) {
        if (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN || wParam == WM_MBUTTONDOWN || wParam == WM_NCLBUTTONDOWN || wParam == WM_NCRBUTTONDOWN || wParam == WM_NCMBUTTONDOWN) {
            if (g_win11FlyoutIsOpen && !g_injectToSysTray) {
                HWND hTaskbar = FindWindowW(L"Shell_TrayWnd", NULL);
                if (hTaskbar) {
                    MSLLHOOKSTRUCT* pMouse = (MSLLHOOKSTRUCT*)lParam;
                    HWND hTarget = WindowFromPoint(pMouse->pt);
                    bool isInsideFlyout = false;
                    HWND hCurrent = hTarget;
                    while (hCurrent) {
                        wchar_t className[256];
                        if (GetClassNameW(hCurrent, className, 256)) {
                            if (wcscmp(className, L"Windows.UI.Core.CoreWindow") == 0 ||
                                wcscmp(className, L"DesktopWindowXamlSource") == 0 ||
                                wcscmp(className, L"PopupHost") == 0 ||
                                wcscmp(className, L"Xaml_WindowedPopupClass") == 0) {
                                isInsideFlyout = true;
                                break;
                            }
                        }
                        hCurrent = GetParent(hCurrent);
                    }
                    if (!isInsideFlyout) {
                        if (g_hSubclassedWnd) {
                            PostMessageW(g_hSubclassedWnd, WM_USER + 4242, 0, 0);
                        } else {
                            PostMessageW(hTaskbar, WM_USER + 4242, 0, 0);
                        }
                        return CallNextHookEx(g_hMouseHook, nCode, wParam, lParam);
                    }
                }
            }
        }
    }
    return CallNextHookEx(g_hMouseHook, nCode, wParam, lParam);
}
void HandleForegroundChange(HWND hwnd) {
    if (g_modUnloaded) return;
    // Ignore an event overtaken by a newer foreground change while queued.
    if (hwnd != GetForegroundWindow()) return;
    {
        if (!hwnd || !IsWindow(hwnd)) return;
        HWND popup = g_win10PopupHwnd.load(std::memory_order_acquire);
        if (popup && hwnd == popup) {
            return;
        }
        HWND sourceHwnd = g_win10SourceHwnd.load(std::memory_order_acquire);
        if (sourceHwnd && hwnd == sourceHwnd) {
            return;
        }
        HWND hTaskbar = FindWindowW(L"Shell_TrayWnd", NULL);
        if (hwnd == hTaskbar || GetAncestor(hwnd, GA_ROOT) == hTaskbar) {
            return;
        }
        wchar_t className[256] = { 0 };
        if (GetClassNameW(hwnd, className, 256)) {
            if (wcscmp(className, L"Windows.UI.Core.CoreWindow") == 0 ||
                wcscmp(className, L"DesktopWindowXamlSource") == 0 ||
                wcscmp(className, L"PopupHost") == 0 ||
                wcscmp(className, L"Xaml_WindowedPopupClass") == 0 ||
                wcscmp(className, L"WhWin10ForecastPopupClass") == 0) {
                return;
            }
        }
        
        // Track valid active application window before flyout activation
        g_hPreFlyoutActiveWnd = hwnd;

        if (g_win11FlyoutIsOpen) {
            if (g_hSubclassedWnd) {
                PostMessageW(g_hSubclassedWnd, WM_USER + 4242, 0, 0);
            }
        }
        else if (popup && IsWindow(popup) && g_win10PopupVisible.load(std::memory_order_acquire)) {
            g_lastHideTickCount.store(GetTickCount(), std::memory_order_release);
            HideWin10Popup(popup);
        }
    }
}
// Out-of-context WinEvent callbacks run on the thread that installed the hook.
// Post only a HWND; all flyout state remains handled by its UI thread.
void CALLBACK WinEventProc(HWINEVENTHOOK, DWORD event, HWND hwnd,
                           LONG, LONG, DWORD, DWORD) {
    if (g_modUnloaded || event != EVENT_SYSTEM_FOREGROUND || !hwnd) return;
    HWND target = IsWindows11()
        ? g_foregroundMessageTarget.load(std::memory_order_acquire)
        : g_win10PopupHwnd.load(std::memory_order_acquire);
    if (target) PostMessageW(target, kForegroundChangedMessage, (WPARAM)hwnd, 0);
}

DWORD WINAPI ForegroundEventThreadProc(LPVOID parameter) {
    HANDLE stopEvent = static_cast<HANDLE>(parameter);
    MSG message{};
    PeekMessageW(&message, nullptr, 0, 0, PM_NOREMOVE); // Create this thread's queue.
    if (WaitForSingleObject(stopEvent, 0) == WAIT_OBJECT_0) return 0;
    HWINEVENTHOOK hook = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND,
                                        nullptr, WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);
    if (!hook) {
        Wh_Log(L"[Wh_WeatherHost] Foreground hook registration failed: %lu", GetLastError());
        return 0;
    }
    Wh_Log(L"[Wh_WeatherHost] Foreground hook installed on owner thread %lu", GetCurrentThreadId());
    bool quit = false;
    while (!quit) {
        DWORD wait = MsgWaitForMultipleObjects(1, &stopEvent, FALSE, INFINITE, QS_ALLINPUT);
        if (wait == WAIT_OBJECT_0) break;
        if (wait != WAIT_OBJECT_0 + 1) {
            Wh_Log(L"[Wh_WeatherHost] Foreground hook message wait failed: %lu", GetLastError());
            break;
        }
        while (WaitForSingleObject(stopEvent, 0) != WAIT_OBJECT_0 &&
               PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) { quit = true; break; }
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }
    BOOL removed = UnhookWinEvent(hook);
    Wh_Log(L"[Wh_WeatherHost] Foreground hook removed on owner thread %lu: success=%d",
           GetCurrentThreadId(), removed);
    // Windows also removes a thread's WinEvent hooks when that thread exits.
    // The unload path joins this thread before permitting DLL unload.
    return 0;
}

void StartForegroundEventThread() {
    if (g_hForegroundEventThread || g_modUnloaded) return;
    g_hForegroundEventStop = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!g_hForegroundEventStop) {
        Wh_Log(L"[Wh_WeatherHost] Foreground hook stop-event creation failed: %lu", GetLastError());
        return;
    }
    g_hForegroundEventThread = CreateThread(nullptr, 0, ForegroundEventThreadProc,
                                            g_hForegroundEventStop, 0, nullptr);
    if (!g_hForegroundEventThread) {
        Wh_Log(L"[Wh_WeatherHost] Foreground hook thread creation failed: %lu", GetLastError());
        CloseHandle(g_hForegroundEventStop);
        g_hForegroundEventStop = nullptr;
    }
}

void StopForegroundEventThread() {
    g_foregroundMessageTarget.store(nullptr, std::memory_order_release);
    if (g_hForegroundEventStop) SetEvent(g_hForegroundEventStop);
    if (g_hForegroundEventThread) {
        WaitForSingleObject(g_hForegroundEventThread, INFINITE);
        CloseHandle(g_hForegroundEventThread);
        g_hForegroundEventThread = nullptr;
        Wh_Log(L"[Wh_WeatherHost] Foreground hook owner thread joined");
    }
    if (g_hForegroundEventStop) {
        CloseHandle(g_hForegroundEventStop);
        g_hForegroundEventStop = nullptr;
    }
}

void LoadModConfiguration() {
    PCWSTR city = Wh_GetStringSetting(L"location");
    std::wstring newLocation = city ? city : L"auto";
    {
        std::lock_guard<std::mutex> lock(g_locationMutex);
        g_location = newLocation;
    }
    Wh_FreeStringSetting(city);
    g_showWeatherAlerts = Wh_GetIntSetting(L"showWeatherAlerts") != 0;
    g_useCelsius = Wh_GetIntSetting(L"useCelsius") != 0;
    g_showTempDecimals = Wh_GetIntSetting(L"showTempDecimals") != 0;
    g_updateInterval = Wh_GetIntSetting(L"updateInterval");
    if (g_updateInterval < 5)
        g_updateInterval = 5;
    if (g_updateInterval > 120)
        g_updateInterval = 120;
    int newWeatherStyle = Wh_GetIntSetting(L"weatherStyle");
    PCWSTR packPathStr = Wh_GetStringSetting(L"customIconPackPath");
    SetIconConfiguration(newWeatherStyle, packPathStr ? packPathStr : L"");
    Wh_FreeStringSetting(packPathStr);
    g_showConditionName = Wh_GetIntSetting(L"showConditionName") != 0;
    g_textOffset = Wh_GetIntSetting(L"textOffset");
    g_itemsRepeaterOffset = Wh_GetIntSetting(L"itemsRepeaterOffset");
    g_fontSize = Wh_GetIntSetting(L"fontSize");
    g_iconFontSize = Wh_GetIntSetting(L"iconFontSize");
    if (g_iconFontSize <= 0)
        g_iconFontSize = 16;
    PCWSTR f1 = Wh_GetStringSetting(L"line1FontFamily");
    g_line1FontFamily = (f1 && *f1) ? f1 : L"Segoe UI";
    Wh_FreeStringSetting(f1);
    g_line1FontSize = Wh_GetIntSetting(L"line1FontSize");
    if (g_line1FontSize <= 0) {
        g_line1FontSize = g_fontSize > 0 ? g_fontSize : 13;
    }
    g_line1Bold = Wh_GetIntSetting(L"line1Bold") != 0;
    PCWSTR f2 = Wh_GetStringSetting(L"line2FontFamily");
    g_line2FontFamily = (f2 && *f2) ? f2 : L"Segoe UI";
    Wh_FreeStringSetting(f2);
    g_line2FontSize = Wh_GetIntSetting(L"line2FontSize");
    if (g_line2FontSize <= 0) {
        g_line2FontSize = (g_fontSize - 2 > 8) ? (g_fontSize - 2) : 11;
    }
    g_line2Bold = Wh_GetIntSetting(L"line2Bold") != 0;
        g_debugLogs = Wh_GetIntSetting(L"debugLogs") != 0;
    g_injectToSysTray = Wh_GetIntSetting(L"injectToSysTray") != 0;
    g_win11FlyoutAnimation = Wh_GetIntSetting(L"win11FlyoutAnimation") != 0;
    g_animateIcons = Wh_GetIntSetting(L"animateIcons") != 0;
    g_useAcrylic = Wh_GetIntSetting(L"useAcrylic") != 0;
    g_acrylicOpacity = Wh_GetIntSetting(L"acrylicOpacity");
    if (g_acrylicOpacity < 0)
        g_acrylicOpacity = 0;
    if (g_acrylicOpacity > 100)
        g_acrylicOpacity = 100;
    PCWSTR bgUrlStr = Wh_GetStringSetting(L"bgImageVideoUrl");
    g_bgImageVideoUrl = bgUrlStr ? bgUrlStr : L"";
    Wh_FreeStringSetting(bgUrlStr);
    g_bgImageVideoOpacity = Wh_GetIntSetting(L"bgImageVideoOpacity");
    if (g_bgImageVideoOpacity < 0) g_bgImageVideoOpacity = 0;
    if (g_bgImageVideoOpacity > 100) g_bgImageVideoOpacity = 100;
    PCWSTR bgStretchStr = Wh_GetStringSetting(L"bgImageVideoStretch");
    if (bgStretchStr) {
        std::wstring s(bgStretchStr);
        std::transform(s.begin(), s.end(), s.begin(), ::towlower);
        if (s == L"none" || s == L"0") {
            g_bgImageVideoStretch = 0;
        }
        else if (s == L"fill" || s == L"1") {
            g_bgImageVideoStretch = 1;
        }
        else if (s == L"uniform" || s == L"2" || s == L"fit" || s == L"uniformtofit") {
            g_bgImageVideoStretch = 2;
        }
        else if (s == L"uniformtofill" || s == L"uniform_to_fill" || s == L"3" || s == L"crop") {
            g_bgImageVideoStretch = 3;
        }
        Wh_FreeStringSetting(bgStretchStr);
    }
    g_forecastDaysFetch = Wh_GetIntSetting(L"forecastDaysFetch");
    if (g_forecastDaysFetch < 1)
        g_forecastDaysFetch = 7;
    if (g_forecastDaysFetch > 16)
        g_forecastDaysFetch = 16;
    PCWSTR hotkeyModsStr = Wh_GetStringSetting(L"hotkeyModifiers");
    std::wstring mods = (hotkeyModsStr && *hotkeyModsStr) ? hotkeyModsStr : L"Win+Alt";
    Wh_FreeStringSetting(hotkeyModsStr);
    PCWSTR hotkeyKeyStr = Wh_GetStringSetting(L"hotkeyKey");
    std::wstring keyStr = (hotkeyKeyStr && *hotkeyKeyStr) ? hotkeyKeyStr : L"W";
    Wh_FreeStringSetting(hotkeyKeyStr);
    std::wstring lowerMods = mods;
    for (auto& c : lowerMods) c = towlower(c);
    g_hkWin = (lowerMods.find(L"win") != std::wstring::npos);
    g_hkAlt = (lowerMods.find(L"alt") != std::wstring::npos);
    g_hkCtrl = (lowerMods.find(L"ctrl") != std::wstring::npos || lowerMods.find(L"control") != std::wstring::npos);
    g_hkShift = (lowerMods.find(L"shift") != std::wstring::npos);
    g_hkVk = 0;
    if (!keyStr.empty()) {
        if (keyStr.length() == 1) {
            g_hkVk = towupper(keyStr[0]);
        }
        else if (keyStr == L"F1") g_hkVk = VK_F1;
        else if (keyStr == L"F2") g_hkVk = VK_F2;
        else if (keyStr == L"F3") g_hkVk = VK_F3;
        else if (keyStr == L"F4") g_hkVk = VK_F4;
        else if (keyStr == L"F5") g_hkVk = VK_F5;
        else if (keyStr == L"F6") g_hkVk = VK_F6;
        else if (keyStr == L"F7") g_hkVk = VK_F7;
        else if (keyStr == L"F8") g_hkVk = VK_F8;
        else if (keyStr == L"F9") g_hkVk = VK_F9;
        else if (keyStr == L"F10") g_hkVk = VK_F10;
        else if (keyStr == L"F11") g_hkVk = VK_F11;
        else if (keyStr == L"F12") g_hkVk = VK_F12;
        else if (keyStr == L"Space") g_hkVk = VK_SPACE;
        else if (keyStr == L"Tab") g_hkVk = VK_TAB;
        else if (keyStr == L"Enter") g_hkVk = VK_RETURN;
    }
    PCWSTR colStr = Wh_GetStringSetting(L"textColor");
    if (colStr && wcslen(colStr) == 7 && colStr[0] == L'#') {
        int r, g, b;
        if (swscanf(colStr + 1, L"%02x%02x%02x", &r, &g, &b) == 3) {
            g_textColor = RGB(r, g, b);
        }
    }
    else {
        g_textColor = RGB(255, 255, 255);
    }
    Wh_FreeStringSetting(colStr);
    g_radarOpacity = Wh_GetIntSetting(L"radarOpacity");
    if (g_radarOpacity < 0) g_radarOpacity = 0;
    if (g_radarOpacity > 100) g_radarOpacity = 100;
    g_precipitationOpacity = std::clamp(Wh_GetIntSetting(L"precipitationOpacity"), 0, 100);

    if (g_debugLogs)
        Wh_Log(
            L"[EP_WeatherHost] Reloaded settings: loc=%s, metric=%d, fontS=%d",
            newLocation.c_str(), g_useCelsius.load() ? 1 : 0, g_fontSize);
    if (g_weatherStyle != 2) {
        ClearWin10TaskbarSvgCache();
    }
    {
        std::lock_guard<std::mutex> stateLock(g_currentWeatherMutex);
        g_cachedIconUrl = GetWeatherIconPathOrUrl(g_cachedWeatherCode, g_cachedIsDay != 0);
        auto mapping = GetCodeMapping(g_cachedWeatherCode);
        g_cachedIcon = mapping.first;
    }
    EnterCriticalSection(&g_forecastLock);
    for (auto& d : g_forecastDaily) {
        d.iconUrl = GetWeatherIconPathOrUrl(d.weatherCode, true);
    }
    for (auto& h : g_forecastHourly) {
        h.iconUrl = GetWeatherIconPathOrUrl(h.weatherCode, h.isDay != 0);
    }
    LeaveCriticalSection(&g_forecastLock);
    QueueWeatherUpdateOnUIThread();
    if (g_hSubclassedWnd && IsWindow(g_hSubclassedWnd)) {
        PostMessageW(g_hSubclassedWnd, g_msgHotkeyControl, 1, 0);
    }
    if (g_hForceUpdateEvent) {
        SetEvent(g_hForceUpdateEvent);
    }
}
BOOL CALLBACK ForceTaskbarUpdateCallback(HWND hTaskbar, LPARAM lParam) {
    WCHAR className[256];
    if (GetClassNameW(hTaskbar, className, 256)) {
        if (wcscmp(className, L"Shell_TrayWnd") == 0 || wcscmp(className, L"Shell_SecondaryTrayWnd") == 0) {
            HWND hReBar = FindWindowExW(hTaskbar, nullptr, L"ReBarWindow32", nullptr);
            if (hReBar) {
                HWND hMSTask = FindWindowExW(hReBar, nullptr, L"MSTaskSwWClass", nullptr);
                if (hMSTask) {
                    PostMessageW(hMSTask, 0x452, 3, 0);
                }
            }
            PostMessageW(hTaskbar, WM_SIZE, 0, 0);
        }
    }
    return TRUE;
}
void ForceTaskbarUpdateOriginal() {
    EnumWindows(ForceTaskbarUpdateCallback, 0);
}
DWORD WINAPI SubclassWatchdogThread(LPVOID lpParam) {
    int checkCount = 0;
    while (!g_bThreadShouldTerm) {
        HWND currentTarget = ShouldUseXamlTaskbar() ? FindSystemAnchorWnd() : FindTrayNotifyWnd();
        if (currentTarget && (currentTarget != g_hSubclassedWnd || !IsWindow(g_hSubclassedWnd))) {
            AlignOverlayWindow();
        }
        DWORD sleepTime = (checkCount < 60) ? 500 : 2000;
        DWORD elapsed = 0;
        while (elapsed < sleepTime && !g_bThreadShouldTerm) {
            if (WaitForSingleObject(g_hForceUpdateEvent, 100) == WAIT_OBJECT_0) break;
            elapsed += 100;
        }
        checkCount++;
    }
    return 0;
}
// Windhawk mod Entry Point
BOOL Wh_ModInit() {
    g_modUnloaded = false;
    g_isShellProcess = IsShellProcess();
    if (!g_isShellProcess) {
        return TRUE;
    }

    HWND hInitActive = GetForegroundWindow();
    HWND hTaskbar = FindWindowW(L"Shell_TrayWnd", NULL);
    if (hInitActive && hInitActive != hTaskbar && GetAncestor(hInitActive, GA_ROOT) != hTaskbar) {
        g_hPreFlyoutActiveWnd = hInitActive;
    }
    // Initial check if taskbar module is loaded or intercept via library load
    // hooks
    HMODULE mod = GetModuleHandle(L"Taskbar.View.dll");
    if (!mod)
        mod = GetModuleHandle(L"ExplorerExtensions.dll");
    if (mod) {
        g_taskbarViewDllLoaded = true;
        HookTaskbarViewDllSymbols(mod);
    }
    else {
        HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
        if (hNtdll) {
            auto pLdrLoadDll = (LdrLoadDll_t)GetProcAddress(hNtdll, "LdrLoadDll");
            if (pLdrLoadDll) {
                WindhawkUtils::SetFunctionHook(pLdrLoadDll, LdrLoadDll_Hook, &LdrLoadDll_Original);
            }
        }
        WindhawkUtils::SetFunctionHook(LoadLibraryExW, LoadLibraryExW_Hook,
            &LoadLibraryExW_Original);
    }
    if (g_hSubclassedWnd && IsWindow(g_hSubclassedWnd)) {
        SetWindowPos(g_hSubclassedWnd, NULL, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    }
    g_msgHotkeyControl = RegisterWindowMessageW(L"WhWeatherHost_HotkeyControl");
    InitializeCriticalSection(&g_forecastLock);
    InitializeCriticalSection(&g_subclassLock);
    g_hForceUpdateEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!ShouldUseXamlTaskbar()) {
        // Auto-reset event used both for retry backoff and to wake the Win10
        // popup STA immediately when the user requests the flyout. The STA itself
        // is started in Wh_ModAfterInit, after Windhawk has finished applying hooks.
        g_hPopupRetryEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
    }
    LoadModConfiguration();
    if (g_debugLogs) {
        Wh_Log(L"[Wh_WeatherHost] Candidate zoom-settings-alertstate-20260929; ClockSubclass=%p TaskListSubclass=%p VisualStatesHook=%p",
               (void*)ClockSubclassWndProc, (void*)TaskListSubclassWndProc,
               (void*)TaskListButton_UpdateVisualStates_Hook);
    }
    g_bThreadShouldTerm = false;
    StartForegroundEventThread();
    // Background weather crawl
    g_hQueryThread =
        CreateThread(NULL, 0, QueryWeatherPipeline, NULL, 0, nullptr);
    // Watchdog thread for rapid, automated taskbar subclass injection/maintenance
    g_hWatchdogThread =
        CreateThread(NULL, 0, SubclassWatchdogThread, NULL, 0, NULL);
    // Force tasks to refresh immediately so scan triggers without clicking
    ForceTaskbarUpdateOriginal();
    AlignOverlayWindow();
    // Force a visual state update on the taskbar to immediately trigger our
    // subclass hook
    hTaskbar = FindWindowW(L"Shell_TrayWnd", NULL);
    if (hTaskbar) {
        SendMessageTimeoutW(hTaskbar, WM_SETTINGCHANGE, 0,
            (LPARAM)L"ImmersiveColorSet", SMTO_ABORTIFHUNG, 1000, NULL);
        SendMessageTimeoutW(hTaskbar, WM_THEMECHANGED, 0, 0,
            SMTO_ABORTIFHUNG, 1000, NULL);
    }
    QueueWeatherUpdateOnUIThread();
    if (g_debugLogs)
        Wh_Log(
            L"[EP_WeatherHost] Modern Native XAML Injection Mod successfully "
            L"loaded.");
    return TRUE;
}
void Wh_ModAfterInit() {
    if (!g_isShellProcess || ShouldUseXamlTaskbar()) return;

    // Start proactively to keep normal first-open latency at zero. If Explorer's
    // desktop XAML infrastructure is still too early, PopupThreadProc remains
    // alive and retries until it becomes available.
    EnsureWin10PopupThreadStarted();
}

// Windhawk pre-unload phase: hooks are still installed here, so use them to
// synchronously close XAML UI before Windhawk removes hook targets.
void Wh_ModBeforeUninit() {
    if (!g_isShellProcess) return;

    const bool useXamlTaskbar = ShouldUseXamlTaskbar();

    g_modUnloaded = true;
    StopForegroundEventThread();
    g_bThreadShouldTerm = true;
    if (g_hForceUpdateEvent) SetEvent(g_hForceUpdateEvent);
    {
        // Wait for any already-admitted thread creation to publish its handle.
        std::lock_guard<std::mutex> lock(g_win10PopupThreadMutex);
        if (g_hPopupRetryEvent) SetEvent(g_hPopupRetryEvent);
    }

    if (g_hKeyboardHook) { UnhookWindowsHookEx(g_hKeyboardHook); g_hKeyboardHook = NULL; }
    if (g_hMouseHook) { UnhookWindowsHookEx(g_hMouseHook); g_hMouseHook = NULL; }

    if (useXamlTaskbar) {
        HWND hShellTray = FindWindowW(L"Shell_TrayWnd", NULL);
        HWND hAnchor = FindSystemAnchorWnd();
        HWND hTarget = g_hSubclassedWnd ? g_hSubclassedWnd : (hAnchor ? hAnchor : hShellTray);
        DWORD uiThread = hTarget ? GetWindowThreadProcessId(hTarget, NULL) : 0;
        if (uiThread != 0 && uiThread == GetCurrentThreadId()) {
            PerformXamlCleanupOnUIThread();
        } else if (hTarget && IsWindow(hTarget)) {
            DWORD_PTR result = 0;
            SendMessageTimeoutW(hTarget, WM_USER + 4245, 0, 0,
                                SMTO_ABORTIFHUNG, 2000, &result);
        }
    } else {
        // The Win10 XAML Island belongs to its dedicated STA. Hide/release its
        // content there; Wh_ModUninit will then stop and join the STA thread.
        HWND popup = g_win10PopupHwnd.load(std::memory_order_acquire);
        if (popup && IsWindow(popup)) {
            DWORD_PTR result = 0;
            SendMessageTimeoutW(popup, WM_USER + 5002, 0, 0,
                                SMTO_ABORTIFHUNG, 2000, &result);
        }
    }
}

// Windhawk mod Exit Point

// A dispatcher can shut down without invoking a queued RunAsync delegate.
// Keep the code containing that delegate mapped if the bounded drain expires.
void PinWeatherModuleIfAsyncCallbacksRemain() {
    int pending = g_pendingRunAsync.load(std::memory_order_acquire);
    if (pending <= 0) return;

    HMODULE module = nullptr;
    if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                               GET_MODULE_HANDLE_EX_FLAG_PIN,
                           reinterpret_cast<LPCWSTR>(&PinWeatherModuleIfAsyncCallbacksRemain), &module)) {
        Wh_Log(L"[EP_WeatherHost] %d dispatcher callbacks remain at unload; "
               L"mod DLL pinned until Explorer exits.", pending);
    } else {
        Wh_Log(L"[EP_WeatherHost] Could not pin mod DLL with %d dispatcher "
               L"callbacks pending (error %lu).", pending, GetLastError());
    }
}

void Wh_ModUninit() {
    Wh_Log(L"[EP_WeatherHost] Wh_ModUninit: Cleanup initiated.");
    g_modUnloaded = true;
    StopForegroundEventThread();
    g_bThreadShouldTerm = true;
    {
        std::lock_guard<std::mutex> lock(g_win10PopupThreadMutex);
        if (g_hPopupRetryEvent) SetEvent(g_hPopupRetryEvent);
    }

    // 1. Signal background worker threads to terminate immediately before touching UI or hooks
    if (g_hForceUpdateEvent) {
        SetEvent(g_hForceUpdateEvent);
    }
    if (g_hQueryThread) {
        WaitForSingleObject(g_hQueryThread, INFINITE);
        CloseHandle(g_hQueryThread);
        g_hQueryThread = NULL;
    }
    if (g_hWatchdogThread) {
        WaitForSingleObject(g_hWatchdogThread, INFINITE);
        CloseHandle(g_hWatchdogThread);
        g_hWatchdogThread = NULL;
    }
    if (g_hForceUpdateEvent) {
        CloseHandle(g_hForceUpdateEvent);
        g_hForceUpdateEvent = NULL;
    }
    
    int waitCounter = 0;
    while (g_pendingRunAsync > 0 && waitCounter < 20) {
        Sleep(50);
        waitCounter++;
    }
    PinWeatherModuleIfAsyncCallbacksRemain();
    
    Wh_Log(L"[EP_WeatherHost] Wh_ModUninit: Background worker threads stopped.");

    if (!g_isShellProcess) {
        Wh_Log(L"[EP_WeatherHost] Wh_ModUninit: Exiting early for non-shell process.");
        return;
    }
    const bool useXamlTaskbar = ShouldUseXamlTaskbar();
    if (g_debugLogs)
        Wh_Log(L"[EP_WeatherHost] Unloading mod...");

    // 2. Remove Windows hooks early so no hook procedures fire
    if (g_hKeyboardHook) { UnhookWindowsHookEx(g_hKeyboardHook); g_hKeyboardHook = NULL; }
    if (g_hMouseHook) { UnhookWindowsHookEx(g_hMouseHook); g_hMouseHook = NULL; }
    Wh_Log(L"[EP_WeatherHost] Wh_ModUninit: Windows hooks unhooked.");

    // 3. Clean up taskbar-owned XAML on its UI thread. The classic backend's
    // XAML objects belong to the dedicated popup STA and are released below.
    if (useXamlTaskbar) {
        HWND hShellTray = FindWindowW(L"Shell_TrayWnd", NULL);
        HWND hAnchor = FindSystemAnchorWnd();
        HWND hTarget = g_hSubclassedWnd ? g_hSubclassedWnd : (hAnchor ? hAnchor : hShellTray);

        DWORD dwUIThreadId = hTarget ? GetWindowThreadProcessId(hTarget, NULL) : 0;
        if (dwUIThreadId != 0 && dwUIThreadId == GetCurrentThreadId()) {
            PerformXamlCleanupOnUIThread();
            Wh_Log(L"[EP_WeatherHost] Wh_ModUninit: UI thread XAML cleanup completed directly.");
        } else if (hTarget && IsWindow(hTarget)) {
            DWORD_PTR dwResult = 0;
            SendMessageTimeoutW(hTarget, WM_USER + 4245, 0, 0, SMTO_ABORTIFHUNG, 2000, &dwResult);
            if (g_hSubclassedWnd && g_hSubclassedWnd != hTarget && IsWindow(g_hSubclassedWnd)) {
                SendMessageTimeoutW(g_hSubclassedWnd, WM_USER + 4245, 0, 0, SMTO_ABORTIFHUNG, 2000, &dwResult);
            }
            Wh_Log(L"[EP_WeatherHost] Wh_ModUninit: Taskbar XAML cleanup message sent.");
        } else {
            PerformXamlCleanupOnUIThread();
        }
    }

    // 4. Safely remove subclassing and restore layout
    HWND hwndToClean = NULL;
    EnterCriticalSection(&g_subclassLock);
    if (g_hSubclassedWnd) {
        hwndToClean = g_hSubclassedWnd;
        RestoreDefaultWindowSize(hwndToClean);
        WindhawkUtils::RemoveWindowSubclassFromAnyThread(hwndToClean, ClockSubclassWndProc);
        g_hSubclassedWnd = NULL;
        g_currentAddedWeatherWidth = 0;
    }
    if (g_hSubclassedTaskListWnd && IsWindow(g_hSubclassedTaskListWnd)) {
        WindhawkUtils::RemoveWindowSubclassFromAnyThread(g_hSubclassedTaskListWnd, TaskListSubclassWndProc);
        SetWindowPos(g_hSubclassedTaskListWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
        g_hSubclassedTaskListWnd = NULL;
    }
    LeaveCriticalSection(&g_subclassLock);
    Wh_Log(L"[EP_WeatherHost] Wh_ModUninit: Subclasses removed.");

    // Clear any last custom clock pixels after restoring native geometry.
    if (hwndToClean && IsWindow(hwndToClean)) {
        HWND taskbarRoot = GetAncestor(hwndToClean, GA_ROOT);
        if (!taskbarRoot) taskbarRoot = FindWindowW(L"Shell_TrayWnd", NULL);
        RedrawWindow(hwndToClean, NULL, NULL,
                     RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN | RDW_UPDATENOW);
        if (taskbarRoot && IsWindow(taskbarRoot)) {
            RedrawWindow(taskbarRoot, NULL, NULL,
                         RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN | RDW_UPDATENOW);
        }
    }

    // 5. Tell the popup thread to tear down gracefully on its own STA apartment (Win10)
    if (g_hPopupRetryEvent) SetEvent(g_hPopupRetryEvent);
    g_win10PopupInitState.store(Win10PopupInitState::Stopping, std::memory_order_release);
    if (g_dwPopupThreadId) {
        HWND popup = g_win10PopupHwnd.load(std::memory_order_acquire);
        if (popup && IsWindow(popup)) {
            PostMessageW(popup, WM_USER + 4246, 0, 0);
        }
        PostThreadMessageW(g_dwPopupThreadId, WM_QUIT, 0, 0);
        if (g_hPopupThread) {
            WaitForSingleObject(g_hPopupThread, INFINITE);
            CloseHandle(g_hPopupThread);
            g_hPopupThread = nullptr;
        }
        g_dwPopupThreadId = 0;
        Wh_Log(L"[EP_WeatherHost] Wh_ModUninit: Popup thread joined.");
    }
    g_pendingWin10OpenClock.store(nullptr, std::memory_order_release);
    g_win10PopupInitState.store(Win10PopupInitState::NotStarted, std::memory_order_release);
    if (g_hPopupRetryEvent) {
        CloseHandle(g_hPopupRetryEvent);
        g_hPopupRetryEvent = NULL;
    }

    // Give time for window destruction messages to process before unregistering the class
    Sleep(50);
    UnregisterClassW(L"WhWin10ForecastPopupClass", GetModuleHandleW(NULL));
    if (hwndToClean && IsWindow(hwndToClean)) {
        SendMessageW(hwndToClean, WM_USER + 4243, 0, 0);
    }

    ClearWin10TaskbarSvgCache();
    if (g_pD2DFactory) {
        g_pD2DFactory->Release();
        g_pD2DFactory = nullptr;
    }
    if (g_pDWriteFactory) {
        g_pDWriteFactory->Release();
        g_pDWriteFactory = nullptr;
    }
    Wh_Log(L"[EP_WeatherHost] Wh_ModUninit: Direct2D and BufferedPaint uninitialized.");

    DeleteCriticalSection(&g_forecastLock);
    DeleteCriticalSection(&g_subclassLock);
    // UI cleanup may have queued callbacks before the subclasses were removed.
    // Do not let a late delegate call back into an unmapped DLL.
    PinWeatherModuleIfAsyncCallbacksRemain();
    Wh_Log(L"[EP_WeatherHost] Wh_ModUninit: Cleanup completed successfully.");
}
// Settings update receiver
void Wh_ModSettingsChanged() {
    if (!g_isShellProcess || g_modUnloaded) {
        return;
    }
    g_isInitScan = true;
    bool oldInjectToSysTray = g_injectToSysTray;
    LoadModConfiguration();
    // If injection target changed, explicitly clean up old injection before re-scanning
    if (oldInjectToSysTray != g_injectToSysTray) {
        std::vector<winrt::Windows::UI::Xaml::FrameworkElement> framesToRescan;
        {
            std::lock_guard<std::mutex> lock(g_pendingMutex);
            for (auto& f : g_scannedFrames) {
                if (auto frame = f.get()) {
                    framesToRescan.push_back(frame);
                }
            }
            // Keep the weak frame list across target changes. A direct rescan
            // doesn't rediscover/re-register the frame, so clearing it here
            // makes Widgets -> tray work but leaves tray -> Widgets with no
            // remembered taskbar frame to scan.
        }
        FrameworkElement targetElement = nullptr;
        {
            std::lock_guard<std::mutex> lock(g_weatherGridMutex);
            for (auto& weakG : g_injectedWeatherGrids) {
                if (auto g = weakG.get()) {
                    targetElement = g;
                    break;
                }
            }
            g_injectedWeatherGrids.clear();
        }
        if (targetElement) {
            try {
                auto dispatcher = targetElement.Dispatcher();
                if (dispatcher && !g_modUnloaded) {
                    g_pendingRunAsync++;
                    try {
                        dispatcher.RunAsync(
                            winrt::Windows::UI::Core::CoreDispatcherPriority::Normal,
                            [targetElement, framesToRescan]() {
struct AutoDecrement { ~AutoDecrement() { g_pendingRunAsync--; } } autoDec;


                                if (g_modUnloaded) return;
                                RestoreRepeaterMarginsOnCurrentUIThread();
                                try {
                                    if (auto parent = targetElement.Parent().try_as<winrt::Windows::UI::Xaml::Controls::Panel>()) {
                                        RemoveInjectedFromPanel(parent);
                                    }
                                    else if (auto grid = targetElement.Parent().try_as<Grid>()) {
                                        RemoveInjectedFromGrid(grid);
                                    }
                                }
                                catch (...) {}
                                for (auto const& frame : framesToRescan) {
                                    try { ScanAndInjectRecursive(frame); } catch (...) {}
                                }
                            });
                    }
                    catch (...) {
                        g_pendingRunAsync--;
                    }
                }
            }
            catch (...) {}
        }
        else {
            for (auto const& frame : framesToRescan) {
                try {
                    auto disp = frame.Dispatcher();
                    if (disp && !g_modUnloaded) {
                        g_pendingRunAsync++;
                        try {
                            disp.RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::Normal, [frame]() {
struct AutoDecrement { ~AutoDecrement() { g_pendingRunAsync--; } } autoDec;

                                if (g_modUnloaded) return;
                                RestoreRepeaterMarginsOnCurrentUIThread();
                                try { ScanAndInjectRecursive(frame); } catch (...) {}
                            });
                        }
                        catch (...) {
                            g_pendingRunAsync--;
                        }
                    }
                } catch (...) {}
            }
        }
        // Trigger a re-scan of the taskbar to find the new target
        ForceTaskbarUpdateOriginal();
    }
    AlignOverlayWindow();
    if (g_hSubclassedWnd && IsWindow(g_hSubclassedWnd)) {
        SetWindowPos(g_hSubclassedWnd, NULL, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    }
    ForceTaskbarUpdateOriginal();
    HWND hTaskbar = FindWindowW(L"Shell_TrayWnd", NULL);
    if (hTaskbar) {
        SendMessageTimeoutW(hTaskbar, WM_SETTINGCHANGE, 0,
            (LPARAM)L"ImmersiveColorSet", SMTO_ABORTIFHUNG, 1000, NULL);
        SendMessageTimeoutW(hTaskbar, WM_THEMECHANGED, 0, 0,
            SMTO_ABORTIFHUNG, 1000, NULL);
    }
    QueueWeatherUpdateOnUIThread();
}
