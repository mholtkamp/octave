#include "Nodes/Widgets/ListViewItemWidget.h"
#include "Nodes/Widgets/ListViewWidget.h"
#include "InputDevices.h"

FORCE_LINK_DEF(ListViewItemWidget);
DEFINE_NODE(ListViewItemWidget, Widget);

void ListViewItemWidget::Create()
{
    Widget::Create();
    SetName("ListViewItem");

    // Default size - will be overridden by content or ListView settings
    SetDimensions(100.0f, 30.0f);
}

void ListViewItemWidget::Tick(float deltaTime)
{
    Widget::Tick(deltaTime);

    HandlePointerEvents();
}

void ListViewItemWidget::SetIndex(int32_t index)
{
    mIndex = index;
}

int32_t ListViewItemWidget::GetIndex() const
{
    return mIndex;
}

void ListViewItemWidget::SetListView(ListViewWidget* listView)
{
    mListView = listView;
}

ListViewWidget* ListViewItemWidget::GetListView() const
{
    return mListView;
}

void ListViewItemWidget::SetContentWidget(Widget* widget)
{
    mContentWidget = widget;

    // Auto-size to content if no fixed size
    if (mContentWidget != nullptr)
    {
        // Check if parent ListView has fixed sizes
        bool hasFixedWidth = false;
        bool hasFixedHeight = false;
        if (mListView != nullptr)
        {
            hasFixedWidth = mListView->GetItemWidth() > 0.0f;
            hasFixedHeight = mListView->GetItemHeight() > 0.0f;
        }

        if (!hasFixedWidth)
        {
            SetWidth(mContentWidget->GetWidth());
        }
        if (!hasFixedHeight)
        {
            SetHeight(mContentWidget->GetHeight());
        }
    }
}

Widget* ListViewItemWidget::GetContentWidget() const
{
    return mContentWidget;
}

void ListViewItemWidget::SetSelected(bool selected)
{
    if (mSelected != selected)
    {
        mSelected = selected;

        if (mSelected)
        {
            EmitSignal("Selected", { this });
            CallFunction("OnSelected", { this });
        }
        else
        {
            EmitSignal("Deselected", { this });
            CallFunction("OnDeselected", { this });
        }
    }
}

bool ListViewItemWidget::IsSelected() const
{
    return mSelected;
}

bool ListViewItemWidget::IsHovered() const
{
    return mHovered;
}

void ListViewItemWidget::HandlePointerEvents()
{
    if (!ShouldHandleInput() || mListView == nullptr)
    {
        return;
    }

    bool isHovered = ContainsMouse();

    // Hover enter
    if (isHovered && !mWasHovered)
    {
        mHovered = true;
        mListView->OnItemHoverEnter(mIndex);
        EmitSignal("HoverEnter", { this });
        CallFunction("OnHoverEnter", { this });
    }
    // Hover exit
    else if (!isHovered && mWasHovered)
    {
        mHovered = false;
        mListView->OnItemHoverExit(mIndex);
        EmitSignal("HoverExit", { this });
        CallFunction("OnHoverExit", { this });
    }

    // Click - on pointer release while hovering
    if (isHovered && IsPointerJustUp(0))
    {
        mListView->OnItemClicked(mIndex);
        EmitSignal("Clicked", { this });
        CallFunction("OnClicked", { this });
    }

    mWasHovered = isHovered;
}
