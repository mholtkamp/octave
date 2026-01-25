#if EDITOR

#include "ThemeModule.h"
#include "../../JsonSettings.h"
#include "EditorTheme.h"

#include "document.h"
#include "imgui.h"
#include "System/System.h"
#include "Log.h"
#include <AssetManager.h>

DEFINE_PREFERENCES_MODULE(ThemeModule, "Theme", "Appearance")

ThemeModule::ThemeModule()
{
    RefreshAvailableFonts();
}

ThemeModule::~ThemeModule()
{
}

void ThemeModule::RefreshAvailableFonts()
{
    mAvailableFonts.clear();
    mAvailableFonts = AssetManager::Get()->GetAvailableFontFiles();
    mAvailableFonts.push_back("Default");


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
    std::string themeName = JsonSettings::GetString(doc, "theme", "Dark");
    mCurrentTheme = EditorTheme::GetThemeTypeFromName(themeName);
    mSelectedThemeIndex = static_cast<int>(mCurrentTheme);

    mCurrentFont = JsonSettings::GetString(doc, "font", "Default");

    // Refresh fonts and find the index
    RefreshAvailableFonts();

    // Apply theme on load
    EditorTheme::ApplyTheme(mCurrentTheme);
}

void ThemeModule::SaveSettings(rapidjson::Document& doc)
{
    JsonSettings::SetString(doc, "theme", EditorTheme::GetThemeName(mCurrentTheme));
    JsonSettings::SetString(doc, "font", mCurrentFont);
    LoadFont();
}

#endif
