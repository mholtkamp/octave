#if API_VULKAN

// TODO: Remove Renderer.h include after VulkanContext is in.
#include "Renderer.h"

#include "Graphics/Vulkan/DestroyQueue.h"
#include "Graphics/Vulkan/VulkanUtils.h"
#include "Graphics/Vulkan/VulkanContext.h"

#include "Graphics/Vulkan/Image.h"
#include "Graphics/Vulkan/Buffer.h"
#include "Graphics/Vulkan/DescriptorSet.h"
#include "Graphics/Vulkan/Pipeline.h"

DestroyQueue::DestroyQueue()
{

}

DestroyQueue::~DestroyQueue()
{

}

void DestroyQueue::Destroy(Image* image)
{
    uint32_t frameIndex = GetFrameIndex();
    mImages[frameIndex].push_back(image);
}

void DestroyQueue::Destroy(Buffer* buffer)
{
    uint32_t frameIndex = GetFrameIndex();
    mBuffers[frameIndex].push_back(buffer);
}

void DestroyQueue::Destroy(MultiBuffer* multiBuffer)
{
    uint32_t frameIndex = GetFrameIndex();
    mMultiBuffers[frameIndex].push_back(multiBuffer);
}

void DestroyQueue::Destroy(Shader* shader)
{
    uint32_t frameIndex = GetFrameIndex();
    mShaders[frameIndex].push_back(shader);
}

void DestroyQueue::Destroy(Pipeline* pipeline)
{
    uint32_t frameIndex = GetFrameIndex();
    mPipelines[frameIndex].push_back(pipeline);
}

void DestroyQueue::Destroy(VkCommandBuffer commandBuffer)
{
    uint32_t frameIndex = GetFrameIndex();
    mCommandBuffers[frameIndex].push_back(commandBuffer);
}

void DestroyQueue::Flush(uint32_t frameIndex)
{
    VkDevice device = GetVulkanDevice();
    VkCommandPool commandPool = GetVulkanContext()->GetCommandPool();

    for (uint32_t i = 0; i < mCommandBuffers[frameIndex].size(); ++i)
    {
        vkFreeCommandBuffers(device, commandPool, 1, &mCommandBuffers[frameIndex][i]);
    }

    for (uint32_t i = 0; i < mPipelines[frameIndex].size(); ++i)
    {
        delete mPipelines[frameIndex][i];
    }

    for (uint32_t i = 0; i < mShaders[frameIndex].size(); ++i)
    {
        delete mShaders[frameIndex][i];
    }

    for (uint32_t i = 0; i < mMultiBuffers[frameIndex].size(); ++i)
    {
        delete mMultiBuffers[frameIndex][i];
    }

    for (uint32_t i = 0; i < mImages[frameIndex].size(); ++i)
    {
        delete mImages[frameIndex][i];
    }

    for (uint32_t i = 0; i < mBuffers[frameIndex].size(); ++i)
    {
        delete mBuffers[frameIndex][i];
    }

    mCommandBuffers[frameIndex].clear();
    mMultiBuffers[frameIndex].clear();
    mImages[frameIndex].clear();
    mBuffers[frameIndex].clear();
    mShaders[frameIndex].clear();
    mPipelines[frameIndex].clear();
}

void DestroyQueue::FlushAll()
{
    for (uint32_t i = 0; i < MAX_FRAMES; ++i)
    {
        Flush(i);
    }
}

#endif