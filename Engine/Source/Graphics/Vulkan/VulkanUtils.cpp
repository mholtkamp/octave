#include "Graphics/Vulkan/VulkanUtils.h"
#include "Graphics/Vulkan/VulkanContext.h"
#include "Graphics/Vulkan/Pipeline.h"

#include "Engine.h"
#include "Renderer.h"
#include "Assets/Font.h"
#include "Assets/Material.h"
#include "Assets/MaterialBase.h"
#include "Assets/MaterialInstance.h"
#include "Assets/MaterialLite.h"
#include "Assets/Texture.h"
#include "Assets/StaticMesh.h"
#include "Assets/SkeletalMesh.h"
#include "Nodes/3D/StaticMesh3d.h"
#include "Nodes/3D/SkeletalMesh3d.h"
#include "Nodes/3D/ShadowMesh3d.h"
#include "Nodes/3D/InstancedMesh3d.h"
#include "Nodes/3D/TextMesh3d.h"
#include "Nodes/3D/Particle3d.h"
#include "Nodes/Widgets/Quad.h"
#include "Nodes/Widgets/Text.h"
#include "Nodes/Widgets/Poly.h"
#include "Utilities.h"
#include "Vertex.h"
#include "Maths.h"

#if EDITOR
#include "EditorState.h"
#endif

extern PFN_vkCmdBeginDebugUtilsLabelEXT CmdBeginDebugUtilsLabelEXT;
extern PFN_vkCmdEndDebugUtilsLabelEXT CmdEndDebugUtilsLabelEXT;
extern PFN_vkCmdInsertDebugUtilsLabelEXT CmdInsertDebugUtilsLabelEXT;
extern PFN_vkSetDebugUtilsObjectNameEXT SetDebugUtilsObjectNameEXT;

VkFormat ConvertPixelFormat(PixelFormat pixelFormat, bool srgb)
{
    VkFormat format = VK_FORMAT_UNDEFINED;

    switch (pixelFormat)
    {
    case PixelFormat::LA4: format = VK_FORMAT_R8G8_UNORM; break;
    case PixelFormat::RGB565: format = VK_FORMAT_R5G6B5_UNORM_PACK16; break;
    case PixelFormat::RGBA5551: format = VK_FORMAT_R5G5B5A1_UNORM_PACK16; break;
    case PixelFormat::RGBA8: format = srgb ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM; break;

#if PLATFORM_ANDROID
    case PixelFormat::CMPR: format = VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK; break;
#else
    case PixelFormat::CMPR: format = srgb ? VK_FORMAT_BC1_RGBA_SRGB_BLOCK : VK_FORMAT_BC1_RGBA_UNORM_BLOCK; break;
#endif

    case PixelFormat::R8: format = srgb ? VK_FORMAT_R8_SRGB : VK_FORMAT_R8_UNORM; break;
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

VkCullModeFlags ConvertCullMode(CullMode cullMode)
{
    VkCullModeFlags retMode = VK_CULL_MODE_NONE;

    if (cullMode == CullMode::Front)
        retMode = VK_CULL_MODE_FRONT_BIT;
    else if (cullMode == CullMode::Back)
        retMode = VK_CULL_MODE_BACK_BIT;

    return retMode;
}

UniformBlock WriteUniformBlock(void* data, uint32_t size)
{
    UniformBuffer* uniformBuffer = GetVulkanContext()->GetFrameUniformBuffer();
    UniformBlock retBlock = uniformBuffer->AllocBlock(size);
    memcpy(retBlock.mData, data, size);

    return retBlock;
}

void CreateBuffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer& buffer,
    VramAllocation& bufferMemory)
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

    VramAllocator::Alloc(memRequirements.size, memRequirements.alignment, memoryType, bufferMemory);

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
    else if ((oldLayout == VK_IMAGE_LAYOUT_UNDEFINED || oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED) &&
        newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    }
    else if ((oldLayout == VK_IMAGE_LAYOUT_UNDEFINED || oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED) &&
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
    else if ((oldLayout == VK_IMAGE_LAYOUT_UNDEFINED || oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED) && 
        newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }
    else if ((oldLayout == VK_IMAGE_LAYOUT_UNDEFINED || oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED) &&
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
        //LogWarning("Unsupported layout transition.");
        //OCT_ASSERT(0);

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

uint32_t GetFrameNumber()
{
    return GetVulkanContext()->GetFrameNumber();
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
    case VK_FORMAT_R8G8B8A8_SRGB: size = 4; break;

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

std::vector<VkVertexInputBindingDescription> GetVertexBindingDescription(VertexType type)
{
    std::vector<VkVertexInputBindingDescription> bindings;

    // Binding 0
    {
        VkVertexInputBindingDescription desc;

        switch (type)
        {
        case VertexType::Vertex:
            desc.stride = sizeof(Vertex);
            break;
        case VertexType::VertexInstanceColor:
            desc.stride = sizeof(Vertex);
            break;
        case VertexType::VertexColor:
            desc.stride = sizeof(VertexColor);
            break;
        case VertexType::VertexColorInstanceColor:
            desc.stride = sizeof(VertexColor);
            break;
        case VertexType::VertexUI:
            desc.stride = sizeof(VertexUI);
            break;
        case VertexType::VertexLine:
            desc.stride = sizeof(VertexLine);
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
        bindings.push_back(desc);
    }

    // Binding 1
    if (type == VertexType::VertexInstanceColor ||
        type == VertexType::VertexColorInstanceColor)
    {
        VkVertexInputBindingDescription desc;
        desc.stride = sizeof(uint32_t);
        desc.binding = 1;
        desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindings.push_back(desc);
    }

    return bindings;
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

    case VertexType::VertexInstanceColor:
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
        // Color
        attributeDescriptions.push_back(VkVertexInputAttributeDescription());
        attributeDescriptions[4].binding = 1;
        attributeDescriptions[4].location = 4;
        attributeDescriptions[4].format = VK_FORMAT_R8G8B8A8_UNORM;
        attributeDescriptions[4].offset = 0;
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

    case VertexType::VertexColorInstanceColor:
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
        attributeDescriptions[4].binding = 1;
        attributeDescriptions[4].location = 4;
        attributeDescriptions[4].format = VK_FORMAT_R8G8B8A8_UNORM;
        attributeDescriptions[4].offset = 0;
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

    case VertexType::VertexLine:
        // Position
        attributeDescriptions.push_back(VkVertexInputAttributeDescription());
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(VertexLine, mPosition);
        // Color
        attributeDescriptions.push_back(VkVertexInputAttributeDescription());
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R8G8B8A8_UNORM;
        attributeDescriptions[1].offset = offsetof(VertexLine, mColor);
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

void WriteGeometryUniformData(GeometryData& outData, World* world, Node3D* comp, const glm::mat4& transform)
{
    Camera3D* camera = world->GetActiveCamera();

    outData.mWVPMatrix = camera->GetViewProjectionMatrix() * transform;
    outData.mWorldMatrix = transform;
    outData.mNormalMatrix = glm::transpose(glm::inverse(transform));
    outData.mColor = glm::vec4(0.25f, 0.25f, 1.0f, 1.0f);
    outData.mHitCheckId = 0;
    outData.mHasBakedLighting = false;
    outData.mNumLights = 0;

    if (comp != nullptr)
    {
        outData.mHitCheckId = comp->GetHitCheckId();

#if EDITOR
        if (Renderer::Get()->GetDebugMode() == DEBUG_WIREFRAME &&
            GetEditorState()->IsNodeSelected(comp))
        {
            if (GetEditorState()->GetSelectedNode() == comp)
            {
                outData.mColor = SELECTED_COMP_COLOR;
            }
            else
            {
                outData.mColor = MULTI_SELECTED_COMP_COLOR;
            }
        }
#endif
    }
}

void GatherGeometryLightUniformData(GeometryData& outData, Primitive3D* primitive, Material* material, bool isStaticMesh)
{
    // Find overlapping point lights
    uint32_t numLights = 0;

    bool useAllDomain = true;
    bool useStaticDomain = false;
    uint8_t lightingChannels = primitive->GetLightingChannels();
    Bounds bounds = primitive->GetBounds();

    if (isStaticMesh)
    {
        StaticMesh3D* staticMeshComp = (StaticMesh3D*)primitive;
        bool useBakedLighting = staticMeshComp->HasBakedLighting();
        bool hasBakedColor = (staticMeshComp->GetInstanceColors().size() > 0);

        // Don't reapply static/all lighting if the mesh already has baked lighting.
        useAllDomain = !useBakedLighting || !hasBakedColor;
        useStaticDomain = useBakedLighting && !hasBakedColor;
    }

    // We can skip this if the material doesn't use lighting.
    if (material != nullptr && 
        (!material->IsLite() || ((MaterialLite*)material)->GetShadingModel() != ShadingModel::Unlit))
    {
        const std::vector<LightData>& lights = Renderer::Get()->GetLightData();
        uint32_t lightIndices[MAX_LIGHTS_PER_DRAW] = {};

        // Don't worry about sorting for now. Just choose the first X overlapping lights.
        for (uint32_t i = 0; i < lights.size() && i < MAX_LIGHTS_PER_FRAME; ++i)
        {
            LightingDomain domain = lights[i].mDomain;

            if ((domain == LightingDomain::Static && !useStaticDomain) ||
                (domain == LightingDomain::All && !useAllDomain) ||
                ((lights[i].mLightingChannels & lightingChannels) == 0))
            {
                continue;
            }

            bool overlaps = false;
            if (lights[i].mType == LightType::Directional)
            {
                // Global light always overlaps.
                overlaps = true;
            }
            else
            {
                // If the local light is overlapping the geometry bounds, then just add it to the list.
                float dist2 = glm::distance2(lights[i].mPosition, bounds.mCenter);

                float maxDist = (bounds.mRadius + lights[i].mRadius);
                float maxDist2 = maxDist * maxDist;

                // Overlap!
                if (dist2 < maxDist2)
                {
                    overlaps = true;
                }
            }

            if (overlaps)
            {
                // Light indices are packed as bytes into 32-bit uints.
                // Lights0 contains indices for lights 0 - 3
                // Lights1 contains indices for lights 4 - 7
                uint32_t lightNum = numLights;
                uint32_t& lightIndexInt = (lightNum >= 4) ? outData.mLights1 : outData.mLights0;
                uint32_t shiftedIdx = 0;

                if (lightNum >= 4)
                {
                    shiftedIdx = i << (8 * (lightNum - 4));
                }
                else
                {
                    shiftedIdx = i << (8 * lightNum);
                }

                lightIndexInt |= shiftedIdx;

                ++numLights;

                if (numLights >= MAX_LIGHTS_PER_DRAW)
                {
                    break;
                }
            }
        }
    }

    outData.mNumLights = numLights;
}

void WriteMaterialLiteUniformData(MaterialData& outData, MaterialLite* material)
{
    Texture* textures[4] = {};
    textures[0] = material->GetTexture((TextureSlot)0);
    textures[1] = material->GetTexture((TextureSlot)1);
    textures[2] = material->GetTexture((TextureSlot)2);
    textures[3] = material->GetTexture((TextureSlot)3);

    outData.mUvOffset0 = material->GetUvOffset(0);
    outData.mUvScale0 = material->GetUvScale(0);
    outData.mUvOffset1 = material->GetUvOffset(1);
    outData.mUvScale1 = material->GetUvScale(1);
    outData.mColor = material->GetColor();
    outData.mFresnelColor = material->GetFresnelColor();
    outData.mShadingModel = static_cast<uint32_t>(material->GetShadingModel());
    outData.mBlendMode = static_cast<uint32_t>(material->GetBlendMode());
    outData.mToonSteps = material->GetToonSteps();
    outData.mFresnelPower = material->GetFresnelPower();
    outData.mSpecular = material->GetSpecular();
    outData.mOpacity = material->GetOpacity();
    outData.mMaskCutoff = material->GetMaskCutoff();
    outData.mShininess = material->GetShininess();
    outData.mFresnelEnabled = static_cast<uint32_t>(material->IsFresnelEnabled());
    outData.mVertexColorMode = static_cast<uint32_t>(material->GetVertexColorMode());
    outData.mApplyFog = static_cast<uint32_t>(material->ShouldApplyFog());
    outData.mEmission = material->GetEmission();
    outData.mWrapLighting = material->GetWrapLighting();
    outData.mUvMaps[0] = material->GetUvMap(0);
    outData.mUvMaps[1] = material->GetUvMap(1);
    outData.mUvMaps[2] = material->GetUvMap(2);
    outData.mUvMaps[3] = material->GetUvMap(3);
    outData.mTevModes[0] = textures[0] ? (uint32_t)material->GetTevMode(0) : (uint32_t)TevMode::Count;
    outData.mTevModes[1] = textures[1] ? (uint32_t)material->GetTevMode(1) : (uint32_t)TevMode::Count;
    outData.mTevModes[2] = textures[2] ? (uint32_t)material->GetTevMode(2) : (uint32_t)TevMode::Count;
    outData.mTevModes[3] = textures[3] ? (uint32_t)material->GetTevMode(3) : (uint32_t)TevMode::Count;
}

void WriteMaterialCustomUniformData(MaterialData& outData, Material* material)
{

}

VkPipelineColorBlendAttachmentState GetBasicBlendState(BasicBlendState basicBlendState)
{
    VkPipelineColorBlendAttachmentState blendState = {};

    switch (basicBlendState)
    {
    case BasicBlendState::Opaque:
        blendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT;
        blendState.blendEnable = false;
        blendState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        blendState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendState.colorBlendOp = VK_BLEND_OP_ADD;
        blendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendState.alphaBlendOp = VK_BLEND_OP_ADD;
        break;
    case BasicBlendState::Translucent:
        blendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        blendState.blendEnable = true;
        blendState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        blendState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blendState.colorBlendOp = VK_BLEND_OP_ADD;
        blendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendState.alphaBlendOp = VK_BLEND_OP_ADD;
        break;
    case BasicBlendState::Additive:
        blendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        blendState.blendEnable = true;
        blendState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        blendState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
        blendState.colorBlendOp = VK_BLEND_OP_ADD;
        blendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        blendState.alphaBlendOp = VK_BLEND_OP_ADD;
        break;
    }

    return blendState;
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
    VkFormat format = ConvertPixelFormat(PixelFormat::RGBA8 /*texture->GetFormat()*/, texture->IsSrgb());

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

void BindForwardVertexType(VertexType vertType, Material* material, bool instanced = false)
{
    VulkanContext* ctx = GetVulkanContext();
    MaterialResource* res = material ? material->GetResource() : nullptr;

    // Instances should reference their base material.
    if (material && material->IsInstance())
    {
        MaterialInstance* inst = (MaterialInstance*)material;
        MaterialBase* base = inst->GetBaseMaterial();
        res = base ? base->GetResource() : res;
    }

    // Always bind correct vert shader even
    Shader* vertShader = material ? res->mVertexShaders[(uint32_t)vertType] : nullptr;

    const char* vertShaderName = instanced ? "ForwardInstanced.vert" : "Forward.vert";

    switch (vertType)
    {
    case VertexType::VertexColor:
    case VertexType::VertexInstanceColor:
    case VertexType::VertexColorInstanceColor:
        vertShaderName = instanced ? "ForwardInstancedColor.vert" : "ForwardColor.vert";
        break;
    case VertexType::VertexSkinned:
        vertShaderName = "ForwardSkinned.vert";
        break;
    case VertexType::VertexParticle:
        vertShaderName = "ForwardParticle.vert";
        break;

    default:
        break;
    }

    ctx->SetVertexType(vertType);

    if (vertShader != nullptr)
    {
        ctx->SetVertexShader(vertShader);
    }
    else
    {
        ctx->SetVertexShader(vertShaderName);
    }
}

void CreateMaterialResource(Material* material)
{
    MaterialResource* resource = material->GetResource();
	
	if (!material->IsLite())
    {
        MaterialBase* base = material->IsBase() ? (MaterialBase*)material : nullptr;

        if (material->IsInstance())
        {
            MaterialInstance* inst = (MaterialInstance*)material;
            base = inst->GetBaseMaterial();
        }

        if (base != nullptr)
        {
            DestroyMaterialResource(material);

            // Vertex Shaders
            for (uint32_t i = 0; i < (uint32_t)VertexType::Max; ++i)
            {
                const std::vector<uint8_t>& sprv = base->GetVertexShaderCode((VertexType)i);
                const uint8_t* data = sprv.data();
                uint32_t size = (uint32_t)sprv.size();

                if (size > 0)
                {
                    resource->mVertexShaders[i] = new Shader((const char*)data, size, ShaderStage::Vertex, "Material Vert Shader");
                }
            }

            // Fragment Shader
            {
                const std::vector<uint8_t>& sprv = base->GetFragmentShaderCode();
                const uint8_t* data = sprv.data();
                uint32_t size = (uint32_t)sprv.size();

                if (size > 0)
                {
                    resource->mFragmentShader = new Shader((const char*)data, size, ShaderStage::Fragment, "Material Frag Shader");
                }
            }
        }
    }
}

void DestroyMaterialResource(Material* material)
{
    MaterialResource* resource = material->GetResource();
    DestroyQueue* destroyQueue = GetVulkanContext()->GetDestroyQueue();

    for (uint32_t i = 0; i < (uint32_t)VertexType::Max; ++i)
    {
        if (resource->mVertexShaders[i] != nullptr)
        {
            destroyQueue->Destroy(resource->mVertexShaders[i]);
            resource->mVertexShaders[i] = nullptr;
        }

    }

    if (resource->mFragmentShader != nullptr)
    {
        destroyQueue->Destroy(resource->mFragmentShader);
        resource->mFragmentShader = nullptr;
    }
}

void BindMaterialResource(Material* material)
{
    if (material == nullptr)
        return;

    VulkanContext* ctx = GetVulkanContext();
    MaterialResource* res = material->GetResource();

    // Instances should reference their base material.
    if (material->IsInstance())
    {
        MaterialInstance* inst = (MaterialInstance*)material;
        MaterialBase* base = inst->GetBaseMaterial();
        res = base ? base->GetResource() : res;
    }

    Shader* matFragShader = res->mFragmentShader;

    if (matFragShader == nullptr)
    {
        matFragShader = GetVulkanContext()->GetGlobalShader("Forward.frag");
    }

    ctx->SetFragmentShader(matFragShader);

    bool depthEnabled = !material->IsDepthTestDisabled();
    ctx->SetDepthTestEnabled(depthEnabled);
    ctx->SetDepthWriteEnabled(depthEnabled);

    VkCullModeFlags cullMode = ConvertCullMode(material->GetCullMode());
    ctx->SetCullMode(cullMode);

    BlendMode blendMode = material->GetBlendMode();
    switch (blendMode)
    {
    case BlendMode::Opaque:
    case BlendMode::Masked:
        ctx->SetBlendState(BasicBlendState::Opaque, 0);
        break;
    case BlendMode::Translucent:
        ctx->SetBlendState(BasicBlendState::Translucent, 0);
        ctx->SetDepthWriteEnabled(false);
        break;
    case BlendMode::Additive:
        ctx->SetBlendState(BasicBlendState::Additive, 0);
        ctx->SetDepthWriteEnabled(false);
        break;
    }
}

void BindMaterialDescriptorSet(Material* material)
{
    if (material == nullptr)
        return;

    VkCommandBuffer cb = GetCommandBuffer();

    MaterialResource* resource = material->GetResource();
    std::vector<ShaderParameter>& params = material->GetParameters();

    if (material->IsLite())
    {
        MaterialLite* matLite = (MaterialLite*)material;

        Texture* textures[4] = {};
        textures[0] = matLite->GetTexture((TextureSlot)0);
        textures[1] = matLite->GetTexture((TextureSlot)1);
        textures[2] = matLite->GetTexture((TextureSlot)2);
        textures[3] = matLite->GetTexture((TextureSlot)3);

        // Update uniform buffer data
        MaterialData ubo = {};
        WriteMaterialLiteUniformData(ubo, matLite);

        UniformBlock uniformBlock = WriteUniformBlock(&ubo, sizeof(ubo));

        // Ensure we are using valid textures
        Renderer* renderer = Renderer::Get();
        for (uint32_t i = 0; i < 4; ++i)
        {
            Texture* texture = textures[i];
            if (texture == nullptr)
            {
                texture = renderer->mWhiteTexture.Get<Texture>();
                OCT_ASSERT(texture != nullptr);
            }
        }

        DescriptorSet::Begin("Lite Material DS")
            .WriteUniformBuffer(MD_UNIFORM_BUFFER, uniformBlock)
            .WriteImage(MD_TEXTURE_START + 0, textures[0]->GetResource()->mImage)
            .WriteImage(MD_TEXTURE_START + 1, textures[1]->GetResource()->mImage)
            .WriteImage(MD_TEXTURE_START + 2, textures[2]->GetResource()->mImage)
            .WriteImage(MD_TEXTURE_START + 3, textures[3]->GetResource()->mImage)
            .Build()
            .Bind(cb, 2);
    }
    else
    {
        // 4 KB should be enough for all of our vector/scalar params right??
        uint32_t uboSize = 0;
        uint8_t uboData[MAX_MATERIAL_UBO_SIZE];
        material->WriteShaderUniformParams(uboData, uboSize);

        DescriptorSet matSet = DescriptorSet::Begin("Material DS");

        Renderer* renderer = Renderer::Get();
        
        // Update uniform buffer data
        UniformBlock uniformBlock = WriteUniformBlock(uboData, uboSize);
        matSet.WriteUniformBuffer(MD_UNIFORM_BUFFER, uniformBlock);

        for (uint32_t i = 0; i < params.size(); ++i)
        {
            ShaderParameter& param = params[i];
            if (param.mType == ShaderParameterType::Texture)
            {
                Texture* texture = param.mTextureValue.Get<Texture>();
                if (texture == nullptr)
                {
                    texture = renderer->mWhiteTexture.Get<Texture>();
                    OCT_ASSERT(texture != nullptr);
                }

                matSet.WriteImage(param.mOffset, texture->GetResource()->mImage);
            }
        }

        matSet.Build();
        matSet.Bind(cb, 2);
    }
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

void BindGeometryDescriptorSet(StaticMesh3D* staticMeshComp)
{
    VkCommandBuffer cb = GetCommandBuffer();
    StaticMeshCompResource* resource = staticMeshComp->GetResource();

    Renderer* renderer = Renderer::Get();
    OCT_UNUSED(renderer);

    World* world = staticMeshComp->GetWorld();
    GeometryData ubo = {};

    WriteGeometryUniformData(ubo, world, staticMeshComp, staticMeshComp->GetRenderTransform());
    ubo.mHasBakedLighting = staticMeshComp->HasBakedLighting();

    GatherGeometryLightUniformData(ubo, staticMeshComp, staticMeshComp->GetMaterial(), true);

    UniformBlock uniformBlock = WriteUniformBlock(&ubo, sizeof(ubo));

    if (staticMeshComp->IsInstancedMesh3D())
    {
        InstancedMeshCompResource* instResource = ((InstancedMesh3D*)staticMeshComp)->GetInstancedMeshResource();

        DescriptorSet::Begin("StaticMesh3D DS")
            .WriteUniformBuffer(GD_UNIFORM_BUFFER, uniformBlock)
            .WriteStorageBuffer(GD_INSTANCE_DATA_BUFFER, instResource->mInstanceDataBuffer)
            //.WriteStoragebuffer(GD_INSTANCE_COLOR_BUFFER, instResource->mInstanceColorBuffer)
            .Build()
            .Bind(cb, 1);
    }
    else
    {
        DescriptorSet::Begin("StaticMesh3D DS")
            .WriteUniformBuffer(GD_UNIFORM_BUFFER, uniformBlock)
            .Build()
            .Bind(cb, 1);
    }
}

void UpdateStaticMeshCompResourceColors(StaticMesh3D* staticMeshComp)
{
    StaticMeshCompResource* resource = staticMeshComp->GetResource();

    const std::vector<uint32_t>& instanceColors = staticMeshComp->GetInstanceColors();
    uint32_t colorBufferSize = sizeof(uint32_t) * uint32_t(instanceColors.size());
    if (instanceColors.size() == 0)
    {
        if (resource->mColorVertexBuffer != nullptr)
        {
            GetDestroyQueue()->Destroy(resource->mColorVertexBuffer);
            resource->mColorVertexBuffer = nullptr;
        }
    }
    else
    {
        if (resource->mColorVertexBuffer != nullptr &&
            resource->mColorVertexBuffer->GetSize() < colorBufferSize)
        {
            // Need to reallocate to handle more vertices, so delete the current buffer.
            GetDestroyQueue()->Destroy(resource->mColorVertexBuffer);
            resource->mColorVertexBuffer = nullptr;
        }

        if (resource->mColorVertexBuffer == nullptr)
        {
            resource->mColorVertexBuffer = new Buffer(
                BufferType::Vertex,
                colorBufferSize,
                "Static Mesh Instance Colors",
                nullptr,
                false);
        }

        resource->mColorVertexBuffer->Update(instanceColors.data(), colorBufferSize);
    }
}

void DestroyStaticMeshCompResource(StaticMesh3D* staticMeshComp)
{
    StaticMeshCompResource* resource = staticMeshComp->GetResource();
    if (resource->mColorVertexBuffer)
    {
        GetDestroyQueue()->Destroy(resource->mColorVertexBuffer);
        resource->mColorVertexBuffer = nullptr;
    }

    if (staticMeshComp->IsInstancedMesh3D())
    {
        InstancedMeshCompResource* instResource = ((InstancedMesh3D*)staticMeshComp)->GetInstancedMeshResource();

        if (instResource->mInstanceDataBuffer != nullptr)
        {
            GetDestroyQueue()->Destroy(instResource->mInstanceDataBuffer);
            instResource->mInstanceDataBuffer = nullptr;
        }

        if (instResource->mVertexColorBuffer != nullptr)
        {
            GetDestroyQueue()->Destroy(instResource->mVertexColorBuffer);
            instResource->mVertexColorBuffer = nullptr;
        }
    }
}

void DrawStaticMeshComp(StaticMesh3D* staticMeshComp, StaticMesh* meshOverride)
{
    StaticMesh* mesh = meshOverride ? meshOverride : staticMeshComp->GetStaticMesh();
    StaticMeshCompResource* resource = staticMeshComp->GetResource();

    if (mesh != nullptr)
    {
        VkCommandBuffer cb = GetCommandBuffer();

        BindStaticMeshResource(mesh);

        bool useMaterial = GetVulkanContext()->AreMaterialsEnabled();

        // Determine vertex type for binding appropriate pipeline
        VertexType vertexType = VertexType::Vertex;
        if (useMaterial &&
            staticMeshComp->GetInstanceColors().size() == mesh->GetNumVertices() &&
            resource->mColorVertexBuffer != nullptr)
        {
            if (mesh->HasVertexColor())
            {
                vertexType = VertexType::VertexColorInstanceColor;
            }
            else
            {
                vertexType = VertexType::VertexInstanceColor;
            }

            // Bind color instance buffer at binding #1
            VkBuffer vertexBuffers[] = { resource->mColorVertexBuffer->Get() };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(cb, 1, 1, vertexBuffers, offsets);
        }
        else if (mesh->HasVertexColor())
        {
            vertexType = VertexType::VertexColor;
        }

        Material* material = nullptr;
        
        if (useMaterial)
        {
            material = staticMeshComp->GetMaterial();
            material = material ? material : Renderer::Get()->GetDefaultMaterial();
        }

        BindForwardVertexType(vertexType, material);
        BindMaterialResource(material);
        GetVulkanContext()->CommitPipeline();

        BindGeometryDescriptorSet(staticMeshComp);
        BindMaterialDescriptorSet(material);

        vkCmdDrawIndexed(cb,
            mesh->GetNumIndices(),
            1,
            0,
            0,
            0);
    }
}

void DestroySkeletalMeshCompResource(SkeletalMesh3D* skeletalMeshComp)
{
    SkeletalMeshCompResource* resource = skeletalMeshComp->GetResource();

    if (resource->mVertexBuffer != nullptr)
    {
        GetDestroyQueue()->Destroy(resource->mVertexBuffer);
        resource->mVertexBuffer = nullptr;
    }
}

void ReallocateSkeletalMeshCompVertexBuffer(SkeletalMesh3D* skeletalMeshComp, uint32_t numVertices)
{
    SkeletalMeshCompResource* resource = skeletalMeshComp->GetResource();

    if (IsCpuSkinningRequired(skeletalMeshComp))
    {
        if (resource->mVertexBuffer != nullptr)
        {
            GetDestroyQueue()->Destroy(resource->mVertexBuffer);
            resource->mVertexBuffer = nullptr;
        }

        resource->mVertexBuffer = new MultiBuffer(BufferType::Vertex, numVertices * sizeof(Vertex), "SkeletalMesh3D Skinned Vertices");
    }
}

void UpdateSkeletalMeshCompVertexBuffer(SkeletalMesh3D* skeletalMeshComp, const std::vector<Vertex>& skinnedVertices)
{
    SkeletalMeshCompResource* resource = skeletalMeshComp->GetResource();

    if (resource->mVertexBuffer != nullptr)
    {
        OCT_ASSERT(resource->mVertexBuffer->GetBuffer()->GetSize() == skinnedVertices.size() * sizeof(Vertex));
        resource->mVertexBuffer->Update(skinnedVertices.data(), skinnedVertices.size() * sizeof(Vertex), 0);
    }
}

void BindGeometryDescriptorSet(SkeletalMesh3D* skeletalMeshComp)
{
    VkCommandBuffer cb = GetCommandBuffer();
    SkeletalMeshCompResource* resource = skeletalMeshComp->GetResource();

    Renderer* renderer = Renderer::Get();
    OCT_UNUSED(renderer);

    World* world = skeletalMeshComp->GetWorld();
    Camera3D* camera = world->GetActiveCamera();
    uint32_t numBoneInfluences = 1;

    switch (skeletalMeshComp->GetBoneInfluenceMode())
    {
    case BoneInfluenceMode::One: numBoneInfluences = 1; break;
    case BoneInfluenceMode::Four: numBoneInfluences = 4; break;
    default: numBoneInfluences = 1; break;
    }

    glm::mat4 transform = skeletalMeshComp->GetRenderTransform();

    UniformBlock uniformBlock;

    if (!IsCpuSkinningRequired(skeletalMeshComp))
    {
        SkinnedGeometryData ubo = {};
        WriteGeometryUniformData(ubo.mBase, world, skeletalMeshComp, transform);
        GatherGeometryLightUniformData(ubo.mBase, skeletalMeshComp, skeletalMeshComp->GetMaterial(), false);

        for (uint32_t i = 0; i < skeletalMeshComp->GetNumBones(); ++i)
        {
            ubo.mBoneMatrices[i] = skeletalMeshComp->GetBoneTransform(i);
        }
        ubo.mNumBoneInfluences = numBoneInfluences;

        uniformBlock = WriteUniformBlock(&ubo, sizeof(ubo));
    }
    else
    {
        GeometryData ubo = {};
        WriteGeometryUniformData(ubo, world, skeletalMeshComp, transform);
        GatherGeometryLightUniformData(ubo, skeletalMeshComp, skeletalMeshComp->GetMaterial(), false);

        uniformBlock = WriteUniformBlock(&ubo, sizeof(ubo));
    }

    DescriptorSet::Begin("SkeletalMesh3D DS")
        .WriteUniformBuffer(GD_UNIFORM_BUFFER, uniformBlock)
        .Build()
        .Bind(cb, 1);
}

void DrawSkeletalMeshComp(SkeletalMesh3D* skeletalMeshComp)
{
    SkeletalMeshCompResource* resource = skeletalMeshComp->GetResource();
    SkeletalMesh* mesh = skeletalMeshComp->GetSkeletalMesh();

    if (mesh != nullptr)
    {
        VkCommandBuffer cb = GetCommandBuffer();

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

        Material* material = nullptr;

        if (GetVulkanContext()->AreMaterialsEnabled())
        {
            material = skeletalMeshComp->GetMaterial();
            material = material ? material : Renderer::Get()->GetDefaultMaterial();
        }

        VertexType vertType = IsCpuSkinningRequired(skeletalMeshComp) ? VertexType::Vertex : VertexType::VertexSkinned;
        BindForwardVertexType(vertType, material);
        BindMaterialResource(material);
        GetVulkanContext()->CommitPipeline();

        BindGeometryDescriptorSet(skeletalMeshComp);
        BindMaterialDescriptorSet(material);

        vkCmdDrawIndexed(cb,
            mesh->GetNumIndices(),
            1,
            0,
            0,
            0);
    }
}

bool IsCpuSkinningRequired(SkeletalMesh3D* skeletalMeshComp)
{
#if PLATFORM_ANDROID
    // GPU skinning is really slow on Android because of the massive vertex data size.
    // It can be optimized for sure, but for now, just don't use it on Android.
    // Ways to optimize:
    // - Use one Uint for bone weights
    // - Use different shader with only one texcoord if second isn't used.
    // - Use variant that doesn't take bone weights and does single-bone skinning.
    return true;
#else
    if (skeletalMeshComp->GetSkeletalMesh() == nullptr)
    {
        return false;
    }
    else
    {
        return skeletalMeshComp->GetSkeletalMesh()->GetNumBones() > MAX_GPU_BONES;
    }
#endif
}

void DrawShadowMeshComp(ShadowMesh3D* shadowMeshComp)
{
    VulkanContext* context = GetVulkanContext();
    StaticMesh* mesh = shadowMeshComp->GetStaticMesh();
    StaticMeshCompResource* resource = shadowMeshComp->GetResource();

    if (GetVulkanContext()->AreMaterialsEnabled() &&
        mesh != nullptr)
    {
        VkCommandBuffer cb = GetCommandBuffer();

        BindStaticMeshResource(mesh);

        // Step 1, render backfaces and write the desired shadow intensity to the scene color's Alpha channel.
        // Depth test is reversed.
        BindPipelineConfig(PipelineConfig::ShadowMeshBack);
        context->SetVertexType(shadowMeshComp->GetVertexType());
        GetVulkanContext()->CommitPipeline();
        BindGeometryDescriptorSet(shadowMeshComp);
        vkCmdDrawIndexed(cb, mesh->GetNumIndices(), 1, 0, 0, 0);

        // Step 2, render front faces and blend the shadow color to the scene colors's RGB channels based on the scene color's Alpha.
        // Depth test is normal
        BindPipelineConfig(PipelineConfig::ShadowMeshFront);
        context->SetVertexType(shadowMeshComp->GetVertexType());
        GetVulkanContext()->CommitPipeline();
        BindGeometryDescriptorSet(shadowMeshComp);
        vkCmdDrawIndexed(cb, mesh->GetNumIndices(), 1, 0, 0, 0);
    }
}

static void UpdateInstancedMeshResource(InstancedMesh3D* instancedMeshComp)
{
    Camera3D* camera = instancedMeshComp->GetWorld()->GetActiveCamera();

    InstancedMeshCompResource* instResource = instancedMeshComp->GetInstancedMeshResource();
    uint32_t numInstances = instancedMeshComp->GetNumInstances();
    uint32_t totalNumVerts = instancedMeshComp->GetTotalVertexCount();

    // Destroy existing buffers
    if (instResource->mInstanceDataBuffer != nullptr)
    {
        GetDestroyQueue()->Destroy(instResource->mInstanceDataBuffer);
        instResource->mInstanceDataBuffer = nullptr;
    }

    if (instResource->mVertexColorBuffer != nullptr)
    {
        GetDestroyQueue()->Destroy(instResource->mVertexColorBuffer);
        instResource->mVertexColorBuffer = nullptr;
    }

    // Generate instance data
    const std::vector<MeshInstanceData>& meshInstanceData = instancedMeshComp->GetInstanceData();
    std::vector<MeshInstanceBufferData> meshInstanceBufferData;
    meshInstanceBufferData.resize(numInstances);
    OCT_ASSERT(meshInstanceData.size() == numInstances);

    for (uint32_t i = 0; i < numInstances; ++i)
    {
        meshInstanceBufferData[i].mTransform = instancedMeshComp->CalculateInstanceTransform(i);
    }

    // Allocate and fill the buffers
    instResource->mInstanceDataBuffer = new Buffer(
        BufferType::Storage,
        sizeof(MeshInstanceBufferData) * numInstances,
        "InstanceDataBuffer",
        meshInstanceBufferData.data(),
        false);

    instResource->mDirty = false;
}

void DrawInstancedMeshComp(InstancedMesh3D* instancedMeshComp)
{
    VulkanContext* context = GetVulkanContext();
    StaticMesh* mesh = instancedMeshComp->GetStaticMesh();
    StaticMeshCompResource* resource = instancedMeshComp->GetResource();
    InstancedMeshCompResource* instResource = instancedMeshComp->GetInstancedMeshResource();

    uint32_t numInstances = instancedMeshComp->GetNumInstances();
    uint32_t totalNumVerts = instancedMeshComp->GetTotalVertexCount();

    if (mesh != nullptr &&
        numInstances > 0)
    {
        if (instResource->mDirty)
        {
            UpdateInstancedMeshResource(instancedMeshComp);
        }

        VkCommandBuffer cb = GetCommandBuffer();

        BindStaticMeshResource(mesh);

        bool useMaterial = GetVulkanContext()->AreMaterialsEnabled();

        // Determine vertex type for binding appropriate pipeline
        VertexType vertexType = VertexType::Vertex;
        if (useMaterial &&
            instancedMeshComp->GetInstanceColors().size() == mesh->GetNumVertices() &&
            resource->mColorVertexBuffer != nullptr)
        {
            if (mesh->HasVertexColor())
            {
                vertexType = VertexType::VertexColorInstanceColor;
            }
            else
            {
                vertexType = VertexType::VertexInstanceColor;
            }

            // Bind color instance buffer at binding #1
            VkBuffer vertexBuffers[] = { resource->mColorVertexBuffer->Get() };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(cb, 1, 1, vertexBuffers, offsets);
        }
        else if (mesh->HasVertexColor())
        {
            vertexType = VertexType::VertexColor;
        }

        Material* material = nullptr;

        if (useMaterial)
        {
            material = instancedMeshComp->GetMaterial();
            material = material ? material : Renderer::Get()->GetDefaultMaterial();
        }

        BindForwardVertexType(vertexType, material, true);
        BindMaterialResource(material);

#if EDITOR
        Shader* prevFragShader = context->GetPipelineState().mFragmentShader;
        if (context->GetCurrentRenderPassId() == RenderPassId::HitCheck)
        {
            Shader* instancedHitCheckFragShader = context->GetGlobalShader("HitCheckInstanced.frag");
            context->SetFragmentShader(instancedHitCheckFragShader);
        }
        else if (context->GetCurrentRenderPassId() == RenderPassId::Selected)
        {
            Shader* instancedSelectedFragShader = context->GetGlobalShader("SelectedInstanced.frag");
            context->SetFragmentShader(instancedSelectedFragShader);
        }
#endif

        GetVulkanContext()->CommitPipeline();

        BindGeometryDescriptorSet(instancedMeshComp);
        BindMaterialDescriptorSet(material);

        vkCmdDrawIndexed(cb,
            mesh->GetNumIndices(),
            numInstances,
            0,
            0,
            0);

#if EDITOR
        if (context->GetCurrentRenderPassId() == RenderPassId::HitCheck || 
            context->GetCurrentRenderPassId() == RenderPassId::Selected)
        {
            context->SetFragmentShader(prevFragShader);
        }
#endif
    }
}

void DestroyTextMeshCompResource(TextMesh3D* textMeshComp)
{
    TextMeshCompResource* resource = textMeshComp->GetResource();

    if (resource->mVertexBuffer != nullptr)
    {
        GetDestroyQueue()->Destroy(resource->mVertexBuffer);
        resource->mVertexBuffer = nullptr;
    }
}

void UpdateTextMeshCompVertexBuffer(TextMesh3D* textMeshComp, const std::vector<Vertex>& vertices)
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

void DrawTextMeshComp(TextMesh3D* textMeshComp)
{
    TextMeshCompResource* resource = textMeshComp->GetResource();
    if (resource->mVertexBuffer == nullptr || textMeshComp->GetNumVisibleCharacters() == 0)
        return;

    VkCommandBuffer cb = GetCommandBuffer();

    VkDeviceSize offset = 0;
    VkBuffer vertexBuffer = resource->mVertexBuffer->Get();
    vkCmdBindVertexBuffers(cb, 0, 1, &vertexBuffer, &offset);

    Material* material = nullptr; 

    if (GetVulkanContext()->AreMaterialsEnabled())
    {
        material = textMeshComp->GetMaterial();
        material = material ? material : Renderer::Get()->GetDefaultMaterial();
    }

    BindForwardVertexType(VertexType::Vertex, material);
    BindMaterialResource(material);
    GetVulkanContext()->CommitPipeline();

    BindGeometryDescriptorSet(textMeshComp);
    BindMaterialDescriptorSet(material);

    vkCmdDraw(cb, TEXT_VERTS_PER_CHAR * textMeshComp->GetNumVisibleCharacters(), 1, 0, 0);
}

void BindGeometryDescriptorSet(TextMesh3D* textMeshComp)
{
    VkCommandBuffer cb = GetCommandBuffer();
    TextMeshCompResource* resource = textMeshComp->GetResource();

    Renderer* renderer = Renderer::Get();
    OCT_UNUSED(renderer);

    World* world = textMeshComp->GetWorld();
    GeometryData ubo = {};

    WriteGeometryUniformData(ubo, world, textMeshComp, textMeshComp->GetRenderTransform());
    GatherGeometryLightUniformData(ubo, textMeshComp, textMeshComp->GetMaterial(), false);

    UniformBlock uniformBlock = WriteUniformBlock(&ubo, sizeof(ubo));
    DescriptorSet::Begin("TextMesh3D DS")
        .WriteUniformBuffer(GD_UNIFORM_BUFFER, uniformBlock)
        .Build()
        .Bind(cb, 1);
}

void DestroyParticleCompResource(Particle3D* particleComp)
{
    ParticleCompResource* resource = particleComp->GetResource();

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

void BindGeometryDescriptorSet(Particle3D* particleComp)
{
    // TODO: Reduce code duplication in updating these uniform buffers...
    VkCommandBuffer cb = GetCommandBuffer();
    ParticleCompResource* resource = particleComp->GetResource();

    Renderer* renderer = Renderer::Get();
    OCT_UNUSED(renderer);

    World* world = particleComp->GetWorld();
    Camera3D* camera = world->GetActiveCamera();

    const glm::mat4 transform = particleComp->GetUseLocalSpace() ? particleComp->GetTransform() : glm::mat4(1);

    GeometryData ubo = {};
    WriteGeometryUniformData(ubo, world, particleComp, transform);
    GatherGeometryLightUniformData(ubo, particleComp, particleComp->GetMaterial(), false);

    UniformBlock uniformBlock = WriteUniformBlock(&ubo, sizeof(ubo));
    DescriptorSet::Begin("Particle3D DS")
        .WriteUniformBuffer(GD_UNIFORM_BUFFER, uniformBlock)
        .Build()
        .Bind(cb, 1);
}

void UpdateParticleCompVertexBuffer(Particle3D* particleComp, const std::vector<VertexParticle>& vertices)
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

        resource->mVertexBuffer = new MultiBuffer(
            BufferType::Vertex,
            numVertices * sizeof(VertexParticle),
            "Particle Vertex Buffer");

        resource->mIndexBuffer = new MultiBuffer(
            BufferType::Index,
            numIndices * sizeof(IndexType),
            "Particle Index Buffer");

        resource->mNumVerticesAllocated = numVertices;
    }

    resource->mVertexBuffer->Update(vertices.data(), numVertices * sizeof(VertexParticle));

    IndexType* indices = (IndexType*)resource->mIndexBuffer->GetBuffer()->Map();
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
    resource->mIndexBuffer->GetBuffer()->Unmap();
}

void DrawParticleComp(Particle3D* particleComp)
{
    if (particleComp->GetNumParticles() > 0 &&
        particleComp->GetNumVertices() > 0)
    {
        ParticleCompResource* resource = particleComp->GetResource();
        VkCommandBuffer cb = GetCommandBuffer();

        Material* material = nullptr;
        
        if (GetVulkanContext()->AreMaterialsEnabled())
        {
            material = particleComp->GetMaterial();
            material = material ? material : Renderer::Get()->GetDefaultMaterial();
        }

        VkDeviceSize offset = 0;
        VkBuffer vertexBuffer = resource->mVertexBuffer->Get();
        vkCmdBindVertexBuffers(cb, 0, 1, &vertexBuffer, &offset);
        vkCmdBindIndexBuffer(cb, resource->mIndexBuffer->Get(), 0, VK_INDEX_TYPE_UINT32);

        BindForwardVertexType(VertexType::VertexParticle, material);
        BindMaterialResource(material);
        GetVulkanContext()->CommitPipeline();

        BindGeometryDescriptorSet(particleComp);
        BindMaterialDescriptorSet(material);

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
    resource->mVertexBuffer = new MultiBuffer(BufferType::Vertex, 4 * sizeof(VertexUI), "Quad Vertices");
}

void DestroyQuadResource(Quad* quad)
{
    QuadResource* resource = quad->GetResource();

    if (resource->mVertexBuffer != nullptr)
    {
        GetDestroyQueue()->Destroy(resource->mVertexBuffer);
        resource->mVertexBuffer = nullptr;
    }
}

void UpdateQuadResourceVertexData(Quad* quad)
{
    QuadResource* resource = quad->GetResource();
    resource->mVertexBuffer->Update(quad->GetVertices(), sizeof(VertexUI) * 4, 0);
}

void BindGeometryDescriptorSet(Quad* quad)
{
    VkCommandBuffer cb = GetCommandBuffer();

    // Uniform Buffer
    QuadUniformData ubo = {};
    ubo.mTransform = glm::mat4(quad->GetTransform());
    ubo.mColor = quad->GetColor();
    UniformBlock uniformBlock = WriteUniformBlock(&ubo, sizeof(ubo));

    // Descriptor Set
    Renderer* renderer = Renderer::Get();
    Texture* texture = quad->GetTexture() ? quad->GetTexture() : renderer->mWhiteTexture.Get<Texture>();

    DescriptorSet::Begin("Quad DS")
        .WriteUniformBuffer(0, uniformBlock)
        .WriteImage(1, texture->GetResource()->mImage)
        .Build()
        .Bind(cb, 1);
}

void DrawQuad(Quad* quad)
{
    QuadResource* resource = quad->GetResource();
    VkCommandBuffer cb = GetCommandBuffer();
    VulkanContext* context = GetVulkanContext();

    // Make sure to bind the quad pipeline. Quad and text rendering will be interleaved.
    BindPipelineConfig(PipelineConfig::Quad);

    VkDeviceSize offset = 0;
    VkBuffer vertexBuffer = resource->mVertexBuffer->Get();
    vkCmdBindVertexBuffers(cb, 0, 1, &vertexBuffer, &offset);

    context->CommitPipeline();

    BindGeometryDescriptorSet(quad);

    vkCmdDraw(cb, 4, 1, 0, 0);
}

void CreateTextResource(Text* text)
{
    TextResource* resource = text->GetResource();

    CreateTextResourceVertexBuffer(text);
    UpdateTextResourceVertexData(text);
}

void DestroyTextResource(Text* text)
{
    TextResource* resource = text->GetResource();

    DestroyTextResourceVertexBuffer(text);
}

void CreateTextResourceVertexBuffer(Text* text)
{
    TextResource* resource = text->GetResource();

    if (text->GetText().size() > 0 && text->GetNumCharactersAllocated() > 0)
    {
        resource->mVertexBuffer = new MultiBuffer(BufferType::Vertex, text->GetNumCharactersAllocated() * TEXT_VERTS_PER_CHAR * sizeof(VertexUI), "Text Vertex Buffer");
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

void BindGeometryDescriptorSet(Text* text)
{
    VkCommandBuffer cb = GetCommandBuffer();
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

    UniformBlock uniformBlock = WriteUniformBlock(&ubo, sizeof(ubo));

    // Descriptors
    Renderer* renderer = Renderer::Get();
    Texture* texture = renderer->mWhiteTexture.Get<Texture>();
    Font* font = text->GetFont();

    if (font != nullptr &&
        font->GetTexture() != nullptr)
    {
        texture = font->GetTexture();
    }

    DescriptorSet::Begin("Text DS")
        .WriteUniformBuffer(0, uniformBlock)
        .WriteImage(1, texture->GetResource()->mImage)
        .Build()
        .Bind(cb, 1);
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
        BindPipelineConfig(PipelineConfig::Text);

        VkDeviceSize offset = 0;
        VkBuffer vertexBuffer = resource->mVertexBuffer->Get();
        vkCmdBindVertexBuffers(cb, 0, 1, &vertexBuffer, &offset);

        GetVulkanContext()->CommitPipeline();

        BindGeometryDescriptorSet(text);

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
        resource->mVertexBuffer = new MultiBuffer(BufferType::Vertex, numVerts * sizeof(VertexUI), "Poly Vertices");
        resource->mNumVerts = numVerts;
    }

    UpdatePolyResourceVertexData(poly);
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
}

void BindGeometryDescriptorSet(Poly* poly)
{
    VkCommandBuffer cb = GetCommandBuffer();
    PolyResource* resource = poly->GetResource();

    // Uniform Buffer
    PolyUniformData ubo = {};
    ubo.mTransform = glm::mat4(poly->GetTransform());
    ubo.mColor = poly->GetColor();
    ubo.mX = poly->GetRect().mX;
    ubo.mY = poly->GetRect().mY;
    ubo.mPad0 = 1337.0f;
    ubo.mPad1 = 1337.0f;
    UniformBlock uniformBlock = WriteUniformBlock(&ubo, sizeof(ubo));

    // Descriptor Set
    Renderer* renderer = Renderer::Get();
    Texture* texture = poly->GetTexture() ? poly->GetTexture() : renderer->mWhiteTexture.Get<Texture>();
    
    DescriptorSet::Begin("Poly DS")
        .WriteUniformBuffer(0, uniformBlock)
        .WriteImage(1, texture->GetResource()->mImage)
        .Build()
        .Bind(cb, 1);
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
            resource->mVertexBuffer = new MultiBuffer(BufferType::Vertex, numVerts * sizeof(VertexUI), "Poly Vertices");
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

        BindPipelineConfig(PipelineConfig::Poly);

        VkDeviceSize offset = 0;
        VkBuffer vertexBuffer = resource->mVertexBuffer->Get();
        vkCmdBindVertexBuffers(cb, 0, 1, &vertexBuffer, &offset);

        GetVulkanContext()->CommitPipeline();

        BindGeometryDescriptorSet(poly);

        if (GetVulkanContext()->HasFeatureWideLines())
        {
            vkCmdSetLineWidth(cb, poly->GetLineWidth());
        }

        vkCmdDraw(cb, numVerts, 1, 0, 0);
    }
}

void DrawStaticMesh(StaticMesh* mesh, Material* material, const glm::mat4& transform, glm::vec4 color, uint32_t hitCheckId)
{
    OCT_ASSERT(mesh != nullptr);
    if (mesh != nullptr)
    {
        VkCommandBuffer cb = GetCommandBuffer();

        GeometryData ubo = {};
        WriteGeometryUniformData(ubo, Renderer::Get()->GetCurrentWorld(), nullptr, transform);
        ubo.mColor = color;
        ubo.mHitCheckId = hitCheckId;
        UniformBlock uniformBlock = WriteUniformBlock(&ubo, sizeof(ubo));

        BindStaticMeshResource(mesh);

        if (GetVulkanContext()->AreMaterialsEnabled())
        {
            if (material == nullptr)
            {
                material = Renderer::Get()->GetDefaultMaterial();
                OCT_ASSERT(material != nullptr);
            }
        }

        VertexType vertType = mesh->HasVertexColor() ? VertexType::VertexColor : VertexType::Vertex;
        BindForwardVertexType(vertType, material);

        BindMaterialResource(material);
        GetVulkanContext()->CommitPipeline();

        DescriptorSet::Begin("Free Mesh DS")
            .WriteUniformBuffer(GD_UNIFORM_BUFFER, uniformBlock)
            .Build()
            .Bind(cb, 1);
        BindMaterialDescriptorSet(material);

        vkCmdDrawIndexed(cb,
            mesh->GetNumIndices(),
            1,
            0,
            0,
            0);
    }
}
