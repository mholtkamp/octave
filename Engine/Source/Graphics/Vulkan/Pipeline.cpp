#if API_VULKAN

#include "Graphics/Vulkan/Pipeline.h"
#include "Graphics/Vulkan/VulkanUtils.h"
#include "Graphics/Vulkan/VulkanContext.h"

#include "Renderer.h"
#include "Utilities.h"
#include <vector>

using namespace std;

Pipeline::Pipeline() :
    mPipelineLayout(VK_NULL_HANDLE),
    mName("Pipeline"),
    mPipelineId(PipelineId::Count),
    mRenderpass(VK_NULL_HANDLE),
    mSubpass(0),
    mComputePipeline(false),
    mFragmentShaderPath("Shaders/bin/Forward.frag"),
    mViewportWidth(0),
    mViewportHeight(0),
    mRasterizerDiscard(VK_FALSE),
    mPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST),
    mPolygonMode(VK_POLYGON_MODE_FILL),
    mLineWidth(1.0f),
    mCullMode(VK_CULL_MODE_BACK_BIT),
    mFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE),
    mDepthBias(0.0f),
    mDynamicLineWidth(false),
    mDepthTestEnabled(VK_TRUE),
    mDepthWriteEnabled(VK_TRUE),
    mDepthCompareOp(VK_COMPARE_OP_LESS)
{
    AddOpaqueBlendAttachmentState();
}

Pipeline::~Pipeline()
{

}

void Pipeline::SetVertexConfig(VertexType vertexType, const std::string& path)
{
    ClearVertexConfigs();
    AddVertexConfig(vertexType, path);
}

void Pipeline::AddVertexConfig(VertexType vertexType, const std::string& path)
{
    VertexConfig config = {};
    config.mVertexType = vertexType;
    config.mVertexShaderPath = path;
    mVertexConfigs.push_back(config);
}

void Pipeline::SetMeshVertexConfigs(
    const std::string& staticPath,
    const std::string& staticColorPath,
    const std::string& skinnedPath,
    const std::string& particlePath)
{
    ClearVertexConfigs();
    AddVertexConfig(VertexType::Vertex, staticPath);
    AddVertexConfig(VertexType::VertexColor, staticColorPath);
    AddVertexConfig(VertexType::VertexSkinned, skinnedPath);
    AddVertexConfig(VertexType::VertexParticle, particlePath);
}

void Pipeline::ClearVertexConfigs()
{
    mVertexConfigs.clear();
}

void Pipeline::SetFragmentShader(const std::string& path)
{
    mFragmentShaderPath = path;
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

void Pipeline::CreateGraphicsPipeline()
{
    VulkanContext* context = GetVulkanContext();
    VkDevice device = context->GetDevice();

    VkShaderModule fragShaderModule = VK_NULL_HANDLE;

    if (mFragmentShaderPath != "")
    {
        vector<char> fragShaderCode = ReadFile(mFragmentShaderPath);
        fragShaderModule = CreateShaderModule(fragShaderCode);
    }

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = mPrimitiveTopology;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkExtent2D& swapchainExtent = context->GetSwapchainExtent();

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)((mViewportWidth == 0) ? swapchainExtent.width : mViewportWidth);
    viewport.height = (float)((mViewportHeight == 0) ? swapchainExtent.height : mViewportHeight);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent.width = (mViewportWidth == 0) ? swapchainExtent.width : mViewportWidth;
    scissor.extent.height = (mViewportHeight == 0) ? swapchainExtent.height : mViewportHeight;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = mRasterizerDiscard;
    rasterizer.polygonMode = mPolygonMode;
    rasterizer.lineWidth = mLineWidth;
    rasterizer.cullMode = mCullMode;
    rasterizer.frontFace = mFrontFace;
    rasterizer.depthBiasEnable = (mDepthBias == 0.0) ? VK_FALSE : VK_TRUE;
    rasterizer.depthBiasConstantFactor = mDepthBias;
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
    depthStencil.depthTestEnable = mDepthTestEnabled;
    depthStencil.depthWriteEnable = mDepthWriteEnabled;
    depthStencil.depthCompareOp = mDepthCompareOp;
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
    colorBlending.attachmentCount = static_cast<uint32_t>(mBlendAttachments.size());
    colorBlending.pAttachments = mBlendAttachments.data();
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    std::vector<VkDynamicState> dynStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    if (mDynamicLineWidth)
    {
        dynStates.push_back(VK_DYNAMIC_STATE_LINE_WIDTH);
    }

    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = (uint32_t)dynStates.size();
    dynamicState.pDynamicStates = dynStates.data();

    CreatePipelineLayout();

    VkGraphicsPipelineCreateInfo ciPipeline = {};
    ciPipeline.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    ciPipeline.stageCount = (mFragmentShaderPath == "") ? 1 : 2;
    ciPipeline.pInputAssemblyState = &inputAssembly;
    ciPipeline.pViewportState = &viewportState;
    ciPipeline.pRasterizationState = &rasterizer;
    ciPipeline.pMultisampleState = &multisampling;
    ciPipeline.pDepthStencilState = &depthStencil;
    ciPipeline.pColorBlendState = &colorBlending;
    ciPipeline.pDynamicState = &dynamicState;
    ciPipeline.layout = mPipelineLayout;
    ciPipeline.renderPass = (mRenderpass == VK_NULL_HANDLE) ? context->GetForwardRenderPass() : mRenderpass;
    ciPipeline.subpass = mSubpass;
    ciPipeline.basePipelineHandle = VK_NULL_HANDLE;
    ciPipeline.basePipelineIndex = -1;

    // Must have at least one vertex config, even if not using vertex buffers.
    OCT_ASSERT(mVertexConfigs.size() > 0);
    mPipelines.resize((uint32_t) VertexType::Max, VK_NULL_HANDLE);

    for (uint32_t i = 0; i < mVertexConfigs.size(); ++i)
    {
        const VertexConfig vertexConfig = mVertexConfigs[i];
        vector<char> vertShaderCode = ReadFile(vertexConfig.mVertexShaderPath);

        VkShaderModule vertShaderModule;
        vertShaderModule = CreateShaderModule(vertShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        VkVertexInputBindingDescription bindingDescription;
        std::vector<VkVertexInputAttributeDescription> attributeDescription;

        if (vertexConfig.mVertexType != VertexType::Max)
        {
            bindingDescription = GetVertexBindingDescription(vertexConfig.mVertexType);
            attributeDescription = GetVertexAttributeDescriptions(vertexConfig.mVertexType);

            vertexInputInfo.vertexBindingDescriptionCount = 1;
            vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
            vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
            vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();
        }
        else
        {
            vertexInputInfo.vertexBindingDescriptionCount = 0;
            vertexInputInfo.vertexAttributeDescriptionCount = 0;
        }

        ciPipeline.pStages = shaderStages;
        ciPipeline.pVertexInputState = &vertexInputInfo;
        uint32_t pipelineIndex = vertexConfig.mVertexType == VertexType::Max ? 0 : uint32_t(vertexConfig.mVertexType);

        if (vkCreateGraphicsPipelines(GetVulkanDevice(),
            VK_NULL_HANDLE,
            1,
            &ciPipeline,
            nullptr,
            &mPipelines[pipelineIndex]) != VK_SUCCESS)
        {
            LogError("Failed to create graphics pipeline");
            OCT_ASSERT(0);
        }

        SetDebugObjectName(VK_OBJECT_TYPE_PIPELINE, (uint64_t)mPipelines[pipelineIndex], mName.c_str());

        vkDestroyShaderModule(device, vertShaderModule, nullptr);
    }

    if (mFragmentShaderPath != "")
    {
        vkDestroyShaderModule(device, fragShaderModule, nullptr);
    }
}

void Pipeline::CreateComputePipeline()
{
    VkDevice device = GetVulkanDevice();

    vector<char> computeShaderCode = ReadFile(mComputeShaderPath);

    VkShaderModule computeShaderModule;

    computeShaderModule = CreateShaderModule(computeShaderCode);

    VkPipelineShaderStageCreateInfo computeShaderStageInfo = {};
    computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computeShaderStageInfo.module = computeShaderModule;
    computeShaderStageInfo.pName = "main";

    CreatePipelineLayout();

    VkComputePipelineCreateInfo ci = { };
    ci.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    ci.stage = computeShaderStageInfo;
    ci.layout = mPipelineLayout;
    ci.basePipelineHandle = VK_NULL_HANDLE;
    ci.basePipelineIndex = -1;

    mPipelines.resize(1, VK_NULL_HANDLE);

    if (vkCreateComputePipelines(device,
        VK_NULL_HANDLE,
        1,
        &ci,
        nullptr,
        &mPipelines[0]) != VK_SUCCESS)
    {
        LogError("Failed to create compute pipeline");
        OCT_ASSERT(0);
    }

    vkDestroyShaderModule(device, computeShaderModule, nullptr);
}

void Pipeline::Create(VkRenderPass renderPass)
{
    // Ensure that a renderpass has been set before creating the pipeline.
    mRenderpass = renderPass;
    OCT_ASSERT(mRenderpass != VK_NULL_HANDLE || mComputePipeline);
    OCT_ASSERT(mPipelineId != PipelineId::Count);

    PopulateLayoutBindings();
    CreateDescriptorSetLayouts();

    if (mComputePipeline)
    {
        CreateComputePipeline();
    }
    else
    {
        CreateGraphicsPipeline();
    }
}


void Pipeline::Destroy()
{
    VkDevice device = GetVulkanDevice();

    for (uint32_t i = 0; i < mPipelines.size(); ++i)
    {
        vkDestroyPipeline(device, mPipelines[i], nullptr);
    }

    vkDestroyPipelineLayout(device, mPipelineLayout, nullptr);

    for (VkDescriptorSetLayout layout : mDescriptorSetLayouts)
    {
        vkDestroyDescriptorSetLayout(device, layout, nullptr);
    }

    mDescriptorSetLayouts.clear();
    mLayoutBindings.clear();
}

void Pipeline::BindPipeline(VkCommandBuffer commandBuffer, VertexType vertexType)
{
    VkPipelineBindPoint bindPoint = mComputePipeline ? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS;
    vkCmdBindPipeline(commandBuffer, bindPoint, vertexType == VertexType::Max ? mPipelines[0] : mPipelines[(uint32_t) vertexType]);
}

VkPipelineLayout Pipeline::GetPipelineLayout()
{
    return mPipelineLayout;
}

PipelineId Pipeline::GetId() const
{
    return mPipelineId;
}

VkShaderModule Pipeline::CreateShaderModule(const std::vector<char>& code)
{
    VkDevice device = GetVulkanDevice();
    VkShaderModuleCreateInfo ciModule = {};
    ciModule.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    ciModule.codeSize = code.size();

    vector<uint32_t> codeLong(code.size() / sizeof(uint32_t) + 1);
    memcpy(codeLong.data(), code.data(), code.size());
    ciModule.pCode = codeLong.data();

    VkShaderModule module;

    if (vkCreateShaderModule(device, &ciModule, nullptr, &module) != VK_SUCCESS)
    {
        LogError("Failed to create shader module");
        OCT_ASSERT(0);
    }

    SetDebugObjectName(VK_OBJECT_TYPE_SHADER_MODULE, (uint64_t)module, mName.c_str());

    return module;
}

void Pipeline::AddOpaqueBlendAttachmentState()
{
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT;
    colorBlendAttachment.blendEnable = false;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    mBlendAttachments.push_back(colorBlendAttachment);
}

void Pipeline::AddMixBlendAttachmentState()
{
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = true;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    mBlendAttachments.push_back(colorBlendAttachment);
}

void Pipeline::AddAdditiveBlendAttachmentState()
{
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = true;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    mBlendAttachments.push_back(colorBlendAttachment);
}

void Pipeline::AddLayoutBinding(VkDescriptorType type, VkShaderStageFlags stageFlags)
{
    VkDescriptorSetLayoutBinding layoutBinding = {};
    layoutBinding.descriptorCount = 1;
    layoutBinding.descriptorType = type;
    layoutBinding.pImmutableSamplers = nullptr;
    layoutBinding.stageFlags = stageFlags;
    layoutBinding.binding = static_cast<uint32_t>(mLayoutBindings.back().size());

    mLayoutBindings.back().push_back(layoutBinding);
}

void Pipeline::PushSet()
{
    mLayoutBindings.push_back(std::vector<VkDescriptorSetLayoutBinding>());
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
    for (uint32_t i = 0; i < mLayoutBindings.size(); ++i)
    {
        VkDescriptorSetLayoutCreateInfo ciDescriptorSetLayout = {};
        ciDescriptorSetLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        ciDescriptorSetLayout.bindingCount = static_cast<uint32_t>(mLayoutBindings[i].size());
        ciDescriptorSetLayout.pBindings = mLayoutBindings[i].data();

        mDescriptorSetLayouts.push_back(VK_NULL_HANDLE);

        if (vkCreateDescriptorSetLayout(GetVulkanDevice(),
            &ciDescriptorSetLayout,
            nullptr,
            &mDescriptorSetLayouts[i]) != VK_SUCCESS)
        {
            LogError("Failed to create descriptor set layout");
            OCT_ASSERT(0);
        }
    }
}

void Pipeline::PopulateLayoutBindings()
{
    // Global descriptor set
    PushSet();
    AddLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    AddLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
}

#endif
