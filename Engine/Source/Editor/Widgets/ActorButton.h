#pragma once

#include "Widgets/Button.h"

class ActorButton : public Button
{
public:

    ActorButton();
    ~ActorButton();

    virtual void Update() override;

    void SetActor(Actor* actor);
    Actor* GetActor();

    static float sActorButtonHeight;

protected:

    Actor* mActor;
    bool mPrevSelectedActor;
};