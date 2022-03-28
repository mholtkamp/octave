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
    vec4 mTint;

} quadData;
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
    mat3 transform = mat3(quadData.mTransform);
    vec2 outPos = (transform * vec3(inPosition, 1.0)).xy;
    outPos = (outPos / global.mInterfaceResolution) * 2.0f - 1.0f ;
    gl_Position = vec4(outPos, 0.0, 1.0);
}
