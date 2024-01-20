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
#if VERTEX_TYPE_SKINNED
	SkinnedGeometryUniforms geometry;
#else
	GeometryUniforms geometry;
#endif
};

---CUSTOM-PARAMS---

// Vertex Inputs
#if VERTEX_TYPE_BASIC
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexcoord0;
layout(location = 2) in vec2 inTexcoord1;
layout(location = 3) in vec3 inNormal;
#elif VERTEX_TYPE_COLOR
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexcoord0;
layout(location = 2) in vec2 inTexcoord1;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec4 inColor;
#elif VERTEX_TYPE_PARTICLE
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexcoord;
layout(location = 2) in vec4 inColor;
#elif VERTEX_TYPE_SKINNED
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexcoord0;
layout(location = 2) in vec2 inTexcoord1;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in uvec4 inBoneIndices;
layout(location = 5) in vec4 inBoneWeights;
#endif

// Varying Outputs
layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec2 outTexcoord0;
layout(location = 2) out vec2 outTexcoord1;
layout(location = 3) out vec3 outNormal;
layout(location = 4) out vec4 outColor;

out gl_PerVertex 
{
    vec4 gl_Position;
};

---CUSTOM-CODE---

void main()
{
    VertIn vIn;
    VertOut vOut;

#if VERTEX_TYPE_BASIC
    vIn.mPosition = (geometry.mWorldMatrix * vec4(inPosition, 1.0)).xyz;
    vIn.mUv0 = inTexcoord0;
    vIn.mUv1 = inTexcoord1;
    vIn.mNormal = normalize((geometry.mNormalMatrix * vec4(inNormal, 0.0)).xyz);
    vIn.mColor = vec4(1,1,1,1);
#elif VERTEX_TYPE_COLOR
    vIn.mPosition = (geometry.mWorldMatrix * vec4(inPosition, 1.0)).xyz;   
    vIn.mUv0 = inTexcoord0;
    vIn.mUv1 = inTexcoord1;
    vIn.mNormal = normalize((geometry.mNormalMatrix * vec4(inNormal, 0.0)).xyz);
    vIn.mColor = inColor;
#elif VERTEX_TYPE_PARTICLE
    vIn.mPosition = (geometry.mWorldMatrix * vec4(inPosition, 1.0)).xyz;
    vIn.mUv0 = inTexcoord;
    vIn.mUv1 = inTexcoord;
    vIn.mNormal = normalize((geometry.mNormalMatrix * vec4(0.0, 0.0, 1.0, 0.0)).xyz);
    vIn.mColor = inColor;
#elif VERTEX_TYPE_SKINNED
    vec3 skinnedPosition = inPosition;
    vec3 skinnedNormal = inNormal;
    SkinVertex(skinnedPosition, skinnedNormal, inBoneIndices, inBoneWeights, geometry);

    vIn.mPosition = (geometry.mWorldMatrix * vec4(skinnedPosition, 1.0)).xyz;
    vIn.mUv0 = inTexcoord0;
    vIn.mUv1 = inTexcoord1;
    vIn.mNormal = normalize((geometry.mNormalMatrix * vec4(skinnedNormal, 0.0)).xyz);
    vIn.mColor = vec4(1, 1, 1, 1);
#endif
    
    // If the user's vertex shader does nothing, then the output should be the input.
    vOut.mPosition = vIn.mPosition;
    vOut.mUv0 = vIn.mUv0;
    vOut.mUv1 = vIn.mUv1;
    vOut.mNormal = vIn.mNormal;
    vOut.mColor = vIn.mColor;

    VertMain(vIn, vOut);

    // Convert the world space position to clip space.
    gl_Position = global.mViewProj * vec4(vOut.mPosition, 1.0);
}  

