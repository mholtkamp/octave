#pragma once

#if API_VULKAN

#include "Allocator.h"

#include <vulkan/vulkan.h>

class DestroyQueue;

enum class BufferType
{
    Vertex,
    Index,
    Uniform,
    Transfer,
    Storage,

    Count
};

class Buffer
{
public:

    Buffer(
        BufferType type,
        size_t size,
        const char* debugObjectName,
        const void* srcData = nullptr,
        bool hostVisible = true);

    void Update(const void* srcData, size_t srcSize, size_t dstOffset = 0);

    void* Map();
    void Unmap();

    VkBuffer Get();
    size_t GetSize() const;
    BufferType GetType() const;

private:

    friend class DestroyQueue;
    ~Buffer();

    VkBuffer mBuffer = VK_NULL_HANDLE;
    Allocation mMemory;
    BufferType mType = BufferType::Count;
    size_t mSize = 0;
    bool mHostVisible = false;
};

#endif