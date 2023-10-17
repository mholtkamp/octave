#include "Widgets/Console.h"
#include "Assets/Font.h"
#include "Renderer.h"
#include "Engine.h"

#include "AssetManager.h"

FORCE_LINK_DEF(Console);
DEFINE_WIDGET(Console, Canvas);

Console::Console()
{
    glm::vec2 res = Renderer::Get()->GetScreenResolution(0);

    mFont = LoadAsset<Font>("F_RobotoMono16");

    mOutputCanvas = new Canvas();
    AddChild(mOutputCanvas);

    const uint32_t defaultNumLines = 5;
    SetNumOutputLines(defaultNumLines);

    SetRect(0.0f, 0.0f, res.x, res.y);
    mOutputCanvas->SetRect(0.0f, 0.0f, res.x, res.y);

}

void Console::WriteOutput(const char* output, glm::vec4 color)
{
    // TODO: Do we need a separate mutex for Console vs the Log mutex?
    // Or can these functions also lock the log mutex?
    // Either way I think we need to be locking some mutex here. For instance if the main
    // thread called SetNumOutputLines() but the async load thread is calling WriteOutput() via LogX()
    if (mNumOutputLines > 0)
    {
        uint32_t numChildren = mOutputCanvas->GetNumChildren();
        OCT_ASSERT(numChildren > 0);

        uint32_t lastIndex = numChildren - 1;
        ConsoleOutputText* text = (ConsoleOutputText*)mOutputCanvas->RemoveChild(lastIndex);
        text->SetText(output);
        text->SetColor(color);
        text->SetVisible(true);
        text->mLifetime = mTextLifetime + mTextFadeTime;
        mOutputCanvas->AddChild(text, 0);
        MarkDirty();
    }
}

void Console::Update()
{
    if (IsDirty())
    {
        // Update the output Text lines
        const float xPos = 0.0f;
        float yPos = 0.0f;
        for (uint32_t i = 0; i < mOutputCanvas->GetNumChildren(); ++i)
        {
            ConsoleOutputText* text = (ConsoleOutputText*)mOutputCanvas->GetChild(i);
            text->SetPosition(xPos, yPos);
            text->SetTextSize(mTextSize);
            text->SetFont(mFont.Get<Font>());

            yPos += mTextSize;
        }
    }

    // Update fade out
    for (uint32_t i = 0; i < mOutputCanvas->GetNumChildren(); ++i)
    {
        ConsoleOutputText* text = (ConsoleOutputText*)mOutputCanvas->GetChild(i);

        if (text->mLifetime > 0.0f)
        {
            text->mLifetime -= GetAppClock()->DeltaTime();

            if (text->mLifetime <= 0.0f)
            {
                text->mLifetime = 0.0f;
                text->SetVisible(false);
            }
            else if (text->mLifetime < mTextFadeTime)
            {
                // TODO: Fade out smoothly. Might want to add a Tint value to Text so we don't
                // have to update the entire text vertex buffer just to fade out...
            }
        }
    }


    Canvas::Update();
}

void Console::ProcessInput(const char* input)
{
    // TODO
}

void Console::SetNumOutputLines(uint32_t numLines)
{
    if (mNumOutputLines != numLines)
    {
        // Destroy old widgets
        for (int32_t i = (int32_t)mOutputCanvas->GetNumChildren() - 1; i >= 0 ; --i)
        {
            Widget* oldText = mOutputCanvas->RemoveChild(i);
            delete oldText;
        }

        mOutputLines.clear();

        for (uint32_t i = 0; i < numLines; ++i)
        {
            // Allocate Text widgets and parent them to this widget.
            ConsoleOutputText* newText = new ConsoleOutputText();
            newText->SetText("");
            mOutputCanvas->AddChild(newText);
            mOutputLines.push_back(newText);
        }

        mNumOutputLines = numLines;
        MarkDirty();
    }
}

void Console::SetTextSize(float size)
{
    mTextSize = size;
    MarkDirty();
}

void Console::SetFont(Font* font)
{
    mFont = font;
    MarkDirty();
}

void Console::SetTextLifetime(float lifetime)
{
    mTextLifetime = lifetime;
}

void Console::SetTextFadeTime(float fadeTime)
{
    mTextFadeTime = fadeTime;
}
