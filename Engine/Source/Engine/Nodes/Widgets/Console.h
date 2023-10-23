#pragma once

#include "Nodes/Widgets/Canvas.h"
#include "Nodes/Widgets/Text.h"

class ConsoleOutputText : public Text
{
public:
    float mLifetime = 0.0f;
};

class Console : public Canvas
{
public:

    DECLARE_NODE(Console, Canvas);

    Console();

    void WriteOutput(const char* output, glm::vec4 color = { 1, 1, 1, 1 });

    virtual void Tick(float deltaTime) override;

    void ProcessInput(const char* input);
    void SetNumOutputLines(uint32_t numLines);
    void SetTextSize(float size);
    void SetFont(Font* font);
    void SetTextLifetime(float lifetime);
    void SetTextFadeTime(float fadeTime);

protected:

    uint32_t mNumOutputLines = 0;

    FontRef mFont;
    float mTextSize = 14.0f;
    float mTextLifetime = 5.0f;
    float mTextFadeTime = 0.0f;

    Canvas* mOutputCanvas = nullptr;
    Canvas* mInputCanvas = nullptr;
    Text* mInputLine = nullptr;
    std::vector<Text*> mOutputLines;
};