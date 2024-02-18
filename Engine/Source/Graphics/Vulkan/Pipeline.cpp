#if API_VULKAN

#include "Graphics/Vulkan/Pipeline.h"
#include "Graphics/Vulkan/VulkanUtils.h"
#include "Graphics/Vulkan/VulkanContext.h"

#include "Renderer.h"
#include "Utilities.h"
#include <vector>

using namespace std;

Pipeline::Pipeline()
{
}

Pipeline::~Pipeline()
{
    Destroy();
}

VkDescriptorSetLayout Pipeline::GetDescriptorSetLayout(uint32_t index)
{
    if (index > mDescriptorSetLayouts.size())
    {
        LogError("Accessing invalid descriptor set");
        OCT_ASSERT(0);
    }
    return mDescriptorSetLayouts[index];
}

void Pipeline::CreateGraphicsPipeline(VkPipelineCache cache, VkSpecializationInfo* specInfo)
{
    VulkanContext* context = GetVulkanContext();
    VkDevice device = context->GetDevice();

    // Limit features
    if (!context->HasFeatureWideLines())
    {
        mState.mLineWidth = 1.0f;
    }

    if (!context->HasFeatureFillModeNonSolid())
    {
        mState.mPolygonMode = VK_POLYGON_MODE_FILL;
    }

    VkShaderModule vertShaderModule = mState.mVertexShader->mModule;
    VkShaderModule fragShaderModule = VK_NULL_HANDLE;

    if (mState.mFragmentShader != nullptr)
    {
        fragShaderModule = mState.mFragmentShader->mModule;
    }

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";
    fragShaderStageInfo.pSpecializationInfo = specInfo;

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

    if (mState.mVertexType != VertexType::Max)
    {
        bindingDescriptions = GetVertexBindingDescription(mState.mVertexType);
        attributeDescriptions = GetVertexAttributeDescriptions(mState.mVertexType);

        vertexInputInfo.vertexBindingDescriptionCount = (uint32_t)bindingDescriptions.size();
        vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
    }
    else
    {
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
    }

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = mState.mPrimitiveTopology;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // These are dynamic now.
    VkViewport viewport = {};
    VkRect2D scissor = {};

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = mState.mRasterizerDiscard;
    rasterizer.polygonMode = mState.mPolygonMode;
    rasterizer.lineWidth = mState.mLineWidth;
    rasterizer.cullMode = mState.mCullMode;
    rasterizer.frontFace = mState.mFrontFace;
    rasterizer.depthBiasEnable = (mState.mDepthBias == 0.0) ? VK_FALSE : VK_TRUE;
    rasterizer.depthBiasConstantFactor = mState.mDepthBias;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = mState.mDepthTestEnabled;
    depthStencil.depthWriteEnable = mState.mDepthWriteEnabled;
    depthStencil.depthCompareOp = mState.mDepthCompareOp;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {};

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1; // static_cast<uint32_t>(MAX_RENDER_TARGETS);
    colorBlending.pAttachments = mState.mBlendStates;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    std::vector<VkDynamicState> dynStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    if (mState.mDynamicLineWidth && context->HasFeatureWideLines())
    {
        dynStates.push_back(VK_DYNAMIC_STATE_LINE_WIDTH);
    }

    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = (uint32_t)dynStates.size();
    dynamicState.pDynamicStates = dynStates.data();

    CreateDescriptorSetLayouts();
    CreatePipelineLayout();

    VkGraphicsPipelineCreateInfo ciPipeline = {};
    ciPipeline.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    ciPipeline.stageCount = (fragShaderModule == VK_NULL_HANDLE) ? 1 : 2;
    ciPipeline.pInputAssemblyState = &inputAssembly;
    ciPipeline.pViewportState = &viewportState;
    ciPipeline.pRasterizationState = &rasterizer;
    ciPipeline.pMultisampleState = &multisampling;
    ciPipeline.pDepthStencilState = &depthStencil;
    ciPipeline.pColorBlendState = &colorBlending;
    ciPipeline.pDynamicState = &dynamicState;
    ciPipeline.layout = mPipelineLayout;
    ciPipeline.renderPass = mState.mRenderPass;
    ciPipeline.subpass = 0;
    ciPipeline.basePipelineHandle = VK_NULL_HANDLE;
    ciPipeline.basePipelineIndex = -1;
    ciPipeline.pStages = shaderStages;
    ciPipeline.pVertexInputState = &vertexInputInfo;

    if (vkCreateGraphicsPipelines(GetVulkanDevice(),
        cache,
        1,
        &ciPipeline,
        nullptr,
        &mPipeline) != VK_SUCCESS)
    {
        LogError("Failed to create graphics pipeline");
        OCT_ASSERT(0);
    }

    SetDebugObjectName(VK_OBJECT_TYPE_PIPELINE, (uint64_t)mPipeline, mName.c_str());
}

void Pipeline::CreateComputePipeline(VkPipelineCache cache, VkSpecializationInfo* specInfo)
{
    VkDevice device = GetVulkanDevice();

    VkShaderModule computeShaderModule = mState.mComputeShader->mModule;

    VkPipelineShaderStageCreateInfo computeShaderStageInfo = {};
    computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computeShaderStageInfo.module = computeShaderModule;
    computeShaderStageInfo.pName = "main";

    CreateDescriptorSetLayouts();
    CreatePipelineLayout();

    VkComputePipelineCreateInfo ci = { };
    ci.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    ci.stage = computeShaderStageInfo;
    ci.layout = mPipelineLayout;
    ci.basePipelineHandle = VK_NULL_HANDLE;
    ci.basePipelineIndex = -1;

    if (vkCreateComputePipelines(device,
        cache,
        1,
        &ci,
        nullptr,
        &mPipeline) != VK_SUCCESS)
    {
        LogError("Failed to create compute pipeline");
        OCT_ASSERT(0);
    }

}

void Pipeline::Create(const PipelineState& state, VkPipelineCache cache, VkSpecializationInfo* specInfo)
{
    LogDebug("Creating new pipeline");

    // We probably don't need to store the state internally, but it might help for debugging.
    // If we have tons of pipelines and its consuming a lot of memory, then we can pass a ref of the state
    // into the CreateXPipeline() funcs.
    mState = state;

    // Ensure that a renderpass has been set before creating the pipeline.
    OCT_ASSERT(mState.mRenderPass != VK_NULL_HANDLE || mState.mComputeShader != nullptr);

    if (mComputePipeline)
    {
        CreateComputePipeline(cache, specInfo);
    }
    else
    {
        CreateGraphicsPipeline(cache, specInfo);
    }
}

void Pipeline::Destroy()
{
    VkDevice device = GetVulkanDevice();

    vkDestroyPipeline(device, mPipeline, nullptr);
    mPipeline = VK_NULL_HANDLE;

    vkDestroyPipelineLayout(device, mPipelineLayout, nullptr);
    mPipelineLayout = VK_NULL_HANDLE;

    for (VkDescriptorSetLayout layout : mDescriptorSetLayouts)
    {
        vkDestroyDescriptorSetLayout(device, layout, nullptr);
    }

    mDescriptorSetLayouts.clear();
}

VkPipeline Pipeline::GetVkPipeline() const
{
    return mPipeline;
}

void Pipeline::Bind(VkCommandBuffer commandBuffer)
{
    VkPipelineBindPoint bindPoint = mComputePipeline ? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS;
    vkCmdBindPipeline(commandBuffer, bindPoint, mPipeline);
}

VkPipelineLayout Pipeline::GetPipelineLayout()
{
    return mPipelineLayout;
}

bool Pipeline::IsComputePipeline() const
{
    return mComputePipeline;
}

void Pipeline::CreatePipelineLayout()
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(mDescriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = mDescriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = 0;

    if (vkCreatePipelineLayout(GetVulkanDevice(), &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
    {
        LogError("Failed to create pipeline layout!");
        OCT_ASSERT(0);
    }
}

void Pipeline::CreateDescriptorSetLayouts()
{
    // This can easily be changed.
    const uint32_t kMaxNumBindings = 32;

    std::vector<VkDescriptorSetLayoutBinding> setBindings[MAX_BOUND_DESCRIPTOR_SETS];

    auto gatherDescriptorBindings = [&](Shader* shader)
    {
        OCT_ASSERT(shader != nullptr);

        for (uint32_t i = 0; i < MAX_BOUND_DESCRIPTOR_SETS; ++i)
        {
            const std::vector<VkDescriptorSetLayoutBinding>& bindings = shader->mDescriptorBindings[i];

            for (uint32_t j = 0; j < bindings.size(); ++j)
            {
                const VkDescriptorSetLayoutBinding& binding = bindings[j];

                if (binding.binding >= kMaxNumBindings)
                {
                    LogError("Need to increase max bindings in Pipeline::CreateDescriptorSetLayouts()");
                    OCT_ASSERT(0);
                    continue;
                }

                VkDescriptorSetLayoutBinding* setBinding = nullptr;
                for (uint32_t k = 0; k < setBindings[i].size(); ++k)
                {
                    if (setBindings[i][k].binding == binding.binding)
                    {
                        // Found an existing binding
                        setBinding = &setBindings[i][k];
                        break;
                    }
                }

                if (setBinding != nullptr)
                {
                    OCT_ASSERT(setBinding->binding == binding.binding);
                    OCT_ASSERT(setBinding->descriptorCount == binding.descriptorCount);
                    OCT_ASSERT(setBinding->descriptorType == binding.descriptorType);
                }
                else
                {
                    setBindings[i].push_back(VkDescriptorSetLayoutBinding());
                    setBinding = &(setBindings[i].back());
                }

                OCT_ASSERT(setBinding != nullptr);

                // Add this binding
                setBinding->binding = binding.binding;
                setBinding->descriptorCount = binding.descriptorCount;
                setBinding->descriptorType = binding.descriptorType;
                setBinding->stageFlags |= binding.stageFlags;
            }
        }
    };

    if (mComputePipeline)
    {
        gatherDescriptorBindings(mState.mComputeShader);
    }
    else
    {
        gatherDescriptorBindings(mState.mVertexShader);
        gatherDescriptorBindings(mState.mFragmentShader);
    }

    for (uint32_t i = 0; i < MAX_BOUND_DESCRIPTOR_SETS; ++i)
    {
        const std::vector<VkDescriptorSetLayoutBinding>& bindings = setBindings[i];

        if (bindings.size() == 0)
            continue;

        VkDescriptorSetLayoutCreateInfo ciDescriptorSetLayout = {};
        ciDescriptorSetLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        ciDescriptorSetLayout.bindingCount = static_cast<uint32_t>(bindings.size());
        ciDescriptorSetLayout.pBindings = bindings.data();

        mDescriptorSetLayouts.push_back(VK_NULL_HANDLE);

        if (vkCreateDescriptorSetLayout(GetVulkanDevice(),
            &ciDescriptorSetLayout,
            nullptr,
            &(mDescriptorSetLayouts.back())) != VK_SUCCESS)
        {
            LogError("Failed to create descriptor set layout");
            OCT_ASSERT(0);
        }
    }
}

#endif
