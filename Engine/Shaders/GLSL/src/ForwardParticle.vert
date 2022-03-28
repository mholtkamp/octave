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
layout(location = 1) in vec2 inTexcoord;
layout(location = 2) in vec4 inColor;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec2 outTexcoord;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec4 outShadowCoordinate;
layout(location = 4) out vec4 outColor;

out gl_PerVertex 
{
    vec4 gl_Position;
};

void main()
{
    gl_Position = geometry.mWVP * vec4(inPosition, 1.0);
    
    outPosition = (geometry.mWorldMatrix * vec4(inPosition, 1.0)).xyz;    
    outTexcoord = inTexcoord;    
    outNormal = normalize((geometry.mNormalMatrix * vec4(0.0, 0.0, 1.0, 0.0)).xyz);
    outColor = inColor;

    // Shadow map coordinate computation
    vec4 shadowCoord = (SHADOW_BIAS_MAT * global.mDirectionalLightVP) * vec4(outPosition, 1.0);
    shadowCoord = shadowCoord / shadowCoord.w;
    outShadowCoordinate = shadowCoord;
}
