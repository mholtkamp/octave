#pragma once

#if EDITOR

#include <string>
#include <vector>
#include "document.h"

class PreferencesModule
{
public:
    virtual ~PreferencesModule();

    // Required overrides
    virtual const char* GetName() const = 0;
    virtual void Render() = 0;

    // Optional overrides
    virtual const char* GetParentPath() const { return ""; }
    virtual void LoadSettings(const rapidjson::Document& doc);
    virtual void SaveSettings(rapidjson::Document& doc);

    // Hierarchy management
    void AddSubModule(PreferencesModule* sub);
    const std::vector<PreferencesModule*>& GetSubModules() const;

    // Path utilities
    std::string GetFullPath() const;
    std::string GetSettingsFilePath() const;

    // Dirty flag for unsaved changes
    bool IsDirty() const { return mDirty; }
    void SetDirty(bool dirty) { mDirty = dirty; }

protected:
    std::vector<PreferencesModule*> mSubModules;
    bool mDirty = false;
};

// Registration macros (following Factory.h pattern)
#define DECLARE_PREFERENCES_MODULE(Class) \
    static const char* GetStaticName(); \
    static const char* GetStaticParentPath();

#define DEFINE_PREFERENCES_MODULE(Class, Name, ParentPath) \
    const char* Class::GetStaticName() { return Name; } \
    const char* Class::GetStaticParentPath() { return ParentPath; }

#endif
