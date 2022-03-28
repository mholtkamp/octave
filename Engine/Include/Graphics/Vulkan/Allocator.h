#pragma once

#if API_VULKAN

#include <vulkan/vulkan.h>
#include <vector>

struct Allocation
{
    VkDeviceMemory mDeviceMemory;
    uint32_t mType;
    int64_t mID;
    VkDeviceSize mSize;
    VkDeviceSize mOffset;
    VkDeviceSize mPaddedSize;

    Allocation() :
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

struct MemoryChunk
{
    int64_t mID;
    uint64_t mOffset;
    uint64_t mSize;
    bool mFree;

    MemoryChunk() :
        mID(-1),
        mOffset(0),
        mSize(0),
        mFree(true)
    {

    }
};

struct MemoryBlock
{
    MemoryChunk* AllocateChunk(uint64_t size);
    bool FreeChunk(int64_t id);

    MemoryBlock() :
        mDeviceMemory(0),
        mSize(0),
        mAvailableMemory(0),
        mLargestChunk(0),
        mMemoryType(0)
    {
        
    }

    std::vector<MemoryChunk> mChunks;
    VkDeviceMemory mDeviceMemory;
    uint64_t mSize;
    uint64_t mAvailableMemory;
    uint64_t mLargestChunk;
    uint32_t mMemoryType;
};

class Allocator
{
public:

    static void Alloc(uint64_t size, uint64_t alignment, uint32_t memoryType, Allocation& outAllocation);
    static void Free(Allocation& allocation);

    static uint64_t GetNumBlocksAllocated();
    static uint64_t GetNumAllocations();
    static uint64_t GetNumAllocatedBytes();

    static const uint64_t sDefaultBlockSize;

private:

    static MemoryBlock* AllocateBlock(uint64_t newBlockSize, uint32_t memoryType);
    static void FreeBlock(MemoryBlock& block);


    static std::vector<MemoryBlock> sBlocks;
    static uint64_t sNumAllocations;
    static uint64_t sNumAllocatedBytes;
};

#endif