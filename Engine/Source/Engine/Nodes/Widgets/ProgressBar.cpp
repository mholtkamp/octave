
#include "Nodes/Widgets/ProgressBar.h"
#include "Nodes/Widgets/Quad.h"
#include "Nodes/Widgets/Text.h"
#include "Engine.h"
#include "Renderer.h"
#include "Assets/Texture.h"

FORCE_LINK_DEF(ProgressBar);
DEFINE_NODE(ProgressBar, Widget);

bool ProgressBar::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);

    OCT_ASSERT(prop != nullptr);
    ProgressBar* progressBar = static_cast<ProgressBar*>(prop->mOwner);
    bool success = false;

    progressBar->MarkDirty();

    return success;
}

void ProgressBar::Create()
{
    Super::Create();

    SetName("ProgressBar");

    mBackground = CreateChild<Quad>("Background");
    mFill = CreateChild<Quad>("Fill");
    mText = CreateChild<Text>("Text");

    mBackground->SetTransient(true);
    mFill->SetTransient(true);
    mText->SetTransient(true);

#if EDITOR
    mBackground->mHiddenInTree = true;
    mFill->mHiddenInTree = true;
    mText->mHiddenInTree = true;
#endif

    // Setup background - FullStretch
    mBackground->SetAnchorMode(AnchorMode::FullStretch);
    mBackground->SetMargins(0.0f, 0.0f, 0.0f, 0.0f);
    mBackground->SetColor(mBackgroundColor);

    // Setup fill - TopLeft, sized dynamically
    mFill->SetAnchorMode(AnchorMode::TopLeft);
    mFill->SetColor(mFillColor);

    // Setup text - FullStretch, centered
    mText->SetAnchorMode(AnchorMode::FullStretch);
    mText->SetMargins(0.0f, 0.0f, 0.0f, 0.0f);
    mText->SetHorizontalJustification(Justification::Center);
    mText->SetVerticalJustification(Justification::Center);
    mText->SetColor(mTextColor);

    // Default dimensions
    SetDimensions(200, 24);

    MarkDirty();
}

void ProgressBar::GatherProperties(std::vector<Property>& props)
{
    Super::GatherProperties(props);

    {
        SCOPED_CATEGORY("ProgressBar");

        props.push_back(Property(DatumType::Float, "Value", this, &mValue, 1, HandlePropChange));
        props.push_back(Property(DatumType::Float, "Min Value", this, &mMinValue, 1, HandlePropChange));
        props.push_back(Property(DatumType::Float, "Max Value", this, &mMaxValue, 1, HandlePropChange));
        props.push_back(Property(DatumType::Bool, "Show Percentage", this, &mShowPercentage, 1, HandlePropChange));
        props.push_back(Property(DatumType::Float, "Padding", this, &mPadding, 1, HandlePropChange));
    }

    {
        SCOPED_CATEGORY("Visual");

        props.push_back(Property(DatumType::Asset, "Background Texture", this, &mBackgroundTexture, 1, HandlePropChange, int32_t(Texture::GetStaticType())));
        props.push_back(Property(DatumType::Asset, "Fill Texture", this, &mFillTexture, 1, HandlePropChange, int32_t(Texture::GetStaticType())));
        props.push_back(Property(DatumType::Color, "Background Color", this, &mBackgroundColor, 1, HandlePropChange));
        props.push_back(Property(DatumType::Color, "Fill Color", this, &mFillColor, 1, HandlePropChange));
        props.push_back(Property(DatumType::Color, "Text Color", this, &mTextColor, 1, HandlePropChange));
    }
}

void ProgressBar::PreRender()
{
    Super::PreRender();

    if (IsDirty())
    {
        UpdateAppearance();
    }
}

void ProgressBar::UpdateAppearance()
{
    float width = GetWidth();
    float height = GetHeight();

    // Update background
    mBackground->SetTexture(mBackgroundTexture.Get<Texture>());
    mBackground->SetColor(mBackgroundColor);

    // Calculate fill width based on ratio
    float ratio = GetRatio();
    float fillWidth = (width - mPadding * 2.0f) * ratio;
    float fillHeight = height - mPadding * 2.0f;

    mFill->SetPosition(mPadding, mPadding);
    mFill->SetDimensions(glm::max(fillWidth, 0.0f), fillHeight);
    mFill->SetTexture(mFillTexture.Get<Texture>());
    mFill->SetColor(mFillColor);

    // Update percentage text
    if (mShowPercentage)
    {
        int percentage = (int)(ratio * 100.0f + 0.5f);
        mText->SetText(std::to_string(percentage) + "%");
        mText->SetColor(mTextColor);
        mText->SetVisible(true);
    }
    else
    {
        mText->SetVisible(false);
    }
}

void ProgressBar::SetValue(float value)
{
    value = glm::clamp(value, mMinValue, mMaxValue);
    if (mValue != value)
    {
        mValue = value;
        MarkDirty();

        if (!IsDestroyed())
        {
            EmitSignal("ValueChanged", { this });
            CallFunction("OnValueChanged", { this });
        }
    }
}

float ProgressBar::GetValue() const
{
    return mValue;
}

void ProgressBar::SetMinValue(float min)
{
    if (mMinValue != min)
    {
        mMinValue = min;
        mValue = glm::clamp(mValue, mMinValue, mMaxValue);
        MarkDirty();
    }
}

float ProgressBar::GetMinValue() const
{
    return mMinValue;
}

void ProgressBar::SetMaxValue(float max)
{
    if (mMaxValue != max)
    {
        mMaxValue = max;
        mValue = glm::clamp(mValue, mMinValue, mMaxValue);
        MarkDirty();
    }
}

float ProgressBar::GetMaxValue() const
{
    return mMaxValue;
}

float ProgressBar::GetRatio() const
{
    if (mMaxValue <= mMinValue)
    {
        return 0.0f;
    }
    return (mValue - mMinValue) / (mMaxValue - mMinValue);
}

void ProgressBar::SetShowPercentage(bool show)
{
    if (mShowPercentage != show)
    {
        mShowPercentage = show;
        MarkDirty();
    }
}

bool ProgressBar::IsShowingPercentage() const
{
    return mShowPercentage;
}

void ProgressBar::SetBackgroundColor(glm::vec4 color)
{
    if (mBackgroundColor != color)
    {
        mBackgroundColor = color;
        MarkDirty();
    }
}

glm::vec4 ProgressBar::GetBackgroundColor()
{
    return mBackgroundColor;
}

void ProgressBar::SetFillColor(glm::vec4 color)
{
    if (mFillColor != color)
    {
        mFillColor = color;
        MarkDirty();
    }
}

glm::vec4 ProgressBar::GetFillColor()
{
    return mFillColor;
}

void ProgressBar::SetTextColor(glm::vec4 color)
{
    if (mTextColor != color)
    {
        mTextColor = color;
        MarkDirty();
    }
}

glm::vec4 ProgressBar::GetTextColor()
{
    return mTextColor;
}

Quad* ProgressBar::GetBackgroundQuad()
{
    return mBackground;
}

Quad* ProgressBar::GetFillQuad()
{
    return mFill;
}

Text* ProgressBar::GetTextWidget()
{
    return mText;
}
