#pragma once

#include "EngineTypes.h"
#include "System/SystemTypes.h"
#include "VulkanTypes.h"
#include "Pipeline.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>

struct MaterialPipelineRequest
{
    uint32_t mId = 0;
};

struct MaterialPipelineResult
{
    uint32_t mId = 0;
    Pipeline* mPipeline = nullptr;
};

class MaterialPipelineCache
{
public:

    void Create();
    void Destroy();

    Pipeline* GetPipeline(Material* material, VertexType vertexType);
    Pipeline* GetPipeline(uint32_t id, VertexType vertexType);
    uint32_t GetNumPipelines() const;
    void Reset();
    void Update();
    void Enable(bool enable);

protected:

    static ThreadFuncRet BuildThreadFunc(void* arg);

    // These two data structures can be queried on main thread without locking mutex.
    std::unordered_map<uint32_t, Pipeline*> mPipelines;

    MutexObject* mMutex = nullptr;
    //CondObject* mWorkCondition = nullptr;
    std::vector<MaterialPipelineRequest> mRequests;
    std::vector<MaterialPipelineResult> mResults;
    VkRenderPass mRenderPass;
    bool mEnabled = false;

    ThreadObject* mBuildThread = nullptr;
};
