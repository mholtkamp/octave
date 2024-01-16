#include "PipelineConfigs.h"
#include "VulkanContext.h"

static PipelineState sPipelineConfigs[(uint32_t)PipelineConfig::Count];


void InitPipelineConfigs()
{
    //PipelineState& stateShadow = sPipelineConfigs[(uint32_t)PipelineConfig::Shadow];
    //stateShadow.mRasterizerDiscard = false;
    //stateShadow.mFragmentShader = GetVulkanContext()->GetSha
}

void BindPipelineConfig(PipelineConfig config)
{
    uint32_t index = (uint32_t)config;
    GetVulkanContext()->SetPipelineState(sPipelineConfigs[index]);
}
