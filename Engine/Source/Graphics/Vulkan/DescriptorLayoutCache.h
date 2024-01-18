#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>

class DescriptorLayoutCache
{
public:

    void Create();
    void Destroy();

    VkDescriptorSetLayout CreateLayout(VkDescriptorSetLayoutCreateInfo* info);

    struct DescriptorLayoutInfo
    {
        std::vector<VkDescriptorSetLayoutBinding> mBindings;

        bool operator==(const DescriptorLayoutInfo& other) const;

        size_t Hash() const;
    };

protected:

    struct DescriptorLayoutHash
    {
        size_t operator()(const DescriptorLayoutInfo& k) const
        {
            return k.Hash();
        }
    };

    std::unordered_map<DescriptorLayoutInfo, VkDescriptorSetLayout, DescriptorLayoutHash> mLayoutMap;

};
