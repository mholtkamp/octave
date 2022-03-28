#if EDITOR

#include "Widgets/ActorButton.h"
#include "Actor.h"
#include "Widgets/Panel.h"
#include "Widgets/Quad.h"
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

void ActorButton::Update()
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

    Button::Update();
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
