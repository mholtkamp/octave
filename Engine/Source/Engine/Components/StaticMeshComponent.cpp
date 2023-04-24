#include "Components/StaticMeshComponent.h"

#include "Assets/StaticMesh.h"
#include "Renderer.h"
#include "AssetManager.h"
#include "Log.h"
#include "Utilities.h"

#include "Graphics/Graphics.h"

#include <btBulletDynamicsCommon.h>

FORCE_LINK_DEF(StaticMeshComponent);
DEFINE_COMPONENT(StaticMeshComponent);

static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);
    OCT_ASSERT(prop != nullptr);
    StaticMeshComponent* meshComp = static_cast<StaticMeshComponent*>(prop->mOwner);
    bool success = false;

    if (prop->mName == "Static Mesh")
    {
        meshComp->SetStaticMesh(*(StaticMesh**)newValue);
        success = true;
    }
    else if (prop->mName == "Use Triangle Collision")
    {
        meshComp->SetUseTriangleCollision(*(bool*)newValue);
        success = true;
    }
    else if (prop->mName == "Clear Baked Lighting")
    {
        meshComp->ClearInstanceColors();
        success = true;
    }

    return success;
}

static StaticMesh* GetDefaultMesh()
{
    // TODO: replace this with calls to the asset manager.
    static StaticMeshRef defaultMesh = LoadAsset("SM_Cube");
    return defaultMesh.Get<StaticMesh>();
}

StaticMeshComponent::StaticMeshComponent() :
    mStaticMesh(nullptr),
    mUseTriangleCollision(false),
    mBakeLighting(true)
{
    mName = "Static Mesh";
}

StaticMeshComponent::~StaticMeshComponent()
{

}

const char* StaticMeshComponent::GetTypeName() const
{
    return "StaticMesh";
}

void StaticMeshComponent::GatherProperties(std::vector<Property>& outProps)
{
    MeshComponent::GatherProperties(outProps);
    static bool sFakeBool = false;
    outProps.push_back(Property(DatumType::Asset, "Static Mesh", this, &mStaticMesh, 1, HandlePropChange, int32_t(StaticMesh::GetStaticType())));
    outProps.push_back(Property(DatumType::Bool, "Use Triangle Collision", this, &mUseTriangleCollision, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Bool, "Bake Lighting", this, &mBakeLighting, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Bool, "Clear Baked Lighting", this, &sFakeBool, 1, HandlePropChange));
}

void StaticMeshComponent::Create()
{
    MeshComponent::Create();
    GFX_CreateStaticMeshCompResource(this);
    SetStaticMesh(GetDefaultMesh());
}

void StaticMeshComponent::Destroy()
{
    MeshComponent::Destroy();
    GFX_DestroyStaticMeshCompResource(this);
}

StaticMeshCompResource* StaticMeshComponent::GetResource()
{
    return &mResource;
}

void StaticMeshComponent::SaveStream(Stream& stream)
{
    MeshComponent::SaveStream(stream);
    stream.WriteAsset(mStaticMesh);
    stream.WriteBool(mUseTriangleCollision);
}

void StaticMeshComponent::LoadStream(Stream& stream)
{
    MeshComponent::LoadStream(stream);

    AssetRef meshRef;
    stream.ReadAsset(meshRef);
    if (meshRef.Get<StaticMesh>() == nullptr)
        meshRef = GetDefaultMesh();

    mUseTriangleCollision = stream.ReadBool();

    // Set mesh only after loading everything else (like mUseTriangleCollision)
    SetStaticMesh(meshRef.Get<StaticMesh>());
}

bool StaticMeshComponent::IsStaticMeshComponent() const
{
    return true;
}

bool StaticMeshComponent::IsSkeletalMeshComponent() const
{
    return false;
}

void StaticMeshComponent::SetStaticMesh(StaticMesh* staticMesh)
{
    if (mStaticMesh.Get() != staticMesh)
    {
        mStaticMesh = staticMesh;
        RecreateCollisionShape();
        ClearInstanceColors();
    }
}

StaticMesh* StaticMeshComponent::GetStaticMesh()
{
    return mStaticMesh.Get<StaticMesh>();
}

void StaticMeshComponent::SetUseTriangleCollision(bool triangleCol)
{
    if (mUseTriangleCollision != triangleCol)
    {
        mUseTriangleCollision = triangleCol;
        RecreateCollisionShape();
    }
}

bool StaticMeshComponent::GetUseTriangleCollision() const
{
    return mUseTriangleCollision;
}

void StaticMeshComponent::SetBakeLighting(bool bake)
{
    mBakeLighting = bake;
}

bool StaticMeshComponent::GetBakeLighting() const
{
    return mBakeLighting;
}

Material* StaticMeshComponent::GetMaterial()
{
    Material* mat = mMaterialOverride.Get<Material>();

    if (!mat && mStaticMesh.Get())
    {
        mat = mStaticMesh.Get<StaticMesh>()->GetMaterial();
    }

    return mat;
}

void StaticMeshComponent::Render()
{
    GFX_DrawStaticMeshComp(this);
}

VertexType StaticMeshComponent::GetVertexType() const
{
    VertexType retType = VertexType::Vertex;

    if (mStaticMesh.Get<StaticMesh>() != nullptr && 
        mStaticMesh.Get<StaticMesh>()->HasVertexColor())
    {
        retType = VertexType::VertexColor;
    }

    return retType;
}

void StaticMeshComponent::GatherProxyDraws(std::vector<DebugDraw>& inoutDraws)
{
#if DEBUG_DRAW_ENABLED
    MeshComponent::GatherProxyDraws(inoutDraws);

    if (Renderer::Get()->GetDebugMode() == DEBUG_COLLISION &&
        GetType() == StaticMeshComponent::GetStaticType())
    {
        StaticMesh* staticMesh = mStaticMesh.Get<StaticMesh>();

        if (staticMesh != nullptr &&
            mCollisionShape != nullptr)
        {
            uint32_t numCollisionShapes = 0;
            btCollisionShape* collisionShapes[MAX_COLLISION_SHAPES] = {};
            glm::mat4 collisionTransforms[MAX_COLLISION_SHAPES] = {};
            uint32_t collisionMeshIndex = 0;
            OCT_UNUSED(collisionMeshIndex); // Only used in EDITOR

            btCompoundShape* compoundShape = nullptr;

            if (mCollisionShape->getShapeType() == COMPOUND_SHAPE_PROXYTYPE)
            {
                compoundShape = static_cast<btCompoundShape*>(mCollisionShape);
                for (uint32_t i = 0; i < uint32_t(compoundShape->getNumChildShapes()); ++i)
                {
                    if (i >= MAX_COLLISION_SHAPES)
                    {
                        LogError("Too many collision shapes");
                        break;
                    }

                    collisionShapes[numCollisionShapes] = compoundShape->getChildShape(i);
                    const btTransform& bTransform = compoundShape->getChildTransform(i);
                    btQuaternion bRotation = bTransform.getRotation();
                    btVector3 bPosition = bTransform.getOrigin();

                    glm::quat rotation = glm::quat(bRotation.w(), bRotation.x(), bRotation.y(), bRotation.z());
                    glm::vec3 position = { bPosition.x(), bPosition.y(), bPosition.z() };

                    collisionTransforms[i] = glm::mat4(1);
                    collisionTransforms[i] = glm::translate(collisionTransforms[i], position);
                    collisionTransforms[i] *= glm::toMat4(rotation);
                        
                    ++numCollisionShapes;
                }
            }
            else
            {
                collisionShapes[0] = mCollisionShape;
                numCollisionShapes = 1;
            }

            for (uint32_t i = 0; i < numCollisionShapes; ++i)
            {
                btCollisionShape* collisionShape = collisionShapes[i];
                int shapeType = collisionShape->getShapeType();

                if (shapeType == EMPTY_SHAPE_PROXYTYPE)
                    continue;

                DebugDraw debugDraw;

                switch (shapeType)
                {
                case BOX_SHAPE_PROXYTYPE:
                {
                    // Assuming that default cube mesh has half extents equal to 1,1,1
                    btBoxShape* boxShape = static_cast<btBoxShape*>(collisionShape);
                    btVector3 halfExtents = boxShape->getHalfExtentsWithMargin();

                    glm::mat4 scale = glm::scale(glm::vec3(1 / mScale.x, 1 / mScale.y, 1 / mScale.z));
                    collisionTransforms[i] = scale * collisionTransforms[i];
                    collisionTransforms[i] = glm::scale(collisionTransforms[i], { halfExtents.x(), halfExtents.y(), halfExtents.z()});

                    debugDraw.mMesh = LoadAsset<StaticMesh>("SM_Cube");
                    debugDraw.mTransform = mTransform;
                    break;
                }
                case SPHERE_SHAPE_PROXYTYPE:
                {
                    // Assuming that default sphere mesh has a radius of 1
                    btSphereShape* sphereShape = static_cast<btSphereShape*>(collisionShape);
                    float radius = sphereShape->getRadius();

                    glm::mat4 scale = glm::scale(glm::vec3(1 / mScale.x, 1 / mScale.y, 1 / mScale.z));
                    collisionTransforms[i] = scale * collisionTransforms[i];
                    collisionTransforms[i] = glm::scale(collisionTransforms[i], {radius, radius, radius});

                    debugDraw.mMesh = LoadAsset<StaticMesh>("SM_Sphere");
                    debugDraw.mTransform = mTransform;
                    break;
                }
                case CONVEX_HULL_SHAPE_PROXYTYPE:
                {
#if CREATE_CONVEX_COLLISION_MESH
                    // We only create StaticMesh objects for convex hulls when in editor.
                    debugDraw.mMesh = staticMesh->mCollisionMeshes[collisionMeshIndex];
                    debugDraw.mTransform = mTransform;

                    ++collisionMeshIndex;
#endif
                    break;
                }
                case SCALED_TRIANGLE_MESH_SHAPE_PROXYTYPE:
                {
                    debugDraw.mMesh = staticMesh;
                    debugDraw.mTransform = mTransform;
                    break;
                }
                }

                if (compoundShape != nullptr)
                {
                    debugDraw.mTransform = debugDraw.mTransform * collisionTransforms[i];
                }

                debugDraw.mColor = GetCollisionDebugColor();
                inoutDraws.push_back(debugDraw);
            }
        }
    }
#endif
}

Bounds StaticMeshComponent::GetLocalBounds() const
{
    if (mStaticMesh != nullptr)
    {
        return mStaticMesh.Get<StaticMesh>()->GetBounds();
    }
    else
    {
        return MeshComponent::GetLocalBounds();
    }
}

void StaticMeshComponent::ClearInstanceColors()
{
    mInstanceColors.clear();
    GFX_UpdateStaticMeshCompResourceColors(this);
}

void StaticMeshComponent::SetInstanceColors(const std::vector<uint32_t>& colors)
{
    mInstanceColors = colors;
    GFX_UpdateStaticMeshCompResourceColors(this);
}

std::vector<uint32_t>& StaticMeshComponent::GetInstanceColors()
{
    return mInstanceColors;
}

bool StaticMeshComponent::HasBakedLighting() const
{
    return (mBakeLighting && mInstanceColors.size() > 0);
}

void StaticMeshComponent::RecreateCollisionShape()
{
    StaticMesh* staticMesh = mStaticMesh.Get<StaticMesh>();

    if (staticMesh != nullptr)
    {
        if (mUseTriangleCollision && staticMesh->GetTriangleCollisionShape())
        {
            glm::vec3 scale = GetAbsoluteScale();
            btVector3 btscale = btVector3(scale.x, scale.y, scale.z);
            btScaledBvhTriangleMeshShape* scaledTriangleShape = new btScaledBvhTriangleMeshShape(staticMesh->GetTriangleCollisionShape(), btscale);
            SetCollisionShape(scaledTriangleShape);
        }
        else
        {
            btCollisionShape* newShape = CloneCollisionShape(staticMesh->GetCollisionShape());
            SetCollisionShape(newShape);
        }
    }
    else
    {
        SetCollisionShape(PrimitiveComponent::GetEmptyCollisionShape());
    }
}