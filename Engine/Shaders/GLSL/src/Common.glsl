#define MAX_POINTLIGHTS 7
#define MAX_TEXTURES 4

#define SHADING_MODEL_UNLIT 0
#define SHADING_MODEL_LIT 1
#define SHADING_MODEL_TOON 2

#define BLEND_MODE_OPAQUE 0
#define BLEND_MODE_MASKED 1
#define BLEND_MODE_TRANSLUCENT 2
#define BLEND_MODE_ADDITIVE 3

#define VERTEX_COLOR_NONE 0
#define VERTEX_COLOR_MODULATE 1
#define VERTEX_COLOR_TEXTURE_BLEND 2

#define FOG_FUNC_LINEAR 0
#define FOG_FUNC_EXPONENTIAL 1

#define TEV_MODE_REPLACE 0
#define TEV_MODE_MODULATE 1
#define TEV_MODE_DECAL 2
#define TEV_MODE_ADD 3
#define TEV_MODE_SIGNED_ADD 4
#define TEV_MODE_SUBTRACT 5
#define TEV_MODE_INTERPOLATE 6
#define TEV_MODE_PASS 7

#define MAX_BONES 64
#define MAX_BONE_INFLUENCES 4

#define PI 3.14159265359

struct GlobalUniforms
{
    mat4 mViewProj;
    mat4 mDirectionalLightVP;
    mat4 mViewToWorld;

    vec4 mDirectionalLightDirection;
    vec4 mDirectionalLightColor;
    vec4 mAmbientLightColor;
    vec4 mViewPosition;
    vec4 mViewDirection;
    vec2 mScreenDimensions;
    vec2 mInterfaceResolution;
    vec4 mPointLightPositions[MAX_POINTLIGHTS];
    vec4 mPointLightColors[MAX_POINTLIGHTS];
    vec4 mShadowColor;

    int mNumPointLights;
    int mVisualizationMode;
    float mGameElapsedTime;
    float mRealElapsedTime;

    vec4 mFogColor;

    int mFogEnabled;
    int mFogDensityFunc;
    float mFogNear;
    float mFogFar;

    float mNearHalfWidth;
    float mNearHalfHeight;
    float mNearDist;
    uint mFrameNumber;

    uint mPathTracingEnabled;
    uint mPad0;
    uint mPad1;
    uint mPad2;
};

struct GeometryUniforms 
{
    mat4 mWVP;
    mat4 mWorldMatrix;
    mat4 mNormalMatrix;
    mat4 mLightWVP;
	vec4 mColor;

	uint mHitCheckId;
    uint mPadding0;
    uint mPadding1;
    uint mPadding2;
};

struct SkinnedGeometryUniforms 
{
    mat4 mWVP;
    mat4 mWorldMatrix;
    mat4 mNormalMatrix;
    mat4 mLightWVP;
	vec4 mColor;

	uint mHitCheckId;
    uint mPadding0;
    uint mPadding1;
    uint mPadding2;

    mat4 mBoneMatrices[MAX_BONES];

    uint mNumBoneInfluences;
    uint mPadding3;
    uint mPadding4;
    uint mPadding5;
};

struct MaterialUniforms
{
    vec2 mUvOffset0;
    vec2 mUvScale0;

    vec2 mUvOffset1;
    vec2 mUvScale1;

    vec4 mColor;
    vec4 mFresnelColor;

    uint mShadingModel;
    uint mBlendMode;
    uint mToonSteps;
    float mFresnelPower;

    float mSpecular;
    float mOpacity;
    float mMaskCutoff;
    float mShininess;

    uint mFresnelEnabled;
    uint mVertexColorMode;
    uint mApplyFog;
    float mPadding0;

    uvec4 mUvMaps; // MAX_TEXTURES
    uvec4 mTevModes; // MAX_TEXTURES
};

const mat4 SHADOW_BIAS_MAT = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );
