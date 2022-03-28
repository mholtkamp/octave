#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (set = 1, binding = 0) uniform sampler2D samplerLitColor;

layout (location = 0) in vec2 inTexcoord;

layout (location = 0) out vec4 outFinalColor;

void main()
{
    outFinalColor = texture(samplerLitColor, inTexcoord);
}