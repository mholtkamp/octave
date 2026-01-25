#if EDITOR

#include "PreferencesManager.h"
#include "PreferencesModule.h"
#include "JsonSettings.h"

#include "document.h"

#include "Log.h"

// Include all module headers
#include "General/GeneralModule.h"
#include "Appearance/AppearanceModule.h"
#include "Appearance/Viewport/ViewportModule.h"
#include "Appearance/Theme/ThemeModule.h"

PreferencesManager* PreferencesManager::sInstance = nullptr;

void PreferencesManager::Create()
{
    Destroy();
    sInstance = new PreferencesManager();

    // Register built-in modules
    // Root modules
    sInstance->RegisterModule(new GeneralModule());

    // Appearance module with sub-modules
    AppearanceModule* appearance = new AppearanceModule();
    appearance->AddSubModule(new ThemeModule());
    appearance->AddSubModule(new ViewportModule());
    sInstance->RegisterModule(appearance);

    // Load all settings on startup
    sInstance->LoadAllSettings();
}

void PreferencesManager::Destroy()
{
    if (sInstance != nullptr)
    {
        delete sInstance;
        sInstance = nullptr;
    }
}

PreferencesManager* PreferencesManager::Get()
{
    return sInstance;
}

PreferencesManager::PreferencesManager()
{
}

PreferencesManager::~PreferencesManager()
{
    for (PreferencesModule* module : mRootModules)
    {
        delete module;
    }
    mRootModules.clear();
}

void PreferencesManager::RegisterModule(PreferencesModule* module)
{
    mRootModules.push_back(module);
}

const std::vector<PreferencesModule*>& PreferencesManager::GetRootModules() const
{
    return mRootModules;
}

PreferencesModule* PreferencesManager::FindModule(const std::string& path) const
{
    for (PreferencesModule* module : mRootModules)
    {
        PreferencesModule* found = FindModuleRecursive(module, path);
        if (found != nullptr)
        {
            return found;
        }
    }
    return nullptr;
}

PreferencesModule* PreferencesManager::FindModuleRecursive(PreferencesModule* module, const std::string& path) const
{
    if (module->GetFullPath() == path)
    {
        return module;
    }

    for (PreferencesModule* sub : module->GetSubModules())
    {
        PreferencesModule* found = FindModuleRecursive(sub, path);
        if (found != nullptr)
        {
            return found;
        }
    }

    return nullptr;
}

void PreferencesManager::LoadAllSettings()
{
    for (PreferencesModule* module : mRootModules)
    {
        LoadModuleSettings(module);
    }
}

void PreferencesManager::SaveAllSettings()
{
    for (PreferencesModule* module : mRootModules)
    {
        SaveModuleSettings(module);
    }
}

void PreferencesManager::LoadModuleSettings(PreferencesModule* module)
{
    rapidjson::Document doc;
    std::string filePath = module->GetSettingsFilePath();

    if (JsonSettings::LoadFromFile(filePath, doc))
    {
        module->LoadSettings(doc);
    }

    module->SetDirty(false);

    // Load sub-module settings
    for (PreferencesModule* sub : module->GetSubModules())
    {
        LoadModuleSettings(sub);
    }
}

void PreferencesManager::SaveModuleSettings(PreferencesModule* module)
{
    rapidjson::Document doc;
    doc.SetObject();

    // Add version for future compatibility
    JsonSettings::SetInt(doc, "version", 1);

    module->SaveSettings(doc);

    std::string filePath = module->GetSettingsFilePath();
    JsonSettings::SaveToFile(filePath, doc);

    module->SetDirty(false);

    // Save sub-module settings
    for (PreferencesModule* sub : module->GetSubModules())
    {
        SaveModuleSettings(sub);
    }
}

std::string PreferencesManager::GetPreferencesDirectory() const
{
    return JsonSettings::GetPreferencesDirectory();
}

#endif
