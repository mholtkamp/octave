#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "Common.glsl"
#include "Fog.glsl"

const float SHADAOW_DEPTH_BIAS = 0.0005f;

layout (constant_id = 0) const uint kShadingModel = SHADING_MODEL_LIT;
layout (constant_id = 1) const uint kBlendMode = BLEND_MODE_OPAQUE;
layout (constant_id = 2) const uint kVertexColorMode = VERTEX_COLOR_NONE;
layout (constant_id = 3) const bool kHasSpecular = false;
layout (constant_id = 4) const bool kHasUv1 = false;
layout (constant_id = 5) const bool kHasFresnel = false;
layout (constant_id = 6) const bool kHasFog = false;
layout (constant_id = 7) const bool kHasWrapLighting = false;
layout (constant_id = 8) const uint kNumTextures = 1;

layout (constant_id = 9) const uint kTev0 = TEV_MODE_REPLACE;
layout (constant_id = 10) const uint kTev1 = TEV_MODE_PASS;
layout (constant_id = 11) const uint kTev2 = TEV_MODE_PASS;
layout (constant_id = 12) const uint kTev3 = TEV_MODE_PASS;

layout (set = 0, binding = 0) uniform GlobalUniformBuffer 
{
    GlobalUniforms global;
};

layout (set = 0, binding = 1) uniform sampler2D shadowSampler;

layout (set = 1, binding = 0) uniform GeometryUniformBuffer 
{
	GeometryUniforms geometry;
};

layout(set = 2, binding = 0) uniform MaterialUniformBuffer
{
    MaterialUniforms material;
};

layout(set = 2, binding = 1) uniform sampler2D sampler0;
layout(set = 2, binding = 2) uniform sampler2D sampler1;
layout(set = 2, binding = 3) uniform sampler2D sampler2;
layout(set = 2, binding = 4) uniform sampler2D sampler3;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexcoord0;
layout(location = 2) in vec2 inTexcoord1;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec4 inColor;

layout(location = 0) out vec4 outColor;

vec4 CalculateLighting(uint shadingModel, vec3 L, vec3 N, vec3 V, vec4 color, float attenuation)
{
    vec4 retLighting = vec4(0,0,0,0);

    if (shadingModel == SHADING_MODEL_LIT)
    {
        float diffuseIntensity = CalcLightIntensity(N, L, kHasWrapLighting ? material.mWrapLighting : 0.0);
        vec4 lightColor = diffuseIntensity * color;

        if (kHasSpecular)
        {
            vec3 reflectDir = reflect(-L, N);  
            float specularIntensity = pow(max(dot(V, reflectDir), 0.0), material.mShininess);
            vec4 specularColor = material.mSpecular * specularIntensity * color;
            lightColor += specularColor;
        }

        retLighting = attenuation * lightColor;
    }
    else if (shadingModel == SHADING_MODEL_TOON)
    {
        float power = clamp(dot(L, N), 0.0, 1.0) * attenuation;
        uint toonSteps = material.mToonSteps;
        power = floor(max(power - 0.001, 0.0) * toonSteps)/toonSteps * (toonSteps/ (toonSteps - 1.0));
        retLighting = power * color;
    }

    return retLighting;
}

float CalculateShadow(vec4 sc)
{
    float visibility = 0.0f;

    if (texture(shadowSampler, sc.xy /*+ vec2(dx*x, dy*y)*/).r + SHADAOW_DEPTH_BIAS >=  sc.z)
    {
        visibility = 1.0f;
    }

    return visibility;
}

void main()
{
    vec2 texCoord0 = (inTexcoord0 + material.mUvOffset0) * material.mUvScale0;
    vec2 texCoord1 = kHasUv1 ? ((inTexcoord1 + material.mUvOffset1) * material.mUvScale1) : inTexcoord0;

    uint shadingModel = kShadingModel;

    vec4 vertColor = (kVertexColorMode == VERTEX_COLOR_NONE) ? vec4(1,1,1,1) : inColor;
    vec4 diffuse = vec4(1,1,1,1);

    if (kNumTextures >= 1)
        diffuse = BlendTexture(material, diffuse, 0, sampler0, texCoord0, texCoord1, vertColor.r, kTev0, kVertexColorMode);

    if (kNumTextures >= 2)
        diffuse = BlendTexture(material, diffuse, 1, sampler1, texCoord0, texCoord1, vertColor.g, kTev1, kVertexColorMode);

    if (kNumTextures >= 3)
        diffuse = BlendTexture(material, diffuse, 2, sampler2, texCoord0, texCoord1, vertColor.b, kTev2, kVertexColorMode);

    if (kNumTextures >= 4)
        diffuse = BlendTexture(material, diffuse, 3, sampler3, texCoord0, texCoord1, 0.0, kTev3, kVertexColorMode);

    diffuse *= material.mColor;

    if (kBlendMode == BLEND_MODE_MASKED && diffuse.a < material.mMaskCutoff)
    {
        discard;
    }

    outColor = diffuse;

    bool hasBakedLighting = (geometry.mHasBakedLighting != 0); // kHasBakedLighting

    // Apply baked lighting modulation first
    if (hasBakedLighting)
    {
        if (kVertexColorMode == VERTEX_COLOR_MODULATE)
        {
            vec4 modColor = vertColor;
            outColor *= modColor;
        }
        else if (kVertexColorMode == VERTEX_COLOR_TEXTURE_BLEND)
        {
            float modValue = vertColor.a;
            outColor *= modValue;
        }
    }

    vec3 N = normalize(inNormal);
    vec3 V = normalize(global.mViewPosition.xyz - inPosition);

    if (shadingModel != SHADING_MODEL_UNLIT)
    {
        if (!hasBakedLighting)
        {
            outColor = vec4(0, 0, 0, 1);
        }

        vec4 totalLight = hasBakedLighting ? vec4(0, 0, 0, 1) : global.mAmbientLightColor;

        uint numLights = geometry.mNumLights;

        for (uint i = 0; i < numLights; ++i)
        {
            uint lightIndex = 0;

            if (i < 4)
            {
                lightIndex = (geometry.mLights0 >> (i * 8)) & 0xff;
            }
            else
            {
                lightIndex = (geometry.mLights1 >> ((i - 4) * 8)) & 0xff;
            }

            LightData light = global.mLights[lightIndex];

            if (light.mType == LIGHT_TYPE_DIRECTIONAL)
            {
                vec3 L = -1.0 * normalize(light.mDirection);
                vec4 lightColor = light.mColor;

                vec4 dirLighting = CalculateLighting(shadingModel, L, N, V, lightColor, 1.0);
                float shadowVis = 1.0; //CalculateShadow(inShadowCoordinate);
                totalLight += dirLighting * shadowVis;
            }
            else if (light.mType == LIGHT_TYPE_POINT)
            {
                vec3 lightPos = light.mPosition;
                vec4 lightColor = light.mColor;
                float lightRadius = light.mRadius;

                vec3 toLight = lightPos - inPosition;
                float dist = length(toLight);

                vec3 L = normalize(toLight);
                float attenuation =  1.0 - clamp(dist / lightRadius, 0.0, 1.0);

                totalLight += CalculateLighting(shadingModel, L, N, V, lightColor, attenuation);
            }
        }

        outColor += totalLight * diffuse;
    }

    if (!hasBakedLighting)
    {
        if (kVertexColorMode == VERTEX_COLOR_MODULATE)
        {
            outColor *= vertColor;
        }
        else if (kVertexColorMode == VERTEX_COLOR_TEXTURE_BLEND)
        {
            outColor *= vertColor.a;
        }
    }

    if (kHasFresnel)
    {
        float intensity = pow((1 - abs(dot(N, V))), material.mFresnelPower);
        outColor += intensity * material.mFresnelColor;
    }

    if (kHasFog)
    {
        ApplyFog(outColor, inPosition, global);
    }

    if (kBlendMode == BLEND_MODE_TRANSLUCENT ||
        kBlendMode == BLEND_MODE_ADDITIVE)
    {
        outColor.a = vertColor.a * diffuse.a * material.mOpacity;
    }
}
