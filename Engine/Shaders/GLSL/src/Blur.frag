#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "Common.glsl"

struct BlurUniforms
{
    int mHorizontal;
    int mNumSamples;
    float mBlurSize;
    int mPad0;
};

layout (set = 0, binding = 0) uniform GlobalUniformBuffer 
{
    GlobalUniforms global;
};

layout (set = 1, binding = 0) uniform BlurUniformBuffer 
{
	BlurUniforms blur;
};

layout (set = 1, binding = 1) uniform sampler2D srcTexture;

layout (location = 0) in vec2 inTexcoord;

layout (location = 0) out vec4 outColor;

void main()
{
    int numSamples = blur.mNumSamples;
    float blurSize = blur.mBlurSize;

    vec4 totalColor = vec4(0,0,0,0);

    if (blur.mHorizontal != 0)
    {
        float invAspect = global.mSceneViewport.w / global.mSceneViewport.z;
        blurSize = invAspect * blurSize;

        for (int i = 0; i < numSamples; ++i)
        {
            float delta = i / (numSamples - 1.0) - 0.5;
            vec2 uv = inTexcoord + vec2(delta * blurSize, 0);
            totalColor += texture(srcTexture, uv);
        }
    }
    else
    {
        for (int i = 0; i < numSamples; ++i)
        {
            float delta = i / (numSamples - 1.0) - 0.5;
            vec2 uv = inTexcoord + vec2(0, delta * blurSize);
            totalColor += texture(srcTexture, uv);
        }
    }

    outColor = totalColor / numSamples;
}


