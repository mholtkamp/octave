#pragma once

#if API_VULKAN

#include "Graphics/GraphicsConstants.h"
#include "Graphics/Vulkan/VulkanConstants.h"

#include <stdint.h>
#include <vulkan/vulkan.h>

class Image;
class Buffer;
class UniformBuffer;

enum class DescriptorType
{
    Uniform,
    Image,
    StorageBuffer,
    StorageImage,

    Count
};

struct DescriptorBinding
{
    DescriptorType mType = DescriptorType::Count;
    void* mObject = nullptr;
};

class DescriptorSet
{
public:

    DescriptorSet(VkDescriptorSetLayout layout);

    // Updates the current frame's descriptor.
    void UpdateImageDescriptor(int32_t binding, Image* image);
    void UpdateUniformDescriptor(int32_t binding, UniformBuffer* uniformBuffer);
    void UpdateStorageBufferDescriptor(int32_t binding, Buffer* storageBuffer);
    void UpdateStorageImageDescriptor(int32_t binding, Image* storageImage);

    void Bind(VkCommandBuffer cb, uint32_t index, VkPipelineLayout pipelineLayout);

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
};

#endif // API_VULKAN