// ==WindhawkMod==
// @id              wh-weather
// @name            Windhawk Weather
// @description     A lightweight taskbar weather widget and flyout that injects directly into the Windows Taskbar UI Tree.
// @version         1.1.0
// @author          bbmaster123
// @include         explorer.exe
// @compilerOptions -DWINVER=0x0A00 -lgdi32 -luser32 -luxtheme -lwinhttp -lshlwapi -lole32 -luuid -lshell32 -loleaut32 -ldwmapi -lruntimeobject -lshcore -lversion -lcomctl32 -ld2d1 -ldwrite
// ==/WindhawkMod==

// ==WindhawkModSettings==
/*
- location: "auto"
  $name: Location (City Name or "auto" for IP-based Geolocation)
- mockWeatherAlert: ""
  $name: Mock Weather Alert (Custom text to display as an advisory banner, e.g. "Air Quality Alert - Wildfire Smoke". Leave blank for live alerts)
- useCelsius: true
  $name: Use Metric System (°C, km/h)
  $description: Uncheck to use Imperial System (°F, mph)
- updateInterval: 15
  $name: Update Interval (between 5 and 120 minutes)
- weatherStyle: false
  $name: Weather Icon Style
  $description: Uncheck for Colorful Modern Emoji icons, check for Segoe MDL2 Assets vector symbols
- showConditionName: true
  $name: Show Condition Name (e.g. "Sunny", "Cloudy")
- itemsRepeaterOffset: 110
  $name: Taskbar Items Shift (pixels to push app icons away from weather)
- textOffset: 10
  $name: Horizontal Offset (Pixels)
- textColor: "#FFFFFF"
  $name: Text Color
- fontSize: 13
  $name: Primary Weather Text Font Size
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
- forecastDaysFetch: 14
  $name: Forecast API query length (Days, max 16)
- injectToSysTray: false
  $name: Inject to System Tray (Windows 11)
  $description: Check to inject next to the clock/system tray instead of replacing the Widgets button
- panelFontScale: 100
  $name: Forecast Panel Font Scale (%)
  $description: Scale the text size in the forecast popup panel (e.g. 100 for default, 120 for larger text)
- hotkeyModifiers: "Win+Alt"
  $name: Hotkey Modifiers
  $description: Combination of Win, Alt, Ctrl, Shift separated by '+' (e.g. "Win+Alt", "Ctrl+Shift", or "None")
- hotkeyKey: "W"
  $name: Hotkey Key
  $description: A single letter or number (e.g. "W", "I", "F8")
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
#include <dwrite.h>

#undef GetCurrentTime

#include <winrt/Windows.Foundation.Collections.h>
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


#include <atomic>
#include <mutex>


using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Media;

// Global settings
std::wstring g_location;
bool g_useCelsius = false;
int g_updateInterval = 15;
int g_weatherStyle = 0;  // 0 = Emoji, 1 = Segoe MDL2
bool g_showConditionName = true;
bool g_useAcrylic = true;
int g_acrylicOpacity = 85;
std::wstring g_bgImageVideoUrl = L"";
int g_bgImageVideoOpacity = 30;
int g_bgImageVideoStretch = 3;
int g_forecastDaysFetch = 14;
int g_textOffset = 10;
int g_itemsRepeaterOffset = 110;
COLORREF g_textColor = RGB(255, 255, 255);
int g_fontSize = 13;
int g_iconFontSize = 16;
bool g_debugLogs = false;
bool g_injectToSysTray = false;

double g_panelFontScale = 1.0;
bool g_hkWin = false;
bool g_hkAlt = false;
bool g_hkCtrl = false;
bool g_hkShift = false;
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
UINT g_hkVk = 0;
HHOOK g_hKeyboardHook = NULL;
bool g_hotkeyRegistered = false;
int g_currentAddedWeatherWidth = 0;
bool g_lastWasHorizontal = true;
int g_cleanCx = 0;
int g_cleanCy = 0;
int g_cleanX = 0;
int g_cleanY = 0;

std::wstring g_line1FontFamily = L"Segoe UI";
int g_line1FontSize = 13;
bool g_line1Bold = true;
std::wstring g_line2FontFamily = L"Segoe UI";
int g_line2FontSize = 11;
bool g_line2Bold = false;
int g_density = 2;

// Weather state Cached values
std::wstring g_cachedTemp = L"--°F";
std::wstring g_cachedCondition = L"Loading";
std::wstring g_cachedIcon = L"⏳";
std::wstring g_activeWarning = L"";
std::wstring g_mockWeatherAlert = L"";
std::wstring g_displayCity = L"Detecting...";
double g_cachedLatitude = 40.7128;  // New York Default
double g_cachedLongitude = -74.0060;
bool g_weatherAcquired = false;

// Background Thread values
HANDLE g_hQueryThread = NULL;
HANDLE g_hWatchdogThread = NULL;
DWORD g_dwThreadId = 0;
bool g_bThreadShouldTerm = false;
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
};

struct ThreadXamlState {
    winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager manager = nullptr;
    winrt::Windows::UI::Xaml::Hosting::DesktopWindowXamlSource source = nullptr;
    HWND popupHwnd = nullptr;
    HWND sourceHwnd = nullptr;
};

ThreadXamlState g_globalXamlState;

ThreadXamlState& GetThreadXamlState() {
    return g_globalXamlState;
}

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

extern HWND g_hSubclassedWnd;
bool IsWindows11();
void AlignOverlayWindow();
void RestoreDefaultWindowSize(HWND hwndToClean);
int GetRequiredWeatherWidth(HWND hWnd);
void InvalidateClockParentRegion(HWND hWnd);
static bool g_inInternalResize = false;
void ForceTaskbarUpdateOriginal();
void CleanupXamlMedia(winrt::Windows::UI::Xaml::UIElement const& element);

std::pair<std::wstring, std::wstring> GetCodeMapping(int code);

// Injected Grid reference
winrt::weak_ref<Grid> g_injectedWeatherGrid = nullptr;
std::mutex g_weatherGridMutex;

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

    ClockSearchData data = {NULL};
    EnumChildWindows(hShell, FindClockEnumChildProc, (LPARAM)&data);

    return data.hClock;
}

HWND FindTrayNotifyWnd() {
    HWND hShell = NULL;
    while ((hShell = FindWindowExW(NULL, hShell, L"Shell_TrayWnd", NULL))) {
        DWORD processId;
        GetWindowThreadProcessId(hShell, &processId);
        if (processId == GetCurrentProcessId()) {
            return FindWindowExW(hShell, NULL, L"TrayNotifyWnd", NULL);
        }
    }
    return NULL;
}

HWND FindTrayClockWnd() {
    HWND hTray = FindTrayNotifyWnd();
    if (!hTray)
        return NULL;
    return FindWindowExW(hTray, NULL, L"TrayClockWClass", NULL);
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
    AnchorSearchData data = {NULL};
    EnumChildWindows(hShell, FindAnchorEnumChildProc, (LPARAM)&data);
    return data.hAnchor ? data.hAnchor : hShell;
}

// Update margin of injected XAML Grid dynamically based on real-time taskbar
// layout state
bool ShouldUseXamlTaskbar();

void UpdateInjectedWeatherLayout(Grid weatherGrid) {
    if (!weatherGrid)
        return;
    try {
        HWND hAnchor = FindSystemAnchorWnd();
        if (hAnchor) {
            RECT anchorRect;
            GetWindowRect(hAnchor, &anchorRect);

            // Resolve correct parent taskbar window (primary or secondary for
            // multi-monitor)
            HWND hParentTaskbar = GetAncestor(hAnchor, GA_ROOT);
            if (!hParentTaskbar)
                hParentTaskbar = FindWindowW(L"Shell_TrayWnd", NULL);

            if (hParentTaskbar) {
                RECT trayRect;
                GetWindowRect(hParentTaskbar, &trayRect);

                bool isHorizontal = (trayRect.right - trayRect.left) >
                                    (trayRect.bottom - trayRect.top);
                int marginFromRight = trayRect.right - anchorRect.left - g_textOffset;
                
                if (isHorizontal) {
                    if (g_injectToSysTray) {
                        weatherGrid.HorizontalAlignment(HorizontalAlignment::Right);
                        weatherGrid.VerticalAlignment(VerticalAlignment::Center);
                        weatherGrid.Margin(Thickness{0, 0, (double)marginFromRight, 0});

                        // Reserve space next to the sibling TaskbarFrameRepeater/ItemsRepeater
                        try {
                            if (auto parentGrid = weatherGrid.Parent().try_as<Grid>()) {
                                auto siblings = parentGrid.Children();
                                for (uint32_t i = 0; i < siblings.Size(); i++) {
                                    try {
                                        if (auto sibling = siblings.GetAt(i).try_as<FrameworkElement>()) {
                                            std::wstring sibClass(winrt::get_class_name(sibling).c_str());
                                            if (sibClass.find(L"Repeater") != std::wstring::npos || sibClass.find(L"TaskbarFrameRepeater") != std::wstring::npos) {
                                                sibling.Margin(Thickness{0, 0, (double)marginFromRight + (double)g_itemsRepeaterOffset, 0});
                                            }
                                        }
                                    } catch (...) {}
                                }
                            }
                        } catch (...) {}
                    } else {
                        weatherGrid.HorizontalAlignment(HorizontalAlignment::Stretch);
                        weatherGrid.VerticalAlignment(VerticalAlignment::Stretch);
                        weatherGrid.Margin(Thickness{0, 0, 0, 0});
                    }
                } else {
                    weatherGrid.HorizontalAlignment(HorizontalAlignment::Center);
                    weatherGrid.VerticalAlignment(g_injectToSysTray ? VerticalAlignment::Bottom : VerticalAlignment::Stretch);
                    if (g_injectToSysTray) {
                        weatherGrid.Margin(
                            Thickness{0, 0, 0,
                                      (double)(trayRect.bottom - anchorRect.top -
                                               g_textOffset)});
                        // Reserve space on vertical taskbars
                        try {
                            if (auto parentGrid = weatherGrid.Parent().try_as<Grid>()) {
                                auto siblings = parentGrid.Children();
                                for (uint32_t i = 0; i < siblings.Size(); i++) {
                                    try {
                                        if (auto sibling = siblings.GetAt(i).try_as<FrameworkElement>()) {
                                            std::wstring sibClass(winrt::get_class_name(sibling).c_str());
                                            if (sibClass.find(L"Repeater") != std::wstring::npos || sibClass.find(L"TaskbarFrameRepeater") != std::wstring::npos) {
                                                sibling.Margin(Thickness{0, 0, 0, (double)(trayRect.bottom - anchorRect.top - g_textOffset) + (double)(g_itemsRepeaterOffset / 1.5)});
                                            }
                                        }
                                    } catch (...) {}
                                }
                            }
                        } catch (...) {}
                    } else {
                        weatherGrid.Margin(Thickness{0, 0, 0, 0});
                    }
                }
            }
        } else {
            weatherGrid.HorizontalAlignment(HorizontalAlignment::Right);
            weatherGrid.VerticalAlignment(VerticalAlignment::Center);
            weatherGrid.Margin(Thickness{0, 0, 100, 0});
        }
    } catch (...) {
    }
}

// State for interactive flyout
winrt::weak_ref<winrt::Windows::UI::Xaml::Controls::Button> g_weakXamlWeatherButton;
std::function<void()> g_showWin11Flyout = nullptr;
bool g_win11FlyoutIsOpen = false;
winrt::Windows::UI::Xaml::Controls::Flyout g_activeFlyout = nullptr;
ULONGLONG g_lastClosedTickCount = 0;
std::wstring g_selectedDate = L"";
std::wstring g_selectedHour = L"";
double g_graphScrollOffset = -1.0;
double g_dailyScrollOffset = 0.0;

int g_selectedGraphTab = 0;  // 0=Temp, 1=Precip, 2=Wind

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

void PopulateForecastUI(winrt::Windows::UI::Xaml::Controls::Grid rootGrid,
                        std::wstring condition,
                        std::wstring currentIcon,
                        std::wstring currentTemp,
                        bool animate = true,
                        bool animateGraph = true,
                        std::wstring customPrecip = L"",
                        std::wstring customHumidity = L"",
                        std::wstring customWind = L"");

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

    PopulateForecastUI(rootGrid, condition, currentIcon, currentTemp);



    flyout.Content(rootGrid);

    flyout.Opened([](auto const&, auto const&) {
        g_win11FlyoutIsOpen = true;
    });
    flyout.Closed([](auto const&, auto const&) {
        g_win11FlyoutIsOpen = false;
        g_lastClosedTickCount = GetTickCount64();
    });

    // Make the flyout background transparent so our custom Grid shapes it
    winrt::Windows::UI::Xaml::Style flyoutStyle(
        winrt::xaml_typename<
            winrt::Windows::UI::Xaml::Controls::FlyoutPresenter>());
    winrt::Windows::UI::Xaml::Setter bgSetter(
        winrt::Windows::UI::Xaml::Controls::Control::BackgroundProperty(),
        winrt::box_value(
            SolidColorBrush{winrt::Windows::UI::Colors::Transparent()}));
    
    // Set Margin on the FlyoutPresenter to 0 to prevent double margin spacing
    winrt::Windows::UI::Xaml::Setter marginSetter(
        winrt::Windows::UI::Xaml::FrameworkElement::MarginProperty(),
        winrt::box_value(Thickness{0, 0, 0, 0}));

    winrt::Windows::UI::Xaml::Setter padSetter(
        winrt::Windows::UI::Xaml::Controls::Control::PaddingProperty(),
        winrt::box_value(Thickness{0, 0, 0, 0}));
    winrt::Windows::UI::Xaml::Setter borSetter(
        winrt::Windows::UI::Xaml::Controls::Control::BorderThicknessProperty(),
        winrt::box_value(Thickness{0, 0, 0, 0}));
    winrt::Windows::UI::Xaml::Setter cornerSetter(
        winrt::Windows::UI::Xaml::Controls::Control::CornerRadiusProperty(),
        winrt::box_value(winrt::Windows::UI::Xaml::CornerRadius{8.0, 8.0, 8.0, 8.0}));
    // Override MaxWidth to allow wider flyout panels
    winrt::Windows::UI::Xaml::Setter maxWidthSetter(
        winrt::Windows::UI::Xaml::FrameworkElement::MaxWidthProperty(),
        winrt::box_value((double)9999.0));

    flyoutStyle.Setters().Append(bgSetter);
    flyoutStyle.Setters().Append(marginSetter);
    flyoutStyle.Setters().Append(maxWidthSetter);
    flyoutStyle.Setters().Append(padSetter);
    flyoutStyle.Setters().Append(borSetter);
    flyoutStyle.Setters().Append(cornerSetter);
    flyout.FlyoutPresenterStyle(flyoutStyle);

    try {
        flyout.ShouldConstrainToRootBounds(false);
        flyout.Placement(winrt::Windows::UI::Xaml::Controls::Primitives::
                             FlyoutPlacementMode::Top);
        flyout.AllowFocusOnInteraction(true);  // Allow interactions
        flyout.OverlayInputPassThroughElement(
            winrt::Windows::UI::Xaml::Controls::Primitives::FlyoutBase::
                GetAttachedFlyout(rootGrid));
    } catch (...) {
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

void PopulateForecastUI(winrt::Windows::UI::Xaml::Controls::Grid rootGrid,
                        std::wstring condition,
                        std::wstring currentIcon,
                        std::wstring currentTemp,
                        bool animate,
                        bool animateGraph,
                        std::wstring customPrecip,
                        std::wstring customHumidity,
                        std::wstring customWind) {
    using namespace winrt::Windows::UI::Xaml;
    using namespace winrt::Windows::UI::Xaml::Controls;
    using namespace winrt::Windows::UI::Xaml::Media;
    using namespace winrt::Windows::UI::Xaml::Shapes;

    bool localAcquired;
    std::vector<DailyForecast> localDaily;
    std::vector<HourlyForecast> localHourly;
    std::wstring localWindSpeed, localPrecipProb, localHumidity;

    EnterCriticalSection(&g_forecastLock);
    localDaily = g_forecastDaily;
    localHourly = g_forecastHourly;
    localWindSpeed = g_cachedWindSpeed;
    localPrecipProb = g_cachedPrecipProb;
    localHumidity = g_cachedHumidity;
    localAcquired = g_forecastAcquired;
    LeaveCriticalSection(&g_forecastLock);

    if (!localAcquired) {
        rootGrid.Children().Clear();
        
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
        text.Foreground(SolidColorBrush{IsSystemDarkMode() ? winrt::Windows::UI::Colors::White() : winrt::Windows::UI::Colors::Black()});
        text.HorizontalAlignment(HorizontalAlignment::Center);

        stack.Children().Append(ring);
        stack.Children().Append(text);
        rootGrid.Children().Append(stack);

        winrt::Windows::UI::Color bgColor = IsSystemDarkMode() ? winrt::Windows::UI::Colors::Black() : winrt::Windows::UI::Colors::White();
        bgColor.A = 1; // Prevent fully transparent black bug on Win10 XAML islands
        if (g_useAcrylic) {
            try {
                AcrylicBrush acrylic;
                acrylic.BackgroundSource(AcrylicBackgroundSource::HostBackdrop);
                acrylic.TintColor(bgColor);
                acrylic.TintOpacity(g_acrylicOpacity / 100.0);
                acrylic.FallbackColor(bgColor);
                rootGrid.Background(acrylic);
            } catch (...) {
                rootGrid.Background(SolidColorBrush{bgColor});
            }
        } else {
            rootGrid.Background(SolidColorBrush{bgColor});
        }
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
    auto bgColor =
        isDark ? winrt::Windows::UI::ColorHelper::FromArgb(255, 30, 30, 30)
               : winrt::Windows::UI::ColorHelper::FromArgb(255, 240, 240, 240);
    auto dividerColor =
        isDark ? winrt::Windows::UI::ColorHelper::FromArgb(40, 255, 255, 255)
               : winrt::Windows::UI::ColorHelper::FromArgb(20, 0, 0, 0);
    auto cardBgColor =
        isDark ? winrt::Windows::UI::ColorHelper::FromArgb(30, 255, 255, 255)
               : winrt::Windows::UI::ColorHelper::FromArgb(15, 0, 0, 0);
    auto activeTabColor =
        isDark ? winrt::Windows::UI::ColorHelper::FromArgb(255, 129, 212, 250)
               : winrt::Windows::UI::ColorHelper::FromArgb(255, 0, 120, 215);

    int panelWidth = 336;  // 4 cards: 4 * 70 + 3 * 8 + 32 = 336
    if (g_forecastDaysFetch >= 14)
        panelWidth = 570;  // 7 cards: 7 * 70 + 6 * 8 + 32 = 570
    else if (g_forecastDaysFetch >= 10)
        panelWidth = 492;  // 6 cards: 6 * 70 + 5 * 8 + 32 = 492

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
            } else {
                if (first.try_as<Border>()) {
                    hasBg = true;
                }
            }
        }
    } catch (...) {}

    if (hasBg) {
        // Remove all children except the first one (the background element)
        while (rootGrid.Children().Size() > 1) {
            try {
                auto child = rootGrid.Children().GetAt(1);
                CleanupXamlMedia(child);
            } catch (...) {}
            rootGrid.Children().RemoveAt(1);
        }
    } else {
        try {
            uint32_t size = rootGrid.Children().Size();
            for (uint32_t i = 0; i < size; ++i) {
                CleanupXamlMedia(rootGrid.Children().GetAt(i));
            }
        } catch (...) {}
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
                winrt::Windows::Foundation::Uri uri{g_bgImageVideoUrl};
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
            } else {
                Border imgBorder;
                imgBorder.HorizontalAlignment(HorizontalAlignment::Stretch);
                imgBorder.VerticalAlignment(VerticalAlignment::Stretch);
                if (IsWindows11()) {
                    imgBorder.CornerRadius(CornerRadius{8.0, 8.0, 8.0, 8.0});
                } else {
                    imgBorder.CornerRadius(CornerRadius{0.0, 0.0, 0.0, 0.0});
                }

                ImageBrush imgBrush;
                winrt::Windows::Foundation::Uri uri{g_bgImageVideoUrl};
                winrt::Windows::UI::Xaml::Media::Imaging::BitmapImage bitmap{uri};
                imgBrush.ImageSource(bitmap);
                imgBrush.Stretch(stretchMode);
                imgBrush.Opacity(g_bgImageVideoOpacity / 100.0);

                imgBorder.Background(imgBrush);
                rootGrid.Children().Append(imgBorder);
            }
        } catch (...) {
            // Gracefully ignore error loading background
        }
    }

    double panelHeight = 365.0;
    std::wstring displayWarning = g_activeWarning;
    if (!g_mockWeatherAlert.empty()) {
        displayWarning = g_mockWeatherAlert;
    }
    if (!displayWarning.empty()) {
        panelHeight += 32.0;
    }

    rootGrid.Width(panelWidth);
    rootGrid.MaxWidth(9999.0);
    rootGrid.Height(panelHeight);
    rootGrid.MaxHeight(panelHeight);

    if (IsWindows11()) {
        rootGrid.CornerRadius(CornerRadius{8.0, 8.0, 8.0, 8.0});
        try {
            winrt::Windows::UI::Xaml::Media::RectangleGeometry clipGeo;
            clipGeo.Rect(winrt::Windows::Foundation::Rect{0, 0, (float)panelWidth, (float)panelHeight});
            rootGrid.Clip(clipGeo);
        } catch (...) {}
    } else {
        rootGrid.CornerRadius(CornerRadius{0.0, 0.0, 0.0, 0.0});
        try {
            rootGrid.Clip(nullptr);
        } catch (...) {}
    }

    try {
        if (!g_useAcrylic)
            throw std::exception();

        AcrylicBrush acrylic;
        acrylic.BackgroundSource(AcrylicBackgroundSource::HostBackdrop);
        acrylic.TintColor(bgColor);
        acrylic.TintOpacity(g_acrylicOpacity / 100.0);
        acrylic.FallbackColor(bgColor);
        rootGrid.Background(acrylic);
    } catch (...) {
        rootGrid.Background(SolidColorBrush{bgColor});
    }

    winrt::Windows::UI::Color borderColor;
    borderColor.A = 64; // Increased from 40 for better visibility
    borderColor.R = 128;
    borderColor.G = 128;
    borderColor.B = 128;
    rootGrid.BorderBrush(SolidColorBrush{borderColor});
    rootGrid.BorderThickness(Thickness{1, 1, 1, 1});
    rootGrid.UseLayoutRounding(true);

    StackPanel mainStack;
    mainStack.Orientation(Orientation::Vertical);

    // Optimized static padding and spacing for extreme compactness
    double sidePad = 8.0;
    double vertPad = 4.0;
    mainStack.Padding(Thickness{sidePad, vertPad, sidePad, vertPad});

    double mSpacing = 3.0;
    mainStack.Spacing(mSpacing);

    if (animate) {
        winrt::Windows::UI::Xaml::Media::Animation::TransitionCollection mainTrans;
        winrt::Windows::UI::Xaml::Media::Animation::EntranceThemeTransition mainEnt;
        mainEnt.IsStaggeringEnabled(true);
        mainEnt.FromVerticalOffset(15.0);
        mainTrans.Append(mainEnt);
        mainStack.ChildrenTransitions(mainTrans);
    }

    if (!displayWarning.empty()) {
        Grid warningBar;
        warningBar.CornerRadius(CornerRadius{4, 4, 4, 4});
        warningBar.Padding(Thickness{6, 4, 6, 4});
        warningBar.Margin(Thickness{0, 2, 0, 4});
        
        bool isDark = IsSystemDarkMode();
        winrt::Windows::UI::Color warningBgColor = winrt::Windows::UI::ColorHelper::FromArgb(
            isDark ? 30 : 20, 239, 68, 68);
        winrt::Windows::UI::Color warningBorderColor = winrt::Windows::UI::ColorHelper::FromArgb(
            isDark ? 90 : 70, 239, 68, 68);
        winrt::Windows::UI::Color warningTextColor = winrt::Windows::UI::ColorHelper::FromArgb(
            255, isDark ? 248 : 185, isDark ? 113 : 28, isDark ? 113 : 28);

        warningBar.Background(SolidColorBrush{warningBgColor});
        warningBar.BorderBrush(SolidColorBrush{warningBorderColor});
        warningBar.BorderThickness(Thickness{1, 1, 1, 1});

        StackPanel warningStack;
        warningStack.Orientation(Orientation::Horizontal);
        warningStack.HorizontalAlignment(HorizontalAlignment::Center);
        warningStack.Spacing(6);

        TextBlock warningIcon;
        warningIcon.Text(L"⚠️");
        warningIcon.FontSize(11.5 * g_panelFontScale);
        warningIcon.VerticalAlignment(VerticalAlignment::Center);

        TextBlock warningText;
        warningText.Text(displayWarning);
        warningText.FontSize(11 * g_panelFontScale);
        warningText.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());
        warningText.Foreground(SolidColorBrush{warningTextColor});
        warningText.VerticalAlignment(VerticalAlignment::Center);

        warningStack.Children().Append(warningIcon);
        warningStack.Children().Append(warningText);
        warningBar.Children().Append(warningStack);
        mainStack.Children().Append(warningBar);
    }

    // --- Header Row (Location & Details aligned level)
    Grid headerGrid;
    GridLength h1Len = {1.0, GridUnitType::Star};
    GridLength h2Len = {1.0, GridUnitType::Auto};
    ColumnDefinition hCol1;
    hCol1.Width(h1Len);
    ColumnDefinition hCol2;
    hCol2.Width(h2Len);
    headerGrid.ColumnDefinitions().Append(hCol1);
    headerGrid.ColumnDefinitions().Append(hCol2);

    StackPanel titleStack;
    titleStack.Orientation(Orientation::Horizontal);
    titleStack.Spacing(8);
    titleStack.VerticalAlignment(VerticalAlignment::Top);
    titleStack.Margin(Thickness{6, 2, 0, 0});
    Grid::SetColumn(titleStack, 0);

    TextBlock cityTitle;
    cityTitle.Text(g_displayCity);
    cityTitle.FontSize(20 * g_panelFontScale);
    cityTitle.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());
    cityTitle.Foreground(SolidColorBrush{primaryColor});
    cityTitle.VerticalAlignment(VerticalAlignment::Center);
    cityTitle.Margin(Thickness{0, 0, 0, 0});
    titleStack.Children().Append(cityTitle);

    Button refreshBtn;
    refreshBtn.Content(winrt::box_value(L"\uE72C")); // Refresh Segoe MDL2 icon
    refreshBtn.FontFamily(winrt::Windows::UI::Xaml::Media::FontFamily(L"Segoe MDL2 Assets"));
    refreshBtn.FontSize(12 * g_panelFontScale);
    refreshBtn.Foreground(SolidColorBrush{secondaryColor});
    refreshBtn.Background(SolidColorBrush{winrt::Windows::UI::Colors::Transparent()});
    refreshBtn.BorderThickness(Thickness{0, 0, 0, 0});
    refreshBtn.Padding(Thickness{4, 4, 4, 4});
    refreshBtn.VerticalAlignment(VerticalAlignment::Center);
    refreshBtn.Margin(Thickness{0, 0, 0, 0});
    try {
        refreshBtn.UseSystemFocusVisuals(false);
        refreshBtn.IsTabStop(false);
    } catch (...) {}
    
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
        } catch (...) {}
    });

    titleStack.Children().Append(refreshBtn);
    headerGrid.Children().Append(titleStack);

    StackPanel rightCurrent;
    rightCurrent.Orientation(Orientation::Vertical);
    rightCurrent.HorizontalAlignment(HorizontalAlignment::Right);
    rightCurrent.VerticalAlignment(VerticalAlignment::Top);
    rightCurrent.Spacing(3);
    rightCurrent.Margin(Thickness{0, 2, 0, 0});

    auto MakeSmallText = [secondaryColor](std::wstring text) {
        TextBlock tb;
        tb.Text(text);
        tb.FontSize(12 * g_panelFontScale);
        tb.HorizontalAlignment(HorizontalAlignment::Right);
        tb.Foreground(SolidColorBrush{secondaryColor});
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
    condText.FontSize(13);
    condText.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());
    condText.Foreground(SolidColorBrush{primaryColor});
    condText.HorizontalAlignment(HorizontalAlignment::Right);
    condText.Margin(Thickness{0, 0, 0, 4}); 
    rightCurrent.Children().Append(condText);

    rightCurrent.Children().Append(
        MakeSmallText(L"Precipitation: " + dispPrecip));
    rightCurrent.Children().Append(
        MakeSmallText(L"Humidity: " + dispHumidity));

    // Split wind speed (e.g. "12.0 mph NNW") so wind speed is on one line and the direction (NNW) is on a new line below.
    std::wstring windSpeedOnly = dispWind;
    std::wstring windDirOnly = L"";
    size_t lastSpace = dispWind.find_last_of(L' ');
    if (lastSpace != std::wstring::npos) {
        windSpeedOnly = dispWind.substr(0, lastSpace);
        windDirOnly = dispWind.substr(lastSpace + 1);
    }

    rightCurrent.Children().Append(MakeSmallText(L"Wind: " + windSpeedOnly));
    if (!windDirOnly.empty()) {
        TextBlock dirTb = MakeSmallText(L"Direction: " + windDirOnly);
        dirTb.HorizontalAlignment(HorizontalAlignment::Right);
        dirTb.Margin(Thickness{0, -1, 0, 0});
        rightCurrent.Children().Append(dirTb);
    }

    Grid::SetColumn(rightCurrent, 1);
    headerGrid.Children().Append(rightCurrent);

    mainStack.Children().Append(headerGrid);

    // --- Weather Temp & Icon details (now in leftCurrent horizontal layout)
    StackPanel leftCurrent;
    leftCurrent.Orientation(Orientation::Vertical);
    leftCurrent.Spacing(1);
    leftCurrent.Margin(Thickness{4, -46, 0, -12}); // Move up significantly more to tighten the vertical space

    // A row for the Icon and the Temperature side-by-side
    StackPanel topWeatherRow;
    topWeatherRow.Orientation(Orientation::Horizontal);
    topWeatherRow.Spacing(10);
    topWeatherRow.VerticalAlignment(VerticalAlignment::Center);

    if (g_weatherStyle == 1) {
        FontIcon bigIcon;
        bigIcon.FontFamily(
            winrt::Windows::UI::Xaml::Media::FontFamily(L"Segoe UI Symbol"));
        bigIcon.Glyph(currentIcon);
        bigIcon.FontSize(42 * g_panelFontScale);
        bigIcon.Foreground(
            SolidColorBrush{GetXamlIconColor(condition)});
        bigIcon.VerticalAlignment(VerticalAlignment::Center);
        bigIcon.Margin(Thickness{0, -4, 0, 0});
        topWeatherRow.Children().Append(bigIcon);
    } else {
        TextBlock bigIcon;
        bigIcon.Text(currentIcon);
        bigIcon.FontSize(48 * g_panelFontScale);        
        bigIcon.Margin(Thickness{-2, -18, 0, 0});
        bigIcon.VerticalAlignment(VerticalAlignment::Center);
        topWeatherRow.Children().Append(bigIcon);
    }

    TextBlock bigTemp;
    std::wstring displayTemp = currentTemp;
    if (!displayTemp.empty() && displayTemp.back() == L'°') {
        displayTemp += (g_useCelsius ? L"C" : L"F");
    }
    bigTemp.Text(EnforceOneDecimalTemp(displayTemp));
    bigTemp.FontSize(32 * g_panelFontScale);
    bigTemp.FontWeight(winrt::Windows::UI::Text::FontWeights::Bold());
    bigTemp.Foreground(SolidColorBrush{primaryColor});
    bigTemp.VerticalAlignment(VerticalAlignment::Center);
    bigTemp.Margin(Thickness{0, -14, 0, 0}); // move temperature text up a few pixels
    topWeatherRow.Children().Append(bigTemp);

    leftCurrent.Children().Append(topWeatherRow);

    try {
        bigTemp.Transitions(winrt::Windows::UI::Xaml::Media::Animation::TransitionCollection{});
        topWeatherRow.Transitions(winrt::Windows::UI::Xaml::Media::Animation::TransitionCollection{});
        leftCurrent.Transitions(winrt::Windows::UI::Xaml::Media::Animation::TransitionCollection{});
    } catch (...) {}

    mainStack.Children().Append(leftCurrent);

    auto MakeTabBtn = [rootGrid, condition, currentIcon, currentTemp,
                       primaryColor, tertiaryColor, activeTabColor, animate](
                          std::wstring text, int tabIndex, bool isSelected) {
        Grid tabGrid;
        tabGrid.Padding(Thickness{0, 0, 0, 0});
        tabGrid.Background(SolidColorBrush{winrt::Windows::UI::Colors::Transparent()});

        try {
            if (animate) {
                winrt::Windows::UI::Xaml::Media::Animation::TransitionCollection tabGridTransitions;
                winrt::Windows::UI::Xaml::Media::Animation::AddDeleteThemeTransition addDelete;
                tabGridTransitions.Append(addDelete);
                tabGrid.ChildrenTransitions(tabGridTransitions);
            }
        } catch (...) {
        }

        TextBlock tb;
        tb.Text(text);
        tb.FontSize(13 * g_panelFontScale);
        tb.Margin(Thickness{0, 4.0, 0, 8.0});
        tb.FontWeight(isSelected
                          ? winrt::Windows::UI::Text::FontWeights::SemiBold()
                          : winrt::Windows::UI::Text::FontWeights::Normal());
        tb.Foreground(
            SolidColorBrush{isSelected ? primaryColor : tertiaryColor});
        tb.HorizontalAlignment(HorizontalAlignment::Center);
        tabGrid.Children().Append(tb);

        if (isSelected) {
            winrt::Windows::UI::Xaml::Shapes::Rectangle underline;
            underline.Height(1.0);
            underline.Fill(SolidColorBrush{activeTabColor});
            underline.VerticalAlignment(VerticalAlignment::Bottom);
            underline.Margin(Thickness{0, 0, 0, 5.0}); // Raised up from bottom
            tabGrid.Children().Append(underline);
        }

        tabGrid.PointerEntered([tb, isSelected, activeTabColor](auto const&, auto const&) {
            if (!isSelected) {
                tb.Foreground(SolidColorBrush{activeTabColor});
            }
        });
        tabGrid.PointerExited([tb, isSelected, tertiaryColor](auto const&, auto const&) {
            if (!isSelected) {
                tb.Foreground(SolidColorBrush{tertiaryColor});
            }
        });

        auto weakRoot = winrt::make_weak(rootGrid);
        tabGrid.Tapped([weakRoot, tabIndex, condition, currentIcon, currentTemp](auto const&, auto const&) {
            if (g_selectedGraphTab != tabIndex) {
                g_selectedGraphTab = tabIndex;
                if (auto root = weakRoot.get()) {
                    PopulateForecastUI(root, condition, currentIcon,
                                       currentTemp, false);
                }
            }
        });

        return tabGrid;
    };

    StackPanel graphSection;
    graphSection.Orientation(Orientation::Vertical);
    graphSection.Spacing(0);

    Grid tabHeaderGrid;
    tabHeaderGrid.Margin(Thickness{0, 2.0, 0, 3.0});

    winrt::Windows::UI::Xaml::Shapes::Rectangle div1;
    div1.Height(1);
    div1.Fill(SolidColorBrush{dividerColor});
    div1.VerticalAlignment(VerticalAlignment::Bottom);
    div1.Margin(Thickness{0, 0, 0, 0});
    tabHeaderGrid.Children().Append(div1);

    StackPanel tabs;
    tabs.Orientation(Orientation::Horizontal);
    tabs.Spacing(24);
    tabs.Margin(Thickness{8.0, 0, 0, 2.0});
    tabs.VerticalAlignment(VerticalAlignment::Bottom);
    tabs.Children().Append(
        MakeTabBtn(L"Temperature", 0, g_selectedGraphTab == 0));
    tabs.Children().Append(
        MakeTabBtn(L"Precipitation", 1, g_selectedGraphTab == 1));
    tabs.Children().Append(MakeTabBtn(L"Wind", 2, g_selectedGraphTab == 2));
    tabHeaderGrid.Children().Append(tabs);

    graphSection.Children().Append(tabHeaderGrid);

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
            double v = (g_selectedGraphTab == 0)   ? h.tempRaw
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
        if (maxVal == minVal) {
            maxVal += 1;
            minVal -= 1;
        }
        double range = maxVal - minVal;

        const double graphHeight = 60.0;
        const double graphOuterWidth = std::max(280.0, rootGrid.Width() - 32.0);

            double paddingTB =
            (g_density == 2) ? 4.0 : ((g_density == 1) ? 9.0 : 12.0);

        // We make columns 55.0px wide, and display ALL points seamlessly!
        double colWidth = 55.0;
        double canvasWidth = std::max(graphOuterWidth, colWidth * (double)std::max((size_t)0, currentDayHourly.size() - 1) + 40.0);

        Grid graphContainer;
        graphContainer.Background(SolidColorBrush{cardBgColor});
        graphContainer.CornerRadius(CornerRadius{6.0, 6.0, 6.0, 6.0});
        graphContainer.Padding(
            Thickness{0.0, paddingTB, 0.0, paddingTB});

        // Setup smooth tab/day click fade animation transitions
        if (animateGraph) {
            winrt::Windows::UI::Xaml::Media::Animation::TransitionCollection
                graphTransitions;
            winrt::Windows::UI::Xaml::Media::Animation::ContentThemeTransition
                graphTransition;
            try {
                graphTransitions.Append(graphTransition);
                graphContainer.ChildrenTransitions(graphTransitions);
            } catch (...) {
            }
        }

        Canvas canvas;
        canvas.Height(graphHeight + 34.0);
        canvas.Width(canvasWidth);

        // Draw 3 horizontal dashed guidelines to provide separating visual context
        double gridYLines[] = {18.0, graphHeight / 2.0 + 18.0,
                               graphHeight + 18.0};
        for (double ly : gridYLines) {
            winrt::Windows::UI::Xaml::Shapes::Line gl;
            gl.X1(0.0);
            gl.Y1(ly);
            gl.X2(canvasWidth);
            gl.Y2(ly);
            gl.Stroke(SolidColorBrush{dividerColor});
            gl.StrokeThickness(0.75);

            try {
                winrt::Windows::UI::Xaml::Media::DoubleCollection strokeDash;
                strokeDash.Append(4.0);
                strokeDash.Append(4.0);
                gl.StrokeDashArray(strokeDash);
            } catch (...) {
            }

            canvas.Children().Append(gl);
        }

        winrt::Windows::UI::Xaml::Shapes::Polyline poly;
        poly.Stroke(SolidColorBrush{winrt::Windows::UI::ColorHelper::FromArgb(
            242, 250, 214, 53)});  // Yellow
        if (g_selectedGraphTab == 1)
            poly.Stroke(SolidColorBrush{activeTabColor});  // Blue for precip
        if (g_selectedGraphTab == 2)
            poly.Stroke(
                SolidColorBrush{winrt::Windows::UI::ColorHelper::FromArgb(
                    42, 200, 200, 200)});  // Gray for wind
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
            double v = (g_selectedGraphTab == 0)   ? h.tempRaw
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
                winrt::Windows::Foundation::Point{(float)x, (float)y + 18.0f});

            // Value Label
            TextBlock valTb;
            wchar_t vBuf[64];
            if (g_selectedGraphTab == 0) {
                swprintf(vBuf, L"%.0f°", v);
            } else if (g_selectedGraphTab == 1) {
                swprintf(vBuf, L"%.0f%%", v);
            } else {
                if (g_useCelsius) {
                    swprintf(vBuf, L"%.1f\nkm/h", v);
                } else {
                    swprintf(vBuf, L"%.1f\nmph", v);
                }
            }
            valTb.Text(vBuf);
            valTb.FontSize(11 * g_panelFontScale);
            valTb.Foreground(SolidColorBrush{secondaryColor});
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
            Canvas::SetTop(valTb, y + 18.0 - valYOffset);
            canvas.Children().Append(valTb);
            valTbs.push_back(valTb);

            // Time Label
            TextBlock timeTb;
            timeTb.Text(h.timeString);
            timeTb.FontSize(10 * g_panelFontScale);
            timeTb.Foreground(SolidColorBrush{tertiaryColor});
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
            polygonPoints.Append(winrt::Windows::Foundation::Point{lastPt.X, (float)graphHeight + 18.0f});
            polygonPoints.Append(winrt::Windows::Foundation::Point{firstPt.X, (float)graphHeight + 18.0f});
        }
        shadedArea.Points(polygonPoints);

        winrt::Windows::UI::Xaml::Media::LinearGradientBrush gradient;
        gradient.StartPoint(winrt::Windows::Foundation::Point{0.0f, 0.0f});
        gradient.EndPoint(winrt::Windows::Foundation::Point{0.0f, 1.0f});

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
                hoverCol.Background(SolidColorBrush{winrt::Windows::UI::ColorHelper::FromArgb(30, activeTabColor.R, activeTabColor.G, activeTabColor.B)});
                hoverCol.BorderBrush(SolidColorBrush{winrt::Windows::UI::ColorHelper::FromArgb(80, activeTabColor.R, activeTabColor.G, activeTabColor.B)});
                hoverCol.BorderThickness(Thickness{1, 1, 1, 1});
            } else {
                hoverCol.Background(SolidColorBrush{winrt::Windows::UI::Colors::Transparent()});
                hoverCol.BorderThickness(Thickness{0, 0, 0, 0});
            }
            hoverCol.CornerRadius(winrt::Windows::UI::Xaml::CornerRadius{4.0, 4.0, 4.0, 4.0});
            Canvas::SetLeft(hoverCol, x - (hoverCol.Width() / 2.0));
            Canvas::SetTop(hoverCol, 0);

            TextBlock valTb = valTbs[i];
            TextBlock timeTb = timeTbs[i];

            if (isSelectedHour) {
                valTb.Foreground(SolidColorBrush{primaryColor});
                timeTb.Foreground(SolidColorBrush{primaryColor});
                valTb.FontWeight(winrt::Windows::UI::Text::FontWeights::Bold());
                timeTb.FontWeight(winrt::Windows::UI::Text::FontWeights::Bold());
            }

            hoverCol.PointerEntered([valTb, timeTb, isSelectedHour, activeTabColor](auto const& sender, auto const&) {
                valTb.Foreground(SolidColorBrush{activeTabColor});
                timeTb.Foreground(SolidColorBrush{activeTabColor});
                if (!isSelectedHour) {
                    if (auto col = sender.template try_as<Grid>()) {
                        col.Background(SolidColorBrush{winrt::Windows::UI::ColorHelper::FromArgb(20, activeTabColor.R, activeTabColor.G, activeTabColor.B)});
                        col.BorderBrush(SolidColorBrush{winrt::Windows::UI::ColorHelper::FromArgb(40, activeTabColor.R, activeTabColor.G, activeTabColor.B)});
                        col.BorderThickness(Thickness{1, 1, 1, 1});
                    }
                }
            });
            hoverCol.PointerExited([valTb, timeTb, isSelectedHour, primaryColor, secondaryColor, tertiaryColor, activeTabColor](auto const& sender, auto const&) {
                if (isSelectedHour) {
                    valTb.Foreground(SolidColorBrush{primaryColor});
                    timeTb.Foreground(SolidColorBrush{primaryColor});
                    valTb.FontWeight(winrt::Windows::UI::Text::FontWeights::Bold());
                    timeTb.FontWeight(winrt::Windows::UI::Text::FontWeights::Bold());
                    if (auto col = sender.template try_as<Grid>()) {
                        col.Background(SolidColorBrush{winrt::Windows::UI::ColorHelper::FromArgb(30, activeTabColor.R, activeTabColor.G, activeTabColor.B)});
                        col.BorderBrush(SolidColorBrush{winrt::Windows::UI::ColorHelper::FromArgb(80, activeTabColor.R, activeTabColor.G, activeTabColor.B)});
                        col.BorderThickness(Thickness{1, 1, 1, 1});
                    }
                } else {
                    valTb.Foreground(SolidColorBrush{secondaryColor});
                    timeTb.Foreground(SolidColorBrush{tertiaryColor});
                    if (auto col = sender.template try_as<Grid>()) {
                        col.Background(SolidColorBrush{winrt::Windows::UI::Colors::Transparent()});
                        col.BorderThickness(Thickness{0, 0, 0, 0});
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
            hoverCol.Tapped([weakRoot, h_timeString, h_conditionName, h_icon, h_temp, h_precipProb, h_humidity, h_windSpeed](auto const&, auto const&) {
                g_selectedHour = h_timeString;
                if (auto root = weakRoot.get()) {
                    PopulateForecastUI(root, h_conditionName, h_icon, h_temp, false, false, h_precipProb, h_humidity, h_windSpeed);
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

    winrt::Windows::UI::Xaml::Shapes::Rectangle div2;
    div2.Height(1);
    div2.Fill(SolidColorBrush{dividerColor});
    mainStack.Children().Append(div2);

    // --- Daily Forecast (Horizontal Scroll or Grid)
    if (!localDaily.empty()) {
        ScrollViewer dailyScroll;
        dailyScroll.HorizontalScrollBarVisibility(ScrollBarVisibility::Hidden);
        dailyScroll.VerticalScrollBarVisibility(ScrollBarVisibility::Disabled);
        dailyScroll.HorizontalScrollMode(ScrollMode::Enabled);
        dailyScroll.VerticalScrollMode(ScrollMode::Disabled);
        dailyScroll.Margin(Thickness{3, 0, 0, 0});
        
        if (g_dailyScrollOffset > 0.0) {
            dailyScroll.ChangeView(g_dailyScrollOffset, nullptr, nullptr, true);
            dailyScroll.Loaded([=](auto const&, auto const&) {
                dailyScroll.ChangeView(g_dailyScrollOffset, nullptr, nullptr, true);
            });
        }

        dailyScroll.ViewChanged([=](auto const& sender, auto const&) {
            auto sv = sender.template try_as<ScrollViewer>();
            if (sv) {
                g_dailyScrollOffset = sv.HorizontalOffset();
            }
        });

        StackPanel dailyStack;
        dailyStack.Orientation(Orientation::Horizontal);
        dailyStack.Spacing(8);
        size_t maxDailyToShow = localDaily.size();
        if (localDaily.size() <= 7) {
            dailyStack.HorizontalAlignment(HorizontalAlignment::Center);
            dailyStack.Margin(Thickness{0, 0, 0, 0});
        } else {
            dailyStack.HorizontalAlignment(HorizontalAlignment::Left);
            dailyStack.Margin(Thickness{3, 0, 0, 0});
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
            dayCard.CornerRadius(CornerRadius{8, 8, 8, 8});
            dayCard.Padding(Thickness{4, 5, 4, 5});
            dayCard.Width(70.0); /* Force exact width for predictable layouts */
            dayCard.Background(SolidColorBrush{
                isSel ? cardBgColor
                      : winrt::Windows::UI::Colors::Transparent()});

            std::wstring dayNum;
            if (d.rawDate.length() >= 10) {
                dayNum = d.rawDate.substr(8, 2);
                if (dayNum.length() > 0 && dayNum[0] == L'0') dayNum = dayNum.substr(1);
            }

            StackPanel dayHeader;
            dayHeader.Orientation(Orientation::Horizontal);
            dayHeader.Spacing(i == 0 ? 3 : 6);
            dayHeader.HorizontalAlignment(HorizontalAlignment::Center);

            TextBlock tDay;
            tDay.Text(i == 0 ? L"Today" : d.dayName);
            tDay.FontSize(11 * g_panelFontScale);
            tDay.Foreground(SolidColorBrush{primaryColor});
            tDay.VerticalAlignment(VerticalAlignment::Center);
            dayHeader.Children().Append(tDay);

            if (!dayNum.empty()) {
                Border dateBorder;
                dateBorder.CornerRadius(CornerRadius{4, 4, 4, 4});
                dateBorder.Background(SolidColorBrush{winrt::Windows::UI::ColorHelper::FromArgb(30, 128, 128, 128)});
                dateBorder.Padding(Thickness{4, 1, 4, 1});
                dateBorder.VerticalAlignment(VerticalAlignment::Center);
                dateBorder.Margin(Thickness{0, 0, 0, 0});

                TextBlock tDate;
                tDate.Text(dayNum);
                tDate.FontSize(10 * g_panelFontScale);
                tDate.Foreground(SolidColorBrush{tertiaryColor});
                tDate.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());
                tDate.VerticalAlignment(VerticalAlignment::Center);
                
                dateBorder.Child(tDate);
                dayHeader.Children().Append(dateBorder);
            }

            Border headerBorder;
            headerBorder.CornerRadius(CornerRadius{4, 4, 4, 4});
            winrt::Windows::UI::Color bgCol = isDark ? winrt::Windows::UI::ColorHelper::FromArgb(40, 255, 255, 255) : winrt::Windows::UI::ColorHelper::FromArgb(22, 0, 0, 0);
            winrt::Windows::UI::Color borderCol = isDark ? winrt::Windows::UI::ColorHelper::FromArgb(60, 255, 255, 255) : winrt::Windows::UI::ColorHelper::FromArgb(35, 0, 0, 0);
            headerBorder.Background(SolidColorBrush{bgCol});
            headerBorder.BorderBrush(SolidColorBrush{borderCol});
            headerBorder.BorderThickness(Thickness{1, 1, 1, 1});
            headerBorder.Padding(Thickness{4, 2, 4, 2});
            headerBorder.HorizontalAlignment(HorizontalAlignment::Center);
            headerBorder.Child(dayHeader);

            StackPanel dayCol;
            dayCol.Orientation(Orientation::Vertical);
            dayCol.Spacing(4);
            dayCol.HorizontalAlignment(HorizontalAlignment::Center);
            dayCol.Children().Append(headerBorder);

            if (g_weatherStyle == 1) {
                FontIcon hd;
                hd.FontFamily(winrt::Windows::UI::Xaml::Media::FontFamily(
                    L"Segoe UI Symbol"));
                hd.Glyph(d.icon);
                hd.FontSize(25 * g_panelFontScale);
                hd.Foreground(
                    SolidColorBrush{GetXamlIconColor(d.condition)});
                hd.HorizontalAlignment(HorizontalAlignment::Center);
                dayCol.Children().Append(hd);
            } else {
                TextBlock hd;
                hd.Text(d.icon);
                hd.FontSize(32 * g_panelFontScale);
                hd.HorizontalAlignment(HorizontalAlignment::Center);
                dayCol.Children().Append(hd);
            }

            StackPanel minmax;
            minmax.Orientation(Orientation::Horizontal);
            minmax.Spacing(4);
            minmax.HorizontalAlignment(HorizontalAlignment::Center);

            std::wstring cleanMax = d.tempMax;
            size_t mpos = cleanMax.find(L"°");
            if (mpos != std::wstring::npos)
                cleanMax = cleanMax.substr(0, mpos + 1);
            TextBlock tMax;
            tMax.Text(cleanMax);
            tMax.FontSize(12 * g_panelFontScale);
            tMax.Foreground(SolidColorBrush{primaryColor});
            tMax.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());

            std::wstring cleanMin = d.tempMin;
            mpos = cleanMin.find(L"°");
            if (mpos != std::wstring::npos)
                cleanMin = cleanMin.substr(0, mpos + 1);
            TextBlock tMin;
            tMin.Text(cleanMin);
            tMin.FontSize(12 * g_panelFontScale);
            tMin.Foreground(SolidColorBrush{secondaryColor});

            minmax.Children().Append(tMax);
            minmax.Children().Append(tMin);
            dayCol.Children().Append(minmax);

            dayCard.Children().Append(dayCol);

            std::wstring dDate = d.rawDate;
            std::wstring dCond = d.condition;
            std::wstring dIcon = d.icon;
            std::wstring dTemp = d.tempMax;

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
            dayCard.Tapped([weakRoot, dDate, dCond, dIcon, dTemp, dPrecip, dHumidity, dWind](auto const&, auto const&) {
                if (g_selectedDate != dDate) {
                    g_selectedDate = dDate;
                    g_graphScrollOffset = -1.0;
                    if (auto root = weakRoot.get()) {
                        PopulateForecastUI(root, dCond, dIcon,
                                           dTemp, false, false,
                                           dPrecip, dHumidity, dWind);
                    }
                }
            });

            dayCard.PointerEntered([dDate](auto const& sender, auto const&) {
                if (g_selectedDate != dDate) {
                    if (auto card = sender.template try_as<Grid>()) {
                        card.Background(SolidColorBrush{winrt::Windows::UI::ColorHelper::FromArgb(25, 128, 128, 128)});
                    }
                }
            });
            dayCard.PointerExited([dDate](auto const& sender, auto const&) {
                if (g_selectedDate != dDate) {
                    if (auto card = sender.template try_as<Grid>()) {
                        card.Background(SolidColorBrush{winrt::Windows::UI::Colors::Transparent()});
                    }
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

    rootGrid.Children().Append(mainStack);
}
// Programmatically modify the XAML visual children in place
void UpdateWeatherXamlElements(Grid weatherGrid,
                               std::wstring temp,
                               std::wstring icon,
                               std::wstring condition,
                               bool acquired) {
    if (!weatherGrid)
        return;
    try {
        UpdateInjectedWeatherLayout(weatherGrid);

        weatherGrid.Children().Clear();

        Button buttonElement;

        // Remove standard Windows borders and establish padding
        winrt::Windows::UI::Color transColor;
        transColor.A = 0;
        transColor.R = 0;
        transColor.G = 0;
        transColor.B = 0;
        buttonElement.Background(SolidColorBrush{transColor});
        buttonElement.BorderBrush(nullptr);
        buttonElement.BorderThickness(Thickness{0});

        bool isHorizontal = true;
        HWND hAnchor = FindSystemAnchorWnd();
        if (hAnchor) {
            HWND hParentTaskbar = GetAncestor(hAnchor, GA_ROOT);
            if (!hParentTaskbar)
                hParentTaskbar = FindWindowW(L"Shell_TrayWnd", NULL);
            if (hParentTaskbar) {
                RECT trayRect;
                GetWindowRect(hParentTaskbar, &trayRect);
                isHorizontal = (trayRect.right - trayRect.left) > (trayRect.bottom - trayRect.top);
            }
        }

        // Scale padding by density setting
        double padLeftRight =
            (g_density == 2) ? 8.0 : ((g_density == 1) ? 10.0 : 12.0);
        double padTopBottom =
            (g_density == 2) ? 1.0 : ((g_density == 1) ? 1.5 : 2.0);
        buttonElement.Padding(
            isHorizontal ? Thickness{padLeftRight, padTopBottom, padLeftRight, padTopBottom}
                         : Thickness{padTopBottom, padLeftRight, padTopBottom, padLeftRight});
        buttonElement.VerticalAlignment(VerticalAlignment::Stretch);
        buttonElement.HorizontalAlignment(HorizontalAlignment::Stretch);

        try {
            buttonElement.CornerRadius(CornerRadius{4.0, 4.0, 4.0, 4.0});
            buttonElement.UseSystemFocusVisuals(false);
        } catch (...) {
        }

        ToolTip toolTip;
        toolTip.Content(winrt::box_value(g_displayCity + L"\n" + condition +
                                         L", " + temp +
                                         L"\nClick for full forecast"));
        ToolTipService::SetToolTip(buttonElement, toolTip);

        StackPanel stackPanel;
        stackPanel.Orientation(isHorizontal ? Orientation::Horizontal : Orientation::Vertical);

        // Scale stackPanel spacing by density
        double spacingVal =
            (g_density == 2) ? 3.0 : ((g_density == 1) ? 4.5 : 6.0);
        stackPanel.Spacing(isHorizontal ? spacingVal : 1.0);
        stackPanel.VerticalAlignment(VerticalAlignment::Center);
        stackPanel.HorizontalAlignment(isHorizontal ? HorizontalAlignment::Left : HorizontalAlignment::Center);

        winrt::Windows::UI::Color xamlColor;
        xamlColor.A = 255;
        xamlColor.R = GetRValue(g_textColor);
        xamlColor.G = GetGValue(g_textColor);
        xamlColor.B = GetBValue(g_textColor);
        SolidColorBrush foregroundBrush{xamlColor};

        if (g_weatherStyle == 1) {  // Segoe PUA or Segoe MDL2 Assets
            FontIcon iconBlock;
            iconBlock.FontFamily(winrt::Windows::UI::Xaml::Media::FontFamily(
                L"Segoe UI Symbol"));
            iconBlock.Glyph(icon);
            iconBlock.FontSize((double)g_iconFontSize);
            iconBlock.Foreground(foregroundBrush);
            iconBlock.VerticalAlignment(VerticalAlignment::Center);
            iconBlock.HorizontalAlignment(HorizontalAlignment::Center);
            if (isHorizontal) {
                iconBlock.Margin(winrt::Windows::UI::Xaml::Thickness{-6.0, 0, 0, 0});
            } else {
                iconBlock.Margin(winrt::Windows::UI::Xaml::Thickness{0, 0, 0, 0});
            }
            stackPanel.Children().Append(iconBlock);
        } else {  // Emojis
            TextBlock iconBlock;
            iconBlock.Text(icon);
            iconBlock.FontSize((double)g_iconFontSize);
            iconBlock.VerticalAlignment(VerticalAlignment::Center);
            iconBlock.HorizontalAlignment(HorizontalAlignment::Center);
            stackPanel.Children().Append(iconBlock);
        }

        StackPanel textContainer;
        textContainer.Orientation(Orientation::Vertical);
        textContainer.VerticalAlignment(VerticalAlignment::Center);
        textContainer.HorizontalAlignment(isHorizontal ? HorizontalAlignment::Left : HorizontalAlignment::Center);
        textContainer.Spacing(0.0);

        TextBlock tempBlock;
        if (!g_line1FontFamily.empty()) {
            tempBlock.FontFamily(
                winrt::Windows::UI::Xaml::Media::FontFamily(g_line1FontFamily));
        }
        tempBlock.FontSize((double)g_line1FontSize);
        tempBlock.FontWeight(
            g_line1Bold ? winrt::Windows::UI::Text::FontWeights::Bold()
                        : winrt::Windows::UI::Text::FontWeights::Normal());
        tempBlock.Foreground(foregroundBrush);

        TextBlock condBlock;
        if (!g_line2FontFamily.empty()) {
            condBlock.FontFamily(
                winrt::Windows::UI::Xaml::Media::FontFamily(g_line2FontFamily));
        }
        condBlock.FontSize((double)g_line2FontSize);
        condBlock.FontWeight(
            g_line2Bold ? winrt::Windows::UI::Text::FontWeights::Bold()
                        : winrt::Windows::UI::Text::FontWeights::Normal());
        try {
            condBlock.Opacity(0.85);
        } catch (...) {
        }
        condBlock.Foreground(foregroundBrush);

        if (!isHorizontal) {
            tempBlock.HorizontalAlignment(HorizontalAlignment::Center);
            tempBlock.TextAlignment(winrt::Windows::UI::Xaml::TextAlignment::Center);
            condBlock.HorizontalAlignment(HorizontalAlignment::Center);
            condBlock.TextAlignment(winrt::Windows::UI::Xaml::TextAlignment::Center);
        }

        if (acquired) {
            tempBlock.Text(temp);
            condBlock.Text(g_showConditionName ? condition : L"");
        } else {
            tempBlock.Text(L"Weather");
            condBlock.Text(L"Loading...");
        }

        textContainer.Children().Append(tempBlock);
        if (isHorizontal && (g_showConditionName || !acquired)) {
            textContainer.Children().Append(condBlock);
        }

        stackPanel.Children().Append(textContainer);
        buttonElement.Content(stackPanel);
        g_weakXamlWeatherButton = winrt::make_weak(buttonElement);

       if (acquired) {
            Flyout flyout = CreateForecastFlyout(condition, icon, temp);
            g_showWin11Flyout = [flyout, buttonElement, stackPanel, condition, icon, temp]() mutable {
                try {
                    if (g_win11FlyoutIsOpen) {
                        if (g_activeFlyout) {
                            g_activeFlyout.Hide();
                        } else {
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
                    auto rootGrid = flyout.Content().try_as<winrt::Windows::UI::Xaml::Controls::Grid>();
                    if (rootGrid) {
                        PopulateForecastUI(rootGrid, condition, icon, temp);
                    }
                    g_activeFlyout = flyout;
                } catch (...) {}
                try {
                    winrt::Windows::UI::Xaml::Controls::Primitives::FlyoutShowOptions showOptions;
                    
                    auto placement = winrt::Windows::UI::Xaml::Controls::Primitives::FlyoutPlacementMode::Top;
                    float xOffset = 0.0f;
                    float yOffset = -24.0f;

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

                    double contentHeight = buttonElement.ActualHeight();
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
                             MONITORINFO mi = {sizeof(mi)};
                             if (GetMonitorInfoW(hMon, &mi)) {
                                 int trayWidth = trayRect.right - trayRect.left;
                                 int trayHeight = trayRect.bottom - trayRect.top;
                                 bool isHorizontal = trayWidth > trayHeight;

                                 if (isHorizontal) {
                                     if (trayRect.top <= mi.rcMonitor.top + (mi.rcMonitor.bottom - mi.rcMonitor.top) / 2) {
                                         placement = winrt::Windows::UI::Xaml::Controls::Primitives::FlyoutPlacementMode::Bottom;
                                         yOffset = (float)contentHeight + 24.0f;
                                     } else {
                                         placement = winrt::Windows::UI::Xaml::Controls::Primitives::FlyoutPlacementMode::Top;
                                         yOffset = -24.0f;
                                     }
                                     xOffset = (float)(contentWidth / 2.0);
                                 } else {
                                     if (trayRect.left <= mi.rcMonitor.left + (mi.rcMonitor.right - mi.rcMonitor.left) / 2) {
                                         placement = winrt::Windows::UI::Xaml::Controls::Primitives::FlyoutPlacementMode::Right;
                                         xOffset = (float)contentWidth + 24.0f;
                                         yOffset = (float)(contentHeight / 2.0);
                                     } else {
                                         placement = winrt::Windows::UI::Xaml::Controls::Primitives::FlyoutPlacementMode::Left;
                                         xOffset = -24.0f;
                                         yOffset = (float)(contentHeight / 2.0);
                                     }
                                 }
                             }
                        }
                    }
                    flyout.Placement(placement);
                    showOptions.Placement(placement);
                    showOptions.Position(winrt::Windows::Foundation::Point{xOffset, yOffset});
                    flyout.ShowAt(buttonElement, showOptions);
                } catch (...) {
                    flyout.ShowAt(buttonElement);
                }
            };
            buttonElement.Click([](auto const&, auto const&) {
                if (g_showWin11Flyout) {
                    g_showWin11Flyout();
                }
            });
        }


        weatherGrid.Children().Append(buttonElement);
    } catch (...) {
    }
}

// Dynamic WinHttp pull system
std::wstring RequestHttpData(const std::wstring& host,
                             const std::wstring& path,
                             bool secure) {
    std::wstring response;
    HINTERNET hSession =
        WinHttpOpen(L"EP_WeatherHost/2.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                    WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession)
        return L"";

    WinHttpSetTimeouts(hSession, 3000, 3000, 4000, 4000);

    INTERNET_PORT port =
        secure ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;
    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), port, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return L"";
    }

    DWORD flags = secure ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET hRequest = WinHttpOpenRequest(
        hConnect, L"GET", path.c_str(), NULL, WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return L"";
    }

    BOOL bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS,
                                       0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);

    if (bResults) {
        bResults = WinHttpReceiveResponse(hRequest, NULL);
    }

    if (bResults) {
        DWORD dwSize = 0;
        do {
            if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
                break;
            if (dwSize == 0)
                break;

            std::string tempBuffer;
            tempBuffer.resize(dwSize);
            DWORD dwDownloaded = 0;
            if (WinHttpReadData(hRequest, &tempBuffer[0], dwSize,
                                &dwDownloaded)) {
                tempBuffer.resize(dwDownloaded);
                int wlen = MultiByteToWideChar(CP_UTF8, 0, tempBuffer.c_str(),
                                               -1, NULL, 0);
                if (wlen > 0) {
                    std::wstring wtemp(wlen, 0);
                    MultiByteToWideChar(CP_UTF8, 0, tempBuffer.c_str(), -1,
                                        &wtemp[0], wlen);
                    if (!wtemp.empty() && wtemp.back() == L'\0') {
                        wtemp.pop_back();
                    }
                    response += wtemp;
                }
            }
        } while (dwSize > 0);
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return response;
}

// JSON Block parser
std::wstring ExtractJSONObject(const std::wstring& json,
                               const std::wstring& key) {
    size_t pos = json.find(L"\"" + key + L"\"");
    if (pos == std::wstring::npos)
        return L"";

    pos = json.find(L"{", pos);
    if (pos == std::wstring::npos)
        return L"";

    int braceCount = 1;
    size_t endPos = pos + 1;
    while (endPos < json.length() && braceCount > 0) {
        if (json[endPos] == L'{') {
            braceCount++;
        } else if (json[endPos] == L'}') {
            braceCount--;
        }
        endPos++;
    }
    if (braceCount == 0) {
        return json.substr(pos, endPos - pos);
    }
    return L"";
}

// Extract JSON Value properties
std::wstring ExtractJSONValue(const std::wstring& json,
                              const std::wstring& key,
                              wchar_t endChar = L',') {
    size_t pos = json.find(L"\"" + key + L"\"");
    if (pos == std::wstring::npos) {
        pos = json.find(key);
        if (pos == std::wstring::npos)
            return L"";
    }

    pos = json.find(L":", pos);
    if (pos == std::wstring::npos)
        return L"";
    pos++;

    while (pos < json.length() &&
           (json[pos] == L' ' || json[pos] == L'"' || json[pos] == L'\t')) {
        pos++;
    }

    size_t endPos = pos;
    while (endPos < json.length() && json[endPos] != endChar &&
           json[endPos] != L'}' && json[endPos] != L']') {
        if (json[endPos] == L'"')
            break;
        endPos++;
    }

    std::wstring val = json.substr(pos, endPos - pos);
    while (!val.empty() && (val.back() == L' ' || val.back() == L'"' ||
                            val.back() == L'\r' || val.back() == L'\n')) {
        val.pop_back();
    }
    return val;
}

// Extract JSON Array
std::wstring ExtractJSONArray(const std::wstring& json,
                              const std::wstring& key) {
    size_t pos = json.find(L"\"" + key + L"\"");
    if (pos == std::wstring::npos)
        return L"";

    pos = json.find(L"[", pos);
    if (pos == std::wstring::npos)
        return L"";

    size_t endPos = json.find(L"]", pos);
    if (endPos == std::wstring::npos)
        return L"";

    return json.substr(pos, endPos - pos + 1);
}

// Tokenize standard bracket JSON array
std::vector<std::wstring> ParseJSONArray(const std::wstring& arrayStr) {
    std::vector<std::wstring> results;
    size_t start = arrayStr.find(L"[");
    if (start == std::wstring::npos)
        start = 0;
    else
        start++;

    size_t end = arrayStr.rfind(L"]");
    if (end == std::wstring::npos)
        end = arrayStr.length();

    std::wstring inner = arrayStr.substr(start, end - start);
    std::wstring current;
    for (size_t i = 0; i < inner.length(); i++) {
        wchar_t c = inner[i];
        if (c == L',') {
            std::wstring item = current;
            while (!item.empty() &&
                   (item.front() == L' ' || item.front() == L'"' ||
                    item.front() == L'\t'))
                item.erase(item.begin());
            while (!item.empty() &&
                   (item.back() == L' ' || item.back() == L'"' ||
                    item.back() == L'\t'))
                item.pop_back();
            results.push_back(item);
            current.clear();
        } else {
            current += c;
        }
    }
    if (!current.empty()) {
        std::wstring item = current;
        while (!item.empty() && (item.front() == L' ' || item.front() == L'"' ||
                                 item.front() == L'\t'))
            item.erase(item.begin());
        while (!item.empty() && (item.back() == L' ' || item.back() == L'"' ||
                                 item.back() == L'\t'))
            item.pop_back();
        results.push_back(item);
    }
    return results;
}

std::wstring GetWindDirectionString(double degrees) {
    const wchar_t* directions[] = { L"N", L"NNE", L"NE", L"ENE", L"E", L"ESE", L"SE", L"SSE", L"S", L"SSW", L"SW", L"WSW", L"W", L"WNW", L"NW", L"NNW" };
    int index = (int)((degrees + 11.25) / 22.5) % 16;
    return directions[index];
}

// Sakamoto calendar calculator
std::wstring GetDayOfWeek(int year, int month, int day) {
    static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    if (month < 3)
        year -= 1;
    int dow =
        (year + year / 4 - year / 100 + year / 400 + t[month - 1] + day) % 7;
    const wchar_t* days[] = {L"Sun", L"Mon", L"Tue", L"Wed",
                             L"Thu", L"Fri", L"Sat"};
    return days[dow];
}

// Resolve weather codes
std::pair<std::wstring, std::wstring> GetCodeMapping(int code) {
    std::wstring icon = L"❓";
    std::wstring cond = L"Cloudy";

    if (g_weatherStyle == 0) {  // Emoji Style
        if (code == 0) {
            icon = L"☀️";
            cond = L"Clear";
        } else if (code >= 1 && code <= 3) {
            icon = code == 1 ? L"🌤️" : (code == 2 ? L"⛅" : L"☁️");
            cond = code == 1 ? L"Mainly Clear"
                             : (code == 2 ? L"Partly Cloudy" : L"Cloudy");
        } else if (code == 45 || code == 48) {
            icon = L"🌫️";
            cond = L"Foggy";
        } else if (code >= 51 && code <= 55) {
            icon = L"🌧️";
            cond = L"Drizzle";
        } else if (code == 56 || code == 57) {
            icon = L"🌧️";
            cond = L"Freezing Drizzle";
        } else if (code >= 61 && code <= 65) {
            icon = L"🌧️";
            cond = L"Rain";
        } else if (code == 66 || code == 67) {
            icon = L"🌧️";
            cond = L"Freezing Rain";
        } else if (code >= 71 && code <= 77) {
            icon = L"❄️";
            cond = L"Snow";
        } else if (code >= 80 && code <= 82) {
            icon = L"🌧️";
            cond = L"Showers";
        } else if (code == 85 || code == 86) {
            icon = L"❄️";
            cond = L"Snow Showers";
        } else if (code >= 95) {
            icon = L"⛈️";
            cond = L"Thunderstorm";
        } else {
            icon = L"⛅";
            cond = L"Cloudy";
        }
    } else { // Segoe MDL2 icon style
        if (code == 0) {
            icon = L"\u2600";  // Sunny
            cond = L"Clear";
        } else if (code >= 1 && code <= 3) {
            icon = code == 1 ? L"\u26C5" : (code == 2 ? L"\u26C5" : L"\u2601");
            cond = code == 1 ? L"Mainly Clear"
                             : (code == 2 ? L"Partly Cloudy" : L"Cloudy");
        } else if (code == 45 || code == 48) {
            icon = L"\u2601";  // Fog
            cond = L"Foggy";
        } else if (code >= 51 && code <= 55) {
            icon = L"\u2614";  // Rain Umbrella
            cond = L"Drizzle";
        } else if (code == 56 || code == 57) {
            icon = L"\u2614";
            cond = L"Freezing Drizzle";
        } else if (code >= 61 && code <= 65) {
            icon = L"\u2614";
            cond = L"Rain";
        } else if (code == 66 || code == 67) {
            icon = L"\u2614";
            cond = L"Freezing Rain";
        } else if (code >= 71 && code <= 77) {
            icon = L"\u2744";  // Snowflake
            cond = L"Snow";
        } else if (code >= 80 && code <= 82) {
            icon = L"\u2614";  // Showers
            cond = L"Showers";
        } else if (code == 85 || code == 86) {
            icon = L"\u2744";
            cond = L"Flurries";
        } else if (code >= 95) {
            icon = L"\u26A1";  // Lightning
            cond = L"Storm";
        } else {
            icon = L"\u2601";  // Cloudy default
            cond = L"Cloudy";
        }
    }
    return {icon, cond};
}

void MapOpenMeteoCode(int code, int is_day) {
    if (g_weatherStyle == 0) {  // Emoji Style
        if (code == 0) {
            g_cachedIcon = is_day ? L"☀️" : L"🌙";
            g_cachedCondition = L"Clear";
        } else if (code >= 1 && code <= 3) {
            g_cachedIcon = code == 1 ? L"🌤️" : (code == 2 ? L"⛅" : L"☁️");
            g_cachedCondition =
                code == 1 ? L"Mainly Clear"
                          : (code == 2 ? L"Partly Cloudy" : L"Cloudy");
        } else if (code == 45 || code == 48) {
            g_cachedIcon = L"🌫️";
            g_cachedCondition = L"Foggy";
        } else if (code >= 51 && code <= 55) {
            g_cachedIcon = L"🌧️";
            g_cachedCondition = L"Drizzle";
        } else if (code == 56 || code == 57) {
            g_cachedIcon = L"🌧️";
            g_cachedCondition = L"Freezing Drizzle";
        } else if (code >= 61 && code <= 65) {
            g_cachedIcon = L"🌧️";
            g_cachedCondition = L"Rain";
        } else if (code == 66 || code == 67) {
            g_cachedIcon = L"🌧️";
            g_cachedCondition = L"Freezing Rain";
        } else if (code >= 71 && code <= 77) {
            g_cachedIcon = L"❄️";
            g_cachedCondition = L"Snow";
        } else if (code >= 80 && code <= 82) {
            g_cachedIcon = L"🌧️";
            g_cachedCondition = L"Showers";
        } else if (code == 85 || code == 86) {
            g_cachedIcon = L"❄️";
            g_cachedCondition = L"Snow Showers";
        } else if (code >= 95) {
            g_cachedIcon = L"⛈️";
            g_cachedCondition = L"Thunderstorm";
        } else {
            g_cachedIcon = L"⛅";
            g_cachedCondition = L"Cloudy";
        }
    } else { // Segoe MDL2 icon style
        if (code == 0) {
            g_cachedIcon = is_day ? L"\u2600" : L"\u263D";
            g_cachedCondition = L"Clear";
        } else if (code >= 1 && code <= 3) {
            g_cachedIcon = code == 1 ? L"\u26C5" : (code == 2 ? L"\u26C5" : L"\u2601");
            g_cachedCondition =
                code == 1 ? L"Mainly Clear"
                          : (code == 2 ? L"Partly Cloudy" : L"Cloudy");
        } else if (code == 45 || code == 48) {
            g_cachedIcon = L"\u2601";
            g_cachedCondition = L"Foggy";
        } else if (code >= 51 && code <= 55) {
            g_cachedIcon = L"\u2614";
            g_cachedCondition = L"Drizzle";
        } else if (code == 56 || code == 57) {
            g_cachedIcon = L"\u2614";
            g_cachedCondition = L"Freezing Drizzle";
        } else if (code >= 61 && code <= 65) {
            g_cachedIcon = L"\u2614";
            g_cachedCondition = L"Rain";
        } else if (code == 66 || code == 67) {
            g_cachedIcon = L"\u2614";
            g_cachedCondition = L"Freezing Rain";
        } else if (code >= 71 && code <= 77) {
            g_cachedIcon = L"\u2744";
            g_cachedCondition = L"Snow";
        } else if (code >= 80 && code <= 82) {
            g_cachedIcon = L"\u2614";
            g_cachedCondition = L"Showers";
        } else if (code == 85 || code == 86) {
            g_cachedIcon = L"\u2744";
            g_cachedCondition = L"Snow Showers";
        } else if (code >= 95) {
            g_cachedIcon = L"\u26A1";
            g_cachedCondition = L"Thunderstorm";
        } else {
            g_cachedIcon = L"\u2601";
            g_cachedCondition = L"Cloudy";
        }
    }
}

bool IsWindows11() {
    static int build = 0;
    if (build == 0) {
        OSVERSIONINFOEXW osvi = {sizeof(osvi)};
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
    std::wstring temp = g_cachedTemp;
    std::wstring icon = g_cachedIcon;
    std::wstring condition = g_cachedCondition;
    bool acquired = g_weatherAcquired;

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
            ThreadXamlState& state = GetThreadXamlState();
            if (state.popupHwnd && IsWindowVisible(state.popupHwnd)) {
                PostMessageW(g_hSubclassedWnd, WM_USER + 4244, 0, 0);
            }
        }
    }

    Grid targetGrid = nullptr;
    {
        std::lock_guard<std::mutex> lock(g_weatherGridMutex);
        if (g_injectedWeatherGrid) {
            targetGrid = g_injectedWeatherGrid.get();
        }
    }

    if (targetGrid) {
        try {
            auto dispatcher = targetGrid.Dispatcher();
            if (dispatcher) {
                auto weakGrid = winrt::make_weak(targetGrid);
                dispatcher.RunAsync(
                    winrt::Windows::UI::Core::CoreDispatcherPriority::Normal,
                    [weakGrid, temp, icon, condition, acquired]() {
                        try {
                            if (auto grid = weakGrid.get()) {
                                UpdateWeatherXamlElements(grid, temp, icon, condition, acquired);
                            }
                        } catch (...) {}
                    });
            }
        } catch (...) {}
    }
}

// Background meteorological synchronizer thread
DWORD WINAPI QueryWeatherPipeline(LPVOID lpParam) {
    winrt::init_apartment();
    if (g_debugLogs)
        Wh_Log(L"[EP_WeatherHost] Background thread PIPELINE started");

    while (!g_bThreadShouldTerm) {
        bool updateSuccess = false;

        std::wstring loc = g_location;
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
                if (op.wait_for(std::chrono::seconds(1)) == winrt::Windows::Foundation::AsyncStatus::Completed) {
                    pos = op.GetResults();
                } else {
                    op.Cancel();
                }
                if (pos) {
                    auto coordinate = pos.Coordinate();
                    if (coordinate) {
                        auto point = coordinate.Point();
                        if (point) {
                            auto position = point.Position();
                            g_cachedLatitude = position.Latitude;
                            g_cachedLongitude = position.Longitude;
                            
                            std::wstring path = L"/data/reverse-geocode-client?latitude=" + std::to_wstring(g_cachedLatitude) + 
                                                L"&longitude=" + std::to_wstring(g_cachedLongitude) + L"&localityLanguage=en";
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
                                
                                g_displayCity = !fullCity.empty() ? fullCity : L"Local Area";
                            } else {
                                g_displayCity = L"My Location";
                            }
                            
                            if (g_debugLogs) {
                                Wh_Log(L"[EP_WeatherHost] WinRT Geolocator success: %s (Lat=%f, Lon=%f)", 
                                       g_displayCity.c_str(), g_cachedLatitude, g_cachedLongitude);
                            }
                            preciseSuccess = true;
                            updateSuccess = true;
                        }
                    }
                }
            } catch (...) {
                if (g_debugLogs) {
                    Wh_Log(L"[EP_WeatherHost] WinRT Geolocator failed. To fix this, enable 'Location services' and 'Let desktop apps access your location' in Windows Privacy settings. Falling back to IP Geolocation.");
                }
            }

            if (!preciseSuccess) {
                if (g_debugLogs)
                    Wh_Log(L"[EP_WeatherHost] Triggering Geolocation IP Search...");
                std::wstring geoJson =
                    RequestHttpData(L"ip-api.com", L"/json", false);
                if (geoJson.empty()) {
                    if (g_debugLogs)
                        Wh_Log(
                            L"[EP_WeatherHost] ip-api.com failed, trying "
                            L"ipapi.co...");
                    geoJson = RequestHttpData(L"ipapi.co", L"/json/", true);
                }
                if (geoJson.empty()) {
                    if (g_debugLogs)
                        Wh_Log(
                            L"[EP_WeatherHost] ipapi.co failed, trying "
                            L"ipinfo.io...");
                    geoJson = RequestHttpData(L"ipinfo.io", L"/json", true);
                }

                if (!geoJson.empty()) {
                    std::wstring latStr = ExtractJSONValue(geoJson, L"lat");
                    if (latStr.empty())
                        latStr = ExtractJSONValue(geoJson, L"latitude");
                    std::wstring lonStr = ExtractJSONValue(geoJson, L"lon");
                    if (lonStr.empty())
                        lonStr = ExtractJSONValue(geoJson, L"longitude");
                    std::wstring cityStr = ExtractJSONValue(geoJson, L"city");
                    std::wstring regionStr =
                        ExtractJSONValue(geoJson, L"regionName");
                    if (regionStr.empty())
                        regionStr = ExtractJSONValue(geoJson, L"region");
                    std::wstring countryStr =
                        ExtractJSONValue(geoJson, L"countryCode");
                    if (countryStr.empty())
                        countryStr = ExtractJSONValue(geoJson, L"country");

                    if (!latStr.empty() && !lonStr.empty()) {
                        g_cachedLatitude = wcstod(latStr.c_str(), NULL);
                        g_cachedLongitude = wcstod(lonStr.c_str(), NULL);

                        std::wstring fullCity = cityStr;
                        if (!fullCity.empty() && !regionStr.empty())
                            fullCity += L", " + regionStr;
                        if (!fullCity.empty() && !countryStr.empty())
                            fullCity += L", " + countryStr;

                        g_displayCity =
                            !fullCity.empty() ? fullCity : L"Local Weather";
                        if (g_debugLogs)
                            Wh_Log(L"[EP_WeatherHost] Geolocated by IP to %s (%f, %f)",
                                   g_displayCity.c_str(), g_cachedLatitude,
                                   g_cachedLongitude);
                        updateSuccess = true;
                    }
                }
            }

            if (!preciseSuccess && !updateSuccess) {
                if (g_debugLogs) {
                    Wh_Log(L"[EP_WeatherHost] Geolocation failed entirely. Using default coordinates (New York) and setting display name to 'My Location'.");
                }
                g_cachedLatitude = 40.7128;
                g_cachedLongitude = -74.0060;
                g_displayCity = L"My Location";
                updateSuccess = true;
            }
        } else {
            if (g_debugLogs)
                Wh_Log(L"[EP_WeatherHost] Resolving coordinates for city: %s",
                       loc.c_str());

            // Clean up city name input: replace separators with spaces
            std::wstring cleanLoc = L"";
            for (wchar_t c : loc) {
                if (c == L',' || c == L';' || c == L'.') {
                    cleanLoc += L" ";
                } else {
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
                g_cachedLatitude = wcstod(latStr.c_str(), NULL);
                g_cachedLongitude = wcstod(lonStr.c_str(), NULL);

                std::wstring fullCity = nameStr;
                if (!fullCity.empty() && !adminStr.empty())
                    fullCity += L", " + adminStr;
                if (!fullCity.empty() && !countryStr.empty())
                    fullCity += L", " + countryStr;

                g_displayCity = !fullCity.empty() ? fullCity : loc;
                if (g_debugLogs)
                    Wh_Log(
                        L"[EP_WeatherHost] Resolved Coordinates: %s (%f, %f)",
                        g_displayCity.c_str(), g_cachedLatitude,
                        g_cachedLongitude);
                updateSuccess = true;
            } else {
                // Clear out stale cached coordinates to avoid showing incorrect
                // cities (like Sarnia)
                g_cachedLatitude = 0.0;
                g_cachedLongitude = 0.0;
                g_displayCity = L"Location not found";
                g_cachedTemp = L"--";
                g_cachedCondition = L"Check settings";
                g_activeWarning = L"";
                if (g_debugLogs)
                    Wh_Log(
                        L"[EP_WeatherHost] Geocoding failure: location (%s) "
                        L"not found on Open-Meteo",
                        loc.c_str());
            }
        }

        if (g_cachedLatitude != 0.0 || g_cachedLongitude != 0.0) {
            wchar_t pathBuf[512];
            swprintf(
                pathBuf,
                L"/v1/"
                L"forecast?latitude=%.4f&longitude=%.4f&current=temperature_2m,"
                L"relative_humidity_2m,is_day,weather_code,wind_speed_10m,wind_direction_10m&"
                L"hourly=temperature_2m,weathercode,precipitation_probability,"
                L"wind_speed_10m,wind_direction_10m,relative_humidity_2m&daily=weathercode,temperature_2m_max,"
                L"temperature_2m_min&timezone=auto&forecast_days=%d",
                g_cachedLatitude, g_cachedLongitude, g_forecastDaysFetch);
            std::wstring forecastJson =
                RequestHttpData(L"api.open-meteo.com", pathBuf, true);

            if (!forecastJson.empty()) {
                std::wstring alertStr = L"";
                try {
                    wchar_t alertPath[512];
                    swprintf(alertPath, L"/v3/alerts/headlines?geocode=%.4f,%.4f&format=json&language=en-US&apiKey=e1f10a1e78da46f5b10a1e78da96f525", g_cachedLatitude, g_cachedLongitude);
                    std::wstring alertsJson = RequestHttpData(L"api.weather.com", alertPath, true);
                    if (g_debugLogs) {
                        Wh_Log(L"[EP_WeatherHost] Weather.com Alerts API returned %d chars: %.100s", (int)alertsJson.length(), alertsJson.c_str());
                    }
                    if (!alertsJson.empty()) {
                        std::wstring alertsArray = ExtractJSONArray(alertsJson, L"alerts");
                        if (!alertsArray.empty() && alertsArray.length() > 5) {
                            alertStr = ExtractJSONValue(alertsArray, L"eventDescription");
                            if (alertStr.empty()) {
                                alertStr = ExtractJSONValue(alertsArray, L"headlineText");
                            }
                            if (g_debugLogs) {
                                Wh_Log(L"[EP_WeatherHost] Found active alert: %s", alertStr.c_str());
                            }
                        } else {
                            if (g_debugLogs) {
                                Wh_Log(L"[EP_WeatherHost] No active alerts in the array");
                            }
                        }
                    }
                } catch (...) {
                    if (g_debugLogs) {
                        Wh_Log(L"[EP_WeatherHost] Alerts API query encountered an exception");
                    }
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

                int codeVal = !codeStr.empty() ? _wtoi(codeStr.c_str()) : 0;
                double tempVal = !tempStr.empty() ? wcstod(tempStr.c_str(), NULL) : 0.0;
                double windVal = !windSpeedStr.empty() ? wcstod(windSpeedStr.c_str(), NULL) : 0.0;

                if (alertStr.empty()) {
                    if (codeVal == 99 || codeVal == 96) {
                        alertStr = L"Severe Thunderstorm Warning";
                    } else if (codeVal == 95) {
                        alertStr = L"Thunderstorm Advisory";
                    } else if (codeVal == 82) {
                        alertStr = L"Torrential Rain Advisory";
                    } else if (codeVal == 75 || codeVal == 86) {
                        alertStr = L"Heavy Snow Warning";
                    } else if (codeVal == 73 || codeVal == 85) {
                        alertStr = L"Snow Advisory";
                    } else if (codeVal == 71) {
                        alertStr = L"Winter Weather Advisory";
                    } else if (codeVal == 67) {
                        alertStr = L"Ice Storm Warning";
                    } else if (codeVal == 66) {
                        alertStr = L"Freezing Rain Advisory";
                    } else if (codeVal == 65) {
                        alertStr = L"Heavy Rain Advisory";
                    } else if (codeVal == 45 || codeVal == 48) {
                        alertStr = L"Dense Fog Advisory";
                    } else if (tempVal >= 40.0) {
                        alertStr = L"Extreme Heat Warning";
                    } else if (tempVal >= 35.0) {
                        alertStr = L"Heat Advisory";
                    } else if (tempVal <= -15.0) {
                        alertStr = L"Extreme Cold Warning";
                    } else if (tempVal <= -5.0) {
                        alertStr = L"Winter Weather Advisory";
                    } else if (windVal >= 60.0) {
                        alertStr = L"High Wind Warning";
                    } else if (windVal >= 40.0) {
                        alertStr = L"Wind Advisory";
                    }
                }

                std::wstring formattedWindSpeed = L"--";
                if (!windSpeedStr.empty()) {
                    double ws = wcstod(windSpeedStr.c_str(), NULL);
                    double wd = windDirStr.empty() ? 0.0 : wcstod(windDirStr.c_str(), NULL);
                    std::wstring dirStr = GetWindDirectionString(wd);
                    wchar_t wsBuf[128];
                    if (g_useCelsius) {
                        swprintf(wsBuf, L"%.1f km/h %s", ws, dirStr.c_str());
                    } else {
                        double mph = ws * 0.621371;
                        swprintf(wsBuf, L"%.1f mph %s", mph, dirStr.c_str());
                    }
                    formattedWindSpeed = wsBuf;
                }

                std::wstring timeArrStr = L"";
                std::wstring codeArrStr = L"";
                std::wstring maxArrStr = L"";
                std::wstring minArrStr = L"";
                if (!dailyBlock.empty()) {
                    timeArrStr = ExtractJSONArray(dailyBlock, L"time");
                    codeArrStr = ExtractJSONArray(dailyBlock, L"weathercode");
                    maxArrStr =
                        ExtractJSONArray(dailyBlock, L"temperature_2m_max");
                    minArrStr =
                        ExtractJSONArray(dailyBlock, L"temperature_2m_min");
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

                std::wstring currentPrecipProb = L"0%";

                size_t startHourIdx = 0;
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
                if (startHourIdx < hPrecipVec.size()) {
                    currentPrecipProb = hPrecipVec[startHourIdx] + L"%";
                }

                size_t numDays = timeVec.size();
                if (codeVec.size() < numDays)
                    numDays = codeVec.size();
                if (maxVec.size() < numDays)
                    numDays = maxVec.size();
                if (minVec.size() < numDays)
                    numDays = minVec.size();

                std::vector<DailyForecast> parsedBriefForecasts;
                for (size_t i = 0; i < numDays; i++) {
                    DailyForecast dayData;

                    std::wstring rawDate = timeVec[i];
                    dayData.rawDate = rawDate;
                    int year = 0, month = 0, dayInt = 0;
                    if (swscanf(rawDate.c_str(), L"%d-%d-%d", &year, &month,
                                &dayInt) == 3) {
                        dayData.dayName = GetDayOfWeek(year, month, dayInt);
                    } else {
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

                    parsedBriefForecasts.push_back(dayData);
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

                    hourData.precipProb = hPrecipVec[i] + L"%";
                    if (i < hHumidVec.size()) {
                        hourData.humidity = hHumidVec[i] + L"%";
                    } else {
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
                        } else {
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
                g_activeWarning = alertStr;
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
                    g_cachedTemp = formattedTemp;

                    int code = !codeStr.empty() ? _wtoi(codeStr.c_str()) : 0;
                    int is_day = !dayStr.empty() ? _wtoi(dayStr.c_str()) : 1;

                    MapOpenMeteoCode(code, is_day);
                    g_weatherAcquired = true;
                    updateSuccess = true;

                    if (g_debugLogs) {
                        Wh_Log(
                            L"[EP_WeatherHost] Weather Updated! Temp=%s, "
                            L"Code=%d, Condition=%s, City=%s",
                            g_cachedTemp.c_str(), code,
                            g_cachedCondition.c_str(), g_displayCity.c_str());
                    }

                    QueueWeatherUpdateOnUIThread();
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
void RemoveInjectedFromGrid(Grid grid) {
    if (!grid)
        return;
    try {
        auto children = grid.Children();
        for (int i = (int)children.Size() - 1; i >= 0; i--) {
            if (auto fe = children.GetAt(i).try_as<FrameworkElement>()) {
                std::wstring name(fe.Name());
                if (name == L"WhWeatherHostGrid") {
                    children.RemoveAt(i);
                } else {
                    std::wstring sibClass(winrt::get_class_name(fe).c_str());
                    if (sibClass.find(L"Repeater") != std::wstring::npos || sibClass.find(L"TaskbarFrameRepeater") != std::wstring::npos) {
                        fe.Margin(Thickness{0, 0, 0, 0});
                    }
                }
            }
        }
    } catch (...) {
    }
}

void RemoveInjectedFromPanel(winrt::Windows::UI::Xaml::Controls::Panel panel) {
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
            }
        }
    } catch (...) {
    }
}

// Native direct XAML injection of Weather Grid
void InjectContentIntoGrid(FrameworkElement element,
                           std::wstring_view uniqueName) {
    auto grid = element.try_as<Grid>();
    if (!grid)
        return;

    RemoveInjectedFromGrid(grid);

    Grid weatherGrid;
    weatherGrid.Name(uniqueName);

    // Make sure we span the entire Grid, bypassing any column definitions,
    // columns, or layout cells so that margin aligns globally
    try {
        Grid::SetColumn(weatherGrid, 0);
        Grid::SetColumnSpan(weatherGrid, 99);
        Grid::SetRow(weatherGrid, 0);
        Grid::SetRowSpan(weatherGrid, 99);
    } catch (...) {
    }

    UpdateInjectedWeatherLayout(weatherGrid);

    // Transparent solid color brush background
    weatherGrid.Background(winrt::Windows::UI::Xaml::Media::SolidColorBrush(
        winrt::Windows::UI::Colors::Transparent()));

    UpdateWeatherXamlElements(weatherGrid, g_cachedTemp, g_cachedIcon,
                              g_cachedCondition, g_weatherAcquired);

    grid.Children().Append(weatherGrid);

    auto weakWeatherGrid = winrt::make_weak(weatherGrid);
    element.SizeChanged([weakWeatherGrid](auto const&, auto const&) {
        if (auto wg = weakWeatherGrid.get()) {
            try {
                auto dispatcher = wg.Dispatcher();
                if (dispatcher) {
                    dispatcher.RunAsync(
                        winrt::Windows::UI::Core::CoreDispatcherPriority::Normal,
                        [weakWeatherGrid]() {
                            try {
                                if (auto wg2 = weakWeatherGrid.get()) {
                                    UpdateInjectedWeatherLayout(wg2);
                                }
                            } catch (...) {}
                        });
                }
            } catch (...) {}
        }
    });

    {
        std::lock_guard<std::mutex> lock(g_weatherGridMutex);
        g_injectedWeatherGrid = winrt::make_weak(weatherGrid);
    }
}

// Helper to find widgets button
bool FindAndInjectWidgetsButton(FrameworkElement element) {
    if (!element) return false;
    
    std::wstring name(element.Name());
    std::wstring className(winrt::get_class_name(element).c_str());

    if (className.find(L"AugmentedEntryPoint") != std::wstring::npos ||
        name == L"WidgetsButton" || name == L"Widgets") {
        
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
                    std::wstring childName(existingChild.Name());
                    if (childName != L"WhWeatherHostGrid") {
                        existingChild.Opacity(0.0);
                        existingChild.IsHitTestVisible(false);
                        try {
                            existingChild.Margin(Thickness{0,0,0,0});
                        } catch(...) {}
                    }
                }
            }
            
            if (auto grid = innerPanel.try_as<Grid>()) {
                InjectContentIntoGrid(grid, L"WhWeatherHostGrid");
                return true;
            } else {
                RemoveInjectedFromPanel(innerPanel);
                Grid weatherGrid;
                weatherGrid.Name(L"WhWeatherHostGrid");
                UpdateInjectedWeatherLayout(weatherGrid);
                weatherGrid.Background(winrt::Windows::UI::Xaml::Media::SolidColorBrush(winrt::Windows::UI::Colors::Transparent()));
                UpdateWeatherXamlElements(weatherGrid, g_cachedTemp, g_cachedIcon, g_cachedCondition, g_weatherAcquired);
                innerPanel.Children().Append(weatherGrid);
                
                auto weakWg = winrt::make_weak(weatherGrid);
                element.SizeChanged([weakWg](auto const&, auto const&) {
                    if (auto wg = weakWg.get()) {
                        try {
                            auto dispatcher = wg.Dispatcher();
                            if (dispatcher) {
                                dispatcher.RunAsync(
                                    winrt::Windows::UI::Core::CoreDispatcherPriority::Normal,
                                    [weakWg]() {
                                        try {
                                            if (auto wg2 = weakWg.get()) {
                                                UpdateInjectedWeatherLayout(wg2);
                                            }
                                        } catch (...) {}
                                    });
                            }
                        } catch (...) {}
                    }
                });
                {
                    std::lock_guard<std::mutex> lock(g_weatherGridMutex);
                    g_injectedWeatherGrid = winrt::make_weak(weatherGrid);
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

// Tree scanning logic targeting TaskbarFrameRepeater/ItemsRepeater parent only
void ScanAndInjectRecursive(FrameworkElement element) {
    if (!element)
        return;
        
    if (g_injectToSysTray) {
        if (FindAndInjectRepeater(element)) {
            return;
        }
    } else {
        // First pass: try to find the actual widgets button to hijack
        if (FindAndInjectWidgetsButton(element)) {
            return;
        }

        // Try to find the target TaskbarFrameRepeater and inject to its parent grid
        if (FindAndInjectRepeater(element)) {
            return;
        }
    }
}

std::atomic<bool> g_scanPending = false;
std::vector<winrt::weak_ref<FrameworkElement>> g_scannedFrames;
std::mutex g_pendingMutex;

void ScheduleScanAsync(FrameworkElement startNode) {
    if (!startNode || g_scanPending.exchange(true))
        return;
    auto weak = winrt::make_weak(startNode);
    try {
        startNode.Dispatcher().RunAsync(
            winrt::Windows::UI::Core::CoreDispatcherPriority::Low, [weak]() {
                g_scanPending = false;
                if (auto node = weak.get()) {
                    FrameworkElement current = node;
                    while (current) {
                        if (winrt::get_class_name(current) ==
                            L"Taskbar.TaskbarFrame") {
                            void* abi = winrt::get_abi(current);
                            {
                                std::lock_guard<std::mutex> lock(
                                    g_pendingMutex);
                                bool alreadyScanned = false;
                                for (auto& f : g_scannedFrames) {
                                    if (auto existing = f.get()) {
                                        if (winrt::get_abi(existing) == abi) {
                                            alreadyScanned = true;
                                            break;
                                        }
                                    }
                                }
                                if (alreadyScanned)
                                    break;
                                g_scannedFrames.push_back(
                                    winrt::make_weak(current));
                            }

                            ScanAndInjectRecursive(current);
                            return;
                        }
                        auto parent = VisualTreeHelper::GetParent(current);
                        current = parent ? parent.try_as<FrameworkElement>()
                                         : nullptr;
                    }
                    ScanAndInjectRecursive(node);
                }
            });
    } catch (...) {
        g_scanPending = false;
    }
}

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
         (void*)TaskListButton_UpdateVisualStates_Hook}};
    return HookSymbols(module, symbolHooks, ARRAYSIZE(symbolHooks));
}

std::atomic<bool> g_taskbarViewDllLoaded = false;

typedef struct _UNICODE_STRING_MINI {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING_MINI, *PUNICODE_STRING_MINI;

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
UINT g_msgHotkeyControl = 0;

bool g_win10WeatherHovered = false;
bool g_win10WeatherPressed = false;
bool g_win10TrackingMouse = false;
bool g_modUnloaded = false;
int g_lastAddedWeatherWidth = 115;

typedef BOOL (WINAPI *PFN_ALPHABLEND)(HDC, int, int, int, int, HDC, int, int, int, int, BLENDFUNCTION);

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
                BITMAPINFO bmi = {{0}};
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
                    
                    BLENDFUNCTION bf = {0};
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

    std::wstring icon = g_cachedIcon;
    std::wstring temp = g_cachedTemp;
    std::wstring cond = g_cachedCondition;

    if (!g_weatherAcquired) {
        icon = (g_weatherStyle == 0) ? L"☁️" : L"\u2601";
        temp = L"Weather";
        cond = L"Loading...";
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

    int baseIconSize = (g_weatherStyle == 1) ? g_iconFontSize + 4 : g_iconFontSize;
    HFONT hFontIcon = CreateFontW(
        -(baseIconSize), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        (g_weatherStyle == 1) ? L"Segoe UI Symbol" : L"Segoe UI Emoji");
    
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFontIcon);
    SIZE szIcon = {0};
    GetTextExtentPoint32W(hdc, icon.c_str(), icon.length(), &szIcon);

    int maxTextWidth = 0;
    SIZE szTemp = {0}, szCond = {0};
    if (g_showConditionName) {
        SelectObject(hdc, hFontTemp);
        GetTextExtentPoint32W(hdc, temp.c_str(), temp.length(), &szTemp);
        SelectObject(hdc, hFontCond);
        GetTextExtentPoint32W(hdc, cond.c_str(), cond.length(), &szCond);
        maxTextWidth = szTemp.cx > szCond.cx ? szTemp.cx : szCond.cx;
    } else {
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
    int requiredWidth = szIcon.cx + spacing + maxTextWidth + 8; // 8px total padding (4 left, 4 right)
    if (requiredWidth < minimumWidth) {
        requiredWidth = minimumWidth;
    }
    // Set a sensible maximum width to avoid taking over the whole taskbar under weird inputs (e.g. 250px)
    if (requiredWidth > 250) {
        requiredWidth = 250;
    }

    g_lastAddedWeatherWidth = requiredWidth;
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
                    } else {
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

    std::wstring icon = g_cachedIcon;
    std::wstring temp = g_cachedTemp;
    std::wstring cond = g_cachedCondition;

    if (!g_weatherAcquired) {
        icon = (g_weatherStyle == 0) ? L"☁️" : L"\u2601";
        temp = L"Weather";
        cond = L"Loading...";
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
    RECT rcParentBg = {0, 0, weatherWidth, height};
    BP_PAINTPARAMS params = {sizeof(BP_PAINTPARAMS), 0, NULL, NULL};
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
    } else {
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
        } else {
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

    int baseIconSize = (g_weatherStyle == 1) ? g_iconFontSize + 4 : g_iconFontSize;
    HFONT hFontIcon = CreateFontW(
        -(baseIconSize), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        (g_weatherStyle == 1) ? L"Segoe UI Symbol" : L"Segoe UI Emoji");
    
    HFONT hOldFont = (HFONT)SelectObject(drawDc, hFontIcon);
    
    // Calculate precise centering
    SIZE szIcon = {0};
    GetTextExtentPoint32W(drawDc, icon.c_str(), icon.length(), &szIcon);
    if (szIcon.cx <= 0) {
        szIcon.cx = baseIconSize;
    }
    
    int maxTextWidth = 0;
    SIZE szTemp = {0}, szCond = {0};
    if (g_showConditionName) {
        SelectObject(drawDc, hFontTemp);
        GetTextExtentPoint32W(drawDc, temp.c_str(), temp.length(), &szTemp);
        SelectObject(drawDc, hFontCond);
        GetTextExtentPoint32W(drawDc, cond.c_str(), cond.length(), &szCond);
        maxTextWidth = szTemp.cx > szCond.cx ? szTemp.cx : szCond.cx;
    } else {
        SelectObject(drawDc, hFontTemp);
        GetTextExtentPoint32W(drawDc, temp.c_str(), temp.length(), &szTemp);
        maxTextWidth = szTemp.cx;
    }
    
    bool useColorEmoji = (g_weatherStyle == 0);
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
        RECT rcIcon = {iconX, startY, iconX + szIcon.cx, startY + iconH};

        if (useColorEmoji) {
            RECT rcEmoji = rcIcon;
            rcEmoji.top -= 1;
            rcEmoji.bottom -= 1;
            DrawColorEmojiDirect2D(drawDc, icon, rcEmoji, baseIconSize);
        } else {
            DrawTextAlpha(hFontIcon, icon, rcIcon, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX | DT_NOCLIP, false, iconCol);
        }

        int tempY = startY + iconH + stackSpacing;
        RECT rcTemp = {0, tempY, weatherWidth, tempY + tempH};
        DrawTextAlpha(hFontTemp, temp, rcTemp, DT_SINGLELINE | DT_CENTER | DT_TOP | DT_NOPREFIX | DT_NOCLIP);
    } else {
        // Horizontal layout (original side-by-side logic)
        int spacing = (g_weatherStyle == 0) ? 2 : 8;
        int totalWidth = szIcon.cx + spacing + maxTextWidth;
        int startX = (weatherWidth - totalWidth) / 2;
        if (g_weatherStyle == 0) {
            startX -= 6; // Keep emoji icons exactly where they are
        } else {
            startX += 0; // Move assets icons right by 6px relative to emoji icons (i.e. -6 + 6 = 0)
        }
        if (startX < 4) startX = 4;
        
        RECT rcIcon = {startX, 1, startX + szIcon.cx, height + 1};
        
        if (useColorEmoji) {
            RECT rcEmoji = rcIcon;
            rcEmoji.top -= 1; // minor optical adjustment to move emoji up slightly
            rcEmoji.bottom -= 1;
            DrawColorEmojiDirect2D(drawDc, icon, rcEmoji, baseIconSize);
        } else {
            DrawTextAlpha(hFontIcon, icon, rcIcon, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_NOPREFIX | DT_NOCLIP, false, iconCol);
        }

        int textLeft = startX + szIcon.cx + spacing;
        int textWidth = weatherWidth - textLeft;

        if (g_showConditionName) {
            RECT rcTemp = {textLeft, 0, textLeft + textWidth, height / 2 + 1};
            DrawTextAlpha(hFontTemp, temp, rcTemp, DT_SINGLELINE | DT_BOTTOM | DT_LEFT | DT_NOPREFIX | DT_NOCLIP);

            RECT rcCond = {textLeft, height / 2 - 1, textLeft + textWidth, height};
            DrawTextAlpha(hFontCond, cond, rcCond, DT_SINGLELINE | DT_TOP | DT_LEFT | DT_NOPREFIX | DT_NOCLIP);
        } else {
            RECT rcTemp = {textLeft, 0, textLeft + textWidth, height};
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
IDesktopWindowXamlSourceNative : public IUnknown {
public:
    virtual HRESULT STDMETHODCALLTYPE AttachToWindow(HWND parent) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_WindowHandle(HWND* value) = 0;
};

// Global state removed - replaced with GetThreadXamlState()
DWORD g_lastHideTickCount = 0;
bool g_popupWasVisibleOnLButtonDown = false;

void CleanupXamlMedia(winrt::Windows::UI::Xaml::UIElement const& element) {
    if (!element) return;
    try {
        if (auto media = element.try_as<winrt::Windows::UI::Xaml::Controls::MediaElement>()) {
            try {
                media.Stop();
                media.Source(nullptr);
            } catch (...) {}
        } else if (auto panel = element.try_as<winrt::Windows::UI::Xaml::Controls::Panel>()) {
            try {
                uint32_t size = panel.Children().Size();
                for (uint32_t i = 0; i < size; ++i) {
                    CleanupXamlMedia(panel.Children().GetAt(i));
                }
                panel.Children().Clear();
            } catch (...) {}
        } else if (auto border = element.try_as<winrt::Windows::UI::Xaml::Controls::Border>()) {
            try {
                auto child = border.Child();
                if (child) {
                    CleanupXamlMedia(child);
                    border.Child(nullptr);
                }
            } catch (...) {}
        } else if (auto contentControl = element.try_as<winrt::Windows::UI::Xaml::Controls::ContentControl>()) {
            try {
                if (auto contentElem = contentControl.Content().try_as<winrt::Windows::UI::Xaml::UIElement>()) {
                    CleanupXamlMedia(contentElem);
                }
                contentControl.Content(nullptr);
            } catch (...) {}
        }
    } catch (...) {}
}

void HideWin10PopupInternal(HWND hWnd) {
    if (hWnd && IsWindow(hWnd)) {
        ShowWindow(hWnd, SW_HIDE);
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
            } catch (...) {}
        }
    }
}

void HideWin10Popup(HWND hWnd) {
    ThreadXamlState& state = GetThreadXamlState();
    if (state.popupHwnd && IsWindow(state.popupHwnd)) {
        PostMessageW(state.popupHwnd, WM_USER + 5002, 0, 0);
    }
}

void ApplyWin10Acrylic(HWND hWnd);
void ShowWin10ForecastPopupInternal(HWND hClock);
void UpdateWin10PopupInternal(HWND hWnd);

LRESULT CALLBACK Win10PopupWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    ThreadXamlState& state = GetThreadXamlState();
    if (uMsg == WM_ACTIVATE) {
        if (LOWORD(wParam) == WA_INACTIVE) {
            HWND hActive = (HWND)lParam;
            if (hActive != hWnd && hActive != state.sourceHwnd && !IsChild(hWnd, hActive)) {
                g_lastHideTickCount = GetTickCount();
                PostMessageW(hWnd, WM_USER + 5002, 0, 0); // Hide popup
                return 0;
            }
        }
    } else if (uMsg == (WM_USER + 5001)) {
        HWND hClock = (HWND)wParam;
        ShowWin10ForecastPopupInternal(hClock);
        return 0;
    } else if (uMsg == (WM_USER + 5002)) {
        HideWin10PopupInternal(hWnd);
        return 0;
    } else if (uMsg == (WM_USER + 5003)) {
        UpdateWin10PopupInternal(hWnd);
        return 0;
    } else if (uMsg == (WM_USER + 5004)) {
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
            } catch (...) {}
            try {
                state.source.Close();
            } catch (...) {}
            state.source = nullptr;
        }
        if (state.manager) {
            state.manager = nullptr;
        }
        state.popupHwnd = nullptr;
        state.sourceHwnd = nullptr;
        DestroyWindow(hWnd);
        PostQuitMessage(0);
        return 0;
    } else if (uMsg == (WM_USER + 4246)) {
        SendMessageW(hWnd, WM_USER + 5004, 0, 0);
        return 0;
    } else if (uMsg == WM_SIZE) {
        HWND hXAML = (HWND)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
        if (hXAML) {
            SetWindowPos(hXAML, NULL, 0, 0, LOWORD(lParam), HIWORD(lParam), SWP_NOZORDER | SWP_NOACTIVATE);
        }
    } else if (uMsg == WM_ERASEBKGND) {
        return 1;
    } else if (uMsg == WM_DESTROY) {
        if (state.source) {
            try {
                state.source.Close();
            } catch (...) {}
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

    WNDCLASSW wc = {0};
    wc.style = CS_DROPSHADOW;
    wc.lpfnWndProc = Win10PopupWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"WhWin10ForecastPopupClass";
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    RegisterClassW(&wc);
    registered = true;
}

DWORD WINAPI PopupThreadProc(LPVOID lpParam) {
    try {
        winrt::init_apartment(winrt::apartment_type::single_threaded);
    } catch (...) {
        if (g_debugLogs) Wh_Log(L"[Wh_WeatherHost] PopupThreadProc: init_apartment failed!");
        return 0;
    }

    ThreadXamlState& state = GetThreadXamlState();
    try {
        state.manager = winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager::InitializeForCurrentThread();
    } catch (...) {
        if (g_debugLogs) Wh_Log(L"[Wh_WeatherHost] PopupThreadProc: InitializeForCurrentThread failed!");
        return 0;
    }

    RegisterWin10PopupClass();

    DWORD dwExStyle = WS_EX_TOPMOST | WS_EX_TOOLWINDOW;
    if (!IsWindows11() && g_useAcrylic) {
        dwExStyle |= WS_EX_LAYERED | 0x00200000L; // WS_EX_NOREDIRECTIONBITMAP
    }

    state.popupHwnd = CreateWindowExW(
        dwExStyle,
        L"WhWin10ForecastPopupClass", L"",
        WS_POPUP,
        0, 0, 100, 100,
        NULL, NULL, GetModuleHandleW(NULL), NULL
    );

    if (!state.popupHwnd) {
        if (g_debugLogs) Wh_Log(L"[Wh_WeatherHost] PopupThreadProc: CreateWindowExW failed, error %d", GetLastError());
        return 0;
    }

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    if (g_debugLogs) Wh_Log(L"[Wh_WeatherHost] PopupThreadProc: message loop exited");
    return 0;
}

void UpdateWin10PopupInternal(HWND hWnd) {
    ThreadXamlState& state = GetThreadXamlState();
    if (state.popupHwnd && IsWindowVisible(state.popupHwnd) && state.source) {
        try {
            winrt::Windows::UI::Xaml::Controls::Grid rootGrid;
            g_selectedDate = L"";
            g_selectedHour = L"";
            g_selectedGraphTab = 0;
            g_graphScrollOffset = -1.0;
            g_dailyScrollOffset = 0.0;
            PopulateForecastUI(rootGrid, g_cachedCondition, g_cachedIcon, g_cachedTemp);
            
            double panelWidth = 336.0;
            if (g_forecastDaysFetch >= 14)
                panelWidth = 570.0;
            else if (g_forecastDaysFetch >= 10)
                panelWidth = 492.0;
            double panelHeight = 365.0;

            std::wstring displayWarning = g_activeWarning;
            if (!g_mockWeatherAlert.empty()) {
                displayWarning = g_mockWeatherAlert;
            }
            if (!displayWarning.empty()) {
                panelHeight += 32.0;
            }

            rootGrid.Width(panelWidth);
            rootGrid.Height(panelHeight);

            try {
                auto oldContent = state.source.Content();
                if (oldContent) {
                    CleanupXamlMedia(oldContent);
                }
            } catch (...) {}
            state.source.Content(rootGrid);

            // Dynamically resize the HWND if height changed
            double GetDpiScaleForWindow(HWND);
            double dpiScale = GetDpiScaleForWindow(state.popupHwnd);
            int physicalWidth = (int)ceil(panelWidth * dpiScale);
            int physicalHeight = (int)ceil(panelHeight * dpiScale);
            RECT rect;
            if (GetWindowRect(state.popupHwnd, &rect)) {
                int newY = rect.top;
                int currentHeight = rect.bottom - rect.top;
                if (physicalHeight != currentHeight) {
                    newY = rect.bottom - physicalHeight;
                }
                SetWindowPos(state.popupHwnd, NULL, rect.left, newY, physicalWidth, physicalHeight, SWP_NOZORDER | SWP_NOACTIVATE);
            }

            RedrawWindow(state.popupHwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
        } catch (...) {}
    }
}

double GetDpiScaleForWindow(HWND hWnd) {
    typedef UINT (WINAPI *GetDpiForWindow_t)(HWND);
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

struct ACCENT_POLICY_WEATHER {
    int State;
    int Flags;
    int Color;
    int AnimationId;
};

struct WINDOWCOMPOSITIONATTRIBDATA_WEATHER {
    int Attrib;
    PVOID pvData;
    SIZE_T cbData;
};

typedef BOOL (WINAPI *SetWindowCompositionAttribute_t)(HWND, WINDOWCOMPOSITIONATTRIBDATA_WEATHER*);

void ApplyWin10Acrylic(HWND hWnd) {
    if (IsWindows11() || !g_useAcrylic) return;
    HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
    if (hUser32) {
        auto pfnSetWindowCompositionAttribute = (SetWindowCompositionAttribute_t)GetProcAddress(hUser32, "SetWindowCompositionAttribute");
        if (pfnSetWindowCompositionAttribute) {
            ACCENT_POLICY_WEATHER accent = {0};
            accent.State = 4; // ACCENT_ENABLE_ACRYLICBLURBEHIND
            accent.Flags = 2; // Draw thin borders to avoid default thick top/left lines
            DWORD alpha = (g_acrylicOpacity * 255) / 100;
            BOOL isDark = IsSystemDarkMode();
            DWORD bgRgb = isDark ? 0x202020 : 0xE0E0E0;
            accent.Color = (alpha << 24) | bgRgb;
            
            WINDOWCOMPOSITIONATTRIBDATA_WEATHER data = {0};
            data.Attrib = 19; // WCA_ACCENT_POLICY
            data.pvData = &accent;
            data.cbData = sizeof(accent);
            pfnSetWindowCompositionAttribute(hWnd, &data);
        }
    }
}


void ShowWin10ForecastPopup(HWND hClock) {
    ThreadXamlState& state = GetThreadXamlState();
    if (state.popupHwnd && IsWindow(state.popupHwnd)) {
        PostMessageW(state.popupHwnd, WM_USER + 5001, (WPARAM)hClock, 0);
    }
}

void ShowWin10ForecastPopupInternal(HWND hClock) {
    ThreadXamlState& state = GetThreadXamlState();
    try {
        static thread_local bool winrtInit = []() {
            try { winrt::init_apartment(winrt::apartment_type::single_threaded); } catch(...) {}
            return true;
        }();
        (void)winrtInit;

        RegisterWin10PopupClass();

        if (!state.manager) {
            try {
                state.manager = winrt::Windows::UI::Xaml::Hosting::WindowsXamlManager::InitializeForCurrentThread();
            } catch (...) {
            }
        }

        double panelWidth = 336.0;
        if (g_forecastDaysFetch >= 14)
            panelWidth = 570.0;
        else if (g_forecastDaysFetch >= 10)
            panelWidth = 492.0;

        double panelHeight = 365.0;
        std::wstring displayWarning = g_activeWarning;
        if (!g_mockWeatherAlert.empty()) {
            displayWarning = g_mockWeatherAlert;
        }
        if (!displayWarning.empty()) {
            panelHeight += 32.0;
        }

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
            MONITORINFO mi = {sizeof(mi)};
            if (GetMonitorInfoW(hMon, &mi)) {
                if (trayWidth > trayHeight) {
                    if (trayRect.top <= mi.rcMonitor.top + (mi.rcMonitor.bottom - mi.rcMonitor.top) / 2) {
                        edge = ABE_TOP;
                    } else {
                        edge = ABE_BOTTOM;
                    }
                } else {
                    if (trayRect.left <= mi.rcMonitor.left + (mi.rcMonitor.right - mi.rcMonitor.left) / 2) {
                        edge = ABE_LEFT;
                    } else {
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
        } else if (edge == ABE_LEFT) {
            offsetX = clockRect.right + 10;
            offsetY = clockRect.top + (weatherWidth / 2.0) - (physicalHeight / 2.0);
        } else if (edge == ABE_RIGHT) {
            offsetX = clockRect.left - physicalWidth - 10;
            offsetY = clockRect.top + (weatherWidth / 2.0) - (physicalHeight / 2.0);
        }

        // Keep it on the correct monitor and work area
        HMONITOR hMonitor = MonitorFromRect(&clockRect, MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi = {sizeof(mi)};
        if (GetMonitorInfoW(hMonitor, &mi)) {
            RECT rcWork = mi.rcWork;
            if (offsetX < rcWork.left + 15) offsetX = rcWork.left + 15;
            if (offsetY < rcWork.top + 15) offsetY = rcWork.top + 15;
            if (offsetX + physicalWidth > rcWork.right - 15) offsetX = rcWork.right - physicalWidth - 15;
            if (offsetY + physicalHeight > rcWork.bottom - 15) offsetY = rcWork.bottom - physicalHeight - 15;
        } else {
            int screenWidth = GetSystemMetrics(SM_CXSCREEN);
            int screenHeight = GetSystemMetrics(SM_CYSCREEN);
            if (offsetX < 15) offsetX = 15;
            if (offsetY < 15) offsetY = 15;
            if (offsetX + physicalWidth > screenWidth - 15) offsetX = screenWidth - physicalWidth - 15;
            if (offsetY + physicalHeight > screenHeight - 15) offsetY = screenHeight - physicalHeight - 15;
        }

        bool needsRecreate = !state.popupHwnd || !IsWindow(state.popupHwnd) || !state.source || !state.sourceHwnd || !IsWindow(state.sourceHwnd);
        if (!needsRecreate) {
            try {
                (void)state.source.Content();
            } catch (...) {
                needsRecreate = true;
            }
        }

        if (needsRecreate) {
            if (state.source) { try { state.source.Close(); } catch(...) {} state.source = nullptr; }
            if (state.popupHwnd && IsWindow(state.popupHwnd)) DestroyWindow(state.popupHwnd);
            state.popupHwnd = nullptr;
            state.sourceHwnd = nullptr;

            // Include WS_EX_LAYERED for acrylic composition support on Windows 10
            DWORD dwExStyle = WS_EX_TOPMOST | WS_EX_TOOLWINDOW;
            if (!IsWindows11() && g_useAcrylic) {
                dwExStyle |= WS_EX_LAYERED | 0x00200000L; // WS_EX_NOREDIRECTIONBITMAP
            }
            HWND hOwner = hClock;
            if (!hOwner) {
                hOwner = FindWindowW(L"Shell_TrayWnd", NULL);
            }
            state.popupHwnd = CreateWindowExW(
                dwExStyle,
                L"WhWin10ForecastPopupClass", L"",
                WS_POPUP,
                (int)offsetX, (int)offsetY, physicalWidth, physicalHeight,
                hOwner, NULL, GetModuleHandleW(NULL), NULL
            );

            if (state.popupHwnd) {
                if (!IsWindows11() && g_useAcrylic) {
                    SetLayeredWindowAttributes(state.popupHwnd, 0, 255, LWA_ALPHA);
                }

                // Apply transparency and acrylic-supporting attributes IMMEDIATELY while hidden
                MARGINS margins = {0, 0, 0, 0};
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
                        nativeSource->Release();

                        SetWindowPos(state.popupHwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
                        InvalidateRect(state.popupHwnd, NULL, TRUE);
                        UpdateWindow(state.popupHwnd);
                    } else {
                        throw winrt::hresult_error(hr);
                    }
                } catch (...) {
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
        } else {
            MoveWindow(state.popupHwnd, (int)offsetX, (int)offsetY, physicalWidth, physicalHeight, TRUE);

            if (!IsWindows11() && g_useAcrylic) {
                SetLayeredWindowAttributes(state.popupHwnd, 0, 255, LWA_ALPHA);
            }

            MARGINS margins = {0, 0, 0, 0};
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
            g_selectedDate = L"";
            g_selectedHour = L"";
            g_selectedGraphTab = 0;
            g_graphScrollOffset = -1.0;
            g_dailyScrollOffset = 0.0;
            if (g_debugLogs) Wh_Log(L"[Wh_WeatherHost] Populating UI");
            PopulateForecastUI(rootGrid, g_cachedCondition, g_cachedIcon, g_cachedTemp);

            rootGrid.Width(panelWidth);
            rootGrid.Height(panelHeight);

            if (g_debugLogs) Wh_Log(L"[Wh_WeatherHost] Setting content");
            try {
                auto oldContent = state.source.Content();
                if (oldContent) {
                    CleanupXamlMedia(oldContent);
                }
            } catch (...) {}
            state.source.Content(rootGrid);

            if (state.sourceHwnd) {
                ShowWindow(state.sourceHwnd, SW_SHOW);
            }
            if (g_debugLogs) Wh_Log(L"[Wh_WeatherHost] Showing window");
            SetWindowPos(state.popupHwnd, NULL, (int)offsetX, (int)offsetY, physicalWidth, physicalHeight, SWP_NOZORDER | SWP_NOACTIVATE);
            ShowWindow(state.popupHwnd, SW_SHOW);
            SetForegroundWindow(state.popupHwnd);
            RedrawWindow(state.popupHwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);

            if (g_debugLogs) {
                Wh_Log(L"[Wh_WeatherHost] Opened Win10 forecast popup at offset (%f, %f) using XAML Island HWND=%p", offsetX, offsetY, state.popupHwnd);
            }
        }
    } catch (const winrt::hresult_error& e) {
        if (g_debugLogs) {
            Wh_Log(L"[Wh_WeatherHost] Error creating/showing Win10 forecast popup. WinRT Exception: %s (0x%08X).", e.message().c_str(), e.code().value);
        }
        if (state.source) { try { state.source.Close(); } catch(...) {} state.source = nullptr; }
        if (state.popupHwnd && IsWindow(state.popupHwnd)) DestroyWindow(state.popupHwnd);
        state.popupHwnd = nullptr;
        state.sourceHwnd = nullptr;
    } catch (const std::exception& e) {
        if (g_debugLogs) {
            Wh_Log(L"[Wh_WeatherHost] Error creating/showing Win10 forecast popup. Std Exception: %S.", e.what());
        }
        if (state.source) { try { state.source.Close(); } catch(...) {} state.source = nullptr; }
        if (state.popupHwnd && IsWindow(state.popupHwnd)) DestroyWindow(state.popupHwnd);
        state.popupHwnd = nullptr;
        state.sourceHwnd = nullptr;
    } catch (...) {
        if (g_debugLogs) {
            Wh_Log(L"[Wh_WeatherHost] Error creating/showing Win10 forecast popup. Unknown Exception.");
        }
        if (state.source) { try { state.source.Close(); } catch(...) {} state.source = nullptr; }
        if (state.popupHwnd && IsWindow(state.popupHwnd)) DestroyWindow(state.popupHwnd);
        state.popupHwnd = nullptr;
        state.sourceHwnd = nullptr;
    }
}

void InvalidateClockParentRegion(HWND hWnd) {
    // Improved parent-chain invalidation by bbmaster123 to perfectly clear visual remnants when resizing/shifting
    RECT rcWin;
    GetWindowRect(hWnd, &rcWin);
    
    // Expand the invalidation region slightly in all directions to catch and clean 
    // any remnants, ghost icons, or fragments left behind when the widget shifts or resizes.
    RECT rcExpanded = rcWin;
    rcExpanded.left -= 60;
    rcExpanded.top -= 60;
    rcExpanded.right += 60;
    rcExpanded.bottom += 60;
    
    // Walk up the parent chain and invalidate/redraw the region on every ancestor
    HWND hCurr = GetParent(hWnd);
    while (hCurr) {
        RECT rcMap = rcExpanded;
        MapWindowPoints(NULL, hCurr, (LPPOINT)&rcMap, 2);
        
        // Force repaint of the background of this ancestor in the specified region
        RedrawWindow(hCurr, &rcMap, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_ALLCHILDREN);
        
        hCurr = GetParent(hCurr);
    }
    
    // Also explicitly target Shell_TrayWnd just in case
    HWND hTaskbar = FindWindowW(L"Shell_TrayWnd", NULL);
    if (hTaskbar) {
        RECT rcMap = rcExpanded;
        MapWindowPoints(NULL, hTaskbar, (LPPOINT)&rcMap, 2);
        RedrawWindow(hTaskbar, &rcMap, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    }
}

void RestoreDefaultWindowSize(HWND hwndToClean) {
    if (g_currentAddedWeatherWidth > 0) {
        RECT rc = {0};
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
        } else {
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

LRESULT CALLBACK ClockSubclassWndProc(HWND hWnd,
                                      UINT uMsg,
                                      WPARAM wParam,
                                      LPARAM lParam,
                                      DWORD_PTR dwRefData) {
    if (uMsg == WM_HOTKEY) {
        if (wParam == 4242) {
            try {
                ThreadXamlState& state = GetThreadXamlState();
                if (state.popupHwnd && IsWindowVisible(state.popupHwnd)) {
                    g_lastHideTickCount = GetTickCount();
                    HideWin10Popup(state.popupHwnd);
                } else {
                    if (GetTickCount() - g_lastHideTickCount > 250) {
                        ShowWin10ForecastPopup(hWnd);
                    }
                }
            } catch (...) {}
            return 0;
        }
    }

    if (g_msgHotkeyControl && uMsg == g_msgHotkeyControl) {
        if (g_hotkeyRegistered) {
            if (g_hKeyboardHook) { UnhookWindowsHookEx(g_hKeyboardHook); g_hKeyboardHook = NULL; }
            g_hotkeyRegistered = false;
        }
        if (wParam == 1 && g_hkVk != 0) {
            if (!g_hKeyboardHook) {
                HMODULE hMod = NULL;
                GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR)LowLevelKeyboardProc, &hMod);
                g_hKeyboardHook = SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardProc, hMod, 0);
            }
            g_hotkeyRegistered = true;
        }
        return 0;
    }

    if (uMsg == (WM_USER + 4242)) {
        try {
            if (ShouldUseXamlTaskbar()) {
                if (auto button = g_weakXamlWeatherButton.get()) {
                    if (!g_win11FlyoutIsOpen) {
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
                                    } else {
                                        SetForegroundWindow(hRoot);
                                        SetActiveWindow(hRoot);
                                        SetFocus(hRoot);
                                    }
                                } else {
                                    SetForegroundWindow(hRoot);
                                    SetActiveWindow(hRoot);
                                    SetFocus(hRoot);
                                }

                                // Use undocumented SwitchToThisWindow to fully activate/foreground
                                typedef void (WINAPI *pfnSwitchToThisWindow)(HWND, BOOL);
                                HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
                                if (hUser32) {
                                    auto pSwitch = (pfnSwitchToThisWindow)GetProcAddress(hUser32, "SwitchToThisWindow");
                                    if (pSwitch) {
                                        pSwitch(hRoot, TRUE);
                                    }
                                }
                            }
                        }
                        try {
                            button.Focus(winrt::Windows::UI::Xaml::FocusState::Programmatic);
                        } catch (...) {}
                    }
                    if (g_showWin11Flyout) {
                        g_showWin11Flyout();
                    }
                }
                return 0;
            }

            if (wParam == 1) {
                ThreadXamlState& state = GetThreadXamlState();
                if (state.popupHwnd && IsWindow(state.popupHwnd)) {
                    g_lastHideTickCount = GetTickCount();
                    HideWin10Popup(state.popupHwnd);
                }
            } else {
                ThreadXamlState& state = GetThreadXamlState();
                if (state.popupHwnd && IsWindow(state.popupHwnd) && IsWindowVisible(state.popupHwnd)) {
                    g_lastHideTickCount = GetTickCount();
                    HideWin10Popup(state.popupHwnd);
                } else {
                    if (GetTickCount() - g_lastHideTickCount > 250) {
                        ShowWin10ForecastPopup(hWnd);
                    }
                }
            }
        } catch (...) {}
        return 0;
    }

    if (uMsg == (WM_USER + 4244)) {
        ThreadXamlState& state = GetThreadXamlState();
        if (state.popupHwnd && IsWindow(state.popupHwnd)) {
            PostMessageW(state.popupHwnd, WM_USER + 5003, 0, 0);
        }
        return 0;
    }

    if (!ShouldUseXamlTaskbar()) {
        if (uMsg == WM_WINDOWPOSCHANGING) {
            WINDOWPOS* lpwp = (WINDOWPOS*)lParam;
            if (g_inInternalResize) return DefSubclassProc(hWnd, uMsg, wParam, lParam);

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

            bool isHorizontal = curCx > curCy;

            if (g_debugLogs) {
                Wh_Log(L"[Wh_WeatherHost] WM_WINDOWPOSCHANGING entered: lpwp->flags=0x%08X, proposed cx=%d, cy=%d, x=%d, y=%d, curAddedWidth=%d, targetAddedWidth=%d, isHorizontal=%d",
                       lpwp->flags, lpwp->cx, lpwp->cy, lpwp->x, lpwp->y, g_currentAddedWeatherWidth, weatherWidth, isHorizontal);
            }

            int cleanCx = 0;
            int cleanCy = 0;
            if (!(lpwp->flags & SWP_NOSIZE)) {
                cleanCx = lpwp->cx;
                cleanCy = lpwp->cy;
            } else {
                if (isHorizontal) {
                    cleanCx = curCx - g_currentAddedWeatherWidth;
                    cleanCy = curCy;
                } else {
                    cleanCx = curCx;
                    cleanCy = curCy - g_currentAddedWeatherWidth;
                }
            }

            int cleanX = 0;
            int cleanY = 0;
            if (!(lpwp->flags & SWP_NOMOVE)) {
                cleanX = lpwp->x;
                cleanY = lpwp->y;
            } else {
                if (isHorizontal) {
                    cleanX = curX + g_currentAddedWeatherWidth;
                    cleanY = curY;
                } else {
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
                } else {
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
            lpwp->flags |= SWP_NOCOPYBITS;

            if (g_debugLogs) {
                Wh_Log(L"[Wh_WeatherHost] WM_WINDOWPOSCHANGING adjusted: flags=0x%08X, adjusted cx=%d, cy=%d, x=%d, y=%d",
                       lpwp->flags, lpwp->cx, lpwp->cy, lpwp->x, lpwp->y);
            }

            return DefSubclassProc(hWnd, uMsg, wParam, lParam);
        } else if (uMsg == WM_TIMER && wParam == 1234) {
            KillTimer(hWnd, 1234);
            HDC hdc = GetDCEx(hWnd, NULL, DCX_WINDOW | DCX_CACHE);
            if (hdc) {
                PaintWin10Weather(hWnd, hdc);
                ReleaseDC(hWnd, hdc);
            }
            return 0;
        } else if (uMsg == WM_WINDOWPOSCHANGED) {
            LRESULT res = DefSubclassProc(hWnd, uMsg, wParam, lParam);
            InvalidateClockParentRegion(hWnd);
            RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
            HDC hdc = GetDCEx(hWnd, NULL, DCX_WINDOW | DCX_CACHE);
            if (hdc) {
                PaintWin10Weather(hWnd, hdc);
                ReleaseDC(hWnd, hdc);
            }
            ThreadXamlState& state = GetThreadXamlState();
            if (state.popupHwnd && IsWindowVisible(state.popupHwnd)) {
                try {
                    ShowWin10ForecastPopup(hWnd);
                } catch (...) {}
            }
            return res;
        } else if (uMsg == WM_NCCALCSIZE) {
            LRESULT res = DefSubclassProc(hWnd, uMsg, wParam, lParam);
            int weatherWidth = GetRequiredWeatherWidth(hWnd);
            if (g_modUnloaded) weatherWidth = 0;
            
            RECT* clientRect = nullptr;
            if (wParam == TRUE) {
                NCCALCSIZE_PARAMS* pnc = (NCCALCSIZE_PARAMS*)lParam;
                clientRect = &pnc->rgrc[0];
            } else {
                clientRect = (RECT*)lParam;
            }
            
            bool isHorizontal = (clientRect->right - clientRect->left) > (clientRect->bottom - clientRect->top);
            if (isHorizontal) {
                clientRect->left += weatherWidth;
            } else {
                clientRect->top += weatherWidth;
            }
            return res;
        } else if (uMsg == WM_SIZE || uMsg == WM_MOVE || uMsg == WM_NOTIFY || uMsg == WM_COMMAND) {
            LRESULT res = DefSubclassProc(hWnd, uMsg, wParam, lParam);
            if (uMsg == WM_SIZE || uMsg == WM_MOVE) {
                RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW | RDW_ALLCHILDREN);
            }
            HDC hdc = GetDCEx(hWnd, NULL, DCX_WINDOW | DCX_CACHE);
            if (hdc) {
                PaintWin10Weather(hWnd, hdc);
                ReleaseDC(hWnd, hdc);
            }
            return res;
        } else if (uMsg == WM_NCPAINT || uMsg == WM_PAINT || uMsg == 0x0318 /*WM_PRINTCLIENT*/ || uMsg == WM_PRINT) {
            LRESULT res = DefSubclassProc(hWnd, uMsg, wParam, lParam);
            HDC hdc = GetDCEx(hWnd, NULL, DCX_WINDOW | DCX_CACHE);
            if (hdc) {
                PaintWin10Weather(hWnd, hdc);
                ReleaseDC(hWnd, hdc);
            }
            return res;
        } else if (uMsg == WM_ERASEBKGND) {
            return TRUE;
        } else if (uMsg == WM_NCHITTEST) {
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
            } else {
                if (pt.y >= -weatherWidth && pt.y < 0) {
                    return HTCLIENT;
                }
            }
            return DefSubclassProc(hWnd, uMsg, wParam, lParam);
        } else if (uMsg == WM_MOUSEMOVE) {
            RECT rc;
            GetClientRect(hWnd, &rc);
            int height = rc.bottom - rc.top;
            int width = rc.right - rc.left;
            bool isHorizontal = width >= height;

            int pos = isHorizontal ? GET_X_LPARAM(lParam) : GET_Y_LPARAM(lParam);
            int weatherWidth = g_currentAddedWeatherWidth;
            bool overBtn = (pos >= -weatherWidth && pos < 0);

            if (!g_win10TrackingMouse && overBtn) {
                TRACKMOUSEEVENT tme = {sizeof(tme), TME_LEAVE, hWnd, 0};
                TrackMouseEvent(&tme);
                g_win10TrackingMouse = true;
            }

            if (overBtn != g_win10WeatherHovered) {
                g_win10WeatherHovered = overBtn;
                InvalidateClockParentRegion(hWnd);
                RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW);
            }
            return DefSubclassProc(hWnd, uMsg, wParam, lParam);
        } else if (uMsg == WM_MOUSELEAVE) {
            g_win10WeatherHovered = false;
            g_win10WeatherPressed = false;
            g_win10TrackingMouse = false;
            InvalidateClockParentRegion(hWnd);
            RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW);
            return DefSubclassProc(hWnd, uMsg, wParam, lParam);
        } else if (uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONDBLCLK) {
            RECT rc;
            GetClientRect(hWnd, &rc);
            bool isHorizontal = (rc.right - rc.left) >= (rc.bottom - rc.top);
            int pos = isHorizontal ? GET_X_LPARAM(lParam) : GET_Y_LPARAM(lParam);
            int weatherWidth = g_currentAddedWeatherWidth;

            if (pos >= -weatherWidth && pos < 0) {
                ThreadXamlState& state = GetThreadXamlState();
                g_popupWasVisibleOnLButtonDown = (state.popupHwnd && IsWindow(state.popupHwnd) && IsWindowVisible(state.popupHwnd));
                g_win10WeatherPressed = true;
                InvalidateClockParentRegion(hWnd);
                RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW);
                SetCapture(hWnd);
                return 0;
            }
            return DefSubclassProc(hWnd, uMsg, wParam, lParam);
        } else if (uMsg == WM_LBUTTONUP) {
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
                RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW);
                PostMessageW(hWnd, WM_USER + 4242, g_popupWasVisibleOnLButtonDown ? 1 : 0, 0);
                return 0;
            }
            return DefSubclassProc(hWnd, uMsg, wParam, lParam);
        } else if (uMsg == (WM_USER + 4243)) {
            HideWin10Popup(hWnd);
            return 0;
        }
    } else {
        if (uMsg == WM_WINDOWPOSCHANGED || uMsg == WM_SIZE ||
            uMsg == WM_SHOWWINDOW) {
            Grid targetGrid = nullptr;
            {
                std::lock_guard<std::mutex> lock(g_weatherGridMutex);
                if (g_injectedWeatherGrid)
                    targetGrid = g_injectedWeatherGrid.get();
            }
            if (targetGrid) {
                try {
                    auto dispatcher = targetGrid.Dispatcher();
                    if (dispatcher) {
                        auto weakGrid = winrt::make_weak(targetGrid);
                        dispatcher.RunAsync(
                            winrt::Windows::UI::Core::CoreDispatcherPriority::Normal,
                            [weakGrid]() {
                                try {
                                    if (auto grid = weakGrid.get()) {
                                        UpdateWeatherXamlElements(grid, g_cachedTemp, g_cachedIcon, g_cachedCondition, g_weatherAcquired);
                                    }
                                } catch (...) {}
                            });
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
        g_hSubclassedWnd = hTarget;
        g_currentAddedWeatherWidth = 0;
        g_cleanCx = 0;
        g_cleanCy = 0;
        g_cleanX = 0;
        g_cleanY = 0;
        if (WindhawkUtils::SetWindowSubclassFromAnyThread(hTarget, ClockSubclassWndProc, NULL)) {
            if (g_debugLogs) {
                Wh_Log(
                    L"[Wh_WeatherHost] Successfully subclassed Target HWND=%p "
                    L"(XamlTaskbar=%d)",
                    hTarget, ShouldUseXamlTaskbar());
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
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            KBDLLHOOKSTRUCT* pKbd = (KBDLLHOOKSTRUCT*)lParam;
            if (pKbd->vkCode == g_hkVk) {
                bool win = (GetAsyncKeyState(VK_LWIN) & 0x8000) || (GetAsyncKeyState(VK_RWIN) & 0x8000);
                bool alt = (GetAsyncKeyState(VK_LMENU) & 0x8000) || (GetAsyncKeyState(VK_RMENU) & 0x8000);
                bool ctrl = (GetAsyncKeyState(VK_LCONTROL) & 0x8000) || (GetAsyncKeyState(VK_RCONTROL) & 0x8000);
                bool shift = (GetAsyncKeyState(VK_LSHIFT) & 0x8000) || (GetAsyncKeyState(VK_RSHIFT) & 0x8000);
                
                if (win == g_hkWin && alt == g_hkAlt && ctrl == g_hkCtrl && shift == g_hkShift) {
                    if (g_hSubclassedWnd) {
                        PostMessageW(g_hSubclassedWnd, WM_USER + 4242, 0, 0);
                    }
                    return 1; // Block the keypress from reaching the system
                }
            }
        }
    }
    return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
}
void LoadModConfiguration() {
    PCWSTR city = Wh_GetStringSetting(L"location");
    g_location = city ? city : L"auto";
    Wh_FreeStringSetting(city);

    PCWSTR mockAlertStr = Wh_GetStringSetting(L"mockWeatherAlert");
    g_mockWeatherAlert = mockAlertStr ? mockAlertStr : L"";
    Wh_FreeStringSetting(mockAlertStr);

    g_useCelsius = Wh_GetIntSetting(L"useCelsius") != 0;

    g_updateInterval = Wh_GetIntSetting(L"updateInterval");
    if (g_updateInterval < 5)
        g_updateInterval = 5;
    if (g_updateInterval > 120)
        g_updateInterval = 120;

    g_weatherStyle = Wh_GetIntSetting(L"weatherStyle");
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

    g_density = 2; // Keep most compact option
    g_debugLogs = true; // Disable unneeded logging

    g_injectToSysTray = Wh_GetIntSetting(L"injectToSysTray") != 0;
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
        } else if (s == L"fill" || s == L"1") {
            g_bgImageVideoStretch = 1;
        } else if (s == L"uniform" || s == L"2" || s == L"fit" || s == L"uniformtofit") {
            g_bgImageVideoStretch = 2;
        } else if (s == L"uniformtofill" || s == L"uniform_to_fill" || s == L"3" || s == L"crop") {
            g_bgImageVideoStretch = 3;
        }
        Wh_FreeStringSetting(bgStretchStr);
    }

    g_forecastDaysFetch = Wh_GetIntSetting(L"forecastDaysFetch");
    if (g_forecastDaysFetch < 1)
        g_forecastDaysFetch = 7;
    if (g_forecastDaysFetch > 16)
        g_forecastDaysFetch = 16;

    int scalePct = Wh_GetIntSetting(L"panelFontScale");
    if (scalePct <= 0) scalePct = 100;
    g_panelFontScale = scalePct / 100.0;

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
        } else if (keyStr == L"F1") g_hkVk = VK_F1;
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
    } else {
        g_textColor = RGB(255, 255, 255);
    }
    Wh_FreeStringSetting(colStr);

    if (g_debugLogs)
        Wh_Log(
            L"[EP_WeatherHost] Reloaded settings: loc=%s, metric=%d, fontS=%d",
            g_location.c_str(), g_useCelsius, g_fontSize);

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
    g_isShellProcess = IsShellProcess();
    if (!g_isShellProcess) {
        return TRUE;
    }

    BufferedPaintInit();

    g_msgHotkeyControl = RegisterWindowMessageW(L"WhWeatherHost_HotkeyControl");

    InitializeCriticalSection(&g_forecastLock);
    InitializeCriticalSection(&g_subclassLock);
    g_hForceUpdateEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    LoadModConfiguration();

    // Create dedicated popup STA thread
    g_hPopupThread = CreateThread(NULL, 0, PopupThreadProc, NULL, 0, &g_dwPopupThreadId);

    AlignOverlayWindow();

    g_bThreadShouldTerm = false;

    // Background weather crawl
    g_hQueryThread =
        CreateThread(NULL, 0, QueryWeatherPipeline, NULL, 0, &g_dwThreadId);

    // Watchdog thread for rapid, automated taskbar subclass injection/maintenance
    g_hWatchdogThread = CreateThread(NULL, 0, SubclassWatchdogThread, NULL, 0, NULL);

    // Initial check if taskbar module is loaded or intercept via library load
    // hooks
    HMODULE mod = GetModuleHandle(L"Taskbar.View.dll");
    if (!mod)
        mod = GetModuleHandle(L"ExplorerExtensions.dll");
    if (mod) {
        g_taskbarViewDllLoaded = true;
        HookTaskbarViewDllSymbols(mod);
    } else {
        WindhawkUtils::SetFunctionHook(LoadLibraryExW, LoadLibraryExW_Hook,
                                       &LoadLibraryExW_Original);
        HMODULE hNt = GetModuleHandleW(L"ntdll.dll");
        if (hNt) {
            void* pLdrLoadDll = (void*)GetProcAddress(hNt, "LdrLoadDll");
            if (pLdrLoadDll) {
                WindhawkUtils::SetFunctionHook(pLdrLoadDll, (void*)LdrLoadDll_Hook,
                                               (void**)&LdrLoadDll_Original);
            }
        }
    }

    HWND hTaskbar = FindWindowW(L"Shell_TrayWnd", NULL);
    if (hTaskbar) {
        PostMessageW(hTaskbar, WM_SETTINGCHANGE, 0, (LPARAM)L"ImmersiveColorSet");
        PostMessageW(hTaskbar, WM_THEMECHANGED, 0, 0);
    }

    // Force tasks to refresh immediately so scan triggers without clicking
    ForceTaskbarUpdateOriginal();

    if (g_debugLogs)
        Wh_Log(
            L"[EP_WeatherHost] Modern Native XAML Injection Mod successfully "
            L"loaded.");
    return TRUE;
}

// Windhawk mod Exit Point
BOOL CALLBACK CleanupXamlWindowsCallback(HWND hWnd, LPARAM lParam) {
    WCHAR className[256];
    if (GetClassNameW(hWnd, className, 256) && wcscmp(className, L"WhWin10ForecastPopupClass") == 0) {
        SendMessageW(hWnd, WM_USER + 4246, 0, 0);
    }
    return TRUE;
}

void Wh_ModUninit() {
    if (!g_isShellProcess) {
        return;
    }

    try {
        g_showWin11Flyout = nullptr;
    } catch (...) {}
    try {
        g_weakXamlWeatherButton = nullptr;
    } catch (...) {}

    if (g_activeFlyout) {
        try {
            g_activeFlyout.Hide();
        } catch (...) {}
        try {
            g_activeFlyout = nullptr;
        } catch (...) {}
    }

    if (g_debugLogs)
        Wh_Log(L"[EP_WeatherHost] Unloading mod...");

    g_bThreadShouldTerm = true;
    if (g_hForceUpdateEvent) SetEvent(g_hForceUpdateEvent);

    // 1. Safely remove subclassing and restore layout
    EnterCriticalSection(&g_subclassLock);
    if (g_hSubclassedWnd) {
        g_modUnloaded = true;
        HWND hwndToClean = g_hSubclassedWnd;

        if (g_hKeyboardHook) { UnhookWindowsHookEx(g_hKeyboardHook); g_hKeyboardHook = NULL; }
        KillTimer(hwndToClean, 1234);
        RestoreDefaultWindowSize(hwndToClean);
        WindhawkUtils::RemoveWindowSubclassFromAnyThread(hwndToClean, ClockSubclassWndProc);
        g_hSubclassedWnd = NULL;
        g_currentAddedWeatherWidth = 0;
    }
    LeaveCriticalSection(&g_subclassLock);

    // 2. Clean up XAML and its child windows directly across all threads
    EnumWindows(CleanupXamlWindowsCallback, 0);

    // 3. Wait for background threads to terminate
    if (g_hQueryThread) {
        WaitForSingleObject(g_hQueryThread, 5000);
        CloseHandle(g_hQueryThread);
        g_hQueryThread = NULL;
    }

    if (g_hWatchdogThread) {
        WaitForSingleObject(g_hWatchdogThread, 2000);
        CloseHandle(g_hWatchdogThread);
        g_hWatchdogThread = NULL;
    }

    if (g_hPopupThread) {
        WaitForSingleObject(g_hPopupThread, 3000);
        CloseHandle(g_hPopupThread);
        g_hPopupThread = NULL;
    }

    if (g_hForceUpdateEvent) {
        CloseHandle(g_hForceUpdateEvent);
        g_hForceUpdateEvent = NULL;
    }

    // Give time for window destruction messages to process before unregistering the class
    Sleep(50);
    UnregisterClassW(L"WhWin10ForecastPopupClass", GetModuleHandleW(NULL));


    try {
        g_activeFlyout = nullptr;
    } catch (...) {}

    Grid targetGrid = nullptr;
    {
        std::lock_guard<std::mutex> lock(g_weatherGridMutex);
        if (g_injectedWeatherGrid)
            targetGrid = g_injectedWeatherGrid.get();
    }
    if (targetGrid) {
        try {
            auto dispatcher = targetGrid.Dispatcher();
            if (dispatcher.HasThreadAccess()) {
                if (auto parent = targetGrid.Parent().try_as<Grid>()) {
                    RemoveInjectedFromGrid(parent);
                }
            } else {
                auto weakGrid = winrt::make_weak(targetGrid);
                auto action = dispatcher.RunAsync(
                    winrt::Windows::UI::Core::CoreDispatcherPriority::Normal,
                    [weakGrid]() {
                        try {
                            if (auto grid = weakGrid.get()) {
                                if (auto parent = grid.Parent().try_as<Grid>()) {
                                    RemoveInjectedFromGrid(parent);
                                }
                            }
                        } catch (...) {
                        }
                    });
                try {
                    // action.get(); // Wait synchronously for completion instead of timing out
                } catch (...) {
                }
            }
        } catch (...) {
        }
    }

    DeleteCriticalSection(&g_forecastLock);
    DeleteCriticalSection(&g_subclassLock);

    if (g_pDWriteFactory) {
        g_pDWriteFactory->Release();
        g_pDWriteFactory = nullptr;
    }
    if (g_pD2DFactory) {
        g_pD2DFactory->Release();
        g_pD2DFactory = nullptr;
    }

    BufferedPaintUnInit();
}

// Settings update receiver
void Wh_ModSettingsChanged() {
    if (!g_isShellProcess) {
        return;
    }
    LoadModConfiguration();
    AlignOverlayWindow();
    if (g_hSubclassedWnd && IsWindow(g_hSubclassedWnd)) {
        SetWindowPos(g_hSubclassedWnd, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    }
    ForceTaskbarUpdateOriginal();
}
