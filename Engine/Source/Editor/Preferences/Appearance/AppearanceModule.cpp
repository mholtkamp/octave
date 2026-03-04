#if EDITOR

#include "AppearanceModule.h"
#include "../JsonSettings.h"

#include "document.h"
#include "imgui.h"
#include "imgui_dock.h"

DEFINE_PREFERENCES_MODULE(AppearanceModule, "Appearance", "")

AppearanceModule::AppearanceModule()
{
    // ApplyTabRounding will be called after LoadSettings
}

AppearanceModule::~AppearanceModule()
{
}

void AppearanceModule::Render()
{
    bool changed = false;

    ImGui::Text("Panel Tab Rounding");

    ImGui::Text("Left");
    ImGui::SameLine();
    if (ImGui::SliderFloat("##TabRoundingLeft", &mTabRoundingLeft, 0.0f, 15.0f, "%.1f"))
    {
        changed = true;
        ApplyTabRounding();
    }

    ImGui::Text("Right");
    ImGui::SameLine();
    if (ImGui::SliderFloat("##TabRoundingRight", &mTabRoundingRight, 0.0f, 15.0f, "%.1f"))
    {
        changed = true;
        ApplyTabRounding();
    }

    if (changed)
    {
        SetDirty(true);
    }
}

void AppearanceModule::LoadSettings(const rapidjson::Document& doc)
{
    mTabRoundingLeft = JsonSettings::GetFloat(doc, "tabRoundingLeft", 0.0f);
    mTabRoundingRight = JsonSettings::GetFloat(doc, "tabRoundingRight", 0.0f);
    ApplyTabRounding();
}

void AppearanceModule::SaveSettings(rapidjson::Document& doc)
{
    JsonSettings::SetFloat(doc, "tabRoundingLeft", mTabRoundingLeft);
    JsonSettings::SetFloat(doc, "tabRoundingRight", mTabRoundingRight);
}

void AppearanceModule::ApplyTabRounding()
{
    ImGui::SetDockTabRounding(mTabRoundingLeft, mTabRoundingRight);
}

#endif
