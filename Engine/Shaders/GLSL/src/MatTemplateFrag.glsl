#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "Common.glsl"
#include "Fog.glsl"

layout (set = 0, binding = 0) uniform GlobalUniformBuffer 
{
    GlobalUniforms global;
};

layout (set = 0, binding = 1) uniform sampler2D shadowSampler;

layout (set = 1, binding = 0) uniform GeometryUniformBuffer 
{
	GeometryUniforms geometry;
};

---CUSTOM-PARAMS---

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexcoord0;
layout(location = 2) in vec2 inTexcoord1;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec4 inColor;

layout(location = 0) out vec4 outColor;

struct FragIn
{
    vec3 mPosition;
    vec2 mUv0;
    vec2 mUv1;
    vec3 mNormal;
    vec4 mColor;
};

struct FragOut
{
    vec4 mColor;
};

---CUSTOM-CODE---

void main()
{
    FragIn fIn;
    FragOut fOut;

    fIn.mPosition = inPosition;
    fIn.mUv0 = inTexcoord0;
    fIn.mUv1 = inTexcoord1;
    fIn.mNormal = inNormal;
    fIn.mColor = inColor;

    fOut.mColor = inColor;

    FragMain(fIn, fOut);

#if MATERIAL_MASKED
    // For now, I am ignoring Material's MaskCutoff value, so shaders will
    // just have to offset the fOut.mColor.a value to get the desired result.
    if (fOut.mColor.a < 0.5)
    {
        discard;
    }
#endif

#if MATERIAL_APPLY_FOG
    ApplyFog(outColor, inPosition, global);
#endif
}
