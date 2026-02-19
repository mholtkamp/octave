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

layout (location = 0) out vec4 outColor;

void main()
{
	vec4 litColor = global.mSelectedColor;
	vec4 unlitColor = vec4(global.mSelectedColor.rgb, global.mSelectedColor.a * 0.3);
	float checkerSize = global.mSelectedCheckerSize;
	bool x = fract(gl_FragCoord.x / checkerSize) >= 0.5;
	bool y = fract(gl_FragCoord.y / checkerSize) >= 0.5;

	if (x ^^ y)
	{
		outColor = litColor;
	}
	else
	{
		outColor = unlitColor;
	}
}
