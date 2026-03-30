#include "Nodes/Widgets/ListViewWidget.h"
#include "Nodes/Widgets/ListViewItemWidget.h"
#include "Nodes/Widgets/ScrollContainer.h"
#include "Nodes/Widgets/ArrayWidget.h"
#include "Assets/Scene.h"
#include "Log.h"
#include "InputDevices.h"

FORCE_LINK_DEF(ListViewWidget);
DEFINE_NODE(ListViewWidget, Widget);

static const char* sOrientationStrings[] = {
    "Vertical",
    "Horizontal"
};

bool ListViewWidget::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);
    OCT_ASSERT(prop->mOwner != nullptr);
    ListViewWidget* listView = static_cast<ListViewWidget*>(prop->mOwner);

    bool success = false;

    if (prop->mName == "Item Template")
    {
        listView->SetItemTemplate(*(Scene**)newValue);
        success = true;
    }
    else if (prop->mName == "Spacing")
    {
        listView->SetSpacing(*static_cast<const float*>(newValue));
        success = true;
    }
    else if (prop->mName == "Orientation")
    {
        listView->SetOrientation(static_cast<ArrayOrientation>(*static_cast<const uint8_t*>(newValue)));
        success = true;
    }
    else if (prop->mName == "Item Width")
    {
        listView->SetItemWidth(*static_cast<const float*>(newValue));
        success = true;
    }
    else if (prop->mName == "Item Height")
    {
        listView->SetItemHeight(*static_cast<const float*>(newValue));
        success = true;
    }

    return success;
}

void ListViewWidget::Create()
{
    Widget::Create();
    SetName("ListView");

    // Enable scissor clipping
    mUseScissor = true;

    // Set default size
    SetDimensions(200.0f, 300.0f);

    // Containers are created lazily in EnsureContainers()
    mScrollContainer = nullptr;
    mArrayWidget = nullptr;

    MarkDirty();
}

void ListViewWidget::Tick(float deltaTime)
{
    Widget::Tick(deltaTime);
}

void ListViewWidget::PreRender()
{
    Widget::PreRender();

    EnsureContainers();
}

void ListViewWidget::GatherProperties(std::vector<Property>& props)
{
    Widget::GatherProperties(props);

    SCOPED_CATEGORY("ListView");

    props.push_back(Property(DatumType::Asset, "Item Template", this, &mItemTemplate, 1,
        HandlePropChange, int32_t(Scene::GetStaticType())));
    props.push_back(Property(DatumType::Float, "Spacing", this, &mSpacing, 1, HandlePropChange));
    props.push_back(Property(DatumType::Byte, "Orientation", this, &mOrientation, 1,
        HandlePropChange, NULL_DATUM, int32_t(ArrayOrientation::Count), sOrientationStrings));
    props.push_back(Property(DatumType::Float, "Item Width", this, &mItemWidth, 1, HandlePropChange));
    props.push_back(Property(DatumType::Float, "Item Height", this, &mItemHeight, 1, HandlePropChange));
}

void ListViewWidget::EnsureContainers()
{
    if (mScrollContainer != nullptr)
    {
        return;
    }

    // Create ScrollContainer
    mScrollContainer = CreateChild<ScrollContainer>("ScrollContainer");
    mScrollContainer->SetTransient(true);
    mScrollContainer->SetAnchorMode(AnchorMode::FullStretch);
    mScrollContainer->SetMargins(0.0f, 0.0f, 0.0f, 0.0f);
    mScrollContainer->SetScrollSizeMode(ScrollSizeMode::FitWidth);
    mScrollContainer->SetChildInputPriority(true);
#if EDITOR
    mScrollContainer->mHiddenInTree = true;
#endif

    // Create ArrayWidget inside ScrollContainer
    mArrayWidget = mScrollContainer->CreateChild<ArrayWidget>("ArrayWidget");
    mArrayWidget->SetTransient(true);
    mArrayWidget->SetAnchorMode(AnchorMode::TopLeft);
    mArrayWidget->SetOrientation(mOrientation);
    mArrayWidget->SetSpacing(mSpacing);
#if EDITOR
    mArrayWidget->mHiddenInTree = true;
#endif
}

// Template

void ListViewWidget::SetItemTemplate(Scene* scene)
{
    mItemTemplate = scene;
    // Optionally rebuild items if data exists
    if (!mData.empty())
    {
        RebuildItems();
    }
    MarkDirty();
}

Scene* ListViewWidget::GetItemTemplate() const
{
    return mItemTemplate.Get<Scene>();
}

// Data Management

void ListViewWidget::SetData(const std::vector<Datum>& data)
{
    mData = data;
    mSelectedIndex = -1;  // Clear selection when data changes
    RebuildItems();
}

void ListViewWidget::AddItem(const Datum& data, int32_t index)
{
    EnsureContainers();

    if (index < 0 || index > (int32_t)mData.size())
    {
        index = (int32_t)mData.size();
    }

    // Insert data
    mData.insert(mData.begin() + index, data);

    // Create item widget
    ListViewItemWidget* item = CreateItem(index, data);

    // Insert into items vector
    mItems.insert(mItems.begin() + index, item);

    // Update indices for items after insertion
    UpdateItemIndices();

    // Update selection index if needed
    if (mSelectedIndex >= index)
    {
        mSelectedIndex++;
    }

    MarkDirty();
}

void ListViewWidget::RemoveItem(int32_t index)
{
    if (index < 0 || index >= (int32_t)mItems.size())
    {
        return;
    }

    // Destroy item widget
    if (mItems[index] != nullptr)
    {
        mItems[index]->Destroy();
    }
    mItems.erase(mItems.begin() + index);

    // Remove data
    mData.erase(mData.begin() + index);

    // Update indices
    UpdateItemIndices();

    // Update selection
    if (mSelectedIndex == index)
    {
        mSelectedIndex = -1;
        FireSelectionChanged(-1);
    }
    else if (mSelectedIndex > index)
    {
        mSelectedIndex--;
    }

    MarkDirty();
}

void ListViewWidget::UpdateItem(int32_t index, const Datum& data)
{
    if (index < 0 || index >= (int32_t)mItems.size())
    {
        return;
    }

    mData[index] = data;
    FireItemUpdate(index, data, mItems[index]);
}

void ListViewWidget::Clear()
{
    // Destroy all items
    for (auto* item : mItems)
    {
        if (item != nullptr)
        {
            item->Destroy();
        }
    }
    mItems.clear();
    mData.clear();

    if (mSelectedIndex != -1)
    {
        mSelectedIndex = -1;
        FireSelectionChanged(-1);
    }

    MarkDirty();
}

int32_t ListViewWidget::GetItemCount() const
{
    return (int32_t)mData.size();
}

Datum ListViewWidget::GetItemData(int32_t index) const
{
    if (index >= 0 && index < (int32_t)mData.size())
    {
        return mData[index];
    }
    return Datum();
}

// Layout

void ListViewWidget::SetSpacing(float spacing)
{
    mSpacing = spacing;
    if (mArrayWidget != nullptr)
    {
        mArrayWidget->SetSpacing(mSpacing);
    }
    MarkDirty();
}

float ListViewWidget::GetSpacing() const
{
    return mSpacing;
}

void ListViewWidget::SetOrientation(ArrayOrientation orientation)
{
    mOrientation = orientation;
    if (mArrayWidget != nullptr)
    {
        mArrayWidget->SetOrientation(mOrientation);
    }
    if (mScrollContainer != nullptr)
    {
        // Adjust scroll mode based on orientation
        if (mOrientation == ArrayOrientation::Vertical)
        {
            mScrollContainer->SetScrollSizeMode(ScrollSizeMode::FitWidth);
        }
        else
        {
            mScrollContainer->SetScrollSizeMode(ScrollSizeMode::FitHeight);
        }
    }
    MarkDirty();
}

ArrayOrientation ListViewWidget::GetOrientation() const
{
    return mOrientation;
}

// Item sizing

void ListViewWidget::SetItemWidth(float width)
{
    mItemWidth = width;
    // Apply to all existing items
    for (auto* item : mItems)
    {
        ApplyItemSize(item);
    }
    MarkDirty();
}

float ListViewWidget::GetItemWidth() const
{
    return mItemWidth;
}

void ListViewWidget::SetItemHeight(float height)
{
    mItemHeight = height;
    // Apply to all existing items
    for (auto* item : mItems)
    {
        ApplyItemSize(item);
    }
    MarkDirty();
}

float ListViewWidget::GetItemHeight() const
{
    return mItemHeight;
}

// Selection

void ListViewWidget::SetSelectedIndex(int32_t index)
{
    if (index < -1 || index >= (int32_t)mItems.size())
    {
        index = -1;
    }

    if (mSelectedIndex != index)
    {
        // Deselect previous
        if (mSelectedIndex >= 0 && mSelectedIndex < (int32_t)mItems.size())
        {
            mItems[mSelectedIndex]->SetSelected(false);
        }

        mSelectedIndex = index;

        // Select new
        if (mSelectedIndex >= 0)
        {
            mItems[mSelectedIndex]->SetSelected(true);
        }

        FireSelectionChanged(mSelectedIndex);
    }
}

int32_t ListViewWidget::GetSelectedIndex() const
{
    return mSelectedIndex;
}

Datum ListViewWidget::GetSelectedData() const
{
    if (mSelectedIndex >= 0 && mSelectedIndex < (int32_t)mData.size())
    {
        return mData[mSelectedIndex];
    }
    return Datum();
}

void ListViewWidget::ClearSelection()
{
    SetSelectedIndex(-1);
}

// Access

ScrollContainer* ListViewWidget::GetScrollContainer()
{
    EnsureContainers();
    return mScrollContainer;
}

ArrayWidget* ListViewWidget::GetArrayWidget()
{
    EnsureContainers();
    return mArrayWidget;
}

ListViewItemWidget* ListViewWidget::GetItem(int32_t index)
{
    if (index >= 0 && index < (int32_t)mItems.size())
    {
        return mItems[index];
    }
    return nullptr;
}

// Scroll control

void ListViewWidget::ScrollToItem(int32_t index)
{
    if (mScrollContainer == nullptr || index < 0 || index >= (int32_t)mItems.size())
    {
        return;
    }

    ListViewItemWidget* item = mItems[index];
    if (item == nullptr)
    {
        return;
    }

    // Calculate item position relative to ArrayWidget
    float itemPos = (mOrientation == ArrayOrientation::Vertical) ? item->GetY() : item->GetX();
    float itemSize = (mOrientation == ArrayOrientation::Vertical) ? item->GetHeight() : item->GetWidth();
    float viewportSize = (mOrientation == ArrayOrientation::Vertical) ? mScrollContainer->GetHeight() : mScrollContainer->GetWidth();

    glm::vec2 offset = mScrollContainer->GetScrollOffset();
    float currentOffset = (mOrientation == ArrayOrientation::Vertical) ? offset.y : offset.x;

    // If item is above viewport, scroll to show it at top
    if (itemPos < currentOffset)
    {
        if (mOrientation == ArrayOrientation::Vertical)
        {
            mScrollContainer->SetScrollOffsetY(itemPos);
        }
        else
        {
            mScrollContainer->SetScrollOffsetX(itemPos);
        }
    }
    // If item is below viewport, scroll to show it at bottom
    else if (itemPos + itemSize > currentOffset + viewportSize)
    {
        float newOffset = itemPos + itemSize - viewportSize;
        if (mOrientation == ArrayOrientation::Vertical)
        {
            mScrollContainer->SetScrollOffsetY(newOffset);
        }
        else
        {
            mScrollContainer->SetScrollOffsetX(newOffset);
        }
    }
}

// Protected methods

void ListViewWidget::RebuildItems()
{
    EnsureContainers();

    // Destroy all existing items
    for (auto* item : mItems)
    {
        if (item != nullptr)
        {
            item->Destroy();
        }
    }
    mItems.clear();

    // Create new items
    for (size_t i = 0; i < mData.size(); ++i)
    {
        ListViewItemWidget* item = CreateItem((int32_t)i, mData[i]);
        mItems.push_back(item);
    }

    MarkDirty();
}

ListViewItemWidget* ListViewWidget::CreateItem(int32_t index, const Datum& data)
{
    // Create item wrapper
    ListViewItemWidget* item = mArrayWidget->CreateChild<ListViewItemWidget>(
        ("Item" + std::to_string(index)).c_str());
    item->SetIndex(index);
    item->SetListView(this);

    // Instantiate template if set
    Scene* scene = mItemTemplate.Get<Scene>();
    if (scene != nullptr)
    {
        NodePtr content = scene->Instantiate();
        if (content != nullptr && content->IsWidget())
        {
            Widget* contentWidget = static_cast<Widget*>(content.Get());
            contentWidget->Attach(item);
            item->SetContentWidget(contentWidget);
        }
    }

    // Apply item sizing
    ApplyItemSize(item);

    // Fire Lua callback
    FireItemGenerate(index, data, item);

    return item;
}

void ListViewWidget::DestroyItem(int32_t index)
{
    if (index >= 0 && index < (int32_t)mItems.size())
    {
        if (mItems[index] != nullptr)
        {
            mItems[index]->Destroy();
        }
    }
}

void ListViewWidget::UpdateItemIndices()
{
    for (size_t i = 0; i < mItems.size(); ++i)
    {
        if (mItems[i] != nullptr)
        {
            mItems[i]->SetIndex((int32_t)i);
        }
    }
}

void ListViewWidget::ApplyItemSize(ListViewItemWidget* item)
{
    if (item == nullptr)
    {
        return;
    }

    if (mItemWidth > 0.0f)
    {
        item->SetWidth(mItemWidth);
    }
    if (mItemHeight > 0.0f)
    {
        item->SetHeight(mItemHeight);
    }
}

void ListViewWidget::FireItemGenerate(int32_t index, const Datum& data, ListViewItemWidget* item)
{
    // Call Lua function "OnItemGenerate" with (index, data, item)
    std::vector<Datum> args;
    args.push_back(Datum(index));
    args.push_back(data);
    args.push_back(Datum(item));
    CallFunction("OnItemGenerate", args);
}

void ListViewWidget::FireItemUpdate(int32_t index, const Datum& data, ListViewItemWidget* item)
{
    // Call Lua function "OnItemUpdate" with (index, data, item)
    std::vector<Datum> args;
    args.push_back(Datum(index));
    args.push_back(data);
    args.push_back(Datum(item));
    CallFunction("OnItemUpdate", args);
}

void ListViewWidget::FireSelectionChanged(int32_t index)
{
    Datum data;
    if (index >= 0 && index < (int32_t)mData.size())
    {
        data = mData[index];
    }

    std::vector<Datum> args;
    args.push_back(Datum(index));
    args.push_back(data);

    EmitSignal("SelectionChanged", args);
    CallFunction("OnSelectionChanged", args);
}

// Item event handlers

void ListViewWidget::OnItemClicked(int32_t index)
{
    // Update selection
    SetSelectedIndex(index);

    // Fire callback
    if (index >= 0 && index < (int32_t)mData.size())
    {
        std::vector<Datum> args;
        args.push_back(Datum(index));
        args.push_back(mData[index]);

        EmitSignal("ItemClicked", args);
        CallFunction("OnItemClicked", args);
    }
}

void ListViewWidget::OnItemHoverEnter(int32_t index)
{
    if (index >= 0 && index < (int32_t)mData.size())
    {
        std::vector<Datum> args;
        args.push_back(Datum(index));
        args.push_back(mData[index]);

        EmitSignal("ItemHoverEnter", args);
        CallFunction("OnItemHoverEnter", args);
    }
}

void ListViewWidget::OnItemHoverExit(int32_t index)
{
    if (index >= 0 && index < (int32_t)mData.size())
    {
        std::vector<Datum> args;
        args.push_back(Datum(index));
        args.push_back(mData[index]);

        EmitSignal("ItemHoverExit", args);
        CallFunction("OnItemHoverExit", args);
    }
}
