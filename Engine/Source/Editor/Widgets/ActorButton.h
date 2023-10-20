#pragma once

#include "Nodes/Widgets/Button.h"

class ActorButton : public Button
{
public:

    ActorButton();
    ~ActorButton();

    virtual void Tick(float deltaTime) override;

    void SetActor(Actor* actor);
    Actor* GetActor();

    static float sActorButtonHeight;

protected:

    Actor* mActor;
    bool mPrevSelectedActor;
};