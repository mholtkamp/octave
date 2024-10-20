#include "Nodes/3D/StaticMesh3d.h"

#include "Assets/StaticMesh.h"
#include "Renderer.h"
#include "AssetManager.h"
#include "Log.h"
#include "Utilities.h"

#include "Graphics/Graphics.h"

#include <btBulletDynamicsCommon.h>

FORCE_LINK_DEF(StaticMesh3D);
DEFINE_NODE(StaticMesh3D, Mesh3D);

bool StaticMesh3D::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);
    OCT_ASSERT(prop != nullptr);
    StaticMesh3D* meshComp = static_cast<StaticMesh3D*>(prop->mOwner);
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

StaticMesh3D::StaticMesh3D() :
    mStaticMesh(nullptr),
    mUseTriangleCollision(false),
    mBakeLighting(false),
    mHasBakedLighting(false)
{
    mName = "Static Mesh";
}

StaticMesh3D::~StaticMesh3D()
{

}

const char* StaticMesh3D::GetTypeName() const
{
    return "StaticMesh";
}

void StaticMesh3D::GatherProperties(std::vector<Property>& outProps)
{
    Mesh3D::GatherProperties(outProps);

    SCOPED_CATEGORY("Mesh");

    static bool sFakeBool = false;
    outProps.push_back(Property(DatumType::Asset, "Static Mesh", this, &mStaticMesh, 1, HandlePropChange, int32_t(StaticMesh::GetStaticType())));
    outProps.push_back(Property(DatumType::Bool, "Use Triangle Collision", this, &mUseTriangleCollision, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Bool, "Bake Lighting", this, &mBakeLighting, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Bool, "Clear Baked Lighting", this, &sFakeBool, 1, HandlePropChange));
}

void StaticMesh3D::Create()
{
    Mesh3D::Create();
    GFX_CreateStaticMeshCompResource(this);
    SetStaticMesh(GetDefaultMesh());
}

void StaticMesh3D::Destroy()
{
    Mesh3D::Destroy();
    GFX_DestroyStaticMeshCompResource(this);
}

StaticMeshCompResource* StaticMesh3D::GetResource()
{
    return &mResource;
}

void StaticMesh3D::SaveStream(Stream& stream, Platform platform)
{
    Mesh3D::SaveStream(stream, platform);

    uint32_t numInstanceColors = (uint32_t)mInstanceColors.size();
    stream.WriteUint32(numInstanceColors);
    for (uint32_t i = 0; i < numInstanceColors; ++i)
    {
        stream.WriteUint32(mInstanceColors[i]);
    }
}

void StaticMesh3D::LoadStream(Stream& stream, Platform platform, uint32_t version)
{
    Mesh3D::LoadStream(stream, platform, version);
    
    // Load instance colors after setting the static mesh. Otherwise it will clear.
    uint32_t numInstanceColors = stream.ReadUint32();
    mInstanceColors.resize(numInstanceColors);
    for (uint32_t i = 0; i < numInstanceColors; ++i)
    {
        mInstanceColors[i] = stream.ReadUint32();

#if PLATFORM_DOLPHIN
        ReverseColorUint32(mInstanceColors[i]);
#endif
    }

    GFX_UpdateStaticMeshCompResourceColors(this);
}

bool StaticMesh3D::IsStaticMesh3D() const
{
    return true;
}

bool StaticMesh3D::IsSkeletalMesh3D() const
{
    return false;
}

void StaticMesh3D::SetStaticMesh(StaticMesh* staticMesh)
{
    if (mStaticMesh.Get() != staticMesh)
    {
        mStaticMesh = staticMesh;
        RecreateCollisionShape();
        ClearInstanceColors();
    }
}

StaticMesh* StaticMesh3D::GetStaticMesh()
{
    return mStaticMesh.Get<StaticMesh>();
}

void StaticMesh3D::SetUseTriangleCollision(bool triangleCol)
{
    if (mUseTriangleCollision != triangleCol)
    {
        mUseTriangleCollision = triangleCol;
        RecreateCollisionShape();
    }
}

bool StaticMesh3D::GetUseTriangleCollision() const
{
    return mUseTriangleCollision;
}

void StaticMesh3D::SetBakeLighting(bool bake)
{
    mBakeLighting = bake;
}

bool StaticMesh3D::GetBakeLighting() const
{
    return mBakeLighting;
}

Material* StaticMesh3D::GetMaterial()
{
    Material* mat = mMaterialOverride.Get<Material>();

    if (!mat && mStaticMesh.Get())
    {
        mat = mStaticMesh.Get<StaticMesh>()->GetMaterial();
    }

    return mat;
}

void StaticMesh3D::Render()
{
    GFX_DrawStaticMeshComp(this);
}

VertexType StaticMesh3D::GetVertexType() const
{
    VertexType retType = VertexType::Vertex;

    if (mStaticMesh.Get<StaticMesh>() != nullptr && 
        mStaticMesh.Get<StaticMesh>()->HasVertexColor())
    {
        retType = VertexType::VertexColor;
    }

    return retType;
}

void StaticMesh3D::DrawDebugCollision(std::vector<DebugDraw>& inoutDraws, btCollisionShape* collisionShape, const glm::mat4& parentTransform)
{
    DebugDrawCollisionShape(collisionShape, this, parentTransform, &inoutDraws);
}

void StaticMesh3D::GatherProxyDraws(std::vector<DebugDraw>& inoutDraws)
{
#if DEBUG_DRAW_ENABLED
    Mesh3D::GatherProxyDraws(inoutDraws);

    if (Renderer::Get()->GetDebugMode() == DEBUG_COLLISION)
    {
        StaticMesh* staticMesh = mStaticMesh.Get<StaticMesh>();

        if (staticMesh != nullptr &&
            mCollisionShape != nullptr)
        {
            DrawDebugCollision(inoutDraws, mCollisionShape, mTransform);
        }
    }
#endif
}

Bounds StaticMesh3D::GetLocalBounds() const
{
    if (mStaticMesh != nullptr)
    {
        return mStaticMesh.Get<StaticMesh>()->GetBounds();
    }
    else
    {
        return Mesh3D::GetLocalBounds();
    }
}

void StaticMesh3D::ClearInstanceColors()
{
    mInstanceColors.clear();
    mHasBakedLighting = false;
    GFX_UpdateStaticMeshCompResourceColors(this);
}

void StaticMesh3D::SetInstanceColors(const std::vector<uint32_t>& colors, bool bakedLighting)
{
    mInstanceColors = colors;
    mHasBakedLighting = bakedLighting;
    GFX_UpdateStaticMeshCompResourceColors(this);
}

std::vector<uint32_t>& StaticMesh3D::GetInstanceColors()
{
    return mInstanceColors;
}

bool StaticMesh3D::HasBakedLighting() const
{
    return mHasBakedLighting;
}

bool StaticMesh3D::HasInstanceColors() const
{
    return (mInstanceColors.size() > 0);
}

void StaticMesh3D::RecreateCollisionShape()
{
    StaticMesh* staticMesh = mStaticMesh.Get<StaticMesh>();

    if (staticMesh != nullptr)
    {
        if (mUseTriangleCollision && 
            staticMesh->IsTriangleCollisionMeshEnabled() &&
            staticMesh->GetTriangleCollisionShape())
        {
            glm::vec3 scale = GetWorldScale();
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
        SetCollisionShape(Primitive3D::GetEmptyCollisionShape());
    }
}