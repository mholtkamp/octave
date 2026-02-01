#if EDITOR

#include "GeneralModule.h"
#include "../JsonSettings.h"

#include "document.h"
#include "imgui.h"

DEFINE_PREFERENCES_MODULE(GeneralModule, "General", "")

GeneralModule::GeneralModule()
{
}

GeneralModule::~GeneralModule()
{
}

void GeneralModule::Render()
{
    bool changed = false;

    if (ImGui::Checkbox("Auto-save Settings", &mAutoSave))
    {
        changed = true;
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Automatically save settings when closing the preferences window.");

    ImGui::Spacing();

    if (ImGui::SliderInt("Recent Projects Limit", &mRecentProjectsLimit, 1, 20))
    {
        changed = true;
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Maximum number of recent projects to remember.");

    ImGui::Spacing();

    if (ImGui::Checkbox("Show Welcome Screen", &mShowWelcomeScreen))
    {
        changed = true;
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Show the welcome screen when opening the editor.");

    if (changed)
    {
        SetDirty(true);
    }
}

void GeneralModule::LoadSettings(const rapidjson::Document& doc)
{
    mAutoSave = JsonSettings::GetBool(doc, "autoSave", true);
    mRecentProjectsLimit = JsonSettings::GetInt(doc, "recentProjectsLimit", 10);
    mShowWelcomeScreen = JsonSettings::GetBool(doc, "showWelcomeScreen", true);
}

void GeneralModule::SaveSettings(rapidjson::Document& doc)
{
    JsonSettings::SetBool(doc, "autoSave", mAutoSave);
    JsonSettings::SetInt(doc, "recentProjectsLimit", mRecentProjectsLimit);
    JsonSettings::SetBool(doc, "showWelcomeScreen", mShowWelcomeScreen);
}

#endif
