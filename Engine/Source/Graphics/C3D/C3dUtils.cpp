#if API_C3D

#include "Graphics/C3D/C3dUtils.h"

#include "Engine.h"
#include "World.h"
#include "Renderer.h"
#include "Log.h"
#include "Assets/Texture.h"

#include "Components/PointLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/CameraComponent.h"

#include <glm/gtc/matrix_transform.hpp>

static inline float LinearAttenFunc(float dist, float maxDist, float none)
{
    return (1.0f - (dist / maxDist));
}

uint32_t CalculateShininessLevel(float shininess)
{
    shininess = glm::max<uint32_t>(shininess, 2);
    float shinyLog = log2(shininess);
    uint32_t shinyLevel = uint32_t((shinyLog - 1.0f) + 0.5f);
    shinyLevel = glm::clamp<uint32_t>(shinyLevel, 0u, NumShininessLevels - 1u);
    assert(shinyLevel < NumShininessLevels);
    return shinyLevel;
}

uint32_t CalculateFresnelPowerLevel(float fresnelPower)
{
    float fresnelLevels[NumFresnelPowerLevels] =
    {
        1.0f, 1.5f, 2.0f, 4.0f
    };

    int32_t closest = -1;
    float closestDist = FLT_MAX;
    for (int32_t i = 0; i < NumFresnelPowerLevels; ++i)
    {
        float dist = fabs(fresnelLevels[i] - fresnelPower);
        if (dist < closestDist)
        {
            closest = i;
            closestDist = dist;
        }
    }

    return closest;
}

void CopyMatrixGlmToC3d(C3D_Mtx* dst, const glm::mat4& src)
{
    glm::mat4 transMat = glm::transpose(src);

    dst->r[0].x = transMat[0][0];
    dst->r[0].y = transMat[0][1];
    dst->r[0].z = transMat[0][2];
    dst->r[0].w = transMat[0][3];

    dst->r[1].x = transMat[1][0];
    dst->r[1].y = transMat[1][1];
    dst->r[1].z = transMat[1][2];
    dst->r[1].w = transMat[1][3];

    dst->r[2].x = transMat[2][0];
    dst->r[2].y = transMat[2][1];
    dst->r[2].z = transMat[2][2];
    dst->r[2].w = transMat[2][3];

    dst->r[3].x = transMat[3][0];
    dst->r[3].y = transMat[3][1];
    dst->r[3].z = transMat[3][2];
    dst->r[3].w = transMat[3][3];
}

void BindVertexShader(ShaderId shaderId)
{
    if (shaderId != gC3dContext.mLastBoundShaderId)
    {
        C3D_BindProgram(&gC3dContext.mShaderPrograms[uint32_t(shaderId)]);
        gC3dContext.mLastBoundShaderId = shaderId;
    }
}

void BindStaticMesh(StaticMesh* mesh)
{
    // Bind shader program
    BindVertexShader(ShaderId::StaticMesh);
    bool hasColor = mesh->HasVertexColor();

    // Setup vertex attributes
    C3D_AttrInfo* attrInfo = C3D_GetAttrInfo();
    AttrInfo_Init(attrInfo);
    AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // v0=position
    AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 2); // v1=texcoord0
    AttrInfo_AddLoader(attrInfo, 2, GPU_FLOAT, 2); // v2=texcoord1
    AttrInfo_AddLoader(attrInfo, 3, GPU_FLOAT, 3); // v3=normal
    if (hasColor)
    {
        AttrInfo_AddLoader(attrInfo, 4, GPU_UNSIGNED_BYTE, 4); // v4=color
    }
    else
    {
        AttrInfo_AddFixed(attrInfo, 4); // v4=color
        C3D_FixedAttribSet(4, 255.0f, 255.0f, 255.0f, 255.0f);
    }

    // Setup vertex buffer state
    C3D_BufInfo* bufInfo = C3D_GetBufInfo();
    BufInfo_Init(bufInfo);
    if (hasColor)
    {
        BufInfo_Add(bufInfo, mesh->GetResource()->mVertexData, sizeof(VertexColor), 5, 0x43210);
    }
    else
    {
        BufInfo_Add(bufInfo, mesh->GetResource()->mVertexData, sizeof(Vertex), 4, 0x3210);
    }
}

void BindMaterial(Material* material)
{
    if (material != gC3dContext.mLastBoundMaterial)
    {
        ResetTexEnv();
        ResetLightingEnv();

        ShadingModel shadingModel = material->GetShadingModel();
        BlendMode blendMode = material->GetBlendMode();
        VertexColorMode vertexColorMode = material->GetVertexColorMode();
        glm::vec4 color = material->GetColor();
        float opacity = material->GetOpacity();
        bool depthless = material->IsDepthTestDisabled();

        glm::vec4 materialColor = glm::clamp(color, 0.0f, 1.0f);
        uint8_t matColor4[4] =
        {
            uint8_t(materialColor.r * 255.0f),
            uint8_t(materialColor.g * 255.0f),
            uint8_t(materialColor.b * 255.0f),
            uint8_t(opacity * 255.0f)
        };

        float specular = 0.0f;

        if (shadingModel == ShadingModel::Lit ||
            shadingModel == ShadingModel::Toon)
        {
            glm::vec4 ambientColor = GetWorld()->GetAmbientLightColor();
            bool toon = (shadingModel == ShadingModel::Toon);
            specular = toon ? 0.0f : material->GetSpecular();
            float specular1 = toon ? 1.0f : 0.0f;
            // Material color gets modulated in the TexEnv.
            C3D_Material c3dMaterial =
            {
                { ambientColor.b, ambientColor.g, ambientColor.r }, //ambient
                { 1.0f, 1.0f, 1.0f }, //diffuse
                { specular, specular, specular }, //specular0
                { specular1, specular1, specular1 }, //specular1
                { 0.0f, 0.0f, 0.0f }, //emission
            };

            C3D_LightEnvBind(&gC3dContext.mLightEnv);
            C3D_LightEnvMaterial(&gC3dContext.mLightEnv, &c3dMaterial);
        }

        if (shadingModel == ShadingModel::Toon)
        {
            uint32_t numSteps = material->GetToonSteps();
            uint32_t toonLevel = numSteps == 3 ? ToonLevel3 : ToonLevel2;
            C3D_LightEnvLut(&gC3dContext.mLightEnv, GPU_LUT_D1, GPU_LUTINPUT_LN, false, &gC3dContext.mToonLut[toonLevel]);
            specular = 0.0f;
        }

        if (specular > 0.0f)
        {
            // If we have a specular component, we need to choose the light lut that best matches its shininess.
            // Otherwise, just leave the light lut as it was since I think it's only used for specular light.
            uint32_t shininessLevel = CalculateShininessLevel(material->GetShininess());
            C3D_LightEnvLut(&gC3dContext.mLightEnv, GPU_LUT_D0, GPU_LUTINPUT_NH, false, &gC3dContext.mLightLut[shininessLevel]);
        }

        bool fresnelEnabled = material->IsFresnelEnabled();
        uint8_t fresnelColor[4] = {};

        if (fresnelEnabled)
        {
            uint32_t fresnelPowerLevel = CalculateFresnelPowerLevel(material->GetFresnelPower());
            C3D_LightEnvLut(&gC3dContext.mLightEnv, GPU_LUT_FR, GPU_LUTINPUT_NV, false, &gC3dContext.mFresnelLut[fresnelPowerLevel]);
            C3D_LightEnvFresnel(&gC3dContext.mLightEnv, GPU_PRI_ALPHA_FRESNEL);

            glm::vec4 fresnelColorFloat = material->GetFresnelColor();
            fresnelColorFloat = glm::clamp(fresnelColorFloat, 0.0f, 1.0f);

            fresnelColor[0] = uint8_t(fresnelColorFloat.r * 255.0f);
            fresnelColor[1] = uint8_t(fresnelColorFloat.g * 255.0f);
            fresnelColor[2] = uint8_t(fresnelColorFloat.b * 255.0f);
            fresnelColor[3] = uint8_t(255);
        }
        else
        {
            C3D_LightEnvFresnel(&gC3dContext.mLightEnv, GPU_NO_FRESNEL);
        }

        // Diffuse is in Primary Fragment Color
        // Specular is in Secondary Fragment Color

        int32_t tevIdx = 0;
        C3D_TexEnv* env = nullptr;

        // Blend textures first
        bool vertexColorBlend = (vertexColorMode == VertexColorMode::TextureBlend);
        for (uint32_t i = 0; i < 3; ++i)
        {
            Texture* texture = material->GetTexture((TextureSlot)i);
            TevMode tevMode = (i == 0) ? TevMode::Replace : material->GetTevMode(i);

            if (i == 0 && texture == nullptr)
                texture = Renderer::Get()->mWhiteTexture.Get<Texture>();

            if (tevMode != TevMode::Pass &&
                texture != nullptr)
            {
                C3D_TexBind(i, &texture->GetResource()->mTex);

                env = C3D_GetTexEnv(tevIdx);
                C3D_TexEnvInit(env);
                ConfigTev(env, i, tevMode, vertexColorBlend);
                ++tevIdx;
            }
        }

        GPU_TEVSRC lightSrc = GPU_PRIMARY_COLOR;
        if (shadingModel == ShadingModel::Toon)
            lightSrc = GPU_FRAGMENT_SECONDARY_COLOR;
        else if (shadingModel == ShadingModel::Lit)
            lightSrc = GPU_FRAGMENT_PRIMARY_COLOR;

        env = C3D_GetTexEnv(tevIdx);
        C3D_TexEnvInit(env);
        C3D_TexEnvSrc(env, C3D_RGB, GPU_PREVIOUS, lightSrc, GPU_PRIMARY_COLOR);
        C3D_TexEnvFunc(env, C3D_RGB, GPU_MODULATE);
        C3D_TexEnvSrc(env, C3D_Alpha, GPU_PREVIOUS, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR);
        C3D_TexEnvFunc(env, C3D_Alpha, GPU_MODULATE);
        if (vertexColorBlend && shadingModel == ShadingModel::Unlit)
        {
            C3D_TexEnvOpRgb(env, GPU_TEVOP_RGB_SRC_COLOR, GPU_TEVOP_RGB_SRC_ALPHA, GPU_TEVOP_RGB_SRC_COLOR);
        }
        ++tevIdx;

        if (shadingModel == ShadingModel::Lit && specular > 0.0f)
        {
            env = C3D_GetTexEnv(tevIdx);
            C3D_TexEnvInit(env);
            C3D_TexEnvSrc(env, C3D_RGB, GPU_PREVIOUS, GPU_FRAGMENT_SECONDARY_COLOR, GPU_PRIMARY_COLOR);
            C3D_TexEnvFunc(env, C3D_RGB, GPU_ADD);
            C3D_TexEnvSrc(env, C3D_Alpha, GPU_PREVIOUS, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR);
            C3D_TexEnvFunc(env, C3D_Alpha, GPU_REPLACE);
            ++tevIdx;
        }

        env = C3D_GetTexEnv(tevIdx);
        C3D_TexEnvInit(env);
        C3D_TexEnvColor(env, *reinterpret_cast<uint32_t*>(matColor4));
        C3D_TexEnvSrc(env, C3D_Both, GPU_PREVIOUS, GPU_CONSTANT, GPU_PRIMARY_COLOR);
        C3D_TexEnvFunc(env, C3D_Both, GPU_MODULATE);
        C3D_TexEnvSrc(env, C3D_Alpha, GPU_PREVIOUS, GPU_CONSTANT, GPU_PRIMARY_COLOR);
        C3D_TexEnvFunc(env, C3D_Alpha, GPU_MODULATE);
        ++tevIdx;

        if (fresnelEnabled)
        {
            env = C3D_GetTexEnv(tevIdx);
            C3D_TexEnvInit(env);
            C3D_TexEnvColor(env, *reinterpret_cast<uint32_t*>(fresnelColor));
            C3D_TexEnvOpRgb(env, GPU_TEVOP_RGB_SRC_COLOR, GPU_TEVOP_RGB_SRC_ALPHA, GPU_TEVOP_RGB_SRC_COLOR);
            C3D_TexEnvSrc(env, C3D_RGB, GPU_CONSTANT, GPU_FRAGMENT_PRIMARY_COLOR, GPU_PREVIOUS);
            C3D_TexEnvFunc(env, C3D_RGB, GPU_MULTIPLY_ADD);
            C3D_TexEnvSrc(env, C3D_Alpha, GPU_PREVIOUS, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR);
            C3D_TexEnvFunc(env, C3D_Alpha, GPU_REPLACE);
            ++tevIdx;
        }

        bool depthEnabled = !depthless;

        // Blending
        if (blendMode == BlendMode::Opaque)
        {
            C3D_AlphaTest(false, GPU_ALWAYS, 0);
            C3D_DepthTest(depthEnabled, GPU_GREATER, depthEnabled ? GPU_WRITE_ALL : GPU_WRITE_COLOR);
            //C3D_EarlyDepthTest(true, GPU_EARLYDEPTH_GREATER, 0);
            C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ZERO, GPU_ZERO, GPU_ZERO);
        }
        else if (blendMode == BlendMode::Masked)
        {
            C3D_AlphaTest(true, GPU_GEQUAL, int32_t(material->GetMaskCutoff() * 255.0f));
            C3D_DepthTest(depthEnabled, GPU_GREATER, depthEnabled ? GPU_WRITE_ALL : GPU_WRITE_COLOR);
            //C3D_EarlyDepthTest(false, GPU_EARLYDEPTH_GREATER, 0);
            C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ZERO, GPU_ZERO, GPU_ZERO);
        }
        else if (blendMode == BlendMode::Translucent)
        {
            C3D_AlphaTest(false, GPU_ALWAYS, 0);
            C3D_DepthTest(depthEnabled, GPU_GREATER, GPU_WRITE_COLOR);
            //C3D_EarlyDepthTest(true, GPU_EARLYDEPTH_GREATER, 0);
            C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA, GPU_ZERO, GPU_ZERO);
        }
        else if (blendMode == BlendMode::Additive)
        {
            C3D_AlphaTest(false, GPU_ALWAYS, 0);
            C3D_DepthTest(depthEnabled, GPU_GREATER, GPU_WRITE_COLOR);
            //C3D_EarlyDepthTest(true, GPU_EARLYDEPTH_GREATER, 0);
            C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ONE, GPU_ZERO, GPU_ZERO);
        }

        gC3dContext.mLastBoundMaterial = material;
    }
}

void SetupLighting()
{
    // Create a null light environment?
    C3D_LightEnvInit(&gC3dContext.mNoLightEnv);

    C3D_LightEnvInit(&gC3dContext.mLightEnv);
    C3D_LightEnvBind(&gC3dContext.mLightEnv);
    C3D_LightEnvLut(&gC3dContext.mLightEnv, GPU_LUT_D0, GPU_LUTINPUT_NH, false, &gC3dContext.mLightLut[Shininess32]);

    CameraComponent* cameraComp = GetWorld()->GetActiveCamera();
    if (cameraComp == nullptr)
    {
        return;
    }

    uint32_t lightIndex = 0;
    DirectionalLightComponent* dirLightComp = GetWorld()->GetDirectionalLight();

    // Setup directional light
    if (dirLightComp)
    {
        glm::vec3 lightPosWS = -dirLightComp->GetDirection() * 10000.0f;
        glm::vec4 lightPosVS = cameraComp->GetViewMatrix() * glm::vec4(lightPosWS, 1.0f);

        C3D_FVec lightVec = FVec4_New(lightPosVS.x, lightPosVS.y, lightPosVS.z, 1.0f);
        glm::vec4 lightColor = dirLightComp->GetColor();

        C3D_Light& light = gC3dContext.mLights[0];
        C3D_LightInit(&light, &gC3dContext.mLightEnv);
        C3D_LightColor(&light, lightColor.r, lightColor.g, lightColor.b);
        C3D_LightPosition(&light, &lightVec);
        C3D_LightDistAttnEnable(&light, false);

        lightIndex++;
    }

    // Setup point lights
    const std::vector<PointLightComponent*>& pointLights = GetWorld()->GetPointLights();

    for (uint32_t i = 0; i < pointLights.size() && lightIndex < 8; ++i)
    {
        PointLightComponent* pointLightComp = pointLights[i];
        C3D_Light& light = gC3dContext.mLights[lightIndex];

        if (pointLightComp)
        {
            glm::vec3 lightPosWS = pointLightComp->GetAbsolutePosition();
            glm::vec4 lightPosVS = cameraComp->GetViewMatrix() * glm::vec4(lightPosWS, 1.0f);
            float lightRadius = pointLightComp->GetRadius();

            glm::vec4 lightColor = pointLightComp->GetColor();
            C3D_FVec lightVec = FVec4_New(lightPosVS.x, lightPosVS.y, lightPosVS.z, 1.0f);

            C3D_LightInit(&light, &gC3dContext.mLightEnv);
            C3D_LightColor(&light, lightColor.r, lightColor.g, lightColor.b);
            C3D_LightPosition(&light, &lightVec);
            C3D_LightDistAttnEnable(&light, true);

            // Generate a new Lut if the light radius is different than last frame.
            if (gC3dContext.mLightRadii[lightIndex] != lightRadius)
            {
                gC3dContext.mLightRadii[lightIndex] = lightRadius;
                LightLutDA_Create(&gC3dContext.mLightAttenuationLuts[lightIndex], LinearAttenFunc, 0.0f, lightRadius, lightRadius, 0.0f);
            }

            C3D_LightDistAttn(&light, &gC3dContext.mLightAttenuationLuts[lightIndex]);

            lightIndex++;
        }
    }
}

void PrepareForwardRendering()
{
    C3D_CullFace(GPU_CULL_BACK_CCW);
}

void PrepareUiRendering()
{
    C3D_AlphaTest(false, GPU_ALWAYS, 0);
    C3D_DepthTest(false, GPU_GREATER, GPU_WRITE_COLOR);
    C3D_EarlyDepthTest(false, GPU_EARLYDEPTH_GREATER, 0);
    C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);

    ResetTexEnv();
    ResetLightingEnv();

    C3D_CullFace(GPU_CULL_NONE);
}

void UploadUvOffsetScale(int8_t uniformLoc, Material* material, uint32_t uvIndex)
{
    glm::vec2 uvOffset = material->GetUvOffset(uvIndex);
    glm::vec2 uvScale = material->GetUvScale(uvIndex);
    glm::vec4 uvOffsetScale = glm::vec4(uvOffset.x, uvOffset.y, uvScale.x, uvScale.y);
    C3D_FVUnifSet(
        GPU_VERTEX_SHADER,
        uniformLoc,
        uvOffsetScale.x,
        uvOffsetScale.y,
        uvOffsetScale.z,
        uvOffsetScale.w);
}

void ResetTexEnv()
{
    C3D_TexEnvInit(C3D_GetTexEnv(0));
    C3D_TexEnvInit(C3D_GetTexEnv(1));
    C3D_TexEnvInit(C3D_GetTexEnv(2));
    C3D_TexEnvInit(C3D_GetTexEnv(3));
    C3D_TexEnvInit(C3D_GetTexEnv(4));
    C3D_TexEnvInit(C3D_GetTexEnv(5));
}

void ResetLightingEnv()
{
    C3D_LightEnvBind(&gC3dContext.mNoLightEnv);
}

void QueueLinearFree(void* data)
{
    gC3dContext.mLinearDestroyQueue[gC3dContext.mFrameIndex].push_back(data);
}

void QueueTexFree(C3D_Tex tex)
{
    gC3dContext.mTexDestroyQueue[gC3dContext.mFrameIndex].push_back(tex);
}

void ProcessQueuedFrees()
{
    std::vector<void*>& linearQueue = gC3dContext.mLinearDestroyQueue[gC3dContext.mFrameIndex];
    std::vector<C3D_Tex>& texQueue = gC3dContext.mTexDestroyQueue[gC3dContext.mFrameIndex];

    for (uint32_t i = 0; i < linearQueue.size(); ++i)
    {
        linearFree(linearQueue[i]);
    }

    for (uint32_t i = 0; i < texQueue.size(); ++i)
    {
        C3D_TexDelete(&texQueue[i]);
    }

    linearQueue.clear();
    texQueue.clear();
}

uint32_t GlmColorToRGBA8(glm::vec4 color)
{
    uint8_t a = (uint8_t) glm::clamp(uint32_t(color.a * 255), 0ul, 255ul);
    uint8_t b = (uint8_t) glm::clamp(uint32_t(color.b * 255), 0ul, 255ul);
    uint8_t g = (uint8_t) glm::clamp(uint32_t(color.g * 255), 0ul, 255ul);
    uint8_t r = (uint8_t) glm::clamp(uint32_t(color.r * 255), 0ul, 255ul);

    return ((a << 24) | (b << 16) | (g << 8) | (r << 0));
}

uint32_t GlmColorToRGB8(glm::vec4 color)
{
    uint8_t b = (uint8_t)glm::clamp(uint32_t(color.b * 255), 0ul, 255ul);
    uint8_t g = (uint8_t)glm::clamp(uint32_t(color.g * 255), 0ul, 255ul);
    uint8_t r = (uint8_t)glm::clamp(uint32_t(color.r * 255), 0ul, 255ul);

    return ((b << 16) | (g << 8) | (r << 0));
}

void ConfigTev(C3D_TexEnv* env, uint32_t textureSlot, TevMode mode, bool vertexColorBlend)
{
    GPU_TEVSRC texSrc = GPU_TEXTURE0;
    GPU_TEVOP_RGB blendColorSrc = GPU_TEVOP_RGB_SRC_R;

    switch (textureSlot)
    {
        case 0: texSrc = GPU_TEXTURE0; blendColorSrc = GPU_TEVOP_RGB_SRC_R; break;
        case 1: texSrc = GPU_TEXTURE1; blendColorSrc = GPU_TEVOP_RGB_SRC_G; break;
        case 2: texSrc = GPU_TEXTURE2; blendColorSrc = GPU_TEVOP_RGB_SRC_B; break;
    }

    if (textureSlot == 0)
    {
        // always replace for tex 0
        C3D_TexEnvSrc(env, C3D_RGB, texSrc, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR);
        C3D_TexEnvFunc(env, C3D_RGB, GPU_REPLACE);
        C3D_TexEnvSrc(env, C3D_Alpha, texSrc, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR);
        C3D_TexEnvFunc(env, C3D_Alpha, GPU_REPLACE);
    }
    else if (vertexColorBlend)
    {
        C3D_TexEnvSrc(env, C3D_RGB, texSrc, GPU_PREVIOUS, GPU_PRIMARY_COLOR);
        C3D_TexEnvFunc(env, C3D_RGB, GPU_INTERPOLATE);
        C3D_TexEnvSrc(env, C3D_Alpha, GPU_PREVIOUS, GPU_PREVIOUS, GPU_PREVIOUS);
        C3D_TexEnvFunc(env, C3D_Alpha, GPU_REPLACE);
        C3D_TexEnvOpRgb(env, GPU_TEVOP_RGB_SRC_COLOR, GPU_TEVOP_RGB_SRC_COLOR, blendColorSrc);
    }
    else if (mode == TevMode::Decal)
    {
        C3D_TexEnvSrc(env, C3D_RGB, texSrc, GPU_PREVIOUS, texSrc);
        C3D_TexEnvFunc(env, C3D_RGB, GPU_INTERPOLATE);
        C3D_TexEnvSrc(env, C3D_Alpha, GPU_PREVIOUS, GPU_PREVIOUS, GPU_PREVIOUS);
        C3D_TexEnvFunc(env, C3D_Alpha, GPU_REPLACE);
        C3D_TexEnvOpRgb(env, GPU_TEVOP_RGB_SRC_COLOR, GPU_TEVOP_RGB_SRC_COLOR, GPU_TEVOP_RGB_SRC_ALPHA);
    }
    else
    {
        GPU_COMBINEFUNC tevFunc = GPU_REPLACE;

        switch (mode)
        {
            case TevMode::Replace: tevFunc = GPU_REPLACE; break;
            case TevMode::Modulate: tevFunc = GPU_MODULATE; break;
            case TevMode::Add: tevFunc = GPU_ADD; break;
            case TevMode::SignedAdd: tevFunc = GPU_ADD_SIGNED; break;
            case TevMode::Subtract: tevFunc = GPU_SUBTRACT; break;
            default: tevFunc = GPU_REPLACE; break;
        }

        C3D_TexEnvSrc(env, C3D_RGB, GPU_PREVIOUS, texSrc, GPU_PRIMARY_COLOR);
        C3D_TexEnvFunc(env, C3D_RGB, tevFunc);
        C3D_TexEnvSrc(env, C3D_Alpha, GPU_PREVIOUS, texSrc, GPU_PRIMARY_COLOR);
        C3D_TexEnvFunc(env, C3D_Alpha, tevFunc);
    }
}

#endif
