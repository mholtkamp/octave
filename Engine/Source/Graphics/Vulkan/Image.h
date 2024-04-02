#pragma once

#if API_VULKAN

#include "Graphics/Vulkan/VramAllocator.h"
#include "Maths.h"

#include <vulkan/vulkan.h>

class DestroyQueue;

struct ImageDesc
{
    uint32_t mWidth = 4;
    uint32_t mHeight = 4;
    VkFormat mFormat = VK_FORMAT_R8G8B8A8_UNORM;
    VkImageUsageFlags mUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    uint32_t mMipLevels = 1;
    uint32_t mLayers = 1;
};

struct SamplerDesc
{
    VkFilter mMagFilter = VK_FILTER_LINEAR;
    VkFilter mMinFilter = VK_FILTER_LINEAR;
    VkSamplerAddressMode mAddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkBorderColor mBorderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    float mMaxAnisotropy = 1.0;
    bool mAnisotropyEnable = false;
};

class Image
{
public:
    Image(ImageDesc imageDesc, SamplerDesc samplerDesc, const char* debugObjectName);

    // External image? Kind of a hack needed for using swapchain images with RenderPassCache
    Image(VkImage image, VkImageView imageView, VkSampler sampler, VkFormat format, uint32_t width, uint32_t height);

    VkImage Get() const;
    VkImageView GetView() const;
    VkSampler GetSampler() const;

    VkFormat GetFormat() const;
    uint32_t GetWidth() const;
    uint32_t GetHeight() const;

    void Update(const void* srcData);

    void Transition(VkImageLayout layout, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
    void GenerateMips();
    void Clear(glm::vec4 color);

    uint64_t GetId() const;

private:

    friend class DestroyQueue;
    ~Image();

    uint64_t mId = 0;
    VkImage mImage = VK_NULL_HANDLE;
    VkImageView mImageView = VK_NULL_HANDLE;
    VkSampler mSampler = VK_NULL_HANDLE;
    VramAllocation mMemory;

    uint32_t mWidth = 0;
    uint32_t mHeight = 0;
    VkFormat mFormat = VK_FORMAT_UNDEFINED;
    VkImageUsageFlags mUsage = {};
    uint32_t mMipLevels = 0;
    uint32_t mLayers = 0;

    VkFilter mMagFilter = VK_FILTER_LINEAR;
    VkFilter mMinFilter = VK_FILTER_LINEAR;
    VkSamplerAddressMode mAddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkBorderColor mBorderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    float mMaxAnisotropy = 1.0;
    bool mAnisotropyEnable = false;

    VkImageLayout mLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

    bool mExternal = false;
};

#endif