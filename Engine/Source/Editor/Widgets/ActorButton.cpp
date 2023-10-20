#if EDITOR

#include "Nodes/Widgets/ActorButton.h"
#include "Nodes/Node.h"
#include "Nodes/Widgets/Panel.h"
#include "Nodes/Widgets/Quad.h"
#include "EditorState.h"

float ActorButton::sActorButtonHeight = 22;

ActorButton::ActorButton() :
    mActor(nullptr),
    mPrevSelectedActor(false)
{
    SetDimensions(Panel::sDefaultWidth, sActorButtonHeight);
}

ActorButton::~ActorButton()
{

}

void ActorButton::Tick(float deltaTime)
{
    Actor* selectedActor = GetSelectedActor();
    if (mActor == selectedActor)
    {
        if (!mPrevSelectedActor)
        {
            GetQuad()->SetColor({ 0.8f, 0.2f, 0.9f, 1.0f },
                { 0.8f, 0.2f, 0.9f, 1.0f },
                { 1.0f, 0.2f, 0.9f, 1.0f },
                { 1.0f, 0.2f, 0.9f, 1.0f });
        }
        mPrevSelectedActor = true;
    }
    else
    {
        if (mPrevSelectedActor)
        {
            GetQuad()->SetColor({ 0.2f, 0.2f, 0.8f, 1.0f },
                { 0.2f, 0.2f, 0.8f, 1.0f },
                { 0.5f, 0.2f, 0.8f, 0.5f },
                { 0.5f, 0.2f, 0.8f, 0.5f });
        }
        mPrevSelectedActor = false;
    }

    Button::Tick(deltaTime);
}

void ActorButton::SetActor(Actor* actor)
{
    mActor = actor;
    SetVisible(mActor != nullptr);

    if (mActor)
    {
        SetTextString(mActor->GetName());
    }
}

Actor* ActorButton::GetActor()
{
    return mActor;
}

#endif
