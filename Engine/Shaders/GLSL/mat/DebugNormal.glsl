MAT_SCALAR(NormAlpha);
MAT_SCALAR(DispAlpha);

MAT_VECTOR(OffColor);

MAT_TEXTURE(OffTex);

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
