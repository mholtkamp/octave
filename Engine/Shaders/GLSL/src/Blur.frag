#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "Common.glsl"

#define BOX_BLUR 1

#define BLUR_MAX_SAMPLES 32

struct BlurUniforms
{
    int mHorizontal;
    int mNumSamples;
    float mBlurSize;
    float mSigmaRatio;

    int mBoxBlur;
    int mInputWidth;
    int mInputHeight;
    int mPad2;

    // Using a float array will align the floats to 16 bytes anyway.
    vec4 mGaussianWeights[BLUR_MAX_SAMPLES];
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
    int numSamples = clamp(blur.mNumSamples, 1, BLUR_MAX_SAMPLES);
    float blurSize = blur.mBlurSize;

    vec4 totalColor = vec4(0,0,0,0);

    float weightSum = 0.0;

    if (blur.mHorizontal != 0)
    {
        float invAspect = float(blur.mInputHeight) / blur.mInputWidth;
        blurSize = invAspect * blurSize;

        for (int i = 0; i < numSamples; ++i)
        {
            float offset = i / (numSamples - 1.0) - 0.5;
            vec2 uv = inTexcoord + vec2(offset * blurSize, 0);

            float sampleWeight = (blur.mBoxBlur != 0) ? 1.0 : blur.mGaussianWeights[i].x;
            weightSum += sampleWeight;
            totalColor += texture(srcTexture, uv) * sampleWeight;
        }
    }
    else
    {
        for (int i = 0; i < numSamples; ++i)
        {
            float offset = i / (numSamples - 1.0) - 0.5;
            vec2 uv = inTexcoord + vec2(0, offset * blurSize);

            float sampleWeight = (blur.mBoxBlur != 0) ? 1.0 : blur.mGaussianWeights[i].x;
            weightSum += sampleWeight;
            totalColor += texture(srcTexture, uv) * sampleWeight;
        }
    }

    outColor = totalColor / weightSum;
}


