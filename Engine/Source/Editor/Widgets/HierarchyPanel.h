#if NODE_CONV_EDITOR

#pragma once

#include "Panel.h"
#include "Nodes/Widgets/ModalList.h"

class Node;
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
    virtual void Tick(float deltaTime) override;

    void OnEditorModeChanged();

    static const float sHierarchyPanelHeight;

protected:

    static void ActionHandler(Button* button);
    static void ShowAddComponentPrompt();
    static void DeleteComponent(Component* comp);
    static void AttachSelectedComponent(Component* newParent, int32_t boneIdx);
    static void SetRootComponent(Component* newParent);
    static void HandleRenameComponent(TextField* tf);
    static void HandleBonePrompt(TextField* tf);

    static const int32_t sMaxButtons = 15;
    HierarchyButton* mButtons[sMaxButtons];
    Text* mBlueprintText = nullptr;

    int32_t mListOffset;
    int32_t mNumButtons = sMaxButtons;
};

#endif 
