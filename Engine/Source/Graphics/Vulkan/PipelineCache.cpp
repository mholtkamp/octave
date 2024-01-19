#include "PipelineCache.h"

#include "Stream.h"
#include "VulkanUtils.h"
#include "VulkanContext.h"
#include "VulkanConstants.h"

#define PIPELINE_CACHE_SAVE_NAME "PipelineCache.sav"

void PipelineCache::Create()
{
    // Create vk pipeline cache object.
    const char* initData = nullptr;
    size_t initSize = 0;

#if 0
    Stream pipelineData;
    if (SYS_DoesSaveExist(PIPELINE_CACHE_SAVE_NAME))
    {
        if (SYS_ReadSave(PIPELINE_CACHE_SAVE_NAME, pipelineData))
        {
            initData = pipelineData.GetData();
            initSize = (size_t)pipelineData.GetSize();
        }
    }
#endif

    OCT_ASSERT(mPipelineCache == VK_NULL_HANDLE);
    VkPipelineCacheCreateInfo ciCache = {};
    ciCache.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    ciCache.pInitialData = (void*)initData;
    ciCache.initialDataSize = initSize;
    ciCache.flags = 0;

    if (vkCreatePipelineCache(GetVulkanDevice(), &ciCache, nullptr, &mPipelineCache) != VK_SUCCESS)
    {
        LogError("Failed to create pipeline cache");
        OCT_ASSERT(0);
    }
}

void PipelineCache::Destroy()
{
    SaveToFile();

    // Write out vk pipeline cache blob
    OCT_ASSERT(mPipelineCache != VK_NULL_HANDLE);
    vkDestroyPipelineCache(GetVulkanDevice(), mPipelineCache, nullptr);
}

void PipelineCache::SaveToFile()
{
    VkDevice device = GetVulkanDevice();

    if (mPipelineCache != VK_NULL_HANDLE)
    {
        size_t cacheSize = 0;
        VkResult res = vkGetPipelineCacheData(device, mPipelineCache, &cacheSize, nullptr);
        OCT_ASSERT(res == VK_SUCCESS);

        if (cacheSize > 0)
        {
            char* cacheData = (char*)malloc(sizeof(char) * cacheSize);
            OCT_ASSERT(cacheData);

            res = vkGetPipelineCacheData(device, mPipelineCache, &cacheSize, cacheData);
            OCT_ASSERT(res == VK_SUCCESS);

            Stream stream;
            stream.WriteBytes((uint8_t*)cacheData, (uint32_t)cacheSize);

            SYS_WriteSave(PIPELINE_CACHE_SAVE_NAME, stream);

            free(cacheData);
            cacheData = nullptr;
        }
    }
}

VkPipelineCache PipelineCache::GetPipelineCacheObj()
{
    return mPipelineCache;
}

Pipeline* PipelineCache::Resolve(const PipelineState& state)
{
    VkDevice device = GetVulkanDevice();

    auto it = mPipelineMap.find(state);
    if (it != mPipelineMap.end())
    {
        return it->second;
    }
    else
    {
        Pipeline* newPipeline = new Pipeline();
        newPipeline->Create(state, mPipelineCache);

        mPipelineMap[state] = newPipeline;
        return newPipeline;
    }
}
