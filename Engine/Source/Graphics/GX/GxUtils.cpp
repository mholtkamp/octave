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
    VertexColorMode vertexColorMode = material->GetVertexColorMode();
    glm::vec4 color = material->GetColor();
    float opacity = material->GetOpacity();
    bool depthless = material->IsDepthTestDisabled();

    // Setup TexCoord matrices
    glm::vec2 uvOffset0 = material->GetUvOffset(0);
    glm::vec2 uvScale0 = material->GetUvScale(0);
    Mtx texMatrix0;
    guMtxScale(texMatrix0, uvScale0.x, uvScale0.y, 1.0f);
    guMtxTransApply(texMatrix0, texMatrix0, uvOffset0.x, uvOffset0.y, 0.0f);
    GX_LoadTexMtxImm(texMatrix0, GX_TEXMTX0, GX_TG_MTX3x4);

    glm::vec2 uvOffset1 = material->GetUvOffset(1);
    glm::vec2 uvScale1 = material->GetUvScale(1);
    Mtx texMatrix1;
    guMtxScale(texMatrix1, uvScale1.x, uvScale1.y, 1.0f);
    guMtxTransApply(texMatrix1, texMatrix1, uvOffset1.x, uvOffset1.y, 0.0f);
    GX_LoadTexMtxImm(texMatrix1, GX_TEXMTX1, GX_TG_MTX3x4);

    uint32_t tevStage = 0;
    bool vertexColorBlend = (vertexColorMode == VertexColorMode::TextureBlend);

    uint32_t texIdx = 0;
    for (uint32_t i = 0; i < 4; ++i)
    {
        Texture* texture = material->GetTexture(TextureSlot(TEXTURE_0 + i));
        TevMode tevMode = (i == 0) ? TevMode::Replace : material->GetTevMode(i);
        uint32_t uvMap = material->GetUvMap(TextureSlot(TEXTURE_0 + i));

        if (i == 0 && texture == nullptr)
            texture = Renderer::Get()->mWhiteTexture.Get<Texture>();

        if (tevMode != TevMode::Pass &&
            texture != nullptr)
        {
            // First texture sample is done in TEV 0, if we have to add earlier stages,
            // then this assertion will need to be modified.
            assert(texIdx == tevStage);

            GX_SetTexCoordGen(GX_TEXCOORD0 + texIdx, GX_TG_MTX3x4, GX_TG_TEX0 + uvMap, GX_TEXMTX0 + uvMap);
            GX_LoadTexObj(&texture->GetResource()->mGxTexObj, GX_TEXMAP0 + texIdx);
            GX_SetTevOrder(GX_TEVSTAGE0 + texIdx, GX_TEXCOORD0 + texIdx, GX_TEXMAP0 + texIdx, matColorChannel);

            ConfigTev(texIdx, tevMode, vertexColorBlend);

            texIdx++;
            tevStage++;
        }
    }

    GX_SetNumTexGens(texIdx);

#if 0
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
#endif

    gGxContext.mLighting.mEnabled = !(shadingModel == ShadingModel::Unlit);

    glm::vec4 materialColor = color;
    materialColor = glm::clamp(materialColor, 0.0f, 1.0f);
    GX_SetChanMatColor(matColorChannel, { uint8_t(materialColor.r * 255.0f),
                                        uint8_t(materialColor.g * 255.0f),
                                        uint8_t(materialColor.b * 255.0f),
                                        uint8_t(opacity * 255.f) });

    GX_SetTevOrder(tevStage, GX_TEXCOORDNULL, GX_TEXMAP_NULL, matColorChannel);
    GX_SetTevColorIn(tevStage, GX_CC_ZERO, GX_CC_CPREV, GX_CC_RASC, GX_CC_ZERO);
    GX_SetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
    GX_SetTevAlphaIn(tevStage, GX_CA_ZERO, GX_CA_APREV, GX_CA_RASA, GX_CA_ZERO);
    GX_SetTevAlphaOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
    tevStage++;


    // Vertex color modulation
    if (useVertexColor)
    {
        GX_SetTevOrder(tevStage, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
        GX_SetTevColorIn(tevStage, GX_CC_ZERO, GX_CC_RASC, GX_CC_CPREV, GX_CC_ZERO);
        GX_SetTevAlphaIn(tevStage, GX_CA_ZERO, GX_CA_RASA, GX_CA_APREV, GX_CA_ZERO);
        GX_SetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GX_SetTevAlphaOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        tevStage++;

        gGxContext.mLighting.mColorChannel = true;
    }
    else
    {
        gGxContext.mLighting.mColorChannel = false;
    }

    assert(tevStage <= 16);
    GX_SetNumTevStages(tevStage);

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
    uint32_t texOffset0 = offsetof(Vertex, mTexcoord0);
    uint32_t texOffset1 = offsetof(Vertex, mTexcoord1);

    if (staticMesh->HasVertexColor())
    {
        posOffset = offsetof(VertexColor, mPosition);
        nrmOffset = offsetof(VertexColor, mNormal);
        clrOffset = offsetof(VertexColor, mColor);
        texOffset0 = offsetof(VertexColor, mTexcoord0);
        texOffset1 = offsetof(VertexColor, mTexcoord1);
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
    GX_SetVtxDesc(GX_VA_TEX1, GX_INDEX16);

    // Set Attribute Formats
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
    if (staticMesh->HasVertexColor())
    {
        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    }
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX1, GX_TEX_ST, GX_F32, 0);

    // Set Array
    uint32_t vertexSize = staticMesh->GetVertexSize();
    GX_SetArray(GX_VA_POS, vertBytes + posOffset, vertexSize);
    GX_SetArray(GX_VA_NRM, vertBytes + nrmOffset, vertexSize);
    if (staticMesh->HasVertexColor())
    {
        GX_SetArray(GX_VA_CLR0, vertBytes + clrOffset, vertexSize);
    }
    GX_SetArray(GX_VA_TEX0, vertBytes + texOffset0, vertexSize);
    GX_SetArray(GX_VA_TEX1, vertBytes + texOffset1, vertexSize);

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
    uint32_t texOffset0 = offsetof(VertexSkinned, mTexcoord0);
    uint32_t texOffset1 = offsetof(VertexSkinned, mTexcoord1);

    // Set Vertex Format
    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_PTNMTXIDX, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_POS, GX_INDEX16);
    GX_SetVtxDesc(GX_VA_NRM, GX_INDEX16);
    GX_SetVtxDesc(GX_VA_TEX0, GX_INDEX16);
    GX_SetVtxDesc(GX_VA_TEX1, GX_INDEX16);

    // Set Attribute Formats
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_PTNMTXIDX, 0, GX_U8, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX1, GX_TEX_ST, GX_F32, 0);

    // Set Array
    uint32_t vertexSize = sizeof(VertexSkinned);
    //GX_SetArray(GX_VA_PTNMTXIDX, vertBytes + mtxOffset, vertexSize);
    GX_SetArray(GX_VA_POS, vertBytes + posOffset, vertexSize);
    GX_SetArray(GX_VA_NRM, vertBytes + nrmOffset, vertexSize);
    GX_SetArray(GX_VA_TEX0, vertBytes + texOffset0, vertexSize);
    GX_SetArray(GX_VA_TEX1, vertBytes + texOffset1, vertexSize);

    // TODO: Are both of these cache functions necessary to call?
    DCFlushRange(vertBytes, numVertices * vertexSize);
    GX_InvVtxCache();
}

void ConfigTev(uint32_t textureSlot, TevMode mode, bool vertexColorBlend)
{
    uint8_t tevStage = (uint8_t) (GX_TEVSTAGE0 + textureSlot);

    switch (textureSlot)
    {
        case 0: /*blendColorSrc = GPU_TEVOP_RGB_SRC_R;*/ break;
        case 1: /*blendColorSrc = GPU_TEVOP_RGB_SRC_G;*/ break;
        case 2: /*blendColorSrc = GPU_TEVOP_RGB_SRC_B;*/ break;
    }

    if (textureSlot == 0)
    {
        mode = TevMode::Replace;
    }

    if (false /*vertexColorBlend*/)
    {

    }
    else
    {
        switch (mode)
        {
            case TevMode::Modulate:
                GX_SetTevColorIn(tevStage, GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_ZERO);
                GX_SetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
                GX_SetTevAlphaIn(tevStage, GX_CA_ZERO, GX_CA_TEXA, GX_CA_APREV, GX_CA_ZERO);
                GX_SetTevAlphaOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
            break;

            case TevMode::Decal:
                GX_SetTevColorIn(tevStage, GX_CC_CPREV, GX_CC_TEXC, GX_CC_TEXA, GX_CC_ZERO);
                GX_SetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
                GX_SetTevAlphaIn(tevStage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
                GX_SetTevAlphaOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
            break;

            case TevMode::Add:
                GX_SetTevColorIn(tevStage, GX_CC_ZERO, GX_CC_CPREV, GX_CC_ONE, GX_CC_TEXC);
                GX_SetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
                GX_SetTevAlphaIn(tevStage, GX_CA_TEXA, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
                GX_SetTevAlphaOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
            break;

            case TevMode::SignedAdd:
                GX_SetTevColorIn(tevStage, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_CPREV);
                GX_SetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_SUBHALF, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
                GX_SetTevAlphaIn(tevStage, GX_CA_TEXA, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
                GX_SetTevAlphaOp(tevStage, GX_TEV_ADD, GX_TB_SUBHALF, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
            break;
            
            case TevMode::Subtract:
                GX_SetTevColorIn(tevStage, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_CPREV);
                GX_SetTevColorOp(tevStage, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
                GX_SetTevAlphaIn(tevStage, GX_CA_TEXA, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
                GX_SetTevAlphaOp(tevStage, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
            break;

            case TevMode::Replace:
            default:
                GX_SetTevColorIn(tevStage, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
                GX_SetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
                GX_SetTevAlphaIn(tevStage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
                GX_SetTevAlphaOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
            break;
        }
    }
}

#endif