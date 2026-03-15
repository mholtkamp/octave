#if EDITOR

#include "GeneralModule.h"
#include "../JsonSettings.h"
#include "Log.h"

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

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Debugging");
    ImGui::Spacing();

    if (ImGui::Checkbox("Show Debug Log In Editor", &mShowDebugInEditor))
    {
        changed = true;
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Show the debug log panel in the editor.");

    ImGui::Spacing();

    if (ImGui::Checkbox("Show Debug Logs In Build", &mShowDebugLogsInBuild))
    {
        SetDebugLogsInBuildEnabled(mShowDebugLogsInBuild);
        changed = true;
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Show debug log messages in the in-game console during builds.");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Build");
    ImGui::Spacing();

    if (ImGui::Checkbox("Check Build Dependencies On Startup", &mCheckBuildDepsOnStartup))
    {
        changed = true;
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Check for missing build tools at startup and show a warning window if any are missing.");

    if (changed)
    {
        SetDirty(true);
    }
}

void GeneralModule::SetCheckBuildDepsOnStartup(bool value)
{
    mCheckBuildDepsOnStartup = value;
    SetDirty(true);
}

void GeneralModule::LoadSettings(const rapidjson::Document& doc)
{
    mAutoSave = JsonSettings::GetBool(doc, "autoSave", true);
    mRecentProjectsLimit = JsonSettings::GetInt(doc, "recentProjectsLimit", 10);
    mShowWelcomeScreen = JsonSettings::GetBool(doc, "showWelcomeScreen", true);
    mShowDebugInEditor = JsonSettings::GetBool(doc, "showDebugInEditor", true);
    mShowDebugLogsInBuild = JsonSettings::GetBool(doc, "showDebugLogsInBuild", true);
    SetDebugLogsInBuildEnabled(mShowDebugLogsInBuild);
    mCheckBuildDepsOnStartup = JsonSettings::GetBool(doc, "checkBuildDepsOnStartup", true);
}

void GeneralModule::SaveSettings(rapidjson::Document& doc)
{
    JsonSettings::SetBool(doc, "autoSave", mAutoSave);
    JsonSettings::SetInt(doc, "recentProjectsLimit", mRecentProjectsLimit);
    JsonSettings::SetBool(doc, "showWelcomeScreen", mShowWelcomeScreen);
    JsonSettings::SetBool(doc, "showDebugInEditor", mShowDebugInEditor);
    JsonSettings::SetBool(doc, "showDebugLogsInBuild", mShowDebugLogsInBuild);
    JsonSettings::SetBool(doc, "checkBuildDepsOnStartup", mCheckBuildDepsOnStartup);
}

#endif
