#if API_VULKAN

#include "Graphics/Vulkan/Image.h"
#include "Graphics/Vulkan/VulkanUtils.h"
#include "Graphics/Vulkan/VulkanContext.h"

#include "Assertion.h"

// TODO: Remove the renderer include
#include "Renderer.h"

Image::Image(ImageDesc imageDesc, SamplerDesc samplerDesc, const char* debugObjectName)
{
    mWidth = imageDesc.mWidth;
    mHeight = imageDesc.mHeight;
    mFormat = imageDesc.mFormat;
    mUsage = imageDesc.mUsage;
    mMipLevels = imageDesc.mMipLevels;
    mLayers = imageDesc.mLayers;

    OCT_ASSERT(mWidth > 0);
    OCT_ASSERT(mHeight > 0);
    OCT_ASSERT(mFormat != VK_FORMAT_UNDEFINED);
    OCT_ASSERT(mUsage != 0);
    OCT_ASSERT(mMipLevels > 0);
    OCT_ASSERT(mLayers > 0);

    mMagFilter = samplerDesc.mMagFilter;
    mMinFilter = samplerDesc.mMinFilter;
    mAddressMode = samplerDesc.mAddressMode;
    mBorderColor = samplerDesc.mBorderColor;
    mMaxAnisotropy = samplerDesc.mMaxAnisotropy;
    mAnisotropyEnable = samplerDesc.mAnisotropyEnable;

    VkDevice device = GetVulkanDevice();

    VkImageAspectFlags aspectFlags = GetFormatImageAspect(mFormat);

    // Image
    VkImageCreateInfo ciImage = {};
    ciImage.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ciImage.imageType = VK_IMAGE_TYPE_2D;
    ciImage.extent.width = mWidth;
    ciImage.extent.height = mHeight;
    ciImage.extent.depth = 1;
    ciImage.mipLevels = mMipLevels;
    ciImage.arrayLayers = mLayers;
    ciImage.format = mFormat;
    ciImage.tiling = VK_IMAGE_TILING_OPTIMAL;
    ciImage.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    ciImage.usage = mUsage;
    ciImage.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ciImage.samples = VK_SAMPLE_COUNT_1_BIT;
    ciImage.flags = 0;

    if (mLayers > 1)
    {
        ciImage.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        ciImage.flags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
    }

    if (vkCreateImage(device, &ciImage, nullptr, &mImage) != VK_SUCCESS)
    {
        LogError("Failed to create image");
        OCT_ASSERT(0);
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, mImage, &memRequirements);
    uint32_t memoryType = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VramAllocator::Alloc(memRequirements.size, memRequirements.alignment, memoryType, mMemory);
    vkBindImageMemory(device, mImage, mMemory.mDeviceMemory, mMemory.mOffset);

    // ImageView
    VkImageViewCreateInfo ciImageView = {};
    ciImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ciImageView.image = mImage;
    ciImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ciImageView.format = mFormat;
    ciImageView.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    ciImageView.subresourceRange.aspectMask = aspectFlags;
    ciImageView.subresourceRange.baseMipLevel = 0;
    ciImageView.subresourceRange.baseArrayLayer = 0;
    ciImageView.subresourceRange.layerCount = mLayers;
    ciImageView.subresourceRange.levelCount = mMipLevels;

    if (vkCreateImageView(device, &ciImageView, nullptr, &mImageView) != VK_SUCCESS)
    {
        LogError("Failed to create image view");
        OCT_ASSERT(0);
    }

    // Sampler
    VkSamplerCreateInfo ciSampler = {};
    ciSampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    ciSampler.magFilter = mMagFilter;
    ciSampler.minFilter = mMinFilter;
    ciSampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    ciSampler.addressModeU = mAddressMode;
    ciSampler.addressModeV = mAddressMode;
    ciSampler.addressModeW = mAddressMode;
    ciSampler.mipLodBias = 0.0f;
    ciSampler.compareEnable = VK_FALSE;
    ciSampler.compareOp = VK_COMPARE_OP_NEVER;
    ciSampler.minLod = 0.0f;
    ciSampler.maxLod = VK_LOD_CLAMP_NONE;
    ciSampler.borderColor = mBorderColor;
    ciSampler.maxAnisotropy = 1.0;
    ciSampler.anisotropyEnable = VK_FALSE;

    if (vkCreateSampler(device, &ciSampler, nullptr, &mSampler) != VK_SUCCESS)
    {
        LogError("Failed to create sampler");
        OCT_ASSERT(0);
    }

    if (debugObjectName != nullptr)
    {
        SetDebugObjectName(VK_OBJECT_TYPE_IMAGE, (uint64_t)mImage, debugObjectName);
        SetDebugObjectName(VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)mImageView, debugObjectName);
        SetDebugObjectName(VK_OBJECT_TYPE_SAMPLER, (uint64_t)mSampler, debugObjectName);
    }
}

Image::~Image()
{
    VkDevice device = GetVulkanDevice();

    vkDestroySampler(device, mSampler, nullptr);
    mSampler = VK_NULL_HANDLE;

    vkDestroyImageView(device, mImageView, nullptr);
    mImageView = VK_NULL_HANDLE;

    vkDestroyImage(device, mImage, nullptr);
    mImage = VK_NULL_HANDLE;

    VramAllocator::Free(mMemory);
}

VkImage Image::Get() const
{
    return mImage;
}

VkImageView Image::GetView() const
{
    return mImageView;
}

VkSampler Image::GetSampler() const
{
    return mSampler;
}

VkFormat Image::GetFormat() const
{
    return mFormat;
}

uint32_t Image::GetWidth() const
{
    return mWidth;
}

uint32_t Image::GetHeight() const
{
    return mHeight;
}

void Image::Update(const void* srcData)
{
    OCT_ASSERT(srcData != nullptr);
    OCT_ASSERT(mImage != VK_NULL_HANDLE);

    uint32_t imageSize = 0;
    if (IsFormatBlockCompressed(mFormat))
    {
        const uint32_t blockSize = 4;
        uint32_t blockWidth = (mWidth + blockSize - 1) / blockSize;
        uint32_t blockHeight = (mHeight + blockSize - 1) / blockSize;
        imageSize = GetFormatBlockSize(mFormat) * blockWidth * blockHeight;
    }
    else
    {
        imageSize = GetFormatPixelSize(mFormat) * mWidth * mHeight;
    }

    if (imageSize == 0)
    {
        LogError("Failed to calculate image size in Image::Update()");
    }

    if (srcData != nullptr &&
        imageSize > 0)
    {
        Buffer* stagingBuffer = new Buffer(BufferType::Transfer, imageSize, "Staging Buffer", srcData);

        VkImageLayout savedLayout = mLayout;
        Transition(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        CopyBufferToImage(stagingBuffer->Get(), mImage, mWidth, mHeight);
        Transition(savedLayout != VK_IMAGE_LAYOUT_PREINITIALIZED ? savedLayout : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        GetDestroyQueue()->Destroy(stagingBuffer);
    }
}

void Image::Transition(VkImageLayout layout, VkCommandBuffer commandBuffer)
{
    // Can't transition to the same layout
    if (layout == mLayout)
        return;

    TransitionImageLayout(
        mImage,
        mFormat,
        mLayout,
        layout,
        mMipLevels,
        mLayers,
        commandBuffer);

    mLayout = layout;
}

void Image::GenerateMips()
{
    VkCommandBuffer blitCmd = BeginCommandBuffer();

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = mImage;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    // Transition whole texture to transfer destination
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = mLayers;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mMipLevels;
    barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(blitCmd,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    // Copy down mips from n-1 to n
    for (uint32_t f = 0; f < mLayers; ++f)
    {
        for (uint32_t i = 1; i < mMipLevels; i++)
        {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(blitCmd,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier);

            VkImageBlit imageBlit{};

            int32_t srcWidth = int32_t(mWidth >> (i - 1));
            int32_t srcHeight = int32_t(mHeight >> (i - 1));
            int32_t dstWidth = int32_t(mWidth >> i);
            int32_t dstHeight = int32_t(mHeight >> i);

            // Source
            imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageBlit.srcSubresource.layerCount = 1;
            imageBlit.srcSubresource.mipLevel = i - 1;
            imageBlit.srcSubresource.baseArrayLayer = f;
            imageBlit.srcOffsets[1].x = srcWidth > 0 ? srcWidth : 1;
            imageBlit.srcOffsets[1].y = srcHeight > 0 ? srcHeight : 1;
            imageBlit.srcOffsets[1].z = 1;

            // Destination
            imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageBlit.dstSubresource.layerCount = 1;
            imageBlit.dstSubresource.mipLevel = i;
            imageBlit.dstSubresource.baseArrayLayer = f;
            imageBlit.dstOffsets[1].x = dstWidth > 0 ? dstWidth : 1;
            imageBlit.dstOffsets[1].y = dstHeight > 0 ? dstHeight : 1;
            imageBlit.dstOffsets[1].z = 1;

            // Blit from previous level
            vkCmdBlitImage(
                blitCmd,
                mImage,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                mImage,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &imageBlit,
                VK_FILTER_LINEAR);

            // Barrier to transition the mip we copied from to SRV layout
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(blitCmd,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier);
        }

        // Transition the last mips layout to Shader Read Only
        barrier.subresourceRange.baseMipLevel = mMipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(blitCmd,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);
    }

    EndCommandBuffer(blitCmd);
}

void Image::Clear(glm::vec4 color)
{
    if (mImage != VK_NULL_HANDLE)
    {
        VkImageLayout originalLayout = mLayout;
        Transition(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        VkCommandBuffer cb = BeginCommandBuffer();

        VkClearColorValue clearValue;
        memset(&clearValue, 0, sizeof(VkClearColorValue));
        clearValue.float32[0] = color.r;
        clearValue.float32[1] = color.g;
        clearValue.float32[2] = color.b;
        clearValue.float32[3] = color.a;

        VkImageSubresourceRange subresourceRange;
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.layerCount = mLayers;
        subresourceRange.levelCount = mMipLevels;

        vkCmdClearColorImage(cb, mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearValue, 1, &subresourceRange);
        EndCommandBuffer(cb);

        Transition(originalLayout != VK_IMAGE_LAYOUT_UNDEFINED ? originalLayout : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
}

#endif