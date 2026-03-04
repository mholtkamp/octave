#pragma once

#if EDITOR

#include "../PreferencesModule.h"

/**
 * @brief Parent preferences module for packaging configuration.
 *
 * This module serves as a container for packaging settings such as
 * Docker image configuration and other build-related settings.
 */
class PackagingModule : public PreferencesModule
{
public:
    DECLARE_PREFERENCES_MODULE(PackagingModule)

    PackagingModule();
    virtual ~PackagingModule();

    virtual const char* GetName() const override { return GetStaticName(); }
    virtual const char* GetParentPath() const override { return GetStaticParentPath(); }
    virtual void Render() override;
    virtual void LoadSettings(const rapidjson::Document& doc) override;
    virtual void SaveSettings(rapidjson::Document& doc) override;
};

#endif
