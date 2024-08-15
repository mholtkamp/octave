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
	const vec4 defaultLitColor = vec4(0.2, 0.1, 1.0, 0.6);
	const vec4 defaultUnlitColor = vec4(0.8, 0.1, 1.0, 0.2);
	const vec4 instanceLitColor = vec4(1.0, 0.1, 0.6, 0.6);
	const vec4 instanceUnlitColor = vec4(1.0, 0.1, 0.3, 0.2);
	bool x = fract(gl_FragCoord.x/8) >= 0.5;
	bool y = fract(gl_FragCoord.y/8) >= 0.5;

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