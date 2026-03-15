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

layout(location = 5) flat in uint inInstanceIndex;

layout (location = 0) out vec4 outColor;

void main()
{
	vec4 defaultLitColor = global.mSelectedColor;
	vec4 defaultUnlitColor = vec4(global.mSelectedColor.rgb, global.mSelectedColor.a * 0.3);
	const vec4 instanceLitColor = vec4(1.0, 0.1, 0.6, 0.6);
	const vec4 instanceUnlitColor = vec4(1.0, 0.1, 0.3, 0.2);
	float checkerSize = global.mSelectedCheckerSize;
	bool x = fract(gl_FragCoord.x / checkerSize) >= 0.5;
	bool y = fract(gl_FragCoord.y / checkerSize) >= 0.5;

	vec4 litColor = defaultLitColor;
	vec4 unlitColor = defaultUnlitColor;

	if (global.mSelectedInstance != -1 &&
		global.mSelectedInstance == int(inInstanceIndex))
	{
		litColor = instanceLitColor;
		unlitColor = instanceUnlitColor;
	}

	if (x ^^ y)
	{
		outColor = litColor;
	}
	else
	{
		outColor = unlitColor;
	}
}
