#if EDITOR

#include "ThemeModule.h"
#include "../../JsonSettings.h"
#include "EditorTheme.h"

#include "document.h"
#include "imgui.h"
#include "System/System.h"
#include "../../../FeatureFlags.h"
#include "Log.h"
#include <AssetManager.h>
#include <Engine.h>

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
    RefreshAvailableFonts();
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

    if (ImGui::BeginCombo("##ThemeCombo", themeNames[mSelectedThemeIndex].c_str()))
    {
        for (int i = 0; i < static_cast<int>(themeNames.size()); ++i)
        {
            bool isSelected = (mSelectedThemeIndex == i);
            if (ImGui::Selectable(themeNames[i].c_str(), isSelected))
            {
                mSelectedThemeIndex = i;
                mCurrentTheme = static_cast<EditorThemeType>(i);
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
    mSelectedThemeIndex = static_cast<int>(mCurrentTheme);

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
