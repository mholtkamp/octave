#pragma once

#include "Nodes/Widgets/Widget.h"

class Texture;
class Quad;
class Text;
class Button;

class OCTAVE_API SpinBox : public Widget
{
public:

    DECLARE_NODE(SpinBox, Widget);

    virtual void Create() override;
    virtual void GatherProperties(std::vector<Property>& props) override;
    virtual void Tick(float deltaTime) override;
    virtual void PreRender() override;

    // Value
    void SetValue(float value);
    float GetValue() const;
    void SetMinValue(float min);
    float GetMinValue() const;
    void SetMaxValue(float max);
    float GetMaxValue() const;
    void SetStep(float step);
    float GetStep() const;

    // Display
    void SetPrefix(const std::string& prefix);
    const std::string& GetPrefix() const;
    void SetSuffix(const std::string& suffix);
    const std::string& GetSuffix() const;

    // Visual
    void SetBackgroundColor(glm::vec4 color);
    glm::vec4 GetBackgroundColor();
    void SetTextColor(glm::vec4 color);
    glm::vec4 GetTextColor();
    void SetButtonColor(glm::vec4 color);
    glm::vec4 GetButtonColor();

    // Children
    Quad* GetBackground();
    Text* GetTextWidget();
    Button* GetIncrementButton();
    Button* GetDecrementButton();

protected:

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

    void UpdateAppearance();
    void Increment();
    void Decrement();

    // Value
    float mValue = 0.0f;
    float mMinValue = 0.0f;
    float mMaxValue = 100.0f;
    float mStep = 1.0f;

    // Display
    std::string mPrefix;
    std::string mSuffix;
    int32_t mDecimalPlaces = 0;

    // Visual
    TextureRef mBackgroundTexture;
    glm::vec4 mBackgroundColor = { 0.2f, 0.2f, 0.2f, 1.0f };
    glm::vec4 mTextColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    glm::vec4 mButtonColor = { 0.35f, 0.35f, 0.35f, 1.0f };
    float mButtonWidth = 20.0f;

    // Children
    Quad* mBackground = nullptr;
    Text* mText = nullptr;
    Button* mIncrementButton = nullptr;
    Button* mDecrementButton = nullptr;
};
