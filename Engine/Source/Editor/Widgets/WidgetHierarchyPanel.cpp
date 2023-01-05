#if EDITOR

#include "Widgets/WidgetHierarchyPanel.h"
#include "Widgets/HierarchyButton.h"
#include "ActionManager.h"
#include "PanelManager.h"
#include "EditorState.h"
#include "InputDevices.h"
#include "Widgets/ActionList.h"
#include "Renderer.h"

#include <functional>

const float WidgetHierarchyPanel::sPanelHeight = 240;

static Widget* sActionWidget = nullptr;

void OnCreateNativeWidgetButtonPressed(Button* button)
{
    const std::string& className = button->GetTextString();

    Widget* parentWidget = GetSelectedWidget();
    WidgetHierarchyPanel* hierPanel = PanelManager::Get()->GetWidgetHierarchyPanel();

    if (parentWidget == nullptr)
    {
        parentWidget = hierPanel->GetRootWidget();
    }

    Widget* widget = Widget::CreateInstance(className.c_str());
    OCT_ASSERT(widget != nullptr);

    if (parentWidget != nullptr)
    {
        parentWidget->AddChild(widget);
    }
    else
    {
        hierPanel->SetRootWidget(widget);
    }

    ActionManager::Get()->EXE_AddWidget(widget);

    SetSelectedWidget(widget);
    hierPanel->RefreshButtons();

    Renderer::Get()->SetModalWidget(nullptr);
}

void OnHierarchyButtonPressed(Button* button)
{
    if (button != nullptr)
    {
        Widget* selectedWidget = GetSelectedWidget();

        HierarchyButton* hierButton = static_cast<HierarchyButton*>(button);
        Widget* widget = hierButton->GetWidget();

        if (selectedWidget != widget)
        {
            SetSelectedWidget(widget);
        }
        else
        {
            SetSelectedWidget(nullptr);
        }
    }
}

void WidgetHierarchyPanel::ActionHandler(Button* button)
{
    std::string buttonText = button->GetTextString();
    bool hideList = true;

    WidgetHierarchyPanel* hierPanel = PanelManager::Get()->GetWidgetHierarchyPanel();

    if (buttonText == "Add Native Widget")
    {
        ShowAddNativeWidgetPrompt();
        hideList = false;
    }
    else if (buttonText == "Add Mapped Widget")
    {
        ShowAddMappedWidgetPrompt();
        hideList = false;
    }
    else if (buttonText == "Delete Widget")
    {
        DeleteWidget(sActionWidget);
    }
    else if (buttonText == "Attach Selected")
    {
        if (sActionWidget != nullptr)
        {
            AttachSelectedWidget(sActionWidget);
        }
    }
    else if (buttonText == "Set Root Widget")
    {
        if (sActionWidget != nullptr)
        {
            hierPanel->SetRootWidget(sActionWidget);
        }
    }
    else if (buttonText == "Rename")
    {
        const char* defaultText = sActionWidget ?
            sActionWidget->GetName().c_str() :
            nullptr;

        ShowTextPrompt("Rename", HandleRenameWidget, defaultText);
        hideList = false;
    }

    if (hideList)
    {
        GetActionList()->Hide();
    }
}

void WidgetHierarchyPanel::ShowAddNativeWidgetPrompt()
{
    std::vector<std::string> actions;
    const std::vector<Factory*>& widgetFactories = Widget::GetFactoryList();

    for (uint32_t i = 0; i < widgetFactories.size(); ++i)
    {
        actions.push_back(widgetFactories[i]->GetClassName());
    }

    ActionList* actionList = GetActionList();
    actionList->SetActions(actions, OnCreateNativeWidgetButtonPressed);
}

void WidgetHierarchyPanel::DeleteWidget(Widget* widget)
{
    WidgetHierarchyPanel* panel = PanelManager::Get()->GetWidgetHierarchyPanel();
    if (widget != nullptr)
    {
        Widget* rootWidget = panel->GetRootWidget();

        bool isRoot = (widget == rootWidget);
        bool hasChildren = (widget->GetNumChildren() > 0);

        if (isRoot)
        {
            LogError("Cannot delete root widget");
        }
        else if (hasChildren)
        {
            // TODO: Fix this. No reason we can't delete all children.
            LogError("Cannot delete component with children");
        }
        else
        {
            ActionManager::Get()->EXE_RemoveWidget(widget);
            SetSelectedWidget(rootWidget);

            panel->RefreshButtons();
        }
    }
}

void WidgetHierarchyPanel::AttachSelectedWidget(Widget* newParent)
{
    WidgetHierarchyPanel* panel = PanelManager::Get()->GetWidgetHierarchyPanel();

    if (newParent == nullptr)
        return;

    Widget* selWidget = GetSelectedWidget();

    if (selWidget == nullptr)
        return;

    Widget* child = selWidget;
    Widget* parent = newParent;

    if (child->GetParent() != parent)
    {
        ActionManager::Get()->EXE_AttachWidget(child, parent);
    }

    panel->RefreshButtons();
}

void WidgetHierarchyPanel::SetRootWidget(Widget* newRoot)
{
    WidgetHierarchyPanel* panel = PanelManager::Get()->GetWidgetHierarchyPanel();

    if (newRoot == nullptr)
        return;

    Widget* oldRoot = panel->GetRootWidget();

    if (newRoot != oldRoot)
    {
        ActionManager::Get()->EXE_SetRootWidget(newRoot);
    }

    panel->RefreshButtons();
}

void WidgetHierarchyPanel::HandleRenameWidget(TextField* tf)
{
    WidgetHierarchyPanel* panel = PanelManager::Get()->GetWidgetHierarchyPanel();

    const std::string& newName = tf->GetTextString();

    if (newName != "" &&
        sActionWidget != nullptr)
    {
        ActionManager::Get()->EXE_EditProperty(sActionWidget, PropertyOwnerType::Widget, "Name", 0, newName);
    }

    Renderer::Get()->SetModalWidget(nullptr);
    panel->RefreshButtons();
}

WidgetHierarchyPanel::WidgetHierarchyPanel() :
    mListOffset(0)
{
    SetTitle("Hierarchy");
    SetAnchorMode(AnchorMode::TopRight);
    SetPosition(-sDefaultWidth, 0.0f);
    SetDimensions(Panel::sDefaultWidth, sPanelHeight);

    for (int32_t i = 0; i < sNumButtons; ++i)
    {
        mButtons[i] = new HierarchyButton();
        mButtons[i]->SetComponent(nullptr);
        mButtons[i]->SetPosition(0, i * mButtons[i]->GetHeight());
        mButtons[i]->SetPressedHandler(OnHierarchyButtonPressed);
        mBodyCanvas->AddChild(mButtons[i]);
    }
}

void WidgetHierarchyPanel::OnSelectedWidgetChanged()
{
    RefreshButtons();
}

void WidgetHierarchyPanel::RefreshButtons()
{
    Widget* rootWidget = GetRootWidget();

    if (rootWidget != nullptr)
    {

        //const std::vector<Component*>& components = selectedActor->GetComponents();

        // Rearrange by hierarchy
        std::vector<Widget*> widgetList;
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
        for (int32_t i = 0; i < sNumButtons; ++i)
        {
            mButtons[i]->SetWidget(nullptr);
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
