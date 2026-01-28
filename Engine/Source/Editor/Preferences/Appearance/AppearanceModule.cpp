#if EDITOR

#include "AppearanceModule.h"
#include "../JsonSettings.h"

#include "document.h"
#include "imgui.h"

DEFINE_PREFERENCES_MODULE(AppearanceModule, "Appearance", "")

AppearanceModule::AppearanceModule()
{
}

AppearanceModule::~AppearanceModule()
{
}

void AppearanceModule::Render()
{
    ImGui::TextDisabled("Theme options coming in a future update.");
    ImGui::TextDisabled("Interface Scale can be adjusted via View > Interface Scale.");
}

void AppearanceModule::LoadSettings(const rapidjson::Document& doc)
{
    // Future theme settings will be loaded here
}

void AppearanceModule::SaveSettings(rapidjson::Document& doc)
{
    // Future theme settings will be saved here
}

#endif
