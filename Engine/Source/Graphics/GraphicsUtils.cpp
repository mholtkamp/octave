#include "Graphics/GraphicsUtils.h"

const char* GetRenderPassName(RenderPassId id)
{
    const char* name = "Render Pass";

    switch (id)
    {
    case RenderPassId::Shadows: name = "Shadows"; break;
    case RenderPassId::Forward: name = "Forward"; break;
    case RenderPassId::PostProcess: name = "Post Process"; break;
    case RenderPassId::Ui: name = "UI"; break;
    case RenderPassId::HitCheck: name = "Hit Check"; break;
    default: break;
    }

    return name;
}