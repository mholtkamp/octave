
#define RAY_TRACE_LIGHT_POINT 0
#define RAY_TRACE_LIGHT_DIRECTIONAL 1

struct Ray
{
    vec3 mOrigin;
    vec3 mDirection;
};

struct RayTraceVertex
{
    vec3 mPosition;
    float mPad0;

    vec2 mTexcoord0;
    vec2 mTexcoord1;

    vec3 mNormal;
    float mPad1;

    vec4 mColor;
};

struct RayTraceTriangle
{
    RayTraceVertex mVertices[3];
};

struct RayTraceMesh
{
    vec4 mBounds;

    uint mStartTriangleIndex;
    uint mNumTriangles;
    uint mCastShadows;
    uint mHasBakedLighting;

    uvec4 mTextures;
    
    MaterialUniforms mMaterial;
};

struct RayTraceLight
{
    vec3 mPosition;
    float mRadius;

    vec4 mColor;

    vec3 mDirection;
    uint mLightType;

    uint mCastShadows;
    uint mPad0;
    uint mPad1;
    uint mPad2;
};

struct LightBakeVertex
{
    vec3 mPosition;
    float mPad0;

    vec3 mNormal;
    float mPad1;

    vec4 mDirectLight;
    vec4 mIndirectLight;
};

struct VertexLightData
{
    vec4 mDirectLight;
    vec4 mIndirectLight;
};

struct DiffuseTriangle
{
    uvec3 mVertexIndices;
    uint mPad0;
};

struct RayTraceUniforms
{
    uint mNumTriangles;
    uint mNumMeshes;
    uint mNumLights;
    uint mMaxBounces;

    uint mRaysPerThread;
    uint mAccumulatedFrames;
    uint mNumBakeVertices;
    uint mNumBakeTriangles;

    float mShadowBias;
    uint mDiffuseDirect;
    uint mDiffuseIndirect;
    int mBakeMeshIndex;

    uint mReceiveShadows;
    uint mPad0;
    uint mPad1;
    uint mPad2;

    vec4 mSkyZenithColor;
    vec4 mSkyHorizonColor;
    vec4 mGroundColor;
};

struct HitInfo
{
    bool mHit;
    float mDistance;
    vec3 mPosition;
    vec3 mNormal;
    vec2 mUv0;
    vec2 mUv1;
    vec4 mColor;

    RayTraceMesh mMesh;
};
