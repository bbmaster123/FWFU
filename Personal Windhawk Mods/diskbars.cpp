// ==WindhawkMod==
// @id             disk-usage-bar-color
// @name           Disk Usage Bar Color 
// @description    Advanced disk usage bars with rounded corners and gloss effect
// @version        1.0
// @author         bbmaster123/gemini
// @include        explorer.exe
// @compilerOptions -luxtheme -lmsimg32 -lgdi32 -lgdiplus
// ==/WindhawkMod==

// ==WindhawkModSettings==
/*
- barColorStart: FF00FF00
  $name: Empty/Healthy (Start)
- barColorEnd: FFE81123
  $name: Full/Warning (End)
- trackColor: 20000000
  $name: Track Color (Background)
- borderColor: 40FFFFFF
  $name: Custom Border Color
- borderThickness: 1
  $name: Border Thickness
- cornerRadius: 6
  $name: Corner Radius
- showGloss: 1
  $name: Enable Glossy Overlay (1 or 0)
- topInset: 2
  $name: Top Inset
- bottomInset: 2
  $name: Bottom Inset
- leftInset: 2
  $name: Left Inset
- rightInset: 2
  $name: Right Inset
*/
// ==/WindhawkModSettings==

#include <windhawk_utils.h>
#include <uxtheme.h>
#include <gdiplus.h>
#include <string>

using namespace Gdiplus;

static decltype(&DrawThemeBackground) DrawThemeBackground_orig = nullptr;
static ARGB g_barColorStart, g_barColorEnd, g_trackColor, g_borderColor;
static int g_cornerRadius, g_showGloss;
static int g_topInset, g_bottomInset, g_leftInset, g_rightInset;
static float g_borderThickness;

static ARGB ParseHexARGB(PCWSTR hex, ARGB fallback) {
    if (!hex || wcslen(hex) < 6) return fallback;
    std::wstring s(hex);
    if (s.length() == 6) s = L"FF" + s;
    try { return (ARGB)std::stoul(s, nullptr, 16); } catch (...) { return fallback; }
}

static void LoadSettings() {
    PCWSTR hex = Wh_GetStringSetting(L"barColorStart");
    g_barColorStart = ParseHexARGB(hex, 0xFF00FF00); 
    Wh_FreeStringSetting(hex);
    hex = Wh_GetStringSetting(L"barColorEnd");
    g_barColorEnd = ParseHexARGB(hex, 0xFFE81123);
    Wh_FreeStringSetting(hex);
    hex = Wh_GetStringSetting(L"trackColor");
    g_trackColor = ParseHexARGB(hex, 0x20000000); 
    Wh_FreeStringSetting(hex);
    hex = Wh_GetStringSetting(L"borderColor");
    g_borderColor = ParseHexARGB(hex, 0x40FFFFFF); 
    Wh_FreeStringSetting(hex);

    g_cornerRadius = Wh_GetIntSetting(L"cornerRadius");
    g_showGloss = Wh_GetIntSetting(L"showGloss");
    g_topInset = Wh_GetIntSetting(L"topInset");
    g_bottomInset = Wh_GetIntSetting(L"bottomInset");
    g_leftInset = Wh_GetIntSetting(L"leftInset");
    g_rightInset = Wh_GetIntSetting(L"rightInset");
    g_borderThickness = (float)Wh_GetIntSetting(L"borderThickness");
    if (g_borderThickness <= 0) g_borderThickness = 1.0f;
}

ARGB InterpolateColor(ARGB start, ARGB end, float ratio) {
    BYTE a1 = (start >> 24) & 0xff, r1 = (start >> 16) & 0xff, g1 = (start >> 8) & 0xff, b1 = start & 0xff;
    BYTE a2 = (end >> 24) & 0xff,   r2 = (end >> 16) & 0xff,   g2 = (end >> 8) & 0xff,   b2 = end & 0xff;
    return Color::MakeARGB(
        (BYTE)(a1 + (a2 - a1) * ratio),
        (BYTE)(r1 + (r2 - r1) * ratio),
        (BYTE)(g1 + (g2 - g1) * ratio),
        (BYTE)(b1 + (b2 - b1) * ratio)
    );
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

static void PaintFinalProgress(HDC hdc, LPCRECT pRect, float fillRatio, bool isFill) {
    Graphics graphics{hdc};
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);
    // Use PixelOffsetMode to help with sub-pixel alignment
    graphics.SetPixelOffsetMode(PixelOffsetModeHighQuality);

    RectF rect{(REAL)pRect->left, (REAL)pRect->top, (REAL)(pRect->right - pRect->left), (REAL)(pRect->bottom - pRect->top)};

    if (isFill) {
        rect.X += (float)g_leftInset; rect.Y += (float)g_topInset;
        rect.Width -= (float)(g_leftInset + g_rightInset); rect.Height -= (float)(g_topInset + g_bottomInset);
        if (rect.Width <= 0 || rect.Height <= 0) return;

        GraphicsPath path;
        GetRoundedPath(&path, rect, (float)g_cornerRadius);
        
        ARGB currentColor = InterpolateColor(g_barColorStart, g_barColorEnd, fillRatio);
        SolidBrush fillBrush{Color{currentColor}};
        
        // Fill the main bar
        graphics.FillPath(&fillBrush, &path);

        if (g_showGloss) {
            // FIX: Clip the gloss to the rounded path so it doesn't bleed out of the corners
            graphics.SetClip(&path);

            RectF glossRect = rect; 
            glossRect.Height /= 2.2f; // Slightly taller gloss for better look
            
            // FIX: Use a Path Gradient or at least ensure the gloss follows the curve
            LinearGradientBrush glossBrush{glossRect, Color{100, 255, 255, 255}, Color{0, 255, 255, 255}, 90.0f};
            
            // We fill the top half, but since we SetClip, the rounded corners are respected
            graphics.FillRectangle(&glossBrush, glossRect);
            
            graphics.ResetClip();
        }
    } else {
        GraphicsPath path;
        GetRoundedPath(&path, rect, (float)g_cornerRadius);
        SolidBrush trackBrush{Color{g_trackColor}};
        graphics.FillPath(&trackBrush, &path);

        if ((g_borderColor >> 24) & 0xFF) {
            // FIX: Offset the border by half the thickness to ensure it stays inside the bounds
            Pen borderPen{Color{g_borderColor}, g_borderThickness};
            borderPen.SetAlignment(PenAlignmentInset);
            graphics.DrawPath(&borderPen, &path);
        }
    }
}

// Robust check: Is it wide and short like a progress bar, and NOT in the NavPane tree?
static bool IsTargetControl(HDC hdc, LPCRECT pRect) {
    if (!pRect) return false;
    int w = pRect->right - pRect->left;
    int h = pRect->bottom - pRect->top;

    // Disk bars are thin. Navigation pane selection blocks are usually the height of a text line (20-30px).
    if (w < 40 || h < 4 || h > 20) return false;
    if (w < h * 3) return false; 

    HWND hwnd = WindowFromDC(hdc);
    if (hwnd) {
        WCHAR cls[256];
        GetClassNameW(hwnd, cls, 256);
        if (wcscmp(cls, L"SysTreeView32") == 0) return false;
    }
    return true;
}

HRESULT WINAPI HookedDrawThemeBackground(HTHEME hTheme, HDC hdc, INT iPartId, INT iStateId, LPCRECT pRect, LPCRECT pClipRect) {
    if (IsTargetControl(hdc, pRect)) {
        if (iPartId == 5) {
            // Use state to determine if it's the "Full" red bar vs normal
            float ratio = (iStateId == 2) ? 0.90f : 0.40f; 
            PaintFinalProgress(hdc, pRect, ratio, true);
            return S_OK;
        }
        if (iPartId >= 1 && iPartId <= 11 && iPartId != 5) {
            if (iPartId == 1) PaintFinalProgress(hdc, pRect, 0, false);
            return S_OK;
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