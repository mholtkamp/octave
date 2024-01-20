MAT_SCALAR(NormAlpha);
MAT_SCALAR(DispAlpha);

MAT_VECTOR(OffColor);

MAT_TEXTURE(OffTex);

void VertMain(VertIn vIn, inout VertOut vOut)
{
    //vOut.mPosition = vIn.mPosition + vIn.mNormal * DispAlpha;
    vOut.mPosition = vIn.mPosition;
}

void FragMain(FragIn fIn, inout FragOut fOut)
{
    vec4 outColor = OffColor * vec4(fIn.mNormal, 1.0);
    outColor *= texture(OffTex, fIn.mUv0);
    fOut.mColor = vec4(fIn.mNormal, 1.0) + outColor;
}
