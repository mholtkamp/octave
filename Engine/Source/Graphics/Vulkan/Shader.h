#pragma once

#include "Graphics/Vulkan/VulkanConstants.h"

#include <vulkan/vulkan.h>
#include <spirv_cross/spirv_cross.hpp>

enum class ShaderStage : uint8_t
{
    Vertex,
    Fragment,
    Compute,

    Count
};

class Shader
{
public:

    Shader(const std::string& filePath, ShaderStage stage, const std::string& name);
    Shader(const char* data, uint32_t size, ShaderStage stage, const std::string& name);


protected:

    friend class DestroyQueue;
    ~Shader();

    void Create(const char* data, uint32_t size);

public:

    std::string mName;
    ShaderStage mStage;
    VkShaderModule mModule = VK_NULL_HANDLE;
    std::vector<VkDescriptorSetLayoutBinding> mDescriptorBindings[MAX_BOUND_DESCRIPTOR_SETS];
};

