#include "Graphics/Vulkan/VulkanUtils.h"
#include "Graphics/Vulkan/VulkanContext.h"
#include "Graphics/Vulkan/Pipeline.h"

#include "Engine.h"
#include "Renderer.h"
#include "Assets/Font.h"
#include "Assets/Material.h"
#include "Assets/Texture.h"
#include "Assets/StaticMesh.h"
#include "Assets/SkeletalMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/ShadowMeshComponent.h"
#include "Components/TextMeshComponent.h"
#include "Components/ParticleComponent.h"
#include "Widgets/Quad.h"
#include "Widgets/Text.h"
#include "Widgets/Poly.h"
#include "Utilities.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

extern PFN_vkCmdBeginDebugUtilsLabelEXT CmdBeginDebugUtilsLabelEXT;
extern PFN_vkCmdEndDebugUtilsLabelEXT CmdEndDebugUtilsLabelEXT;
extern PFN_vkCmdInsertDebugUtilsLabelEXT CmdInsertDebugUtilsLabelEXT;
extern PFN_vkSetDebugUtilsObjectNameEXT SetDebugUtilsObjectNameEXT;

VkFormat ConvertPixelFormat(PixelFormat pixelFormat)
{
    VkFormat format = VK_FORMAT_UNDEFINED;

    switch (pixelFormat)
    {
    case PixelFormat::LA4: format = VK_FORMAT_R8G8_UNORM; break;
    case PixelFormat::RGB565: format = VK_FORMAT_R5G6B5_UNORM_PACK16; break;
    case PixelFormat::RGBA5551: format = VK_FORMAT_R5G5B5A1_UNORM_PACK16; break;
    case PixelFormat::RGBA8: format = VK_FORMAT_R8G8B8A8_UNORM; break;

#if PLATFORM_ANDROID
    case PixelFormat::CMPR: format = VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK; break;
#else
    case PixelFormat::CMPR: format = VK_FORMAT_BC1_RGBA_UNORM_BLOCK; break;
#endif

    case PixelFormat::R8: format = VK_FORMAT_R8_UNORM; break;
    case PixelFormat::R32U: format = VK_FORMAT_R32_UINT; break;
    case PixelFormat::R32F: format = VK_FORMAT_R32_SFLOAT; break;
    case PixelFormat::RGBA16F: format = VK_FORMAT_R16G16B16A16_SFLOAT; break;

    case PixelFormat::Depth24Stencil8: format = VK_FORMAT_D24_UNORM_S8_UINT; break;
	case PixelFormat::Depth32FStencil8: format = VK_FORMAT_D32_SFLOAT_S8_UINT; break;
    case PixelFormat::Depth16: format = VK_FORMAT_D16_UNORM; break;
    case PixelFormat::Depth32F: format = VK_FORMAT_D32_SFLOAT; break;

    default: break;
    }

    if (format == VK_FORMAT_UNDEFINED)
    {
        LogError("Unsupported PixelFormat in ConvertPixelFormat()");
        OCT_ASSERT(0);
    }

    return format;
}

void CreateBuffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer& buffer,
    Allocation& bufferMemory)
{
    VkDevice device = GetVulkanDevice();

    VkBufferCreateInfo ciBuffer = {};
    ciBuffer.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    ciBuffer.size = size;
    ciBuffer.usage = usage;
    ciBuffer.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ciBuffer.flags = 0;

    if (vkCreateBuffer(device, &ciBuffer, nullptr, &buffer) != VK_SUCCESS)
    {
        LogError("Failed to create vertex buffer");
        OCT_ASSERT(0);
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);
    uint32_t memoryType = FindMemoryType(memRequirements.memoryTypeBits, properties);

    Allocator::Alloc(memRequirements.size, memRequirements.alignment, memoryType, bufferMemory);

    vkBindBufferMemory(device, buffer, bufferMemory.mDeviceMemory, bufferMemory.mOffset);
}

void TransitionImageLayout(
    VkImage image,
    VkFormat format,
    VkImageLayout oldLayout,
    VkImageLayout newLayout,
    int32_t mipLevels,
    int32_t layerCount,
    VkCommandBuffer commandBuffer)
{
    VkCommandBuffer singleCb = VK_NULL_HANDLE;

    if (commandBuffer == VK_NULL_HANDLE)
    {
        singleCb = BeginCommandBuffer();
        commandBuffer = singleCb;
    }

    VkPipelineStageFlags srcMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkPipelineStageFlags dstMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = layerCount;

    if (format == VK_FORMAT_D16_UNORM ||
        format == VK_FORMAT_D32_SFLOAT)
    {
        // Shadow maps are a depth-only format. Do not use stencil bit.
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    else if (format == VK_FORMAT_D24_UNORM_S8_UINT ||
		format == VK_FORMAT_D32_SFLOAT_S8_UINT)
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    else
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR &&
        newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    }
    else
    {
        //throw std::invalid_argument("Unsupported layout transition!");
        barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    }

    vkCmdPipelineBarrier(commandBuffer,
        srcMask,
        dstMask,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &barrier);

    if (singleCb != VK_NULL_HANDLE)
    {
        EndCommandBuffer(commandBuffer);
    }
}

void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = BeginCommandBuffer();

    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    EndCommandBuffer(commandBuffer);
}

void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer = BeginCommandBuffer();

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { width, height, 1 };

    vkCmdCopyBufferToImage(commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region);


    EndCommandBuffer(commandBuffer);
}

uint32_t GetFrameIndex()
{
    return GetVulkanContext()->GetFrameIndex();
}

DestroyQueue* GetDestroyQueue()
{
    return GetVulkanContext()->GetDestroyQueue();
}

VkDevice GetVulkanDevice()
{
    return GetVulkanContext()->GetDevice();
}

VkCommandBuffer GetCommandBuffer()
{
    return GetVulkanContext()->GetCommandBuffer();
}

void DeviceWaitIdle()
{
    vkDeviceWaitIdle(GetVulkanDevice());
}

VkCommandBuffer BeginCommandBuffer()
{
    VkDevice device = GetVulkanDevice();
    VkCommandPool commandPool = GetVulkanContext()->GetCommandPool();

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    SetDebugObjectName(VK_OBJECT_TYPE_COMMAND_BUFFER, (uint64_t)commandBuffer, "SingleCommandBuffer");

    return commandBuffer;
}

void EndCommandBuffer(VkCommandBuffer commandBuffer)
{
    VkQueue graphicsQueue = GetVulkanContext()->GetGraphicsQueue();

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);

    GetDestroyQueue()->Destroy(commandBuffer);
}

uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDevice physicalDevice = GetVulkanContext()->GetPhysicalDevice();
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
    {
        if (typeFilter & (1 << i) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    LogError("Failed to find suitable memory type");
    OCT_ASSERT(0);
    return 0;
}

uint32_t GetFormatPixelSize(VkFormat format)
{
    // Return size in bytes per pixel
    uint32_t size = 0;

    switch (format)
    {
    case VK_FORMAT_R8_UNORM: size = 1; break;
    case VK_FORMAT_R8G8_UNORM: size = 2; break;
    case VK_FORMAT_R5G6B5_UNORM_PACK16: size = 2; break;
    case VK_FORMAT_R5G5B5A1_UNORM_PACK16: size = 2; break;
    case VK_FORMAT_R8G8B8A8_UNORM: size = 4; break;

    case VK_FORMAT_R32_UINT: size = 4; break;
    case VK_FORMAT_R32_SFLOAT: size = 4; break;
    case VK_FORMAT_R16G16B16A16_SFLOAT: size = 8; break;

    // Depth
    case VK_FORMAT_D24_UNORM_S8_UINT: size = 4; break;
	case VK_FORMAT_D32_SFLOAT_S8_UINT: OCT_ASSERT(0); break; // Not sure how to handle this?
    case VK_FORMAT_D16_UNORM: size = 2; break;
    case VK_FORMAT_D32_SFLOAT: size = 4; break;

    default: break;
    }

    if (size == 0)
    {
        LogError("Unsupported format pixel size queried");
        OCT_ASSERT(0);
    }

    return size;
}

uint32_t GetFormatBlockSize(VkFormat format)
{
    uint32_t size = 0;

    switch (format)
    {
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK: size = 8; break;
    case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK: size = 8; break;
    default: break;
    }

    if (size == 0)
    {
        LogError("Unsupported format block size queried");
        OCT_ASSERT(0);
    }

    return size;
}

bool IsFormatBlockCompressed(VkFormat format)
{
    // Will probably only support BC1 and ETC2 formats.
    // Trying to keep things simple for now.
    // Desktop -> BC1
    // Android -> ETC2
    bool isCompressed =
        format == VK_FORMAT_BC1_RGBA_UNORM_BLOCK ||
        format == VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;

    return isCompressed;
}

VkImageAspectFlags GetFormatImageAspect(VkFormat format)
{
    VkImageAspectFlags flags = 0;

    switch (format)
    {
    case VK_FORMAT_D24_UNORM_S8_UINT:
	case VK_FORMAT_D32_SFLOAT_S8_UINT:
        flags |= VK_IMAGE_ASPECT_STENCIL_BIT;
        // Intentional fallthrough
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_D32_SFLOAT:
        flags |= VK_IMAGE_ASPECT_DEPTH_BIT;
        break;
    default:
        flags |= VK_IMAGE_ASPECT_COLOR_BIT; 
        break;
    }

    return flags;
}

VkVertexInputBindingDescription GetVertexBindingDescription(VertexType type)
{
    VkVertexInputBindingDescription desc = {};

    switch (type)
    {
    case VertexType::Vertex:
        desc.stride = sizeof(Vertex);
        break;
    case VertexType::VertexColor:
        desc.stride = sizeof(VertexColor);
        break;
    case VertexType::VertexUI:
        desc.stride = sizeof(VertexUI);
        break;
    case VertexType::VertexColorSimple:
        desc.stride = sizeof(VertexColorSimple);
        break;
    case VertexType::VertexSkinned:
        desc.stride = sizeof(VertexSkinned);
        break;
    case VertexType::VertexParticle:
        desc.stride = sizeof(VertexParticle);
        break;

    default: OCT_ASSERT(0); break;
    }

    desc.binding = 0;
    desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return desc;
}

std::vector<VkVertexInputAttributeDescription> GetVertexAttributeDescriptions(VertexType type)
{
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

    switch (type)
    {
    case VertexType::Vertex:
        // Position
        attributeDescriptions.push_back(VkVertexInputAttributeDescription());
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, mPosition);
        // Texcoord0
        attributeDescriptions.push_back(VkVertexInputAttributeDescription());
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, mTexcoord0);
        // Texcoord1
        attributeDescriptions.push_back(VkVertexInputAttributeDescription());
        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, mTexcoord1);
        // Normal
        attributeDescriptions.push_back(VkVertexInputAttributeDescription());
        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Vertex, mNormal);
        break;

    case VertexType::VertexColor:
        // Position
        attributeDescriptions.push_back(VkVertexInputAttributeDescription());
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(VertexColor, mPosition);
        // Texcoord0
        attributeDescriptions.push_back(VkVertexInputAttributeDescription());
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(VertexColor, mTexcoord0);
        // Texcoord1
        attributeDescriptions.push_back(VkVertexInputAttributeDescription());
        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(VertexColor, mTexcoord1);
        // Normal
        attributeDescriptions.push_back(VkVertexInputAttributeDescription());
        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(VertexColor, mNormal);
        // Color
        attributeDescriptions.push_back(VkVertexInputAttributeDescription());
        attributeDescriptions[4].binding = 0;
        attributeDescriptions[4].location = 4;
        attributeDescriptions[4].format = VK_FORMAT_R8G8B8A8_UNORM;
        attributeDescriptions[4].offset = offsetof(VertexColor, mColor);
        break;

    case VertexType::VertexUI:
        // Position
        attributeDescriptions.push_back(VkVertexInputAttributeDescription());
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(VertexUI, mPosition);
        // Texcoord
        attributeDescriptions.push_back(VkVertexInputAttributeDescription());
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(VertexUI, mTexcoord);
        // Color
        attributeDescriptions.push_back(VkVertexInputAttributeDescription());
        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R8G8B8A8_UNORM;
        attributeDescriptions[2].offset = offsetof(VertexUI, mColor);
        break;

    case VertexType::VertexColorSimple:
        // Position
        attributeDescriptions.push_back(VkVertexInputAttributeDescription());
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(VertexColorSimple, mPosition);
        // Color
        attributeDescriptions.push_back(VkVertexInputAttributeDescription());
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R8G8B8A8_UNORM;
        attributeDescriptions[1].offset = offsetof(VertexColorSimple, mColor);
        break;

    case VertexType::VertexSkinned:
        // Position
        attributeDescriptions.push_back(VkVertexInputAttributeDescription());
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(VertexSkinned, mPosition);
        // Texcoord0
        attributeDescriptions.push_back(VkVertexInputAttributeDescription());
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(VertexSkinned, mTexcoord0);
        // Texcoord1
        attributeDescriptions.push_back(VkVertexInputAttributeDescription());
        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(VertexSkinned, mTexcoord1);
        // Normal
        attributeDescriptions.push_back(VkVertexInputAttributeDescription());
        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(VertexSkinned, mNormal);
        // Bone Indices
        attributeDescriptions.push_back(VkVertexInputAttributeDescription());
        attributeDescriptions[4].binding = 0;
        attributeDescriptions[4].location = 4;
        attributeDescriptions[4].format = VK_FORMAT_R8G8B8A8_UINT;
        attributeDescriptions[4].offset = offsetof(VertexSkinned, mBoneIndices);
        // Bone Weights
        attributeDescriptions.push_back(VkVertexInputAttributeDescription());
        attributeDescriptions[5].binding = 0;
        attributeDescriptions[5].location = 5;
        attributeDescriptions[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[5].offset = offsetof(VertexSkinned, mBoneWeights);
        break;

    case VertexType::VertexParticle:
        // Position
        attributeDescriptions.push_back(VkVertexInputAttributeDescription());
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(VertexParticle, mPosition);
        // Texcoord
        attributeDescriptions.push_back(VkVertexInputAttributeDescription());
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(VertexParticle, mTexcoord);
        // Color
        attributeDescriptions.push_back(VkVertexInputAttributeDescription());
        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R8G8B8A8_UNORM;
        attributeDescriptions[2].offset = offsetof(VertexParticle, mColor);
        break;

    default: OCT_ASSERT(0); break;
    }

    return attributeDescriptions;
}

void SetDebugObjectName(VkObjectType objectType, uint64_t object, const char* name)
{
#if _DEBUG
    if (GetVulkanContext()->IsValidationEnabled())
    {
        // Set a name for the command buffer
        VkDebugUtilsObjectNameInfoEXT cmdBufferNameInfo = {};
        cmdBufferNameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        cmdBufferNameInfo.pNext = nullptr;
        cmdBufferNameInfo.objectType = objectType;
        cmdBufferNameInfo.objectHandle = object;
        cmdBufferNameInfo.pObjectName = name;

        SetDebugUtilsObjectNameEXT(GetVulkanDevice(), &cmdBufferNameInfo);
    }
#endif
}

void BeginDebugLabel(const char* name, glm::vec4 color)
{
#if _DEBUG
	if (GetVulkanContext()->IsValidationEnabled())
	{
		VkDebugUtilsLabelEXT label;
		label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
		label.pNext = nullptr;
		label.pLabelName = name;
		label.color[0] = color.r;
		label.color[1] = color.g;
		label.color[2] = color.b;
		label.color[3] = color.a;

		CmdBeginDebugUtilsLabelEXT(GetCommandBuffer(), &label);
	}
#endif
}

void EndDebugLabel()
{
#if _DEBUG
	if (GetVulkanContext()->IsValidationEnabled())
	{
		CmdEndDebugUtilsLabelEXT(GetCommandBuffer());
	}
#endif
}

void WriteGeometryUniformData(GeometryData& outData, World* world, const glm::mat4& transform)
{
    CameraComponent* camera = world->GetActiveCamera();
    DirectionalLightComponent* dirLight = world->GetDirectionalLight();

    outData.mWVPMatrix = camera->GetViewProjectionMatrix() * transform;
    outData.mWorldMatrix = transform;
    outData.mNormalMatrix = glm::transpose(glm::inverse(transform));
    outData.mLightWVPMatrix = dirLight ? (dirLight->GetViewProjectionMatrix() * transform) : glm::mat4(1);
    outData.mColor = glm::vec4(0.25f, 0.25f, 1.0f, 1.0f);
    outData.mHitCheckId = 0;
}

#if _DEBUG
void FullPipelineBarrier()
{
    // This should only be called for debugging.
    VkMemoryBarrier memoryBarrier = {};
    memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    memoryBarrier.pNext = nullptr;
    memoryBarrier.srcAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT |
        VK_ACCESS_INDEX_READ_BIT |
        VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT |
        VK_ACCESS_UNIFORM_READ_BIT |
        VK_ACCESS_INPUT_ATTACHMENT_READ_BIT |
        VK_ACCESS_SHADER_READ_BIT |
        VK_ACCESS_SHADER_WRITE_BIT |
        VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_TRANSFER_READ_BIT |
        VK_ACCESS_TRANSFER_WRITE_BIT |
        VK_ACCESS_HOST_READ_BIT |
        VK_ACCESS_HOST_WRITE_BIT,
        memoryBarrier.dstAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT |
        VK_ACCESS_INDEX_READ_BIT |
        VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT |
        VK_ACCESS_UNIFORM_READ_BIT |
        VK_ACCESS_INPUT_ATTACHMENT_READ_BIT |
        VK_ACCESS_SHADER_READ_BIT |
        VK_ACCESS_SHADER_WRITE_BIT |
        VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_TRANSFER_READ_BIT |
        VK_ACCESS_TRANSFER_WRITE_BIT |
        VK_ACCESS_HOST_READ_BIT |
        VK_ACCESS_HOST_WRITE_BIT;

    vkCmdPipelineBarrier(GetCommandBuffer(),
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, // srcStageMask
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, // dstStageMask
        0,
        1,                                  // memoryBarrierCount
        &memoryBarrier,                     // pMemoryBarriers
        0, nullptr,
        0, nullptr);
}
#endif

void CreateTextureResource(Texture* texture, uint8_t* pixels)
{
    TextureResource* resource = texture->GetResource();

    // TODO: Handle other pixel formats
    VkFormat format = ConvertPixelFormat(PixelFormat::RGBA8 /*texture->GetFormat()*/);

    ImageDesc imageDesc;
    imageDesc.mWidth = texture->GetWidth();
    imageDesc.mHeight = texture->GetHeight();
    imageDesc.mFormat = format;
    imageDesc.mUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageDesc.mMipLevels = texture->GetMipLevels();
    imageDesc.mLayers = texture->GetLayers();

    SamplerDesc samplerDesc;

    FilterType filterType = texture->GetFilterType();
    WrapMode wrapMode = texture->GetWrapMode();

    switch (filterType)
    {
    case FilterType::Nearest:
        samplerDesc.mMagFilter = VK_FILTER_NEAREST;
        samplerDesc.mMinFilter = VK_FILTER_NEAREST;
        break;
    case FilterType::Linear:
        samplerDesc.mMagFilter = VK_FILTER_LINEAR;
        samplerDesc.mMinFilter = VK_FILTER_LINEAR;
        break;
    default: break;
    }

    switch (wrapMode)
    {
    case WrapMode::Clamp:
        samplerDesc.mAddressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        break;
    case WrapMode::Repeat:
        samplerDesc.mAddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        break;
    case WrapMode::Mirror:
        samplerDesc.mAddressMode = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        break;
    default: break;
    }

    resource->mImage = new Image(imageDesc, samplerDesc, "Texture (Asset)");

    if (pixels != nullptr)
    {
        resource->mImage->Update(pixels);
    }
    else
    {
        resource->mImage->Clear(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
    }

    if (texture->IsMipmapped())
    {
        resource->mImage->GenerateMips();
    }
}

void DestroyTextureResource(Texture* texture)
{
    TextureResource* resource = texture->GetResource();

    if (resource->mImage != nullptr)
    {
        GetDestroyQueue()->Destroy(resource->mImage);
        resource->mImage = nullptr;
    }
}

void CreateMaterialResource(Material* material)
{
    MaterialResource* resource = material->GetResource();
    VkDescriptorSetLayout layout = GetVulkanContext()->GetPipeline(PipelineId::Opaque)->GetDescriptorSetLayout((uint32_t)DescriptorSetBinding::Material);

    resource->mUniformBuffer = new UniformBuffer(sizeof(MaterialData), "Material Uniforms");
    resource->mDescriptorSet = new DescriptorSet(layout);

    UpdateMaterialResource(material);
}

void DestroyMaterialResource(Material* material)
{
    MaterialResource* resource = material->GetResource();

    if (resource->mUniformBuffer != nullptr)
    {
        GetDestroyQueue()->Destroy(resource->mUniformBuffer);
        resource->mUniformBuffer = nullptr;
    }

    if (resource->mDescriptorSet != nullptr)
    {
        GetDestroyQueue()->Destroy(resource->mDescriptorSet);
        resource->mDescriptorSet = nullptr;
    }
}

void BindMaterialResource(Material* material, Pipeline* pipeline)
{
    MaterialResource* resource = material->GetResource();
    VkCommandBuffer cb = GetCommandBuffer();

    if (material->IsDirty(GetFrameIndex()))
    {
        UpdateMaterialResource(material);
    }

    resource->mDescriptorSet->Bind(cb, (uint32_t)DescriptorSetBinding::Material, pipeline->GetPipelineLayout());
}

void UpdateMaterialResource(Material* material)
{
    MaterialResource* resource = material->GetResource();

    Texture* textures[4] = {};
    textures[0] = material->GetTexture((TextureSlot)0);
    textures[1] = material->GetTexture((TextureSlot)1);
    textures[2] = material->GetTexture((TextureSlot)2);
    textures[3] = material->GetTexture((TextureSlot)3);

    // Update uniform buffer data
    MaterialData ubo = {};
    ubo.mUvOffset0 = material->GetUvOffset(0);
    ubo.mUvScale0 = material->GetUvScale(0);
    ubo.mUvOffset1 = material->GetUvOffset(1);
    ubo.mUvScale1 = material->GetUvScale(1);
    ubo.mColor = material->GetColor();
    ubo.mFresnelColor = material->GetFresnelColor();
    ubo.mShadingModel = static_cast<uint32_t>(material->GetShadingModel());
    ubo.mBlendMode = static_cast<uint32_t>(material->GetBlendMode());
    ubo.mToonSteps = material->GetToonSteps();
    ubo.mFresnelPower = material->GetFresnelPower();
    ubo.mSpecular = material->GetSpecular();
    ubo.mOpacity = material->GetOpacity();
    ubo.mMaskCutoff = material->GetMaskCutoff();
    ubo.mShininess = material->GetShininess();
    ubo.mFresnelEnabled = static_cast<uint32_t>(material->IsFresnelEnabled());
    ubo.mVertexColorMode = static_cast<uint32_t>(material->GetVertexColorMode());
    ubo.mApplyFog = static_cast<uint32_t>(material->ShouldApplyFog());
    ubo.mUvMaps[0] = material->GetUvMap(0);
    ubo.mUvMaps[1] = material->GetUvMap(1);
    ubo.mUvMaps[2] = material->GetUvMap(2);
    ubo.mUvMaps[3] = material->GetUvMap(3);
    ubo.mTevModes[0] = textures[0] ? (uint32_t) material->GetTevMode(0) : (uint32_t)TevMode::Count;
    ubo.mTevModes[1] = textures[1] ? (uint32_t) material->GetTevMode(1) : (uint32_t)TevMode::Count;
    ubo.mTevModes[2] = textures[2] ? (uint32_t) material->GetTevMode(2) : (uint32_t)TevMode::Count;
    ubo.mTevModes[3] = textures[3] ? (uint32_t) material->GetTevMode(3) : (uint32_t)TevMode::Count;

    resource->mUniformBuffer->Update(&ubo, sizeof(ubo));

    // Update descriptor bindings
    Renderer* renderer = Renderer::Get();
    OCT_ASSERT(resource->mDescriptorSet != nullptr);
    resource->mDescriptorSet->UpdateUniformDescriptor(MD_UNIFORM_BUFFER, resource->mUniformBuffer);

    for (uint32_t i = 0; i < MATERIAL_MAX_TEXTURES; ++i)
    {
        Texture* texture = textures[i];
        if (texture == nullptr)
        {
            texture = renderer->mWhiteTexture.Get<Texture>();
            OCT_ASSERT(texture != nullptr);
        }

        resource->mDescriptorSet->UpdateImageDescriptor(MD_TEXTURE_0 + i, texture->GetResource()->mImage);
    }

    material->ClearDirty(GetFrameIndex());
}

Pipeline* GetMaterialPipeline(Material* material)
{
    Pipeline* pipeline = nullptr;
    VulkanContext* context = GetVulkanContext();
    bool depthless = material->IsDepthTestDisabled();

    // Right now, only blend mode determines pipeline selection.
    // After adding vertex coloured meshes or skeletal meshes, pipeline
    // selection may become slightly more complicated.
    // 
    // Update: Skeletal/Coloured vertex permutations are stored internally inside Pipeline
    // but depthless is stored as separate PipelineId.
    // If we have to add another permutation... we'll need to create some sort of 
    // pipeline cache system, and only create them as needed.

    PipelineId pipelineId = PipelineId::Count;

    switch (material->GetBlendMode())
    {
    case BlendMode::Opaque:
    case BlendMode::Masked:
        pipelineId = depthless ? PipelineId::DepthlessOpaque : PipelineId::Opaque;
        break;
    case BlendMode::Translucent:
        pipelineId = depthless ? PipelineId::DepthlessTranslucent : PipelineId::Translucent;
        break;
    case BlendMode::Additive:
        pipelineId = depthless ? PipelineId::DepthlessAdditive : PipelineId::Additive;
        break;

    default:
        pipeline = context->GetPipeline(PipelineId::Opaque);
        break;
    }

    if (pipelineId != PipelineId::Count)
    {
        pipeline = context->GetPipeline(pipelineId);
    }

    return pipeline;
}

void CreateStaticMeshResource(StaticMesh* staticMesh, bool hasColor, uint32_t numVertices, void* vertices, uint32_t numIndices, IndexType* indices)
{
    StaticMeshResource* resource = staticMesh->GetResource();

    uint32_t vertexSize = hasColor ? sizeof(VertexColor) : sizeof(Vertex);
    resource->mVertexBuffer = new Buffer(BufferType::Vertex, numVertices * vertexSize, "StaticMesh Vertices", vertices, false);
    resource->mIndexBuffer = new Buffer(BufferType::Index, numIndices * sizeof(IndexType), "StaticMesh Indices", indices, false);
}

void DestroyStaticMeshResource(StaticMesh* staticMesh)
{
    StaticMeshResource* resource = staticMesh->GetResource();

    if (resource->mVertexBuffer != nullptr)
    {
        GetDestroyQueue()->Destroy(resource->mVertexBuffer);
        resource->mVertexBuffer = nullptr;
    }

    if (resource->mIndexBuffer != nullptr)
    {
        GetDestroyQueue()->Destroy(resource->mIndexBuffer);
        resource->mIndexBuffer = nullptr;
    }
}

void BindStaticMeshResource(StaticMesh* staticMesh)
{
    StaticMeshResource* resource = staticMesh->GetResource();

    VkCommandBuffer cb = GetCommandBuffer();
    VkBuffer vertexBuffers[] = { resource->mVertexBuffer->Get() };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(cb, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(cb, resource->mIndexBuffer->Get(), 0, VK_INDEX_TYPE_UINT32);
}

void CreateSkeletalMeshResource(SkeletalMesh* skeletalMesh, uint32_t numVertices, VertexSkinned* vertices, uint32_t numIndices, IndexType* indices)
{
    SkeletalMeshResource* resource = skeletalMesh->GetResource();
    resource->mVertexBuffer = new Buffer(BufferType::Vertex, sizeof(VertexSkinned) * numVertices, "SkeletalMesh Vertices", vertices, true);
    resource->mIndexBuffer = new Buffer(BufferType::Index, numIndices * sizeof(uint32_t), "SkeletalMesh Indices", indices, false);
}

void DestroySkeletalMeshResource(SkeletalMesh* skeletalMesh)
{
    SkeletalMeshResource* resource = skeletalMesh->GetResource();

    if (resource->mVertexBuffer != nullptr)
    {
        GetDestroyQueue()->Destroy(resource->mVertexBuffer);
        resource->mVertexBuffer = nullptr;
    }

    if (resource->mIndexBuffer != nullptr)
    {
        GetDestroyQueue()->Destroy(resource->mIndexBuffer);
        resource->mIndexBuffer = nullptr;
    }
}

void BindSkeletalMeshResource(SkeletalMesh* skeletalMesh)
{
    SkeletalMeshResource* resource = skeletalMesh->GetResource();

    VkCommandBuffer cb = GetCommandBuffer();
    VkBuffer vertexBuffers[] = { resource->mVertexBuffer->Get() };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(cb, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(cb, resource->mIndexBuffer->Get(), 0, VK_INDEX_TYPE_UINT32);
}

void BindSkeletalMeshResourceIndices(SkeletalMesh* skeletalMesh)
{
    SkeletalMeshResource* resource = skeletalMesh->GetResource();

    VkCommandBuffer cb = GetCommandBuffer();
    vkCmdBindIndexBuffer(cb, resource->mIndexBuffer->Get(), 0, VK_INDEX_TYPE_UINT32);
}

void CreateStaticMeshCompResource(StaticMeshComponent* staticMeshComp)
{
    StaticMeshCompResource* resource = staticMeshComp->GetResource();
    VkDescriptorSetLayout layout = GetVulkanContext()->GetPipeline(PipelineId::Opaque)->GetDescriptorSetLayout((uint32_t)DescriptorSetBinding::Geometry);

    resource->mUniformBuffer = new UniformBuffer(sizeof(GeometryData), "Geometry Uniforms");
    resource->mDescriptorSet = new DescriptorSet(layout);

    resource->mDescriptorSet->UpdateUniformDescriptor(GD_UNIFORM_BUFFER, resource->mUniformBuffer);
}

void DestroyStaticMeshCompResource(StaticMeshComponent* staticMeshComp)
{
    StaticMeshCompResource* resource = staticMeshComp->GetResource();

    if (resource->mUniformBuffer != nullptr)
    {
        GetDestroyQueue()->Destroy(resource->mUniformBuffer);
        resource->mUniformBuffer = nullptr;
    }

    if (resource->mDescriptorSet != nullptr)
    {
        GetDestroyQueue()->Destroy(resource->mDescriptorSet);
        resource->mDescriptorSet = nullptr;
    }
}

uint32_t GetHitCheckId(TransformComponent* comp)
{
    uint32_t retId = 0;
#if EDITOR
    Actor* owner = comp->GetOwner();
    uint32_t actorId = owner->GetHitCheckId();
    uint32_t compId = 0;

    for (uint32_t i = 0; i < owner->GetNumComponents(); ++i)
    {
        if (owner->GetComponent(i) == comp)
        {
            compId = i;
            break;
        }
    }

    retId = (actorId << 16) | compId;
#endif
    return retId;
}

void UpdateStaticMeshCompResource(StaticMeshComponent* staticMeshComp)
{
    StaticMeshCompResource* resource = staticMeshComp->GetResource();

    Renderer* renderer = Renderer::Get();
    OCT_UNUSED(renderer);

    World* world = staticMeshComp->GetWorld();
    GeometryData ubo = {};

    WriteGeometryUniformData(ubo, world, staticMeshComp->GetRenderTransform());

    ubo.mHitCheckId = GetHitCheckId(staticMeshComp);

#if EDITOR
    if (renderer->GetDebugMode() == DEBUG_WIREFRAME &&
        world->IsComponentSelected(staticMeshComp))
    {
        if (world->GetSelectedComponent() == staticMeshComp)
        {
            ubo.mColor = SELECTED_COMP_COLOR;
        }
        else
        {
            ubo.mColor = MULTI_SELECTED_COMP_COLOR;
        }
    }
#endif

    resource->mUniformBuffer->Update(&ubo, sizeof(ubo));
}

void DrawStaticMeshComp(StaticMeshComponent* staticMeshComp, StaticMesh* meshOverride)
{
    StaticMesh* mesh = meshOverride ? meshOverride : staticMeshComp->GetStaticMesh();
    StaticMeshCompResource* resource = staticMeshComp->GetResource();

    if (mesh != nullptr)
    {
        VkCommandBuffer cb = GetCommandBuffer();

        UpdateStaticMeshCompResource(staticMeshComp);

        BindStaticMeshResource(mesh);

        Material* material = staticMeshComp->GetMaterial();

        if (material == nullptr)
        {
            material = Renderer::Get()->GetDefaultMaterial();
            OCT_ASSERT(material != nullptr);
        }

        Pipeline* pipeline = nullptr;
        if (GetVulkanContext()->GetCurrentRenderPassId() == RenderPassId::Forward)
        {
            // During the Forward pass, it is expected that that the material sets the necessary pipeline.
            // This could be moved up to the Renderer in the future to reduce CPU cost.
            pipeline = GetMaterialPipeline(material);
            GetVulkanContext()->BindPipeline(pipeline, mesh->HasVertexColor() ? VertexType::VertexColor : VertexType::Vertex);
        }
        else
        {
            pipeline = GetVulkanContext()->GetCurrentlyBoundPipeline();
            GetVulkanContext()->RebindPipeline(mesh->HasVertexColor() ? VertexType::VertexColor : VertexType::Vertex);
        }

        OCT_ASSERT(pipeline);

        BindMaterialResource(material, pipeline);
        resource->mDescriptorSet->Bind(cb, (uint32_t)DescriptorSetBinding::Geometry, pipeline->GetPipelineLayout());

        vkCmdDrawIndexed(cb,
            mesh->GetNumIndices(),
            1,
            0,
            0,
            0);
    }
}

void CreateSkeletalMeshCompResource(SkeletalMeshComponent* skeletalMeshComp)
{
    SkeletalMeshCompResource* resource = skeletalMeshComp->GetResource();

    size_t bufferSize = IsCpuSkinningRequired(skeletalMeshComp) ? sizeof(GeometryData) : sizeof(SkinnedGeometryData);
    resource->mUniformBuffer = new UniformBuffer(bufferSize, "Skinned Geometry Uniforms");

    VkDescriptorSetLayout layout = GetVulkanContext()->GetPipeline(PipelineId::Opaque)->GetDescriptorSetLayout(1);
    resource->mDescriptorSet = new DescriptorSet(layout);

    resource->mDescriptorSet->UpdateUniformDescriptor(GD_UNIFORM_BUFFER, resource->mUniformBuffer);
}

void DestroySkeletalMeshCompResource(SkeletalMeshComponent* skeletalMeshComp)
{
    SkeletalMeshCompResource* resource = skeletalMeshComp->GetResource();

    if (resource->mUniformBuffer != nullptr)
    {
        GetDestroyQueue()->Destroy(resource->mUniformBuffer);
        resource->mUniformBuffer = nullptr;
    }

    if (resource->mDescriptorSet != nullptr)
    {
        GetDestroyQueue()->Destroy(resource->mDescriptorSet);
        resource->mDescriptorSet = nullptr;
    }

    if (resource->mVertexBuffer != nullptr)
    {
        GetDestroyQueue()->Destroy(resource->mVertexBuffer);
        resource->mVertexBuffer = nullptr;
    }
}

void ReallocateSkeletalMeshCompVertexBuffer(SkeletalMeshComponent* skeletalMeshComp, uint32_t numVertices)
{
    SkeletalMeshCompResource* resource = skeletalMeshComp->GetResource();

    if (IsCpuSkinningRequired(skeletalMeshComp))
    {
        if (resource->mVertexBuffer != nullptr)
        {
            GetDestroyQueue()->Destroy(resource->mVertexBuffer);
            resource->mVertexBuffer = nullptr;
        }

        resource->mVertexBuffer = new Buffer(BufferType::Vertex, numVertices * sizeof(Vertex), "SkeletalMeshComponent Skinned Vertices");
    }
}

void UpdateSkeletalMeshCompVertexBuffer(SkeletalMeshComponent* skeletalMeshComp, const std::vector<Vertex>& skinnedVertices)
{
    SkeletalMeshCompResource* resource = skeletalMeshComp->GetResource();

    if (resource->mVertexBuffer != nullptr)
    {
        OCT_ASSERT(resource->mVertexBuffer->GetSize() == skinnedVertices.size() * sizeof(Vertex));
        resource->mVertexBuffer->Update(skinnedVertices.data(), skinnedVertices.size() * sizeof(Vertex), 0);
    }
}

void UpdateSkeletalMeshCompUniformBuffer(SkeletalMeshComponent* skeletalMeshComp)
{
    SkeletalMeshCompResource* resource = skeletalMeshComp->GetResource();

    Renderer* renderer = Renderer::Get();
    OCT_UNUSED(renderer);

    World* world = skeletalMeshComp->GetWorld();
    CameraComponent* camera = world->GetActiveCamera();
    DirectionalLightComponent* dirLight = world->GetDirectionalLight();
    uint32_t numBoneInfluences = 1;

    switch (skeletalMeshComp->GetBoneInfluenceMode())
    {
    case BoneInfluenceMode::One: numBoneInfluences = 1; break;
    case BoneInfluenceMode::Four: numBoneInfluences = 4; break;
    default: numBoneInfluences = 1; break;
    }

    glm::mat4 transform = skeletalMeshComp->GetRenderTransform();
    glm::vec4 uniformColor = glm::vec4(0);

#if EDITOR
    if (renderer->GetDebugMode() == DEBUG_WIREFRAME)
    {
        uniformColor = glm::vec4(0.25f, 0.25f, 1.0f, 1.0f);

        if (world->IsComponentSelected(skeletalMeshComp))
        {
            if (world->GetSelectedComponent() == skeletalMeshComp)
            {
                uniformColor = SELECTED_COMP_COLOR;
            }
            else
            {
                uniformColor = MULTI_SELECTED_COMP_COLOR;
            }
        }
    }
#endif

    if (!IsCpuSkinningRequired(skeletalMeshComp))
    {
        SkinnedGeometryData ubo = {};
        ubo.mBase.mWVPMatrix = camera->GetViewProjectionMatrix() * transform;
        ubo.mBase.mWorldMatrix = transform;
        ubo.mBase.mNormalMatrix = glm::transpose(glm::inverse(transform));
        ubo.mBase.mLightWVPMatrix = dirLight ? (dirLight->GetViewProjectionMatrix() * transform) : glm::mat4(1);
        ubo.mBase.mColor = uniformColor; // Currently used for wireframe only.
        ubo.mBase.mHitCheckId = GetHitCheckId(skeletalMeshComp);

        for (uint32_t i = 0; i < skeletalMeshComp->GetNumBones(); ++i)
        {
            ubo.mBoneMatrices[i] = skeletalMeshComp->GetBoneTransform(i);
        }
        ubo.mNumBoneInfluences = numBoneInfluences;

        resource->mUniformBuffer->Update(&ubo, sizeof(ubo));
    }
    else
    {
        GeometryData ubo = {};
        ubo.mWVPMatrix = camera->GetViewProjectionMatrix() * transform;
        ubo.mWorldMatrix = transform;
        ubo.mNormalMatrix = glm::transpose(glm::inverse(transform));
        ubo.mLightWVPMatrix = dirLight ? (dirLight->GetViewProjectionMatrix() * transform) : glm::mat4(1);
        ubo.mColor = uniformColor; // Currently used for wireframe only.
        ubo.mHitCheckId = GetHitCheckId(skeletalMeshComp);

        resource->mUniformBuffer->Update(&ubo, sizeof(ubo));
    }
}

void DrawSkeletalMeshComp(SkeletalMeshComponent* skeletalMeshComp)
{
    SkeletalMeshCompResource* resource = skeletalMeshComp->GetResource();
    SkeletalMesh* mesh = skeletalMeshComp->GetSkeletalMesh();

    if (mesh != nullptr)
    {
        VkCommandBuffer cb = GetCommandBuffer();

        UpdateSkeletalMeshCompUniformBuffer(skeletalMeshComp);

        if (IsCpuSkinningRequired(skeletalMeshComp))
        {
            VkDeviceSize offset = 0;
            VkBuffer vertexBuffer = resource->mVertexBuffer->Get();
            vkCmdBindVertexBuffers(cb, 0, 1, &vertexBuffer, &offset);

            BindSkeletalMeshResourceIndices(mesh);
        }
        else
        {
            BindSkeletalMeshResource(mesh);
        }

        Material* material = skeletalMeshComp->GetMaterial();

        if (material == nullptr)
        {
            material = Renderer::Get()->GetDefaultMaterial();
            OCT_ASSERT(material != nullptr);
        }

        Pipeline* pipeline = nullptr;
        if (GetVulkanContext()->GetCurrentRenderPassId() == RenderPassId::Forward)
        {
            // During the Forward pass, it is expected that that the material sets the necessary pipeline.
            // This could be moved up to the Renderer in the future to reduce CPU cost.
            pipeline = GetMaterialPipeline(material);
            GetVulkanContext()->BindPipeline(pipeline, IsCpuSkinningRequired(skeletalMeshComp) ? VertexType::Vertex : VertexType::VertexSkinned);
        }
        else
        {
            pipeline = GetVulkanContext()->GetCurrentlyBoundPipeline();
            GetVulkanContext()->RebindPipeline(IsCpuSkinningRequired(skeletalMeshComp) ? VertexType::Vertex : VertexType::VertexSkinned);
        }

        OCT_ASSERT(pipeline);

        BindMaterialResource(material, pipeline);
        resource->mDescriptorSet->Bind(cb, (uint32_t)DescriptorSetBinding::Geometry, pipeline->GetPipelineLayout());

        vkCmdDrawIndexed(cb,
            mesh->GetNumIndices(),
            1,
            0,
            0,
            0);
    }
}

bool IsCpuSkinningRequired(SkeletalMeshComponent* skeletalMeshComp)
{
    if (skeletalMeshComp->GetSkeletalMesh() == nullptr)
    {
        return false;
    }
    else
    {
        return skeletalMeshComp->GetSkeletalMesh()->GetNumBones() > MAX_GPU_BONES;
    }
}

void DrawShadowMeshComp(ShadowMeshComponent* shadowMeshComp)
{
    VulkanContext* context = GetVulkanContext();
    StaticMesh* mesh = shadowMeshComp->GetStaticMesh();
    StaticMeshCompResource* resource = shadowMeshComp->GetResource();

    if (GetVulkanContext()->GetCurrentRenderPassId() == RenderPassId::Forward &&
        mesh != nullptr)
    {
        VkCommandBuffer cb = GetCommandBuffer();

        UpdateStaticMeshCompResource(shadowMeshComp);

        BindStaticMeshResource(mesh);

        // Step 1, render backfaces and write the desired shadow intensity to the scene color's Alpha channel.
        // Depth test is reversed.
        Pipeline* backPipeline = context->GetPipeline(PipelineId::ShadowMeshBack);
        context->BindPipeline(backPipeline, shadowMeshComp->GetVertexType());
        resource->mDescriptorSet->Bind(cb, (uint32_t)DescriptorSetBinding::Geometry, backPipeline->GetPipelineLayout());
        vkCmdDrawIndexed(cb, mesh->GetNumIndices(), 1, 0, 0, 0);

        // Step 2, render front faces and blend the shadow color to the scene colors's RGB channels based on the scene color's Alpha.
        // Depth test is normal
        Pipeline* frontPipeline = context->GetPipeline(PipelineId::ShadowMeshFront);
        context->BindPipeline(frontPipeline, shadowMeshComp->GetVertexType());
        resource->mDescriptorSet->Bind(cb, (uint32_t)DescriptorSetBinding::Geometry, frontPipeline->GetPipelineLayout());
        vkCmdDrawIndexed(cb, mesh->GetNumIndices(), 1, 0, 0, 0);

        // Step 3, render front faces without depth testing to clear scene color's alpha channel.
        Pipeline* clearPipeline = context->GetPipeline(PipelineId::ShadowMeshClear);
        context->BindPipeline(clearPipeline, shadowMeshComp->GetVertexType());
        resource->mDescriptorSet->Bind(cb, (uint32_t)DescriptorSetBinding::Geometry, clearPipeline->GetPipelineLayout());
        vkCmdDrawIndexed(cb, mesh->GetNumIndices(), 1, 0, 0, 0);
    }
}

void CreateTextMeshCompResource(TextMeshComponent* textMeshComp)
{
    TextMeshCompResource* resource = textMeshComp->GetResource();

    resource->mUniformBuffer = new UniformBuffer(sizeof(GeometryData), "Text Geometry Uniforms");

    VkDescriptorSetLayout layout = GetVulkanContext()->GetPipeline(PipelineId::Opaque)->GetDescriptorSetLayout(1);
    resource->mDescriptorSet = new DescriptorSet(layout);

    resource->mDescriptorSet->UpdateUniformDescriptor(GD_UNIFORM_BUFFER, resource->mUniformBuffer);
}

void DestroyTextMeshCompResource(TextMeshComponent* textMeshComp)
{
    TextMeshCompResource* resource = textMeshComp->GetResource();

    if (resource->mUniformBuffer != nullptr)
    {
        GetDestroyQueue()->Destroy(resource->mUniformBuffer);
        resource->mUniformBuffer = nullptr;
    }

    if (resource->mDescriptorSet != nullptr)
    {
        GetDestroyQueue()->Destroy(resource->mDescriptorSet);
        resource->mDescriptorSet = nullptr;
    }

    if (resource->mVertexBuffer != nullptr)
    {
        GetDestroyQueue()->Destroy(resource->mVertexBuffer);
        resource->mVertexBuffer = nullptr;
    }
}

void UpdateTextMeshCompVertexBuffer(TextMeshComponent* textMeshComp, const std::vector<Vertex>& vertices)
{
    TextMeshCompResource* resource = textMeshComp->GetResource();

    if (resource->mVertexBuffer != nullptr &&
        resource->mVertexBuffer->GetSize() < vertices.size() * sizeof(Vertex))
    {
        GetDestroyQueue()->Destroy(resource->mVertexBuffer);
        resource->mVertexBuffer = nullptr;
    }

    if (resource->mVertexBuffer == nullptr)
    {
        resource->mVertexBuffer = new Buffer(BufferType::Vertex, vertices.size() * sizeof(Vertex), "TextMeshComp Vertices");

    }

    if (resource->mVertexBuffer != nullptr && vertices.size() > 0)
    {
        OCT_ASSERT(resource->mVertexBuffer->GetSize() >= vertices.size() * sizeof(Vertex));
        resource->mVertexBuffer->Update(vertices.data(), vertices.size() * sizeof(Vertex), 0);
    }
}

void DrawTextMeshComp(TextMeshComponent* textMeshComp)
{
    TextMeshCompResource* resource = textMeshComp->GetResource();
    if (resource->mVertexBuffer == nullptr || textMeshComp->GetNumVisibleCharacters() == 0)
        return;

    VkCommandBuffer cb = GetCommandBuffer();

    UpdateTextMeshCompUniformBuffer(textMeshComp);

    VkDeviceSize offset = 0;
    VkBuffer vertexBuffer = resource->mVertexBuffer->Get();
    vkCmdBindVertexBuffers(cb, 0, 1, &vertexBuffer, &offset);

    Material* material = textMeshComp->GetMaterial();

    if (material == nullptr)
    {
        material = Renderer::Get()->GetDefaultMaterial();
        OCT_ASSERT(material != nullptr);
    }

    Pipeline* pipeline = nullptr;
    if (GetVulkanContext()->GetCurrentRenderPassId() == RenderPassId::Forward)
    {
        // During the Forward pass, it is expected that that the material sets the necessary pipeline.
        // This could be moved up to the Renderer in the future to reduce CPU cost.
        pipeline = GetMaterialPipeline(material);
        GetVulkanContext()->BindPipeline(pipeline, VertexType::Vertex);
    }
    else
    {
        pipeline = GetVulkanContext()->GetCurrentlyBoundPipeline();
        GetVulkanContext()->RebindPipeline(VertexType::Vertex);
    }

    OCT_ASSERT(pipeline);

    BindMaterialResource(material, pipeline);
    resource->mDescriptorSet->Bind(cb, (uint32_t)DescriptorSetBinding::Geometry, pipeline->GetPipelineLayout());

    vkCmdDraw(cb, TEXT_VERTS_PER_CHAR * textMeshComp->GetNumVisibleCharacters(), 1, 0, 0);
}

void UpdateTextMeshCompUniformBuffer(TextMeshComponent* textMeshComp)
{
    TextMeshCompResource* resource = textMeshComp->GetResource();

    Renderer* renderer = Renderer::Get();
    OCT_UNUSED(renderer);

    World* world = textMeshComp->GetWorld();
    GeometryData ubo = {};

    WriteGeometryUniformData(ubo, world, textMeshComp->GetRenderTransform());

    ubo.mHitCheckId = GetHitCheckId(textMeshComp);

#if EDITOR
    if (renderer->GetDebugMode() == DEBUG_WIREFRAME &&
        world->IsComponentSelected(textMeshComp))
    {
        if (world->GetSelectedComponent() == textMeshComp)
        {
            ubo.mColor = SELECTED_COMP_COLOR;
        }
        else
        {
            ubo.mColor = MULTI_SELECTED_COMP_COLOR;
        }
    }
#endif

    resource->mUniformBuffer->Update(&ubo, sizeof(ubo));
}

void CreateParticleCompResource(ParticleComponent* particleComp)
{
    ParticleCompResource* resource = particleComp->GetResource();

    resource->mUniformBuffer = new UniformBuffer(sizeof(GeometryData), "Particle Geometry Uniforms");

    VkDescriptorSetLayout layout = GetVulkanContext()->GetPipeline(PipelineId::Opaque)->GetDescriptorSetLayout(1);
    resource->mDescriptorSet = new DescriptorSet(layout);

    resource->mDescriptorSet->UpdateUniformDescriptor(GD_UNIFORM_BUFFER, resource->mUniformBuffer);
}

void DestroyParticleCompResource(ParticleComponent* particleComp)
{
    ParticleCompResource* resource = particleComp->GetResource();

    if (resource->mUniformBuffer != nullptr)
    {
        GetDestroyQueue()->Destroy(resource->mUniformBuffer);
        resource->mUniformBuffer = nullptr;
    }

    if (resource->mDescriptorSet != nullptr)
    {
        GetDestroyQueue()->Destroy(resource->mDescriptorSet);
        resource->mDescriptorSet = nullptr;
    }

    if (resource->mVertexBuffer != nullptr)
    {
        GetDestroyQueue()->Destroy(resource->mVertexBuffer);
        resource->mVertexBuffer = nullptr;
    }

    if (resource->mIndexBuffer != nullptr)
    {
        GetDestroyQueue()->Destroy(resource->mIndexBuffer);
        resource->mIndexBuffer = nullptr;
    }
}

void UpdateParticleCompResource(ParticleComponent* particleComp)
{
    // TODO: Reduce code duplication in updating these uniform buffers...
    ParticleCompResource* resource = particleComp->GetResource();

    Renderer* renderer = Renderer::Get();
    OCT_UNUSED(renderer);

    World* world = particleComp->GetWorld();
    CameraComponent* camera = world->GetActiveCamera();
    DirectionalLightComponent* dirLight = world->GetDirectionalLight();

    const glm::mat4 transform = particleComp->GetUseLocalSpace() ? particleComp->GetTransform() : glm::mat4(1);
    glm::vec4 uniformColor = glm::vec4(0.25f, 0.25f, 1.0f, 1.0f);

#if EDITOR
    if (renderer->GetDebugMode() == DEBUG_WIREFRAME)
    {
        uniformColor = glm::vec4(0.25f, 0.25f, 1.0f, 1.0f);

        if (world->IsComponentSelected(particleComp))
        {
            if (world->GetSelectedComponent() == particleComp)
            {
                uniformColor = SELECTED_COMP_COLOR;
            }
            else
            {
                uniformColor = MULTI_SELECTED_COMP_COLOR;
            }
        }
    }
#endif

    GeometryData ubo = {};
    ubo.mWVPMatrix = camera->GetViewProjectionMatrix() * transform;
    ubo.mWorldMatrix = transform;
    ubo.mNormalMatrix = glm::transpose(glm::inverse(transform));
    ubo.mLightWVPMatrix = dirLight ? (dirLight->GetViewProjectionMatrix() * transform) : glm::mat4(1);
    ubo.mColor = uniformColor;
    ubo.mHitCheckId = GetHitCheckId(particleComp);

    resource->mUniformBuffer->Update(&ubo, sizeof(ubo));
}

void UpdateParticleCompVertexBuffer(ParticleComponent* particleComp, const std::vector<VertexParticle>& vertices)
{
    if (vertices.size() == 0)
        return;

    ParticleCompResource* resource = particleComp->GetResource();

    uint32_t numVertices = uint32_t(vertices.size());
    OCT_ASSERT(numVertices % 4 == 0);
    uint32_t numIndices = (3 * numVertices) / 2; // 6 indices per 4 vertices

    if (resource->mNumVerticesAllocated < numVertices)
    {
        if (resource->mVertexBuffer != nullptr)
        {
            GetDestroyQueue()->Destroy(resource->mVertexBuffer);
            resource->mVertexBuffer = nullptr;
        }

        if (resource->mIndexBuffer != nullptr)
        {
            GetDestroyQueue()->Destroy(resource->mIndexBuffer);
            resource->mIndexBuffer = nullptr;
        }


        resource->mVertexBuffer = new Buffer(
            BufferType::Vertex,
            numVertices * sizeof(VertexParticle),
            "Particle Vertex Buffer");

        resource->mIndexBuffer = new Buffer(
            BufferType::Index,
            numIndices * sizeof(IndexType),
            "Particle Index Buffer");

        resource->mNumVerticesAllocated = numVertices;
    }

    resource->mVertexBuffer->Update(vertices.data(), numVertices * sizeof(VertexParticle));

    IndexType* indices = (IndexType*)resource->mIndexBuffer->Map();
    uint32_t i = 0;
    uint32_t v = 0;

    while (v < numVertices)
    {
        // First Triangle (upper left)
        indices[i + 0] = v + 0;
        indices[i + 1] = v + 1;
        indices[i + 2] = v + 2;

        // Second Triangle (bottom right)
        indices[i + 3] = v + 2;
        indices[i + 4] = v + 1;
        indices[i + 5] = v + 3;

        v += 4;
        i += 6;
    }
    resource->mIndexBuffer->Unmap();
}

void DrawParticleComp(ParticleComponent* particleComp)
{
    if (particleComp->GetNumParticles() > 0 &&
        particleComp->GetNumVertices() > 0)
    {
        ParticleCompResource* resource = particleComp->GetResource();
        VkCommandBuffer cb = GetCommandBuffer();

        UpdateParticleCompResource(particleComp);

        Material* material = particleComp->GetMaterial();

        if (material == nullptr)
        {
            material = Renderer::Get()->GetDefaultMaterial();
            OCT_ASSERT(material != nullptr);
        }

        Pipeline* pipeline = nullptr;
        if (GetVulkanContext()->GetCurrentRenderPassId() == RenderPassId::Forward)
        {
            // During the Forward pass, it is expected that that the material sets the necessary pipeline.
            // This could be moved up to the Renderer in the future to reduce CPU cost.
            pipeline = GetMaterialPipeline(material);
            GetVulkanContext()->BindPipeline(pipeline, VertexType::VertexParticle);
        }
        else
        {
            pipeline = GetVulkanContext()->GetCurrentlyBoundPipeline();
            GetVulkanContext()->RebindPipeline(VertexType::VertexParticle);
        }

        OCT_ASSERT(pipeline);

        BindMaterialResource(material, pipeline);
        resource->mDescriptorSet->Bind(cb, (uint32_t)DescriptorSetBinding::Geometry, pipeline->GetPipelineLayout());

        VkDeviceSize offset = 0;
        VkBuffer vertexBuffer = resource->mVertexBuffer->Get();
        vkCmdBindVertexBuffers(cb, 0, 1, &vertexBuffer, &offset);
        vkCmdBindIndexBuffer(cb, resource->mIndexBuffer->Get(), 0, VK_INDEX_TYPE_UINT32);

        // Note: because in the editor, selected components and hitcheck rendering will render things
        // that would normally be frustum culled, the number of vertices might not match what is expected
        // based on the number of particles, so use vertex count here to determine the number of indices.
        uint32_t numIndices = (particleComp->GetNumVertices() / 2) * 3; // 6 indices per particle (two triangles)

        vkCmdDrawIndexed(
            cb,
            numIndices,
            1,
            0,
            0,
            0);
    }
}

void CreateQuadResource(Quad* quad)
{
    QuadResource* resource = quad->GetResource();

    OCT_ASSERT(resource->mVertexBuffer == nullptr);
    resource->mVertexBuffer = new Buffer(BufferType::Vertex, 4 * sizeof(VertexUI), "Quad Vertices");

    OCT_ASSERT(resource->mUniformBuffer == nullptr);
    resource->mUniformBuffer = new UniformBuffer(sizeof(QuadUniformData), "Quad Uniforms");
    
    OCT_ASSERT(resource->mDescriptorSet == nullptr);
    VkDescriptorSetLayout layout = GetVulkanContext()->GetPipeline(PipelineId::Quad)->GetDescriptorSetLayout(1);
    resource->mDescriptorSet = new DescriptorSet(layout);

    UpdateQuadResource(quad);
}

void DestroyQuadResource(Quad* quad)
{
    QuadResource* resource = quad->GetResource();

    if (resource->mVertexBuffer != nullptr)
    {
        GetDestroyQueue()->Destroy(resource->mVertexBuffer);
        resource->mVertexBuffer = nullptr;
    }

    if (resource->mUniformBuffer != nullptr)
    {
        GetDestroyQueue()->Destroy(resource->mUniformBuffer);
        resource->mUniformBuffer = nullptr;
    }

    if (resource->mDescriptorSet != nullptr)
    {
        GetDestroyQueue()->Destroy(resource->mDescriptorSet);
        resource->mDescriptorSet = nullptr;
    }
}

void UpdateQuadResource(Quad* quad)
{
    // Vertex Buffer
    QuadResource* resource = quad->GetResource();
    resource->mVertexBuffer->Update(quad->GetVertices(), sizeof(VertexUI) * 4, 0);

    // Uniform Buffer
    QuadUniformData ubo = {};
    ubo.mTransform = glm::mat4(quad->GetTransform());
    ubo.mColor = quad->GetColor();
    ubo.mTint = quad->GetTint();
    resource->mUniformBuffer->Update(&ubo, sizeof(ubo));

    // Descriptor Set
    Renderer* renderer = Renderer::Get();
    Texture* texture = quad->GetTexture() ? quad->GetTexture() : renderer->mWhiteTexture.Get<Texture>();
    resource->mDescriptorSet->UpdateUniformDescriptor(0, resource->mUniformBuffer);
    resource->mDescriptorSet->UpdateImageDescriptor(1, texture->GetResource()->mImage);
}

void DrawQuad(Quad* quad)
{
    QuadResource* resource = quad->GetResource();
    VkCommandBuffer cb = GetCommandBuffer();

    // Make sure to bind the quad pipeline. Quad and text rendering will be interleaved.
    Pipeline* quadPipeline = GetVulkanContext()->GetPipeline(PipelineId::Quad);
    GetVulkanContext()->BindPipeline(quadPipeline, VertexType::VertexUI);

    VkDeviceSize offset = 0;
    VkBuffer vertexBuffer = resource->mVertexBuffer->Get();
    vkCmdBindVertexBuffers(cb, 0, 1, &vertexBuffer, &offset);

    resource->mDescriptorSet->Bind(cb, (uint32_t)DescriptorSetBinding::Quad, quadPipeline->GetPipelineLayout());

    vkCmdDraw(cb, 4, 1, 0, 0);
}

void CreateTextResource(Text* text)
{
    TextResource* resource = text->GetResource();

    CreateTextResourceVertexBuffer(text);

    OCT_ASSERT(resource->mUniformBuffer == nullptr);
    resource->mUniformBuffer = new UniformBuffer(sizeof(TextUniformData), "Text Uniforms");

    OCT_ASSERT(resource->mDescriptorSet == nullptr);
    VkDescriptorSetLayout layout = GetVulkanContext()->GetPipeline(PipelineId::Text)->GetDescriptorSetLayout(1);
    resource->mDescriptorSet = new DescriptorSet(layout);

    UpdateTextResourceUniformData(text);
    UpdateTextResourceVertexData(text);
}

void DestroyTextResource(Text* text)
{
    TextResource* resource = text->GetResource();

    DestroyTextResourceVertexBuffer(text);

    if (resource->mUniformBuffer != nullptr)
    {
        GetDestroyQueue()->Destroy(resource->mUniformBuffer);
        resource->mUniformBuffer = nullptr;
    }

    if (resource->mDescriptorSet != nullptr)
    {
        GetDestroyQueue()->Destroy(resource->mDescriptorSet);
        resource->mDescriptorSet = nullptr;
    }
}

void CreateTextResourceVertexBuffer(Text* text)
{
    TextResource* resource = text->GetResource();

    if (text->GetText().size() > 0 && text->GetNumCharactersAllocated() > 0)
    {
        resource->mVertexBuffer = new Buffer(BufferType::Vertex, text->GetNumCharactersAllocated() * TEXT_VERTS_PER_CHAR * sizeof(VertexUI), "Text Vertex Buffer");
        resource->mNumBufferCharsAllocated = text->GetNumCharactersAllocated();
    }
}

void DestroyTextResourceVertexBuffer(Text* text)
{
    TextResource* resource = text->GetResource();

    if (resource->mVertexBuffer != nullptr)
    {
        GetDestroyQueue()->Destroy(resource->mVertexBuffer);
        resource->mVertexBuffer = nullptr;
        resource->mNumBufferCharsAllocated = 0;
    }
}

void UpdateTextResourceUniformData(Text* text)
{
    TextResource* resource = text->GetResource();

    // Uniform Buffer
    int32_t fontSize = text->GetFont() ? text->GetFont()->GetSize() : 32;
    glm::vec2 justifiedOffset = text->GetJustifiedOffset();

    TextUniformData ubo = {};
    ubo.mTransform = glm::mat4(text->GetTransform());
    ubo.mColor = text->GetColor();
    ubo.mX = text->GetRect().mX + justifiedOffset.x;
    ubo.mY = text->GetRect().mY + justifiedOffset.y;
    ubo.mCutoff = text->GetCutoff();
    ubo.mOutlineSize = text->GetOutlineSize();
    ubo.mScale = text->GetScaledTextSize() / fontSize;
    ubo.mSoftness = text->GetSoftness();
    ubo.mPadding1 = 1337;
    ubo.mPadding2 = 1337;
    ubo.mDistanceField = false;
    ubo.mEffect = 0;

    resource->mUniformBuffer->Update(&ubo, sizeof(ubo));

    // Descriptors
    Renderer* renderer = Renderer::Get();
    Texture* texture = renderer->mWhiteTexture.Get<Texture>();
    Font* font = text->GetFont();

    if (font != nullptr &&
        font->GetTexture() != nullptr)
    {
        texture = font->GetTexture();
    }

    resource->mDescriptorSet->UpdateUniformDescriptor(0, resource->mUniformBuffer);
    resource->mDescriptorSet->UpdateImageDescriptor(1, texture->GetResource()->mImage);
}

void UpdateTextResourceVertexData(Text* text)
{
    TextResource* resource = text->GetResource();
    uint32_t numCharsAllocated = text->GetNumCharactersAllocated();
    if (resource->mNumBufferCharsAllocated < numCharsAllocated)
    {
        DestroyTextResourceVertexBuffer(text);
        CreateTextResourceVertexBuffer(text);
    }

    if (resource->mVertexBuffer != nullptr)
    {
        // Copy over vertex array from system memory
        resource->mVertexBuffer->Update(text->GetVertices(), sizeof(VertexUI) * TEXT_VERTS_PER_CHAR * numCharsAllocated);
    }
}

void DrawTextWidget(Text* text)
{
    TextResource* resource = text->GetResource();

    if (text->GetText().size() > 0 && resource->mVertexBuffer != nullptr)
    {
        VkCommandBuffer cb = GetCommandBuffer();
        Pipeline* textPipeline = GetVulkanContext()->GetPipeline(PipelineId::Text);
        GetVulkanContext()->BindPipeline(textPipeline, VertexType::VertexUI);

        VkDeviceSize offset = 0;
        VkBuffer vertexBuffer = resource->mVertexBuffer->Get();
        vkCmdBindVertexBuffers(cb, 0, 1, &vertexBuffer, &offset);

        resource->mDescriptorSet->Bind(cb, (uint32_t)DescriptorSetBinding::Text, textPipeline->GetPipelineLayout());

        vkCmdDraw(cb, 6 * text->GetNumVisibleCharacters(), 1, 0, 0);
    }
}

void CreatePolyResource(Poly* poly)
{
    PolyResource* resource = poly->GetResource();

    uint32_t numVerts = poly->GetNumVertices();
    if (numVerts > 0)
    {
        OCT_ASSERT(resource->mVertexBuffer == nullptr);
        resource->mVertexBuffer = new Buffer(BufferType::Vertex, numVerts * sizeof(VertexUI), "Poly Vertices");
        resource->mNumVerts = numVerts;
    }

    OCT_ASSERT(resource->mUniformBuffer == nullptr);
    resource->mUniformBuffer = new UniformBuffer(sizeof(PolyUniformData), "Poly Uniforms");

    OCT_ASSERT(resource->mDescriptorSet == nullptr);
    VkDescriptorSetLayout layout = GetVulkanContext()->GetPipeline(PipelineId::Poly)->GetDescriptorSetLayout(1);
    resource->mDescriptorSet = new DescriptorSet(layout);

    UpdatePolyResourceVertexData(poly);
    UpdatePolyResourceUniformData(poly);
}

void DestroyPolyResource(Poly* poly)
{
    PolyResource* resource = poly->GetResource();

    if (resource->mVertexBuffer != nullptr)
    {
        GetDestroyQueue()->Destroy(resource->mVertexBuffer);
        resource->mVertexBuffer = nullptr;
        resource->mNumVerts = 0;
    }

    if (resource->mUniformBuffer != nullptr)
    {
        GetDestroyQueue()->Destroy(resource->mUniformBuffer);
        resource->mUniformBuffer = nullptr;
    }

    if (resource->mDescriptorSet != nullptr)
    {
        GetDestroyQueue()->Destroy(resource->mDescriptorSet);
        resource->mDescriptorSet = nullptr;
    }
}

void UpdatePolyResourceUniformData(Poly* poly)
{
    PolyResource* resource = poly->GetResource();

    // Uniform Buffer
    PolyUniformData ubo = {};
    ubo.mTransform = glm::mat4(poly->GetTransform());
    ubo.mColor = poly->GetColor();
    ubo.mX = poly->GetRect().mX;
    ubo.mY = poly->GetRect().mY;
    ubo.mPad0 = 1337.0f;
    ubo.mPad1 = 1337.0f;
    resource->mUniformBuffer->Update(&ubo, sizeof(ubo));

    // Descriptor Set
    Renderer* renderer = Renderer::Get();
    Texture* texture = poly->GetTexture() ? poly->GetTexture() : renderer->mWhiteTexture.Get<Texture>();
    resource->mDescriptorSet->UpdateUniformDescriptor(0, resource->mUniformBuffer);
    resource->mDescriptorSet->UpdateImageDescriptor(1, texture->GetResource()->mImage);
}

void UpdatePolyResourceVertexData(Poly* poly)
{
    // Vertex Buffer
    PolyResource* resource = poly->GetResource();
    uint32_t numVerts = poly->GetNumVertices();
    uint32_t vbSize = numVerts * sizeof(VertexUI);

    if (resource->mVertexBuffer != nullptr &&
        resource->mVertexBuffer->GetSize() < vbSize)
    {
        GetDestroyQueue()->Destroy(resource->mVertexBuffer);
        resource->mVertexBuffer = nullptr;
        resource->mNumVerts = 0;
    }

    if (numVerts > 0)
    {
        if (resource->mVertexBuffer == nullptr)
        {
            resource->mVertexBuffer = new Buffer(BufferType::Vertex, numVerts * sizeof(VertexUI), "Poly Vertices");
            resource->mNumVerts = numVerts;
        }

        resource->mVertexBuffer->Update(poly->GetVertices(), numVerts * sizeof(VertexUI), 0);
    }
}

void DrawPoly(Poly* poly)
{
    PolyResource* resource = poly->GetResource();
    uint32_t numVerts = resource->mNumVerts;

    if (numVerts > 0)
    {
        VkCommandBuffer cb = GetCommandBuffer();

        Pipeline* polygonPipeline = GetVulkanContext()->GetPipeline(PipelineId::Poly);
        GetVulkanContext()->BindPipeline(polygonPipeline, VertexType::VertexUI);

        VkDeviceSize offset = 0;
        VkBuffer vertexBuffer = resource->mVertexBuffer->Get();
        vkCmdBindVertexBuffers(cb, 0, 1, &vertexBuffer, &offset);

        resource->mDescriptorSet->Bind(cb, (uint32_t)DescriptorSetBinding::Poly, polygonPipeline->GetPipelineLayout());

        vkCmdSetLineWidth(cb, poly->GetLineWidth());

        vkCmdDraw(cb, numVerts, 1, 0, 0);
    }
}

void DrawStaticMesh(StaticMesh* mesh, Material* material, const glm::mat4& transform, glm::vec4 color, uint32_t hitCheckId)
{
    OCT_ASSERT(mesh != nullptr);
    if (mesh != nullptr)
    {
        VkCommandBuffer cb = GetCommandBuffer();

        // Setup uniform buffer
        UniformBufferArena& uniformArena = GetVulkanContext()->GetMeshUniformBufferArena();
        UniformBuffer* uniformBuffer = uniformArena.Alloc(sizeof(GeometryData), "DrawStaticMesh Uniforms");
        GeometryData ubo = {};
        WriteGeometryUniformData(ubo, GetWorld(), transform);
        ubo.mColor = color;
        ubo.mHitCheckId = hitCheckId;
        uniformBuffer->Update(&ubo, sizeof(ubo));

        BindStaticMeshResource(mesh);

        if (material == nullptr)
        {
            material = Renderer::Get()->GetDefaultMaterial();
            OCT_ASSERT(material != nullptr);
        }

        Pipeline* pipeline = nullptr;
        if (GetVulkanContext()->GetCurrentRenderPassId() == RenderPassId::Forward)
        {
            // During the Forward pass, it is expected that that the material sets the necessary pipeline.
            // This could be moved up to the Renderer in the future to reduce CPU cost.
            pipeline = GetMaterialPipeline(material);
            GetVulkanContext()->BindPipeline(pipeline, mesh->HasVertexColor() ? VertexType::VertexColor : VertexType::Vertex);
        }
        else
        {
            pipeline = GetVulkanContext()->GetCurrentlyBoundPipeline();
            GetVulkanContext()->RebindPipeline(mesh->HasVertexColor() ? VertexType::VertexColor : VertexType::Vertex);
        }

        OCT_ASSERT(pipeline);
        BindMaterialResource(material, pipeline);

        DescriptorSetArena& descriptorArena = GetVulkanContext()->GetMeshDescriptorSetArena();
        DescriptorSet* descriptorSet = descriptorArena.Alloc(pipeline->GetDescriptorSetLayout((uint32_t)DescriptorSetBinding::Geometry));
        descriptorSet->UpdateUniformDescriptor(GD_UNIFORM_BUFFER, uniformBuffer);
        descriptorSet->Bind(cb, (uint32_t)DescriptorSetBinding::Geometry, pipeline->GetPipelineLayout());

        vkCmdDrawIndexed(cb,
            mesh->GetNumIndices(),
            1,
            0,
            0,
            0);
    }
}
