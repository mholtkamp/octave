#if EDITOR

#include "Widgets/OutlinerPanel.h"
#include "Widgets/ActorButton.h"
#include "Widgets/PropertiesPanel.h"
#include "Engine.h"
#include "World.h"
#include "Widgets/Quad.h"
#include "EditorState.h"
#include "ActionManager.h"
#include "PanelManager.h"
#include "InputDevices.h"
#include "Log.h"

#include <glm/glm.hpp>

void OnActorButtonPressed(Button* button)
{
    if (button != nullptr)
    {
        Actor* selectedActor = GetSelectedActor();

        ActorButton* actorButton = static_cast<ActorButton*>(button);
        Actor* actor = actorButton->GetActor();
        Component* component = actor ? actor->GetRootComponent() : nullptr;

        if (selectedActor != actor)
        {
            SetSelectedComponent(component);
        }
        else
        {
            SetSelectedComponent(nullptr);
        }
    }
}

OutlinerPanel::OutlinerPanel()
{
    SetName("Outliner");
    SetAnchorMode(AnchorMode::LeftStretch);
    SetYRatio(0.0f);
    SetHeightRatio(0.5f);
}

void OutlinerPanel::Update()
{
    World* world = GetWorld();

    float canvasHeight = mBodyCanvas->GetHeight();
    float buttonHeight = ActorButton::sActorButtonHeight;
    int32_t numAvailableButtons = int32_t(canvasHeight / buttonHeight);
    assert(numAvailableButtons < 100); // Something might be wrong?

    while (mActorButtons.size() < numAvailableButtons)
    {
        int32_t i = (int32_t)mActorButtons.size();

        ActorButton* button = new ActorButton();
        mActorButtons.push_back(button);
        mBodyCanvas->AddChild(button);
        button->SetActor(nullptr);
        button->SetPosition(0, i * mActorButtons[i]->GetHeight());
        button->SetPressedHandler(OnActorButtonPressed);
    }

    if (world)
    {
        const std::vector<Actor*>& actors = world->GetActors();

        SetMaxScroll(int32_t(actors.size()) - 1);
        int32_t listOffset = mScroll;

        for (int32_t i = 0; i < int32_t(mActorButtons.size()); ++i)
        {
            int32_t actorIndex = listOffset + i;

            if (actorIndex < int32_t(actors.size()) &&
                i < numAvailableButtons)
            {
                mActorButtons[i]->SetActor(actors[actorIndex]);
            }
            else
            {
                mActorButtons[i]->SetActor(nullptr);
            }
        }
    }
    else
    {
        for (int32_t i = 0; i < int32_t(mActorButtons.size()); ++i)
        {
            mActorButtons[i]->SetActor(nullptr);
        }
    }

    Panel::Update();
}

void OutlinerPanel::HandleInput()
{
    Panel::HandleInput();

    if (IsMouseInsidePanel())
    {
        if (IsKeyJustDown(KEY_DELETE))
        {
            ActionManager::Get()->DeleteSelectedActors();
        }
    }
}

#endif
