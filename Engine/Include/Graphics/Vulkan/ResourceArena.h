#pragma once

#include "DestroyQueue.h"

#include <vector>
#include <stdint.h>

class ResourceArena
{
public:

    ResourceArena()
    {

    }

    ~ResourceArena()
    {
        // User needs to call Destroy();
        assert(mResources.size() == 0);
    }

    ResourceArena(const ResourceArena&) = delete;
    ResourceArena& operator=(const ResourceArena&) = delete;

    virtual void DestroyResource(void* resource) = 0;

    void Reset()
    {
        mIndex = 0;
    }

    void Destroy()
    {
        Reset();
        for (uint32_t i = 0; i < mResources.size(); ++i)
        {
            DestroyResource(mResources[i]);
        }

        mResources.clear();
        mResources.shrink_to_fit();
    }
protected:

    std::vector<void*> mResources;
    uint32_t mIndex = 0;
};

class DescriptorSetArena : public ResourceArena
{
public:

    DescriptorSet* Alloc(VkDescriptorSetLayout layout)
    {
        DescriptorSet* retSet = nullptr;
        if (mIndex >= (uint32_t)mResources.size())
        {
            retSet = new DescriptorSet(layout);
            mResources.push_back(retSet);
        }
        else
        {
            retSet = (DescriptorSet*)mResources[mIndex];
        }

        ++mIndex;
        return retSet;
    }

    virtual void DestroyResource(void* res) override
    {
        GetDestroyQueue()->Destroy((DescriptorSet*)res);
    }
};

class UniformBufferArena : public ResourceArena
{
public:

    UniformBuffer* Alloc(size_t size, const char* debugName, const void* srcData = nullptr)
    {
        UniformBuffer* retUniformBuffer = nullptr;
        if (mIndex >= (uint32_t)mResources.size())
        {
            retUniformBuffer = new UniformBuffer(size, debugName, srcData);
            mResources.push_back(retUniformBuffer);
        }
        else
        {
            retUniformBuffer = (UniformBuffer*)mResources[mIndex];
        }

        ++mIndex;
        return retUniformBuffer;
    }

    virtual void DestroyResource(void* res) override
    {
        GetDestroyQueue()->Destroy((UniformBuffer*) res);
    }
};
