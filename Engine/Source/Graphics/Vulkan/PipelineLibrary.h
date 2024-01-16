#pragma once

#include "VulkanTypes.h"
#include "Pipeline.h"

class PipelineLibrary
{


    std::unordered_map<PipelineState, Pipeline*> mPipelines;
};
