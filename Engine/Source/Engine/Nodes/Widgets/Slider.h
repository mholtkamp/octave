#pragma once

#include "Nodes/Widgets/Widget.h"

class Texture;
class Quad;

class OCTAVE_API Slider : public Widget
{
public:

    DECLARE_NODE(Slider, Widget);

    virtual void Create() override;
    virtual void GatherProperties(std::vector<Property>& props) override;
    virtual void Tick(float deltaTime) override;
    virtual void PreRender() override;

    void SetValue(float value);
    float GetValue() const;
    void SetMinValue(float minValue);
    float GetMinValue() const;
    void SetMaxValue(float maxValue);
    float GetMaxValue() const;
    void SetStep(float step);
    float GetStep() const;
    void SetRange(float minValue, float maxValue);

    void SetBackgroundTexture(Texture* texture);
    Texture* GetBackgroundTexture();
    void SetGrabberTexture(Texture* texture);
    Texture* GetGrabberTexture();

    void SetBackgroundColor(glm::vec4 color);
    glm::vec4 GetBackgroundColor();
    void SetGrabberColor(glm::vec4 color);
    glm::vec4 GetGrabberColor();
    void SetGrabberHoveredColor(glm::vec4 color);
    glm::vec4 GetGrabberHoveredColor();
    void SetGrabberPressedColor(glm::vec4 color);
    glm::vec4 GetGrabberPressedColor();

    void SetGrabberWidth(float width);
    float GetGrabberWidth() const;
    void SetTrackHeight(float height);
    float GetTrackHeight() const;

    bool IsDragging() const;

    Quad* GetBackground();
    Quad* GetGrabber();

protected:

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

    void UpdateAppearance();
    void UpdateGrabberPosition();
    float GetValueFromPosition(float x);
    float GetValueRatio() const;
    float SnapToStep(float value);

    // Value
    float mValue = 0.0f;
    float mMinValue = 0.0f;
    float mMaxValue = 1.0f;
    float mStep = 0.0f;

    // Visual
    TextureRef mBackgroundTexture;
    TextureRef mGrabberTexture;
    glm::vec4 mBackgroundColor = { 0.3f, 0.3f, 0.3f, 1.0f };
    glm::vec4 mGrabberColor = { 0.7f, 0.7f, 0.7f, 1.0f };
    glm::vec4 mGrabberHoveredColor = { 0.8f, 0.8f, 0.8f, 1.0f };
    glm::vec4 mGrabberPressedColor = { 0.6f, 0.6f, 0.6f, 1.0f };
    float mGrabberWidth = 20.0f;
    float mTrackHeight = 8.0f;

    // State
    bool mDragging = false;
    bool mGrabberHovered = false;

    // Children
    Quad* mBackground = nullptr;
    Quad* mGrabber = nullptr;
};
