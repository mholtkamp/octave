#include "TonemapPass.h"

#include "Graphics/Vulkan/VulkanContext.h"
#include "Graphics/Vulkan/RayTracer.h"
#include "Renderer.h"

void TonemapPass::Create()
{
    mName = "Tonemap";
}

void TonemapPass::Destroy()
{

}

void TonemapPass::Render(Image* input, Image* output)
{
    VulkanContext* context = GetVulkanContext();
    VkCommandBuffer cb = GetCommandBuffer();

    Image* pathTraceImage = context->IsRayTracingSupported() ? context->GetRayTracer()->GetPathTraceImage() : input;

    // Setup - Prepare uniforms
    mUniforms.mPathTracingEnabled = Renderer::Get()->IsPathTracingEnabled();

    // Set render target
    RenderPassSetup rpSetup;
    rpSetup.mColorImages[0] = context->GetSwapchainImage();
    rpSetup.mLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    rpSetup.mStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    rpSetup.mPreLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    rpSetup.mPostLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    context->BeginVkRenderPass(rpSetup, true);

    // Bind vertex/index buffers for full screen quad.
    context->BindFullscreenVertexBuffer(cb);

    // Set vertex + fragment shader
    context->SetVertexShader("ScreenRect.vert");
    context->SetFragmentShader("Tonemap.frag");

    // Commit graphics pipeline
    context->CommitPipeline();

    // Bind descriptor set
    UniformBlock block = WriteUniformBlock(&mUniforms, sizeof(TonemapUniforms));

    DescriptorSet::Begin("Tonemap DS")
        .WriteUniformBuffer(0, block)
        .WriteImage(1, input)
        .WriteImage(2, pathTraceImage)
        .Build()
        .Bind(cb, 1);

    // Draw 
    vkCmdDraw(cb, 4, 1, 0, 0);

    // End render pass
    context->EndVkRenderPass();
}

bool TonemapPass::IsEnabled() const
{
    // This pass needs to be always enabled.
    return true;
}

void TonemapPass::GatherProperties(std::vector<Property>& props)
{
    PostProcessPass::GatherProperties(props);
}
