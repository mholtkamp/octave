#pragma once

#include "Nodes/Widgets/Selector.h"

class CheckBox : public Selector
{
public:

    DECLARE_NODE(CheckBox, Selector);

    CheckBox();
    ~CheckBox();

    bool IsChecked() const;
    void SetChecked(bool checked);
};