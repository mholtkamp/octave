#pragma once

#include "Nodes/Widgets/Widget.h"
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

    DECLARE_NODE(ArrayWidget, Widget);

    ArrayWidget();

    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual void Tick(float deltaTime) override;
    virtual void EditorTick(float deltaTime) override;

protected:

    void TickCommon(float deltaTime);

    //static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

    float mSpacing = 0.0f;
    ArrayOrientation mOrientation = ArrayOrientation::Vertical;
    bool mCenter = false;
};
