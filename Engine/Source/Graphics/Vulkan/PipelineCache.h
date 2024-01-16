#pragma once

#include "VulkanTypes.h"
#include "Pipeline.h"

class PipelineCache
{


    std::unordered_map<PipelineState, Pipeline*> mPipelines;
};
