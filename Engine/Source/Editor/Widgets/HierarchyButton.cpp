#if EDITOR

#include "Widgets/HierarchyButton.h"
#include "Widgets/Panel.h"
#include "Nodes/Node.h"
#include "Nodes/Node.h"
#include "EditorState.h"
#include "Widgets/Quad.h"

HierarchyButton::HierarchyButton()
{
    SetDimensions(Panel::sDefaultWidth, 22);
}

void HierarchyButton::SetComponent(Component* component)
{
    mComponent = component;
    SetVisible(mComponent != nullptr);

    if (mComponent != nullptr)
    {
        SetTextString(mComponent->GetName());
    }

    mWidgetMode = false;
}

void HierarchyButton::SetWidget(Widget* widget)
{
    mWidget = widget;
    SetVisible(mWidget != nullptr);

    if (mWidget != nullptr)
    {
        SetTextString(mWidget->GetName());
    }

    mWidgetMode = true;
}

Component* HierarchyButton::GetComponent()
{
    return mComponent;
}

Widget* HierarchyButton::GetWidget()
{
    return mWidget;
}

bool HierarchyButton::IsSelected() const
{
    bool selected = false;

    if (mWidgetMode)
    {
        Widget* selectedWidget = GetSelectedWidget();
        selected = (mWidget == selectedWidget);
    }
    else
    {
        Component* selectedComponent = GetSelectedComponent();
        selected = (mComponent == selectedComponent);
    }

    return selected;
}

void HierarchyButton::Update()
{
    if (IsSelected())
    {
        if (!mPrevSelected)
        {
            GetQuad()->SetColor({ 0.8f, 0.2f, 0.9f, 1.0f },
                { 0.8f, 0.2f, 0.9f, 1.0f },
                { 1.0f, 0.2f, 0.9f, 1.0f },
                { 1.0f, 0.2f, 0.9f, 1.0f });
        }
        mPrevSelected = true;
    }
    else
    {
        if (mPrevSelected)
        {
            GetQuad()->SetColor({ 0.2f, 0.2f, 0.8f, 1.0f },
                { 0.2f, 0.2f, 0.8f, 1.0f },
                { 0.5f, 0.2f, 0.8f, 0.5f },
                { 0.5f, 0.2f, 0.8f, 0.5f });
        }
        mPrevSelected = false;
    }

    Button::Update();
}

#endif
