#if EDITOR

#include "ThemeModule.h"
#include "CssThemeManager.h"
#include "../../JsonSettings.h"
#include "EditorTheme.h"

#include "document.h"
#include "imgui.h"
#include "System/System.h"
#include "../../../FeatureFlags.h"
#include "Log.h"
#include <AssetManager.h>
#include <Engine.h>
#include <cstring>

DEFINE_PREFERENCES_MODULE(ThemeModule, "Theme", "Appearance")

std::string ThemeModule::GetSettingsFilePathStatic()
{
    return JsonSettings::GetPreferencesDirectory() + "/Appearance_Theme.json";
}

std::string ThemeModule::LoadSavedFontPreference()
{
    rapidjson::Document doc;
    std::string settingsPath = GetSettingsFilePathStatic();

    if (JsonSettings::LoadFromFile(settingsPath, doc))
    {
        std::string savedFont = JsonSettings::GetString(doc, "font", "Default");
        if (!savedFont.empty())
        {
            return savedFont;
        }
    }

    const std::string& configFont = GetEngineConfig()->mCurrentFont;
    if (!configFont.empty())
    {
        return configFont;
    }

    return "Default";
}

ThemeModule::ThemeModule()
{
    if (GetFeatureFlagsEditor().mShowTheming) {
    CssThemeManager::Get().LoadThemeList();
    EditorTheme::RefreshThemeNames();
    RefreshAvailableFonts();
    memset(mNewThemeName, 0, sizeof(mNewThemeName));
}
}

ThemeModule::~ThemeModule()
{
}

void ThemeModule::RefreshAvailableFonts()
{
    mAvailableFonts.clear();
    mAvailableFonts.push_back("Default");

    auto FoundAvailableFonts = AssetManager::Get()->GetAvailableFontFiles();
    for (const auto& fontFile : FoundAvailableFonts)
    {
        mAvailableFonts.push_back(fontFile);
    }


    // Scan for .ttf files in Engine/Assets/Fonts
    DirEntry dirEntry;
   
    // Find current font index
    mSelectedFontIndex = 0;
    for (size_t i = 0; i < mAvailableFonts.size(); ++i)
    {
        if (mAvailableFonts[i] == mCurrentFont)
        {
            mSelectedFontIndex = static_cast<int>(i);
            break;
        }
    }

}

void ThemeModule::Render()
{
    bool changed = false;

    // Theme Selection
    ImGui::Text("Theme");

    const std::vector<std::string>& themeNames = EditorTheme::GetThemeNames();
    int builtInCount = static_cast<int>(EditorThemeType::Count);

    // Clamp selected index
    if (mSelectedThemeIndex >= static_cast<int>(themeNames.size()))
        mSelectedThemeIndex = 0;

    if (ImGui::BeginCombo("##ThemeCombo", themeNames[mSelectedThemeIndex].c_str()))
    {
        for (int i = 0; i < static_cast<int>(themeNames.size()); ++i)
        {
            // Add separator between built-in and custom themes
            if (i == builtInCount && CssThemeManager::Get().GetThemeCount() > 0)
                ImGui::Separator();

            bool isSelected = (mSelectedThemeIndex == i);
            if (ImGui::Selectable(themeNames[i].c_str(), isSelected))
            {
                mSelectedThemeIndex = i;

                // Convert flat index to EditorThemeType
                if (i < builtInCount)
                {
                    mCurrentTheme = static_cast<EditorThemeType>(i);
                }
                else
                {
                    mCurrentTheme = MakeCustomThemeType(i - builtInCount);
                }
                changed = true;

                // Apply theme immediately for preview
                EditorTheme::ApplyTheme(mCurrentTheme);
            }
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Select the editor color theme.");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Custom Themes Section
    ImGui::Text("Custom Themes");

    if (ImGui::Button("Import CSS Theme..."))
    {
        std::vector<std::string> filePaths = SYS_OpenFileDialog();
        if (!filePaths.empty())
        {
            mPendingImportPath = filePaths[0];
            mShowImportDialog = true;
            memset(mNewThemeName, 0, sizeof(mNewThemeName));

            // Suggest a name from the filename
            std::string filename = mPendingImportPath;
            size_t lastSlash = filename.find_last_of("/\\");
            if (lastSlash != std::string::npos)
                filename = filename.substr(lastSlash + 1);
            size_t dot = filename.find_last_of('.');
            if (dot != std::string::npos)
                filename = filename.substr(0, dot);
            strncpy(mNewThemeName, filename.c_str(), sizeof(mNewThemeName) - 1);

            ImGui::OpenPopup("Name Custom Theme");
        }
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Import a .css file as a custom editor theme.");

    // Import naming popup
    if (ImGui::BeginPopupModal("Name Custom Theme", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Enter a name for this theme:");
        ImGui::InputText("##ThemeName", mNewThemeName, sizeof(mNewThemeName));

        ImGui::Spacing();

        if (ImGui::Button("Import", ImVec2(120, 0)))
        {
            std::string name = mNewThemeName;
            if (!name.empty())
            {
                if (CssThemeManager::Get().ImportTheme(mPendingImportPath, name))
                {
                    EditorTheme::RefreshThemeNames();

                    // Select the newly imported theme
                    int customIdx = CssThemeManager::Get().GetThemeCount() - 1;
                    mCurrentTheme = MakeCustomThemeType(customIdx);
                    mSelectedThemeIndex = builtInCount + customIdx;
                    EditorTheme::ApplyTheme(mCurrentTheme);
                    changed = true;
                }
            }
            mShowImportDialog = false;
            mPendingImportPath.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            mShowImportDialog = false;
            mPendingImportPath.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // List imported themes with Reimport/Remove buttons
    const auto& customThemes = CssThemeManager::Get().GetThemes();
    if (!customThemes.empty())
    {
        ImGui::Spacing();
        int removeIdx = -1;
        int reimportIdx = -1;

        if (ImGui::BeginTable("##CustomThemes", 3, ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Reimport", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Remove", ImGuiTableColumnFlags_WidthFixed);

            for (int i = 0; i < static_cast<int>(customThemes.size()); ++i)
            {
                ImGui::PushID(i);
                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                ImGui::Text("%s", customThemes[i].Name.c_str());
                if (!customThemes[i].IsValid)
                {
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "(invalid)");
                }

                ImGui::TableNextColumn();
                if (ImGui::SmallButton("Reimport"))
                {
                    reimportIdx = i;
                }

                ImGui::TableNextColumn();
                if (ImGui::SmallButton("Remove"))
                {
                    removeIdx = i;
                }

                ImGui::PopID();
            }
            ImGui::EndTable();
        }

        if (reimportIdx >= 0)
        {
            std::vector<std::string> filePaths = SYS_OpenFileDialog();
            if (!filePaths.empty())
            {
                CssThemeManager::Get().ReimportThemeFromFile(reimportIdx, filePaths[0]);
            }
            else
            {
                // No file selected - just re-parse the stored copy
                CssThemeManager::Get().ReimportTheme(reimportIdx);
            }

            // If this is the currently active theme, reapply it
            if (IsCustomTheme(mCurrentTheme) && GetCustomThemeIndex(mCurrentTheme) == reimportIdx)
            {
                EditorTheme::ApplyTheme(mCurrentTheme);
            }
        }

        if (removeIdx >= 0)
        {
            // If removing the active custom theme, fall back to Dark
            if (IsCustomTheme(mCurrentTheme) && GetCustomThemeIndex(mCurrentTheme) == removeIdx)
            {
                mCurrentTheme = EditorThemeType::Dark;
                mSelectedThemeIndex = 0;
                EditorTheme::ApplyTheme(mCurrentTheme);
                changed = true;
            }
            else if (IsCustomTheme(mCurrentTheme) && GetCustomThemeIndex(mCurrentTheme) > removeIdx)
            {
                // Adjust index since one before us was removed
                int newCustomIdx = GetCustomThemeIndex(mCurrentTheme) - 1;
                mCurrentTheme = MakeCustomThemeType(newCustomIdx);
                mSelectedThemeIndex = builtInCount + newCustomIdx;
            }

            CssThemeManager::Get().RemoveTheme(removeIdx);
            EditorTheme::RefreshThemeNames();
            changed = true;
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Font Selection
    ImGui::Text("Editor Font");

    if (ImGui::BeginCombo("##FontCombo", mAvailableFonts[mSelectedFontIndex].c_str()))
    {
        for (int i = 0; i < static_cast<int>(mAvailableFonts.size()); ++i)
        {
            bool isSelected = (mSelectedFontIndex == i);
            if (ImGui::Selectable(mAvailableFonts[i].c_str(), isSelected))
            {
                mSelectedFontIndex = i;
                mCurrentFont = mAvailableFonts[i];
                changed = true;
            }
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Select the editor font. Requires restart to take effect.");

    ImGui::SameLine();
    if (ImGui::Button("Refresh##Fonts"))
    {
        RefreshAvailableFonts();
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Refresh the list of available fonts from Engine/Assets/Fonts.");

    ImGui::Spacing();
    ImGui::TextDisabled("Note: Font changes require an editor restart.");
    ImGui::TextDisabled("Place .ttf or .otf files in Engine/Assets/Fonts/");

    if (changed)
    {
        SetDirty(true);
    }
}

void ThemeModule::LoadSettings(const rapidjson::Document& doc)
{
    if (!GetFeatureFlagsEditor().mShowTheming) {
        return;
	}

    std::string themeName = JsonSettings::GetString(doc, "theme", "Dark");
    mCurrentTheme = EditorTheme::GetThemeTypeFromName(themeName);

    // Convert theme type to flat index for combo box
    int builtInCount = static_cast<int>(EditorThemeType::Count);
    if (IsCustomTheme(mCurrentTheme))
    {
        mSelectedThemeIndex = builtInCount + GetCustomThemeIndex(mCurrentTheme);
    }
    else
    {
        mSelectedThemeIndex = static_cast<int>(mCurrentTheme);
    }

    std::string fallbackFont = GetEngineConfig()->mCurrentFont.empty() ? "Default" : GetEngineConfig()->mCurrentFont;
    mCurrentFont = JsonSettings::GetString(doc, "font", fallbackFont);
    if (mCurrentFont.empty())
    {
        mCurrentFont = "Default";
    }

    GetMutableEngineConfig()->mCurrentFont = mCurrentFont;

    // Refresh fonts and find the index
    RefreshAvailableFonts();

    // Apply theme on load
    EditorTheme::ApplyTheme(mCurrentTheme);
}

void ThemeModule::SaveSettings(rapidjson::Document& doc)
{
    JsonSettings::SetString(doc, "theme", EditorTheme::GetThemeName(mCurrentTheme));
    JsonSettings::SetString(doc, "font", mCurrentFont);

    GetMutableEngineConfig()->mCurrentFont = mCurrentFont;

    if (GetEngineState()->mProjectDirectory != "")
    {
        WriteEngineConfig();
    }
}

#endif
