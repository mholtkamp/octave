#pragma once

#if API_VULKAN

#include "Graphics/GraphicsConstants.h"
#include "Graphics/Vulkan/Buffer.h"

class DestroyQueue;

class UniformBuffer
{
public:

    UniformBuffer(size_t size, const char* debugName, const void* srcData = nullptr);

    void Update(const void* srcData, size_t srcSize);

    VkBuffer Get();
    VkBuffer Get(uint32_t frameIndex);

    Buffer* GetBuffer();
    Buffer* GetBuffer(uint32_t frameIndex);

private:

    friend class DestroyQueue;
    ~UniformBuffer();

    Buffer* mBuffers[MAX_FRAMES] = {};
};

#endif