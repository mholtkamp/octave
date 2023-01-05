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

static void EnsureUniqueComponentName(Component* comp)
{
    Actor* actor = comp->GetOwner();
    std::string baseName = comp->GetName();
    uint32_t number = 1;

    for (int32_t i = 0; i < (int32_t)actor->GetNumComponents(); ++i)
    {
        if (actor->GetComponent(i) != comp &&
            actor->GetComponent(i)->GetName() == comp->GetName())
        {
            number++;
            char numStr[32] = {};
            snprintf(numStr, 32, "%d", number);

            comp->SetName(baseName + " " + numStr);

            // Restart the check
            i = -1;
        }
    }
}

void OnCreateCompButtonPressed(Button* button)
{
    const std::string& className = button->GetTextString();

    Actor* actor = GetSelectedActor();
    Component* selectedComp = GetSelectedComponent();

    if (actor != nullptr)
    {
        Component* newComp = actor->CreateComponent(className.c_str());

        EnsureUniqueComponentName(newComp);

        if (newComp->IsTransformComponent())
        {
            TransformComponent* newTrans = (TransformComponent*)newComp;
            TransformComponent* parentTrans = selectedComp->IsTransformComponent() ? (TransformComponent*)selectedComp : actor->GetRootComponent();
            OCT_ASSERT(parentTrans != nullptr);
            newTrans->Attach(parentTrans);
        }

        ActionManager::Get()->EXE_AddComponent(newComp);

        SetSelectedComponent(newComp);

        // Adding a new component should break the blueprint link.
        // For now, you cannot override blueprint instance components
        actor->SetBlueprintSource(nullptr);

        PanelManager::Get()->GetHierarchyPanel()->RefreshCompButtons();
    }

    Renderer::Get()->SetModalWidget(nullptr);
}

static void OnHierarchyButtonPressed(Button* button)
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
    else if (buttonText == "Set Root Component")
    {
        if (sActionComponent != nullptr)
        {
            SetRootComponent(sActionComponent.Get());
        }
    }
    else if (buttonText == "Rename Component")
    {
        const char* defaultText = sActionComponent.Get() ? 
            sActionComponent.Get()->GetName().c_str() : 
            nullptr;

        ShowTextPrompt("Rename", HandleRenameComponent, defaultText);
        hideList = false;
    }

    if (hideList)
    {
        GetActionList()->Hide();
    }
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
        bool isDefault = false;
        bool hasChildren = false;

        isRoot = (comp == actor->GetRootComponent());
        isDefault = comp->IsDefault();

        if (comp->IsTransformComponent())
        {
            TransformComponent* transComp = (TransformComponent*)comp;
            hasChildren = transComp->GetChildren().size() > 0;
        }

        if (isRoot)
        {
            LogError("Cannot delete root component");
        }
        else if (isDefault)
        {
            LogError("Cannot delete default component (must be changed in code)");
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
    if (newParent == nullptr)
        return;

    if (!newParent->IsTransformComponent())
        return;

    Component* selComp = GetSelectedComponent();

    if (selComp == nullptr)
        return;

    if (!selComp->IsTransformComponent())
        return;

    if (selComp->IsDefault())
    {
        LogError("Cannot re-parent default components. Please change attachment in code.");
        return;
    }

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

void HierarchyPanel::SetRootComponent(Component* newRoot)
{
    if (newRoot == nullptr)
        return;

    if (!newRoot->IsTransformComponent())
        return;

    TransformComponent* newRootTrans = (TransformComponent*)newRoot;
    TransformComponent* oldRootTrans = newRoot->GetOwner()->GetRootComponent();

    if (newRoot->IsDefault() ||
        oldRootTrans->IsDefault())
    {
        LogError("Cannot adjust root for default components. Please root in code.");
        return;
    }

    if (newRootTrans != oldRootTrans)
    {
        ActionManager::Get()->EXE_SetRootComponent(newRootTrans);

        // Reparenting components should break the blueprint link.
        // For now, you cannot override blueprint instance components
        newRoot->GetOwner()->SetBlueprintSource(nullptr);
    }

    PanelManager::Get()->GetHierarchyPanel()->RefreshCompButtons();
}

void HierarchyPanel::HandleRenameComponent(TextField* tf)
{
    const std::string& newName = tf->GetTextString();

    if (newName != "" &&
        sActionComponent != nullptr)
    {
        ActionManager::Get()->EXE_EditProperty(sActionComponent.Get(), PropertyOwnerType::Component, "Name", 0, newName);
    }

    Renderer::Get()->SetModalWidget(nullptr);
    PanelManager::Get()->GetHierarchyPanel()->RefreshCompButtons();
}

HierarchyPanel::HierarchyPanel() :
    mListOffset(0)
{
    SetTitle("Hierarchy");
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
                    TransformComponent* child = comp->GetChild(i);

                    if (child->GetOwner() == selectedActor)
                    {
                        AddTransComp(child, depth + 1);
                    }
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
                OCT_ASSERT(!components[i]->IsTransformComponent() ||
                    static_cast<TransformComponent*>(components[i])->GetParent()->GetOwner() != selectedActor);
            }
        }

        // This assertion is not true if another actor is attached.
        //OCT_ASSERT(compList.size() == components.size());

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
                actions.push_back("Set Root Component");
                actions.push_back("Rename Component");
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
