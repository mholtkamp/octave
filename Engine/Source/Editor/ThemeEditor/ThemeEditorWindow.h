#pragma once

#if EDITOR

#include "imgui.h"
#include <string>

class ThemeEditorWindow
{
public:
    ThemeEditorWindow();
    ~ThemeEditorWindow();

    void Open();
    void Close();
    void Draw();
    void DrawInspectOverlay();

    bool IsOpen() const { return mIsOpen; }
    bool IsInspectModeActive() const { return mIsOpen && mInspectMode; }

private:
    void DrawHeader();
    void DrawColorsTab();
    void DrawStylePropertiesTab();
    void DrawFooter();

    // Color category helpers
    void DrawColorGroup(const char* label, int* colIndices, int count,
                        const char* cssSelector);
    void DrawPanelColorGroup();

    // Save/Export
    void DoSave();
    void DoSaveAs();
    void DoExportCss();

    // Build CssThemeData from working state for save/export
    void BuildThemeData();

    bool mIsOpen = false;
    bool mInspectMode = false;

    // Working copy of style (pushed to ImGui live)
    ImGuiStyle mWorkingStyle;
    // Snapshot for revert on cancel
    ImGuiStyle mBaseStyle;

    // Dock tab text color (not part of ImGuiStyle)
    ImVec4 mDockTabTextColor = ImVec4(1, 1, 1, 1);
    bool mHasDockTabTextColor = false;

    // Dock splitter / tab bar overrides
    ImVec4 mDockSplitterColor = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
    bool mHasDockSplitterColor = false;
    ImVec4 mDockSplitterHoverColor = ImVec4(0.5f, 0.5f, 0.8f, 1.0f);
    bool mHasDockSplitterHoverColor = false;
    ImVec4 mDockTabBarBg = ImVec4(0.1f, 0.1f, 0.15f, 1.0f);
    bool mHasDockTabBarBg = false;

    // Panel background overrides
    ImVec4 mPanelAssetsBg;        bool mHasPanelAssetsBg = false;
    ImVec4 mPanelSceneBg;         bool mHasPanelSceneBg = false;
    ImVec4 mPanelPropertiesBg;    bool mHasPanelPropertiesBg = false;
    ImVec4 mPanelDebugLogBg;      bool mHasPanelDebugLogBg = false;

    // Theme identity
    char mThemeName[64] = "My Theme";
    int mBaseThemeIndex = 0; // 0=Dark, 1=Light, 2=FutureDark, 3=Classic
    int mEditingCustomIndex = -1; // -1 = new theme, >=0 = editing existing

    // Save As popup
    bool mShowSaveAsPopup = false;
    char mSaveAsName[64] = {};
};

ThemeEditorWindow* GetThemeEditorWindow();

#endif
