#include "Rect.h"
#include <glm/glm.hpp>

Rect::Rect() :
    mX(0.0f),
    mY(0.0f),
    mWidth(1.0f),
    mHeight(1.0f)
{

}

Rect::Rect(float x, float y, float width, float height) :
    mX(x),
    mY(y),
    mWidth(width),
    mHeight(height)
{
}

bool Rect::ContainsPoint(float x, float y) const
{
    if (x < mX ||
        x > mX + mWidth ||
        y < mY ||
        y > mY + mHeight)
    {
        return false;
    }

    return true;
}

bool Rect::OverlapsRect(const Rect& otherRect) const
{
    if (otherRect.Right() < Left() ||
        otherRect.Left() > Right() ||
        otherRect.Bottom() < Top() ||
        otherRect.Top() > Bottom())
    {
        return false;
    }

    return true;
}

void Rect::Clamp(const Rect& otherRect)
{
    if (otherRect.Left() > Left())
    {
        mWidth -= otherRect.mX - mX;
        mX = otherRect.mX;
        
        mWidth = glm::max(mWidth, 0.0f);
    }

    if (otherRect.Top() > Top())
    {
        mHeight -= otherRect.mY - mY;
        mY = otherRect.mY;

        mHeight = glm::max(mHeight, 0.0f);
    }

    if (otherRect.Right() < Right())
    {
        mWidth -= Right() - otherRect.Right();
        mWidth = glm::max(mWidth, 0.0f);
    }

    if (otherRect.Bottom() < Bottom())
    {
        mHeight -= Bottom() - otherRect.Bottom();
        mHeight = glm::max(mHeight, 0.0f);
    }
}
