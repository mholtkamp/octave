#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "Common.glsl"
#include "Skinning.glsl"

layout (set = 0, binding = 0) uniform GlobalUniformBuffer 
{
    GlobalUniforms global;
};

layout (set = 1, binding = 0) uniform GeometryUniformBuffer 
{
	SkinnedGeometryUniforms geometry;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexcoord0;
layout(location = 2) in vec2 inTexcoord1;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in uvec4 inBoneIndices;
layout(location = 5) in vec4 inBoneWeights;

layout(location = 0) out vec2 outTexcoord;

out gl_PerVertex 
{
    vec4 gl_Position;
};

void main()
{
    vec3 skinnedPosition = inPosition;
    vec3 skinnedNormal = inNormal;
    SkinVertex(skinnedPosition, skinnedNormal, inBoneIndices, inBoneWeights, geometry);

    gl_Position = global.mShadowViewProj * geometry.mWorldMatrix * vec4(skinnedPosition, 1.0);
    outTexcoord = inTexcoord0;
}