#pragma once

#include "Nodes/Widgets/VerticalList.h"

class ModalList : public VerticalList
{
public:

    DECLARE_NODE(ModalList, VerticalList);

    ModalList();

    virtual void Tick(float deltaTime) override;
};