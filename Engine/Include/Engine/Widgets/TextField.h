#pragma once

#include "Widgets/Button.h"
#include "Widgets/Quad.h"

#include "ScriptableFuncPointer.h"

typedef void(*TextFieldHandlerFP)(class TextField* textField);

class TextField : public Button
{
public:

    DECLARE_WIDGET(TextField, Button);

    TextField();
    ~TextField();

    virtual void Update() override;
    virtual void SetState(ButtonState newState) override;
    virtual void SetTextString(const std::string& newTextString) override;

    void SetTextEditHandler(TextFieldHandlerFP handler);
    void SetTextConfirmHandler(TextFieldHandlerFP handler);
    void SetScriptTextEditHandler(const char* tableName, const char* funcName);
    void SetScriptTextConfirmHandler(const char* tableName, const char* funcName);

    static TextField* GetSelectedTextField();
    static void SetSelectedTextField(TextField* newField);

protected:

    static TextField* sSelectedTextField;
    static float sCursorBlinkTime;
    static const float sCursorBlinkPeriod;

    uint8_t ConvertKeyCodeToChar(uint8_t keyCode, bool shiftDown);

    ScriptableFP<TextFieldHandlerFP> mTextEditHandler;
    ScriptableFP<TextFieldHandlerFP> mTextConfirmHandler;

    Quad* mCursorQuad;
    uint32_t mMaxCharacters;
};