
#include "Nodes/Widgets/SpinBox.h"
#include "Nodes/Widgets/Quad.h"
#include "Nodes/Widgets/Text.h"
#include "Nodes/Widgets/Button.h"
#include "InputDevices.h"
#include "Engine.h"
#include "Renderer.h"
#include "Assets/Texture.h"

#include <sstream>
#include <iomanip>

FORCE_LINK_DEF(SpinBox);
DEFINE_NODE(SpinBox, Widget);

bool SpinBox::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);

    OCT_ASSERT(prop != nullptr);
    SpinBox* spinBox = static_cast<SpinBox*>(prop->mOwner);
    bool success = false;

    spinBox->MarkDirty();

    return success;
}

void SpinBox::Create()
{
    Super::Create();

    SetName("SpinBox");

    mBackground = CreateChild<Quad>("Background");
    mText = CreateChild<Text>("Text");
    mDecrementButton = CreateChild<Button>("Decrement");
    mIncrementButton = CreateChild<Button>("Increment");

    mBackground->SetTransient(true);
    mText->SetTransient(true);
    mDecrementButton->SetTransient(true);
    mIncrementButton->SetTransient(true);

#if EDITOR
    mBackground->mHiddenInTree = true;
    mText->mHiddenInTree = true;
    mDecrementButton->mHiddenInTree = true;
    mIncrementButton->mHiddenInTree = true;
#endif

    // Setup background - FullStretch
    mBackground->SetAnchorMode(AnchorMode::FullStretch);
    mBackground->SetMargins(0.0f, 0.0f, 0.0f, 0.0f);
    mBackground->SetColor(mBackgroundColor);

    // Setup text - FullStretch with margins for buttons
    mText->SetAnchorMode(AnchorMode::FullStretch);
    mText->SetHorizontalJustification(Justification::Center);
    mText->SetVerticalJustification(Justification::Center);
    mText->SetColor(mTextColor);

    // Setup decrement button - TopLeft
    mDecrementButton->SetAnchorMode(AnchorMode::TopLeft);
    mDecrementButton->SetTextString("-");
    mDecrementButton->SetNormalColor(mButtonColor);

    // Setup increment button - TopLeft (positioned on right)
    mIncrementButton->SetAnchorMode(AnchorMode::TopLeft);
    mIncrementButton->SetTextString("+");
    mIncrementButton->SetNormalColor(mButtonColor);

    // Default dimensions
    SetDimensions(120, 30);

    MarkDirty();
}

void SpinBox::GatherProperties(std::vector<Property>& props)
{
    Super::GatherProperties(props);

    {
        SCOPED_CATEGORY("SpinBox");

        props.push_back(Property(DatumType::Float, "Value", this, &mValue, 1, HandlePropChange));
        props.push_back(Property(DatumType::Float, "Min Value", this, &mMinValue, 1, HandlePropChange));
        props.push_back(Property(DatumType::Float, "Max Value", this, &mMaxValue, 1, HandlePropChange));
        props.push_back(Property(DatumType::Float, "Step", this, &mStep, 1, HandlePropChange));
        props.push_back(Property(DatumType::Integer, "Decimal Places", this, &mDecimalPlaces, 1, HandlePropChange));
        props.push_back(Property(DatumType::String, "Prefix", this, &mPrefix, 1, HandlePropChange));
        props.push_back(Property(DatumType::String, "Suffix", this, &mSuffix, 1, HandlePropChange));
        props.push_back(Property(DatumType::Float, "Button Width", this, &mButtonWidth, 1, HandlePropChange));
    }

    {
        SCOPED_CATEGORY("Visual");

        props.push_back(Property(DatumType::Asset, "Background Texture", this, &mBackgroundTexture, 1, HandlePropChange, int32_t(Texture::GetStaticType())));
        props.push_back(Property(DatumType::Color, "Background Color", this, &mBackgroundColor, 1, HandlePropChange));
        props.push_back(Property(DatumType::Color, "Text Color", this, &mTextColor, 1, HandlePropChange));
        props.push_back(Property(DatumType::Color, "Button Color", this, &mButtonColor, 1, HandlePropChange));
    }
}

void SpinBox::Tick(float deltaTime)
{
    Super::Tick(deltaTime);

    if (!ShouldHandleInput())
    {
        return;
    }

    // Check if buttons were activated
    if (mDecrementButton->GetState() == ButtonState::Pressed && IsPointerJustDown(0))
    {
        Decrement();
    }

    if (mIncrementButton->GetState() == ButtonState::Pressed && IsPointerJustDown(0))
    {
        Increment();
    }

    // Handle scroll wheel when hovered
    if (ContainsMouse())
    {
        int32_t scrollDelta = GetScrollWheelDelta();
        if (scrollDelta > 0)
        {
            Increment();
        }
        else if (scrollDelta < 0)
        {
            Decrement();
        }
    }
}

void SpinBox::PreRender()
{
    Super::PreRender();

    if (IsDirty())
    {
        UpdateAppearance();
    }
}

void SpinBox::UpdateAppearance()
{
    float width = GetWidth();
    float height = GetHeight();

    // Update background
    mBackground->SetTexture(mBackgroundTexture.Get<Texture>());
    mBackground->SetColor(mBackgroundColor);

    // Position decrement button on left
    mDecrementButton->SetPosition(0.0f, 0.0f);
    mDecrementButton->SetDimensions(mButtonWidth, height);
    mDecrementButton->SetNormalColor(mButtonColor);

    // Position increment button on right
    mIncrementButton->SetPosition(width - mButtonWidth, 0.0f);
    mIncrementButton->SetDimensions(mButtonWidth, height);
    mIncrementButton->SetNormalColor(mButtonColor);

    // Update text margins
    mText->SetMargins(mButtonWidth, 0.0f, mButtonWidth, 0.0f);
    mText->SetColor(mTextColor);

    // Format value text
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(mDecimalPlaces) << mValue;
    std::string valueStr = oss.str();

    std::string displayText;
    if (!mPrefix.empty())
    {
        displayText = mPrefix + " ";
    }
    displayText += valueStr;
    if (!mSuffix.empty())
    {
        displayText += " " + mSuffix;
    }

    mText->SetText(displayText);
}

void SpinBox::Increment()
{
    SetValue(mValue + mStep);
}

void SpinBox::Decrement()
{
    SetValue(mValue - mStep);
}

void SpinBox::SetValue(float value)
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

float SpinBox::GetValue() const
{
    return mValue;
}

void SpinBox::SetMinValue(float min)
{
    if (mMinValue != min)
    {
        mMinValue = min;
        mValue = glm::clamp(mValue, mMinValue, mMaxValue);
        MarkDirty();
    }
}

float SpinBox::GetMinValue() const
{
    return mMinValue;
}

void SpinBox::SetMaxValue(float max)
{
    if (mMaxValue != max)
    {
        mMaxValue = max;
        mValue = glm::clamp(mValue, mMinValue, mMaxValue);
        MarkDirty();
    }
}

float SpinBox::GetMaxValue() const
{
    return mMaxValue;
}

void SpinBox::SetStep(float step)
{
    if (mStep != step)
    {
        mStep = step;
        MarkDirty();
    }
}

float SpinBox::GetStep() const
{
    return mStep;
}

void SpinBox::SetPrefix(const std::string& prefix)
{
    if (mPrefix != prefix)
    {
        mPrefix = prefix;
        MarkDirty();
    }
}

const std::string& SpinBox::GetPrefix() const
{
    return mPrefix;
}

void SpinBox::SetSuffix(const std::string& suffix)
{
    if (mSuffix != suffix)
    {
        mSuffix = suffix;
        MarkDirty();
    }
}

const std::string& SpinBox::GetSuffix() const
{
    return mSuffix;
}

void SpinBox::SetBackgroundColor(glm::vec4 color)
{
    if (mBackgroundColor != color)
    {
        mBackgroundColor = color;
        MarkDirty();
    }
}

glm::vec4 SpinBox::GetBackgroundColor()
{
    return mBackgroundColor;
}

void SpinBox::SetTextColor(glm::vec4 color)
{
    if (mTextColor != color)
    {
        mTextColor = color;
        MarkDirty();
    }
}

glm::vec4 SpinBox::GetTextColor()
{
    return mTextColor;
}

void SpinBox::SetButtonColor(glm::vec4 color)
{
    if (mButtonColor != color)
    {
        mButtonColor = color;
        MarkDirty();
    }
}

glm::vec4 SpinBox::GetButtonColor()
{
    return mButtonColor;
}

Quad* SpinBox::GetBackground()
{
    return mBackground;
}

Text* SpinBox::GetTextWidget()
{
    return mText;
}

Button* SpinBox::GetIncrementButton()
{
    return mIncrementButton;
}

Button* SpinBox::GetDecrementButton()
{
    return mDecrementButton;
}
