#include "RenderPassCache.h"
#include "VulkanContext.h"
#include "VulkanUtils.h"
#include "Image.h"

void RenderPassCache::Create()
{

}

void RenderPassCache::Destroy()
{
    Clear(false);
}

void RenderPassCache::Clear(bool framebuffersOnly)
{
    VkDevice device = GetVulkanDevice();

    for (auto pair : mFramebufferMap)
    {
        vkDestroyFramebuffer(device, pair.second, nullptr);
        pair.second = VK_NULL_HANDLE;
    }

    mFramebufferMap.clear();

    if (!framebuffersOnly)
    {
        for (auto pair : mRenderPassMap)
        {
            vkDestroyRenderPass(device, pair.second, nullptr);
            pair.second = VK_NULL_HANDLE;
        }

        mRenderPassMap.clear();
    }
}


VkRenderPass RenderPassCache::ResolveRenderPass(const RenderPassConfig& config)
{
    VkDevice device = GetVulkanDevice();

    auto it = mRenderPassMap.find(config);
    if (it != mRenderPassMap.end())
    {
        return it->second;
    }


    VkRenderPass renderPass = VK_NULL_HANDLE;

    // Existing render pass not found.

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
        if (config.mColorFormats[i] != VK_FORMAT_UNDEFINED)
        {
            attachments[numAttachments].flags = 0;
            attachments[numAttachments].format = config.mColorFormats[i];
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

    if (config.mDepthFormat != VK_FORMAT_UNDEFINED)
    {
        attachments[numAttachments].flags = 0;
        attachments[numAttachments].format = config.mDepthFormat;
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

#if VULKAN_VERBOSE_LOGGING
    LogDebug("Creating new RenderPass");
#endif

    if (vkCreateRenderPass(device, &ciRenderPass, nullptr, &renderPass) != VK_SUCCESS)
    {
        LogError("Failed to create render pass");
        OCT_ASSERT(0);
    }

    SetDebugObjectName(VK_OBJECT_TYPE_RENDER_PASS, (uint64_t)renderPass, config.mDebugName);

    mRenderPassMap[config] = renderPass;
    return renderPass;
}

VkFramebuffer RenderPassCache::ResolveFramebuffer(const FramebufferConfig& config)
{
    VkDevice device = GetVulkanDevice();

    auto it = mFramebufferMap.find(config);
    if (it != mFramebufferMap.end())
    {
        return it->second;
    }

    // Could not find a pre-existing framebuffer. Need to create a new one.
    VkFramebuffer framebuffer = VK_NULL_HANDLE;

    // Create Framebuffer
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
    ciFramebuffer.renderPass = config.mRenderPass;
    ciFramebuffer.attachmentCount = numAttachments;
    ciFramebuffer.pAttachments = attachmentViews;
    ciFramebuffer.width = config.mDepthImage ? config.mDepthImage->GetWidth() : config.mColorImages[0]->GetWidth();
    ciFramebuffer.height = config.mDepthImage ? config.mDepthImage->GetHeight() : config.mColorImages[0]->GetHeight();
    ciFramebuffer.layers = 1;

#if VULKAN_VERBOSE_LOGGING
    LogDebug("Creating new Framebuffer");
#endif

    if (vkCreateFramebuffer(device, &ciFramebuffer, nullptr, &framebuffer) != VK_SUCCESS)
    {
        LogError("Failed to create framebuffer.");
        OCT_ASSERT(0);
    }

    SetDebugObjectName(VK_OBJECT_TYPE_FRAMEBUFFER, (uint64_t)framebuffer, config.mDebugName);

    mFramebufferMap[config] = framebuffer;
    return framebuffer;
}


bool RenderPassConfig::operator==(const RenderPassConfig& other) const
{
    for (uint32_t i = 0; i < MAX_RENDER_TARGETS; ++i)
    {
        if (mColorFormats[i] != other.mColorFormats[i])
            return false;
    }

    if (mDepthFormat != other.mDepthFormat)
        return false;

    if (mLoadOp != other.mLoadOp)
        return false;

    if (mStoreOp != other.mStoreOp)
        return false;

    if (mDepthLoadOp != other.mDepthLoadOp)
        return false;

    if (mDepthStoreOp != other.mDepthStoreOp)
        return false;

    if (mPreLayout != other.mPreLayout)
        return false;

    if (mPostLayout != other.mPostLayout)
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
        result ^= sizeHasher((size_t)mColorFormats);
    }

    result ^= sizeHasher((size_t)mDepthFormat);
    result ^= sizeHasher((size_t)mLoadOp);
    result ^= sizeHasher((size_t)mStoreOp);
    result ^= sizeHasher((size_t)mDepthLoadOp);
    result ^= sizeHasher((size_t)mDepthStoreOp);
    result ^= sizeHasher((size_t)mPreLayout);
    result ^= sizeHasher((size_t)mPostLayout);

    return result;
}

bool FramebufferConfig::operator==(const FramebufferConfig& other) const
{
    for (uint32_t i = 0; i < MAX_RENDER_TARGETS; ++i)
    {
        uint64_t idThis = mColorImages[i] ? mColorImages[i]->GetId() : 0;
        uint64_t idOther = other.mColorImages[i] ? other.mColorImages[i]->GetId() : 0;

        if (idThis != idOther)
            return false;
    }

    {
        uint64_t idThis = mDepthImage ? mDepthImage->GetId() : 0;
        uint64_t idOther = other.mDepthImage ? other.mDepthImage->GetId() : 0;

        if (idThis != idOther)
            return false;
    }

    if (mRenderPass != other.mRenderPass)
        return false;

    return true;
}

size_t FramebufferConfig::Hash() const
{
    using std::size_t;
    using std::hash;

    hash<size_t> sizeHasher;
    size_t result = 0;

    for (uint32_t i = 0; i < MAX_RENDER_TARGETS; ++i)
    {
        uint64_t colorId = mColorImages[i] ? mColorImages[i]->GetId() : 0;
        result ^= sizeHasher((size_t)colorId);
    }

    uint64_t depthId = mDepthImage ? mDepthImage->GetId() : 0;
    result ^= sizeHasher((size_t)depthId);

    result ^= sizeHasher((size_t)mRenderPass);

    return result;
}
