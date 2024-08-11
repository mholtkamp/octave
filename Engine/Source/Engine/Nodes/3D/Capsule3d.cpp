#include "Nodes/3D/Capsule3d.h"

#include "AssetManager.h"
#include "Renderer.h"

#include "BulletCollision/CollisionShapes/btBoxShape.h"

FORCE_LINK_DEF(Capsule3D);
DEFINE_NODE(Capsule3D, Primitive3D);

bool Capsule3D::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);
    OCT_ASSERT(prop != nullptr);
    Capsule3D* capsuleComp = static_cast<Capsule3D*>(prop->mOwner);
    bool success = false;

    if (prop->mName == "Height")
    {
        capsuleComp->SetHeight(*(float*)newValue);
        success = true;
    }
    else if (prop->mName == "Radius")
    {
        capsuleComp->SetRadius(*(float*)newValue);
        success = true;
    }

    return success;
}

const float Capsule3D::sDefaultHeight = 2.0f;
const float Capsule3D::sDefaultRadius = 1.0f;

Capsule3D::Capsule3D()
{
    mName = "Capsule";
    mHeight = sDefaultHeight;
    mRadius = sDefaultRadius;
}

Capsule3D::~Capsule3D()
{

}

const char* Capsule3D::GetTypeName() const
{
    return "Capsule";
}

void Capsule3D::GatherProperties(std::vector<Property>& outProps)
{
    Primitive3D::GatherProperties(outProps);

    SCOPED_CATEGORY("Capsule");

    outProps.push_back(Property(DatumType::Float, "Height", this, &mHeight, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Float, "Radius", this, &mRadius, 1, HandlePropChange));
}

void Capsule3D::GatherProxyDraws(std::vector<DebugDraw>& inoutDraws)
{
#if DEBUG_DRAW_ENABLED
    Primitive3D::GatherProxyDraws(inoutDraws);

    const float halfHeight = mHeight / 2.0f;
    const float hScale = mHeight / sDefaultHeight;
    const float rScale = mRadius / sDefaultRadius;
    const glm::vec4 color =
        (Renderer::Get()->GetDebugMode() == DEBUG_COLLISION) ?
        GetCollisionDebugColor() :
        glm::vec4(0.8f, 0.2f, 0.8f, 1.0f);

    {
        // Cylinder
        DebugDraw debugDraw;
        debugDraw.mMesh = LoadAsset<StaticMesh>("SM_CapsuleCylinder");
        debugDraw.mNode = this;
        debugDraw.mColor = color;
        debugDraw.mTransform = glm::scale(mTransform, { rScale, hScale, rScale });
        inoutDraws.push_back(debugDraw);
    }

    {
        // Top Cap
        DebugDraw debugDraw;
        debugDraw.mMesh = LoadAsset<StaticMesh>("SM_CapsuleCap");
        debugDraw.mNode = this;
        debugDraw.mColor = color;
        glm::mat4 trans = MakeTransform({ 0.0f, halfHeight, 0.0f }, {}, { rScale, rScale, rScale });
        debugDraw.mTransform = mTransform * trans;
        inoutDraws.push_back(debugDraw);
    }

    {
        // Bottom Cap
        DebugDraw debugDraw;
        debugDraw.mMesh = LoadAsset<StaticMesh>("SM_CapsuleCap");
        debugDraw.mNode = this;
        debugDraw.mColor = color;
        glm::mat4 trans = MakeTransform({ 0.0f, -halfHeight, 0.0f }, {180.0f, 0.0f, 0.0f}, { rScale, rScale, rScale });
        debugDraw.mTransform = mTransform * trans;
        inoutDraws.push_back(debugDraw);
    }

#endif
}

void Capsule3D::Create()
{
    Primitive3D::Create();

    btCapsuleShape* capsuleShape = new btCapsuleShape(mRadius, mHeight);
    SetCollisionShape(capsuleShape);

    UpdateRigidBody();
}

float Capsule3D::GetHeight() const
{
    return mHeight;
}

void Capsule3D::SetHeight(float height)
{
    if (mHeight != height)
    {
        mHeight = height;
        UpdateRigidBody();
    }
}

float Capsule3D::GetRadius() const
{
    return mRadius;
}

void Capsule3D::SetRadius(float radius)
{
    if (mRadius != radius)
    {
        mRadius = radius;
        UpdateRigidBody();
    }
}

void Capsule3D::UpdateRigidBody()
{
    EnableRigidBody(false);

    btCapsuleShape* capsuleShape = static_cast<btCapsuleShape*>(mCollisionShape);

    if (capsuleShape->getRadius() != mRadius)
    {
        // Workaround for issue where capsule margin can't be changed after creation.
        // The margin needs to be equal to the radius.
        DestroyComponentCollisionShape();
        btCapsuleShape* newCapsuleShape = new btCapsuleShape(mRadius, mHeight);
        SetCollisionShape(newCapsuleShape);
    }
    else
    {
        capsuleShape->setImplicitShapeDimensions({ mRadius, 0.5f * mHeight, mRadius });
        // setMargin() is NOP'd out for capsules so you CAN'T change it???
        // An easy fix is to remove the override version that ignores the margin change...
        // But for now (so we don't have to edit bullet source code), if the radius changes, we just recreate
        // the collision shape. See above...
        //capsuleShape->setMargin(mRadius);
    }

    EnableRigidBody(true);
}