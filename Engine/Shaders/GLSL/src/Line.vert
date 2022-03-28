#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "Common.glsl"

layout (set = 0, binding = 0) uniform GlobalUniformBuffer 
{
	GlobalUniforms global;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 outColor;

out gl_PerVertex 
{
    vec4 gl_Position;
};

void main()
{
    gl_Position = global.mViewProj * vec4(inPosition, 1.0);
    outColor = inColor;
}