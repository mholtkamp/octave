
#include "Nodes/Widgets/Slider.h"
#include "Nodes/Widgets/Quad.h"
#include "InputDevices.h"
#include "Engine.h"
#include "Renderer.h"

FORCE_LINK_DEF(Slider);
DEFINE_NODE(Slider, Widget);

bool Slider::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);

    OCT_ASSERT(prop != nullptr);
    Slider* slider = static_cast<Slider*>(prop->mOwner);
    bool success = false;

    slider->MarkDirty();

    return success;
}

void Slider::Create()
{
    Super::Create();

    SetName("Slider");
    mUseScissor = true;

    mBackground = CreateChild<Quad>("Background");
    mGrabber = CreateChild<Quad>("Grabber");

    mBackground->SetTransient(true);
    mGrabber->SetTransient(true);

#if EDITOR
    mBackground->mHiddenInTree = true;
    mGrabber->mHiddenInTree = true;
#endif

    // Default dimensions
    SetDimensions(150, 30);
}

void Slider::GatherProperties(std::vector<Property>& props)
{
    Super::GatherProperties(props);

    {
        SCOPED_CATEGORY("Slider");

        props.push_back(Property(DatumType::Float, "Value", this, &mValue, 1, HandlePropChange));
        props.push_back(Property(DatumType::Float, "Min Value", this, &mMinValue, 1, HandlePropChange));
        props.push_back(Property(DatumType::Float, "Max Value", this, &mMaxValue, 1, HandlePropChange));
        props.push_back(Property(DatumType::Float, "Step", this, &mStep, 1, HandlePropChange));

        props.push_back(Property(DatumType::Float, "Grabber Width", this, &mGrabberWidth, 1, HandlePropChange));
        props.push_back(Property(DatumType::Float, "Track Height", this, &mTrackHeight, 1, HandlePropChange));
    }

    {
        SCOPED_CATEGORY("Background");
        props.push_back(Property(DatumType::Asset, "Background Texture", this, &mBackgroundTexture, 1, HandlePropChange, int32_t(Texture::GetStaticType())));
        props.push_back(Property(DatumType::Color, "Background Color", this, &mBackgroundColor, 1, HandlePropChange));
    }

    {
        SCOPED_CATEGORY("Grabber");
        props.push_back(Property(DatumType::Asset, "Grabber Texture", this, &mGrabberTexture, 1, HandlePropChange, int32_t(Texture::GetStaticType())));
        props.push_back(Property(DatumType::Color, "Grabber Color", this, &mGrabberColor, 1, HandlePropChange));
        props.push_back(Property(DatumType::Color, "Grabber Hovered Color", this, &mGrabberHoveredColor, 1, HandlePropChange));
        props.push_back(Property(DatumType::Color, "Grabber Pressed Color", this, &mGrabberPressedColor, 1, HandlePropChange));
    }
}

void Slider::Tick(float deltaTime)
{
    Super::Tick(deltaTime);

    if (!ShouldHandleInput())
    {
        return;
    }

    // Ensure rect is up to date
    if (IsDirty())
    {
        UpdateRect();
    }

    bool containsMouse = ContainsMouse();
    int32_t mouseX, mouseY;
    GetMousePosition(mouseX, mouseY);

    // Check if mouse is over grabber
    Rect grabberRect = mGrabber->GetRect();
    bool overGrabber = grabberRect.ContainsPoint((float)mouseX, (float)mouseY);

    if (mDragging)
    {
        if (IsPointerDown(0))
        {
            // Calculate new value from mouse position
            float newValue = GetValueFromPosition((float)mouseX);
            SetValue(newValue);
        }
        else
        {
            // Mouse released - end drag
            mDragging = false;
            EmitSignal("DragEnded", { this });
            CallFunction("OnDragEnded", { this });
            MarkDirty();
        }
    }
    else if (containsMouse)
    {
        bool wasHovered = mGrabberHovered;
        mGrabberHovered = overGrabber;

        if (wasHovered != mGrabberHovered)
        {
            MarkDirty();
        }

        if (IsPointerJustDown(0))
        {
            if (overGrabber)
            {
                // Start dragging
                mDragging = true;
                EmitSignal("DragStarted", { this });
                CallFunction("OnDragStarted", { this });
                MarkDirty();
            }
            else
            {
                // Click on track - jump to position
                float newValue = GetValueFromPosition((float)mouseX);
                SetValue(newValue);
            }
        }

        // Mouse wheel
        int32_t scrollDelta = GetScrollWheelDelta();
        if (scrollDelta != 0)
        {
            float stepAmount = (mStep > 0.0f) ? mStep : (mMaxValue - mMinValue) * 0.05f;
            SetValue(mValue + stepAmount * (scrollDelta > 0 ? 1.0f : -1.0f));
        }
    }
    else
    {
        if (mGrabberHovered)
        {
            mGrabberHovered = false;
            MarkDirty();
        }
    }

    // Keyboard/gamepad input when slider contains mouse or is being dragged
    if (containsMouse || mDragging)
    {
        float stepAmount = (mStep > 0.0f) ? mStep : (mMaxValue - mMinValue) * 0.05f;

        if (IsKeyJustDownRepeat(OCTAVE_KEY_LEFT) ||
            IsGamepadButtonJustDown(GAMEPAD_L_LEFT, 0) ||
            IsGamepadButtonJustDown(GAMEPAD_LEFT, 0))
        {
            SetValue(mValue - stepAmount);
        }

        if (IsKeyJustDownRepeat(OCTAVE_KEY_RIGHT) ||
            IsGamepadButtonJustDown(GAMEPAD_L_RIGHT, 0) ||
            IsGamepadButtonJustDown(GAMEPAD_RIGHT, 0))
        {
            SetValue(mValue + stepAmount);
        }
    }
}

void Slider::PreRender()
{
    Super::PreRender();

    if (IsDirty())
    {
        UpdateAppearance();
        UpdateGrabberPosition();
    }
}

void Slider::UpdateAppearance()
{
    // Update background
    mBackground->SetTexture(mBackgroundTexture.Get<Texture>());
    mBackground->SetColor(mBackgroundColor);

    // Update grabber based on state
    mGrabber->SetTexture(mGrabberTexture.Get<Texture>());

    if (mDragging)
    {
        mGrabber->SetColor(mGrabberPressedColor);
    }
    else if (mGrabberHovered)
    {
        mGrabber->SetColor(mGrabberHoveredColor);
    }
    else
    {
        mGrabber->SetColor(mGrabberColor);
    }
}

void Slider::UpdateGrabberPosition()
{
    float widgetWidth = GetWidth();
    float widgetHeight = GetHeight();

    // Position background (centered track)
    float trackY = (widgetHeight - mTrackHeight) * 0.5f;
    mBackground->SetAnchorMode(AnchorMode::TopLeft);
    mBackground->SetPosition(0.0f, trackY);
    mBackground->SetDimensions(widgetWidth, mTrackHeight);

    // Position grabber based on value ratio
    float ratio = GetValueRatio();
    float grabberRange = widgetWidth - mGrabberWidth;
    float grabberX = ratio * grabberRange;
    float grabberY = (widgetHeight - widgetHeight) * 0.5f; // Full height grabber

    mGrabber->SetAnchorMode(AnchorMode::TopLeft);
    mGrabber->SetPosition(grabberX, 0.0f);
    mGrabber->SetDimensions(mGrabberWidth, widgetHeight);
}

float Slider::GetValueFromPosition(float x)
{
    Rect rect = GetRect();
    float localX = x - rect.mX;

    // Account for grabber width (half on each side)
    float halfGrabber = mGrabberWidth * 0.5f;
    float trackStart = halfGrabber;
    float trackEnd = rect.mWidth - halfGrabber;
    float trackWidth = trackEnd - trackStart;

    if (trackWidth <= 0.0f)
    {
        return mMinValue;
    }

    // Calculate ratio
    float ratio = (localX - trackStart) / trackWidth;
    ratio = glm::clamp(ratio, 0.0f, 1.0f);

    // Convert to value
    float value = mMinValue + ratio * (mMaxValue - mMinValue);
    return SnapToStep(value);
}

float Slider::GetValueRatio() const
{
    if (mMaxValue <= mMinValue)
    {
        return 0.0f;
    }
    return glm::clamp((mValue - mMinValue) / (mMaxValue - mMinValue), 0.0f, 1.0f);
}

float Slider::SnapToStep(float value)
{
    if (mStep <= 0.0f)
    {
        return glm::clamp(value, mMinValue, mMaxValue);
    }

    float snapped = mMinValue + glm::round((value - mMinValue) / mStep) * mStep;
    return glm::clamp(snapped, mMinValue, mMaxValue);
}

void Slider::SetValue(float value)
{
    float newValue = SnapToStep(value);

    if (mValue != newValue)
    {
        mValue = newValue;
        MarkDirty();

        if (!IsDestroyed())
        {
            EmitSignal("ValueChanged", { this });
            CallFunction("OnValueChanged", { this });
        }
    }
}

float Slider::GetValue() const
{
    return mValue;
}

void Slider::SetMinValue(float minValue)
{
    if (mMinValue != minValue)
    {
        mMinValue = minValue;

        // Re-clamp current value
        if (mValue < mMinValue)
        {
            SetValue(mMinValue);
        }

        MarkDirty();
    }
}

float Slider::GetMinValue() const
{
    return mMinValue;
}

void Slider::SetMaxValue(float maxValue)
{
    if (mMaxValue != maxValue)
    {
        mMaxValue = maxValue;

        // Re-clamp current value
        if (mValue > mMaxValue)
        {
            SetValue(mMaxValue);
        }

        MarkDirty();
    }
}

float Slider::GetMaxValue() const
{
    return mMaxValue;
}

void Slider::SetStep(float step)
{
    if (mStep != step)
    {
        mStep = step;

        // Re-snap current value
        SetValue(mValue);
    }
}

float Slider::GetStep() const
{
    return mStep;
}

void Slider::SetRange(float minValue, float maxValue)
{
    mMinValue = minValue;
    mMaxValue = maxValue;

    // Re-clamp current value
    SetValue(mValue);
}

void Slider::SetBackgroundTexture(Texture* texture)
{
    if (mBackgroundTexture != texture)
    {
        mBackgroundTexture = texture;
        MarkDirty();
    }
}

Texture* Slider::GetBackgroundTexture()
{
    return mBackgroundTexture.Get<Texture>();
}

void Slider::SetGrabberTexture(Texture* texture)
{
    if (mGrabberTexture != texture)
    {
        mGrabberTexture = texture;
        MarkDirty();
    }
}

Texture* Slider::GetGrabberTexture()
{
    return mGrabberTexture.Get<Texture>();
}

void Slider::SetBackgroundColor(glm::vec4 color)
{
    if (mBackgroundColor != color)
    {
        mBackgroundColor = color;
        MarkDirty();
    }
}

glm::vec4 Slider::GetBackgroundColor()
{
    return mBackgroundColor;
}

void Slider::SetGrabberColor(glm::vec4 color)
{
    if (mGrabberColor != color)
    {
        mGrabberColor = color;
        MarkDirty();
    }
}

glm::vec4 Slider::GetGrabberColor()
{
    return mGrabberColor;
}

void Slider::SetGrabberHoveredColor(glm::vec4 color)
{
    if (mGrabberHoveredColor != color)
    {
        mGrabberHoveredColor = color;
        MarkDirty();
    }
}

glm::vec4 Slider::GetGrabberHoveredColor()
{
    return mGrabberHoveredColor;
}

void Slider::SetGrabberPressedColor(glm::vec4 color)
{
    if (mGrabberPressedColor != color)
    {
        mGrabberPressedColor = color;
        MarkDirty();
    }
}

glm::vec4 Slider::GetGrabberPressedColor()
{
    return mGrabberPressedColor;
}

void Slider::SetGrabberWidth(float width)
{
    if (mGrabberWidth != width)
    {
        mGrabberWidth = width;
        MarkDirty();
    }
}

float Slider::GetGrabberWidth() const
{
    return mGrabberWidth;
}

void Slider::SetTrackHeight(float height)
{
    if (mTrackHeight != height)
    {
        mTrackHeight = height;
        MarkDirty();
    }
}

float Slider::GetTrackHeight() const
{
    return mTrackHeight;
}

bool Slider::IsDragging() const
{
    return mDragging;
}

Quad* Slider::GetBackground()
{
    return mBackground;
}

Quad* Slider::GetGrabber()
{
    return mGrabber;
}
