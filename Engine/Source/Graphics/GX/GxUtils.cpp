#if API_GX

#include "Graphics/GX/GxUtils.h"
#include "Graphics/GX/GxTypes.h"
#include "Graphics/GraphicsTypes.h"

#include "World.h"
#include "Engine.h"
#include "Renderer.h"
#include "Vertex.h"
#include "Assets/SkeletalMesh.h"

#include "Components/CameraComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SkeletalMeshComponent.h"

extern GxContext gGxContext;

void SetupLights()
{
    CameraComponent* cameraComp = GetWorld()->GetActiveCamera();
    if (cameraComp == nullptr)
    {
        return;
    }

    gGxContext.mLighting.mLightMask = 0;
    DirectionalLightComponent* dirLightComp = GetWorld()->GetDirectionalLight();

    // Setup directional light
    if (dirLightComp)
    {
        GXLightObj gxDirLight;

        glm::vec3 lightPosWS = -dirLightComp->GetDirection() * 10000.0f;
        glm::vec4 lightPosVS = cameraComp->GetViewMatrix() * glm::vec4(lightPosWS, 1.0f);

        glm::vec4 lightColor = dirLightComp->GetColor();
        lightColor = glm::clamp(lightColor, 0.0f, 1.0f);
        GXColor gxLightColor = { uint8_t(lightColor.r * 255.0f),
                                 uint8_t(lightColor.g * 255.0f),
                                 uint8_t(lightColor.b * 255.0f),
                                 uint8_t(lightColor.a * 255.0f) };

        GX_InitLightPos(&gxDirLight, lightPosVS.x, lightPosVS.y, lightPosVS.z);
        GX_InitLightColor(&gxDirLight, gxLightColor);
        GX_InitLightDistAttn(&gxDirLight, 0.0f, 1.0f, GX_DA_OFF);
        GX_InitLightSpot(&gxDirLight, 0.0f, GX_SP_OFF);
        GX_LoadLightObj(&gxDirLight, GX_LIGHT0);
        gGxContext.mLighting.mLightMask |= GX_LIGHT0;
    }

    glm::vec4 ambientColor = GetWorld()->GetAmbientLightColor();
    ambientColor = glm::clamp(ambientColor, 0.0f, 1.0f);
    GX_SetChanAmbColor(GX_COLOR0A0, { uint8_t(ambientColor.r * 255.0f),
                                      uint8_t(ambientColor.g * 255.0f),
                                      uint8_t(ambientColor.b * 255.0f),
                                      uint8_t(ambientColor.a * 255.0f) });

    GX_SetChanAmbColor(GX_COLOR1A1, { uint8_t(ambientColor.r * 255.0f),
                                      uint8_t(ambientColor.g * 255.0f),
                                      uint8_t(ambientColor.b * 255.0f),
                                      uint8_t(ambientColor.a * 255.0f) });

    // Setup point lights
    const std::vector<PointLightComponent*>& pointLights = GetWorld()->GetPointLights();

    for (uint32_t i = 0; i < pointLights.size(); ++i)
    {
        PointLightComponent* pointLightComp = pointLights[i];
        GXLightObj gxPointLight;

        if (pointLightComp)
        {
            glm::vec3 lightPosWS = pointLightComp->GetAbsolutePosition();
            glm::vec4 lightPosVS = cameraComp->GetViewMatrix() * glm::vec4(lightPosWS, 1.0f);

            glm::vec4 lightColor = pointLightComp->GetColor();
            lightColor = glm::clamp(lightColor, 0.0f, 1.0f);
            GXColor gxLightColor = { uint8_t(lightColor.r * 255.0f),
                                     uint8_t(lightColor.g * 255.0f),
                                     uint8_t(lightColor.b * 255.0f),
                                     uint8_t(lightColor.a * 255.0f) };

            GX_InitLightPos(&gxPointLight, lightPosVS.x, lightPosVS.y, lightPosVS.z);
            GX_InitLightColor(&gxPointLight, gxLightColor);
            GX_InitLightDistAttn(&gxPointLight, pointLightComp->GetRadius(), 0.25f, GX_DA_MEDIUM);
            GX_InitLightSpot(&gxPointLight, 0.0f, GX_SP_OFF);
            GX_LoadLightObj(&gxPointLight, GX_LIGHT1 << i);
            gGxContext.mLighting.mLightMask |= (GX_LIGHT1 << i);
        }
    }
}

void SetupLightingChannels()
{
    static LightingState prevState(false, false, 0, 0, 0, 0);
    LightingState& curState = gGxContext.mLighting;

    if (curState != prevState)
    {
        // If we are using vertex color modulation, then we use the first channel for that
        // since the vertex color can't go as input in the COLOR1A1 unless you have two vertex color attributes.
        GX_SetNumChans(curState.mColorChannel ? 2 : 1);
        GX_SetChanCtrl(
            curState.mColorChannel ? GX_COLOR1A1 : GX_COLOR0A0,
            curState.mEnabled,
            GX_SRC_REG,
            curState.mMaterialSrc,
            curState.mLightMask,
            curState.mDiffuseFunc,
            curState.mAttenuationFunc);

        if (curState.mColorChannel)
        {
            GX_SetChanCtrl(
                GX_COLOR0A0,
                false,
                GX_SRC_VTX,
                GX_SRC_VTX,
                0,
                GX_DF_NONE,
                GX_AF_NONE);
        }

        prevState = curState;
    }
}

void PrepareForwardRendering()
{
    GX_SetCullMode(GX_CULL_FRONT);
}

void PrepareUiRendering()
{
    GX_SetCullMode(GX_CULL_NONE);

    // Setup matrices
    glm::vec2 res = Renderer::Get()->GetScreenResolution();
    Mtx44 projectionUI;
    guOrtho(projectionUI, 0, res.y, 0, res.x, -100.0f, 100.0f);
    GX_LoadProjectionMtx(projectionUI, GX_ORTHOGRAPHIC);

    Mtx modelViewUI;
    guMtxIdentity(modelViewUI);
    GX_LoadPosMtxImm(modelViewUI, GX_PNMTX0);

    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

    // Probably not needed since lighting is disabled, but set material color to white.
    GX_SetChanMatColor(GX_COLOR0A0, { 255, 255, 255, 255 });

    gGxContext.mLighting.mColorChannel = false;
    gGxContext.mLighting.mEnabled = false;
    gGxContext.mLighting.mMaterialSrc = GX_SRC_VTX;
}

bool IsCpuSkinningRequired(SkeletalMeshComponent* component)
{
    if (component->GetSkeletalMesh() == nullptr)
    {
        return false;
    }
    else
    {
        return component->GetSkeletalMesh()->GetNumBones() > MAX_GPU_BONES;
    }
}

void BindMaterial(Material* material, bool useVertexColor)
{
    // If we are using vertex color modulation, then we need to use the first channel
    // since there is only one color vertex attribute.
    uint8_t matColorChannel = useVertexColor ? GX_COLOR1A1 : GX_COLOR0A0;

    ShadingModel shadingModel = material->GetShadingModel();
    BlendMode blendMode = material->GetBlendMode();
    glm::vec4 color = material->GetColor();
    float opacity = material->GetOpacity();
    bool depthless = material->IsDepthTestDisabled();

    // Setup TexCoord matrix
    glm::vec2 uvOffset = material->GetUvOffset();
    glm::vec2 uvScale = material->GetUvScale();
    Mtx texMatrix;
    guMtxScale(texMatrix, uvScale.x, uvScale.y, 1.0f);
    guMtxTransApply(texMatrix, texMatrix, uvOffset.x, uvOffset.y, 0.0f);
    GX_LoadTexMtxImm(texMatrix, GX_TEXMTX0, GX_TG_MTX3x4);

    uint32_t numTevStages = 1;

    // Shading
    if (shadingModel == ShadingModel::Unlit)
    {
        gGxContext.mLighting.mEnabled = false;

        // Setup color / texture channel stuff
        Texture* texture = material->GetTexture(TEXTURE_0);

        if (texture != nullptr)
        {
            GX_SetNumTexGens(1);
            GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_TEX0, GX_TEXMTX0);
            GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
            GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, matColorChannel);
            GX_LoadTexObj(&texture->GetResource()->mGxTexObj, GX_TEXMAP0);

            glm::vec4 materialColor = glm::clamp(color, 0.0f, 1.0f);
            GX_SetChanMatColor(matColorChannel, { uint8_t(materialColor.r * 255.0f),
                                              uint8_t(materialColor.g * 255.0f),
                                              uint8_t(materialColor.b * 255.0f),
                                              uint8_t(opacity * 255.f) });
        }
        else
        {
            GX_SetNumTexGens(0);
            GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, matColorChannel);
            GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
        }
    }
    else if (shadingModel == ShadingModel::Lit ||
            shadingModel == ShadingModel::Toon) // TODO: Support toon and fresnel
    {
        gGxContext.mLighting.mEnabled = true;

        Texture* texture = material->GetTexture(TEXTURE_0);

        glm::vec4 materialColor = color;
        materialColor = glm::clamp(materialColor, 0.0f, 1.0f);
        GX_SetChanMatColor(matColorChannel, { uint8_t(materialColor.r * 255.0f),
                                          uint8_t(materialColor.g * 255.0f),
                                          uint8_t(materialColor.b * 255.0f),
                                          uint8_t(opacity * 255.f) });

        GX_SetNumTexGens(1);
        GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_TEX0, GX_TEXMTX0);
        GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
        GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, matColorChannel);
        GX_LoadTexObj(&texture->GetResource()->mGxTexObj, GX_TEXMAP0);
    }

    // Vertex color modulation
    if (useVertexColor)
    {
        assert(numTevStages > 0 && numTevStages <= 15);
        GX_SetNumTevStages(numTevStages + 1);
        uint8_t vertTevStage = GX_TEVSTAGE0 + numTevStages;
        GX_SetTevOrder(vertTevStage, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);

        GX_SetTevColorIn(vertTevStage, GX_CC_ZERO, GX_CC_RASC, GX_CC_CPREV, GX_CC_ZERO);
        GX_SetTevAlphaIn(vertTevStage, GX_CA_ZERO, GX_CA_RASA, GX_CA_APREV, GX_CA_ZERO);

        GX_SetTevColorOp(vertTevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GX_SetTevAlphaOp(vertTevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);

        gGxContext.mLighting.mColorChannel = true;
    }
    else
    {
        GX_SetNumTevStages(numTevStages);
        gGxContext.mLighting.mColorChannel = false;
    }

    gGxContext.mLighting.mMaterialSrc = GX_SRC_REG;

    // We need the alpha channel for doing simple shadows.
    GX_SetColorUpdate(GX_TRUE);
    GX_SetAlphaUpdate(GX_FALSE);

    uint8_t depthEnabled = depthless ? GX_FALSE : GX_TRUE;

    // Blending
    if (blendMode == BlendMode::Opaque)
    {
        GX_SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
        GX_SetZMode(depthEnabled, GX_LEQUAL, depthEnabled);
        GX_SetZCompLoc(GX_TRUE);
        GX_SetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
    }
    else if (blendMode == BlendMode::Masked)
    {
        GX_SetAlphaCompare(GX_GREATER, int32_t(material->GetMaskCutoff() * 255.0f), GX_AOP_AND, GX_ALWAYS, 0);
        GX_SetZMode(depthEnabled, GX_LEQUAL, depthEnabled);
        GX_SetZCompLoc(GX_FALSE);
        GX_SetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
    }
    else if (blendMode == BlendMode::Translucent)
    {
        GX_SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
        GX_SetZMode(depthEnabled, GX_LEQUAL, GX_FALSE);
        GX_SetZCompLoc(GX_TRUE);
        GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    }
    else if (blendMode == BlendMode::Additive)
    {
        GX_SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
        GX_SetZMode(depthEnabled, GX_LEQUAL, GX_FALSE);
        GX_SetZCompLoc(GX_TRUE);
        GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_CLEAR);
    }
}

void BindStaticMesh(StaticMesh* staticMesh)
{
    uint8_t* vertBytes = staticMesh->HasVertexColor() ? (uint8_t*)staticMesh->GetColorVertices() : (uint8_t*)staticMesh->GetVertices();
    uint32_t numVertices = staticMesh->GetNumVertices();

    uint32_t posOffset = offsetof(Vertex, mPosition);
    uint32_t nrmOffset = offsetof(Vertex, mNormal);
    uint32_t clrOffset = 0;
    uint32_t texOffset = offsetof(Vertex, mTexcoord);

    if (staticMesh->HasVertexColor())
    {
        posOffset = offsetof(VertexColor, mPosition);
        nrmOffset = offsetof(VertexColor, mNormal);
        clrOffset = offsetof(VertexColor, mColor);
        texOffset = offsetof(VertexColor, mTexcoord);
    }

    // Set Vertex Format
    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_POS, GX_INDEX16);
    GX_SetVtxDesc(GX_VA_NRM, GX_INDEX16);
    if (staticMesh->HasVertexColor())
    {
        GX_SetVtxDesc(GX_VA_CLR0, GX_INDEX16);
    }
    GX_SetVtxDesc(GX_VA_TEX0, GX_INDEX16);

    // Set Attribute Formats
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
    if (staticMesh->HasVertexColor())
    {
        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    }
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

    // Set Array
    uint32_t vertexSize = staticMesh->GetVertexSize();
    GX_SetArray(GX_VA_POS, vertBytes + posOffset, vertexSize);
    GX_SetArray(GX_VA_NRM, vertBytes + nrmOffset, vertexSize);
    if (staticMesh->HasVertexColor())
    {
        GX_SetArray(GX_VA_CLR0, vertBytes + clrOffset, vertexSize);
    }
    GX_SetArray(GX_VA_TEX0, vertBytes + texOffset, vertexSize);

    // TODO: Are both of these cache functions necessary to call?
    DCFlushRange(vertBytes, numVertices * vertexSize);
    GX_InvVtxCache();
}

void BindSkeletalMesh(SkeletalMesh* skeletalMesh)
{
    uint8_t* vertBytes = (uint8_t*)skeletalMesh->GetVertices().data();
    uint32_t numVertices = skeletalMesh->GetNumVertices();

    //uint32_t mtxOffset = offsetof(VertexSkinned, mBoneIndices);
    uint32_t posOffset = offsetof(VertexSkinned, mPosition);
    uint32_t nrmOffset = offsetof(VertexSkinned, mNormal);
    uint32_t texOffset = offsetof(VertexSkinned, mTexcoord);

    // Set Vertex Format
    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_PTNMTXIDX, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_POS, GX_INDEX16);
    GX_SetVtxDesc(GX_VA_NRM, GX_INDEX16);
    GX_SetVtxDesc(GX_VA_TEX0, GX_INDEX16);

    // Set Attribute Formats
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_PTNMTXIDX, 0, GX_U8, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

    // Set Array
    uint32_t vertexSize = sizeof(VertexSkinned);
    //GX_SetArray(GX_VA_PTNMTXIDX, vertBytes + mtxOffset, vertexSize);
    GX_SetArray(GX_VA_POS, vertBytes + posOffset, vertexSize);
    GX_SetArray(GX_VA_NRM, vertBytes + nrmOffset, vertexSize);
    GX_SetArray(GX_VA_TEX0, vertBytes + texOffset, vertexSize);

    // TODO: Are both of these cache functions necessary to call?
    DCFlushRange(vertBytes, numVertices * vertexSize);
    GX_InvVtxCache();
}

#endif