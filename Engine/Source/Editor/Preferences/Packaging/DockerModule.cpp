#if EDITOR

#include "DockerModule.h"
#include "../JsonSettings.h"

#include "document.h"
#include "imgui.h"

#include <cstring>

DEFINE_PREFERENCES_MODULE(DockerModule, "Docker", "Packaging")

DockerModule::DockerModule()
{
}

DockerModule::~DockerModule()
{
}

void DockerModule::Render()
{
    bool changed = false;

    ImGui::Text("Docker Image");
    ImGui::Separator();

    ImGui::TextDisabled("The Docker image used for building games.");
    ImGui::Spacing();

    ImGui::Text("Image:");
    ImGui::SetNextItemWidth(-1);
    char imageBuffer[256];
    strncpy(imageBuffer, mDockerImage.c_str(), sizeof(imageBuffer) - 1);
    imageBuffer[sizeof(imageBuffer) - 1] = '\0';
    if (ImGui::InputText("##DockerImage", imageBuffer, sizeof(imageBuffer)))
    {
        mDockerImage = imageBuffer;
        changed = true;
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Docker image name (e.g., vltmedia/octavegameengine-linux:dev)");
    }

    ImGui::Spacing();
    ImGui::TextDisabled("Default: vltmedia/octavegameengine-linux:dev");

    if (changed)
    {
        SetDirty(true);
    }
}

void DockerModule::LoadSettings(const rapidjson::Document& doc)
{
    mDockerImage = JsonSettings::GetString(doc, "dockerImage", "vltmedia/octavegameengine-linux:dev");
}

void DockerModule::SaveSettings(rapidjson::Document& doc)
{
    JsonSettings::SetString(doc, "dockerImage", mDockerImage);
}

#endif
