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

constexpr uint8_t kMeshColGroup = 0x02;
constexpr uint8_t kInstanceColGroup = 0x04;
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
    mSpacing = glm::clamp(mSpacing, 1.0f, 100.0f);

    UpdateDynamicsWorld();

    if (GetEditorState()->GetViewport3D()->ShouldHandleInput())
    {
        UpdateHotkeys();
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
        else
        {
            LogError("Failed to generate instant mesh paint collision!!");
        }
    }
    else
    {
        btTransform transform = ConvertToBulletTransform(paintCol.mPosition, paintCol.mRotation);
        paintCol.mCollisionObject->setWorldTransform(transform);

        btCollisionShape* triShape = CreateTriCollisionShape(paintCol.mMesh.Get<StaticMesh>(), paintCol.mScale);
        paintCol.mCollisionObject->setCollisionShape(triShape);
    }

    int colGroup = instMesh ? kInstanceColGroup : kMeshColGroup;
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
                InstancedMesh3D* instMesh = node->As<InstancedMesh3D>();

                if (meshNode && 
                    meshNode->GetStaticMesh() &&
                    (!onlySelected || GetEditorState()->IsNodeSelected(node)) &&
                    (instMesh == nullptr || instMesh->GetNumInstances() > 0))
                {
                    StaticMesh* curMesh = meshNode->GetStaticMesh();
                    glm::vec3 curPosition = meshNode->GetWorldPosition();
                    glm::quat curRotation = meshNode->GetWorldRotationQuat();
                    glm::vec3 curScale = meshNode->GetWorldScale();

                    if (instMesh != nullptr)
                    {
                        instMesh->UpdateInstanceData();
                    }

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
                    bool instChanged = instMesh ? instMesh->WasInstanceDataUpdatedThisFrame() : false;

                    if (meshChanged ||
                        posChanged ||
                        rotChanged ||
                        sclChanged ||
                        instChanged)
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

void PaintManager::UpdateHotkeys()
{
    if (IsKeyJustDown(KEY_E))
    {
        mInstanceOptions.mErase = !mInstanceOptions.mErase;
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
            RayTestResult rayResult;
            mSpherePosition = camera->TraceScreenToWorld(mouseX, mouseY, ~(kPaintSphereColGroup | kInstanceColGroup), rayResult);
            mSphereNormal = rayResult.mHitNormal;
            mSphereValid = (rayResult.mHitNode != nullptr);
            GetWorld(0)->RestoreDynamicsWorld();
        }
    }

    // Draw the sphere
    const glm::vec4 kFullOpacityColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    const glm::vec4 kZeroOpacityColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    glm::vec4 matColor = glm::mix(kZeroOpacityColor, kFullOpacityColor, mOpacity);
    if (GetEditorState()->GetPaintMode() == PaintMode::Instance)
    {
        // In instance paint mode, use full opacity color when painting, and zero opacity color when erasing.
        matColor = mInstanceOptions.mErase ? kZeroOpacityColor : kFullOpacityColor;
    }

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
    World* world = GetWorld(0);
    PaintMode paintMode = GetEditorState()->GetPaintMode();

    int32_t mouseX = 0;
    int32_t mouseY = 0;
    GetMousePosition(mouseX, mouseY);
    glm::vec2 curMousePos = glm::vec2(float(mouseX), float(mouseY));

    Camera3D* camera = world->GetActiveCamera();
    glm::vec3 cameraFwd = camera ? camera->GetForwardVector() : glm::vec3(0.0f, 0.0f, -1.0f);

    bool paint = false;
    if (IsMouseButtonJustDown(MOUSE_LEFT))
    {
        paint = true;
        mPendingColorData.clear();
        mPendingInstanceData = PendingInstanceData();
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
        mLastPaintMousePos = curMousePos;

        if (paintMode == PaintMode::Color)
        {
            std::vector<ActionSetInstanceColorsData> colorData;

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
                    if (mColorOptions.mOnlyFacingNormals)
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
                        glm::vec4 src = mColorOptions.mColor;
                        glm::vec4 out = dst;

                        float falloff = 1.0f - (dist / mRadius);
                        float a = mOpacity * falloff;

                        switch (mColorOptions.mBlendMode)
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
        else if (paint && paintMode == PaintMode::Instance)
        {
#if 0
            for (int32_t i = 0; i < mDynamicsWorld->getNumCollisionObjects(); ++i)
            {
                btCollisionObject* colObj = mDynamicsWorld->getCollisionObjectArray()[i];
                btCollisionShape* colShape = colObj->getCollisionShape();

                if (colObj && colShape)
                {
                    Primitive3D* prim3d = (Primitive3D*)colObj->getUserPointer();
                    if (prim3d != nullptr)
                    {
                        DebugDrawCollisionShape(colShape, prim3d, prim3d->GetTransform());
                    }
                }
            }
#endif
            InstancedMesh3D* instMesh = nullptr;

            if (mPendingInstanceData.mMeshNode != nullptr)
            {
                instMesh = mPendingInstanceData.mMeshNode;
            }
            else
            {
                StaticMesh* staticMesh = mInstanceOptions.mMesh.Get<StaticMesh>();
                if (staticMesh != nullptr)
                {
                    std::vector<InstancedMesh3D*> instNodes;
                    world->FindNodes<InstancedMesh3D>(instNodes);

                    for (uint32_t i = 0; i < instNodes.size(); ++i)
                    {
                        if (instNodes[i]->GetStaticMesh() == staticMesh &&
                            instNodes[i]->HasTag("Paint"))
                        {
                            instMesh = instNodes[i];
                            break;
                        }
                    }

                    if (instMesh == nullptr)
                    {
                        instMesh = world->SpawnNode<InstancedMesh3D>();
                        instMesh->SetName("Painted_" + staticMesh->GetName());
                        instMesh->AddTag("Paint");
                        instMesh->SetStaticMesh(staticMesh);
                    }
                }

                mPendingInstanceData.mMeshNode = instMesh;
                mPendingInstanceData.mOriginalData = instMesh->GetInstanceData();
                mPendingInstanceData.mData = mPendingInstanceData.mOriginalData;
            }

            if (instMesh != nullptr)
            {
                if (mInstanceOptions.mErase)
                {
                    glm::mat4 nodeTransform = instMesh->GetTransform();

#if 0
                    // Use bounds to determine erase
                    auto it = mMeshCollisionMap.find(instMesh);
                    if (it != mMeshCollisionMap.end())
                    {
                        btCollisionShape* colShape = it->second.mCollisionObject->getCollisionShape();
                        btCompoundShape* compShape = nullptr;
                        if (colShape->getShapeType() == COMPOUND_SHAPE_PROXYTYPE)
                        {
                            compShape = (btCompoundShape*)colShape;
                        }

                        OCT_ASSERT(compShape);
                        int32_t numChildren = compShape->getNumChildShapes();
                        for (int32_t c = numChildren - 1; c >= 0; --c)
                        {
                            btCollisionShape* childShape = compShape->getChildShape(c);
                            OCT_ASSERT(childShape->getShapeType() == SPHERE_SHAPE_PROXYTYPE);
                            btSphereShape* childSphere = (btSphereShape*) childShape;

                            const btTransform& childTransform = compShape->getChildTransform(c);
                            glm::vec3 childPos = BulletToGlm(childTransform.getOrigin());
                            glm::vec3 childWorldPos = nodeTransform * glm::vec4(childPos, 1.0f);

                            float dist2 = glm::distance2(childWorldPos, mSpherePosition);
                            float boundsRad = (mRadius);
                            float boundsRad2 = boundsRad * boundsRad;
                            bool eraseInstance = (dist2 < boundsRad2);

                            if (eraseInstance)
                            {
                                mPendingInstanceData.mData.erase(mPendingInstanceData.mData.begin() + c);
                                instMesh->RemoveInstanceData(c);
                            }
                        }
                    }
#else
                    // Use instance position to determine erase
                    const float sphereRad2 = mRadius * mRadius;
                    int32_t numInst = (int32_t)instMesh->GetNumInstances();

                    for (int32_t i = numInst - 1; i >= 0; --i)
                    {
                        glm::vec3 instPos = instMesh->GetInstanceData(i).mPosition;
                        instPos = nodeTransform * glm::vec4(instPos, 1.0f);

                        float dist2 = glm::distance2(mSpherePosition, instPos);

                        if (dist2 < sphereRad2)
                        {
                            mPendingInstanceData.mData.erase(mPendingInstanceData.mData.begin() + i);
                            instMesh->RemoveInstanceData(i);
                        }
                    }
#endif
                }
                else
                {
                    // Determine number of meshes to paint. Function of Radius and Density.
                    float area = PI * mRadius * mRadius;
                    float density = mInstanceOptions.mDensity;
                    if (density < 1.0f)
                    {
                        density = pow(density, 2.0f);
                    }
                    float fNumMeshes = area * mInstanceOptions.mDensity * 0.01f;

                    int32_t numMeshes = int32_t(fNumMeshes + 0.5f);
                    numMeshes = glm::clamp(numMeshes, 1, 1000);

                    const float minSep2 = mInstanceOptions.mMinSeparation * mInstanceOptions.mMinSeparation;

                    glm::mat4 invParentTransform = glm::inverse(instMesh->GetParentTransform());

                    for (int32_t i = 0; i < numMeshes; ++i)
                    {
                        glm::vec3 normal = mSphereNormal;
                        glm::vec3 helperVec = (fabs(normal.y) > 0.9f) ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
                        glm::vec3 tangent = glm::cross(normal, helperVec);

                        // Cast N ray tests at random points.
                        float randDist = mRadius * sqrtf(Maths::RandRange(0.0f, 1.0f));
                        float angle = Maths::RandRange(0.0f, 360.0f);

                        glm::vec3 randPoint = tangent * randDist;
                        randPoint = glm::rotate(randPoint, angle * DEGREES_TO_RADIANS, normal);
                        randPoint += mSpherePosition;

                        RayTestResult rayResult;
                        world->OverrideDynamicsWorld(mDynamicsWorld);
                        world->RayTest(randPoint + normal * mRadius, randPoint + normal * -mRadius, kMeshColGroup, rayResult);
                        world->RestoreDynamicsWorld();

                        float dist2 = glm::distance2(randPoint, mSpherePosition);

                        if (rayResult.mHitNode != nullptr &&
                            dist2 < (mRadius * mRadius))
                        {
                            randPoint = rayResult.mHitPosition;

                            if (mInstanceOptions.mMinSeparation > 0.0f)
                            {
                                glm::vec3 randPointLocal = invParentTransform * glm::vec4(randPoint, 1.0f);

                                const std::vector<MeshInstanceData>& instData = instMesh->GetInstanceData();
                                bool tooClose = false;

                                for (const MeshInstanceData& instDatum : instData)
                                {
                                    float instDist2 = glm::distance2(randPointLocal, instDatum.mPosition);
                                    if (instDist2 < minSep2)
                                    {
                                        tooClose = true;
                                        break;;
                                    }
                                }

                                if (tooClose)
                                {
                                    continue;
                                }
                            }

                            glm::vec3 instRot = glm::vec3(0.0f, 0.0f, 0.0f);
                            if (mInstanceOptions.mAlignWithNormal)
                            {
                                glm::mat4 rotMat = glm::orientation(normal, glm::vec3(0.0f, 1.0f, 0.0f));
                                glm::quat rotQuat = glm::quat(rotMat);

                                if (isnan(rotQuat.x))
                                {
                                    // When normal is opposite of upvector we get nan so... just assume if the quat is nan then we have a 
                                    // normal that is pointing straight down?
                                    rotQuat = glm::quat(DEGREES_TO_RADIANS *  glm::vec3(180.0f, 0.0f, 0.0f));
                                }

                                instRot = glm::eulerAngles(rotQuat) * RADIANS_TO_DEGREES;
                            }

                            MeshInstanceData newInstData;
                            newInstData.mPosition = randPoint + Maths::RandRange(mInstanceOptions.mMinPosition, mInstanceOptions.mMaxPosition);
                            newInstData.mRotation = instRot + Maths::RandRange(mInstanceOptions.mMinRotation, mInstanceOptions.mMaxRotation);
                            float instScale = Maths::RandRange(mInstanceOptions.mMinScale, mInstanceOptions.mMaxScale);
                            newInstData.mScale = glm::vec3(instScale, instScale, instScale);

                            // Transform from world to local space... at least position (probably need to do same for rot/scale)
                            if (instMesh->GetParent())
                            {
                                newInstData.mPosition = invParentTransform * glm::vec4(newInstData.mPosition, 1.0f);
                            }

                            mPendingInstanceData.mData.push_back(newInstData);
                            instMesh->AddInstanceData(newInstData);
                        }
                    }
                }
            }
        }
    }

    if (IsMouseButtonJustUp(MOUSE_LEFT))
    {
        if (mPendingColorData.size() > 0)
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
        else if (mPendingInstanceData.mMeshNode != nullptr)
        {
            mPendingInstanceData.mMeshNode->SetInstanceData(mPendingInstanceData.mOriginalData);
            ActionManager::Get()->EXE_SetInstanceData(
                mPendingInstanceData.mMeshNode,
                -1,
                mPendingInstanceData.mData);
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
