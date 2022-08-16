#pragma once

#if API_GX

class Texture;
class StaticMesh;
class SkeletalMesh;
class StaticMeshComponent;
class SkeletalMeshComponent;

#include "Assets/Material.h"

void SetupLights();
void SetupLightingChannels();

void PrepareForwardRendering();
void PrepareUiRendering();

bool IsCpuSkinningRequired(SkeletalMeshComponent* component);

void BindMaterial(Material* material, bool useVertexColor);
void BindStaticMesh(StaticMesh* staticMesh);
void BindSkeletalMesh(SkeletalMesh* skeletalMesh);

void ConfigTev(uint32_t textureSlot, TevMode mode, bool vertexColorBlend);

#endif