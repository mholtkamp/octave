#pragma once

#include "Canvas.h"

class VerticalList : public Canvas
{
public:

    DECLARE_NODE(VerticalList, Canvas);

    VerticalList();
    virtual ~VerticalList();

    virtual void Tick(float deltaTime) override;

    void AddListItem(Widget* widget);
    void RemoveListItem(Widget* widget);
    Widget* RemoveListItem(uint32_t index);
    void RemoveAllListItems();


    Widget* GetListItem(uint32_t index);
    uint32_t GetNumListItems() const;

    void SetDisplayCount(uint32_t count);
    uint32_t GetDisplayCount() const;
    void SetDisplayOffset(int32_t offset);
    int32_t GetDisplayOffset() const;

    void SetFitContents(bool fit);

protected:

    void RebuildDisplay();

    uint32_t mDisplayCount = 5;
    int32_t mDisplayOffset = 0;
    std::vector<Widget*> mWidgets;
    bool mRebuildDisplay = false;
    bool mFitContents = true;
};