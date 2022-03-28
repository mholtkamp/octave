#pragma once

#include "Widgets/Canvas.h"
#include <glm/glm.hpp>

class Text;
class Quad;

class Panel : public Canvas
{
public:

    Panel();
    ~Panel();

    void SetName(const std::string& name);
    const std::string& GetName() const;

    virtual bool ShouldHandleInput() override;

    virtual void SetupDimensions(float width, float height);

    virtual void HandleInput();
    const bool IsMouseInsidePanel() const;

    int32_t GetScroll() const;
    void SetMinScroll(int32_t minScroll);
    void SetMaxScroll(int32_t maxScroll);

public:

    static const float sDefaultWidth;
    static const float sDefaultHeight;
    static const float sDefaultHeaderHeight;
    static const glm::vec4 sHeaderColorLeft;
    static const glm::vec4 sHeaderColorRight;
    static const glm::vec4 sBodyColorTop;
    static const glm::vec4 sBodyColorBot;

protected:

    std::string mName;

    Canvas* mHeaderCanvas;
    Canvas* mBodyCanvas;
    Text* mHeaderText;
    Quad* mHeaderQuad;
    Quad* mBodyQuad;

    int32_t mScroll;
    int32_t mMinScroll;
    int32_t mMaxScroll;
};