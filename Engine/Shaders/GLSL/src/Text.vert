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

    float mX;
    float mY;
    float mCutoff;
    float mOutlineSize;

    float mScale;
    float mPadding1;
    float mPadding2;
    float mPadding3;

    int mDistanceField;
    int mEffect;
} textData;
layout (set = 1, binding = 1) uniform sampler2D quadSampler;

layout (location = 0) in vec2 inPosition;
layout (location = 1) in vec2 inTexcoord;
layout (location = 2) in vec4 inColor;

layout (location = 0) out vec2 outTexcoord;
layout (location = 1) out vec4 outColor;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main() 
{    
    outTexcoord = inTexcoord;
    outColor = inColor;
    vec2 position = inPosition.xy * textData.mScale + vec2(textData.mX, textData.mY);

    mat3 transform = mat3(textData.mTransform);
    position = (transform * vec3(position, 1.0)).xy;

    position.xy /= global.mInterfaceResolution;
    position.xy *= 2.0f;
    position.xy -= 1.0f;
    
    gl_Position = vec4(position.xy, 0.0, 1.0);
}
