#include "Graphics/GraphicsUtils.h"

const char* GetRenderPassName(RenderPassId id)
{
    const char* name = "Render Pass";

    switch (id)
    {
    case RenderPassId::Shadows: name = "Shadows"; break;
    case RenderPassId::Forward: name = "Forward"; break;
    case RenderPassId::Selected: name = "Selected"; break;
    case RenderPassId::Ui: name = "UI"; break;
    case RenderPassId::Clear: name = "Clear"; break;
    case RenderPassId::HitCheck: name = "Hit Check"; break;
    default: break;
    }

    return name;
}