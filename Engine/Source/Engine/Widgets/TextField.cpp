#include "Widgets/TextField.h"
#include "Engine.h"
#include "Clock.h"
#include "InputDevices.h"
#include "Widgets/Text.h"
#include "Widgets/Quad.h"
#include "Renderer.h"
#include "ScriptEvent.h"

#include "Components/ScriptComponent.h"

FORCE_LINK_DEF(TextField);
DEFINE_FACTORY(TextField, Widget);

TextField* TextField::sSelectedTextField = nullptr;
float TextField::sCursorBlinkTime = 0.0f;
const float TextField::sCursorBlinkPeriod = 0.3f;

TextField::TextField() :
    mCursorQuad(nullptr),
    mMaxCharacters(64)
{
    mCursorQuad = new Quad();
    mCursorQuad->SetVisible(false);
    mCursorQuad->SetAnchorMode(AnchorMode::FullStretch);
    mCursorQuad->SetMargins(0.0f, 0.0f, 0.0f, 0.0f);
    AddChild(mCursorQuad);
}

TextField::~TextField()
{
    if (sSelectedTextField == this)
    {
        sSelectedTextField = nullptr;
    }
}

void TextField::Update()
{
    // If not the active text field, just update as if it was a button.
    if (mState != ButtonState::Pressed)
    {
        Button::Update();
        return;
    }

    if (IsDirty())
    {
        UpdateAppearance();
    }

    if (sSelectedTextField != this)
    {
        SetSelectedTextField(this);
    }

    // Handle Blinky Cursor
    const Clock* clock = GetAppClock();
    float deltaTime = clock->DeltaTime();
    sCursorBlinkTime -= deltaTime;

    if (sCursorBlinkTime <= 0.0f)
    {
        // For now, cursor will just highlight the whole text field.
        mCursorQuad->SetDimensions(mRect.mWidth, mRect.mHeight);
        mCursorQuad->SetColor(glm::vec4(1, 1, 1, 0.2f));
        //mCursorQuad->SetDimensions(10, mAbsoluteRect.mHeight * 0.8f);
        //mCursorQuad->SetPosition(10, mAbsoluteRect.mHeight * 0.1f);

        mCursorQuad->SetVisible(!mCursorQuad->IsVisible());
        sCursorBlinkTime = sCursorBlinkPeriod;
    }

    // Add/Remove text characters based on keyboard input
    const std::vector<int32_t>& pressedKeys = GetEngineState()->mInput.mJustDownKeys;
    bool textStringModified = false;
    const bool shiftDown = IsShiftDown();

    for (uint32_t i = 0; i < pressedKeys.size(); ++i)
    {
        uint8_t keyCode = uint8_t(pressedKeys[i]);
        uint8_t charToAdd = ConvertKeyCodeToChar(keyCode, shiftDown);

        if (charToAdd >= ' ' &&
            charToAdd <= '~' &&
            mText->GetText().size() < mMaxCharacters)
        {
            std::string newText = mText->GetText();
            newText += charToAdd;
            mText->SetText(newText);
            textStringModified = true;
        }

        // Remove characters
        // TODO: If cursor movement is added, make backspace remove characters behind cursor.
        //        and delete remove characters in front of the cursor.
        if (keyCode == KEY_BACKSPACE||
            keyCode == KEY_DELETE)
        {
            if (IsControlDown())
            {
                // For now, delete everything
                mText->SetText("");
            }
            else
            {
                std::string newText = mText->GetText();

                if (!newText.empty())
                {
                    newText.pop_back();
                }

                mText->SetText(newText);
            }

            textStringModified = true;
        }
    }

    if (textStringModified)
    {
        if (mTextEditHandler.mFuncPointer != nullptr)
        {
            mTextEditHandler.mFuncPointer(this);
        }
        if (mTextEditHandler.mScriptTableName != "")
        {
            ScriptEvent::WidgetState(
                mTextEditHandler.mScriptTableName,
                mTextEditHandler.mScriptFuncName,
                this);
        }
    }

    if (IsKeyJustDown(KEY_ENTER) && IsShiftDown())
    {
        std::string newText = mText->GetText();
        newText.push_back('\n');
        mText->SetText(newText);
    }
    else if (IsKeyJustDown(KEY_ENTER) ||
        ((IsMouseButtonJustUp(MOUSE_LEFT) || IsMouseButtonJustUp(MOUSE_RIGHT)) && !ContainsMouse()))
    {
        SetSelectedTextField(nullptr);
        Button::Update();
    }
    else
    {
        Widget::Update();
    }
}

void TextField::SetState(ButtonState newState)
{
    const bool justPressed = newState != mState && newState == ButtonState::Pressed;
    Button::SetState(newState);
    mCursorQuad->SetVisible(newState == ButtonState::Pressed);

    if (justPressed)
    {
        OnPressed();
    }
}

void TextField::SetTextString(const std::string& newTextString)
{
    if (sSelectedTextField != this)
    {
        Button::SetTextString(newTextString);
    }
}

void TextField::SetTextEditHandler(TextFieldHandlerFP handler)
{
    mTextEditHandler.mFuncPointer = handler;
}

void TextField::SetTextConfirmHandler(TextFieldHandlerFP handler)
{
    mTextConfirmHandler.mFuncPointer = handler;
}

void TextField::SetScriptTextEditHandler(const char* tableName, const char* funcName)
{
    mTextEditHandler.mScriptTableName = tableName;
    mTextEditHandler.mScriptFuncName = funcName;
}

void TextField::SetScriptTextConfirmHandler(const char* tableName, const char* funcName)
{
    mTextConfirmHandler.mScriptTableName = tableName;
    mTextConfirmHandler.mScriptFuncName = funcName;
}


TextField* TextField::GetSelectedTextField()
{
    return sSelectedTextField;
}

void TextField::SetSelectedTextField(TextField * newField)
{
    if (sSelectedTextField != nullptr)
    {
        sSelectedTextField->SetState(ButtonState::Normal);
        const ScriptableFP<TextFieldHandlerFP>& handler = sSelectedTextField->mTextConfirmHandler;
        if (handler.mFuncPointer != nullptr)
        {
            handler.mFuncPointer(sSelectedTextField);
        }
        if (handler.mScriptTableName != "")
        {
            ScriptEvent::WidgetState(
                handler.mScriptTableName,
                handler.mScriptFuncName,
                sSelectedTextField);
        }
    }

    sSelectedTextField = newField;
    sCursorBlinkTime = 0.0f;

    if (sSelectedTextField != nullptr)
    {
        sSelectedTextField->SetState(ButtonState::Pressed);
    }
}

uint8_t TextField::ConvertKeyCodeToChar(uint8_t keyCode, bool shiftDown)
{
    uint8_t retChar = 0;
    retChar = INP_ConvertKeyCodeToChar(keyCode);

    if (retChar >= 'A' &&
        retChar <= 'Z' &&
        !shiftDown)
    {
        // If not shifted, make the character lower-case.
        // TODO: handle caps lock state.
        retChar += 32;
    }
    else if (shiftDown)
    {
        switch (retChar)
        {
        case '`': retChar = '~'; break;
        case '1': retChar = '!'; break;
        case '2': retChar = '@'; break;
        case '3': retChar = '#'; break;
        case '4': retChar = '$'; break;
        case '5': retChar = '%'; break;
        case '6': retChar = '^'; break;
        case '7': retChar = '&'; break;
        case '8': retChar = '*'; break;
        case '9': retChar = '('; break;
        case '0': retChar = ')'; break;
        case '-': retChar = '_'; break;
        case '=': retChar = '+'; break;
        case '[': retChar = '{'; break;
        case ']': retChar = '}'; break;
        case '\\': retChar = '|'; break;
        case ';': retChar = ':'; break;
        case '\'': retChar = '\"'; break;
        case ',': retChar = '<'; break;
        case '.': retChar = '>'; break;
        case '/': retChar = '?'; break;
        }
    }

    return retChar;
}
