#pragma once

#if API_VULKAN

#include <vulkan/vulkan.h>
#include <vector>

struct VramAllocation
{
    VkDeviceMemory mDeviceMemory;
    uint32_t mType;
    int64_t mID;
    VkDeviceSize mSize;
    VkDeviceSize mOffset;
    VkDeviceSize mPaddedSize;

    VramAllocation() :
        mDeviceMemory(VK_NULL_HANDLE),
        mType(0),
        mID(-1),
        mSize(0),
        mOffset(0),
        mPaddedSize(0)
    {

    }

    bool IsValid() const
    {
        return mDeviceMemory != VK_NULL_HANDLE;
    }
};

struct VramMemoryChunk
{
    int64_t mID;
    uint64_t mOffset;
    uint64_t mSize;
    bool mFree;

    VramMemoryChunk() :
        mID(-1),
        mOffset(0),
        mSize(0),
        mFree(true)
    {

    }
};

struct VramMemoryBlock
{
    VramMemoryChunk* AllocateChunk(uint64_t size);
    bool FreeChunk(int64_t id);

    VramMemoryBlock() :
        mDeviceMemory(0),
        mSize(0),
        mAvailableMemory(0),
        mLargestChunk(0),
        mMemoryType(0)
    {
        
    }

    std::vector<VramMemoryChunk> mChunks;
    VkDeviceMemory mDeviceMemory;
    uint64_t mSize;
    uint64_t mAvailableMemory;
    uint64_t mLargestChunk;
    uint32_t mMemoryType;
};

class VramAllocator
{
public:

    static void Alloc(uint64_t size, uint64_t alignment, uint32_t memoryType, VramAllocation& outAllocation);
    static void Free(VramAllocation& allocation);

    static uint64_t GetNumBlocksAllocated();
    static uint64_t GetNumAllocations();
    static uint64_t GetNumAllocatedBytes();

    static const uint64_t sDefaultBlockSize;

private:

    static VramMemoryBlock* AllocateBlock(uint64_t newBlockSize, uint32_t memoryType);
    static void FreeBlock(VramMemoryBlock& block);


    static std::vector<VramMemoryBlock> sBlocks;
    static uint64_t sNumAllocations;
    static uint64_t sNumAllocatedBytes;
};

#endif