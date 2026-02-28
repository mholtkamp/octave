#if EDITOR

#include "ThemeEditorWindow.h"
#include "CssGenerator.h"
#include "Preferences/Appearance/Theme/CssThemeParser.h"
#include "Preferences/Appearance/Theme/CssThemeManager.h"
#include "Preferences/Appearance/Theme/EditorTheme.h"
#include "Preferences/JsonSettings.h"
#include "System/System.h"
#include "Stream.h"
#include "Log.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_dock.h"

#include <cstring>
#include <cmath>

// ---- Singleton ----

static ThemeEditorWindow sThemeEditorWindow;

ThemeEditorWindow* GetThemeEditorWindow()
{
    return &sThemeEditorWindow;
}

// ---- Construction ----

ThemeEditorWindow::ThemeEditorWindow() = default;
ThemeEditorWindow::~ThemeEditorWindow() = default;

// ---- Open / Close ----

void ThemeEditorWindow::Open()
{
    if (mIsOpen)
        return;

    mIsOpen = true;
    mInspectMode = false;
    mEditingCustomIndex = -1;
    mShowSaveAsPopup = false;

    // Snapshot current style
    mWorkingStyle = ImGui::GetStyle();
    mBaseStyle = mWorkingStyle;

    // Determine base theme index from name list
    mBaseThemeIndex = 0;
    strncpy(mThemeName, "My Theme", sizeof(mThemeName));
}

void ThemeEditorWindow::Close()
{
    if (!mIsOpen)
        return;

    mIsOpen = false;
    mInspectMode = false;

    // Revert to base style and clear panel color overrides
    ImGui::GetStyle() = mBaseStyle;
    CssThemeParser::ClearPanelColors();
    ImGui::ClearDockSplitterColors();
    ImGui::ClearDockTabBarBg();
}

// ---- Draw ----

void ThemeEditorWindow::Draw()
{
    if (!mIsOpen)
        return;

    ImGui::SetNextWindowSize(ImVec2(480, 640), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Theme Editor", &mIsOpen, ImGuiWindowFlags_NoCollapse))
    {
        ImGui::End();
        if (!mIsOpen)
            Close();
        return;
    }

    DrawHeader();

    if (ImGui::BeginTabBar("##ThemeEditorTabs"))
    {
        if (ImGui::BeginTabItem("Colors"))
        {
            DrawColorsTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Style Properties"))
        {
            DrawStylePropertiesTab();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    DrawFooter();

    // Save As popup
    if (mShowSaveAsPopup)
    {
        ImGui::OpenPopup("Save Theme As...");
        mShowSaveAsPopup = false;
    }
    if (ImGui::BeginPopupModal("Save Theme As...", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Theme Name:");
        ImGui::InputText("##SaveAsName", mSaveAsName, sizeof(mSaveAsName));

        if (ImGui::Button("Save", ImVec2(120, 0)))
        {
            if (strlen(mSaveAsName) > 0)
            {
                // Generate CSS and write to a temp file, then import
                BuildThemeData();
                CssThemeData themeData;
                // Build the theme data struct from our working state
                themeData.hasPanelAssetsBg = mHasPanelAssetsBg;    themeData.PanelAssetsBg = mPanelAssetsBg;
                themeData.hasPanelSceneBg = mHasPanelSceneBg;      themeData.PanelSceneBg = mPanelSceneBg;
                themeData.hasPanelPropertiesBg = mHasPanelPropertiesBg; themeData.PanelPropertiesBg = mPanelPropertiesBg;
                themeData.hasPanelDebugLogBg = mHasPanelDebugLogBg; themeData.PanelDebugLogBg = mPanelDebugLogBg;
                themeData.hasDockSplitterColor = mHasDockSplitterColor;       themeData.DockSplitterColor = mDockSplitterColor;
                themeData.hasDockSplitterHoverColor = mHasDockSplitterHoverColor; themeData.DockSplitterHoverColor = mDockSplitterHoverColor;
                themeData.hasDockTabBarBg = mHasDockTabBarBg;                 themeData.DockTabBarBg = mDockTabBarBg;

                std::string css = CssGenerator::GenerateCss(
                    mWorkingStyle, mHasDockTabTextColor, mDockTabTextColor, &themeData);

                std::string tempDir = JsonSettings::GetPreferencesDirectory();
                std::string tempPath = tempDir + "/_theme_editor_temp.css";

                Stream stream;
                stream.WriteBytes((const uint8_t*)css.c_str(), (uint32_t)css.size());
                stream.WriteFile(tempPath.c_str());

                CssThemeManager::Get().ImportTheme(tempPath, mSaveAsName);
                EditorTheme::RefreshThemeNames();

                // Now editing this theme
                mEditingCustomIndex = CssThemeManager::Get().GetThemeCount() - 1;
                strncpy(mThemeName, mSaveAsName, sizeof(mThemeName));

                // Update base style to current
                mBaseStyle = mWorkingStyle;

                SYS_RemoveFile(tempPath.c_str());

                LogDebug("Theme Editor: Saved as '%s'", mSaveAsName);
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::End();

    if (!mIsOpen)
        Close();
}

// ---- Header ----

void ThemeEditorWindow::DrawHeader()
{
    // Base theme selector
    const char* baseThemes[] = { "Dark", "Light", "Future Dark", "Classic" };
    ImGui::Text("Base:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120);
    if (ImGui::Combo("##BaseTheme", &mBaseThemeIndex, baseThemes, IM_ARRAYSIZE(baseThemes)))
    {
        // Apply base theme to working style
        EditorThemeType type = static_cast<EditorThemeType>(mBaseThemeIndex);
        EditorTheme::ApplyTheme(type);
        mWorkingStyle = ImGui::GetStyle();
        mHasDockTabTextColor = false;
        mHasDockSplitterColor = false;
        mHasDockSplitterHoverColor = false;
        mHasDockTabBarBg = false;
        ImGui::ClearDockSplitterColors();
        ImGui::ClearDockTabBarBg();
        CssThemeParser::ClearPanelColors();
        mHasPanelAssetsBg = false;
        mHasPanelSceneBg = false;
        mHasPanelPropertiesBg = false;
        mHasPanelDebugLogBg = false;
        mEditingCustomIndex = -1;
    }

    ImGui::SameLine();
    ImGui::Text("Name:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    ImGui::InputText("##ThemeName", mThemeName, sizeof(mThemeName));

    ImGui::Checkbox("Inspect Mode", &mInspectMode);

    ImGui::Separator();
}

// ---- Color Categories ----

// Helper to draw a group of ImGuiCol entries with ColorEdit4 widgets
void ThemeEditorWindow::DrawColorGroup(const char* label, int* colIndices, int count,
                                        const char* cssSelector)
{
    if (!ImGui::CollapsingHeader(label))
        return;

    ImGui::Indent(8.0f);

    // Find matching selector mappings for CSS label display
    const SelectorMapping* mappings = CssThemeParser::GetSelectorMappings();
    int mappingCount = CssThemeParser::GetSelectorMappingCount();

    for (int i = 0; i < count; ++i)
    {
        int col = colIndices[i];
        if (col < 0 || col >= ImGuiCol_COUNT)
            continue;

        // Build label with CSS selector hint
        const char* colName = ImGui::GetStyleColorName(col);
        std::string cssHint;

        for (int m = 0; m < mappingCount; ++m)
        {
            if (mappings[m].imguiCol == col)
            {
                cssHint = std::string(" (") + mappings[m].selector + " { " + mappings[m].property + " })";
                break;
            }
        }

        std::string fullLabel = std::string(colName) + cssHint + "##col" + std::to_string(col);

        if (ImGui::ColorEdit4(fullLabel.c_str(), &mWorkingStyle.Colors[col].x,
            ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf))
        {
            // Push change to live style
            ImGui::GetStyle().Colors[col] = mWorkingStyle.Colors[col];
        }
    }

    ImGui::Unindent(8.0f);
}

void ThemeEditorWindow::DrawPanelColorGroup()
{
    if (!ImGui::CollapsingHeader("Editor Panels"))
        return;

    ImGui::Indent(8.0f);

    auto DrawPanelColor = [](const char* label, const char* css,
                             ImVec4& color, bool& hasColor)
    {
        ImGui::Checkbox((std::string("Override##") + css).c_str(), &hasColor);
        if (hasColor)
        {
            ImGui::SameLine();
            std::string fullLabel = std::string(label) + " (" + css + " { background })##" + css;
            ImGui::ColorEdit4(fullLabel.c_str(), &color.x,
                ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
        }
    };

    DrawPanelColor("Assets", "panel-assets", mPanelAssetsBg, mHasPanelAssetsBg);
    DrawPanelColor("Scene", "panel-scene", mPanelSceneBg, mHasPanelSceneBg);
    DrawPanelColor("Properties", "panel-properties", mPanelPropertiesBg, mHasPanelPropertiesBg);
    DrawPanelColor("Debug Log", "panel-debug-log", mPanelDebugLogBg, mHasPanelDebugLogBg);

    // Push panel colors to global state for live preview
    {
        CssThemeData tempData;
        tempData.hasPanelAssetsBg = mHasPanelAssetsBg;    tempData.PanelAssetsBg = mPanelAssetsBg;
        tempData.hasPanelSceneBg = mHasPanelSceneBg;      tempData.PanelSceneBg = mPanelSceneBg;
        tempData.hasPanelPropertiesBg = mHasPanelPropertiesBg; tempData.PanelPropertiesBg = mPanelPropertiesBg;
        tempData.hasPanelDebugLogBg = mHasPanelDebugLogBg; tempData.PanelDebugLogBg = mPanelDebugLogBg;
        CssThemeParser::SetPanelColors(tempData);
    }

    ImGui::Unindent(8.0f);
}

// ---- Colors Tab ----

void ThemeEditorWindow::DrawColorsTab()
{
    ImGui::BeginChild("##ColorsScroll", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() - 4));

    // Window
    {
        int cols[] = { ImGuiCol_WindowBg, ImGuiCol_ChildBg };
        DrawColorGroup("Window", cols, 2, "window");
    }
    // Popup
    {
        int cols[] = { ImGuiCol_PopupBg };
        DrawColorGroup("Popup", cols, 1, "popup");
    }
    // Text
    {
        int cols[] = { ImGuiCol_Text, ImGuiCol_TextDisabled, ImGuiCol_TextSelectedBg };
        DrawColorGroup("Text", cols, 3, "text");
    }
    // Border
    {
        int cols[] = { ImGuiCol_Border, ImGuiCol_BorderShadow };
        DrawColorGroup("Border", cols, 2, "border");
    }
    // Frame
    {
        int cols[] = { ImGuiCol_FrameBg, ImGuiCol_FrameBgHovered, ImGuiCol_FrameBgActive };
        DrawColorGroup("Frame (Input Fields)", cols, 3, "frame");
    }
    // Title
    {
        int cols[] = { ImGuiCol_TitleBg, ImGuiCol_TitleBgActive, ImGuiCol_TitleBgCollapsed };
        DrawColorGroup("Title", cols, 3, "title");
    }
    // Menu Bar
    {
        int cols[] = { ImGuiCol_MenuBarBg };
        DrawColorGroup("Menu Bar", cols, 1, "menubar");
    }
    // Button
    {
        int cols[] = { ImGuiCol_Button, ImGuiCol_ButtonHovered, ImGuiCol_ButtonActive };
        DrawColorGroup("Button", cols, 3, "button");
    }
    // Header
    {
        int cols[] = { ImGuiCol_Header, ImGuiCol_HeaderHovered, ImGuiCol_HeaderActive };
        DrawColorGroup("Header (Selection)", cols, 3, "header");
    }
    // Tab (with dock tab text color)
    if (ImGui::CollapsingHeader("Tab"))
    {
        ImGui::Indent(8.0f);

        // Dock tab text color override
        ImGui::Checkbox("Override Tab Text Color", &mHasDockTabTextColor);
        if (mHasDockTabTextColor)
        {
            ImGui::SameLine();
            if (ImGui::ColorEdit4("Tab Text (tab { color })##DockTabText", &mDockTabTextColor.x,
                ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf))
            {
                ImGui::SetDockTabTextColor(ImGui::ColorConvertFloat4ToU32(mDockTabTextColor));
            }
        }
        else
        {
            ImGui::ClearDockTabTextColor();
        }

        int tabCols[] = {
            ImGuiCol_Tab, ImGuiCol_TabHovered, ImGuiCol_TabActive,
            ImGuiCol_TabUnfocused, ImGuiCol_TabUnfocusedActive
        };
        const SelectorMapping* mappings = CssThemeParser::GetSelectorMappings();
        int mappingCount = CssThemeParser::GetSelectorMappingCount();

        for (int i = 0; i < 5; ++i)
        {
            int col = tabCols[i];
            const char* colName = ImGui::GetStyleColorName(col);
            std::string cssHint;
            for (int m = 0; m < mappingCount; ++m)
            {
                if (mappings[m].imguiCol == col)
                {
                    cssHint = std::string(" (") + mappings[m].selector + " { " + mappings[m].property + " })";
                    break;
                }
            }
            std::string fullLabel = std::string(colName) + cssHint + "##col" + std::to_string(col);
            if (ImGui::ColorEdit4(fullLabel.c_str(), &mWorkingStyle.Colors[col].x,
                ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf))
            {
                ImGui::GetStyle().Colors[col] = mWorkingStyle.Colors[col];
            }
        }

        ImGui::Unindent(8.0f);
    }
    // Dock (splitter + tab bar)
    if (ImGui::CollapsingHeader("Dock"))
    {
        ImGui::Indent(8.0f);

        // Splitter color
        ImGui::Checkbox("Override Splitter Color", &mHasDockSplitterColor);
        if (mHasDockSplitterColor)
        {
            ImGui::SameLine();
            if (ImGui::ColorEdit4("Splitter (panel-border { color })##DockSplitter", &mDockSplitterColor.x,
                ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf))
            {
                ImGui::SetDockSplitterColor(ImGui::ColorConvertFloat4ToU32(mDockSplitterColor));
            }
        }
        else
        {
            ImGui::ClearDockSplitterColors();
            // Re-apply hover if still enabled
            if (mHasDockSplitterHoverColor)
                ImGui::SetDockSplitterHoverColor(ImGui::ColorConvertFloat4ToU32(mDockSplitterHoverColor));
        }

        // Splitter hover color
        ImGui::Checkbox("Override Splitter Hover Color", &mHasDockSplitterHoverColor);
        if (mHasDockSplitterHoverColor)
        {
            ImGui::SameLine();
            if (ImGui::ColorEdit4("Splitter Hover (panel-border:hover { color })##DockSplitterHover", &mDockSplitterHoverColor.x,
                ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf))
            {
                ImGui::SetDockSplitterHoverColor(ImGui::ColorConvertFloat4ToU32(mDockSplitterHoverColor));
            }
        }
        else
        {
            // Clear just the hover; re-apply base if still enabled
            ImGui::ClearDockSplitterColors();
            if (mHasDockSplitterColor)
                ImGui::SetDockSplitterColor(ImGui::ColorConvertFloat4ToU32(mDockSplitterColor));
        }

        // Tab bar background
        ImGui::Checkbox("Override Tab Bar Background", &mHasDockTabBarBg);
        if (mHasDockTabBarBg)
        {
            ImGui::SameLine();
            if (ImGui::ColorEdit4("Tab Bar Bg (dock-tabbar { background })##DockTabBarBg", &mDockTabBarBg.x,
                ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf))
            {
                ImGui::SetDockTabBarBg(ImGui::ColorConvertFloat4ToU32(mDockTabBarBg));
            }
        }
        else
        {
            ImGui::ClearDockTabBarBg();
        }

        ImGui::Unindent(8.0f);
    }
    // Scrollbar
    {
        int cols[] = { ImGuiCol_ScrollbarBg, ImGuiCol_ScrollbarGrab, ImGuiCol_ScrollbarGrabHovered, ImGuiCol_ScrollbarGrabActive };
        DrawColorGroup("Scrollbar", cols, 4, "scrollbar");
    }
    // Separator
    {
        int cols[] = { ImGuiCol_Separator, ImGuiCol_SeparatorHovered, ImGuiCol_SeparatorActive };
        DrawColorGroup("Separator", cols, 3, "separator");
    }
    // Resize Grip
    {
        int cols[] = { ImGuiCol_ResizeGrip, ImGuiCol_ResizeGripHovered, ImGuiCol_ResizeGripActive };
        DrawColorGroup("Resize Grip", cols, 3, "resize-grip");
    }
    // Checkbox
    {
        int cols[] = { ImGuiCol_CheckMark };
        DrawColorGroup("Checkbox", cols, 1, "checkbox");
    }
    // Slider
    {
        int cols[] = { ImGuiCol_SliderGrab, ImGuiCol_SliderGrabActive };
        DrawColorGroup("Slider", cols, 2, "slider");
    }
    // Plot
    {
        int cols[] = { ImGuiCol_PlotLines, ImGuiCol_PlotLinesHovered, ImGuiCol_PlotHistogram, ImGuiCol_PlotHistogramHovered };
        DrawColorGroup("Plot", cols, 4, "plot");
    }
    // Table
    {
        int cols[] = { ImGuiCol_TableHeaderBg, ImGuiCol_TableBorderStrong, ImGuiCol_TableBorderLight, ImGuiCol_TableRowBg, ImGuiCol_TableRowBgAlt };
        DrawColorGroup("Table", cols, 5, "table");
    }
    // Drag & Drop
    {
        int cols[] = { ImGuiCol_DragDropTarget };
        DrawColorGroup("Drag & Drop", cols, 1, "drag-drop");
    }
    // Navigation
    {
        int cols[] = { ImGuiCol_NavHighlight, ImGuiCol_NavWindowingHighlight, ImGuiCol_NavWindowingDimBg };
        DrawColorGroup("Navigation", cols, 3, "nav");
    }
    // Modal
    {
        int cols[] = { ImGuiCol_ModalWindowDimBg };
        DrawColorGroup("Modal", cols, 1, "modal");
    }

    // Editor Panels
    DrawPanelColorGroup();

    ImGui::EndChild();
}

// ---- Style Properties Tab ----

void ThemeEditorWindow::DrawStylePropertiesTab()
{
    ImGui::BeginChild("##StyleScroll", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() - 4));

    ImGuiStyle& s = mWorkingStyle;
    ImGuiStyle& live = ImGui::GetStyle();
    bool changed = false;

    // Helper macros for brevity
    #define STYLE_FLOAT(label, member, speed, min, max) \
        if (ImGui::DragFloat(label, &s.member, speed, min, max, "%.1f")) { live.member = s.member; changed = true; }
    #define STYLE_VEC2(label, member, speed, min, max) \
        if (ImGui::DragFloat2(label, &s.member.x, speed, min, max, "%.1f")) { live.member = s.member; changed = true; }

    if (ImGui::CollapsingHeader("General"))
    {
        ImGui::Indent(8.0f);
        STYLE_FLOAT("Alpha", Alpha, 0.01f, 0.1f, 1.0f);
        STYLE_FLOAT("Disabled Alpha", DisabledAlpha, 0.01f, 0.0f, 1.0f);
        ImGui::Unindent(8.0f);
    }

    if (ImGui::CollapsingHeader("Window"))
    {
        ImGui::Indent(8.0f);
        STYLE_VEC2("Window Padding", WindowPadding, 0.5f, 0.0f, 40.0f);
        STYLE_FLOAT("Window Rounding", WindowRounding, 0.5f, 0.0f, 16.0f);
        STYLE_FLOAT("Window Border Size", WindowBorderSize, 0.5f, 0.0f, 4.0f);
        STYLE_VEC2("Window Min Size", WindowMinSize, 1.0f, 1.0f, 200.0f);
        STYLE_VEC2("Window Title Align", WindowTitleAlign, 0.01f, 0.0f, 1.0f);
        ImGui::Unindent(8.0f);
    }

    if (ImGui::CollapsingHeader("Child"))
    {
        ImGui::Indent(8.0f);
        STYLE_FLOAT("Child Rounding", ChildRounding, 0.5f, 0.0f, 16.0f);
        STYLE_FLOAT("Child Border Size", ChildBorderSize, 0.5f, 0.0f, 4.0f);
        ImGui::Unindent(8.0f);
    }

    if (ImGui::CollapsingHeader("Popup"))
    {
        ImGui::Indent(8.0f);
        STYLE_FLOAT("Popup Rounding", PopupRounding, 0.5f, 0.0f, 16.0f);
        STYLE_FLOAT("Popup Border Size", PopupBorderSize, 0.5f, 0.0f, 4.0f);
        ImGui::Unindent(8.0f);
    }

    if (ImGui::CollapsingHeader("Frame"))
    {
        ImGui::Indent(8.0f);
        STYLE_VEC2("Frame Padding", FramePadding, 0.5f, 0.0f, 40.0f);
        STYLE_FLOAT("Frame Rounding", FrameRounding, 0.5f, 0.0f, 16.0f);
        STYLE_FLOAT("Frame Border Size", FrameBorderSize, 0.5f, 0.0f, 4.0f);
        ImGui::Unindent(8.0f);
    }

    if (ImGui::CollapsingHeader("Item / Spacing"))
    {
        ImGui::Indent(8.0f);
        STYLE_VEC2("Item Spacing", ItemSpacing, 0.5f, 0.0f, 40.0f);
        STYLE_VEC2("Item Inner Spacing", ItemInnerSpacing, 0.5f, 0.0f, 40.0f);
        STYLE_VEC2("Cell Padding", CellPadding, 0.5f, 0.0f, 40.0f);
        STYLE_FLOAT("Indent Spacing", IndentSpacing, 0.5f, 0.0f, 60.0f);
        STYLE_FLOAT("Columns Min Spacing", ColumnsMinSpacing, 0.5f, 0.0f, 40.0f);
        ImGui::Unindent(8.0f);
    }

    if (ImGui::CollapsingHeader("Scrollbar"))
    {
        ImGui::Indent(8.0f);
        STYLE_FLOAT("Scrollbar Size", ScrollbarSize, 0.5f, 1.0f, 30.0f);
        STYLE_FLOAT("Scrollbar Rounding", ScrollbarRounding, 0.5f, 0.0f, 16.0f);
        ImGui::Unindent(8.0f);
    }

    if (ImGui::CollapsingHeader("Grab"))
    {
        ImGui::Indent(8.0f);
        STYLE_FLOAT("Grab Min Size", GrabMinSize, 0.5f, 1.0f, 30.0f);
        STYLE_FLOAT("Grab Rounding", GrabRounding, 0.5f, 0.0f, 16.0f);
        ImGui::Unindent(8.0f);
    }

    if (ImGui::CollapsingHeader("Tab"))
    {
        ImGui::Indent(8.0f);
        STYLE_FLOAT("Tab Rounding", TabRounding, 0.5f, 0.0f, 16.0f);
        STYLE_FLOAT("Tab Border Size", TabBorderSize, 0.5f, 0.0f, 4.0f);
        ImGui::Unindent(8.0f);
    }

    if (ImGui::CollapsingHeader("Alignment"))
    {
        ImGui::Indent(8.0f);
        STYLE_VEC2("Button Text Align", ButtonTextAlign, 0.01f, 0.0f, 1.0f);
        STYLE_VEC2("Selectable Text Align", SelectableTextAlign, 0.01f, 0.0f, 1.0f);
        ImGui::Unindent(8.0f);
    }

    #undef STYLE_FLOAT
    #undef STYLE_VEC2

    ImGui::EndChild();
}

// ---- Footer ----

void ThemeEditorWindow::DrawFooter()
{
    ImGui::Separator();

    if (ImGui::Button("Save"))
    {
        DoSave();
    }
    ImGui::SameLine();
    if (ImGui::Button("Save As..."))
    {
        DoSaveAs();
    }
    ImGui::SameLine();
    if (ImGui::Button("Export CSS"))
    {
        DoExportCss();
    }
}

// ---- Save / Export ----

void ThemeEditorWindow::BuildThemeData()
{
    // Nothing extra to build - we work directly with mWorkingStyle + member vars
}

void ThemeEditorWindow::DoSave()
{
    if (mEditingCustomIndex >= 0 && mEditingCustomIndex < CssThemeManager::Get().GetThemeCount())
    {
        // Overwrite existing custom theme
        CssThemeData panelData;
        panelData.hasPanelAssetsBg = mHasPanelAssetsBg;    panelData.PanelAssetsBg = mPanelAssetsBg;
        panelData.hasPanelSceneBg = mHasPanelSceneBg;      panelData.PanelSceneBg = mPanelSceneBg;
        panelData.hasPanelPropertiesBg = mHasPanelPropertiesBg; panelData.PanelPropertiesBg = mPanelPropertiesBg;
        panelData.hasPanelDebugLogBg = mHasPanelDebugLogBg; panelData.PanelDebugLogBg = mPanelDebugLogBg;
        panelData.hasDockSplitterColor = mHasDockSplitterColor;       panelData.DockSplitterColor = mDockSplitterColor;
        panelData.hasDockSplitterHoverColor = mHasDockSplitterHoverColor; panelData.DockSplitterHoverColor = mDockSplitterHoverColor;
        panelData.hasDockTabBarBg = mHasDockTabBarBg;                 panelData.DockTabBarBg = mDockTabBarBg;

        std::string css = CssGenerator::GenerateCss(
            mWorkingStyle, mHasDockTabTextColor, mDockTabTextColor, &panelData);

        // Write to a temp file and reimport
        std::string tempDir = JsonSettings::GetPreferencesDirectory();
        std::string tempPath = tempDir + "/_theme_editor_temp.css";

        Stream stream;
        stream.WriteBytes((const uint8_t*)css.c_str(), (uint32_t)css.size());
        stream.WriteFile(tempPath.c_str());

        CssThemeManager::Get().ReimportThemeFromFile(mEditingCustomIndex, tempPath);
        SYS_RemoveFile(tempPath.c_str());

        mBaseStyle = mWorkingStyle;
        LogDebug("Theme Editor: Saved '%s'", mThemeName);
    }
    else
    {
        // No existing theme - redirect to Save As
        DoSaveAs();
    }
}

void ThemeEditorWindow::DoSaveAs()
{
    strncpy(mSaveAsName, mThemeName, sizeof(mSaveAsName));
    mShowSaveAsPopup = true;
}

void ThemeEditorWindow::DoExportCss()
{
    std::string savePath = SYS_SaveFileDialog();
    if (savePath.empty())
        return;

    // Ensure .css extension
    if (savePath.size() < 4 || savePath.substr(savePath.size() - 4) != ".css")
        savePath += ".css";

    CssThemeData panelData;
    panelData.hasPanelAssetsBg = mHasPanelAssetsBg;    panelData.PanelAssetsBg = mPanelAssetsBg;
    panelData.hasPanelSceneBg = mHasPanelSceneBg;      panelData.PanelSceneBg = mPanelSceneBg;
    panelData.hasPanelPropertiesBg = mHasPanelPropertiesBg; panelData.PanelPropertiesBg = mPanelPropertiesBg;
    panelData.hasPanelDebugLogBg = mHasPanelDebugLogBg; panelData.PanelDebugLogBg = mPanelDebugLogBg;
    panelData.hasDockSplitterColor = mHasDockSplitterColor;       panelData.DockSplitterColor = mDockSplitterColor;
    panelData.hasDockSplitterHoverColor = mHasDockSplitterHoverColor; panelData.DockSplitterHoverColor = mDockSplitterHoverColor;
    panelData.hasDockTabBarBg = mHasDockTabBarBg;                 panelData.DockTabBarBg = mDockTabBarBg;

    std::string css = CssGenerator::GenerateCss(
        mWorkingStyle, mHasDockTabTextColor, mDockTabTextColor, &panelData);

    Stream stream;
    stream.WriteBytes((const uint8_t*)css.c_str(), (uint32_t)css.size());
    stream.WriteFile(savePath.c_str());

    LogDebug("Theme Editor: Exported CSS to '%s'", savePath.c_str());
}

// ---- Inspect Overlay ----

void ThemeEditorWindow::DrawInspectOverlay()
{
    if (!mInspectMode || !mIsOpen)
        return;

    ImGuiContext& g = *GImGui;
    ImGuiWindow* hoveredWindow = g.HoveredWindow;

    if (hoveredWindow == nullptr)
        return;

    // Skip if hovering over the Theme Editor window itself
    ImGuiWindow* themeEditorWin = ImGui::FindWindowByName("Theme Editor");
    if (hoveredWindow == themeEditorWin)
        return;

    // Check for parent window match too (popups etc.)
    if (themeEditorWin != nullptr && hoveredWindow->ParentWindow == themeEditorWin)
        return;

    ImVec2 mousePos = ImGui::GetMousePos();

    // Try to identify what ImGuiCol the hovered element uses by matching
    // vertex colors in the hovered window's draw list against style colors.
    ImDrawList* drawList = hoveredWindow->DrawList;
    if (drawList == nullptr || drawList->CmdBuffer.Size == 0)
        return;

    const ImGuiStyle& style = mWorkingStyle;
    const SelectorMapping* mappings = CssThemeParser::GetSelectorMappings();
    int mappingCount = CssThemeParser::GetSelectorMappingCount();

    // Walk draw commands in reverse (topmost first) and find a filled rect
    // whose bounding box contains the mouse and whose color matches a style color.
    std::string foundCss;

    for (int cmdIdx = drawList->CmdBuffer.Size - 1; cmdIdx >= 0 && foundCss.empty(); --cmdIdx)
    {
        const ImDrawCmd& cmd = drawList->CmdBuffer[cmdIdx];
        if (cmd.ElemCount == 0)
            continue;

        unsigned int idxStart = cmd.IdxOffset;
        unsigned int idxEnd = cmd.IdxOffset + cmd.ElemCount;

        // Walk triangles in reverse (6 indices = 1 filled quad)
        for (int idx = (int)idxEnd - 6; idx >= (int)idxStart && foundCss.empty(); idx -= 6)
        {
            if (idx < 0 || idx + 5 >= drawList->IdxBuffer.Size)
                break;

            // Get bounding box of the 6 vertices forming this quad
            float minX = 1e9f, minY = 1e9f, maxX = -1e9f, maxY = -1e9f;
            ImU32 vertColor = 0;

            for (int vi = 0; vi < 6; ++vi)
            {
                ImDrawIdx vertIdx = drawList->IdxBuffer[idx + vi];
                if (vertIdx >= (ImDrawIdx)drawList->VtxBuffer.Size)
                    goto nextQuad;
                const ImDrawVert& vert = drawList->VtxBuffer[vertIdx];
                if (vert.pos.x < minX) minX = vert.pos.x;
                if (vert.pos.y < minY) minY = vert.pos.y;
                if (vert.pos.x > maxX) maxX = vert.pos.x;
                if (vert.pos.y > maxY) maxY = vert.pos.y;
                vertColor = vert.col;
            }

            // Check if mouse is inside this quad's bounding box
            if (mousePos.x >= minX && mousePos.x <= maxX &&
                mousePos.y >= minY && mousePos.y <= maxY)
            {
                // Match vertex color against style colors
                for (int m = 0; m < mappingCount; ++m)
                {
                    int col = mappings[m].imguiCol;
                    if (col < 0 || col >= ImGuiCol_COUNT)
                        continue;

                    ImU32 styleColor = ImGui::ColorConvertFloat4ToU32(style.Colors[col]);
                    if (vertColor == styleColor)
                    {
                        foundCss = std::string(mappings[m].selector) + " { " + mappings[m].property + " }";
                        break;
                    }
                }

                // Check panel background colors
                if (foundCss.empty())
                {
                    if (mHasPanelAssetsBg && vertColor == ImGui::ColorConvertFloat4ToU32(mPanelAssetsBg))
                        foundCss = "panel-assets { background }";
                    else if (mHasPanelSceneBg && vertColor == ImGui::ColorConvertFloat4ToU32(mPanelSceneBg))
                        foundCss = "panel-scene { background }";
                    else if (mHasPanelPropertiesBg && vertColor == ImGui::ColorConvertFloat4ToU32(mPanelPropertiesBg))
                        foundCss = "panel-properties { background }";
                    else if (mHasPanelDebugLogBg && vertColor == ImGui::ColorConvertFloat4ToU32(mPanelDebugLogBg))
                        foundCss = "panel-debug-log { background }";
                }
            }
            nextQuad:;
        }
    }

    // Show tooltip
    if (!foundCss.empty())
    {
        ImGui::BeginTooltip();
        ImGui::Text("CSS: %s", foundCss.c_str());
        ImGui::EndTooltip();
    }
    else
    {
        // Show window name at least
        ImGui::BeginTooltip();
        ImGui::Text("Window: %s", hoveredWindow->Name);
        ImGui::EndTooltip();
    }
}

#endif
