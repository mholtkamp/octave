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
layout(location = 4) in vec4 inColor;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = global.mShadowColor;

    ApplyFog(outColor, inPosition, global);
}