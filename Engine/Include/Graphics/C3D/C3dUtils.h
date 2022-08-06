#pragma once

#if API_C3D

#include "Graphics/C3D/C3dTypes.h"

#include "Assets/Material.h"

#include <glm/glm.hpp>

class StaticMesh;

void CopyMatrixGlmToC3d(C3D_Mtx* dst, const glm::mat4& src);

void BindVertexShader(ShaderId shaderId);
void BindStaticMesh(StaticMesh* mesh);
void BindMaterial(Material* material);
void SetupLighting();
void PrepareForwardRendering();
void PrepareUiRendering();

void UploadUvOffsetScale(int8_t uniformLoc, Material* material);

void ResetTexEnv();
void ResetLightingEnv();

void QueueLinearFree(void* data);
void QueueTexFree(C3D_Tex tex);
void ProcessQueuedFrees();

uint32_t GlmColorToRGBA8(glm::vec4 color);
uint32_t GlmColorToRGB8(glm::vec4 color);

void ConfigTev(C3D_TexEnv* env, uint32_t slot, TevMode mode);

#endif
