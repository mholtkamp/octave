#pragma once

#include "Widgets/Widget.h"

class Canvas : public Widget
{
public:

    DECLARE_FACTORY(Canvas, Widget);

    Canvas();
    virtual ~Canvas();
};