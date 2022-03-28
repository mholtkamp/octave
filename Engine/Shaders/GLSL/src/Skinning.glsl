
void SkinVertex(
    inout vec3 position,
    inout vec3 normal, 
    uvec4 boneIndices,
    vec4 boneWeights,
    SkinnedGeometryUniforms geometry)
{
    mat4 boneMatrix;

    if (geometry.mNumBoneInfluences == 1)
    {
        boneMatrix = geometry.mBoneMatrices[boneIndices.x];
    }
    else
    {
        boneMatrix =  geometry.mBoneMatrices[boneIndices.x] * boneWeights.x;
        boneMatrix += geometry.mBoneMatrices[boneIndices.y] * boneWeights.y;
        boneMatrix += geometry.mBoneMatrices[boneIndices.z] * boneWeights.z;
        boneMatrix += geometry.mBoneMatrices[boneIndices.w] * boneWeights.w;
    }

    position = (boneMatrix * vec4(position, 1.0f)).xyz;
    normal = (boneMatrix * vec4(normal, 0.0f)).xyz;
}
