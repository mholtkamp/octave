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

enum class DescriptorType
{
    Uniform,
    Image,
    ImageArray,
    StorageBuffer,
    StorageImage,

    Count
};

struct DescriptorBinding
{
    DescriptorType mType = DescriptorType::Count;
    void* mObject = nullptr;
    uint32_t mOffset = 0;
    uint32_t mSize = 0; // Only filled for Uniform Blocks
    std::vector<Image*> mImageArray;
};

class DescriptorSet
{
public:

    DescriptorSet(VkDescriptorSetLayout layout, const char* name = "");

    // Updates the current frame's descriptor.
    void UpdateImageDescriptor(int32_t binding, Image* image);
    void UpdateImageArrayDescriptor(int32_t binding, const std::vector<Image*>& imageArray);
    void UpdateUniformDescriptor(int32_t binding, UniformBuffer* uniformBuffer);
    void UpdateUniformDescriptor(int32_t binding, const UniformBlock& block);
    void UpdateStorageBufferDescriptor(int32_t binding, Buffer* storageBuffer);
    void UpdateStorageImageDescriptor(int32_t binding, Image* storageImage);

    void Bind(VkCommandBuffer cb, uint32_t index, VkPipelineLayout pipelineLayout, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);

    VkDescriptorSet Get();
    VkDescriptorSet Get(uint32_t frameIndex);

private:

    friend class DestroyQueue;
    ~DescriptorSet();

    void MarkDirty();
    void RefreshBindings(uint32_t frameIndex);

    DescriptorBinding mBindings[MAX_DESCRIPTORS_PER_SET] = { };

    VkDescriptorSet mDescriptorSets[MAX_FRAMES] = { };
    bool mDirty[MAX_FRAMES] = { };
    uint32_t mLastFrameBound = UINT_MAX;
    std::string mName;
};

#endif // API_VULKAN