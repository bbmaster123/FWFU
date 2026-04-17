// ==WindhawkMod==
// @id             explorer-selection-style-universal
// @name           Universal Explorer Selection Styler
// @description    Customize the selection and hover rectangles in both the Navigation Pane and the Main File View
// @version        1.1.1
// @author         dirtyrazkl/bbmaster123/AI
// @include        explorer.exe
// @compilerOptions -luxtheme -lmsimg32 -lgdi32 -lgdiplus
// ==/WindhawkMod==

// ==WindhawkModSettings==
/*
- selectionColor: 4424BBFF
  $name: Selection Fill Color
- borderColor: 8824BBFF
  $name: Selection Border Color
- borderThickness: 2
  $name: Border Thickness
- cornerRadius: 5
  $name: Corner Radius
- horizontalInset: 1
  $name: Horizontal Inset
- verticalInset: 1
  $name: Vertical Inset
*/
// ==/WindhawkModSettings==

#include <windhawk_utils.h>
#include <uxtheme.h>
#include <gdiplus.h>
#include <string>

using namespace Gdiplus;

static decltype(&DrawThemeBackground) DrawThemeBackground_orig = nullptr;
static ARGB g_selectionColor, g_borderColor;
static int g_cornerRadius, g_hInset, g_vInset;
static float g_borderThickness;

static ARGB ParseHexARGB(PCWSTR hex, ARGB fallback) {
    if (!hex || wcslen(hex) < 6) return fallback;
    std::wstring s(hex);
    if (s.length() == 6) s = L"FF" + s;
    try { return (ARGB)std::stoul(s, nullptr, 16); } catch (...) { return fallback; }
}

static void LoadSettings() {
    PCWSTR hex = Wh_GetStringSetting(L"selectionColor");
    g_selectionColor = ParseHexARGB(hex, 0x4424BBFF); 
    Wh_FreeStringSetting(hex);
    hex = Wh_GetStringSetting(L"borderColor");
    g_borderColor = ParseHexARGB(hex, 0x8824BBFF); 
    Wh_FreeStringSetting(hex);
    g_cornerRadius = Wh_GetIntSetting(L"cornerRadius");
    g_hInset = Wh_GetIntSetting(L"horizontalInset");
    g_vInset = Wh_GetIntSetting(L"verticalInset");
    g_borderThickness = (float)Wh_GetIntSetting(L"borderThickness");
}

static void GetRoundedPath(GraphicsPath* path, RectF rect, float radius) {
    if (radius <= 0) { path->AddRectangle(rect); return; }
    float d = radius * 2.0f;
    if (d > rect.Width) d = rect.Width;
    if (d > rect.Height) d = rect.Height;
    path->AddArc(rect.X, rect.Y, d, d, 180, 90);
    path->AddArc(rect.X + rect.Width - d, rect.Y, d, d, 270, 90);
    path->AddArc(rect.X + rect.Width - d, rect.Y + rect.Height - d, d, d, 0, 90);
    path->AddArc(rect.X, rect.Y + rect.Height - d, d, d, 90, 90);
    path->CloseFigure();
}

static void PaintSelection(HDC hdc, LPCRECT pRect) {
    Graphics graphics{hdc};
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);
    RectF rect{(REAL)pRect->left + g_hInset, (REAL)pRect->top + g_vInset, 
               (REAL)(pRect->right - pRect->left) - (g_hInset * 2), 
               (REAL)(pRect->bottom - pRect->top) - (g_vInset * 2)};

    if (rect.Width <= 0 || rect.Height <= 0) return;

    GraphicsPath path;
    GetRoundedPath(&path, rect, (float)g_cornerRadius);
    
    // Fill
    SolidBrush fillBrush{Color{g_selectionColor}};
    graphics.FillPath(&fillBrush, &path);

    // Border
    if ((g_borderColor >> 24) & 0xFF) {
        Pen borderPen{Color{g_borderColor}, g_borderThickness};
        borderPen.SetAlignment(PenAlignmentInset);
        graphics.DrawPath(&borderPen, &path);
    }
}

HRESULT WINAPI HookedDrawThemeBackground(HTHEME hTheme, HDC hdc, INT iPartId, INT iStateId, LPCRECT pRect, LPCRECT pClipRect) {
    if (pRect) {
        WCHAR themeCls[256];
        // Using GetThemeClassList or checking the theme handle properties
        static auto pfn = (HRESULT(WINAPI*)(HTHEME, LPWSTR, int))GetProcAddress(GetModuleHandleW(L"uxtheme.dll"), MAKEINTRESOURCEA(74));
        if (pfn && SUCCEEDED(pfn(hTheme, themeCls, 256))) {
            std::wstring cls(themeCls);
            
            // Catch TreeView, ListView, and the modern Explorer variant
            if (cls.find(L"TreeView") != std::wstring::npos || 
                cls.find(L"ListView") != std::wstring::npos || 
                cls.find(L"ItemsView") != std::wstring::npos ||
                cls.find(L"Explorer") != std::wstring::npos) {

                // Parts: 1 (Item), 5/6 (Selection/Highlight)
                // States: 2-6 (Hot, Selected, etc.)
                if (iPartId == 1 || iPartId == 5 || iPartId == 6 || iPartId == 63) {
                    if (iStateId >= 2 && iStateId <= 6) {
                        PaintSelection(hdc, pRect);
                        return S_OK;
                    }
                }
            }
        }
    }
    return DrawThemeBackground_orig(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);
}

ULONG_PTR g_gdiplusToken;
BOOL Wh_ModInit() {
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL);
    LoadSettings();
    WindhawkUtils::SetFunctionHook(DrawThemeBackground, HookedDrawThemeBackground, &DrawThemeBackground_orig);
    return TRUE;
}

void Wh_ModUninit() { GdiplusShutdown(g_gdiplusToken); }
void Wh_ModSettingsChanged() { LoadSettings(); }
