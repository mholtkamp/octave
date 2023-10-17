#pragma once

#include "Panel.h"
#include <vector>

class ActorButton;
class Button;
class TextField;

class OutlinerPanel : public Panel
{
public:

    OutlinerPanel();
    virtual void Update() override;
    virtual void HandleInput() override;

protected:

    static void ActionListHandler(Button* button);
    static void HandleRenameActor(TextField* tf);

    std::vector<ActorButton*> mActorButtons;
};