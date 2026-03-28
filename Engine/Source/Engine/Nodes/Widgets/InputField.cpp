
#include "Nodes/Widgets/InputField.h"
#include "Nodes/Widgets/Quad.h"
#include "Nodes/Widgets/Text.h"
#include "InputDevices.h"
#include "Engine.h"
#include "Renderer.h"
#include "Utilities.h"
#include "System/System.h"
#include "Assets/Font.h"
#include "Assets/Texture.h"

FORCE_LINK_DEF(InputField);
DEFINE_NODE(InputField, Widget);

bool InputField::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);

    OCT_ASSERT(prop != nullptr);
    InputField* inputField = static_cast<InputField*>(prop->mOwner);
    bool success = false;

    inputField->MarkDirty();

    return success;
}

void InputField::Create()
{
    Super::Create();

    SetName("InputField");
    mUseScissor = true;

    mBackground = CreateChild<Quad>("Background");
    mSelection = CreateChild<Quad>("Selection");
    mText = CreateChild<Text>("Text");
    mCaret = CreateChild<Quad>("Caret");

    mBackground->SetTransient(true);
    mSelection->SetTransient(true);
    mText->SetTransient(true);
    mCaret->SetTransient(true);

#if EDITOR
    mBackground->mHiddenInTree = true;
    mSelection->mHiddenInTree = true;
    mText->mHiddenInTree = true;
    mCaret->mHiddenInTree = true;
#endif

    // Setup defaults
    mBackground->SetAnchorMode(AnchorMode::FullStretch);
    mBackground->SetRatios(0.0f, 0.0f, 1.0f, 1.0f);
    mBackground->SetColor(mBackgroundColor);

    mSelection->SetAnchorMode(AnchorMode::TopLeft);
    mSelection->SetColor(mSelectionColor);
    mSelection->SetVisible(false);

    // Text uses FullStretch to fill the widget, with left justification
    mText->SetAnchorMode(AnchorMode::FullStretch);
    mText->SetMargins(mTextPadding, 0.0f, mTextPadding, 0.0f);
    mText->SetHorizontalJustification(Justification::Left);
    mText->SetVerticalJustification(Justification::Center);
    mText->SetColor(mTextColor);

    mCaret->SetAnchorMode(AnchorMode::TopLeft);
    mCaret->SetColor(mCaretColor);
    mCaret->SetVisible(false);

    // Default dimensions
    SetDimensions(200, 30);

    MarkDirty();
}

void InputField::GatherProperties(std::vector<Property>& props)
{
    Super::GatherProperties(props);
    GatherInputFieldProperties(props);
}

void InputField::GatherInputFieldProperties(std::vector<Property>& props)
{
    {
        SCOPED_CATEGORY("InputField");

        props.push_back(Property(DatumType::String, "Text", this, &mTextContent, 1, HandlePropChange));
        props.push_back(Property(DatumType::String, "Placeholder", this, &mPlaceholderText, 1, HandlePropChange));
        props.push_back(Property(DatumType::Bool, "Editable", this, &mEditable, 1, HandlePropChange));
        props.push_back(Property(DatumType::Bool, "Password Mode", this, &mPasswordMode, 1, HandlePropChange));
        props.push_back(Property(DatumType::Integer, "Max Length", this, &mMaxLength, 1, HandlePropChange));
        props.push_back(Property(DatumType::Float, "Text Padding", this, &mTextPadding, 1, HandlePropChange));
    }

    {
        SCOPED_CATEGORY("Colors");
        props.push_back(Property(DatumType::Asset, "Background Texture", this, &mBackgroundTexture, 1, HandlePropChange, int32_t(Texture::GetStaticType())));
        props.push_back(Property(DatumType::Color, "Background Color", this, &mBackgroundColor, 1, HandlePropChange));
        props.push_back(Property(DatumType::Color, "Focused Background Color", this, &mFocusedBackgroundColor, 1, HandlePropChange));
        props.push_back(Property(DatumType::Color, "Text Color", this, &mTextColor, 1, HandlePropChange));
        props.push_back(Property(DatumType::Color, "Placeholder Color", this, &mPlaceholderColor, 1, HandlePropChange));
        props.push_back(Property(DatumType::Color, "Caret Color", this, &mCaretColor, 1, HandlePropChange));
        props.push_back(Property(DatumType::Color, "Selection Color", this, &mSelectionColor, 1, HandlePropChange));
    }

    if (mText != nullptr)
    {
        SCOPED_CATEGORY("Text Style");
        mText->GatherTextProperties(props);
    }
}

void InputField::Tick(float deltaTime)
{
    Super::Tick(deltaTime);

    // Update caret blink
    if (mFocused)
    {
        mCaretBlinkTime += deltaTime;
        if (mCaretBlinkTime >= mCaretBlinkRate)
        {
            mCaretBlinkTime -= mCaretBlinkRate;
            mCaretVisible = !mCaretVisible;
            MarkDirty();
        }
    }

    if (!ShouldHandleInput())
    {
        return;
    }

    if (IsDirty())
    {
        UpdateRect();
    }

    bool containsMouse = ContainsMouse();

    // Handle click to focus
    if (IsPointerJustDown(0))
    {
        if (containsMouse)
        {
            SetFocused(true);

            // Position caret at click location
            int32_t mouseX, mouseY;
            GetMousePosition(mouseX, mouseY);
            mCaretPosition = GetCharacterAtPosition((float)mouseX);
            ClearSelection();
            mCaretBlinkTime = 0.0f;
            mCaretVisible = true;
            mSelectionAnchor = mCaretPosition;
            MarkDirty();
        }
        else
        {
            SetFocused(false);
        }
    }

    // Handle mouse drag for selection
    if (mFocused && IsPointerDown(0) && containsMouse)
    {
        int32_t mouseX, mouseY;
        GetMousePosition(mouseX, mouseY);
        int32_t newPos = GetCharacterAtPosition((float)mouseX);

        if (newPos != mCaretPosition)
        {
            mCaretPosition = newPos;

            if (mSelectionAnchor >= 0)
            {
                if (mCaretPosition != mSelectionAnchor)
                {
                    mSelectionStart = glm::min(mSelectionAnchor, mCaretPosition);
                    mSelectionEnd = glm::max(mSelectionAnchor, mCaretPosition);
                }
                else
                {
                    ClearSelection();
                }
            }

            mCaretBlinkTime = 0.0f;
            mCaretVisible = true;
            MarkDirty();
        }
    }

    // Handle text input only when focused
    if (!mFocused || !mEditable)
    {
        return;
    }

    // Process typed characters
    const std::vector<int32_t>& justDownKeys = GetEngineState()->mInput.mJustDownKeys;
    for (int32_t key : justDownKeys)
    {
        char c = (char)ConvertKeyCodeToChar((uint8_t)key, IsShiftDown());
        if (c != 0 && c >= 32)
        {
            InsertCharacter(c);
        }
    }

    // Navigation keys
    bool shift = IsShiftDown();
    bool ctrl = IsControlDown();

    if (IsKeyJustDownRepeat(OCTAVE_KEY_LEFT))
    {
        if (ctrl)
        {
            MoveCaretToWordBoundary(-1, shift);
        }
        else
        {
            MoveCaret(-1, shift);
        }
    }

    if (IsKeyJustDownRepeat(OCTAVE_KEY_RIGHT))
    {
        if (ctrl)
        {
            MoveCaretToWordBoundary(1, shift);
        }
        else
        {
            MoveCaret(1, shift);
        }
    }

    if (IsKeyJustDown(OCTAVE_KEY_HOME))
    {
        int32_t oldPos = mCaretPosition;
        mCaretPosition = 0;
        if (shift)
        {
            ExtendSelection();
        }
        else
        {
            ClearSelection();
        }
        if (oldPos != mCaretPosition)
        {
            mCaretBlinkTime = 0.0f;
            mCaretVisible = true;
            MarkDirty();
        }
    }

    if (IsKeyJustDown(OCTAVE_KEY_END))
    {
        int32_t oldPos = mCaretPosition;
        mCaretPosition = (int32_t)mTextContent.size();
        if (shift)
        {
            ExtendSelection();
        }
        else
        {
            ClearSelection();
        }
        if (oldPos != mCaretPosition)
        {
            mCaretBlinkTime = 0.0f;
            mCaretVisible = true;
            MarkDirty();
        }
    }

    // Delete keys
    if (IsKeyJustDownRepeat(OCTAVE_KEY_BACKSPACE))
    {
        if (HasSelection())
        {
            DeleteSelection();
        }
        else
        {
            DeleteCharacter(false);
        }
    }

    if (IsKeyJustDownRepeat(OCTAVE_KEY_DELETE))
    {
        if (HasSelection())
        {
            DeleteSelection();
        }
        else
        {
            DeleteCharacter(true);
        }
    }

    // Enter/submit
    if (IsKeyJustDown(OCTAVE_KEY_ENTER))
    {
        EmitSignal("TextSubmitted", { this });
        CallFunction("OnTextSubmitted", { this });
    }

    // Copy/Paste/Cut
    if (ctrl)
    {
        if (IsKeyJustDown(OCTAVE_KEY_C) && HasSelection())
        {
            SYS_SetClipboardText(GetSelectedText());
        }

        if (IsKeyJustDown(OCTAVE_KEY_V))
        {
            std::string clipboard = SYS_GetClipboardText();
            if (!clipboard.empty())
            {
                if (HasSelection())
                {
                    DeleteSelection();
                }
                InsertText(clipboard);
            }
        }

        if (IsKeyJustDown(OCTAVE_KEY_X) && HasSelection())
        {
            SYS_SetClipboardText(GetSelectedText());
            DeleteSelection();
        }

        if (IsKeyJustDown(OCTAVE_KEY_A))
        {
            SelectAll();
        }
    }
}

void InputField::EditorTick(float deltaTime)
{
    Super::EditorTick(deltaTime);

    if (IsDirty())
    {
        UpdateDisplayText();
        UpdateCaretVisual();
        UpdateSelectionVisual();
    }
}

void InputField::PreRender()
{
    Super::PreRender();

    if (IsDirty())
    {
        UpdateDisplayText();
        UpdateCaretVisual();
        UpdateSelectionVisual();
    }
}

void InputField::InsertCharacter(char c)
{
    if (!mEditable)
    {
        return;
    }

    if (HasSelection())
    {
        DeleteSelection();
    }

    // Check max length
    if (mMaxLength > 0 && (int32_t)mTextContent.size() >= mMaxLength)
    {
        return;
    }

    mTextContent.insert(mCaretPosition, 1, c);
    mCaretPosition++;
    mCaretBlinkTime = 0.0f;
    mCaretVisible = true;
    EmitTextChanged();
    MarkDirty();
}

void InputField::InsertText(const std::string& text)
{
    if (!mEditable || text.empty())
    {
        return;
    }

    std::string filtered = text;

    // Remove newlines
    filtered.erase(std::remove(filtered.begin(), filtered.end(), '\n'), filtered.end());
    filtered.erase(std::remove(filtered.begin(), filtered.end(), '\r'), filtered.end());

    // Check max length
    if (mMaxLength > 0)
    {
        int32_t available = mMaxLength - (int32_t)mTextContent.size();
        if (available <= 0)
        {
            return;
        }
        if ((int32_t)filtered.size() > available)
        {
            filtered = filtered.substr(0, available);
        }
    }

    mTextContent.insert(mCaretPosition, filtered);
    mCaretPosition += (int32_t)filtered.size();
    mCaretBlinkTime = 0.0f;
    mCaretVisible = true;
    EmitTextChanged();
    MarkDirty();
}

void InputField::DeleteCharacter(bool forward)
{
    if (!mEditable)
    {
        return;
    }

    if (forward)
    {
        // Delete key
        if (mCaretPosition < (int32_t)mTextContent.size())
        {
            mTextContent.erase(mCaretPosition, 1);
            EmitTextChanged();
            MarkDirty();
        }
    }
    else
    {
        // Backspace
        if (mCaretPosition > 0)
        {
            mCaretPosition--;
            mTextContent.erase(mCaretPosition, 1);
            mCaretBlinkTime = 0.0f;
            mCaretVisible = true;
            EmitTextChanged();
            MarkDirty();
        }
    }
}

void InputField::MoveCaret(int32_t delta, bool select)
{
    int32_t oldPos = mCaretPosition;

    // If we have a selection and not extending it, jump to edge
    if (HasSelection() && !select)
    {
        if (delta < 0)
        {
            mCaretPosition = mSelectionStart;
        }
        else
        {
            mCaretPosition = mSelectionEnd;
        }
        ClearSelection();
    }
    else
    {
        mCaretPosition = glm::clamp(mCaretPosition + delta, 0, (int32_t)mTextContent.size());
    }

    if (select)
    {
        ExtendSelection();
    }
    else
    {
        ClearSelection();
        mSelectionAnchor = mCaretPosition;
    }

    if (oldPos != mCaretPosition)
    {
        mCaretBlinkTime = 0.0f;
        mCaretVisible = true;
        MarkDirty();
    }
}

void InputField::MoveCaretToWordBoundary(int32_t direction, bool select)
{
    int32_t oldPos = mCaretPosition;

    if (direction < 0)
    {
        // Move left to word boundary
        while (mCaretPosition > 0 && mTextContent[mCaretPosition - 1] == ' ')
        {
            mCaretPosition--;
        }
        while (mCaretPosition > 0 && mTextContent[mCaretPosition - 1] != ' ')
        {
            mCaretPosition--;
        }
    }
    else
    {
        // Move right to word boundary
        int32_t len = (int32_t)mTextContent.size();
        while (mCaretPosition < len && mTextContent[mCaretPosition] != ' ')
        {
            mCaretPosition++;
        }
        while (mCaretPosition < len && mTextContent[mCaretPosition] == ' ')
        {
            mCaretPosition++;
        }
    }

    if (select)
    {
        ExtendSelection();
    }
    else
    {
        ClearSelection();
        mSelectionAnchor = mCaretPosition;
    }

    if (oldPos != mCaretPosition)
    {
        mCaretBlinkTime = 0.0f;
        mCaretVisible = true;
        MarkDirty();
    }
}

void InputField::ExtendSelection()
{
    if (mSelectionAnchor < 0)
    {
        mSelectionAnchor = mCaretPosition;
    }

    if (mCaretPosition != mSelectionAnchor)
    {
        mSelectionStart = glm::min(mSelectionAnchor, mCaretPosition);
        mSelectionEnd = glm::max(mSelectionAnchor, mCaretPosition);
    }
    else
    {
        ClearSelection();
    }
}

float InputField::GetCaretXPosition()
{
    Font* font = mText->GetFont();
    if (font == nullptr)
    {
        return mTextPadding;
    }

    float textSize = mText->GetTextSize();
    float scale = textSize / (float)font->GetSize();
    const std::vector<Character>& fontChars = font->GetCharacters();

    std::string displayText = mPasswordMode ? std::string(mTextContent.size(), mPasswordChar) : mTextContent;

    float x = mTextPadding;
    int32_t pos = glm::min(mCaretPosition, (int32_t)displayText.size());

    for (int32_t i = 0; i < pos; i++)
    {
        char c = displayText[i];
        if (c >= ' ' && c <= '~')
        {
            const Character& charInfo = fontChars[c - ' '];
            x += charInfo.mAdvance * scale;
        }
    }

    return x - mScrollOffset;
}

int32_t InputField::GetCharacterAtPosition(float x)
{
    Font* font = mText->GetFont();
    if (font == nullptr)
    {
        return 0;
    }

    Rect rect = GetRect();
    float localX = x - rect.mX - mTextPadding + mScrollOffset;

    if (localX <= 0.0f)
    {
        return 0;
    }

    float textSize = mText->GetTextSize();
    float scale = textSize / (float)font->GetSize();
    const std::vector<Character>& fontChars = font->GetCharacters();

    std::string displayText = mPasswordMode ? std::string(mTextContent.size(), mPasswordChar) : mTextContent;

    float accum = 0.0f;
    for (int32_t i = 0; i < (int32_t)displayText.size(); i++)
    {
        char c = displayText[i];
        if (c >= ' ' && c <= '~')
        {
            const Character& charInfo = fontChars[c - ' '];
            float charWidth = charInfo.mAdvance * scale;
            if (localX < accum + charWidth * 0.5f)
            {
                return i;
            }
            accum += charWidth;
        }
    }

    return (int32_t)displayText.size();
}

void InputField::UpdateDisplayText()
{
    if (mBackground == nullptr || mText == nullptr)
    {
        return;
    }

    mBackground->SetTexture(mBackgroundTexture.Get<Texture>());
    mBackground->SetColor(mFocused ? mFocusedBackgroundColor : mBackgroundColor);

    if (mTextContent.empty())
    {
        mText->SetText(mPlaceholderText);
        mText->SetColor(mPlaceholderColor);
    }
    else if (mPasswordMode)
    {
        mText->SetText(std::string(mTextContent.size(), mPasswordChar));
        mText->SetColor(mTextColor);
    }
    else
    {
        mText->SetText(mTextContent);
        mText->SetColor(mTextColor);
    }

    // Update text margins for padding and scrolling
    mText->SetLeftMargin(mTextPadding - mScrollOffset);
}

void InputField::UpdateCaretVisual()
{
    if (mCaret == nullptr)
    {
        return;
    }

    if (!mFocused || !mCaretVisible)
    {
        mCaret->SetVisible(false);
        return;
    }

    float caretX = GetCaretXPosition();
    float caretWidth = 2.0f;
    float widgetHeight = GetHeight();
    float caretPadding = 4.0f;
    float caretHeight = widgetHeight - caretPadding * 2.0f;

    mCaret->SetPosition(caretX, caretPadding);
    mCaret->SetDimensions(caretWidth, caretHeight);
    mCaret->SetColor(mCaretColor);
    mCaret->SetVisible(true);
}

void InputField::UpdateSelectionVisual()
{
    if (mSelection == nullptr)
    {
        return;
    }

    if (!HasSelection())
    {
        mSelection->SetVisible(false);
        return;
    }

    Font* font = mText->GetFont();
    if (font == nullptr)
    {
        mSelection->SetVisible(false);
        return;
    }

    float textSize = mText->GetTextSize();
    float scale = textSize / (float)font->GetSize();
    const std::vector<Character>& fontChars = font->GetCharacters();

    std::string displayText = mPasswordMode ? std::string(mTextContent.size(), mPasswordChar) : mTextContent;

    // Calculate selection start X
    float startX = mTextPadding;
    for (int32_t i = 0; i < mSelectionStart && i < (int32_t)displayText.size(); i++)
    {
        char c = displayText[i];
        if (c >= ' ' && c <= '~')
        {
            const Character& charInfo = fontChars[c - ' '];
            startX += charInfo.mAdvance * scale;
        }
    }

    // Calculate selection end X
    float endX = mTextPadding;
    for (int32_t i = 0; i < mSelectionEnd && i < (int32_t)displayText.size(); i++)
    {
        char c = displayText[i];
        if (c >= ' ' && c <= '~')
        {
            const Character& charInfo = fontChars[c - ' '];
            endX += charInfo.mAdvance * scale;
        }
    }

    float widgetHeight = GetHeight();
    float selectionPadding = 4.0f;

    mSelection->SetPosition(startX - mScrollOffset, selectionPadding);
    mSelection->SetDimensions(endX - startX, widgetHeight - selectionPadding * 2.0f);
    mSelection->SetColor(mSelectionColor);
    mSelection->SetVisible(true);
}

void InputField::EnsureCaretVisible()
{
    float caretX = GetCaretXPosition();
    float widgetWidth = GetWidth();

    if (caretX < mTextPadding)
    {
        mScrollOffset += caretX - mTextPadding;
    }
    else if (caretX > widgetWidth - mTextPadding)
    {
        mScrollOffset += caretX - (widgetWidth - mTextPadding);
    }

    if (mScrollOffset < 0.0f)
    {
        mScrollOffset = 0.0f;
    }
}

void InputField::EmitTextChanged()
{
    if (!IsDestroyed())
    {
        EmitSignal("TextChanged", { this });
        CallFunction("OnTextChanged", { this });
    }
}

void InputField::SetText(const std::string& text)
{
    if (mTextContent != text)
    {
        mTextContent = text;

        // Enforce max length
        if (mMaxLength > 0 && (int32_t)mTextContent.size() > mMaxLength)
        {
            mTextContent = mTextContent.substr(0, mMaxLength);
        }

        // Clamp caret position
        mCaretPosition = glm::clamp(mCaretPosition, 0, (int32_t)mTextContent.size());
        ClearSelection();
        mScrollOffset = 0.0f;
        MarkDirty();
    }
}

const std::string& InputField::GetText() const
{
    return mTextContent;
}

void InputField::SetPlaceholder(const std::string& placeholder)
{
    if (mPlaceholderText != placeholder)
    {
        mPlaceholderText = placeholder;
        MarkDirty();
    }
}

const std::string& InputField::GetPlaceholder() const
{
    return mPlaceholderText;
}

void InputField::SetCaretPosition(int32_t pos)
{
    pos = glm::clamp(pos, 0, (int32_t)mTextContent.size());
    if (mCaretPosition != pos)
    {
        mCaretPosition = pos;
        mCaretBlinkTime = 0.0f;
        mCaretVisible = true;
        MarkDirty();
    }
}

int32_t InputField::GetCaretPosition() const
{
    return mCaretPosition;
}

void InputField::SelectAll()
{
    if (!mTextContent.empty())
    {
        mSelectionStart = 0;
        mSelectionEnd = (int32_t)mTextContent.size();
        mSelectionAnchor = 0;
        mCaretPosition = mSelectionEnd;
        MarkDirty();
    }
}

void InputField::ClearSelection()
{
    if (mSelectionStart >= 0 || mSelectionEnd >= 0)
    {
        mSelectionStart = -1;
        mSelectionEnd = -1;
        MarkDirty();
    }
}

bool InputField::HasSelection() const
{
    return mSelectionStart >= 0 && mSelectionEnd >= 0 && mSelectionStart != mSelectionEnd;
}

std::string InputField::GetSelectedText() const
{
    if (!HasSelection())
    {
        return "";
    }

    return mTextContent.substr(mSelectionStart, mSelectionEnd - mSelectionStart);
}

void InputField::DeleteSelection()
{
    if (!HasSelection() || !mEditable)
    {
        return;
    }

    mTextContent.erase(mSelectionStart, mSelectionEnd - mSelectionStart);
    mCaretPosition = mSelectionStart;
    ClearSelection();
    mSelectionAnchor = mCaretPosition;
    mCaretBlinkTime = 0.0f;
    mCaretVisible = true;
    EmitTextChanged();
    MarkDirty();
}

void InputField::Select(int32_t start, int32_t end)
{
    start = glm::clamp(start, 0, (int32_t)mTextContent.size());
    end = glm::clamp(end, 0, (int32_t)mTextContent.size());

    if (start > end)
    {
        std::swap(start, end);
    }

    mSelectionStart = start;
    mSelectionEnd = end;
    mSelectionAnchor = start;
    mCaretPosition = end;
    MarkDirty();
}

int32_t InputField::GetSelectionStart() const
{
    return mSelectionStart;
}

int32_t InputField::GetSelectionEnd() const
{
    return mSelectionEnd;
}

void InputField::SetFocused(bool focused)
{
    if (mFocused != focused)
    {
        mFocused = focused;

        if (focused)
        {
            mCaretBlinkTime = 0.0f;
            mCaretVisible = true;
            mSelectionAnchor = mCaretPosition;
        }
        else
        {
            ClearSelection();
        }

        MarkDirty();

        if (!IsDestroyed())
        {
            EmitSignal("FocusChanged", { this });
            CallFunction("OnFocusChanged", { this });
        }
    }
}

bool InputField::IsFocused() const
{
    return mFocused;
}

void InputField::SetPasswordMode(bool enabled)
{
    if (mPasswordMode != enabled)
    {
        mPasswordMode = enabled;
        MarkDirty();
    }
}

bool InputField::IsPasswordMode() const
{
    return mPasswordMode;
}

void InputField::SetMaxLength(int32_t maxLen)
{
    if (mMaxLength != maxLen)
    {
        mMaxLength = maxLen;

        // Truncate if needed
        if (mMaxLength > 0 && (int32_t)mTextContent.size() > mMaxLength)
        {
            mTextContent = mTextContent.substr(0, mMaxLength);
            mCaretPosition = glm::clamp(mCaretPosition, 0, (int32_t)mTextContent.size());
            ClearSelection();
            MarkDirty();
        }
    }
}

int32_t InputField::GetMaxLength() const
{
    return mMaxLength;
}

void InputField::SetEditable(bool editable)
{
    mEditable = editable;
}

bool InputField::IsEditable() const
{
    return mEditable;
}

void InputField::SetBackgroundTexture(Texture* texture)
{
    if (mBackgroundTexture != texture)
    {
        mBackgroundTexture = texture;
        MarkDirty();
    }
}

Texture* InputField::GetBackgroundTexture()
{
    return mBackgroundTexture.Get<Texture>();
}

void InputField::SetBackgroundColor(glm::vec4 color)
{
    if (mBackgroundColor != color)
    {
        mBackgroundColor = color;
        MarkDirty();
    }
}

glm::vec4 InputField::GetBackgroundColor()
{
    return mBackgroundColor;
}

void InputField::SetFocusedBackgroundColor(glm::vec4 color)
{
    if (mFocusedBackgroundColor != color)
    {
        mFocusedBackgroundColor = color;
        MarkDirty();
    }
}

glm::vec4 InputField::GetFocusedBackgroundColor()
{
    return mFocusedBackgroundColor;
}

void InputField::SetTextColor(glm::vec4 color)
{
    if (mTextColor != color)
    {
        mTextColor = color;
        MarkDirty();
    }
}

glm::vec4 InputField::GetTextColor()
{
    return mTextColor;
}

void InputField::SetPlaceholderColor(glm::vec4 color)
{
    if (mPlaceholderColor != color)
    {
        mPlaceholderColor = color;
        MarkDirty();
    }
}

glm::vec4 InputField::GetPlaceholderColor()
{
    return mPlaceholderColor;
}

void InputField::SetCaretColor(glm::vec4 color)
{
    if (mCaretColor != color)
    {
        mCaretColor = color;
        MarkDirty();
    }
}

glm::vec4 InputField::GetCaretColor()
{
    return mCaretColor;
}

void InputField::SetSelectionColor(glm::vec4 color)
{
    if (mSelectionColor != color)
    {
        mSelectionColor = color;
        MarkDirty();
    }
}

glm::vec4 InputField::GetSelectionColor()
{
    return mSelectionColor;
}

void InputField::SetTextPadding(float padding)
{
    if (mTextPadding != padding)
    {
        mTextPadding = padding;
        MarkDirty();
    }
}

float InputField::GetTextPadding() const
{
    return mTextPadding;
}

Quad* InputField::GetBackground()
{
    return mBackground;
}

Text* InputField::GetTextWidget()
{
    return mText;
}

Quad* InputField::GetCaret()
{
    return mCaret;
}
