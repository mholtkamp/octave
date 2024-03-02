#include "PostProcessChain.h"
#include "VulkanUtils.h"
#include "VulkanContext.h"
#include "Renderer.h"

PostProcessChain::PostProcessChain()
{
    mPasses[(uint32_t)PostProcessPassId::Blur] = &mBlurPass;
    mPasses[(uint32_t)PostProcessPassId::Tonemap] = &mTonemapPass;
}

void PostProcessChain::Create()
{
    Image* sceneColor = GetVulkanContext()->GetSceneColorImage();

    ImageDesc image;
    image.mFormat = sceneColor->GetFormat();
    image.mWidth = sceneColor->GetWidth();
    image.mHeight = sceneColor->GetHeight();
    image.mLayers = 1;
    image.mMipLevels = 1;
    image.mUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    SamplerDesc sampler;
    sampler.mAddressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

    mAuxSceneColor = new Image(image, sampler, "Aux Scene Color");
    mAuxSceneColor->Transition(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    for (uint32_t i = 0; i < uint32_t(PostProcessPassId::Count); ++i)
    {
        mPasses[i]->Create();
    }
}

void PostProcessChain::Destroy()
{
    GetDestroyQueue()->Destroy(mAuxSceneColor);

    for (uint32_t i = 0; i < uint32_t(PostProcessPassId::Count); ++i)
    {
        mPasses[i]->Destroy();
    }
}

void PostProcessChain::Resize()
{
    Destroy();
    Create();
}

void PostProcessChain::Render()
{
    VulkanContext* context = GetVulkanContext();
    Image* sceneColor = context->GetSceneColorImage();
    Renderer* renderer = Renderer::Get();

    Image* input = sceneColor;
    Image* output = mAuxSceneColor;

    context->BeginGpuTimestamp("PostProcess");

    for (uint32_t i = 0; i < uint32_t(PostProcessPassId::Count); ++i)
    {
        if (mPasses[i]->IsEnabled())
        {
            BeginDebugLabel(mPasses[i]->GetName().c_str());
            mPasses[i]->Render(input, output);
            EndDebugLabel();

            // Swap input/output
            Image* newInput = output;
            output = input;
            input = newInput;
        }
    }

    context->EndGpuTimestamp("PostProcess");

}

void PostProcessChain::GatherProperties(std::vector<Property>& props)
{
    for (uint32_t i = 0; i < uint32_t(PostProcessPassId::Count); ++i)
    {
        mPasses[i]->GatherProperties(props);
    }
}
