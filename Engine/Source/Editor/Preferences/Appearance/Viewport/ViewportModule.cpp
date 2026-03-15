#if EDITOR

#include "ViewportModule.h"
#include "../../JsonSettings.h"
#include "../../PreferencesManager.h"

#include "document.h"
#include "imgui.h"
#include "Renderer.h"
#include "Engine.h"
#include "../../../Grid.h"

DEFINE_PREFERENCES_MODULE(ViewportModule, "Viewport", "Appearance")

ViewportModule* ViewportModule::sInstance = nullptr;
bool ViewportModule::sSyncingGridState = false;

ViewportModule::ViewportModule()
{
    sInstance = this;
}

ViewportModule::~ViewportModule()
{
    if (sInstance == this)
    {
        sInstance = nullptr;
    }
}

void ViewportModule::Render()
{
    bool changed = false;

    ImGui::Text("Menu Bar Padding");
    if (ImGui::SliderFloat("##MenuBarPadding", &mMenuBarPadding, 1.0f, 12.0f, "%.0f px"))
    {
        changed = true;
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Set the vertical padding of the main menu bar.");

    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Background Color");
    if (ImGui::ColorEdit4("##BackgroundColor", &mBackgroundColor.x, ImGuiColorEditFlags_NoInputs))
    {
        changed = true;
        ApplyBackgroundColorToRenderer();
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Set the viewport background color.");

    ImGui::Spacing();

    if (ImGui::Checkbox("Show Grid", &mShowGrid))
    {
        changed = true;
        ApplyGridVisibility();
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Toggle grid visibility in the viewport.");

    ImGui::Spacing();

    ImGui::BeginDisabled(!mShowGrid);
    {
        ImGui::Text("Grid Color");
        if (ImGui::ColorEdit4("##GridColor", &mGridColor.x, ImGuiColorEditFlags_NoInputs))
        {
            changed = true;
            SetGridColor(mGridColor);
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Set the grid line color.");

        ImGui::Spacing();

        // ImGui::Text("Grid Size");
        // if (ImGui::SliderFloat("##GridSize", &mGridSize, 0.1f, 10.0f, "%.1f"))
        // {
        //     changed = true;
        // }
        // if (ImGui::IsItemHovered()) ImGui::SetTooltip("Set the spacing between grid lines.");
    }
    ImGui::EndDisabled();

    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Selected Color");
    if (ImGui::ColorEdit4("##SelectedColor", &mSelectedColor.x, ImGuiColorEditFlags_AlphaBar))
    {
        changed = true;
        ApplySelectedOverlay();
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Set the selection overlay color and opacity.");

    ImGui::Spacing();

    ImGui::Text("Checker Size");
    if (ImGui::SliderFloat("##SelectedCheckerSize", &mSelectedCheckerSize, 1.0f, 32.0f, "%.0f px"))
    {
        changed = true;
        ApplySelectedOverlay();
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Set the size of the selection checker pattern in pixels.");

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
    mSelectedColor = JsonSettings::GetVec4(doc, "selectedColor", glm::vec4(0.2f, 0.1f, 1.0f, 0.6f));
    mSelectedCheckerSize = JsonSettings::GetFloat(doc, "selectedCheckerSize", 8.0f);
    mMenuBarPadding = JsonSettings::GetFloat(doc, "menuBarPadding", 8.0f);

    ApplyBackgroundColorToRenderer();
    SetGridColor(mGridColor);
    ApplyGridVisibility();
    ApplySelectedOverlay();
}

void ViewportModule::SaveSettings(rapidjson::Document& doc)
{
    JsonSettings::SetVec4(doc, "backgroundColor", mBackgroundColor);
    JsonSettings::SetBool(doc, "showGrid", mShowGrid);
    JsonSettings::SetVec4(doc, "gridColor", mGridColor);
    JsonSettings::SetFloat(doc, "gridSize", mGridSize);
    JsonSettings::SetVec4(doc, "selectedColor", mSelectedColor);
    JsonSettings::SetFloat(doc, "selectedCheckerSize", mSelectedCheckerSize);
    JsonSettings::SetFloat(doc, "menuBarPadding", mMenuBarPadding);
}

void ViewportModule::ApplyBackgroundColorToRenderer() const
{
    if (!IsPlayingInEditor())
    {
        Renderer::Get()->SetClearColor(mBackgroundColor);
    }
}

ViewportModule* ViewportModule::Get()
{
    return sInstance;
}

void ViewportModule::HandleExternalGridToggle(bool enabled)
{
    if (sSyncingGridState)
    {
        return;
    }

    ViewportModule* instance = Get();
    if (instance == nullptr)
    {
        return;
    }

    if (instance->mShowGrid != enabled)
    {
        instance->mShowGrid = enabled;
        instance->SetDirty(true);

        PreferencesManager* prefs = PreferencesManager::Get();
        if (prefs)
        {
            prefs->SaveModule(instance);
        }
    }
}

void ViewportModule::ApplySelectedOverlay() const
{
    Renderer::Get()->SetSelectedColor(mSelectedColor);
    Renderer::Get()->SetSelectedCheckerSize(mSelectedCheckerSize);
}

void ViewportModule::ApplyGridVisibility()
{
    sSyncingGridState = true;
    EnableGrid(mShowGrid);
    sSyncingGridState = false;
}

#endif
