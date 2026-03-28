
#include "Nodes/Widgets/CheckBox.h"
#include "Nodes/Widgets/Quad.h"
#include "Nodes/Widgets/Text.h"
#include "InputDevices.h"
#include "Engine.h"
#include "Renderer.h"
#include "Assets/Texture.h"

FORCE_LINK_DEF(CheckBox);
DEFINE_NODE(CheckBox, Widget);

bool CheckBox::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);

    OCT_ASSERT(prop != nullptr);
    CheckBox* checkBox = static_cast<CheckBox*>(prop->mOwner);
    bool success = false;

    checkBox->MarkDirty();

    return success;
}

void CheckBox::Create()
{
    Super::Create();

    SetName("CheckBox");

    mBoxQuad = CreateChild<Quad>("Box");
    mText = CreateChild<Text>("Text");

    mBoxQuad->SetTransient(true);
    mText->SetTransient(true);

#if EDITOR
    mBoxQuad->mHiddenInTree = true;
    mText->mHiddenInTree = true;
#endif

    // Setup box quad - TopLeft, manually centered vertically
    mBoxQuad->SetAnchorMode(AnchorMode::TopLeft);
    mBoxQuad->SetColor(mUncheckedColor);

    // Setup text - FullStretch with left margin for box+spacing
    mText->SetAnchorMode(AnchorMode::FullStretch);
    mText->SetHorizontalJustification(Justification::Left);
    mText->SetVerticalJustification(Justification::Center);
    mText->SetText(mTextContent);
    mText->SetColor(mTextColor);

    // Default dimensions
    SetDimensions(150, 24);

    MarkDirty();
}

void CheckBox::GatherProperties(std::vector<Property>& props)
{
    Super::GatherProperties(props);

    {
        SCOPED_CATEGORY("CheckBox");

        props.push_back(Property(DatumType::Bool, "Checked", this, &mChecked, 1, HandlePropChange));
        props.push_back(Property(DatumType::String, "Text", this, &mTextContent, 1, HandlePropChange));
        props.push_back(Property(DatumType::Float, "Box Size", this, &mBoxSize, 1, HandlePropChange));
        props.push_back(Property(DatumType::Float, "Spacing", this, &mSpacing, 1, HandlePropChange));
    }

    {
        SCOPED_CATEGORY("Visual");

        props.push_back(Property(DatumType::Asset, "Checked Texture", this, &mCheckedTexture, 1, HandlePropChange, int32_t(Texture::GetStaticType())));
        props.push_back(Property(DatumType::Asset, "Unchecked Texture", this, &mUncheckedTexture, 1, HandlePropChange, int32_t(Texture::GetStaticType())));
        props.push_back(Property(DatumType::Color, "Checked Color", this, &mCheckedColor, 1, HandlePropChange));
        props.push_back(Property(DatumType::Color, "Unchecked Color", this, &mUncheckedColor, 1, HandlePropChange));
        props.push_back(Property(DatumType::Color, "Hovered Color", this, &mHoveredColor, 1, HandlePropChange));
        props.push_back(Property(DatumType::Color, "Text Color", this, &mTextColor, 1, HandlePropChange));
    }

    {
        SCOPED_CATEGORY("Text Style");
        mText->GatherTextProperties(props);
    }
}

void CheckBox::Tick(float deltaTime)
{
    Super::Tick(deltaTime);

    if (!ShouldHandleInput())
    {
        return;
    }

    if (IsDirty())
    {
        UpdateRect();
    }

    bool containsMouse = ContainsMouse();

    // Update hover state
    if (mHovered != containsMouse)
    {
        mHovered = containsMouse;
        MarkDirty();
    }

    // Handle click to toggle
    if (IsPointerJustUp(0) && containsMouse)
    {
        Toggle();
    }
}

void CheckBox::EditorTick(float deltaTime)
{
    Super::EditorTick(deltaTime);

    if (IsDirty())
    {
        UpdateAppearance();
    }
}

void CheckBox::PreRender()
{
    Super::PreRender();

    if (IsDirty())
    {
        UpdateAppearance();
    }
}

void CheckBox::UpdateAppearance()
{
    if (mBoxQuad == nullptr || mText == nullptr)
    {
        return;
    }

    float height = GetHeight();

    // Update box - centered vertically within parent
    float boxY = (height - mBoxSize) * 0.5f;
    mBoxQuad->SetPosition(0.0f, boxY);
    mBoxQuad->SetDimensions(mBoxSize, mBoxSize);

    // Update box texture and color
    if (mChecked)
    {
        mBoxQuad->SetTexture(mCheckedTexture.Get<Texture>());
        mBoxQuad->SetColor(mCheckedColor);
    }
    else
    {
        mBoxQuad->SetTexture(mUncheckedTexture.Get<Texture>());
        if (mHovered)
        {
            mBoxQuad->SetColor(mHoveredColor);
        }
        else
        {
            mBoxQuad->SetColor(mUncheckedColor);
        }
    }

    // Update text - FullStretch with left margin for box+spacing
    float leftMargin = mBoxSize + mSpacing;
    mText->SetMargins(leftMargin, 0.0f, 0.0f, 0.0f);
    mText->SetText(mTextContent);
    mText->SetColor(mTextColor);
}

void CheckBox::SetChecked(bool checked)
{
    if (mChecked != checked)
    {
        mChecked = checked;
        MarkDirty();

        if (!IsDestroyed())
        {
            EmitSignal("Toggled", { this });
            CallFunction("OnToggled", { this });
        }
    }
}

bool CheckBox::IsChecked() const
{
    return mChecked;
}

void CheckBox::Toggle()
{
    SetChecked(!mChecked);
}

void CheckBox::SetText(const std::string& text)
{
    if (mTextContent != text)
    {
        mTextContent = text;
        MarkDirty();
    }
}

const std::string& CheckBox::GetText() const
{
    return mTextContent;
}

void CheckBox::SetCheckedTexture(Texture* texture)
{
    if (mCheckedTexture != texture)
    {
        mCheckedTexture = texture;
        MarkDirty();
    }
}

Texture* CheckBox::GetCheckedTexture()
{
    return mCheckedTexture.Get<Texture>();
}

void CheckBox::SetUncheckedTexture(Texture* texture)
{
    if (mUncheckedTexture != texture)
    {
        mUncheckedTexture = texture;
        MarkDirty();
    }
}

Texture* CheckBox::GetUncheckedTexture()
{
    return mUncheckedTexture.Get<Texture>();
}

void CheckBox::SetCheckedColor(glm::vec4 color)
{
    if (mCheckedColor != color)
    {
        mCheckedColor = color;
        MarkDirty();
    }
}

glm::vec4 CheckBox::GetCheckedColor()
{
    return mCheckedColor;
}

void CheckBox::SetUncheckedColor(glm::vec4 color)
{
    if (mUncheckedColor != color)
    {
        mUncheckedColor = color;
        MarkDirty();
    }
}

glm::vec4 CheckBox::GetUncheckedColor()
{
    return mUncheckedColor;
}

void CheckBox::SetTextColor(glm::vec4 color)
{
    if (mTextColor != color)
    {
        mTextColor = color;
        MarkDirty();
    }
}

glm::vec4 CheckBox::GetTextColor()
{
    return mTextColor;
}

void CheckBox::SetBoxSize(float size)
{
    if (mBoxSize != size)
    {
        mBoxSize = size;
        MarkDirty();
    }
}

float CheckBox::GetBoxSize() const
{
    return mBoxSize;
}

void CheckBox::SetSpacing(float spacing)
{
    if (mSpacing != spacing)
    {
        mSpacing = spacing;
        MarkDirty();
    }
}

float CheckBox::GetSpacing() const
{
    return mSpacing;
}

Quad* CheckBox::GetBoxQuad()
{
    return mBoxQuad;
}

Text* CheckBox::GetTextWidget()
{
    return mText;
}
