#include "Widgets/Canvas.h"
#include "Renderer.h"

FORCE_LINK_DEF(Canvas);
DEFINE_FACTORY(Canvas, Widget);

Canvas::Canvas()
{
    mUseScissor = true;
}

Canvas::~Canvas()
{

}
