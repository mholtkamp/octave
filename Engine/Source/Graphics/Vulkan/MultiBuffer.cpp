#if API_VULKAN

#include "Graphics/Vulkan/MultiBuffer.h"
#include "Graphics/Vulkan/VulkanUtils.h"
#include "Graphics/Vulkan/DestroyQueue.h"

MultiBuffer::MultiBuffer(BufferType bufferType, size_t size, const char* debugName, const void* srcData)
{
    for (uint32_t i = 0; i < MAX_FRAMES; ++i)
    {
        mBuffers[i] = new Buffer(bufferType, size, debugName, srcData, true);
    }
}

MultiBuffer::~MultiBuffer()
{
    for (uint32_t i = 0; i < MAX_FRAMES; ++i)
    {
        GetDestroyQueue()->Destroy(mBuffers[i]);
        mBuffers[i] = nullptr;
    }
}

void MultiBuffer::Update(const void* srcData, size_t srcSize, size_t dstOffset)
{
    // Uniform buffers can only update the current frames buffer because
    // the previous frame that was submitted to the GPU might still be
    // using its uniform buffer.
    uint32_t frameIndex = GetFrameIndex();
    mBuffers[frameIndex]->Update(srcData, srcSize, dstOffset);
}

VkBuffer MultiBuffer::Get()
{
    uint32_t frameIndex = GetFrameIndex();
    return Get(frameIndex);
}

VkBuffer MultiBuffer::Get(uint32_t frameIndex)
{
    return mBuffers[frameIndex]->Get();
}

Buffer* MultiBuffer::GetBuffer()
{
    uint32_t frameIndex = GetFrameIndex();
    return GetBuffer(frameIndex);
}

Buffer* MultiBuffer::GetBuffer(uint32_t frameIndex)
{
    return mBuffers[frameIndex];
}

UniformBuffer::UniformBuffer(size_t size, const char* debugName, const void* srcData) : 
    MultiBuffer(BufferType::Uniform, size, debugName, srcData)
{

}

#endif