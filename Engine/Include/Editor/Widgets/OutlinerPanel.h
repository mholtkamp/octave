#pragma once

#include "Panel.h"
#include <vector>

class ActorButton;

class OutlinerPanel : public Panel
{
public:

    OutlinerPanel();
    virtual void Update() override;
    virtual void HandleInput() override;

protected:

    std::vector<ActorButton*> mActorButtons;
};