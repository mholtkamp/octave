#if API_VULKAN

// TODO: Remove Renderer.h include after VulkanContext is in.
#include "Renderer.h"

#include "Graphics/Vulkan/DestroyQueue.h"
#include "Graphics/Vulkan/VulkanUtils.h"
#include "Graphics/Vulkan/VulkanContext.h"

#include "Graphics/Vulkan/Image.h"
#include "Graphics/Vulkan/Buffer.h"
#include "Graphics/Vulkan/DescriptorSet.h"

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

void DestroyQueue::Destroy(UniformBuffer* uniformBuffer)
{
    uint32_t frameIndex = GetFrameIndex();
    mUniformBuffers[frameIndex].push_back(uniformBuffer);
}

void DestroyQueue::Destroy(DescriptorSet* descriptorSet)
{
    uint32_t frameIndex = GetFrameIndex();
    mDescriptorSets[frameIndex].push_back(descriptorSet);
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

    for (uint32_t i = 0; i < mDescriptorSets[frameIndex].size(); ++i)
    {
        delete mDescriptorSets[frameIndex][i];
    }

    for (uint32_t i = 0; i < mUniformBuffers[frameIndex].size(); ++i)
    {
        delete mUniformBuffers[frameIndex][i];
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
    mDescriptorSets[frameIndex].clear();
    mUniformBuffers[frameIndex].clear();
    mImages[frameIndex].clear();
    mBuffers[frameIndex].clear();
}

void DestroyQueue::FlushAll()
{
    for (uint32_t i = 0; i < MAX_FRAMES; ++i)
    {
        Flush(i);
    }
}

#endif