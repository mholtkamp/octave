#pragma once

#include "Panel.h"
#include "Widgets/ModalList.h"

class Actor;
class Component;
class HierarchyButton;
class Button;
class TextField;

class HierarchyPanel : public Panel
{
public:

    HierarchyPanel();
    void OnSelectedComponentChanged();
    void RefreshCompButtons();

    virtual void HandleInput() override;
    virtual void Update() override;

    void OnEditorModeChanged();

    static const float sHierarchyPanelHeight;

protected:

    static void ActionHandler(Button* button);
    static void ShowAddComponentPrompt();
    static void DeleteComponent(Component* comp);
    static void AttachSelectedComponent(Component* newParent);
    static void SetRootComponent(Component* newParent);
    static void HandleRenameComponent(TextField* tf);

    static const int32_t sMaxButtons = 15;
    HierarchyButton* mButtons[sMaxButtons];
    Text* mBlueprintText = nullptr;

    int32_t mListOffset;
    int32_t mNumButtons = sMaxButtons;
};