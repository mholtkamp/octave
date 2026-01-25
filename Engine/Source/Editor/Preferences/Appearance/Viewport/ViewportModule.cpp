#if EDITOR

#include "ViewportModule.h"
#include "../../JsonSettings.h"

#include "document.h"
#include "imgui.h"
#include "Renderer.h"

DEFINE_PREFERENCES_MODULE(ViewportModule, "Viewport", "Appearance")

ViewportModule::ViewportModule()
{
}

ViewportModule::~ViewportModule()
{
}

void ViewportModule::Render()
{
    bool changed = false;

    ImGui::Text("Background Color");
    if (ImGui::ColorEdit4("##BackgroundColor", &mBackgroundColor.x, ImGuiColorEditFlags_NoInputs))
    {
        changed = true;
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Set the viewport background color.");

    ImGui::Spacing();

    if (ImGui::Checkbox("Show Grid", &mShowGrid))
    {
        changed = true;
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Toggle grid visibility in the viewport.");

    ImGui::Spacing();

    ImGui::BeginDisabled(!mShowGrid);
    {
        ImGui::Text("Grid Color");
        if (ImGui::ColorEdit4("##GridColor", &mGridColor.x, ImGuiColorEditFlags_NoInputs))
        {
            changed = true;
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Set the grid line color.");

        ImGui::Spacing();

        ImGui::Text("Grid Size");
        if (ImGui::SliderFloat("##GridSize", &mGridSize, 0.1f, 10.0f, "%.1f"))
        {
            changed = true;
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Set the spacing between grid lines.");
    }
    ImGui::EndDisabled();

    if (changed)
    {
        SetDirty(true);
    }
}

void ViewportModule::LoadSettings(const rapidjson::Document& doc)
{
    mBackgroundColor = JsonSettings::GetVec4(doc, "backgroundColor", glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
    mShowGrid = JsonSettings::GetBool(doc, "showGrid", true);
    mGridColor = JsonSettings::GetVec4(doc, "gridColor", glm::vec4(0.3f, 0.3f, 0.3f, 1.0f));
    mGridSize = JsonSettings::GetFloat(doc, "gridSize", 1.0f);
}

void ViewportModule::SaveSettings(rapidjson::Document& doc)
{
    JsonSettings::SetVec4(doc, "backgroundColor", mBackgroundColor);
    JsonSettings::SetBool(doc, "showGrid", mShowGrid);
    JsonSettings::SetVec4(doc, "gridColor", mGridColor);
    JsonSettings::SetFloat(doc, "gridSize", mGridSize);
}

#endif
