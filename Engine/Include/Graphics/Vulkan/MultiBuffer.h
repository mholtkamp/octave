#pragma once

#if API_VULKAN

#include "Graphics/GraphicsConstants.h"
#include "Graphics/Vulkan/Buffer.h"

class DestroyQueue;

class MultiBuffer
{
public:

    MultiBuffer(BufferType bufferType, size_t size, const char* debugName, const void* srcData = nullptr);

    void Update(const void* srcData, size_t srcSize, size_t dstOffset = 0);

    VkBuffer Get();
    VkBuffer Get(uint32_t frameIndex);

    Buffer* GetBuffer();
    Buffer* GetBuffer(uint32_t frameIndex);

protected:

    friend class DestroyQueue;
    virtual ~MultiBuffer();

    Buffer* mBuffers[MAX_FRAMES] = {};
};

class UniformBuffer : public MultiBuffer
{
public:

    UniformBuffer(size_t size, const char* debugName, const void* srcData = nullptr);
};

#endif