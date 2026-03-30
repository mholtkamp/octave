#pragma once

#include "Nodes/Widgets/Widget.h"
#include "AssetRef.h"

class Quad;
class Text;
class Texture;

class OCTAVE_API ToolTipWidget : public Widget
{
public:

    DECLARE_NODE(ToolTipWidget, Widget);

    virtual void Create() override;
    virtual void PreRender() override;
    virtual void GatherProperties(std::vector<Property>& props) override;

    // Content
    void SetTooltipTitle(const std::string& name);
    const std::string& GetTooltipTitle() const;
    void SetTooltipText(const std::string& description);
    const std::string& GetTooltipText() const;
    void SetContent(const std::string& name, const std::string& description);
    void ConfigureFromWidget(Widget* widget);

    // Styling - Background
    void SetBackgroundColor(glm::vec4 color);
    glm::vec4 GetBackgroundColor() const;
    void SetBackgroundTexture(Texture* texture);
    Texture* GetBackgroundTexture();
    void SetCornerRadius(float radius);
    float GetCornerRadius() const;

    // Styling - Text
    void SetTitleFontSize(float size);
    float GetTitleFontSize() const;
    void SetTextFontSize(float size);
    float GetTextFontSize() const;
    void SetTitleColor(glm::vec4 color);
    glm::vec4 GetTitleColor() const;
    void SetTextColor(glm::vec4 color);
    glm::vec4 GetTextColor() const;

    // Styling - Layout
    void SetPadding(float left, float top, float right, float bottom);
    float GetPaddingLeft() const;
    float GetPaddingTop() const;
    float GetPaddingRight() const;
    float GetPaddingBottom() const;
    void SetMaxWidth(float width);
    float GetMaxWidth() const;
    void SetTitleTextSpacing(float spacing);
    float GetTitleTextSpacing() const;

    // Access child widgets
    Quad* GetBackground();
    Text* GetTitleText();
    Text* GetDescriptionText();

protected:

    void UpdateLayout();
    void EnsureChildWidgets();

    // Content
    std::string mTooltipTitle;
    std::string mTooltipText;

    // Styling - Background
    glm::vec4 mBackgroundColor = glm::vec4(0.1f, 0.1f, 0.15f, 0.95f);
    float mCornerRadius = 4.0f;
    TextureRef mBackgroundTexture;

    // Styling - Text
    float mTitleFontSize = 14.0f;
    float mTextFontSize = 12.0f;
    glm::vec4 mTitleColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    glm::vec4 mTextColor = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);

    // Styling - Layout
    float mPaddingLeft = 8.0f;
    float mPaddingTop = 6.0f;
    float mPaddingRight = 8.0f;
    float mPaddingBottom = 6.0f;
    float mMaxWidth = 300.0f;
    float mTitleTextSpacing = 4.0f;

    // Child widgets (transient)
    Quad* mBackground = nullptr;
    Text* mTitleTextWidget = nullptr;
    Text* mDescriptionTextWidget = nullptr;
};
