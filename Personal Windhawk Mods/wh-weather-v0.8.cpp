// ==WindhawkMod==
// @id              ep-weather-host
// @name            EP Weather Host
// @description     A high-performance modern native taskbar weather widget that injects directly into the Windows Taskbar UI Tree, featuring auto IP geolocation, customizable cities, Open-Meteo API integration, and beautiful Segoe/Emoji indicators.
// @version         2.0.0
// @author          Gemini
// @include         explorer.exe
// @compilerOptions -DWINVER=0x0A00 -lgdi32 -luser32 -luxtheme -lwinhttp -lshlwapi -lole32 -luuid -lshell32 -loleaut32 -ldwmapi -lruntimeobject -lshcore -lversion
// ==/WindhawkMod==

// ==WindhawkModSettings==
/*
- location: "auto"
  $name: Location (City Name or "auto" for IP-based Geolocation)
- useCelsius: 0
  $name: Use Metric System (°C, km/h)
  $description: Uncheck to use Imperial System (°F, mph)
- updateInterval: 15
  $name: Update Interval (Minutes)
  $description: Frequency of pulling weather data from the API (between 5 and 120 minutes)
- weatherStyle: 0
  $name: Weather Icon Style
  $description: 0 for Colorful Modern Emoji icons, 1 for Segoe MDL2 Assets vector symbols
- showConditionName: 1
  $name: Show Condition Name (e.g. "Sunny", "Cloudy")
- textOffset: 10
  $name: Horizontal X Offset from Taskbar Clock (Pixels)
- textColor: "#FFFFFF"
  $name: Text Color
- fontSize: 13
  $name: Weather Text Font Size
- iconFontSize: 16
  $name: Weather Icon Font Size
- useAcrylic: 1
  $name: Enable Acrylic Effect in Forecast Panel
- acrylicOpacity: 85
  $name: Acrylic Opacity (0-100)
- forecastDaysFetch: 14
  $name: Forecast API query length (Days, max 16)
- debugLogs: 1
  $name: Write Detailed Debug Logs to Windhawk
*/
// ==/WindhawkModSettings==

#include <windhawk_utils.h>
#include <windows.h>
#include <string>
#include <vector>
#include <winhttp.h>
#include <shlwapi.h>
#include <roapi.h>
#include <winstring.h>

#undef GetCurrentTime

#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt/Windows.UI.Xaml.Shapes.h>
#include <winrt/Windows.UI.Xaml.Interop.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.UI.Xaml.Media.Animation.h>
#include <algorithm>
#include <winrt/Windows.UI.Xaml.Input.h>
#include <winrt/Windows.UI.Text.h>
#include <winrt/base.h>

#include <atomic>
#include <mutex>
#include <chrono>

using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Media;

// Global settings
std::wstring g_location;
bool g_useCelsius = false;
int g_updateInterval = 15;
int g_weatherStyle = 0; // 0 = Emoji, 1 = Segoe MDL2
bool g_showConditionName = true;
bool g_useAcrylic = true;
int g_acrylicOpacity = 85;
int g_forecastDaysFetch = 14;
int g_textOffset = 10;
COLORREF g_textColor = RGB(255, 255, 255);
int g_fontSize = 13;
int g_iconFontSize = 16;
bool g_debugLogs = true;

// Weather state Cached values
std::wstring g_cachedTemp = L"--°F";
std::wstring g_cachedCondition = L"Loading";
std::wstring g_cachedIcon = L"⏳";
std::wstring g_displayCity = L"Detecting...";
double g_cachedLatitude = 40.7128; // New York Default
double g_cachedLongitude = -74.0060;
bool g_weatherAcquired = false;

// Background Thread values
HANDLE g_hQueryThread = NULL;
DWORD g_dwThreadId = 0;
bool g_bThreadShouldTerm = false;
HANDLE g_hForceUpdateEvent = NULL;

// Forecast Definitions and Structures
struct DailyForecast {
    std::wstring dayName;
    std::wstring icon;
    std::wstring condition;
    std::wstring tempMax;
    std::wstring tempMin;
    std::wstring rawDate;
};

struct HourlyForecast {
    std::wstring timeString;
    std::wstring icon;
    std::wstring temp;
    std::wstring precipProb;
    std::wstring rawDate;
    double tempRaw;
    double precipRaw;
    double windkphRaw;
};

std::vector<DailyForecast> g_forecastDaily;
std::vector<HourlyForecast> g_forecastHourly;
std::wstring g_cachedWindSpeed = L"--";
std::wstring g_cachedPrecipProb = L"0%";
std::wstring g_cachedHumidity = L"0%";
CRITICAL_SECTION g_forecastLock;
bool g_forecastAcquired = false;

std::pair<std::wstring, std::wstring> GetCodeMapping(int code);

// Injected Grid reference
winrt::weak_ref<Grid> g_injectedWeatherGrid = nullptr;
std::mutex g_weatherGridMutex;


struct ClockSearchData { HWND hClock; };

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
    if (!hShell) return NULL;

    HWND hTray = FindWindowExW(hShell, NULL, L"TrayNotifyWnd", NULL);
    if (hTray) {
        HWND hClock = FindWindowExW(hTray, NULL, L"TrayClockWClass", NULL);
        if (hClock) return hClock;
    }

    ClockSearchData data = { NULL };
    EnumChildWindows(hShell, FindClockEnumChildProc, (LPARAM)&data);

    return data.hClock;
}

struct AnchorSearchData { HWND hAnchor; };

static BOOL CALLBACK FindAnchorEnumChildProc(HWND hwnd, LPARAM lParam) {
    WCHAR className[256];
    GetClassNameW(hwnd, className, 256);
    if (wcscmp(className, L"TrayNotifyWnd") == 0) {
        ((AnchorSearchData*)lParam)->hAnchor = hwnd;
        return FALSE;
    }
    if (wcscmp(className, L"TrayClockWClass") == 0) {
        if (!((AnchorSearchData*)lParam)->hAnchor) {
            ((AnchorSearchData*)lParam)->hAnchor = hwnd;
        }
    }
    return TRUE;
}

// Find the System Tray area or fall back to system clock window as alignment anchor
HWND FindSystemAnchorWnd() {
    HWND hShell = FindWindowW(L"Shell_TrayWnd", NULL);
    if (!hShell) return NULL;

    HWND hTray = FindWindowExW(hShell, NULL, L"TrayNotifyWnd", NULL);
    if (hTray && IsWindowVisible(hTray)) {
        return hTray;
    }

    // Fallback search
    AnchorSearchData data = { NULL };
    EnumChildWindows(hShell, FindAnchorEnumChildProc, (LPARAM)&data);
    return data.hAnchor ? data.hAnchor : hShell;
}

// Update margin of injected XAML Grid dynamically based on real-time taskbar layout state
void UpdateInjectedWeatherLayout(Grid weatherGrid) {
    if (!weatherGrid) return;
    try {
        HWND hAnchor = FindSystemAnchorWnd();
        if (hAnchor) {
            RECT anchorRect;
            GetWindowRect(hAnchor, &anchorRect);
            
            // Resolve correct parent taskbar window (primary or secondary for multi-monitor)
            HWND hParentTaskbar = GetAncestor(hAnchor, GA_ROOT);
            if (!hParentTaskbar) hParentTaskbar = FindWindowW(L"Shell_TrayWnd", NULL);
            
            if (hParentTaskbar) {
                RECT trayRect;
                GetWindowRect(hParentTaskbar, &trayRect);
                
                int marginFromRight = trayRect.right - anchorRect.left + g_textOffset;
                bool isHorizontal = (trayRect.right - trayRect.left) > (trayRect.bottom - trayRect.top);
                if (isHorizontal) {
                    weatherGrid.HorizontalAlignment(HorizontalAlignment::Right);
                    weatherGrid.VerticalAlignment(VerticalAlignment::Center);
                    weatherGrid.Margin(Thickness{ 0, 0, (double)marginFromRight, 0 });
                } else {
                    weatherGrid.HorizontalAlignment(HorizontalAlignment::Center);
                    weatherGrid.VerticalAlignment(VerticalAlignment::Bottom);
                    weatherGrid.Margin(Thickness{ 0, 0, 0, (double)(trayRect.bottom - anchorRect.top + g_textOffset) });
                }
            }
        } else {
            weatherGrid.HorizontalAlignment(HorizontalAlignment::Right);
            weatherGrid.VerticalAlignment(VerticalAlignment::Center);
            weatherGrid.Margin(Thickness{ 0, 0, 100, 0 });
        }
    } catch (...) {}
}


// State for interactive flyout
std::wstring g_selectedDate = L"";
int g_selectedGraphTab = 0; // 0=Temp, 1=Precip, 2=Wind


bool IsSystemDarkMode() {
    DWORD data = 1;
    DWORD dataSize = sizeof(data);
    LSTATUS status = RegGetValueW(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        L"SystemUsesLightTheme",
        RRF_RT_REG_DWORD,
        nullptr,
        &data,
        &dataSize
    );
    if (status == ERROR_SUCCESS) {
        return data == 0;
    }
    return true; // Default to dark 
}

void PopulateForecastUI(winrt::Windows::UI::Xaml::Controls::Grid rootGrid, std::wstring condition, std::wstring currentIcon, std::wstring currentTemp);


winrt::Windows::UI::Xaml::Controls::Flyout CreateForecastFlyout(std::wstring condition, std::wstring currentIcon, std::wstring currentTemp) {
    using namespace winrt::Windows::UI::Xaml;
    using namespace winrt::Windows::UI::Xaml::Controls;
    using namespace winrt::Windows::UI::Xaml::Media;

    Flyout flyout;
    Grid rootGrid;
    
    // Clear selection state on new open
    g_selectedDate = L""; 
    g_selectedGraphTab = 0;
    
    PopulateForecastUI(rootGrid, condition, currentIcon, currentTemp);
    
    flyout.Content(rootGrid);

    // Make the flyout background transparent so our custom Grid shapes it
    winrt::Windows::UI::Xaml::Style flyoutStyle(winrt::xaml_typename<winrt::Windows::UI::Xaml::Controls::FlyoutPresenter>());
    winrt::Windows::UI::Xaml::Setter bgSetter(winrt::Windows::UI::Xaml::Controls::Control::BackgroundProperty(), winrt::box_value(SolidColorBrush{ winrt::Windows::UI::Colors::Transparent() }));
    winrt::Windows::UI::Xaml::Setter padSetter(winrt::Windows::UI::Xaml::Controls::Control::PaddingProperty(), winrt::box_value(Thickness{0,0,0,0}));
    winrt::Windows::UI::Xaml::Setter borSetter(winrt::Windows::UI::Xaml::Controls::Control::BorderThicknessProperty(), winrt::box_value(Thickness{0,0,0,0}));
    // Override MaxWidth to allow wider flyout panels
    winrt::Windows::UI::Xaml::Setter maxWidthSetter(winrt::Windows::UI::Xaml::FrameworkElement::MaxWidthProperty(), winrt::box_value((double)9999.0));
    
    flyoutStyle.Setters().Append(bgSetter);
    flyoutStyle.Setters().Append(maxWidthSetter);
    flyoutStyle.Setters().Append(padSetter);
    flyoutStyle.Setters().Append(borSetter);
    flyout.FlyoutPresenterStyle(flyoutStyle);
    
    try {
        flyout.ShouldConstrainToRootBounds(false);
        flyout.Placement(winrt::Windows::UI::Xaml::Controls::Primitives::FlyoutPlacementMode::Top);
        flyout.AllowFocusOnInteraction(true); // Allow interactions
        flyout.OverlayInputPassThroughElement(winrt::Windows::UI::Xaml::Controls::Primitives::FlyoutBase::GetAttachedFlyout(rootGrid));
    } catch (...) {}

    return flyout;
}

void PopulateForecastUI(winrt::Windows::UI::Xaml::Controls::Grid rootGrid, std::wstring condition, std::wstring currentIcon, std::wstring currentTemp) {
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

    if (!localAcquired) return;
    
    
    bool isInitial = false;
    if (g_selectedDate.empty() && !localDaily.empty()) {
        g_selectedDate = localDaily[0].rawDate;
        isInitial = true;
    }
    
    bool isDark = IsSystemDarkMode();
    
    auto primaryColor = isDark ? winrt::Windows::UI::Colors::White() : winrt::Windows::UI::Colors::Black();
    auto secondaryColor = isDark ? winrt::Windows::UI::ColorHelper::FromArgb(255, 200, 200, 200) : winrt::Windows::UI::ColorHelper::FromArgb(255, 90, 90, 90);
    auto tertiaryColor = isDark ? winrt::Windows::UI::ColorHelper::FromArgb(255, 150, 150, 150) : winrt::Windows::UI::ColorHelper::FromArgb(255, 120, 120, 120);
    auto bgColor = isDark ? winrt::Windows::UI::ColorHelper::FromArgb(255, 30, 30, 30) : winrt::Windows::UI::ColorHelper::FromArgb(255, 240, 240, 240);
    auto dividerColor = isDark ? winrt::Windows::UI::ColorHelper::FromArgb(40, 255, 255, 255) : winrt::Windows::UI::ColorHelper::FromArgb(20, 0, 0, 0);
    auto cardBgColor = isDark ? winrt::Windows::UI::ColorHelper::FromArgb(30, 255, 255, 255) : winrt::Windows::UI::ColorHelper::FromArgb(15, 0, 0, 0);
    auto activeTabColor = isDark ? winrt::Windows::UI::ColorHelper::FromArgb(255, 129, 212, 250) : winrt::Windows::UI::ColorHelper::FromArgb(255, 0, 120, 215);

    rootGrid.Children().Clear();

    
    int panelWidth = 304; // 4 cards: 4 * 62 + 3 * 8 + 32 = 304
    if (g_forecastDaysFetch >= 14) panelWidth = 584; // 8 cards: 8 * 62 + 7 * 8 + 32 = 584
    else if (g_forecastDaysFetch >= 10) panelWidth = 444; // 6 cards: 6 * 62 + 5 * 8 + 32 = 444
    rootGrid.Width(panelWidth);
    rootGrid.MaxWidth(9999.0);

    rootGrid.CornerRadius(CornerRadius{ 8.0, 8.0, 8.0, 8.0 });
    
    // Attempt Acrylic Backdrop
    try {
        if (!g_useAcrylic) throw std::exception();
        AcrylicBrush acrylic;
        acrylic.BackgroundSource(AcrylicBackgroundSource::HostBackdrop);
        acrylic.TintColor(bgColor);
        acrylic.TintOpacity(g_acrylicOpacity / 100.0);
        acrylic.FallbackColor(bgColor);
        rootGrid.Background(acrylic);
    } catch (...) {
        rootGrid.Background(SolidColorBrush{ bgColor });
    }

    StackPanel mainStack;
    mainStack.Orientation(Orientation::Vertical);
    mainStack.Padding(Thickness{ 16, 16, 16, 16 });
    mainStack.Spacing(12.0);
    
    // Add entrance animations
    winrt::Windows::UI::Xaml::Media::Animation::TransitionCollection transitions;
    winrt::Windows::UI::Xaml::Media::Animation::EntranceThemeTransition entrance;
    transitions.Append(entrance);
    
    if (isInitial) {
        mainStack.ChildrenTransitions(transitions);
    }
    

    // --- Header Row
    TextBlock cityTitle;
    cityTitle.Text(g_displayCity);
    cityTitle.FontSize(20);
    cityTitle.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());
    cityTitle.Foreground(SolidColorBrush{ primaryColor });
    mainStack.Children().Append(cityTitle);

    // --- Current Weather Details
    Grid currentGrid;
    GridLength c1Len = { 1.0, GridUnitType::Auto };
    GridLength c2Len = { 1.0, GridUnitType::Star };
    ColumnDefinition col1; col1.Width(c1Len);
    ColumnDefinition col2; col2.Width(c2Len);
    currentGrid.ColumnDefinitions().Append(col1);
    currentGrid.ColumnDefinitions().Append(col2);

    StackPanel leftCurrent; leftCurrent.Orientation(Orientation::Horizontal);
    leftCurrent.Spacing(12);
    
    if (g_weatherStyle == 1) { 
        FontIcon bigIcon;
        bigIcon.FontFamily(winrt::Windows::UI::Xaml::Media::FontFamily(L"Segoe MDL2 Assets"));
        bigIcon.Glyph(currentIcon);
        bigIcon.FontSize(42);
        bigIcon.Foreground(SolidColorBrush{ winrt::Windows::UI::ColorHelper::FromArgb(255, 250, 214, 53) }); // Google-like yellow sun
        leftCurrent.Children().Append(bigIcon);
    } else {
        TextBlock bigIcon;
        bigIcon.Text(currentIcon);
        bigIcon.FontSize(38);
        leftCurrent.Children().Append(bigIcon);
    }
    
    StackPanel tempCond; tempCond.Orientation(Orientation::Vertical);
    tempCond.VerticalAlignment(VerticalAlignment::Center);

    StackPanel bigTempStack; bigTempStack.Orientation(Orientation::Horizontal);
    TextBlock bigTemp;
    bigTemp.Text(currentTemp);
    bigTemp.FontSize(32);
    bigTemp.FontWeight(winrt::Windows::UI::Text::FontWeights::Bold());
    bigTemp.Foreground(SolidColorBrush{ primaryColor });
    bigTempStack.Children().Append(bigTemp);
    tempCond.Children().Append(bigTempStack);
    
    leftCurrent.Children().Append(tempCond);
    Grid::SetColumn(leftCurrent, 0);
    currentGrid.Children().Append(leftCurrent);

    StackPanel rightCurrent; rightCurrent.Orientation(Orientation::Vertical);
    rightCurrent.HorizontalAlignment(HorizontalAlignment::Right);
    rightCurrent.VerticalAlignment(VerticalAlignment::Center);
    rightCurrent.Spacing(4);

    auto MakeSmallText = [secondaryColor](std::wstring text) {
        TextBlock tb;
        tb.Text(text);
        tb.FontSize(12);
        tb.Foreground(SolidColorBrush{ secondaryColor });
        return tb;
    };
    
    rightCurrent.Children().Append(MakeSmallText(L"Precipitation: " + localPrecipProb));
    rightCurrent.Children().Append(MakeSmallText(L"Humidity: " + localHumidity));
    rightCurrent.Children().Append(MakeSmallText(L"Wind: " + localWindSpeed));
    Grid::SetColumn(rightCurrent, 1);
    currentGrid.Children().Append(rightCurrent);

    mainStack.Children().Append(currentGrid);
    
    TextBlock condText;
    condText.Text(condition);
    condText.FontSize(14);
    condText.HorizontalAlignment(HorizontalAlignment::Right);
    condText.Foreground(SolidColorBrush{ primaryColor });
    mainStack.Children().Append(condText);

    auto MakeTabBtn = [rootGrid, condition, currentIcon, currentTemp, primaryColor, tertiaryColor, activeTabColor](std::wstring text, int tabIndex, bool isSelected) {
        Grid tabGrid;
        tabGrid.Padding(Thickness{0,0,0,4});
        
        TextBlock tb; tb.Text(text); tb.FontSize(13);
        tb.FontWeight(isSelected ? winrt::Windows::UI::Text::FontWeights::SemiBold() : winrt::Windows::UI::Text::FontWeights::Normal());
        tb.Foreground(SolidColorBrush{ isSelected ? primaryColor : tertiaryColor });
        tb.HorizontalAlignment(HorizontalAlignment::Center);
        tabGrid.Children().Append(tb);
        
        if (isSelected) {
            winrt::Windows::UI::Xaml::Shapes::Rectangle underline;
            underline.Height(3);
            underline.Fill(SolidColorBrush{ activeTabColor });
            underline.VerticalAlignment(VerticalAlignment::Bottom);
            tabGrid.Children().Append(underline);
        }
        
        tabGrid.Tapped([=](auto const&, auto const&) mutable {
            if (g_selectedGraphTab != tabIndex) {
                g_selectedGraphTab = tabIndex;
                PopulateForecastUI(rootGrid, condition, currentIcon, currentTemp);
            }
        });
        
        return tabGrid;
    };

    StackPanel tabs; tabs.Orientation(Orientation::Horizontal); tabs.Spacing(24);
    tabs.Children().Append(MakeTabBtn(L"Temperature", 0, g_selectedGraphTab == 0));
    tabs.Children().Append(MakeTabBtn(L"Precipitation", 1, g_selectedGraphTab == 1));
    tabs.Children().Append(MakeTabBtn(L"Wind", 2, g_selectedGraphTab == 2));
    
    mainStack.Children().Append(tabs);

    winrt::Windows::UI::Xaml::Shapes::Rectangle div1; div1.Height(1); div1.Fill(SolidColorBrush{ dividerColor });
    mainStack.Children().Append(div1);

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
            double v = (g_selectedGraphTab == 0) ? h.tempRaw : (g_selectedGraphTab == 1) ? h.precipRaw : h.windkphRaw;
            if (v > maxVal) maxVal = v;
            if (v < minVal) minVal = v;
        }
        if (maxVal == minVal) { maxVal += 1; minVal -= 1; }
        double range = maxVal - minVal;
        
        const double graphHeight = 60.0;
        const double graphWidth = std::max(280.0, rootGrid.Width() - 32.0);
        
        Canvas canvas;
        canvas.Height(graphHeight + 30);
        canvas.Width(graphWidth);
        
        winrt::Windows::UI::Xaml::Shapes::Polyline poly;
        poly.Stroke(SolidColorBrush{ winrt::Windows::UI::ColorHelper::FromArgb(255, 250, 214, 53) }); // Yellow
        if (g_selectedGraphTab == 1) poly.Stroke(SolidColorBrush{ activeTabColor }); // Blue for precip
        if (g_selectedGraphTab == 2) poly.Stroke(SolidColorBrush{ winrt::Windows::UI::ColorHelper::FromArgb(255, 200, 200, 200) }); // Gray for wind
        poly.StrokeThickness(2.0);
        
        winrt::Windows::UI::Xaml::Media::PointCollection points;
        
        int step = (currentDayHourly.size() <= 8) ? 1 : (currentDayHourly.size() / 6); // show ~6 points
        if (step < 1) step = 1;
        
        int pointCount = 0;
        for (size_t i = 0; i < currentDayHourly.size(); i += step) pointCount++;
        
        double xStep = graphWidth / (double)std::max(1, pointCount - 1);
        
        int pIdx = 0;
        for (size_t i = 0; i < currentDayHourly.size(); i += step) {
            auto h = currentDayHourly[i];
            double v = (g_selectedGraphTab == 0) ? h.tempRaw : (g_selectedGraphTab == 1) ? h.precipRaw : h.windkphRaw;
            double normY = (v - minVal) / range; // 0 to 1
            double x = pIdx * xStep;
            double y = graphHeight - (normY * graphHeight);
            
            points.Append(winrt::Windows::Foundation::Point{(float)x, (float)y + 10.0f});
            
            // Value Label
            TextBlock valTb; 
            wchar_t vBuf[32];
            swprintf(vBuf, L"%.0f", v);
            valTb.Text(vBuf);
            valTb.FontSize(11);
            valTb.Foreground(SolidColorBrush{ secondaryColor });
            Canvas::SetLeft(valTb, x - 5);
            Canvas::SetTop(valTb, y - 10);
            canvas.Children().Append(valTb);
            
            // Time Label
            TextBlock timeTb; 
            timeTb.Text(h.timeString);
            timeTb.FontSize(10);
            timeTb.Foreground(SolidColorBrush{ tertiaryColor });
            Canvas::SetLeft(timeTb, x - 10);
            Canvas::SetTop(timeTb, graphHeight + 15.0);
            canvas.Children().Append(timeTb);
            
            pIdx++;
        }
        
        poly.Points(points);
        canvas.Children().Append(poly);
        
        mainStack.Children().Append(canvas);
    }

    winrt::Windows::UI::Xaml::Shapes::Rectangle div2; div2.Height(1); div2.Fill(SolidColorBrush{ dividerColor });
    mainStack.Children().Append(div2);

    // --- Daily Forecast (Horizontal Scroll or Grid)
    if (!localDaily.empty()) {
        ScrollViewer dailyScroll;
        dailyScroll.HorizontalScrollBarVisibility(ScrollBarVisibility::Hidden);
        dailyScroll.VerticalScrollBarVisibility(ScrollBarVisibility::Disabled);
        dailyScroll.HorizontalScrollMode(ScrollMode::Enabled);
        dailyScroll.VerticalScrollMode(ScrollMode::Disabled);
        
        StackPanel dailyStack; dailyStack.Orientation(Orientation::Horizontal);
        dailyStack.Spacing(8);
        
        winrt::Windows::UI::Xaml::Media::Animation::TransitionCollection dailyTrans;
        winrt::Windows::UI::Xaml::Media::Animation::RepositionThemeTransition repo;
        dailyTrans.Append(repo);
        dailyStack.ChildrenTransitions(dailyTrans);

        
        for(size_t i = 0; i < localDaily.size(); i++) {
            auto d = localDaily[i];
            bool isSel = (d.rawDate == g_selectedDate);
            
            Grid dayCard;
            dayCard.CornerRadius(CornerRadius{8,8,8,8});
            dayCard.Padding(Thickness{4, 8, 4, 8});
            dayCard.Width(62.0); /* Force exact width for predictable layouts */
            dayCard.Background(SolidColorBrush{ isSel ? cardBgColor : winrt::Windows::UI::Colors::Transparent() });
            
            StackPanel dayCol; dayCol.Orientation(Orientation::Vertical); dayCol.Spacing(6);
            dayCol.HorizontalAlignment(HorizontalAlignment::Center);
            
            TextBlock tDay; tDay.Text(i == 0 ? L"Today" : d.dayName);
            tDay.FontSize(12); tDay.Foreground(SolidColorBrush{ primaryColor });
            tDay.HorizontalAlignment(HorizontalAlignment::Center);
            dayCol.Children().Append(tDay);
            
            if (g_weatherStyle == 1) { 
                FontIcon hd; hd.FontFamily(winrt::Windows::UI::Xaml::Media::FontFamily(L"Segoe MDL2 Assets"));
                hd.Glyph(d.icon); hd.FontSize(20); hd.Foreground(SolidColorBrush{ winrt::Windows::UI::ColorHelper::FromArgb(255, 250, 214, 53) });
                hd.HorizontalAlignment(HorizontalAlignment::Center); dayCol.Children().Append(hd);
            } else {
                TextBlock hd; hd.Text(d.icon); hd.FontSize(22);
                hd.HorizontalAlignment(HorizontalAlignment::Center); dayCol.Children().Append(hd);
            }
            
            StackPanel minmax; minmax.Orientation(Orientation::Horizontal);
            minmax.Spacing(4); minmax.HorizontalAlignment(HorizontalAlignment::Center);
            
            std::wstring cleanMax = d.tempMax;
            size_t mpos = cleanMax.find(L"°"); if (mpos != std::wstring::npos) cleanMax = cleanMax.substr(0, mpos + 1);
            TextBlock tMax; tMax.Text(cleanMax); tMax.FontSize(12);
            tMax.Foreground(SolidColorBrush{ primaryColor });
            tMax.FontWeight(winrt::Windows::UI::Text::FontWeights::SemiBold());
            
            std::wstring cleanMin = d.tempMin;
            mpos = cleanMin.find(L"°"); if (mpos != std::wstring::npos) cleanMin = cleanMin.substr(0, mpos + 1);
            TextBlock tMin; tMin.Text(cleanMin); tMin.FontSize(12);
            tMin.Foreground(SolidColorBrush{ secondaryColor });
            
            minmax.Children().Append(tMax);
            minmax.Children().Append(tMin);
            dayCol.Children().Append(minmax);
            
            dayCard.Children().Append(dayCol);
            
            std::wstring dDate = d.rawDate;
            dayCard.Tapped([=](auto const&, auto const&) mutable {
                if (g_selectedDate != dDate) {
                    g_selectedDate = dDate;
                    PopulateForecastUI(rootGrid, condition, currentIcon, currentTemp);
                }
            });
            
            dailyStack.Children().Append(dayCard);
        }
        
        dailyScroll.Content(dailyStack);
        mainStack.Children().Append(dailyScroll);
    }

    rootGrid.Children().Append(mainStack);
}
// Programmatically modify the XAML visual children in place
void UpdateWeatherXamlElements(Grid weatherGrid, std::wstring temp, std::wstring icon, std::wstring condition, bool acquired) {
    if (!weatherGrid) return;
    try {
        UpdateInjectedWeatherLayout(weatherGrid);
        
        weatherGrid.Children().Clear();
        
        Button buttonElement;
        
        // Remove standard Windows borders and establish padding
        winrt::Windows::UI::Color transColor;
        transColor.A = 0; transColor.R = 0; transColor.G = 0; transColor.B = 0;
        buttonElement.Background(SolidColorBrush{ transColor });
        buttonElement.BorderBrush(nullptr);
        buttonElement.BorderThickness(Thickness{ 0 });
        buttonElement.Padding(Thickness{ 6, 2, 6, 2 });
        buttonElement.VerticalAlignment(VerticalAlignment::Stretch);
        buttonElement.HorizontalAlignment(HorizontalAlignment::Stretch);
        
        try {
            buttonElement.CornerRadius(CornerRadius{ 4.0, 4.0, 4.0, 4.0 });
            
            ToolTip toolTip;
            toolTip.Content(winrt::box_value(g_displayCity + L"\n" + condition + L", " + temp + L"\nClick for full forecast"));
            ToolTipService::SetToolTip(buttonElement, toolTip);
            
            if (acquired) {
                Flyout flyout = CreateForecastFlyout(condition, icon, temp);
                buttonElement.Flyout(flyout);
            }
        } catch (...) {}
        
        StackPanel stackPanel;
        stackPanel.Orientation(Orientation::Horizontal);
        stackPanel.Spacing(6.0);
        stackPanel.VerticalAlignment(VerticalAlignment::Center);
        stackPanel.HorizontalAlignment(HorizontalAlignment::Left);
        
        winrt::Windows::UI::Color xamlColor;
        xamlColor.A = 255;
        xamlColor.R = GetRValue(g_textColor);
        xamlColor.G = GetGValue(g_textColor);
        xamlColor.B = GetBValue(g_textColor);
        SolidColorBrush foregroundBrush{ xamlColor };
        
        if (g_weatherStyle == 1) { // Segoe MDL2 Assets
            FontIcon iconBlock;
            iconBlock.FontFamily(winrt::Windows::UI::Xaml::Media::FontFamily(L"Segoe MDL2 Assets"));
            iconBlock.Glyph(icon);
            iconBlock.FontSize((double)g_iconFontSize);
            iconBlock.Foreground(foregroundBrush);
            iconBlock.VerticalAlignment(VerticalAlignment::Center);
            stackPanel.Children().Append(iconBlock);
        } else { // Emojis
            TextBlock iconBlock;
            iconBlock.Text(icon);
            iconBlock.FontSize((double)g_iconFontSize);
            iconBlock.VerticalAlignment(VerticalAlignment::Center);
            stackPanel.Children().Append(iconBlock);
        }
        
        StackPanel textContainer;
        textContainer.Orientation(Orientation::Vertical);
        textContainer.VerticalAlignment(VerticalAlignment::Center);
        textContainer.Spacing(0.0);
        
        TextBlock tempBlock;
        tempBlock.FontSize((double)g_fontSize);
        tempBlock.FontWeight(winrt::Windows::UI::Text::FontWeights::Bold());
        tempBlock.Foreground(foregroundBrush);
        
        TextBlock condBlock;
        condBlock.FontSize((double)(g_fontSize - 2.0 > 8.0 ? g_fontSize - 2.0 : 8.0));
        condBlock.FontWeight(winrt::Windows::UI::Text::FontWeights::Normal());
        try {
            condBlock.Opacity(0.8);
        } catch (...) {}
        condBlock.Foreground(foregroundBrush);
        
        if (acquired) {
            tempBlock.Text(temp);
            condBlock.Text(g_showConditionName ? condition : L"");
        } else {
            tempBlock.Text(L"Weather");
            condBlock.Text(L"Loading...");
        }
        
        textContainer.Children().Append(tempBlock);
        if (g_showConditionName || !acquired) {
            textContainer.Children().Append(condBlock);
        }
        
        stackPanel.Children().Append(textContainer);
        buttonElement.Content(stackPanel);
        weatherGrid.Children().Append(buttonElement);
    } catch (...) {}
}

// Dynamic WinHttp pull system
std::wstring RequestHttpData(const std::wstring& host, const std::wstring& path, bool secure) {
    std::wstring response;
    HINTERNET hSession = WinHttpOpen(L"EP_WeatherHost/2.0",
                                     WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                     WINHTTP_NO_PROXY_NAME,
                                     WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return L"";

    WinHttpSetTimeouts(hSession, 6000, 6000, 10000, 10000);

    INTERNET_PORT port = secure ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;
    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), port, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return L"";
    }

    DWORD flags = secure ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(),
                                            NULL, WINHTTP_NO_REFERER,
                                            WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return L"";
    }

    BOOL bResults = WinHttpSendRequest(hRequest,
                                       WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                       WINHTTP_NO_REQUEST_DATA, 0, 0, 0);

    if (bResults) {
        bResults = WinHttpReceiveResponse(hRequest, NULL);
    }

    if (bResults) {
        DWORD dwSize = 0;
        do {
            if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) break;
            if (dwSize == 0) break;

            std::string tempBuffer;
            tempBuffer.resize(dwSize);
            DWORD dwDownloaded = 0;
            if (WinHttpReadData(hRequest, &tempBuffer[0], dwSize, &dwDownloaded)) {
                tempBuffer.resize(dwDownloaded);
                int wlen = MultiByteToWideChar(CP_UTF8, 0, tempBuffer.c_str(), -1, NULL, 0);
                if (wlen > 0) {
                    std::wstring wtemp(wlen, 0);
                    MultiByteToWideChar(CP_UTF8, 0, tempBuffer.c_str(), -1, &wtemp[0], wlen);
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
std::wstring ExtractJSONObject(const std::wstring& json, const std::wstring& key) {
    size_t pos = json.find(L"\"" + key + L"\"");
    if (pos == std::wstring::npos) return L"";
    
    pos = json.find(L"{", pos);
    if (pos == std::wstring::npos) return L"";
    
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
std::wstring ExtractJSONValue(const std::wstring& json, const std::wstring& key, wchar_t endChar = L',') {
    size_t pos = json.find(L"\"" + key + L"\"");
    if (pos == std::wstring::npos) {
        pos = json.find(key);
        if (pos == std::wstring::npos) return L"";
    }
    
    pos = json.find(L":", pos);
    if (pos == std::wstring::npos) return L"";
    pos++;
    
    while (pos < json.length() && (json[pos] == L' ' || json[pos] == L'"' || json[pos] == L'\t')) {
        pos++;
    }
    
    size_t endPos = pos;
    while (endPos < json.length() && json[endPos] != endChar && json[endPos] != L'}' && json[endPos] != L']') {
        if (json[endPos] == L'"') break;
        endPos++;
    }
    
    std::wstring val = json.substr(pos, endPos - pos);
    while (!val.empty() && (val.back() == L' ' || val.back() == L'"' || val.back() == L'\r' || val.back() == L'\n')) {
        val.pop_back();
    }
    return val;
}

// Extract JSON Array
std::wstring ExtractJSONArray(const std::wstring& json, const std::wstring& key) {
    size_t pos = json.find(L"\"" + key + L"\"");
    if (pos == std::wstring::npos) return L"";
    
    pos = json.find(L"[", pos);
    if (pos == std::wstring::npos) return L"";
    
    size_t endPos = json.find(L"]", pos);
    if (endPos == std::wstring::npos) return L"";
    
    return json.substr(pos, endPos - pos + 1);
}

// Tokenize standard bracket JSON array
std::vector<std::wstring> ParseJSONArray(const std::wstring& arrayStr) {
    std::vector<std::wstring> results;
    size_t start = arrayStr.find(L"[");
    if (start == std::wstring::npos) start = 0;
    else start++;
    
    size_t end = arrayStr.rfind(L"]");
    if (end == std::wstring::npos) end = arrayStr.length();
    
    std::wstring inner = arrayStr.substr(start, end - start);
    std::wstring current;
    for (size_t i = 0; i < inner.length(); i++) {
        wchar_t c = inner[i];
        if (c == L',') {
            std::wstring item = current;
            while (!item.empty() && (item.front() == L' ' || item.front() == L'"' || item.front() == L'\t')) item.erase(item.begin());
            while (!item.empty() && (item.back() == L' ' || item.back() == L'"' || item.back() == L'\t')) item.pop_back();
            results.push_back(item);
            current.clear();
        } else {
            current += c;
        }
    }
    if (!current.empty()) {
        std::wstring item = current;
        while (!item.empty() && (item.front() == L' ' || item.front() == L'"' || item.front() == L'\t')) item.erase(item.begin());
        while (!item.empty() && (item.back() == L' ' || item.back() == L'"' || item.back() == L'\t')) item.pop_back();
        results.push_back(item);
    }
    return results;
}

// Sakamoto calendar calculator
std::wstring GetDayOfWeek(int year, int month, int day) {
    static int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
    if (month < 3) year -= 1;
    int dow = (year + year/4 - year/100 + year/400 + t[month-1] + day) % 7;
    const wchar_t* days[] = { L"Sun", L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat" };
    return days[dow];
}

// Resolve weather codes
std::pair<std::wstring, std::wstring> GetCodeMapping(int code) {
    std::wstring icon = L"❓";
    std::wstring cond = L"Cloudy";
    
    if (g_weatherStyle == 0) { // Emoji Style
        if (code == 0) {
            icon = L"☀️";
            cond = L"Clear";
        } else if (code >= 1 && code <= 3) {
            icon = code == 1 ? L"🌤️" : (code == 2 ? L"⛅" : L"☁️");
            cond = code == 1 ? L"Mainly Clear" : (code == 2 ? L"Partly Cloudy" : L"Cloudy");
        } else if (code == 45 || code == 48) {
            icon = L"🌫️";
            cond = L"Fog";
        } else if (code >= 51 && code <= 55) {
            icon = L"🌧️";
            cond = L"Drizzle";
        } else if (code >= 61 && code <= 65) {
            icon = L"🌧️";
            cond = L"Rain";
        } else if (code >= 71 && code <= 75) {
            icon = L"❄️";
            cond = L"Snow";
        } else if (code >= 80 && code <= 82) {
            icon = L"🌧️";
            cond = L"Showers";
        } else if (code == 85 || code == 86) {
            icon = L"❄️";
            cond = L"Flurries";
        } else if (code >= 95) {
            icon = L"⛈️";
            cond = L"Storm";
        } else {
            icon = L"⛅";
            cond = L"Cloudy";
        }
    } else { // Segoe MDL2 icon style
        if (code == 0) {
            icon = L"\uE706";
            cond = L"Clear";
        } else if (code >= 1 && code <= 3) {
            icon = L"\uE7C9";
            cond = code == 3 ? L"Cloudy" : L"Partly Cloudy";
        } else if (code == 45 || code == 48) {
            icon = L"\uE7C8";
            cond = L"Foggy";
        } else if (code >= 51 && code <= 65) {
            icon = L"\uE708";
            cond = L"Rainy";
        } else if (code >= 71 && code <= 75) {
            icon = L"\uE70A";
            cond = L"Snowy";
        } else if (code >= 80 && code <= 82) {
            icon = L"\uE708";
            cond = L"Showers";
        } else if (code >= 95) {
            icon = L"\uE7CE";
            cond = L"Stormy";
        } else {
            icon = L"\uE707";
            cond = L"Cloudy";
        }
    }
    return { icon, cond };
}

void MapOpenMeteoCode(int code, int is_day) {
    if (g_weatherStyle == 0) { // Emoji Style
        if (code == 0) {
            g_cachedIcon = is_day ? L"☀️" : L"🌙";
            g_cachedCondition = L"Clear";
        } else if (code >= 1 && code <= 3) {
            g_cachedIcon = code == 1 ? L"🌤️" : (code == 2 ? L"⛅" : L"☁️");
            g_cachedCondition = code == 1 ? L"Mainly Clear" : (code == 2 ? L"Partly Cloudy" : L"Cloudy");
        } else if (code == 45 || code == 48) {
            g_cachedIcon = L"🌫️";
            g_cachedCondition = L"Foggy";
        } else if (code >= 51 && code <= 55) {
            g_cachedIcon = L"🌧️";
            g_cachedCondition = L"Drizzle";
        } else if (code >= 61 && code <= 65) {
            g_cachedIcon = L"🌧️";
            g_cachedCondition = L"Rain";
        } else if (code >= 71 && code <= 75) {
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
            g_cachedIcon = is_day ? L"\uE706" : L"\uE708";
            g_cachedCondition = L"Clear";
        } else if (code >= 1 && code <= 3) {
            g_cachedIcon = L"\uE7C9";
            g_cachedCondition = code == 3 ? L"Cloudy" : L"Partly Cloudy";
        } else if (code == 45 || code == 48) {
            g_cachedIcon = L"\uE7C8";
            g_cachedCondition = L"Foggy";
        } else if (code >= 51 && code <= 65) {
            g_cachedIcon = L"\uE708";
            g_cachedCondition = L"Rainy";
        } else if (code >= 71 && code <= 75) {
            g_cachedIcon = L"\uE70A";
            g_cachedCondition = L"Snowy";
        } else if (code >= 80 && code <= 82) {
            g_cachedIcon = L"\uE708";
            g_cachedCondition = L"Showers";
        } else if (code >= 95) {
            g_cachedIcon = L"\uE7CE";
            g_cachedCondition = L"Stormy";
        } else {
            g_cachedIcon = L"\uE707";
            g_cachedCondition = L"Cloudy";
        }
    }
}

// Queue XAML widget re-fill on UI thread safely from background weather update loops
void QueueWeatherUpdateOnUIThread() {
    std::wstring temp = g_cachedTemp;
    std::wstring icon = g_cachedIcon;
    std::wstring condition = g_cachedCondition;
    bool acquired = g_weatherAcquired;

    Grid targetGrid = nullptr;
    {
        std::lock_guard<std::mutex> lock(g_weatherGridMutex);
        if (g_injectedWeatherGrid) {
            targetGrid = g_injectedWeatherGrid.get();
        }
    }

    if (targetGrid) {
        targetGrid.Dispatcher().RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::Normal, [targetGrid, temp, icon, condition, acquired]() {
            try {
                UpdateWeatherXamlElements(targetGrid, temp, icon, condition, acquired);
            } catch (...) {}
        });
    }
}

// Background meteorological synchronizer thread
DWORD WINAPI QueryWeatherPipeline(LPVOID lpParam) {
    if (g_debugLogs) Wh_Log(L"[EP_WeatherHost] Background thread PIPELINE started");

    while (!g_bThreadShouldTerm) {
        bool updateSuccess = false;
        
        std::wstring loc = g_location;
        if (loc.length() > 1 && loc.front() == L'"' && loc.back() == L'"') {
            loc = loc.substr(1, loc.length() - 2); // Strip quotes just in case
        }
        if (loc == L"auto" || loc.empty()) {
            if (g_debugLogs) Wh_Log(L"[EP_WeatherHost] Triggering Geolocation IP Search...");
            std::wstring geoJson = RequestHttpData(L"ip-api.com", L"/json", false);
            if (!geoJson.empty()) {
                std::wstring latStr = ExtractJSONValue(geoJson, L"lat");
                if (latStr.empty()) latStr = ExtractJSONValue(geoJson, L"latitude");
                std::wstring lonStr = ExtractJSONValue(geoJson, L"lon");
                if (lonStr.empty()) lonStr = ExtractJSONValue(geoJson, L"longitude");
                std::wstring cityStr = ExtractJSONValue(geoJson, L"city");
                std::wstring regionStr = ExtractJSONValue(geoJson, L"regionName");
                if (regionStr.empty()) regionStr = ExtractJSONValue(geoJson, L"region");
                std::wstring countryStr = ExtractJSONValue(geoJson, L"countryCode");
                if (countryStr.empty()) countryStr = ExtractJSONValue(geoJson, L"country");
                
                if (!latStr.empty() && !lonStr.empty()) {
                    g_cachedLatitude = wcstod(latStr.c_str(), NULL);
                    g_cachedLongitude = wcstod(lonStr.c_str(), NULL);
                    
                    std::wstring fullCity = cityStr;
                    if (!fullCity.empty() && !regionStr.empty()) fullCity += L", " + regionStr;
                    if (!fullCity.empty() && !countryStr.empty()) fullCity += L", " + countryStr;
                    
                    g_displayCity = !fullCity.empty() ? fullCity : L"Local Weather";
                    if (g_debugLogs) Wh_Log(L"[EP_WeatherHost] Geoloacted to %s (%f, %f)", g_displayCity.c_str(), g_cachedLatitude, g_cachedLongitude);
                    updateSuccess = true;
                }
            }
        } else {
            if (g_debugLogs) Wh_Log(L"[EP_WeatherHost] Resolving coordinates for city: %s", loc.c_str());
            std::wstring escapedLoc = loc;
            size_t sPos;
            while ((sPos = escapedLoc.find(L" ")) != std::wstring::npos) {
                escapedLoc.replace(sPos, 1, L"%20");
            }
            
            std::wstring path = L"/v1/search?name=" + escapedLoc + L"&count=1&language=en&format=json";
            std::wstring geoJson = RequestHttpData(L"geocoding-api.open-meteo.com", path.c_str(), true);
            if (!geoJson.empty()) {
                std::wstring latStr = ExtractJSONValue(geoJson, L"latitude");
                std::wstring lonStr = ExtractJSONValue(geoJson, L"longitude");
                std::wstring nameStr = ExtractJSONValue(geoJson, L"name");
                std::wstring adminStr = ExtractJSONValue(geoJson, L"admin1");
                std::wstring countryStr = ExtractJSONValue(geoJson, L"country_code");
                if (countryStr.empty()) countryStr = ExtractJSONValue(geoJson, L"country");
                
                if (!latStr.empty() && !lonStr.empty()) {
                    g_cachedLatitude = wcstod(latStr.c_str(), NULL);
                    g_cachedLongitude = wcstod(lonStr.c_str(), NULL);
                    
                    std::wstring fullCity = nameStr;
                    if (!fullCity.empty() && !adminStr.empty()) fullCity += L", " + adminStr;
                    if (!fullCity.empty() && !countryStr.empty()) fullCity += L", " + countryStr;
                    
                    g_displayCity = !fullCity.empty() ? fullCity : loc;
                    if (g_debugLogs) Wh_Log(L"[EP_WeatherHost] Resolved Coordinates: %s (%f, %f)", g_displayCity.c_str(), g_cachedLatitude, g_cachedLongitude);
                    updateSuccess = true;
                }
            }
        }

        if (g_cachedLatitude != 0.0 || g_cachedLongitude != 0.0) {
            wchar_t pathBuf[512];
            swprintf(pathBuf, L"/v1/forecast?latitude=%.4f&longitude=%.4f&current=temperature_2m,relative_humidity_2m,is_day,weather_code,wind_speed_10m&hourly=temperature_2m,weathercode,precipitation_probability,wind_speed_10m&daily=weathercode,temperature_2m_max,temperature_2m_min&timezone=auto&forecast_days=%d", g_cachedLatitude, g_cachedLongitude, g_forecastDaysFetch);
            std::wstring forecastJson = RequestHttpData(L"api.open-meteo.com", pathBuf, true);
            
            if (!forecastJson.empty()) {
                std::wstring currentBlock = ExtractJSONObject(forecastJson, L"current");
                std::wstring dailyBlock = ExtractJSONObject(forecastJson, L"daily");
                std::wstring hourlyBlock = ExtractJSONObject(forecastJson, L"hourly");
                
                std::wstring tempStr = L"";
                std::wstring codeStr = L"";
                std::wstring dayStr = L"";
                std::wstring windSpeedStr = L"";
                std::wstring humidityStr = L"";
                if (!currentBlock.empty()) {
                    tempStr = ExtractJSONValue(currentBlock, L"temperature_2m");
                    codeStr = ExtractJSONValue(currentBlock, L"weather_code");
                    dayStr = ExtractJSONValue(currentBlock, L"is_day");
                    windSpeedStr = ExtractJSONValue(currentBlock, L"wind_speed_10m");
                    humidityStr = ExtractJSONValue(currentBlock, L"relative_humidity_2m");
                }
                
                std::wstring formattedWindSpeed = L"--";
                if (!windSpeedStr.empty()) {
                    double ws = wcstod(windSpeedStr.c_str(), NULL);
                    wchar_t wsBuf[64];
                    if (g_useCelsius) {
                        swprintf(wsBuf, L"%.1f km/h", ws);
                    } else {
                        double mph = ws * 0.621371;
                        swprintf(wsBuf, L"%.1f mph", mph);
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
                    maxArrStr = ExtractJSONArray(dailyBlock, L"temperature_2m_max");
                    minArrStr = ExtractJSONArray(dailyBlock, L"temperature_2m_min");
                }

                std::wstring hTimeArrStr = L"";
                std::wstring hTempArrStr = L"";
                std::wstring hCodeArrStr = L"";
                std::wstring hPrecipArrStr = L"";
                std::wstring hWindArrStr = L"";
                if (!hourlyBlock.empty()) {
                    hTimeArrStr = ExtractJSONArray(hourlyBlock, L"time");
                    hTempArrStr = ExtractJSONArray(hourlyBlock, L"temperature_2m");
                    hCodeArrStr = ExtractJSONArray(hourlyBlock, L"weathercode");
                    hPrecipArrStr = ExtractJSONArray(hourlyBlock, L"precipitation_probability");
                    hWindArrStr = ExtractJSONArray(hourlyBlock, L"wind_speed_10m");
                }

                std::vector<std::wstring> timeVec = ParseJSONArray(timeArrStr);
                std::vector<std::wstring> codeVec = ParseJSONArray(codeArrStr);
                std::vector<std::wstring> maxVec = ParseJSONArray(maxArrStr);
                std::vector<std::wstring> minVec = ParseJSONArray(minArrStr);

                std::vector<std::wstring> hTimeVec = ParseJSONArray(hTimeArrStr);
                std::vector<std::wstring> hTempVec = ParseJSONArray(hTempArrStr);
                std::vector<std::wstring> hCodeVec = ParseJSONArray(hCodeArrStr);
                std::vector<std::wstring> hPrecipVec = ParseJSONArray(hPrecipArrStr);
                std::vector<std::wstring> hWindVec = ParseJSONArray(hWindArrStr);

                std::wstring currentPrecipProb = L"0%";
                
                size_t startHourIdx = 0;
                if (!currentBlock.empty()) {
                    std::wstring currentTimeStr = ExtractJSONValue(currentBlock, L"time");
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
                if (codeVec.size() < numDays) numDays = codeVec.size();
                if (maxVec.size() < numDays) numDays = maxVec.size();
                if (minVec.size() < numDays) numDays = minVec.size();

                std::vector<DailyForecast> parsedBriefForecasts;
                for (size_t i = 0; i < numDays; i++) {
                    DailyForecast dayData;
                    
                    std::wstring rawDate = timeVec[i];
                    dayData.rawDate = rawDate;
                    int year = 0, month = 0, dayInt = 0;
                    if (swscanf(rawDate.c_str(), L"%d-%d-%d", &year, &month, &dayInt) == 3) {
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
                size_t limitHours = g_forecastDaysFetch * 24; // Parse all requested days of hourly data
                for (size_t i = startHourIdx; i < hTimeVec.size() && (i - startHourIdx) < limitHours; i++) {
                    if (i >= hTempVec.size() || i >= hCodeVec.size() || i >= hPrecipVec.size() || i >= hWindVec.size()) break;
                    
                    HourlyForecast hourData;
                    std::wstring rawTime = hTimeVec[i];
                    
                    size_t tIndex = rawTime.find(L"T");
                    if (tIndex != std::wstring::npos) {
                        hourData.rawDate = rawTime.substr(0, tIndex);
                    }
                    
                    int militaryHour = 0;
                    if (tIndex != std::wstring::npos && tIndex + 2 < rawTime.length()) {
                        std::wstring hourSub = rawTime.substr(tIndex + 1, 2);
                        militaryHour = _wtoi(hourSub.c_str());
                    }
                    
                    int displayHour = militaryHour % 12;
                    if (displayHour == 0) displayHour = 12;
                    wchar_t timeBuf[32];
                    swprintf(timeBuf, L"%d %s", displayHour, (militaryHour >= 12) ? L"PM" : L"AM");
                    hourData.timeString = timeBuf;
                    
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
                    
                    hourData.precipProb = hPrecipVec[i] + L"%";
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
                    swprintf(formattedTemp, L"%.1f°%c", tempVal, g_useCelsius ? 'C' : 'F');
                    g_cachedTemp = formattedTemp;
                    
                    int code = !codeStr.empty() ? _wtoi(codeStr.c_str()) : 0;
                    int is_day = !dayStr.empty() ? _wtoi(dayStr.c_str()) : 1;
                    
                    MapOpenMeteoCode(code, is_day);
                    g_weatherAcquired = true;
                    updateSuccess = true;
                    
                    if (g_debugLogs) {
                        Wh_Log(L"[EP_WeatherHost] Weather Updated! Temp=%s, Code=%d, Condition=%s, City=%s",
                               g_cachedTemp.c_str(), code, g_cachedCondition.c_str(), g_displayCity.c_str());
                    }
                    
                    QueueWeatherUpdateOnUIThread();
                }
            }
        }
        
        DWORD sleepMs = updateSuccess ? (g_updateInterval * 60 * 1000) : (60 * 1000);
        DWORD waitStatus = WaitForSingleObject(g_hForceUpdateEvent, sleepMs);
        if (waitStatus == WAIT_OBJECT_0) {
            ResetEvent(g_hForceUpdateEvent);
        }
    }
    
    return 0;
}

// Remove previously injected containers
void RemoveInjectedFromGrid(Grid grid) {
    if (!grid) return;
    try {
        auto children = grid.Children();
        for (int i = (int)children.Size() - 1; i >= 0; i--) {
            if (auto fe = children.GetAt(i).try_as<FrameworkElement>()) {
                std::wstring name(fe.Name());
                if (name == L"EPWeatherHostGrid") {
                    children.RemoveAt(i);
                }
            }
        }
    } catch (...) {}
}

// Native direct XAML injection of Weather Grid
void InjectContentIntoGrid(FrameworkElement element, std::wstring_view uniqueName) {
    auto grid = element.try_as<Grid>();
    if (!grid) return;

    RemoveInjectedFromGrid(grid);

    Grid weatherGrid;
    weatherGrid.Name(uniqueName);
    
    // Make sure we span the entire RootGrid, bypassing any column definitions, columns, or layout cells so that margin aligns globally
    try {
        Grid::SetColumn(weatherGrid, 0);
        Grid::SetColumnSpan(weatherGrid, 99);
        Grid::SetRow(weatherGrid, 0);
        Grid::SetRowSpan(weatherGrid, 99);
    } catch (...) {}

    UpdateInjectedWeatherLayout(weatherGrid);
    
    // Transparent solid color brush background
    weatherGrid.Background(winrt::Windows::UI::Xaml::Media::SolidColorBrush(winrt::Windows::UI::Colors::Transparent()));

    UpdateWeatherXamlElements(weatherGrid, g_cachedTemp, g_cachedIcon, g_cachedCondition, g_weatherAcquired);

    grid.Children().Append(weatherGrid);

    auto weakWeatherGrid = winrt::make_weak(weatherGrid);
    element.SizeChanged([weakWeatherGrid](auto const&, auto const&) {
        if (auto wg = weakWeatherGrid.get()) {
            wg.Dispatcher().RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::Normal, [wg]() {
                try {
                    UpdateInjectedWeatherLayout(wg);
                } catch (...) {}
            });
        }
    });

    {
        std::lock_guard<std::mutex> lock(g_weatherGridMutex);
        g_injectedWeatherGrid = winrt::make_weak(weatherGrid);
    }
}

// Tree scanning logic
void ScanAndInjectRecursive(FrameworkElement element) {
    if (!element) return;
    
    std::wstring name(element.Name());
    if (name == L"RootGrid") {
        InjectContentIntoGrid(element, L"EPWeatherHostGrid");
        return;
    }

    int count = VisualTreeHelper::GetChildrenCount(element);
    for (int i = 0; i < count; i++) {
        auto child = VisualTreeHelper::GetChild(element, i).try_as<FrameworkElement>();
        if (child) ScanAndInjectRecursive(child);
    }
}

std::atomic<bool> g_scanPending = false;
std::vector<winrt::weak_ref<FrameworkElement>> g_scannedFrames;
std::mutex g_pendingMutex;

void ScheduleScanAsync(FrameworkElement startNode) {
    if (!startNode || g_scanPending.exchange(true)) return;
    auto weak = winrt::make_weak(startNode);
    try {
        startNode.Dispatcher().RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::Low, [weak]() {
            g_scanPending = false;
            if (auto node = weak.get()) {
                FrameworkElement current = node;
                while (current) {
                    if (winrt::get_class_name(current) == L"Taskbar.TaskbarFrame") {
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

FrameworkElement GetFrameworkElementFromNative(void* pThis) {
    try {
        void* iUnknownPtr = (void**)pThis + 3;
        winrt::Windows::Foundation::IUnknown iUnknown;
        winrt::copy_from_abi(iUnknown, iUnknownPtr);
        return iUnknown.try_as<FrameworkElement>();
    } catch (...) { return nullptr; }
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
        { {LR"(private: void __cdecl winrt::Taskbar::implementation::TaskListButton::UpdateVisualStates(void))"}, (void**)&TaskListButton_UpdateVisualStates_Original, (void*)TaskListButton_UpdateVisualStates_Hook }
    };
    return HookSymbols(module, symbolHooks, ARRAYSIZE(symbolHooks));
}

std::atomic<bool> g_taskbarViewDllLoaded = false;

using LoadLibraryExW_t = decltype(&LoadLibraryExW);
LoadLibraryExW_t LoadLibraryExW_Original;
HMODULE WINAPI LoadLibraryExW_Hook(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
    HMODULE module = LoadLibraryExW_Original(lpLibFileName, hFile, dwFlags);
    if (module && !g_taskbarViewDllLoaded && (wcsstr(lpLibFileName, L"Taskbar.View.dll") || wcsstr(lpLibFileName, L"ExplorerExtensions.dll"))) {
        if (!g_taskbarViewDllLoaded.exchange(true)) {
            HookTaskbarViewDllSymbols(module);
            Wh_ApplyHookOperations();
        }
    }
    return module;
}

WNDPROC g_pfnOldClockWndProc = NULL;
HWND g_hSubclassedWnd = NULL;

LRESULT CALLBACK ClockSubclassWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_WINDOWPOSCHANGED || uMsg == WM_SIZE || uMsg == WM_SHOWWINDOW) {
        Grid targetGrid = nullptr;
        {
            std::lock_guard<std::mutex> lock(g_weatherGridMutex);
            if (g_injectedWeatherGrid) targetGrid = g_injectedWeatherGrid.get();
        }
        if (targetGrid) {
            targetGrid.Dispatcher().RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::Normal, [targetGrid]() {
                try {
                    UpdateInjectedWeatherLayout(targetGrid);
                } catch (...) {}
            });
        }
    }
    if (g_pfnOldClockWndProc) {
        return CallWindowProcW(g_pfnOldClockWndProc, hWnd, uMsg, wParam, lParam);
    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

void AlignOverlayWindow() {
    HWND hAnchor = FindSystemAnchorWnd();
    if (hAnchor && g_hSubclassedWnd != hAnchor) {
        if (g_hSubclassedWnd && g_pfnOldClockWndProc) {
            SetWindowLongPtrW(g_hSubclassedWnd, GWLP_WNDPROC, (LONG_PTR)g_pfnOldClockWndProc);
        }
        g_hSubclassedWnd = hAnchor;
        g_pfnOldClockWndProc = (WNDPROC)SetWindowLongPtrW(hAnchor, GWLP_WNDPROC, (LONG_PTR)ClockSubclassWndProc);
        if (g_debugLogs && g_pfnOldClockWndProc) {
            Wh_Log(L"[EP_WeatherHost] Successfully subclassed System Anchor HWND=%p", hAnchor);
        }
    }
}

void LoadModConfiguration() {
    PCWSTR city = Wh_GetStringSetting(L"location");
    g_location = city ? city : L"auto";
    Wh_FreeStringSetting(city);

    g_useCelsius = Wh_GetIntSetting(L"useCelsius") != 0;
    
    g_updateInterval = Wh_GetIntSetting(L"updateInterval");
    if (g_updateInterval < 5) g_updateInterval = 5;
    if (g_updateInterval > 120) g_updateInterval = 120;

    g_weatherStyle = Wh_GetIntSetting(L"weatherStyle");
    g_showConditionName = Wh_GetIntSetting(L"showConditionName") != 0;
    g_textOffset = Wh_GetIntSetting(L"textOffset");
    g_fontSize = Wh_GetIntSetting(L"fontSize");
    g_iconFontSize = Wh_GetIntSetting(L"iconFontSize");
    if (g_iconFontSize <= 0) g_iconFontSize = 16;
    g_debugLogs = Wh_GetIntSetting(L"debugLogs") != 0;
    g_useAcrylic = Wh_GetIntSetting(L"useAcrylic") != 0;
    g_acrylicOpacity = Wh_GetIntSetting(L"acrylicOpacity");
    if (g_acrylicOpacity < 0) g_acrylicOpacity = 0;
    if (g_acrylicOpacity > 100) g_acrylicOpacity = 100;
    g_forecastDaysFetch = Wh_GetIntSetting(L"forecastDaysFetch");
    if (g_forecastDaysFetch < 1) g_forecastDaysFetch = 7;
    if (g_forecastDaysFetch > 16) g_forecastDaysFetch = 16;

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

    if (g_debugLogs) Wh_Log(L"[EP_WeatherHost] Reloaded settings: loc=%s, metric=%d, fontS=%d", g_location.c_str(), g_useCelsius, g_fontSize);
    
    QueueWeatherUpdateOnUIThread();

    if (g_hForceUpdateEvent) {
        SetEvent(g_hForceUpdateEvent);
    }
}

// Windhawk mod Entry Point
BOOL Wh_ModInit() {
    InitializeCriticalSection(&g_forecastLock);
    g_hForceUpdateEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    
    LoadModConfiguration();

    g_bThreadShouldTerm = false;

    // Background weather crawl
    g_hQueryThread = CreateThread(NULL, 0, QueryWeatherPipeline, NULL, 0, &g_dwThreadId);

    // Initial check if taskbar module is loaded or intercept via library load hooks
    HMODULE mod = GetModuleHandle(L"Taskbar.View.dll");
    if (!mod) mod = GetModuleHandle(L"ExplorerExtensions.dll");
    if (mod) {
        g_taskbarViewDllLoaded = true;
        HookTaskbarViewDllSymbols(mod);
    } else {
        WindhawkUtils::SetFunctionHook(LoadLibraryExW, LoadLibraryExW_Hook, &LoadLibraryExW_Original);
    }

    AlignOverlayWindow();
    
    // Force a visual state update on the taskbar to immediately trigger our subclass hook
    HWND hTaskbar = FindWindowW(L"Shell_TrayWnd", NULL);
    if (hTaskbar) {
        PostMessageW(hTaskbar, WM_SETTINGCHANGE, 0, (LPARAM)L"ImmersiveColorSet");
        PostMessageW(hTaskbar, WM_THEMECHANGED, 0, 0);
    }

    if (g_debugLogs) Wh_Log(L"[EP_WeatherHost] Modern Native XAML Injection Mod successfully loaded.");
    return TRUE;
}

// Windhawk mod Exit Point
void Wh_ModUninit() {
    if (g_debugLogs) Wh_Log(L"[EP_WeatherHost] Unloading mod...");
    
    if (g_hSubclassedWnd && g_pfnOldClockWndProc) {
        SetWindowLongPtrW(g_hSubclassedWnd, GWLP_WNDPROC, (LONG_PTR)g_pfnOldClockWndProc);
        g_pfnOldClockWndProc = NULL;
        g_hSubclassedWnd = NULL;
    }
    g_bThreadShouldTerm = true;
    if (g_hForceUpdateEvent) {
        SetEvent(g_hForceUpdateEvent);
    }
    // Give background thread a moment to exit
    Sleep(50);
    if (g_hForceUpdateEvent) {
        CloseHandle(g_hForceUpdateEvent);
        g_hForceUpdateEvent = NULL;
    }

    g_bThreadShouldTerm = true;
    if (g_hForceUpdateEvent) {
        SetEvent(g_hForceUpdateEvent);
    }
    
    if (g_hQueryThread) {
        WaitForSingleObject(g_hQueryThread, 3000);
        CloseHandle(g_hQueryThread);
        g_hQueryThread = NULL;
    }

    if (g_hForceUpdateEvent) {
        CloseHandle(g_hForceUpdateEvent);
        g_hForceUpdateEvent = NULL;
    }

    Grid targetGrid = nullptr;
    {
        std::lock_guard<std::mutex> lock(g_weatherGridMutex);
        if (g_injectedWeatherGrid) targetGrid = g_injectedWeatherGrid.get();
    }
    if (targetGrid) {
        targetGrid.Dispatcher().RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::Normal, [targetGrid]() {
            try {
                if (auto parent = targetGrid.Parent().try_as<Grid>()) {
                    RemoveInjectedFromGrid(parent);
                }
            } catch (...) {}
        });
    }

    DeleteCriticalSection(&g_forecastLock);
}

// Settings update receiver
void Wh_ModSettingsChanged() {
    LoadModConfiguration();
    AlignOverlayWindow();
}
