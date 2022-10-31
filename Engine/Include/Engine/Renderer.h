#pragma once

#include <vector>
#include "glm/glm.hpp"
#include <array>

#include "EngineTypes.h"
#include "Assets/Texture.h"
#include "Assets/StaticMesh.h"
#include "Assets/Material.h"
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

class Renderer
{
public:

    static void Create();
    static void Destroy();
    static Renderer* Get();

    void Render(World* world);
    ~Renderer();
    void Initialize();

    void AddWidget(class Widget* widget, int32_t index = -1, int32_t screenIndex = 0);
    void RemoveWidget(class Widget* widget, int32_t screenIndex = 0);
    void RemoveAllWidgets(int32_t screenIndex = -1);
    void EnableStatsOverlay(bool enable);
    bool IsStatsOverlayEnabled() const;

    // Renderer never assumes ownership of the modal widget.
    // The caller needs to hold on to the widget so they can free it when needed.
    void SetModalWidget(Widget* widget);
    Widget* GetModalWidget();
    bool IsInModalWidgetUpdate() const;

    void DirtyAllWidgets();
    const std::vector<Widget*>& GetWidgets(int32_t screenIndex = 0);

    Console* GetConsoleWidget();
    StatsOverlay* GetStatsWidget();

    void SetEngineState(EngineState* engineState);
    uint32_t GetFrameNumber() const;
    uint32_t GetFrameIndex() const;
    uint32_t GetScreenIndex() const;

    glm::vec2 GetScreenResolution(int32_t screen = -1);
    glm::vec2 GetActiveScreenResolution();

    float GetGlobalUiScale() const;
    void SetGlobalUiScale(float scale);

    Actor* ProcessHitCheck(World* world, int32_t x, int32_t y);

    void SetDebugMode(DebugMode mode);
    DebugMode GetDebugMode() const;
    void EnableProxyRendering(bool enable);
    bool IsProxyRenderingEnabled() const;
    void SetBoundsDebugMode(BoundsDebugMode mode);
    BoundsDebugMode GetBoundsDebugMode() const;

    void EnableFrustumCulling(bool enable);
    bool IsFrustumCullingEnabled() const;

    Texture* GetBlackTexture();
    Material* GetDefaultMaterial();

    void LoadDefaultTextures();
    void LoadDefaultMaterials();
    void LoadDefaultMeshes();
    void LoadDefaultFonts();

    void AddDebugDraw(const DebugDraw& draw);
    void RemoveDebugDrawsForActor(Actor* actor);
    const std::vector<DebugDraw>& GetDebugDraws() const;

    const std::vector<LightData>& GetLightData() const;

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
    void RenderDraws(const std::vector<DrawData>& drawData, PipelineId pipelineId);
    void RenderDebugDraws(const std::vector<DebugDraw>& draws, PipelineId pipelineId = PipelineId::Count);
    void FrustumCull(CameraComponent* camera);
    int32_t FrustumCullDraws(const CameraFrustum& frustum, std::vector<DrawData>& drawData);
    int32_t FrustumCullDraws(const CameraFrustum& frustum, std::vector<DebugDraw>& drawData);
    int32_t FrustumCullLights(const CameraFrustum& frustum, std::vector<LightData>& lightData);

    void RenderShadowCasters(World* world);
    void RenderSelectedGeometry(World* world);

    void UpdateDebugDraws();

    std::vector<Widget*> mWidgets0;
    std::vector<Widget*> mWidgets1;  // Second-screen widget for 3DS

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

    std::vector<LightData> mLightData;

    std::vector<DebugDraw> mDebugDraws;
    std::vector<DebugDraw> mCollisionDraws;

    uint32_t mFrameIndex = 0;
    uint32_t mScreenIndex = 0;
    uint32_t mFrameNumber = 0;
    float mGlobalUiScale = 1.0f;
    DebugMode mDebugMode = DEBUG_NONE;
    BoundsDebugMode mBoundsDebugMode = BoundsDebugMode::Off;
    bool mFrustumCulling = true;
    bool mEnableProxyRendering = false;
    bool mInModalWidgetUpdate = false;
};
