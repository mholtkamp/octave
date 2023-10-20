#if EDITOR

#include "Nodes/Widgets/ActionList.h"

#include "Renderer.h"
#include "Engine.h"
#include "InputDevices.h"
#include "AssetManager.h"

static const float kListWidth = 200.0f;

ActionList::ActionList()
{
    SetDimensions(kListWidth, 500.0f);

    mBg = new Quad();
    mBg->SetColor({ 0.4f, 0.2f, 0.2f, 0.5f });
    mBg->SetAnchorMode(AnchorMode::FullStretch);
    mBg->SetMargins(0, 0, 0, 0);
    mBg->SetVisible(false);
    AddChild(mBg);

    mTitle = new Text();
    mTitle->SetTextSize(20.0f);
    mTitle->SetText("");
    mTitle->SetVisible(false);
    AddChild(mTitle);

    mArrowTop = new Quad();
    mArrowTop->SetAnchorMode(AnchorMode::TopLeft);
    mArrowTop->SetPosition(30.0f, -20.0f);
    mArrowTop->SetDimensions(28.0f, 18.0f);
    mArrowTop->SetTexture(LoadAsset<Texture>("T_MiniArrow"));
    mArrowTop->SetVisible(false);
    AddChild(mArrowTop);

    mArrowBot = new Quad();
    mArrowBot->SetAnchorMode(AnchorMode::BottomLeft);
    mArrowBot->SetPosition(30.0f, 2.0f);
    mArrowBot->SetDimensions(28.0f, 18.0f);
    mArrowBot->SetTexture(LoadAsset<Texture>("T_MiniArrow"));
    mArrowBot->SetVisible(false);
    mArrowBot->SetUvScale({ 1.0f, -1.0f }); // Flip it to point down
    AddChild(mArrowBot);

    mList = new VerticalList();
    mList->SetDisplayCount(10);
    AddChild(mList);
}

ActionList::~ActionList()
{
    mList->RemoveAllListItems();
    for (uint32_t i = 0; i < mButtons.size(); ++i)
    {
        delete mButtons[i];
        mButtons[i] = nullptr;
    }

    mButtons.clear();
}

void ActionList::Tick(float deltaTime)
{
    if (mVisibleDelay > 0)
    {
        MarkDirty();
    }

    Widget::Tick(deltaTime);

    if (Renderer::Get()->GetModalWidget() == this &&
        !ContainsMouse() &&
        (IsMouseButtonJustUp(MOUSE_LEFT) || IsMouseButtonJustUp(MOUSE_RIGHT)))
    {
        Renderer::Get()->SetModalWidget(nullptr);
    }

    bool showBotArrow = mList->GetDisplayOffset() + mList->GetDisplayCount() < mList->GetNumListItems();
    mArrowBot->SetVisible(showBotArrow);

    bool showTopArrow = mList->GetDisplayOffset() > 0;
    mArrowTop->SetVisible(showTopArrow);

    if (mVisibleDelay > 0)
    {
        mVisibleDelay--;
        if (mVisibleDelay == 0)
        {
            mList->SetVisible(true);
        }
    }
}

void ActionList::SetActions(const std::vector<std::string>& options, ButtonHandlerFP handler)
{
    uint32_t numOptions = (uint32_t)options.size();

    mList->RemoveAllListItems();
    mList->SetDisplayOffset(0);
    MoveToMousePosition();

    if (numOptions > mButtons.size())
    {
        while (mButtons.size() != numOptions)
        {
            mButtons.push_back(new Button());
            Button* button = mButtons.back();
            button->SetHeight(22.0f);
            button->SetWidth(kListWidth);
            button->GetText()->SetTextSize(16.0f);
        }
    }

    for (uint32_t i = 0; i < numOptions; ++i)
    {
        mButtons[i]->SetTextString(options[i]);
        mButtons[i]->SetPressedHandler(handler);
        mList->AddListItem(mButtons[i]);
    }

    mList->Update();
    SetDimensions(mList->GetDimensions());
    UpdateRect();

    Renderer::Get()->SetModalWidget(this);

    FitInsideParent();

    mList->SetVisible(false);
    mVisibleDelay = 2;
}

void ActionList::Hide()
{
    if (Renderer::Get()->GetModalWidget() == this)
    {
        Renderer::Get()->SetModalWidget(nullptr);
    }
}

#endif
