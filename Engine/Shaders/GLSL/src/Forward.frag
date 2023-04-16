#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "Common.glsl"
#include "Fog.glsl"

const float SHADAOW_DEPTH_BIAS = 0.0005f;

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
layout(location = 4) in vec4 inShadowCoordinate;
layout(location = 5) in vec4 inColor;

layout(location = 0) out vec4 outColor;

vec4 CalculateLighting(uint shadingModel, vec3 L, vec3 N, vec3 V, vec4 color, float attenuation)
{
    vec4 retLighting = vec4(0,0,0,0);

    if (shadingModel == SHADING_MODEL_LIT)
    {
        float diffuseIntensity = clamp(dot(L, N), 0.0, 1.0);
        vec4 diffuseColor = diffuseIntensity * color;

        vec3 reflectDir = reflect(-L, N);  
        float specularIntensity = pow(max(dot(V, reflectDir), 0.0), material.mShininess);
        vec4 specularColor = material.mSpecular * specularIntensity * color;

        retLighting = attenuation * (diffuseColor + specularColor);
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
    vec2 texCoord1 = (inTexcoord1 + material.mUvOffset1) * material.mUvScale1;

    uint shadingModel = material.mShadingModel;

    vec4 diffuse = vec4(0,0,0,0);

    diffuse = BlendTexture(material, diffuse, 0, sampler0, texCoord0, texCoord1, inColor.r);
    diffuse = BlendTexture(material, diffuse, 1, sampler1, texCoord0, texCoord1, inColor.g);
    diffuse = BlendTexture(material, diffuse, 2, sampler2, texCoord0, texCoord1, inColor.b);
    diffuse = BlendTexture(material, diffuse, 3, sampler3, texCoord0, texCoord1, 0.0);

    diffuse *= material.mColor;

    if (material.mBlendMode == BLEND_MODE_MASKED && diffuse.a < material.mMaskCutoff)
    {
        discard;
    }

    outColor = diffuse;

    vec3 N = normalize(inNormal);
    vec3 V = normalize(global.mViewPosition.xyz - inPosition);

    if (shadingModel != SHADING_MODEL_UNLIT)
    {
        outColor = vec4(0, 0, 0, 1);

        vec4 totalLight = global.mAmbientLightColor;

        // Directional Light
        {
            vec3 L = -1.0 * normalize(global.mDirectionalLightDirection.xyz);
            vec4 lightColor = global.mDirectionalLightColor;

            vec4 dirLighting = CalculateLighting(shadingModel, L, N, V, lightColor, 1.0);
            float shadowVis = 1.0f; //CalculateShadow(inShadowCoordinate);
            totalLight += dirLighting * shadowVis;
        }

        //Point Lights
        for (int i = 0; i < global.mNumPointLights; ++i)
        {
            vec3 lightPos = global.mPointLightPositions[i].xyz;
            vec4 lightColor = global.mPointLightColors[i];
            float lightRadius = global.mPointLightPositions[i].w;

            vec3 toLight = lightPos - inPosition;
            float dist = length(toLight);

            vec3 L = normalize(toLight);
            float attenuation =  1.0 - clamp(dist / lightRadius, 0.0, 1.0);

            totalLight += CalculateLighting(shadingModel, L, N, V, lightColor, attenuation);
        }

        outColor = totalLight * diffuse;
    }

    if (material.mVertexColorMode == VERTEX_COLOR_MODULATE)
    {
        outColor *= inColor;
    }
    else if (material.mVertexColorMode == VERTEX_COLOR_TEXTURE_BLEND)
    {
        outColor *= inColor.a;
    }

    if (material.mFresnelEnabled != 0)
    {
        float intensity = pow((1 - abs(dot(N, V))), material.mFresnelPower);
        outColor += intensity * material.mFresnelColor;
    }

    if (material.mApplyFog != 0)
    {
        ApplyFog(outColor, inPosition, global);
    }

    outColor.a = inColor.a * diffuse.a * material.mOpacity;
}
