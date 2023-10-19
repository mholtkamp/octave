#pragma once

#include "Nodes/Widgets/Widget.h"
#include "Nodes/Widgets/TextField.h"
#include "Nodes/Widgets/Text.h"
#include "Nodes/Widgets/Quad.h"

#include "Renderer.h"

class TextEntry : public Widget
{
public:
    TextEntry()
    {
        SetDimensions(200.0f, 60.0f);

        mBg = new Quad();
        mBg->SetColor(glm::vec4(0.0f, 0.1f, 1.0f, 0.7f));
        mBg->SetAnchorMode(AnchorMode::FullStretch);
        mBg->SetMargins(0.0f, 0.0f, 0.0f, 0.0f);
        AddChild(mBg);

        mTitle = new Text();
        mTitle->SetTextSize(20.0f);
        AddChild(mTitle);

        mTextField = new TextField();
        mTextField->SetTextString("");
        mTextField->SetAnchorMode(AnchorMode::BottomStretch);
        mTextField->SetPosition(0.0f, -30.0f);
        mTextField->SetLeftMargin(0.0f);
        mTextField->SetRightMargin(0.0f);
        AddChild(mTextField);
    }

    void Prompt(const char* title, TextFieldHandlerFP confirmHandler, const char* defaultText = nullptr)
    {
        mTitle->SetText(title);
        mTextField->SetTextConfirmHandler(confirmHandler);

        MoveToMousePosition();
        FitInsideParent();
        mTextField->SetTextString(defaultText ? defaultText : "");
        TextField::SetSelectedTextField(mTextField);
        Renderer::Get()->SetModalWidget(this);
    }

    TextField* mTextField = nullptr;
    Text* mTitle = nullptr;
    Quad* mBg = nullptr;

};