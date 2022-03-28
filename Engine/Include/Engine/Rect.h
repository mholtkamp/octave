#pragma once

class Rect
{
public:

    Rect();
    Rect(float x, float y, float width, float height);

    float mX;
    float mY;
    float mWidth;
    float mHeight;

    bool ContainsPoint(float x, float y) const;
    bool OverlapsRect(const Rect& otherRect) const;
    void Clamp(const Rect& otherRect);

    float Top() const { return mY; }
    float Bottom() const { return mY + mHeight; }
    float Left() const { return mX; }
    float Right() const { return mX + mWidth; }
};