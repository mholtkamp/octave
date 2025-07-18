#include "Nodes/Widgets/ArrayWidget.h"

FORCE_LINK_DEF(ArrayWidget);
DEFINE_NODE(ArrayWidget, Widget);

static const char* sArrayOrientationStrings[] =
{
    "Vertical",
    "Horizontal"
};
static_assert(int32_t(ArrayOrientation::Count) == 2, "Need to update string conversion table");


void ArrayWidget::Create()
{
    Super::Create();
    SetName("Array");
}

void ArrayWidget::GatherProperties(std::vector<Property>& outProps)
{
    Widget::GatherProperties(outProps);

    SCOPED_CATEGORY("Array");

    outProps.push_back(Property(DatumType::Float, "Spacing", this, &mSpacing));
    outProps.push_back(Property(DatumType::Byte, "Orientation", this, &mOrientation, 1, nullptr, NULL_DATUM, int32_t(ArrayOrientation::Count), sArrayOrientationStrings));
    outProps.push_back(Property(DatumType::Bool, "Center", this, &mCenter));
}

void ArrayWidget::Tick(float deltaTime)
{
    Widget::Tick(deltaTime);
    TickCommon(deltaTime);
}

void ArrayWidget::EditorTick(float deltaTime)
{
    Widget::EditorTick(deltaTime);
    TickCommon(deltaTime);
}

void ArrayWidget::TickCommon(float deltaTime)
{
    float offset = 0.0f;
    bool vertical = (mOrientation == ArrayOrientation::Vertical);

    uint32_t numChilden = GetNumChildren();

    for (uint32_t i = 0; i < numChilden; ++i)
    {
        Widget* child = GetChildWidget(i);

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
