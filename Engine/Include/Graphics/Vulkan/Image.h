#pragma once

#if API_VULKAN

#include "Allocator.h"
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

    VkImage Get() const;
    VkImageView GetView() const;
    VkSampler GetSampler() const;

    VkFormat GetFormat() const;

    void Update(const void* srcData);

    void Transition(VkImageLayout layout, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
    void GenerateMips();
    void Clear(glm::vec4 color);

private:

    friend class DestroyQueue;
    ~Image();

    VkImage mImage = VK_NULL_HANDLE;
    VkImageView mImageView = VK_NULL_HANDLE;
    VkSampler mSampler = VK_NULL_HANDLE;
    Allocation mMemory;

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
};

#endif