#pragma once

#include "Nodes/Widgets/Widget.h"

class ListViewWidget;

class OCTAVE_API ListViewItemWidget : public Widget
{
public:

    DECLARE_NODE(ListViewItemWidget, Widget);

    virtual void Create() override;
    virtual void Tick(float deltaTime) override;

    // Index in parent ListView
    void SetIndex(int32_t index);
    int32_t GetIndex() const;

    // Parent ListView reference
    void SetListView(ListViewWidget* listView);
    ListViewWidget* GetListView() const;

    // Content widget (the instantiated template)
    void SetContentWidget(Widget* widget);
    Widget* GetContentWidget() const;

    // Selection state
    void SetSelected(bool selected);
    bool IsSelected() const;

    // Hover state
    bool IsHovered() const;

protected:

    void HandlePointerEvents();

    int32_t mIndex = -1;
    ListViewWidget* mListView = nullptr;
    Widget* mContentWidget = nullptr;
    bool mSelected = false;
    bool mHovered = false;
    bool mWasHovered = false;
};
