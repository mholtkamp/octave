#pragma once

#include "Nodes/Widgets/Widget.h"
#include "Nodes/Widgets/ArrayWidget.h"
#include "AssetRef.h"

class Scene;
class ScrollContainer;
class ArrayWidget;
class ListViewItemWidget;

class OCTAVE_API ListViewWidget : public Widget
{
public:

    DECLARE_NODE(ListViewWidget, Widget);

    virtual void Create() override;
    virtual void Tick(float deltaTime) override;
    virtual void PreRender() override;
    virtual void GatherProperties(std::vector<Property>& props) override;

    // Template
    void SetItemTemplate(Scene* scene);
    Scene* GetItemTemplate() const;

    // Data Management
    void SetData(const std::vector<Datum>& data);
    void AddItem(const Datum& data, int32_t index = -1);
    void RemoveItem(int32_t index);
    void UpdateItem(int32_t index, const Datum& data);
    void Clear();
    int32_t GetItemCount() const;
    Datum GetItemData(int32_t index) const;

    // Layout
    void SetSpacing(float spacing);
    float GetSpacing() const;
    void SetOrientation(ArrayOrientation orientation);
    ArrayOrientation GetOrientation() const;

    // Item sizing (0 = auto-size to content)
    void SetItemWidth(float width);
    float GetItemWidth() const;
    void SetItemHeight(float height);
    float GetItemHeight() const;

    // Single selection
    void SetSelectedIndex(int32_t index);
    int32_t GetSelectedIndex() const;
    Datum GetSelectedData() const;
    void ClearSelection();

    // Access
    ScrollContainer* GetScrollContainer();
    ArrayWidget* GetArrayWidget();
    ListViewItemWidget* GetItem(int32_t index);

    // Scroll control
    void ScrollToItem(int32_t index);

protected:

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

    void EnsureContainers();
    void RebuildItems();
    ListViewItemWidget* CreateItem(int32_t index, const Datum& data);
    void DestroyItem(int32_t index);
    void UpdateItemIndices();
    void ApplyItemSize(ListViewItemWidget* item);

    void FireItemGenerate(int32_t index, const Datum& data, ListViewItemWidget* item);
    void FireItemUpdate(int32_t index, const Datum& data, ListViewItemWidget* item);
    void FireSelectionChanged(int32_t index);

    // Item event handlers (called by items)
    friend class ListViewItemWidget;
    void OnItemClicked(int32_t index);
    void OnItemHoverEnter(int32_t index);
    void OnItemHoverExit(int32_t index);

    // Properties
    SceneRef mItemTemplate;
    float mSpacing = 0.0f;
    ArrayOrientation mOrientation = ArrayOrientation::Vertical;
    float mItemWidth = 0.0f;   // 0 = auto-size
    float mItemHeight = 0.0f;  // 0 = auto-size
    int32_t mSelectedIndex = -1;  // -1 = no selection

    // Transient children
    ScrollContainer* mScrollContainer = nullptr;
    ArrayWidget* mArrayWidget = nullptr;

    // Data & Items
    std::vector<Datum> mData;
    std::vector<ListViewItemWidget*> mItems;
};
