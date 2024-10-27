#if API_C3D

#include "Graphics/Graphics.h"
#include "Graphics/GraphicsTypes.h"

#include "Engine.h"
#include "Renderer.h"
#include "Log.h"
#include "Maths.h"
#include "Assets/Material.h"
#include "Assets/SkeletalMesh.h"
#include "Assets/StaticMesh.h"
#include "Assets/Material.h"
#include "Assets/Texture.h"
#include "Nodes/Widgets/Quad.h"
#include "Nodes/Widgets/Text.h"
#include "Assets/Font.h"

#include "Nodes/3D/StaticMesh3d.h"
#include "Nodes/3D/SkeletalMesh3d.h"
#include "Nodes/3D/ShadowMesh3d.h"
#include "Nodes/3D/TextMesh3d.h"
#include "Nodes/3D/Particle3d.h"

#include "Assertion.h"
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <vector>

#include <3ds.h>
#include <citro3d.h>
#include <tex3ds.h>

#include "Graphics/C3D/C3dTypes.h"
#include "Graphics/C3D/C3dUtils.h"

#include "System/System.h"

#include "StaticMesh_shbin.h"
#include "SkeletalMesh_shbin.h"
#include "Particle_shbin.h"
#include "Quad_shbin.h"
#include "Text_shbin.h"

#define VRAM_TEXTURES 1

#define RGBA8_SIZE 4

C3dContext gC3dContext;

static const uint8_t* sVertexShaderBinaries[uint32_t(ShaderId::Count)] =
{
    StaticMesh_shbin,
    SkeletalMesh_shbin,
    Particle_shbin,
    Quad_shbin,
    Text_shbin
};
static_assert(uint32_t(ShaderId::Count) == 5, "Need to update shader binary array");

static uint32_t sVertexShaderSizes[uint32_t(ShaderId::Count)] =
{
    StaticMesh_shbin_size,
    SkeletalMesh_shbin_size,
    Particle_shbin_size,
    Quad_shbin_size,
    Text_shbin_size
};
static_assert(uint32_t(ShaderId::Count) == 5, "Need to update shader binary size array");

#define CLEAR_COLOR 0x000000FF
//#define CLEAR_COLOR 0x333333FF

#define DISPLAY_TRANSFER_FLAGS \
	(GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | \
	GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | \
	GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))


float FresnelFunc(float x, float power)
{
    return powf(1.0f - fabs(x), power);
}

float ToonFunc(float x, float levels)
{
    const float factor = levels - 1;
    return floorf(0.5f + x * factor) / factor;
}

void GFX_Initialize()
{
    C3D_Init(0xC0000 /*C3D_DEFAULT_CMDBUF_SIZE*/);

    // Initialize the render targets
    gC3dContext.mRenderTargetLeft = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    gC3dContext.mRenderTargetRight = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    gC3dContext.mRenderTargetBottom = C3D_RenderTargetCreate(240, 320, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    C3D_RenderTargetSetOutput(gC3dContext.mRenderTargetLeft, GFX_TOP, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);
    C3D_RenderTargetSetOutput(gC3dContext.mRenderTargetRight, GFX_TOP, GFX_RIGHT, DISPLAY_TRANSFER_FLAGS);
    C3D_RenderTargetSetOutput(gC3dContext.mRenderTargetBottom, GFX_BOTTOM, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);

    // Setup Light LUT
    LightLut_Phong(&gC3dContext.mLightLut[Shininess2], 2);
    LightLut_Phong(&gC3dContext.mLightLut[Shininess4], 4);
    LightLut_Phong(&gC3dContext.mLightLut[Shininess8], 8);
    LightLut_Phong(&gC3dContext.mLightLut[Shininess16], 16);
    LightLut_Phong(&gC3dContext.mLightLut[Shininess32], 32);
    LightLut_Phong(&gC3dContext.mLightLut[Shininess64], 64);

    // Setup Fresnel LUT
    LightLut_FromFunc(&gC3dContext.mFresnelLut[FresnelPower1], FresnelFunc, 1.0, false);
    LightLut_FromFunc(&gC3dContext.mFresnelLut[FresnelPower1_5], FresnelFunc, 1.5, false);
    LightLut_FromFunc(&gC3dContext.mFresnelLut[FresnelPower2], FresnelFunc, 2.0, false);
    LightLut_FromFunc(&gC3dContext.mFresnelLut[FresnelPower4], FresnelFunc, 4.0, false);

    // Toon LUT
    LightLut_FromFunc(&gC3dContext.mToonLut[ToonLevel2], ToonFunc, 2, false);
    LightLut_FromFunc(&gC3dContext.mToonLut[ToonLevel3], ToonFunc, 3, false);

    // Load shaders
    for (uint32_t i = 0; i < uint32_t(ShaderId::Count); ++i)
    {
        gC3dContext.mShaderDvlbs[i] = DVLB_ParseFile((uint32_t*)(sVertexShaderBinaries[i]), sVertexShaderSizes[i]);
        shaderProgramInit(&gC3dContext.mShaderPrograms[i]);
        shaderProgramSetVsh(&gC3dContext.mShaderPrograms[i], &gC3dContext.mShaderDvlbs[i]->DVLE[0]);
    }

    // Fetch uniform locations

    // StaticMesh Uniforms
    {
        shaderInstance_s* shader = gC3dContext.mShaderPrograms[uint32_t(ShaderId::StaticMesh)].vertexShader;
        gC3dContext.mStaticMeshLocs.mWorldViewMtx = shaderInstanceGetUniformLocation(shader, "WorldViewMtx");
        gC3dContext.mStaticMeshLocs.mNormalMtx = shaderInstanceGetUniformLocation(shader, "NormalMtx");
        gC3dContext.mStaticMeshLocs.mProjMtx = shaderInstanceGetUniformLocation(shader, "ProjMtx");
        gC3dContext.mStaticMeshLocs.mUvOffsetScale0 = shaderInstanceGetUniformLocation(shader, "UvOffsetScale0");
        gC3dContext.mStaticMeshLocs.mUvOffsetScale1 = shaderInstanceGetUniformLocation(shader, "UvOffsetScale1");
        gC3dContext.mStaticMeshLocs.mUvMaps = shaderInstanceGetUniformLocation(shader, "UvMaps");
    }

    // SkeletalMesh Uniforms
    {
        shaderInstance_s* shader = gC3dContext.mShaderPrograms[uint32_t(ShaderId::SkeletalMesh)].vertexShader;
        gC3dContext.mSkeletalMeshLocs.mWorldViewMtx = shaderInstanceGetUniformLocation(shader, "WorldViewMtx");
        gC3dContext.mSkeletalMeshLocs.mNormalMtx = shaderInstanceGetUniformLocation(shader, "NormalMtx");
        gC3dContext.mSkeletalMeshLocs.mProjMtx = shaderInstanceGetUniformLocation(shader, "ProjMtx");
        gC3dContext.mSkeletalMeshLocs.mUvOffsetScale0 = shaderInstanceGetUniformLocation(shader, "UvOffsetScale0");
        gC3dContext.mSkeletalMeshLocs.mUvOffsetScale1 = shaderInstanceGetUniformLocation(shader, "UvOffsetScale1");
        gC3dContext.mSkeletalMeshLocs.mUvMaps = shaderInstanceGetUniformLocation(shader, "UvMaps");
        gC3dContext.mSkeletalMeshLocs.mBoneMtx = shaderInstanceGetUniformLocation(shader, "BoneMtx");
    }

    // Particle Uniforms
    {
        shaderInstance_s* shader = gC3dContext.mShaderPrograms[uint32_t(ShaderId::Particle)].vertexShader;
        gC3dContext.mParticleLocs.mWorldViewMtx = shaderInstanceGetUniformLocation(shader, "WorldViewMtx");
        gC3dContext.mParticleLocs.mProjMtx = shaderInstanceGetUniformLocation(shader, "ProjMtx");
        gC3dContext.mParticleLocs.mUvOffsetScale = shaderInstanceGetUniformLocation(shader, "UvOffsetScale");
    }

    // Quad Uniforms
    {
        shaderInstance_s* shader = gC3dContext.mShaderPrograms[uint32_t(ShaderId::Quad)].vertexShader;
        gC3dContext.mQuadLocs.mProjMtx = shaderInstanceGetUniformLocation(shader, "ProjMtx");
    }

    // Text Uniforms
    {
        shaderInstance_s* shader = gC3dContext.mShaderPrograms[uint32_t(ShaderId::Text)].vertexShader;
        gC3dContext.mTextLocs.mWorldViewMtx = shaderInstanceGetUniformLocation(shader, "WorldViewMtx");
        gC3dContext.mTextLocs.mProjMtx = shaderInstanceGetUniformLocation(shader, "ProjMtx");
    }
}

void GFX_Shutdown()
{
    // Load shaders
    for (uint32_t i = 0; i < uint32_t(ShaderId::Count); ++i)
    {
        shaderProgramFree(&gC3dContext.mShaderPrograms[i]);
        DVLB_Free(gC3dContext.mShaderDvlbs[i]);
    }

    C3D_Fini();
}

void GFX_BeginFrame()
{
    gC3dContext.mWorld = Renderer::Get()->GetCurrentWorld();

    if (Renderer::Get()->IsRenderingFirstScreen())
    {
        // According to Oreo:
        // C3D_GetDrawingTime() is the time in miliseconds between C3D_FrameEnd() and the callback when the gpu finishes processing the commands.
        // C3D_GetProcessingTime() is the time in miliseconds between C3D_FrameBegin() and C3D_FrameEnd()
        //LogDebug("Draw: %.2f, Proc: %.2f", C3D_GetDrawingTime(), C3D_GetProcessingTime());
        GetProfiler()->SetGpuStatTime("Draw", C3D_GetDrawingTime());
        GetProfiler()->SetGpuStatTime("Processing", C3D_GetProcessingTime());

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

        // Take care of pending deletes.
        ProcessQueuedFrees();
    }

    // Setup lighting now that our light array might have changed this frame
    SetupLighting(0x01, false);

    gC3dContext.mLastBoundShaderId = ShaderId::Count;
    gC3dContext.mLastBoundMaterial = nullptr;
}

void GFX_EndFrame()
{
    if (Renderer::Get()->IsRenderingLastScreen())
    {
        C3D_FrameEnd(0);

        gC3dContext.mFrameIndex = (gC3dContext.mFrameIndex + 1) % MAX_FRAMES;

        // For debugging - Finish processing command list
        //C3D_FrameSplit(GX_CMDLIST_FLUSH);
        //gspWaitForP3D();
    }
}

void GFX_BeginScreen(uint32_t screenIndex)
{
    gC3dContext.mCurrentScreen = screenIndex;
}

void GFX_BeginView(uint32_t viewIndex)
{
    if (gC3dContext.mCurrentScreen == 0)
    {
        gC3dContext.mCurrentView = viewIndex;

        float slider = GetEngineState()->mSystem.mSlider;
        gC3dContext.mIod = slider / 3;

        if (viewIndex == 0)
        {
            gC3dContext.mIod *= -1.0f;
            C3D_RenderTargetClear(gC3dContext.mRenderTargetLeft, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
            C3D_FrameDrawOn(gC3dContext.mRenderTargetLeft);
        }
        else
        {
            C3D_RenderTargetClear(gC3dContext.mRenderTargetRight, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
            C3D_FrameDrawOn(gC3dContext.mRenderTargetRight);
        }

        // Need to recompute perspective matrix
        Camera3D* camera = gC3dContext.mWorld->GetActiveCamera();
        if (camera != nullptr)
        {
            camera->ComputeMatrices();
        }
    }
    else if (gC3dContext.mCurrentScreen == 1)
    {
        // Only a single view on screen 2.
        C3D_RenderTargetClear(gC3dContext.mRenderTargetBottom, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
        C3D_FrameDrawOn(gC3dContext.mRenderTargetBottom);
    }
}

bool GFX_ShouldCullLights()
{
    return true;
}

void GFX_BeginRenderPass(RenderPassId renderPassId)
{
    switch (renderPassId)
    {
    case RenderPassId::Forward:
        PrepareForwardRendering();
        break;
    case RenderPassId::Ui:
        PrepareUiRendering();
        break;
    default:
        break;
    }
}

void GFX_EndRenderPass()
{

}

void GFX_SetPipelineState(PipelineConfig config)
{

}

void GFX_SetViewport(int32_t x, int32_t y, int32_t width, int32_t height, bool handlePrerotation)
{
    // Reverse width and height on 3ds?
    C3D_SetViewport(uint32_t(y), uint32_t(x), uint32_t(height), uint32_t(width));
}

void GFX_SetScissor(int32_t x, int32_t y, int32_t width, int32_t height, bool handlePrerotation)
{
    //C3D_SetScissor(GPU_SCISSOR_NORMAL, uint32_t(y), uint32_t(x), uint32_t(y + height), uint32_t(x + width));

    if (gC3dContext.mCurrentScreen == 0)
    {
        C3D_SetScissor(GPU_SCISSOR_NORMAL, 240 - uint32_t(y + height), 400 - uint32_t(x + width), 240 - uint32_t(y), 400 - uint32_t(x));
    }
    else
    {
        C3D_SetScissor(GPU_SCISSOR_NORMAL, 240 - uint32_t(y + height), 320 - uint32_t(x + width), 240 - uint32_t(y), 320 - uint32_t(x));
    }
}

glm::mat4 GFX_MakePerspectiveMatrix(float fovyDegrees, float aspectRatio, float zNear, float zFar)
{
    C3D_Mtx projection;
    Mtx_PerspStereoTilt(&projection, C3D_AngleFromDegrees(fovyDegrees), aspectRatio /*C3D_AspectRatioTop*/, zNear, zFar, gC3dContext.mIod, 2.0f, false);
    //Mtx_PerspStereoTilt(&projection, C3D_AngleFromDegrees(40.0f), C3D_AspectRatioTop, 0.01f, 1000.0f, iod, 2.0f, false);
    glm::mat4 retMatrix = *reinterpret_cast<glm::mat4*>(&projection);
    return retMatrix;
}

glm::mat4 GFX_MakeOrthographicMatrix(float left, float right, float bottom, float top, float zNear, float zFar)
{
    C3D_Mtx projection;
    Mtx_OrthoTilt(&projection, left, right, bottom, top, zNear, zFar, false);
    //Mtx_OrthoTilt(&projection, 0.0, 400.0, 0.0, 240.0, 0.0, 1.0, true);
    glm::mat4 retMatrix = *reinterpret_cast<glm::mat4*>(&projection);
    return retMatrix;
}

void GFX_SetFog(const FogSettings& fogSettings)
{
    // TODO: Fog isn't really working as expected... I don't understand the fog lut format.
    if (fogSettings.mEnabled)
    {
        Camera3D* camera = gC3dContext.mWorld->GetActiveCamera();
        float camNearZ = camera ? camera->GetNearZ() : 0.1f;
        float camFarZ = camera ? camera->GetFarZ() : 100.0f;
        float nearZ = fogSettings.mNear;
        float farZ = fogSettings.mFar;

        uint32_t color = GlmColorToRGB8(fogSettings.mColor);

        // Only recalculate the fog Lut if densityFunc/near/far values changed.
        if (fogSettings.mNear != gC3dContext.mFogNear ||
            fogSettings.mFar != gC3dContext.mFogFar ||
            fogSettings.mDensityFunc != gC3dContext.mFogDensityFunc)
        {
            gC3dContext.mFogNear = fogSettings.mNear;
            gC3dContext.mFogFar = fogSettings.mFar;
            gC3dContext.mFogDensityFunc = fogSettings.mDensityFunc;

            if (false/*fogSettings.mDensityFunc == FogDensityFunc::Exponential*/)
            {
                FogLut_Exp(&gC3dContext.mFogLut, 0.05f, 1.5f, camNearZ, camFarZ);
            }
            else
            {
                float data[256];

                for (uint32_t i = 0; i <= 128; ++i)
                {
                    float x = FogLut_CalcZ(i / 128.0f, camNearZ, camFarZ);

                    //LogDebug("%d: x = %.2f n = % .2f f = %.2f", i, x, nearZ, farZ);

                    float val = glm::clamp(((x - nearZ) / (farZ - nearZ)), 0.0f, 1.0f);
                    val = 1.0f - val;

                    //LogDebug("val: %f", val);

                    if (i == 0)
                    {
                        data[i] = 0.0f;
                    }
                    else
                    {
                        if (i < 128)
                        {
                            data[i] = val;
                        }

                        data[i + 127] = val - data[i - 1];

                    }
                }

                FogLut_FromArray(&gC3dContext.mFogLut, data);
            }
        }

        C3D_FogGasMode(GPU_FOG, GPU_DEPTH_DENSITY, false);
        C3D_FogColor(color);
        C3D_FogLutBind(&gC3dContext.mFogLut);
    }
    else
    {
        C3D_FogGasMode(GPU_NO_FOG, GPU_PLAIN_DENSITY, false);
    }

    gC3dContext.mFogEnabled = fogSettings.mEnabled;
    gC3dContext.mMaterialApplyFog = fogSettings.mEnabled;
}

void GFX_DrawLines(const std::vector<Line>& lines)
{

}

void GFX_DrawFullscreen()
{

}

void GFX_ResizeWindow()
{

}

void GFX_Reset()
{

}

Node3D* GFX_ProcessHitCheck(World* world, int32_t x, int32_t y, uint32_t* outInstance)
{
    return nullptr;
}

uint32_t GFX_GetNumViews()
{
    SystemState& system = GetEngineState()->mSystem;
    return (gC3dContext.mCurrentScreen == 0 && system.mSlider > 0.0f) ? 2 : 1;
}

void GFX_SetFrameRate(int32_t frameRate)
{
    C3D_FrameRate(float(frameRate));
}

void GFX_PathTrace()
{

}

void GFX_BeginLightBake()
{

}

void GFX_UpdateLightBake()
{

}

void GFX_EndLightBake()
{

}

bool GFX_IsLightBakeInProgress()
{
    return false;
}

float GFX_GetLightBakeProgress()
{
    return 0.0f;
}

void GFX_EnableMaterials(bool enable)
{

}

void GFX_BeginGpuTimestamp(const char* name)
{

}

void GFX_EndGpuTimestamp(const char* name)
{

}

// Texture
void GFX_CreateTextureResource(Texture* texture, std::vector<uint8_t>& data)
{
    TextureResource* resource = texture->GetResource();

    resource->mT3dsData = SYS_AlignedMalloc((uint32_t)data.size(), 32);
    memcpy(resource->mT3dsData, data.data(), data.size());

    Tex3DS_Texture t3x = Tex3DS_TextureImport(
        resource->mT3dsData,
        (uint32_t)data.size(),
        &resource->mTex,
        nullptr,
        true);

    if (!t3x)
    {
        LogError("Failed to import t3x file");
    }

    // Delete the t3x object since we don't need it
    Tex3DS_TextureFree(t3x);

    free(resource->mT3dsData);
    resource->mT3dsData = nullptr;

    GPU_TEXTURE_FILTER_PARAM gpuFilter = GPU_LINEAR;
    GPU_TEXTURE_WRAP_PARAM gpuWrap = GPU_REPEAT;

    switch (texture->GetFilterType())
    {
    case FilterType::Linear: gpuFilter = GPU_LINEAR; break;
    case FilterType::Nearest: gpuFilter = GPU_NEAREST; break;
    default: break;
    }

    switch (texture->GetWrapMode())
    {
    case WrapMode::Clamp: gpuWrap = GPU_CLAMP_TO_EDGE; break;
    case WrapMode::Repeat: gpuWrap = GPU_REPEAT; break;
    case WrapMode::Mirror: gpuWrap = GPU_MIRRORED_REPEAT; break;
    default: break;
    }

    C3D_TexSetFilter(&resource->mTex, gpuFilter, gpuFilter);
    C3D_TexSetWrap(&resource->mTex, gpuWrap, gpuWrap);

    if (texture->IsMipmapped())
    {
        C3D_TexSetFilterMipmap(&resource->mTex, GPU_LINEAR);
    }
}

void GFX_DestroyTextureResource(Texture* texture)
{
    TextureResource* resource = texture->GetResource();
    QueueTexFree(resource->mTex);

    if (resource->mT3dsData != nullptr)
    {
        SYS_AlignedFree(resource->mT3dsData);
        resource->mT3dsData = nullptr;
    }
}

// Material
void GFX_CreateMaterialResource(Material* material)
{

}

void GFX_DestroyMaterialResource(Material* material)
{

}

// StaticMesh
void GFX_CreateStaticMeshResource(StaticMesh* staticMesh, bool hasColor, uint32_t numVertices, void* vertices, uint32_t numIndices, IndexType* indices)
{
    StaticMeshResource* resource = staticMesh->GetResource();

    uint32_t vertexSize = hasColor ? sizeof(VertexColor) : sizeof(Vertex);

    resource->mVertexData = linearAlloc(numVertices * vertexSize);
    resource->mIndexData = linearAlloc(numIndices * sizeof(IndexType));

    memcpy(resource->mVertexData, vertices, numVertices * vertexSize);
    memcpy(resource->mIndexData, indices, numIndices * sizeof(IndexType));

    // Flush the data cache so the GPU gets the updated data.
    GSPGPU_FlushDataCache(resource->mVertexData, numVertices * vertexSize);
    GSPGPU_FlushDataCache(resource->mIndexData, numIndices * sizeof(IndexType));

    // TODO: Use VRAM for static mesh vertex/index data
    // This snippet was taken from 
    // https://github.com/XProger/OpenLara/blob/a4eac0afa15bf11c1c62544cd62f50f277fa2a6c/src/gapi/c3d.h
#if 0
    void mmCopy(void *dst, void *src, size_t size) {
        if (mmIsVRAM(dst)) {
            GSPGPU_FlushDataCache(src, size);
            GX_RequestDma((u32*)src, (u32*)dst, size);
            gspWaitForDMA();
        }
        else {
            memcpy(dst, src, size);
            GSPGPU_FlushDataCache(dst, size);
        }
    }
#endif

}

void GFX_DestroyStaticMeshResource(StaticMesh* staticMesh)
{
    StaticMeshResource* resource = staticMesh->GetResource();

    if (resource->mVertexData != nullptr)
    {
        QueueLinearFree(resource->mVertexData);
        resource->mVertexData = nullptr;
    }

    if (resource->mIndexData != nullptr)
    {
        QueueLinearFree(resource->mIndexData);
        resource->mIndexData = nullptr;
    }
}

// SkeletalMesh
void GFX_CreateSkeletalMeshResource(SkeletalMesh* skeletalMesh, uint32_t numVertices, VertexSkinned* vertices, uint32_t numIndices, IndexType* indices)
{
    SkeletalMeshResource* resource = skeletalMesh->GetResource();
    resource->mVertexData = linearAlloc(sizeof(VertexSkinned) * numVertices);
    resource->mIndexData = linearAlloc(numIndices * sizeof(IndexType));

    memcpy(resource->mVertexData, vertices, sizeof(VertexSkinned) * numVertices);
    memcpy(resource->mIndexData, indices, sizeof(IndexType) * numIndices);

    // Flush the data cache so the GPU gets the updated data.
    GSPGPU_FlushDataCache(resource->mVertexData, sizeof(VertexSkinned) * numVertices);
    GSPGPU_FlushDataCache(resource->mIndexData, sizeof(IndexType) * numIndices);
}

void GFX_DestroySkeletalMeshResource(SkeletalMesh* skeletalMesh)
{
    SkeletalMeshResource* resource = skeletalMesh->GetResource();

    if (resource->mVertexData != nullptr)
    {
        QueueLinearFree(resource->mVertexData);
        resource->mVertexData = nullptr;
    }

    if (resource->mIndexData != nullptr)
    {
        QueueLinearFree(resource->mIndexData);
        resource->mIndexData = nullptr;
    }
}

// StaticMeshComp
void GFX_CreateStaticMeshCompResource(StaticMesh3D* staticMeshComp)
{

}

void GFX_DestroyStaticMeshCompResource(StaticMesh3D* staticMeshComp)
{

}

void GFX_UpdateStaticMeshCompResourceColors(StaticMesh3D* staticMeshComp)
{
    StaticMeshCompResource* resource = staticMeshComp->GetResource();

    const std::vector<uint32_t>& instanceColors = staticMeshComp->GetInstanceColors();
    uint32_t colorBufferSize = sizeof(uint32_t) * uint32_t(instanceColors.size());

    if (instanceColors.size() == 0)
    {
        if (resource->mColorVertexData != nullptr)
        {
            QueueLinearFree(resource->mColorVertexData);
            resource->mColorVertexData = nullptr;
        }
    }
    else
    {
        if (resource->mColorVertexData != nullptr)
        {
            QueueLinearFree(resource->mColorVertexData);
            resource->mColorVertexData = nullptr;
        }

        if (resource->mColorVertexData == nullptr)
        {
            resource->mColorVertexData = linearAlloc(colorBufferSize);
            memcpy(resource->mColorVertexData, instanceColors.data(), colorBufferSize);
            GSPGPU_FlushDataCache(resource->mColorVertexData, colorBufferSize);
        }
    }
}

void GFX_DrawStaticMeshComp(StaticMesh3D* staticMeshComp, StaticMesh* meshOverride)
{
    StaticMeshCompResource* meshCompResource = staticMeshComp->GetResource();
    StaticMesh* mesh = meshOverride ? meshOverride : staticMeshComp->GetStaticMesh();

    if (mesh != nullptr)
    {
        bool useBakedLighting = staticMeshComp->HasBakedLighting();
        bool hasInstanceColors = staticMeshComp->HasInstanceColors();
        const void* instanceColors = nullptr;

        if (hasInstanceColors)
        {
            OCT_ASSERT(meshCompResource->mColorVertexData != nullptr);
            instanceColors = meshCompResource->mColorVertexData;
        }

        BindStaticMesh(mesh, instanceColors);

        MaterialLite* material = Material::AsLite(staticMeshComp->GetMaterial());

        if (material == nullptr)
        {
            material = Renderer::Get()->GetDefaultMaterial();
            OCT_ASSERT(material != nullptr);
        }

        BindMaterial(material, staticMeshComp, useBakedLighting);

        // Upload Uniforms
        C3D_Mtx worldMtx;
        C3D_Mtx viewMtx;
        C3D_Mtx worldViewMtx;

        glm::mat4 modelSrc = staticMeshComp->GetRenderTransform();
        glm::mat4 viewSrc = gC3dContext.mWorld->GetActiveCamera()->GetViewMatrix();

        CopyMatrixGlmToC3d(&worldMtx, modelSrc);
        CopyMatrixGlmToC3d(&viewMtx, viewSrc);
        Mtx_Multiply(&worldViewMtx, &viewMtx, &worldMtx);

        C3D_Mtx projMtx;
        glm::mat4 projSrc = gC3dContext.mWorld->GetActiveCamera()->GetProjectionMatrix();
        memcpy(&projMtx, &projSrc, sizeof(float) * 4 * 4);

        C3D_Mtx normalMtx;
        memcpy(&normalMtx, &worldViewMtx, sizeof(float) * 4 * 4);
        Mtx_Inverse(&normalMtx);
        Mtx_Transpose(&normalMtx);

        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, gC3dContext.mStaticMeshLocs.mWorldViewMtx, &worldViewMtx);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, gC3dContext.mStaticMeshLocs.mNormalMtx, &normalMtx);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, gC3dContext.mStaticMeshLocs.mProjMtx, &projMtx);

        UploadUvOffsetScale(gC3dContext.mStaticMeshLocs.mUvOffsetScale0, material, 0);
        UploadUvOffsetScale(gC3dContext.mStaticMeshLocs.mUvOffsetScale1, material, 1);
        
        C3D_FVUnifSet(GPU_VERTEX_SHADER, gC3dContext.mStaticMeshLocs.mUvMaps, material->GetUvMap(0), material->GetUvMap(1), material->GetUvMap(2), 0);

        // Draw
        C3D_DrawElements(
            GPU_TRIANGLES,
            mesh->GetNumIndices(),
            C3D_UNSIGNED_SHORT,
            mesh->GetResource()->mIndexData);
    }
}

// SkeletalMeshComp
void GFX_CreateSkeletalMeshCompResource(SkeletalMesh3D* skeletalMeshComp)
{

}

void GFX_DestroySkeletalMeshCompResource(SkeletalMesh3D* skeletalMeshComp)
{
    SkeletalMeshCompResource* resource = skeletalMeshComp->GetResource();
    resource->mVertexData.Free();
}

void GFX_ReallocateSkeletalMeshCompVertexBuffer(SkeletalMesh3D* skeletalMeshComp, uint32_t numVertices)
{
    SkeletalMeshCompResource* resource = skeletalMeshComp->GetResource();

    if (GFX_IsCpuSkinningRequired(skeletalMeshComp))
    {
        resource->mVertexData.Free();
        resource->mVertexData.Alloc(numVertices * sizeof(Vertex));
    }
}

void GFX_UpdateSkeletalMeshCompVertexBuffer(SkeletalMesh3D* skeletalMeshComp, const std::vector<Vertex>& skinnedVertices)
{
    SkeletalMeshCompResource* resource = skeletalMeshComp->GetResource();

    if (resource->mVertexData.Get() != nullptr)
    {
        resource->mVertexData.Update(skinnedVertices.data(), skinnedVertices.size() * sizeof(Vertex));
    }
}

void GFX_DrawSkeletalMeshComp(SkeletalMesh3D* skeletalMeshComp)
{
    SkeletalMesh* mesh = skeletalMeshComp->GetSkeletalMesh();

    if (mesh != nullptr)
    {
        // Bind shader program
        bool cpuSkinned = GFX_IsCpuSkinningRequired(skeletalMeshComp);

        int8_t worldViewMtxLoc = -1;
        int8_t normalMtxLoc = -1;
        int8_t projMtxLoc = -1;
        int8_t uvOffsetScaleLoc0 = -1;
        int8_t uvOffsetScaleLoc1 = -1;
        int8_t uvMapsLoc = -1;

        if (cpuSkinned)
        {
            // Use the static mesh program
            BindVertexShader(ShaderId::StaticMesh);

            // Setup vertex attributes
            C3D_AttrInfo* attrInfo = C3D_GetAttrInfo();
            AttrInfo_Init(attrInfo);
            AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // v0=position
            AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 2); // v1=texcoord0
            AttrInfo_AddLoader(attrInfo, 2, GPU_FLOAT, 2); // v2=texcoord1
            AttrInfo_AddLoader(attrInfo, 3, GPU_FLOAT, 3); // v3=normal
            AttrInfo_AddFixed(attrInfo, 4); // v4=color
            C3D_FixedAttribSet(3, 255.0f, 255.0f, 255.0f, 255.0f);

            C3D_BufInfo* bufInfo = C3D_GetBufInfo();
            BufInfo_Init(bufInfo);
            BufInfo_Add(bufInfo, skeletalMeshComp->GetResource()->mVertexData.Get(), sizeof(Vertex), 4, 0x3210);

            worldViewMtxLoc = gC3dContext.mStaticMeshLocs.mWorldViewMtx;
            normalMtxLoc = gC3dContext.mStaticMeshLocs.mNormalMtx;
            projMtxLoc = gC3dContext.mStaticMeshLocs.mProjMtx;
            uvOffsetScaleLoc0 = gC3dContext.mStaticMeshLocs.mUvOffsetScale0;
            uvOffsetScaleLoc1 = gC3dContext.mStaticMeshLocs.mUvOffsetScale1;
            uvMapsLoc = gC3dContext.mStaticMeshLocs.mUvMaps;
        }
        else
        {
            BindVertexShader(ShaderId::SkeletalMesh);

            // Setup vertex attributes
            C3D_AttrInfo* attrInfo = C3D_GetAttrInfo();
            AttrInfo_Init(attrInfo);
            AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3);          // v0=position
            AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 2);          // v1=texcoord0
            AttrInfo_AddLoader(attrInfo, 2, GPU_FLOAT, 2);          // v2=texcoord1
            AttrInfo_AddLoader(attrInfo, 3, GPU_FLOAT, 3);          // v3=normal
            AttrInfo_AddLoader(attrInfo, 4, GPU_UNSIGNED_BYTE, 4);  // v4=boneIndex
            AttrInfo_AddLoader(attrInfo, 5, GPU_FLOAT, 4);          // v5=boneWeight

            C3D_BufInfo* bufInfo = C3D_GetBufInfo();
            BufInfo_Init(bufInfo);
            BufInfo_Add(bufInfo, mesh->GetResource()->mVertexData, sizeof(VertexSkinned), 6, 0x543210);

            // Setup bone uniforms
            for (uint32_t i = 0; i < skeletalMeshComp->GetNumBones(); ++i)
            {
                C3D_Mtx boneMtx;
                glm::mat4 srcMtx = skeletalMeshComp->GetBoneTransform(i);

                CopyMatrixGlmToC3d(&boneMtx, srcMtx);
                C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, gC3dContext.mSkeletalMeshLocs.mBoneMtx + (i * 4), &boneMtx);
            }

            worldViewMtxLoc = gC3dContext.mSkeletalMeshLocs.mWorldViewMtx;
            normalMtxLoc = gC3dContext.mSkeletalMeshLocs.mNormalMtx;
            projMtxLoc = gC3dContext.mSkeletalMeshLocs.mProjMtx;
            uvOffsetScaleLoc0 = gC3dContext.mSkeletalMeshLocs.mUvOffsetScale0;
            uvOffsetScaleLoc1 = gC3dContext.mSkeletalMeshLocs.mUvOffsetScale1;
            uvMapsLoc = gC3dContext.mSkeletalMeshLocs.mUvMaps;
        }

        MaterialLite* material = Material::AsLite(skeletalMeshComp->GetMaterial());

        if (material == nullptr)
        {
            material = Renderer::Get()->GetDefaultMaterial();
            OCT_ASSERT(material != nullptr);
        }

        BindMaterial(material, skeletalMeshComp, false);

        // Upload Uniforms
        C3D_Mtx worldMtx;
        C3D_Mtx viewMtx;
        C3D_Mtx worldViewMtx;

        glm::mat4 modelSrc = skeletalMeshComp->GetRenderTransform();
        glm::mat4 viewSrc = gC3dContext.mWorld->GetActiveCamera()->GetViewMatrix();

        CopyMatrixGlmToC3d(&worldMtx, modelSrc);
        CopyMatrixGlmToC3d(&viewMtx, viewSrc);
        Mtx_Multiply(&worldViewMtx, &viewMtx, &worldMtx);

        C3D_Mtx projMtx;
        glm::mat4 projSrc = gC3dContext.mWorld->GetActiveCamera()->GetProjectionMatrix();
        memcpy(&projMtx, &projSrc, sizeof(float) * 4 * 4);

        C3D_Mtx normalMtx;
        memcpy(&normalMtx, &worldViewMtx, sizeof(float) * 4 * 4);
        Mtx_Inverse(&normalMtx);
        Mtx_Transpose(&normalMtx);

        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, worldViewMtxLoc, &worldViewMtx);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, normalMtxLoc, &normalMtx);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, projMtxLoc, &projMtx);

        UploadUvOffsetScale(uvOffsetScaleLoc0, material, 0);
        UploadUvOffsetScale(uvOffsetScaleLoc1, material, 1);
        C3D_FVUnifSet(GPU_VERTEX_SHADER, uvMapsLoc, material->GetUvMap(0), material->GetUvMap(1), material->GetUvMap(2), 0);

        // Draw
        C3D_DrawElements(
            GPU_TRIANGLES,
            mesh->GetNumIndices(),
            C3D_UNSIGNED_SHORT,
            mesh->GetResource()->mIndexData);
    }
}

bool GFX_IsCpuSkinningRequired(SkeletalMesh3D* skeletalMeshComp)
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

// ShadowMeshComp
void GFX_DrawShadowMeshComp(ShadowMesh3D* shadowMeshComp)
{
    StaticMesh* mesh = shadowMeshComp->GetStaticMesh();

    if (mesh != nullptr)
    {
        BindStaticMesh(mesh, nullptr);

        // Upload Uniforms
        C3D_Mtx worldMtx;
        C3D_Mtx viewMtx;
        C3D_Mtx worldViewMtx;

        glm::mat4 modelSrc = shadowMeshComp->GetRenderTransform();
        glm::mat4 viewSrc = gC3dContext.mWorld->GetActiveCamera()->GetViewMatrix();

        CopyMatrixGlmToC3d(&worldMtx, modelSrc);
        CopyMatrixGlmToC3d(&viewMtx, viewSrc);
        Mtx_Multiply(&worldViewMtx, &viewMtx, &worldMtx);

        C3D_Mtx projMtx;
        glm::mat4 projSrc = gC3dContext.mWorld->GetActiveCamera()->GetProjectionMatrix();
        memcpy(&projMtx, &projSrc, sizeof(float) * 4 * 4);

        C3D_Mtx normalMtx;
        memcpy(&normalMtx, &worldViewMtx, sizeof(float) * 4 * 4);
        Mtx_Inverse(&normalMtx);
        Mtx_Transpose(&normalMtx);

        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, gC3dContext.mStaticMeshLocs.mWorldViewMtx, &worldViewMtx);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, gC3dContext.mStaticMeshLocs.mNormalMtx, &normalMtx);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, gC3dContext.mStaticMeshLocs.mProjMtx, &projMtx);

        ResetTexEnv();
        ResetLightingEnv();

        glm::vec4 shadowColor = gC3dContext.mWorld->GetShadowColor();

        shadowColor = glm::clamp(shadowColor, 0.0f, 1.0f);
        uint8_t matColor4[4] =
        {
            uint8_t(shadowColor.r * 255.0f),
            uint8_t(shadowColor.g * 255.0f),
            uint8_t(shadowColor.b * 255.0f),
            uint8_t(shadowColor.a * 255.0f)
        };

        // Shading, render pure shadow color.
        C3D_TexEnv* env = C3D_GetTexEnv(0);
        C3D_TexEnvInit(env);
        C3D_TexEnvColor(env, *reinterpret_cast<uint32_t*>(matColor4));
        C3D_TexEnvSrc(env, C3D_Both, GPU_CONSTANT, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR);
        C3D_TexEnvFunc(env, C3D_Both, GPU_MODULATE);

        C3D_AlphaTest(false, GPU_ALWAYS, 0);

        // (1) Back faces. Reverse depth test. Write shadow opacity in alpha.
        C3D_DepthTest(true, GPU_LEQUAL, GPU_WRITE_COLOR);
        C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_ZERO, GPU_ONE, GPU_ONE, GPU_ZERO);
        C3D_CullFace(GPU_CULL_FRONT_CCW);
        C3D_DrawElements(GPU_TRIANGLES, mesh->GetNumIndices(), C3D_UNSIGNED_SHORT, mesh->GetResource()->mIndexData);

        // (2) Front faces. Normal depth test. Blend shadow color based on framebuffer alpha.
        C3D_DepthTest(true, GPU_GREATER, GPU_WRITE_COLOR);
        C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_DST_ALPHA, GPU_ONE_MINUS_DST_ALPHA, GPU_ZERO, GPU_ONE);
        C3D_CullFace(GPU_CULL_BACK_CCW);
        C3D_DrawElements(GPU_TRIANGLES, mesh->GetNumIndices(), C3D_UNSIGNED_SHORT, mesh->GetResource()->mIndexData);

        // (3) Clear faces. Normal depth test. Blend shadow color based on framebuffer alpha.
        C3D_DepthTest(false, GPU_ALWAYS, GPU_WRITE_COLOR);
        C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_ZERO, GPU_ONE, GPU_ZERO, GPU_ZERO);
        C3D_CullFace(GPU_CULL_BACK_CCW);
        C3D_DrawElements(GPU_TRIANGLES, mesh->GetNumIndices(), C3D_UNSIGNED_SHORT, mesh->GetResource()->mIndexData);

        gC3dContext.mLastBoundMaterial = nullptr;
    }
}

// InstancedMeshComp
void GFX_DrawInstancedMeshComp(InstancedMesh3D* instancedMeshComp)
{

}

// TextMeshComp
void GFX_CreateTextMeshCompResource(TextMesh3D* textMeshComp)
{

}

void GFX_DestroyTextMeshCompResource(TextMesh3D* textMeshComp)
{

}

void GFX_UpdateTextMeshCompVertexBuffer(TextMesh3D* textMeshComp, const std::vector<Vertex>& vertices)
{
    if (vertices.size() == 0 ||
        textMeshComp->GetNumVisibleCharacters() == 0)
    {
        return;
    }

    TextMeshCompResource* resource = textMeshComp->GetResource();

    uint32_t numVertices = uint32_t(textMeshComp->GetNumVisibleCharacters() * 6);
    OCT_ASSERT(numVertices % 6 == 0);
    OCT_ASSERT(numVertices <= vertices.size());

    size_t newBufferSize = numVertices * sizeof(Vertex);

    if (resource->mVertexData.GetSize() < newBufferSize)
    {
        resource->mVertexData.Free();
        resource->mVertexData.Alloc(newBufferSize);
    }

    resource->mVertexData.Update(vertices.data(), numVertices * sizeof(Vertex));

    // Flush the data cache so the GPU gets the updated data.
    GSPGPU_FlushDataCache(resource->mVertexData.Get(), numVertices * sizeof(Vertex));
}

void GFX_DrawTextMeshComp(TextMesh3D* textMeshComp)
{
    if (textMeshComp->GetNumVisibleCharacters() == 0)
        return;

    TextMeshCompResource* resource = textMeshComp->GetResource();

    uint32_t numChars = (uint32_t)textMeshComp->GetNumVisibleCharacters();
    uint32_t numVertices = numChars * 6;

    // Bind shader program
    BindVertexShader(ShaderId::StaticMesh);

    // Setup vertex attributes
    C3D_AttrInfo* attrInfo = C3D_GetAttrInfo();
    AttrInfo_Init(attrInfo);
    AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // v0=position
    AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 2); // v1=texcoord0
    AttrInfo_AddLoader(attrInfo, 2, GPU_FLOAT, 2); // v2=texcoord1
    AttrInfo_AddLoader(attrInfo, 3, GPU_FLOAT, 3); // v3=normal
    AttrInfo_AddFixed(attrInfo, 4); // v4=color
    C3D_FixedAttribSet(4, 255.0f, 255.0f, 255.0f, 255.0f);

    // Setup vertex buffer state
    C3D_BufInfo* bufInfo = C3D_GetBufInfo();
    BufInfo_Init(bufInfo);
    BufInfo_Add(bufInfo, resource->mVertexData.Get(), sizeof(Vertex), 4, 0x3210);

    MaterialLite* material = Material::AsLite(textMeshComp->GetMaterial());

    if (material == nullptr)
    {
        material = Renderer::Get()->GetDefaultMaterial();
        OCT_ASSERT(material != nullptr);
    }

    BindMaterial(material, textMeshComp, false);

    // Upload Uniforms
    C3D_Mtx worldMtx;
    C3D_Mtx viewMtx;
    C3D_Mtx worldViewMtx;

    glm::mat4 modelSrc = textMeshComp->GetRenderTransform();
    glm::mat4 viewSrc = gC3dContext.mWorld->GetActiveCamera()->GetViewMatrix();

    CopyMatrixGlmToC3d(&worldMtx, modelSrc);
    CopyMatrixGlmToC3d(&viewMtx, viewSrc);
    Mtx_Multiply(&worldViewMtx, &viewMtx, &worldMtx);

    C3D_Mtx projMtx;
    glm::mat4 projSrc = gC3dContext.mWorld->GetActiveCamera()->GetProjectionMatrix();
    memcpy(&projMtx, &projSrc, sizeof(float) * 4 * 4);

    C3D_Mtx normalMtx;
    memcpy(&normalMtx, &worldViewMtx, sizeof(float) * 4 * 4);
    Mtx_Inverse(&normalMtx);
    Mtx_Transpose(&normalMtx);

    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, gC3dContext.mStaticMeshLocs.mWorldViewMtx, &worldViewMtx);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, gC3dContext.mStaticMeshLocs.mNormalMtx, &normalMtx);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, gC3dContext.mStaticMeshLocs.mProjMtx, &projMtx);

    UploadUvOffsetScale(gC3dContext.mStaticMeshLocs.mUvOffsetScale0, material, 0);
    UploadUvOffsetScale(gC3dContext.mStaticMeshLocs.mUvOffsetScale1, material, 1);

    C3D_FVUnifSet(GPU_VERTEX_SHADER, gC3dContext.mStaticMeshLocs.mUvMaps, material->GetUvMap(0), material->GetUvMap(1), material->GetUvMap(2), 0);

    // Draw
    C3D_DrawArrays(GPU_TRIANGLES, 0, numVertices);
}

// ParticleComp
void GFX_CreateParticleCompResource(Particle3D* particleComp)
{

}

void GFX_DestroyParticleCompResource(Particle3D* particleComp)
{
    ParticleCompResource* resource = particleComp->GetResource();

    resource->mVertexData.Free();
    resource->mIndexData.Free();
}

void GFX_UpdateParticleCompVertexBuffer(Particle3D* particleComp, const std::vector<VertexParticle>& vertices)
{
    if (vertices.size() == 0)
        return;

    ParticleCompResource* resource = particleComp->GetResource();

    uint32_t numVertices = uint32_t(vertices.size());
    OCT_ASSERT(numVertices % 4 == 0);
    uint32_t numIndices = (3 * numVertices) / 2; // 6 indices per 4 vertices

    uint32_t vertexCapacity = uint32_t(vertices.capacity());
    OCT_ASSERT(vertexCapacity % 4 == 0);
    uint32_t indexCapacity = (3 * vertexCapacity) / 2; // 6 indices per 4 vertices

    if (resource->mNumVerticesAllocated < vertexCapacity)
    {
        resource->mVertexData.Free();
        resource->mIndexData.Free();

        resource->mVertexData.Alloc(vertexCapacity * sizeof(VertexParticle));
        resource->mIndexData.Alloc(indexCapacity * sizeof(IndexType));

        resource->mNumVerticesAllocated = vertexCapacity;
    }

    IndexType* indices = (IndexType*)resource->mIndexData.Get();
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

    resource->mVertexData.Update(vertices.data(), numVertices * sizeof(VertexParticle));

    // Flush the data cache so the GPU gets the updated data.
    //resource->mIndexData.Update(indices, numIndices * sizeof(IndexType));
    GSPGPU_FlushDataCache(resource->mIndexData.Get(), numIndices * sizeof(IndexType));
}

void GFX_DrawParticleComp(Particle3D* particleComp)
{
    if (particleComp->GetNumParticles() > 0)
    {
        const std::vector<VertexParticle>& vertices = particleComp->GetVertices();
        uint32_t numVertices = (uint32_t)vertices.size();

        BindVertexShader(ShaderId::Particle);

        // Setup vertex attributes
        C3D_AttrInfo* attrInfo = C3D_GetAttrInfo();
        AttrInfo_Init(attrInfo);
        AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // v0=position
        AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 2); // v1=texcoord0
        AttrInfo_AddLoader(attrInfo, 2, GPU_UNSIGNED_BYTE, 4); // v2=color

        // Setup vertex buffer state
        C3D_BufInfo* bufInfo = C3D_GetBufInfo();
        BufInfo_Init(bufInfo);
        BufInfo_Add(bufInfo, particleComp->GetResource()->mVertexData.Get(), sizeof(VertexParticle), 3, 0x210);

        // TODO: Do we need to flush the cache??
        // Maybe only if vertex data is stored in VRAM?
        //DCFlushRange((void*)vertices.data(), numVertices * sizeof(VertexParticle));
        //GX_InvVtxCache();

        MaterialLite* material = Material::AsLite(particleComp->GetMaterial());

        if (material == nullptr)
        {
            material = Renderer::Get()->GetDefaultMaterial();
            OCT_ASSERT(material != nullptr);
        }

        BindMaterial(material, particleComp, false);

        // Upload Uniforms
        C3D_Mtx worldMtx;
        C3D_Mtx viewMtx;
        C3D_Mtx worldViewMtx;

        glm::mat4 modelSrc = particleComp->GetUseLocalSpace() ? particleComp->GetTransform() : glm::mat4(1);
        glm::mat4 viewSrc = gC3dContext.mWorld->GetActiveCamera()->GetViewMatrix();

        CopyMatrixGlmToC3d(&worldMtx, modelSrc);
        CopyMatrixGlmToC3d(&viewMtx, viewSrc);
        Mtx_Multiply(&worldViewMtx, &viewMtx, &worldMtx);

        C3D_Mtx projMtx;
        glm::mat4 projSrc = gC3dContext.mWorld->GetActiveCamera()->GetProjectionMatrix();
        memcpy(&projMtx, &projSrc, sizeof(float) * 4 * 4);

        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, gC3dContext.mParticleLocs.mWorldViewMtx, &worldViewMtx);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, gC3dContext.mParticleLocs.mProjMtx, &projMtx);

        UploadUvOffsetScale(gC3dContext.mParticleLocs.mUvOffsetScale, material, 0);

        // Draw
        uint32_t numParticles = particleComp->GetNumParticles();
        OCT_ASSERT(numParticles * 4 == numVertices);

        C3D_DrawElements(
            GPU_TRIANGLES,
            numParticles * 6,
            C3D_UNSIGNED_SHORT,
            particleComp->GetResource()->mIndexData.Get());
    }
}

// Quad
void GFX_CreateQuadResource(Quad* quad)
{
    QuadResource* resource = quad->GetResource();

    resource->mVertexData.Alloc(4 * sizeof(VertexUI), quad->GetVertices());
}

void GFX_DestroyQuadResource(Quad* quad)
{
    QuadResource* resource = quad->GetResource();
    resource->mVertexData.Free();
}

void GFX_UpdateQuadResourceVertexData(Quad* quad)
{
    QuadResource* resource = quad->GetResource();
    resource->mVertexData.Update(quad->GetVertices(), 4 * sizeof(VertexUI));
}

void GFX_DrawQuad(Quad* quad)
{
    ResetTexEnv();
    ResetLightingEnv();

    QuadResource* resource = quad->GetResource();
    Texture* texture = quad->GetTexture() ? quad->GetTexture() : Renderer::Get()->mWhiteTexture.Get<Texture>();

    // Bind shader program
    BindVertexShader(ShaderId::Quad);

    // Setup vertex attributes
    C3D_AttrInfo* attrInfo = C3D_GetAttrInfo();
    AttrInfo_Init(attrInfo);
    AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 2); // v0=position
    AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 2); // v1=texcoord0
    AttrInfo_AddLoader(attrInfo, 2, GPU_UNSIGNED_BYTE, 4); // v1=color

    // Setup vertex buffer state
    C3D_BufInfo* bufInfo = C3D_GetBufInfo();
    BufInfo_Init(bufInfo);
    BufInfo_Add(bufInfo, resource->mVertexData.Get(), sizeof(VertexUI), 3, 0x210);

    // Setup Texture Environment
    // TEV 0 = Texture sample + vertex color modulation
    C3D_TexBind(0, &texture->GetResource()->mTex);
    C3D_TexEnv* env = C3D_GetTexEnv(0);
    C3D_TexEnvInit(env);
    C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR);
    C3D_TexEnvFunc(env, C3D_Both, GPU_MODULATE);

    // TEV 1 = Constant color modulation
    env = C3D_GetTexEnv(1);
    C3D_TexEnvInit(env);
    C3D_TexEnvSrc(env, C3D_Both, GPU_PREVIOUS, GPU_CONSTANT, GPU_PRIMARY_COLOR);
    C3D_TexEnvFunc(env, C3D_Both, GPU_MODULATE);
    glm::vec4 uniColor = glm::clamp(quad->GetColor(), 0.0f, 1.0f);
    uint8_t uniColor4[4] =
    {
        uint8_t(uniColor.r * 255.0f),
        uint8_t(uniColor.g * 255.0f),
        uint8_t(uniColor.b * 255.0f),
        uint8_t(uniColor.a * 255.0f)
    };
    C3D_TexEnvColor(env, *reinterpret_cast<uint32_t*>(uniColor4));

    // Setup Light Environment

    // Upload Uniforms
    C3D_Mtx modelViewMtx;
    Mtx_Identity(&modelViewMtx);
    ApplyWidgetRotation(modelViewMtx, quad);
    C3D_Mtx projMtx;
    glm::vec2 ires = Renderer::Get()->GetScreenResolution(gC3dContext.mCurrentScreen);
    Mtx_OrthoTilt(&projMtx, 0.0, ires.x, ires.y, 0.0f, -1.0, 1.0, false);
    C3D_Mtx mvpMtx;
    Mtx_Multiply(&mvpMtx, &projMtx, &modelViewMtx);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, gC3dContext.mQuadLocs.mProjMtx, &mvpMtx);

    C3D_CullFace(GPU_CULL_NONE);
    C3D_DepthTest(false, GPU_ALWAYS, GPU_WRITE_COLOR);

    // Draw
    C3D_DrawArrays(GPU_TRIANGLE_STRIP, 0, 4);
}

// Text
void GFX_CreateTextResource(Text* text)
{
    GFX_UpdateTextResourceVertexData(text);
}

void GFX_DestroyTextResource(Text* text)
{
    TextResource* resource = text->GetResource();
    resource->mVertexData.Free();
    resource->mNumBufferCharsAllocated = 0;
}

void GFX_UpdateTextResourceUniformData(Text* text)
{

}

void GFX_UpdateTextResourceVertexData(Text* text)
{
    TextResource* resource = text->GetResource();

    uint32_t numCharsAllocated = text->GetNumCharactersAllocated();
    if (resource->mNumBufferCharsAllocated < numCharsAllocated)
    {
        resource->mVertexData.Free();

        if (text->GetText().size() > 0 && text->GetNumCharactersAllocated() > 0)
        {
            resource->mVertexData.Alloc(text->GetNumCharactersAllocated() * TEXT_VERTS_PER_CHAR * sizeof(VertexUI));
            resource->mNumBufferCharsAllocated = text->GetNumCharactersAllocated();
        }
    }

    if (resource->mVertexData.Get() != nullptr && 
        text->GetNumVisibleCharacters() > 0 &&
        resource->mNumBufferCharsAllocated > 0)
    {
        // Copy over the data
        resource->mVertexData.Update(text->GetVertices(), sizeof(VertexUI) * TEXT_VERTS_PER_CHAR * resource->mNumBufferCharsAllocated);
    }
}

void GFX_DrawText(Text* text)
{
    uint32_t numVisibleChars = text->GetNumVisibleCharacters();

    if (numVisibleChars == 0)
        return;

    ResetTexEnv();
    ResetLightingEnv();

    TextResource* resource = text->GetResource();

    Rect rect = text->GetRect();
    glm::vec2 justOff = text->GetJustifiedOffset();
    Font* font = text->GetFont();

    // Bind shader program
    BindVertexShader(ShaderId::Text);

    // Setup vertex attributes
    C3D_AttrInfo* attrInfo = C3D_GetAttrInfo();
    AttrInfo_Init(attrInfo);
    AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 2); // v0=position
    AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 2); // v1=texcoord0
    AttrInfo_AddLoader(attrInfo, 2, GPU_UNSIGNED_BYTE, 4); // v1=color

    // Setup vertex buffer state
    C3D_BufInfo* bufInfo = C3D_GetBufInfo();
    BufInfo_Init(bufInfo);
    BufInfo_Add(bufInfo, resource->mVertexData.Get(), sizeof(VertexUI), 3, 0x210);

    // Setup Texture Environment
    // TEV 0 = Font texture + vertex color modulation
    C3D_TexBind(0, &font->GetTexture()->GetResource()->mTex);
    C3D_TexEnv* env = C3D_GetTexEnv(0);
    C3D_TexEnvInit(env);
    C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR);
    C3D_TexEnvFunc(env, C3D_Both, GPU_MODULATE);
    C3D_TexEnvOpRgb(env, GPU_TEVOP_RGB_SRC_COLOR, GPU_TEVOP_RGB_SRC_COLOR, GPU_TEVOP_RGB_SRC_COLOR);
    C3D_TexEnvOpAlpha(env, GPU_TEVOP_A_SRC_ALPHA, GPU_TEVOP_A_SRC_ALPHA, GPU_TEVOP_A_SRC_ALPHA);

    // TEV 1 = Constant color modulation
    env = C3D_GetTexEnv(1);
    C3D_TexEnvInit(env);
    C3D_TexEnvSrc(env, C3D_Both, GPU_PREVIOUS, GPU_CONSTANT, GPU_PRIMARY_COLOR);
    C3D_TexEnvFunc(env, C3D_Both, GPU_MODULATE);
    glm::vec4 uniColor = glm::clamp(text->GetColor(), 0.0f, 1.0f);
    uint8_t uniColor4[4] =
    {
        uint8_t(uniColor.r * 255.0f),
        uint8_t(uniColor.g * 255.0f),
        uint8_t(uniColor.b * 255.0f),
        uint8_t(uniColor.a * 255.0f)
    };
    C3D_TexEnvColor(env, *reinterpret_cast<uint32_t*>(uniColor4));

    // Setup Light Environment

    // Upload Uniforms
    C3D_Mtx worldViewMtx;
    glm::vec2 translation = glm::vec2(rect.mX + justOff.x, rect.mY + justOff.y);
    int32_t fontSize = font ? font->GetSize() : 32;
    float textScale = text->GetScaledTextSize() / fontSize;

    Mtx_Identity(&worldViewMtx);
    Mtx_Scale(&worldViewMtx, textScale, textScale, 1.0f);
    Mtx_Translate(&worldViewMtx, translation.x, translation.y, 0.0f, false);
    ApplyWidgetRotation(worldViewMtx, text);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, gC3dContext.mTextLocs.mWorldViewMtx, &worldViewMtx);

    C3D_Mtx projMtx;
    glm::vec2 ires = Renderer::Get()->GetScreenResolution(gC3dContext.mCurrentScreen);
    Mtx_OrthoTilt(&projMtx, 0.0, ires.x, ires.y, 0.0f, -1.0, 1.0, false);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, gC3dContext.mTextLocs.mProjMtx, &projMtx);

    C3D_CullFace(GPU_CULL_NONE);
    C3D_DepthTest(false, GPU_ALWAYS, GPU_WRITE_COLOR);

    // Draw
    C3D_DrawArrays(GPU_TRIANGLES, 0, 6 * numVisibleChars);
}

// Poly
void GFX_CreatePolyResource(Poly* poly)
{

}

void GFX_DestroyPolyResource(Poly* poly)
{

}

void GFX_UpdatePolyResourceUniformData(Poly* poly)
{

}

void GFX_UpdatePolyResourceVertexData(Poly* poly)
{

}

void GFX_DrawPoly(Poly* poly)
{

}

void GFX_DrawStaticMesh(StaticMesh* mesh, Material* material, const glm::mat4& transform, glm::vec4 color)
{
    // TODO: For debug drawing
}

void GFX_RenderPostProcessPasses()
{

}

#endif