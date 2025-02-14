<ResourceDictionary
    xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation"
    xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml"
    xmlns:d="http://schemas.microsoft.com/expression/blend/2008"
    xmlns:mc="http://schemas.openxmlformats.org/markup-compatibility/2006"
    xmlns:muxc="using:Microsoft.UI.Xaml.Controls"
    xmlns:wss="using:Windows.Storage.Streams"
    xmlns:taskbar="using:Taskbar"
    xmlns:udk="using:WindowsUdk.UI.Shell"
    x:Class="Taskbar.TaskbarResources">
    <ResourceDictionary.ThemeDictionaries>
        <ResourceDictionary x:Key="Light">
            <SolidColorBrush x:Key="WindowSharingBrush" Color="#FFC42B1C" />
            <SolidColorBrush x:Key="ThumbBarPressedBackgroundBrush" Color="#FFFFFFFF" Opacity="1" />
            <SolidColorBrush x:Key="ThumbBarPointerOverBackgroundBrush" Color="#FFFFFFFF" Opacity="0.85" />
            <StaticResource x:Key="ThumbBarDisabledBorderBrush" ResourceKey="ThumbBarBorderBrush" />
            <SolidColorBrush x:Key="ThumbBarDisabledBackgroundBrush" Color="#FF808080" Opacity="0.1" />
            <SolidColorBrush x:Key="ThumbBarBorderBrush" Color="#FF000000" Opacity="0.0578" />
            <SolidColorBrush x:Key="ThumbBarBackgroundBrush" Color="#FFFFFFFF" Opacity="0.7" />
            <StaticResource x:Key="TextOnAccentFillColorSelectedTextBrush" ResourceKey="SystemControlForegroundAltHighBrush" />
            <ElementTheme x:Key="TaskbarTheme" Value="Light" />
            <StaticResource x:Key="TaskbarRecommendatonDividerBrush" ResourceKey="DividerStrokeColorBrush" />
            <StaticResource x:Key="TaskbarButtonPlateBrush" ResourceKey="SystemControlHighlightChromeWhiteBrush" />
            <StaticResource x:Key="TaskListLabelForegroundBrush" ResourceKey="TextFillColorPrimaryBrush" />
            <SolidColorBrush x:Key="TaskItemThumbnailTextStroke" Color="#9B000000" />
            <SolidColorBrush x:Key="TaskItemThumbnailBackgroundPressedBrush" Color="#4DF9F9F9" />
            <SolidColorBrush x:Key="TaskItemThumbnailBackgroundPointerOverBrush" Color="#B2FFFFFF" />
            <SolidColorBrush x:Key="TaskItemThumbnailBackgroundBrush" Color="#00FFFFFF" />
            <SolidColorBrush x:Key="TaskItemThumbnailAltTextStroke" Color="#9B000000" />
            <Color x:Key="StrokeShadowColor" Value="#0F000000" />
            <SolidColorBrush x:Key="StrokeShadowBrush" Color="{StaticResource StrokeShadowColor}" />
            <Color x:Key="ShellTaskbarItemStrokeColorTertiary" Value="#07000000" />
            <SolidColorBrush x:Key="ShellTaskbarItemStrokeColorTertiaryBrush" Color="{StaticResource ShellTaskbarItemStrokeColorTertiary}" />
            <Color x:Key="ShellTaskbarItemStrokeColorSecondary" Value="#0F000000" />
            <Color x:Key="ShellTaskbarItemStrokeColorQuinary" Value="#05000000" />
            <SolidColorBrush x:Key="ShellTaskbarItemStrokeColorQuinaryBrush" Color="{StaticResource ShellTaskbarItemStrokeColorQuinary}" />
            <Color x:Key="ShellTaskbarItemStrokeColorQuarternary" Value="#05000000" />
            <Color x:Key="ShellTaskbarItemStrokeColorPrimary" Value="#17000000" />
            <LinearGradientBrush x:Key="ShellTaskbarItemGradientStrokeColorSecondaryBrush" MappingMode="Absolute" StartPoint="0,0" EndPoint="0,3">
                <LinearGradientBrush.RelativeTransform>
                    <ScaleTransform ScaleY="-1" CenterY="0.5" />
                </LinearGradientBrush.RelativeTransform>
                <LinearGradientBrush.GradientStops>
                    <GradientStop Offset="0.33" Color="{StaticResource ShellTaskbarItemStrokeColorSecondary}" />
                    <GradientStop Offset="1" Color="{StaticResource ShellTaskbarItemStrokeColorQuarternary}" />
                </LinearGradientBrush.GradientStops>
            </LinearGradientBrush>
            <LinearGradientBrush x:Key="ShellTaskbarItemGradientStrokeColorPrimaryBrush" MappingMode="Absolute" StartPoint="0,0" EndPoint="0,3">
                <LinearGradientBrush.RelativeTransform>
                    <ScaleTransform ScaleY="-1" CenterY="0.5" />
                </LinearGradientBrush.RelativeTransform>
                <LinearGradientBrush.GradientStops>
                    <GradientStop Offset="0.33" Color="{StaticResource ShellTaskbarItemStrokeColorPrimary}" />
                    <GradientStop Offset="1" Color="{StaticResource ShellTaskbarItemStrokeColorTertiary}" />
                </LinearGradientBrush.GradientStops>
            </LinearGradientBrush>
            <Color x:Key="ShellTaskbarItemFillColorTransparent" Value="#00FFFFFF" />
            <SolidColorBrush x:Key="ShellTaskbarItemFillColorTransparentBrush" Color="{StaticResource ShellTaskbarItemFillColorTransparent}" />
            <Color x:Key="ShellTaskbarItemFillColorTertiary" Value="#4CFFFFFF" />
            <SolidColorBrush x:Key="ShellTaskbarItemFillColorTertiaryBrush" Color="{StaticResource ShellTaskbarItemFillColorTertiary}" />
            <Color x:Key="ShellTaskbarItemFillColorSecondary" Value="#80FFFFFF" />
            <SolidColorBrush x:Key="ShellTaskbarItemFillColorSecondaryBrush" Color="{StaticResource ShellTaskbarItemFillColorSecondary}" />
            <Color x:Key="ShellTaskbarItemFillColorPrimary" Value="#B3FFFFFF" />
            <SolidColorBrush x:Key="ShellTaskbarItemFillColorPrimaryBrush" Color="{StaticResource ShellTaskbarItemFillColorPrimary}" />
            <SolidColorBrush x:Key="ShellTaskbarItemFillColorAltSecondaryBrush" Color="{StaticResource ShellTaskbarItemFillColorSecondary}" />
            <SolidColorBrush x:Key="ShareWindow_ConfirmPressedForegroundBrush" Color="#FFFFFFFF" />
            <SolidColorBrush x:Key="ShareWindow_ConfirmPressedBorderBrush" Color="#D0268826" />
            <SolidColorBrush x:Key="ShareWindow_ConfirmPressedBackgroundBrush" Color="#CC0F7A0F" />
            <SolidColorBrush x:Key="ShareWindow_ConfirmPointerOverForegroundBrush" Color="#FFFFFFFF" />
            <SolidColorBrush x:Key="ShareWindow_ConfirmPointerOverBorderBrush" Color="#E7248824" />
            <SolidColorBrush x:Key="ShareWindow_ConfirmPointerOverBackgroundBrush" Color="#E50E7C0E" />
            <SolidColorBrush x:Key="ShareWindow_ConfirmForegroundBrush" Color="#FFFFFFFF" />
            <SolidColorBrush x:Key="ShareWindow_ConfirmBorderBrush" Color="#FF228522" />
            <SolidColorBrush x:Key="ShareWindow_ConfirmBackgroundBrush" Color="#FF0F7B0F" />
            <SolidColorBrush x:Key="ShareWindowButtonStopSharingStrokeBrush" Opacity="0.08" Color="#FFFFFFFF" />
            <SolidColorBrush x:Key="ShareWindowButtonStopSharingShadowBrush" Opacity="0.4" Color="#FF000000" />
            <SolidColorBrush x:Key="ShareWindowButtonStopSharingBackgroundPressedBrush" Color="#FF8E192E" />
            <SolidColorBrush x:Key="ShareWindowButtonStopSharingBackgroundPointerOverBrush" Color="#FFA72037" />
            <SolidColorBrush x:Key="ShareWindowButtonStopSharingBackgroundBrush" Color="#FFC4314B" />
            <SolidColorBrush x:Key="ShareWindowButtonForegroundPressedBrush" Opacity="0.4458" Color="#FF000000" />
            <SolidColorBrush x:Key="ShareWindowButtonBackgroundPressedBrush" Opacity="0.3" Color="#FFF9F9F9" />
            <SolidColorBrush x:Key="ShareWindowButtonBackgroundPointerOverBrush" Opacity="0.5" Color="#FFF9F9F9" />
            <SolidColorBrush x:Key="ShareWindowButtonBackgroundBrush" Opacity="0.7" Color="#FFFFFFFF" />
            <Color x:Key="ShadowColor" Value="#29000000" />
            <SolidColorBrush x:Key="ShadowBrush" Color="{StaticResource ShadowColor}" />
            <StaticResource x:Key="SearchPillButtonForeground" ResourceKey="TextFillColorPrimary" />
            <StaticResource x:Key="SearchPillButtonForegroundPressed" ResourceKey="TextFillColorSecondary" />
            <StaticResource x:Key="SearchPillButtonForegroundHover" ResourceKey="TextFillColorPrimary" />
            <StaticResource x:Key="SearchPillButtonForegroundBrush" ResourceKey="TextFillColorPrimaryBrush" />
            <StaticResource x:Key="SearchPillButtonForegroundActive" ResourceKey="TextOnAccentFillColorPrimary" />
            <StaticResource x:Key="SearchPillButtonBorderPressedBrush" ResourceKey="ShellTaskbarItemStrokeColorQuinaryBrush" />
            <StaticResource x:Key="SearchPillButtonBorderHoverBrush" ResourceKey="ShellTaskbarItemGradientStrokeColorSecondaryBrush" />
            <StaticResource x:Key="SearchPillButtonBorderBrush" ResourceKey="ShellTaskbarItemGradientStrokeColorSecondaryBrush" />
            <StaticResource x:Key="SearchPillButtonBorderActiveBrush" ResourceKey="ShellTaskbarItemGradientStrokeColorSecondaryBrush" />
            <StaticResource x:Key="SearchPillButtonBackground" ResourceKey="ControlFillColorDefault" />
            <StaticResource x:Key="SearchPillButtonBackgroundPressed" ResourceKey="ControlFillColorTertiary" />
            <StaticResource x:Key="SearchPillButtonBackgroundHover" ResourceKey="ControlFillColorSecondary" />
            <LinearGradientBrush x:Key="SearchPillButtonBackgroundBrush" MappingMode="RelativeToBoundingBox" StartPoint="0.5,1.0" EndPoint="0.5,0.0">
                <LinearGradientBrush.RelativeTransform>
                    <RotateTransform Angle="147.72" CenterX="0.5" CenterY="0.5" />
                </LinearGradientBrush.RelativeTransform>
                <LinearGradientBrush.GradientStops>
                    <GradientStop Offset="-0.482" Color="{StaticResource SearchPillButtonBackground}" />
                    <GradientStop Offset="0.5723" Color="{StaticResource SearchPillButtonBackground}" />
                </LinearGradientBrush.GradientStops>
            </LinearGradientBrush>
            <Color x:Key="SearchPillButtonBackgroundActiveSecondary" Value="#FF0078D4" />
            <Color x:Key="SearchPillButtonBackgroundActivePrimary" Value="#FF4DD2FF" />
            <Thickness x:Key="SearchBoxTextControlBorderThemeThicknessFocused" Value="1" />
            <StaticResource x:Key="SearchBoxTextControlBorderBrush" ResourceKey="ControlElevationBorderBrush" />
            <StaticResource x:Key="SearchBoxTextControlBorderBrushPointerOver" ResourceKey="ControlElevationBorderBrush" />
            <StaticResource x:Key="SearchBoxTextControlBorderBrushPointerOver3" ResourceKey="ControlElevationBorderBrush" />
            <StaticResource x:Key="SearchBoxTextControlBorderBrushFocused" ResourceKey="ControlStrokeColorDefaultBrush" />
            <StaticResource x:Key="SearchBoxTextControlBorderBrushFocused3" ResourceKey="ControlStrokeColorDefaultBrush" />
            <StaticResource x:Key="SearchBoxTextControlBorderBrush3" ResourceKey="ControlElevationBorderBrush" />
            <StaticResource x:Key="SearchBoxTextControlBackground" ResourceKey="ControlFillColorDefaultBrush" />
            <StaticResource x:Key="SearchBoxTextControlBackgroundPointerOver" ResourceKey="ControlFillColorSecondaryBrush" />
            <StaticResource x:Key="SearchBoxTextControlBackgroundPointerOver3" ResourceKey="ControlFillColorSecondaryBrush" />
            <StaticResource x:Key="SearchBoxTextControlBackgroundFocused" ResourceKey="ControlFillColorInputActiveBrush" />
            <StaticResource x:Key="SearchBoxTextControlBackground3" ResourceKey="ControlFillColorDefaultBrush" />
            <SolidColorBrush x:Key="RunningIndicatorRequestingAttentionBrush" Color="#FFC42B1C" />
            <StaticResource x:Key="RunningIndicatorBrush" ResourceKey="ControlStrongStrokeColorDefaultBrush" />
            <StaticResource x:Key="RunningIndicatorActiveBrush" ResourceKey="AccentFillColorDefaultBrush" />
            <SolidColorBrush x:Key="RequestingAttentionPressedBrush" Color="#FFFEEEF0" />
            <SolidColorBrush x:Key="RequestingAttentionHoverBrush" Color="#FFF8E7E5" />
            <SolidColorBrush x:Key="RequestingAttentionBrush" Color="#FFE9AFAA" />
            <StaticResource x:Key="MostRecentlyUsedDividerBrush" ResourceKey="DividerStrokeColorBrush" />
            <Double x:Key="IconPressedOpacity" Value="0.7" />
            <StaticResource x:Key="GripperBrush" ResourceKey="ControlFillContrastNeutralBrush" />
            <Thickness x:Key="GleamEntryPointButtonBorderThemeThickness" Value="1" />
            <StaticResource x:Key="GleamEntryPointButtonBorderBrush" ResourceKey="SubtleFillColorTransparentBrush" />
            <StaticResource x:Key="GleamEntryPointButtonBorderBrushPressed" ResourceKey="SubtleFillColorTransparentBrush" />
            <StaticResource x:Key="GleamEntryPointButtonBorderBrushPointerOver" ResourceKey="SubtleFillColorTransparentBrush" />
            <StaticResource x:Key="GleamEntryPointButtonBackground" ResourceKey="SubtleFillColorTransparentBrush" />
            <StaticResource x:Key="GleamEntryPointButtonBackgroundPressed" ResourceKey="SubtleFillColorTertiaryBrush" />
            <StaticResource x:Key="GleamEntryPointButtonBackgroundPointerOver" ResourceKey="SubtleFillColorSecondaryBrush" />
            <StaticResource x:Key="ExtendedUIRecommendationForeground" ResourceKey="TextFillColorPrimaryBrush" />
            <StaticResource x:Key="ExtendedUIRecommendationForegroundPressed" ResourceKey="TextFillColorSecondaryBrush" />
            <StaticResource x:Key="ExtendedUIRecommendationForegroundPointerOver" ResourceKey="TextFillColorPrimaryBrush" />
            <StaticResource x:Key="ExtendedUIRecommendationBackground" ResourceKey="SubtleFillColorTransparentBrush" />
            <StaticResource x:Key="ExtendedUIRecommendationBackgroundPressed" ResourceKey="SubtleFillColorTertiaryBrush" />
            <StaticResource x:Key="ExtendedUIRecommendationBackgroundPointerOver" ResourceKey="SubtleFillColorSecondaryBrush" />
            <SolidColorBrush x:Key="DividerStrokeColorBrush" Color="#33000000" />
            <StaticResource x:Key="CustomThemeSearchPillButtonForeground" ResourceKey="TextFillColorPrimary" />
            <StaticResource x:Key="CustomThemeSearchPillButtonForegroundPressed" ResourceKey="TextFillColorSecondary" />
            <StaticResource x:Key="CustomThemeSearchPillButtonForegroundHover" ResourceKey="TextFillColorPrimary" />
            <StaticResource x:Key="CustomThemeSearchPillButtonForegroundBrush" ResourceKey="TextFillColorPrimaryBrush" />
            <StaticResource x:Key="CustomThemeSearchPillButtonForegroundActive" ResourceKey="TextOnAccentFillColorPrimary" />
            <StaticResource x:Key="CustomThemeSearchPillButtonBorderPressedBrush" ResourceKey="ShellTaskbarItemStrokeColorQuinaryBrush" />
            <StaticResource x:Key="CustomThemeSearchPillButtonBorderHoverBrush" ResourceKey="ShellTaskbarItemGradientStrokeColorSecondaryBrush" />
            <StaticResource x:Key="CustomThemeSearchPillButtonBorderBrush" ResourceKey="ShellTaskbarItemGradientStrokeColorSecondaryBrush" />
            <StaticResource x:Key="CustomThemeSearchPillButtonBorderActiveBrush" ResourceKey="ShellTaskbarItemGradientStrokeColorSecondaryBrush" />
            <StaticResource x:Key="CustomThemeSearchPillButtonBackground" ResourceKey="ControlFillColorDefault" />
            <StaticResource x:Key="CustomThemeSearchPillButtonBackgroundPressed" ResourceKey="ControlFillColorTertiary" />
            <StaticResource x:Key="CustomThemeSearchPillButtonBackgroundHover" ResourceKey="ControlFillColorSecondary" />
            <LinearGradientBrush x:Key="CustomThemeSearchPillButtonBackgroundBrush" MappingMode="RelativeToBoundingBox" StartPoint="0.5,1.0" EndPoint="0.5,0.0">
                <LinearGradientBrush.RelativeTransform>
                    <RotateTransform Angle="147.72" CenterX="0.5" CenterY="0.5" />
                </LinearGradientBrush.RelativeTransform>
                <LinearGradientBrush.GradientStops>
                    <GradientStop Offset="-0.482" Color="{StaticResource CustomThemeSearchPillButtonBackground}" />
                    <GradientStop Offset="0.5723" Color="{StaticResource CustomThemeSearchPillButtonBackground}" />
                </LinearGradientBrush.GradientStops>
            </LinearGradientBrush>
            <Color x:Key="CustomThemeSearchPillButtonBackgroundActiveSecondary" Value="#FF0078D4" />
            <Color x:Key="CustomThemeSearchPillButtonBackgroundActivePrimary" Value="#FF4DD2FF" />
            <StaticResource x:Key="CustomThemeSearchBoxTextControlPlaceholderForeground" ResourceKey="TextFillColorSecondaryBrush" />
            <StaticResource x:Key="CustomThemeSearchBoxTextControlPlaceholderForegroundPointerOver" ResourceKey="TextFillColorSecondaryBrush" />
            <StaticResource x:Key="CustomThemeSearchBoxTextControlPlaceholderForegroundFocused" ResourceKey="TextFillColorTertiaryBrush" />
            <StaticResource x:Key="CustomThemeSearchBoxTextControlBorderBrush" ResourceKey="ControlElevationBorderBrush" />
            <StaticResource x:Key="CustomThemeSearchBoxTextControlBorderBrushPointerOver" ResourceKey="ControlElevationBorderBrush" />
            <StaticResource x:Key="CustomThemeSearchBoxTextControlBorderBrushFocused" ResourceKey="ControlStrokeColorDefaultBrush" />
            <StaticResource x:Key="CustomThemeSearchBoxTextControlBackground" ResourceKey="ControlFillColorDefaultBrush" />
            <StaticResource x:Key="CustomThemeSearchBoxTextControlBackgroundPointerOver" ResourceKey="ControlFillColorSecondaryBrush" />
            <StaticResource x:Key="CustomThemeSearchBoxTextControlBackgroundFocused" ResourceKey="ControlFillColorInputActiveBrush" />
            <Color x:Key="ControlStrongStrokeColorDefault" Value="#72000000" />
            <SolidColorBrush x:Key="ControlStrongStrokeColorDefaultBrush" Color="{StaticResource ControlStrongStrokeColorDefault}" />
            <Color x:Key="ControlStrongFillColorDefault" Value="#72000000" />
            <SolidColorBrush x:Key="ControlStrongFillColorDefaultBrush" Color="{StaticResource ControlStrongFillColorDefault}" />
            <Color x:Key="ControlFillContrastNeutral" Value="#74000000" />
            <SolidColorBrush x:Key="ControlFillContrastNeutralBrush" Color="{StaticResource ControlFillContrastNeutral}" />
            <Color x:Key="ControlAltFillColorTertiary" Value="#0F000000" />
            <SolidColorBrush x:Key="ControlAltFillColorTertiaryBrush" Color="{StaticResource ControlAltFillColorTertiary}" />
            <SolidColorBrush x:Key="CaptionControlFillBrush" Color="#00FFFFFF" />
            <SolidColorBrush x:Key="CaptionControlBorderBrush" Color="#00FFFFFF" />
            <StaticResource x:Key="BadgeForegroundBrush" ResourceKey="TextOnAccentFillColorSelectedTextBrush" />
            <StaticResource x:Key="BadgeBackgroundBrush" ResourceKey="AccentFillColorDefaultBrush" />
            <Thickness x:Key="BackgroundElementMultiWindowRequestingAttentionMargin" Value="0,0,6,0" />
            <Thickness x:Key="BackgroundElementMultiWindowMargin" Value="0,0,4,0" />
            <Thickness x:Key="BackgroundElementMultiWindowLargeIconMargin" Value="0,0,7,0" />
            <Color x:Key="AugmentedEntryPointButtonOverlayBackgroundColor" Value="#D9FCFCFC" />
            <SolidColorBrush x:Key="AugmentedEntryPointButtonOverlayBackgroundBrush" Color="{StaticResource AugmentedEntryPointButtonOverlayBackgroundColor}" />
            <Color x:Key="AugmentedEntryPointButtonBadgeBackgroundColorWarning" Value="#FF9D5D00" />
            <Color x:Key="AugmentedEntryPointButtonBadgeBackgroundColorGood" Value="#FF0F7B0F" />
            <Color x:Key="AugmentedEntryPointButtonBadgeBackgroundColorAttention" Value="#FFC42B1C" />
            <Color x:Key="AugmentedEntryPointButtonBadgeBackgroundColorAccent" Value="#FF005FB7" />
            <StaticResource x:Key="AugmentedEntryPointButtonBadgeBackgroundBrush" ResourceKey="BadgeBackgroundBrush" />
            <SolidColorBrush x:Key="AugmentedEntryPointButtonBadgeBackgroundBrushWarning" Color="{StaticResource AugmentedEntryPointButtonBadgeBackgroundColorWarning}" />
            <SolidColorBrush x:Key="AugmentedEntryPointButtonBadgeBackgroundBrushGood" Color="{StaticResource AugmentedEntryPointButtonBadgeBackgroundColorGood}" />
            <SolidColorBrush x:Key="AugmentedEntryPointButtonBadgeBackgroundBrushAttention" Color="{StaticResource AugmentedEntryPointButtonBadgeBackgroundColorAttention}" />
            <SolidColorBrush x:Key="AugmentedEntryPointButtonBadgeBackgroundBrushAccent" Color="{StaticResource AugmentedEntryPointButtonBadgeBackgroundColorAccent}" />
            <StaticResource x:Key="AccentFillColorDefaultBrush" ResourceKey="SystemControlBackgroundAccentBrush" />
        </ResourceDictionary>
        <ResourceDictionary x:Key="Dark">
            <SolidColorBrush x:Key="WindowSharingBrush" Color="#FFFF99A4" />
            <SolidColorBrush x:Key="ThumbBarPressedBackgroundBrush" Color="#FFFFFFFF" Opacity="0.3" />
            <SolidColorBrush x:Key="ThumbBarPointerOverBackgroundBrush" Color="#FFFFFFFF" Opacity="0.23" />
            <SolidColorBrush x:Key="ThumbBarDisabledBorderBrush" Color="#FFFFFFFF" Opacity="0.05" />
            <SolidColorBrush x:Key="ThumbBarDisabledBackgroundBrush" Color="#FF808080" Opacity="0.1" />
            <SolidColorBrush x:Key="ThumbBarBorderBrush" Color="#FFFFFFFF" Opacity="0.15" />
            <SolidColorBrush x:Key="ThumbBarBackgroundBrush" Color="#FFFFFFFF" Opacity="0.15" />
            <StaticResource x:Key="TextOnAccentFillColorSelectedTextBrush" ResourceKey="SystemControlForegroundAltHighBrush" />
            <ElementTheme x:Key="TaskbarTheme" Value="Dark" />
            <StaticResource x:Key="TaskbarRecommendatonDividerBrush" ResourceKey="DividerStrokeColorBrush" />
            <SolidColorBrush x:Key="TaskbarButtonPlateBrush" Opacity="0.0605" Color="#FFFFFFFF" />
            <StaticResource x:Key="TaskListLabelForegroundBrush" ResourceKey="TextFillColorPrimaryBrush" />
            <SolidColorBrush x:Key="TaskItemThumbnailTextStroke" Color="#C8FFFFFF" />
            <SolidColorBrush x:Key="TaskItemThumbnailBackgroundPressedBrush" Color="#08FFFFFF" />
            <SolidColorBrush x:Key="TaskItemThumbnailBackgroundPointerOverBrush" Color="#0FFFFFFF" />
            <SolidColorBrush x:Key="TaskItemThumbnailBackgroundBrush" Color="#00FFFFFF" />
            <SolidColorBrush x:Key="TaskItemThumbnailAltTextStroke" Color="#C8FFFFFF" />
            <SolidColorBrush x:Key="StrokeShadowBrush" Opacity="0.08" Color="#FFFFFFFF" />
            <Color x:Key="ShellTaskbarItemStrokeColorTertiary" Value="#15FFFFFF" />
            <SolidColorBrush x:Key="ShellTaskbarItemStrokeColorTertiaryBrush" Color="{StaticResource ShellTaskbarItemStrokeColorTertiary}" />
            <Color x:Key="ShellTaskbarItemStrokeColorSecondary" Value="#1AFFFFFF" />
            <Color x:Key="ShellTaskbarItemStrokeColorQuinary" Value="#0AFFFFFF" />
            <SolidColorBrush x:Key="ShellTaskbarItemStrokeColorQuinaryBrush" Color="{StaticResource ShellTaskbarItemStrokeColorQuinary}" />
            <Color x:Key="ShellTaskbarItemStrokeColorQuarternary" Value="#0FFFFFFF" />
            <Color x:Key="ShellTaskbarItemStrokeColorPrimary" Value="#20FFFFFF" />
            <LinearGradientBrush x:Key="ShellTaskbarItemGradientStrokeColorSecondaryBrush" MappingMode="Absolute" StartPoint="0,0" EndPoint="0,3">
                <LinearGradientBrush.GradientStops>
                    <GradientStop Offset="0.33" Color="{StaticResource ShellTaskbarItemStrokeColorSecondary}" />
                    <GradientStop Offset="1" Color="{StaticResource ShellTaskbarItemStrokeColorQuarternary}" />
                </LinearGradientBrush.GradientStops>
            </LinearGradientBrush>
            <StaticResource x:Key="ShellTaskbarItemGradientStrokeColorPrimaryBrush" ResourceKey="ShellTaskbarItemGradientStrokeColorSecondaryBrush" />
            <Color x:Key="ShellTaskbarItemFillColorTransparent" Value="#00FFFFFF" />
            <SolidColorBrush x:Key="ShellTaskbarItemFillColorTransparentBrush" Color="{StaticResource ShellTaskbarItemFillColorTransparent}" />
            <Color x:Key="ShellTaskbarItemFillColorTertiary" Value="#0AFFFFFF" />
            <SolidColorBrush x:Key="ShellTaskbarItemFillColorTertiaryBrush" Color="{StaticResource ShellTaskbarItemFillColorTertiary}" />
            <Color x:Key="ShellTaskbarItemFillColorSecondary" Value="#0FFFFFFF" />
            <SolidColorBrush x:Key="ShellTaskbarItemFillColorSecondaryBrush" Color="{StaticResource ShellTaskbarItemFillColorSecondary}" />
            <Color x:Key="ShellTaskbarItemFillColorPrimary" Value="#15FFFFFF" />
            <SolidColorBrush x:Key="ShellTaskbarItemFillColorPrimaryBrush" Color="{StaticResource ShellTaskbarItemFillColorPrimary}" />
            <SolidColorBrush x:Key="ShellTaskbarItemFillColorAltSecondaryBrush" Color="{StaticResource ShellTaskbarItemFillColorSecondary}" />
            <SolidColorBrush x:Key="ShareWindow_ConfirmPressedForegroundBrush" Color="#FF000000" />
            <SolidColorBrush x:Key="ShareWindow_ConfirmPressedBorderBrush" Color="#D07BCF6E" />
            <SolidColorBrush x:Key="ShareWindow_ConfirmPressedBackgroundBrush" Color="#CC6BCA5F" />
            <SolidColorBrush x:Key="ShareWindow_ConfirmPointerOverForegroundBrush" Color="#FF000000" />
            <SolidColorBrush x:Key="ShareWindow_ConfirmPointerOverBorderBrush" Color="#E778D16C" />
            <SolidColorBrush x:Key="ShareWindow_ConfirmPointerOverBackgroundBrush" Color="#E56CCC5F" />
            <SolidColorBrush x:Key="ShareWindow_ConfirmForegroundBrush" Color="#FF000000" />
            <SolidColorBrush x:Key="ShareWindow_ConfirmBorderBrush" Color="#FF78CF6C" />
            <SolidColorBrush x:Key="ShareWindow_ConfirmBackgroundBrush" Color="#FF6CCB5F" />
            <SolidColorBrush x:Key="ShareWindowButtonStopSharingStrokeBrush" Opacity="0.08" Color="#FFFFFFFF" />
            <SolidColorBrush x:Key="ShareWindowButtonStopSharingShadowBrush" Opacity="0.3" Color="#FF000000" />
            <SolidColorBrush x:Key="ShareWindowButtonStopSharingBackgroundPressedBrush" Color="#FF8E192E" />
            <SolidColorBrush x:Key="ShareWindowButtonStopSharingBackgroundPointerOverBrush" Color="#FFA72037" />
            <SolidColorBrush x:Key="ShareWindowButtonStopSharingBackgroundBrush" Color="#FFC4314B" />
            <SolidColorBrush x:Key="ShareWindowButtonForegroundPressedBrush" Opacity="0.5442" Color="#FFFFFFFF" />
            <SolidColorBrush x:Key="ShareWindowButtonBackgroundPressedBrush" Opacity="0.0419" Color="#FFFFFFFF" />
            <SolidColorBrush x:Key="ShareWindowButtonBackgroundPointerOverBrush" Opacity="0.0837" Color="#FFFFFFFF" />
            <SolidColorBrush x:Key="ShareWindowButtonBackgroundBrush" Opacity="0.0605" Color="#FFFFFFFF" />
            <SolidColorBrush x:Key="ShadowBrush" Opacity="0.1044" Color="#FFFFFFFF" />
            <StaticResource x:Key="SearchPillButtonForeground" ResourceKey="TextFillColorPrimary" />
            <StaticResource x:Key="SearchPillButtonForegroundPressed" ResourceKey="TextFillColorSecondary" />
            <StaticResource x:Key="SearchPillButtonForegroundHover" ResourceKey="TextFillColorPrimary" />
            <StaticResource x:Key="SearchPillButtonForegroundBrush" ResourceKey="TextFillColorPrimaryBrush" />
            <StaticResource x:Key="SearchPillButtonForegroundActive" ResourceKey="TextFillColorPrimary" />
            <StaticResource x:Key="SearchPillButtonBorderPressedBrush" ResourceKey="ShellTaskbarItemStrokeColorQuinaryBrush" />
            <StaticResource x:Key="SearchPillButtonBorderHoverBrush" ResourceKey="ShellTaskbarItemGradientStrokeColorSecondaryBrush" />
            <StaticResource x:Key="SearchPillButtonBorderBrush" ResourceKey="ShellTaskbarItemGradientStrokeColorSecondaryBrush" />
            <StaticResource x:Key="SearchPillButtonBorderActiveBrush" ResourceKey="ShellTaskbarItemGradientStrokeColorSecondaryBrush" />
            <StaticResource x:Key="SearchPillButtonBackground" ResourceKey="ControlFillColorDefault" />
            <StaticResource x:Key="SearchPillButtonBackgroundPressed" ResourceKey="ControlFillColorTertiary" />
            <StaticResource x:Key="SearchPillButtonBackgroundHover" ResourceKey="ControlFillColorSecondary" />
            <LinearGradientBrush x:Key="SearchPillButtonBackgroundBrush" MappingMode="RelativeToBoundingBox" StartPoint="0.5,1.0" EndPoint="0.5,0.0">
                <LinearGradientBrush.RelativeTransform>
                    <RotateTransform Angle="147.72" CenterX="0.5" CenterY="0.5" />
                </LinearGradientBrush.RelativeTransform>
                <LinearGradientBrush.GradientStops>
                    <GradientStop Offset="-0.482" Color="{StaticResource SearchPillButtonBackground}" />
                    <GradientStop Offset="0.5723" Color="{StaticResource SearchPillButtonBackground}" />
                </LinearGradientBrush.GradientStops>
            </LinearGradientBrush>
            <Color x:Key="SearchPillButtonBackgroundActiveSecondary" Value="#FF0078D4" />
            <Color x:Key="SearchPillButtonBackgroundActivePrimary" Value="#FF4DD2FF" />
            <Thickness x:Key="SearchBoxTextControlBorderThemeThicknessFocused" Value="1" />
            <StaticResource x:Key="SearchBoxTextControlBorderBrush" ResourceKey="ControlElevationBorderBrush" />
            <StaticResource x:Key="SearchBoxTextControlBorderBrushPointerOver" ResourceKey="ControlElevationBorderBrush" />
            <StaticResource x:Key="SearchBoxTextControlBorderBrushPointerOver3" ResourceKey="SearchBoxControlElevationBorderBrush3" />
            <StaticResource x:Key="SearchBoxTextControlBorderBrushFocused" ResourceKey="ControlStrokeColorDefaultBrush" />
            <SolidColorBrush x:Key="SearchBoxTextControlBorderBrushFocused3" Color="#FFFFFFFF" Opacity="0.18" />
            <StaticResource x:Key="SearchBoxTextControlBorderBrush3" ResourceKey="SearchBoxControlElevationBorderBrush3" />
            <SolidColorBrush x:Key="SearchBoxTextControlBackground" Color="#FFFFFFFF" Opacity="0.1166" />
            <SolidColorBrush x:Key="SearchBoxTextControlBackgroundPointerOver" Color="#FFFFFFFF" Opacity="0.1345" />
            <SolidColorBrush x:Key="SearchBoxTextControlBackgroundPointerOver3" Color="#FFFFFFFF" Opacity="0.1659" />
            <StaticResource x:Key="SearchBoxTextControlBackgroundFocused" ResourceKey="ControlFillColorInputActiveBrush" />
            <SolidColorBrush x:Key="SearchBoxTextControlBackground3" Color="#FFFFFFFF" Opacity="0.1435" />
            <LinearGradientBrush x:Key="SearchBoxControlElevationBorderBrush3" MappingMode="Absolute" StartPoint="0,0" EndPoint="0,3">
                <LinearGradientBrush.GradientStops>
                    <GradientStop Offset="0.33" Color="#4DFFFFFF" />
                    <GradientStop Offset="1" Color="#2EFFFFFF" />
                </LinearGradientBrush.GradientStops>
            </LinearGradientBrush>
            <SolidColorBrush x:Key="RunningIndicatorRequestingAttentionBrush" Color="#FFFF99A4" />
            <StaticResource x:Key="RunningIndicatorBrush" ResourceKey="ControlStrongStrokeColorDefaultBrush" />
            <StaticResource x:Key="RunningIndicatorActiveBrush" ResourceKey="AccentFillColorDefaultBrush" />
            <SolidColorBrush x:Key="RequestingAttentionPressedBrush" Color="#FF4C302F" />
            <SolidColorBrush x:Key="RequestingAttentionHoverBrush" Color="#FF8A5259" />
            <SolidColorBrush x:Key="RequestingAttentionBrush" Color="#FF7F434A" />
            <StaticResource x:Key="MostRecentlyUsedDividerBrush" ResourceKey="DividerStrokeColorBrush" />
            <Double x:Key="IconPressedOpacity" Value="0.8" />
            <StaticResource x:Key="GripperBrush" ResourceKey="ControlStrongStrokeColorDefaultBrush" />
            <Thickness x:Key="GleamEntryPointButtonBorderThemeThickness" Value="1" />
            <StaticResource x:Key="GleamEntryPointButtonBorderBrush" ResourceKey="SubtleFillColorTransparentBrush" />
            <StaticResource x:Key="GleamEntryPointButtonBorderBrushPressed" ResourceKey="SubtleFillColorTransparentBrush" />
            <StaticResource x:Key="GleamEntryPointButtonBorderBrushPointerOver" ResourceKey="SubtleFillColorTransparentBrush" />
            <StaticResource x:Key="GleamEntryPointButtonBackground" ResourceKey="SubtleFillColorTransparentBrush" />
            <StaticResource x:Key="GleamEntryPointButtonBackgroundPressed" ResourceKey="SubtleFillColorTertiaryBrush" />
            <StaticResource x:Key="GleamEntryPointButtonBackgroundPointerOver" ResourceKey="SubtleFillColorSecondaryBrush" />
            <StaticResource x:Key="ExtendedUIRecommendationForeground" ResourceKey="TextFillColorPrimaryBrush" />
            <StaticResource x:Key="ExtendedUIRecommendationForegroundPressed" ResourceKey="TextFillColorSecondaryBrush" />
            <StaticResource x:Key="ExtendedUIRecommendationForegroundPointerOver" ResourceKey="TextFillColorPrimaryBrush" />
            <StaticResource x:Key="ExtendedUIRecommendationBackground" ResourceKey="SubtleFillColorTransparentBrush" />
            <StaticResource x:Key="ExtendedUIRecommendationBackgroundPressed" ResourceKey="SubtleFillColorTertiaryBrush" />
            <StaticResource x:Key="ExtendedUIRecommendationBackgroundPointerOver" ResourceKey="SubtleFillColorSecondaryBrush" />
            <SolidColorBrush x:Key="DividerStrokeColorBrush" Color="#33FFFFFF" />
            <LinearGradientBrush x:Key="CustomThemeShellTaskbarItemGradientStrokeColorSecondaryBrush" MappingMode="Absolute" StartPoint="0,0" EndPoint="0,3">
                <LinearGradientBrush.RelativeTransform>
                    <ScaleTransform ScaleY="-1" CenterY="0.5" />
                </LinearGradientBrush.RelativeTransform>
                <LinearGradientBrush.GradientStops>
                    <GradientStop Offset="0.33" Color="#0F000000" />
                    <GradientStop Offset="1" Color="#05000000" />
                </LinearGradientBrush.GradientStops>
            </LinearGradientBrush>
            <Color x:Key="CustomThemeSearchPillButtonForeground" Value="#E4000000" />
            <Color x:Key="CustomThemeSearchPillButtonForegroundPressed" Value="#9E000000" />
            <Color x:Key="CustomThemeSearchPillButtonForegroundHover" Value="#E4000000" />
            <SolidColorBrush x:Key="CustomThemeSearchPillButtonForegroundBrush" Color="#E4000000" />
            <StaticResource x:Key="CustomThemeSearchPillButtonForegroundActive" ResourceKey="TextFillColorPrimary" />
            <SolidColorBrush x:Key="CustomThemeSearchPillButtonBorderPressedBrush" Color="#05000000" />
            <StaticResource x:Key="CustomThemeSearchPillButtonBorderHoverBrush" ResourceKey="CustomThemeShellTaskbarItemGradientStrokeColorSecondaryBrush" />
            <StaticResource x:Key="CustomThemeSearchPillButtonBorderBrush" ResourceKey="CustomThemeShellTaskbarItemGradientStrokeColorSecondaryBrush" />
            <StaticResource x:Key="CustomThemeSearchPillButtonBorderActiveBrush" ResourceKey="CustomThemeShellTaskbarItemGradientStrokeColorSecondaryBrush" />
            <Color x:Key="CustomThemeSearchPillButtonBackground" Value="#F2FFFFFF" />
            <Color x:Key="CustomThemeSearchPillButtonBackgroundPressed" Value="#CEFFFFFF" />
            <Color x:Key="CustomThemeSearchPillButtonBackgroundHover" Value="#E5FFFFFF" />
            <LinearGradientBrush x:Key="CustomThemeSearchPillButtonBackgroundBrush" MappingMode="RelativeToBoundingBox" StartPoint="0.5,1.0" EndPoint="0.5,0.0">
                <LinearGradientBrush.RelativeTransform>
                    <RotateTransform Angle="147.72" CenterX="0.5" CenterY="0.5" />
                </LinearGradientBrush.RelativeTransform>
                <LinearGradientBrush.GradientStops>
                    <GradientStop Offset="-0.482" Color="{StaticResource CustomThemeSearchPillButtonBackground}" />
                    <GradientStop Offset="0.5723" Color="{StaticResource CustomThemeSearchPillButtonBackground}" />
                </LinearGradientBrush.GradientStops>
            </LinearGradientBrush>
            <Color x:Key="CustomThemeSearchPillButtonBackgroundActiveSecondary" Value="#FF0078D4" />
            <Color x:Key="CustomThemeSearchPillButtonBackgroundActivePrimary" Value="#FF4DD2FF" />
            <SolidColorBrush x:Key="CustomThemeSearchBoxTextControlPlaceholderForeground" Color="#9E000000" />
            <SolidColorBrush x:Key="CustomThemeSearchBoxTextControlPlaceholderForegroundPointerOver" Color="#9E000000" />
            <SolidColorBrush x:Key="CustomThemeSearchBoxTextControlPlaceholderForegroundFocused" Color="#72000000" />
            <StaticResource x:Key="CustomThemeSearchBoxTextControlBorderBrush" ResourceKey="CustomThemeSearchBoxControlElevationBorderBrush" />
            <StaticResource x:Key="CustomThemeSearchBoxTextControlBorderBrushPointerOver" ResourceKey="CustomThemeSearchBoxControlElevationBorderBrush" />
            <SolidColorBrush x:Key="CustomThemeSearchBoxTextControlBorderBrushFocused" Color="#0F000000" />
            <SolidColorBrush x:Key="CustomThemeSearchBoxTextControlBackground" Color="#FFFFFFFF" Opacity="0.9462" />
            <SolidColorBrush x:Key="CustomThemeSearchBoxTextControlBackgroundPointerOver" Color="#FFFFFFFF" Opacity="0.8969" />
            <SolidColorBrush x:Key="CustomThemeSearchBoxTextControlBackgroundFocused" Color="#FFFFFFFF" />
            <LinearGradientBrush x:Key="CustomThemeSearchBoxControlElevationBorderBrush" MappingMode="Absolute" StartPoint="0,0" EndPoint="0,3">
                <LinearGradientBrush.RelativeTransform>
                    <ScaleTransform ScaleY="-1" CenterY="0.5" />
                </LinearGradientBrush.RelativeTransform>
                <LinearGradientBrush.GradientStops>
                    <GradientStop Offset="0.33" Color="#29000000" />
                    <GradientStop Offset="1" Color="#0F000000" />
                </LinearGradientBrush.GradientStops>
            </LinearGradientBrush>
            <Color x:Key="ControlStrongStrokeColorDefault" Value="#8BFFFFFF" />
            <SolidColorBrush x:Key="ControlStrongStrokeColorDefaultBrush" Color="{StaticResource ControlStrongStrokeColorDefault}" />
            <Color x:Key="ControlStrongFillColorDefault" Value="#8BFFFFFF" />
            <SolidColorBrush x:Key="ControlStrongFillColorDefaultBrush" Color="{StaticResource ControlStrongFillColorDefault}" />
            <Color x:Key="ControlAltFillColorTertiary" Value="#0BFFFFFF" />
            <SolidColorBrush x:Key="ControlAltFillColorTertiaryBrush" Color="{StaticResource ControlAltFillColorTertiary}" />
            <SolidColorBrush x:Key="CaptionControlFillBrush" Color="#00FFFFFF" />
            <SolidColorBrush x:Key="CaptionControlBorderBrush" Color="#00FFFFFF" />
            <StaticResource x:Key="BadgeForegroundBrush" ResourceKey="TextOnAccentFillColorSelectedTextBrush" />
            <StaticResource x:Key="BadgeBackgroundBrush" ResourceKey="AccentFillColorDefaultBrush" />
            <Thickness x:Key="BackgroundElementMultiWindowRequestingAttentionMargin" Value="0,0,5,0" />
            <Thickness x:Key="BackgroundElementMultiWindowMargin" Value="0,0,5,0" />
            <Thickness x:Key="BackgroundElementMultiWindowLargeIconMargin" Value="0,0,7,0" />
            <Color x:Key="AugmentedEntryPointButtonOverlayBackgroundColor" Value="#F52C2C2C" />
            <SolidColorBrush x:Key="AugmentedEntryPointButtonOverlayBackgroundBrush" Color="{StaticResource AugmentedEntryPointButtonOverlayBackgroundColor}" />
            <Color x:Key="AugmentedEntryPointButtonBadgeBackgroundColor" Value="#00000000" />
            <Color x:Key="AugmentedEntryPointButtonBadgeBackgroundColorWarning" Value="#FFFCE100" />
            <Color x:Key="AugmentedEntryPointButtonBadgeBackgroundColorGood" Value="#FF6CCB5F" />
            <Color x:Key="AugmentedEntryPointButtonBadgeBackgroundColorAttention" Value="#FFFF99A4" />
            <Color x:Key="AugmentedEntryPointButtonBadgeBackgroundColorAccent" Value="#FF60CDFF" />
            <StaticResource x:Key="AugmentedEntryPointButtonBadgeBackgroundBrush" ResourceKey="BadgeBackgroundBrush" />
            <SolidColorBrush x:Key="AugmentedEntryPointButtonBadgeBackgroundBrushWarning" Color="{StaticResource AugmentedEntryPointButtonBadgeBackgroundColorWarning}" />
            <SolidColorBrush x:Key="AugmentedEntryPointButtonBadgeBackgroundBrushGood" Color="{StaticResource AugmentedEntryPointButtonBadgeBackgroundColorGood}" />
            <SolidColorBrush x:Key="AugmentedEntryPointButtonBadgeBackgroundBrushAttention" Color="{StaticResource AugmentedEntryPointButtonBadgeBackgroundColorAttention}" />
            <SolidColorBrush x:Key="AugmentedEntryPointButtonBadgeBackgroundBrushAccent" Color="{StaticResource AugmentedEntryPointButtonBadgeBackgroundColorAccent}" />
            <SolidColorBrush x:Key="AccentFillColorDefaultBrush" Color="{ThemeResource SystemAccentColorLight2}" />
        </ResourceDictionary>
        <ResourceDictionary x:Key="HighContrast">
            <StaticResource x:Key="WindowSharingBrush" ResourceKey="SystemControlForegroundBaseHighBrush" />
            <StaticResource x:Key="ThumbBarPressedBackgroundBrush" ResourceKey="SystemColorHighlightTextBrush" />
            <StaticResource x:Key="ThumbBarPointerOverBackgroundBrush" ResourceKey="SystemColorHighlightBrush" />
            <StaticResource x:Key="ThumbBarDisabledBorderBrush" ResourceKey="SystemColorGrayTextBrush" />
            <StaticResource x:Key="ThumbBarDisabledBackgroundBrush" ResourceKey="SystemColorInactiveCaptionTextBrush" />
            <StaticResource x:Key="ThumbBarBorderBrush" ResourceKey="SystemColorButtonTextBrush" />
            <StaticResource x:Key="ThumbBarBackgroundBrush" ResourceKey="SystemColorWindowBrush" />
            <StaticResource x:Key="TextOnAccentFillColorSelectedTextBrush" ResourceKey="SystemControlHighlightAltAccentBrush" />
            <ElementTheme x:Key="TaskbarTheme" Value="Default" />
            <StaticResource x:Key="TaskbarRecommendatonDividerBrush" ResourceKey="SystemColorWindowTextColorBrush" />
            <StaticResource x:Key="TaskbarButtonPlateBrush" ResourceKey="SystemControlHighlightAltAccentBrush" />
            <StaticResource x:Key="TaskListLabelForegroundBrush" ResourceKey="SystemColorButtonTextColorBrush" />
            <StaticResource x:Key="TaskItemThumbnailTextStroke" ResourceKey="SystemColorWindowTextColor" />
            <StaticResource x:Key="TaskItemThumbnailBackgroundPressedBrush" ResourceKey="SystemColorWindowColor" />
            <StaticResource x:Key="TaskItemThumbnailBackgroundPointerOverBrush" ResourceKey="SystemColorHighlightTextColor" />
            <StaticResource x:Key="TaskItemThumbnailBackgroundBrush" ResourceKey="SystemColorWindowColor" />
            <StaticResource x:Key="TaskItemThumbnailAltTextStroke" ResourceKey="SystemColorHighlightColor" />
            <StaticResource x:Key="StrokeShadowBrush" ResourceKey="ButtonBorderBrush" />
            <StaticResource x:Key="ShellTaskbarItemStrokeColorQuinaryBrush" ResourceKey="SystemControlHighlightAccentBrush" />
            <StaticResource x:Key="ShellTaskbarItemGradientStrokeColorSecondaryBrush" ResourceKey="SystemControlHighlightAccentBrush" />
            <StaticResource x:Key="ShellTaskbarItemGradientStrokeColorPrimaryBrush" ResourceKey="SystemControlHighlightAccentBrush" />
            <StaticResource x:Key="ShellTaskbarItemFillColorTransparentBrush" ResourceKey="SystemControlPageBackgroundAltMediumBrush" />
            <StaticResource x:Key="ShellTaskbarItemFillColorTertiaryBrush" ResourceKey="SystemControlPageBackgroundAltMediumBrush" />
            <StaticResource x:Key="ShellTaskbarItemFillColorSecondaryBrush" ResourceKey="SystemControlPageBackgroundAltMediumBrush" />
            <StaticResource x:Key="ShellTaskbarItemFillColorPrimaryBrush" ResourceKey="SystemControlHighlightAltAccentBrush" />
            <StaticResource x:Key="ShellTaskbarItemFillColorAltSecondaryBrush" ResourceKey="SystemControlHighlightAltAccentBrush" />
            <StaticResource x:Key="ShareWindow_ConfirmPressedForegroundBrush" ResourceKey="SystemColorHighlightBrush" />
            <StaticResource x:Key="ShareWindow_ConfirmPressedBorderBrush" ResourceKey="SystemColorHighlightBrush" />
            <StaticResource x:Key="ShareWindow_ConfirmPressedBackgroundBrush" ResourceKey="SystemColorHighlightTextColor" />
            <StaticResource x:Key="ShareWindow_ConfirmPointerOverForegroundBrush" ResourceKey="SystemColorButtonFaceBrush" />
            <StaticResource x:Key="ShareWindow_ConfirmPointerOverBorderBrush" ResourceKey="SystemColorHighlightBrush" />
            <StaticResource x:Key="ShareWindow_ConfirmPointerOverBackgroundBrush" ResourceKey="SystemColorButtonTextBrush" />
            <StaticResource x:Key="ShareWindow_ConfirmForegroundBrush" ResourceKey="SystemColorHighlightTextColor" />
            <StaticResource x:Key="ShareWindow_ConfirmBorderBrush" ResourceKey="SystemColorBackgroundBrush" />
            <StaticResource x:Key="ShareWindow_ConfirmBackgroundBrush" ResourceKey="SystemColorHighlightBrush" />
            <StaticResource x:Key="ShareWindowButtonStopSharingStrokeBrush" ResourceKey="ButtonBorderBrush" />
            <StaticResource x:Key="ShareWindowButtonStopSharingShadowBrush" ResourceKey="SystemControlTransparentBrush" />
            <StaticResource x:Key="ShareWindowButtonStopSharingBackgroundPressedBrush" ResourceKey="SystemControlHighlightAltAccentBrush" />
            <StaticResource x:Key="ShareWindowButtonStopSharingBackgroundPointerOverBrush" ResourceKey="SystemControlHighlightAltAccentBrush" />
            <StaticResource x:Key="ShareWindowButtonStopSharingBackgroundBrush" ResourceKey="ButtonBackgroundThemeBrush" />
            <StaticResource x:Key="ShareWindowButtonForegroundPressedBrush" ResourceKey="SystemControlHighlightAccentBrush" />
            <StaticResource x:Key="ShareWindowButtonBackgroundPressedBrush" ResourceKey="SystemControlHighlightAltAccentBrush" />
            <StaticResource x:Key="ShareWindowButtonBackgroundPointerOverBrush" ResourceKey="SystemControlHighlightAltAccentBrush" />
            <StaticResource x:Key="ShareWindowButtonBackgroundBrush" ResourceKey="SystemControlBackgroundAccentBrush" />
            <StaticResource x:Key="ShadowBrush" ResourceKey="SystemControlTransparentBrush" />
            <StaticResource x:Key="SearchPillButtonForegroundPressedBrush" ResourceKey="SystemColorHighlightColorBrush" />
            <StaticResource x:Key="SearchPillButtonForegroundHoverBrush" ResourceKey="SystemColorHighlightColorBrush" />
            <StaticResource x:Key="SearchPillButtonForegroundBrush" ResourceKey="SystemColorButtonTextColorBrush" />
            <StaticResource x:Key="SearchPillButtonForegroundActiveBrush" ResourceKey="SystemColorHighlightColorBrush" />
            <StaticResource x:Key="SearchPillButtonBorderPressedBrush" ResourceKey="SystemColorHighlightColorBrush" />
            <StaticResource x:Key="SearchPillButtonBorderHoverBrush" ResourceKey="SystemColorHighlightColorBrush" />
            <StaticResource x:Key="SearchPillButtonBorderBrush" ResourceKey="SystemColorButtonTextColorBrush" />
            <StaticResource x:Key="SearchPillButtonBorderActiveBrush" ResourceKey="SystemColorHighlightColorBrush" />
            <StaticResource x:Key="SearchPillButtonBackgroundPressedBrush" ResourceKey="SystemColorButtonFaceColorBrush" />
            <StaticResource x:Key="SearchPillButtonBackgroundHoverBrush" ResourceKey="SystemColorHighlightTextColorBrush" />
            <StaticResource x:Key="SearchPillButtonBackgroundBrush" ResourceKey="SystemColorButtonFaceColorBrush" />
            <StaticResource x:Key="SearchPillButtonBackgroundActiveBrush" ResourceKey="SystemColorButtonFaceColorBrush" />
            <Thickness x:Key="SearchBoxTextControlBorderThemeThicknessFocused" Value="2" />
            <StaticResource x:Key="SearchBoxTextControlBorderBrush" ResourceKey="SystemColorButtonTextColorBrush" />
            <StaticResource x:Key="SearchBoxTextControlBorderBrushPointerOver" ResourceKey="SystemColorHighlightColorBrush" />
            <StaticResource x:Key="SearchBoxTextControlBorderBrushPointerOver3" ResourceKey="SystemColorHighlightColorBrush" />
            <StaticResource x:Key="SearchBoxTextControlBorderBrushFocused" ResourceKey="SystemColorHighlightColorBrush" />
            <StaticResource x:Key="SearchBoxTextControlBorderBrushFocused3" ResourceKey="SystemColorHighlightColorBrush" />
            <StaticResource x:Key="SearchBoxTextControlBorderBrush3" ResourceKey="SystemColorButtonTextColorBrush" />
            <StaticResource x:Key="SearchBoxTextControlBackground" ResourceKey="SystemColorButtonFaceColorBrush" />
            <StaticResource x:Key="SearchBoxTextControlBackgroundPointerOver" ResourceKey="SystemColorButtonFaceColorBrush" />
            <StaticResource x:Key="SearchBoxTextControlBackgroundPointerOver3" ResourceKey="SystemColorButtonFaceColorBrush" />
            <StaticResource x:Key="SearchBoxTextControlBackgroundFocused" ResourceKey="SystemColorButtonFaceColorBrush" />
            <StaticResource x:Key="SearchBoxTextControlBackground3" ResourceKey="SystemColorButtonFaceColorBrush" />
            <StaticResource x:Key="ScrollBarButtonArrowForeground" ResourceKey="SystemColorButtonTextColorBrush" />
            <StaticResource x:Key="ScrollBarButtonArrowForegroundPressed" ResourceKey="SystemColorHighlightColorBrush" />
            <StaticResource x:Key="ScrollBarButtonArrowForegroundPointerOver" ResourceKey="SystemColorHighlightColorBrush" />
            <StaticResource x:Key="ScrollBarButtonArrowForegroundDisabled" ResourceKey="SystemColorGrayTextColorBrush" />
            <StaticResource x:Key="RunningIndicatorRequestingAttentionBrush" ResourceKey="SystemColorHighlightTextBrush" />
            <StaticResource x:Key="RunningIndicatorBrush" ResourceKey="SystemControlHighlightAccentBrush" />
            <StaticResource x:Key="RunningIndicatorActiveBrush" ResourceKey="SystemControlHighlightAccentBrush" />
            <StaticResource x:Key="RequestingAttentionPressedBrush" ResourceKey="SystemColorHighlightBrush" />
            <StaticResource x:Key="RequestingAttentionHoverBrush" ResourceKey="SystemColorHighlightBrush" />
            <StaticResource x:Key="RequestingAttentionBrush" ResourceKey="SystemColorHighlightBrush" />
            <StaticResource x:Key="MostRecentlyUsedDividerBrush" ResourceKey="SystemColorWindowTextColorBrush" />
            <Double x:Key="IconPressedOpacity" Value="1" />
            <StaticResource x:Key="GripperBrush" ResourceKey="SystemControlHighlightAccentBrush" />
            <Thickness x:Key="GleamEntryPointButtonBorderThemeThickness" Value="1" />
            <StaticResource x:Key="GleamEntryPointButtonBorderBrush" ResourceKey="SystemColorButtonFaceColorBrush" />
            <StaticResource x:Key="GleamEntryPointButtonBorderBrushPressed" ResourceKey="SystemColorHighlightColorBrush" />
            <StaticResource x:Key="GleamEntryPointButtonBorderBrushPointerOver" ResourceKey="SystemColorHighlightColorBrush" />
            <StaticResource x:Key="GleamEntryPointButtonBackground" ResourceKey="SystemColorButtonFaceColorBrush" />
            <StaticResource x:Key="GleamEntryPointButtonBackgroundPressed" ResourceKey="SystemColorButtonFaceColorBrush" />
            <StaticResource x:Key="GleamEntryPointButtonBackgroundPointerOver" ResourceKey="SystemColorHighlightTextColorBrush" />
            <StaticResource x:Key="ExtendedUIRecommendationForeground" ResourceKey="SystemColorWindowTextColor" />
            <StaticResource x:Key="ExtendedUIRecommendationForegroundPressed" ResourceKey="SystemColorHighlightTextColor" />
            <StaticResource x:Key="ExtendedUIRecommendationForegroundPointerOver" ResourceKey="SystemColorHighlightTextColor" />
            <StaticResource x:Key="ExtendedUIRecommendationBackground" ResourceKey="SubtleFillColorTransparentBrush" />
            <StaticResource x:Key="ExtendedUIRecommendationBackgroundPressed" ResourceKey="SystemColorHighlightColor" />
            <StaticResource x:Key="ExtendedUIRecommendationBackgroundPointerOver" ResourceKey="SystemColorHighlightColor" />
            <StaticResource x:Key="CustomThemeSearchPillButtonForegroundPressedBrush" ResourceKey="SystemColorHighlightColorBrush" />
            <StaticResource x:Key="CustomThemeSearchPillButtonForegroundHoverBrush" ResourceKey="SystemColorHighlightColorBrush" />
            <StaticResource x:Key="CustomThemeSearchPillButtonForegroundBrush" ResourceKey="SystemColorButtonTextColorBrush" />
            <StaticResource x:Key="CustomThemeSearchPillButtonForegroundActiveBrush" ResourceKey="SystemColorHighlightColorBrush" />
            <StaticResource x:Key="CustomThemeSearchPillButtonBorderPressedBrush" ResourceKey="SystemColorHighlightColorBrush" />
            <StaticResource x:Key="CustomThemeSearchPillButtonBorderHoverBrush" ResourceKey="SystemColorHighlightColorBrush" />
            <StaticResource x:Key="CustomThemeSearchPillButtonBorderBrush" ResourceKey="SystemColorButtonTextColorBrush" />
            <StaticResource x:Key="CustomThemeSearchPillButtonBorderActiveBrush" ResourceKey="SystemColorHighlightColorBrush" />
            <StaticResource x:Key="CustomThemeSearchPillButtonBackgroundPressedBrush" ResourceKey="SystemColorButtonFaceColorBrush" />
            <StaticResource x:Key="CustomThemeSearchPillButtonBackgroundHoverBrush" ResourceKey="SystemColorHighlightTextColorBrush" />
            <StaticResource x:Key="CustomThemeSearchPillButtonBackgroundBrush" ResourceKey="SystemColorButtonFaceColorBrush" />
            <StaticResource x:Key="CustomThemeSearchPillButtonBackgroundActiveBrush" ResourceKey="SystemColorButtonFaceColorBrush" />
            <StaticResource x:Key="CustomThemeSearchBoxTextControlPlaceholderForeground" ResourceKey="SystemColorWindowTextColorBrush" />
            <StaticResource x:Key="CustomThemeSearchBoxTextControlPlaceholderForegroundPointerOver" ResourceKey="SystemColorWindowTextColorBrush" />
            <StaticResource x:Key="CustomThemeSearchBoxTextControlPlaceholderForegroundFocused" ResourceKey="SystemColorButtonTextColorBrush" />
            <StaticResource x:Key="CustomThemeSearchBoxTextControlBorderBrush" ResourceKey="SystemColorButtonTextColorBrush" />
            <StaticResource x:Key="CustomThemeSearchBoxTextControlBorderBrushPointerOver" ResourceKey="SystemColorHighlightColorBrush" />
            <StaticResource x:Key="CustomThemeSearchBoxTextControlBorderBrushFocused" ResourceKey="SystemColorHighlightColorBrush" />
            <StaticResource x:Key="CustomThemeSearchBoxTextControlBackground" ResourceKey="SystemColorButtonFaceColorBrush" />
            <StaticResource x:Key="CustomThemeSearchBoxTextControlBackgroundPointerOver" ResourceKey="SystemColorButtonFaceColorBrush" />
            <StaticResource x:Key="CustomThemeSearchBoxTextControlBackgroundFocused" ResourceKey="SystemColorButtonFaceColorBrush" />
            <StaticResource x:Key="CaptionControlFillBrush" ResourceKey="SystemColorButtonFaceColor" />
            <StaticResource x:Key="CaptionControlBorderBrush" ResourceKey="SystemColorButtonTextColor" />
            <StaticResource x:Key="BadgeForegroundBrush" ResourceKey="SystemControlHighlightAltAccentBrush" />
            <StaticResource x:Key="BadgeBackgroundBrush" ResourceKey="SystemControlHighlightAccentBrush" />
            <Thickness x:Key="BackgroundElementMultiWindowRequestingAttentionMargin" Value="0,0,6,0" />
            <Thickness x:Key="BackgroundElementMultiWindowMargin" Value="0,0,4,0" />
            <Thickness x:Key="BackgroundElementMultiWindowLargeIconMargin" Value="0,0,7,0" />
            <StaticResource x:Key="AugmentedEntryPointButtonOverlayBackgroundBrush" ResourceKey="BadgeBackgroundBrush" />
            <StaticResource x:Key="AugmentedEntryPointButtonBadgeBackgroundBrush" ResourceKey="BadgeBackgroundBrush" />
            <StaticResource x:Key="AugmentedEntryPointButtonBadgeBackgroundBrushWarning" ResourceKey="BadgeBackgroundBrush" />
            <StaticResource x:Key="AugmentedEntryPointButtonBadgeBackgroundBrushGood" ResourceKey="BadgeBackgroundBrush" />
            <StaticResource x:Key="AugmentedEntryPointButtonBadgeBackgroundBrushAttention" ResourceKey="BadgeBackgroundBrush" />
            <StaticResource x:Key="AugmentedEntryPointButtonBadgeBackgroundBrushAccent" ResourceKey="BadgeBackgroundBrush" />
            <StaticResource x:Key="AccentFillColorDefaultBrush" ResourceKey="SystemControlHighlightAccentBrush" />
        </ResourceDictionary>
    </ResourceDictionary.ThemeDictionaries>
    <ControlTemplate x:Key="VerticalButtonTemplate" TargetType="RepeatButton">
        <ControlTemplate.Template>
            <Grid x:Name="Root" Background="{ThemeResource ShellTaskbarItemFillColorTransparentBrush}" BorderBrush="#00FFFFFF" BorderThickness="{StaticResource TaskbarButtonOutlineThickness}" CornerRadius="{StaticResource ControlLargeCornerRadius}">
                <Grid.BackgroundTransition>
                    <BrushTransition Duration="0:0:0.083" />
                </Grid.BackgroundTransition>
                <Grid.Children>
                    <FontIcon x:Name="Arrow"
                        RenderTransformOrigin="0.5, 0.5"
                        FontFamily="{ThemeResource SymbolThemeFontFamily}"
                        Foreground="{ThemeResource ScrollBarButtonArrowForeground}"
                        FontSize="{StaticResource ScrollBarButtonArrowIconFontSize}"
                        MirroredWhenRightToLeft="True">
                        <FontIcon.RenderTransform>
                            <ScaleTransform x:Name="ScaleTransform" ScaleY="1" ScaleX="1" />
                        </FontIcon.RenderTransform>
                    </FontIcon>
                </Grid.Children>
                <Grid.VisualStateGroups>
                    <VisualStateGroup x:Name="CommonStates">
                        <VisualStateGroup.States>
                            <VisualState x:Name="Normal" />
                            <VisualState x:Name="PointerOver">
                                <VisualState.Setters>
                                    <Setter Target="Root.Background" Value="{ThemeResource ShellTaskbarItemFillColorAltSecondaryBrush}" />
                                    <Setter Target="Root.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorSecondaryBrush}" />
                                    <Setter Target="Arrow.Foreground" Value="{ThemeResource ScrollBarButtonArrowForegroundPointerOver}" />
                                </VisualState.Setters>
                            </VisualState>
                            <VisualState x:Name="Pressed">
                                <VisualState.Setters>
                                    <Setter Target="Root.Background" Value="{ThemeResource ShellTaskbarItemFillColorTertiaryBrush}" />
                                    <Setter Target="Root.BorderBrush" Value="{ThemeResource ShellTaskbarItemStrokeColorQuinaryBrush}" />
                                    <Setter Target="Arrow.Foreground" Value="{ThemeResource ScrollBarButtonArrowForegroundPressed}" />
                                </VisualState.Setters>
                            </VisualState>
                            <VisualState x:Name="Disabled">
                                <VisualState.Setters>
                                    <Setter Target="Arrow.Foreground" Value="{ThemeResource ScrollBarButtonArrowForegroundDisabled}" />
                                </VisualState.Setters>
                            </VisualState>
                        </VisualStateGroup.States>
                    </VisualStateGroup>
                </Grid.VisualStateGroups>
            </Grid>
        </ControlTemplate.Template>
    </ControlTemplate>
    <Double x:Key="ToolTipMargin" Value="12" />
    <Thickness x:Key="ToggleMenuFlyoutItemPlaceholderThemeThickness" Value="0" />
    <Thickness x:Key="ToggleMenuFlyoutItemDoublePlaceholderThemeThickness" Value="28,0,0,0" />
    <Double x:Key="ThumbnailViewMinWidth" Value="100" />
    <Double x:Key="ThumbnailViewMaxWidth" Value="216" />
    <Double x:Key="ThumbnailScrollableViewMinWidth" Value="216" />
    <Double x:Key="ThumbnailScrollableViewMaxWidth" Value="500" />
    <Double x:Key="ThumbnailIconSize" Value="16" />
    <Double x:Key="ThumbBarToolTipMargin" Value="4" />
    <Double x:Key="ThumbBarIconSize" Value="16" />
    <DataTemplate x:Key="ThumbBarButtonTemplate">
        <DataTemplate.Template>
            <ThumbBarButton x:Name="ThumbBarButton" Style="{StaticResource ThumbBarButtonStyle}">
                <ThumbBarButton.ToolTip>
                    <ToolTip IsEnabled="True" Placement="Top" VerticalOffset="{StaticResource ThumbBarToolTipMargin}" />
                </ThumbBarButton.ToolTip>
                <ThumbBarButton.Content>
                    <ContentPresenter x:Name="BorderElement" CornerRadius="3" BorderThickness="1" BorderBrush="{ThemeResource ThumbBarBorderBrush}">
                        <ContentPresenter.VisualStateGroups>
                            <VisualStateGroup x:Name="CommonStates">
                                <VisualStateGroup.States>
                                    <VisualState x:Name="Normal">
                                        <VisualState.Setters>
                                            <Setter Target="BorderElement.Background" Value="{ThemeResource ThumbBarBackgroundBrush}" />
                                        </VisualState.Setters>
                                    </VisualState>
                                    <VisualState x:Name="NoBackgroundState">
                                        <VisualState.Setters>
                                            <Setter Target="BorderElement.Background" Value="Transparent" />
                                            <Setter Target="BorderElement.BorderBrush" Value="Transparent" />
                                        </VisualState.Setters>
                                    </VisualState>
                                    <VisualState x:Name="PointerOver">
                                        <VisualState.Setters>
                                            <Setter Target="BorderElement.Background" Value="{ThemeResource ThumbBarPointerOverBackgroundBrush}" />
                                        </VisualState.Setters>
                                    </VisualState>
                                    <VisualState x:Name="Pressed">
                                        <VisualState.Setters>
                                            <Setter Target="BorderElement.Background" Value="{ThemeResource ThumbBarPressedBackgroundBrush}" />
                                        </VisualState.Setters>
                                    </VisualState>
                                    <VisualState x:Name="Disabled">
                                        <VisualState.Setters>
                                            <Setter Target="BorderElement.Background" Value="{ThemeResource ThumbBarDisabledBackgroundBrush}" />
                                            <Setter Target="BorderElement.BorderBrush" Value="{ThemeResource ThumbBarDisabledBorderBrush}" />
                                        </VisualState.Setters>
                                    </VisualState>
                                </VisualStateGroup.States>
                            </VisualStateGroup>
                        </ContentPresenter.VisualStateGroups>
                        <ContentPresenter.Content>
                            <Image Width="{StaticResource ThumbBarIconSize}" Height="{StaticResource ThumbBarIconSize}" />
                        </ContentPresenter.Content>
                    </ContentPresenter>
                </ThumbBarButton.Content>
            </ThumbBarButton>
        </DataTemplate.Template>
    </DataTemplate>
    <Style x:Key="ThumbBarButtonStyle" TargetType="ThumbBarButton">
        <Setter Property="HorizontalContentAlignment" Value="Center" />
        <Setter Property="VerticalContentAlignment" Value="Center" />
        <Setter Property="Width" Value="24" />
        <Setter Property="Height" Value="24" />
        <Setter Property="Padding" Value="0" />
        <Setter Property="UseSystemFocusVisuals" Value="True" />
        <Setter Property="FocusVisualMargin" Value="-2" />
        <Setter Property="FocusVisualSecondaryThickness" Value="0" />
        <Setter Property="IsTabStop" Value="True" />
    </Style>
    <Double x:Key="ThumbBarButtonSize" Value="16" />
    <Style x:Key="TaskbarProgressIndicatorStyle" TargetType="ProgressBar">
        <Setter Property="Foreground" Value="{ThemeResource ProgressBarForeground}" />
        <Setter Property="Background" Value="{ThemeResource ProgressBarBackground}" />
        <Setter Property="BorderThickness" Value="{ThemeResource ProgressBarBorderThemeThickness}" />
        <Setter Property="BorderBrush" Value="{ThemeResource ProgressBarBorderBrush}" />
        <Setter Property="MinHeight" Value="{StaticResource ProgressBarMinHeight}" />
        <Setter Property="Maximum" Value="100" />
        <Setter Property="IsTabStop" Value="False" />
        <Setter Property="VerticalAlignment" Value="Center" />
        <Setter Property="CornerRadius" Value="{StaticResource ProgressBarCornerRadius}" />
        <Setter Property="Template">
            <Setter.Value>
                <ControlTemplate TargetType="ProgressBar">
                    <ControlTemplate.Template>
                        <Grid x:Name="LayoutRoot">
                            <Grid.VisualStateGroups>
                                <VisualStateGroup x:Name="CommonStates">
                                    <VisualStateGroup.Transitions>
                                        <VisualTransition From="Updating" To="Determinate">
                                            <VisualTransition.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <RepositionThemeAnimation TargetName="DeterminateProgressBarIndicator">
                                                            <RepositionThemeAnimation.FromHorizontalOffset>
                                                                <Binding Path="TemplateSettings.IndicatorLengthDelta">
                                                                    <Binding.RelativeSource>
                                                                        <RelativeSource Mode="TemplatedParent" />
                                                                    </Binding.RelativeSource>
                                                                </Binding>
                                                            </RepositionThemeAnimation.FromHorizontalOffset>
                                                        </RepositionThemeAnimation>
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualTransition.Storyboard>
                                        </VisualTransition>
                                        <VisualTransition From="UpdatingError" To="Error">
                                            <VisualTransition.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <RepositionThemeAnimation TargetName="DeterminateProgressBarIndicator">
                                                            <RepositionThemeAnimation.FromHorizontalOffset>
                                                                <Binding Path="TemplateSettings.IndicatorLengthDelta">
                                                                    <Binding.RelativeSource>
                                                                        <RelativeSource Mode="TemplatedParent" />
                                                                    </Binding.RelativeSource>
                                                                </Binding>
                                                            </RepositionThemeAnimation.FromHorizontalOffset>
                                                        </RepositionThemeAnimation>
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualTransition.Storyboard>
                                        </VisualTransition>
                                        <VisualTransition From="Paused" To="Determinate">
                                            <VisualTransition.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ColorAnimation TargetName="DeterminateProgressBarIndicator" TargetProperty="(Shape.Fill).(SolidColorBrush.Color)" To="{TemplateBinding Foreground}" Duration="0:0:0.167" />
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualTransition.Storyboard>
                                        </VisualTransition>
                                        <VisualTransition From="Error" To="Determinate">
                                            <VisualTransition.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ColorAnimation TargetName="DeterminateProgressBarIndicator" TargetProperty="(Shape.Fill).(SolidColorBrush.Color)" To="{TemplateBinding Foreground}" Duration="0:0:0.167" />
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualTransition.Storyboard>
                                        </VisualTransition>
                                        <VisualTransition From="Indeterminate" To="Determinate">
                                            <VisualTransition.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <FadeInThemeAnimation TargetName="IndeterminateProgressBarIndicator" />
                                                        <FadeInThemeAnimation TargetName="IndeterminateProgressBarIndicator2" />
                                                        <FadeInThemeAnimation TargetName="ProgressBarTrack" />
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualTransition.Storyboard>
                                        </VisualTransition>
                                        <VisualTransition From="IndeterminatePaused" To="Indeterminate">
                                            <VisualTransition.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <DoubleAnimationUsingKeyFrames TargetProperty="Opacity" TargetName="IndeterminateProgressBarIndicator">
                                                            <DoubleAnimationUsingKeyFrames.KeyFrames>
                                                                <DiscreteDoubleKeyFrame KeyTime="0" Value="0" />
                                                            </DoubleAnimationUsingKeyFrames.KeyFrames>
                                                        </DoubleAnimationUsingKeyFrames>
                                                        <DoubleAnimationUsingKeyFrames TargetName="IndeterminateProgressBarIndicator2" TargetProperty="(UIElement.RenderTransform).(CompositeTransform.TranslateX)">
                                                            <DoubleAnimationUsingKeyFrames.KeyFrames>
                                                                <SplineDoubleKeyFrame KeyTime="0.333" KeySpline="1.0, 0.0, 1.0, 1.0">
                                                                    <SplineDoubleKeyFrame.Value>
                                                                        <Binding Path="TemplateSettings.ContainerAnimationEndPosition">
                                                                            <Binding.RelativeSource>
                                                                                <RelativeSource Mode="TemplatedParent" />
                                                                            </Binding.RelativeSource>
                                                                        </Binding>
                                                                    </SplineDoubleKeyFrame.Value>
                                                                </SplineDoubleKeyFrame>
                                                            </DoubleAnimationUsingKeyFrames.KeyFrames>
                                                        </DoubleAnimationUsingKeyFrames>
                                                        <DoubleAnimationUsingKeyFrames TargetName="ProgressBarTrack" TargetProperty="(UIElement.RenderTransform).(CompositeTransform.TranslateX)">
                                                            <DoubleAnimationUsingKeyFrames.KeyFrames>
                                                                <SplineDoubleKeyFrame KeyTime="0" Value="0" KeySpline="1.0, 0.0, 1.0, 1.0" />
                                                                <SplineDoubleKeyFrame KeyTime="0.5" KeySpline="1.0, 0.0, 1.0, 1.0">
                                                                    <SplineDoubleKeyFrame.Value>
                                                                        <Binding Path="TemplateSettings.ContainerAnimationEndPosition">
                                                                            <Binding.RelativeSource>
                                                                                <RelativeSource Mode="TemplatedParent" />
                                                                            </Binding.RelativeSource>
                                                                        </Binding>
                                                                    </SplineDoubleKeyFrame.Value>
                                                                </SplineDoubleKeyFrame>
                                                            </DoubleAnimationUsingKeyFrames.KeyFrames>
                                                        </DoubleAnimationUsingKeyFrames>
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualTransition.Storyboard>
                                        </VisualTransition>
                                        <VisualTransition From="IndeterminateError" To="Indeterminate">
                                            <VisualTransition.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <DoubleAnimationUsingKeyFrames TargetProperty="Opacity" TargetName="IndeterminateProgressBarIndicator">
                                                            <DoubleAnimationUsingKeyFrames.KeyFrames>
                                                                <DiscreteDoubleKeyFrame KeyTime="0" Value="0" />
                                                            </DoubleAnimationUsingKeyFrames.KeyFrames>
                                                        </DoubleAnimationUsingKeyFrames>
                                                        <DoubleAnimationUsingKeyFrames TargetName="IndeterminateProgressBarIndicator2" TargetProperty="(UIElement.RenderTransform).(CompositeTransform.TranslateX)">
                                                            <DoubleAnimationUsingKeyFrames.KeyFrames>
                                                                <SplineDoubleKeyFrame KeyTime="0.333" KeySpline="1.0, 0.0, 1.0, 1.0">
                                                                    <SplineDoubleKeyFrame.Value>
                                                                        <Binding Path="TemplateSettings.ContainerAnimationEndPosition">
                                                                            <Binding.RelativeSource>
                                                                                <RelativeSource Mode="TemplatedParent" />
                                                                            </Binding.RelativeSource>
                                                                        </Binding>
                                                                    </SplineDoubleKeyFrame.Value>
                                                                </SplineDoubleKeyFrame>
                                                            </DoubleAnimationUsingKeyFrames.KeyFrames>
                                                        </DoubleAnimationUsingKeyFrames>
                                                        <DoubleAnimationUsingKeyFrames TargetName="ProgressBarTrack" TargetProperty="(UIElement.RenderTransform).(CompositeTransform.TranslateX)">
                                                            <DoubleAnimationUsingKeyFrames.KeyFrames>
                                                                <SplineDoubleKeyFrame KeyTime="0" Value="0" KeySpline="1.0, 0.0, 1.0, 1.0" />
                                                                <SplineDoubleKeyFrame KeyTime="0.5" KeySpline="1.0, 0.0, 1.0, 1.0">
                                                                    <SplineDoubleKeyFrame.Value>
                                                                        <Binding Path="TemplateSettings.ContainerAnimationEndPosition">
                                                                            <Binding.RelativeSource>
                                                                                <RelativeSource Mode="TemplatedParent" />
                                                                            </Binding.RelativeSource>
                                                                        </Binding>
                                                                    </SplineDoubleKeyFrame.Value>
                                                                </SplineDoubleKeyFrame>
                                                            </DoubleAnimationUsingKeyFrames.KeyFrames>
                                                        </DoubleAnimationUsingKeyFrames>
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualTransition.Storyboard>
                                        </VisualTransition>
                                    </VisualStateGroup.Transitions>
                                    <VisualStateGroup.States>
                                        <VisualState x:Name="Normal" />
                                        <VisualState x:Name="Determinate" />
                                        <VisualState x:Name="Updating" />
                                        <VisualState x:Name="UpdatingError">
                                            <VisualState.Setters>
                                                <Setter Target="DeterminateProgressBarIndicator.(Shape.Fill).(SolidColorBrush.Color)" Value="{ThemeResource ProgressBarErrorForegroundColor}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="Error">
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ColorAnimation TargetName="DeterminateProgressBarIndicator" TargetProperty="(Shape.Fill).(SolidColorBrush.Color)" To="{ThemeResource ProgressBarErrorForegroundColor}" Duration="0:0:0.167" />
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                        </VisualState>
                                        <VisualState x:Name="Paused">
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ColorAnimation TargetName="DeterminateProgressBarIndicator" TargetProperty="(Shape.Fill).(SolidColorBrush.Color)" To="{ThemeResource ProgressBarPausedForegroundColor}" Duration="0:0:0.167" />
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                        </VisualState>
                                        <VisualState x:Name="Indeterminate">
                                            <VisualState.Setters>
                                                <Setter Target="IndeterminateProgressBarIndicator.Opacity" Value="1" />
                                                <Setter Target="IndeterminateProgressBarIndicator2.Opacity" Value="1" />
                                                <Setter Target="ProgressBarTrack.Opacity" Value="0" />
                                            </VisualState.Setters>
                                            <VisualState.Storyboard>
                                                <Storyboard RepeatBehavior="Forever">
                                                    <Storyboard.Children>
                                                        <DoubleAnimationUsingKeyFrames TargetName="IndeterminateProgressBarIndicator" TargetProperty="(UIElement.RenderTransform).(CompositeTransform.TranslateX)">
                                                            <DoubleAnimationUsingKeyFrames.KeyFrames>
                                                                <DiscreteDoubleKeyFrame KeyTime="0">
                                                                    <DiscreteDoubleKeyFrame.Value>
                                                                        <Binding Path="TemplateSettings.ContainerAnimationStartPosition">
                                                                            <Binding.RelativeSource>
                                                                                <RelativeSource Mode="TemplatedParent" />
                                                                            </Binding.RelativeSource>
                                                                        </Binding>
                                                                    </DiscreteDoubleKeyFrame.Value>
                                                                </DiscreteDoubleKeyFrame>
                                                                <SplineDoubleKeyFrame KeyTime="1.5" KeySpline="0.4, 0.0, 0.6, 1.0">
                                                                    <SplineDoubleKeyFrame.Value>
                                                                        <Binding Path="TemplateSettings.ContainerAnimationEndPosition">
                                                                            <Binding.RelativeSource>
                                                                                <RelativeSource Mode="TemplatedParent" />
                                                                            </Binding.RelativeSource>
                                                                        </Binding>
                                                                    </SplineDoubleKeyFrame.Value>
                                                                </SplineDoubleKeyFrame>
                                                                <DiscreteDoubleKeyFrame KeyTime="2">
                                                                    <DiscreteDoubleKeyFrame.Value>
                                                                        <Binding Path="TemplateSettings.ContainerAnimationEndPosition">
                                                                            <Binding.RelativeSource>
                                                                                <RelativeSource Mode="TemplatedParent" />
                                                                            </Binding.RelativeSource>
                                                                        </Binding>
                                                                    </DiscreteDoubleKeyFrame.Value>
                                                                </DiscreteDoubleKeyFrame>
                                                            </DoubleAnimationUsingKeyFrames.KeyFrames>
                                                        </DoubleAnimationUsingKeyFrames>
                                                        <DoubleAnimationUsingKeyFrames TargetName="IndeterminateProgressBarIndicator2" TargetProperty="(UIElement.RenderTransform).(CompositeTransform.TranslateX)">
                                                            <DoubleAnimationUsingKeyFrames.KeyFrames>
                                                                <DiscreteDoubleKeyFrame KeyTime="0">
                                                                    <DiscreteDoubleKeyFrame.Value>
                                                                        <Binding Path="TemplateSettings.Container2AnimationStartPosition">
                                                                            <Binding.RelativeSource>
                                                                                <RelativeSource Mode="TemplatedParent" />
                                                                            </Binding.RelativeSource>
                                                                        </Binding>
                                                                    </DiscreteDoubleKeyFrame.Value>
                                                                </DiscreteDoubleKeyFrame>
                                                                <DiscreteDoubleKeyFrame KeyTime="0.75">
                                                                    <DiscreteDoubleKeyFrame.Value>
                                                                        <Binding Path="TemplateSettings.Container2AnimationStartPosition">
                                                                            <Binding.RelativeSource>
                                                                                <RelativeSource Mode="TemplatedParent" />
                                                                            </Binding.RelativeSource>
                                                                        </Binding>
                                                                    </DiscreteDoubleKeyFrame.Value>
                                                                </DiscreteDoubleKeyFrame>
                                                                <SplineDoubleKeyFrame KeyTime="2" KeySpline="0.4, 0.0, 0.6, 1.0">
                                                                    <SplineDoubleKeyFrame.Value>
                                                                        <Binding Path="TemplateSettings.Container2AnimationEndPosition">
                                                                            <Binding.RelativeSource>
                                                                                <RelativeSource Mode="TemplatedParent" />
                                                                            </Binding.RelativeSource>
                                                                        </Binding>
                                                                    </SplineDoubleKeyFrame.Value>
                                                                </SplineDoubleKeyFrame>
                                                            </DoubleAnimationUsingKeyFrames.KeyFrames>
                                                        </DoubleAnimationUsingKeyFrames>
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                        </VisualState>
                                        <VisualState x:Name="IndeterminateError">
                                            <VisualState.Setters>
                                                <Setter Target="IndeterminateProgressBarIndicator.Opacity" Value="0" />
                                                <Setter Target="DeterminateProgressBarIndicator.Opacity" Value="0" />
                                                <Setter Target="IndeterminateProgressBarIndicator2.Opacity" Value="1" />
                                                <Setter Target="ProgressBarTrack.Opacity" Value="0" />
                                            </VisualState.Setters>
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <DoubleAnimationUsingKeyFrames TargetName="IndeterminateProgressBarIndicator2" TargetProperty="(UIElement.RenderTransform).(CompositeTransform.TranslateX)">
                                                            <DoubleAnimationUsingKeyFrames.KeyFrames>
                                                                <SplineDoubleKeyFrame KeyTime="0.167" KeySpline="1.0, 1.0, 0.0, 1.0">
                                                                    <SplineDoubleKeyFrame.Value>
                                                                        <Binding Path="TemplateSettings.Container2AnimationEndPosition">
                                                                            <Binding.RelativeSource>
                                                                                <RelativeSource Mode="TemplatedParent" />
                                                                            </Binding.RelativeSource>
                                                                        </Binding>
                                                                    </SplineDoubleKeyFrame.Value>
                                                                </SplineDoubleKeyFrame>
                                                                <SplineDoubleKeyFrame KeyTime="0.167" KeySpline="1.0, 1.0, 0.0, 1.0">
                                                                    <SplineDoubleKeyFrame.Value>
                                                                        <Binding Path="TemplateSettings.Container2AnimationStartPosition">
                                                                            <Binding.RelativeSource>
                                                                                <RelativeSource Mode="TemplatedParent" />
                                                                            </Binding.RelativeSource>
                                                                        </Binding>
                                                                    </SplineDoubleKeyFrame.Value>
                                                                </SplineDoubleKeyFrame>
                                                                <SplineDoubleKeyFrame KeyTime="0.75" KeySpline="0.0, 0.0, 0.0, 1.0">
                                                                    <SplineDoubleKeyFrame.Value>
                                                                        <Binding Path="TemplateSettings.ContainerAnimationMidPosition">
                                                                            <Binding.RelativeSource>
                                                                                <RelativeSource Mode="TemplatedParent" />
                                                                            </Binding.RelativeSource>
                                                                        </Binding>
                                                                    </SplineDoubleKeyFrame.Value>
                                                                </SplineDoubleKeyFrame>
                                                            </DoubleAnimationUsingKeyFrames.KeyFrames>
                                                        </DoubleAnimationUsingKeyFrames>
                                                        <ColorAnimation TargetName="IndeterminateProgressBarIndicator2" TargetProperty="(Shape.Fill).(SolidColorBrush.Color)" To="{ThemeResource ProgressBarErrorForegroundColor}" Duration="0:0:0.167" />
                                                        <DoubleAnimationUsingKeyFrames TargetName="ProgressBarTrack" TargetProperty="(UIElement.RenderTransform).(CompositeTransform.TranslateX)">
                                                            <DoubleAnimationUsingKeyFrames.KeyFrames>
                                                                <SplineDoubleKeyFrame KeyTime="0" KeySpline="0.0, 0.0, 0.0, 1.0">
                                                                    <SplineDoubleKeyFrame.Value>
                                                                        <Binding Path="TemplateSettings.Container2AnimationStartPosition">
                                                                            <Binding.RelativeSource>
                                                                                <RelativeSource Mode="TemplatedParent" />
                                                                            </Binding.RelativeSource>
                                                                        </Binding>
                                                                    </SplineDoubleKeyFrame.Value>
                                                                </SplineDoubleKeyFrame>
                                                                <SplineDoubleKeyFrame KeyTime="0.75" Value="0" KeySpline="0.0, 0.0, 0.0, 1.0" />
                                                            </DoubleAnimationUsingKeyFrames.KeyFrames>
                                                        </DoubleAnimationUsingKeyFrames>
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                        </VisualState>
                                        <VisualState x:Name="IndeterminatePaused">
                                            <VisualState.Setters>
                                                <Setter Target="IndeterminateProgressBarIndicator.Opacity" Value="0" />
                                                <Setter Target="DeterminateProgressBarIndicator.Opacity" Value="0" />
                                                <Setter Target="IndeterminateProgressBarIndicator2.Opacity" Value="1" />
                                                <Setter Target="ProgressBarTrack.Opacity" Value="0" />
                                            </VisualState.Setters>
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <DoubleAnimationUsingKeyFrames TargetName="IndeterminateProgressBarIndicator2" TargetProperty="(UIElement.RenderTransform).(CompositeTransform.TranslateX)">
                                                            <DoubleAnimationUsingKeyFrames.KeyFrames>
                                                                <SplineDoubleKeyFrame KeyTime="0.167" KeySpline="1.0, 1.0, 0.0, 1.0">
                                                                    <SplineDoubleKeyFrame.Value>
                                                                        <Binding Path="TemplateSettings.Container2AnimationEndPosition">
                                                                            <Binding.RelativeSource>
                                                                                <RelativeSource Mode="TemplatedParent" />
                                                                            </Binding.RelativeSource>
                                                                        </Binding>
                                                                    </SplineDoubleKeyFrame.Value>
                                                                </SplineDoubleKeyFrame>
                                                                <SplineDoubleKeyFrame KeyTime="0.167" KeySpline="1.0, 1.0, 0.0, 1.0">
                                                                    <SplineDoubleKeyFrame.Value>
                                                                        <Binding Path="TemplateSettings.Container2AnimationStartPosition">
                                                                            <Binding.RelativeSource>
                                                                                <RelativeSource Mode="TemplatedParent" />
                                                                            </Binding.RelativeSource>
                                                                        </Binding>
                                                                    </SplineDoubleKeyFrame.Value>
                                                                </SplineDoubleKeyFrame>
                                                                <SplineDoubleKeyFrame KeyTime="0.75" KeySpline="0.0, 0.0, 0.0, 1.0">
                                                                    <SplineDoubleKeyFrame.Value>
                                                                        <Binding Path="TemplateSettings.ContainerAnimationMidPosition">
                                                                            <Binding.RelativeSource>
                                                                                <RelativeSource Mode="TemplatedParent" />
                                                                            </Binding.RelativeSource>
                                                                        </Binding>
                                                                    </SplineDoubleKeyFrame.Value>
                                                                </SplineDoubleKeyFrame>
                                                            </DoubleAnimationUsingKeyFrames.KeyFrames>
                                                        </DoubleAnimationUsingKeyFrames>
                                                        <ColorAnimation TargetName="IndeterminateProgressBarIndicator2" TargetProperty="(Shape.Fill).(SolidColorBrush.Color)" To="{ThemeResource ProgressBarPausedForegroundColor}" Duration="0:0:0.167" />
                                                        <DoubleAnimationUsingKeyFrames TargetName="ProgressBarTrack" TargetProperty="(UIElement.RenderTransform).(CompositeTransform.TranslateX)">
                                                            <DoubleAnimationUsingKeyFrames.KeyFrames>
                                                                <SplineDoubleKeyFrame KeyTime="0" KeySpline="0.0, 0.0, 0.0, 1.0">
                                                                    <SplineDoubleKeyFrame.Value>
                                                                        <Binding Path="TemplateSettings.Container2AnimationStartPosition">
                                                                            <Binding.RelativeSource>
                                                                                <RelativeSource Mode="TemplatedParent" />
                                                                            </Binding.RelativeSource>
                                                                        </Binding>
                                                                    </SplineDoubleKeyFrame.Value>
                                                                </SplineDoubleKeyFrame>
                                                                <SplineDoubleKeyFrame KeyTime="0.75" Value="0" KeySpline="0.0, 0.0, 0.0, 1.0" />
                                                            </DoubleAnimationUsingKeyFrames.KeyFrames>
                                                        </DoubleAnimationUsingKeyFrames>
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                        </VisualState>
                                    </VisualStateGroup.States>
                                </VisualStateGroup>
                            </Grid.VisualStateGroups>
                            <Grid.Children>
                                <Border x:Name="ProgressBarRoot" BorderBrush="{TemplateBinding BorderBrush}" BorderThickness="{TemplateBinding BorderThickness}" Padding="{TemplateBinding Padding}" CornerRadius="{TemplateBinding CornerRadius}">
                                    <Border.Child>
                                        <Border>
                                            <Border.Clip>
                                                <Binding Path="TemplateSettings.ClipRect">
                                                    <Binding.RelativeSource>
                                                        <RelativeSource Mode="TemplatedParent" />
                                                    </Binding.RelativeSource>
                                                </Binding>
                                            </Border.Clip>
                                            <Border.Child>
                                                <Grid Height="{TemplateBinding MinHeight}">
                                                    <Grid.Children>
                                                        <Rectangle x:Name="ProgressBarTrack"
                                                            Margin="0,1"
                                                            Fill="{TemplateBinding Background}"
                                                            Width="{TemplateBinding Width}"
                                                            VerticalAlignment="Stretch"
                                                            RadiusX="1"
                                                            RadiusY="1">
                                                            <Rectangle.RenderTransform>
                                                                <CompositeTransform />
                                                            </Rectangle.RenderTransform>
                                                        </Rectangle>
                                                        <Rectangle x:Name="DeterminateProgressBarIndicator" Fill="{TemplateBinding Foreground}" HorizontalAlignment="Left">
                                                            <Rectangle.RadiusX>
                                                                <Binding Path="CornerRadius" Converter="{StaticResource TopLeftCornerRadiusDoubleValueConverter}">
                                                                    <Binding.RelativeSource>
                                                                        <RelativeSource Mode="TemplatedParent" />
                                                                    </Binding.RelativeSource>
                                                                </Binding>
                                                            </Rectangle.RadiusX>
                                                            <Rectangle.RadiusY>
                                                                <Binding Path="CornerRadius" Converter="{StaticResource BottomRightCornerRadiusDoubleValueConverter}">
                                                                    <Binding.RelativeSource>
                                                                        <RelativeSource Mode="TemplatedParent" />
                                                                    </Binding.RelativeSource>
                                                                </Binding>
                                                            </Rectangle.RadiusY>
                                                        </Rectangle>
                                                        <Rectangle x:Name="IndeterminateProgressBarIndicator" Fill="{TemplateBinding Foreground}" HorizontalAlignment="Left" Opacity="0">
                                                            <Rectangle.RadiusX>
                                                                <Binding Path="CornerRadius" Converter="{StaticResource TopLeftCornerRadiusDoubleValueConverter}">
                                                                    <Binding.RelativeSource>
                                                                        <RelativeSource Mode="TemplatedParent" />
                                                                    </Binding.RelativeSource>
                                                                </Binding>
                                                            </Rectangle.RadiusX>
                                                            <Rectangle.RadiusY>
                                                                <Binding Path="CornerRadius" Converter="{StaticResource BottomRightCornerRadiusDoubleValueConverter}">
                                                                    <Binding.RelativeSource>
                                                                        <RelativeSource Mode="TemplatedParent" />
                                                                    </Binding.RelativeSource>
                                                                </Binding>
                                                            </Rectangle.RadiusY>
                                                            <Rectangle.RenderTransform>
                                                                <CompositeTransform />
                                                            </Rectangle.RenderTransform>
                                                        </Rectangle>
                                                        <Rectangle x:Name="IndeterminateProgressBarIndicator2" Fill="{TemplateBinding Foreground}" HorizontalAlignment="Left" Opacity="0">
                                                            <Rectangle.RadiusX>
                                                                <Binding Path="CornerRadius" Converter="{StaticResource TopLeftCornerRadiusDoubleValueConverter}">
                                                                    <Binding.RelativeSource>
                                                                        <RelativeSource Mode="TemplatedParent" />
                                                                    </Binding.RelativeSource>
                                                                </Binding>
                                                            </Rectangle.RadiusX>
                                                            <Rectangle.RadiusY>
                                                                <Binding Path="CornerRadius" Converter="{StaticResource BottomRightCornerRadiusDoubleValueConverter}">
                                                                    <Binding.RelativeSource>
                                                                        <RelativeSource Mode="TemplatedParent" />
                                                                    </Binding.RelativeSource>
                                                                </Binding>
                                                            </Rectangle.RadiusY>
                                                            <Rectangle.RenderTransform>
                                                                <CompositeTransform />
                                                            </Rectangle.RenderTransform>
                                                        </Rectangle>
                                                    </Grid.Children>
                                                </Grid>
                                            </Border.Child>
                                        </Border>
                                    </Border.Child>
                                </Border>
                            </Grid.Children>
                        </Grid>
                    </ControlTemplate.Template>
                </ControlTemplate>
            </Setter.Value>
        </Setter>
    </Style>
    <DataTemplate x:Key="TaskbarExtensionTemplate">
        <DataTemplate.Template>
            <ContentControl VerticalContentAlignment="Stretch" HorizontalContentAlignment="Stretch" />
        </DataTemplate.Template>
    </DataTemplate>
    <Thickness x:Key="TaskbarContextMenuMargin" Value="12" />
    <Thickness x:Key="TaskbarButtonTabletPostureHitTestableMargin" Value="6,4" />
    <Thickness x:Key="TaskbarButtonShyTabletPostureHitTestableMargin" Value="3,6" />
    <Thickness x:Key="TaskbarButtonShadowThickness" Value="0,0,0,1" />
    <Thickness x:Key="TaskbarButtonOutlineThickness" Value="1" />
    <Double x:Key="TaskbarButtonOutlineThicknessValue" Value="1" />
    <Thickness x:Key="TaskbarButtonMargin" Value="0" />
    <Thickness x:Key="TaskbarButtonHitTestableMargin" Value="2,4" />
    <DataTemplate x:Key="TaskListLabeledItemTemplate">
        <DataTemplate.Template>
            <TaskListButton x:Name="TaskListButton" Style="{StaticResource TaskListLabeledButton}">
                <TaskListButton.ToolTip>
                    <ToolTip VerticalOffset="{StaticResource ToolTipMargin}" Placement="Top" />
                </TaskListButton.ToolTip>
            </TaskListButton>
        </DataTemplate.Template>
    </DataTemplate>
    <DataTemplate x:Key="TaskListLabeledItemTemplateWithDownloadIcon">
        <DataTemplate.Template>
            <TaskListButton x:Name="TaskListButton" Style="{StaticResource TaskListLabeledButton}">
                <TaskListButton.ToolTip>
                    <ToolTip VerticalOffset="{StaticResource ToolTipMargin}" Placement="Top" />
                </TaskListButton.ToolTip>
            </TaskListButton>
        </DataTemplate.Template>
    </DataTemplate>
    <DataTemplate x:Key="TaskListLabeledItemTemplateRecommendation">
        <DataTemplate.Template>
            <TaskListButton x:Name="TaskListButton" IsDraggable="False" Style="{StaticResource TaskListLabeledButtonRecommendation}">
                <TaskListButton.ToolTip>
                    <ToolTip VerticalOffset="{StaticResource ToolTipMargin}" Placement="Top" />
                </TaskListButton.ToolTip>
            </TaskListButton>
        </DataTemplate.Template>
    </DataTemplate>
    <DataTemplate x:Key="TaskListLabeledItemOverflowTemplate">
        <DataTemplate.Template>
            <TaskListButton x:Name="TaskListButton" HasLabel="False" Style="{StaticResource TaskListLabeledButton}">
                <TaskListButton.ToolTip>
                    <ToolTip VerticalOffset="{StaticResource ToolTipMargin}" Placement="Top" />
                </TaskListButton.ToolTip>
            </TaskListButton>
        </DataTemplate.Template>
    </DataTemplate>
    <DataTemplate x:Key="TaskListLabeledItemOverflowTemplateWithDownloadIcon">
        <DataTemplate.Template>
            <TaskListButton x:Name="TaskListButton" HasLabel="False" Style="{StaticResource TaskListLabeledButton}">
                <TaskListButton.ToolTip>
                    <ToolTip VerticalOffset="{StaticResource ToolTipMargin}" Placement="Top" />
                </TaskListButton.ToolTip>
            </TaskListButton>
        </DataTemplate.Template>
    </DataTemplate>
    <Style x:Key="TaskListLabeledButton" TargetType="TaskListButton">
        <Setter Property="HorizontalAlignment" Value="Stretch" />
        <Setter Property="VerticalAlignment" Value="Stretch" />
        <Setter Property="HorizontalContentAlignment" Value="Stretch" />
        <Setter Property="VerticalContentAlignment" Value="Stretch" />
        <Setter Property="UseSystemFocusVisuals" Value="True" />
        <Setter Property="FocusVisualMargin" Value="-2" />
        <Setter Property="FocusVisualSecondaryThickness" Value="0" />
        <Setter Property="CornerRadius" Value="{StaticResource ControlLargeCornerRadius}" />
        <Setter Property="IsTabStop" Value="True" />
        <Setter Property="Template">
            <Setter.Value>
                <ControlTemplate TargetType="TaskListButton">
                    <ControlTemplate.Template>
                        <TaskListLabeledButtonPanel x:Name="IconPanel" Background="#00FFFFFF">
                            <TaskListLabeledButtonPanel.ColumnDefinitions>
                                <ColumnDefinition x:Name="IconColumnDefinition" />
                                <ColumnDefinition Width="Auto" />
                            </TaskListLabeledButtonPanel.ColumnDefinitions>
                            <TaskListLabeledButtonPanel.Children>
                                <DeferredElement>
                                    <Rectangle x:Name="MostRecentlyUsedDivider"
                                        Width="1"
                                        HorizontalAlignment="Left"
                                        VerticalAlignment="Center"
                                        Fill="{ThemeResource MostRecentlyUsedDividerBrush}"
                                        IsHitTestVisible="False"
                                        ColumnSpan="2" />
                                </DeferredElement>
                                <Border CornerRadius="{TemplateBinding CornerRadius}" IsTemplateFocusTarget="True" IsHitTestVisible="False" ColumnSpan="2" />
                                <DeferredElement>
                                    <Border x:Name="SharedWindowElement"
                                        BorderBrush="#00FFFFFF"
                                        BorderThickness="{StaticResource TaskbarButtonOutlineThickness}"
                                        CornerRadius="{TemplateBinding CornerRadius}"
                                        IsHitTestVisible="False"
                                        ColumnSpan="2">
                                        <Border.BackgroundTransition>
                                            <BrushTransition Duration="0:0:0.083" />
                                        </Border.BackgroundTransition>
                                    </Border>
                                </DeferredElement>
                                <Border x:Name="BackgroundElement"
                                    Background="{ThemeResource ShellTaskbarItemFillColorTransparentBrush}"
                                    BorderBrush="#00FFFFFF"
                                    BorderThickness="{StaticResource TaskbarButtonOutlineThickness}"
                                    CornerRadius="{TemplateBinding CornerRadius}"
                                    Margin="0"
                                    IsHitTestVisible="False"
                                    ColumnSpan="2">
                                    <Border.BackgroundTransition>
                                        <BrushTransition Duration="0:0:0.083" />
                                    </Border.BackgroundTransition>
                                </Border>
                                <DeferredElement>
                                    <Border x:Name="MultiWindowElement"
                                        BorderThickness="{StaticResource TaskbarButtonOutlineThickness}"
                                        CornerRadius="{StaticResource ControlLargeCornerRadius}"
                                        HorizontalAlignment="Right"
                                        IsHitTestVisible="False"
                                        ColumnSpan="2">
                                        <Border.BackgroundTransition>
                                            <BrushTransition Duration="0:0:0.083" />
                                        </Border.BackgroundTransition>
                                    </Border>
                                </DeferredElement>
                                <DeferredElement>
                                    <Rectangle x:Name="RunningIndicator"
                                        Height="3"
                                        VerticalAlignment="Bottom"
                                        Margin="{StaticResource TaskListButtonIndicatorMargin}"
                                        Fill="{ThemeResource RunningIndicatorBrush}"
                                        IsHitTestVisible="False"
                                        RadiusX="1.5"
                                        RadiusY="1.5" />
                                </DeferredElement>
                                <DeferredElement>
                                    <ProgressBar x:Name="ProgressIndicator"
                                        Maximum="1"
                                        VerticalAlignment="Bottom"
                                        Margin="{StaticResource TaskListButtonIndicatorMargin}"
                                        IsHitTestVisible="False"
                                        Style="{StaticResource TaskbarProgressIndicatorStyle}" />
                                </DeferredElement>
                                <Image x:Name="Icon" AccessibilityView="Raw" IsHitTestVisible="False" Stretch="Uniform" />
                                <DeferredElement>
                                    <Rectangle x:Name="DefaultIcon"
                                        Fill="{ThemeResource RunningIndicatorBrush}"
                                        RadiusX="5"
                                        RadiusY="5"
                                        HorizontalAlignment="Center"
                                        VerticalAlignment="Center"
                                        AccessibilityView="Raw"
                                        IsHitTestVisible="False"
                                        Stretch="Uniform" />
                                </DeferredElement>
                                <DeferredElement>
                                    <Image x:Name="OverlayIcon"
                                        HorizontalAlignment="Center"
                                        VerticalAlignment="Center"
                                        AccessibilityView="Raw"
                                        IsHitTestVisible="False"
                                        Stretch="Uniform" />
                                </DeferredElement>
                                <DeferredElement>
                                    <Badge x:Name="BadgeControl" VerticalAlignment="Center" HorizontalAlignment="Center" AccessibilityView="Raw" IsHitTestVisible="False" />
                                </DeferredElement>
                                <DeferredElement>
                                    <Beacon x:Name="BeaconControl" VerticalAlignment="Center" HorizontalAlignment="Center" AccessibilityView="Raw" IsHitTestVisible="False" />
                                </DeferredElement>
                                <DeferredElement>
                                    <TextBlock x:Name="LabelControl" Column="1" AccessibilityView="Raw" Style="{StaticResource TaskListLabelTextBlockStyle}" />
                                </DeferredElement>
                            </TaskListLabeledButtonPanel.Children>
                            <TaskListLabeledButtonPanel.VisualStateGroups>
                                <VisualStateGroup x:Name="MarginStates">
                                    <VisualStateGroup.States>
                                        <VisualState x:Name="NormalMargin" />
                                        <VisualState x:Name="NormalMultiWindowMargin">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Margin" Value="{ThemeResource BackgroundElementMultiWindowMargin}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="LargeIconMultiWindowMargin">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Margin" Value="{ThemeResource BackgroundElementMultiWindowLargeIconMargin}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="RequestingAttentionMultiWindowMargin">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Margin" Value="{ThemeResource BackgroundElementMultiWindowRequestingAttentionMargin}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                    </VisualStateGroup.States>
                                </VisualStateGroup>
                                <VisualStateGroup x:Name="CommonStates">
                                    <VisualStateGroup.States>
                                        <VisualState x:Name="InactiveNormal" />
                                        <VisualState x:Name="InactivePointerOver">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorAltSecondaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorSecondaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="InactivePressed">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorTertiaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemStrokeColorQuinaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActiveNormal">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorSecondaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorPrimaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="WindowSharingNormal">
                                            <VisualState.Setters>
                                                <Setter Target="SharedWindowElement.BorderBrush" Value="{ThemeResource WindowSharingBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="WindowSharingPointerOver">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorPrimaryBrush}" />
                                                <Setter Target="SharedWindowElement.BorderBrush" Value="{ThemeResource WindowSharingBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="WindowSharingPressed">
                                            <VisualState.Setters>
                                                <Setter Target="SharedWindowElement.BorderBrush" Value="{ThemeResource WindowSharingBrush}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorTertiaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="WindowSharingActive">
                                            <VisualState.Setters>
                                                <Setter Target="SharedWindowElement.BorderBrush" Value="{ThemeResource WindowSharingBrush}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorSecondaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="WindowSharingActivePointerOver">
                                            <VisualState.Setters>
                                                <Setter Target="SharedWindowElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorPrimaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource WindowSharingBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="WindowSharingActivePressed">
                                            <VisualState.Setters>
                                                <Setter Target="SharedWindowElement.BorderBrush" Value="{ThemeResource WindowSharingBrush}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorTertiaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="RequestingAttention">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource RequestingAttentionBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource RequestingAttentionBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="RequestingAttentionPointerOver">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource RequestingAttentionHoverBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource RequestingAttentionHoverBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="RequestingAttentionPressed">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource RequestingAttentionPressedBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource RequestingAttentionPressedBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="WindowSharingRequestingAttention">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource RequestingAttentionBrush}" />
                                                <Setter Target="SharedWindowElement.BorderBrush" Value="{ThemeResource WindowSharingBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="WindowSharingRequestingAttentionPointerOver">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource RequestingAttentionHoverBrush}" />
                                                <Setter Target="SharedWindowElement.BorderBrush" Value="{ThemeResource WindowSharingBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="WindowSharingRequestingAttentionPressed">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource RequestingAttentionPressedBrush}" />
                                                <Setter Target="SharedWindowElement.BorderBrush" Value="{ThemeResource WindowSharingBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActivePointerOver">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorPrimaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorSecondaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActivePressed">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorTertiaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemStrokeColorQuinaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="MultiWindowNormal" />
                                        <VisualState x:Name="MultiWindowActive">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorSecondaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorPrimaryBrush}" />
                                                <Setter Target="MultiWindowElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorSecondaryBrush}" />
                                                <Setter Target="MultiWindowElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorPrimaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="MultiWindowPointerOver">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorPrimaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorSecondaryBrush}" />
                                                <Setter Target="MultiWindowElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorPrimaryBrush}" />
                                                <Setter Target="MultiWindowElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorSecondaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="MultiWindowPressed">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorTertiaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemStrokeColorQuinaryBrush}" />
                                                <Setter Target="MultiWindowElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorTertiaryBrush}" />
                                                <Setter Target="MultiWindowElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemStrokeColorQuinaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="MultiWindowSharingNormal">
                                            <VisualState.Setters>
                                                <Setter Target="SharedWindowElement.BorderBrush" Value="{ThemeResource WindowSharingBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="MultiWindowSharingActive">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorSecondaryBrush}" />
                                                <Setter Target="MultiWindowElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorSecondaryBrush}" />
                                                <Setter Target="SharedWindowElement.BorderBrush" Value="{ThemeResource WindowSharingBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="MultiWindowSharingPointerOver">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorPrimaryBrush}" />
                                                <Setter Target="MultiWindowElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorPrimaryBrush}" />
                                                <Setter Target="SharedWindowElement.BorderBrush" Value="{ThemeResource WindowSharingBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="MultiWindowSharingPressed">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorTertiaryBrush}" />
                                                <Setter Target="SharedWindowElement.BorderBrush" Value="{ThemeResource WindowSharingBrush}" />
                                                <Setter Target="MultiWindowElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorTertiaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="RequestingAttentionMulti">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource RequestingAttentionBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource RequestingAttentionBrush}" />
                                                <Setter Target="MultiWindowElement.Background" Value="{ThemeResource RequestingAttentionBrush}" />
                                                <Setter Target="MultiWindowElement.BorderBrush" Value="{ThemeResource RequestingAttentionBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="RequestingAttentionMultiPointerOver">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource RequestingAttentionHoverBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource RequestingAttentionHoverBrush}" />
                                                <Setter Target="MultiWindowElement.Background" Value="{ThemeResource RequestingAttentionHoverBrush}" />
                                                <Setter Target="MultiWindowElement.BorderBrush" Value="{ThemeResource RequestingAttentionHoverBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="RequestingAttentionMultiPressed">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource RequestingAttentionPressedBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource RequestingAttentionPressedBrush}" />
                                                <Setter Target="MultiWindowElement.Background" Value="{ThemeResource RequestingAttentionPressedBrush}" />
                                                <Setter Target="MultiWindowElement.BorderBrush" Value="{ThemeResource RequestingAttentionPressedBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="RequestingAttentionMultiWindowSharing">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource RequestingAttentionBrush}" />
                                                <Setter Target="SharedWindowElement.BorderBrush" Value="{ThemeResource WindowSharingBrush}" />
                                                <Setter Target="MultiWindowElement.Background" Value="{ThemeResource RequestingAttentionBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="RequestingAttentionMultiWindowSharingPointerOver">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource RequestingAttentionHoverBrush}" />
                                                <Setter Target="SharedWindowElement.BorderBrush" Value="{ThemeResource WindowSharingBrush}" />
                                                <Setter Target="MultiWindowElement.Background" Value="{ThemeResource RequestingAttentionHoverBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="RequestingAttentionMultiWindowSharingPressed">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource RequestingAttentionPressedBrush}" />
                                                <Setter Target="SharedWindowElement.BorderBrush" Value="{ThemeResource WindowSharingBrush}" />
                                                <Setter Target="MultiWindowElement.Background" Value="{ThemeResource RequestingAttentionPressedBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                    </VisualStateGroup.States>
                                </VisualStateGroup>
                                <VisualStateGroup x:Name="RunningIndicatorStates">
                                    <VisualStateGroup.States>
                                        <VisualState x:Name="NoRunningIndicator" />
                                        <VisualState x:Name="InactiveRunningIndicator" />
                                        <VisualState x:Name="ActiveRunningIndicator">
                                            <VisualState.Setters>
                                                <Setter Target="RunningIndicator.Fill" Value="{ThemeResource RunningIndicatorActiveBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="RequestingAttentionRunningIndicator">
                                            <VisualState.Setters>
                                                <Setter Target="RunningIndicator.Fill" Value="{ThemeResource RunningIndicatorRequestingAttentionBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                    </VisualStateGroup.States>
                                </VisualStateGroup>
                                <VisualStateGroup x:Name="DefaultIconStates">
                                    <VisualStateGroup.States>
                                        <VisualState x:Name="UseDefaultIcon">
                                            <VisualState.Setters>
                                                <Setter Target="DefaultIcon.Visibility" Value="Visible" />
                                                <Setter Target="Icon.Visibility" Value="Collapsed" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="UseApplicationIcon">
                                            <VisualState.Setters>
                                                <Setter Target="DefaultIcon.Visibility" Value="Collapsed" />
                                                <Setter Target="Icon.Visibility" Value="Visible" />
                                            </VisualState.Setters>
                                        </VisualState>
                                    </VisualStateGroup.States>
                                </VisualStateGroup>
                            </TaskListLabeledButtonPanel.VisualStateGroups>
                        </TaskListLabeledButtonPanel>
                    </ControlTemplate.Template>
                </ControlTemplate>
            </Setter.Value>
        </Setter>
    </Style>
    <Style x:Key="TaskListLabeledButtonRecommendation" TargetType="TaskListButton">
        <Setter Property="HorizontalAlignment" Value="Stretch" />
        <Setter Property="VerticalAlignment" Value="Stretch" />
        <Setter Property="HorizontalContentAlignment" Value="Stretch" />
        <Setter Property="VerticalContentAlignment" Value="Stretch" />
        <Setter Property="UseSystemFocusVisuals" Value="True" />
        <Setter Property="FocusVisualMargin" Value="-2" />
        <Setter Property="FocusVisualSecondaryThickness" Value="0" />
        <Setter Property="CornerRadius" Value="{StaticResource ControlLargeCornerRadius}" />
        <Setter Property="IsTabStop" Value="True" />
        <Setter Property="Template">
            <Setter.Value>
                <ControlTemplate TargetType="TaskListButton">
                    <ControlTemplate.Template>
                        <TaskListLabeledButtonPanel x:Name="IconPanel" Background="#00FFFFFF">
                            <TaskListLabeledButtonPanel.ColumnDefinitions>
                                <ColumnDefinition x:Name="IconColumnDefinition" />
                                <ColumnDefinition Width="Auto" />
                            </TaskListLabeledButtonPanel.ColumnDefinitions>
                            <TaskListLabeledButtonPanel.Children>
                                <Grid Padding="0,11">
                                    <Grid.Children>
                                        <DeferredElement>
                                            <Rectangle x:Name="MostRecentlyUsedDivider"
                                                Width="1"
                                                HorizontalAlignment="Left"
                                                VerticalAlignment="Center"
                                                Fill="{ThemeResource TaskbarRecommendatonDividerBrush}"
                                                IsHitTestVisible="False"
                                                ColumnSpan="2" />
                                        </DeferredElement>
                                    </Grid.Children>
                                </Grid>
                                <Border CornerRadius="{TemplateBinding CornerRadius}" IsTemplateFocusTarget="True" IsHitTestVisible="False" ColumnSpan="2" />
                                <Border x:Name="BackgroundElement"
                                    Background="{ThemeResource ShellTaskbarItemFillColorTransparentBrush}"
                                    BorderBrush="#00FFFFFF"
                                    BorderThickness="{StaticResource TaskbarButtonOutlineThickness}"
                                    CornerRadius="{TemplateBinding CornerRadius}"
                                    Margin="0"
                                    IsHitTestVisible="False"
                                    ColumnSpan="2">
                                    <Border.BackgroundTransition>
                                        <BrushTransition Duration="0:0:0.083" />
                                    </Border.BackgroundTransition>
                                </Border>
                                <Image x:Name="Icon" AccessibilityView="Raw" IsHitTestVisible="False" Stretch="Uniform" />
                                <DeferredElement>
                                    <Rectangle x:Name="DefaultIcon"
                                        Fill="{ThemeResource RunningIndicatorBrush}"
                                        RadiusX="5"
                                        RadiusY="5"
                                        HorizontalAlignment="Center"
                                        VerticalAlignment="Center"
                                        AccessibilityView="Raw"
                                        IsHitTestVisible="False"
                                        Stretch="Uniform" />
                                </DeferredElement>
                                <DeferredElement>
                                    <Image x:Name="OverlayIcon"
                                        HorizontalAlignment="Center"
                                        VerticalAlignment="Center"
                                        AccessibilityView="Raw"
                                        IsHitTestVisible="False"
                                        Stretch="Uniform" />
                                </DeferredElement>
                                <DeferredElement>
                                    <Badge x:Name="BadgeControl" VerticalAlignment="Center" HorizontalAlignment="Center" AccessibilityView="Raw" IsHitTestVisible="False" />
                                </DeferredElement>
                                <DeferredElement>
                                    <TextBlock x:Name="LabelControl" Column="1" AccessibilityView="Raw" Style="{StaticResource TaskListLabelTextBlockStyle}" />
                                </DeferredElement>
                            </TaskListLabeledButtonPanel.Children>
                            <TaskListLabeledButtonPanel.VisualStateGroups>
                                <VisualStateGroup x:Name="MarginStates">
                                    <VisualStateGroup.States>
                                        <VisualState x:Name="NormalMargin" />
                                    </VisualStateGroup.States>
                                </VisualStateGroup>
                                <VisualStateGroup x:Name="CommonStates">
                                    <VisualStateGroup.States>
                                        <VisualState x:Name="InactiveNormal" />
                                        <VisualState x:Name="InactivePointerOver">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorAltSecondaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorSecondaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="InactivePressed">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorTertiaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemStrokeColorQuinaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActiveNormal">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorSecondaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorPrimaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="RequestingAttention">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource RequestingAttentionBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource RequestingAttentionBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="RequestingAttentionPointerOver">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource RequestingAttentionHoverBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource RequestingAttentionHoverBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="RequestingAttentionPressed">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource RequestingAttentionPressedBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource RequestingAttentionPressedBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActivePointerOver">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorPrimaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorSecondaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActivePressed">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorTertiaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemStrokeColorQuinaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                    </VisualStateGroup.States>
                                </VisualStateGroup>
                                <VisualStateGroup x:Name="DefaultIconStates">
                                    <VisualStateGroup.States>
                                        <VisualState x:Name="UseDefaultIcon">
                                            <VisualState.Setters>
                                                <Setter Target="DefaultIcon.Visibility" Value="Visible" />
                                                <Setter Target="Icon.Visibility" Value="Collapsed" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="UseApplicationIcon">
                                            <VisualState.Setters>
                                                <Setter Target="DefaultIcon.Visibility" Value="Collapsed" />
                                                <Setter Target="Icon.Visibility" Value="Visible" />
                                            </VisualState.Setters>
                                        </VisualState>
                                    </VisualStateGroup.States>
                                </VisualStateGroup>
                            </TaskListLabeledButtonPanel.VisualStateGroups>
                        </TaskListLabeledButtonPanel>
                    </ControlTemplate.Template>
                </ControlTemplate>
            </Setter.Value>
        </Setter>
    </Style>
    <Style x:Key="TaskListLabelTextBlockStyle" TargetType="TextBlock">
        <Setter Property="VerticalAlignment" Value="Center" />
        <Setter Property="HorizontalAlignment" Value="Center" />
        <Setter Property="Foreground" Value="{ThemeResource TaskListLabelForegroundBrush}" />
        <Setter Property="FontSize" Value="12" />
        <Setter Property="MaxLines" Value="1" />
        <Setter Property="TextTrimming" Value="Clip" />
        <Setter Property="HorizontalTextAlignment" Value="Left" />
        <Setter Property="Padding" Value="{StaticResource TaskListLabelPadding}" />
        <Setter Property="FontFamily" Value="Segoe UI Variable" />
    </Style>
    <Thickness x:Key="TaskListLabelPadding" Value="0,0,8,0" />
    <Thickness x:Key="TaskListButtonIndicatorMargin" Value="0,0,0,1" />
    <Style x:Key="TaskItemThumbnailViewStyle" TargetType="TaskItemThumbnailView">
        <Setter Property="HorizontalAlignment" Value="Left" />
        <Setter Property="VerticalAlignment" Value="Stretch" />
        <Setter Property="MinWidth" Value="{StaticResource ThumbnailViewMinWidth}" />
        <Setter Property="MaxWidth" Value="{StaticResource ThumbnailViewMaxWidth}" />
        <Setter Property="UseSystemFocusVisuals" Value="True" />
        <Setter Property="IsTabStop" Value="True" />
        <Setter Property="Template">
            <Setter.Value>
                <ControlTemplate TargetType="TaskItemThumbnailView">
                    <ControlTemplate.Template>
                        <Grid XYFocusKeyboardNavigation="Enabled">
                            <Grid.ColumnDefinitions>
                                <ColumnDefinition Width="Auto" />
                                <ColumnDefinition Width="*" />
                                <ColumnDefinition Width="Auto" />
                            </Grid.ColumnDefinitions>
                            <Grid.RowDefinitions>
                                <RowDefinition Height="40" />
                                <RowDefinition Height="*" />
                            </Grid.RowDefinitions>
                            <Grid.Children>
                                <Border x:Name="BackgroundBorder" RowSpan="2" ColumnSpan="3" Background="{ThemeResource TaskItemThumbnailBackgroundBrush}" CornerRadius="8" />
                                <ItemsRepeater x:Name="IconsRepeater" Margin="8,0" VerticalAlignment="Center" IsHitTestVisible="False">
                                    <ItemsRepeater.ItemTemplate>
                                        <DataTemplate>
                                            <DataTemplate.Template>
                                                <Image Width="{StaticResource ThumbnailIconSize}" Height="{StaticResource ThumbnailIconSize}" VerticalAlignment="Center" Stretch="Uniform" />
                                            </DataTemplate.Template>
                                        </DataTemplate>
                                    </ItemsRepeater.ItemTemplate>
                                    <ItemsRepeater.Layout>
                                        <StackLayout Orientation="Horizontal" Spacing="8" />
                                    </ItemsRepeater.Layout>
                                </ItemsRepeater>
                                <TextBlock x:Name="DisplayNameTextBlock"
                                    Column="1"
                                    VerticalAlignment="Center"
                                    IsHitTestVisible="False"
                                    TextTrimming="CharacterEllipsis"
                                    TextWrapping="NoWrap"
                                    Foreground="{ThemeResource TaskItemThumbnailTextStroke}" />
                                <Button x:Name="CloseButton"
                                    Column="2"
                                    Width="32"
                                    Height="32"
                                    Margin="4"
                                    Padding="0"
                                    BorderThickness="{StaticResource TaskbarButtonOutlineThickness}"
                                    FontFamily="{ThemeResource SymbolThemeFontFamily}"
                                    FontSize="10"
                                    Background="{ThemeResource CaptionControlFillBrush}"
                                    Foreground="{ThemeResource TaskItemThumbnailTextStroke}"
                                    BorderBrush="{ThemeResource CaptionControlBorderBrush}"
                                    IsTabStop="True"
                                    Content="">
                                    <Button.Resources>
                                        <ResourceDictionary>
                                            <ResourceDictionary.ThemeDictionaries>
                                                <ResourceDictionary x:Key="Light">
                                                    <SolidColorBrush x:Key="ButtonForegroundPressed" Color="#FFFFFFFF" />
                                                    <SolidColorBrush x:Key="ButtonForegroundPointerOver" Color="#FFFFFFFF" />
                                                    <SolidColorBrush x:Key="ButtonBorderBrushPressed" Color="#0F000000" />
                                                    <SolidColorBrush x:Key="ButtonBorderBrushPointerOver" Color="#0F000000" />
                                                    <SolidColorBrush x:Key="ButtonBackgroundPressed" Color="#E5C42B1C" />
                                                    <SolidColorBrush x:Key="ButtonBackgroundPointerOver" Color="#FFC42B1C" />
                                                </ResourceDictionary>
                                                <ResourceDictionary x:Key="Dark">
                                                    <SolidColorBrush x:Key="ButtonForegroundPressed" Color="#FFFFFFFF" />
                                                    <SolidColorBrush x:Key="ButtonForegroundPointerOver" Color="#FFFFFFFF" />
                                                    <SolidColorBrush x:Key="ButtonBorderBrushPressed" Color="#12FFFFFF" />
                                                    <SolidColorBrush x:Key="ButtonBorderBrushPointerOver" Color="#12FFFFFF" />
                                                    <SolidColorBrush x:Key="ButtonBackgroundPressed" Color="#E5C42B1C" />
                                                    <SolidColorBrush x:Key="ButtonBackgroundPointerOver" Color="#FFC42B1C" />
                                                </ResourceDictionary>
                                                <ResourceDictionary x:Key="HighContrast">
                                                    <StaticResource x:Key="ButtonForegroundPressed" ResourceKey="SystemColorHighlightTextColor" />
                                                    <StaticResource x:Key="ButtonForegroundPointerOver" ResourceKey="SystemColorHighlightTextColor" />
                                                    <StaticResource x:Key="ButtonBorderBrushPressed" ResourceKey="SystemColorHighlightColor" />
                                                    <StaticResource x:Key="ButtonBorderBrushPointerOver" ResourceKey="SystemColorHighlightColor" />
                                                    <StaticResource x:Key="ButtonBackgroundPressed" ResourceKey="SystemColorHighlightColor" />
                                                    <StaticResource x:Key="ButtonBackgroundPointerOver" ResourceKey="SystemColorHighlightColor" />
                                                </ResourceDictionary>
                                            </ResourceDictionary.ThemeDictionaries>
                                        </ResourceDictionary>
                                    </Button.Resources>
                                </Button>
                                <Grid x:Name="DetailedViewGrid" XYFocusKeyboardNavigation="Enabled" Row="1" ColumnSpan="3">
                                    <Grid.RowDefinitions>
                                        <RowDefinition Height="Auto" />
                                        <RowDefinition Height="*" />
                                        <RowDefinition Height="Auto" />
                                        <RowDefinition Height="Auto" />
                                    </Grid.RowDefinitions>
                                    <Grid.Children>
                                        <DeferredElement>
                                            <ItemsRepeater x:Name="RecommendationsRepeater"
                                                HorizontalAlignment="Center"
                                                Row="0"
                                                ColumnSpan="3"
                                                Margin="5,0"
                                                ItemTemplate="{StaticResource ExtendedUIRecommendationTemplate}">
                                                <ItemsRepeater.Layout>
                                                    <StackLayout Orientation="Vertical" Spacing="8" />
                                                </ItemsRepeater.Layout>
                                            </ItemsRepeater>
                                        </DeferredElement>
                                        <Border x:Name="ThumbnailVisualHostWrapper"
                                            Row="1"
                                            Margin="0,0,0,8"
                                            VerticalAlignment="Center"
                                            Background="#00FFFFFF"
                                            HorizontalAlignment="Center">
                                            <Border.Child>
                                                <Border x:Name="ThumbnailVisualHost"
                                                    Margin="8,0"
                                                    IsHitTestVisible="False"
                                                    VerticalAlignment="Center"
                                                    HorizontalAlignment="Center"
                                                    CornerRadius="2"
                                                    FlowDirection="LeftToRight" />
                                            </Border.Child>
                                        </Border>
                                        <DeferredElement>
                                            <ItemsRepeater x:Name="ThumbBarRepeater" Row="2" Margin="8,0,8,8" HorizontalAlignment="Center" ItemTemplate="{StaticResource ThumbBarButtonTemplate}">
                                                <ItemsRepeater.Layout>
                                                    <StackLayout Orientation="Horizontal" Spacing="4" />
                                                </ItemsRepeater.Layout>
                                            </ItemsRepeater>
                                        </DeferredElement>
                                        <DeferredElement>
                                            <ShareWindowPanel x:Name="ShareWindowPanel" Row="3" IsTabStop="True" />
                                        </DeferredElement>
                                    </Grid.Children>
                                </Grid>
                            </Grid.Children>
                            <Grid.VisualStateGroups>
                                <VisualStateGroup x:Name="CommonStates">
                                    <VisualStateGroup.States>
                                        <VisualState x:Name="Normal" />
                                        <VisualState x:Name="PointerOver">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundBorder.Background" Value="{ThemeResource TaskItemThumbnailBackgroundPointerOverBrush}" />
                                                <Setter Target="DisplayNameTextBlock.Foreground" Value="{ThemeResource TaskItemThumbnailAltTextStroke}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="Pressed">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundBorder.Background" Value="{ThemeResource TaskItemThumbnailBackgroundPressedBrush}" />
                                                <Setter Target="DisplayNameTextBlock.Foreground" Value="{ThemeResource TaskItemThumbnailAltTextStroke}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="RequestingAttention">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundBorder.Background" Value="{ThemeResource RequestingAttentionBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="RequestingAttentionPointerOver">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundBorder.Background" Value="{ThemeResource RequestingAttentionHoverBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="RequestingAttentionPressed">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundBorder.Background" Value="{ThemeResource RequestingAttentionPressedBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                    </VisualStateGroup.States>
                                </VisualStateGroup>
                            </Grid.VisualStateGroups>
                        </Grid>
                    </ControlTemplate.Template>
                </ControlTemplate>
            </Setter.Value>
        </Setter>
    </Style>
    <DataTemplate x:Key="TaskItemThumbnailTemplate">
        <DataTemplate.Template>
            <TaskItemThumbnailView ShowDetailedView="True" Style="{StaticResource TaskItemThumbnailViewStyle}">
                <TaskItemThumbnailView.ToolTip>
                    <ToolTip IsEnabled="True" Placement="Top" VerticalOffset="{StaticResource ToolTipMargin}" />
                </TaskItemThumbnailView.ToolTip>
            </TaskItemThumbnailView>
        </DataTemplate.Template>
    </DataTemplate>
    <Style x:Key="TaskItemThumbnailScrollableViewStyle" TargetType="TaskItemThumbnailView" BasedOn="{StaticResource TaskItemThumbnailViewStyle}">
        <Setter Property="HorizontalAlignment" Value="Stretch" />
        <Setter Property="MinWidth" Value="{StaticResource ThumbnailScrollableViewMinWidth}" />
        <Setter Property="MaxWidth" Value="{StaticResource ThumbnailScrollableViewMaxWidth}" />
    </Style>
    <DataTemplate x:Key="TaskItemThumbnailScrollableTemplate">
        <DataTemplate.Template>
            <TaskItemThumbnailView ShowDetailedView="False" Style="{StaticResource TaskItemThumbnailScrollableViewStyle}">
                <TaskItemThumbnailView.ToolTip>
                    <ToolTip IsEnabled="True" Placement="Top" VerticalOffset="{StaticResource ToolTipMargin}" />
                </TaskItemThumbnailView.ToolTip>
            </TaskItemThumbnailView>
        </DataTemplate.Template>
    </DataTemplate>
    <Style x:Key="TaskItemThumbnailScrollableListStyle" TargetType="ScrollViewer">
        <Setter Property="HorizontalScrollMode" Value="Disabled" />
        <Setter Property="VerticalScrollMode" Value="Enabled" />
        <Setter Property="IsHorizontalRailEnabled" Value="False" />
        <Setter Property="IsVerticalRailEnabled" Value="True" />
        <Setter Property="IsTabStop" Value="False" />
        <Setter Property="ZoomMode" Value="Disabled" />
        <Setter Property="HorizontalContentAlignment" Value="Left" />
        <Setter Property="VerticalContentAlignment" Value="Top" />
        <Setter Property="VerticalScrollBarVisibility" Value="Hidden" />
        <Setter Property="HorizontalScrollBarVisibility" Value="Disabled" />
        <Setter Property="VerticalSnapPointsType" Value="Mandatory" />
        <Setter Property="VerticalSnapPointsAlignment" Value="Far" />
        <Setter Property="UseSystemFocusVisuals" Value="{StaticResource UseSystemFocusVisuals}" />
        <Setter Property="Template">
            <Setter.Value>
                <ControlTemplate TargetType="ScrollViewer">
                    <ControlTemplate.Template>
                        <Border x:Name="Root">
                            <Border.Child>
                                <Grid>
                                    <Grid.RowDefinitions>
                                        <RowDefinition Height="Auto" />
                                        <RowDefinition Height="*" />
                                        <RowDefinition Height="Auto" />
                                    </Grid.RowDefinitions>
                                    <Grid.Children>
                                        <RepeatButton x:Name="VerticalBackwardButton"
                                            Row="0"
                                            IsTabStop="False"
                                            Interval="50"
                                            MinHeight="20"
                                            Template="{StaticResource VerticalButtonTemplate}"
                                            AllowFocusOnInteraction="False"
                                            AllowDrop="True"
                                            HorizontalAlignment="Stretch"
                                            Visibility="Collapsed"
                                            AccessibilityView="Raw" />
                                        <ScrollContentPresenter x:Name="ScrollContentPresenter" Row="1" ContentTemplate="{TemplateBinding ContentTemplate}" />
                                        <RepeatButton x:Name="VerticalForwardButton"
                                            Row="2"
                                            IsTabStop="False"
                                            Interval="50"
                                            MinHeight="20"
                                            Template="{StaticResource VerticalButtonTemplate}"
                                            AllowFocusOnInteraction="False"
                                            HorizontalAlignment="Stretch"
                                            Visibility="Collapsed"
                                            AccessibilityView="Raw" />
                                    </Grid.Children>
                                </Grid>
                            </Border.Child>
                            <Border.VisualStateGroups>
                                <VisualStateGroup x:Name="ScrollPositionStates">
                                    <VisualStateGroup.States>
                                        <VisualState x:Name="NoScroll">
                                            <VisualState.Setters>
                                                <Setter Target="VerticalBackwardButton.Visibility" Value="Collapsed" />
                                                <Setter Target="VerticalForwardButton.Visibility" Value="Collapsed" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ScrollUpDown">
                                            <VisualState.Setters>
                                                <Setter Target="VerticalBackwardButton.Visibility" Value="Visible" />
                                                <Setter Target="VerticalForwardButton.Visibility" Value="Visible" />
                                            </VisualState.Setters>
                                        </VisualState>
                                    </VisualStateGroup.States>
                                </VisualStateGroup>
                            </Border.VisualStateGroups>
                        </Border>
                    </ControlTemplate.Template>
                </ControlTemplate>
            </Setter.Value>
        </Setter>
    </Style>
    <Double x:Key="SmallTaskbarLabeledButtonMinWidth" Value="76" />
    <Double x:Key="SmallTaskbarButtonExtent" Value="32" />
    <Double x:Key="SmallNumericBadgeFontSize" Value="11" />
    <Double x:Key="SmallGlyphBadgeFontSize" Value="12" />
    <Double x:Key="ShareWindowButtonIconSize" Value="16" />
    <Double x:Key="SearchPillButtonMinExtent" Value="100" />
    <Thickness x:Key="SearchPillButtonMargin" Value="1,4" />
    <CornerRadius x:Key="SearchPillButtonCornerRadius" Value="16" />
    <Thickness x:Key="SearchBoxWithGleamButtonContainerMargin" Value="1,3,3,3" />
    <Thickness x:Key="SearchBoxWithGleamButtonContainerMargin_Velocity43766227" Value="1,3,3,0" />
    <Double x:Key="SearchBoxWithGleamButtonContainerHeight" Value="26" />
    <Double x:Key="SearchBoxWithGleamButtonContainerHeightShyTaskbar" Value="40" />
    <CornerRadius x:Key="SearchBoxWithGleamButtonContainerCornerRadius" Value="13" />
    <Thickness x:Key="SearchBoxTextWideGleamMargin" Value="37,0,62,1" />
    <Thickness x:Key="SearchBoxTextMediumGleamMargin" Value="37,0,38,1" />
    <Thickness x:Key="SearchBoxTextMargin" Value="37,0,16,1" />
    <CornerRadius x:Key="SearchBoxRoundCornerRadius" Value="16" />
    <Style x:Key="SearchBoxOnTaskbar" TargetType="SearchBoxButton">
        <Setter Property="HorizontalAlignment" Value="Stretch" />
        <Setter Property="VerticalAlignment" Value="Stretch" />
        <Setter Property="HorizontalContentAlignment" Value="Stretch" />
        <Setter Property="VerticalContentAlignment" Value="Stretch" />
        <Setter Property="IsTabStop" Value="True" />
        <Setter Property="UseSystemFocusVisuals" Value="True" />
        <Setter Property="FocusVisualMargin" Value="-2" />
        <Setter Property="FocusVisualSecondaryThickness" Value="0" />
        <Setter Property="CornerRadius" Value="{StaticResource ControlLargeCornerRadius}" />
        <Setter Property="Template">
            <Setter.Value>
                <ControlTemplate TargetType="SearchBoxButton">
                    <ControlTemplate.Template>
                        <TaskListButtonPanel x:Name="ExperienceToggleButtonRootPanel" Background="#00FFFFFF">
                            <TaskListButtonPanel.Children>
                                <Border x:Name="BackgroundElement"
                                    Background="{ThemeResource ShellTaskbarItemFillColorTransparentBrush}"
                                    BorderBrush="#00FFFFFF"
                                    IsTemplateFocusTarget="True"
                                    BorderThickness="{StaticResource TaskbarButtonOutlineThickness}"
                                    CornerRadius="{TemplateBinding CornerRadius}" />
                                <AnimatedVisualPlayer x:Name="Icon"
                                    HorizontalAlignment="Center"
                                    VerticalAlignment="Center"
                                    AutoPlay="False"
                                    IsHitTestVisible="False"
                                    Stretch="Uniform"
                                    AnimationOptimization="Resources"
                                    AccessibilityView="Raw" />
                                <TextBlock x:Name="SearchBoxTextBlock"
                                    IsHitTestVisible="False"
                                    Style="{StaticResource BodyTextBlockStyle}"
                                    Margin="{StaticResource SearchBoxTextMargin}"
                                    Foreground="{ThemeResource TextControlPlaceholderForeground}"
                                    Visibility="Collapsed"
                                    TextTrimming="WordEllipsis"
                                    HorizontalAlignment="Left"
                                    VerticalAlignment="Center" />
                                <Grid HorizontalAlignment="Right">
                                    <Grid.Children>
                                        <DeferredElement>
                                            <Button x:Name="GleamEntryPointButton" Style="{StaticResource GleamEntryPointButtonStyle}" AutomationId="SearchGleamButton" Visibility="Collapsed">
                                                <Button.ToolTip>
                                                    <ToolTip Placement="Top" VerticalOffset="{StaticResource ToolTipMargin}" />
                                                </Button.ToolTip>
                                            </Button>
                                        </DeferredElement>
                                        <DeferredElement>
                                            <Grid x:Name="DynamicSearchBoxGleamContainer"
                                                IsHitTestVisible="False"
                                                Margin="{StaticResource SearchBoxGleamContainerMargin}"
                                                CornerRadius="{StaticResource SearchBoxGleamContainerCornerRadius}"
                                                VerticalAlignment="Center"
                                                Visibility="Collapsed">
                                                <Grid.Children>
                                                    <Image x:Name="DynamicSearchBoxGleamImage" Height="{StaticResource SearchBoxGleamContainerHeight}" />
                                                </Grid.Children>
                                            </Grid>
                                        </DeferredElement>
                                    </Grid.Children>
                                </Grid>
                            </TaskListButtonPanel.Children>
                            <TaskListButtonPanel.VisualStateGroups>
                                <VisualStateGroup x:Name="CommonStates">
                                    <VisualStateGroup.States>
                                        <VisualState x:Name="InactiveNormal_SearchIcon">
                                            <VisualState.Setters>
                                                <Setter Target="SearchBoxTextBlock.Visibility" Value="Collapsed" />
                                                <Setter Target="BackgroundElement.BackgroundTransition" Value="{StaticResource BackgroundElementBrushTransition}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="InactivePointerOver_SearchIcon">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.BackgroundTransition" Value="{StaticResource BackgroundElementBrushTransition}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorAltSecondaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorSecondaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="InactivePressed_SearchIcon">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.BackgroundTransition" Value="{StaticResource BackgroundElementBrushTransition}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorTertiaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemStrokeColorQuinaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActiveNormal_SearchIcon">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.BackgroundTransition" Value="{StaticResource BackgroundElementBrushTransition}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorSecondaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorPrimaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActivePointerOver_SearchIcon">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.BackgroundTransition" Value="{StaticResource BackgroundElementBrushTransition}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorPrimaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorSecondaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActivePressed_SearchIcon">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.BackgroundTransition" Value="{StaticResource BackgroundElementBrushTransition}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorTertiaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemStrokeColorQuinaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="InactiveNormal_SearchBox_Wave3">
                                            <VisualState.Setters>
                                                <Setter Target="Icon.HorizontalAlignment" Value="Left" />
                                                <Setter Target="Icon.Margin" Value="{StaticResource SearchBoxIconMargin}" />
                                                <Setter Target="ExperienceToggleButtonRootPanel.Background">
                                                    <Setter.Value>
                                                        <NullExtension />
                                                    </Setter.Value>
                                                </Setter>
                                                <Setter Target="BackgroundElement.CornerRadius" Value="{StaticResource SearchBoxRoundCornerRadius}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource SearchBoxTextControlBackground3}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource SearchBoxTextControlBorderBrush3}" />
                                                <Setter Target="BackgroundElement.MinWidth" Value="{StaticResource SearchBoxButtonExtent}" />
                                                <Setter Target="SearchBoxTextBlock.Visibility" Value="Visible" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="InactivePointerOver_SearchBox_Wave3">
                                            <VisualState.Setters>
                                                <Setter Target="Icon.HorizontalAlignment" Value="Left" />
                                                <Setter Target="Icon.Margin" Value="{StaticResource SearchBoxIconMargin}" />
                                                <Setter Target="ExperienceToggleButtonRootPanel.Background">
                                                    <Setter.Value>
                                                        <NullExtension />
                                                    </Setter.Value>
                                                </Setter>
                                                <Setter Target="BackgroundElement.CornerRadius" Value="{StaticResource SearchBoxRoundCornerRadius}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource SearchBoxTextControlBackgroundPointerOver3}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource SearchBoxTextControlBorderBrushPointerOver3}" />
                                                <Setter Target="SearchBoxTextBlock.Foreground" Value="{ThemeResource TextControlPlaceholderForegroundPointerOver}" />
                                                <Setter Target="BackgroundElement.MinWidth" Value="{StaticResource SearchBoxButtonExtent}" />
                                                <Setter Target="SearchBoxTextBlock.Visibility" Value="Visible" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="InactivePressed_SearchBox_Wave3">
                                            <VisualState.Setters>
                                                <Setter Target="Icon.HorizontalAlignment" Value="Left" />
                                                <Setter Target="Icon.Margin" Value="{StaticResource SearchBoxIconMargin}" />
                                                <Setter Target="ExperienceToggleButtonRootPanel.Background">
                                                    <Setter.Value>
                                                        <NullExtension />
                                                    </Setter.Value>
                                                </Setter>
                                                <Setter Target="BackgroundElement.CornerRadius" Value="{StaticResource SearchBoxRoundCornerRadius}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource SearchBoxTextControlBackgroundFocused}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource SearchBoxTextControlBorderBrushFocused3}" />
                                                <Setter Target="SearchBoxTextBlock.Foreground" Value="{ThemeResource TextControlPlaceholderForegroundFocused}" />
                                                <Setter Target="BackgroundElement.BorderThickness" Value="{ThemeResource SearchBoxTextControlBorderThemeThicknessFocused}" />
                                                <Setter Target="BackgroundElement.MinWidth" Value="{StaticResource SearchBoxButtonExtent}" />
                                                <Setter Target="SearchBoxTextBlock.Visibility" Value="Visible" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActiveNormal_SearchBox_Wave3">
                                            <VisualState.Setters>
                                                <Setter Target="Icon.HorizontalAlignment" Value="Left" />
                                                <Setter Target="Icon.Margin" Value="{StaticResource SearchBoxIconMargin}" />
                                                <Setter Target="ExperienceToggleButtonRootPanel.Background">
                                                    <Setter.Value>
                                                        <NullExtension />
                                                    </Setter.Value>
                                                </Setter>
                                                <Setter Target="BackgroundElement.CornerRadius" Value="{StaticResource SearchBoxRoundCornerRadius}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource SearchBoxTextControlBackgroundFocused}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource SearchBoxTextControlBorderBrushFocused3}" />
                                                <Setter Target="SearchBoxTextBlock.Foreground" Value="{ThemeResource TextControlPlaceholderForegroundFocused}" />
                                                <Setter Target="BackgroundElement.BorderThickness" Value="{ThemeResource SearchBoxTextControlBorderThemeThicknessFocused}" />
                                                <Setter Target="BackgroundElement.MinWidth" Value="{StaticResource SearchBoxButtonExtent}" />
                                                <Setter Target="SearchBoxTextBlock.Visibility" Value="Visible" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActivePointerOver_SearchBox_Wave3">
                                            <VisualState.Setters>
                                                <Setter Target="Icon.HorizontalAlignment" Value="Left" />
                                                <Setter Target="Icon.Margin" Value="{StaticResource SearchBoxIconMargin}" />
                                                <Setter Target="ExperienceToggleButtonRootPanel.Background">
                                                    <Setter.Value>
                                                        <NullExtension />
                                                    </Setter.Value>
                                                </Setter>
                                                <Setter Target="BackgroundElement.CornerRadius" Value="{StaticResource SearchBoxRoundCornerRadius}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource SearchBoxTextControlBackgroundFocused}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource SearchBoxTextControlBorderBrushFocused3}" />
                                                <Setter Target="SearchBoxTextBlock.Foreground" Value="{ThemeResource TextControlPlaceholderForegroundFocused}" />
                                                <Setter Target="BackgroundElement.BorderThickness" Value="{ThemeResource SearchBoxTextControlBorderThemeThicknessFocused}" />
                                                <Setter Target="BackgroundElement.MinWidth" Value="{StaticResource SearchBoxButtonExtent}" />
                                                <Setter Target="SearchBoxTextBlock.Visibility" Value="Visible" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActivePressed_SearchBox_Wave3">
                                            <VisualState.Setters>
                                                <Setter Target="Icon.HorizontalAlignment" Value="Left" />
                                                <Setter Target="Icon.Margin" Value="{StaticResource SearchBoxIconMargin}" />
                                                <Setter Target="ExperienceToggleButtonRootPanel.Background">
                                                    <Setter.Value>
                                                        <NullExtension />
                                                    </Setter.Value>
                                                </Setter>
                                                <Setter Target="BackgroundElement.CornerRadius" Value="{StaticResource SearchBoxRoundCornerRadius}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource SearchBoxTextControlBackgroundFocused}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource SearchBoxTextControlBorderBrushFocused3}" />
                                                <Setter Target="SearchBoxTextBlock.Foreground" Value="{ThemeResource TextControlPlaceholderForegroundFocused}" />
                                                <Setter Target="BackgroundElement.BorderThickness" Value="{ThemeResource SearchBoxTextControlBorderThemeThicknessFocused}" />
                                                <Setter Target="BackgroundElement.MinWidth" Value="{StaticResource SearchBoxButtonExtent}" />
                                                <Setter Target="SearchBoxTextBlock.Visibility" Value="Visible" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="InactiveNormal_SearchBoxCustomTheme">
                                            <VisualState.Setters>
                                                <Setter Target="Icon.HorizontalAlignment" Value="Left" />
                                                <Setter Target="Icon.Margin" Value="{StaticResource SearchBoxIconMargin}" />
                                                <Setter Target="ExperienceToggleButtonRootPanel.Background">
                                                    <Setter.Value>
                                                        <NullExtension />
                                                    </Setter.Value>
                                                </Setter>
                                                <Setter Target="BackgroundElement.CornerRadius" Value="{StaticResource SearchBoxRoundCornerRadius}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource CustomThemeSearchBoxTextControlBackground}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource CustomThemeSearchBoxTextControlBorderBrush}" />
                                                <Setter Target="SearchBoxTextBlock.Foreground" Value="{ThemeResource CustomThemeSearchBoxTextControlPlaceholderForeground}" />
                                                <Setter Target="BackgroundElement.MinWidth" Value="{StaticResource SearchBoxButtonExtent}" />
                                                <Setter Target="SearchBoxTextBlock.Visibility" Value="Visible" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="InactivePointerOver_SearchBoxCustomTheme">
                                            <VisualState.Setters>
                                                <Setter Target="Icon.HorizontalAlignment" Value="Left" />
                                                <Setter Target="Icon.Margin" Value="{StaticResource SearchBoxIconMargin}" />
                                                <Setter Target="ExperienceToggleButtonRootPanel.Background">
                                                    <Setter.Value>
                                                        <NullExtension />
                                                    </Setter.Value>
                                                </Setter>
                                                <Setter Target="BackgroundElement.CornerRadius" Value="{StaticResource SearchBoxRoundCornerRadius}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource CustomThemeSearchBoxTextControlBackgroundPointerOver}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource CustomThemeSearchBoxTextControlBorderBrushPointerOver}" />
                                                <Setter Target="SearchBoxTextBlock.Foreground" Value="{ThemeResource CustomThemeSearchBoxTextControlPlaceholderForegroundPointerOver}" />
                                                <Setter Target="BackgroundElement.MinWidth" Value="{StaticResource SearchBoxButtonExtent}" />
                                                <Setter Target="SearchBoxTextBlock.Visibility" Value="Visible" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="InactivePressed_SearchBoxCustomTheme">
                                            <VisualState.Setters>
                                                <Setter Target="Icon.HorizontalAlignment" Value="Left" />
                                                <Setter Target="Icon.Margin" Value="{StaticResource SearchBoxIconMargin}" />
                                                <Setter Target="ExperienceToggleButtonRootPanel.Background">
                                                    <Setter.Value>
                                                        <NullExtension />
                                                    </Setter.Value>
                                                </Setter>
                                                <Setter Target="BackgroundElement.CornerRadius" Value="{StaticResource SearchBoxRoundCornerRadius}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource CustomThemeSearchBoxTextControlBackgroundFocused}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource CustomThemeSearchBoxTextControlBorderBrushFocused}" />
                                                <Setter Target="SearchBoxTextBlock.Foreground" Value="{ThemeResource CustomThemeSearchBoxTextControlPlaceholderForegroundFocused}" />
                                                <Setter Target="BackgroundElement.BorderThickness" Value="{ThemeResource SearchBoxTextControlBorderThemeThicknessFocused}" />
                                                <Setter Target="BackgroundElement.MinWidth" Value="{StaticResource SearchBoxButtonExtent}" />
                                                <Setter Target="SearchBoxTextBlock.Visibility" Value="Visible" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActiveNormal_SearchBoxCustomTheme">
                                            <VisualState.Setters>
                                                <Setter Target="Icon.HorizontalAlignment" Value="Left" />
                                                <Setter Target="Icon.Margin" Value="{StaticResource SearchBoxIconMargin}" />
                                                <Setter Target="ExperienceToggleButtonRootPanel.Background">
                                                    <Setter.Value>
                                                        <NullExtension />
                                                    </Setter.Value>
                                                </Setter>
                                                <Setter Target="BackgroundElement.CornerRadius" Value="{StaticResource SearchBoxRoundCornerRadius}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource CustomThemeSearchBoxTextControlBackgroundFocused}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource CustomThemeSearchBoxTextControlBorderBrushFocused}" />
                                                <Setter Target="SearchBoxTextBlock.Foreground" Value="{ThemeResource CustomThemeSearchBoxTextControlPlaceholderForegroundFocused}" />
                                                <Setter Target="BackgroundElement.BorderThickness" Value="{ThemeResource SearchBoxTextControlBorderThemeThicknessFocused}" />
                                                <Setter Target="BackgroundElement.MinWidth" Value="{StaticResource SearchBoxButtonExtent}" />
                                                <Setter Target="SearchBoxTextBlock.Visibility" Value="Visible" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActivePointerOver_SearchBoxCustomTheme">
                                            <VisualState.Setters>
                                                <Setter Target="Icon.HorizontalAlignment" Value="Left" />
                                                <Setter Target="Icon.Margin" Value="{StaticResource SearchBoxIconMargin}" />
                                                <Setter Target="ExperienceToggleButtonRootPanel.Background">
                                                    <Setter.Value>
                                                        <NullExtension />
                                                    </Setter.Value>
                                                </Setter>
                                                <Setter Target="BackgroundElement.CornerRadius" Value="{StaticResource SearchBoxRoundCornerRadius}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource CustomThemeSearchBoxTextControlBackgroundFocused}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource CustomThemeSearchBoxTextControlBorderBrushFocused}" />
                                                <Setter Target="SearchBoxTextBlock.Foreground" Value="{ThemeResource CustomThemeSearchBoxTextControlPlaceholderForegroundFocused}" />
                                                <Setter Target="BackgroundElement.BorderThickness" Value="{ThemeResource SearchBoxTextControlBorderThemeThicknessFocused}" />
                                                <Setter Target="BackgroundElement.MinWidth" Value="{StaticResource SearchBoxButtonExtent}" />
                                                <Setter Target="SearchBoxTextBlock.Visibility" Value="Visible" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActivePressed_SearchBoxCustomTheme">
                                            <VisualState.Setters>
                                                <Setter Target="Icon.HorizontalAlignment" Value="Left" />
                                                <Setter Target="Icon.Margin" Value="{StaticResource SearchBoxIconMargin}" />
                                                <Setter Target="ExperienceToggleButtonRootPanel.Background">
                                                    <Setter.Value>
                                                        <NullExtension />
                                                    </Setter.Value>
                                                </Setter>
                                                <Setter Target="BackgroundElement.CornerRadius" Value="{StaticResource SearchBoxRoundCornerRadius}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource CustomThemeSearchBoxTextControlBackgroundFocused}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource CustomThemeSearchBoxTextControlBorderBrushFocused}" />
                                                <Setter Target="SearchBoxTextBlock.Foreground" Value="{ThemeResource CustomThemeSearchBoxTextControlPlaceholderForegroundFocused}" />
                                                <Setter Target="BackgroundElement.BorderThickness" Value="{ThemeResource SearchBoxTextControlBorderThemeThicknessFocused}" />
                                                <Setter Target="BackgroundElement.MinWidth" Value="{StaticResource SearchBoxButtonExtent}" />
                                                <Setter Target="SearchBoxTextBlock.Visibility" Value="Visible" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="InactiveNormal_SearchBox_Wave3_OnShyTaskbar">
                                            <VisualState.Setters>
                                                <Setter Target="Icon.HorizontalAlignment" Value="Left" />
                                                <Setter Target="Icon.Margin" Value="{StaticResource SearchBoxIconMargin}" />
                                                <Setter Target="ExperienceToggleButtonRootPanel.Background">
                                                    <Setter.Value>
                                                        <NullExtension />
                                                    </Setter.Value>
                                                </Setter>
                                                <Setter Target="BackgroundElement.CornerRadius" Value="{StaticResource SearchBoxOnShyTaskbarRoundCornerRadius}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource SearchBoxTextControlBackground3}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource SearchBoxTextControlBorderBrush3}" />
                                                <Setter Target="BackgroundElement.MinWidth" Value="{StaticResource SearchBoxButtonExtent}" />
                                                <Setter Target="SearchBoxTextBlock.Visibility" Value="Visible" />
                                                <Setter Target="SearchBoxTextBlock.TextWrapping" Value="NoWrap" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="InactivePointerOver_SearchBox_Wave3_OnShyTaskbar">
                                            <VisualState.Setters>
                                                <Setter Target="Icon.HorizontalAlignment" Value="Left" />
                                                <Setter Target="Icon.Margin" Value="{StaticResource SearchBoxIconMargin}" />
                                                <Setter Target="ExperienceToggleButtonRootPanel.Background">
                                                    <Setter.Value>
                                                        <NullExtension />
                                                    </Setter.Value>
                                                </Setter>
                                                <Setter Target="BackgroundElement.CornerRadius" Value="{StaticResource SearchBoxOnShyTaskbarRoundCornerRadius}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource SearchBoxTextControlBackgroundPointerOver3}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource SearchBoxTextControlBorderBrushPointerOver3}" />
                                                <Setter Target="SearchBoxTextBlock.Foreground" Value="{ThemeResource TextControlPlaceholderForegroundPointerOver}" />
                                                <Setter Target="BackgroundElement.MinWidth" Value="{StaticResource SearchBoxButtonExtent}" />
                                                <Setter Target="SearchBoxTextBlock.Visibility" Value="Visible" />
                                                <Setter Target="SearchBoxTextBlock.TextWrapping" Value="NoWrap" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="InactivePressed_SearchBox_Wave3_OnShyTaskbar">
                                            <VisualState.Setters>
                                                <Setter Target="Icon.HorizontalAlignment" Value="Left" />
                                                <Setter Target="Icon.Margin" Value="{StaticResource SearchBoxIconMargin}" />
                                                <Setter Target="ExperienceToggleButtonRootPanel.Background">
                                                    <Setter.Value>
                                                        <NullExtension />
                                                    </Setter.Value>
                                                </Setter>
                                                <Setter Target="BackgroundElement.CornerRadius" Value="{StaticResource SearchBoxOnShyTaskbarRoundCornerRadius}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource SearchBoxTextControlBackgroundFocused}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource SearchBoxTextControlBorderBrushFocused3}" />
                                                <Setter Target="SearchBoxTextBlock.Foreground" Value="{ThemeResource TextControlPlaceholderForegroundFocused}" />
                                                <Setter Target="BackgroundElement.BorderThickness" Value="{ThemeResource SearchBoxTextControlBorderThemeThicknessFocused}" />
                                                <Setter Target="BackgroundElement.MinWidth" Value="{StaticResource SearchBoxButtonExtent}" />
                                                <Setter Target="SearchBoxTextBlock.Visibility" Value="Visible" />
                                                <Setter Target="SearchBoxTextBlock.TextWrapping" Value="NoWrap" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActiveNormal_SearchBox_Wave3_OnShyTaskbar">
                                            <VisualState.Setters>
                                                <Setter Target="Icon.HorizontalAlignment" Value="Left" />
                                                <Setter Target="Icon.Margin" Value="{StaticResource SearchBoxIconMargin}" />
                                                <Setter Target="ExperienceToggleButtonRootPanel.Background">
                                                    <Setter.Value>
                                                        <NullExtension />
                                                    </Setter.Value>
                                                </Setter>
                                                <Setter Target="BackgroundElement.CornerRadius" Value="{StaticResource SearchBoxOnShyTaskbarRoundCornerRadius}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource SearchBoxTextControlBackgroundFocused}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource SearchBoxTextControlBorderBrushFocused3}" />
                                                <Setter Target="SearchBoxTextBlock.Foreground" Value="{ThemeResource TextControlPlaceholderForegroundFocused}" />
                                                <Setter Target="BackgroundElement.BorderThickness" Value="{ThemeResource SearchBoxTextControlBorderThemeThicknessFocused}" />
                                                <Setter Target="BackgroundElement.MinWidth" Value="{StaticResource SearchBoxButtonExtent}" />
                                                <Setter Target="SearchBoxTextBlock.Visibility" Value="Visible" />
                                                <Setter Target="SearchBoxTextBlock.TextWrapping" Value="NoWrap" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActivePointerOver_SearchBox_Wave3_OnShyTaskbar">
                                            <VisualState.Setters>
                                                <Setter Target="Icon.HorizontalAlignment" Value="Left" />
                                                <Setter Target="Icon.Margin" Value="{StaticResource SearchBoxIconMargin}" />
                                                <Setter Target="ExperienceToggleButtonRootPanel.Background">
                                                    <Setter.Value>
                                                        <NullExtension />
                                                    </Setter.Value>
                                                </Setter>
                                                <Setter Target="BackgroundElement.CornerRadius" Value="{StaticResource SearchBoxOnShyTaskbarRoundCornerRadius}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource SearchBoxTextControlBackgroundFocused}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource SearchBoxTextControlBorderBrushFocused3}" />
                                                <Setter Target="SearchBoxTextBlock.Foreground" Value="{ThemeResource TextControlPlaceholderForegroundFocused}" />
                                                <Setter Target="BackgroundElement.BorderThickness" Value="{ThemeResource SearchBoxTextControlBorderThemeThicknessFocused}" />
                                                <Setter Target="BackgroundElement.MinWidth" Value="{StaticResource SearchBoxButtonExtent}" />
                                                <Setter Target="SearchBoxTextBlock.Visibility" Value="Visible" />
                                                <Setter Target="SearchBoxTextBlock.TextWrapping" Value="NoWrap" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActivePressed_SearchBox_Wave3_OnShyTaskbar">
                                            <VisualState.Setters>
                                                <Setter Target="Icon.HorizontalAlignment" Value="Left" />
                                                <Setter Target="Icon.Margin" Value="{StaticResource SearchBoxIconMargin}" />
                                                <Setter Target="ExperienceToggleButtonRootPanel.Background">
                                                    <Setter.Value>
                                                        <NullExtension />
                                                    </Setter.Value>
                                                </Setter>
                                                <Setter Target="BackgroundElement.CornerRadius" Value="{StaticResource SearchBoxOnShyTaskbarRoundCornerRadius}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource SearchBoxTextControlBackgroundFocused}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource SearchBoxTextControlBorderBrushFocused3}" />
                                                <Setter Target="SearchBoxTextBlock.Foreground" Value="{ThemeResource TextControlPlaceholderForegroundFocused}" />
                                                <Setter Target="BackgroundElement.BorderThickness" Value="{ThemeResource SearchBoxTextControlBorderThemeThicknessFocused}" />
                                                <Setter Target="BackgroundElement.MinWidth" Value="{StaticResource SearchBoxButtonExtent}" />
                                                <Setter Target="SearchBoxTextBlock.Visibility" Value="Visible" />
                                                <Setter Target="SearchBoxTextBlock.TextWrapping" Value="NoWrap" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="InactiveNormal_SearchBoxCustomTheme_OnShyTaskbar">
                                            <VisualState.Setters>
                                                <Setter Target="Icon.HorizontalAlignment" Value="Left" />
                                                <Setter Target="Icon.Margin" Value="{StaticResource SearchBoxIconMargin}" />
                                                <Setter Target="ExperienceToggleButtonRootPanel.Background">
                                                    <Setter.Value>
                                                        <NullExtension />
                                                    </Setter.Value>
                                                </Setter>
                                                <Setter Target="BackgroundElement.CornerRadius" Value="{StaticResource SearchBoxOnShyTaskbarRoundCornerRadius}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource CustomThemeSearchBoxTextControlBackground}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource CustomThemeSearchBoxTextControlBorderBrush}" />
                                                <Setter Target="SearchBoxTextBlock.Foreground" Value="{ThemeResource CustomThemeSearchBoxTextControlPlaceholderForeground}" />
                                                <Setter Target="BackgroundElement.MinWidth" Value="{StaticResource SearchBoxButtonExtent}" />
                                                <Setter Target="SearchBoxTextBlock.Visibility" Value="Visible" />
                                                <Setter Target="SearchBoxTextBlock.TextWrapping" Value="NoWrap" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="InactivePointerOver_SearchBoxCustomTheme_OnShyTaskbar">
                                            <VisualState.Setters>
                                                <Setter Target="Icon.HorizontalAlignment" Value="Left" />
                                                <Setter Target="Icon.Margin" Value="{StaticResource SearchBoxIconMargin}" />
                                                <Setter Target="ExperienceToggleButtonRootPanel.Background">
                                                    <Setter.Value>
                                                        <NullExtension />
                                                    </Setter.Value>
                                                </Setter>
                                                <Setter Target="BackgroundElement.CornerRadius" Value="{StaticResource SearchBoxOnShyTaskbarRoundCornerRadius}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource CustomThemeSearchBoxTextControlBackgroundPointerOver}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource CustomThemeSearchBoxTextControlBorderBrushPointerOver}" />
                                                <Setter Target="SearchBoxTextBlock.Foreground" Value="{ThemeResource CustomThemeSearchBoxTextControlPlaceholderForegroundPointerOver}" />
                                                <Setter Target="BackgroundElement.MinWidth" Value="{StaticResource SearchBoxButtonExtent}" />
                                                <Setter Target="SearchBoxTextBlock.Visibility" Value="Visible" />
                                                <Setter Target="SearchBoxTextBlock.TextWrapping" Value="NoWrap" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="InactivePressed_SearchBoxCustomTheme_OnShyTaskbar">
                                            <VisualState.Setters>
                                                <Setter Target="Icon.HorizontalAlignment" Value="Left" />
                                                <Setter Target="Icon.Margin" Value="{StaticResource SearchBoxIconMargin}" />
                                                <Setter Target="ExperienceToggleButtonRootPanel.Background">
                                                    <Setter.Value>
                                                        <NullExtension />
                                                    </Setter.Value>
                                                </Setter>
                                                <Setter Target="BackgroundElement.CornerRadius" Value="{StaticResource SearchBoxOnShyTaskbarRoundCornerRadius}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource CustomThemeSearchBoxTextControlBackgroundFocused}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource CustomThemeSearchBoxTextControlBorderBrushFocused}" />
                                                <Setter Target="SearchBoxTextBlock.Foreground" Value="{ThemeResource CustomThemeSearchBoxTextControlPlaceholderForegroundFocused}" />
                                                <Setter Target="BackgroundElement.BorderThickness" Value="{ThemeResource SearchBoxTextControlBorderThemeThicknessFocused}" />
                                                <Setter Target="BackgroundElement.MinWidth" Value="{StaticResource SearchBoxButtonExtent}" />
                                                <Setter Target="SearchBoxTextBlock.Visibility" Value="Visible" />
                                                <Setter Target="SearchBoxTextBlock.TextWrapping" Value="NoWrap" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActiveNormal_SearchBoxCustomTheme_OnShyTaskbar">
                                            <VisualState.Setters>
                                                <Setter Target="Icon.HorizontalAlignment" Value="Left" />
                                                <Setter Target="Icon.Margin" Value="{StaticResource SearchBoxIconMargin}" />
                                                <Setter Target="ExperienceToggleButtonRootPanel.Background">
                                                    <Setter.Value>
                                                        <NullExtension />
                                                    </Setter.Value>
                                                </Setter>
                                                <Setter Target="BackgroundElement.CornerRadius" Value="{StaticResource SearchBoxOnShyTaskbarRoundCornerRadius}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource CustomThemeSearchBoxTextControlBackgroundFocused}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource CustomThemeSearchBoxTextControlBorderBrushFocused}" />
                                                <Setter Target="SearchBoxTextBlock.Foreground" Value="{ThemeResource CustomThemeSearchBoxTextControlPlaceholderForegroundFocused}" />
                                                <Setter Target="BackgroundElement.BorderThickness" Value="{ThemeResource SearchBoxTextControlBorderThemeThicknessFocused}" />
                                                <Setter Target="BackgroundElement.MinWidth" Value="{StaticResource SearchBoxButtonExtent}" />
                                                <Setter Target="SearchBoxTextBlock.Visibility" Value="Visible" />
                                                <Setter Target="SearchBoxTextBlock.TextWrapping" Value="NoWrap" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActivePointerOver_SearchBoxCustomTheme_OnShyTaskbar">
                                            <VisualState.Setters>
                                                <Setter Target="Icon.HorizontalAlignment" Value="Left" />
                                                <Setter Target="Icon.Margin" Value="{StaticResource SearchBoxIconMargin}" />
                                                <Setter Target="ExperienceToggleButtonRootPanel.Background">
                                                    <Setter.Value>
                                                        <NullExtension />
                                                    </Setter.Value>
                                                </Setter>
                                                <Setter Target="BackgroundElement.CornerRadius" Value="{StaticResource SearchBoxOnShyTaskbarRoundCornerRadius}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource CustomThemeSearchBoxTextControlBackgroundFocused}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource CustomThemeSearchBoxTextControlBorderBrushFocused}" />
                                                <Setter Target="SearchBoxTextBlock.Foreground" Value="{ThemeResource CustomThemeSearchBoxTextControlPlaceholderForegroundFocused}" />
                                                <Setter Target="BackgroundElement.BorderThickness" Value="{ThemeResource SearchBoxTextControlBorderThemeThicknessFocused}" />
                                                <Setter Target="BackgroundElement.MinWidth" Value="{StaticResource SearchBoxButtonExtent}" />
                                                <Setter Target="SearchBoxTextBlock.Visibility" Value="Visible" />
                                                <Setter Target="SearchBoxTextBlock.TextWrapping" Value="NoWrap" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActivePressed_SearchBoxCustomTheme_OnShyTaskbar">
                                            <VisualState.Setters>
                                                <Setter Target="Icon.HorizontalAlignment" Value="Left" />
                                                <Setter Target="Icon.Margin" Value="{StaticResource SearchBoxIconMargin}" />
                                                <Setter Target="ExperienceToggleButtonRootPanel.Background">
                                                    <Setter.Value>
                                                        <NullExtension />
                                                    </Setter.Value>
                                                </Setter>
                                                <Setter Target="BackgroundElement.CornerRadius" Value="{StaticResource SearchBoxOnShyTaskbarRoundCornerRadius}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource CustomThemeSearchBoxTextControlBackgroundFocused}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource CustomThemeSearchBoxTextControlBorderBrushFocused}" />
                                                <Setter Target="SearchBoxTextBlock.Foreground" Value="{ThemeResource CustomThemeSearchBoxTextControlPlaceholderForegroundFocused}" />
                                                <Setter Target="BackgroundElement.BorderThickness" Value="{ThemeResource SearchBoxTextControlBorderThemeThicknessFocused}" />
                                                <Setter Target="BackgroundElement.MinWidth" Value="{StaticResource SearchBoxButtonExtent}" />
                                                <Setter Target="SearchBoxTextBlock.Visibility" Value="Visible" />
                                                <Setter Target="SearchBoxTextBlock.TextWrapping" Value="NoWrap" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="SearchActive_SearchBoxWithDynamicGleam">
                                            <VisualState.Setters>
                                                <Setter Target="Icon.Visibility" Value="Collapsed" />
                                                <Setter Target="ExperienceToggleButtonRootPanel.Background">
                                                    <Setter.Value>
                                                        <NullExtension />
                                                    </Setter.Value>
                                                </Setter>
                                                <Setter Target="BackgroundElement.Background" Value="Transparent" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="Transparent" />
                                                <Setter Target="BackgroundElement.MinWidth" Value="{StaticResource SearchBoxButtonExtent}" />
                                                <Setter Target="SearchBoxTextBlock.Visibility" Value="Collapsed" />
                                                <Setter Target="DynamicSearchBoxGleamContainer.Visibility" Value="Collapsed" />
                                            </VisualState.Setters>
                                        </VisualState>
                                    </VisualStateGroup.States>
                                </VisualStateGroup>
                                <VisualStateGroup x:Name="SearchHighlightsStates">
                                    <VisualStateGroup.States>
                                        <VisualState x:Name="NoSearchHighlightsGleam" />
                                        <VisualState x:Name="SearchHighlightsWideGleamButton">
                                            <VisualState.Setters>
                                                <Setter Target="DynamicSearchBoxGleamContainer.Visibility" Value="Visible" />
                                                <Setter Target="GleamEntryPointButton.Visibility" Value="Visible" />
                                                <Setter Target="SearchBoxTextBlock.Margin" Value="{StaticResource SearchBoxTextWideGleamMargin}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="SearchHighlightsMediumGleamButton">
                                            <VisualState.Setters>
                                                <Setter Target="DynamicSearchBoxGleamContainer.Visibility" Value="Visible" />
                                                <Setter Target="GleamEntryPointButton.Visibility" Value="Visible" />
                                                <Setter Target="SearchBoxTextBlock.Margin" Value="{StaticResource SearchBoxTextMediumGleamMargin}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                    </VisualStateGroup.States>
                                </VisualStateGroup>
                                <VisualStateGroup x:Name="SearchHighlights_ShyTaskbarSearchBoxGleamStates">
                                    <VisualStateGroup.States>
                                        <VisualState x:Name="SearchHighlights_ShyTaskbarSearchBoxGleamNotInUse" />
                                        <VisualState x:Name="SearchHighlights_ShyTaskbarSearchBoxGleamRest">
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ObjectAnimationUsingKeyFrames TargetName="DynamicSearchBoxGleamContainer" TargetProperty="CornerRadius">
                                                            <ObjectAnimationUsingKeyFrames.KeyFrames>
                                                                <DiscreteObjectKeyFrame KeyTime="0" Value="{StaticResource SearchBoxGleamContainerShyTaskbarCornerRadius}" />
                                                            </ObjectAnimationUsingKeyFrames.KeyFrames>
                                                        </ObjectAnimationUsingKeyFrames>
                                                        <ObjectAnimationUsingKeyFrames TargetName="DynamicSearchBoxGleamContainer" TargetProperty="Background">
                                                            <ObjectAnimationUsingKeyFrames.KeyFrames>
                                                                <DiscreteObjectKeyFrame KeyTime="0" Value="{ThemeResource GleamEntryPointButtonBackground}" />
                                                            </ObjectAnimationUsingKeyFrames.KeyFrames>
                                                        </ObjectAnimationUsingKeyFrames>
                                                        <ObjectAnimationUsingKeyFrames TargetName="DynamicSearchBoxGleamContainer" TargetProperty="BorderBrush">
                                                            <ObjectAnimationUsingKeyFrames.KeyFrames>
                                                                <DiscreteObjectKeyFrame KeyTime="0" Value="{ThemeResource GleamEntryPointButtonBorderBrush}" />
                                                            </ObjectAnimationUsingKeyFrames.KeyFrames>
                                                        </ObjectAnimationUsingKeyFrames>
                                                        <ObjectAnimationUsingKeyFrames TargetName="DynamicSearchBoxGleamContainer" TargetProperty="BorderThickness">
                                                            <ObjectAnimationUsingKeyFrames.KeyFrames>
                                                                <DiscreteObjectKeyFrame KeyTime="0" Value="1" />
                                                            </ObjectAnimationUsingKeyFrames.KeyFrames>
                                                        </ObjectAnimationUsingKeyFrames>
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                        </VisualState>
                                        <VisualState x:Name="SearchHighlights_ShyTaskbarSearchBoxGleamHover">
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ObjectAnimationUsingKeyFrames TargetName="DynamicSearchBoxGleamContainer" TargetProperty="CornerRadius">
                                                            <ObjectAnimationUsingKeyFrames.KeyFrames>
                                                                <DiscreteObjectKeyFrame KeyTime="0" Value="{StaticResource SearchBoxGleamContainerShyTaskbarCornerRadius}" />
                                                            </ObjectAnimationUsingKeyFrames.KeyFrames>
                                                        </ObjectAnimationUsingKeyFrames>
                                                        <ObjectAnimationUsingKeyFrames TargetName="DynamicSearchBoxGleamContainer" TargetProperty="Background">
                                                            <ObjectAnimationUsingKeyFrames.KeyFrames>
                                                                <DiscreteObjectKeyFrame KeyTime="0" Value="{ThemeResource GleamEntryPointButtonBackgroundPointerOver}" />
                                                            </ObjectAnimationUsingKeyFrames.KeyFrames>
                                                        </ObjectAnimationUsingKeyFrames>
                                                        <ObjectAnimationUsingKeyFrames TargetName="DynamicSearchBoxGleamContainer" TargetProperty="BorderBrush">
                                                            <ObjectAnimationUsingKeyFrames.KeyFrames>
                                                                <DiscreteObjectKeyFrame KeyTime="0" Value="{ThemeResource GleamEntryPointButtonBorderBrushPointerOver}" />
                                                            </ObjectAnimationUsingKeyFrames.KeyFrames>
                                                        </ObjectAnimationUsingKeyFrames>
                                                        <ObjectAnimationUsingKeyFrames TargetName="DynamicSearchBoxGleamContainer" TargetProperty="BorderThickness">
                                                            <ObjectAnimationUsingKeyFrames.KeyFrames>
                                                                <DiscreteObjectKeyFrame KeyTime="0" Value="1" />
                                                            </ObjectAnimationUsingKeyFrames.KeyFrames>
                                                        </ObjectAnimationUsingKeyFrames>
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                        </VisualState>
                                        <VisualState x:Name="SearchHighlights_ShyTaskbarSearchBoxGleamPressed">
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ObjectAnimationUsingKeyFrames TargetName="DynamicSearchBoxGleamContainer" TargetProperty="CornerRadius">
                                                            <ObjectAnimationUsingKeyFrames.KeyFrames>
                                                                <DiscreteObjectKeyFrame KeyTime="0" Value="{StaticResource SearchBoxGleamContainerShyTaskbarCornerRadius}" />
                                                            </ObjectAnimationUsingKeyFrames.KeyFrames>
                                                        </ObjectAnimationUsingKeyFrames>
                                                        <ObjectAnimationUsingKeyFrames TargetName="DynamicSearchBoxGleamContainer" TargetProperty="Background">
                                                            <ObjectAnimationUsingKeyFrames.KeyFrames>
                                                                <DiscreteObjectKeyFrame KeyTime="0" Value="{ThemeResource GleamEntryPointButtonBackgroundPressed}" />
                                                            </ObjectAnimationUsingKeyFrames.KeyFrames>
                                                        </ObjectAnimationUsingKeyFrames>
                                                        <ObjectAnimationUsingKeyFrames TargetName="DynamicSearchBoxGleamContainer" TargetProperty="BorderBrush">
                                                            <ObjectAnimationUsingKeyFrames.KeyFrames>
                                                                <DiscreteObjectKeyFrame KeyTime="0" Value="{ThemeResource GleamEntryPointButtonBorderBrushPressed}" />
                                                            </ObjectAnimationUsingKeyFrames.KeyFrames>
                                                        </ObjectAnimationUsingKeyFrames>
                                                        <ObjectAnimationUsingKeyFrames TargetName="DynamicSearchBoxGleamContainer" TargetProperty="BorderThickness">
                                                            <ObjectAnimationUsingKeyFrames.KeyFrames>
                                                                <DiscreteObjectKeyFrame KeyTime="0" Value="1" />
                                                            </ObjectAnimationUsingKeyFrames.KeyFrames>
                                                        </ObjectAnimationUsingKeyFrames>
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                        </VisualState>
                                    </VisualStateGroup.States>
                                </VisualStateGroup>
                            </TaskListButtonPanel.VisualStateGroups>
                        </TaskListButtonPanel>
                    </ControlTemplate.Template>
                </ControlTemplate>
            </Setter.Value>
        </Setter>
    </Style>
    <CornerRadius x:Key="SearchBoxOnShyTaskbarRoundCornerRadius" Value="30" />
    <Style x:Key="SearchBoxLowerLeft" TargetType="SearchBoxButton">
        <Setter Property="HorizontalAlignment" Value="Stretch" />
        <Setter Property="VerticalAlignment" Value="Stretch" />
        <Setter Property="HorizontalContentAlignment" Value="Stretch" />
        <Setter Property="VerticalContentAlignment" Value="Stretch" />
        <Setter Property="IsTabStop" Value="True" />
        <Setter Property="UseSystemFocusVisuals" Value="True" />
        <Setter Property="CornerRadius" Value="{StaticResource ControlLargeCornerRadius}" />
        <Setter Property="Template">
            <Setter.Value>
                <ControlTemplate TargetType="SearchBoxButton">
                    <ControlTemplate.Template>
                        <TaskListButtonPanel x:Name="ExperienceToggleButtonRootPanel" Background="#00FFFFFF">
                            <TaskListButtonPanel.Children>
                                <Border x:Name="BackgroundElement"
                                    Background="{ThemeResource ShellTaskbarItemFillColorTransparentBrush}"
                                    BorderBrush="#00FFFFFF"
                                    BorderThickness="{StaticResource TaskbarButtonOutlineThickness}"
                                    CornerRadius="{TemplateBinding CornerRadius}"
                                    IsHitTestVisible="False">
                                    <Border.BackgroundTransition>
                                        <BrushTransition Duration="0:0:0.083" />
                                    </Border.BackgroundTransition>
                                </Border>
                                <AnimatedVisualPlayer x:Name="Icon"
                                    HorizontalAlignment="Center"
                                    VerticalAlignment="Center"
                                    AutoPlay="False"
                                    IsHitTestVisible="False"
                                    Stretch="Uniform" />
                                <Grid x:Name="SearchBoxContentGrid" Padding="14,0,16,0" HorizontalAlignment="Center" VerticalAlignment="Center" IsHitTestVisible="False">
                                    <Grid.ColumnDefinitions>
                                        <ColumnDefinition Width="*" />
                                        <ColumnDefinition Width="3*" />
                                    </Grid.ColumnDefinitions>
                                    <Grid.Children>
                                        <FontIcon
                                            Column="0"
                                            Margin="0,0,8,0"
                                            HorizontalAlignment="Left"
                                            VerticalAlignment="Center"
                                            FontFamily="{ThemeResource SymbolThemeFontFamily}"
                                            FontSize="16"
                                            Glyph="" />
                                        <TextBlock x:Name="SearchBoxTextBlock" Column="1" HorizontalAlignment="Center" VerticalAlignment="Center" FontSize="12" />
                                    </Grid.Children>
                                </Grid>
                            </TaskListButtonPanel.Children>
                            <TaskListButtonPanel.VisualStateGroups>
                                <VisualStateGroup x:Name="CommonStates">
                                    <VisualStateGroup.States>
                                        <VisualState x:Name="InactiveNormal">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorPrimaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorSecondaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="InactivePointerOver">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorSecondaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorSecondaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="InactivePressed">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorTertiaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemStrokeColorQuinaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActiveNormal">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorPrimaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorPrimaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActivePointerOver">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorSecondaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorSecondaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActivePressed">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorTertiaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemStrokeColorQuinaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                    </VisualStateGroup.States>
                                </VisualStateGroup>
                            </TaskListButtonPanel.VisualStateGroups>
                        </TaskListButtonPanel>
                    </ControlTemplate.Template>
                </ControlTemplate>
            </Setter.Value>
        </Setter>
    </Style>
    <DataTemplate x:Key="SearchBoxLaunchListItemTemplate">
        <DataTemplate.Template>
            <SearchBoxLaunchListButton x:Name="SearchBoxLaunchListButton" HorizontalContentAlignment="Stretch" VerticalContentAlignment="Stretch">
                <SearchBoxLaunchListButton.ToolTip>
                    <ToolTip IsEnabled="False" Placement="Top" VerticalOffset="{StaticResource ToolTipMargin}" />
                </SearchBoxLaunchListButton.ToolTip>
            </SearchBoxLaunchListButton>
        </DataTemplate.Template>
    </DataTemplate>
    <DataTemplate x:Key="SearchBoxItemTemplate">
        <DataTemplate.Template>
            <SearchBoxButton x:Name="SearchBoxButton" HorizontalContentAlignment="Stretch" VerticalContentAlignment="Stretch">
                <SearchBoxButton.ToolTip>
                    <ToolTip Placement="Top" VerticalOffset="{StaticResource ToolTipMargin}" />
                </SearchBoxButton.ToolTip>
            </SearchBoxButton>
        </DataTemplate.Template>
    </DataTemplate>
    <DataTemplate x:Key="SearchBoxItemOverflowTemplate">
        <DataTemplate.Template>
            <SearchBoxButton x:Name="SearchBoxButton" HorizontalContentAlignment="Stretch" VerticalContentAlignment="Stretch" IsHoverInvokeEnabled="False">
                <SearchBoxButton.ToolTip>
                    <ToolTip Placement="Top" VerticalOffset="{StaticResource ToolTipMargin}" />
                </SearchBoxButton.ToolTip>
            </SearchBoxButton>
        </DataTemplate.Template>
    </DataTemplate>
    <Thickness x:Key="SearchBoxIconMargin" Value="11,0,0,0" />
    <CornerRadius x:Key="SearchBoxGleamContainerShyTaskbarCornerRadius" Value="20" />
    <Thickness x:Key="SearchBoxGleamContainerMargin" Value="0,2,2,2" />
    <Double x:Key="SearchBoxGleamContainerHeight" Value="28" />
    <CornerRadius x:Key="SearchBoxGleamContainerCornerRadius" Value="0,14,14,0" />
    <Thickness x:Key="SearchBoxButtonNormalPadding" Value="0,8" />
    <Thickness x:Key="SearchBoxButtonMargin" Value="2,0" />
    <Thickness x:Key="SearchBoxButtonLowerLeftMargin" Value="0" />
    <CornerRadius x:Key="SearchBoxButtonLeftAlignedCornerRadius" Value="4,4,4,4" />
    <Thickness x:Key="SearchBoxButtonHitTestableMargin" Value="12,4,0,4" />
    <Double x:Key="SearchBoxButtonExtent" Value="220" />
    <SolidColorBrush x:Key="SearchBoxButtonBorder" Color="{ThemeResource ShellTaskbarItemStrokeColorTertiaryBrush}" />
    <SolidColorBrush x:Key="SearchBoxButtonBackground" Color="{ThemeResource ShellTaskbarItemFillColorPrimaryBrush}" />
    <DataTemplate x:Key="OverflowItemTemplate">
        <DataTemplate.Template>
            <OverflowToggleButton x:Name="OverflowButton" AutomationId="OverflowButton" HorizontalContentAlignment="Stretch" VerticalContentAlignment="Stretch">
                <OverflowToggleButton.ToolTip>
                    <ToolTip Placement="Top" VerticalOffset="{StaticResource ToolTipMargin}" />
                </OverflowToggleButton.ToolTip>
            </OverflowToggleButton>
        </DataTemplate.Template>
    </DataTemplate>
    <Style x:Key="OverflowHorizontalScrollViewerStyle" TargetType="ScrollViewer">
        <Setter Property="HorizontalScrollMode" Value="Disabled" />
        <Setter Property="VerticalScrollMode" Value="Disabled" />
        <Setter Property="IsHorizontalRailEnabled" Value="True" />
        <Setter Property="IsVerticalRailEnabled" Value="False" />
        <Setter Property="IsTabStop" Value="False" />
        <Setter Property="ZoomMode" Value="Disabled" />
        <Setter Property="HorizontalContentAlignment" Value="Left" />
        <Setter Property="VerticalContentAlignment" Value="Top" />
        <Setter Property="VerticalScrollBarVisibility" Value="Disabled" />
        <Setter Property="HorizontalScrollBarVisibility" Value="Hidden" />
        <Setter Property="HorizontalSnapPointsType" Value="Mandatory" />
        <Setter Property="HorizontalSnapPointsAlignment" Value="Far" />
        <Setter Property="UseSystemFocusVisuals" Value="{StaticResource UseSystemFocusVisuals}" />
        <Setter Property="Template">
            <Setter.Value>
                <ControlTemplate TargetType="ScrollViewer">
                    <ControlTemplate.Template>
                        <Border x:Name="Root">
                            <Border.Child>
                                <Grid>
                                    <Grid.ColumnDefinitions>
                                        <ColumnDefinition Width="Auto" />
                                        <ColumnDefinition Width="*" />
                                        <ColumnDefinition Width="Auto" />
                                    </Grid.ColumnDefinitions>
                                    <Grid.Children>
                                        <RepeatButton x:Name="HorizontalDecreaseButton"
                                            Column="0"
                                            IsTabStop="False"
                                            Interval="50"
                                            Template="{StaticResource HorizontalDecrementTemplate}"
                                            AllowFocusOnInteraction="False"
                                            VerticalAlignment="Center"
                                            Visibility="Collapsed"
                                            AccessibilityView="Raw" />
                                        <Grid ColumnSpan="2" Visibility="{TemplateBinding ComputedHorizontalScrollBarVisibility}" Padding="{ThemeResource ScrollViewerScrollBarMargin}">
                                            <Grid.Children>
                                                <ScrollBar x:Name="HorizontalScrollBar"
                                                    IsTabStop="False"
                                                    Maximum="{TemplateBinding ScrollableWidth}"
                                                    Orientation="Horizontal"
                                                    Value="{TemplateBinding HorizontalOffset}"
                                                    ViewportSize="{TemplateBinding ViewportWidth}" />
                                            </Grid.Children>
                                        </Grid>
                                        <ScrollContentPresenter x:Name="ScrollContentPresenter" Column="1" ContentTemplate="{TemplateBinding ContentTemplate}" />
                                        <RepeatButton x:Name="HorizontalIncreaseButton"
                                            Column="2"
                                            IsTabStop="False"
                                            Interval="50"
                                            Template="{StaticResource HorizontalIncrementTemplate}"
                                            AllowFocusOnInteraction="False"
                                            VerticalAlignment="Center"
                                            Visibility="Collapsed"
                                            AccessibilityView="Raw" />
                                    </Grid.Children>
                                </Grid>
                            </Border.Child>
                            <Border.VisualStateGroups>
                                <VisualStateGroup x:Name="ScrollingIndicatorStates">
                                    <VisualStateGroup.Transitions>
                                        <VisualTransition From="MouseIndicator" To="NoIndicator">
                                            <VisualTransition.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ObjectAnimationUsingKeyFrames TargetName="HorizontalScrollBar" TargetProperty="IndicatorMode">
                                                            <ObjectAnimationUsingKeyFrames.KeyFrames>
                                                                <DiscreteObjectKeyFrame KeyTime="{ThemeResource ScrollViewerSeparatorContractDelay}">
                                                                    <DiscreteObjectKeyFrame.Value>
                                                                        <ScrollingIndicatorMode Value="None" />
                                                                    </DiscreteObjectKeyFrame.Value>
                                                                </DiscreteObjectKeyFrame>
                                                            </ObjectAnimationUsingKeyFrames.KeyFrames>
                                                        </ObjectAnimationUsingKeyFrames>
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualTransition.Storyboard>
                                        </VisualTransition>
                                        <VisualTransition From="MouseIndicatorFull" To="NoIndicator">
                                            <VisualTransition.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ObjectAnimationUsingKeyFrames TargetName="HorizontalScrollBar" TargetProperty="IndicatorMode">
                                                            <ObjectAnimationUsingKeyFrames.KeyFrames>
                                                                <DiscreteObjectKeyFrame KeyTime="{ThemeResource ScrollViewerSeparatorContractDelay}">
                                                                    <DiscreteObjectKeyFrame.Value>
                                                                        <ScrollingIndicatorMode Value="None" />
                                                                    </DiscreteObjectKeyFrame.Value>
                                                                </DiscreteObjectKeyFrame>
                                                            </ObjectAnimationUsingKeyFrames.KeyFrames>
                                                        </ObjectAnimationUsingKeyFrames>
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualTransition.Storyboard>
                                        </VisualTransition>
                                        <VisualTransition From="MouseIndicatorFull" To="MouseIndicator">
                                            <VisualTransition.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ObjectAnimationUsingKeyFrames TargetName="HorizontalScrollBar" TargetProperty="IndicatorMode">
                                                            <ObjectAnimationUsingKeyFrames.KeyFrames>
                                                                <DiscreteObjectKeyFrame KeyTime="{ThemeResource ScrollViewerSeparatorContractDelay}">
                                                                    <DiscreteObjectKeyFrame.Value>
                                                                        <ScrollingIndicatorMode Value="MouseIndicator" />
                                                                    </DiscreteObjectKeyFrame.Value>
                                                                </DiscreteObjectKeyFrame>
                                                            </ObjectAnimationUsingKeyFrames.KeyFrames>
                                                        </ObjectAnimationUsingKeyFrames>
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualTransition.Storyboard>
                                        </VisualTransition>
                                        <VisualTransition From="TouchIndicator" To="NoIndicator">
                                            <VisualTransition.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ObjectAnimationUsingKeyFrames TargetName="HorizontalScrollBar" TargetProperty="IndicatorMode">
                                                            <ObjectAnimationUsingKeyFrames.KeyFrames>
                                                                <DiscreteObjectKeyFrame KeyTime="0.5">
                                                                    <DiscreteObjectKeyFrame.Value>
                                                                        <ScrollingIndicatorMode Value="None" />
                                                                    </DiscreteObjectKeyFrame.Value>
                                                                </DiscreteObjectKeyFrame>
                                                            </ObjectAnimationUsingKeyFrames.KeyFrames>
                                                        </ObjectAnimationUsingKeyFrames>
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualTransition.Storyboard>
                                        </VisualTransition>
                                    </VisualStateGroup.Transitions>
                                    <VisualStateGroup.States>
                                        <VisualState x:Name="NoIndicator" />
                                        <VisualState x:Name="TouchIndicator">
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ObjectAnimationUsingKeyFrames TargetName="HorizontalScrollBar" TargetProperty="IndicatorMode">
                                                            <ObjectAnimationUsingKeyFrames.KeyFrames>
                                                                <DiscreteObjectKeyFrame KeyTime="0">
                                                                    <DiscreteObjectKeyFrame.Value>
                                                                        <ScrollingIndicatorMode Value="TouchIndicator" />
                                                                    </DiscreteObjectKeyFrame.Value>
                                                                </DiscreteObjectKeyFrame>
                                                            </ObjectAnimationUsingKeyFrames.KeyFrames>
                                                        </ObjectAnimationUsingKeyFrames>
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                        </VisualState>
                                        <VisualState x:Name="MouseIndicator">
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ObjectAnimationUsingKeyFrames TargetName="HorizontalScrollBar" TargetProperty="IndicatorMode">
                                                            <ObjectAnimationUsingKeyFrames.KeyFrames>
                                                                <DiscreteObjectKeyFrame KeyTime="0">
                                                                    <DiscreteObjectKeyFrame.Value>
                                                                        <ScrollingIndicatorMode Value="MouseIndicator" />
                                                                    </DiscreteObjectKeyFrame.Value>
                                                                </DiscreteObjectKeyFrame>
                                                            </ObjectAnimationUsingKeyFrames.KeyFrames>
                                                        </ObjectAnimationUsingKeyFrames>
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                        </VisualState>
                                        <VisualState x:Name="MouseIndicatorFull">
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ObjectAnimationUsingKeyFrames TargetName="HorizontalScrollBar" TargetProperty="IndicatorMode">
                                                            <ObjectAnimationUsingKeyFrames.KeyFrames>
                                                                <DiscreteObjectKeyFrame KeyTime="0">
                                                                    <DiscreteObjectKeyFrame.Value>
                                                                        <ScrollingIndicatorMode Value="MouseIndicator" />
                                                                    </DiscreteObjectKeyFrame.Value>
                                                                </DiscreteObjectKeyFrame>
                                                            </ObjectAnimationUsingKeyFrames.KeyFrames>
                                                        </ObjectAnimationUsingKeyFrames>
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                        </VisualState>
                                    </VisualStateGroup.States>
                                </VisualStateGroup>
                                <VisualStateGroup x:Name="ScrollPositionStates">
                                    <VisualStateGroup.States>
                                        <VisualState x:Name="NoScroll" />
                                        <VisualState x:Name="ScrollLeftOnly">
                                            <VisualState.Setters>
                                                <Setter Target="HorizontalDecreaseButton.Visibility" Value="Visible" />
                                                <Setter Target="HorizontalIncreaseButton.Visibility" Value="Visible" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ScrollRightOnly">
                                            <VisualState.Setters>
                                                <Setter Target="HorizontalDecreaseButton.Visibility" Value="Visible" />
                                                <Setter Target="HorizontalIncreaseButton.Visibility" Value="Visible" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ScrollLeftAndRight">
                                            <VisualState.Setters>
                                                <Setter Target="HorizontalDecreaseButton.Visibility" Value="Visible" />
                                                <Setter Target="HorizontalIncreaseButton.Visibility" Value="Visible" />
                                            </VisualState.Setters>
                                        </VisualState>
                                    </VisualStateGroup.States>
                                </VisualStateGroup>
                            </Border.VisualStateGroups>
                        </Border>
                    </ControlTemplate.Template>
                </ControlTemplate>
            </Setter.Value>
        </Setter>
    </Style>
    <DataTemplate x:Key="MeetNowItemTemplate">
        <DataTemplate.Template>
            <ExperienceToggleButton x:Name="MeetNowButton" HorizontalContentAlignment="Stretch" VerticalContentAlignment="Stretch">
                <ExperienceToggleButton.ToolTip>
                    <ToolTip Placement="Top" VerticalOffset="{StaticResource ToolTipMargin}" />
                </ExperienceToggleButton.ToolTip>
            </ExperienceToggleButton>
        </DataTemplate.Template>
    </DataTemplate>
    <DataTemplate x:Key="MeetNowItemOverflowTemplate">
        <DataTemplate.Template>
            <ExperienceToggleButton x:Name="MeetNowButton" HorizontalContentAlignment="Stretch" VerticalContentAlignment="Stretch" IsHoverInvokeEnabled="False">
                <ExperienceToggleButton.ToolTip>
                    <ToolTip Placement="Top" VerticalOffset="{StaticResource ToolTipMargin}" />
                </ExperienceToggleButton.ToolTip>
            </ExperienceToggleButton>
        </DataTemplate.Template>
    </DataTemplate>
    <Double x:Key="MediumTaskbarLabeledButtonMinWidth" Value="84" />
    <Double x:Key="MediumTaskbarLabeledButtonMaxWidth" Value="180" />
    <Double x:Key="MediumTaskbarButtonTabletPostureExtent" Value="52" />
    <Double x:Key="MediumTaskbarButtonShyTabletPostureExtent" Value="66" />
    <Double x:Key="MediumTaskbarButtonExtent" Value="44" />
    <DataTemplate x:Key="LaunchListItemTemplate">
        <DataTemplate.Template>
            <ExperienceToggleButton x:Name="LaunchListButton" HorizontalContentAlignment="Stretch" VerticalContentAlignment="Stretch">
                <ExperienceToggleButton.ToolTip>
                    <ToolTip Placement="Top" VerticalOffset="{StaticResource ToolTipMargin}" />
                </ExperienceToggleButton.ToolTip>
            </ExperienceToggleButton>
        </DataTemplate.Template>
    </DataTemplate>
    <DataTemplate x:Key="LaunchListItemOverflowTemplate">
        <DataTemplate.Template>
            <ExperienceToggleButton x:Name="LaunchListButton" HorizontalContentAlignment="Stretch" VerticalContentAlignment="Stretch" IsHoverInvokeEnabled="False">
                <ExperienceToggleButton.ToolTip>
                    <ToolTip Placement="Top" VerticalOffset="{StaticResource ToolTipMargin}" />
                </ExperienceToggleButton.ToolTip>
            </ExperienceToggleButton>
        </DataTemplate.Template>
    </DataTemplate>
    <Double x:Key="LargeNumericBadgeFontSize" Value="13" />
    <Double x:Key="LargeGlyphBadgeFontSize" Value="14" />
    <ControlTemplate x:Key="HorizontalIncrementTemplate" TargetType="RepeatButton">
        <ControlTemplate.Template>
            <Grid x:Name="Root" Background="{ThemeResource ShellTaskbarItemFillColorTransparentBrush}" BorderBrush="#00FFFFFF" BorderThickness="{StaticResource TaskbarButtonOutlineThickness}" CornerRadius="{StaticResource ControlLargeCornerRadius}">
                <Grid.BackgroundTransition>
                    <BrushTransition Duration="0:0:0.083" />
                </Grid.BackgroundTransition>
                <Grid.Children>
                    <FontIcon x:Name="Arrow"
                        RenderTransformOrigin="0.5, 0.5"
                        FontFamily="{ThemeResource SymbolThemeFontFamily}"
                        Glyph=""
                        Foreground="{ThemeResource ScrollBarButtonArrowForeground}"
                        FontSize="{StaticResource ScrollBarButtonArrowIconFontSize}"
                        MirroredWhenRightToLeft="True">
                        <FontIcon.RenderTransform>
                            <ScaleTransform x:Name="ScaleTransform" ScaleY="1" ScaleX="1" />
                        </FontIcon.RenderTransform>
                    </FontIcon>
                </Grid.Children>
                <Grid.VisualStateGroups>
                    <VisualStateGroup x:Name="CommonStates">
                        <VisualStateGroup.States>
                            <VisualState x:Name="Normal" />
                            <VisualState x:Name="PointerOver">
                                <VisualState.Setters>
                                    <Setter Target="Root.Background" Value="{ThemeResource ShellTaskbarItemFillColorAltSecondaryBrush}" />
                                    <Setter Target="Root.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorSecondaryBrush}" />
                                    <Setter Target="Arrow.Foreground" Value="{ThemeResource ScrollBarButtonArrowForegroundPointerOver}" />
                                </VisualState.Setters>
                            </VisualState>
                            <VisualState x:Name="Pressed">
                                <VisualState.Setters>
                                    <Setter Target="Root.Background" Value="{ThemeResource ShellTaskbarItemFillColorTertiaryBrush}" />
                                    <Setter Target="Root.BorderBrush" Value="{ThemeResource ShellTaskbarItemStrokeColorQuinaryBrush}" />
                                    <Setter Target="Arrow.Foreground" Value="{ThemeResource ScrollBarButtonArrowForegroundPressed}" />
                                </VisualState.Setters>
                            </VisualState>
                            <VisualState x:Name="Disabled">
                                <VisualState.Setters>
                                    <Setter Target="Arrow.Foreground" Value="{ThemeResource ScrollBarButtonArrowForegroundDisabled}" />
                                </VisualState.Setters>
                            </VisualState>
                        </VisualStateGroup.States>
                    </VisualStateGroup>
                </Grid.VisualStateGroups>
            </Grid>
        </ControlTemplate.Template>
    </ControlTemplate>
    <ControlTemplate x:Key="HorizontalDecrementTemplate" TargetType="RepeatButton">
        <ControlTemplate.Template>
            <Grid x:Name="Root" Background="{ThemeResource ShellTaskbarItemFillColorTransparentBrush}" BorderBrush="#00FFFFFF" BorderThickness="{StaticResource TaskbarButtonOutlineThickness}" CornerRadius="{StaticResource ControlLargeCornerRadius}">
                <Grid.BackgroundTransition>
                    <BrushTransition Duration="0:0:0.083" />
                </Grid.BackgroundTransition>
                <Grid.Children>
                    <FontIcon x:Name="Arrow"
                        RenderTransformOrigin="0.5, 0.5"
                        FontFamily="{ThemeResource SymbolThemeFontFamily}"
                        Glyph=""
                        Foreground="{ThemeResource ScrollBarButtonArrowForeground}"
                        FontSize="{StaticResource ScrollBarButtonArrowIconFontSize}"
                        MirroredWhenRightToLeft="True">
                        <FontIcon.RenderTransform>
                            <ScaleTransform x:Name="ScaleTransform" ScaleY="1" ScaleX="1" />
                        </FontIcon.RenderTransform>
                    </FontIcon>
                </Grid.Children>
                <Grid.VisualStateGroups>
                    <VisualStateGroup x:Name="CommonStates">
                        <VisualStateGroup.States>
                            <VisualState x:Name="Normal" />
                            <VisualState x:Name="PointerOver">
                                <VisualState.Setters>
                                    <Setter Target="Root.Background" Value="{ThemeResource ShellTaskbarItemFillColorAltSecondaryBrush}" />
                                    <Setter Target="Root.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorSecondaryBrush}" />
                                    <Setter Target="Arrow.Foreground" Value="{ThemeResource ScrollBarButtonArrowForegroundPointerOver}" />
                                </VisualState.Setters>
                            </VisualState>
                            <VisualState x:Name="Pressed">
                                <VisualState.Setters>
                                    <Setter Target="Root.Background" Value="{ThemeResource ShellTaskbarItemFillColorTertiaryBrush}" />
                                    <Setter Target="Root.BorderBrush" Value="{ThemeResource ShellTaskbarItemStrokeColorQuinaryBrush}" />
                                    <Setter Target="Arrow.Foreground" Value="{ThemeResource ScrollBarButtonArrowForegroundPressed}" />
                                </VisualState.Setters>
                            </VisualState>
                            <VisualState x:Name="Disabled">
                                <VisualState.Setters>
                                    <Setter Target="Arrow.Foreground" Value="{ThemeResource ScrollBarButtonArrowForegroundDisabled}" />
                                </VisualState.Setters>
                            </VisualState>
                        </VisualStateGroup.States>
                    </VisualStateGroup>
                </Grid.VisualStateGroups>
            </Grid>
        </ControlTemplate.Template>
    </ControlTemplate>
    <Double x:Key="GripperPortraitWidth" Value="96" />
    <Double x:Key="GripperLandscapeWidth" Value="120" />
    <Style x:Key="GleamEntryPointButtonStyle" TargetType="Button">
        <Setter Property="Background" Value="{ThemeResource GleamEntryPointButtonBackground}" />
        <Setter Property="BackgroundSizing" Value="InnerBorderEdge" />
        <Setter Property="BorderBrush" Value="{ThemeResource GleamEntryPointButtonBorderBrush}" />
        <Setter Property="BorderThickness" Value="{ThemeResource GleamEntryPointButtonBorderThemeThickness}" />
        <Setter Property="Padding" Value="0,2,2,2" />
        <Setter Property="HorizontalAlignment" Value="Stretch" />
        <Setter Property="VerticalAlignment" Value="Stretch" />
        <Setter Property="UseSystemFocusVisuals" Value="True" />
        <Setter Property="FocusVisualMargin" Value="-4,-2,-2,-2" />
        <Setter Property="FocusVisualSecondaryThickness" Value="0" />
        <Setter Property="CornerRadius" Value="16" />
        <Setter Property="Template">
            <Setter.Value>
                <ControlTemplate TargetType="Button">
                    <ControlTemplate.Template>
                        <Border CornerRadius="0,16,16,0" Background="#00FFFFFF">
                            <Border.VisualStateGroups>
                                <VisualStateGroup x:Name="CommonStates">
                                    <VisualStateGroup.States>
                                        <VisualState x:Name="Normal" />
                                        <VisualState x:Name="PointerOver">
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ObjectAnimationUsingKeyFrames TargetName="ContentPresenter" TargetProperty="Background">
                                                            <ObjectAnimationUsingKeyFrames.KeyFrames>
                                                                <DiscreteObjectKeyFrame KeyTime="0" Value="{ThemeResource GleamEntryPointButtonBackgroundPointerOver}" />
                                                            </ObjectAnimationUsingKeyFrames.KeyFrames>
                                                        </ObjectAnimationUsingKeyFrames>
                                                        <ObjectAnimationUsingKeyFrames TargetName="ContentPresenter" TargetProperty="BorderBrush">
                                                            <ObjectAnimationUsingKeyFrames.KeyFrames>
                                                                <DiscreteObjectKeyFrame KeyTime="0" Value="{ThemeResource GleamEntryPointButtonBorderBrushPointerOver}" />
                                                            </ObjectAnimationUsingKeyFrames.KeyFrames>
                                                        </ObjectAnimationUsingKeyFrames>
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                        </VisualState>
                                        <VisualState x:Name="Pressed">
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ObjectAnimationUsingKeyFrames TargetName="ContentPresenter" TargetProperty="Background">
                                                            <ObjectAnimationUsingKeyFrames.KeyFrames>
                                                                <DiscreteObjectKeyFrame KeyTime="0" Value="{ThemeResource GleamEntryPointButtonBackgroundPressed}" />
                                                            </ObjectAnimationUsingKeyFrames.KeyFrames>
                                                        </ObjectAnimationUsingKeyFrames>
                                                        <ObjectAnimationUsingKeyFrames TargetName="ContentPresenter" TargetProperty="BorderBrush">
                                                            <ObjectAnimationUsingKeyFrames.KeyFrames>
                                                                <DiscreteObjectKeyFrame KeyTime="0" Value="{ThemeResource GleamEntryPointButtonBorderBrushPressed}" />
                                                            </ObjectAnimationUsingKeyFrames.KeyFrames>
                                                        </ObjectAnimationUsingKeyFrames>
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                        </VisualState>
                                    </VisualStateGroup.States>
                                </VisualStateGroup>
                            </Border.VisualStateGroups>
                            <Border.Child>
                                <ContentPresenter x:Name="ContentPresenter"
                                    Background="{TemplateBinding Background}"
                                    BackgroundSizing="{TemplateBinding BackgroundSizing}"
                                    BorderBrush="{TemplateBinding BorderBrush}"
                                    BorderThickness="{TemplateBinding BorderThickness}"
                                    Content="{TemplateBinding Content}"
                                    ContentTemplate="{TemplateBinding ContentTemplate}"
                                    ContentTransitions="{TemplateBinding ContentTransitions}"
                                    CornerRadius="14"
                                    Margin="{TemplateBinding Padding}"
                                    AccessibilityView="Raw">
                                    <ContentPresenter.BackgroundTransition>
                                        <BrushTransition Duration="0:0:0.083" />
                                    </ContentPresenter.BackgroundTransition>
                                </ContentPresenter>
                            </Border.Child>
                        </Border>
                    </ControlTemplate.Template>
                </ControlTemplate>
            </Setter.Value>
        </Setter>
    </Style>
    <Style x:Key="GleamEntryPointButtonOnShyTaskbarStyle" TargetType="Button">
        <Setter Property="Background" Value="{ThemeResource GleamEntryPointButtonBackground}" />
        <Setter Property="BackgroundSizing" Value="InnerBorderEdge" />
        <Setter Property="BorderBrush" Value="{ThemeResource GleamEntryPointButtonBorderBrush}" />
        <Setter Property="BorderThickness" Value="{ThemeResource GleamEntryPointButtonBorderThemeThickness}" />
        <Setter Property="Padding" Value="0,2,2,2" />
        <Setter Property="HorizontalAlignment" Value="Stretch" />
        <Setter Property="VerticalAlignment" Value="Stretch" />
        <Setter Property="UseSystemFocusVisuals" Value="True" />
        <Setter Property="FocusVisualMargin" Value="-4,-2,-2,-2" />
        <Setter Property="FocusVisualSecondaryThickness" Value="0" />
        <Setter Property="CornerRadius" Value="30" />
        <Setter Property="Template">
            <Setter.Value>
                <ControlTemplate TargetType="Button">
                    <ControlTemplate.Template>
                        <Border CornerRadius="0,30,30,0" Background="#00FFFFFF">
                            <Border.VisualStateGroups>
                                <VisualStateGroup x:Name="CommonStates">
                                    <VisualStateGroup.States>
                                        <VisualState x:Name="Normal" />
                                        <VisualState x:Name="PointerOver">
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ObjectAnimationUsingKeyFrames TargetName="ContentPresenter" TargetProperty="Background">
                                                            <ObjectAnimationUsingKeyFrames.KeyFrames>
                                                                <DiscreteObjectKeyFrame KeyTime="0" Value="{ThemeResource GleamEntryPointButtonBackgroundPointerOver}" />
                                                            </ObjectAnimationUsingKeyFrames.KeyFrames>
                                                        </ObjectAnimationUsingKeyFrames>
                                                        <ObjectAnimationUsingKeyFrames TargetName="ContentPresenter" TargetProperty="BorderBrush">
                                                            <ObjectAnimationUsingKeyFrames.KeyFrames>
                                                                <DiscreteObjectKeyFrame KeyTime="0" Value="{ThemeResource GleamEntryPointButtonBorderBrushPointerOver}" />
                                                            </ObjectAnimationUsingKeyFrames.KeyFrames>
                                                        </ObjectAnimationUsingKeyFrames>
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                        </VisualState>
                                        <VisualState x:Name="Pressed">
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ObjectAnimationUsingKeyFrames TargetName="ContentPresenter" TargetProperty="Background">
                                                            <ObjectAnimationUsingKeyFrames.KeyFrames>
                                                                <DiscreteObjectKeyFrame KeyTime="0" Value="{ThemeResource GleamEntryPointButtonBackgroundPressed}" />
                                                            </ObjectAnimationUsingKeyFrames.KeyFrames>
                                                        </ObjectAnimationUsingKeyFrames>
                                                        <ObjectAnimationUsingKeyFrames TargetName="ContentPresenter" TargetProperty="BorderBrush">
                                                            <ObjectAnimationUsingKeyFrames.KeyFrames>
                                                                <DiscreteObjectKeyFrame KeyTime="0" Value="{ThemeResource GleamEntryPointButtonBorderBrushPressed}" />
                                                            </ObjectAnimationUsingKeyFrames.KeyFrames>
                                                        </ObjectAnimationUsingKeyFrames>
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                        </VisualState>
                                    </VisualStateGroup.States>
                                </VisualStateGroup>
                            </Border.VisualStateGroups>
                            <Border.Child>
                                <ContentPresenter x:Name="ContentPresenter"
                                    Background="{TemplateBinding Background}"
                                    BackgroundSizing="{TemplateBinding BackgroundSizing}"
                                    BorderBrush="{TemplateBinding BorderBrush}"
                                    BorderThickness="{TemplateBinding BorderThickness}"
                                    Content="{TemplateBinding Content}"
                                    ContentTemplate="{TemplateBinding ContentTemplate}"
                                    ContentTransitions="{TemplateBinding ContentTransitions}"
                                    CornerRadius="28"
                                    Margin="{TemplateBinding Padding}"
                                    AccessibilityView="Raw">
                                    <ContentPresenter.BackgroundTransition>
                                        <BrushTransition Duration="0:0:0.083" />
                                    </ContentPresenter.BackgroundTransition>
                                </ContentPresenter>
                            </Border.Child>
                        </Border>
                    </ControlTemplate.Template>
                </ControlTemplate>
            </Setter.Value>
        </Setter>
    </Style>
    <Style x:Key="GleamEntryPointButtonOnShyTaskbarRedirectedBackplateStyle" BasedOn="{StaticResource GleamEntryPointButtonOnShyTaskbarStyle}" TargetType="Button">
        <Setter Property="Template">
            <Setter.Value>
                <ControlTemplate TargetType="Button">
                    <ControlTemplate.Template>
                        <Border CornerRadius="0,30,30,0" Background="#00FFFFFF">
                            <Border.Child>
                                <ContentPresenter x:Name="ContentPresenter"
                                    Background="#00FFFFFF"
                                    BackgroundSizing="{TemplateBinding BackgroundSizing}"
                                    BorderBrush="#00FFFFFF"
                                    BorderThickness="{TemplateBinding BorderThickness}"
                                    Content="{TemplateBinding Content}"
                                    ContentTemplate="{TemplateBinding ContentTemplate}"
                                    ContentTransitions="{TemplateBinding ContentTransitions}"
                                    CornerRadius="28"
                                    Margin="{TemplateBinding Padding}"
                                    AccessibilityView="Raw">
                                    <ContentPresenter.BackgroundTransition>
                                        <BrushTransition Duration="0:0:0.083" />
                                    </ContentPresenter.BackgroundTransition>
                                </ContentPresenter>
                            </Border.Child>
                        </Border>
                    </ControlTemplate.Template>
                </ControlTemplate>
            </Setter.Value>
        </Setter>
    </Style>
    <Double x:Key="GleamButtonToolTipMargin" Value="20" />
    <DataTemplate x:Key="ExtendedUIRecommendationTemplate">
        <DataTemplate.Template>
            <Button
                HorizontalAlignment="Stretch"
                HorizontalContentAlignment="Left"
                Margin="0,0,0,3"
                BorderThickness="0"
                Padding="11,8,11,9">
                <Button.Resources>
                    <ResourceDictionary>
                        <ResourceDictionary.ThemeDictionaries>
                            <ResourceDictionary x:Key="Light">
                                <StaticResource x:Key="ButtonForeground" ResourceKey="ExtendedUIRecommendationForeground" />
                                <StaticResource x:Key="ButtonForegroundPressed" ResourceKey="ExtendedUIRecommendationForegroundPressed" />
                                <StaticResource x:Key="ButtonForegroundPointerOver" ResourceKey="ExtendedUIRecommendationForegroundPointerOver" />
                                <StaticResource x:Key="ButtonBackground" ResourceKey="ExtendedUIRecommendationBackground" />
                                <StaticResource x:Key="ButtonBackgroundPressed" ResourceKey="ExtendedUIRecommendationBackgroundPressed" />
                                <StaticResource x:Key="ButtonBackgroundPointerOver" ResourceKey="ExtendedUIRecommendationBackgroundPointerOver" />
                            </ResourceDictionary>
                            <ResourceDictionary x:Key="Dark">
                                <StaticResource x:Key="ButtonForeground" ResourceKey="ExtendedUIRecommendationForeground" />
                                <StaticResource x:Key="ButtonForegroundPressed" ResourceKey="ExtendedUIRecommendationForegroundPressed" />
                                <StaticResource x:Key="ButtonForegroundPointerOver" ResourceKey="ExtendedUIRecommendationForegroundPointerOver" />
                                <StaticResource x:Key="ButtonBackground" ResourceKey="ExtendedUIRecommendationBackground" />
                                <StaticResource x:Key="ButtonBackgroundPressed" ResourceKey="ExtendedUIRecommendationBackgroundPressed" />
                                <StaticResource x:Key="ButtonBackgroundPointerOver" ResourceKey="ExtendedUIRecommendationBackgroundPointerOver" />
                            </ResourceDictionary>
                            <ResourceDictionary x:Key="HighContrast">
                                <StaticResource x:Key="ButtonForeground" ResourceKey="ExtendedUIRecommendationForeground" />
                                <StaticResource x:Key="ButtonForegroundPressed" ResourceKey="ExtendedUIRecommendationForegroundPressed" />
                                <StaticResource x:Key="ButtonForegroundPointerOver" ResourceKey="ExtendedUIRecommendationForegroundPointerOver" />
                                <StaticResource x:Key="ButtonBackground" ResourceKey="ExtendedUIRecommendationBackground" />
                                <StaticResource x:Key="ButtonBackgroundPressed" ResourceKey="ExtendedUIRecommendationBackgroundPressed" />
                                <StaticResource x:Key="ButtonBackgroundPointerOver" ResourceKey="ExtendedUIRecommendationBackgroundPointerOver" />
                            </ResourceDictionary>
                        </ResourceDictionary.ThemeDictionaries>
                    </ResourceDictionary>
                </Button.Resources>
                <Button.Content>
                    <StackPanel Orientation="Horizontal">
                        <StackPanel.Children>
                            <Image x:Name="RecommendationIcon" Width="16" Height="16" VerticalAlignment="Center" />
                            <TextBlock x:Name="RecommendationTextBlock" FontFamily="Segoe UI Variable" FontSize="14" Margin="12,0,0,0" />
                        </StackPanel.Children>
                    </StackPanel>
                </Button.Content>
            </Button>
        </DataTemplate.Template>
    </DataTemplate>
    <CornerRadius x:Key="ControlLargeCornerRadius" Value="4" />
    <Double x:Key="ControlLargeCornerRadiusValue" Value="4" />
    <BrushTransition x:Key="BackgroundElementBrushTransition" Duration="0:0:0.083" />
    <Style x:Key="AugmentedEntryPointTextStyle" TargetType="TextBlock">
        <Setter Property="IsTextScaleFactorEnabled" Value="False" />
    </Style>
    <Style x:Key="AugmentedEntryPointShyTabletOverlayStyle" TargetType="Grid">
        <Setter Property="CornerRadius" Value="8" />
        <Setter Property="Background" Value="{ThemeResource AugmentedEntryPointButtonOverlayBackgroundBrush}" />
        <Setter Property="RenderTransform">
            <Setter.Value>
                <TranslateTransform X="14" Y="-12" />
            </Setter.Value>
        </Setter>
        <Setter Property="Height" Value="16" />
    </Style>
    <Style x:Key="AugmentedEntryPointOverlayStyle" TargetType="Grid">
        <Setter Property="CornerRadius" Value="8" />
        <Setter Property="Background" Value="{ThemeResource AugmentedEntryPointButtonOverlayBackgroundBrush}" />
        <Setter Property="RenderTransform">
            <Setter.Value>
                <TranslateTransform X="8" Y="-5" />
            </Setter.Value>
        </Setter>
        <Setter Property="Height" Value="16" />
    </Style>
    <CornerRadius x:Key="AugmentedEntryPointLeftAlignedCornerRadius" Value="4,0,0,4" />
    <DataTemplate x:Key="AugmentedEntryPointItemTemplate">
        <DataTemplate.Template>
            <AugmentedEntryPointButton x:Name="AugmentedEntryPointButton" HorizontalContentAlignment="Stretch" VerticalContentAlignment="Stretch">
                <AugmentedEntryPointButton.ToolTip>
                    <ToolTip Placement="Top" VerticalOffset="{StaticResource ToolTipMargin}" />
                </AugmentedEntryPointButton.ToolTip>
            </AugmentedEntryPointButton>
        </DataTemplate.Template>
    </DataTemplate>
    <DataTemplate x:Key="AugmentedEntryPointItemOverflowTemplate">
        <DataTemplate.Template>
            <AugmentedEntryPointButton x:Name="AugmentedEntryPointButton" HorizontalContentAlignment="Stretch" VerticalContentAlignment="Stretch" IsHoverInvokeEnabled="False">
                <AugmentedEntryPointButton.ToolTip>
                    <ToolTip Placement="Top" VerticalOffset="{StaticResource ToolTipMargin}" />
                </AugmentedEntryPointButton.ToolTip>
            </AugmentedEntryPointButton>
        </DataTemplate.Template>
    </DataTemplate>
    <Thickness x:Key="AugmentedEntryPointCenterAlignSmallMargin" Value="10,0,58,0" />
    <Thickness x:Key="AugmentedEntryPointButtonWithSearchBoxVariant2Margin" Value="0" />
    <Thickness x:Key="AugmentedEntryPointButtonWithSearchBoxVariant1WithCenterClusterMargin" Value="6,0,58,0" />
    <Thickness x:Key="AugmentedEntryPointButtonWithSearchBoxVariant1Margin" Value="0,0,8,0" />
    <Thickness x:Key="AugmentedEntryPointButtonWithSearchBoxOnTaskbarMargin" Value="0" />
    <Thickness x:Key="AugmentedEntryPointButtonWithCenterClusterMargin" Value="6,0,58,0" />
    <Thickness x:Key="AugmentedEntryPointButtonShyTabletHitTestableMargin" Value="3,6" />
    <Thickness x:Key="AugmentedEntryPointButtonMargin" Value="6,0,8,0" />
    <Thickness x:Key="AugmentedEntryPointButtonHitTestableMargin" Value="6,4,2,4" />
    <Double x:Key="AugmentedEntryPointButtonExtent" Value="152" />
    <Thickness x:Key="AugmentedEntryPointButtonCenterAlignSmallTabletMargin" Value="4,0,18,0" />
    <Thickness x:Key="AugmentedEntryPointButtonCenterAlignLargeTabletMargin" Value="7,0,21,0" />
    <Style x:Key="AugmentedEntryPointBadgeStyle" TargetType="Grid">
        <Setter Property="CornerRadius" Value="8" />
        <Setter Property="Background" Value="{ThemeResource AugmentedEntryPointButtonBadgeBackgroundBrush}" />
        <Setter Property="Height" Value="16" />
    </Style>
    <Style TargetType="TaskbarFrame">
        <Setter Property="Name" Value="TaskbarFrame" />
        <Setter Property="Background" Value="#00FFFFFF" />
        <Setter Property="TaskbarTheme" Value="{ThemeResource TaskbarTheme}" />
        <Setter Property="IsTabStop" Value="False" />
        <Setter Property="Template">
            <Setter.Value>
                <ControlTemplate TargetType="TaskbarFrame">
                    <ControlTemplate.Template>
                        <Grid x:Name="RootGrid" Background="#00FFFFFF" BackgroundSizing="InnerBorderEdge">
                            <Grid.Children>
                                <DeferredElement>
                                    <TaskbarBackground x:Name="BackgroundControl" AcrylicThemeBrush="{ThemeResource AcrylicInAppFillColorBaseBrush}" AccentAcrylicThemeBrush="{ThemeResource AccentAcrylicInAppFillColorBaseBrush}" AccessibilityView="Raw" IsHitTestVisible="False" />
                                </DeferredElement>
                                <ItemsRepeater x:Name="TaskbarFrameRepeater" TabFocusNavigation="Local" Margin="0">
                                    <ItemsRepeater.ItemTemplate>
                                        <TaskbarItemTemplateSelector
                                            LaunchListItemTemplate="{StaticResource LaunchListItemTemplate}"
                                            MeetNowItemTemplate="{StaticResource MeetNowItemTemplate}"
                                            OverflowItemTemplate="{StaticResource OverflowItemTemplate}"
                                            AugmentedEntryPointItemTemplate="{StaticResource AugmentedEntryPointItemTemplate}"
                                            SearchBoxLaunchListItemTemplate="{StaticResource SearchBoxLaunchListItemTemplate}"
                                            SearchBoxItemTemplate="{StaticResource SearchBoxItemTemplate}"
                                            TaskListLabeledItemTemplate="{StaticResource TaskListLabeledItemTemplate}"
                                            TaskListLabeledItemTemplateWithDownloadIcon="{StaticResource TaskListLabeledItemTemplateWithDownloadIcon}" />
                                    </ItemsRepeater.ItemTemplate>
                                    <ItemsRepeater.Layout>
                                        <TaskbarCollapsibleLayout />
                                    </ItemsRepeater.Layout>
                                </ItemsRepeater>
                                <DeferredElement>
                                    <Gripper x:Name="GripperControl"
                                        Width="{StaticResource GripperLandscapeWidth}"
                                        Height="24"
                                        VerticalAlignment="Top"
                                        HorizontalAlignment="Center"
                                        AccessibilityView="Raw"
                                        IsHitTestVisible="False" />
                                </DeferredElement>
                                <DeferredElement>
                                    <Rectangle x:Name="ScreenEdgeStroke" Height="2" VerticalAlignment="Bottom" Fill="#00FFFFFF" />
                                </DeferredElement>
                            </Grid.Children>
                        </Grid>
                    </ControlTemplate.Template>
                </ControlTemplate>
            </Setter.Value>
        </Setter>
    </Style>
    <Style TargetType="TaskbarBackground">
        <Setter Property="IsTabStop" Value="False" />
        <Setter Property="Template">
            <Setter.Value>
                <ControlTemplate TargetType="TaskbarBackground">
                    <ControlTemplate.Template>
                        <Grid>
                            <Grid.Children>
                                <Rectangle x:Name="BackgroundFill">
                                    <Rectangle.Fill>
                                        <AcrylicBrush BackgroundSource="Backdrop" />
                                    </Rectangle.Fill>
                                </Rectangle>
                                <Rectangle x:Name="BackgroundStroke" Fill="{ThemeResource SurfaceStrokeColorDefaultBrush}" Height="1" VerticalAlignment="Top" />
                            </Grid.Children>
                        </Grid>
                    </ControlTemplate.Template>
                </ControlTemplate>
            </Setter.Value>
        </Setter>
    </Style>
    <Style TargetType="TaskItemThumbnailScrollableList">
        <Setter Property="HorizontalAlignment" Value="Left" />
        <Setter Property="VerticalAlignment" Value="Bottom" />
        <Setter Property="Name" Value="TaskItemThumbnailScrollableList" />
        <Setter Property="MinWidth" Value="100" />
        <Setter Property="MinHeight" Value="40" />
        <Setter Property="Template">
            <Setter.Value>
                <ControlTemplate TargetType="TaskItemThumbnailScrollableList">
                    <ControlTemplate.Template>
                        <ScrollViewer x:Name="TaskItemThumbnailScrollableListScrollViewer" Style="{StaticResource TaskItemThumbnailScrollableListStyle}" ManipulationMode="TranslateY,TranslateInertia,System">
                            <ScrollViewer.Content>
                                <ItemsRepeater x:Name="TaskItemThumbnailListRepeater">
                                    <ItemsRepeater.ItemTemplate>
                                        <TaskItemHoverUITemplateSelector TaskItemThumbnailTemplate="{StaticResource TaskItemThumbnailScrollableTemplate}" />
                                    </ItemsRepeater.ItemTemplate>
                                    <ItemsRepeater.Layout>
                                        <TaskItemThumbnailScrollableListLayout />
                                    </ItemsRepeater.Layout>
                                </ItemsRepeater>
                            </ScrollViewer.Content>
                        </ScrollViewer>
                    </ControlTemplate.Template>
                </ControlTemplate>
            </Setter.Value>
        </Setter>
    </Style>
    <Style TargetType="TaskItemThumbnailList">
        <Setter Property="HorizontalAlignment" Value="Left" />
        <Setter Property="VerticalAlignment" Value="Bottom" />
        <Setter Property="Name" Value="TaskItemThumbnailList" />
        <Setter Property="MinWidth" Value="100" />
        <Setter Property="MinHeight" Value="40" />
        <Setter Property="Template">
            <Setter.Value>
                <ControlTemplate TargetType="TaskItemThumbnailList">
                    <ControlTemplate.Template>
                        <ItemsRepeater x:Name="TaskItemThumbnailListRepeater">
                            <ItemsRepeater.ItemTemplate>
                                <TaskItemHoverUITemplateSelector TaskItemThumbnailTemplate="{StaticResource TaskItemThumbnailTemplate}" />
                            </ItemsRepeater.ItemTemplate>
                            <ItemsRepeater.Layout>
                                <TaskItemThumbnailListLayout />
                            </ItemsRepeater.Layout>
                        </ItemsRepeater>
                    </ControlTemplate.Template>
                </ControlTemplate>
            </Setter.Value>
        </Setter>
    </Style>
    <Style TargetType="ShareWindowPanel">
        <Setter Property="Template">
            <Setter.Value>
                <ControlTemplate TargetType="ShareWindowPanel">
                    <ControlTemplate.Template>
                        <Grid x:Name="SharePanelGrid" Margin="8,0,8,8" ColumnSpacing="8">
                            <Grid.ColumnDefinitions>
                                <ColumnDefinition Width="*" />
                                <ColumnDefinition Width="*" />
                            </Grid.ColumnDefinitions>
                            <Grid.Children>
                                <ShareWindowButton x:Name="ShareButton" ColumnSpan="2" Height="32" HorizontalAlignment="Stretch" />
                                <Button x:Name="ConfirmButton"
                                    Height="32"
                                    Column="0"
                                    HorizontalAlignment="Stretch"
                                    Background="{StaticResource ShareWindow_ConfirmBackgroundBrush}"
                                    BorderBrush="{StaticResource ShareWindow_ConfirmBorderBrush}"
                                    BorderThickness="1"
                                    Foreground="{StaticResource ShareWindow_ConfirmForegroundBrush}">
                                    <Button.Resources>
                                        <ResourceDictionary>
                                            <StaticResource x:Key="ButtonForegroundPressed" ResourceKey="ShareWindow_ConfirmPressedForegroundBrush" />
                                            <StaticResource x:Key="ButtonForegroundPointerOver" ResourceKey="ShareWindow_ConfirmPointerOverForegroundBrush" />
                                            <StaticResource x:Key="ButtonBorderBrushPressed" ResourceKey="ShareWindow_ConfirmPressedBorderBrush" />
                                            <StaticResource x:Key="ButtonBorderBrushPointerOver" ResourceKey="ShareWindow_ConfirmPointerOverBorderBrush" />
                                            <StaticResource x:Key="ButtonBackgroundPressed" ResourceKey="ShareWindow_ConfirmPressedBackgroundBrush" />
                                            <StaticResource x:Key="ButtonBackgroundPointerOver" ResourceKey="ShareWindow_ConfirmPointerOverBackgroundBrush" />
                                        </ResourceDictionary>
                                    </Button.Resources>
                                    <Button.ToolTip>
                                        <ToolTip IsEnabled="True" Placement="Top" VerticalOffset="{StaticResource ToolTipMargin}" />
                                    </Button.ToolTip>
                                </Button>
                                <Button x:Name="CancelButton" Height="32" Column="1" HorizontalAlignment="Stretch">
                                    <Button.ToolTip>
                                        <ToolTip IsEnabled="True" Placement="Top" VerticalOffset="{StaticResource ToolTipMargin}" />
                                    </Button.ToolTip>
                                </Button>
                            </Grid.Children>
                        </Grid>
                    </ControlTemplate.Template>
                </ControlTemplate>
            </Setter.Value>
        </Setter>
    </Style>
    <Style TargetType="ShareWindowButton">
        <Setter Property="HorizontalAlignment" Value="Stretch" />
        <Setter Property="VerticalAlignment" Value="Stretch" />
        <Setter Property="HorizontalContentAlignment" Value="Stretch" />
        <Setter Property="VerticalContentAlignment" Value="Stretch" />
        <Setter Property="CornerRadius" Value="{StaticResource ControlLargeCornerRadius}" />
        <Setter Property="Template">
            <Setter.Value>
                <ControlTemplate TargetType="ShareWindowButton">
                    <ControlTemplate.Template>
                        <Grid Background="#00FFFFFF">
                            <Grid.ColumnDefinitions>
                                <ColumnDefinition Width="*" />
                                <ColumnDefinition Width="Auto" />
                                <ColumnDefinition Width="Auto" />
                                <ColumnDefinition Width="*" />
                            </Grid.ColumnDefinitions>
                            <Grid.Children>
                                <Border x:Name="BackgroundElement"
                                    ColumnSpan="4"
                                    Background="{ThemeResource ShareWindowButtonBackgroundBrush}"
                                    BorderBrush="{ThemeResource StrokeShadowBrush}"
                                    BorderThickness="{StaticResource TaskbarButtonOutlineThickness}"
                                    CornerRadius="{TemplateBinding CornerRadius}"
                                    IsHitTestVisible="False" />
                                <Border x:Name="ShadowElement"
                                    ColumnSpan="4"
                                    Margin="1,0"
                                    BorderBrush="{StaticResource ShadowBrush}"
                                    BorderThickness="{StaticResource TaskbarButtonShadowThickness}"
                                    CornerRadius="{TemplateBinding CornerRadius}"
                                    IsHitTestVisible="False" />
                                <Image x:Name="ShareWindowButtonIcon"
                                    Column="1"
                                    Width="{StaticResource ThumbnailIconSize}"
                                    Height="{StaticResource ThumbnailIconSize}"
                                    Margin="0,0,8,0"
                                    AccessibilityView="Raw"
                                    IsHitTestVisible="False"
                                    Stretch="Uniform" />
                                <Path x:Name="StopSharingGlyph"
                                    Column="1"
                                    Width="{StaticResource ShareWindowButtonIconSize}"
                                    Height="{StaticResource ShareWindowButtonIconSize}"
                                    Margin="0,1,8,0"
                                    VerticalAlignment="Center"
                                    AccessibilityView="Raw"
                                    Data="M1.33333 4.16699C1.33333 3.33857 2.00491 2.66699 2.83333 2.66699H13.1667C13.9951 2.66699 14.6667 3.33857 14.6667 4.16699V11.8337C14.6667 12.6621 13.9951 13.3337 13.1667 13.3337H2.83333C2.00491 13.3337 1.33333 12.6621 1.33333 11.8337V4.16699ZM2.83333 3.66699C2.55719 3.66699 2.33333 3.89085 2.33333 4.16699V11.8337C2.33333 12.1098 2.55719 12.3337 2.83333 12.3337H13.1667C13.4428 12.3337 13.6667 12.1098 13.6667 11.8337V4.16699C13.6667 3.89085 13.4428 3.66699 13.1667 3.66699H2.83333Z M5.47978 5.47652C5.67504 5.28126 5.99163 5.28126 6.18689 5.47652L8.00047 7.29016L9.8166 5.47904C10.0121 5.28405 10.3287 5.28449 10.5237 5.48002C10.7187 5.67556 10.7183 5.99214 10.5227 6.18713L8.7076 7.99722L10.5232 9.81289C10.7185 10.0082 10.7185 10.3247 10.5232 10.52C10.3279 10.7152 10.0114 10.7152 9.81613 10.52L8.00027 8.70409L6.18713 10.5198C5.99201 10.7151 5.67543 10.7154 5.48003 10.5202C5.28463 10.3251 5.28441 10.0085 5.47953 9.81309L7.29313 7.99696L5.47978 6.18363C5.28452 5.98837 5.28452 5.67179 5.47978 5.47652Z"
                                    Fill="{ThemeResource SystemControlForegroundChromeWhiteBrush}"
                                    IsHitTestVisible="False"
                                    Visibility="Collapsed" />
                                <TextBlock x:Name="ShareWindowButtonLabel" Column="2" VerticalAlignment="Center" AccessibilityView="Raw" IsHitTestVisible="False" />
                            </Grid.Children>
                            <Grid.VisualStateGroups>
                                <VisualStateGroup x:Name="CommonStates">
                                    <VisualStateGroup.States>
                                        <VisualState x:Name="StartSharingNormal" />
                                        <VisualState x:Name="StartSharingPointerOver">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShareWindowButtonBackgroundPointerOverBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="StartSharingPressed">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShareWindowButtonBackgroundPressedBrush}" />
                                                <Setter Target="ShareWindowButtonIcon.Opacity" Value="{ThemeResource IconPressedOpacity}" />
                                                <Setter Target="ShareWindowButtonLabel.Foreground" Value="{ThemeResource ShareWindowButtonForegroundPressedBrush}" />
                                                <Setter Target="ShadowElement.Opacity" Value="0" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="StopSharingNormal">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShareWindowButtonStopSharingBackgroundBrush}" />
                                                <Setter Target="ShareWindowButtonLabel.Foreground" Value="{ThemeResource SystemControlForegroundChromeWhiteBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShareWindowButtonStopSharingStrokeBrush}" />
                                                <Setter Target="ShadowElement.BorderBrush" Value="{ThemeResource ShareWindowButtonStopSharingShadowBrush}" />
                                                <Setter Target="StopSharingGlyph.Visibility" Value="Visible" />
                                                <Setter Target="ShareWindowButtonIcon.Visibility" Value="Collapsed" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="StopSharingPointerOver">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShareWindowButtonStopSharingBackgroundPointerOverBrush}" />
                                                <Setter Target="ShareWindowButtonLabel.Foreground" Value="{ThemeResource SystemControlForegroundChromeWhiteBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShareWindowButtonStopSharingStrokeBrush}" />
                                                <Setter Target="ShadowElement.BorderBrush" Value="{ThemeResource ShareWindowButtonStopSharingShadowBrush}" />
                                                <Setter Target="StopSharingGlyph.Visibility" Value="Visible" />
                                                <Setter Target="ShareWindowButtonIcon.Visibility" Value="Collapsed" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="StopSharingPressed">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShareWindowButtonStopSharingBackgroundPressedBrush}" />
                                                <Setter Target="ShareWindowButtonLabel.Foreground" Value="{ThemeResource SystemControlForegroundChromeWhiteBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShareWindowButtonStopSharingStrokeBrush}" />
                                                <Setter Target="ShadowElement.BorderBrush" Value="{ThemeResource ShareWindowButtonStopSharingShadowBrush}" />
                                                <Setter Target="ShadowElement.Opacity" Value="0" />
                                                <Setter Target="StopSharingGlyph.Visibility" Value="Visible" />
                                                <Setter Target="ShareWindowButtonIcon.Visibility" Value="Collapsed" />
                                                <Setter Target="ShareWindowButtonIcon.Opacity" Value="{ThemeResource IconPressedOpacity}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                    </VisualStateGroup.States>
                                </VisualStateGroup>
                            </Grid.VisualStateGroups>
                        </Grid>
                    </ControlTemplate.Template>
                </ControlTemplate>
            </Setter.Value>
        </Setter>
    </Style>
    <Style TargetType="SearchBoxLaunchListButton">
        <Setter Property="HorizontalAlignment" Value="Stretch" />
        <Setter Property="VerticalAlignment" Value="Stretch" />
        <Setter Property="HorizontalContentAlignment" Value="Stretch" />
        <Setter Property="VerticalContentAlignment" Value="Stretch" />
        <Setter Property="IsTabStop" Value="True" />
        <Setter Property="UseSystemFocusVisuals" Value="True" />
        <Setter Property="CornerRadius" Value="{StaticResource ControlLargeCornerRadius}" />
        <Setter Property="Template">
            <Setter.Value>
                <ControlTemplate TargetType="SearchBoxLaunchListButton">
                    <ControlTemplate.Template>
                        <TaskListButtonPanel x:Name="ExperienceToggleButtonRootPanel" Background="#00FFFFFF">
                            <TaskListButtonPanel.Children>
                                <Border x:Name="SearchPillBackgroundElement"
                                    Margin="{StaticResource SearchPillButtonMargin}"
                                    MinWidth="{StaticResource SearchPillButtonMinExtent}"
                                    Background="{ThemeResource SearchPillButtonBackgroundBrush}"
                                    BorderBrush="{ThemeResource SearchPillButtonBorderBrush}"
                                    BorderThickness="{StaticResource TaskbarButtonOutlineThickness}"
                                    CornerRadius="{StaticResource SearchPillButtonCornerRadius}"
                                    IsHitTestVisible="False"
                                    Visibility="Visible">
                                    <Border.BackgroundTransition>
                                        <BrushTransition Duration="0:0:0.083" />
                                    </Border.BackgroundTransition>
                                </Border>
                                <Border x:Name="BackgroundElement"
                                    Background="{ThemeResource ShellTaskbarItemFillColorTransparentBrush}"
                                    BorderBrush="#00FFFFFF"
                                    BorderThickness="{StaticResource TaskbarButtonOutlineThickness}"
                                    CornerRadius="{TemplateBinding CornerRadius}"
                                    IsHitTestVisible="False"
                                    Visibility="Collapsed">
                                    <Border.BackgroundTransition>
                                        <BrushTransition Duration="0:0:0.083" />
                                    </Border.BackgroundTransition>
                                </Border>
                                <AnimatedVisualPlayer x:Name="Icon"
                                    HorizontalAlignment="Center"
                                    VerticalAlignment="Center"
                                    AutoPlay="False"
                                    IsHitTestVisible="False"
                                    Stretch="Uniform"
                                    Visibility="Collapsed" />
                                <Grid x:Name="SearchBoxContentGrid"
                                    Padding="22,0"
                                    HorizontalAlignment="Center"
                                    VerticalAlignment="Center"
                                    IsHitTestVisible="False"
                                    Visibility="Visible">
                                    <Grid.ColumnDefinitions>
                                        <ColumnDefinition Width="*" />
                                        <ColumnDefinition Width="3*" />
                                    </Grid.ColumnDefinitions>
                                    <Grid.Children>
                                        <FontIcon x:Name="SearchBoxFontIcon"
                                            Column="0"
                                            Margin="0,0,4,0"
                                            HorizontalAlignment="Left"
                                            VerticalAlignment="Center"
                                            FontFamily="{StaticResource SymbolThemeFontFamily}"
                                            Foreground="{ThemeResource SearchPillButtonForegroundBrush}"
                                            FontSize="16"
                                            Glyph=""
                                            MirroredWhenRightToLeft="True" />
                                        <TextBlock x:Name="SearchBoxTextBlock"
                                            Column="1"
                                            HorizontalAlignment="Center"
                                            VerticalAlignment="Center"
                                            Foreground="{ThemeResource SearchPillButtonForegroundBrush}"
                                            Style="{StaticResource CaptionTextBlockStyle}" />
                                    </Grid.Children>
                                </Grid>
                            </TaskListButtonPanel.Children>
                            <TaskListButtonPanel.VisualStateGroups>
                                <VisualStateGroup x:Name="CommonStates">
                                    <VisualStateGroup.States>
                                        <VisualState x:Name="InactiveNormal">
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0:0:0.167"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[0].(GradientStop.Color)"
                                                            To="{ThemeResource SearchPillButtonBackground}" />
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0:0:0.167"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[1].(GradientStop.Color)"
                                                            To="{ThemeResource SearchPillButtonBackground}" />
                                                        <ColorAnimation Duration="0:0:0.167" TargetName="SearchBoxFontIcon" TargetProperty="(FontIcon.Foreground).(SolidColorBrush.Color)" To="{ThemeResource SearchPillButtonForeground}" />
                                                        <ColorAnimation Duration="0:0:0.167" TargetName="SearchBoxTextBlock" TargetProperty="(TextBlock.Foreground).(SolidColorBrush.Color)" To="{ThemeResource SearchPillButtonForeground}" />
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                        </VisualState>
                                        <VisualState x:Name="InactivePointerOver">
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0:0:0.167"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[0].(GradientStop.Color)"
                                                            To="{ThemeResource SearchPillButtonBackgroundHover}" />
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0:0:0.167"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[1].(GradientStop.Color)"
                                                            To="{ThemeResource SearchPillButtonBackgroundHover}" />
                                                        <ColorAnimation Duration="0:0:0.167" TargetName="SearchBoxFontIcon" TargetProperty="(FontIcon.Foreground).(SolidColorBrush.Color)" To="{ThemeResource SearchPillButtonForegroundHover}" />
                                                        <ColorAnimation Duration="0:0:0.167" TargetName="SearchBoxTextBlock" TargetProperty="(TextBlock.Foreground).(SolidColorBrush.Color)" To="{ThemeResource SearchPillButtonForegroundHover}" />
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                            <VisualState.Setters>
                                                <Setter Target="SearchPillBackgroundElement.BorderBrush" Value="{ThemeResource SearchPillButtonBorderHoverBrush}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorAltSecondaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorSecondaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="InactivePressed">
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0:0:0.167"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[0].(GradientStop.Color)"
                                                            To="{ThemeResource SearchPillButtonBackgroundPressed}" />
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0:0:0.167"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[1].(GradientStop.Color)"
                                                            To="{ThemeResource SearchPillButtonBackgroundPressed}" />
                                                        <ColorAnimation Duration="0:0:0.167" TargetName="SearchBoxFontIcon" TargetProperty="(FontIcon.Foreground).(SolidColorBrush.Color)" To="{ThemeResource SearchPillButtonForegroundPressed}" />
                                                        <ColorAnimation Duration="0:0:0.167" TargetName="SearchBoxTextBlock" TargetProperty="(TextBlock.Foreground).(SolidColorBrush.Color)" To="{ThemeResource SearchPillButtonForegroundPressed}" />
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                            <VisualState.Setters>
                                                <Setter Target="SearchPillBackgroundElement.BorderBrush" Value="{ThemeResource SearchPillButtonBorderPressedBrush}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorTertiaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemStrokeColorQuinaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActiveNormal_Color">
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0:0:0.167"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[0].(GradientStop.Color)"
                                                            To="{ThemeResource SearchPillButtonBackgroundActivePrimary}" />
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0:0:0.167"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[1].(GradientStop.Color)"
                                                            To="{ThemeResource SearchPillButtonBackgroundActiveSecondary}" />
                                                        <ColorAnimation Duration="0:0:0.167" TargetName="SearchBoxFontIcon" TargetProperty="(FontIcon.Foreground).(SolidColorBrush.Color)" To="{ThemeResource SearchPillButtonForegroundActive}" />
                                                        <ColorAnimation Duration="0:0:0.167" TargetName="SearchBoxTextBlock" TargetProperty="(TextBlock.Foreground).(SolidColorBrush.Color)" To="{ThemeResource SearchPillButtonForegroundActive}" />
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                            <VisualState.Setters>
                                                <Setter Target="SearchPillBackgroundElement.BorderBrush" Value="{ThemeResource SearchPillButtonBorderActiveBrush}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorSecondaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorPrimaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActivePointerOver_Color">
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0:0:0.167"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[0].(GradientStop.Color)"
                                                            To="{ThemeResource SearchPillButtonBackgroundActivePrimary}" />
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0:0:0.167"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[1].(GradientStop.Color)"
                                                            To="{ThemeResource SearchPillButtonBackgroundActiveSecondary}" />
                                                        <ColorAnimation Duration="0:0:0.167" TargetName="SearchBoxFontIcon" TargetProperty="(FontIcon.Foreground).(SolidColorBrush.Color)" To="{ThemeResource SearchPillButtonForegroundActive}" />
                                                        <ColorAnimation Duration="0:0:0.167" TargetName="SearchBoxTextBlock" TargetProperty="(TextBlock.Foreground).(SolidColorBrush.Color)" To="{ThemeResource SearchPillButtonForegroundActive}" />
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                            <VisualState.Setters>
                                                <Setter Target="SearchPillBackgroundElement.BorderBrush" Value="{ThemeResource SearchPillButtonBorderActiveBrush}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorPrimaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorSecondaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActivePressed_Color">
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0:0:0.167"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[0].(GradientStop.Color)"
                                                            To="{ThemeResource SearchPillButtonBackgroundActivePrimary}" />
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0:0:0.167"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[1].(GradientStop.Color)"
                                                            To="{ThemeResource SearchPillButtonBackgroundActiveSecondary}" />
                                                        <ColorAnimation Duration="0:0:0.167" TargetName="SearchBoxFontIcon" TargetProperty="(FontIcon.Foreground).(SolidColorBrush.Color)" To="{ThemeResource SearchPillButtonForegroundActive}" />
                                                        <ColorAnimation Duration="0:0:0.167" TargetName="SearchBoxTextBlock" TargetProperty="(TextBlock.Foreground).(SolidColorBrush.Color)" To="{ThemeResource SearchPillButtonForegroundActive}" />
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                            <VisualState.Setters>
                                                <Setter Target="SearchPillBackgroundElement.BorderBrush" Value="{ThemeResource SearchPillButtonBorderActiveBrush}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorTertiaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemStrokeColorQuinaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="InactiveNormal_NoAnimations">
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[0].(GradientStop.Color)"
                                                            To="{ThemeResource SearchPillButtonBackground}" />
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[1].(GradientStop.Color)"
                                                            To="{ThemeResource SearchPillButtonBackground}" />
                                                        <ColorAnimation Duration="0" TargetName="SearchBoxFontIcon" TargetProperty="(FontIcon.Foreground).(SolidColorBrush.Color)" To="{ThemeResource SearchPillButtonForeground}" />
                                                        <ColorAnimation Duration="0" TargetName="SearchBoxTextBlock" TargetProperty="(TextBlock.Foreground).(SolidColorBrush.Color)" To="{ThemeResource SearchPillButtonForeground}" />
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                        </VisualState>
                                        <VisualState x:Name="InactivePointerOver_NoAnimations">
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[0].(GradientStop.Color)"
                                                            To="{ThemeResource SearchPillButtonBackgroundHover}" />
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[1].(GradientStop.Color)"
                                                            To="{ThemeResource SearchPillButtonBackgroundHover}" />
                                                        <ColorAnimation Duration="0" TargetName="SearchBoxFontIcon" TargetProperty="(FontIcon.Foreground).(SolidColorBrush.Color)" To="{ThemeResource SearchPillButtonForegroundHover}" />
                                                        <ColorAnimation Duration="0" TargetName="SearchBoxTextBlock" TargetProperty="(TextBlock.Foreground).(SolidColorBrush.Color)" To="{ThemeResource SearchPillButtonForegroundHover}" />
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                            <VisualState.Setters>
                                                <Setter Target="SearchPillBackgroundElement.BorderBrush" Value="{ThemeResource SearchPillButtonBorderHoverBrush}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorAltSecondaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorSecondaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="InactivePressed_NoAnimations">
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[0].(GradientStop.Color)"
                                                            To="{ThemeResource SearchPillButtonBackgroundPressed}" />
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[1].(GradientStop.Color)"
                                                            To="{ThemeResource SearchPillButtonBackgroundPressed}" />
                                                        <ColorAnimation Duration="0" TargetName="SearchBoxFontIcon" TargetProperty="(FontIcon.Foreground).(SolidColorBrush.Color)" To="{ThemeResource SearchPillButtonForegroundPressed}" />
                                                        <ColorAnimation Duration="0" TargetName="SearchBoxTextBlock" TargetProperty="(TextBlock.Foreground).(SolidColorBrush.Color)" To="{ThemeResource SearchPillButtonForegroundPressed}" />
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                            <VisualState.Setters>
                                                <Setter Target="SearchPillBackgroundElement.BorderBrush" Value="{ThemeResource SearchPillButtonBorderPressedBrush}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorTertiaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemStrokeColorQuinaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActiveNormal_Color_NoAnimations">
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[0].(GradientStop.Color)"
                                                            To="{ThemeResource SearchPillButtonBackgroundActivePrimary}" />
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[1].(GradientStop.Color)"
                                                            To="{ThemeResource SearchPillButtonBackgroundActiveSecondary}" />
                                                        <ColorAnimation Duration="0" TargetName="SearchBoxFontIcon" TargetProperty="(FontIcon.Foreground).(SolidColorBrush.Color)" To="{ThemeResource SearchPillButtonForegroundActive}" />
                                                        <ColorAnimation Duration="0" TargetName="SearchBoxTextBlock" TargetProperty="(TextBlock.Foreground).(SolidColorBrush.Color)" To="{ThemeResource SearchPillButtonForegroundActive}" />
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                            <VisualState.Setters>
                                                <Setter Target="SearchPillBackgroundElement.BorderBrush" Value="{ThemeResource SearchPillButtonBorderActiveBrush}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorSecondaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorPrimaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActivePointerOver_Color_NoAnimations">
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[0].(GradientStop.Color)"
                                                            To="{ThemeResource SearchPillButtonBackgroundActivePrimary}" />
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[1].(GradientStop.Color)"
                                                            To="{ThemeResource SearchPillButtonBackgroundActiveSecondary}" />
                                                        <ColorAnimation Duration="0" TargetName="SearchBoxFontIcon" TargetProperty="(FontIcon.Foreground).(SolidColorBrush.Color)" To="{ThemeResource SearchPillButtonForegroundActive}" />
                                                        <ColorAnimation Duration="0" TargetName="SearchBoxTextBlock" TargetProperty="(TextBlock.Foreground).(SolidColorBrush.Color)" To="{ThemeResource SearchPillButtonForegroundActive}" />
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                            <VisualState.Setters>
                                                <Setter Target="SearchPillBackgroundElement.BorderBrush" Value="{ThemeResource SearchPillButtonBorderActiveBrush}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorPrimaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorSecondaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActivePressed_Color_NoAnimations">
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[0].(GradientStop.Color)"
                                                            To="{ThemeResource SearchPillButtonBackgroundActivePrimary}" />
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[1].(GradientStop.Color)"
                                                            To="{ThemeResource SearchPillButtonBackgroundActiveSecondary}" />
                                                        <ColorAnimation Duration="0" TargetName="SearchBoxFontIcon" TargetProperty="(FontIcon.Foreground).(SolidColorBrush.Color)" To="{ThemeResource SearchPillButtonForegroundActive}" />
                                                        <ColorAnimation Duration="0" TargetName="SearchBoxTextBlock" TargetProperty="(TextBlock.Foreground).(SolidColorBrush.Color)" To="{ThemeResource SearchPillButtonForegroundActive}" />
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                            <VisualState.Setters>
                                                <Setter Target="SearchPillBackgroundElement.BorderBrush" Value="{ThemeResource SearchPillButtonBorderActiveBrush}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorTertiaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemStrokeColorQuinaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="InactiveNormal_SearchPillCustomTheme">
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0:0:0.167"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[0].(GradientStop.Color)"
                                                            To="{ThemeResource CustomThemeSearchPillButtonBackground}" />
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0:0:0.167"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[1].(GradientStop.Color)"
                                                            To="{ThemeResource CustomThemeSearchPillButtonBackground}" />
                                                        <ColorAnimation Duration="0:0:0.167" TargetName="SearchBoxFontIcon" TargetProperty="(FontIcon.Foreground).(SolidColorBrush.Color)" To="{ThemeResource CustomThemeSearchPillButtonForeground}" />
                                                        <ColorAnimation Duration="0:0:0.167" TargetName="SearchBoxTextBlock" TargetProperty="(TextBlock.Foreground).(SolidColorBrush.Color)" To="{ThemeResource CustomThemeSearchPillButtonForeground}" />
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                        </VisualState>
                                        <VisualState x:Name="InactivePointerOver_SearchPillCustomTheme">
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0:0:0.167"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[0].(GradientStop.Color)"
                                                            To="{ThemeResource CustomThemeSearchPillButtonBackgroundHover}" />
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0:0:0.167"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[1].(GradientStop.Color)"
                                                            To="{ThemeResource CustomThemeSearchPillButtonBackgroundHover}" />
                                                        <ColorAnimation Duration="0:0:0.167" TargetName="SearchBoxFontIcon" TargetProperty="(FontIcon.Foreground).(SolidColorBrush.Color)" To="{ThemeResource CustomThemeSearchPillButtonForegroundHover}" />
                                                        <ColorAnimation Duration="0:0:0.167" TargetName="SearchBoxTextBlock" TargetProperty="(TextBlock.Foreground).(SolidColorBrush.Color)" To="{ThemeResource CustomThemeSearchPillButtonForegroundHover}" />
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                            <VisualState.Setters>
                                                <Setter Target="SearchPillBackgroundElement.BorderBrush" Value="{ThemeResource CustomThemeSearchPillButtonBorderHoverBrush}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorAltSecondaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorSecondaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="InactivePressed_SearchPillCustomTheme">
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0:0:0.167"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[0].(GradientStop.Color)"
                                                            To="{ThemeResource CustomThemeSearchPillButtonBackgroundPressed}" />
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0:0:0.167"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[1].(GradientStop.Color)"
                                                            To="{ThemeResource CustomThemeSearchPillButtonBackgroundPressed}" />
                                                        <ColorAnimation Duration="0:0:0.167" TargetName="SearchBoxFontIcon" TargetProperty="(FontIcon.Foreground).(SolidColorBrush.Color)" To="{ThemeResource CustomThemeSearchPillButtonForegroundPressed}" />
                                                        <ColorAnimation Duration="0:0:0.167" TargetName="SearchBoxTextBlock" TargetProperty="(TextBlock.Foreground).(SolidColorBrush.Color)" To="{ThemeResource CustomThemeSearchPillButtonForegroundPressed}" />
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                            <VisualState.Setters>
                                                <Setter Target="SearchPillBackgroundElement.BorderBrush" Value="{ThemeResource CustomThemeSearchPillButtonBorderPressedBrush}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorTertiaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemStrokeColorQuinaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActiveNormal_Color_SearchPillCustomTheme">
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0:0:0.167"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[0].(GradientStop.Color)"
                                                            To="{ThemeResource CustomThemeSearchPillButtonBackgroundActivePrimary}" />
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0:0:0.167"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[1].(GradientStop.Color)"
                                                            To="{ThemeResource CustomThemeSearchPillButtonBackgroundActiveSecondary}" />
                                                        <ColorAnimation Duration="0:0:0.167" TargetName="SearchBoxFontIcon" TargetProperty="(FontIcon.Foreground).(SolidColorBrush.Color)" To="{ThemeResource CustomThemeSearchPillButtonForegroundActive}" />
                                                        <ColorAnimation Duration="0:0:0.167" TargetName="SearchBoxTextBlock" TargetProperty="(TextBlock.Foreground).(SolidColorBrush.Color)" To="{ThemeResource CustomThemeSearchPillButtonForegroundActive}" />
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                            <VisualState.Setters>
                                                <Setter Target="SearchPillBackgroundElement.BorderBrush" Value="{ThemeResource CustomThemeSearchPillButtonBorderActiveBrush}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorSecondaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorPrimaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActivePointerOver_Color_SearchPillCustomTheme">
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0:0:0.167"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[0].(GradientStop.Color)"
                                                            To="{ThemeResource CustomThemeSearchPillButtonBackgroundActivePrimary}" />
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0:0:0.167"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[1].(GradientStop.Color)"
                                                            To="{ThemeResource CustomThemeSearchPillButtonBackgroundActiveSecondary}" />
                                                        <ColorAnimation Duration="0:0:0.167" TargetName="SearchBoxFontIcon" TargetProperty="(FontIcon.Foreground).(SolidColorBrush.Color)" To="{ThemeResource CustomThemeSearchPillButtonForegroundActive}" />
                                                        <ColorAnimation Duration="0:0:0.167" TargetName="SearchBoxTextBlock" TargetProperty="(TextBlock.Foreground).(SolidColorBrush.Color)" To="{ThemeResource CustomThemeSearchPillButtonForegroundActive}" />
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                            <VisualState.Setters>
                                                <Setter Target="SearchPillBackgroundElement.BorderBrush" Value="{ThemeResource CustomThemeSearchPillButtonBorderActiveBrush}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorPrimaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorSecondaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActivePressed_Color_SearchPillCustomTheme">
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0:0:0.167"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[0].(GradientStop.Color)"
                                                            To="{ThemeResource CustomThemeSearchPillButtonBackgroundActivePrimary}" />
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0:0:0.167"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[1].(GradientStop.Color)"
                                                            To="{ThemeResource CustomThemeSearchPillButtonBackgroundActiveSecondary}" />
                                                        <ColorAnimation Duration="0:0:0.167" TargetName="SearchBoxFontIcon" TargetProperty="(FontIcon.Foreground).(SolidColorBrush.Color)" To="{ThemeResource CustomThemeSearchPillButtonForegroundActive}" />
                                                        <ColorAnimation Duration="0:0:0.167" TargetName="SearchBoxTextBlock" TargetProperty="(TextBlock.Foreground).(SolidColorBrush.Color)" To="{ThemeResource CustomThemeSearchPillButtonForegroundActive}" />
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                            <VisualState.Setters>
                                                <Setter Target="SearchPillBackgroundElement.BorderBrush" Value="{ThemeResource CustomThemeSearchPillButtonBorderActiveBrush}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorTertiaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemStrokeColorQuinaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="InactiveNormal_NoAnimations_SearchPillCustomTheme">
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[0].(GradientStop.Color)"
                                                            To="{ThemeResource CustomThemeSearchPillButtonBackground}" />
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[1].(GradientStop.Color)"
                                                            To="{ThemeResource CustomThemeSearchPillButtonBackground}" />
                                                        <ColorAnimation Duration="0" TargetName="SearchBoxFontIcon" TargetProperty="(FontIcon.Foreground).(SolidColorBrush.Color)" To="{ThemeResource CustomThemeSearchPillButtonForeground}" />
                                                        <ColorAnimation Duration="0" TargetName="SearchBoxTextBlock" TargetProperty="(TextBlock.Foreground).(SolidColorBrush.Color)" To="{ThemeResource CustomThemeSearchPillButtonForeground}" />
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                        </VisualState>
                                        <VisualState x:Name="InactivePointerOver_NoAnimations_SearchPillCustomTheme">
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[0].(GradientStop.Color)"
                                                            To="{ThemeResource CustomThemeSearchPillButtonBackgroundHover}" />
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[1].(GradientStop.Color)"
                                                            To="{ThemeResource CustomThemeSearchPillButtonBackgroundHover}" />
                                                        <ColorAnimation Duration="0" TargetName="SearchBoxFontIcon" TargetProperty="(FontIcon.Foreground).(SolidColorBrush.Color)" To="{ThemeResource CustomThemeSearchPillButtonForegroundHover}" />
                                                        <ColorAnimation Duration="0" TargetName="SearchBoxTextBlock" TargetProperty="(TextBlock.Foreground).(SolidColorBrush.Color)" To="{ThemeResource CustomThemeSearchPillButtonForegroundHover}" />
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                            <VisualState.Setters>
                                                <Setter Target="SearchPillBackgroundElement.BorderBrush" Value="{ThemeResource CustomThemeSearchPillButtonBorderHoverBrush}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorAltSecondaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorSecondaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="InactivePressed_NoAnimations_SearchPillCustomTheme">
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[0].(GradientStop.Color)"
                                                            To="{ThemeResource CustomThemeSearchPillButtonBackgroundPressed}" />
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[1].(GradientStop.Color)"
                                                            To="{ThemeResource CustomThemeSearchPillButtonBackgroundPressed}" />
                                                        <ColorAnimation Duration="0" TargetName="SearchBoxFontIcon" TargetProperty="(FontIcon.Foreground).(SolidColorBrush.Color)" To="{ThemeResource CustomThemeSearchPillButtonForegroundPressed}" />
                                                        <ColorAnimation Duration="0" TargetName="SearchBoxTextBlock" TargetProperty="(TextBlock.Foreground).(SolidColorBrush.Color)" To="{ThemeResource CustomThemeSearchPillButtonForegroundPressed}" />
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                            <VisualState.Setters>
                                                <Setter Target="SearchPillBackgroundElement.BorderBrush" Value="{ThemeResource CustomThemeSearchPillButtonBorderPressedBrush}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorTertiaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemStrokeColorQuinaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActiveNormal_Color_NoAnimations_SearchPillCustomTheme">
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[0].(GradientStop.Color)"
                                                            To="{ThemeResource CustomThemeSearchPillButtonBackgroundActivePrimary}" />
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[1].(GradientStop.Color)"
                                                            To="{ThemeResource CustomThemeSearchPillButtonBackgroundActiveSecondary}" />
                                                        <ColorAnimation Duration="0" TargetName="SearchBoxFontIcon" TargetProperty="(FontIcon.Foreground).(SolidColorBrush.Color)" To="{ThemeResource CustomThemeSearchPillButtonForegroundActive}" />
                                                        <ColorAnimation Duration="0" TargetName="SearchBoxTextBlock" TargetProperty="(TextBlock.Foreground).(SolidColorBrush.Color)" To="{ThemeResource CustomThemeSearchPillButtonForegroundActive}" />
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                            <VisualState.Setters>
                                                <Setter Target="SearchPillBackgroundElement.BorderBrush" Value="{ThemeResource CustomThemeSearchPillButtonBorderActiveBrush}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorSecondaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorPrimaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActivePointerOver_Color_NoAnimations_SearchPillCustomTheme">
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[0].(GradientStop.Color)"
                                                            To="{ThemeResource CustomThemeSearchPillButtonBackgroundActivePrimary}" />
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[1].(GradientStop.Color)"
                                                            To="{ThemeResource CustomThemeSearchPillButtonBackgroundActiveSecondary}" />
                                                        <ColorAnimation Duration="0" TargetName="SearchBoxFontIcon" TargetProperty="(FontIcon.Foreground).(SolidColorBrush.Color)" To="{ThemeResource CustomThemeSearchPillButtonForegroundActive}" />
                                                        <ColorAnimation Duration="0" TargetName="SearchBoxTextBlock" TargetProperty="(TextBlock.Foreground).(SolidColorBrush.Color)" To="{ThemeResource CustomThemeSearchPillButtonForegroundActive}" />
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                            <VisualState.Setters>
                                                <Setter Target="SearchPillBackgroundElement.BorderBrush" Value="{ThemeResource CustomThemeSearchPillButtonBorderActiveBrush}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorPrimaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorSecondaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActivePressed_Color_NoAnimations_SearchPillCustomTheme">
                                            <VisualState.Storyboard>
                                                <Storyboard>
                                                    <Storyboard.Children>
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[0].(GradientStop.Color)"
                                                            To="{ThemeResource CustomThemeSearchPillButtonBackgroundActivePrimary}" />
                                                        <ColorAnimation
                                                            EnableDependentAnimation="True"
                                                            Duration="0"
                                                            TargetName="SearchPillBackgroundElement"
                                                            TargetProperty="(Border.Background).(GradientBrush.GradientStops)[1].(GradientStop.Color)"
                                                            To="{ThemeResource CustomThemeSearchPillButtonBackgroundActiveSecondary}" />
                                                        <ColorAnimation Duration="0" TargetName="SearchBoxFontIcon" TargetProperty="(FontIcon.Foreground).(SolidColorBrush.Color)" To="{ThemeResource CustomThemeSearchPillButtonForegroundActive}" />
                                                        <ColorAnimation Duration="0" TargetName="SearchBoxTextBlock" TargetProperty="(TextBlock.Foreground).(SolidColorBrush.Color)" To="{ThemeResource CustomThemeSearchPillButtonForegroundActive}" />
                                                    </Storyboard.Children>
                                                </Storyboard>
                                            </VisualState.Storyboard>
                                            <VisualState.Setters>
                                                <Setter Target="SearchPillBackgroundElement.BorderBrush" Value="{ThemeResource CustomThemeSearchPillButtonBorderActiveBrush}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorTertiaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemStrokeColorQuinaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="InactiveNormal_HC" />
                                        <VisualState x:Name="InactivePointerOver_HC">
                                            <VisualState.Setters>
                                                <Setter Target="SearchPillBackgroundElement.Background" Value="{ThemeResource SearchPillButtonBackgroundHoverBrush}" />
                                                <Setter Target="SearchPillBackgroundElement.BorderBrush" Value="{ThemeResource SearchPillButtonBorderHoverBrush}" />
                                                <Setter Target="SearchBoxFontIcon.Foreground" Value="{ThemeResource SearchPillButtonForegroundHoverBrush}" />
                                                <Setter Target="SearchBoxTextBlock.Foreground" Value="{ThemeResource SearchPillButtonForegroundHoverBrush}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorAltSecondaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorSecondaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="InactivePressed_HC">
                                            <VisualState.Setters>
                                                <Setter Target="SearchPillBackgroundElement.Background" Value="{ThemeResource SearchPillButtonBackgroundPressedBrush}" />
                                                <Setter Target="SearchPillBackgroundElement.BorderBrush" Value="{ThemeResource SearchPillButtonBorderPressedBrush}" />
                                                <Setter Target="SearchBoxFontIcon.Foreground" Value="{ThemeResource SearchPillButtonForegroundPressedBrush}" />
                                                <Setter Target="SearchBoxTextBlock.Foreground" Value="{ThemeResource SearchPillButtonForegroundPressedBrush}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorTertiaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemStrokeColorQuinaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActiveNormal_Color_HC">
                                            <VisualState.Setters>
                                                <Setter Target="SearchPillBackgroundElement.Background" Value="{ThemeResource SearchPillButtonBackgroundActiveBrush}" />
                                                <Setter Target="SearchPillBackgroundElement.BorderBrush" Value="{ThemeResource SearchPillButtonBorderActiveBrush}" />
                                                <Setter Target="SearchBoxFontIcon.Foreground" Value="{ThemeResource SearchPillButtonForegroundActiveBrush}" />
                                                <Setter Target="SearchBoxTextBlock.Foreground" Value="{ThemeResource SearchPillButtonForegroundActiveBrush}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorSecondaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorPrimaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActivePointerOver_Color_HC">
                                            <VisualState.Setters>
                                                <Setter Target="SearchPillBackgroundElement.Background" Value="{ThemeResource SearchPillButtonBackgroundActiveBrush}" />
                                                <Setter Target="SearchPillBackgroundElement.BorderBrush" Value="{ThemeResource SearchPillButtonBorderActiveBrush}" />
                                                <Setter Target="SearchBoxFontIcon.Foreground" Value="{ThemeResource SearchPillButtonForegroundActiveBrush}" />
                                                <Setter Target="SearchBoxTextBlock.Foreground" Value="{ThemeResource SearchPillButtonForegroundActiveBrush}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorPrimaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorSecondaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActivePressed_Color_HC">
                                            <VisualState.Setters>
                                                <Setter Target="SearchPillBackgroundElement.Background" Value="{ThemeResource SearchPillButtonBackgroundActiveBrush}" />
                                                <Setter Target="SearchPillBackgroundElement.BorderBrush" Value="{ThemeResource SearchPillButtonBorderActiveBrush}" />
                                                <Setter Target="SearchBoxFontIcon.Foreground" Value="{ThemeResource SearchPillButtonForegroundActiveBrush}" />
                                                <Setter Target="SearchBoxTextBlock.Foreground" Value="{ThemeResource SearchPillButtonForegroundActiveBrush}" />
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorTertiaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemStrokeColorQuinaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                    </VisualStateGroup.States>
                                </VisualStateGroup>
                            </TaskListButtonPanel.VisualStateGroups>
                        </TaskListButtonPanel>
                    </ControlTemplate.Template>
                </ControlTemplate>
            </Setter.Value>
        </Setter>
    </Style>
    <Style TargetType="OverflowToggleButton">
        <Setter Property="HorizontalAlignment" Value="Stretch" />
        <Setter Property="VerticalAlignment" Value="Stretch" />
        <Setter Property="HorizontalContentAlignment" Value="Stretch" />
        <Setter Property="VerticalContentAlignment" Value="Stretch" />
        <Setter Property="IsTabStop" Value="True" />
        <Setter Property="UseSystemFocusVisuals" Value="True" />
        <Setter Property="FocusVisualMargin" Value="-2" />
        <Setter Property="FocusVisualSecondaryThickness" Value="0" />
        <Setter Property="CornerRadius" Value="{StaticResource ControlLargeCornerRadius}" />
        <Setter Property="Template">
            <Setter.Value>
                <ControlTemplate TargetType="OverflowToggleButton">
                    <ControlTemplate.Template>
                        <TaskListButtonPanel x:Name="OverflowToggleButtonRootPanel" Background="#00FFFFFF">
                            <TaskListButtonPanel.Children>
                                <DeferredElement>
                                    <Rectangle x:Name="LeftOverflowButtonDivider"
                                        Width="1"
                                        HorizontalAlignment="Left"
                                        VerticalAlignment="Center"
                                        Fill="{ThemeResource MostRecentlyUsedDividerBrush}"
                                        IsHitTestVisible="False" />
                                </DeferredElement>
                                <Border x:Name="BackgroundElement"
                                    Background="{ThemeResource ShellTaskbarItemFillColorTransparentBrush}"
                                    BorderBrush="#00FFFFFF"
                                    BorderThickness="{StaticResource TaskbarButtonOutlineThickness}"
                                    CornerRadius="{TemplateBinding CornerRadius}"
                                    IsTemplateFocusTarget="True"
                                    IsHitTestVisible="False">
                                    <Border.BackgroundTransition>
                                        <BrushTransition Duration="0:0:0.083" />
                                    </Border.BackgroundTransition>
                                </Border>
                                <SymbolIcon x:Name="Icon" Symbol="More" HorizontalAlignment="Center" VerticalAlignment="Center" />
                                <Rectangle x:Name="RightOverflowButtonDivider"
                                    Width="1"
                                    HorizontalAlignment="Right"
                                    VerticalAlignment="Center"
                                    Fill="{ThemeResource MostRecentlyUsedDividerBrush}"
                                    IsHitTestVisible="False" />
                            </TaskListButtonPanel.Children>
                            <TaskListButtonPanel.VisualStateGroups>
                                <VisualStateGroup x:Name="CommonStates">
                                    <VisualStateGroup.States>
                                        <VisualState x:Name="InactiveNormal" />
                                        <VisualState x:Name="InactivePointerOver">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorAltSecondaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorSecondaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="InactivePressed">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorTertiaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemStrokeColorQuinaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActiveNormal">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorSecondaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorPrimaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActivePointerOver">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorPrimaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorSecondaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActivePressed">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorTertiaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemStrokeColorQuinaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                    </VisualStateGroup.States>
                                </VisualStateGroup>
                            </TaskListButtonPanel.VisualStateGroups>
                        </TaskListButtonPanel>
                    </ControlTemplate.Template>
                </ControlTemplate>
            </Setter.Value>
        </Setter>
    </Style>
    <Style TargetType="OverflowFlyoutList">
        <Setter Property="HorizontalAlignment" Value="Left" />
        <Setter Property="VerticalAlignment" Value="Bottom" />
        <Setter Property="Name" Value="OverflowFlyoutList" />
        <Setter Property="Template">
            <Setter.Value>
                <ControlTemplate TargetType="OverflowFlyoutList">
                    <ControlTemplate.Template>
                        <ScrollViewer x:Name="OverflowScrollView" Style="{StaticResource OverflowHorizontalScrollViewerStyle}" ManipulationMode="TranslateX,TranslateInertia,System">
                            <ScrollViewer.Content>
                                <ItemsRepeater x:Name="OverflowFlyoutListRepeater" TabFocusNavigation="Local">
                                    <ItemsRepeater.ItemTemplate>
                                        <TaskbarItemTemplateSelector
                                            LaunchListItemTemplate="{StaticResource LaunchListItemOverflowTemplate}"
                                            MeetNowItemTemplate="{StaticResource MeetNowItemOverflowTemplate}"
                                            OverflowItemTemplate="{StaticResource OverflowItemTemplate}"
                                            AugmentedEntryPointItemTemplate="{StaticResource AugmentedEntryPointItemOverflowTemplate}"
                                            SearchBoxItemTemplate="{StaticResource SearchBoxItemOverflowTemplate}"
                                            TaskListLabeledItemTemplate="{StaticResource TaskListLabeledItemOverflowTemplate}"
                                            TaskListLabeledItemTemplateWithDownloadIcon="{StaticResource TaskListLabeledItemOverflowTemplateWithDownloadIcon}" />
                                    </ItemsRepeater.ItemTemplate>
                                    <ItemsRepeater.Layout>
                                        <OverflowFlyoutLayout />
                                    </ItemsRepeater.Layout>
                                </ItemsRepeater>
                            </ScrollViewer.Content>
                        </ScrollViewer>
                    </ControlTemplate.Template>
                </ControlTemplate>
            </Setter.Value>
        </Setter>
    </Style>
    <Style TargetType="Gripper">
        <Setter Property="IsEnabled" Value="False" />
        <Setter Property="Template">
            <Setter.Value>
                <ControlTemplate TargetType="Gripper">
                    <ControlTemplate.Template>
                        <Rectangle x:Name="Gripper"
                            Fill="{ThemeResource GripperBrush}"
                            Height="3"
                            VerticalAlignment="Center"
                            RadiusX="1.5"
                            RadiusY="1.5" />
                    </ControlTemplate.Template>
                </ControlTemplate>
            </Setter.Value>
        </Setter>
    </Style>
    <Style TargetType="FlyoutFrame">
        <Setter Property="Padding" Value="0,0,0,12" />
        <Setter Property="HorizontalAlignment" Value="Left" />
        <Setter Property="VerticalAlignment" Value="Bottom" />
        <Setter Property="TaskbarTheme" Value="{ThemeResource TaskbarTheme}" />
        <Setter Property="Template">
            <Setter.Value>
                <ControlTemplate TargetType="FlyoutFrame">
                    <ControlTemplate.Template>
                        <Canvas x:Name="HoverFlyoutCanvas">
                            <Canvas.Children>
                                <Grid x:Name="HoverFlyoutGrid" Padding="{TemplateBinding Padding}" HorizontalAlignment="Left" VerticalAlignment="Bottom" Background="#00FFFFFF">
                                    <Grid.Children>
                                        <Border x:Name="HoverFlyoutBackgroundShadowReceiver" Margin="-100,-100,-100,-12" IsHitTestVisible="False" />
                                        <Border x:Name="HoverFlyoutBackground"
                                            BackgroundSizing="InnerBorderEdge"
                                            BorderBrush="{ThemeResource SurfaceStrokeColorFlyoutBrush}"
                                            BorderThickness="{ThemeResource FlyoutBorderThemeThickness}"
                                            CornerRadius="{ThemeResource OverlayCornerRadius}"
                                            IsHitTestVisible="False"
                                            Translation="0,0,32">
                                            <Border.Shadow>
                                                <ThemeShadow x:Name="HoverFlyoutBackgroundShadow" />
                                            </Border.Shadow>
                                            <Border.Child>
                                                <TaskbarBackground x:Name="HoverFlyoutBackgroundControl" IsBorderVisible="False" AcrylicThemeBrush="{ThemeResource AcrylicInAppFillColorDefaultBrush}" AccentAcrylicThemeBrush="{ThemeResource AccentAcrylicInAppFillColorDefaultBrush}" />
                                            </Border.Child>
                                        </Border>
                                        <ContentPresenter x:Name="HoverFlyoutContent" Margin="{ThemeResource FlyoutBorderThemeThickness}" />
                                    </Grid.Children>
                                </Grid>
                            </Canvas.Children>
                        </Canvas>
                    </ControlTemplate.Template>
                </ControlTemplate>
            </Setter.Value>
        </Setter>
    </Style>
    <Style TargetType="ExperienceToggleButton">
        <Setter Property="HorizontalAlignment" Value="Stretch" />
        <Setter Property="VerticalAlignment" Value="Stretch" />
        <Setter Property="HorizontalContentAlignment" Value="Stretch" />
        <Setter Property="VerticalContentAlignment" Value="Stretch" />
        <Setter Property="IsTabStop" Value="True" />
        <Setter Property="UseSystemFocusVisuals" Value="True" />
        <Setter Property="FocusVisualMargin" Value="-2" />
        <Setter Property="FocusVisualSecondaryThickness" Value="0" />
        <Setter Property="CornerRadius" Value="{StaticResource ControlLargeCornerRadius}" />
        <Setter Property="Template">
            <Setter.Value>
                <ControlTemplate TargetType="ExperienceToggleButton">
                    <ControlTemplate.Template>
                        <TaskListButtonPanel x:Name="ExperienceToggleButtonRootPanel" Background="#00FFFFFF">
                            <TaskListButtonPanel.Children>
                                <Border x:Name="BackgroundElement"
                                    Background="{ThemeResource ShellTaskbarItemFillColorTransparentBrush}"
                                    BorderBrush="#00FFFFFF"
                                    BorderThickness="{StaticResource TaskbarButtonOutlineThickness}"
                                    CornerRadius="{TemplateBinding CornerRadius}"
                                    IsTemplateFocusTarget="True"
                                    IsHitTestVisible="False">
                                    <Border.BackgroundTransition>
                                        <BrushTransition Duration="0:0:0.083" />
                                    </Border.BackgroundTransition>
                                </Border>
                                <AnimatedVisualPlayer x:Name="Icon"
                                    HorizontalAlignment="Center"
                                    VerticalAlignment="Center"
                                    AutoPlay="False"
                                    IsHitTestVisible="False"
                                    Stretch="Uniform"
                                    AnimationOptimization="Resources"
                                    AccessibilityView="Raw" />
                                <DeferredElement>
                                    <Badge x:Name="BadgeControl" VerticalAlignment="Center" HorizontalAlignment="Center" AccessibilityView="Raw" IsHitTestVisible="False" />
                                </DeferredElement>
                            </TaskListButtonPanel.Children>
                            <TaskListButtonPanel.VisualStateGroups>
                                <VisualStateGroup x:Name="CommonStates">
                                    <VisualStateGroup.States>
                                        <VisualState x:Name="InactiveNormal" />
                                        <VisualState x:Name="InactivePointerOver">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorAltSecondaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorSecondaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="InactivePressed">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorTertiaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemStrokeColorQuinaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActiveNormal">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorSecondaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorPrimaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActivePointerOver">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorPrimaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorSecondaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActivePressed">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorTertiaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemStrokeColorQuinaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                    </VisualStateGroup.States>
                                </VisualStateGroup>
                            </TaskListButtonPanel.VisualStateGroups>
                        </TaskListButtonPanel>
                    </ControlTemplate.Template>
                </ControlTemplate>
            </Setter.Value>
        </Setter>
    </Style>
    <Style TargetType="Beacon">
        <Setter Property="IsEnabled" Value="False" />
        <Setter Property="Template">
            <Setter.Value>
                <ControlTemplate TargetType="Beacon">
                    <ControlTemplate.Template>
                        <Rectangle x:Name="BackgroundRect">
                            <Rectangle.Fill>
                                <SolidColorBrush x:Name="BackgroundOverrideBrush" />
                            </Rectangle.Fill>
                            <Rectangle.VisualStateGroups>
                                <VisualStateGroup x:Name="BackgroundStates">
                                    <VisualStateGroup.States>
                                        <VisualState x:Name="OverrideBackground" />
                                        <VisualState x:Name="NormalBackground">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundRect.Fill" Value="{ThemeResource BadgeBackgroundBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                    </VisualStateGroup.States>
                                </VisualStateGroup>
                            </Rectangle.VisualStateGroups>
                        </Rectangle>
                    </ControlTemplate.Template>
                </ControlTemplate>
            </Setter.Value>
        </Setter>
    </Style>
    <Style TargetType="Badge">
        <Setter Property="IsEnabled" Value="False" />
        <Setter Property="Template">
            <Setter.Value>
                <ControlTemplate TargetType="Badge">
                    <ControlTemplate.Template>
                        <Grid>
                            <Grid.Children>
                                <Rectangle x:Name="BackgroundRect">
                                    <Rectangle.Fill>
                                        <SolidColorBrush x:Name="BackgroundOverrideBrush" />
                                    </Rectangle.Fill>
                                </Rectangle>
                                <TextBlock x:Name="BadgeText"
                                    FontSize="{StaticResource SmallGlyphBadgeFontSize}"
                                    FontFamily="{StaticResource SymbolThemeFontFamily}"
                                    HorizontalAlignment="Stretch"
                                    TextAlignment="Center"
                                    VerticalAlignment="Center"
                                    TextLineBounds="Tight"
                                    OpticalMarginAlignment="TrimSideBearings">
                                    <TextBlock.Foreground>
                                        <SolidColorBrush x:Name="ForegroundOverrideBrush" />
                                    </TextBlock.Foreground>
                                </TextBlock>
                            </Grid.Children>
                            <Grid.VisualStateGroups>
                                <VisualStateGroup x:Name="BadgeKinds">
                                    <VisualStateGroup.States>
                                        <VisualState x:Name="SmallGlyph" />
                                        <VisualState x:Name="LargeGlyph">
                                            <VisualState.Setters>
                                                <Setter Target="BadgeText.FontSize" Value="{StaticResource LargeGlyphBadgeFontSize}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="SmallNumeric">
                                            <VisualState.Setters>
                                                <Setter Target="BadgeText.Padding" Value="4,0" />
                                                <Setter Target="BadgeText.FontSize" Value="{StaticResource SmallNumericBadgeFontSize}" />
                                                <Setter Target="BadgeText.FontFamily" Value="XamlAutoFontFamily" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="LargeNumeric">
                                            <VisualState.Setters>
                                                <Setter Target="BadgeText.Padding" Value="4,0" />
                                                <Setter Target="BadgeText.FontSize" Value="{StaticResource LargeNumericBadgeFontSize}" />
                                                <Setter Target="BadgeText.FontFamily" Value="XamlAutoFontFamily" />
                                            </VisualState.Setters>
                                        </VisualState>
                                    </VisualStateGroup.States>
                                </VisualStateGroup>
                                <VisualStateGroup x:Name="BackgroundStates">
                                    <VisualStateGroup.States>
                                        <VisualState x:Name="OverrideBackground" />
                                        <VisualState x:Name="NormalBackground">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundRect.Fill" Value="{ThemeResource BadgeBackgroundBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                    </VisualStateGroup.States>
                                </VisualStateGroup>
                                <VisualStateGroup x:Name="ForegroundStates">
                                    <VisualStateGroup.States>
                                        <VisualState x:Name="OverrideForeground" />
                                        <VisualState x:Name="NormalForeground">
                                            <VisualState.Setters>
                                                <Setter Target="BadgeText.Foreground" Value="{ThemeResource BadgeForegroundBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                    </VisualStateGroup.States>
                                </VisualStateGroup>
                            </Grid.VisualStateGroups>
                        </Grid>
                    </ControlTemplate.Template>
                </ControlTemplate>
            </Setter.Value>
        </Setter>
    </Style>
    <Style TargetType="AugmentedEntryPointButton">
        <Setter Property="HorizontalAlignment" Value="Stretch" />
        <Setter Property="VerticalAlignment" Value="Stretch" />
        <Setter Property="HorizontalContentAlignment" Value="Stretch" />
        <Setter Property="VerticalContentAlignment" Value="Stretch" />
        <Setter Property="IsTabStop" Value="True" />
        <Setter Property="UseSystemFocusVisuals" Value="True" />
        <Setter Property="FocusVisualMargin" Value="-2" />
        <Setter Property="FocusVisualSecondaryThickness" Value="0" />
        <Setter Property="CornerRadius" Value="{StaticResource ControlLargeCornerRadius}" />
        <Setter Property="Template">
            <Setter.Value>
                <ControlTemplate TargetType="AugmentedEntryPointButton">
                    <ControlTemplate.Template>
                        <TaskListButtonPanel x:Name="ExperienceToggleButtonRootPanel" Background="#00FFFFFF">
                            <TaskListButtonPanel.Children>
                                <Border x:Name="BackgroundElement"
                                    Background="{ThemeResource ShellTaskbarItemFillColorTransparentBrush}"
                                    BorderBrush="#00FFFFFF"
                                    BorderThickness="{StaticResource TaskbarButtonOutlineThickness}"
                                    CornerRadius="{TemplateBinding CornerRadius}"
                                    IsTemplateFocusTarget="True"
                                    IsHitTestVisible="False">
                                    <Border.BackgroundTransition>
                                        <BrushTransition Duration="0:0:0.083" />
                                    </Border.BackgroundTransition>
                                </Border>
                                <AnimatedVisualPlayer x:Name="Icon"
                                    HorizontalAlignment="Center"
                                    VerticalAlignment="Center"
                                    AutoPlay="False"
                                    IsHitTestVisible="False"
                                    Stretch="Uniform"
                                    AnimationOptimization="Resources"
                                    AccessibilityView="Raw" />
                                <Grid x:Name="AugmentedEntryPointContentGrid" HorizontalAlignment="Center" VerticalAlignment="Center" IsHitTestVisible="False" />
                            </TaskListButtonPanel.Children>
                            <TaskListButtonPanel.VisualStateGroups>
                                <VisualStateGroup x:Name="CommonStates">
                                    <VisualStateGroup.States>
                                        <VisualState x:Name="InactiveNormal" />
                                        <VisualState x:Name="InactivePointerOver">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorAltSecondaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorSecondaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="InactivePressed">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorTertiaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemStrokeColorQuinaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActiveNormal">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorSecondaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorPrimaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActivePointerOver">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorPrimaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemGradientStrokeColorSecondaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                        <VisualState x:Name="ActivePressed">
                                            <VisualState.Setters>
                                                <Setter Target="BackgroundElement.Background" Value="{ThemeResource ShellTaskbarItemFillColorTertiaryBrush}" />
                                                <Setter Target="BackgroundElement.BorderBrush" Value="{ThemeResource ShellTaskbarItemStrokeColorQuinaryBrush}" />
                                            </VisualState.Setters>
                                        </VisualState>
                                    </VisualStateGroup.States>
                                </VisualStateGroup>
                            </TaskListButtonPanel.VisualStateGroups>
                        </TaskListButtonPanel>
                    </ControlTemplate.Template>
                </ControlTemplate>
            </Setter.Value>
        </Setter>
    </Style>
    <Style TargetType="AepAnimatedIcon">
        <Setter Property="HorizontalAlignment" Value="Stretch" />
        <Setter Property="VerticalAlignment" Value="Stretch" />
        <Setter Property="HorizontalContentAlignment" Value="Stretch" />
        <Setter Property="VerticalContentAlignment" Value="Stretch" />
        <Setter Property="IsTabStop" Value="False" />
        <Setter Property="Template">
            <Setter.Value>
                <ControlTemplate TargetType="AepAnimatedIcon">
                    <ControlTemplate.Template>
                        <Grid x:Name="AepAnimatedIconGrid" />
                    </ControlTemplate.Template>
                </ControlTemplate>
            </Setter.Value>
        </Setter>
    </Style>
</ResourceDictionary>
