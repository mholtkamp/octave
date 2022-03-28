#if API_VULKAN

#include "Graphics/Vulkan/UniformBuffer.h"
#include "Graphics/Vulkan/VulkanUtils.h"
#include "Graphics/Vulkan/DestroyQueue.h"

UniformBuffer::UniformBuffer(size_t size, const char* debugName, const void* srcData)
{
    for (uint32_t i = 0; i < MAX_FRAMES; ++i)
    {
        mBuffers[i] = new Buffer(BufferType::Uniform, size, debugName, srcData, true);
    }
}

UniformBuffer::~UniformBuffer()
{
    for (uint32_t i = 0; i < MAX_FRAMES; ++i)
    {
        GetDestroyQueue()->Destroy(mBuffers[i]);
        mBuffers[i] = nullptr;
    }
}

void UniformBuffer::Update(const void* srcData, size_t srcSize)
{
    // Uniform buffers can only update the current frames buffer because
    // the previous frame that was submitted to the GPU might still be
    // using its uniform buffer.
    uint32_t frameIndex = GetFrameIndex();
    mBuffers[frameIndex]->Update(srcData, srcSize);
}

VkBuffer UniformBuffer::Get()
{
    uint32_t frameIndex = GetFrameIndex();
    return Get(frameIndex);
}

VkBuffer UniformBuffer::Get(uint32_t frameIndex)
{
    return mBuffers[frameIndex]->Get();
}

Buffer* UniformBuffer::GetBuffer()
{
    uint32_t frameIndex = GetFrameIndex();
    return GetBuffer(frameIndex);
}

Buffer* UniformBuffer::GetBuffer(uint32_t frameIndex)
{
    return mBuffers[frameIndex];
}

#endif