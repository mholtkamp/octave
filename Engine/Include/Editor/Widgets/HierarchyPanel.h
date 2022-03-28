#pragma once

#include "Panel.h"
#include "Widgets/ModalList.h"

class Actor;
class Component;
class HierarchyButton;
class Button;

class HierarchyPanel : public Panel
{
public:

    HierarchyPanel();
    void OnSelectedComponentChanged();
    void RefreshCompButtons();

    virtual void HandleInput() override;
    virtual void Update() override;
    static const float sHierarchyPanelHeight;

protected:

    static void ActionHandler(Button* button);
    static void ShowAddComponentPrompt();
    static void DeleteComponent(Component* comp);
    static void AttachSelectedComponent(Component* newParent);

    static const int32_t sNumButtons = 10;
    HierarchyButton* mButtons[sNumButtons];
    Text* mBlueprintText = nullptr;

    int32_t mListOffset;
};