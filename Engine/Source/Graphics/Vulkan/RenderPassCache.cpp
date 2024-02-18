#include "RenderPassCache.h"
#include "VulkanContext.h"
#include "VulkanUtils.h"
#include "Image.h"

void RenderPassCache::Create()
{

}

void RenderPassCache::Destroy()
{
    Clear();
}

void RenderPassCache::Clear()
{
    VkDevice device = GetVulkanDevice();

    for (auto pair : mRenderPassMap)
    {
        vkDestroyFramebuffer(device, pair.second.mFramebuffer, nullptr);
        pair.second.mFramebuffer = VK_NULL_HANDLE;
        
        vkDestroyRenderPass(device, pair.second.mRenderPass, nullptr);
        pair.second.mRenderPass = VK_NULL_HANDLE;
    }

    mRenderPassMap.clear();
}


RenderPass RenderPassCache::CreateRenderPass(const RenderPassConfig& config)
{
    VkDevice device = GetVulkanDevice();
    RenderPass renderPass;

    auto it = mRenderPassMap.find(config);
    if (it != mRenderPassMap.end())
    {
        return it->second;
    }

    // Existing render pass not found.

    // Create Renderpass
    {
        // I was getting SYNC-HAZARD-READ-AFTER-WRITE validation warnings 
        // (and was getting incorrect rendering results on the RG552). I thought
        // render passes were supposed to automatically handle the transition
        // to the final output (which it seems to do) but unless I add this external 
        // subpass dependency, I get the read after write warnings.
        VkSubpassDependency extDependency = {};
        extDependency.srcSubpass = 0;
        extDependency.dstSubpass = VK_SUBPASS_EXTERNAL;
        extDependency.srcStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
        extDependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        extDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        extDependency.dstAccessMask = 0;
        extDependency.dependencyFlags = 0;

        VkAttachmentDescription attachments[MAX_RENDER_TARGETS + 1] = {};
        VkAttachmentReference colorRefs[MAX_RENDER_TARGETS] = {};

        VkAttachmentReference depthRef = {};
        uint32_t numAttachments = 0;
        uint32_t numColorAttachments = 0;
        uint32_t numDepthAttachments = 0;

        // Color attachments first
        for (uint32_t i = 0; i < MAX_RENDER_TARGETS; ++i)
        {
            if (config.mColorImages[i] != nullptr)
            {
                attachments[numAttachments].flags = 0;
                attachments[numAttachments].format = config.mColorImages[i]->GetFormat();
                attachments[numAttachments].samples = VK_SAMPLE_COUNT_1_BIT;
                attachments[numAttachments].loadOp = config.mLoadOp;
                attachments[numAttachments].storeOp = config.mStoreOp;
                attachments[numAttachments].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachments[numAttachments].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                attachments[numAttachments].initialLayout = config.mPreLayout;
                attachments[numAttachments].finalLayout = config.mPostLayout;

                colorRefs[numAttachments].attachment = numAttachments;
                colorRefs[numAttachments].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                numAttachments++;
                numColorAttachments++;
            }
        }

        if (config.mDepthImage != nullptr)
        {
            attachments[numAttachments].flags = 0;
            attachments[numAttachments].format = config.mDepthImage->GetFormat();
            attachments[numAttachments].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[numAttachments].loadOp = config.mDepthLoadOp;
            attachments[numAttachments].storeOp = config.mDepthStoreOp;
            attachments[numAttachments].stencilLoadOp = config.mDepthLoadOp;
            attachments[numAttachments].stencilStoreOp = config.mDepthStoreOp;
            attachments[numAttachments].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            attachments[numAttachments].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            depthRef.attachment = numAttachments;
            depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            numAttachments++;
            numDepthAttachments++;
        }

        VkSubpassDescription subpass = {};
        subpass.flags = 0;
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.inputAttachmentCount = 0;
        subpass.pInputAttachments = nullptr;
        subpass.colorAttachmentCount = numColorAttachments;
        subpass.pColorAttachments = colorRefs;
        subpass.pResolveAttachments = nullptr;
        subpass.pDepthStencilAttachment = (numDepthAttachments > 0) ? &depthRef : nullptr;
        subpass.preserveAttachmentCount = 0;
        subpass.pPreserveAttachments = nullptr;


        VkRenderPassCreateInfo ciRenderPass = {};
        ciRenderPass.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        ciRenderPass.pNext = nullptr;
        ciRenderPass.flags = 0;
        ciRenderPass.attachmentCount = numAttachments;
        ciRenderPass.pAttachments = attachments;
        ciRenderPass.subpassCount = 1;
        ciRenderPass.pSubpasses = &subpass;
        ciRenderPass.dependencyCount = 1;
        ciRenderPass.pDependencies = &extDependency;

        if (vkCreateRenderPass(device, &ciRenderPass, nullptr, &renderPass.mRenderPass) != VK_SUCCESS)
        {
            LogError("Failed to create forward render pass");
            OCT_ASSERT(0);
        }

        SetDebugObjectName(VK_OBJECT_TYPE_RENDER_PASS, (uint64_t)renderPass.mRenderPass, config.mDebugName);
    }

    // Create Framebuffer
    {
        VkImageView attachmentViews[MAX_RENDER_TARGETS + 1] = {};
        uint32_t numAttachments = 0;

        // Color attachments first
        for (uint32_t i = 0; i < MAX_RENDER_TARGETS; ++i)
        {
            if (config.mColorImages[i] != nullptr)
            {
                attachmentViews[numAttachments] = config.mColorImages[i]->GetView();
                numAttachments++;
            }
        }

        if (config.mDepthImage != nullptr)
        {
            attachmentViews[numAttachments] = config.mDepthImage->GetView();
            numAttachments++;
        }

        VkFramebufferCreateInfo ciFramebuffer = {};
        ciFramebuffer.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        ciFramebuffer.renderPass = renderPass.mRenderPass;
        ciFramebuffer.attachmentCount = numAttachments;
        ciFramebuffer.pAttachments = attachmentViews;
        ciFramebuffer.width = config.mDepthImage ? config.mDepthImage->GetWidth() : config.mColorImages[0]->GetWidth();
        ciFramebuffer.height = config.mDepthImage ? config.mDepthImage->GetHeight() : config.mColorImages[0]->GetHeight();
        ciFramebuffer.layers = 1;

        LogDebug("Creating new RenderPass");

        if (vkCreateFramebuffer(device, &ciFramebuffer, nullptr, &renderPass.mFramebuffer) != VK_SUCCESS)
        {
            LogError("Failed to create framebuffer.");
            OCT_ASSERT(0);
        }

        SetDebugObjectName(VK_OBJECT_TYPE_FRAMEBUFFER, (uint64_t)renderPass.mFramebuffer, config.mDebugName);
    }

    mRenderPassMap[config] = renderPass;
    return renderPass;
}

bool RenderPassConfig::operator==(const RenderPassConfig& other) const
{
    for (uint32_t i = 0; i < MAX_RENDER_TARGETS; ++i)
    {
        if (mColorImages[i] != other.mColorImages[i])
            return false;
    }

    if (mDepthImage != other.mDepthImage)
        return false;

    if (mLoadOp != other.mLoadOp)
        return false;

    if (mStoreOp != other.mStoreOp)
        return false;

    return true;
}

size_t RenderPassConfig::Hash() const
{
    using std::size_t;
    using std::hash;

    hash<size_t> sizeHasher;
    size_t result = 0;

    for (uint32_t i = 0; i < MAX_RENDER_TARGETS; ++i)
    {
        result ^= sizeHasher((size_t)mColorImages[i]);
    }

    result ^= sizeHasher((size_t)mDepthImage);
    result ^= sizeHasher((size_t)mLoadOp);
    result ^= sizeHasher((size_t)mStoreOp);
    result ^= sizeHasher((size_t)mDepthLoadOp);
    result ^= sizeHasher((size_t)mDepthStoreOp);
    result ^= sizeHasher((size_t)mPreLayout);
    result ^= sizeHasher((size_t)mPostLayout);

    return result;
}

