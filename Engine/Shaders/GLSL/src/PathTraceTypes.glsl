
#define PATH_TRACE_LIGHT_POINT 0
#define PATH_TRACE_LIGHT_DIRECTIONAL 1

struct Ray
{
    vec3 mOrigin;
    vec3 mDirection;
};

struct PathTraceVertex
{
    vec3 mPosition;
    float mPad0;

    vec2 mTexcoord0;
    vec2 mTexcoord1;

    vec3 mNormal;
    float mPad1;

    vec4 mColor;
};

struct PathTraceTriangle
{
    PathTraceVertex mVertices[3];
};

struct PathTraceMesh
{
    vec4 mBounds;

    uint mStartTriangleIndex;
    uint mNumTriangles;
    uint mPad0;
    uint mPad1;

    uvec4 mTextures;
    
    MaterialUniforms mMaterial;
};

struct PathTraceLight
{
    vec3 mPosition;
    float mRadius;

    vec4 mColor;

    uint mLightType;
    vec3 mDirection;
};

struct PathTraceUniforms
{
    uint mNumTriangles;
    uint mNumMeshes;
    uint mNumLights;
    uint mMaxBounces;

    uint mRaysPerPixel;
    uint mAccumulatedFrames;
    uint mPad1;
    uint mPad2;
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

    PathTraceMesh mMesh;
};
