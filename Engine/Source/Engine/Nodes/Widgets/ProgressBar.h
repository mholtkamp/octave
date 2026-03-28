#pragma once

#include "Nodes/Widgets/Widget.h"

class Texture;
class Quad;
class Text;

class OCTAVE_API ProgressBar : public Widget
{
public:

    DECLARE_NODE(ProgressBar, Widget);

    virtual void Create() override;
    virtual void GatherProperties(std::vector<Property>& props) override;
    virtual void PreRender() override;

    // Value
    void SetValue(float value);
    float GetValue() const;
    void SetMinValue(float min);
    float GetMinValue() const;
    void SetMaxValue(float max);
    float GetMaxValue() const;
    float GetRatio() const;

    // Display
    void SetShowPercentage(bool show);
    bool IsShowingPercentage() const;

    // Visual
    void SetBackgroundColor(glm::vec4 color);
    glm::vec4 GetBackgroundColor();
    void SetFillColor(glm::vec4 color);
    glm::vec4 GetFillColor();
    void SetTextColor(glm::vec4 color);
    glm::vec4 GetTextColor();

    // Children
    Quad* GetBackgroundQuad();
    Quad* GetFillQuad();
    Text* GetTextWidget();

protected:

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

    void UpdateAppearance();

    // Value
    float mValue = 0.5f;
    float mMinValue = 0.0f;
    float mMaxValue = 1.0f;

    // Display
    bool mShowPercentage = true;

    // Visual
    TextureRef mBackgroundTexture;
    TextureRef mFillTexture;
    glm::vec4 mBackgroundColor = { 0.2f, 0.2f, 0.2f, 1.0f };
    glm::vec4 mFillColor = { 0.3f, 0.6f, 0.9f, 1.0f };
    glm::vec4 mTextColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    float mPadding = 2.0f;

    // Children
    Quad* mBackground = nullptr;
    Quad* mFill = nullptr;
    Text* mText = nullptr;
};
