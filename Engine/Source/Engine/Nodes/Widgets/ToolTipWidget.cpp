#include "Nodes/Widgets/ToolTipWidget.h"
#include "Nodes/Widgets/Quad.h"
#include "Nodes/Widgets/Text.h"
#include "Assets/Texture.h"
#include "Log.h"

FORCE_LINK_DEF(ToolTipWidget);
DEFINE_NODE(ToolTipWidget, Widget);

void ToolTipWidget::Create()
{
    Widget::Create();

    SetName("ToolTip");
    SetVisible(false);
    mUseScissor = false;

    EnsureChildWidgets();

    MarkDirty();
}

void ToolTipWidget::EnsureChildWidgets()
{
    if (mBackground == nullptr)
    {
        mBackground = CreateChild<Quad>("Background");
        mBackground->SetTransient(true);
        mBackground->SetAnchorMode(AnchorMode::FullStretch);
        mBackground->SetMargins(0.0f, 0.0f, 0.0f, 0.0f);
        mBackground->SetColor(mBackgroundColor);
        mBackground->SetCornerRadius(mCornerRadius);
#if EDITOR
        mBackground->mHiddenInTree = true;
#endif
    }

    if (mTitleTextWidget == nullptr)
    {
        mTitleTextWidget = CreateChild<Text>("TitleText");
        mTitleTextWidget->SetTransient(true);
        mTitleTextWidget->SetAnchorMode(AnchorMode::TopLeft);
        mTitleTextWidget->SetTextSize(mTitleFontSize);
        mTitleTextWidget->SetColor(mTitleColor);
        mTitleTextWidget->SetText(mTooltipTitle);
#if EDITOR
        mTitleTextWidget->mHiddenInTree = true;
#endif
    }

    if (mDescriptionTextWidget == nullptr)
    {
        mDescriptionTextWidget = CreateChild<Text>("DescriptionText");
        mDescriptionTextWidget->SetTransient(true);
        mDescriptionTextWidget->SetAnchorMode(AnchorMode::TopLeft);
        mDescriptionTextWidget->SetTextSize(mTextFontSize);
        mDescriptionTextWidget->SetColor(mTextColor);
        mDescriptionTextWidget->SetText(mTooltipText);
        mDescriptionTextWidget->EnableWordWrap(true);
#if EDITOR
        mDescriptionTextWidget->mHiddenInTree = true;
#endif
    }
}

void ToolTipWidget::PreRender()
{
    EnsureChildWidgets();
    UpdateLayout();
    Widget::PreRender();
}

void ToolTipWidget::UpdateLayout()
{
    if (mTitleTextWidget == nullptr || mDescriptionTextWidget == nullptr)
        return;

    // Update text content
    mTitleTextWidget->SetText(mTooltipTitle);
    mDescriptionTextWidget->SetText(mTooltipText);

    // Calculate content dimensions
    float titleWidth = mTitleTextWidget->GetTextWidth();
    float titleHeight = mTitleTextWidget->GetTextHeight();

    // Set max width for description text (accounting for padding)
    float maxContentWidth = mMaxWidth - mPaddingLeft - mPaddingRight;
    mDescriptionTextWidget->SetDimensions(maxContentWidth, 0.0f); // Width constrained, height auto

    float descWidth = mDescriptionTextWidget->GetTextWidth();
    float descHeight = mDescriptionTextWidget->GetTextHeight();

    // Calculate total content dimensions
    float contentWidth = glm::max(titleWidth, descWidth);
    contentWidth = glm::min(contentWidth, maxContentWidth);

    float contentHeight = 0.0f;
    bool hasTitle = !mTooltipTitle.empty();
    bool hasDesc = !mTooltipText.empty();

    if (hasTitle)
    {
        contentHeight += titleHeight;
    }
    if (hasTitle && hasDesc)
    {
        contentHeight += mTitleTextSpacing;
    }
    if (hasDesc)
    {
        contentHeight += descHeight;
    }

    // Calculate tooltip widget dimensions
    float tooltipWidth = contentWidth + mPaddingLeft + mPaddingRight;
    float tooltipHeight = contentHeight + mPaddingTop + mPaddingBottom;

    SetDimensions(tooltipWidth, tooltipHeight);

    // Position title text
    mTitleTextWidget->SetPosition(mPaddingLeft, mPaddingTop);
    mTitleTextWidget->SetDimensions(contentWidth, titleHeight);
    mTitleTextWidget->SetVisible(hasTitle);

    // Position description text
    float descY = mPaddingTop;
    if (hasTitle)
    {
        descY += titleHeight + mTitleTextSpacing;
    }
    mDescriptionTextWidget->SetPosition(mPaddingLeft, descY);
    mDescriptionTextWidget->SetDimensions(contentWidth, descHeight);
    mDescriptionTextWidget->SetVisible(hasDesc);

    // Update background
    if (mBackground != nullptr)
    {
        mBackground->SetColor(mBackgroundColor);
        mBackground->SetCornerRadius(mCornerRadius);
        if (mBackgroundTexture.Get<Texture>() != nullptr)
        {
            mBackground->SetTexture(mBackgroundTexture.Get<Texture>());
        }
    }
}

void ToolTipWidget::GatherProperties(std::vector<Property>& props)
{
    Widget::GatherProperties(props);

    SCOPED_CATEGORY("ToolTip");

    // Content
    props.push_back(Property(DatumType::String, "Tooltip Title", this, &mTooltipTitle));
    props.push_back(Property(DatumType::String, "Tooltip Text", this, &mTooltipText));

    // Styling
    props.push_back(Property(DatumType::Color, "Background Color", this, &mBackgroundColor));
    props.push_back(Property(DatumType::Asset, "Background Texture", this, &mBackgroundTexture, 1, nullptr, int32_t(Texture::GetStaticType())));
    props.push_back(Property(DatumType::Float, "Corner Radius", this, &mCornerRadius));
    props.push_back(Property(DatumType::Float, "Title Font Size", this, &mTitleFontSize));
    props.push_back(Property(DatumType::Float, "Text Font Size", this, &mTextFontSize));
    props.push_back(Property(DatumType::Color, "Title Color", this, &mTitleColor));
    props.push_back(Property(DatumType::Color, "Text Color", this, &mTextColor));

    // Layout
    props.push_back(Property(DatumType::Float, "Padding Left", this, &mPaddingLeft));
    props.push_back(Property(DatumType::Float, "Padding Top", this, &mPaddingTop));
    props.push_back(Property(DatumType::Float, "Padding Right", this, &mPaddingRight));
    props.push_back(Property(DatumType::Float, "Padding Bottom", this, &mPaddingBottom));
    props.push_back(Property(DatumType::Float, "Max Width", this, &mMaxWidth));
    props.push_back(Property(DatumType::Float, "Title Text Spacing", this, &mTitleTextSpacing));
}

void ToolTipWidget::SetTooltipTitle(const std::string& name)
{
    mTooltipTitle = name;
    MarkDirty();
}

const std::string& ToolTipWidget::GetTooltipTitle() const
{
    return mTooltipTitle;
}

void ToolTipWidget::SetTooltipText(const std::string& description)
{
    mTooltipText = description;
    MarkDirty();
}

const std::string& ToolTipWidget::GetTooltipText() const
{
    return mTooltipText;
}

void ToolTipWidget::SetContent(const std::string& name, const std::string& description)
{
    mTooltipTitle = name;
    mTooltipText = description;
    MarkDirty();
}

void ToolTipWidget::ConfigureFromWidget(Widget* widget)
{
    if (widget != nullptr)
    {
        SetContent(widget->GetTooltipName(), widget->GetTooltipDescription());
    }
    else
    {
        SetContent("", "");
    }
}

void ToolTipWidget::SetBackgroundColor(glm::vec4 color)
{
    mBackgroundColor = color;
    if (mBackground != nullptr)
    {
        mBackground->SetColor(color);
    }
    MarkDirty();
}

glm::vec4 ToolTipWidget::GetBackgroundColor() const
{
    return mBackgroundColor;
}

void ToolTipWidget::SetBackgroundTexture(Texture* texture)
{
    mBackgroundTexture = texture;
    if (mBackground != nullptr && texture != nullptr)
    {
        mBackground->SetTexture(texture);
    }
    MarkDirty();
}

Texture* ToolTipWidget::GetBackgroundTexture()
{
    return mBackgroundTexture.Get<Texture>();
}

void ToolTipWidget::SetCornerRadius(float radius)
{
    mCornerRadius = radius;
    if (mBackground != nullptr)
    {
        mBackground->SetCornerRadius(radius);
    }
    MarkDirty();
}

float ToolTipWidget::GetCornerRadius() const
{
    return mCornerRadius;
}

void ToolTipWidget::SetTitleFontSize(float size)
{
    mTitleFontSize = size;
    if (mTitleTextWidget != nullptr)
    {
        mTitleTextWidget->SetTextSize(size);
    }
    MarkDirty();
}

float ToolTipWidget::GetTitleFontSize() const
{
    return mTitleFontSize;
}

void ToolTipWidget::SetTextFontSize(float size)
{
    mTextFontSize = size;
    if (mDescriptionTextWidget != nullptr)
    {
        mDescriptionTextWidget->SetTextSize(size);
    }
    MarkDirty();
}

float ToolTipWidget::GetTextFontSize() const
{
    return mTextFontSize;
}

void ToolTipWidget::SetTitleColor(glm::vec4 color)
{
    mTitleColor = color;
    if (mTitleTextWidget != nullptr)
    {
        mTitleTextWidget->SetColor(color);
    }
    MarkDirty();
}

glm::vec4 ToolTipWidget::GetTitleColor() const
{
    return mTitleColor;
}

void ToolTipWidget::SetTextColor(glm::vec4 color)
{
    mTextColor = color;
    if (mDescriptionTextWidget != nullptr)
    {
        mDescriptionTextWidget->SetColor(color);
    }
    MarkDirty();
}

glm::vec4 ToolTipWidget::GetTextColor() const
{
    return mTextColor;
}

void ToolTipWidget::SetPadding(float left, float top, float right, float bottom)
{
    mPaddingLeft = left;
    mPaddingTop = top;
    mPaddingRight = right;
    mPaddingBottom = bottom;
    MarkDirty();
}

float ToolTipWidget::GetPaddingLeft() const
{
    return mPaddingLeft;
}

float ToolTipWidget::GetPaddingTop() const
{
    return mPaddingTop;
}

float ToolTipWidget::GetPaddingRight() const
{
    return mPaddingRight;
}

float ToolTipWidget::GetPaddingBottom() const
{
    return mPaddingBottom;
}

void ToolTipWidget::SetMaxWidth(float width)
{
    mMaxWidth = width;
    MarkDirty();
}

float ToolTipWidget::GetMaxWidth() const
{
    return mMaxWidth;
}

void ToolTipWidget::SetTitleTextSpacing(float spacing)
{
    mTitleTextSpacing = spacing;
    MarkDirty();
}

float ToolTipWidget::GetTitleTextSpacing() const
{
    return mTitleTextSpacing;
}

Quad* ToolTipWidget::GetBackground()
{
    EnsureChildWidgets();
    return mBackground;
}

Text* ToolTipWidget::GetTitleText()
{
    EnsureChildWidgets();
    return mTitleTextWidget;
}

Text* ToolTipWidget::GetDescriptionText()
{
    EnsureChildWidgets();
    return mDescriptionTextWidget;
}
