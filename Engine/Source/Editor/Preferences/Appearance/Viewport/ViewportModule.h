#pragma once

#if EDITOR

#include "../../PreferencesModule.h"
#include "Maths.h"

class ViewportModule : public PreferencesModule
{
public:
    DECLARE_PREFERENCES_MODULE(ViewportModule)

    ViewportModule();
    virtual ~ViewportModule();

    virtual const char* GetName() const override { return GetStaticName(); }
    virtual const char* GetParentPath() const override { return GetStaticParentPath(); }
    virtual void Render() override;
    virtual void LoadSettings(const rapidjson::Document& doc) override;
    virtual void SaveSettings(rapidjson::Document& doc) override;

    // Settings accessors
    glm::vec4 GetBackgroundColor() const { return mBackgroundColor; }
    bool GetShowGrid() const { return mShowGrid; }
    glm::vec4 GetGridColor() const { return mGridColor; }
    float GetGridSize() const { return mGridSize; }

private:
    glm::vec4 mBackgroundColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
    bool mShowGrid = true;
    glm::vec4 mGridColor = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
    float mGridSize = 1.0f;
};

#endif
