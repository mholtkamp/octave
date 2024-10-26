#if API_GX

#include "Graphics/GX/GxUtils.h"
#include "Graphics/GX/GxTypes.h"
#include "Graphics/GraphicsTypes.h"

#include "World.h"
#include "Engine.h"
#include "Renderer.h"
#include "Vertex.h"
#include "Nodes/Widgets/Widget.h"
#include "Assets/SkeletalMesh.h"

#include "Nodes/3D/Camera3d.h"
#include "Nodes/3D/DirectionalLight3d.h"
#include "Nodes/3D/PointLight3d.h"
#include "Nodes/3D/SkeletalMesh3d.h"

#include <malloc.h>

extern GxContext gGxContext;

void SetupLights()
{
    Camera3D* cameraComp = gGxContext.mWorld->GetActiveCamera();
    if (cameraComp == nullptr)
    {
        return;
    }

    gGxContext.mLighting.mLightMask = 0;

    // Setup lights
    const std::vector<LightData>& lightArray = Renderer::Get()->GetLightData();

    gGxContext.mSceneLightMask = 0;
    gGxContext.mSceneNumLights = 0;

    // Light0 is reserved for directional light, in the future we might allow multiple dir lights.
    for (uint32_t i = 0; i < lightArray.size() && i < MAX_LIGHTS_PER_DRAW; ++i)
    {
        const LightData& lightData = lightArray[i];
        GXLightObj gxLight;

        glm::vec3 lightPosWS = lightData.mPosition;
        if (lightData.mType == LightType::Directional)
        {
            lightPosWS = cameraComp->GetWorldPosition() + -lightData.mDirection * 10000.0f;
        }
        glm::vec4 lightPosVS = cameraComp->GetViewMatrix() * glm::vec4(lightPosWS, 1.0f);

        glm::vec4 lightColor = lightData.mColor / GX_DYNAMIC_LIGHT_SCALE;
        lightColor *= lightData.mIntensity;
        lightColor.a = 1.0f;
        lightColor = glm::clamp(lightColor, 0.0f, 1.0f);
        GXColor gxLightColor = { uint8_t(lightColor.r * 255.0f),
                                    uint8_t(lightColor.g * 255.0f),
                                    uint8_t(lightColor.b * 255.0f),
                                    uint8_t(lightColor.a * 255.0f) };

        GX_InitLightPos(&gxLight, lightPosVS.x, lightPosVS.y, lightPosVS.z);
        GX_InitLightColor(&gxLight, gxLightColor);
        GX_InitLightSpot(&gxLight, 0.0f, GX_SP_OFF);

        if (lightData.mType == LightType::Directional)
        {
            GX_InitLightDistAttn(&gxLight, 0.0f, 1.0f, GX_DA_OFF);
        }
        else
        {
            GX_InitLightDistAttn(&gxLight, lightData.mRadius, 0.25f, GX_DA_MEDIUM);
        }

        GX_LoadLightObj(&gxLight, GX_LIGHT0 << gGxContext.mSceneNumLights);
        gGxContext.mSceneLightMask |= (GX_LIGHT0 << gGxContext.mSceneNumLights);
        gGxContext.mLightData[gGxContext.mSceneNumLights] = lightData;
        gGxContext.mSceneNumLights++;
    }
}

void SetupLightMask(ShadingModel shadingModel, uint8_t lightingChannels, bool useBakedLight)
{
    uint8_t lightMask = 0;

    if (shadingModel != ShadingModel::Unlit)
    {
        // This step needs to determine which light types should affect the primitive being drawn.
        // For instance, an All domain light should not affect a static mesh that has baked lighting.
        for (uint32_t i = 0; i < gGxContext.mSceneNumLights; ++i)
        {
            const LightData& lightData = gGxContext.mLightData[i];

            if ((lightData.mDomain == LightingDomain::All) && useBakedLight)
            {
                continue;
            }

            if ((lightData.mLightingChannels & lightingChannels) == 0)
            {
                continue;
            }

            // In the future, use distance checking to determine which lights should affect a draw.
            // This is slightly tough right now because of the weird attenuation functions.
            // There isn't a point where the light intensity reaches 0 exactly.

            lightMask |= (GX_LIGHT0 << i);
        }
    }

    gGxContext.mLighting.mLightMask = lightMask;
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
        else
        {
            // I was crashing without "zeroing" the second channel even
            // when it wasn't in use.
            GX_SetChanCtrl(
                GX_COLOR1A1,
                false,
                GX_SRC_VTX,
                GX_SRC_VTX,
                0,
                GX_DF_NONE,
                GX_AF_NONE);
        }

        GX_SetChanCtrl(
            curState.mColorChannel ? GX_COLOR1A1 : GX_COLOR0A0,
            curState.mEnabled,
            GX_SRC_REG,
            curState.mMaterialSrc,
            curState.mLightMask,
            curState.mDiffuseFunc,
            curState.mAttenuationFunc);

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
    GX_SetColorUpdate(GX_TRUE);
    GX_SetAlphaUpdate(GX_FALSE);

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

bool IsCpuSkinningRequired(SkeletalMesh3D* component)
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

void BindMaterial(MaterialLite* material, bool useVertexColor, bool useBakedLighting)
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
    guMtxTrans(texMatrix0, uvOffset0.x, uvOffset0.y, 0.0f);
    guMtxScaleApply(texMatrix0, texMatrix0, uvScale0.x, uvScale0.y, 1.0f);
    GX_LoadTexMtxImm(texMatrix0, GX_TEXMTX0, GX_TG_MTX3x4);

    glm::vec2 uvOffset1 = material->GetUvOffset(1);
    glm::vec2 uvScale1 = material->GetUvScale(1);
    Mtx texMatrix1;
    guMtxTrans(texMatrix1, uvOffset1.x, uvOffset1.y, 0.0f);
    guMtxScaleApply(texMatrix1, texMatrix1, uvScale1.x, uvScale1.y, 1.0f);
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
            OCT_ASSERT(texIdx == tevStage);

            GX_SetTexCoordGen(GX_TEXCOORD0 + texIdx, GX_TG_MTX3x4, GX_TG_TEX0 + uvMap, GX_TEXMTX0 + uvMap * 3);
            GX_LoadTexObj(&texture->GetResource()->mGxTexObj, GX_TEXMAP0 + texIdx);
            GX_SetTevOrder(GX_TEVSTAGE0 + texIdx, GX_TEXCOORD0 + texIdx, GX_TEXMAP0 + texIdx, GX_COLOR0A0);

            ConfigTev(texIdx, tevMode, vertexColorBlend);

            texIdx++;
            tevStage++;
        }
    }

    GX_SetNumTexGens(texIdx);

    bool unlit = (shadingModel == ShadingModel::Unlit);
    gGxContext.mLighting.mEnabled = !unlit;

    if (useBakedLighting)
    {
        // Compute the baked color first and save it out to a scratch register. 
        // After resolving the final (dynamically lit) color, add this baked color to it.
        bool fullBake = (vertexColorMode != VertexColorMode::TextureBlend);

        GX_SetTevOrder(tevStage, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
        GX_SetTevColorIn(tevStage, GX_CC_ZERO, fullBake ? GX_CC_RASC : GX_CC_RASA, GX_CC_CPREV, GX_CC_ZERO);
        GX_SetTevAlphaIn(tevStage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
        GX_SetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
        GX_SetTevAlphaOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVREG0);
        tevStage++;
    }

    glm::vec4 materialColor = color;
    materialColor = glm::clamp(materialColor, 0.0f, 1.0f);
    GX_SetChanMatColor(matColorChannel, { uint8_t(materialColor.r * 255.0f),
                                        uint8_t(materialColor.g * 255.0f),
                                        uint8_t(materialColor.b * 255.0f),
                                        uint8_t(opacity * 255.f) });

    glm::vec4 ambientColor = useBakedLighting ? glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) : gGxContext.mWorld->GetAmbientLightColor();
    ambientColor = glm::clamp(ambientColor / GX_DYNAMIC_LIGHT_SCALE, 0.0f, 1.0f);
    ambientColor.a = 1.0f;
    GX_SetChanAmbColor(matColorChannel, { uint8_t(ambientColor.r * 255.0f),
                                      uint8_t(ambientColor.g * 255.0f),
                                      uint8_t(ambientColor.b * 255.0f),
                                      uint8_t(ambientColor.a * 255.0f) });

    GX_SetTevOrder(tevStage, GX_TEXCOORDNULL, GX_TEXMAP_NULL, matColorChannel);
    GX_SetTevColorIn(tevStage, GX_CC_ZERO, GX_CC_CPREV, GX_CC_RASC, GX_CC_ZERO);
    GX_SetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GX_SetTevAlphaIn(tevStage, GX_CA_ZERO, GX_CA_APREV, GX_CA_RASA, GX_CA_ZERO);
    GX_SetTevAlphaOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    tevStage++;

    // Vertex color modulation
    if (useVertexColor)
    {
        if (useBakedLighting)
        {
            // Add in the previously computed baked color to the dynamically lit result.
            GX_SetTevOrder(tevStage, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
            GX_SetTevColorIn(tevStage, GX_CC_CPREV, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
            GX_SetTevAlphaIn(tevStage, GX_CA_APREV, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
            GX_SetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
            GX_SetTevAlphaOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
            tevStage++;
        }
        else if (vertexColorMode == VertexColorMode::Modulate)
        {
            GX_SetTevOrder(tevStage, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
            GX_SetTevColorIn(tevStage, GX_CC_ZERO, GX_CC_RASC, GX_CC_CPREV, GX_CC_ZERO);
            GX_SetTevAlphaIn(tevStage, GX_CA_ZERO, GX_CA_RASA, GX_CA_APREV, GX_CA_ZERO);
            GX_SetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
            GX_SetTevAlphaOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
            tevStage++;
        }

        gGxContext.mLighting.mColorChannel = true;
    }
    else
    {
        gGxContext.mLighting.mColorChannel = false;
    }

    OCT_ASSERT(tevStage <= 16);
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

    bool matApplyFog = material->ShouldApplyFog();
    if (gGxContext.mFogType != GX_FOG_NONE && 
        gGxContext.mApplyFog != matApplyFog)
    {
        gGxContext.mApplyFog = matApplyFog;

        // Need to enable/disable fog because of material property
        if (material->ShouldApplyFog())
        {
            GX_SetFog(
                gGxContext.mFogType,
                gGxContext.mFogStartZ,
                gGxContext.mFogEndZ,
                gGxContext.mFogNearZ,
                gGxContext.mFogFarZ,
                gGxContext.mFogColor);
        }
        else
        {
            GX_SetFog(
                GX_FOG_NONE,
                0.0f,
                0.0f,
                0.0f,
                0.0f,
                {0,0,0,0});
        }
    }

    CullMode cullMode = material->GetCullMode();
    switch (cullMode)
    {
        // Note: Culling is reversed on GX for some reason.
        case CullMode::None: GX_SetCullMode(GX_CULL_NONE); break;
        case CullMode::Back: GX_SetCullMode(GX_CULL_FRONT); break;
        case CullMode::Front: GX_SetCullMode(GX_CULL_BACK); break;
    }
}

void BindStaticMesh(StaticMesh* staticMesh, uint32_t* instanceColors)
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

    bool hasColor = (staticMesh->HasVertexColor() || instanceColors != nullptr);

    // Set Vertex Format
    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_POS, GX_INDEX16);
    GX_SetVtxDesc(GX_VA_NRM, GX_INDEX16);
    if (hasColor)
    {
        GX_SetVtxDesc(GX_VA_CLR0, GX_INDEX16);
    }
    GX_SetVtxDesc(GX_VA_TEX0, GX_INDEX16);
    GX_SetVtxDesc(GX_VA_TEX1, GX_INDEX16);

    // Set Attribute Formats
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
    if (hasColor)
    {
        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    }
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX1, GX_TEX_ST, GX_F32, 0);

    // Set Array
    uint32_t vertexSize = staticMesh->GetVertexSize();
    GX_SetArray(GX_VA_POS, vertBytes + posOffset, vertexSize);
    GX_SetArray(GX_VA_NRM, vertBytes + nrmOffset, vertexSize);
    if (hasColor)
    {
        if (instanceColors != nullptr)
        {
            GX_SetArray(GX_VA_CLR0, instanceColors, sizeof(uint32_t));
        }
        else
        {
            GX_SetArray(GX_VA_CLR0, vertBytes + clrOffset, vertexSize);
        }
    }
    GX_SetArray(GX_VA_TEX0, vertBytes + texOffset0, vertexSize);
    GX_SetArray(GX_VA_TEX1, vertBytes + texOffset1, vertexSize);

    // TODO: Are both of these cache functions necessary to call?
    DCFlushRange(vertBytes, numVertices * vertexSize);

    if (instanceColors != nullptr)
    {
        DCFlushRange(instanceColors, numVertices * sizeof(uint32_t));
    }

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
    uint8_t blendChannel = GX_CH_RED;

    switch (textureSlot)
    {
        case 0: blendChannel = GX_CH_RED; break;
        case 1: blendChannel = GX_CH_GREEN; break;
        case 2: blendChannel = GX_CH_BLUE; break;
    }

    if (textureSlot == 0)
    {
        mode = TevMode::Replace;
    }

    if (vertexColorBlend)
    {
        GX_SetTevSwapMode(tevStage, GX_TEV_SWAP1 + textureSlot, GX_TEV_SWAP0);
        GX_SetTevSwapModeTable(GX_TEV_SWAP1 + textureSlot, blendChannel, blendChannel, blendChannel, GX_CH_ALPHA);

        uint8_t prevColor = textureSlot == 0 ? GX_CC_ZERO : GX_CC_CPREV;

        GX_SetTevColorIn(tevStage, prevColor, GX_CC_TEXC, GX_CC_RASC, GX_CC_ZERO);
        GX_SetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GX_SetTevAlphaIn(tevStage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
        GX_SetTevAlphaOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    }
    else
    {
        GX_SetTevSwapMode(tevStage, GX_TEV_SWAP0, GX_TEV_SWAP0);

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

void ApplyWidgetRotation(Mtx& mtx, Widget* widget)
{
    // Ok the position / rotation / scaling of widgets is in a weird place now.
    // I think everything could be calculated and stored in the widget transform.
    // I think the reason this wasn't the case was because I wanted to batch quad draws together
    // so I needed to have all of there vertices precomputed. But I'm not sure if that will ever be 
    // a possibility, so perhaps just fix the widget transform so that it stores the entire true transform
    // and just let Quads use a static vertex buffer.

    Mtx rotMat;
    guMtxIdentity(rotMat);

    const glm::mat3& trans3 = widget->GetTransform();
    rotMat[0][0] = trans3[0][0];
    rotMat[0][1] = trans3[1][0];
    rotMat[1][0] = trans3[0][1];
    rotMat[1][1] = trans3[1][1];
    rotMat[0][3] = trans3[2][0];
    rotMat[1][3] = trans3[2][1];

    Mtx srcMat;
    memcpy(srcMat, &mtx, sizeof(float) * 4 * 3);

    guMtxConcat(rotMat, srcMat, mtx);
}

void* CreateMeshDisplayList(StaticMesh* staticMesh, bool useColor, uint32_t& outSize)
{
    void* displayList = nullptr;

    IndexType* indices = staticMesh->GetIndices();

    // Generate a display list
    uint32_t gxBeginSize = 3;
    uint32_t elemSize = useColor ? (2 + 2 + 2 + 2 + 2) : (2 + 2 + 2 + 2);
    uint32_t allocSize = gxBeginSize + (elemSize * staticMesh->GetNumFaces() * 3);
    allocSize = (allocSize + 0x1f) & (~0x1f); // 32 byte aligned
    allocSize += 64; // Extra space to account for pipe flush
    displayList = memalign(32, allocSize);

    // This invalidate is needed because the write-gather pipe does not use the cache.
    DCInvalidateRange(displayList, allocSize);

    GX_BeginDispList(displayList, allocSize);

    GX_Begin(GX_TRIANGLES, GX_VTXFMT0, staticMesh->GetNumIndices());

    if (useColor)
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

    outSize = GX_EndDispList();
    OCT_ASSERT(outSize != 0);

    return displayList;
}

void DestroyMeshDisplayList(void* displayList)
{
    if (displayList != nullptr)
    {
        free(displayList);
    }
}

#endif