#if API_VULKAN

#include "Graphics/Vulkan/MultiBuffer.h"
#include "Graphics/Vulkan/VulkanUtils.h"
#include "Graphics/Vulkan/DestroyQueue.h"
#include "Graphics/Vulkan/VulkanContext.h"

#include "Log.h"

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

size_t MultiBuffer::GetSize() const
{
    return mBuffers[0]->GetSize();
}

UniformBuffer::UniformBuffer(size_t size, const char* debugName, const void* srcData) : 
    MultiBuffer(BufferType::Uniform, size, debugName, srcData)
{
#if 0
    // Just have these always mapped for now?
    // Apparently mapping is an expensive operation and its okay to leave it mapped.
    for (uint32_t i = 0; i < MAX_FRAMES; ++i)
    {
        mBuffers[i]->Map();
    }
#endif
}

void UniformBuffer::Reset(uint32_t frameIndex)
{
    if (frameIndex < MAX_FRAMES)
    {
        mHead[frameIndex] = 0;
    }
    else
    {
        LogError("Invalid frame index in UniformBuffer::Reset()");
    }
}

UniformBlock UniformBuffer::AllocBlock(uint32_t blockSize)
{
    UniformBlock retBlock;

    const uint32_t uboAlignment = (uint32_t) GetVulkanContext()->GetDeviceProperties().limits.minUniformBufferOffsetAlignment;
    LogDebug("Ubo Alignment = %d / %x", uboAlignment, uboAlignment);
    
    uint32_t frameIndex = GetFrameIndex();
    int32_t head = mHead[frameIndex];

    if (head + blockSize < GetSize())
    {
        retBlock.mOffset = head;
        retBlock.mSize = blockSize;
        retBlock.mData = ((uint8_t*)GetBuffer(frameIndex)->GetMappedPointer()) + retBlock.mOffset;
        retBlock.mUniformBuffer = this;

        // Move head based on block size, but also ensure proper alignment.
        uint32_t alignedBlockSize = blockSize;
        alignedBlockSize += uboAlignment - 1;
        alignedBlockSize = alignedBlockSize & (~(uboAlignment - 1));

        int32_t newHead = head + alignedBlockSize;

        mHead[frameIndex] = newHead;
    }
    else
    {
        LogError("Uniform buffer overflowed.");
    }

    return retBlock;
}

#endif
