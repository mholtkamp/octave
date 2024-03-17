#include "BlurPass.h"
#include "Graphics/Vulkan/VulkanUtils.h"
#include "Graphics/Vulkan/VulkanContext.h"

void BlurPass::Create()
{
    mName = "Blur";

    VulkanContext* context = GetVulkanContext();
    uint32_t width = context->GetSceneWidth();
    uint32_t height = context->GetSceneHeight();

    ImageDesc image;
    image.mFormat = context->GetSceneColorFormat();
    image.mWidth = width;
    image.mHeight = height;
    image.mLayers = 1;
    image.mMipLevels = 1;
    image.mUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    SamplerDesc sampler;
    sampler.mAddressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

    mXBlurImage = new Image(image, sampler, "BlurX Image");
    mXBlurImage->Transition(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void BlurPass::Destroy()
{
    GetDestroyQueue()->Destroy(mXBlurImage);

    if (mXBlurLowResImage != nullptr)
    {
        GetDestroyQueue()->Destroy(mXBlurLowResImage);
    }

    if (mYBlurLowResImage != nullptr)
    {
        GetDestroyQueue()->Destroy(mYBlurLowResImage);
    }
}

void BlurPass::Render(Image* input, Image* output)
{
    VulkanContext* context = GetVulkanContext();
    VkCommandBuffer cb = GetCommandBuffer();

    int32_t downsampleFactor = glm::clamp<int32_t>(mDownsampleFactor, 1, 10);
    bool downsample = mDownsampleFactor > 1;

    // Determine if we need to resize low res render targets
    if (downsample)
    {
        int32_t lowWidth = input->GetWidth() / mDownsampleFactor;
        int32_t lowHeight = input->GetHeight() / mDownsampleFactor;

        if (mXBlurLowResImage == nullptr ||
            mYBlurLowResImage == nullptr ||
            mXBlurLowResImage->GetWidth() != lowWidth || 
            mXBlurLowResImage->GetHeight() != lowHeight ||
            mYBlurLowResImage->GetWidth() != lowWidth ||
            mYBlurLowResImage->GetHeight() != lowHeight)
        {
            LogDebug("Reallocating low res blur render targets.");

            if (mXBlurLowResImage != nullptr)
            {
                GetDestroyQueue()->Destroy(mXBlurLowResImage);
            }

            if (mYBlurLowResImage != nullptr)
            {
                GetDestroyQueue()->Destroy(mYBlurLowResImage);
            }

            ImageDesc image;
            image.mFormat = context->GetSceneColorFormat();
            image.mWidth = lowWidth;
            image.mHeight = lowHeight;
            image.mLayers = 1;
            image.mMipLevels = 1;
            image.mUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

            SamplerDesc sampler;
            sampler.mAddressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

            mXBlurLowResImage = new Image(image, sampler, "BlurX Low Res Image");
            mXBlurLowResImage->Transition(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            mYBlurLowResImage = new Image(image, sampler, "BlurY Low Res Image");
            mYBlurLowResImage->Transition(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
    }

    mUniforms.mInputWidth = input->GetWidth();
    mUniforms.mInputHeight = input->GetHeight();

    // Populate gaussian weights
    mUniforms.mNumSamples = glm::clamp<int32_t>(mUniforms.mNumSamples, 1, BLUR_MAX_SAMPLES);
    const float E = 2.71828182846f;
    float stdDev = mUniforms.mSigmaRatio * mUniforms.mBlurSize;
    const float stdDev2 = stdDev * stdDev;
    for (int32_t i = 0; i < mUniforms.mNumSamples; ++i)
    {
        float offset = (i / (mUniforms.mNumSamples - 1.0f) - 0.5f) * mUniforms.mBlurSize;
        float gauss = (1.0f / sqrtf(2.0f * PI * stdDev2)) * powf(E, -((offset * offset) / (2 * stdDev2)));
        mUniforms.mGaussianWeights[i].x = gauss;
    }

    // Pass 1 - Horizontal Blur
    {
        mUniforms.mHorizontal = true;

        Image* outImage = downsample ? mXBlurLowResImage : mXBlurImage;

        // Set render target
        RenderPassSetup rpSetup;
        rpSetup.mColorImages[0] = outImage;
        rpSetup.mLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        rpSetup.mStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        context->BeginVkRenderPass(rpSetup, true);

        context->SetViewport(0, 0, outImage->GetWidth(), outImage->GetHeight(), true, false);
        context->SetScissor(0, 0, outImage->GetWidth(), outImage->GetHeight(), true, false);

        // Bind vertex/index buffers for full screen quad.
        context->BindFullscreenVertexBuffer(cb);

        // Set vertex + fragment shader
        context->SetVertexShader("ScreenRect.vert");
        context->SetFragmentShader("Blur.frag");

        // Commit graphics pipeline
        context->CommitPipeline();

        // Bind descriptor set
        UniformBlock block = WriteUniformBlock(&mUniforms, sizeof(BlurUniforms));

        DescriptorSet::Begin("Blur X DS")
            .WriteUniformBuffer(0, block)
            .WriteImage(1, input)
            .Build()
            .Bind(cb, 1);

        // Draw 
        vkCmdDraw(cb, 4, 1, 0, 0);

        // End render pass
        context->EndVkRenderPass();
    }

    // Pass 2 - Vertical Blur
    {
        mUniforms.mHorizontal = false;

        if (downsample)
        {
            mUniforms.mInputWidth = mXBlurLowResImage->GetWidth();
            mUniforms.mInputHeight = mXBlurLowResImage->GetHeight();
        }

        Image* outImage = downsample ? mYBlurLowResImage : output;

        // Set render target
        RenderPassSetup rpSetup;
        rpSetup.mColorImages[0] = outImage;
        rpSetup.mLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        rpSetup.mStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        context->BeginVkRenderPass(rpSetup, true);

        context->SetViewport(0, 0, outImage->GetWidth(), outImage->GetHeight(), true, false);
        context->SetScissor(0, 0, outImage->GetWidth(), outImage->GetHeight(), true, false);

        // Bind vertex/index buffers for full screen quad.
        context->BindFullscreenVertexBuffer(cb);

        // Set vertex + fragment shader
        context->SetVertexShader("ScreenRect.vert");
        context->SetFragmentShader("Blur.frag");

        // Commit graphics pipeline
        context->CommitPipeline();

        // Bind descriptor set
        UniformBlock block = WriteUniformBlock(&mUniforms, sizeof(BlurUniforms));

        DescriptorSet::Begin("Blur Y DS")
            .WriteUniformBuffer(0, block)
            .WriteImage(1, downsample ? mXBlurLowResImage : mXBlurImage)
            .Build()
            .Bind(cb, 1);

        // Draw 
        vkCmdDraw(cb, 4, 1, 0, 0);

        // End render pass
        context->EndVkRenderPass();
    }

    // Pass 3 - Upsample (if needed).
    if (downsample)
    {
        // Set render target
        RenderPassSetup rpSetup;
        rpSetup.mColorImages[0] = output;
        rpSetup.mLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        rpSetup.mStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        context->BeginVkRenderPass(rpSetup, true);

        context->SetViewport(0, 0, output->GetWidth(), output->GetHeight(), true, false);
        context->SetScissor(0, 0, output->GetWidth(), output->GetHeight(), true, false);

        // Bind vertex/index buffers for full screen quad.
        context->BindFullscreenVertexBuffer(cb);

        // Set vertex + fragment shader
        context->SetVertexShader("ScreenRect.vert");
        context->SetFragmentShader("Resample.frag");

        // Commit graphics pipeline
        context->CommitPipeline();

        // Bind descriptor set
        DescriptorSet::Begin("Blur Upsample DS")
            .WriteImage(0, mYBlurLowResImage)
            .Build()
            .Bind(cb, 1);

        // Draw 
        vkCmdDraw(cb, 4, 1, 0, 0);

        // End render pass
        context->EndVkRenderPass();
    }
}

void BlurPass::GatherProperties(std::vector<Property>& props)
{
    PostProcessPass::GatherProperties(props);
    props.push_back(Property(DatumType::Integer, "Blur Samples", nullptr, &mUniforms.mNumSamples));
    props.push_back(Property(DatumType::Float, "Blur Size", nullptr, &mUniforms.mBlurSize));
    props.push_back(Property(DatumType::Float, "Blur Sigma Ratio", nullptr, &mUniforms.mSigmaRatio));
    props.push_back(Property(DatumType::Bool, "Blur Box", nullptr, &mUniforms.mBoxBlur)); // Stored as integer, but it is simply a flag, so not zero = true
    props.push_back(Property(DatumType::Integer, "Blur Downsample Factor", nullptr, &mDownsampleFactor));
}
