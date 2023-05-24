
void ApplyFog(inout vec4 ioColor,
             vec3 fragmentPos,
             GlobalUniforms global)
{
    if (global.mFogEnabled != 0)
    {

        float fragDepth = distance(fragmentPos, global.mViewPosition.xyz);
        float fogLength = (global.mFogFar - global.mFogNear);
        float fogAlpha = clamp((fragDepth - global.mFogNear) / fogLength, 0.0, 1.0);
        fogAlpha *= global.mFogColor.a;
        float fogFactor = 0.0;

        if (global.mFogDensityFunc == FOG_FUNC_LINEAR)
        {
            fogFactor = fogAlpha;
        }
        else if (global.mFogDensityFunc == FOG_FUNC_EXPONENTIAL)
        {
            // Just hack this for now to get close to dolphin exponential.
            fogFactor = 1.0 - clamp(pow(20,-fogAlpha), 0.0 , 1.0);
        }

        ioColor.rgb = mix(ioColor.rgb, global.mFogColor.rgb, fogFactor);
    }
}
