#include "Components/SphereComponent.h"

#include "AssetManager.h"
#include "Renderer.h"

#include "BulletCollision/CollisionShapes/btSphereShape.h"

FORCE_LINK_DEF(SphereComponent);
DEFINE_COMPONENT(SphereComponent);

static bool HandlePropChange(Datum* datum, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);
    OCT_ASSERT(prop != nullptr);
    SphereComponent* sphereComp = static_cast<SphereComponent*>(prop->mOwner);
    bool success = false;

    if (prop->mName == "Radius")
    {
        sphereComp->SetRadius(*(float*)newValue);
        success = true;
    }

    return success;
}

const float SphereComponent::sDefaultRadius = 1.0f;

SphereComponent::SphereComponent()
{
    mName = "Sphere Component";
    mRadius = sDefaultRadius;
}

SphereComponent::~SphereComponent()
{

}

const char* SphereComponent::GetTypeName() const
{
    return "Sphere";
}

void SphereComponent::GatherProperties(std::vector<Property>& outProps)
{
    PrimitiveComponent::GatherProperties(outProps);
    outProps.push_back(Property(DatumType::Float, "Radius", this, &mRadius, 1, HandlePropChange));
}

void SphereComponent::GatherProxyDraws(std::vector<DebugDraw>& inoutDraws)
{
#if DEBUG_DRAW_ENABLED
    PrimitiveComponent::GatherProxyDraws(inoutDraws);

    if (GetType() == SphereComponent::GetStaticType())
    {
        float radiusScale = (mRadius / sDefaultRadius);
        glm::vec4 color =
            (Renderer::Get()->GetDebugMode() == DEBUG_COLLISION) ?
            GetCollisionDebugColor() :
            glm::vec4(0.8f, 0.2f, 0.8f, 1.0f);

        DebugDraw debugDraw;
        debugDraw.mMesh = LoadAsset<StaticMesh>("SM_Sphere");
        debugDraw.mActor = GetOwner();
        debugDraw.mComponent = this;
        debugDraw.mColor = color;
        debugDraw.mTransform = glm::scale(mTransform, { radiusScale, radiusScale, radiusScale });

        inoutDraws.push_back(debugDraw);
    }
#endif
}

void SphereComponent::Create()
{
    PrimitiveComponent::Create();
    SetCollisionShape(new btSphereShape(mRadius));
    UpdateRigidBody();
}

void SphereComponent::SaveStream(Stream& stream)
{
    PrimitiveComponent::SaveStream(stream);
    stream.WriteFloat(mRadius);
}

void SphereComponent::LoadStream(Stream& stream)
{
    PrimitiveComponent::LoadStream(stream);
    SetRadius(stream.ReadFloat());
}

float SphereComponent::GetRadius() const
{
    return mRadius;
}

void SphereComponent::SetRadius(float radius)
{
    if (mRadius != radius)
    {
        mRadius = radius;
        UpdateRigidBody();
    }
}

void SphereComponent::UpdateRigidBody()
{
    EnableRigidBody(false);

    btSphereShape* sphereShape = static_cast<btSphereShape*>(mCollisionShape);

    // This logic is copied from the btSphereShape constructor
    sphereShape->setImplicitShapeDimensions({ mRadius, 0.0f, 0.0f });
    sphereShape->setMargin(mRadius);

    EnableRigidBody(true);
}
