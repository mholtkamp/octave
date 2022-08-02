#define MAX_POINTLIGHTS 7
#define MAX_TEXTURES 4

#define SHADING_MODEL_UNLIT 0
#define SHADING_MODEL_LIT 1
#define SHADING_MODEL_TOON 2

#define BLEND_MODE_OPAQUE 0
#define BLEND_MODE_MASKED 1
#define BLEND_MODE_TRANSLUCENT 2
#define BLEND_MODE_ADDITIVE 3

#define FOG_FUNC_LINEAR 0
#define FOG_FUNC_EXPONENTIAL 1

#define MAX_BONES 64
#define MAX_BONE_INFLUENCES 4

struct GlobalUniforms
{
    mat4 mViewProj;
    mat4 mDirectionalLightVP;

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
    int mPadding0;
    int mPadding1;

    vec4 mFogColor;

    int mFogEnabled;
    int mFogDensityFunc;
    float mFogNear;
    float mFogFar;
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
    vec2 mUvOffset;
    vec2 mUvScale;

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
    float mPadding0;
    float mPadding1;
    float mPadding2;

    uint mUvMaps[MAX_TEXTURES];
};

const mat4 SHADOW_BIAS_MAT = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );
