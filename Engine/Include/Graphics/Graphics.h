#pragma once

#include "GraphicsTypes.h"
#include "Vertex.h"
#include "Line.h"

#include <vector>

class World;
class Actor;
class Texture;
class Material;
class StaticMesh;
class SkeletalMesh;
class StaticMeshComponent;
class SkeletalMeshComponent;
class ShadowMeshComponent;
class TextMeshComponent;
class ParticleComponent;
class CameraComponent;
class TransformComponent;
class Quad;
class Text;
class Poly;

struct FogSettings;

void GFX_Initialize();
void GFX_Shutdown();

void GFX_BeginFrame();
void GFX_EndFrame();
void GFX_BeginScreen(uint32_t screenIndex);
void GFX_BeginView(uint32_t viewIndex);

bool GFX_ShouldCullLights();

void GFX_BeginRenderPass(RenderPassId renderPassId);
void GFX_EndRenderPass();
void GFX_BindPipeline(PipelineId pipelineId, VertexType vertexType = VertexType::Max);

void GFX_SetViewport(int32_t x, int32_t y, int32_t width, int32_t height);
void GFX_SetScissor(int32_t x, int32_t y, int32_t width, int32_t height);
glm::mat4 GFX_MakePerspectiveMatrix(float fovyDegrees, float aspectRatio, float zNear, float zFar);
glm::mat4 GFX_MakeOrthographicMatrix(float left, float right, float bottom, float top, float zNear, float zFar);

void GFX_SetFog(const FogSettings& fogSettings);

void GFX_DrawLines(const std::vector<Line>& lines);
void GFX_DrawFullscreen();

void GFX_ResizeWindow();
TransformComponent* GFX_ProcessHitCheck(World* world, int32_t x, int32_t y);
uint32_t GFX_GetNumViews();

void GFX_SetFrameRate(int32_t frameRate);

void GFX_PathTrace();

void GFX_BeginLightBake();
void GFX_UpdateLightBake();
void GFX_EndLightBake();
bool GFX_IsLightBakeInProgress();
float GFX_GetLightBakeProgress();

// Texture
void GFX_CreateTextureResource(Texture* texture, std::vector<uint8_t>& data);
void GFX_DestroyTextureResource(Texture* texture);

// Material
void GFX_CreateMaterialResource(Material* material);
void GFX_DestroyMaterialResource(Material* material);

// StaticMesh
void GFX_CreateStaticMeshResource(StaticMesh* staticMesh, bool hasColor, uint32_t numVertices, void* vertices, uint32_t numIndices, IndexType* indices);
void GFX_DestroyStaticMeshResource(StaticMesh* staticMesh);

// SkeletalMesh
void GFX_CreateSkeletalMeshResource(SkeletalMesh* skeletalMesh, uint32_t numVertices, VertexSkinned* vertices, uint32_t numIndices, IndexType* indices);
void GFX_DestroySkeletalMeshResource(SkeletalMesh* skeletalMesh);

// StaticMeshComp
void GFX_CreateStaticMeshCompResource(StaticMeshComponent* staticMeshComp);
void GFX_DestroyStaticMeshCompResource(StaticMeshComponent* staticMeshComp);
void GFX_DrawStaticMeshComp(StaticMeshComponent* staticMeshComp, StaticMesh* meshOverride = nullptr);

// SkeletalMeshComp
void GFX_CreateSkeletalMeshCompResource(SkeletalMeshComponent* skeletalMeshComp);
void GFX_DestroySkeletalMeshCompResource(SkeletalMeshComponent* skeletalMeshComp);
void GFX_ReallocateSkeletalMeshCompVertexBuffer(SkeletalMeshComponent* skeletalMeshComp, uint32_t numVertices);
void GFX_UpdateSkeletalMeshCompVertexBuffer(SkeletalMeshComponent* skeletalMeshComp, const std::vector<Vertex>& skinnedVertices);
void GFX_DrawSkeletalMeshComp(SkeletalMeshComponent* skeletalMeshComp);
bool GFX_IsCpuSkinningRequired(SkeletalMeshComponent* skeletalMeshComp);

// ShadowMeshComp
// ShadowMeshComponent uses StaticMeshCompResource for now.
void GFX_DrawShadowMeshComp(ShadowMeshComponent* shadowMeshComp);

// TextMeshComp
void GFX_CreateTextMeshCompResource(TextMeshComponent* textMeshComp);
void GFX_DestroyTextMeshCompResource(TextMeshComponent* textMeshComp);
void GFX_UpdateTextMeshCompVertexBuffer(TextMeshComponent* textMeshComp, const std::vector<Vertex>& vertices);
void GFX_DrawTextMeshComp(TextMeshComponent* textMeshComp);

// ParticleComp
void GFX_CreateParticleCompResource(ParticleComponent* particleComp);
void GFX_DestroyParticleCompResource(ParticleComponent* particleComp);
void GFX_UpdateParticleCompVertexBuffer(ParticleComponent* particleComp, const std::vector<VertexParticle>& vertices);
void GFX_DrawParticleComp(ParticleComponent* particleComp);

// Quad
void GFX_CreateQuadResource(Quad* quad);
void GFX_DestroyQuadResource(Quad* quad);
void GFX_UpdateQuadResource(Quad* quad);
void GFX_DrawQuad(Quad* quad);

// Text
void GFX_CreateTextResource(Text* text);
void GFX_DestroyTextResource(Text* text);
void GFX_UpdateTextResourceUniformData(Text* text);
void GFX_UpdateTextResourceVertexData(Text* text);
void GFX_DrawText(Text* text);

// Polygon
void GFX_CreatePolyResource(Poly* poly);
void GFX_DestroyPolyResource(Poly* poly);
void GFX_UpdatePolyResourceUniformData(Poly* poly);
void GFX_UpdatePolyResourceVertexData(Poly* poly);
void GFX_DrawPoly(Poly* poly);

// Arbitrary mesh draw (for debug drawing)
void GFX_DrawStaticMesh(StaticMesh* mesh, Material* material, const glm::mat4& transform, glm::vec4 color);
