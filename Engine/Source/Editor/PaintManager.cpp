#if EDITOR

#include "PaintManager.h"
#include "World.h"
#include "Engine.h"
#include "Renderer.h"
#include "EditorState.h"
#include "InputDevices.h"

PaintManager::PaintManager()
{
    // Setup physics world
    mCollisionConfig = new btDefaultCollisionConfiguration();
    mCollisionDispatcher = new btCollisionDispatcher(mCollisionConfig);
    mBroadphase = new btDbvtBroadphase();
    mSolver = new btSequentialImpulseConstraintSolver();
    mDynamicsWorld = new btDiscreteDynamicsWorld(mCollisionDispatcher, mBroadphase, mSolver, mCollisionConfig);
}

PaintManager::~PaintManager()
{
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
    UpdateDynamicsWorld();
    UpdatePaintReticle();
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
    btTransform transform = ConvertToBulletTransform(paintCol.mPosition, paintCol.mRotation);
    paintCol.mCollisionObject->setWorldTransform(transform);

    btCollisionShape* triShape = CreateTriCollisionShape(paintCol.mMesh.Get<StaticMesh>(), paintCol.mScale);
    paintCol.mCollisionObject->setCollisionShape(triShape);

    mDynamicsWorld->addCollisionObject(paintCol.mCollisionObject);
}

void PaintManager::RemovePaintMeshCollision(const PaintMeshCollision& paintCol)
{
    // This object changed, so remove it from the collision world, update it, and then add it back
    mDynamicsWorld->removeCollisionObject(paintCol.mCollisionObject);

    btCollisionShape* colShape = paintCol.mCollisionObject->getCollisionShape();
    if (colShape != nullptr)
    {
        paintCol.mCollisionObject->setCollisionShape(nullptr);
        delete colShape;
        colShape = nullptr;
    }
}

void PaintManager::UpdateDynamicsWorld()
{
    World* world = GetWorld(0);
    Node* rootNode = world ? world->GetRootNode() : nullptr;

    if (rootNode != nullptr)
    {
        auto updateMeshDynamics = [&](Node* node) -> bool
            {
                if (!node->IsVisible())
                    return false;

                StaticMesh3D* meshNode = node->As<StaticMesh3D>();
                if (meshNode && meshNode->GetStaticMesh())
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

                        PaintMeshCollision paintCol;
                        paintCol.mCollisionObject = colObject;
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
                }

                return true;
            };

        rootNode->Traverse(updateMeshDynamics);

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

    Camera3D* camera = GetEditorState()->GetEditorCamera();
    if (camera)
    {
        glm::vec3 worldPos = camera->TraceScreenToWorld(mouseX, mouseY, 0xff);

        DebugDraw paintSphereDraw;
        paintSphereDraw.mMesh = LoadAsset<StaticMesh>("SM_Sphere");
        paintSphereDraw.mMaterial = LoadAsset<Material>("M_PaintSphere");
        paintSphereDraw.mColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        paintSphereDraw.mTransform = glm::translate(worldPos);

        Renderer::Get()->AddDebugDraw(paintSphereDraw);
    }
}

#endif
