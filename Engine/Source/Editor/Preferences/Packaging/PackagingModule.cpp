#if EDITOR

#include "PackagingModule.h"
#include "../JsonSettings.h"

#include "document.h"
#include "imgui.h"

DEFINE_PREFERENCES_MODULE(PackagingModule, "Packaging", "")

PackagingModule::PackagingModule()
{
}

PackagingModule::~PackagingModule()
{
}

void PackagingModule::Render()
{
    ImGui::TextDisabled("Select a subcategory from the left panel to configure packaging settings.");
}

void PackagingModule::LoadSettings(const rapidjson::Document& doc)
{
    // Parent module has no settings of its own
}

void PackagingModule::SaveSettings(rapidjson::Document& doc)
{
    // Parent module has no settings of its own
}

#endif
