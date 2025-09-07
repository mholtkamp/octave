#include "Nodes/3D/Sphere3d.h"

#include "AssetManager.h"
#include "Renderer.h"

#include "BulletCollision/CollisionShapes/btSphereShape.h"

FORCE_LINK_DEF(Sphere3D);
DEFINE_NODE(Sphere3D, Primitive3D);

bool Sphere3D::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);
    OCT_ASSERT(prop != nullptr);
    Sphere3D* sphereComp = static_cast<Sphere3D*>(prop->mOwner);
    bool success = false;

    if (prop->mName == "Radius")
    {
        sphereComp->SetRadius(*(float*)newValue);
        success = true;
    }

    return success;
}

const float Sphere3D::sDefaultRadius = 1.0f;

Sphere3D::Sphere3D()
{
    mName = "Sphere";
    mRadius = sDefaultRadius;
}

Sphere3D::~Sphere3D()
{

}

const char* Sphere3D::GetTypeName() const
{
    return "Sphere";
}

void Sphere3D::GatherProperties(std::vector<Property>& outProps)
{
    Primitive3D::GatherProperties(outProps);

    SCOPED_CATEGORY("Sphere");
    outProps.push_back(Property(DatumType::Float, "Radius", this, &mRadius, 1, HandlePropChange));
}

void Sphere3D::GatherProxyDraws(std::vector<DebugDraw>& inoutDraws)
{
#if DEBUG_DRAW_ENABLED
    Primitive3D::GatherProxyDraws(inoutDraws);

    float radiusScale = (mRadius / sDefaultRadius);
    glm::vec4 color =
        (Renderer::Get()->GetDebugMode() == DEBUG_COLLISION) ?
        GetCollisionDebugColor() :
        glm::vec4(0.8f, 0.2f, 0.8f, 1.0f);

    DebugDraw debugDraw;
    debugDraw.mMesh = LoadAsset<StaticMesh>("SM_Sphere");
    debugDraw.mNode = this;
    debugDraw.mColor = color;
    debugDraw.mTransform = glm::scale(mTransform, { radiusScale, radiusScale, radiusScale });

    inoutDraws.push_back(debugDraw);

#endif
}

void Sphere3D::Create()
{
    Primitive3D::Create();
    SetCollisionShape(new btSphereShape(mRadius));
    UpdateRigidBody();
}

float Sphere3D::GetRadius() const
{
    return mRadius;
}

void Sphere3D::SetRadius(float radius)
{
    if (mRadius != radius)
    {
        mRadius = radius;
        UpdateRigidBody();
    }
}

void Sphere3D::UpdateRigidBody()
{
    EnableRigidBody(false);

    btSphereShape* sphereShape = static_cast<btSphereShape*>(mCollisionShape);

    // This logic is copied from the btSphereShape constructor
    sphereShape->setImplicitShapeDimensions({ mRadius, 0.0f, 0.0f });
    sphereShape->setMargin(mRadius);

    EnableRigidBody(true);
}
