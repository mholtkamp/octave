#pragma once

#include "Graphics/Vulkan/VulkanConstants.h"

#include <vulkan/vulkan.h>


class Shader
{
public:

    Shader(const char* filePath, const char* name = "");
    Shader(const char* data, uint32_t size, const char* name = "");


protected:

    friend class DestroyQueue;
    ~Shader();

    void Create(const char* data, uint32_t size);

public:

    std::string mName;
    VkShaderModule mModule = VK_NULL_HANDLE;
    VkDescriptorSetLayout mDescriptorSetLayouts[MAX_BOUND_DESCRIPTOR_SETS] = {};
};

