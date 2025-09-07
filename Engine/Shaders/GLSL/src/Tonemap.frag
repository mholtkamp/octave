#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "Common.glsl"

struct TonemapUniforms
{
    int mPathTracingEnabled;
    int mPad0;
    int mPad1;
    int mPad2;
};

layout (set = 0, binding = 0) uniform GlobalUniformBuffer 
{
    GlobalUniforms global;
};

layout (set = 1, binding = 0) uniform TonemapUniformBuffer
{
    TonemapUniforms tonemap;
};

layout (set = 1, binding = 1) uniform sampler2D samplerLitColor;
layout (set = 1, binding = 2) uniform sampler2D pathTraceImage;

layout (location = 0) in vec2 inTexcoord;

layout (location = 0) out vec4 outFinalColor;

void main()
{
    vec4 sceneColor = vec4(0,0,0,1);

    if (tonemap.mPathTracingEnabled != 0)
    {
        sceneColor = texture(pathTraceImage, inTexcoord);
    }
    else
    {
        sceneColor = texture(samplerLitColor, inTexcoord);
    }

    if (global.mLinearColorSpace != 0)
    {
        // HDR tonemapping
        //color = color / (color + vec3(1.0));

        // Gamma correct
        outFinalColor = LinearToSrgb(sceneColor);
    }
    else
    {
        outFinalColor = sceneColor;
    }
}
