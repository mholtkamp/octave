#if EDITOR

#include "Widgets/HierarchyPanel.h"
#include "Widgets/HierarchyButton.h"
#include "ActionManager.h"
#include "PanelManager.h"
#include "EditorState.h"
#include "Nodes/Node.h"
#include "InputDevices.h"
#include "Widgets/ActionList.h"
#include "Renderer.h"
#include "Nodes/Node.h"
#include "Nodes/3D/Node3d.h"
#include "Nodes/3D/SkeletalMesh3d.h"
#include "Assets/SkeletalMesh.h"
#include "ObjectRef.h"

#include <functional>

const float HierarchyPanel::sHierarchyPanelHeight = 240;

static ComponentRef sActionComponent;

static Actor* GetTargetActor()
{
    Actor* actor = nullptr;

    if (GetEditorMode() == EditorMode::Blueprint)
    {
        actor = GetEditBlueprintActor();
    }
    else
    {
        actor = GetSelectedActor();
    }

    return actor;
}

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

    Actor* actor = GetTargetActor();
    Component* selectedComp = GetSelectedComponent();

    if (selectedComp == nullptr &&
        GetEditorMode() == EditorMode::Blueprint &&
        GetEditBlueprintActor())
    {
        selectedComp = GetEditBlueprintActor()->GetRootComponent();
    }

    if (actor != nullptr &&
        selectedComp != nullptr)
    {
        Component* newComp = actor->CreateComponent(className.c_str());

        EnsureUniqueComponentName(newComp);

        if (newComp->IsNode3D())
        {
            Node3D* newTrans = (Node3D*)newComp;
            Node3D* parentTrans = selectedComp->IsNode3D() ? (Node3D*)selectedComp : actor->GetRootComponent();
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
            AttachSelectedComponent(sActionComponent.Get(), -1);
        }
    }
    else if (buttonText == "Attach To Bone")
    {
        if (sActionComponent != nullptr)
        {
            ShowTextPrompt("Bone Name", HandleBonePrompt);
            hideList = false;
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
    else if (buttonText == "Duplicate Component")
    {
        Component* actionComp = sActionComponent.Get();
        Node3D* actionTrans = actionComp->As<Node3D>();
        Actor* owner = actionComp->GetOwner();
        OCT_ASSERT(owner);

        Component* newComp = owner->CreateComponent(actionComp->GetType());
        if (newComp->IsNode3D())
        {
            OCT_ASSERT(actionTrans);
            Node3D* newTrans = newComp->As<Node3D>();
            Node3D* actionParent = actionTrans->GetParent();

            if (actionParent != nullptr)
            {
                newTrans->Attach(actionParent);
            }
            else
            {
                newTrans->Attach(actionTrans);
            }
        }

        // Copy all properties.
        newComp->Copy(actionComp);

        // Refresh buttons
        PanelManager::Get()->GetHierarchyPanel()->RefreshCompButtons();

        ActionManager::Get()->EXE_AddComponent(newComp);
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

        if (comp->IsNode3D())
        {
            Node3D* transComp = (Node3D*)comp;
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

void HierarchyPanel::AttachSelectedComponent(Component* newParent, int32_t boneIdx)
{
    if (newParent == nullptr)
        return;

    if (!newParent->IsNode3D())
        return;

    Component* selComp = GetSelectedComponent();

    if (selComp == nullptr)
        return;

    if (!selComp->IsNode3D())
        return;

    if (selComp->IsDefault())
    {
        LogError("Cannot re-parent default components. Please change attachment in code.");
        return;
    }

    Node3D* child = (Node3D*)GetSelectedComponent();
    Node3D* parent = (Node3D*)newParent;

    if (child->GetParent() != parent ||
        boneIdx != child->GetParentBoneIndex())
    {
        ActionManager::Get()->EXE_AttachComponent(child, parent, boneIdx);

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

    if (!newRoot->IsNode3D())
        return;

    Node3D* newRootTrans = (Node3D*)newRoot;
    Node3D* oldRootTrans = newRoot->GetOwner()->GetRootComponent();

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

void HierarchyPanel::HandleBonePrompt(TextField* tf)
{
    const std::string& boneName = tf->GetTextString();
    Component* selComp = GetSelectedComponent();

    Node3D* selTrans = selComp ? selComp->As<Node3D>() : nullptr;
    SkeletalMesh3D* parentSk = (sActionComponent != nullptr) ? sActionComponent.Get<SkeletalMesh3D>() : nullptr;

    if (boneName != "" &&
        parentSk != nullptr &&
        parentSk->GetSkeletalMesh() != nullptr)
    {
        int32_t boneIndex = parentSk->GetSkeletalMesh()->FindBoneIndex(boneName);
        AttachSelectedComponent(parentSk, boneIndex);
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

    for (int32_t i = 0; i < sMaxButtons; ++i)
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
    mBlueprintText->SetTextSize(24.0f);
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
    Actor* selectedActor = GetTargetActor();

    if (selectedActor != nullptr)
    {
        bool bp = (selectedActor->GetBlueprintSource() != nullptr);
        mBlueprintText->SetVisible(bp);

        const std::vector<Component*>& components = selectedActor->GetComponents();

        // Rearrange by hierarchy
        std::vector<Component*> compList;
        std::vector<uint32_t> depthList;

        std::function<void(Node3D*, uint32_t)> AddTransComp = [&](Node3D* comp, uint32_t depth)
        {
            if (comp != nullptr)
            {
                compList.push_back(comp);
                depthList.push_back(depth);

                for (uint32_t i = 0; i < comp->GetNumChildren(); ++i)
                {
                    Node3D* child = comp->GetChild(i);

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
                OCT_ASSERT(!components[i]->IsNode3D() ||
                    static_cast<Node3D*>(components[i])->GetParent()->GetOwner() != selectedActor);
            }
        }

        // This assertion is not true if another actor is attached.
        //OCT_ASSERT(compList.size() == components.size());

        mListOffset = glm::min(mListOffset, int32_t(compList.size()) - 1);
        mListOffset = glm::max(mListOffset, 0);

        for (int32_t i = 0; i < sMaxButtons; ++i)
        {
            int32_t index = mListOffset + i;

            if (i < mNumButtons &&
                index < int32_t(compList.size()))
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

        for (int32_t i = 0; i < sMaxButtons; ++i)
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
            for (uint32_t i = 0; i < sMaxButtons; ++i)
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
                if (comp->As<SkeletalMesh3D>())
                {
                    actions.push_back("Attach To Bone");
                }

                actions.push_back("Set Root Component");
                actions.push_back("Rename Component");
                actions.push_back("Duplicate Component");
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

    Actor* actor = GetTargetActor();

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

void HierarchyPanel::OnEditorModeChanged()
{
    EditorMode mode = GetEditorMode();

    if (mode == EditorMode::Blueprint)
    {
        SetAnchorMode(AnchorMode::LeftStretch);
        SetX(0);
        SetYRatio(0.0f);
        SetHeightRatio(0.5f);

        mNumButtons = sMaxButtons;
    }
    else
    {
        SetAnchorMode(AnchorMode::TopRight);
        SetPosition(-sDefaultWidth, 0.0f);
        SetDimensions(Panel::sDefaultWidth, sHierarchyPanelHeight);

        mNumButtons = 10;
    }
}

#endif
