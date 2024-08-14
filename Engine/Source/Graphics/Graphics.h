#pragma once

#include "GraphicsTypes.h"
#include "Vertex.h"
#include "Line.h"

#include <vector>

class World;
class Node;
class Texture;
class Material;
class StaticMesh;
class SkeletalMesh;
class StaticMesh3D;
class SkeletalMesh3D;
class ShadowMesh3D;
class InstancedMesh3D;
class TextMesh3D;
class Particle3D;
class Camera3D;
class Node3D;
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
void GFX_SetPipelineState(PipelineConfig config);

void GFX_SetViewport(int32_t x, int32_t y, int32_t width, int32_t height, bool handlePrerotation = true);
void GFX_SetScissor(int32_t x, int32_t y, int32_t width, int32_t height, bool handlePrerotation = true);
glm::mat4 GFX_MakePerspectiveMatrix(float fovyDegrees, float aspectRatio, float zNear, float zFar);
glm::mat4 GFX_MakeOrthographicMatrix(float left, float right, float bottom, float top, float zNear, float zFar);

void GFX_SetFog(const FogSettings& fogSettings);

void GFX_DrawLines(const std::vector<Line>& lines);
void GFX_DrawFullscreen();

void GFX_ResizeWindow();
void GFX_Reset();
Node3D* GFX_ProcessHitCheck(World* world, int32_t x, int32_t y, uint32_t* outInstance = nullptr);
uint32_t GFX_GetNumViews();

void GFX_SetFrameRate(int32_t frameRate);

void GFX_PathTrace();

void GFX_BeginLightBake();
void GFX_UpdateLightBake();
void GFX_EndLightBake();
bool GFX_IsLightBakeInProgress();
float GFX_GetLightBakeProgress();

void GFX_EnableMaterials(bool enable);

void GFX_BeginGpuTimestamp(const char* name);
void GFX_EndGpuTimestamp(const char* name);

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
void GFX_CreateStaticMeshCompResource(StaticMesh3D* staticMeshComp);
void GFX_DestroyStaticMeshCompResource(StaticMesh3D* staticMeshComp);
void GFX_UpdateStaticMeshCompResourceColors(StaticMesh3D* staticMeshComp);
void GFX_DrawStaticMeshComp(StaticMesh3D* staticMeshComp, StaticMesh* meshOverride = nullptr);

// SkeletalMeshComp
void GFX_CreateSkeletalMeshCompResource(SkeletalMesh3D* skeletalMeshComp);
void GFX_DestroySkeletalMeshCompResource(SkeletalMesh3D* skeletalMeshComp);
void GFX_ReallocateSkeletalMeshCompVertexBuffer(SkeletalMesh3D* skeletalMeshComp, uint32_t numVertices);
void GFX_UpdateSkeletalMeshCompVertexBuffer(SkeletalMesh3D* skeletalMeshComp, const std::vector<Vertex>& skinnedVertices);
void GFX_DrawSkeletalMeshComp(SkeletalMesh3D* skeletalMeshComp);
bool GFX_IsCpuSkinningRequired(SkeletalMesh3D* skeletalMeshComp);

// ShadowMeshComp
// ShadowMesh3D uses StaticMeshCompResource for now.
void GFX_DrawShadowMeshComp(ShadowMesh3D* shadowMeshComp);

// InstancedMeshComp
void GFX_DrawInstancedMeshComp(InstancedMesh3D* instancedMeshComp);

// TextMeshComp
void GFX_CreateTextMeshCompResource(TextMesh3D* textMeshComp);
void GFX_DestroyTextMeshCompResource(TextMesh3D* textMeshComp);
void GFX_UpdateTextMeshCompVertexBuffer(TextMesh3D* textMeshComp, const std::vector<Vertex>& vertices);
void GFX_DrawTextMeshComp(TextMesh3D* textMeshComp);

// ParticleComp
void GFX_CreateParticleCompResource(Particle3D* particleComp);
void GFX_DestroyParticleCompResource(Particle3D* particleComp);
void GFX_UpdateParticleCompVertexBuffer(Particle3D* particleComp, const std::vector<VertexParticle>& vertices);
void GFX_DrawParticleComp(Particle3D* particleComp);

// Quad
void GFX_CreateQuadResource(Quad* quad);
void GFX_DestroyQuadResource(Quad* quad);
void GFX_UpdateQuadResourceVertexData(Quad* quad);
void GFX_DrawQuad(Quad* quad);

// Text
void GFX_CreateTextResource(Text* text);
void GFX_DestroyTextResource(Text* text);
void GFX_UpdateTextResourceVertexData(Text* text);
void GFX_DrawText(Text* text);

// Polygon
void GFX_CreatePolyResource(Poly* poly);
void GFX_DestroyPolyResource(Poly* poly);
void GFX_UpdatePolyResourceVertexData(Poly* poly);
void GFX_DrawPoly(Poly* poly);

// Arbitrary mesh draw (for debug drawing)
void GFX_DrawStaticMesh(StaticMesh* mesh, Material* material, const glm::mat4& transform, glm::vec4 color);

// PostProcess
void GFX_RenderPostProcessPasses();
