#pragma once

#if API_VULKAN

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

#include "Vertex.h"
#include "Shader.h"

#include "Graphics/GraphicsTypes.h"
#include "Graphics/Vulkan/VulkanTypes.h"

struct VertexConfig
{
    VertexType mVertexType = VertexType::Vertex;
    std::string mVertexShaderPath;
};

class Pipeline
{

public:

    Pipeline();

    virtual ~Pipeline();

    void Create(const PipelineState& state, VkPipelineCache cache, VkSpecializationInfo* specInfo = nullptr);

    VkPipeline GetVkPipeline() const;

    void Bind(VkCommandBuffer commandBuffer);

    VkDescriptorSetLayout GetDescriptorSetLayout(uint32_t index);
    VkPipelineLayout GetPipelineLayout();

    bool IsComputePipeline() const;

protected:

    void Destroy();

    void CreateGraphicsPipeline(VkPipelineCache cache, VkSpecializationInfo* specInfo);
    void CreateComputePipeline(VkPipelineCache cache, VkSpecializationInfo* specInfo);

    void CreateDescriptorSetLayouts();
    void CreatePipelineLayout();

public:

    VkPipeline mPipeline = VK_NULL_HANDLE;
    VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
    std::vector<VkDescriptorSetLayout> mDescriptorSetLayouts;

    std::string mName;
    bool mComputePipeline = false;

    // Really only needed for debugging.
    PipelineState mState;
};

#endif // API_VULKAN