#pragma once

#include "Widgets/Widget.h"

class Canvas : public Widget
{
public:

    DECLARE_WIDGET(Canvas, Widget);

    Canvas();
    virtual ~Canvas();
};