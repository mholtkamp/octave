#pragma once

#if API_VULKAN

#include "Enums.h"
#include "Constants.h"
#include "VulkanTypes.h"

#include "Graphics/Vulkan/Pipeline.h"

#include "Assertion.h"

enum class PipelineConfig
{
    Shadow,
    Forward,
    Opaque,
    Translucent,
    Additive,
    ShadowMeshBack,
    ShadowMeshFront,
    ShadowMeshClear,
    PostProcess,
    NullPostProcess,
    Quad,
    Text,
    Poly,
    SelectedGeometry,
    HitCheck,
    Wireframe,
    Collision,
    Line,

    Count
};

void InitPipelineConfigs();
void BindPipelineConfig(PipelineConfig config);

#endif