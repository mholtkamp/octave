#pragma once

#if API_VULKAN

#include "Enums.h"
#include "Constants.h"
#include "VulkanTypes.h"

#include "Graphics/Vulkan/Pipeline.h"

#include "Assertion.h"

void InitPipelineConfigs();
void BindPipelineConfig(PipelineConfig config);

#endif