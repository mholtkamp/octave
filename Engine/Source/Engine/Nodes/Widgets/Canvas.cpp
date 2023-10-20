#include "Nodes/Widgets/Canvas.h"
#include "Renderer.h"

FORCE_LINK_DEF(Canvas);
DEFINE_NODE(Canvas, Widget);

Canvas::Canvas()
{
    mUseScissor = true;
}

Canvas::~Canvas()
{

}
