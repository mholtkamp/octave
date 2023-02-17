#if API_GX

#include "Graphics/Graphics.h"
#include "Graphics/GraphicsTypes.h"
#include "Graphics/GX/GxTypes.h"
#include "Graphics/GX/GxUtils.h"

#include "System/System.h"

#include "Engine.h"
#include "Renderer.h"
#include "Log.h"
#include "Assets/Material.h"
#include "Assets/SkeletalMesh.h"
#include "Assets/StaticMesh.h"
#include "Assets/Material.h"
#include "Assets/Texture.h"
#include "Widgets/Quad.h"
#include "Widgets/Text.h"
#include "Assets/Font.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/ParticleComponent.h"
#include "Components/ShadowMeshComponent.h"
#include "Components/TextMeshComponent.h"

#include "Assertion.h"
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include <gccore.h>
#define DEFAULT_FIFO_SIZE    (256*1024)

GxContext gGxContext;

void GFX_Initialize()
{
    LogDebug("GFX_Initialize");

    EngineState* engineState = GetEngineState();
    SystemState* systemState = &engineState->mSystem;
    GXRModeObj* rmode = systemState->mGxrMode;

    // Alloc fifo
    gGxContext.mGpFifo = memalign(32, DEFAULT_FIFO_SIZE);
    memset(gGxContext.mGpFifo, 0, DEFAULT_FIFO_SIZE);

    GX_Init(gGxContext.mGpFifo, DEFAULT_FIFO_SIZE);

    GX_SetCopyClear({ 0, 0x00, 0x00, 0x00 }, 0x00ffffff);


    // other gx setup
    GX_SetViewport(0, 0, rmode->fbWidth, rmode->efbHeight, 0, 1);
    float yscale = GX_GetYScaleFactor(rmode->efbHeight, rmode->xfbHeight);
    uint32_t xfbHeight = GX_SetDispCopyYScale(yscale);
    GX_SetScissor(0, 0, rmode->fbWidth, rmode->efbHeight);
    GX_SetDispCopySrc(0, 0, rmode->fbWidth, rmode->efbHeight);
    GX_SetDispCopyDst(rmode->fbWidth, xfbHeight);
    GX_SetCopyFilter(rmode->aa, rmode->sample_pattern, GX_TRUE, rmode->vfilter);
    GX_SetFieldMode(rmode->field_rendering, ((rmode->viHeight == 2 * rmode->xfbHeight) ? GX_ENABLE : GX_DISABLE));
    GX_SetPixelFmt(GX_PF_RGBA6_Z24, GX_ZC_LINEAR);

    GX_SetCullMode(GX_CULL_FRONT);
    GX_CopyDisp(systemState->mFrameBuffers[systemState->mFrameIndex], GX_TRUE);
    GX_SetDispCopyGamma(GX_GM_1_0);

    Mtx view;
    Mtx44 perspective;

    // setup our camera at the origin
    // looking down the -z axis with y up
    guVector cam = { 0.0F, 0.0F, 10.0F },
        up = { 0.0F, 1.0F, 0.0F },
        look = { 0.0F, 0.0F, -1.0F };
    guLookAt(view, &cam, &up, &look);


    // setup our projection matrix
    // this creates a perspective matrix with a view angle of 90,
    // and aspect ratio based on the display resolution
    f32 w = rmode->viWidth;
    f32 h = rmode->viHeight;
    guPerspective(perspective, 45, (f32)w / h, 0.1F, 300.0F);
    GX_LoadProjectionMtx(perspective, GX_PERSPECTIVE);

    GX_InvVtxCache();
    GX_InvalidateTexAll();
}

void GFX_Shutdown()
{

}

void GFX_BeginFrame()
{
    GXRModeObj* rmode = GetEngineState()->mSystem.mGxrMode;

    SetupLights();

    GX_SetViewport(0, 0, rmode->fbWidth, rmode->efbHeight, 0, 1);

    Mtx44 projection;
    CameraComponent* camera = GetWorld()->GetActiveCamera();
    glm::mat4 camProjection = camera->GetProjectionMatrix();
    memcpy(&projection, &camProjection, sizeof(Mtx44));
    GX_LoadProjectionMtx(projection, GX_PERSPECTIVE);
}

void GFX_EndFrame()
{
    SystemState* systemState = &GetEngineState()->mSystem;
    systemState->mFrameIndex ^= 1; // flip framebuffer

    GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GX_SetColorUpdate(GX_TRUE);
    GX_SetAlphaUpdate(GX_TRUE);
    GX_CopyDisp(systemState->mFrameBuffers[systemState->mFrameIndex], GX_TRUE);
    GX_DrawDone();

    VIDEO_SetNextFramebuffer(systemState->mFrameBuffers[systemState->mFrameIndex]);
    VIDEO_Flush();
    VIDEO_WaitVSync();
}

void GFX_BeginScreen(uint32_t screenIndex)
{

}

void GFX_BeginView(uint32_t viewIndex)
{

}

bool GFX_ShouldCullLights()
{
    return false;
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

void GFX_BindPipeline(PipelineId pipelineId, VertexType vertexType)
{

}

void GFX_SetViewport(int32_t x, int32_t y, int32_t width, int32_t height)
{
    GX_SetViewport((float) x, float(y), (float) width, float(height), 0, 1);
}

void GFX_SetScissor(int32_t x, int32_t y, int32_t width, int32_t height)
{
    GX_SetScissor(x, y, width, height);
}

glm::mat4 GFX_MakePerspectiveMatrix(float fovyDegrees, float aspectRatio, float zNear, float zFar)
{
    Mtx44 projection;
    guPerspective(projection, fovyDegrees, aspectRatio, zNear, zFar);
    glm::mat4 retMatrix = *reinterpret_cast<glm::mat4*>(&projection);
    return retMatrix;
}

glm::mat4 GFX_MakeOrthographicMatrix(float left, float right, float bottom, float top, float zNear, float zFar)
{
    Mtx44 projection;
    guOrtho(projection, top, bottom, left, right, zNear, zFar);
    glm::mat4 retMatrix = *reinterpret_cast<glm::mat4*>(&projection);
    return retMatrix;
}

void GFX_SetFog(const FogSettings& fogSettings)
{
    CameraComponent* camera = GetWorld()->GetActiveCamera();

    float cameraNear = 0.0f;
    float cameraFar = 100.0f;
    uint8_t fogType = GX_FOG_NONE;

    if (camera->GetProjectionMode() == ProjectionMode::ORTHOGRAPHIC)
    {
        OrthoSettings projSettings = camera->GetOrthoSettings();
        cameraNear = projSettings.mNear;
        cameraFar = projSettings.mFar;

        switch (fogSettings.mDensityFunc)
        {
        case FogDensityFunc::Linear: fogType = GX_FOG_ORTHO_LIN; break;
        case FogDensityFunc::Exponential: fogType = GX_FOG_ORTHO_EXP; break;
        default: break;
        }
    }
    else
    {
        PerspectiveSettings projSettings = camera->GetPerspectiveSettings();
        cameraNear = projSettings.mNear;
        cameraFar = projSettings.mFar;

        switch (fogSettings.mDensityFunc)
        {
        case FogDensityFunc::Linear: fogType = GX_FOG_PERSP_LIN; break;
        case FogDensityFunc::Exponential: fogType = GX_FOG_PERSP_EXP; break;
        default: break;
        }
    }

    glm::vec4 clampedColor = glm::clamp(fogSettings.mColor, 0.0f, 1.0f);
    GXColor fogColor =
    {
        uint8_t(clampedColor.r * 255.0f),
        uint8_t(clampedColor.g * 255.0f),
        uint8_t(clampedColor.b * 255.0f),
        uint8_t(clampedColor.a * 255.0f)
    };

    // Save off the fog state so that we can enable/disable on a per-material basis if needed.
    gGxContext.mFogType = fogSettings.mEnabled ? fogType : GX_FOG_NONE;
    gGxContext.mFogStartZ = fogSettings.mNear;
    gGxContext.mFogEndZ = fogSettings.mFar;
    gGxContext.mFogNearZ = cameraNear;
    gGxContext.mFogFarZ = cameraFar;
    gGxContext.mFogColor = fogColor;

    GX_SetFog(
        gGxContext.mFogType,
        gGxContext.mFogStartZ,
        gGxContext.mFogEndZ,
        gGxContext.mFogNearZ,
        gGxContext.mFogFarZ,
        gGxContext.mFogColor);
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

TransformComponent* GFX_ProcessHitCheck(World* world, int32_t x, int32_t y)
{
    return nullptr;
}

uint32_t GFX_GetNumViews()
{
    return 1;
}

void GFX_SetFrameRate(int32_t frameRate)
{

}

// Texture
void GFX_CreateTextureResource(Texture* texture, std::vector<uint8_t>& data)
{
    TextureResource* resource = texture->GetResource();

    GX_InvalidateTexAll();

    resource->mTplData = SYS_AlignedMalloc((uint32_t)data.size(), 32);
    memcpy(resource->mTplData, data.data(), data.size());

    TPL_OpenTPLFromMemory(&resource->mTplFile, resource->mTplData, (uint32_t)data.size());
    TPL_GetTexture(&resource->mTplFile, 0, &resource->mGxTexObj);

    //uint32_t fmt = 0;
    //uint16_t width = 0;
    //uint16_t height = 0;
    //TPL_GetTextureInfo(&resource->mTplFile, 0, &fmt, &width, &height);
    //LogDebug("Loaded TPL ----- fmt: %d, w: %d, h: %d", fmt, width, height);

    bool mipmapped = texture->IsMipmapped();
    uint8_t minFilter = GX_LINEAR;
    uint8_t magFilter = GX_LINEAR;
    uint8_t wrap = GX_REPEAT;

    switch (texture->GetFilterType())
    {
    case FilterType::Nearest: minFilter = mipmapped ? GX_NEAR_MIP_LIN : GX_NEAR; magFilter = GX_NEAR; break;
    case FilterType::Linear: minFilter = mipmapped ? GX_LIN_MIP_LIN : GX_LINEAR; magFilter = GX_LINEAR; break;
    default: break;
    }

    switch (texture->GetWrapMode())
    {
    case WrapMode::Clamp: wrap = GX_CLAMP; break;
    case WrapMode::Repeat: wrap = GX_REPEAT; break;
    case WrapMode::Mirror: wrap = GX_MIRROR; break;
    default: break;
    }

    GX_InitTexObjWrapMode(&resource->mGxTexObj, wrap, wrap);
    GX_InitTexObjFilterMode(&resource->mGxTexObj, minFilter, magFilter);
}

void GFX_DestroyTextureResource(Texture* texture)
{
    TextureResource* resource = texture->GetResource();

    TPL_CloseTPLFile(&resource->mTplFile);

    if (resource->mTplData != nullptr)
    {
        SYS_AlignedFree(resource->mTplData);
        resource->mTplData = nullptr;
    }

    resource->mGxTexObj = { };
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

    // We need to rearrange the vertex color data?
    // Apparently dolphin expects rgba reversed
    if (hasColor)
    {
        VertexColor* vertices = staticMesh->GetColorVertices();
        for (uint32_t i = 0; i < numVertices; ++i)
        {
            uint8_t* charArray = reinterpret_cast<uint8_t*>(&vertices[i].mColor);
            uint8_t c0 = charArray[0];
            uint8_t c1 = charArray[1];
            uint8_t c2 = charArray[2];
            uint8_t c3 = charArray[3];

            charArray[0] = c3;
            charArray[1] = c2;
            charArray[2] = c1;
            charArray[3] = c0;
        }
    }

    // Generate a display list
    uint32_t gxBeginSize = 3;
    uint32_t elemSize = hasColor ? (2 + 2 + 2 + 2 + 2) : (2 + 2 + 2 + 2);
    uint32_t allocSize = gxBeginSize + (elemSize * staticMesh->GetNumFaces() * 3);
    allocSize = (allocSize + 0x1f) & (~0x1f); // 32 byte aligned
    allocSize += 64; // Extra space to account for pipe flush
    resource->mDisplayList = memalign(32, allocSize);

    // This invalidate is needed because the write-gather pipe does not use the cache.
    DCInvalidateRange(resource->mDisplayList, allocSize);

    GX_BeginDispList(resource->mDisplayList, allocSize);

    GX_Begin(GX_TRIANGLES, GX_VTXFMT0, staticMesh->GetNumIndices());

    if (hasColor)
    {
        for (uint32_t i = 0; i < staticMesh->GetNumFaces(); ++i)
        {
            GX_Position1x16(uint16_t(indices[i * 3 + 0]));
            GX_Normal1x16(uint16_t(indices[i * 3 + 0]));
            GX_Color1x16(uint16_t(indices[i * 3 + 0]));
            GX_TexCoord1x16(uint16_t(indices[i * 3 + 0]));
            GX_TexCoord1x16(uint16_t(indices[i * 3 + 0]));

            GX_Position1x16(uint16_t(indices[i * 3 + 1]));
            GX_Normal1x16(uint16_t(indices[i * 3 + 1]));
            GX_Color1x16(uint16_t(indices[i * 3 + 1]));
            GX_TexCoord1x16(uint16_t(indices[i * 3 + 1]));
            GX_TexCoord1x16(uint16_t(indices[i * 3 + 1]));

            GX_Position1x16(uint16_t(indices[i * 3 + 2]));
            GX_Normal1x16(uint16_t(indices[i * 3 + 2]));
            GX_Color1x16(uint16_t(indices[i * 3 + 2]));
            GX_TexCoord1x16(uint16_t(indices[i * 3 + 2]));
            GX_TexCoord1x16(uint16_t(indices[i * 3 + 2]));
        }
    }
    else
    {
        for (uint32_t i = 0; i < staticMesh->GetNumFaces(); ++i)
        {
            GX_Position1x16(uint16_t(indices[i * 3 + 0]));
            GX_Normal1x16(uint16_t(indices[i * 3 + 0]));
            GX_TexCoord1x16(uint16_t(indices[i * 3 + 0]));
            GX_TexCoord1x16(uint16_t(indices[i * 3 + 0]));

            GX_Position1x16(uint16_t(indices[i * 3 + 1]));
            GX_Normal1x16(uint16_t(indices[i * 3 + 1]));
            GX_TexCoord1x16(uint16_t(indices[i * 3 + 1]));
            GX_TexCoord1x16(uint16_t(indices[i * 3 + 1]));

            GX_Position1x16(uint16_t(indices[i * 3 + 2]));
            GX_Normal1x16(uint16_t(indices[i * 3 + 2]));
            GX_TexCoord1x16(uint16_t(indices[i * 3 + 2]));
            GX_TexCoord1x16(uint16_t(indices[i * 3 + 2]));
        }
    }

    GX_End();

    resource->mDisplayListSize = GX_EndDispList();
    OCT_ASSERT(resource->mDisplayListSize != 0);
}

void GFX_DestroyStaticMeshResource(StaticMesh* staticMesh)
{
    StaticMeshResource* resource = staticMesh->GetResource();

    if (resource->mDisplayList != nullptr)
    {
        free(resource->mDisplayList);
        resource->mDisplayList = nullptr;
        resource->mDisplayListSize = 0;
    }
}

// SkeletalMesh
void GFX_CreateSkeletalMeshResource(SkeletalMesh* skeletalMesh, uint32_t numVertices, VertexSkinned* vertices, uint32_t numIndices, IndexType* indices)
{

}

void GFX_DestroySkeletalMeshResource(SkeletalMesh* skeletalMesh)
{

}

// StaticMeshComp
void GFX_CreateStaticMeshCompResource(StaticMeshComponent* staticMeshComp)
{

}

void GFX_DestroyStaticMeshCompResource(StaticMeshComponent* staticMeshComp)
{

}

void GFX_DrawStaticMeshComp(StaticMeshComponent* staticMeshComp, StaticMesh* meshOverride)
{
    StaticMesh* mesh = meshOverride ? meshOverride : staticMeshComp->GetStaticMesh();

    if (mesh != nullptr)
    {
        BindStaticMesh(mesh);

        Material* material = staticMeshComp->GetMaterial();

        if (material == nullptr)
        {
            material = Renderer::Get()->GetDefaultMaterial();
            OCT_ASSERT(material != nullptr);
        }

        BindMaterial(material, mesh->HasVertexColor());

        Mtx model;
        Mtx view;
        Mtx modelView;

        glm::mat4 modelSrc = glm::transpose(staticMeshComp->GetRenderTransform());
        glm::mat4 viewSrc = glm::transpose(GetWorld()->GetActiveCamera()->GetViewMatrix());

        memcpy(model, &modelSrc, sizeof(float) * 4 * 3);
        memcpy(view, &viewSrc, sizeof(float) * 4 * 3);
        guMtxConcat(view, model, modelView);

        GX_LoadPosMtxImm(modelView, GX_PNMTX0);

        Mtx modelViewInv;
        guMtxInverse(modelView, modelViewInv);
        guMtxTranspose(modelViewInv, modelView);
        GX_LoadNrmMtxImm(modelView, GX_PNMTX0);

        SetupLightingChannels();

        GX_CallDispList(mesh->GetResource()->mDisplayList, mesh->GetResource()->mDisplayListSize);

        if (material->GetVertexColorMode() == VertexColorMode::TextureBlend)
        {
            // Reset swap tables used for texture blending.
            GX_SetTevSwapMode(0, GX_TEV_SWAP0, GX_TEV_SWAP0);
            GX_SetTevSwapMode(1, GX_TEV_SWAP0, GX_TEV_SWAP0);
            GX_SetTevSwapMode(2, GX_TEV_SWAP0, GX_TEV_SWAP0);
        }
    }
}

// SkeletalMeshComp
void GFX_CreateSkeletalMeshCompResource(SkeletalMeshComponent* skeletalMeshComp)
{

}

void GFX_DestroySkeletalMeshCompResource(SkeletalMeshComponent* skeletalMeshComp)
{

}

void GFX_ReallocateSkeletalMeshCompVertexBuffer(SkeletalMeshComponent* skeletalMeshComp, uint32_t numVertices)
{

}

void GFX_UpdateSkeletalMeshCompVertexBuffer(SkeletalMeshComponent* skeletalMeshComp, const std::vector<Vertex>& skinnedVertices)
{

}

void GFX_DrawSkeletalMeshComp(SkeletalMeshComponent* skeletalMeshComp)
{
    SkeletalMesh* mesh = skeletalMeshComp->GetSkeletalMesh();

    if (mesh != nullptr)
    {
        bool cpuSkinned = IsCpuSkinningRequired(skeletalMeshComp);
        if (cpuSkinned)
        {
            Vertex* vertices = skeletalMeshComp->GetSkinnedVertices();
            uint32_t numVertices = skeletalMeshComp->GetNumSkinnedVertices();

            GX_ClearVtxDesc();
            GX_SetVtxDesc(GX_VA_POS, GX_INDEX16);
            GX_SetVtxDesc(GX_VA_NRM, GX_INDEX16);
            GX_SetVtxDesc(GX_VA_TEX0, GX_INDEX16);
            GX_SetVtxDesc(GX_VA_TEX1, GX_INDEX16);

            GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
            GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
            GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
            GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX1, GX_TEX_ST, GX_F32, 0);

            GX_SetArray(GX_VA_POS, &vertices[0].mPosition, sizeof(Vertex));
            GX_SetArray(GX_VA_NRM, &vertices[0].mNormal, sizeof(Vertex));
            GX_SetArray(GX_VA_TEX0, &vertices[0].mTexcoord0, sizeof(Vertex));
            GX_SetArray(GX_VA_TEX1, &vertices[0].mTexcoord1, sizeof(Vertex));

            DCFlushRange(vertices, numVertices * sizeof(Vertex));
            GX_InvVtxCache();
        }
        else
        {
            BindSkeletalMesh(mesh);
        }

        Material* material = skeletalMeshComp->GetMaterial();

        if (material == nullptr)
        {
            material = Renderer::Get()->GetDefaultMaterial();
            OCT_ASSERT(material != nullptr);
        }

        BindMaterial(material, false);


        Mtx model;
        Mtx view;
        Mtx modelView;

        glm::mat4 modelSrc = glm::transpose(skeletalMeshComp->GetRenderTransform());
        glm::mat4 viewSrc = glm::transpose(GetWorld()->GetActiveCamera()->GetViewMatrix());

        memcpy(model, &modelSrc, sizeof(float) * 4 * 3);
        memcpy(view, &viewSrc, sizeof(float) * 4 * 3);
        guMtxConcat(view, model, modelView);

        Mtx modelViewInv;
        Mtx normalMtx;
        guMtxInverse(modelView, modelViewInv);
        guMtxTranspose(modelViewInv, normalMtx);

        if (cpuSkinned)
        {
            // Only need to set up a single position/normal matrix pair
            GX_LoadPosMtxImm(modelView, GX_PNMTX0);
            GX_LoadNrmMtxImm(normalMtx, GX_PNMTX0);
        }
        else
        {
            // Need to set up one position/normal matrix pair per bone.
            uint32_t numBones = skeletalMeshComp->GetNumBones();
            for (uint32_t i = 0; i < numBones; ++i)
            {
                Mtx fullTransform;
                Mtx bone;
                glm::mat4 boneSrc = glm::transpose(skeletalMeshComp->GetBoneTransform(i));
                memcpy(bone, &boneSrc, sizeof(float) * 4 * 3);
                guMtxConcat(modelView, bone, fullTransform);
                GX_LoadPosMtxImm(fullTransform, GX_PNMTX0 + i*3);

                Mtx normalTransform;
                guMtxConcat(normalMtx, bone, normalTransform);
                GX_LoadNrmMtxImm(normalTransform, GX_PNMTX0 + i*3);
            }
        }

        SetupLightingChannels();

        GX_Begin(GX_TRIANGLES, GX_VTXFMT0, mesh->GetNumIndices());

        const uint16_t* indices = mesh->GetIndices();

        if (cpuSkinned)
        {
            for (uint32_t i = 0; i < mesh->GetNumFaces(); ++i)
            {
                GX_Position1x16(indices[i * 3 + 0]);
                GX_Normal1x16(indices[i * 3 + 0]);
                GX_TexCoord1x16(indices[i * 3 + 0]);
                GX_TexCoord1x16(indices[i * 3 + 0]);

                GX_Position1x16(indices[i * 3 + 1]);
                GX_Normal1x16(indices[i * 3 + 1]);
                GX_TexCoord1x16(indices[i * 3 + 1]);
                GX_TexCoord1x16(indices[i * 3 + 1]);

                GX_Position1x16(indices[i * 3 + 2]);
                GX_Normal1x16(indices[i * 3 + 2]);
                GX_TexCoord1x16(indices[i * 3 + 2]);
                GX_TexCoord1x16(indices[i * 3 + 2]);
            }
        }
        else
        {
            const VertexSkinned* verts = mesh->GetVertices().data();

            for (uint32_t i = 0; i < mesh->GetNumFaces(); ++i)
            {
                uint8_t bone0 = 3 * verts[indices[i * 3 + 0]].mBoneIndices[0];
                uint8_t bone1 = 3 * verts[indices[i * 3 + 1]].mBoneIndices[0];
                uint8_t bone2 = 3 * verts[indices[i * 3 + 2]].mBoneIndices[0];

                GX_MatrixIndex1x8(bone0);
                GX_Position1x16(indices[i * 3 + 0]);
                GX_Normal1x16(indices[i * 3 + 0]);
                GX_TexCoord1x16(indices[i * 3 + 0]);
                GX_TexCoord1x16(indices[i * 3 + 0]);

                GX_MatrixIndex1x8(bone1);
                GX_Position1x16(indices[i * 3 + 1]);
                GX_Normal1x16(indices[i * 3 + 1]);
                GX_TexCoord1x16(indices[i * 3 + 1]);
                GX_TexCoord1x16(indices[i * 3 + 1]);

                GX_MatrixIndex1x8(bone2);
                GX_Position1x16(indices[i * 3 + 2]);
                GX_Normal1x16(indices[i * 3 + 2]);
                GX_TexCoord1x16(indices[i * 3 + 2]);
                GX_TexCoord1x16(indices[i * 3 + 2]);
            }
        }

        GX_End();
    }
}

bool GFX_IsCpuSkinningRequired(SkeletalMeshComponent* skeletalMeshComp)
{
    return IsCpuSkinningRequired(skeletalMeshComp);
}

// ShadowMeshComp
void GFX_DrawShadowMeshComp(ShadowMeshComponent* shadowMeshComp)
{
    StaticMesh* mesh = shadowMeshComp->GetStaticMesh();

    if (mesh != nullptr)
    {
        BindStaticMesh(mesh);

        // Shading
        gGxContext.mLighting.mEnabled = false;
        gGxContext.mLighting.mColorChannel = false;

        glm::vec4 shadowColor = GetWorld()->GetShadowColor();
        GX_SetChanMatColor(GX_COLOR0A0, { uint8_t(shadowColor.r * 255.0f),
                                            uint8_t(shadowColor.g * 255.0f),
                                            uint8_t(shadowColor.b * 255.0f),
                                            uint8_t(shadowColor.a * 255.f) });

        GX_SetNumTexGens(0);
        GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
        GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);

        GX_SetNumTevStages(1);

        gGxContext.mLighting.mMaterialSrc = GX_SRC_REG;

        // Matrices
        Mtx model;
        Mtx view;
        Mtx modelView;

        glm::mat4 modelSrc = glm::transpose(shadowMeshComp->GetRenderTransform());
        glm::mat4 viewSrc = glm::transpose(GetWorld()->GetActiveCamera()->GetViewMatrix());

        memcpy(model, &modelSrc, sizeof(float) * 4 * 3);
        memcpy(view, &viewSrc, sizeof(float) * 4 * 3);
        guMtxConcat(view, model, modelView);

        GX_LoadPosMtxImm(modelView, GX_PNMTX0);

        Mtx modelViewInv;
        guMtxInverse(modelView, modelViewInv);
        guMtxTranspose(modelViewInv, modelView);
        GX_LoadNrmMtxImm(modelView, GX_PNMTX0);

        SetupLightingChannels();

        GX_SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
        GX_SetZCompLoc(GX_TRUE);

        // (1) Back faces
        GX_SetZMode(GX_TRUE, GX_GREATER, GX_FALSE);
        GX_SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
        GX_SetColorUpdate(GX_FALSE);
        GX_SetAlphaUpdate(GX_TRUE);
        GX_SetCullMode(GX_CULL_BACK); // Reverse triangle winding
        GX_CallDispList(mesh->GetResource()->mDisplayList, mesh->GetResource()->mDisplayListSize);

        // (2) Front faces
        GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
        GX_SetBlendMode(GX_BM_BLEND, GX_BL_DSTALPHA, GX_BL_INVDSTALPHA, GX_LO_CLEAR);
        GX_SetColorUpdate(GX_TRUE);
        GX_SetAlphaUpdate(GX_FALSE);
        GX_SetCullMode(GX_CULL_FRONT); // Reverse triangle winding
        GX_CallDispList(mesh->GetResource()->mDisplayList, mesh->GetResource()->mDisplayListSize);

        // (3) Clear alpha channel
        GX_SetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
        GX_SetBlendMode(GX_BM_BLEND, GX_BL_ZERO, GX_BL_ZERO, GX_LO_CLEAR);
        GX_SetColorUpdate(GX_FALSE);
        GX_SetAlphaUpdate(GX_TRUE);
        GX_SetCullMode(GX_CULL_FRONT); // Reverse triangle winding
        GX_CallDispList(mesh->GetResource()->mDisplayList, mesh->GetResource()->mDisplayListSize);
    }
}

// TextMeshComp
void GFX_CreateTextMeshCompResource(TextMeshComponent* textMeshComp)
{

}

void GFX_DestroyTextMeshCompResource(TextMeshComponent* textMeshComp)
{

}

void GFX_UpdateTextMeshCompVertexBuffer(TextMeshComponent* textMeshComp, const std::vector<Vertex>& vertices)
{

}

void GFX_DrawTextMeshComp(TextMeshComponent* textMeshComp)
{
    if (textMeshComp->GetNumVisibleCharacters() == 0)
        return;

    const Vertex* vertices = textMeshComp->GetVertices();
    uint32_t numChars = (uint32_t)textMeshComp->GetNumVisibleCharacters();
    uint32_t numVertices = numChars * 6;

    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_NRM, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_TEX1, GX_DIRECT);

    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX1, GX_TEX_ST, GX_F32, 0);

    // TODO: Are both of these cache functions necessary to call?
    DCFlushRange((void*)vertices, numVertices * sizeof(Vertex));
    GX_InvVtxCache();

    Material* material = textMeshComp->GetMaterial();

    if (material == nullptr)
    {
        material = Renderer::Get()->GetDefaultMaterial();
        OCT_ASSERT(material != nullptr);
    }

    BindMaterial(material, false);

    Mtx model;
    Mtx view;
    Mtx modelView;

    glm::mat4 modelSrc = glm::transpose(textMeshComp->GetRenderTransform());
    glm::mat4 viewSrc = glm::transpose(GetWorld()->GetActiveCamera()->GetViewMatrix());

    memcpy(model, &modelSrc, sizeof(float) * 4 * 3);
    memcpy(view, &viewSrc, sizeof(float) * 4 * 3);
    guMtxConcat(view, model, modelView);

    GX_LoadPosMtxImm(modelView, GX_PNMTX0);

    Mtx modelViewInv;
    guMtxInverse(modelView, modelViewInv);
    guMtxTranspose(modelViewInv, modelView);
    GX_LoadNrmMtxImm(modelView, GX_PNMTX0);

    SetupLightingChannels();

    GX_Begin(GX_TRIANGLES, GX_VTXFMT0, numVertices);

    const Vertex* vert = nullptr;

    for (uint32_t i = 0; i < numVertices; i++)
    {
        vert = &vertices[i];
        GX_Position3f32(vert->mPosition.x, vert->mPosition.y, vert->mPosition.z);
        GX_Normal3f32(vert->mNormal.x, vert->mNormal.y, vert->mNormal.z);
        GX_TexCoord2f32(vert->mTexcoord0.x, vert->mTexcoord0.y);
        GX_TexCoord2f32(vert->mTexcoord1.x, vert->mTexcoord1.y);
    }

    GX_End();

    if (material->GetVertexColorMode() == VertexColorMode::TextureBlend)
    {
        // Reset swap tables used for texture blending.
        GX_SetTevSwapMode(0, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GX_SetTevSwapMode(1, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GX_SetTevSwapMode(2, GX_TEV_SWAP0, GX_TEV_SWAP0);
    }
}

// ParticleComp
void GFX_CreateParticleCompResource(ParticleComponent* particleComp)
{

}

void GFX_DestroyParticleCompResource(ParticleComponent* particleComp)
{

}

void GFX_UpdateParticleCompVertexBuffer(ParticleComponent* particleComp, const std::vector<VertexParticle>& vertices)
{

}

void GFX_DrawParticleComp(ParticleComponent* particleComp)
{
    if (particleComp->GetNumParticles() > 0)
    {
        const std::vector<VertexParticle>& vertices = particleComp->GetVertices();
        uint32_t numVertices = (uint32_t)vertices.size();

        GX_ClearVtxDesc();
        GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
        GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
        GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

        // TODO: Are both of these cache functions necessary to call?
        DCFlushRange((void*)vertices.data(), numVertices * sizeof(VertexParticle));
        GX_InvVtxCache();

        Material* material = particleComp->GetMaterial();

        if (material == nullptr)
        {
            material = Renderer::Get()->GetDefaultMaterial();
            OCT_ASSERT(material != nullptr);
        }

        BindMaterial(material, true);

        Mtx model;
        Mtx view;
        Mtx modelView;

        glm::mat4 modelSrc = particleComp->GetUseLocalSpace() ? glm::transpose(particleComp->GetTransform()) : glm::mat4(1);
        glm::mat4 viewSrc = glm::transpose(GetWorld()->GetActiveCamera()->GetViewMatrix());

        memcpy(model, &modelSrc, sizeof(float) * 4 * 3);
        memcpy(view, &viewSrc, sizeof(float) * 4 * 3);
        guMtxConcat(view, model, modelView);

        GX_LoadPosMtxImm(modelView, GX_PNMTX0);

        uint32_t numParticles = particleComp->GetNumParticles();
        OCT_ASSERT(numParticles * 4 == numVertices);

        SetupLightingChannels();

        GX_Begin(GX_TRIANGLES, GX_VTXFMT0, numParticles * 6);

        const VertexParticle* vert = nullptr;

        for (uint32_t i = 0; i < numParticles * 4; i += 4)
        {
            // A particle should have a constant color across all vertices.
            uint8_t* color = (uint8_t*)&(vertices[i + 0].mColor);

            vert = &vertices[i + 0];
            GX_Position3f32(vert->mPosition.x, vert->mPosition.y, vert->mPosition.z);
            GX_Color4u8(color[3], color[2], color[1], color[0]);
            GX_TexCoord2f32(vert->mTexcoord.x, vert->mTexcoord.y);

            vert = &vertices[i + 1];
            GX_Position3f32(vert->mPosition.x, vert->mPosition.y, vert->mPosition.z);
            GX_Color4u8(color[3], color[2], color[1], color[0]);
            GX_TexCoord2f32(vert->mTexcoord.x, vert->mTexcoord.y);

            vert = &vertices[i + 2];
            GX_Position3f32(vert->mPosition.x, vert->mPosition.y, vert->mPosition.z);
            GX_Color4u8(color[3], color[2], color[1], color[0]);
            GX_TexCoord2f32(vert->mTexcoord.x, vert->mTexcoord.y);

            vert = &vertices[i + 2];
            GX_Position3f32(vert->mPosition.x, vert->mPosition.y, vert->mPosition.z);
            GX_Color4u8(color[3], color[2], color[1], color[0]);
            GX_TexCoord2f32(vert->mTexcoord.x, vert->mTexcoord.y);

            vert = &vertices[i + 1];
            GX_Position3f32(vert->mPosition.x, vert->mPosition.y, vert->mPosition.z);
            GX_Color4u8(color[3], color[2], color[1], color[0]);
            GX_TexCoord2f32(vert->mTexcoord.x, vert->mTexcoord.y);

            vert = &vertices[i + 3];
            GX_Position3f32(vert->mPosition.x, vert->mPosition.y, vert->mPosition.z);
            GX_Color4u8(color[3], color[2], color[1], color[0]);
            GX_TexCoord2f32(vert->mTexcoord.x, vert->mTexcoord.y);
        }

        GX_End();
    }
}

// Quad
void GFX_CreateQuadResource(Quad* quad)
{

}

void GFX_DestroyQuadResource(Quad* quad)
{

}

void GFX_UpdateQuadResource(Quad* quad)
{

}

void GFX_DrawQuad(Quad* quad)
{
    // Setup render state / TEVs
    Renderer* renderer = Renderer::Get();
    Texture* texture = quad->GetTexture() ? quad->GetTexture() : renderer->mWhiteTexture.Get<Texture>();

    GX_SetNumTevStages(1);
    GX_SetNumTexGens(1);
    GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GX_LoadTexObj(&texture->GetResource()->mGxTexObj, GX_TEXMAP0);

    glm::vec4 tintColor = glm::clamp(quad->GetTint(), 0.0f, 1.0f);
    GX_SetChanMatColor(GX_COLOR0A0, { uint8_t(tintColor.r * 255.0f),
                                      uint8_t(tintColor.g * 255.0f),
                                      uint8_t(tintColor.b * 255.0f),
                                      uint8_t(tintColor.a * 255.0f) });

    GX_SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
    GX_SetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);

    // Draw verts
    //DCFlushRange(mVertices, 4 * sizeof(VertexUI));
    //GX_InvVtxCache();

    Mtx modelViewUI;
    guMtxIdentity(modelViewUI);
    GX_LoadPosMtxImm(modelViewUI, GX_PNMTX0);

    VertexUI* vertices = quad->GetVertices();

    gGxContext.mLighting.mEnabled = false;
    gGxContext.mLighting.mMaterialSrc = GX_SRC_VTX;
    SetupLightingChannels();

    GX_Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);

    for (uint32_t i = 0; i < 4; ++i)
    {
        GX_Position2f32(vertices[i].mPosition.x,
            vertices[i].mPosition.y);

        // Color needs to come before texcoord for some reason.
        uint8_t* color = (uint8_t*)&(vertices[i].mColor);
        GX_Color4u8(color[3], color[2], color[1], color[0]);

        GX_TexCoord2f32(vertices[i].mTexcoord.x,
            vertices[i].mTexcoord.y);
    }

    GX_End();
}

// Text
void GFX_CreateTextResource(Text* text)
{

}

void GFX_DestroyTextResource(Text* text)
{

}

void GFX_UpdateTextResourceUniformData(Text* text)
{

}

void GFX_UpdateTextResourceVertexData(Text* text)
{

}

void GFX_DrawText(Text* text)
{
    Renderer* renderer = Renderer::Get();

    Rect rect = text->GetRect();
    Font* font = text->GetFont();
    uint32_t numVisibleChars = text->GetNumVisibleCharacters();
    glm::vec2 justOff = text->GetJustifiedOffset();

    Mtx modelViewUI;
    glm::vec2 translation = glm::vec2(rect.mX + justOff.x, rect.mY + justOff.y);
    int32_t fontSize = font ? font->GetSize() : 32;
    float textScale = text->GetScaledTextSize() / fontSize;
    guMtxScale(modelViewUI, textScale, textScale, 1.0);
    guMtxTransApply(modelViewUI, modelViewUI, translation.x, translation.y, 0.0f);
    GX_LoadPosMtxImm(modelViewUI, GX_PNMTX0);

    // Setup render state / TEVs
    Texture* texture = renderer->mWhiteTexture.Get<Texture>();

    if (font != nullptr &&
        font->GetTexture() != nullptr)
    {
        texture = font->GetTexture();
    }

    GX_SetNumTevStages(1);
    GX_SetNumTexGens(1);
    GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);

    // Font's used to be RGB, but they are now stored RGBA so we don't need to swizzle.
    // Keeping the code here as a reference in case swizzling comes in handy in the future.
    //GX_SetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP1);
    //GX_SetTevSwapModeTable(GX_TEV_SWAP1, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_RED);

    GX_LoadTexObj(&texture->GetResource()->mGxTexObj, GX_TEXMAP0);

    GX_SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
    GX_SetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);

    // Draw verts
    //DCFlushRange(mVertices, 4 * sizeof(VertexUI));
    //GX_InvVtxCache();

    VertexUI* vertices = text->GetVertices();

    gGxContext.mLighting.mEnabled = false;
    gGxContext.mLighting.mMaterialSrc = GX_SRC_VTX;
    SetupLightingChannels();

    GX_Begin(GX_TRIANGLES, GX_VTXFMT0, 6 * numVisibleChars);

    for (uint32_t i = 0; i < 6 * numVisibleChars; ++i)
    {
        GX_Position2f32(vertices[i].mPosition.x,
            vertices[i].mPosition.y);

        // Color needs to come before texcoord for some reason.
        uint8_t* color = (uint8_t*)&(vertices[i].mColor);
        GX_Color4u8(color[3], color[2], color[1], color[0]);

        GX_TexCoord2f32(vertices[i].mTexcoord.x,
            vertices[i].mTexcoord.y);
    }

    GX_End();

    //GX_SetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
    //GX_SetTevSwapModeTable(GX_TEV_SWAP1, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA);
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

#endif