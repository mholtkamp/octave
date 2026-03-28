
#include "Nodes/Widgets/LineEdit.h"
#include "Nodes/Widgets/Text.h"
#include "Nodes/Widgets/InputField.h"
#include "Engine.h"
#include "Renderer.h"

FORCE_LINK_DEF(LineEdit);
DEFINE_NODE(LineEdit, Widget);

bool LineEdit::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);

    OCT_ASSERT(prop != nullptr);
    LineEdit* lineEdit = static_cast<LineEdit*>(prop->mOwner);
    bool success = false;

    lineEdit->MarkDirty();

    return success;
}

void LineEdit::Create()
{
    Super::Create();

    SetName("LineEdit");

    mTitleWidget = CreateChild<Text>("Title");
    mInputField = CreateChild<InputField>("InputField");

    mTitleWidget->SetTransient(true);
    mInputField->SetTransient(true);

#if EDITOR
    mTitleWidget->mHiddenInTree = true;
    mInputField->mHiddenInTree = true;
#endif

    // Setup title
    mTitleWidget->SetAnchorMode(AnchorMode::MidLeft);
    mTitleWidget->SetHorizontalJustification(Justification::Left);
    mTitleWidget->SetVerticalJustification(Justification::Center);
    mTitleWidget->SetText(mTitle);
    mTitleWidget->SetColor(mTitleColor);

    // Setup input field - positioned to the right of title
    mInputField->SetAnchorMode(AnchorMode::MidLeft);

    // Default dimensions
    SetDimensions(300, 30);

    MarkDirty();
}

void LineEdit::GatherProperties(std::vector<Property>& props)
{
    Super::GatherProperties(props);

    {
        SCOPED_CATEGORY("LineEdit");

        props.push_back(Property(DatumType::String, "Title", this, &mTitle, 1, HandlePropChange));
        props.push_back(Property(DatumType::Float, "Title Width", this, &mTitleWidth, 1, HandlePropChange));
        props.push_back(Property(DatumType::Float, "Spacing", this, &mSpacing, 1, HandlePropChange));
        props.push_back(Property(DatumType::Color, "Title Color", this, &mTitleColor, 1, HandlePropChange));
    }

    // Gather InputField properties
    if (mInputField != nullptr)
    {
        mInputField->GatherProperties(props);
    }
}

void LineEdit::PreRender()
{
    Super::PreRender();

    if (IsDirty())
    {
        UpdateLayout();
    }
}

void LineEdit::UpdateLayout()
{
    // Update title
    mTitleWidget->SetText(mTitle);
    mTitleWidget->SetColor(mTitleColor);
    mTitleWidget->SetPosition(0.0f, 0.0f);
    mTitleWidget->SetDimensions(mTitleWidth, GetHeight());

    // Update input field - position to the right of title with spacing
    float inputX = mTitleWidth + mSpacing;
    float inputWidth = GetWidth() - inputX;

    mInputField->SetPosition(inputX, 0.0f);
    mInputField->SetDimensions(inputWidth, GetHeight());
}

void LineEdit::SetTitle(const std::string& title)
{
    if (mTitle != title)
    {
        mTitle = title;
        MarkDirty();
    }
}

const std::string& LineEdit::GetTitle() const
{
    return mTitle;
}

void LineEdit::SetTitleWidth(float width)
{
    if (mTitleWidth != width)
    {
        mTitleWidth = width;
        MarkDirty();
    }
}

float LineEdit::GetTitleWidth() const
{
    return mTitleWidth;
}

// InputField passthrough methods

void LineEdit::SetText(const std::string& text)
{
    if (mInputField != nullptr)
    {
        mInputField->SetText(text);
    }
}

const std::string& LineEdit::GetText() const
{
    static std::string empty;
    return mInputField != nullptr ? mInputField->GetText() : empty;
}

void LineEdit::SetPlaceholder(const std::string& placeholder)
{
    if (mInputField != nullptr)
    {
        mInputField->SetPlaceholder(placeholder);
    }
}

const std::string& LineEdit::GetPlaceholder() const
{
    static std::string empty;
    return mInputField != nullptr ? mInputField->GetPlaceholder() : empty;
}

void LineEdit::SetCaretPosition(int32_t pos)
{
    if (mInputField != nullptr)
    {
        mInputField->SetCaretPosition(pos);
    }
}

int32_t LineEdit::GetCaretPosition() const
{
    return mInputField != nullptr ? mInputField->GetCaretPosition() : 0;
}

void LineEdit::SelectAll()
{
    if (mInputField != nullptr)
    {
        mInputField->SelectAll();
    }
}

void LineEdit::ClearSelection()
{
    if (mInputField != nullptr)
    {
        mInputField->ClearSelection();
    }
}

bool LineEdit::HasSelection() const
{
    return mInputField != nullptr ? mInputField->HasSelection() : false;
}

std::string LineEdit::GetSelectedText() const
{
    return mInputField != nullptr ? mInputField->GetSelectedText() : "";
}

void LineEdit::DeleteSelection()
{
    if (mInputField != nullptr)
    {
        mInputField->DeleteSelection();
    }
}

void LineEdit::Select(int32_t start, int32_t end)
{
    if (mInputField != nullptr)
    {
        mInputField->Select(start, end);
    }
}

int32_t LineEdit::GetSelectionStart() const
{
    return mInputField != nullptr ? mInputField->GetSelectionStart() : -1;
}

int32_t LineEdit::GetSelectionEnd() const
{
    return mInputField != nullptr ? mInputField->GetSelectionEnd() : -1;
}

void LineEdit::SetFocused(bool focused)
{
    if (mInputField != nullptr)
    {
        mInputField->SetFocused(focused);
    }
}

bool LineEdit::IsFocused() const
{
    return mInputField != nullptr ? mInputField->IsFocused() : false;
}

void LineEdit::SetPasswordMode(bool enabled)
{
    if (mInputField != nullptr)
    {
        mInputField->SetPasswordMode(enabled);
    }
}

bool LineEdit::IsPasswordMode() const
{
    return mInputField != nullptr ? mInputField->IsPasswordMode() : false;
}

void LineEdit::SetMaxLength(int32_t maxLen)
{
    if (mInputField != nullptr)
    {
        mInputField->SetMaxLength(maxLen);
    }
}

int32_t LineEdit::GetMaxLength() const
{
    return mInputField != nullptr ? mInputField->GetMaxLength() : 0;
}

void LineEdit::SetEditable(bool editable)
{
    if (mInputField != nullptr)
    {
        mInputField->SetEditable(editable);
    }
}

bool LineEdit::IsEditable() const
{
    return mInputField != nullptr ? mInputField->IsEditable() : true;
}

Text* LineEdit::GetTitleWidget()
{
    return mTitleWidget;
}

InputField* LineEdit::GetInputField()
{
    return mInputField;
}
