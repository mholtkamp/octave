#pragma once

#include "Panel.h"
#include "Widgets/ModalList.h"

class Button;
class TextField;
class HierarchyButton;

class WidgetHierarchyPanel : public Panel
{
public:

    WidgetHierarchyPanel();
    void OnSelectedWidgetChanged();
    void RefreshButtons();

    virtual void HandleInput() override;
    virtual void Update() override;

    Widget* GetRootWidget();
    void SetRootWidget(Widget* widget);

    static const float sPanelHeight;

protected:

    static void ActionHandler(Button* button);
    static void ShowAddNativeWidgetPrompt();
    static void ShowAddMappedWidgetPrompt();
    static void DeleteWidget(Widget* widget);
    static void AttachSelectedWidget(Widget* newParent);
    static void SetRootWidget(Widget* newRoot);
    static void HandleRenameWidget(TextField* tf);

    static const int32_t sNumButtons = 10;
    HierarchyButton* mButtons[sNumButtons];

    Widget* mRootWidget = nullptr;

    int32_t mListOffset;
};