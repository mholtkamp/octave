#pragma once

#if API_GX

class Texture;
class Material;
class StaticMesh;
class SkeletalMesh;
class StaticMeshComponent;
class SkeletalMeshComponent;

void SetupLights();
void SetupLightingChannels();

void PrepareForwardRendering();
void PrepareUiRendering();

bool IsCpuSkinningRequired(SkeletalMeshComponent* component);

void BindMaterial(Material* material, bool useVertexColor);
void BindStaticMesh(StaticMesh* staticMesh);
void BindSkeletalMesh(SkeletalMesh* skeletalMesh);

#endif