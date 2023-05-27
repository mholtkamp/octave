#pragma once

#include "Maths.h"

struct Line
{
    Line(glm::vec3 start, glm::vec3 end, glm::vec4 color, float lifetime)
    {
        mStart = start;
        mEnd = end;
        mColor = color;
        mLifetime = lifetime;
    }

    Line()
    {

    }

    bool operator==(const Line& other) const
    {
        // Don't compare lifetime
        return mStart == other.mStart &&
            mEnd == other.mEnd && 
            mColor == other.mColor;
    }

    bool operator!=(const Line& other) const
    {
        return !operator==(other);
    }

    glm::vec3 mStart = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 mEnd = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec4 mColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    float mLifetime = 0.0f;
};
