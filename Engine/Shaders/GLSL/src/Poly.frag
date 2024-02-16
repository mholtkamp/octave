#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "Common.glsl"

layout (set = 0, binding = 0) uniform GlobalUniformBuffer 
{
    GlobalUniforms global;
};

layout (set = 1, binding = 0) uniform PolyUniformBuffer
{
    mat4 mTransform;
    vec4 mColor;
        
    float mX;
    float mY;
    float mPad0;
    float mPad1;

} polyData;
layout (set = 1, binding = 1) uniform sampler2D polySampler;

layout (location = 0) in vec2 inTexcoord;
layout (location = 1) in vec4 inColor;

layout (location = 0) out vec4 outFinalColor;

void main()
{
    vec4 textureColor = LinearToSrgb(texture(polySampler, inTexcoord));
    outFinalColor = textureColor * inColor * polyData.mColor;
}