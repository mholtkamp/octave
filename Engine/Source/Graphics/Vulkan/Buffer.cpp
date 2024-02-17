#if API_VULKAN

#include "Graphics/Vulkan/Buffer.h"
#include "Graphics/Vulkan/VulkanUtils.h"
#include "Graphics/Vulkan/VulkanContext.h"

#include "Renderer.h"

Buffer::Buffer(
    BufferType type,
    size_t size,
    const char* debugObjectName,
    const void* srcData,
    bool hostVisible)
{
    mType = type;
    mSize = size;
    mHostVisible = hostVisible;

    // Create vertex buffer
    VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    switch (mType)
    {
    case BufferType::Vertex: usageFlags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT; break;
    case BufferType::Index: usageFlags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT; break;
    case BufferType::Uniform: usageFlags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT; break;
    case BufferType::Transfer: usageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT; break;
    case BufferType::Storage: usageFlags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; break;
    default: OCT_ASSERT(0); break; // Not valid type
    }

    VkMemoryPropertyFlags memoryFlags = mHostVisible ? (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    CreateBuffer(mSize, usageFlags, memoryFlags, mBuffer, mMemory);

    // If srcData was supplied, perform an update
    if (srcData != nullptr)
    {
        Update(srcData, mSize, 0);
    }

    if (debugObjectName != nullptr)
    {
        SetDebugObjectName(VK_OBJECT_TYPE_BUFFER, (uint64_t)mBuffer, debugObjectName);
    }
}

Buffer::~Buffer()
{
    vkDestroyBuffer(GetVulkanDevice(), mBuffer, nullptr);
    mBuffer = VK_NULL_HANDLE;

    VramAllocator::Free(mMemory);

    mSize = 0;
}

void Buffer::Update(const void* srcData, size_t srcSize, size_t dstOffset)
{
    VkDevice device = GetVulkanDevice();

    if (srcData == nullptr ||
        srcSize == 0 ||
        srcSize > (mSize - dstOffset))
    {
        LogError("Bad params passed to VertexBuffer::Update()");
        OCT_ASSERT(0);
        return;
    }

    // If not host visible, then we need to use a staging buffer to transfer data to device-local memory.
    if (mHostVisible)
    {
        void* data = mMappedPointer;

        if (mMappedPointer == nullptr)
        {
            vkMapMemory(device, mMemory.mDeviceMemory, mMemory.mOffset, srcSize, 0, &data);
        }

        memcpy(data, srcData, srcSize);

        if (mMappedPointer == nullptr)
        {
            vkUnmapMemory(device, mMemory.mDeviceMemory);
        }
    }
    else
    {
        Buffer* stagingBuffer = new Buffer(BufferType::Transfer, srcSize, "Staging Buffer", srcData);
        CopyBuffer(stagingBuffer->Get(), mBuffer, srcSize);
        GetDestroyQueue()->Destroy(stagingBuffer);
    }
}

void* Buffer::Map()
{
    vkMapMemory(GetVulkanDevice(), mMemory.mDeviceMemory, mMemory.mOffset, mMemory.mSize, 0, &mMappedPointer);
    return mMappedPointer;
}

void Buffer::Unmap()
{
    vkUnmapMemory(GetVulkanDevice(), mMemory.mDeviceMemory);
    mMappedPointer = nullptr;
}

VkBuffer Buffer::Get()
{
    return mBuffer;
}

size_t Buffer::GetSize() const
{
    return mSize;
}

BufferType Buffer::GetType() const
{
    return mType;
}

void* Buffer::GetMappedPointer()
{
    return mMappedPointer;
}

#endif