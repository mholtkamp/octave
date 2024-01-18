#include "DescriptorPool.h"

#include "VulkanUtils.h"
#include "VulkanContext.h"

// Referenced: https://vkguide.dev/docs/extra-chapter/abstracting_descriptors/

void DescriptorPool::Create()
{

}

void DescriptorPool::Destroy()
{
    VkDevice device = GetVulkanDevice();

    for (uint32_t i = 0; i < mFreePools.size(); ++i)
    {
        vkDestroyDescriptorPool(device, mFreePools[i], nullptr);
    }

    for (uint32_t i = 0; i < mUsedPools.size(); ++i)
    {
        vkDestroyDescriptorPool(device, mUsedPools[i], nullptr);
    }
}

void DescriptorPool::Reset()
{
    VkDevice device = GetVulkanDevice();
    for (uint32_t i = 0; i < mUsedPools.size(); ++i)
    {
        vkResetDescriptorPool(device, mUsedPools[i], 0);
        mFreePools.push_back(mUsedPools[i]);
    }

    mUsedPools.clear();

    mCurrentPool = VK_NULL_HANDLE;
}

VkDescriptorSet DescriptorPool::Allocate(VkDescriptorSetLayout layout)
{
    VkDevice device = GetVulkanDevice();
    VkDescriptorSet retSet = VK_NULL_HANDLE;

    if (mCurrentPool == VK_NULL_HANDLE)
    {
        mCurrentPool = GrabPool();
        mUsedPools.push_back(mCurrentPool);
    }

    VkDescriptorSetLayout layouts[] = { layout };
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = mCurrentPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = layouts;

    if (vkAllocateDescriptorSets(device, &allocInfo, &retSet) != VK_SUCCESS)
    {
        // Failed to allocate a set, need to grab next pool.
        mCurrentPool = GrabPool();
        mUsedPools.push_back(mCurrentPool);

        allocInfo.descriptorPool = mCurrentPool;
        if (vkAllocateDescriptorSets(device, &allocInfo, &retSet) != VK_SUCCESS)
        {
                LogError("Failed to allocate descriptor set");
                OCT_ASSERT(0);
                return;
        }
    }
}

VkDescriptorPool DescriptorPool::CreatePool()
{
    LogDebug("CreatePool()");

    VkDevice device = GetVulkanDevice();

    VkDescriptorPoolSize poolSizes[4] = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    poolSizes[0].descriptorCount = MAX_UNIFORM_BUFFER_DESCRIPTORS;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = MAX_SAMPLER_DESCRIPTORS;
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[2].descriptorCount = MAX_STORAGE_BUFFER_DESCRIPTORS;
    poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    poolSizes[3].descriptorCount = MAX_STORAGE_IMAGE_DESCRIPTORS;

    VkDescriptorPoolCreateInfo ciPool = {};
    ciPool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    ciPool.poolSizeCount = 4;
    ciPool.pPoolSizes = poolSizes;
    ciPool.maxSets = MAX_DESCRIPTOR_SETS;
    ciPool.flags = 0;

    VkDescriptorPool retPool;
    if (vkCreateDescriptorPool(device, &ciPool, nullptr, &retPool) != VK_SUCCESS)
    {
        LogError("Failed to create descriptor pool");
        OCT_ASSERT(0);
    }

    return retPool;
}

VkDescriptorPool DescriptorPool::GrabPool()
{
    if (mFreePools.size() > 0)
    {
        VkDescriptorPool pool = mFreePools.back();
        mFreePools.pop_back();
        return pool;
    }
    else
    {
        VkDescriptorPool newPool = CreatePool();
        return newPool;
    }
}
