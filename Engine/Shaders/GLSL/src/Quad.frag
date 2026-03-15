#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "Common.glsl"

layout (set = 0, binding = 0) uniform GlobalUniformBuffer
{
    GlobalUniforms global;
};

layout (set = 1, binding = 0) uniform QuadUniformBuffer
{
    mat4 mTransform;
    vec4 mColor;
    vec4 mQuadParams; // x = cornerRadius, y = width, z = height

} quadData;
layout (set = 1, binding = 1) uniform sampler2D quadSampler;

layout (location = 0) in vec2 inTexcoord;
layout (location = 1) in vec4 inColor;
layout (location = 2) in vec2 inLocalPos;

layout (location = 0) out vec4 outFinalColor;

// Signed distance function for a rounded rectangle.
// p = point relative to box center, b = box half-extents, r = corner radius.
float roundBox(vec2 p, vec2 b, float r)
{
    vec2 q = abs(p) - b + vec2(r);
    return length(max(q, 0.0)) - r;
}

void main()
{
    vec4 textureColor = texture(quadSampler, inTexcoord);
    if (global.mLinearColorSpace != 0)
    {
        textureColor = LinearToSrgb(textureColor);
    }

    float alpha = 1.0;
    float cornerRadius = quadData.mQuadParams.x;

    if (cornerRadius > 0.0)
    {
        vec2 quadSize = quadData.mQuadParams.yz;
        vec2 halfSize = quadSize * 0.5;

        // inLocalPos is 0-1 across the quad; convert to pixel-space centered on origin.
        vec2 pixelPos = (inLocalPos - 0.5) * quadSize;

        float d = roundBox(pixelPos, halfSize, cornerRadius);
        alpha = 1.0 - smoothstep(-0.5, 0.5, d);
    }

    outFinalColor = textureColor * inColor * quadData.mColor;
    outFinalColor.a *= alpha;
}
