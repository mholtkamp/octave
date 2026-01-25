#pragma once

#if EDITOR

#include <string>
#include <vector>

class PreferencesModule;

class PreferencesManager
{
public:
    static void Create();
    static void Destroy();
    static PreferencesManager* Get();

    void RegisterModule(PreferencesModule* module);
    const std::vector<PreferencesModule*>& GetRootModules() const;
    PreferencesModule* FindModule(const std::string& path) const;

    void LoadAllSettings();
    void SaveAllSettings();
    std::string GetPreferencesDirectory() const;

protected:
    static PreferencesManager* sInstance;

    PreferencesManager();
    ~PreferencesManager();

    void LoadModuleSettings(PreferencesModule* module);
    void SaveModuleSettings(PreferencesModule* module);
    PreferencesModule* FindModuleRecursive(PreferencesModule* module, const std::string& path) const;

    std::vector<PreferencesModule*> mRootModules;
};

#endif
