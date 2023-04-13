#pragma once

#if API_VULKAN

#include "Allocator.h"
#include "Vertex.h"

#include "Graphics/GraphicsTypes.h"
#include "Graphics/Vulkan/VulkanTypes.h"
#include "Graphics/Vulkan/DestroyQueue.h"

#include <vulkan/vulkan.h>

class World;
class DestroyQueue;
class Pipeline;

class Texture;
class Material;
class StaticMesh;
class SkeletalMesh;
class StaticMeshComponent;
class SkeletalMeshComponent;
class ShadowMeshComponent;
class TextMeshComponent;
class ParticleComponent;
class TransformComponent;
class Quad;
class Text;
class Poly;

VkFormat ConvertPixelFormat(PixelFormat pixelFormat);

void CreateBuffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer& buffer,
    Allocation& bufferMemory);

void TransitionImageLayout(
    VkImage image,
    VkFormat format,
    VkImageLayout oldLayout,
    VkImageLayout newLayout,
    int32_t mipLevels = 1,
    int32_t layerCount = 1,
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

void CopyBuffer(
    VkBuffer srcBuffer,
    VkBuffer dstBuffer,
    VkDeviceSize size);

void CopyBufferToImage(
    VkBuffer buffer,
    VkImage image,
    uint32_t width,
    uint32_t height);

uint32_t GetFrameIndex();
DestroyQueue* GetDestroyQueue();
VkDevice GetVulkanDevice();
VkCommandBuffer GetCommandBuffer();

void DeviceWaitIdle();

VkCommandBuffer BeginCommandBuffer();
void EndCommandBuffer(VkCommandBuffer commandBuffer);
uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

uint32_t GetFormatPixelSize(VkFormat format);
uint32_t GetFormatBlockSize(VkFormat format);
bool IsFormatBlockCompressed(VkFormat format);
VkImageAspectFlags GetFormatImageAspect(VkFormat format);

VkVertexInputBindingDescription GetVertexBindingDescription(VertexType type);
std::vector<VkVertexInputAttributeDescription> GetVertexAttributeDescriptions(VertexType type);

void SetDebugObjectName(VkObjectType objectType, uint64_t object, const char* name);
void BeginDebugLabel(const char* name, glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f });
void EndDebugLabel();

uint32_t GetHitCheckId(TransformComponent* comp);

void WriteGeometryUniformData(GeometryData& outData, World* world, const glm::mat4& transform);
void WriteMaterialUniformData(MaterialData& outData, Material* material);

#if _DEBUG
void FullPipelineBarrier();
#endif

// Texture
void CreateTextureResource(Texture* texture, uint8_t* pixels);
void DestroyTextureResource(Texture* texture);

// Material
void CreateMaterialResource(Material* material);
void DestroyMaterialResource(Material* material);
void BindMaterialResource(Material* material, Pipeline* pipeline);
void UpdateMaterialResource(Material* material);
Pipeline* GetMaterialPipeline(Material* material);

// StaticMesh
void CreateStaticMeshResource(StaticMesh* staticMesh, bool hasColor, uint32_t numVertices, void* vertices, uint32_t numIndices, IndexType* indices);
void DestroyStaticMeshResource(StaticMesh* staticMesh);
void BindStaticMeshResource(StaticMesh* staticMesh);

// SkeletalMesh
void CreateSkeletalMeshResource(SkeletalMesh* skeletalMesh, uint32_t numVertices, VertexSkinned* vertices, uint32_t numIndices, IndexType* indices);
void DestroySkeletalMeshResource(SkeletalMesh* skeletalMesh);
void BindSkeletalMeshResource(SkeletalMesh* skeletalMesh);
void BindSkeletalMeshResourceIndices(SkeletalMesh* skeletalMesh);

// StaticMeshComp
void CreateStaticMeshCompResource(StaticMeshComponent* staticMeshComp);
void DestroyStaticMeshCompResource(StaticMeshComponent* staticMeshComp);
void UpdateStaticMeshCompResource(StaticMeshComponent* staticMeshComp);
void DrawStaticMeshComp(StaticMeshComponent* staticMeshComp, StaticMesh* meshOverride = nullptr);

// SkeletalMeshComp
void CreateSkeletalMeshCompResource(SkeletalMeshComponent* skeletalMeshComp);
void DestroySkeletalMeshCompResource(SkeletalMeshComponent* skeletalMeshComp);
void ReallocateSkeletalMeshCompVertexBuffer(SkeletalMeshComponent* skeletalMeshComp, uint32_t numVertices);
void UpdateSkeletalMeshCompVertexBuffer(SkeletalMeshComponent* skeletalMeshComp, const std::vector<Vertex>& skinnedVertices);
void UpdateSkeletalMeshCompUniformBuffer(SkeletalMeshComponent* skeletalMeshComp);
void DrawSkeletalMeshComp(SkeletalMeshComponent* skeletalMeshComp);
bool IsCpuSkinningRequired(SkeletalMeshComponent* skeletalMeshComp);

// ShadowMeshComp
void DrawShadowMeshComp(ShadowMeshComponent* shadowMeshComp);

// TextMeshComp
void CreateTextMeshCompResource(TextMeshComponent* textMeshComp);
void DestroyTextMeshCompResource(TextMeshComponent* textMeshComp);
void UpdateTextMeshCompVertexBuffer(TextMeshComponent* textMeshComp, const std::vector<Vertex>& vertices);
void DrawTextMeshComp(TextMeshComponent* textMeshComp);
void UpdateTextMeshCompUniformBuffer(TextMeshComponent* textMeshComp);

// ParticleComp
void CreateParticleCompResource(ParticleComponent* particleComp);
void DestroyParticleCompResource(ParticleComponent* particleComp);
void UpdateParticleCompResource(ParticleComponent* particleComp);
void UpdateParticleCompVertexBuffer(ParticleComponent* particleComp, const std::vector<VertexParticle>& vertices);
void DrawParticleComp(ParticleComponent* particleComp);

// Quad
void CreateQuadResource(Quad* quad);
void DestroyQuadResource(Quad* quad);
void UpdateQuadResource(Quad* quad);
void DrawQuad(Quad* quad);

// Text
void CreateTextResource(Text* text);
void DestroyTextResource(Text* text);
void CreateTextResourceVertexBuffer(Text* text);
void DestroyTextResourceVertexBuffer(Text* text);
void UpdateTextResourceUniformData(Text* text);
void UpdateTextResourceVertexData(Text* text);
void DrawTextWidget(Text* text);

// Poly
void CreatePolyResource(Poly* poly);
void DestroyPolyResource(Poly* poly);
void UpdatePolyResourceUniformData(Poly* poly);
void UpdatePolyResourceVertexData(Poly* poly);
void DrawPoly(Poly* poly);

// Arbitrary mesh draw
void DrawStaticMesh(StaticMesh* mesh, Material* material, const glm::mat4& transform, glm::vec4 color, uint32_t hitCheckId = 0);

#endif