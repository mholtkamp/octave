#include "Graphics/Vulkan/MaterialPipelineCache.h"
#include "Graphics/Vulkan/VulkanUtils.h"
#include "Graphics/Vulkan/VulkanContext.h"
#include "Graphics/Vulkan/PipelineConfigs.h"

#include "Assets/Material.h"
#include "System/System.h"
#include "Log.h"

#if 0
ThreadFuncRet MaterialPipelineCache::BuildThreadFunc(void* arg)
{
    MaterialPipelineCache* cache = (MaterialPipelineCache*)arg;

    std::vector<MaterialPipelineRequest>& requests = cache->mRequests;
    std::vector<MaterialPipelineResult>& results = cache->mResults;
    MutexObject* mutex = cache->mMutex;

    while (true)
    {
        uint32_t id = 0;
        bool hasRequest = false;
        bool cacheEnabled = false;

        SYS_LockMutex(mutex);

        cacheEnabled = cache->mEnabled;

        if (cacheEnabled && requests.size() > 0)
        {
            hasRequest = true;
            id = requests[0].mId;
            requests.erase(requests.begin());
        }

        SYS_UnlockMutex(mutex);

        if (!cacheEnabled)
        {
            break;
        }

        if (hasRequest)
        {
            Pipeline *pipeline = nullptr;
            
            static_assert(uint32_t(ShadingModel::Count) <= 8, "Need to update pipeline id!");
            static_assert(uint32_t(BlendMode::Count) <= 8, "Need to update pipeline id!");
            static_assert(uint32_t(VertexColorMode::Count) <= 8, "Need to update pipeline id!");
            static_assert(uint32_t(TevMode::Count) <= 8, "Need to update pipeline id!");
            static_assert(uint32_t(CullMode::Count) <= 4, "Need to update pipeline id!");

            // Reverse the pipeline id to find specialization values
            ShadingModel shadingModel = ShadingModel((id >> 0) & 7);
            BlendMode blendMode = BlendMode((id >> 3) & 7);
            VertexColorMode vertexColorMode = VertexColorMode((id >> 6) & 7);
            VkBool32 hasSpecular = VkBool32((id >> 9) & 1);
            VkBool32 hasUv1 = VkBool32((id >> 10) & 1);
            VkBool32 hasFresnel = VkBool32((id >> 11) & 1);
            VkBool32 hasFog = VkBool32((id >> 12) & 1);
            VkBool32 hasWrapLighting = VkBool32((id >> 13) & 1);
            uint32_t numTextures = uint32_t((id >> 14) & 7);
            uint32_t tev0 = uint32_t((id >> 17) & 7);
            uint32_t tev1 = uint32_t((id >> 20) & 7);
            uint32_t tev2 = uint32_t((id >> 23) & 7);
            uint32_t tev3 = uint32_t((id >> 26) & 7);
            CullMode cullMode = CullMode((id >> 29) & 3);
            bool depthless = uint32_t((id >> 31) & 1);

            // Create base pipeline based on blend mode
            static_assert(uint32_t(BlendMode::Count) == 4, "Use blend mode to pick starting pipeline or adjust blend modes.");

            switch (blendMode)
            {
            case BlendMode::Translucent: pipeline = new TranslucentPipeline(); break;
            case BlendMode::Additive: pipeline = new AdditivePipeline(); break;
            default:
                pipeline = new OpaquePipeline(); break;
                break;
            }

            pipeline->mName = "MaterialSpec";

            // Set vertex configs
            // Don't use skeletal mesh shader because on android we use CPU skinning since gpu skinning is too costly.
            // The material pipeline cache was designed specifically to assist android devices.
            // In the future, maybe just create a Pipeline object based on the vertex type with only that single vertex type's vkPipeline
            pipeline->ClearVertexConfigs();
            pipeline->AddVertexConfig(VertexType::Vertex, ENGINE_SHADER_DIR "Forward.vert");
            pipeline->AddVertexConfig(VertexType::VertexColor, ENGINE_SHADER_DIR "ForwardColor.vert");
            pipeline->AddVertexConfig(VertexType::VertexParticle, ENGINE_SHADER_DIR "ForwardParticle.vert");
            pipeline->SetFragmentShader(ENGINE_SHADER_DIR "ForwardSpec.frag");

            // Set depth / cull pipeline properties
            if (depthless)
            {
                pipeline->mDepthTestEnabled = false;
                pipeline->mDepthWriteEnabled = false;
            }

            switch (cullMode)
            {
            case CullMode::Back: pipeline->mCullMode = VK_CULL_MODE_BACK_BIT; break;
            case CullMode::Front: pipeline->mCullMode = VK_CULL_MODE_FRONT_BIT; break;
            case CullMode::None: pipeline->mCullMode = VK_CULL_MODE_NONE; break;
            default: break;
            }

            // Set renderpass
            // TODO: This isn't thread safe, but as long as the VulkanContext:mForwardRenderPass
            // is only changed in VulkanContext::CreateRenderPass(), stopping the 
            // builder thread in RecreateSwapchain() should be ok.
            pipeline->SetRenderPass(GetVulkanContext()->GetForwardRenderPass() /*mRenderPass*/);

            // Gather specialization constants
            constexpr uint32_t kNumSpecs = 13;
            VkSpecializationMapEntry mapEntries[kNumSpecs] = {};

            // Shading model
            mapEntries[0].constantID = 0;
            mapEntries[0].offset = 0;
            mapEntries[0].size = 4;

            // Blend mode
            mapEntries[1].constantID = 1;
            mapEntries[1].offset = 4;
            mapEntries[1].size = 4;

            // Vertex color mode
            mapEntries[2].constantID = 2;
            mapEntries[2].offset = 8;
            mapEntries[2].size = 4;

            // Has specular
            mapEntries[3].constantID = 3;
            mapEntries[3].offset = 12;
            mapEntries[3].size = 4;

            // Has uv1
            mapEntries[4].constantID = 4;
            mapEntries[4].offset = 16;
            mapEntries[4].size = 4;

            // Has fresnel
            mapEntries[5].constantID = 5;
            mapEntries[5].offset = 20;
            mapEntries[5].size = 4;

            // Has fog
            mapEntries[6].constantID = 6;
            mapEntries[6].offset = 24;
            mapEntries[6].size = 4;

            // Has wrap lighting
            mapEntries[7].constantID = 7;
            mapEntries[7].offset = 28;
            mapEntries[7].size = 4;

            // Num textures
            mapEntries[8].constantID = 8;
            mapEntries[8].offset = 32;
            mapEntries[8].size = 4;

            // Tev 0
            mapEntries[9].constantID = 9;
            mapEntries[9].offset = 36;
            mapEntries[9].size = 4;

            // Tev 1
            mapEntries[10].constantID = 10;
            mapEntries[10].offset = 40;
            mapEntries[10].size = 4;

            // Tev 2
            mapEntries[11].constantID = 11;
            mapEntries[11].offset = 44;
            mapEntries[11].size = 4;

            // Tev 3
            mapEntries[12].constantID = 12;
            mapEntries[12].offset = 48;
            mapEntries[12].size = 4;

            Stream specDataStream;
            specDataStream.WriteUint32((uint32_t)shadingModel);
            specDataStream.WriteUint32((uint32_t)blendMode);
            specDataStream.WriteUint32((uint32_t)vertexColorMode);
            specDataStream.WriteUint32(hasSpecular);
            specDataStream.WriteUint32(hasUv1);
            specDataStream.WriteUint32(hasFresnel);
            specDataStream.WriteUint32(hasFog);
            specDataStream.WriteUint32(hasWrapLighting);
            specDataStream.WriteUint32(numTextures);
            specDataStream.WriteUint32(tev0);
            specDataStream.WriteUint32(tev1);
            specDataStream.WriteUint32(tev2);
            specDataStream.WriteUint32(tev3);

            VkSpecializationInfo specInfo = {};
            specInfo.mapEntryCount = kNumSpecs;
            specInfo.pMapEntries = mapEntries;
            specInfo.pData = specDataStream.GetData();
            specInfo.dataSize = specDataStream.GetSize();

            // Create pipeline
            pipeline->Create(&specInfo);

            {
                SCOPED_LOCK(mutex);
                MaterialPipelineResult res;
                res.mId = id;
                res.mPipeline = pipeline;
                results.push_back(res);
            }
        }
        else
        {
            // TODO: Use condition variable instead.
            SYS_Sleep(1);
        }
    }

    THREAD_RETURN();
}


void MaterialPipelineCache::Create()
{
    mMutex = SYS_CreateMutex();
}

void MaterialPipelineCache::Destroy()
{
    // This call may join + destroy builder thread.
    // This could potentially be a long wait?
    Enable(false);

    SYS_DestroyMutex(mMutex);
    mMutex = nullptr;
}

Pipeline* MaterialPipelineCache::GetPipeline(Material* material, VertexType vertexType)
{
    static_assert(uint32_t(ShadingModel::Count) <= 8, "Need to update pipeline id!");
    static_assert(uint32_t(BlendMode::Count) <= 8, "Need to update pipeline id!");
    static_assert(uint32_t(VertexColorMode::Count) <= 8, "Need to update pipeline id!");
    static_assert(uint32_t(TevMode::Count) <= 8, "Need to update pipeline id!");
    static_assert(uint32_t(CullMode::Count) <= 4, "Need to update pipeline id!");

    // Constant values
    uint32_t shadingModel_3 = (uint32_t)material->GetShadingModel();
    uint32_t blendMode_3 = (uint32_t)material->GetBlendMode();
    uint32_t vertexColorMode_3 = (uint32_t)material->GetVertexColorMode();
    uint32_t hasSpecular_1 = (uint32_t)(material->GetSpecular() > 0.0f);
    uint32_t hasUv1_1 = (uint32_t)(material->GetUvMap(0) == 1 || material->GetUvMap(1) == 1 || material->GetUvMap(2) == 1 || material->GetUvMap(3) == 1);
    uint32_t hasFresnel_1 = (uint32_t)material->IsFresnelEnabled();
    uint32_t hasFog_1 = (uint32_t)material->ShouldApplyFog();
    uint32_t hasWrapLighting_1 = (uint32_t)(material->GetWrapLighting() > 0.0f);
    uint32_t numTextures_3 = (uint32_t)(1 + uint32_t(material->GetTevMode(1) != TevMode::Pass) + uint32_t(material->GetTevMode(2) != TevMode::Pass) + uint32_t(material->GetTevMode(3) != TevMode::Pass));
    uint32_t tev0_3 = (uint32_t)material->GetTevMode(0);
    uint32_t tev1_3 = (uint32_t)material->GetTevMode(1);
    uint32_t tev2_3 = (uint32_t)material->GetTevMode(2);
    uint32_t tev3_3 = (uint32_t)material->GetTevMode(3);
    uint32_t cullMode_2 = (uint32_t)material->GetCullMode();
    uint32_t depthless_1 = (uint32_t)material->IsDepthTestDisabled();

    uint32_t id = 0;
    id += shadingModel_3 << 0;
    id += blendMode_3 << 3;
    id += vertexColorMode_3 << 6;
    id += hasSpecular_1 << 9;
    id += hasUv1_1 << 10;
    id += hasFresnel_1 << 11;
    id += hasFog_1 << 12;
    id += hasWrapLighting_1 << 13;
    id += numTextures_3 << 14;
    id += tev0_3 << 17;
    id += tev1_3 << 20;
    id += tev2_3 << 23;
    id += tev3_3 << 26;
    id += cullMode_2 << 29;
    id += depthless_1 << 31;

    return GetPipeline(id, vertexType);
}

Pipeline* MaterialPipelineCache::GetPipeline(uint32_t id, VertexType vertexType)
{
    Pipeline* retPipeline = nullptr;

    // Based on the pipeline id, check our mPipelines map.
    auto it = mPipelines.find(id);

    // If no pipeline in the map, create a request for the pipeline and return null.
    if (it == mPipelines.end())
    {
        if (mPipelines.size() < mMaxPipelines)
        {
            // Insert nullptr
            mPipelines[id] = nullptr;

            // Enqueue request
            MaterialPipelineRequest request;
            request.mId = id;

            SYS_LockMutex(mMutex);
            mRequests.push_back(request);
            SYS_UnlockMutex(mMutex);
        }
    }
    else
    {
        // See if the vertex type is supported.
        if (it->second != nullptr &&
            it->second->GetVkPipeline(vertexType) != VK_NULL_HANDLE)
        {
            retPipeline = it->second;
        }
    }

    return retPipeline;
}

uint32_t MaterialPipelineCache::GetNumPipelines() const
{
    // This should really just be used for statistics
    return (uint32_t)mPipelines.size();
}

void MaterialPipelineCache::Reset()
{
    // Destroy all pipelines (or queue delete them?)
    DeviceWaitIdle();
    for (auto it = mPipelines.begin(); it != mPipelines.end(); ++it)
    {
        if (it->second != nullptr)
        {
            GetDestroyQueue()->Destroy(it->second);
            it->second = nullptr;
        }
    }
    mPipelines.clear();

    {
        SCOPED_LOCK(mMutex);
        mRequests.clear();

        for (uint32_t i = 0; i < mResults.size(); ++i)
        {
            if (mResults[i].mPipeline != nullptr)
            {
                GetDestroyQueue()->Destroy(mResults[i].mPipeline);
                mResults[i].mPipeline = nullptr;
            }
        }

        mResults.clear();
    }

    // What do we do about pipelines currently being built on the worker thread?
    // For now... nothing?
}

void MaterialPipelineCache::Update()
{
    SCOPED_LOCK(mMutex);
    
    // Move all of the results into the mPipelines map.
    for (uint32_t i = 0; i < mResults.size(); ++i)
    {
        uint32_t id = mResults[i].mId;
        OCT_ASSERT(mResults[i].mPipeline != nullptr);

        if (mPipelines.find(id) == mPipelines.end())
        {
            LogWarning("Result pipeline wasn't requested. This can happen after calling Reset()");
        }

        mPipelines[id] = mResults[i].mPipeline;
    }

    mResults.clear();
}

void MaterialPipelineCache::Enable(bool enable)
{
    SYS_LockMutex(mMutex);
    mEnabled = enable;
    SYS_UnlockMutex(mMutex);

    if (enable)
    { 
        if (mBuildThread == nullptr)
        {
            // Startup the thread
            mBuildThread = SYS_CreateThread(BuildThreadFunc, this);
        }
    }
    else
    {
        if (mBuildThread != nullptr)
        {
            SYS_JoinThread(mBuildThread);
            SYS_DestroyThread(mBuildThread);
            mBuildThread = nullptr;
        }
    }
}

uint32_t MaterialPipelineCache::GetMaxPipelines() const
{
    return mMaxPipelines;
}

void MaterialPipelineCache::SetMaxPipelines(uint32_t maxPipelines)
{
    mMaxPipelines = maxPipelines;
}

#endif