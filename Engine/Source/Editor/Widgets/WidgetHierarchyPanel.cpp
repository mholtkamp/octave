#if EDITOR

#include "Widgets/WidgetHierarchyPanel.h"
#include "Widgets/HierarchyButton.h"
#include "Assets/WidgetMap.h"
#include "ActionManager.h"
#include "PanelManager.h"
#include "EditorState.h"
#include "EditorUtils.h"
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
        parentWidget = GetEditRootWidget();
    }

    Widget* widget = CreateWidget(className);
    OCT_ASSERT(widget != nullptr);

    if (parentWidget != nullptr)
    {
        parentWidget->AddChild(widget);
    }
    else
    {
        SetEditRootWidget(widget);
    }

    ActionManager::Get()->EXE_AddWidget(widget);

    SetSelectedWidget(widget);
    hierPanel->RefreshButtons();

    Renderer::Get()->SetModalWidget(nullptr);
}

void OnCreateMappedWidgetButtonPressed(Button* button)
{
    const std::string& mapName = button->GetTextString();

    WidgetMap* widgetMap = LoadAsset<WidgetMap>(mapName);

    EditorInstantiateMappedWidget(widgetMap);

    Renderer::Get()->SetModalWidget(nullptr);
}

static void OnHierarchyButtonPressed(Button* button)
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
            SetRootWidgetAction(sActionWidget);
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

void WidgetHierarchyPanel::ShowAddMappedWidgetPrompt()
{
    std::vector<std::string> actions;

    auto& assetMap = AssetManager::Get()->GetAssetMap();

    for (auto& pair : assetMap)
    {
        if (pair.second->mType == WidgetMap::GetStaticType())
        {
            actions.push_back(pair.second->mName);
        }
    }

    ActionList* actionList = GetActionList();
    actionList->SetActions(actions, OnCreateMappedWidgetButtonPressed);
}

void WidgetHierarchyPanel::DeleteWidget(Widget* widget)
{
    WidgetHierarchyPanel* panel = PanelManager::Get()->GetWidgetHierarchyPanel();
    if (widget != nullptr)
    {
        ActionManager::Get()->EXE_RemoveWidget(widget);

        Widget* rootWidget = GetEditRootWidget();
        SetSelectedWidget(rootWidget);

        panel->RefreshButtons();
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

void WidgetHierarchyPanel::SetRootWidgetAction(Widget* newRoot)
{
    WidgetHierarchyPanel* panel = PanelManager::Get()->GetWidgetHierarchyPanel();

    if (newRoot == nullptr || newRoot->GetWidgetMap() != nullptr)
        return;

    Widget* oldRoot = GetEditRootWidget();

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
    SetAnchorMode(AnchorMode::LeftStretch);
    SetYRatio(0.0f);
    SetHeightRatio(0.5f);

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
    Widget* rootWidget = GetEditRootWidget();

    if (rootWidget != nullptr)
    {
        //const std::vector<Component*>& components = selectedActor->GetComponents();

        // Rearrange by hierarchy
        std::vector<Widget*> widgetList;
        std::vector<uint32_t> depthList;

        std::function<void(Widget*, uint32_t)> AddWidget = [&](Widget* widget, uint32_t depth)
        {
            if (widget != nullptr)
            {
                widgetList.push_back(widget);
                depthList.push_back(depth);

                for (uint32_t i = 0; i < widget->GetNumChildren(); ++i)
                {
                    Widget* child = widget->GetChild(i);

                    if (widget->GetWidgetMap() == nullptr)
                    {
                        AddWidget(child, depth + 1);
                    }
                }
            }
        };

        AddWidget(rootWidget, 0);

        mListOffset = glm::min(mListOffset, int32_t(widgetList.size()) - 1);
        mListOffset = glm::max(mListOffset, 0);

        for (int32_t i = 0; i < sNumButtons; ++i)
        {
            int32_t index = mListOffset + i;

            if (index < int32_t(widgetList.size()))
            {
                mButtons[i]->SetWidget(widgetList[index]);
                mButtons[i]->SetX(depthList[index] * 10.0f);
            }
            else
            {
                mButtons[i]->SetWidget(nullptr);
                mButtons[i]->SetX(0.0f);
            }
        }

        mCachedNumWidgets = (uint32_t)widgetList.size();
    }
    else
    {
        for (int32_t i = 0; i < sNumButtons; ++i)
        {
            mButtons[i]->SetWidget(nullptr);
        }

        mCachedNumWidgets = 0;
    }
}

void WidgetHierarchyPanel::HandleInput()
{
    Panel::HandleInput();

    if (ShouldHandleInput() &&
        IsMouseInsidePanel())
    {
        bool shiftDown = IsShiftDown();

        if (IsMouseButtonJustUp(MOUSE_RIGHT))
        {
            Widget* widget = nullptr;

            // Check if asset button is selected
            for (uint32_t i = 0; i < sNumButtons; ++i)
            {
                if (mButtons[i]->IsVisible() &&
                    mButtons[i]->ContainsMouse() &&
                    mButtons[i]->GetWidget() != nullptr)
                {
                    widget = mButtons[i]->GetWidget();
                    break;
                }
            }

            std::vector<std::string> actions;
            actions.push_back("Add Native Widget");
            actions.push_back("Add Mapped Widget");

            if (widget != nullptr)
            {
                actions.push_back("Delete Widget");

                if (widget->GetWidgetMap() == nullptr)
                {
                    actions.push_back("Attach Selected");
                    actions.push_back("Set Root Widget");
                }

                actions.push_back("Rename");
            }
            GetActionList()->SetActions(actions, ActionHandler);
            sActionWidget = widget;
        }

        if (shiftDown && IsKeyJustDown(KEY_A))
        {
            ShowAddNativeWidgetPrompt();
        }

        if (shiftDown && IsKeyJustDown(KEY_Q))
        {
            ShowAddMappedWidgetPrompt();
        }

        if (IsKeyJustDown(KEY_DELETE) || IsKeyJustDown(KEY_X))
        {
            DeleteWidget(GetSelectedWidget());
        }
    }
}

void WidgetHierarchyPanel::Update()
{
    Panel::Update();

    bool refresh = false;

    if (GetEditRootWidget() != mCachedEditRoot)
    {
        mCachedEditRoot = GetEditRootWidget();
        refresh = true;
    }

    SetMaxScroll(glm::max(0, int32_t(mCachedNumWidgets) - 1));

    int32_t listOffset = mScroll;

    if (listOffset != mListOffset)
    {
        mListOffset = listOffset;
        refresh = true;
    }

    if (refresh)
    {
        RefreshButtons();
    }
}

#endif
