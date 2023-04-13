
#define PATH_TRACE_LIGHT_POINT 0
#define PATH_TRACE_LIGHT_DIRECTIONAL 1

struct PathTraceVertex
{
    vec3 mPosition;
    float mPad0;

    vec2 mTexcoord0;
    vec2 mTexcoord1;

    vec3 mNormal;
    uint mColor;
};

struct PathTraceTriangle
{
    VertexDatum mVertices[3];
};

struct PathTraceMesh
{
    vec4 mBounds;

    uint mStartTriangleIndex;
    uint mNumTriangles;
    uint mPad0;
    uint mPad1;
    
    MaterialUniforms mMaterial;
};

struct PathTraceLight
{
    vec3 mPosition;
    float mRadius;

    vec4 mColor;

    uint mLightType;
    vec3 mDirection;
}

