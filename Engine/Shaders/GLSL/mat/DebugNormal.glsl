MAT_SCALAR(NormAlpha);
MAT_SCALAR(DispAlpha);
MAT_SCALAR(NoiseScale);

MAT_VECTOR(OffColor);
MAT_VECTOR(NoiseUvSpeed);

MAT_TEXTURE(OffTex);
MAT_TEXTURE(NoiseTex);

void VertMain(VertIn vIn, inout VertOut vOut)
{
    //vOut.mPosition = vIn.mPosition + vIn.mNormal * DispAlpha;
    //vOut.mPosition = vIn.mPosition;

    vec2 noiseUv = vIn.mUv0;
    noiseUv = (noiseUv + NoiseUvSpeed.xy * global.mTime) * NoiseScale;
    float noiseVal = texture(NoiseTex, noiseUv).r;

    vec3 newPos = vIn.mPosition + (vIn.mNormal * noiseVal * DispAlpha);
    vOut.mPosition = newPos;
}

void FragMain(FragIn fIn, inout FragOut fOut)
{
    vec4 normColor = OffColor * vec4(fIn.mNormal, 1.0);
    normColor *= texture(OffTex, fIn.mUv0);

    vec2 noiseUv = fIn.mUv0;
    noiseUv = (noiseUv + NoiseUvSpeed.xy * global.mTime) * NoiseScale;
    float noiseVal = texture(NoiseTex, noiseUv).r;
    vec4 noiseColor = vec4(0, 1, 0, 1) * noiseVal;

    //normColor.r = 2.0 * sin(global.mTime * 8);

    fOut.mColor = normColor + noiseColor;
}
