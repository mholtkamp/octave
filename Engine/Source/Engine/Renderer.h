#pragma once

#include <vector>
#include "glm/glm.hpp"
#include <array>

#include "EngineTypes.h"
#include "Assets/Texture.h"
#include "Assets/StaticMesh.h"
#include "Assets/MaterialLite.h"
#include "Vertex.h"
#include "World.h"
#include "Constants.h"
#include "Log.h"
#include "Profiler.h"

class Widget;
class Console;
class StatsOverlay;
class CameraFrustum;

struct EngineState;

struct FadingLight
{
    // mNode should only be used for comparisons!! If deleted, we want to fade it out, not crash.
    Light3D* mComponent = nullptr;
    LightData mData = {};
    glm::vec4 mColor = { 0.0f, 0.0f, 0.0f, 0.0f };
    float mAlpha = 0.0f;

    FadingLight(Light3D* comp) : mComponent(comp) {}
};

struct LightDistance2
{
    Light3D* mComponent = nullptr;
    float mDistance2 = 0.0f;

    LightDistance2(Light3D* comp, float dist2) : mComponent(comp), mDistance2(dist2) {}
};

class Renderer
{
public:

    static void Create();
    static void Destroy();
    static Renderer* Get();

    void Render(World* world, int32_t screenIndex);
    ~Renderer();
    void Initialize();

    void GatherProperties(std::vector<Property>& props);

    void EnableStatsOverlay(bool enable);
    bool IsStatsOverlayEnabled() const;
    void EnableConsole(bool enable);
    bool IsConsoleEnabled();

    // Renderer never assumes ownership of the modal widget.
    // The caller needs to hold on to the widget so they can free it when needed.
    void SetModalWidget(Widget* widget);
    Widget* GetModalWidget();
    bool IsInModalWidgetUpdate() const;

    void DirtyAllWidgets();

    Console* GetConsoleWidget();
    StatsOverlay* GetStatsWidget();

    void SetEngineState(EngineState* engineState);
    uint32_t GetFrameNumber() const;
    uint32_t GetFrameIndex() const;
    uint32_t GetScreenIndex() const;
    bool IsRenderingFirstScreen() const;
    bool IsRenderingLastScreen() const;
    World* GetCurrentWorld();

    glm::vec2 GetScreenResolution(int32_t screen = -1);
    glm::vec2 GetActiveScreenResolution();

    float GetGlobalUiScale() const;
    void SetGlobalUiScale(float scale);

    Node3D* ProcessHitCheck(World* world, int32_t x, int32_t y, uint32_t* outInstance = nullptr);

    void SetDebugMode(DebugMode mode);
    DebugMode GetDebugMode() const;
    void EnableProxyRendering(bool enable);
    bool IsProxyRenderingEnabled() const;
    void SetBoundsDebugMode(BoundsDebugMode mode);
    BoundsDebugMode GetBoundsDebugMode() const;

    void EnableFrustumCulling(bool enable);
    bool IsFrustumCullingEnabled() const;

    void Enable3dRendering(bool enable);
    bool Is3dRenderingEnabled() const;
    void Enable2dRendering(bool enable);
    bool Is2dRenderingEnabled() const;

    void EnablePathTracing(bool enable);
    bool IsPathTracingEnabled() const;

    Texture* GetBlackTexture();
    MaterialLite* GetDefaultMaterial();

    void LoadDefaultTextures();
    void LoadDefaultMaterials();
    void LoadDefaultMeshes();
    void LoadDefaultFonts();

    void AddDebugDraw(const DebugDraw& draw);
    void RemoveDebugDrawsForNode(Node* node);
    const std::vector<DebugDraw>& GetDebugDraws() const;

    const std::vector<LightData>& GetLightData() const;

    void BeginLightBake();
    void EndLightBake();
    bool IsLightBakeInProgress() const;
    float GetLightBakeProgress() const;

    bool IsLightFadeEnabled() const;
    void EnableLightFade(bool enable);
    void SetLightFadeLimit(uint32_t limit);
    uint32_t GetLightFadeLimit() const;
    void SetLightFadeSpeed(float speed);
    float GetLightFadeSpeed() const;

    bool IsPostProcessPassEnabled(PostProcessPassId passId) const;
    void EnablePostProcessPass(PostProcessPassId passId, bool enable);

    void SetResolutionScale(float scale);
    float GetResolutionScale() const;

    uint32_t GetViewportX();
    uint32_t GetViewportY();
    uint32_t GetViewportWidth();
    uint32_t GetViewportHeight();
    glm::uvec4 GetViewport();
    glm::uvec4 GetSceneViewport();

    // Property Getters
    uint32_t GetRaysPerPixel() const;
    uint32_t GetMaxBounces() const;
    bool IsPathTraceAccumulationEnabled() const;
    uint32_t GetBakeRaysPerVertex() const;
    uint32_t GetBakeMaxBounces() const;
    float GetBakeShadowBias() const;
    uint32_t GetBakeIndirectIterations() const;
    uint32_t GetBakeDiffuseDirectPasses() const;
    uint32_t GetBakeDiffuseIndirectPasses() const;
    glm::vec4 GetSkyZenithColor() const;
    glm::vec4 GetSkyHorizonColor() const;
    glm::vec4 GetGroundColor() const;

    // Default Textures
    TextureRef mWhiteTexture;
    TextureRef mBlackTexture;
    TextureRef mDefaultColorTexture;
    TextureRef mDefaultColorAlphaTexture;
    TextureRef mDefaultNormalTexture;
    TextureRef mDefaultORMTexture;

    // Default Materials
    MaterialRef mDefaultMaterial;

    // Default Meshes
    StaticMeshRef mConeMesh;
    StaticMeshRef mCubeMesh;
    StaticMeshRef mCylinderMesh;
    StaticMeshRef mPlaneMesh;
    StaticMeshRef mSphereMesh;
    StaticMeshRef mSphere112Mesh;
    StaticMeshRef mTorusMesh;

    // Default Fonts
    FontRef mRoboto32;
    FontRef mRoboto16;
    FontRef mRobotoMono16;
    FontRef mRobotoMono8;

private:

    static Renderer* sInstance;
    Renderer();

    void BeginFrame();
    void EndFrame();

    void GatherDrawData(World* world);
    void GatherLightData(World* world);
    void RenderDraws(const std::vector<DrawData>& drawData);
    void RenderDraws(const std::vector<DrawData>& drawData, PipelineConfig pipelineConfig);
    void RenderDebugDraws(const std::vector<DebugDraw>& draws, PipelineConfig pipelineConfig = PipelineConfig::Count);
    void FrustumCull(Camera3D* camera);
    int32_t FrustumCullDraws(const CameraFrustum& frustum, std::vector<DrawData>& drawData);
    int32_t FrustumCullDraws(const CameraFrustum& frustum, std::vector<DebugDraw>& drawData);
    int32_t FrustumCullLights(const CameraFrustum& frustum, std::vector<LightData>& lightData);

    void RenderShadowCasters(World* world);
    void RenderSelectedGeometry(World* world);

    void UpdateDebugDraws();

    Widget* mModalWidget = nullptr;
    StatsOverlay* mStatsWidget = nullptr;
    Console* mConsoleWidget = nullptr;

    EngineState* mEngineState = nullptr;

    bool mInitialized = false;

    std::vector<DrawData> mShadowDraws;
    std::vector<DrawData> mOpaqueDraws;
    std::vector<DrawData> mSimpleShadowDraws;
    std::vector<DrawData> mPostShadowOpaqueDraws; // (post-simple-shadow opaques. not talking about shadow mapping)
    std::vector<DrawData> mTranslucentDraws;
    std::vector<DrawData> mWireframeDraws;
    std::vector<DrawData> mWidgetDraws;

    std::vector<LightData> mLightData;

    std::vector<DebugDraw> mDebugDraws;
    std::vector<DebugDraw> mCollisionDraws;

    World* mCurrentWorld = nullptr;
    uint32_t mFrameIndex = 0;
    uint32_t mScreenIndex = 0;
    uint32_t mFrameNumber = 0;
    float mGlobalUiScale = 1.0f;
    DebugMode mDebugMode = DEBUG_NONE;
    BoundsDebugMode mBoundsDebugMode = BoundsDebugMode::Off;
    bool mFrustumCulling = true;
    bool mEnableProxyRendering = false;
    bool mEnable3dRendering = true;
    bool mEnable2dRendering = true;
    bool mEnablePathTracing = false;
    bool mInModalWidgetUpdate = false;
    bool mEnableLightFade = false;
    uint32_t mLightFadeLimit = 4;
    float mLightFadeSpeed = 1.0f;
    std::vector<FadingLight> mFadingLights;

    // Path tracing
    uint32_t mRaysPerPixel = 4;
    uint32_t mMaxBounces = 4;
    bool mPathTraceAccumulate = true;
    glm::vec4 mSkyZenithColor = { 0.0f, 0.2f, 0.6f, 1.0f };
    glm::vec4 mSkyHorizonColor = { 0.5f, 0.5f, 0.8f, 1.0f };
    glm::vec4 mGroundColor = { 0.3f, 0.35f, 0.32f, 1.0f };

    // Light Bake
    uint32_t mBakeRaysPerVertex = 4;
    uint32_t mBakeMaxBounces = 4;
    float mBakeShadowBias = 0.001f;
    uint32_t mBakeIndirectIterations = 20;
    uint32_t mBakeDiffuseDirectPasses = 2;
    uint32_t mBakeDiffuseIndirectPasses = 2;

    // Post Process
    bool mPostProcessEnables[(uint32_t)PostProcessPassId::Count] = { };

};
