#include "Renderer.h"
#include "AssetManager.h"
#include "Utilities.h"
#include "Engine.h"
#include "Profiler.h"
#include "Constants.h"
#include "Widgets/Widget.h"
#include "Widgets/Console.h"
#include "Widgets/StatsOverlay.h"
#include "Assets/Font.h"
#include "Components/PointLightComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/ShadowMeshComponent.h"
#include "Log.h"
#include "Line.h"
#include "Maths.h"

#include "Graphics/Graphics.h"
#include "Graphics/GraphicsConstants.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <set>
#include <fstream>
#include <algorithm>
#include <malloc.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <btBulletDynamicsCommon.h>

#include <chrono>

#undef min
#undef max

using namespace std;
using namespace std::chrono;

Renderer* Renderer::sInstance = nullptr;

void Renderer::Create()
{
    Destroy();

    sInstance = new Renderer();
}

void Renderer::Destroy()
{
    if (sInstance != nullptr)
    {
        delete sInstance;
        sInstance = nullptr;
    }
}

Renderer* Renderer::Get()
{
    return sInstance;
}

Renderer::~Renderer()
{
    if (mConsoleWidget != nullptr)
    {
        delete mConsoleWidget;
        mConsoleWidget = nullptr;
    }

    if (mStatsWidget != nullptr)
    {
        delete mStatsWidget;
        mStatsWidget = nullptr;
    }

    // Renderer does not assume ownership of a modal widget.
}

void Renderer::Initialize()
{
#if EDITOR
    AssetManager::Get()->ImportEngineAssets();
#endif
    LoadDefaultTextures();
    LoadDefaultMaterials();
    LoadDefaultMeshes();
    LoadDefaultFonts();

#if CONSOLE_ENABLED
    mConsoleWidget = new Console();
#endif

    mStatsWidget = new StatsOverlay();

#if EDITOR
    EnableProxyRendering(true);
#endif
}

Actor* Renderer::ProcessHitCheck(World* world, int32_t x, int32_t y)
{
    return GFX_ProcessHitCheck(world, x, y);
}

void Renderer::SetDebugMode(DebugMode mode)
{
    mDebugMode = mode;

    // TODO: This was in vulkan code. Remove if not needed.
    //UpdateGlobalDescriptorSet();
}

DebugMode Renderer::GetDebugMode() const
{
    return mDebugMode;
}

void Renderer::EnableProxyRendering(bool enable)
{
    mEnableProxyRendering = enable;
}

bool Renderer::IsProxyRenderingEnabled() const
{
    return mEnableProxyRendering;
}

void Renderer::SetBoundsDebugMode(BoundsDebugMode mode)
{
    mBoundsDebugMode = mode;
}

BoundsDebugMode Renderer::GetBoundsDebugMode() const
{
    return mBoundsDebugMode;
}

void Renderer::EnableFrustumCulling(bool enable)
{
    mFrustumCulling = enable;
}

bool Renderer::IsFrustumCullingEnabled() const
{
    return mFrustumCulling;
}

Texture* Renderer::GetBlackTexture()
{
    return mBlackTexture.Get<Texture>();
}

Material* Renderer::GetDefaultMaterial()
{
    return mDefaultMaterial.Get<Material>();
}

void Renderer::SetEngineState(EngineState* engineState)
{
    mEngineState = engineState;
}

uint32_t Renderer::GetFrameNumber() const
{
    return mFrameNumber;
}

uint32_t Renderer::GetFrameIndex() const
{
    return mFrameIndex;
}

uint32_t Renderer::GetScreenIndex() const
{
    return mScreenIndex;
}

glm::vec2 Renderer::GetScreenResolution(int32_t screen)
{
    glm::vec2 res = { 0.0f, 0.0f };

    if (screen == -1)
    {
        screen = mScreenIndex;
    }

    if (screen == 0)
    {
        res.x = (float)GetEngineState()->mWindowWidth;
        res.y = (float)GetEngineState()->mWindowHeight;
    }
    else if (screen == 1)
    {
        res.x = (float)GetEngineState()->mSecondWindowWidth;
        res.y = (float)GetEngineState()->mSecondWindowHeight;
    }

    return res;
}

glm::vec2 Renderer::GetActiveScreenResolution()
{
    return GetScreenResolution(-1);
}

float Renderer::GetGlobalUiScale() const
{
    return mGlobalUiScale;
}

void Renderer::SetGlobalUiScale(float scale)
{
    mGlobalUiScale = scale;
    DirtyAllWidgets();
}

void Renderer::BeginFrame()
{
    GFX_BeginFrame();
    GFX_SetFog(GetWorld()->GetFogSettings());
}

void Renderer::EndFrame()
{
    GFX_EndFrame();

    mFrameNumber++;
    mFrameIndex = mFrameNumber % MAX_FRAMES;
}

void Renderer::AddWidget(class Widget* widget, int32_t index, int32_t screenIndex)
{
    std::vector<Widget*>& widgets = (screenIndex == 1) ? mWidgets1 : mWidgets0;

    if (index >= 0 && index <= (int32_t)widgets.size())
    {
        widgets.insert(widgets.begin() + index, widget);
    }
    else
    {
        widgets.push_back(widget);
    }
}

void Renderer::RemoveWidget(class Widget* widget, int32_t screenIndex)
{
    std::vector<Widget*>& widgets = (screenIndex == 1) ? mWidgets1 : mWidgets0;

    for (int32_t i = (int32_t)widgets.size() - 1; i >= 0; --i)
    {
        if (widgets[i] == widget)
        {
            widgets.erase(widgets.begin() + i);
        }
    }
}

void Renderer::RemoveAllWidgets(int32_t screenIndex)
{
    if (screenIndex == -1)
    {
        mWidgets0.clear();
        mWidgets1.clear();
    }
    else
    {
        std::vector<Widget*>& widgets = (screenIndex == 1) ? mWidgets1 : mWidgets0;
        widgets.clear();
    }
}

void Renderer::EnableStatsOverlay(bool enable)
{
    if (mStatsWidget)
    {
        mStatsWidget->SetVisible(enable);
    }
}

bool Renderer::IsStatsOverlayEnabled() const
{
    return mStatsWidget && mStatsWidget->IsVisible();
}

void Renderer::SetModalWidget(Widget* widget)
{
    mModalWidget = widget;
}

Widget* Renderer::GetModalWidget()
{
    return mModalWidget;
}

bool Renderer::IsInModalWidgetUpdate() const
{
    return mInModalWidgetUpdate;
}

void Renderer::DirtyAllWidgets()
{
    for (Widget* widget : mWidgets0)
    {
        widget->MarkDirty();
    }

    for (Widget* widget : mWidgets1)
    {
        widget->MarkDirty();
    }

    if (mModalWidget != nullptr)
        mModalWidget->MarkDirty();

    if (mConsoleWidget != nullptr)
        mConsoleWidget->MarkDirty();

    if (mStatsWidget != nullptr)
        mStatsWidget->MarkDirty();
}

const std::vector<Widget*>& Renderer::GetWidgets(int32_t screenIndex)
{
    return (screenIndex == 1) ? mWidgets1 : mWidgets0;
}

Console* Renderer::GetConsoleWidget()
{
    return mConsoleWidget;
}

StatsOverlay* Renderer::GetStatsWidget()
{
    return mStatsWidget;
}

Renderer::Renderer()
{

}

void Renderer::LoadDefaultTextures()
{
    mWhiteTexture = LoadAsset("T_White");
    mBlackTexture = LoadAsset("T_Black");
    mDefaultColorTexture = LoadAsset("T_DefaultColor");
    mDefaultColorAlphaTexture = LoadAsset("T_DefaultColorAlpha");
    mDefaultNormalTexture = LoadAsset("T_DefaultNormal");
    mDefaultORMTexture = LoadAsset("T_DefaultORM");
}

void Renderer::LoadDefaultMaterials()
{
    mDefaultMaterial = LoadAsset("M_Default");
}

void Renderer::LoadDefaultMeshes()
{
    mConeMesh = LoadAsset("SM_Cone");
    mCubeMesh = LoadAsset("SM_Cube");
    mCylinderMesh = LoadAsset("SM_Cylinder");
    mPlaneMesh = LoadAsset("SM_Plane");
    mSphereMesh = LoadAsset("SM_Sphere");
    mSphere112Mesh = LoadAsset("SM_Sphere_112");
    mTorusMesh = LoadAsset("SM_Torus");

    // Setup collision on several meshes
    // Did you crash here? Make sure you package the project once from the editor to build .oct files for engine assets.
    mCubeMesh.Get<StaticMesh>()->SetCollisionShape(new btBoxShape(btVector3(1.0f, 1.0f, 1.0f)));
    mSphereMesh.Get<StaticMesh>()->SetCollisionShape(new btSphereShape(1.0f));
}

void Renderer::LoadDefaultFonts()
{
    // Actually, maybe we don't want to load all of them if nothing is using them?
    // Text uses F_Roboto32 by default so just load that for now.
    mRoboto32 = LoadAsset("F_Roboto32");
    //mRoboto16 = LoadAsset("F_Roboto16");
    //mRobotoMono16 = LoadAsset("F_RobotoMono16");
    //mRobotoMono8 = LoadAsset("F_RobotoMono8");
}

void Renderer::AddDebugDraw(const DebugDraw& draw)
{
#if DEBUG_DRAW_ENABLED
    mDebugDraws.push_back(draw);
#endif
}

void Renderer::RemoveDebugDrawsForActor(Actor* actor)
{
#if DEBUG_DRAW_ENABLED
    for (int32_t i = int32_t(mDebugDraws.size()) - 1; i >= 0 ; --i)
    {
        if (mDebugDraws[i].mActor == actor)
        {
            mDebugDraws.erase(mDebugDraws.begin() + i);
        }
    }
#endif
}

const std::vector<DebugDraw>& Renderer::GetDebugDraws() const
{
    return mDebugDraws;
}

void Renderer::GatherDrawData(World* world)
{
    mShadowDraws.clear();
    mOpaqueDraws.clear();
    mSimpleShadowDraws.clear();
    mPostShadowOpaqueDraws.clear();
    mTranslucentDraws.clear();
    mWireframeDraws.clear();
    mCollisionDraws.clear();

    if (world != nullptr)
    {
        const std::vector<Actor*>& actors = world->GetActors();

        for (uint32_t i = 0; i < actors.size(); ++i)
        {
            const std::vector<Component*>& components = actors[i]->GetComponents();

            for (uint32_t c = 0; c < components.size(); ++c)
            {
                Component* comp = components[c];

                if (comp->IsPrimitiveComponent())
                {
                    DrawData data = comp->GetDrawData();
                    PrimitiveComponent* prim = (PrimitiveComponent*)comp;
                    bool simpleShadow = prim->GetType() == ShadowMeshComponent::GetStaticType();

                    if (data.mComponent != nullptr &&
                        comp->IsVisible())
                    {
                        if (simpleShadow)
                        {
                            mSimpleShadowDraws.push_back(data);
                        }
                        else
                        {
                            switch (data.mBlendMode)
                            {
                            case BlendMode::Opaque:
                            case BlendMode::Masked:
                                if (prim->ShouldReceiveSimpleShadows())
                                {
                                    mOpaqueDraws.push_back(data);
                                }
                                else
                                {
                                    mPostShadowOpaqueDraws.push_back(data);
                                }
                                break;
                            case BlendMode::Translucent:
                            case BlendMode::Additive:
                                mTranslucentDraws.push_back(data);
                                break;
                            default:
                                break;
                            }

                            if (prim->ShouldCastShadows())
                            {
                                mShadowDraws.push_back(data);
                            }

                            if (mDebugMode == DEBUG_WIREFRAME)
                            {
                                mWireframeDraws.push_back(data);
                            }
                        }
                    }
                }

#if DEBUG_DRAW_ENABLED
                if (mEnableProxyRendering &&
                    mDebugMode != DEBUG_COLLISION &&
                    comp->IsTransformComponent())
                {
                    TransformComponent* trans = (TransformComponent*)comp;
                    trans->GatherProxyDraws(mDebugDraws);
                }

                if (mDebugMode == DEBUG_COLLISION &&
                    comp->IsPrimitiveComponent())
                {
                    PrimitiveComponent* prim = (PrimitiveComponent*)comp;
                    prim->GatherProxyDraws(mCollisionDraws);
                }
#endif
            }
        }

        CameraComponent* camera = world->GetActiveCamera();

        if (camera)
        {
            glm::vec3 cameraPos = camera->GetAbsolutePosition();

            auto materialSort = [cameraPos](const DrawData& l, const DrawData& r)
            {
                // Depthless materials should render last.
                if (l.mDepthless != r.mDepthless)
                {
                    return r.mDepthless;
                }

                // Sort by blend mode. Render opaque first, then masked.
                if (l.mBlendMode != r.mBlendMode)
                {
                    return l.mBlendMode > r.mBlendMode;
                }

                // Sort by material first (just use address)
                if (l.mMaterial != r.mMaterial)
                {
                    return l.mMaterial < r.mMaterial;
                }

                // Then sort by distance, render closer objects first to get
                // more early depth testing kills.
                float distL = glm::distance2(l.mPosition, cameraPos);
                float distR = glm::distance2(r.mPosition, cameraPos);
                return distL < distR;
            };

            // Sort opaque draw and masked draws by material
            std::sort(mOpaqueDraws.begin(),
                      mOpaqueDraws.end(),
                      materialSort);

            std::sort(mPostShadowOpaqueDraws.begin(),
                      mPostShadowOpaqueDraws.end(),
                      materialSort);

            // Sort translucent draws by distance
            std::sort(mTranslucentDraws.begin(), 
                      mTranslucentDraws.end(),
                      [cameraPos](const DrawData& l, const DrawData& r)
                      {
                          if (l.mDepthless != r.mDepthless)
                          {
                              return r.mDepthless;
                          }

                          if (l.mSortPriority != r.mSortPriority)
                          {
                              return l.mSortPriority < r.mSortPriority;
                          }

                          float distL = glm::distance2(l.mPosition, cameraPos);
                          float distR = glm::distance2(r.mPosition, cameraPos);
                          return distL > distR;
                      });
        }
    }
}

void Renderer::RenderDraws(const std::vector<DrawData>& drawData)
{
    for (uint32_t i = 0; i < drawData.size(); ++i)
    {
        drawData[i].mComponent->Render();
    }
}

void Renderer::RenderDraws(const std::vector<DrawData>& drawData, PipelineId pipelineId)
{
    for (uint32_t i = 0; i < drawData.size(); ++i)
    {
        GFX_BindPipeline(pipelineId, drawData[i].mComponent->GetVertexType());
        drawData[i].mComponent->Render();
    }
}

void Renderer::RenderDebugDraws(const std::vector<DebugDraw>& draws, PipelineId pipelineId)
{
#if DEBUG_DRAW_ENABLED
    for (uint32_t i = 0; i < draws.size(); ++i)
    {
        bool forward = pipelineId == PipelineId::Count;

        if ((!forward && draws[i].mMaterial == nullptr) ||
            (forward && draws[i].mMaterial != nullptr))
        {
            if (!forward)
            {
                GFX_BindPipeline(pipelineId, draws[i].mMesh->HasVertexColor() ? VertexType::VertexColor : VertexType::Vertex);
            }

            GFX_DrawStaticMesh(draws[i].mMesh, draws[i].mMaterial, draws[i].mTransform, draws[i].mColor);
        }
    }
#endif
}

void Renderer::FrustumCull(CameraComponent* camera)
{
    CameraFrustum frustum;
    frustum.SetPosition(camera->GetAbsolutePosition());
    frustum.SetBasis(
        camera->GetForwardVector(),
        camera->GetUpVector(),
        camera->GetRightVector());

    ProjectionMode projMode = camera->GetProjectionMode();
    if (projMode == ProjectionMode::PERSPECTIVE)
    {
        PerspectiveSettings persp = camera->GetPerspectiveSettings();
        frustum.SetPerspective(
            persp.mFovY,
            persp.mAspectRatio,
            persp.mNear,
            persp.mFar);
    }
    else
    {
        OrthoSettings ortho = camera->GetOrthoSettings();
        frustum.SetOrthographic(ortho.mWidth,
            ortho.mHeight,
            ortho.mNear,
            ortho.mFar);
    }

    int32_t drawsCulled = 0;
    drawsCulled += FrustumCullDraws(frustum, mOpaqueDraws);
    drawsCulled += FrustumCullDraws(frustum, mSimpleShadowDraws);
    drawsCulled += FrustumCullDraws(frustum, mPostShadowOpaqueDraws);
    drawsCulled += FrustumCullDraws(frustum, mTranslucentDraws);
    drawsCulled += FrustumCullDraws(frustum, mWireframeDraws);
    //LogDebug("Draws culled: %d", drawsCulled);

#if DEBUG_DRAW_ENABLED
    drawsCulled = 0;
    drawsCulled += FrustumCullDraws(frustum, mDebugDraws);
    drawsCulled += FrustumCullDraws(frustum, mCollisionDraws);
    //LogDebug("DebugDraws culled: %d", drawsCulled);
#endif
}

int32_t Renderer::FrustumCullDraws(const CameraFrustum& frustum, std::vector<DrawData>& drawData)
{
    int32_t drawsCulled = 0;

    // Some code duplication below, but I'm doing this to make sure the branching on ortho doesn't impact performance so much.
    if (frustum.mOrtho)
    {
        for (int32_t i = int32_t(drawData.size()) - 1; i >= 0; --i)
        {
            if (!frustum.IsSphereInFrustumOrtho(drawData[i].mBounds.mCenter, drawData[i].mBounds.mRadius))
            {
                drawData.erase(drawData.begin() + i);
                drawsCulled++;
            }
        }
    }
    else
    {
        for (int32_t i = int32_t(drawData.size()) - 1; i >= 0; --i)
        {
            if (!frustum.IsSphereInFrustum(drawData[i].mBounds.mCenter, drawData[i].mBounds.mRadius))
            {
                drawData.erase(drawData.begin() + i);
                drawsCulled++;
            }
        }
    }

    return drawsCulled;
}

int32_t Renderer::FrustumCullDraws(const CameraFrustum& frustum, std::vector<DebugDraw>& drawData)
{
    int32_t drawsCulled = 0;

    for (int32_t i = int32_t(drawData.size()) - 1; i >= 0; --i)
    {
        Bounds meshBounds = drawData[i].mMesh->GetBounds();
        Bounds worldBounds;
        worldBounds.mCenter = drawData[i].mTransform * glm::vec4(meshBounds.mCenter, 1.0f);

        glm::vec3 absScale = Maths::ExtractScale(drawData[i].mTransform);
        float maxScale = glm::max(glm::max(absScale.x, absScale.y), absScale.z);
        worldBounds.mRadius = maxScale * meshBounds.mRadius;

        bool cull = false;

        if (frustum.mOrtho)
        {
            cull = !frustum.IsSphereInFrustumOrtho(worldBounds.mCenter, worldBounds.mRadius);
        }
        else
        {
            cull = !frustum.IsSphereInFrustum(worldBounds.mCenter, worldBounds.mRadius);
        }

        if (cull)
        {
            drawData.erase(drawData.begin() + i);
            drawsCulled++;
        }
    }

    return drawsCulled;
}

void Renderer::Render(World* world)
{
    if (world == nullptr || 
        GetEngineState()->mConsoleMode ||
        GetEngineState()->mWindowMinimized)
    {
        // Cannot record command buffers yet.
        return;
    }

    {
#if !SYNC_ON_END_FRAME
        SCOPED_CPU_STAT("Vsync");
#endif
        BeginFrame();
    }

    BEGIN_CPU_STAT("Render");

    mScreenIndex = 0;
    GFX_BeginScreen(0);

    for (uint32_t i = 0; i < mWidgets0.size(); ++i)
    {
        mWidgets0[i]->RecursiveUpdate();
    }

    if (mStatsWidget != nullptr && mStatsWidget->IsVisible()) { mStatsWidget->RecursiveUpdate(); }
    if (mConsoleWidget != nullptr && mConsoleWidget->IsVisible()) { mConsoleWidget->RecursiveUpdate(); }

    mInModalWidgetUpdate = true;
    if (mModalWidget != nullptr && mModalWidget->IsVisible()) {  mModalWidget->RecursiveUpdate(); }
    mInModalWidgetUpdate = false;

    CameraComponent* activeCamera = world->GetActiveCamera();

    if (activeCamera != nullptr)
    {
        activeCamera->ComputeMatrices();
    }

    GatherDrawData(world);

    if (mFrustumCulling)
    {
        FrustumCull(activeCamera);
    }

    uint32_t numViews = GFX_GetNumViews();

    for (uint32_t view = 0; view < numViews; ++view)
    {
        GFX_BeginView(view);

        // ***************
        //  Shadow Depths
        // ***************
        GFX_SetViewport(0, 0, SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION);
        GFX_SetScissor(0, 0, SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION);

        GFX_BeginRenderPass(RenderPassId::Shadows);

        DirectionalLightComponent* dirLight = world->GetDirectionalLight();

        if (dirLight && dirLight->ShouldCastShadows())
        {
            RenderDraws(mShadowDraws, PipelineId::Shadow);
        }

        GFX_EndRenderPass();

        GFX_SetViewport(0, 0, mEngineState->mWindowWidth, mEngineState->mWindowHeight);
        GFX_SetScissor(0, 0, mEngineState->mWindowWidth, mEngineState->mWindowHeight);

        // ******************
        //  Forward Pass
        // ******************
        GFX_BeginRenderPass(RenderPassId::Forward);

        if (GetDebugMode() != DEBUG_WIREFRAME)
        {
            RenderDraws(mOpaqueDraws);
            RenderDraws(mSimpleShadowDraws);
            RenderDraws(mPostShadowOpaqueDraws);

            RenderDraws(mTranslucentDraws);

            RenderDebugDraws(mDebugDraws);
        }

        GFX_EndRenderPass();

        // ******************
        //  Post Process
        // ******************

        GFX_BeginRenderPass(RenderPassId::PostProcess);

        // Tonemapping does not look good.
        // Disabling it for now. Also need to totally rewrite postprocessing system.
        // Make it smarter so that it can pingpong between render targets.
        GFX_BindPipeline(/*mDebugMode == DEBUG_NONE ? PipelineId::PostProcess :*/ PipelineId::NullPostProcess);
        GFX_DrawFullscreen();

        RenderDraws(mWireframeDraws, PipelineId::Wireframe);
        RenderDebugDraws(mDebugDraws, PipelineId::Wireframe);

        if (GetDebugMode() == DEBUG_COLLISION)
        {
            RenderDebugDraws(mCollisionDraws, PipelineId::Collision);
        }

        GFX_DrawLines(world->GetLines());

#if EDITOR
        RenderSelectedGeometry(world);
#endif

        GFX_EndRenderPass();

        // ******************
        //  UI
        // ******************
        GFX_BeginRenderPass(RenderPassId::Ui);

        for (uint32_t i = 0; i < mWidgets0.size(); ++i)
        {
            mWidgets0[i]->RecursiveRender();
        }

        if (mStatsWidget != nullptr && mStatsWidget->IsVisible()) { mStatsWidget->RecursiveRender(); }
        if (mConsoleWidget != nullptr && mConsoleWidget->IsVisible()) { mConsoleWidget->RecursiveRender(); }
        if (mModalWidget != nullptr && mModalWidget->IsVisible()) { mModalWidget->RecursiveRender(); }
        GFX_EndRenderPass();
    }

#if SUPPORTS_SECOND_SCREEN
    // Do 3DS bottom screen rendering
    mScreenIndex = 1;
    GFX_BeginScreen(1);

    GFX_SetViewport(0, 0, mEngineState->mSecondWindowWidth, mEngineState->mSecondWindowHeight);
    GFX_SetScissor(0, 0, mEngineState->mSecondWindowWidth, mEngineState->mSecondWindowHeight);

    for (uint32_t i = 0; i < mWidgets1.size(); ++i)
    {
        mWidgets1[i]->RecursiveUpdate();
    }

    for (uint32_t i = 0; i < mWidgets1.size(); ++i)
    {
        mWidgets1[i]->RecursiveRender();
    }

    mScreenIndex = 0;
#endif

    UpdateDebugDraws();

    END_CPU_STAT("Render");

    {
#if SYNC_ON_END_FRAME
        SCOPED_CPU_STAT("Vsync");
#endif
        EndFrame();
    }
}

void Renderer::RenderShadowCasters(World* world)
{
//    if (Renderer::Get()->GetShadowCaster() != nullptr)
//    {
//        for (Actor* actor : mActors)
//        {
//            actor->RenderWithPipeline(Renderer::Get()->GetShadowCaster()->GetPipeline());
//        }
//    }
}

void Renderer::RenderSelectedGeometry(World* world)
{
#if EDITOR
    std::vector<Actor*> selectedActors = world->GetSelectedActors();

    for (uint32_t i = 0; i < selectedActors.size(); ++i)
    {
        selectedActors[i]->RenderSelected();
    }
#endif
}

void Renderer::UpdateDebugDraws()
{
#if DEBUG_DRAW_ENABLED
    float deltaTime = GetAppClock()->DeltaTime();
    for (int32_t i = int32_t(mDebugDraws.size()) - 1; i >= 0; --i)
    {
        mDebugDraws[i].mLife -= deltaTime;

        if (mDebugDraws[i].mLife <= 0.0f)
        {
            mDebugDraws.erase(mDebugDraws.begin() + i);
        }
    }
#endif
}
