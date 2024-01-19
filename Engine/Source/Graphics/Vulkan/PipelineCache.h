#pragma once

#include "VulkanTypes.h"
#include "Pipeline.h"

#include <vulkan/vulkan.h>

class PipelineCache
{
public:

    void Create();
    void Destroy();
    void Clear();
    void SaveToFile();

    VkPipelineCache GetPipelineCacheObj();

    Pipeline* Resolve(const PipelineState& state);

protected:

    std::unordered_map<PipelineState, Pipeline*, PipelineStateHasher> mPipelineMap;
    VkPipelineCache mPipelineCache = VK_NULL_HANDLE;
};
