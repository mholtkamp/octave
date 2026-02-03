#pragma once

#if EDITOR

#include "../../PreferencesModule.h"
#include "EditorTheme.h"
#include <string>

class ThemeModule : public PreferencesModule
{
public:
    DECLARE_PREFERENCES_MODULE(ThemeModule)

    ThemeModule();
    virtual ~ThemeModule();

    virtual const char* GetName() const override { return GetStaticName(); }
    virtual const char* GetParentPath() const override { return GetStaticParentPath(); }
    virtual void Render() override;
    virtual void LoadSettings(const rapidjson::Document& doc) override;
    virtual void SaveSettings(rapidjson::Document& doc) override;

    // Settings accessors
    EditorThemeType GetCurrentTheme() const { return mCurrentTheme; }
    const std::string& GetCurrentFont() const { return mCurrentFont; }
    static std::string LoadSavedFontPreference();

    std::string mCurrentFont = "Default";

private:
    EditorThemeType mCurrentTheme = EditorThemeType::Dark;
    int mSelectedThemeIndex = 0;

    int mSelectedFontIndex = 0;
    std::vector<std::string> mAvailableFonts;

    void RefreshAvailableFonts();
    void LoadFont();
    static std::string GetSettingsFilePathStatic();
};

#endif
