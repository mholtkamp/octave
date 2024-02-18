#pragma once

#include "Image.h"
#include "VulkanConstants.h"
#include <vector>
#include <unordered_map>

struct RenderPassConfig
{
    // Config
    VkFormat mDepthFormat = VK_FORMAT_UNDEFINED;
    VkFormat mColorFormats[MAX_RENDER_TARGETS] = {};
    VkAttachmentLoadOp mLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    VkAttachmentStoreOp mStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    VkAttachmentLoadOp mDepthLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    VkAttachmentStoreOp mDepthStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    VkImageLayout mPreLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkImageLayout mPostLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    // Debug
    const char* mDebugName = "";

    bool operator==(const RenderPassConfig& other) const;
    size_t Hash() const;
};

struct FramebufferConfig
{
    // Config
    Image* mDepthImage = nullptr;
    Image* mColorImages[MAX_RENDER_TARGETS] = {};
    VkRenderPass mRenderPass;

    // Debug
    const char* mDebugName = "";

    bool operator==(const FramebufferConfig& other) const;
    size_t Hash() const;

};

struct RenderPassHash
{
    size_t operator()(const RenderPassConfig& config) const
    {
        return config.Hash();
    }
};

struct FramebufferHash
{
    size_t operator()(const FramebufferConfig& config) const
    {
        return config.Hash();
    }
};

class RenderPassCache
{
public:

    void Create();
    void Destroy();
    void Clear(bool framebuffersOnly);

    VkRenderPass ResolveRenderPass(const RenderPassConfig& config);
    VkFramebuffer ResolveFramebuffer(const FramebufferConfig& config);

protected:

    std::unordered_map<RenderPassConfig, VkRenderPass, RenderPassHash> mRenderPassMap;
    std::unordered_map<FramebufferConfig, VkFramebuffer, FramebufferHash> mFramebufferMap;

};
