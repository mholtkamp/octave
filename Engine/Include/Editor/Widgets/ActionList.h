#pragma once

#include "Widgets/Widget.h"
#include "Widgets/VerticalList.h"
#include "Widgets/Text.h"
#include "Widgets/Quad.h"
#include "Widgets/Button.h"

class ActionList : public Widget
{
public:

    ActionList();
    virtual ~ActionList();

    virtual void Update() override;

    void SetActions(const std::vector<std::string>& options, ButtonHandlerFP handler);

    void Hide();

    Quad* mBg = nullptr;
    Text* mTitle = nullptr;
    Quad* mArrowTop = nullptr;
    Quad* mArrowBot = nullptr;
    VerticalList* mList = nullptr;
    std::vector<Button*> mButtons;
    uint32_t mVisibleDelay = 0;
};

