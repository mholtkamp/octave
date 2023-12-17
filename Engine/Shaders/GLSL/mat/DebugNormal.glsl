MAT_UNIFORMS
{
    vec4 OffColor;
    float NormAlpha;
    float DispAlpha;
};

MAT_TEXTURE(OffTex, 1);

void VertMain(VertIn vIn, inout VertOut vOut)
{
    vOut.mPosition = vIn.mPosition + vIn.mNormal * DispAlpha;
}

void FragMain(FragIn fIn, inout FragOut fOut)
{
    vec4 outColor = OffColor * fIn.mNormal;
    outColor *= texture(Offtex, fIn.mUv0);
    fOut.mColor = outColor;
}
