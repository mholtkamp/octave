#include "Widgets/ModalList.h"
#include "InputDevices.h"
#include "Renderer.h"

#include <algorithm>
#include <glm/glm.hpp>

FORCE_LINK_DEF(ModalList);
DEFINE_WIDGET(ModalList, VerticalList);

ModalList::ModalList()
{

}

void ModalList::Update()
{
    VerticalList::Update();

    if (Renderer::Get()->GetModalWidget() == this &&
        IsMouseButtonJustUp(MOUSE_LEFT) &&
        !ContainsMouse())
    {
        // Close the list
        Renderer::Get()->SetModalWidget(nullptr);
    }
}
