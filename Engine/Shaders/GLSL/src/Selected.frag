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

layout (location = 0) out vec4 outColor;

void main()
{
	const vec4 litColor = vec4(0.2, 0.1, 1.0, 0.6);
	const vec4 unlitColor = vec4(0.8, 0.1, 1.0, 0.2);
	bool x = fract(gl_FragCoord.x/8) >= 0.5;
	bool y = fract(gl_FragCoord.y/8) >= 0.5;
	
	if (x ^^ y)
	{
		outColor = litColor;
	}
	else
	{
		outColor = unlitColor;
	}
}