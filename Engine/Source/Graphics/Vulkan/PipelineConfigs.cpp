#include "PipelineConfigs.h"
#include "VulkanContext.h"

static PipelineState sPipelineConfigs[(uint32_t)PipelineConfig::Count];


void InitPipelineConfigs()
{
    {
        // Shadow
        PipelineState& state = sPipelineConfigs[(uint32_t)PipelineConfig::Shadow];
        state.mVertexShader = gVulkanContext->GetGlobalShader("Shadow.vert");
        state.mFragmentShader = gVulkanContext->GetGlobalShader("Shadow.frag");
        state.mVertexType = VertexType::Vertex;
        state.mRasterizerDiscard = false;
    }

    {
        // Forward
        PipelineState& state = sPipelineConfigs[(uint32_t)PipelineConfig::Forward];
        state.mVertexShader = gVulkanContext->GetGlobalShader("Forward.vert");
        state.mFragmentShader = gVulkanContext->GetGlobalShader("Forward.frag");
        state.mVertexType = VertexType::Vertex;
        state.mDepthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        state.mDepthWriteEnabled = true;
        state.mCullMode = VK_CULL_MODE_BACK_BIT;
        state.mBlendStates[0] = GetBasicBlendState(BasicBlendState::Opaque);
    }

    {
        // Opaque
        PipelineState& state = sPipelineConfigs[(uint32_t)PipelineConfig::Opaque];
        state = sPipelineConfigs[(uint32_t)PipelineConfig::Forward];
    }

    {
        // Translucent
        PipelineState& state = sPipelineConfigs[(uint32_t)PipelineConfig::Translucent];
        state = sPipelineConfigs[(uint32_t)PipelineConfig::Forward];
        state.mDepthWriteEnabled = false;
        state.mBlendStates[0] = GetBasicBlendState(BasicBlendState::Translucent);
    }

    {
        // Additive
        PipelineState& state = sPipelineConfigs[(uint32_t)PipelineConfig::Additive];
        state = sPipelineConfigs[(uint32_t)PipelineConfig::Forward];
        state.mDepthWriteEnabled = false;
        state.mBlendStates[0] = GetBasicBlendState(BasicBlendState::Additive);
    }

    {
        // ShadowMeshBack
        PipelineState& state = sPipelineConfigs[(uint32_t)PipelineConfig::ShadowMeshBack];
        state = sPipelineConfigs[(uint32_t)PipelineConfig::Opaque];
        state.mVertexShader = gVulkanContext->GetGlobalShader("Forward.vert");
        state.mFragmentShader = gVulkanContext->GetGlobalShader("ForwardShadow.frag");
        state.mCullMode = VK_CULL_MODE_FRONT_BIT;
        state.mDepthCompareOp = VK_COMPARE_OP_GREATER;
        state.mDepthWriteEnabled = false;
        state.mBlendStates[0].colorWriteMask = 0;
        state.mBlendStates[0].blendEnable = false;
        state.mStencilTestEnabled = true;
        state.mStencilBack.failOp = VK_STENCIL_OP_KEEP;
        state.mStencilBack.depthFailOp = VK_STENCIL_OP_KEEP;
        state.mStencilBack.passOp = VK_STENCIL_OP_REPLACE;
        state.mStencilBack.compareOp = VK_COMPARE_OP_ALWAYS;
        state.mStencilBack.compareMask = STENCIL_BIT_SCRATCH;
        state.mStencilBack.writeMask = STENCIL_BIT_SCRATCH;
        state.mStencilBack.reference = STENCIL_BIT_SCRATCH;
    }

    {
        // ShadowMeshFront
        PipelineState& state = sPipelineConfigs[(uint32_t)PipelineConfig::ShadowMeshFront];
        state = sPipelineConfigs[(uint32_t)PipelineConfig::Opaque];
        state.mVertexShader = gVulkanContext->GetGlobalShader("Forward.vert");
        state.mFragmentShader = gVulkanContext->GetGlobalShader("ForwardShadow.frag");
        state.mCullMode = VK_CULL_MODE_BACK_BIT;
        state.mDepthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        state.mDepthWriteEnabled = false;
        state.mStencilTestEnabled = true;
        state.mStencilFront.failOp = VK_STENCIL_OP_ZERO;
        state.mStencilFront.depthFailOp = VK_STENCIL_OP_ZERO;
        state.mStencilFront.passOp = VK_STENCIL_OP_ZERO;
        state.mStencilFront.compareOp = VK_COMPARE_OP_EQUAL;
        state.mStencilFront.compareMask = STENCIL_BIT_SCRATCH;
        state.mStencilFront.writeMask = STENCIL_BIT_SCRATCH;
        state.mStencilFront.reference = STENCIL_BIT_SCRATCH;
        state.mBlendStates[0] = GetBasicBlendState(BasicBlendState::Translucent);
    }

    {
        // PostProcess
        PipelineState& state = sPipelineConfigs[(uint32_t)PipelineConfig::PostProcess];
        state.mVertexShader = gVulkanContext->GetGlobalShader("ScreenRect.vert");
        state.mFragmentShader = gVulkanContext->GetGlobalShader("Tonemap.frag");
        state.mVertexType = VertexType::VertexUI;
        state.mPrimitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        state.mCullMode = VK_CULL_MODE_NONE;
        state.mDepthTestEnabled = false;
    }

    {
        // NullPostProcess
        PipelineState& state = sPipelineConfigs[(uint32_t)PipelineConfig::NullPostProcess];
        state = sPipelineConfigs[(uint32_t)PipelineConfig::PostProcess];
        state.mFragmentShader = gVulkanContext->GetGlobalShader("NullPostProcess.frag");
    }

    {
        // Quad
        PipelineState& state = sPipelineConfigs[(uint32_t)PipelineConfig::Quad];
        state.mVertexShader = gVulkanContext->GetGlobalShader("Quad.vert");
        state.mFragmentShader = gVulkanContext->GetGlobalShader("Quad.frag");
        state.mVertexType = VertexType::VertexUI;
        state.mPrimitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        state.mCullMode = VK_CULL_MODE_NONE;
        state.mDepthTestEnabled = false;
        state.mBlendStates[0].blendEnable = true;
        state.mBlendStates[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        state.mBlendStates[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        state.mBlendStates[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    }

    {
        // Text
        PipelineState& state = sPipelineConfigs[(uint32_t)PipelineConfig::Text];
        state.mVertexShader = gVulkanContext->GetGlobalShader("Text.vert");
        state.mFragmentShader = gVulkanContext->GetGlobalShader("Text.frag");
        state.mVertexType = VertexType::VertexUI;
        state.mPrimitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        state.mCullMode = VK_CULL_MODE_NONE;
        state.mDepthTestEnabled = false;
        state.mBlendStates[0].blendEnable = true;
        state.mBlendStates[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        state.mBlendStates[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        state.mBlendStates[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    }

    {
        // Poly
        PipelineState& state = sPipelineConfigs[(uint32_t)PipelineConfig::Poly];
        state.mVertexShader = gVulkanContext->GetGlobalShader("Poly.vert");
        state.mFragmentShader = gVulkanContext->GetGlobalShader("Poly.frag");
        state.mVertexType = VertexType::VertexUI;
        state.mPrimitiveTopology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        state.mCullMode = VK_CULL_MODE_NONE;
        state.mDepthTestEnabled = false;
        state.mDynamicLineWidth = true;
        state.mBlendStates[0].blendEnable = true;
        state.mBlendStates[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        state.mBlendStates[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        state.mBlendStates[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    }

    {
        // Selected
        PipelineState& state = sPipelineConfigs[(uint32_t)PipelineConfig::Selected];
        state = sPipelineConfigs[(uint32_t)PipelineConfig::Forward];
        state.mVertexShader = gVulkanContext->GetGlobalShader("Depth.vert");
        state.mFragmentShader = gVulkanContext->GetGlobalShader("Selected.frag");
        state.mDepthTestEnabled = false;
        state.mDepthCompareOp = VK_COMPARE_OP_ALWAYS;
        state.mBlendStates[0] = GetBasicBlendState(BasicBlendState::Translucent);
    }

    {
        // HitCheck
        PipelineState& state = sPipelineConfigs[(uint32_t)PipelineConfig::HitCheck];
        state = sPipelineConfigs[(uint32_t)PipelineConfig::Forward];
        state.mVertexShader = gVulkanContext->GetGlobalShader("Depth.vert");
        state.mFragmentShader = gVulkanContext->GetGlobalShader("HitCheck.frag");
        state.mDepthCompareOp = VK_COMPARE_OP_LESS;
        state.mBlendStates[0] = GetBasicBlendState(BasicBlendState::Opaque);
    }

    {
        // Wireframe
        PipelineState& state = sPipelineConfigs[(uint32_t)PipelineConfig::Wireframe];
        state = sPipelineConfigs[(uint32_t)PipelineConfig::Forward];
        state.mCullMode = VK_CULL_MODE_NONE;
        state.mDepthTestEnabled = true;
        state.mDepthCompareOp = VK_COMPARE_OP_LESS;
        state.mPolygonMode = VK_POLYGON_MODE_LINE;
        state.mLineWidth = 1.0f;
        state.mFragmentShader = gVulkanContext->GetGlobalShader("Wireframe.frag");
        state.mBlendStates[0] = GetBasicBlendState(BasicBlendState::Opaque);
    }

    {
        // Collision
        PipelineState& state = sPipelineConfigs[(uint32_t)PipelineConfig::Collision];
        state = sPipelineConfigs[(uint32_t)PipelineConfig::Wireframe];
        state.mLineWidth = 2.0f;
        state.mDepthWriteEnabled = false;
        state.mDepthBias = -0.05f;
        state.mDepthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    }

    {
        // Line
        PipelineState& state = sPipelineConfigs[(uint32_t)PipelineConfig::Line];
        state.mVertexShader = gVulkanContext->GetGlobalShader("Line.vert");
        state.mFragmentShader = gVulkanContext->GetGlobalShader("Line.frag");
        state.mVertexType = VertexType::VertexLine;
        state.mPrimitiveTopology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        state.mDepthTestEnabled = true;
        state.mDepthWriteEnabled = false;
        state.mDepthCompareOp = VK_COMPARE_OP_LESS;
        state.mCullMode = VK_CULL_MODE_NONE;
        state.mLineWidth = 1.0f;
    }
}

void BindPipelineConfig(PipelineConfig config)
{
    uint32_t index = (uint32_t)config;
    
    GetVulkanContext()->SetPipelineState(sPipelineConfigs[index]);
}
