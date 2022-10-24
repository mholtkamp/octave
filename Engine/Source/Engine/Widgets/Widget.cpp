#include "Widgets/Widget.h"
#include "Renderer.h"
#include "InputDevices.h"
#include "Engine.h"
#include "Log.h"

#include "Graphics/Graphics.h"

#include <glm/gtx/matrix_transform_2d.hpp>

FORCE_LINK_DEF(Widget);
DEFINE_FACTORY_MANAGER(Widget);
DEFINE_FACTORY(Widget, Widget);

Widget::Widget() :
    mParent(nullptr),
    mTransform(1.0f),
    mColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)),
    mOffset(glm::vec2(0.0f, 0.0f)),
    mSize(glm::vec2(100.0f, 100.0f)),
    mPivot(glm::vec2(0.0f, 0.0f)),
    mScale(glm::vec2(1.0f, 1.0f)),
    mAbsoluteScale(glm::vec2(1.0f, 1.0f)),
    mRotation(0.0f),
    mAnchorMode(AnchorMode::TopLeft),
    mActiveMargins(0),
    mUseScissor(false),
    mVisible(true),
    mScriptOwned(false)
{
    MarkDirty();

    mCachedScissorRect = Rect(0.0f, 0.0f, 10000.0f, 10000.0f);
    mCachedParentScissorRect = Rect(0.0f, 0.0f, 10000.0f, 10000.0f);;
}

Widget::~Widget()
{
    for (uint32_t i = 0; i < mChildren.size(); ++i)
    {
        delete mChildren[i];
        mChildren[i] = nullptr;
    }

    mChildren.clear();
}

// Issue gpu commands to display the widget.
// Recursively render children.
void Widget::RecursiveRender()
{
    Render();

    if (mUseScissor)
    {
        PushScissor();
    }

    for (uint32_t i = 0; i < mChildren.size(); ++i)
    {
        if (mChildren[i]->IsVisible())
        {
            mChildren[i]->RecursiveRender();
        }
    }

    if (mUseScissor)
    {
        PopScissor();
    }
}

void Widget::Render()
{

}

// Refresh any data used for rendering based on this widget's state. Use dirty flag.
// Recursively update children.
void Widget::RecursiveUpdate()
{
    Update();
    mDirty[Renderer::Get()->GetFrameIndex()] = false;

    for (uint32_t i = 0; i < mChildren.size(); ++i)
    {
        if (mChildren[i]->IsVisible())
        {
            mChildren[i]->RecursiveUpdate();
        }
    }
}

void Widget::Update()
{
    if (IsDirty())
    {
        UpdateRect();
    }
}

Rect Widget::GetRect()
{
    return mRect;
}

void Widget::SetX(float x)
{
    if (StretchX())
    {
        mOffset.x = PixelsToRatioX(x);
    }
    else
    {
        mOffset.x = x;
    }

    mActiveMargins &= (~MF_Left);
    MarkDirty();
}

void Widget::SetY(float y)
{
    if (StretchY())
    {
        mOffset.y = PixelsToRatioY(y);
    }
    else
    {
        mOffset.y = y;
    }

    mActiveMargins &= (~MF_Top);
    MarkDirty();
}

void Widget::SetWidth(float width)
{
    if (StretchX())
    {
        mSize.x = PixelsToRatioX(width);
    }
    else
    {
        mSize.x = width;
    }

    mActiveMargins &= (~MF_Right);
    MarkDirty();
}

void Widget::SetHeight(float height)
{
    if (StretchY())
    {
        mSize.y = PixelsToRatioY(height);
    }
    else
    {
        mSize.y = height;
    }

    mActiveMargins &= (~MF_Bottom);
    MarkDirty();
}


void Widget::SetXRatio(float x)
{
    if (StretchX())
    {
        mOffset.x = x;
    }
    else
    {
        mOffset.x = RatioToPixelsX(x);
    }

    mActiveMargins &= (~MF_Left);
    MarkDirty();
}

void Widget::SetYRatio(float y)
{
    if (StretchY())
    {
        mOffset.y = y;
    }
    else
    {
        mOffset.y = RatioToPixelsY(y);
    }

    mActiveMargins &= (~MF_Top);
    MarkDirty();
}

void Widget::SetWidthRatio(float width)
{
    if (StretchX())
    {
        mSize.x = width;
    }
    else
    {
        mSize.x = RatioToPixelsX(width);
    }

    mActiveMargins &= (~MF_Right);
    MarkDirty();
}

void Widget::SetHeightRatio(float height)
{
    if (StretchY())
    {
        mSize.y = height;
    }
    else
    {
        mSize.y = RatioToPixelsY(height);
    }

    mActiveMargins &= (~MF_Bottom);
    MarkDirty();
}

void Widget::SetLeftMargin(float left)
{
    mOffset.x = left;
    mActiveMargins |= MF_Left;
    MarkDirty();
}

void Widget::SetTopMargin(float top)
{
    mOffset.y = top;
    mActiveMargins |= MF_Top;
    MarkDirty();
}

void Widget::SetRightMargin(float right)
{
    mSize.x = right;
    mActiveMargins |= MF_Right;
    MarkDirty();
}

void Widget::SetBottomMargin(float bottom)
{
    mSize.y = bottom;
    mActiveMargins |= MF_Bottom;
    MarkDirty();
}


void Widget::SetPosition(float x, float y)
{
    SetX(x);
    SetY(y);
}

void Widget::SetDimensions(float width, float height)
{
    SetWidth(width);
    SetHeight(height);
}

void Widget::SetPosition(glm::vec2 position)
{
    SetPosition(position.x, position.y);
}

void Widget::SetDimensions(glm::vec2 dimensions)
{
    SetDimensions(dimensions.x, dimensions.y);
}

void Widget::SetRect(float x, float y, float width, float height)
{
    SetPosition(x, y);
    SetDimensions(width, height);
}

void Widget::SetRect(glm::vec2 position, glm::vec2 dimensions)
{
    SetPosition(position);
    SetDimensions(dimensions);
}

void Widget::SetRect(Rect rect)
{
    SetPosition(glm::vec2(rect.mX, rect.mY));
    SetDimensions(glm::vec2(rect.mWidth, rect.mHeight));
}

void Widget::SetRatios(float x, float y, float width, float height)
{
    SetXRatio(x);
    SetYRatio(y);
    SetWidthRatio(width);
    SetHeightRatio(height);
}

void Widget::SetMargins(float left, float top, float right, float bottom)
{
    SetLeftMargin(left);
    SetTopMargin(top);
    SetRightMargin(right);
    SetBottomMargin(bottom);
}

void Widget::SetOffset(float x, float y)
{
    mOffset.x = x;
    mOffset.y = y;
    MarkDirty();
}

glm::vec2 Widget::GetOffset() const
{
    return mOffset;
}

void Widget::SetSize(float x, float y)
{
    mSize.x = x;
    mSize.y = y;
    MarkDirty();
}

glm::vec2 Widget::GetSize() const
{
    return mSize;
}

AnchorMode Widget::GetAnchorMode() const
{
    return mAnchorMode;
}

void Widget::SetAnchorMode(AnchorMode anchorMode)
{
    if (mAnchorMode != anchorMode)
    {
        // When switching anchor mode, we must:
        // (1) Convert between pixel/percent offsets and sizes if stretching behavior changes
        // (2) Preserve position and dimensions on the screen?? Not sure about this one actually...

        bool oldStretchX = AnchorStretchesX(mAnchorMode);
        bool oldStretchY = AnchorStretchesY(mAnchorMode);
        bool newStretchX = AnchorStretchesX(anchorMode);
        bool newStretchY = AnchorStretchesY(anchorMode);

        if (oldStretchX && !newStretchX)
        {
            mOffset.x = RatioToPixelsX(mOffset.x);
            mSize.x = RatioToPixelsX(mSize.x);
        }
        else if (!oldStretchX && newStretchX)
        {
            mOffset.x = PixelsToRatioX(mOffset.x);
            mSize.x = PixelsToRatioX(mSize.x);
        }

        if (oldStretchY && !newStretchY)
        {
            mOffset.y = RatioToPixelsX(mOffset.y);
            mSize.y = RatioToPixelsX(mSize.y);
        }
        else if (!oldStretchY && newStretchY)
        {
            mOffset.y = PixelsToRatioX(mOffset.y);
            mSize.y = PixelsToRatioX(mSize.y);
        }

        mAnchorMode = anchorMode;
        MarkDirty();
    }

}

bool Widget::AnchorStretchesX(AnchorMode mode) const
{
    return
        mode == AnchorMode::TopStretch ||
        mode == AnchorMode::MidHorizontalStretch ||
        mode == AnchorMode::BottomStretch ||
        mode == AnchorMode::FullStretch;
}

bool Widget::AnchorStretchesY(AnchorMode mode) const
{
    return
        mode == AnchorMode::LeftStretch ||
        mode == AnchorMode::MidVerticalStretch ||
        mode == AnchorMode::RightStretch ||
        mode == AnchorMode::FullStretch;
}

glm::vec2 Widget::GetAnchorRatio() const
{
    glm::vec2 ratio = { 0.0f, 0.0f };

    switch (mAnchorMode)
    {
    case AnchorMode::TopLeft: ratio = { 0.0f, 0.0f }; break;
    case AnchorMode::TopMid: ratio = { 0.5f, 0.0f }; break;
    case AnchorMode::TopRight: ratio = { 1.0f, 0.0f }; break;
    case AnchorMode::MidLeft: ratio = { 0.0f, 0.5f }; break;
    case AnchorMode::Mid: ratio = { 0.5f, 0.5f }; break;
    case AnchorMode::MidRight: ratio = { 1.0f, 0.5f }; break;
    case AnchorMode::BottomLeft: ratio = { 0.0f, 1.0f }; break;
    case AnchorMode::BottomMid: ratio = { 0.5f, 1.0f }; break;
    case AnchorMode::BottomRight: ratio = { 1.0f, 1.0f }; break;

    case AnchorMode::TopStretch: ratio = { 0.5f, 0.0f }; break;
    case AnchorMode::MidHorizontalStretch: ratio = { 0.5f, 0.5f }; break;
    case AnchorMode::BottomStretch: ratio = { 0.5f, 1.0f }; break;

    case AnchorMode::LeftStretch: ratio = { 0.0f, 0.5f }; break;
    case AnchorMode::MidVerticalStretch: ratio = { 0.5f, 0.5f }; break;
    case AnchorMode::RightStretch: ratio = { 1.0f, 0.5f }; break;

    case AnchorMode::FullStretch: ratio = { 0.5f, 0.5f }; break;
    }

    return ratio;
}

float Widget::GetX() const
{
    if (StretchX())
    {
        return (mActiveMargins & MF_Left) ?
            mOffset.x :
            GetParentWidth() * mSize.x;
    }
    else
    {
        return mOffset.x;
    }
}

float Widget::GetY() const
{
    if (StretchY())
    {
        return (mActiveMargins & MF_Top) ?
            mOffset.y :
            GetParentHeight() * mSize.y;
    }
    else
    {
        return mOffset.y;
    }
}

float Widget::GetWidth() const
{
    if (StretchX())
    {
        return (mActiveMargins & MF_Right) ?
            GetParentWidth() - GetX() - mSize.x :
            GetParentWidth() * mSize.x;
    }
    else
    {
        return mSize.x;
    }
}

float Widget::GetHeight() const
{
    if (StretchY())
    {
        return (mActiveMargins & MF_Bottom) ?
            GetParentHeight() - GetY() - mSize.y :
            GetParentHeight() * mSize.y;
    }
    else
    {
        return mSize.y;
    }
}

glm::vec2 Widget::GetPosition() const
{
    return { GetX(), GetY() };
}

glm::vec2 Widget::GetDimensions() const
{
    return { GetWidth(), GetHeight() };
}

void Widget::UpdateRect()
{
    Rect parentRect;

    if (mParent != nullptr)
    {
        parentRect = mParent->GetRect();
    }
    else
    {
        glm::vec2 res = Renderer::Get()->GetScreenResolution();
        parentRect.mX = 0.0f;
        parentRect.mY = 0.0f;
        parentRect.mWidth = res.x;
        parentRect.mHeight = res.y;
    }

    glm::vec2 anchorRatio = GetAnchorRatio();
    const bool stretchX = StretchX();
    const bool stretchY = StretchY();

    glm::vec2 anchorPos;
    anchorPos.x = parentRect.mX + anchorRatio.x * parentRect.mWidth;
    anchorPos.y = parentRect.mY + anchorRatio.y * parentRect.mHeight;

    mAbsoluteScale = mScale;
    if (mParent != nullptr)
    {
        mAbsoluteScale *= mParent->mAbsoluteScale;
    }
    else
    {
        mAbsoluteScale *= Renderer::Get()->GetGlobalUiScale();
    }

    if (stretchX)
    {
        mRect.mX = (mActiveMargins & MF_Left) ?
            (parentRect.mX + mOffset.x) :
            (parentRect.mX + parentRect.mWidth * mOffset.x);
        mRect.mWidth = (mActiveMargins & MF_Right) ?
            (parentRect.mX + parentRect.mWidth - mRect.mX - mSize.x) :
            (parentRect.mWidth * mSize.x);
    }
    else
    {
        mRect.mX = anchorPos.x + mOffset.x * mAbsoluteScale.x;
        mRect.mWidth = mSize.x * mAbsoluteScale.x;
    }

    if (stretchY)
    {
        mRect.mY = (mActiveMargins & MF_Top) ?
            (parentRect.mY + mOffset.y) :
            (parentRect.mY + parentRect.mHeight * mOffset.y);
        mRect.mHeight = (mActiveMargins & MF_Bottom) ?
            (parentRect.mY + parentRect.mHeight - mRect.mY - mSize.y) :
            (parentRect.mHeight * mSize.y);
    }
    else
    {
        mRect.mY = anchorPos.y + mOffset.y * mAbsoluteScale.y;
        mRect.mHeight = mSize.y * mAbsoluteScale.y;
    }

    glm::vec2 pivotPoint =
    {
        mRect.mX + mRect.mWidth * mPivot.x,
        mRect.mY + mRect.mHeight * mPivot.y
    };

    mTransform = glm::mat4(1.0f);
    mTransform = glm::translate(mTransform, { pivotPoint.x, pivotPoint.y });
    mTransform = glm::rotate(mTransform, mRotation * DEGREES_TO_RADIANS);
    mTransform = glm::translate(mTransform, { -pivotPoint.x, -pivotPoint.y });

    if (mParent)
    {
        mTransform = mParent->GetTransform() * mTransform;
    }
}

void Widget::FitInsideParent()
{
    if (IsDirty())
    {
        UpdateRect();
    }

    Rect parentRect;

    if (GetParent() != nullptr)
    {
        parentRect.mX = 0.0f;
        parentRect.mY = 0.0f;
        parentRect.mWidth = GetParent()->GetWidth();
        parentRect.mHeight = GetParent()->GetHeight();
    }
    else
    {
        // Fit to screen
        parentRect.mX = 0.0f;
        parentRect.mY = 0.0f;
        parentRect.mWidth = (float)GetEngineState()->mWindowWidth;
        parentRect.mHeight = (float)GetEngineState()->mWindowHeight;
    }

    if (mRect.mX < parentRect.mX)
        SetX(parentRect.mX);
    if (mRect.mY < parentRect.mY)
        SetY(parentRect.mY);
    if (mRect.Right() > parentRect.Right())
        SetX(parentRect.Right() - mRect.mWidth);
    if (mRect.Bottom() > parentRect.Bottom())
        SetY(parentRect.Bottom() - mRect.mHeight);
}

float Widget::GetParentWidth() const
{
    return
        mParent ?
        mParent->GetWidth() :
        Renderer::Get()->GetScreenResolution().x;
}

float Widget::GetParentHeight() const
{
    return
        mParent ?
        mParent->GetHeight() :
        Renderer::Get()->GetScreenResolution().y;
}

void Widget::SetVisible(bool visible)
{
    mVisible = visible;
}

bool Widget::IsVisible() const
{
    return mVisible;
}

void Widget::SetColor(glm::vec4 color)
{
    mColor = color;
    MarkDirty();
}

bool Widget::ShouldHandleInput()
{
    Widget* modal = Renderer::Get()->GetModalWidget();
    bool handleInput = ((modal == nullptr) || (Renderer::Get()->IsInModalWidgetUpdate()));
    return handleInput;
}

void Widget::AddChild(Widget* widget, int32_t index)
{
    if (widget != nullptr)
    {
        if (widget->mParent != nullptr)
        {
            widget->mParent->RemoveChild(widget);
        }

        if (index >= 0 && index <= (int32_t)mChildren.size())
        {
            mChildren.insert(mChildren.begin() + index, widget);
        }
        else
        {
            mChildren.push_back(widget);
        }

        widget->mParent = this;
        widget->MarkDirty();
    }
}

Widget* Widget::RemoveChild(Widget* widget)
{
    Widget* removedWidget = nullptr;

    for (uint32_t i = 0; i < mChildren.size(); ++i)
    {
        if (mChildren[i] == widget)
        {
            removedWidget = mChildren[i];
            removedWidget->mParent = nullptr;
            mChildren.erase(mChildren.begin() + i);
            break;
        }
    }

    return removedWidget;
}

Widget* Widget::RemoveChild(int32_t index)
{
    Widget* removedWidget = nullptr;

    if (index >= 0 &&
        index < int32_t(mChildren.size()))
    {
        removedWidget = mChildren[index];
        removedWidget->mParent = nullptr;
        mChildren.erase(mChildren.begin() + index);
    }

    return removedWidget;
}

Widget* Widget::GetChild(int32_t index)
{
    return mChildren[index];
}

Widget* Widget::GetParent()
{
    return mParent;
}

void Widget::DetachFromParent()
{
    if (mParent != nullptr)
    {
        mParent->RemoveChild(this);
    }
}

uint32_t Widget::GetNumChildren() const
{
    return uint32_t(mChildren.size());
}

void Widget::MarkDirty()
{
    for (uint32_t i = 0; i < MAX_FRAMES; ++i)
    {
        mDirty[i] = true;
    }

    for (uint32_t i = 0; i < mChildren.size(); ++i)
    {
        mChildren[i]->MarkDirty();
    }
}

bool Widget::IsDirty() const
{
    uint32_t frameIndex = Renderer::Get()->GetFrameIndex();
    return mDirty[frameIndex];
}

float Widget::InterfaceToNormalized(float interfaceCoord, float interfaceSize)
{
    return (interfaceCoord / interfaceSize) * 2.0f - 1.0f;
}

bool Widget::IsMouseInside(Rect rect)
{
    int32_t mouseX;
    int32_t mouseY;
    GetMousePosition(mouseX, mouseY);
    const bool containsMouse = rect.ContainsPoint((float)mouseX, (float)mouseY);

    return containsMouse;
}

bool Widget::ContainsMouse(bool testScissor) const
{
    Rect testRect = mRect;
    if (testScissor)
    {
        testRect.Clamp(mCachedScissorRect);
    }

    return IsMouseInside(testRect);
}

bool Widget::ContainsPoint(int32_t x, int32_t y)
{
    Rect testRect = mRect;
    testRect.Clamp(mCachedScissorRect);
    return testRect.ContainsPoint((float)x, (float)y);
}

void Widget::MoveToMousePosition()
{
    // Position at mouse location
    int32_t mouseX = 0;
    int32_t mouseY = 0;
    GetMousePosition(mouseX, mouseY);
    SetPosition((float) mouseX, (float) mouseY);
}

const glm::mat3& Widget::GetTransform()
{
    return mTransform;
}

void Widget::SetRotation(float degrees)
{
    mRotation = degrees;
    MarkDirty();
}

float Widget::GetRotation() const
{
    return mRotation;
}


void Widget::SetPivot(glm::vec2 pivot)
{
    mPivot = pivot;
    MarkDirty();
}

glm::vec2 Widget::GetPivot() const
{
    return mPivot;
}

void Widget::SetScale(glm::vec2 scale)
{
    mScale = scale;
    MarkDirty();
}

glm::vec2 Widget::GetScale() const
{
    return mScale;
}

bool Widget::IsScissorEnabled() const
{
    return mUseScissor;
}

void Widget::EnableScissor(bool enable)
{
    mUseScissor = enable;
}

bool Widget::IsScriptOwned() const
{
    return mScriptOwned;
}

void Widget::SetScriptOwned(bool scriptOwned)
{
    mScriptOwned = scriptOwned;
}

float Widget::PixelsToRatioX(float x) const
{
    float parentWidth = GetParentWidth();
    return (parentWidth > 0.0) ? (x / parentWidth) : 0.0f;
}

float Widget::PixelsToRatioY(float y) const
{
    float parentHeight = GetParentHeight();
    return (parentHeight > 0.0) ? (y / parentHeight) : 0.0f;
}

float Widget::RatioToPixelsX(float x) const
{
    return (GetParentWidth() * x);
}

float Widget::RatioToPixelsY(float y) const
{
    return (GetParentHeight() * y);
}

void Widget::PushScissor()
{
    mCachedScissorRect = mRect;

    glm::vec2 res = Renderer::Get()->GetActiveScreenResolution();
    mCachedParentScissorRect = (mParent != nullptr) ? mParent->mCachedScissorRect : Rect(0, 0, res.x, res.y);

    mCachedScissorRect.Clamp(mCachedParentScissorRect);
    SetScissor(mCachedScissorRect);
}

void Widget::PopScissor()
{
    SetScissor(mCachedParentScissorRect);
}

void Widget::SetScissor(Rect& area)
{
    GFX_SetScissor(int32_t(area.mX + 0.5f),
                   int32_t(area.mY + 0.5f),
                   int32_t(area.mWidth + 0.5f),
                   int32_t(area.mHeight + 0.5f));
}
