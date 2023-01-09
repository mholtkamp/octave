#include "Components/CapsuleComponent.h"

#include "AssetManager.h"
#include "Renderer.h"

#include "BulletCollision/CollisionShapes/btBoxShape.h"

FORCE_LINK_DEF(CapsuleComponent);
DEFINE_COMPONENT(CapsuleComponent);

static bool HandlePropChange(Datum* datum, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);
    OCT_ASSERT(prop != nullptr);
    CapsuleComponent* capsuleComp = static_cast<CapsuleComponent*>(prop->mOwner);
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

const float CapsuleComponent::sDefaultHeight = 2.0f;
const float CapsuleComponent::sDefaultRadius = 1.0f;

CapsuleComponent::CapsuleComponent()
{
    mName = "Capsule Component";
    mHeight = sDefaultHeight;
    mRadius = sDefaultRadius;
}

CapsuleComponent::~CapsuleComponent()
{

}

const char* CapsuleComponent::GetTypeName() const
{
    return "Capsule";
}

void CapsuleComponent::GatherProperties(std::vector<Property>& outProps)
{
    PrimitiveComponent::GatherProperties(outProps);
    outProps.push_back(Property(DatumType::Float, "Height", this, &mHeight, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Float, "Radius", this, &mRadius, 1, HandlePropChange));
}

void CapsuleComponent::GatherProxyDraws(std::vector<DebugDraw>& inoutDraws)
{
#if DEBUG_DRAW_ENABLED
    PrimitiveComponent::GatherProxyDraws(inoutDraws);

    if (GetType() == CapsuleComponent::GetStaticType())
    {
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
            debugDraw.mActor = GetOwner();
            debugDraw.mComponent = this;
            debugDraw.mColor = color;
            debugDraw.mTransform = glm::scale(mTransform, { rScale, hScale, rScale });
            inoutDraws.push_back(debugDraw);
        }

        {
            // Top Cap
            DebugDraw debugDraw;
            debugDraw.mMesh = LoadAsset<StaticMesh>("SM_CapsuleCap");
            debugDraw.mActor = GetOwner();
            debugDraw.mComponent = this;
            debugDraw.mColor = color;
            glm::mat4 trans = MakeTransform({ 0.0f, halfHeight, 0.0f }, {}, { rScale, rScale, rScale });
            debugDraw.mTransform = mTransform * trans;
            inoutDraws.push_back(debugDraw);
        }

        {
            // Bottom Cap
            DebugDraw debugDraw;
            debugDraw.mMesh = LoadAsset<StaticMesh>("SM_CapsuleCap");
            debugDraw.mActor = GetOwner();
            debugDraw.mComponent = this;
            debugDraw.mColor = color;
            glm::mat4 trans = MakeTransform({ 0.0f, -halfHeight, 0.0f }, {180.0f, 0.0f, 0.0f}, { rScale, rScale, rScale });
            debugDraw.mTransform = mTransform * trans;
            inoutDraws.push_back(debugDraw);
        }
    }
#endif
}

void CapsuleComponent::Create()
{
    PrimitiveComponent::Create();

    btCapsuleShape* capsuleShape = new btCapsuleShape(mRadius, mHeight);
    SetCollisionShape(capsuleShape);

    UpdateRigidBody();
}

void CapsuleComponent::SaveStream(Stream& stream)
{
    PrimitiveComponent::SaveStream(stream);
    stream.WriteFloat(mHeight);
    stream.WriteFloat(mRadius);
}

void CapsuleComponent::LoadStream(Stream& stream)
{
    PrimitiveComponent::LoadStream(stream);
    SetHeight(stream.ReadFloat());
    SetRadius(stream.ReadFloat());
}

float CapsuleComponent::GetHeight() const
{
    return mHeight;
}

void CapsuleComponent::SetHeight(float height)
{
    if (mHeight != height)
    {
        mHeight = height;
        UpdateRigidBody();
    }
}

float CapsuleComponent::GetRadius() const
{
    return mRadius;
}

void CapsuleComponent::SetRadius(float radius)
{
    if (mRadius != radius)
    {
        mRadius = radius;
        UpdateRigidBody();
    }
}

void CapsuleComponent::UpdateRigidBody()
{
    EnableRigidBody(false);

    btCapsuleShape* capsuleShape = static_cast<btCapsuleShape*>(mCollisionShape);
    capsuleShape->setImplicitShapeDimensions({mRadius, 0.5f * mHeight, mRadius});

    EnableRigidBody(true);
}