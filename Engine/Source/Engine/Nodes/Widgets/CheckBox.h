#pragma once

#include "Nodes/Widgets/Widget.h"

class Texture;
class Quad;
class Text;

class OCTAVE_API CheckBox : public Widget
{
public:

    DECLARE_NODE(CheckBox, Widget);

    virtual void Create() override;
    virtual void GatherProperties(std::vector<Property>& props) override;
    virtual void Tick(float deltaTime) override;
    virtual void EditorTick(float deltaTime) override;
    virtual void PreRender() override;

    // State
    void SetChecked(bool checked);
    bool IsChecked() const;
    void Toggle();

    // Text
    void SetText(const std::string& text);
    const std::string& GetText() const;

    // Visual
    void SetCheckedTexture(Texture* texture);
    Texture* GetCheckedTexture();
    void SetUncheckedTexture(Texture* texture);
    Texture* GetUncheckedTexture();
    void SetCheckedColor(glm::vec4 color);
    glm::vec4 GetCheckedColor();
    void SetUncheckedColor(glm::vec4 color);
    glm::vec4 GetUncheckedColor();
    void SetTextColor(glm::vec4 color);
    glm::vec4 GetTextColor();
    void SetBoxSize(float size);
    float GetBoxSize() const;
    void SetSpacing(float spacing);
    float GetSpacing() const;

    // Children
    Quad* GetBoxQuad();
    Text* GetTextWidget();

protected:

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

    void UpdateAppearance();

    // State
    bool mChecked = false;

    // Text
    std::string mTextContent = "CheckBox";

    // Visual
    TextureRef mCheckedTexture;
    TextureRef mUncheckedTexture;
    glm::vec4 mCheckedColor = { 0.3f, 0.7f, 0.3f, 1.0f };
    glm::vec4 mUncheckedColor = { 0.4f, 0.4f, 0.4f, 1.0f };
    glm::vec4 mHoveredColor = { 0.5f, 0.5f, 0.5f, 1.0f };
    glm::vec4 mTextColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    float mBoxSize = 20.0f;
    float mSpacing = 8.0f;

    // State
    bool mHovered = false;

    // Children
    Quad* mBoxQuad = nullptr;
    Text* mText = nullptr;
};
