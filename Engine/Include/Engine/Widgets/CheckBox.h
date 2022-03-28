#pragma once

#include "Widgets/Selector.h"

class CheckBox : public Selector
{
public:

    DECLARE_FACTORY(CheckBox, Widget);

    CheckBox();
    ~CheckBox();

    bool IsChecked() const;
    void SetChecked(bool checked);
};