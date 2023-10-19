#pragma once

#include "Nodes/Widgets/Widget.h"
#include "Nodes/Widgets/VerticalList.h"
#include "Nodes/Widgets/Text.h"
#include "Nodes/Widgets/Quad.h"
#include "Nodes/Widgets/Button.h"

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

