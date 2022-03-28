#pragma once

#include "Widgets/Button.h"

class Actor;
class Component;

class HierarchyButton : public Button
{
public:

    HierarchyButton();
    void SetComponent(Component* component);
    Component* GetComponent();

    virtual void Update() override;

protected:

    Component* mComponent;
    bool mPrevSelectedComponent;
};