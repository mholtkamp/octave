#include "DescriptorLayoutCache.h"

#include "VulkanUtils.h"
#include "VulkanContext.h"

// Referenced: https://vkguide.dev/docs/extra-chapter/abstracting_descriptors/

void DescriptorLayoutCache::Create()
{

}

void DescriptorLayoutCache::Destroy()
{
    VkDevice device = GetVulkanDevice();

    for (auto pair : mLayoutMap)
    {
        vkDestroyDescriptorSetLayout(device, pair.second, nullptr);
    }

    mLayoutMap.clear();
}

VkDescriptorSetLayout DescriptorLayoutCache::CreateLayout(VkDescriptorSetLayoutCreateInfo* info)
{
    VkDevice device = GetVulkanDevice();

    DescriptorLayoutInfo layoutInfo;
    layoutInfo.mBindings.reserve(info->bindingCount);
    bool isSorted = true;
    int32_t lastBinding = -1;

    // Copy from the vk createinfo struct into our custom struct
    for (int32_t i = 0; i < (int32_t)info->bindingCount; ++i)
    {
        layoutInfo.mBindings.push_back(info->pBindings[i]);

        if ((int32_t)info->pBindings[i].binding > lastBinding)
            lastBinding = info->pBindings[i].binding;
        else
            isSorted = false;
    }

    // Sort if not already sorted
    if (!isSorted)
    {
        std::sort(layoutInfo.mBindings.begin(),
            layoutInfo.mBindings.end(),
            [](VkDescriptorSetLayoutBinding& a, VkDescriptorSetLayoutBinding& b)
            {
                return a.binding < b.binding;
            }
        );
    }

    // Attempt to grab a pre-existing layout
    auto it = mLayoutMap.find(layoutInfo);
    if (it != mLayoutMap.end())
    {
        return it->second;
    }
    else
    {
        // Pre-existing layout not found, need to create it.

#if VULKAN_VERBOSE_LOGGING
        LogDebug("Creating new descriptor set layout");
#endif
        VkDescriptorSetLayout layout;
        vkCreateDescriptorSetLayout(device, info, nullptr, &layout);

        mLayoutMap[layoutInfo] = layout;
        return layout;
    }
}

bool DescriptorLayoutCache::DescriptorLayoutInfo::operator==(const DescriptorLayoutInfo& other) const
{
    if (other.mBindings.size() != mBindings.size())
        return false;

    for (uint32_t i = 0; i < mBindings.size(); ++i)
    {
        if (other.mBindings[i].binding != mBindings[i].binding)
            return false;

        if (other.mBindings[i].descriptorType != mBindings[i].descriptorType)
            return false;

        if (other.mBindings[i].descriptorCount != mBindings[i].descriptorCount)
            return false;

        if (other.mBindings[i].stageFlags != mBindings[i].stageFlags)
            return false;
    }

    return true;
}

size_t DescriptorLayoutCache::DescriptorLayoutInfo::Hash() const
{
    using std::size_t;
    using std::hash;

    hash<size_t> sizeHasher;

    size_t result = sizeHasher(mBindings.size());

    for (const VkDescriptorSetLayoutBinding& b : mBindings)
    {
        size_t bindingHash = b.binding | b.descriptorType << 8 | b.descriptorCount << 16 | b.stageFlags << 24;
    
        result ^= sizeHasher(bindingHash);
    }

    return result;
}

