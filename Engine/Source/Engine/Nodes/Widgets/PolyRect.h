#pragma once

#include "Nodes/Widgets/Poly.h"

class PolyRect : public Poly
{
public:

    DECLARE_WIDGET(PolyRect, Poly);

    PolyRect();
    virtual ~PolyRect();

    virtual void Update() override;
};
