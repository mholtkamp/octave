#if EDITOR

#include "PaintManager.h"
#include "World.h"
#include "Engine.h"
#include "Renderer.h"
#include "EditorState.h"
#include "InputDevices.h"
#include "Viewport3d.h"
#include "ActionManager.h"

#include "Nodes/3D/InstancedMesh3d.h"

constexpr uint8_t kPaintSphereColGroup = 0x80;
constexpr float kPaintMaxRadius = 10000.0f;
constexpr float kPaintMinRadius = 0.01f;

PaintManager::PaintManager()
{
    // Setup physics world
    mCollisionConfig = new btDefaultCollisionConfiguration();
    mCollisionDispatcher = new btCollisionDispatcher(mCollisionConfig);
    mBroadphase = new btDbvtBroadphase();
    mSolver = new btSequentialImpulseConstraintSolver();
    mDynamicsWorld = new btDiscreteDynamicsWorld(mCollisionDispatcher, mBroadphase, mSolver, mCollisionConfig);

    mSphereGhost = new btPairCachingGhostObject();
    mSphereGhostShape = new btSphereShape(1.0f);
    mGhostPairCallback = new btGhostPairCallback();

    mSphereGhost->setCollisionShape(mSphereGhostShape);
    mSphereGhost->setCollisionFlags(mSphereGhost->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
    mDynamicsWorld->addCollisionObject(mSphereGhost, kPaintSphereColGroup, ~kPaintSphereColGroup);
    mDynamicsWorld->getBroadphase()->getOverlappingPairCache()->setInternalGhostPairCallback(mGhostPairCallback);

    mSphereMesh = LoadAsset<StaticMesh>("SM_Sphere");
    mSphereMaterial = MaterialLite::New(LoadAsset<Material>("M_PaintSphere"));
}

PaintManager::~PaintManager()
{
    mDynamicsWorld->removeCollisionObject(mSphereGhost);
    mDynamicsWorld->getBroadphase()->getOverlappingPairCache()->setInternalGhostPairCallback(nullptr);
    mSphereGhost->setCollisionShape(nullptr);
    delete mGhostPairCallback;
    delete mSphereGhostShape;
    delete mSphereGhost;
    mGhostPairCallback = nullptr;
    mSphereGhostShape = nullptr;
    mSphereGhost = nullptr;

    delete mDynamicsWorld;
    delete mSolver;
    delete mBroadphase;
    delete mCollisionDispatcher;
    delete mCollisionConfig;

    mDynamicsWorld = nullptr;
    mSolver = nullptr;
    mBroadphase = nullptr;
    mCollisionDispatcher = nullptr;
    mCollisionConfig = nullptr;
}

void PaintManager::Update()
{
    // Always clamp paint parameters (Imgui might be adjusting them)
    mRadius = glm::clamp(mRadius, kPaintMinRadius, kPaintMaxRadius);
    mOpacity = glm::clamp(mOpacity, 0.0f, 1.0f);

    if (GetEditorState()->GetViewport3D()->ShouldHandleInput())
    {
        UpdateDynamicsWorld();
        UpdatePaintReticle();
        UpdatePaintDraw();
    }
}

void PaintManager::HandleNodeDestroy(Node* node)
{
    StaticMesh3D* meshNode = node->As<StaticMesh3D>();

    auto it = mMeshCollisionMap.find(meshNode);
    if (it != mMeshCollisionMap.end())
    {
        RemovePaintMeshCollision(it->second);
        delete it->second.mCollisionObject;
        it->second.mCollisionObject = nullptr;

        mMeshCollisionMap.erase(meshNode);
    }
}

static btTransform ConvertToBulletTransform(glm::vec3 position, glm::quat rotation)
{
    btTransform transform;
    transform.setOrigin(btVector3(position.x, position.y, position.z));
    transform.setRotation(btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w));
    return transform;
}

static btCollisionShape* CreateTriCollisionShape(StaticMesh* mesh, glm::vec3 scale)
{
    btVector3 btscale = btVector3(scale.x, scale.y, scale.z);
    btScaledBvhTriangleMeshShape* scaledTriangleShape = new btScaledBvhTriangleMeshShape(mesh->GetTriangleCollisionShape(), btscale);
    return scaledTriangleShape;
}

void PaintManager::AddPaintMeshCollision(const PaintMeshCollision& paintCol)
{
    InstancedMesh3D* instMesh = paintCol.mNode->As<InstancedMesh3D>();

    if (instMesh)
    {
        btCollisionShape* colShape = instMesh->GeneratePaintCollisionShape();

        if (colShape != nullptr)
        {
            btTransform transform = ConvertToBulletTransform(paintCol.mPosition, paintCol.mRotation);
            paintCol.mCollisionObject->setWorldTransform(transform);
            paintCol.mCollisionObject->setCollisionShape(colShape);
        }
    }
    else
    {
        btTransform transform = ConvertToBulletTransform(paintCol.mPosition, paintCol.mRotation);
        paintCol.mCollisionObject->setWorldTransform(transform);

        btCollisionShape* triShape = CreateTriCollisionShape(paintCol.mMesh.Get<StaticMesh>(), paintCol.mScale);
        paintCol.mCollisionObject->setCollisionShape(triShape);
    }

    int colGroup = instMesh ? ColGroup0 : ColGroup1;
    int colMask = ~colGroup;

    mDynamicsWorld->addCollisionObject(paintCol.mCollisionObject, colGroup, colMask);
}

void PaintManager::RemovePaintMeshCollision(const PaintMeshCollision& paintCol)
{
    // This object changed, so remove it from the collision world, update it, and then add it back
    mDynamicsWorld->removeCollisionObject(paintCol.mCollisionObject);

    btCollisionShape* colShape = paintCol.mCollisionObject->getCollisionShape();
    if (colShape != nullptr)
    {
        paintCol.mCollisionObject->setCollisionShape(nullptr);
        DestroyCollisionShape(colShape);
        colShape = nullptr;
    }
}

void PaintManager::UpdateDynamicsWorld()
{
    World* world = GetWorld(0);
    Node* rootNode = world ? world->GetRootNode() : nullptr;

    // Iterate over all collision data and set active to false
    // When we traverse the world and update collision data, we will mark visited nodes as active
    // At the end, we remove and collision data that wasn't visited
    for (auto& meshColPair : mMeshCollisionMap)
    {
        meshColPair.second.mActive = false;
    }

    if (rootNode != nullptr)
    {
        bool onlySelected = GetEditorState()->GetSelectedNodes().size() > 0;

        auto updateMeshDynamics = [&](Node* node) -> bool
            {
                if (!node->IsVisible())
                    return false;

                StaticMesh3D* meshNode = node->As<StaticMesh3D>();
                if (meshNode && 
                    meshNode->GetStaticMesh() &&
                    (!onlySelected || GetEditorState()->IsNodeSelected(node)))
                {
                    StaticMesh* curMesh = meshNode->GetStaticMesh();
                    glm::vec3 curPosition = meshNode->GetWorldPosition();
                    glm::quat curRotation = meshNode->GetWorldRotationQuat();
                    glm::vec3 curScale = meshNode->GetWorldScale();

                    // Does this node already exist in the map?
                    auto it = mMeshCollisionMap.find(meshNode);

                    if (it == mMeshCollisionMap.end())
                    {
                        btCollisionObject* colObject = new btCollisionObject();
                        colObject->setUserPointer(meshNode);

                        PaintMeshCollision paintCol;
                        paintCol.mCollisionObject = colObject;
                        paintCol.mNode = meshNode;
                        paintCol.mPosition = curPosition;
                        paintCol.mRotation = curRotation;
                        paintCol.mScale = curScale;
                        paintCol.mMesh = curMesh;

                        AddPaintMeshCollision(paintCol);

                        mMeshCollisionMap.insert({ meshNode, paintCol });
                        it = mMeshCollisionMap.find(meshNode);
                    }

                    OCT_ASSERT(it != mMeshCollisionMap.end());
                    PaintMeshCollision& paintCol = it->second;

                    btCollisionObject* colObject = paintCol.mCollisionObject;
                    btCollisionShape* colShape = colObject->getCollisionShape();
                    const glm::mat4& curTransform = meshNode->GetTransform();

                    bool meshChanged = curMesh != paintCol.mMesh.Get();
                    bool posChanged = glm::any(glm::epsilonNotEqual(paintCol.mPosition, curPosition, 0.00001f));
                    bool rotChanged = glm::any(glm::epsilonNotEqual(paintCol.mRotation, curRotation, 0.00001f));
                    bool sclChanged = glm::any(glm::epsilonNotEqual(paintCol.mScale, curScale, 0.00001f));

                    if (meshChanged ||
                        posChanged ||
                        rotChanged ||
                        sclChanged)
                    {
                        RemovePaintMeshCollision(paintCol);

                        paintCol.mPosition = curPosition;
                        paintCol.mRotation = curRotation;
                        paintCol.mScale = curScale;
                        paintCol.mMesh = curMesh;

                        AddPaintMeshCollision(paintCol);
                    }

                    paintCol.mActive = true;
                }

                return true;
            };

        rootNode->Traverse(updateMeshDynamics);

        for (auto it = mMeshCollisionMap.begin(); it != mMeshCollisionMap.end(); )
        {
            if (it->second.mActive)
            {
                ++it;
            }
            else
            {
                // No longer in the world
                RemovePaintMeshCollision(it->second);

                delete it->second.mCollisionObject;
                it->second.mCollisionObject = nullptr;

                it = mMeshCollisionMap.erase(it);
            }
        }

        // Update the sphere ghost
        mDynamicsWorld->removeCollisionObject(mSphereGhost);
        btTransform sphereTransform = btTransform::getIdentity();
        sphereTransform.setOrigin(btVector3(mSpherePosition.x, mSpherePosition.y, mSpherePosition.z));
        mSphereGhost->setWorldTransform(sphereTransform);
        mSphereGhostShape->setImplicitShapeDimensions(btVector3(mRadius, 0.0f, 0.0f));
        mSphereGhostShape->setMargin(mRadius);
        mDynamicsWorld->addCollisionObject(mSphereGhost, kPaintSphereColGroup, ~kPaintSphereColGroup);

        // Is this needed? We just need to update overlaps really
        mDynamicsWorld->stepSimulation(0.1f);
    }
}


void PaintManager::UpdatePaintReticle()
{
    int32_t mouseX = 0;
    int32_t mouseY = 0;
    GetMousePosition(mouseX, mouseY);

    glm::clamp<int32_t>(mouseX, 0, GetEngineState()->mWindowWidth);
    glm::clamp<int32_t>(mouseY, 0, GetEngineState()->mWindowHeight);

    int32_t deltaX = 0;
    int32_t deltaY = 0;
    INP_GetMouseDelta(deltaX, deltaY);
    float totalDelta = float(deltaX - deltaY);

    if (!IsMouseButtonDown(MOUSE_LEFT))
    {
        mAdjustmentFinished = false;
    }

    if (!mAdjustRadius && !mAdjustOpacity)
    {
        if (IsKeyJustDown(KEY_F))
        {
            if (IsShiftDown())
            {
                mAdjustOpacity = true;
                mPreAdjustOpacity = mOpacity;
            }
            else
            {
                mAdjustRadius = true;
                mPreAdjustRadius = mRadius;
            }

            INP_ShowCursor(false);
            INP_LockCursor(true);
        }
    }

    if (mAdjustRadius)
    {
        float radiusDelta = totalDelta * 0.05f;
        mRadius += radiusDelta;
        mRadius = glm::clamp(mRadius, kPaintMinRadius, kPaintMaxRadius);

        if (IsMouseButtonJustDown(MOUSE_LEFT))
        {
            mAdjustRadius = false;
            FinishAdjustment();
        }
        else if (IsMouseButtonJustDown(MOUSE_RIGHT))
        {
            mRadius = mPreAdjustRadius;
            mAdjustRadius = false;
            FinishAdjustment();
        }
    }
    else if (mAdjustOpacity)
    {
        float opacityDelta = totalDelta * 0.0025f;
        mOpacity += opacityDelta;
        mOpacity = glm::clamp(mOpacity, 0.0f, 1.0f);

        if (IsMouseButtonJustDown(MOUSE_LEFT))
        {
            mAdjustOpacity = false;
            FinishAdjustment();
        }
        else if (IsMouseButtonJustDown(MOUSE_RIGHT))
        {
            mOpacity = mPreAdjustOpacity;
            mAdjustOpacity = false;
            FinishAdjustment();
        }
    }
    else
    {
        Camera3D* camera = GetEditorState()->GetEditorCamera();
        if (camera)
        {
            GetWorld(0)->OverrideDynamicsWorld(mDynamicsWorld);
            Primitive3D* hitPrim = nullptr;
            mSpherePosition = camera->TraceScreenToWorld(mouseX, mouseY, ~kPaintSphereColGroup, &hitPrim);
            mSphereValid = (hitPrim != nullptr);
            GetWorld(0)->RestoreDynamicsWorld();
        }
    }

    // Draw the sphere
    glm::vec4 matColor = glm::mix(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), mOpacity);
    mSphereMaterial.Get<MaterialLite>()->SetFresnelColor(matColor);

    if (mSphereValid)
    {
        DebugDraw paintSphereDraw;
        paintSphereDraw.mMesh = mSphereMesh.Get<StaticMesh>();
        paintSphereDraw.mMaterial = mSphereMaterial.Get<Material>();
        paintSphereDraw.mColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        paintSphereDraw.mTransform = glm::translate(mSpherePosition) * glm::scale(glm::vec3(mRadius, mRadius, mRadius));
        Renderer::Get()->AddDebugDraw(paintSphereDraw);
    }
}

void PaintManager::UpdatePaintDraw()
{
    int32_t mouseX = 0;
    int32_t mouseY = 0;
    GetMousePosition(mouseX, mouseY);
    glm::vec2 curMousePos = glm::vec2(float(mouseX), float(mouseY));

    Camera3D* camera = GetWorld(0)->GetActiveCamera();
    glm::vec3 cameraFwd = camera ? camera->GetForwardVector() : glm::vec3(0.0f, 0.0f, -1.0f);

    bool paint = false;
    if (IsMouseButtonJustDown(MOUSE_LEFT))
    {
        paint = true;
        mPendingColorData.clear();
    }
    else if (IsMouseButtonDown(MOUSE_LEFT))
    {
        float paintMoveDist = glm::distance(mLastPaintMousePos, curMousePos);
        paint = (paintMoveDist >= mSpacing);
    }

    // Don't paint if we just left clicked after adjusting radius or opacity
    if (mAdjustmentFinished)
    {
        paint = false;
    }

    if (!mSphereValid)
    {
        paint = false;
    }

    if (IsControlDown() || IsShiftDown())
    {
        // Ctrl and shift are used for selecting objects
        paint = false;
    }

    if (paint)
    {
        std::vector<ActionSetInstanceColorsData> colorData;

        mLastPaintMousePos = curMousePos;
        const float sphereRad2 = mRadius * mRadius;
        int32_t numOverlaps = mSphereGhost->getNumOverlappingObjects();

        for (int32_t i = 0; i < numOverlaps; ++i)
        {
            btCollisionObject* colObj = mSphereGhost->getOverlappingObject(i);
            StaticMesh3D* mesh3d = (StaticMesh3D*) colObj->getUserPointer();
            StaticMesh* mesh = mesh3d ? mesh3d->GetStaticMesh() : nullptr;

            if (mesh == nullptr)
                continue;

            uint32_t numVerts = mesh->GetNumVertices();
            bool meshHasColor = mesh->HasVertexColor();
            void* vertices = meshHasColor ? (void*)mesh->GetColorVertices() : (void*)mesh->GetVertices();
            std::vector<uint32_t>& instColors = mesh3d->GetInstanceColors();

            // Do we already have a pending color data for this overlapped mesh?
            PendingColorData* pendingData = nullptr;
            for (uint32_t i = 0; i < mPendingColorData.size(); ++i)
            {
                if (mPendingColorData[i].mData.mMesh3d == mesh3d)
                {
                    pendingData = &mPendingColorData[i];
                    break;
                }
            }

            // Initialize pending draw data.
            if (pendingData == nullptr)
            {
                PendingColorData newPendingData;
                newPendingData.mData.mMesh3d = mesh3d;
                newPendingData.mData.mColors = mesh3d->GetInstanceColors();
                newPendingData.mData.mBakedLight = false;
                newPendingData.mVertexDrawAlpha.resize(numVerts);
                newPendingData.mOriginalData.mMesh3d = mesh3d;
                newPendingData.mOriginalData.mColors = mesh3d->GetInstanceColors();
                newPendingData.mOriginalData.mBakedLight = mesh3d->HasBakedLighting();

                std::vector<uint32_t>& pendColors = newPendingData.mData.mColors;

                // If we don't have valid instance colors, resize and set them all to white.
                if (pendColors.size() != numVerts)
                {
                    pendColors.resize(numVerts);

                    for (uint32_t c = 0; c < numVerts; ++c)
                    {
                        pendColors[c] = 0xffffffff;
                    }
                }

                newPendingData.mOriginalColors = pendColors;

                mPendingColorData.push_back(newPendingData);
                pendingData = &mPendingColorData.back();
            }

            const glm::mat4& transform = mesh3d->GetTransform();
            bool anyVertColored = false;

            for (uint32_t v = 0; v < numVerts; ++v)
            {
                glm::vec3 vertLocalPos = meshHasColor ? ((VertexColor*)vertices)[v].mPosition : ((Vertex*)vertices)[v].mPosition;
                glm::vec3 vertWorldPos = glm::vec3(transform * glm::vec4(vertLocalPos, 1.0f));

                float dist2 = glm::distance2(mSpherePosition, vertWorldPos);

                bool alignedNormal = true;
                if (mOnlyFacingNormals)
                {
                    glm::vec3 normalLocal = meshHasColor ? ((VertexColor*)vertices)[v].mNormal : ((Vertex*)vertices)[v].mNormal;
                    glm::vec3 normalWorld = glm::vec3(transform * glm::vec4(normalLocal, 0.0f));
                    float normalDot = glm::dot(normalWorld, -cameraFwd);

                    alignedNormal = normalDot >= 0.0f;
                }

                if (dist2 < sphereRad2 &&
                    alignedNormal)
                {
                    float dist = sqrtf(dist2);

                    glm::vec4 dst = ColorUint32ToFloat4(pendingData->mOriginalColors[v]);
                    glm::vec4 src = mColor;
                    glm::vec4 out = dst;

                    float falloff = 1.0f - (dist / mRadius);
                    float a = mOpacity * falloff;

                    switch (mBlendMode)
                    {
                    case PaintBlendMode::Mix:
                        out = (src * a) + (dst * (1.0f - a));
                        out.a = dst.a;
                        break;
                    case PaintBlendMode::Add:
                        out = (src * a) + dst;
                        out.a = dst.a;
                        break;
                    case PaintBlendMode::Subtract:
                        out = dst - (src * a);
                        out.a = dst.a;
                        break;
                    case PaintBlendMode::Multiply:
                        out = glm::mix(dst, src * dst, a);
                        out.a = dst.a;
                        break;
                    case PaintBlendMode::AddAlpha:
                        out.a = dst.a + src.a * a;
                        break;
                    case PaintBlendMode::SubtractAlpha:
                        out.a = dst.a - src.a * a;
                        break;
                    }

                    float drawAlpha = falloff;
                    if (drawAlpha > pendingData->mVertexDrawAlpha[v])
                    {
                        pendingData->mVertexDrawAlpha[v] = drawAlpha;
                        pendingData->mData.mColors[v] = ColorFloat4ToUint32(out);
                        pendingData->mAnyVertexPainted = true;
                        anyVertColored = true;
                    }
                }
            }

            // Immediately set instance colors.
            // Once we let go of the mouse, we will reset the colors to the original ones and
            // then maked a call to the action manager with the new set of instance colors so
            // that we can make use of the Undo/Redo system.
            if (anyVertColored)
            {
                mesh3d->SetInstanceColors(pendingData->mData.mColors, false);
            }
        }
    }

    if (IsMouseButtonJustUp(MOUSE_LEFT) &&
        mPendingColorData.size() > 0)
    {
        // Commit our pending color data changes
        std::vector<ActionSetInstanceColorsData> actionData;

        for (uint32_t i = 0; i < mPendingColorData.size(); ++i)
        {
            if (mPendingColorData[i].mAnyVertexPainted)
            {
                // Revert instance colors on mesh3d
                mPendingColorData[i].mData.mMesh3d->SetInstanceColors(mPendingColorData[i].mOriginalData.mColors, mPendingColorData[i].mOriginalData.mBakedLight);

                actionData.push_back(mPendingColorData[i].mData);
            }
        }

        if (actionData.size() > 0)
        {
            ActionManager::Get()->EXE_SetInstanceColors(actionData);
        }
    }
}

void PaintManager::FinishAdjustment()
{
    INP_ShowCursor(true);
    INP_LockCursor(false);
    mAdjustmentFinished = true;
}

#endif
