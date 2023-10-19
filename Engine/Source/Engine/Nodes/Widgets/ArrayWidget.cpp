#include "Nodes/Widgets/ArrayWidget.h"

FORCE_LINK_DEF(ArrayWidget);
DEFINE_WIDGET(ArrayWidget, Widget);

static const char* sArrayOrientationStrings[] =
{
    "Vertical",
    "Horizontal"
};
static_assert(int32_t(ArrayOrientation::Count) == 2, "Need to update string conversion table");


ArrayWidget::ArrayWidget()
{

}

void ArrayWidget::GatherProperties(std::vector<Property>& outProps)
{
    Widget::GatherProperties(outProps);

    outProps.push_back(Property(DatumType::Float, "Spacing", this, &mSpacing));
    outProps.push_back(Property(DatumType::Byte, "Orientation", this, &mOrientation, 1, nullptr, 0, int32_t(ArrayOrientation::Count), sArrayOrientationStrings));
    outProps.push_back(Property(DatumType::Bool, "Center", this, &mCenter));
}

void ArrayWidget::Update()
{
    Widget::Update();

    float offset = 0.0f;
    bool vertical = (mOrientation == ArrayOrientation::Vertical);

    uint32_t numChilden = GetNumChildren();

    for (uint32_t i = 0; i < numChilden; ++i)
    {
        Widget* child = GetChild(i);

        if (vertical)
        {
            float x = 0.0f;

            if (mCenter)
            {
                x = GetWidth() / 2.0f - child->GetWidth() / 2.0f;
            }

            child->SetPosition(x, offset);
            offset += child->GetHeight() + mSpacing;
        }
        else
        {
            float y = 0.0f;
            if (mCenter)
            {
                y = GetHeight() / 2.0f - child->GetHeight() / 2.0f;
            }

            child->SetPosition(offset, y);
            offset += child->GetWidth() + mSpacing;
        }
    }
}
