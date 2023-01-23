#pragma once

#include "Widgets/Widget.h"
#include "Assets/Texture.h"
#include "Vertex.h"
#include "AssetRef.h"

#include "glm/glm.hpp"

enum class ArrayOrientation : uint8_t
{
    Vertical,
    Horizontal,

    Count
};

class ArrayWidget : public Widget
{
public:

    DECLARE_WIDGET(ArrayWidget, Widget);

    ArrayWidget();

    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual void Update() override;

protected:

    //static bool HandlePropChange(Datum* datum, const void* newValue);

    float mSpacing = 0.0f;
    ArrayOrientation mOrientation = ArrayOrientation::Vertical;
    bool mCenter = false;
};
