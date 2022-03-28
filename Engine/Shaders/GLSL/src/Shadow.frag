#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "Common.glsl"

layout(set = 0, binding = 0) uniform GlobalUniformBuffer 
{
    GlobalUniforms global;
};

layout(set = 1, binding = 0) uniform GeometryUniformBuffer 
{
	GeometryUniforms geometry;
};

layout(set = 2, binding = 0) uniform MaterialUniformBuffer
{
    MaterialUniforms material;
};

layout(set = 2, binding = 1) uniform sampler2D sampler0;
layout(set = 2, binding = 2) uniform sampler2D sampler1;
layout(set = 2, binding = 3) uniform sampler2D sampler2;
layout(set = 2, binding = 4) uniform sampler2D sampler3;

layout(location = 0) in vec2 inTexcoord;

void main()
{
    vec4 color = texture(sampler0, inTexcoord);

    if (color.a < material.mMaskCutoff)
    {
        discard;
    }
}