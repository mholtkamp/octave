#include "Widgets/VerticalList.h"
#include "InputDevices.h"

#include <algorithm>
#include <glm/glm.hpp>

FORCE_LINK_DEF(VerticalList);
DEFINE_FACTORY(VerticalList, Widget);

VerticalList::VerticalList()
{

}

VerticalList::~VerticalList()
{
    for (uint32_t i = 0; i < mWidgets.size(); ++i)
    {
        if (mWidgets[i]->GetParent() == nullptr)
        {
            // We need to manually delete orphaned widgets.
            // The ~Widget() destructor will take care of children widgets.
            delete mWidgets[i];
            mWidgets[i] = nullptr;
        }
    }

    mWidgets.clear();
}

void VerticalList::Update()
{
    Canvas::Update();

    // This update needs to handle any scroll wheel input that adjust the display offset.
    if (GetScrollWheelDelta() != 0 &&
        ContainsMouse())
    {
        int32_t maxOffset = glm::max<int32_t>(int32_t(mWidgets.size()) - int32_t(mDisplayCount), 0);
        mDisplayOffset = glm::clamp<int32_t>(mDisplayOffset - GetScrollWheelDelta(), 0, maxOffset);

        mRebuildDisplay = true;
    }

    // If the display needs to be rebuilt, then do that now.
    // RebuildDisplay() will clear the mRebuildDisplay flag.
    if (mRebuildDisplay)
    {
        RebuildDisplay();
    }
}

void VerticalList::AddListItem(Widget* widget)
{
    // Make sure this is not a duplicate item. Or else bad things will happen (like double delete)
    OCT_ASSERT(std::find(mWidgets.begin(), mWidgets.end(), widget) == mWidgets.end());

    mWidgets.push_back(widget);
    mRebuildDisplay = true;
}

void VerticalList::RemoveListItem(Widget* widget)
{
    RemoveChild(widget);

    for (uint32_t i = 0; i < mWidgets.size(); ++i)
    {
        if (mWidgets[i] == widget)
        {
            mWidgets.erase(mWidgets.begin() + i);
            break;
        }
    }

    mRebuildDisplay = true;
}

Widget* VerticalList::RemoveListItem(uint32_t index)
{
    auto it = (mWidgets.begin() + index);
    Widget* retWidget = *it;

    RemoveChild(retWidget);
    mWidgets.erase(it);

    mRebuildDisplay = true;
    return retWidget;
}

void VerticalList::RemoveAllListItems()
{
    for (int32_t i = int32_t(mWidgets.size() - 1); i >= 0; --i)
    {
        RemoveListItem((uint32_t)i);
    }
}

Widget* VerticalList::GetListItem(uint32_t index)
{
    return mWidgets[index];
}

uint32_t VerticalList::GetNumListItems() const
{
    return uint32_t(mWidgets.size());
}

void VerticalList::SetDisplayCount(uint32_t count)
{
    if (mDisplayCount != count)
    {
        mDisplayCount = count;
        mRebuildDisplay = true;
    }
}

uint32_t VerticalList::GetDisplayCount() const
{
    return mDisplayCount;
}

void VerticalList::SetDisplayOffset(int32_t offset)
{
    if (mDisplayOffset != offset)
    {
        mDisplayOffset = offset;
        mRebuildDisplay = true;
    }
}

int32_t VerticalList::GetDisplayOffset() const
{
    return mDisplayOffset;
}

void VerticalList::SetFitContents(bool fit)
{
    mFitContents = fit;
}

void VerticalList::RebuildDisplay()
{
    // Remove all children first.
    for (int32_t i = (int32_t)mChildren.size() - 1; i >= 0; --i)
    {
        RemoveChild(i);
    }

    OCT_ASSERT(mChildren.size() == 0);

    uint32_t numDisplayed = 0;
    uint32_t curIndex = uint32_t(mDisplayOffset);

    while (curIndex < mWidgets.size() &&
           numDisplayed < mDisplayCount)
    {
        Widget* widgetToAdd = mWidgets[curIndex];
        AddChild(widgetToAdd);

        curIndex++;
        numDisplayed++;
    }

    // We have our correct children now.
    // So we just need to position them correctly.
    float yPos = 0.0f;
    float maxWidth = 1.0f;
    for (uint32_t i = 0; i < mChildren.size(); ++i)
    {
        mChildren[i]->SetPosition(0.0f, yPos);
        float width = mChildren[i]->GetWidth();
        float height = mChildren[i]->GetHeight();

        maxWidth = glm::max(maxWidth, width);
        yPos += height;
    }

    // Make sure this list canvas encompasses its children.
    if (mFitContents)
    {
        SetDimensions(maxWidth, yPos);
    }

    mRebuildDisplay = false;
}
