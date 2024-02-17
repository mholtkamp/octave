#if API_VULKAN

#include "Graphics/Vulkan/VramAllocator.h"
#include "Renderer.h"
#include "Log.h"

#include "Graphics/Vulkan/VulkanContext.h"
#include "Graphics/Vulkan/VulkanUtils.h"

#include "Assertion.h"

std::vector<VramMemoryBlock> VramAllocator::sBlocks;
const uint64_t VramAllocator::sDefaultBlockSize = 16777216; // 16 MB Blocks

uint64_t VramAllocator::sNumAllocations = 0;
uint64_t VramAllocator::sNumAllocatedBytes = 0;

static int64_t sNumChunksAllocated = 0;

VramMemoryChunk* VramMemoryBlock::AllocateChunk(uint64_t size)
{
    VramMemoryChunk* chunk = nullptr;

    for (int32_t j = 0; j < int32_t(mChunks.size()); ++j)
    {
        if (mChunks[j].mFree &&
            mChunks[j].mSize >= size)
        {
            // We found a chunk that is big enough, now let's split the chunk into two.
            uint64_t extraSize = mChunks[j].mSize - size;

            if (extraSize > 0)
            {
                mChunks[j].mSize = size;
                
                VramMemoryChunk extraChunk;
                extraChunk.mFree = true;
                extraChunk.mOffset = mChunks[j].mOffset + mChunks[j].mSize;
                extraChunk.mSize = extraSize;
                extraChunk.mID = -1;

                mChunks.insert(mChunks.begin() + j + 1, extraChunk);
            }

            mChunks[j].mFree = false;
            mChunks[j].mID = sNumChunksAllocated++;

            OCT_ASSERT(mChunks[j].mID >= 0); // Did we overflow int64_t?

            // make sure we grab the pointer after inserting the new chunk (as it may reallocate the data).
            chunk = &mChunks[j];
            break;
        }
    }

    return chunk;
}

bool VramMemoryBlock::FreeChunk(int64_t id)
{
    bool bFreed = false;

    for (int32_t j = 0; j < int32_t(mChunks.size()); ++j)
    {
        if (mChunks[j].mID == id)
        {
            bFreed = true;

            // Mark this chunk as freed
            mChunks[j].mFree = true;
            
            // See if we can merge this chunk
            // First try merging it with the next chunk in the list
            if (j < int32_t(mChunks.size()) - 1 &&
                mChunks[j + 1].mFree)
            {
                mChunks[j].mSize += mChunks[j + 1].mSize;
                mChunks.erase(mChunks.begin() + j + 1);
            }

            // Now try merging with the previous chunk
            if (j > 0 &&
                mChunks[j - 1].mFree)
            {
                mChunks[j - 1].mSize += mChunks[j].mSize;
                mChunks.erase(mChunks.begin() + j);
            }
        }
    }

    return bFreed;
}

void VramAllocator::Alloc(uint64_t size, uint64_t alignment, uint32_t memoryType, VramAllocation& outAllocation)
{
    VramMemoryBlock* block = nullptr;
    VramMemoryChunk* chunk = nullptr;

    uint64_t maxAlignSize = size + alignment;
    for (int32_t i = 0; i < int32_t(sBlocks.size()); ++i)
    {
        if (sBlocks[i].mMemoryType == memoryType)
        {
            chunk = sBlocks[i].AllocateChunk(maxAlignSize);

            if (chunk != nullptr)
            {
                block = &sBlocks[i];
                break;
            }
        }
    }

    if (chunk == nullptr)
    {
        uint64_t newBlockSize = maxAlignSize > sDefaultBlockSize ? maxAlignSize : sDefaultBlockSize;
        block = AllocateBlock(newBlockSize, memoryType);
        OCT_ASSERT(block);

        chunk = block->AllocateChunk(maxAlignSize);
    }

    OCT_ASSERT(chunk);

    outAllocation.mDeviceMemory = block->mDeviceMemory;
    outAllocation.mID = chunk->mID;
    outAllocation.mOffset = ((chunk->mOffset + alignment - 1) / alignment) * alignment;
    outAllocation.mSize = size;
    outAllocation.mType = block->mMemoryType;
    outAllocation.mPaddedSize = maxAlignSize;

    sNumAllocations++;
    sNumAllocatedBytes += outAllocation.mPaddedSize;

    //LogDebug("ALLOC: NumAllocations = %lld, NumAllocatedBytes = %lld", sNumAllocations, sNumAllocatedBytes);
}

void VramAllocator::Free(VramAllocation& allocation)
{
    sNumAllocations--;
    sNumAllocatedBytes -= allocation.mPaddedSize;

    //LogDebug("FREE: NumAllocations = %lld, NumAllocatedBytes = %lld", sNumAllocations, sNumAllocatedBytes);

    bool bFreed = false;
    for (int32_t i = 0; i < int32_t(sBlocks.size()); ++i)
    {
        if (sBlocks[i].mMemoryType == allocation.mType)
        {
            bFreed = sBlocks[i].FreeChunk(allocation.mID);

            if (bFreed)
            {
                // If the block is entirely free, deallocate the memory.
                if (sBlocks[i].mChunks.size() == 1)
                {
                    OCT_ASSERT(sBlocks[i].mChunks[0].mFree);
                    FreeBlock(sBlocks[i]);
                }
                break;
            }
        }
    }

    OCT_ASSERT(bFreed);

    allocation.mDeviceMemory = VK_NULL_HANDLE;
    allocation.mID = -1;
    allocation.mOffset = 0;
    allocation.mSize = 0;
    allocation.mType = 0;
}

uint64_t VramAllocator::GetNumBlocksAllocated()
{
    return static_cast<uint64_t>(sBlocks.size());
}

uint64_t VramAllocator::GetNumAllocations()
{
    return sNumAllocations;
}

uint64_t VramAllocator::GetNumAllocatedBytes()
{
    return sNumAllocatedBytes;
}

VramMemoryBlock* VramAllocator::AllocateBlock(uint64_t newBlockSize, uint32_t memoryType)
{
    sBlocks.push_back(VramMemoryBlock());
    VramMemoryBlock& newBlock = sBlocks.back();

    newBlock.mSize = newBlockSize;
    newBlock.mAvailableMemory = newBlockSize;
    newBlock.mLargestChunk = newBlockSize;
    newBlock.mMemoryType = memoryType;

    // Allocate video memory.
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = newBlockSize;
    allocInfo.memoryTypeIndex = memoryType;

    if (vkAllocateMemory(GetVulkanDevice(), &allocInfo, nullptr, &newBlock.mDeviceMemory) != VK_SUCCESS)
    {
        LogError("Failed to allocate image memory");
        OCT_ASSERT(0);
    }

    // Initialize the starting chunk.
    VramMemoryChunk firstChunk;
    firstChunk.mFree = true;
    firstChunk.mID = -1;
    firstChunk.mOffset = 0;
    firstChunk.mSize = newBlockSize;
    newBlock.mChunks.push_back(firstChunk);

    return &newBlock;
}

void VramAllocator::FreeBlock(VramMemoryBlock& block)
{
    int32_t index = 0;

    for (index = 0; index < int32_t(sBlocks.size()); ++index)
    {
        if (block.mDeviceMemory == sBlocks[index].mDeviceMemory)
        {
            break;
        }
    }

    OCT_ASSERT(index < int32_t(sBlocks.size()));

    vkFreeMemory(GetVulkanDevice(), sBlocks[index].mDeviceMemory, nullptr);
    sBlocks.erase(sBlocks.begin() + index);
}

#endif // API_VULKAN