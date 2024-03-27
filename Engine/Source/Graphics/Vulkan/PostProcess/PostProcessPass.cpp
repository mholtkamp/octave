#include "PostProcessPass.h"

#include "Graphics/Vulkan/VulkanContext.h"

void PostProcessPass::Create()
{

}

void PostProcessPass::Destroy()
{

}

void PostProcessPass::Resize()
{

}

void PostProcessPass::Render(Image* input, Image* output)
{
    VulkanContext* context = GetVulkanContext();
    context->SetViewport(0, 0, output->GetWidth(), output->GetHeight(), true, false);
    context->SetScissor(0, 0, output->GetWidth(), output->GetHeight(), true, false);
}

const std::string& PostProcessPass::GetName() const
{
    return mName;
}

bool PostProcessPass::IsEnabled() const
{
    return mEnabled;
}

void PostProcessPass::SetEnabled(bool enable)
{
    mEnabled = enable;
}

void PostProcessPass::GatherProperties(std::vector<Property>& props)
{
    props.push_back(Property(DatumType::Bool, mName, nullptr, &mEnabled));
}
