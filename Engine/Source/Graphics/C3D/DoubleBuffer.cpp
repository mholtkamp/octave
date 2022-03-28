#if API_C3D

#include "Graphics/C3D/DoubleBuffer.h"
#include "Graphics/C3D/C3dTypes.h"
#include "Graphics/C3D/C3dUtils.h"

#include <assert.h>
#include <string.h>
#include <3ds.h>

void DoubleBuffer::Alloc(size_t size, const void* srcData)
{
    for (uint32_t i = 0; i < MAX_FRAMES; ++i)
    {
        assert(mData[i] == nullptr);
        mData[i] = linearAlloc(size);
        mSize = size;

        if (srcData != nullptr)
        {
            memcpy(mData[i], srcData, size);
            GSPGPU_FlushDataCache(mData[i], size);
        }
    }
}

void DoubleBuffer::Free()
{
    for (uint32_t i = 0; i < MAX_FRAMES; ++i)
    {
        if (mData[i] != nullptr)
        {
            QueueLinearFree(mData[i]);
            mData[i] = nullptr;
        }
    }
}

void DoubleBuffer::Update(const void* srcData, size_t srcSize, uint32_t frame)
{
    uint32_t frameIndex = (frame == MAX_FRAMES) ? gC3dContext.mFrameIndex : frame;

    if (mData[frameIndex] != nullptr)
    {
        // Maybe we want to support partial updates?
        //assert(srcSize == mSize);
        assert(frameIndex >= 0 && frameIndex < MAX_FRAMES);

        memcpy(mData[frameIndex], srcData, srcSize);
        GSPGPU_FlushDataCache(mData[frameIndex], srcSize);
    }
}

void DoubleBuffer::UpdateAllFrames(const void* srcData, size_t srcSize)
{
    for (uint32_t i = 0; i < MAX_FRAMES; ++i)
    {
        Update(srcData, srcSize, i);
    }
}

void* DoubleBuffer::Get()
{
    uint32_t frameIndex = gC3dContext.mFrameIndex;
    assert(frameIndex >= 0 && frameIndex < MAX_FRAMES);
    return mData[frameIndex];
}

size_t DoubleBuffer::GetSize() const
{
    return mSize;
}

#endif
