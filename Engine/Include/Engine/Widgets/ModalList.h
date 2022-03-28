#pragma once

#include "Widgets/VerticalList.h"

class ModalList : public VerticalList
{
public:

    DECLARE_FACTORY(ModalList, Widget);

    ModalList();

    virtual void Update() override;
};