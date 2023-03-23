#pragma once

#if API_GX

class Texture;
class StaticMesh;
class SkeletalMesh;
class StaticMeshComponent;
class SkeletalMeshComponent;
class Widget;

#include "Assets/Material.h"

#include <gccore.h>

void SetupLights();
void SetupLightingChannels();

void PrepareForwardRendering();
void PrepareUiRendering();

bool IsCpuSkinningRequired(SkeletalMeshComponent* component);

void BindMaterial(Material* material, bool useVertexColor);
void BindStaticMesh(StaticMesh* staticMesh);
void BindSkeletalMesh(SkeletalMesh* skeletalMesh);

void ConfigTev(uint32_t textureSlot, TevMode mode, bool vertexColorBlend);

void ApplyWidgetRotation(Mtx& mtx, Widget* widget);

#endif