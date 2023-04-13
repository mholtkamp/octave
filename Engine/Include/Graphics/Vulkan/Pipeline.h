#pragma once

#if API_VULKAN

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

#include "Vertex.h"

#include "Graphics/GraphicsTypes.h"

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

    void Create(VkRenderPass renderPass);
    void Destroy();

    void SetVertexConfig(VertexType vertexType, const std::string& path);
    void AddVertexConfig(VertexType vertexType, const std::string& path);
    void SetMeshVertexConfigs(
        const std::string& staticPath,
        const std::string& staticColorPath,
        const std::string& skinnedPath,
        const std::string& particlePath);
    void ClearVertexConfigs();

    void SetFragmentShader(const std::string& path);

    void BindPipeline(VkCommandBuffer commandBuffer, VertexType vertexType);

    VkDescriptorSetLayout GetDescriptorSetLayout(uint32_t index = 0);

    VkPipelineLayout GetPipelineLayout();

    PipelineId GetId() const;

    bool IsComputePipeline() const;

protected:

    void CreateGraphicsPipeline();
    void CreateComputePipeline();

    void PushSet();
    void AddLayoutBinding(VkDescriptorType type, VkShaderStageFlags stageFlags);

    VkShaderModule CreateShaderModule(const std::vector<char>& code);

    virtual void PopulateLayoutBindings();
    void CreateDescriptorSetLayouts();
    void CreatePipelineLayout();

    void AddOpaqueBlendAttachmentState();
    void AddMixBlendAttachmentState();
    void AddAdditiveBlendAttachmentState();

    std::vector<VkPipeline> mPipelines;
    VkPipelineLayout mPipelineLayout;
    std::vector<VkDescriptorSetLayout> mDescriptorSetLayouts;
    
public:

    std::string mName;
    PipelineId mPipelineId;
    VkRenderPass mRenderpass;
    uint32_t mSubpass;
    bool mComputePipeline;

    // Shader stages
    std::vector<VertexConfig> mVertexConfigs;
    std::string mFragmentShaderPath;
    std::string mComputeShaderPath;

    // Viewport
    uint32_t mViewportWidth;
    uint32_t mViewportHeight;

    // Rasterizer stage
    VkBool32 mRasterizerDiscard;
    VkPrimitiveTopology mPrimitiveTopology;
    VkPolygonMode mPolygonMode;
    float mLineWidth;
    VkCullModeFlags mCullMode;
    VkFrontFace mFrontFace;
    float mDepthBias;
    bool mDynamicLineWidth;

    // Depth Stencil state
    VkBool32 mDepthTestEnabled;
    VkBool32 mDepthWriteEnabled;
    VkCompareOp mDepthCompareOp;

    // Color Blend State
    std::vector<VkPipelineColorBlendAttachmentState> mBlendAttachments;

    std::vector<std::vector<VkDescriptorSetLayoutBinding> > mLayoutBindings;
};

#endif // API_VULKAN