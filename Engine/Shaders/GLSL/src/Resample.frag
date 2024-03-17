#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "Common.glsl"

layout (set = 0, binding = 0) uniform GlobalUniformBuffer 
{
    GlobalUniforms global;
};

layout (set = 1, binding = 0) uniform sampler2D srcTexture;

layout (location = 0) in vec2 inTexcoord;

layout (location = 0) out vec4 outFinalColor;

void main()
{
    vec4 srcColor = texture(srcTexture, inTexcoord);
    outFinalColor = srcColor;
}