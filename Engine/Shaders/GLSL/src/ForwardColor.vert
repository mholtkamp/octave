#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "Common.glsl"

layout (set = 0, binding = 0) uniform GlobalUniformBuffer 
{
    GlobalUniforms global;
};

layout (set = 1, binding = 0) uniform GeometryUniformBuffer 
{
	GeometryUniforms geometry;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexcoord0;
layout(location = 2) in vec2 inTexcoord1;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec4 inColor;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec2 outTexcoord0;
layout(location = 2) out vec2 outTexcoord1;
layout(location = 3) out vec3 outNormal;
layout(location = 4) out vec4 outColor;

out gl_PerVertex 
{
    vec4 gl_Position;
};

void main()
{
    gl_Position = geometry.mWVP * vec4(inPosition, 1.0);
    
    outPosition = (geometry.mWorldMatrix * vec4(inPosition, 1.0)).xyz;    
    outTexcoord0 = inTexcoord0;    
    outTexcoord1 = inTexcoord1;    
    outNormal = normalize((geometry.mNormalMatrix * vec4(inNormal, 0.0)).xyz);
    outColor = inColor;
}