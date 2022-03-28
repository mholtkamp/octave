#pragma once

#if API_VULKAN

#include "Graphics/GraphicsConstants.h"
#include "Allocator.h"

#include <vulkan/vulkan.h>
#include <vector>

class Image;
class Buffer;
class DescriptorSet;

class DestroyQueue
{
public:
    DestroyQueue();
    ~DestroyQueue();

    void Destroy(Image* image);
    void Destroy(Buffer* buffer);
    void Destroy(UniformBuffer* uniformBuffer);
    void Destroy(DescriptorSet* descriptorSet);
    void Destroy(VkCommandBuffer commandBuffer);

    void Flush(uint32_t frameIndex);
    void FlushAll();

protected:

    std::vector<Image*> mImages[MAX_FRAMES];
    std::vector<Buffer*> mBuffers[MAX_FRAMES];
    std::vector<UniformBuffer*> mUniformBuffers[MAX_FRAMES];
    std::vector<DescriptorSet*> mDescriptorSets[MAX_FRAMES];
    std::vector<VkCommandBuffer> mCommandBuffers[MAX_FRAMES];
};

#endif
