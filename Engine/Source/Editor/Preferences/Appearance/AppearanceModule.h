#pragma once

#if EDITOR

#include "../PreferencesModule.h"

class AppearanceModule : public PreferencesModule
{
public:
    DECLARE_PREFERENCES_MODULE(AppearanceModule)

    AppearanceModule();
    virtual ~AppearanceModule();

    virtual const char* GetName() const override { return GetStaticName(); }
    virtual const char* GetParentPath() const override { return GetStaticParentPath(); }
    virtual void Render() override;
    virtual void LoadSettings(const rapidjson::Document& doc) override;
    virtual void SaveSettings(rapidjson::Document& doc) override;
};

#endif
