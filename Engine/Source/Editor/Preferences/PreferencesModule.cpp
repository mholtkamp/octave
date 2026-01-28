#if EDITOR

#include "PreferencesModule.h"
#include "JsonSettings.h"

#include "document.h"

PreferencesModule::~PreferencesModule()
{
    for (PreferencesModule* sub : mSubModules)
    {
        delete sub;
    }
    mSubModules.clear();
}

void PreferencesModule::LoadSettings(const rapidjson::Document& doc)
{
    // Default implementation does nothing
    // Subclasses override to load their settings
}

void PreferencesModule::SaveSettings(rapidjson::Document& doc)
{
    // Default implementation does nothing
    // Subclasses override to save their settings
}

void PreferencesModule::AddSubModule(PreferencesModule* sub)
{
    mSubModules.push_back(sub);
}

const std::vector<PreferencesModule*>& PreferencesModule::GetSubModules() const
{
    return mSubModules;
}

std::string PreferencesModule::GetFullPath() const
{
    std::string parentPath = GetParentPath();
    if (parentPath.empty())
    {
        return GetName();
    }
    return parentPath + "/" + GetName();
}

std::string PreferencesModule::GetSettingsFilePath() const
{
    std::string fullPath = GetFullPath();

    // Replace '/' with '_' for file naming
    std::string fileName = fullPath;
    for (char& c : fileName)
    {
        if (c == '/')
        {
            c = '_';
        }
    }

    return JsonSettings::GetPreferencesDirectory() + "/" + fileName + ".json";
}

#endif
