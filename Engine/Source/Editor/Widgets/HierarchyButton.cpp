#if EDITOR

#include "Widgets/HierarchyButton.h"
#include "Widgets/Panel.h"
#include "Components/Component.h"
#include "Actor.h"
#include "EditorState.h"
#include "Widgets/Quad.h"

HierarchyButton::HierarchyButton() :
    mComponent(nullptr),
    mPrevSelectedComponent(false)
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
}

Component* HierarchyButton::GetComponent()
{
    return mComponent;
}

void HierarchyButton::Update()
{
    Component* selectedComponent = GetSelectedComponent();
    if (mComponent == selectedComponent)
    {
        if (!mPrevSelectedComponent)
        {
            GetQuad()->SetColor({ 0.8f, 0.2f, 0.9f, 1.0f },
                { 0.8f, 0.2f, 0.9f, 1.0f },
                { 1.0f, 0.2f, 0.9f, 1.0f },
                { 1.0f, 0.2f, 0.9f, 1.0f });
        }
        mPrevSelectedComponent = true;
    }
    else
    {
        if (mPrevSelectedComponent)
        {
            GetQuad()->SetColor({ 0.2f, 0.2f, 0.8f, 1.0f },
                { 0.2f, 0.2f, 0.8f, 1.0f },
                { 0.5f, 0.2f, 0.8f, 0.5f },
                { 0.5f, 0.2f, 0.8f, 0.5f });
        }
        mPrevSelectedComponent = false;
    }

    Button::Update();
}

#endif
