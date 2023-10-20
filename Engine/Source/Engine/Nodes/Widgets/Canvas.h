#pragma once

#include "Nodes/Widgets/Widget.h"

class Canvas : public Widget
{
public:

    DECLARE_NODE(Canvas, Widget);

    Canvas();
    virtual ~Canvas();
};