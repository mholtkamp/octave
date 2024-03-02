#include "PipelineConfigs.h"
#include "VulkanContext.h"

static PipelineState sPipelineConfigs[(uint32_t)PipelineConfig::Count];


void InitPipelineConfigs()
{
    // Shadow
    PipelineState& stateShadow = sPipelineConfigs[(uint32_t)PipelineConfig::Shadow];
    stateShadow.mVertexShader = gVulkanContext->GetGlobalShader("Shadow.vert");
    stateShadow.mFragmentShader = gVulkanContext->GetGlobalShader("Shadow.frag");
    stateShadow.mVertexType = VertexType::Vertex;
    stateShadow.mRasterizerDiscard = false;

    // Forward
    PipelineState& stateForward = sPipelineConfigs[(uint32_t)PipelineConfig::Forward];
    stateForward.mVertexShader = gVulkanContext->GetGlobalShader("Forward.vert");
    stateForward.mFragmentShader = gVulkanContext->GetGlobalShader("Forward.frag");
    stateForward.mVertexType = VertexType::Vertex;
    stateForward.mDepthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    stateForward.mDepthWriteEnabled = true;
    stateForward.mCullMode = VK_CULL_MODE_BACK_BIT;
    stateForward.mBlendStates[0] = GetBasicBlendState(BasicBlendState::Opaque);

    // Opaque
    PipelineState& stateOpaque = sPipelineConfigs[(uint32_t)PipelineConfig::Opaque];
    stateOpaque = stateForward;

    // Translucent
    PipelineState& stateTranslucent = sPipelineConfigs[(uint32_t)PipelineConfig::Translucent];
    stateTranslucent = stateForward;
    stateTranslucent.mDepthWriteEnabled = false;
    stateTranslucent.mBlendStates[0] = GetBasicBlendState(BasicBlendState::Translucent);

    // Additive
    PipelineState& stateAdditive = sPipelineConfigs[(uint32_t)PipelineConfig::Additive];
    stateAdditive = stateForward;
    stateAdditive.mDepthWriteEnabled = false;
    stateAdditive.mBlendStates[0] = GetBasicBlendState(BasicBlendState::Additive);

    // ShadowMeshBack
    PipelineState& stateShadowMeshBack = sPipelineConfigs[(uint32_t)PipelineConfig::ShadowMeshBack];
    stateShadowMeshBack = stateOpaque;
    stateShadowMeshBack.mVertexShader = gVulkanContext->GetGlobalShader("Forward.vert");
    stateShadowMeshBack.mFragmentShader = gVulkanContext->GetGlobalShader("ForwardShadow.frag");
    stateShadowMeshBack.mCullMode = VK_CULL_MODE_FRONT_BIT;
    stateShadowMeshBack.mDepthCompareOp = VK_COMPARE_OP_GREATER;
    stateShadowMeshBack.mDepthWriteEnabled = false;
    stateShadowMeshBack.mBlendStates[0].colorWriteMask = VK_COLOR_COMPONENT_A_BIT;
    stateShadowMeshBack.mBlendStates[0].blendEnable = true;
    stateShadowMeshBack.mBlendStates[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    stateShadowMeshBack.mBlendStates[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    stateShadowMeshBack.mBlendStates[0].alphaBlendOp = VK_BLEND_OP_ADD;

    // ShadowMeshFront
    PipelineState& stateShadowMeshFront = sPipelineConfigs[(uint32_t)PipelineConfig::ShadowMeshFront];
    stateShadowMeshFront = stateOpaque;
    stateShadowMeshFront.mVertexShader = gVulkanContext->GetGlobalShader("Forward.vert");
    stateShadowMeshFront.mFragmentShader = gVulkanContext->GetGlobalShader("ForwardShadow.frag");
    stateShadowMeshFront.mCullMode = VK_CULL_MODE_BACK_BIT;
    stateShadowMeshFront.mDepthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    stateShadowMeshFront.mDepthWriteEnabled = false;
    stateShadowMeshFront.mBlendStates[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT;
    stateShadowMeshFront.mBlendStates[0].blendEnable = true;
    stateShadowMeshFront.mBlendStates[0].srcColorBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
    stateShadowMeshFront.mBlendStates[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
    stateShadowMeshFront.mBlendStates[0].colorBlendOp = VK_BLEND_OP_ADD;

    // ShadowMeshClear
    PipelineState& stateShadowMeshClear = sPipelineConfigs[(uint32_t)PipelineConfig::ShadowMeshClear];
    stateShadowMeshClear = stateShadowMeshFront;
    stateShadowMeshClear.mDepthCompareOp = VK_COMPARE_OP_ALWAYS;
    stateShadowMeshClear.mBlendStates[0].colorWriteMask = VK_COLOR_COMPONENT_A_BIT;
    stateShadowMeshClear.mBlendStates[0].blendEnable = true;
    stateShadowMeshClear.mBlendStates[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    stateShadowMeshClear.mBlendStates[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    stateShadowMeshClear.mBlendStates[0].alphaBlendOp = VK_BLEND_OP_ADD;

    // PostProcess
    PipelineState& statePostProcess = sPipelineConfigs[(uint32_t)PipelineConfig::PostProcess];
    statePostProcess.mVertexShader = gVulkanContext->GetGlobalShader("ScreenRect.vert");
    statePostProcess.mFragmentShader = gVulkanContext->GetGlobalShader("Tonemap.frag");
    statePostProcess.mVertexType = VertexType::VertexUI;
    statePostProcess.mPrimitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    statePostProcess.mCullMode = VK_CULL_MODE_NONE;
    statePostProcess.mDepthTestEnabled = false;

    // NullPostProcess
    PipelineState& stateNullPostProcess = sPipelineConfigs[(uint32_t)PipelineConfig::NullPostProcess];
    stateNullPostProcess = statePostProcess;
    stateNullPostProcess.mFragmentShader = gVulkanContext->GetGlobalShader("NullPostProcess.frag");

    // Quad
    PipelineState& stateQuad = sPipelineConfigs[(uint32_t)PipelineConfig::Quad];
    stateQuad.mVertexShader = gVulkanContext->GetGlobalShader("Quad.vert");
    stateQuad.mFragmentShader = gVulkanContext->GetGlobalShader("Quad.frag");
    stateQuad.mVertexType = VertexType::VertexUI;
    stateQuad.mPrimitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    stateQuad.mCullMode = VK_CULL_MODE_NONE;
    stateQuad.mDepthTestEnabled = false;
    stateQuad.mBlendStates[0].blendEnable = true;
    stateQuad.mBlendStates[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    stateQuad.mBlendStates[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    stateQuad.mBlendStates[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    // Text
    PipelineState& stateText = sPipelineConfigs[(uint32_t)PipelineConfig::Text];
    stateText.mVertexShader = gVulkanContext->GetGlobalShader("Text.vert");
    stateText.mFragmentShader = gVulkanContext->GetGlobalShader("Text.frag");
    stateText.mVertexType = VertexType::VertexUI;
    stateText.mPrimitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    stateText.mCullMode = VK_CULL_MODE_NONE;
    stateText.mDepthTestEnabled = false;
    stateText.mBlendStates[0].blendEnable = true;
    stateText.mBlendStates[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    stateText.mBlendStates[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    stateText.mBlendStates[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;


    // Poly
    PipelineState& statePoly = sPipelineConfigs[(uint32_t)PipelineConfig::Poly];
    statePoly.mVertexShader = gVulkanContext->GetGlobalShader("Poly.vert");
    statePoly.mFragmentShader = gVulkanContext->GetGlobalShader("Poly.frag");
    stateText.mVertexType = VertexType::VertexUI;
    statePoly.mPrimitiveTopology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    statePoly.mCullMode = VK_CULL_MODE_NONE;
    statePoly.mDepthTestEnabled = false;
    statePoly.mDynamicLineWidth = true;
    statePoly.mBlendStates[0].blendEnable = true;
    statePoly.mBlendStates[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    statePoly.mBlendStates[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    statePoly.mBlendStates[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    // Selected
    PipelineState& stateSelected = sPipelineConfigs[(uint32_t)PipelineConfig::Selected];
    stateSelected = stateForward;
    stateSelected.mVertexShader = gVulkanContext->GetGlobalShader("Depth.vert");
    stateSelected.mFragmentShader = gVulkanContext->GetGlobalShader("Selected.frag");
    stateSelected.mDepthTestEnabled = false;
    stateSelected.mDepthCompareOp = VK_COMPARE_OP_ALWAYS;
    stateSelected.mBlendStates[0] = GetBasicBlendState(BasicBlendState::Translucent);

    // HitCheck
    PipelineState& stateHitCheck = sPipelineConfigs[(uint32_t)PipelineConfig::HitCheck];
    stateHitCheck = stateForward;
    stateHitCheck.mVertexShader = gVulkanContext->GetGlobalShader("Depth.vert");
    stateHitCheck.mFragmentShader = gVulkanContext->GetGlobalShader("HitCheck.frag");
    stateHitCheck.mDepthCompareOp = VK_COMPARE_OP_LESS;
    stateHitCheck.mBlendStates[0] = GetBasicBlendState(BasicBlendState::Opaque);

    // Wireframe
    PipelineState& stateWireframe = sPipelineConfigs[(uint32_t)PipelineConfig::Wireframe];
    stateWireframe = stateForward;
    stateWireframe.mCullMode = VK_CULL_MODE_NONE;
    stateWireframe.mDepthTestEnabled = true;
    stateWireframe.mDepthCompareOp = VK_COMPARE_OP_LESS;
    stateWireframe.mPolygonMode = VK_POLYGON_MODE_LINE;
    stateWireframe.mLineWidth = 1.0f;
    stateWireframe.mFragmentShader = gVulkanContext->GetGlobalShader("Wireframe.frag");
    stateWireframe.mBlendStates[0] = GetBasicBlendState(BasicBlendState::Opaque);

    // Collision
    PipelineState& stateCollision = sPipelineConfigs[(uint32_t)PipelineConfig::Collision];
    stateCollision = stateWireframe;
    stateCollision.mLineWidth = 2.0f;
    stateCollision.mDepthWriteEnabled = false;
    stateCollision.mDepthBias = -0.05f;
    stateCollision.mDepthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

    // Line
    PipelineState& stateLine = sPipelineConfigs[(uint32_t)PipelineConfig::Line];
    stateLine.mVertexShader = gVulkanContext->GetGlobalShader("Line.vert");
    stateLine.mFragmentShader = gVulkanContext->GetGlobalShader("Line.frag");
    stateLine.mVertexType = VertexType::VertexLine;
    stateLine.mPrimitiveTopology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    stateLine.mDepthTestEnabled = true;
    stateLine.mDepthWriteEnabled = false;
    stateLine.mDepthCompareOp = VK_COMPARE_OP_LESS;
    stateLine.mCullMode = VK_CULL_MODE_NONE;
    stateLine.mLineWidth = 1.0f;
}

void BindPipelineConfig(PipelineConfig config)
{
    uint32_t index = (uint32_t)config;
    
    GetVulkanContext()->SetPipelineState(sPipelineConfigs[index]);
}
