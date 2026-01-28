#pragma once

#if EDITOR

#include "../PreferencesModule.h"

class GeneralModule : public PreferencesModule
{
public:
    DECLARE_PREFERENCES_MODULE(GeneralModule)

    GeneralModule();
    virtual ~GeneralModule();

    virtual const char* GetName() const override { return GetStaticName(); }
    virtual const char* GetParentPath() const override { return GetStaticParentPath(); }
    virtual void Render() override;
    virtual void LoadSettings(const rapidjson::Document& doc) override;
    virtual void SaveSettings(rapidjson::Document& doc) override;

    // Settings accessors
    bool GetAutoSave() const { return mAutoSave; }
    int GetRecentProjectsLimit() const { return mRecentProjectsLimit; }
    bool GetShowWelcomeScreen() const { return mShowWelcomeScreen; }

private:
    bool mAutoSave = true;
    int mRecentProjectsLimit = 10;
    bool mShowWelcomeScreen = true;
};

#endif
