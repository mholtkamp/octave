#if EDITOR

#include "Widgets/HierarchyPanel.h"
#include "Widgets/HierarchyButton.h"
#include "ActionManager.h"
#include "PanelManager.h"
#include "EditorState.h"
#include "Actor.h"
#include "InputDevices.h"
#include "Widgets/ActionList.h"
#include "Renderer.h"
#include "Components/Component.h"
#include "Components/TransformComponent.h"
#include "ObjectRef.h"

#include <functional>

const float HierarchyPanel::sHierarchyPanelHeight = 240;

static ComponentRef sActionComponent;

void OnCreateCompButtonPressed(Button* button)
{
    const std::string& className = button->GetTextString();

    Actor* actor = GetSelectedActor();
    Component* selectedComp = GetSelectedComponent();

    if (actor != nullptr)
    {
        Component* newComp = actor->CreateComponent(className.c_str());

        if (newComp->IsTransformComponent())
        {
            TransformComponent* newTrans = (TransformComponent*)newComp;
            TransformComponent* parentTrans = selectedComp->IsTransformComponent() ? (TransformComponent*)selectedComp : actor->GetRootComponent();
            assert(parentTrans != nullptr);
            newTrans->Attach(parentTrans);
        }

        ActionManager::Get()->EXE_AddComponent(newComp);

        // Adding a new component should break the blueprint link.
        // For now, you cannot override blueprint instance components
        actor->SetBlueprintSource(nullptr);

        PanelManager::Get()->GetHierarchyPanel()->RefreshCompButtons();
    }

    Renderer::Get()->SetModalWidget(nullptr);
}

void OnHierarchyButtonPressed(Button* button)
{
    if (button != nullptr)
    {
        Component* selectedComponent = GetSelectedComponent();

        HierarchyButton* hierButton = static_cast<HierarchyButton*>(button);
        Component* comp = hierButton->GetComponent();

        if (selectedComponent != comp)
        {
            SetSelectedComponent(comp);
        }
    }
}

void HierarchyPanel::ActionHandler(Button* button)
{
    std::string buttonText = button->GetTextString();
    bool hideList = true;

    if (buttonText == "Add Component")
    {
        ShowAddComponentPrompt();
        hideList = false;
    }
    else if (buttonText == "Delete Component")
    {
        DeleteComponent(sActionComponent.Get());
    }
    else if (buttonText == "Attach Selected")
    {
        if (sActionComponent != nullptr)
        {
            AttachSelectedComponent(sActionComponent.Get());
        }
    }

    if (hideList)
    {
        GetActionList()->Hide();
    }

    sActionComponent = nullptr;
}

void HierarchyPanel::ShowAddComponentPrompt()
{
    std::vector<std::string> actions;
    const std::vector<Factory*>& compFactories = Component::GetFactoryList();

    for (uint32_t i = 0; i < compFactories.size(); ++i)
    {
        actions.push_back(compFactories[i]->GetClassName());
    }

    ActionList* actionList = GetActionList();
    actionList->SetActions(actions, OnCreateCompButtonPressed);
}

void HierarchyPanel::DeleteComponent(Component* comp)
{
    if (comp != nullptr)
    {
        Actor* actor = comp->GetOwner();
        bool isRoot = false;
        bool hasChildren = false;

        isRoot = (comp == actor->GetRootComponent());

        if (comp->IsTransformComponent())
        {
            TransformComponent* transComp = (TransformComponent*)comp;
            hasChildren = transComp->GetChildren().size() > 0;
        }

        if (isRoot)
        {
            LogError("Cannot delete root component");
        }
        else if (hasChildren)
        {
            LogError("Cannot delete component with children");
        }
        else
        {
            ActionManager::Get()->EXE_RemoveComponent(comp);
            SetSelectedComponent(actor->GetRootComponent());

            // Deleting component should break the blueprint link.
            // For now, you cannot override blueprint instance components
            actor->SetBlueprintSource(nullptr);

            PanelManager::Get()->GetHierarchyPanel()->RefreshCompButtons();
        }
    }
}

void HierarchyPanel::AttachSelectedComponent(Component* newParent)
{
    if (newParent != nullptr &&
        newParent->IsTransformComponent() &&
        GetSelectedComponent() != nullptr &&
        GetSelectedComponent()->IsTransformComponent())
    {
        TransformComponent* child = (TransformComponent*)GetSelectedComponent();
        TransformComponent* parent = (TransformComponent*)newParent;

        if (child->GetParent() != parent)
        {
            ActionManager::Get()->EXE_AttachComponent(child, parent);

            // Reparenting components should break the blueprint link.
            // For now, you cannot override blueprint instance components
            newParent->GetOwner()->SetBlueprintSource(nullptr);
        }

        PanelManager::Get()->GetHierarchyPanel()->RefreshCompButtons();
    }
}

HierarchyPanel::HierarchyPanel() :
    mListOffset(0)
{
    SetName("Hierarchy");
    SetAnchorMode(AnchorMode::TopRight);
    SetPosition(-sDefaultWidth, 0.0f);
    SetDimensions(Panel::sDefaultWidth, sHierarchyPanelHeight);

    for (int32_t i = 0; i < sNumButtons; ++i)
    {
        mButtons[i] = new HierarchyButton();
        mButtons[i]->SetComponent(nullptr);
        mButtons[i]->SetPosition(0, i * mButtons[i]->GetHeight());
        mButtons[i]->SetPressedHandler(OnHierarchyButtonPressed);
        mBodyCanvas->AddChild(mButtons[i]);
    }

    mBlueprintText = new Text();
    mBlueprintText->SetText("BP");
    mBlueprintText->SetColor(glm::vec4(0.1f, 0.0f, 1.0f, 1.0f));
    mBlueprintText->SetAnchorMode(AnchorMode::TopRight);
    mBlueprintText->SetSize(24.0f);
    mBlueprintText->SetPosition(-35.0f, -2.0f);
    mBlueprintText->SetVisible(false);
    mHeaderCanvas->AddChild(mBlueprintText);
}

void HierarchyPanel::OnSelectedComponentChanged()
{
    RefreshCompButtons();
}

void HierarchyPanel::RefreshCompButtons()
{
    Actor* selectedActor = GetSelectedActor();

    if (selectedActor != nullptr)
    {
        bool bp = (selectedActor->GetBlueprintSource() != nullptr);
        mBlueprintText->SetVisible(bp);

        const std::vector<Component*>& components = selectedActor->GetComponents();

        // Rearrange by hierarchy
        std::vector<Component*> compList;
        std::vector<uint32_t> depthList;

        std::function<void(TransformComponent*, uint32_t)> AddTransComp = [&](TransformComponent* comp, uint32_t depth)
        {
            if (comp != nullptr)
            {
                compList.push_back(comp);
                depthList.push_back(depth);

                for (uint32_t i = 0; i < comp->GetNumChildren(); ++i)
                {
                    AddTransComp(comp->GetChild(i), depth + 1);
                }
            }
        };

        AddTransComp(selectedActor->GetRootComponent(), 0);

        // Add rest of the items.
        for (uint32_t i = 0; i < components.size(); ++i)
        {
            if (std::find(compList.begin(), compList.end(), components[i]) == compList.end())
            {
                compList.push_back(components[i]);
                depthList.push_back(0);
                assert(!components[i]->IsTransformComponent());
            }
        }

        assert(compList.size() == components.size());

        mListOffset = glm::min(mListOffset, int32_t(compList.size()) - 1);
        mListOffset = glm::max(mListOffset, 0);

        for (int32_t i = 0; i < sNumButtons; ++i)
        {
            int32_t index = mListOffset + i;

            if (index < int32_t(compList.size()))
            {
                mButtons[i]->SetComponent(compList[index]);
                mButtons[i]->SetX(depthList[index] * 10.0f);
            }
            else
            {
                mButtons[i]->SetComponent(nullptr);
                mButtons[i]->SetX(0.0f);
            }
        }
    }
    else
    {
        mBlueprintText->SetVisible(false);

        for (int32_t i = 0; i < sNumButtons; ++i)
        {
            mButtons[i]->SetComponent(nullptr);
        }
    }
}

void HierarchyPanel::HandleInput()
{
    Panel::HandleInput();

    if (ShouldHandleInput() &&
        IsMouseInsidePanel())
    {
        bool shiftDown = IsShiftDown();

        if (IsMouseButtonJustUp(MOUSE_RIGHT))
        {
            Component* comp = nullptr;

            // Check if asset button is selected
            for (uint32_t i = 0; i < sNumButtons; ++i)
            {
                if (mButtons[i]->IsVisible() &&
                    mButtons[i]->ContainsMouse() &&
                    mButtons[i]->GetComponent() != nullptr)
                {
                    comp = mButtons[i]->GetComponent();
                    break;
                }
            }

            std::vector<std::string> actions;
            actions.push_back("Add Component");

            if (comp != nullptr)
            {
                actions.push_back("Delete Component");
                actions.push_back("Attach Selected");
            }
            GetActionList()->SetActions(actions, ActionHandler);
            sActionComponent = comp;
        }

        if (shiftDown && IsKeyJustDown(KEY_A))
        {
            ShowAddComponentPrompt();
        }

        if (IsKeyJustDown(KEY_DELETE) || IsKeyJustDown(KEY_X))
        {
            DeleteComponent(GetSelectedComponent());
        }
    }
}

void HierarchyPanel::Update()
{
    Panel::Update();

    Actor* actor = GetSelectedActor();

    if (actor != nullptr)
    {
        const std::vector<Component*>& components = actor->GetComponents();
        SetMaxScroll(glm::max(0, int32_t(components.size()) - 1));
    }
    else
    {
        SetMaxScroll(0);
    }

    int32_t listOffset = mScroll;

    if (listOffset != mListOffset)
    {
        mListOffset = listOffset;
        RefreshCompButtons();
    }
}

#endif
