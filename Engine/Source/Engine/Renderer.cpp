#include "Renderer.h"
#include "AssetManager.h"
#include "Utilities.h"
#include "Engine.h"
#include "Profiler.h"
#include "Constants.h"
#include "Nodes/Widgets/Widget.h"
#include "Nodes/Widgets/Console.h"
#include "Nodes/Widgets/StatsOverlay.h"
#include "Assets/Font.h"
#include "Nodes/3D/PointLight3d.h"
#include "Nodes/3D/Primitive3d.h"
#include "Nodes/3D/Particle3d.h"
#include "Nodes/3D/SkeletalMesh3d.h"
#include "Nodes/3D/ShadowMesh3d.h"
#include "Log.h"
#include "Line.h"
#include "Maths.h"
#include "InputDevices.h"

#include "Graphics/Graphics.h"
#include "Graphics/GraphicsConstants.h"

#if EDITOR
#include "EditorState.h"
#include "Viewport2d.h"
#include "PaintManager.h"
#endif

// TEMPORARY!
#if API_VULKAN
#include "Graphics/Vulkan/VulkanContext.h"
#include "Graphics/Vulkan/VulkanUtils.h"
#include "Graphics/Vulkan/PostProcessChain.h"
#endif

#include "Assertion.h"
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <set>
#include <fstream>
#include <algorithm>
#include <malloc.h>

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
        Node::Destruct(mConsoleWidget);
        mConsoleWidget = nullptr;
    }

    if (mStatsWidget != nullptr)
    {
        Node::Destruct(mStatsWidget);
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
    mConsoleWidget = Node::Construct<Console>();
#endif

    mStatsWidget = Node::Construct<StatsOverlay>();

#if (PLATFORM_WINDOWS || PLATFORM_LINUX || PLATFORM_ANDROID) && !_DEBUG
    if (mConsoleWidget != nullptr)
    {
        mConsoleWidget->SetVisible(false);
    }
#endif

    if (mStatsWidget != nullptr)
    {
        mStatsWidget->SetVisible(false);
    }

#if EDITOR
    EnableProxyRendering(true);
#endif
}

void Renderer::GatherProperties(std::vector<Property>& props)
{
    props.push_back(Property(DatumType::Float, "Resolution Scale", nullptr, &(GetEngineState()->mGraphics.mResolutionScale)));

#if API_VULKAN
    GetVulkanContext()->GetPostProcessChain()->GatherProperties(props);
#endif

    props.push_back(Property(DatumType::Bool, "Light Fade", nullptr, &mEnableLightFade));
    props.push_back(Property(DatumType::Integer, "Light Fade Limit", nullptr, &mLightFadeLimit));
    props.push_back(Property(DatumType::Float, "Light Fade Speed", nullptr, &mLightFadeSpeed));

    props.push_back(Property(DatumType::Integer, "Rays Per Pixel", nullptr, &mRaysPerPixel));
    props.push_back(Property(DatumType::Integer, "Max Bounces", nullptr, &mMaxBounces));
    props.push_back(Property(DatumType::Bool, "Accumulate", nullptr, &mPathTraceAccumulate));
    props.push_back(Property(DatumType::Integer, "Bake Rays Per Vertex", nullptr, &mBakeRaysPerVertex));
    props.push_back(Property(DatumType::Integer, "Bake Max Bounces", nullptr, &mBakeMaxBounces));
    props.push_back(Property(DatumType::Float, "Bake Shadow Bias", nullptr, &mBakeShadowBias));
    props.push_back(Property(DatumType::Integer, "Bake Indirect Iterations", nullptr, &mBakeIndirectIterations));
    props.push_back(Property(DatumType::Integer, "Bake Direct Diffusals", nullptr, &mBakeDiffuseDirectPasses));
    props.push_back(Property(DatumType::Integer, "Bake Indirect Diffusals", nullptr, &mBakeDiffuseIndirectPasses));
    props.push_back(Property(DatumType::Color, "Sky Zenith Color", nullptr, &mSkyZenithColor));
    props.push_back(Property(DatumType::Color, "Sky Horizon Color", nullptr, &mSkyHorizonColor));
    props.push_back(Property(DatumType::Color, "Ground Color", nullptr, &mGroundColor));

}

Node3D* Renderer::ProcessHitCheck(World* world, int32_t x, int32_t y, uint32_t* outInstance)
{
    return GFX_ProcessHitCheck(world, x, y, outInstance);
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

void Renderer::Enable3dRendering(bool enable)
{
    mEnable3dRendering = enable;
}

bool Renderer::Is3dRenderingEnabled() const
{
    return mEnable3dRendering;
}

void Renderer::Enable2dRendering(bool enable)
{
    mEnable2dRendering = enable;
}

bool Renderer::Is2dRenderingEnabled() const
{
    return mEnable2dRendering;
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

MaterialLite* Renderer::GetDefaultMaterial()
{
    return mDefaultMaterial.Get<MaterialLite>();
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

bool Renderer::IsRenderingFirstScreen() const
{
    return (mScreenIndex == 0);
}

bool Renderer::IsRenderingLastScreen() const
{
#if SUPPORTS_SECOND_SCREEN
    return mScreenIndex == 1;
#else
    return mScreenIndex == 0;
#endif
}

World* Renderer::GetCurrentWorld()
{
    return mCurrentWorld;
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
        res.x = (float)GetViewportWidth();
        res.y = (float)GetViewportHeight();
    }
    else if (screen == 1)
    {
        // TODO-NODE: Handle second screen / multiple viewports.
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
    GFX_SetFog(mCurrentWorld->GetFogSettings());
}

void Renderer::EndFrame()
{
    GFX_EndFrame();

    if (IsRenderingLastScreen())
    {
        mFrameNumber++;
        mFrameIndex = mFrameNumber % MAX_FRAMES;
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

void Renderer::EnableConsole(bool enable)
{
    if (mConsoleWidget)
    {
        mConsoleWidget->SetVisible(enable);
    }
}

bool Renderer::IsConsoleEnabled()
{
    return mConsoleWidget && mConsoleWidget->IsVisible();
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
    if (mModalWidget != nullptr)
        mModalWidget->MarkDirty();

    if (mConsoleWidget != nullptr)
        mConsoleWidget->MarkDirty();

    if (mStatsWidget != nullptr)
        mStatsWidget->MarkDirty();

    // TODO: Iterate over all worlds if we add multiple worlds
    for (int32_t i = 0; i < GetNumWorlds(); ++i)
    {
        GetWorld(i)->DirtyAllWidgets();
    }
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

void Renderer::RemoveDebugDrawsForNode(Node* node)
{
#if DEBUG_DRAW_ENABLED
    for (int32_t i = int32_t(mDebugDraws.size()) - 1; i >= 0 ; --i)
    {
        if (mDebugDraws[i].mNode == node)
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
    bool enable3D = mEnable3dRendering;
    bool enable2D = mEnable2dRendering;
    bool onlySelected = false;

#if EDITOR
    if (GetEditorState()->GetEditorMode() == EditorMode::Scene2D)
    {
        enable3D = false;
    }
    else if (GetEditorState()->GetEditorMode() == EditorMode::Scene3D)
    {
        enable2D = false;

        if (GetEditorState()->GetPaintMode() != PaintMode::None)
        {
            onlySelected = GetEditorState()->GetSelectedNodes().size() > 0 &&
                !IsControlDown() &&
                GetEditorState()->mPaintManager->mOnlyRenderSelected;
        }
    }
#endif

    mShadowDraws.clear();
    mOpaqueDraws.clear();
    mSimpleShadowDraws.clear();
    mPostShadowOpaqueDraws.clear();
    mTranslucentDraws.clear();
    mWireframeDraws.clear();
    mCollisionDraws.clear();
    mWidgetDraws.clear();

    Camera3D* camera = world ? world->GetActiveCamera() : nullptr;

    if (world != nullptr &&
        camera != nullptr)
    {
        glm::vec3 cameraPos = camera->GetWorldPosition();

        auto gatherDrawData = [&](Node* node) -> bool
        {
            if (!node->IsVisible())
            {
                // If this node is not visible, then return false so we don't
                // traverse farther down the tree. Might have to add a new variable
                // on Node if we want to tread child visibility independent of the parent.
                return false;
            }

#if EDITOR
            if (onlySelected &&
                !GetEditorState()->IsNodeSelected(node))
            {
                // Return true since a child may be selected
                return true;
            }
#endif

            if (enable3D && node->IsPrimitive3D())
            {
                DrawData data = node->GetDrawData();
                data.mNodeType = node->GetType();

                Primitive3D* prim = (Primitive3D*)node;
                bool simpleShadow = (data.mNodeType == ShadowMesh3D::GetStaticType());

                bool distanceCulled = false;
                data.mDistance2 = glm::distance2(cameraPos, data.mBounds.mCenter);
                const float cullDist = prim ? prim->GetCullDistance() : 0.0f;
                if (cullDist > 0.0f)
                {
                    const float cullDist2 = cullDist * cullDist;
                    if (data.mDistance2 > cullDist2)
                    {
                        distanceCulled = true;
                    }
                }

                if (data.mNode != nullptr &&
                    !distanceCulled)
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
            else if (enable2D && node->IsWidget())
            {
                DrawData data = node->GetDrawData();
                data.mNodeType = node->GetType();

                if (data.mNode != nullptr)
                {
                    mWidgetDraws.push_back(data);
                }
            }

#if DEBUG_DRAW_ENABLED
            bool proxyActorEnabled = true;

            if (mEnableProxyRendering &&
                mDebugMode != DEBUG_COLLISION &&
                node->IsNode3D() &&
                proxyActorEnabled)
            {
                Node3D* node3d = (Node3D*)node;
                node3d->GatherProxyDraws(mDebugDraws);
            }

            if (mDebugMode == DEBUG_COLLISION &&
                node->IsPrimitive3D())
            {
                Primitive3D* prim = (Primitive3D*)node;
                prim->GatherProxyDraws(mCollisionDraws);
            }
#endif

            return true;
        };

        if (world != nullptr)
        {
            if (world->GetRootNode() != nullptr)
            {
                world->GetRootNode()->Traverse(gatherDrawData);
            }

            // Need to render these widgets even if in 3D mode.
            enable2D = true;

            if (mStatsWidget != nullptr && mStatsWidget->IsVisible()) { mStatsWidget->Traverse(gatherDrawData); }
            if (mConsoleWidget != nullptr && mConsoleWidget->IsVisible()) { mConsoleWidget->Traverse(gatherDrawData); }
            if (mModalWidget != nullptr && mModalWidget->IsVisible()) { mModalWidget->Traverse(gatherDrawData); }

#if EDITOR
            // Kinda hacky but doing this to draw overlay text when in editor.
            if (GetEditorState()->mOverlayText)
            {
                GetEditorState()->mOverlayText->Traverse(gatherDrawData);
            }
#endif
        }

        auto materialSort = [](const DrawData& l, const DrawData& r)
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
            return l.mDistance2 < r.mDistance2;
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

static void SetLightData(LightData& lightData, Light3D* comp)
{
    lightData.mDomain = comp->GetLightingDomain();
    lightData.mLightingChannels = comp->GetLightingChannels();
    lightData.mPosition = comp->GetWorldPosition();
    lightData.mColor = comp->GetColor();
    lightData.mIntensity = glm::max(comp->GetIntensity(), 0.0f);

    RuntimeId id = comp->InstanceRuntimeId();

    if (id == PointLight3D::ClassRuntimeId())
    {
        PointLight3D* pointComp = comp->As<PointLight3D>();
        lightData.mType = LightType::Point;
        lightData.mRadius = pointComp->GetRadius();
        lightData.mDirection = { 0.0f, 0.0f, 0.0f };

    }
    else if (id == DirectionalLight3D::ClassRuntimeId())
    {
        DirectionalLight3D* dirComp = comp->As<DirectionalLight3D>();
        lightData.mType = LightType::Directional;
        lightData.mDirection = dirComp->GetDirection();
        lightData.mRadius = 0.0f;
    }
}

void Renderer::GatherLightData(World* world)
{
    static std::vector<LightDistance2> sClosestLights;
    sClosestLights.clear();

    mLightData.clear();
    const std::vector<Light3D*>& lights = world->GetLights();

    if (mEnableLightFade)
    {
        float deltaTime = GetEngineState()->mGameDeltaTime;
        uint32_t lightLimit = glm::min<uint32_t>(mLightFadeLimit, MAX_LIGHTS_PER_DRAW);
        glm::vec3 camPos = world->GetActiveCamera()->GetWorldPosition();

        // Step 1 - Determine the closest N lights
        for (uint32_t i = 0; i < lights.size(); ++i)
        {
            if (!lights[i]->IsVisible()
#if !EDITOR
                || lights[i]->GetLightingDomain() == LightingDomain::Static
#endif
                )
            {
                continue;
            }

            glm::vec3 lightPos = lights[i]->GetWorldPosition();
            bool directional = lights[i]->IsDirectionalLight3D();

            float dist2 = directional ? 0.0f : glm::distance2(lightPos, camPos);

            if (sClosestLights.size() < lightLimit)
            {
                sClosestLights.push_back({ lights[i], dist2 });
            }
            else
            {
                // We need to evict the farthest light (if it is farther than this cur light)
                int32_t farthestIdx = -1;
                float farthestDist = 0.0f;

                for (uint32_t j = 0; j < sClosestLights.size(); ++j)
                {
                    if (sClosestLights[j].mDistance2 > dist2 &&
                        sClosestLights[j].mDistance2 > farthestDist)
                    {
                        farthestIdx = (int32_t)j;
                        farthestDist = sClosestLights[j].mDistance2;
                    }
                }

                // We found a light to evict.
                if (farthestIdx >= 0)
                {
                    sClosestLights[farthestIdx] = { lights[i], dist2 };
                }
            }
        }

        // Step 2 - If there is space, add the closest lights to the fading light list (if not already in it).
        std::sort(sClosestLights.begin(),
            sClosestLights.end(),
            [](const LightDistance2& l, const LightDistance2& r)
            {
                return l.mDistance2 < r.mDistance2;
            });

        for (uint32_t i = 0; i < sClosestLights.size(); ++i)
        {
            if (mFadingLights.size() < lightLimit)
            {
                bool alreadyFading = false;
                for (uint32_t j = 0; j < mFadingLights.size(); ++j)
                {
                    if (mFadingLights[j].mComponent == sClosestLights[i].mComponent)
                    {
                        alreadyFading = true;
                        break;
                    }
                }

                if (!alreadyFading)
                {
                    mFadingLights.push_back(FadingLight(sClosestLights[i].mComponent));
                }
            }
            else
            {
                // FadingLights vector is full.
                break;
            }
        }

        for (int32_t i = int32_t(mFadingLights.size()) - 1; i >= 0; --i)
        {
            // Step 3 - Determine which of the persistent N lights need to be faded out.
            bool active = false;
            FadingLight& fadingLight = mFadingLights[i];

            for (uint32_t j = 0; j < sClosestLights.size(); ++j)
            {
                if (fadingLight.mComponent == sClosestLights[j].mComponent)
                {
                    Light3D* light = sClosestLights[j].mComponent;

                    // Ok, this light is still in the closest N lights.
                    active = true;
                    SetLightData(fadingLight.mData, light);
                    fadingLight.mColor = fadingLight.mData.mColor;
                    break;
                }
            }

            // Step 4 - Fade in/out lights. Adjust alpha and update light color based on alpha.
            if (active)
            {
                fadingLight.mAlpha += mLightFadeSpeed * deltaTime;
                fadingLight.mAlpha = glm::min(fadingLight.mAlpha, 1.0f);
            }
            else
            {
                fadingLight.mAlpha -= mLightFadeSpeed * deltaTime;
                fadingLight.mAlpha = glm::max(fadingLight.mAlpha, 0.0f);
            }

            fadingLight.mData.mColor = glm::mix(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), fadingLight.mColor, fadingLight.mAlpha);

            // Step 5 - Copy persistent light data to mLightData if alpha > 0, otherwise remove it from fading light vector
            if (fadingLight.mAlpha <= 0.0f)
            {
                mFadingLights.erase(mFadingLights.begin() + i);
            }
            else
            {
                mLightData.push_back(fadingLight.mData);
            }
        }
    }
    else
    {
        for (uint32_t i = 0; i < lights.size(); ++i)
        {
            if (lights[i]->IsVisible()
#if !EDITOR
                && lights[i]->GetLightingDomain() != LightingDomain::Static
#endif
                )
            {
                LightData lightData;
                SetLightData(lightData, lights[i]);
                mLightData.push_back(lightData);
            }
        }
    }

#if EDITOR
    // If preview lighting is enabled, and there is no directional light in the scene, add a preview dir light.
    if (GetEditorState()->mPreviewLighting && !IsPlayingInEditor())
    {
        bool hasDirLight = false;
        for (uint32_t i = 0; i < mLightData.size(); ++i)
        {
            if (mLightData[i].mType == LightType::Directional)
            {
                hasDirLight = true;
                break;
            }
        }

        if (!hasDirLight)
        {
            LightData previewLight;
            previewLight.mColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
            previewLight.mDirection = glm::vec3(0.57735f, -0.57735, -0.57735);
            previewLight.mDomain = LightingDomain::Dynamic;
            previewLight.mLightingChannels = 0x01;
            previewLight.mPosition = glm::vec3(0.0f, 0.0f, 0.0f);
            previewLight.mRadius = 0.0f;
            previewLight.mIntensity = 1.0f;
            previewLight.mType = LightType::Directional;
            mLightData.push_back(previewLight);
        }
    }
#endif
}

void Renderer::RenderDraws(const std::vector<DrawData>& drawData)
{
    for (uint32_t i = 0; i < drawData.size(); ++i)
    {
        drawData[i].mNode->Render();
    }
}

void Renderer::RenderDraws(const std::vector<DrawData>& drawData, PipelineConfig pipelineConfig)
{
    for (uint32_t i = 0; i < drawData.size(); ++i)
    {
        GFX_SetPipelineState(pipelineConfig);
        drawData[i].mNode->Render();
    }
}

void Renderer::RenderDebugDraws(const std::vector<DebugDraw>& draws, PipelineConfig pipelineConfig)
{
#if DEBUG_DRAW_ENABLED
    for (uint32_t i = 0; i < draws.size(); ++i)
    {
        bool drawMaterials = (pipelineConfig == PipelineConfig::Count);

        if ((!drawMaterials && draws[i].mMaterial == nullptr) ||
            (drawMaterials && draws[i].mMaterial != nullptr))
        {
            if (!drawMaterials)
            {
                GFX_SetPipelineState(pipelineConfig);
            }

            GFX_DrawStaticMesh(draws[i].mMesh, draws[i].mMaterial, draws[i].mTransform, draws[i].mColor);
        }
    }
#endif
}

void Renderer::FrustumCull(Camera3D* camera)
{
    if (camera == nullptr)
        return;

    CameraFrustum frustum;
    frustum.SetPosition(camera->GetWorldPosition());
    frustum.SetBasis(
        camera->GetForwardVector(),
        camera->GetUpVector(),
        camera->GetRightVector());

    float nearZ = camera->GetNearZ();
    float farZ = camera->GetFarZ();

    ProjectionMode projMode = camera->GetProjectionMode();
    if (projMode == ProjectionMode::PERSPECTIVE)
    {
        float fovY = camera->GetFieldOfViewY();
        float aspectRatio = camera->GetAspectRatio();

        frustum.SetPerspective(
            fovY,
            aspectRatio,
            nearZ,
            farZ);
    }
    else
    {
        float orthoWidth = camera->GetOrthoWidth();
        float orthoHeight = camera->GetOrthoHeight();

        frustum.SetOrthographic(orthoWidth,
            orthoHeight,
            nearZ,
            farZ);
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
    if (drawData.mNodeType == SkeletalMesh3D::GetStaticType())
    {
        SkeletalMesh3D* skNode = static_cast<SkeletalMesh3D*>(drawData.mNode);

        if (inFrustum)
        {
            skNode->UpdateAnimation(GetEngineState()->mGameDeltaTime, true);
        }
        else
        {
            AnimationUpdateMode animMode = skNode->GetAnimationUpdateMode();
            if (animMode == AnimationUpdateMode::AlwaysUpdateTimeAndBones)
            {
                skNode->UpdateAnimation(GetEngineState()->mGameDeltaTime, true);
            }
            else if (animMode == AnimationUpdateMode::AlwaysUpdateTime)
            {
                skNode->UpdateAnimation(GetEngineState()->mGameDeltaTime, false);
            }
        }
    }
    else if (drawData.mNodeType == Particle3D::GetStaticType())
    {
        Particle3D* pNode = static_cast<Particle3D*>(drawData.mNode);

        if (inFrustum)
        {
            pNode->Simulate(GetEngineState()->mGameDeltaTime);
            pNode->UpdateVertexBuffer();
        }
        else if (pNode->ShouldAlwaysSimulate())
        {
            pNode->Simulate(GetEngineState()->mGameDeltaTime);
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

void Renderer::Render(World* world, int32_t screenIndex)
{
    if (world == nullptr ||
        GetEngineState()->mConsoleMode)
    {
        // Cannot record command buffers yet.
        return;
    }

    mCurrentWorld = world;
    mScreenIndex = screenIndex;

    bool inGame = IsGameTickEnabled();
    float gameDeltaTime = GetEngineState()->mGameDeltaTime;
    float realDeltaTime = GetEngineState()->mRealDeltaTime;
    bool enable3D = mEnable3dRendering;
    bool enable2D = mEnable2dRendering;

#if EDITOR
    if (GetEditorState()->GetEditorMode() == EditorMode::Scene2D)
    {
        enable3D = false;
    }
#endif

    // Update Renderer's overlay widgets (not widgets in the world.)
    if (mScreenIndex == 0)
    {
        SCOPED_FRAME_STAT("Overlay");

        if (mStatsWidget != nullptr && mStatsWidget->IsVisible()) { mStatsWidget->RecursiveTick(gameDeltaTime, inGame); }
        if (mConsoleWidget != nullptr && mConsoleWidget->IsVisible()) { mConsoleWidget->RecursiveTick(gameDeltaTime, inGame); }

        mInModalWidgetUpdate = true;
        if (mModalWidget != nullptr && mModalWidget->IsVisible()) { mModalWidget->RecursiveTick(gameDeltaTime, inGame); }
        mInModalWidgetUpdate = false;
    }

    Camera3D* activeCamera = world->GetActiveCamera();

    // On 3DS especially, we want to cull before syncing with the GPU
    // otherwise it increases GPU idle time.
    {
        SCOPED_FRAME_STAT("Culling");

        GatherDrawData(world);

        if (enable3D)
        {
            if (activeCamera != nullptr)
            {
                activeCamera->ComputeMatrices();
            }

            GatherLightData(world);

            if (mFrustumCulling)
            {
                FrustumCull(activeCamera);
            }
        }
    }

    // Still update UI and cull when minimized (to update animation and particle simulation)
    if (!GetEngineState()->mWindowMinimized)
    {
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

        GFX_BeginScreen(mScreenIndex);

        uint32_t numViews = GFX_GetNumViews();

        for (uint32_t view = 0; view < numViews; ++view)
        {
            GFX_BeginView(view);

            uint32_t windowWidth = (mScreenIndex == 0) ? GetEngineState()->mWindowWidth : GetEngineState()->mSecondWindowWidth;
            uint32_t windowHeight = (mScreenIndex == 0) ? GetEngineState()->mWindowHeight : GetEngineState()->mSecondWindowHeight;

            glm::uvec4 vp = GetViewport();
            glm::uvec4 svp = GetSceneViewport();

            // The scene viewport is adjusted for render resolution. Matches scene color image / depth image.
            uint32_t sceneViewportX = svp.x;
            uint32_t sceneViewportY = svp.y;
            uint32_t sceneViewportWidth = svp.z;
            uint32_t sceneViewportHeight = svp.w;

            // The regular viewport matches the swapchain image. Used in UI rendering.
            uint32_t viewportX = vp.x;
            uint32_t viewportY = vp.y;
            uint32_t viewportWidth = vp.z;
            uint32_t viewportHeight = vp.w;

            if (enable3D && activeCamera != nullptr)
            {
                if (mEnablePathTracing)
                {
                    GFX_SetViewport(sceneViewportX, sceneViewportY, sceneViewportWidth, sceneViewportHeight);
                    GFX_SetScissor(sceneViewportX, sceneViewportY, sceneViewportWidth, sceneViewportHeight);
                    GFX_BeginRenderPass(RenderPassId::Forward);
                    GFX_EndRenderPass();

                    GFX_PathTrace();
                }
                else
                {
                    // ***************
                    //  Shadow Depths
                    // ***************
                    // TODO: Reimplement shadow maps. Possibly for multiple light sources.
#if 0
                    GFX_SetViewport(0, 0, SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION, false);
                    GFX_SetScissor(0, 0, SHADOW_MAP_RESOLUTION, SHADOW_MAP_RESOLUTION, false);

                    GFX_BeginRenderPass(RenderPassId::Shadows);

                    DirectionalLight3D* dirLight = world->GetDirectionalLight();

                    if (dirLight && dirLight->ShouldCastShadows())
                    {
                        RenderDraws(mShadowDraws, PipelineConfig::Shadow);
                    }

                    GFX_EndRenderPass();
#endif

                    GFX_SetViewport(sceneViewportX, sceneViewportY, sceneViewportWidth, sceneViewportHeight);
                    GFX_SetScissor(sceneViewportX, sceneViewportY, sceneViewportWidth, sceneViewportHeight);

                    // ******************
                    //  Forward Pass
                    // ******************
                    GFX_BeginRenderPass(RenderPassId::Forward);

                    if (GetDebugMode() != DEBUG_WIREFRAME)
                    {
                        GFX_EnableMaterials(true);

                        GFX_SetPipelineState(PipelineConfig::Forward);
                        RenderDraws(mOpaqueDraws);
                        RenderDraws(mSimpleShadowDraws);
                        // Reset state that was altered by shadow mesh draws.
                        GFX_SetPipelineState(PipelineConfig::Forward);
                        RenderDraws(mPostShadowOpaqueDraws);

                        RenderDraws(mTranslucentDraws);

                        RenderDebugDraws(mDebugDraws);

                        GFX_EnableMaterials(false);
                    }

                    RenderDraws(mWireframeDraws, PipelineConfig::Wireframe);
                    RenderDebugDraws(mDebugDraws, PipelineConfig::Wireframe);

                    if (GetDebugMode() == DEBUG_COLLISION)
                    {
                        RenderDebugDraws(mCollisionDraws, PipelineConfig::Collision);
                    }

                    GFX_DrawLines(world->GetLines());

                    GFX_EndRenderPass();
                }

                // ******************
                //  Post Process
                // ******************

                GFX_SetViewport(0, 0, windowWidth, windowHeight);
                GFX_SetScissor(0, 0, windowWidth, windowHeight);

                GFX_RenderPostProcessPasses();

#if EDITOR
                GFX_BeginRenderPass(RenderPassId::Selected);

                GFX_SetViewport(viewportX, viewportY, viewportWidth, viewportHeight);
                GFX_SetScissor(viewportX, viewportY, viewportWidth, viewportHeight);

                RenderSelectedGeometry(world);

                GFX_EndRenderPass();
#endif
                GFX_SetViewport(sceneViewportX, sceneViewportY, sceneViewportWidth, sceneViewportHeight);
                GFX_SetScissor(sceneViewportX, sceneViewportY, sceneViewportWidth, sceneViewportHeight);
            }
            else
            {
                GFX_SetViewport(sceneViewportX, sceneViewportY, sceneViewportWidth, sceneViewportHeight);
                GFX_SetScissor(sceneViewportX, sceneViewportY, sceneViewportWidth, sceneViewportHeight);
                GFX_BeginRenderPass(RenderPassId::Clear);
                GFX_EndRenderPass();
            }

            // ******************
            //  UI
            // ******************
            GFX_SetViewport(viewportX, viewportY, viewportWidth, viewportHeight);
            GFX_BeginRenderPass(RenderPassId::Ui);
            RenderDraws(mWidgetDraws);
            GFX_EndRenderPass();
        }

        END_FRAME_STAT("Render");

        {
#if SYNC_ON_END_FRAME
            SCOPED_FRAME_STAT("Vsync");
#endif
            EndFrame();
        }
    }

    UpdateDebugDraws();

    mCurrentWorld = nullptr;
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
    // Don't render selected geometry when in paint mode
    // We still want to make editing selection easy, so if Ctrl is down
    // then render selected geometry still.
    if (GetEditorState()->GetPaintMode() != PaintMode::None && 
        !IsControlDown())
        return;

    // Rendering selected geometry while playing looks bad,
    // so just skip rendering selected unless we find a good use-case.
    if (!GetEditorState()->mPlayInEditor ||
        GetEditorState()->mEjected)
    {
        std::vector<Node*> selectedNodes = GetEditorState()->GetSelectedNodes();

        for (uint32_t i = 0; i < selectedNodes.size(); ++i)
        {
            if (selectedNodes[i]->GetWorld() == world)
            {
                const bool renderChildren = selectedNodes[i]->IsSceneLinked();
                selectedNodes[i]->RenderSelected(renderChildren);
            }
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

bool Renderer::IsLightFadeEnabled() const
{
    return mEnableLightFade;
}

void Renderer::EnableLightFade(bool enable)
{
    mEnableLightFade = enable;
}

void Renderer::SetLightFadeLimit(uint32_t limit)
{
    mLightFadeLimit = limit;
}

uint32_t Renderer::GetLightFadeLimit() const
{
    return mLightFadeLimit;
}

void Renderer::SetLightFadeSpeed(float speed)
{
    mLightFadeSpeed = speed;
}

float Renderer::GetLightFadeSpeed() const
{
    return mLightFadeSpeed;
}

bool Renderer::IsPostProcessPassEnabled(PostProcessPassId passId) const
{
    bool enabled = false;
    uint32_t iPass = uint32_t(passId);

    // Required post process passes.
    if (passId == PostProcessPassId::Tonemap)
        return true;

    if (iPass >= 0 &&
        iPass < uint32_t(PostProcessPassId::Count))
    {
        enabled = mPostProcessEnables[iPass];
    }

    return enabled;
}

void Renderer::EnablePostProcessPass(PostProcessPassId passId, bool enable)
{
    bool enabled = false;
    uint32_t iPass = uint32_t(passId);

    if (iPass >= 0 &&
        iPass < uint32_t(PostProcessPassId::Count))
    {
        mPostProcessEnables[iPass] = enable;
    }
}

void Renderer::SetResolutionScale(float scale)
{
    scale = glm::clamp(scale, 0.1f, 2.0f);
    GetEngineState()->mGraphics.mResolutionScale = scale;
}

float Renderer::GetResolutionScale() const
{
    return GetEngineState()->mGraphics.mResolutionScale;
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

glm::vec4 Renderer::GetSkyZenithColor() const
{
    return mSkyZenithColor;
}

glm::vec4 Renderer::GetSkyHorizonColor() const
{
    return mSkyHorizonColor;
}

glm::vec4 Renderer::GetGroundColor() const
{
    return mGroundColor;
}

// TODO: Might have to adjust these to handle 3DS (two screens) and split-screen
uint32_t Renderer::GetViewportX()
{
#if EDITOR
    return (IsPlayingInEditor() && !GetEditorState()->mEjected) ? 0 : GetEditorState()->mViewportX;
#else
    return uint32_t(0);
#endif
}

uint32_t Renderer::GetViewportY()
{
#if EDITOR
    return (IsPlayingInEditor() && !GetEditorState()->mEjected) ? 0 : GetEditorState()->mViewportY;
#else
    return uint32_t(0);
#endif
}

uint32_t Renderer::GetViewportWidth()
{
    uint32_t windowWidth = (mScreenIndex == 0) ? GetEngineState()->mWindowWidth : GetEngineState()->mSecondWindowWidth;

#if EDITOR
    return (IsPlayingInEditor() && !GetEditorState()->mEjected) ? windowWidth : GetEditorState()->mViewportWidth;
#else
    return windowWidth;
#endif
}

uint32_t Renderer::GetViewportHeight()
{
    uint32_t windowHeight = (mScreenIndex == 0) ? GetEngineState()->mWindowHeight : GetEngineState()->mSecondWindowHeight;

#if EDITOR
    return (IsPlayingInEditor() && !GetEditorState()->mEjected) ? windowHeight : GetEditorState()->mViewportHeight;
#else
    return windowHeight;
#endif
}

glm::uvec4 Renderer::GetViewport()
{
    return glm::uvec4(GetViewportX(), GetViewportY(), GetViewportWidth(), GetViewportHeight());
}

glm::uvec4 Renderer::GetSceneViewport()
{
    float resScale = GetEngineState()->mGraphics.mResolutionScale;
    int32_t vx = GetViewportX();
    int32_t vy = GetViewportY();
    int32_t vw = GetViewportWidth();
    int32_t vh = GetViewportHeight();

    vx = uint32_t(vx * resScale + 0.5f);
    vy = uint32_t(vy * resScale + 0.5f);
    vw = uint32_t(vw * resScale + 0.5f);
    vh = uint32_t(vh * resScale + 0.5f);

    return glm::uvec4(vx, vy, vw, vh);
}
