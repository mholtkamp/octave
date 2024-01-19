#pragma once

#if API_VULKAN

#include "Graphics/GraphicsConstants.h"
#include "Graphics/Vulkan/VulkanConstants.h"

#include "Graphics/Vulkan/Image.h"

#include <stdint.h>
#include <vulkan/vulkan.h>

class Buffer;
class UniformBuffer;
struct UniformBlock;

struct DescriptorBinding
{
    VkDescriptorType mType = VK_DESCRIPTOR_TYPE_MAX_ENUM;
    void* mObject = nullptr;
    uint32_t mOffset = 0;
    uint32_t mSize = 0; // Only filled for Uniform Blocks
    uint32_t mCount = 1;
    std::vector<Image*> mImageArray;

    int32_t mBinding = -1;
};

class DescriptorSet
{
public:

    static DescriptorSet Begin(const char* name = "");

    // Updates the current frame's descriptor.
    DescriptorSet& WriteImage(int32_t binding, Image* image);
    DescriptorSet& WriteImageArray(int32_t binding, const std::vector<Image*>& imageArray);
    DescriptorSet& WriteUniformBuffer(int32_t binding, UniformBuffer* uniformBuffer);
    DescriptorSet& WriteUniformBuffer(int32_t binding, const UniformBlock& block);
    DescriptorSet& WriteStorageBuffer(int32_t binding, Buffer* storageBuffer);
    DescriptorSet& WriteStorageImage(int32_t binding, Image* storageImage);

    DescriptorSet& Build();

    void Bind(VkCommandBuffer cb, uint32_t index);

    VkDescriptorSet Get() const;
    VkDescriptorSetLayout GetLayout() const;

private:

    void UpdateDescriptors();

    std::vector<DescriptorBinding> mBindings;
    //std::vector<VkDescriptorImageInfo> mImages;
    //std::vector<VkDescriptorBufferInfo> mBuffers;

    VkDescriptorSet mDescriptorSet = VK_NULL_HANDLE;
    VkDescriptorSetLayout mLayout = VK_NULL_HANDLE;
    uint32_t mFrameBuilt = UINT_MAX;
    const char* mName = nullptr;
};

#endif // API_VULKAN