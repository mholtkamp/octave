#pragma once

#include "Widgets/Button.h"

class Actor;
class Component;

class HierarchyButton : public Button
{
public:

    HierarchyButton();
    void SetComponent(Component* component);
    void SetWidget(Widget* widget);
    Component* GetComponent();
    Widget* GetWidget();

    bool IsSelected() const;

    virtual void Update() override;

protected:

    Component* mComponent = nullptr;
    Widget* mWidget = nullptr;
    bool mPrevSelected = false;
    bool mWidgetMode = false;
};