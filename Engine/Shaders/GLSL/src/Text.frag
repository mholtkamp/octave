#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "Common.glsl"

layout (set = 0, binding = 0) uniform GlobalUniformBuffer 
{
    GlobalUniforms global;
};

layout (set = 1, binding = 0) uniform TextUniformBuffer
{
    mat4 mTransform;
    vec4 mColor;
    
    float mX;
    float mY;
    float mCutoff;
    float mOutlineSize;

    float mSize;
    float mSoftness;
    float mPadding1;
    float mPadding2;

    int mDistanceField;
    int mEffect;
} textData;
layout (set = 1, binding = 1) uniform sampler2D fontSampler;

layout (location = 0) in vec2 inTexcoord;
layout (location = 1) in vec4 inColor;

layout (location = 0) out vec4 outFinalColor;

void main()
{
	vec4 fontColor = LinearToSrgb(texture(fontSampler, inTexcoord));

    if (textData.mDistanceField != 0)
    {
        float cutoff = textData.mCutoff;
        float fadeDist = textData.mSoftness;

        //fontColor.a = smoothstep(cutoff - fadeDist, cutoff + fadeDist, fontColor.a);
        
        float scale = 1.0 / max(fwidth(fontColor.a), 0.00001);
        float signedDistance = (fontColor.a - cutoff) * scale;
        fontColor.a = clamp(signedDistance + cutoff + scale * fadeDist, 0.0, 1.0);
    }
    
    outFinalColor = inColor * textData.mColor * fontColor;
}