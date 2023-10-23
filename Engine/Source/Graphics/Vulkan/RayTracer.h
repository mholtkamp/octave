#pragma once

#include "VulkanUtils.h"
#include "MultiBuffer.h"
#include "DescriptorSet.h"
#include "Pipeline.h"
#include "PipelineConfigs.h"
#include "Allocator.h"
#include "DestroyQueue.h"
#include "Buffer.h"
#include "Image.h"
#include "ObjectRef.h"

class RayTracer
{
public:

    void CreateStaticRayTraceResources();
    void DestroyStaticRayTraceResources();
    void CreateDynamicRayTraceResources();
    void DestroyDynamicRayTraceResources();

    void PathTraceWorld();

    void BeginLightBake();
    void UpdateLightBake();
    void EndLightBake();
    bool IsLightBakeInProgress();
    float GetLightBakeProgress();

    Image* GetPathTraceImage();
    LightBakePhase GetLightBakePhase() const;

    void ReadbackLightBakeResults();

protected:

    void UpdateRayTracingScene(
        std::vector<RayTraceTriangle>& triangleData,
        std::vector<RayTraceMesh>& meshData,
        std::vector<RayTraceLight>& lightData,
        int32_t* outBakeMeshIndex = nullptr);
    void UpdateBakeVertexData();

    void DispatchNextLightBake();
    void DispatchNextBakeDiffuse();
    void FinalizeLightBake();

protected:

    // Ray Tracing Resources
    DescriptorSet* mPathTraceDescriptorSet = nullptr;
    Buffer* mRayTraceTriangleBuffer = nullptr;
    Buffer* mRayTraceMeshBuffer = nullptr;
    Buffer* mRayTraceLightBuffer = nullptr;
    UniformBuffer* mRayTraceUniformBuffer = nullptr;
    Buffer* mLightBakeVertexBuffer = nullptr;
    DescriptorSet* mBakeDiffuseDescriptorSet = nullptr;
    Buffer* mBakeAverageBuffer = nullptr;
    Buffer* mBakeDiffuseTriangleBuffer = nullptr;
    Image* mPathTraceImage = nullptr;

    // Path Tracing and Light Baking state
    uint32_t mAccumulatedFrames = 0;
    glm::vec3 mPathTracePrevCameraPos = { 0.0f, 0.0f, 0.0f };
    glm::vec3 mPathTracePrevCameraRot = { 0.0f, 0.0f, 0.0f };
    LightBakePhase mLightBakePhase = LightBakePhase::Count;
    std::vector<NodeRef> mLightBakeNodes;
    std::vector<LightBakeResult> mLightBakeResults;
    int32_t mBakingCompIndex = -1;
    int32_t mNextBakingCompIndex = 0;
    int64_t mBakedFrame = -1;
    uint32_t mTotalDiffusePasses = 1;
};

