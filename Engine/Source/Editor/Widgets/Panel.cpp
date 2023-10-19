#if EDITOR

#include "Nodes/Widgets/Panel.h"
#include "Nodes/Widgets/Canvas.h"
#include "Nodes/Widgets/Text.h"
#include "Nodes/Widgets/Quad.h"
#include "Nodes/Widgets/TextField.h"

#include "Renderer.h"
#include "InputDevices.h"

const float Panel::sDefaultWidth = 180.0f;
const float Panel::sDefaultHeight = 360.0f;
const float Panel::sDefaultHeaderHeight = 26.0f;

const glm::vec4 Panel::sHeaderColorLeft = glm::vec4(0.8f, 0.169f, 0.369f, 1.0f);
const glm::vec4 Panel::sHeaderColorRight = glm::vec4(0.459f, 0.227f, 0.533f, 1.0f);
const glm::vec4 Panel::sBodyColorTop = sHeaderColorLeft * 0.5f;
const glm::vec4 Panel::sBodyColorBot = sHeaderColorRight * 0.5f;

Panel::Panel() :
    mHeaderCanvas(nullptr),
    mBodyCanvas(nullptr),
    mHeaderText(nullptr),
    mHeaderQuad(nullptr),
    mBodyQuad(nullptr),
    mScroll(0),
    mMinScroll(0),
    mMaxScroll(10),
    mScrollMultiplier(1)
{
    mHeaderCanvas = new Canvas();
    mBodyCanvas = new Canvas();
    mHeaderText = new Text();
    mHeaderQuad = new Quad();
    mBodyQuad = new Quad();

    AddChild(mHeaderCanvas);
    AddChild(mBodyCanvas);
    mBodyCanvas->SetName("Body");
    mBodyCanvas->SetAnchorMode(AnchorMode::FullStretch);
    mBodyCanvas->SetMargins(0.0f, sDefaultHeaderHeight, 0.0f, 0.0f);

    mHeaderCanvas->SetName("Header");
    mHeaderCanvas->AddChild(mHeaderQuad);
    mHeaderCanvas->AddChild(mHeaderText);
    mBodyCanvas->AddChild(mBodyQuad);

    mHeaderQuad->SetColor(sHeaderColorLeft, sHeaderColorRight, sHeaderColorLeft, sHeaderColorRight);
    mBodyQuad->SetAnchorMode(AnchorMode::FullStretch);
    mBodyQuad->SetColor(sBodyColorTop, sBodyColorTop, sBodyColorBot, sBodyColorBot);
    mBodyQuad->SetMargins(0.0f, 0.0f, 0.0f, 0.0f);

    mHeaderText->SetTextSize(18.0f);
    mHeaderText->SetPosition(4.0f, 0.0f);

    SetupDimensions(sDefaultWidth, sDefaultHeight);
}

Panel::~Panel()
{
    // Children automatically deleted by ~Widget().
}

void Panel::SetTitle(const std::string& title)
{
    SetName(title);
    mHeaderText->SetText(title);
}

const std::string& Panel::GetName() const
{
    return mName;
}

bool Panel::ShouldHandleInput()
{
    // Don't handle input while a textfield is selected.
    return Canvas::ShouldHandleInput() && (TextField::GetSelectedTextField() == nullptr);
}

void Panel::SetupDimensions(float width, float height)
{
    SetDimensions(width, height);

    const float headerHeight = sDefaultHeaderHeight;

    mHeaderCanvas->SetDimensions(glm::vec2(width, headerHeight));
    mHeaderQuad->SetDimensions(glm::vec2(width, headerHeight));
    mHeaderText->SetDimensions(glm::vec2(width, headerHeight));
}

void Panel::HandleInput()
{
    bool markDirty = false;

    if (ShouldHandleInput() &&
        IsMouseInsidePanel())
    {
        int32_t deltaScroll = 0;
        int32_t wheelDelta = GetScrollWheelDelta();

        if (wheelDelta != 0)
        {
            deltaScroll = -wheelDelta;
        }
        else if (IsKeyJustDownRepeat(KEY_PAGE_UP))
        {
            deltaScroll = -1;
        }
        if (IsKeyJustDownRepeat(KEY_PAGE_DOWN))
        {
            deltaScroll = 1;
        }

        deltaScroll *= mScrollMultiplier;

        if (deltaScroll != 0)
        {
            mScroll += deltaScroll;
            mScroll = glm::clamp(mScroll, mMinScroll, mMaxScroll);
            markDirty = true;
        }
    }

    if (markDirty)
    {
        MarkDirty();
    }
}

const bool Panel::IsMouseInsidePanel() const
{
    return ContainsMouse();
}

int32_t Panel::GetScroll() const
{
    return mScroll;
}

void Panel::SetScroll(int32_t scroll)
{
    mScroll = glm::clamp(scroll, mMinScroll, mMaxScroll);
}

void Panel::SetMinScroll(int32_t minScroll)
{
    mMinScroll = minScroll;
    mScroll = glm::clamp(mScroll, mMinScroll, mMaxScroll);
}

void Panel::SetMaxScroll(int32_t maxScroll)
{
    mMaxScroll = maxScroll;
    mScroll = glm::clamp(mScroll, mMinScroll, mMaxScroll);
}

#endif
