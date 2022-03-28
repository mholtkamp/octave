#pragma once

#if API_C3D

#include "Graphics/GraphicsConstants.h"
#include <stdint.h>
#include <stddef.h>

// Useful for buffers that are updated every frame by the CPU and used by the GPU.
// Like particle system vertex/index buffers, cpu-skinned skeletal mesh vertex buffer, etc

class DoubleBuffer
{
public:

    void Alloc(size_t size, const void* srcData = nullptr);
    void Free();
    void Update(const void* srcData, size_t srcSize, uint32_t frame = MAX_FRAMES);
    void UpdateAllFrames(const void* srcData, size_t srcSize);
    void* Get();
    size_t GetSize() const;

private:

    size_t mSize = 0;
    void* mData[MAX_FRAMES] = {};
};

#endif
