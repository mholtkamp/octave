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
layout(location = 0) out vec4 outColor;

void main()
{
    outColor = SrgbToLinear(geometry.mColor);
}