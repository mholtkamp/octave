#pragma once

#if API_VULKAN

#include "Graphics/GraphicsConstants.h"
#include "Allocator.h"

#include <vulkan/vulkan.h>
#include <vector>

class Image;
class Buffer;
class DescriptorSet;
class Shader;
class Pipeline;

class DestroyQueue
{
public:
    DestroyQueue();
    ~DestroyQueue();

    void Destroy(Image* image);
    void Destroy(Buffer* buffer);
    void Destroy(MultiBuffer* multiBuffer);
    void Destroy(Shader* shader);
    void Destroy(Pipeline* pipeline);
    void Destroy(VkCommandBuffer commandBuffer);

    void Flush(uint32_t frameIndex);
    void FlushAll();

protected:

    std::vector<Image*> mImages[MAX_FRAMES];
    std::vector<Buffer*> mBuffers[MAX_FRAMES];
    std::vector<MultiBuffer*> mMultiBuffers[MAX_FRAMES];
    std::vector<Shader*> mShaders[MAX_FRAMES];
    std::vector<Pipeline*> mPipelines[MAX_FRAMES];
    std::vector<VkCommandBuffer> mCommandBuffers[MAX_FRAMES];
};

#endif
