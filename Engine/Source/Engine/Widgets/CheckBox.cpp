#include "Widgets/CheckBox.h"

FORCE_LINK_DEF(CheckBox);
DEFINE_WIDGET(CheckBox, Selector);

CheckBox::CheckBox()
{
    AddSelection("");
    AddSelection("X");

    mTextPaddingRatio.x = 0.25f;
    SetDimensions(24, 24);
}

CheckBox::~CheckBox()
{
}

bool CheckBox::IsChecked() const
{
    return mSelectionIndex != 0;
}

void CheckBox::SetChecked(bool checked)
{
    SetSelectionIndex(checked ? 1 : 0);
}
