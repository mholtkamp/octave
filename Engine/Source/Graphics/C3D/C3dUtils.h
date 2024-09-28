#pragma once

#if API_C3D

#include "Graphics/C3D/C3dTypes.h"

#include "Assets/Material.h"
#include "Nodes/Widgets/Widget.h"
#include "Maths.h"

class StaticMesh;
class Primitive3D;

void CopyMatrixGlmToC3d(C3D_Mtx* dst, const glm::mat4& src);

void BindVertexShader(ShaderId shaderId);
void BindStaticMesh(StaticMesh* mesh, const void* instanceColors);
void BindMaterial(MaterialLite* material, Primitive3D* primitive, bool useBakedLighting);
void SetupLighting(uint8_t lightingChannels, bool bakedLighting);
void SetupLightEnv(LightEnv& lightEnv, uint8_t lightingChannels, bool bakedLighting);
void PrepareForwardRendering();
void PrepareUiRendering();

void UploadUvOffsetScale(int8_t uniformLoc, MaterialLite* material, uint32_t uvIndex);

void ResetTexEnv();
void ResetLightingEnv();

void QueueLinearFree(void* data);
void QueueTexFree(C3D_Tex tex);
void ProcessQueuedFrees();

uint32_t GlmColorToRGBA8(glm::vec4 color);
uint32_t GlmColorToRGB8(glm::vec4 color);

void ConfigTev(C3D_TexEnv* env, uint32_t slot, TevMode mode, bool vertexColorBlend);
void ApplyWidgetRotation(C3D_Mtx& mtx, Widget* widget);

#endif
