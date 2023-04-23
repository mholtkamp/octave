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
#include "Components/ParticleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/ShadowMeshComponent.h"
#include "Log.h"
#include "Line.h"
#include "Maths.h"

#include "Graphics/Graphics.h"
#include "Graphics/GraphicsConstants.h"

#if EDITOR
#include "EditorState.h"
#endif

#include "Assertion.h"
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
    SCOPED_STAT("Renderer::Initialize")

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

void Renderer::GatherProperties(std::vector<Property>& props)
{
    props.push_back(Property(DatumType::Integer, "Rays Per Pixel", nullptr, &mRaysPerPixel));
    props.push_back(Property(DatumType::Integer, "Max Bounces", nullptr, &mMaxBounces));
    props.push_back(Property(DatumType::Bool, "Accumulate", nullptr, &mPathTraceAccumulate));
    props.push_back(Property(DatumType::Integer, "Bake Rays Per Vertex", nullptr, &mBakeRaysPerVertex));
    props.push_back(Property(DatumType::Integer, "Bake Max Bounces", nullptr, &mBakeMaxBounces));
    props.push_back(Property(DatumType::Float, "Bake Shadow Bias", nullptr, &mBakeShadowBias));
    props.push_back(Property(DatumType::Integer, "Bake Indirect Iterations", nullptr, &mBakeIndirectIterations));
    props.push_back(Property(DatumType::Integer, "Bake Direct Diffusals", nullptr, &mBakeDiffuseDirectPasses));
    props.push_back(Property(DatumType::Integer, "Bake Indirect Diffusals", nullptr, &mBakeDiffuseIndirectPasses));
}

TransformComponent* Renderer::ProcessHitCheck(World* world, int32_t x, int32_t y)
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

void Renderer::EnableWorldRendering(bool enable)
{
    mEnableWorldRendering = enable;
}

bool Renderer::IsWorldRenderingEnabled() const
{
    return mEnableWorldRendering;
}

void Renderer::EnablePathTracing(bool enable)
{
    mEnablePathTracing = enable;
}

bool Renderer::IsPathTracingEnabled() const
{
    return mEnablePathTracing;
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

    // Don't add duplicates of the same Widget.
    if (std::find(widgets.begin(), widgets.end(), widget) == widgets.end())
    {
        if (index >= 0 && index <= (int32_t)widgets.size())
        {
            widgets.insert(widgets.begin() + index, widget);
        }
        else
        {
            widgets.push_back(widget);
        }

        widget->MarkDirty();
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

bool Renderer::HasWidget(const Widget* widget, int32_t screenIndex)
{
    bool hasWidget = false;

    if (widget != nullptr)
    {
        if (screenIndex == 0 || screenIndex == -1)
        {
            hasWidget = std::find(mWidgets0.begin(), mWidgets0.end(), widget) != mWidgets0.end();
        }

        if (!hasWidget)
        {
            if (screenIndex == 1 || screenIndex == -1)
            {
                hasWidget = std::find(mWidgets1.begin(), mWidgets1.end(), widget) != mWidgets1.end();
            }
        }

        if (!hasWidget)
        {
            hasWidget = (mModalWidget == widget);
        }
    }

    return hasWidget;
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
    // TEMP
    mRaysPerPixel = 1;
    mMaxBounces = 3;
    mBakeIndirectIterations = 3;
    mBakeShadowBias = 0.01f;
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
            if (!actors[i]->IsVisible())
                continue;

            const std::vector<Component*>& components = actors[i]->GetComponents();

            for (uint32_t c = 0; c < components.size(); ++c)
            {
                Component* comp = components[c];

                if (comp->IsPrimitiveComponent())
                {
                    DrawData data = comp->GetDrawData();
                    data.mComponentType = comp->GetType();

                    PrimitiveComponent* prim = (PrimitiveComponent*)comp;
                    bool simpleShadow = (data.mComponentType == ShadowMeshComponent::GetStaticType());

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
                bool proxyActorEnabled = true;
#if  EDITOR
                if (GetEditorMode() == EditorMode::Blueprint &&
                    comp->GetOwner() != GetEditBlueprintActor())
                {
                    proxyActorEnabled = false;
                }
#endif

                if (mEnableProxyRendering &&
                    mDebugMode != DEBUG_COLLISION &&
                    comp->IsTransformComponent() &&
                    proxyActorEnabled)
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

void Renderer::GatherLightData(World* world)
{
    mLightData.clear();
    const std::vector<LightComponent*>& comps = world->GetLightComponents();

    for (uint32_t i = 0; i < comps.size(); ++i)
    {
        if (comps[i]->IsVisible() && 
            comps[i]->GetLightingDomain() != LightingDomain::Static)
        {
            LightData lightData;
            lightData.mPosition = comps[i]->GetAbsolutePosition();
            lightData.mColor = comps[i]->GetColor();

            RuntimeId id = comps[i]->InstanceRuntimeId();

            if (id == PointLightComponent::ClassRuntimeId())
            {
                PointLightComponent* pointComp = comps[i]->As<PointLightComponent>();
                lightData.mType = LightType::Point;
                lightData.mRadius = pointComp->GetRadius();
                lightData.mDirection = { 0.0f, 0.0f, 0.0f };

            }
            else if (id == DirectionalLightComponent::ClassRuntimeId())
            {
                DirectionalLightComponent* dirComp = comps[i]->As<DirectionalLightComponent>();
                lightData.mType = LightType::Directional;
                lightData.mDirection = dirComp->GetDirection();
                lightData.mRadius = 0.0f;
            }

            mLightData.push_back(lightData);
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

    int32_t lightsCulled = 0;
    if (GFX_ShouldCullLights())
    {
        lightsCulled += FrustumCullLights(frustum, mLightData);
    }
    //LogDebug("Lights culled: %d", lightsCulled);

#if DEBUG_DRAW_ENABLED
    drawsCulled = 0;
    drawsCulled += FrustumCullDraws(frustum, mDebugDraws);
    drawsCulled += FrustumCullDraws(frustum, mCollisionDraws);
    //LogDebug("DebugDraws culled: %d", drawsCulled);
#endif
}

static inline void HandleCullResult(DrawData& drawData, bool inFrustum)
{
    if (drawData.mComponentType == SkeletalMeshComponent::GetStaticType())
    {
        SkeletalMeshComponent* skComp = static_cast<SkeletalMeshComponent*>(drawData.mComponent);

        if (inFrustum)
        {
            skComp->UpdateAnimation(GetEngineState()->mGameDeltaTime, true);
        }
        else
        {
            AnimationUpdateMode animMode = skComp->GetAnimationUpdateMode();
            if (animMode == AnimationUpdateMode::AlwaysUpdateTimeAndBones)
            {
                skComp->UpdateAnimation(GetEngineState()->mGameDeltaTime, true);
            }
            else if (animMode == AnimationUpdateMode::AlwaysUpdateTime)
            {
                skComp->UpdateAnimation(GetEngineState()->mGameDeltaTime, false);
            }
        }
    }
    else if (drawData.mComponentType == ParticleComponent::GetStaticType())
    {
        ParticleComponent* pComp = static_cast<ParticleComponent*>(drawData.mComponent);

        if (inFrustum)
        {
            pComp->Simulate(GetEngineState()->mGameDeltaTime);
            pComp->UpdateVertexBuffer();
        }
        else if (pComp->ShouldAlwaysSimulate())
        {
            pComp->Simulate(GetEngineState()->mGameDeltaTime);
        }
    }
}

int32_t Renderer::FrustumCullDraws(const CameraFrustum& frustum, std::vector<DrawData>& drawData)
{
    int32_t drawsCulled = 0;

    // Some code duplication below, but I'm doing this to make sure the branching on ortho doesn't impact performance so much.
    if (frustum.mOrtho)
    {
        for (int32_t i = int32_t(drawData.size()) - 1; i >= 0; --i)
        {
            bool inFrustum = frustum.IsSphereInFrustumOrtho(drawData[i].mBounds.mCenter, drawData[i].mBounds.mRadius);
            HandleCullResult(drawData[i], inFrustum);

            if (!inFrustum)
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
            bool inFrustum = frustum.IsSphereInFrustum(drawData[i].mBounds.mCenter, drawData[i].mBounds.mRadius);
            HandleCullResult(drawData[i], inFrustum);

            if (!inFrustum)
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

int32_t Renderer::FrustumCullLights(const CameraFrustum& frustum, std::vector<LightData>& lightData)
{
    int32_t lightsCulled = 0;

    // Some code duplication below, but I'm doing this to make sure the branching on ortho doesn't impact performance so much.
    if (frustum.mOrtho)
    {
        for (int32_t i = int32_t(lightData.size()) - 1; i >= 0; --i)
        {
            bool directional = (lightData[i].mType == LightType::Directional);
            bool inFrustum = directional || frustum.IsSphereInFrustumOrtho(lightData[i].mPosition, lightData[i].mRadius);

            if (!inFrustum)
            {
                lightData.erase(lightData.begin() + i);
                lightsCulled++;
            }
        }
    }
    else
    {
        for (int32_t i = int32_t(lightData.size()) - 1; i >= 0; --i)
        {
            bool directional = (lightData[i].mType == LightType::Directional);
            bool inFrustum = directional || frustum.IsSphereInFrustum(lightData[i].mPosition, lightData[i].mRadius);

            if (!inFrustum)
            {
                lightData.erase(lightData.begin() + i);
                lightsCulled++;
            }
        }
    }

    return lightsCulled;
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

    // Update widgets first. (It's possible a widget destroys an actor or loads a level
    // so make sure this happens before gathering/culling objects.)
    {
        SCOPED_FRAME_STAT("Widgets");

        for (uint32_t i = 0; i < mWidgets0.size(); ++i)
        {
            if (mWidgets0[i]->IsVisible())
            {
                mWidgets0[i]->RecursiveUpdate();
            }
        }

        if (mStatsWidget != nullptr && mStatsWidget->IsVisible()) { mStatsWidget->RecursiveUpdate(); }
        if (mConsoleWidget != nullptr && mConsoleWidget->IsVisible()) { mConsoleWidget->RecursiveUpdate(); }

        mInModalWidgetUpdate = true;
        if (mModalWidget != nullptr && mModalWidget->IsVisible()) { mModalWidget->RecursiveUpdate(); }
        mInModalWidgetUpdate = false;

#if SUPPORTS_SECOND_SCREEN
        mScreenIndex = 1;
        for (uint32_t i = 0; i < mWidgets1.size(); ++i)
        {
            if (mWidgets1[i]->IsVisible())
            {
                mWidgets1[i]->RecursiveUpdate();
            }
        }
        mScreenIndex = 0;
#endif
    }

    CameraComponent* activeCamera = world->GetActiveCamera();

    // On 3DS especially, we want to cull before syncing with the GPU
    // otherwise it increases GPU idle time.
    if (mEnableWorldRendering)
    {
        SCOPED_FRAME_STAT("Culling");

        if (activeCamera != nullptr)
        {
            activeCamera->ComputeMatrices();
        }

        GatherDrawData(world);
        GatherLightData(world);

        if (mFrustumCulling)
        {
            FrustumCull(activeCamera);
        }
    }

    {
#if !SYNC_ON_END_FRAME
        SCOPED_FRAME_STAT("Vsync");
#endif
        BeginFrame();
    }

    BEGIN_FRAME_STAT("Render");

    if (GFX_IsLightBakeInProgress())
    {
        GFX_UpdateLightBake();
    }

    GFX_BeginScreen(0);

    uint32_t numViews = GFX_GetNumViews();

    for (uint32_t view = 0; view < numViews; ++view)
    {
        GFX_BeginView(view);

        if (mEnableWorldRendering)
        {
            if (mEnablePathTracing)
            {
                GFX_SetViewport(0, 0, mEngineState->mWindowWidth, mEngineState->mWindowHeight);
                GFX_SetScissor(0, 0, mEngineState->mWindowWidth, mEngineState->mWindowHeight);
                GFX_BeginRenderPass(RenderPassId::Forward);
                GFX_EndRenderPass();

                GFX_PathTrace();
            }
            else
            {
                // ***************
                //  Shadow Depths
                // ***************
                GFX_SetViewport(0, 0, SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION);
                GFX_SetScissor(0, 0, SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION);

                GFX_BeginRenderPass(RenderPassId::Shadows);

                // TODO: Reimplement shadow maps. Possibly for multiple light sources.
#if 0
                DirectionalLightComponent* dirLight = world->GetDirectionalLight();

                if (dirLight && dirLight->ShouldCastShadows())
                {
                    RenderDraws(mShadowDraws, PipelineId::Shadow);
                }
#endif

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
            }

            // ******************
            //  Post Process
            // ******************

            GFX_BeginRenderPass(RenderPassId::PostProcess);

            // Tonemapping does not look good.
            // Disabling it for now. Also need to totally rewrite postprocessing system.
            // Make it smarter so that it can pingpong between render targets.
            GFX_BindPipeline(PipelineId::PostProcess /*mDebugMode == DEBUG_NONE ? PipelineId::PostProcess : PipelineId::NullPostProcess*/);
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
        }
        else
        {
            GFX_SetViewport(0, 0, mEngineState->mWindowWidth, mEngineState->mWindowHeight);
            GFX_SetScissor(0, 0, mEngineState->mWindowWidth, mEngineState->mWindowHeight);
            GFX_BeginRenderPass(RenderPassId::Clear);
            GFX_EndRenderPass();
        }

        // ******************
        //  UI
        // ******************
        GFX_BeginRenderPass(RenderPassId::Ui);
        Widget::ResetScissor();

        for (uint32_t i = 0; i < mWidgets0.size(); ++i)
        {
            if (mWidgets0[i]->IsVisible())
            {
                mWidgets0[i]->RecursiveRender();
            }
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

    Widget::ResetScissor();

    for (uint32_t i = 0; i < mWidgets1.size(); ++i)
    {
        if (mWidgets1[i]->IsVisible())
        {
            mWidgets1[i]->RecursiveRender();
        }
    }

    mScreenIndex = 0;
#endif

    UpdateDebugDraws();

    END_FRAME_STAT("Render");

    {
#if SYNC_ON_END_FRAME
        SCOPED_FRAME_STAT("Vsync");
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
    // Rendering selected geometry while playing looks bad,
    // so just skip rendering selected unless we find a good use-case.
    if (!GetEditorState()->mPlayInEditor ||
        GetEditorState()->mEjected)
    {
        std::vector<Actor*> selectedActors = world->GetSelectedActors();

        for (uint32_t i = 0; i < selectedActors.size(); ++i)
        {
            selectedActors[i]->RenderSelected();
        }
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

const std::vector<LightData>& Renderer::GetLightData() const
{
    return mLightData;
}

void Renderer::BeginLightBake()
{
    GFX_BeginLightBake();
}

void Renderer::EndLightBake()
{
    GFX_EndLightBake();
}

bool Renderer::IsLightBakeInProgress() const
{
    return GFX_IsLightBakeInProgress();
}

float Renderer::GetLightBakeProgress() const
{
    return GFX_GetLightBakeProgress();
}

uint32_t Renderer::GetRaysPerPixel() const
{
    return mRaysPerPixel;
}

uint32_t Renderer::GetMaxBounces() const
{
    return mMaxBounces;
}

bool Renderer::IsPathTraceAccumulationEnabled() const
{
    return mPathTraceAccumulate;
}

uint32_t Renderer::GetBakeRaysPerVertex() const
{
    return mBakeRaysPerVertex;
}

uint32_t Renderer::GetBakeMaxBounces() const
{
    return mBakeMaxBounces;
}

float Renderer::GetBakeShadowBias() const
{
    return mBakeShadowBias;
}

uint32_t Renderer::GetBakeIndirectIterations() const
{
    return mBakeIndirectIterations;
}

uint32_t Renderer::GetBakeDiffuseDirectPasses() const
{
    return mBakeDiffuseDirectPasses;
}

uint32_t Renderer::GetBakeDiffuseIndirectPasses() const
{
    return mBakeDiffuseIndirectPasses;
}

