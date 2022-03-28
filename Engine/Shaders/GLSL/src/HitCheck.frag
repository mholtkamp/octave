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

layout(location = 0) in vec2 inTexcoord;

layout (location = 0) out uint outId;

void main()
{
	outId = geometry.mHitCheckId;
}