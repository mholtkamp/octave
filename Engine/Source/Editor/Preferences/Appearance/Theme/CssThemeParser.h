#pragma once

#if EDITOR

#include <string>
#include <unordered_map>
#include "imgui.h"

struct CssThemeData
{
    std::unordered_map<int, ImVec4> Colors;

    // Style overrides with flags for partial themes
    float Alpha = 1.0f;                    bool hasAlpha = false;
    float DisabledAlpha = 1.0f;            bool hasDisabledAlpha = false;
    ImVec2 WindowPadding;                  bool hasWindowPadding = false;
    float WindowRounding = 0.0f;           bool hasWindowRounding = false;
    float WindowBorderSize = 1.0f;         bool hasWindowBorderSize = false;
    ImVec2 WindowMinSize;                  bool hasWindowMinSize = false;
    ImVec2 WindowTitleAlign;               bool hasWindowTitleAlign = false;
    float ChildRounding = 0.0f;            bool hasChildRounding = false;
    float ChildBorderSize = 1.0f;          bool hasChildBorderSize = false;
    float PopupRounding = 0.0f;            bool hasPopupRounding = false;
    float PopupBorderSize = 1.0f;          bool hasPopupBorderSize = false;
    ImVec2 FramePadding;                   bool hasFramePadding = false;
    float FrameRounding = 0.0f;            bool hasFrameRounding = false;
    float FrameBorderSize = 0.0f;          bool hasFrameBorderSize = false;
    ImVec2 ItemSpacing;                    bool hasItemSpacing = false;
    ImVec2 ItemInnerSpacing;               bool hasItemInnerSpacing = false;
    ImVec2 CellPadding;                    bool hasCellPadding = false;
    float IndentSpacing = 21.0f;           bool hasIndentSpacing = false;
    float ColumnsMinSpacing = 6.0f;        bool hasColumnsMinSpacing = false;
    float ScrollbarSize = 14.0f;           bool hasScrollbarSize = false;
    float ScrollbarRounding = 9.0f;        bool hasScrollbarRounding = false;
    float GrabMinSize = 10.0f;             bool hasGrabMinSize = false;
    float GrabRounding = 0.0f;             bool hasGrabRounding = false;
    float TabRounding = 4.0f;              bool hasTabRounding = false;
    float TabBorderSize = 0.0f;            bool hasTabBorderSize = false;
    ImVec2 ButtonTextAlign;                bool hasButtonTextAlign = false;
    ImVec2 SelectableTextAlign;            bool hasSelectableTextAlign = false;

    // Dock tab text color (separate from ImGuiCol_Text)
    ImVec4 DockTabTextColor;               bool hasDockTabTextColor = false;

    // Dock splitter / tab bar overrides
    ImVec4 DockSplitterColor;              bool hasDockSplitterColor = false;
    ImVec4 DockSplitterHoverColor;         bool hasDockSplitterHoverColor = false;
    ImVec4 DockTabBarBg;                   bool hasDockTabBarBg = false;

    // Per-panel background overrides
    ImVec4 PanelAssetsBg;                  bool hasPanelAssetsBg = false;
    ImVec4 PanelSceneBg;                   bool hasPanelSceneBg = false;
    ImVec4 PanelPropertiesBg;              bool hasPanelPropertiesBg = false;
    ImVec4 PanelDebugLogBg;                bool hasPanelDebugLogBg = false;
};

struct SelectorMapping
{
    const char* selector;
    const char* property;
    int imguiCol;
};

namespace CssThemeParser
{
    // Parse a CSS file into theme data
    bool ParseFile(const std::string& filePath, CssThemeData& outTheme);

    // Parse a CSS string into theme data
    bool ParseString(const std::string& css, CssThemeData& outTheme);

    // Apply parsed theme data to ImGui (uses StyleColorsDark as base)
    void ApplyTheme(const CssThemeData& themeData);

    // Color parsing utilities
    bool ParseHexColor(const std::string& hex, ImVec4& outColor);
    bool ParseRgbColor(const std::string& rgb, ImVec4& outColor);
    bool ParseRgbaColor(const std::string& rgba, ImVec4& outColor);
    float ParsePxValue(const std::string& value);

    // Selector mapping accessors (for Theme Editor reverse-lookup)
    const SelectorMapping* GetSelectorMappings();
    int GetSelectorMappingCount();

    // Panel background color accessors
    bool GetPanelAssetsBg(ImVec4& outColor);
    bool GetPanelSceneBg(ImVec4& outColor);
    bool GetPanelPropertiesBg(ImVec4& outColor);
    bool GetPanelDebugLogBg(ImVec4& outColor);
    void SetPanelColors(const CssThemeData& data);
    void ClearPanelColors();
}

#endif
