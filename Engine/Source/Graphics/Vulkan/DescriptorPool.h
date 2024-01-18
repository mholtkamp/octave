#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class DescriptorPool
{
public:

    void Create();
    void Destroy();

    void Reset();
    VkDescriptorSet Allocate(VkDescriptorSetLayout layout, const char* name = nullptr);

protected:

    VkDescriptorPool CreatePool();
    VkDescriptorPool GrabPool();

    VkDescriptorPool mCurrentPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorPool> mUsedPools;
    std::vector<VkDescriptorPool> mFreePools;
};
