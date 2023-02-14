#include "Components/BoxComponent.h"

#include "AssetManager.h"
#include "Renderer.h"

#include "BulletCollision/CollisionShapes/btBoxShape.h"

FORCE_LINK_DEF(BoxComponent);
DEFINE_COMPONENT(BoxComponent);

static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);
    OCT_ASSERT(prop != nullptr);
    BoxComponent* boxComp = static_cast<BoxComponent*>(prop->mOwner);
    bool success = false;

    if (prop->mName == "Extents")
    {
        boxComp->SetExtents(*(glm::vec3*)newValue);
        success = true;
    }

    return success;
}

const float BoxComponent::sDefaultExtent = 2.0f;

BoxComponent::BoxComponent()
{
    mName = "Box Component";
    mExtents = { sDefaultExtent, sDefaultExtent, sDefaultExtent };
}

BoxComponent::~BoxComponent()
{

}

const char* BoxComponent::GetTypeName() const
{
    return "Box";
}

void BoxComponent::GatherProperties(std::vector<Property>& outProps)
{
    PrimitiveComponent::GatherProperties(outProps);
    outProps.push_back(Property(DatumType::Vector, "Extents", this, &mExtents, 1, HandlePropChange));
}

void BoxComponent::GatherProxyDraws(std::vector<DebugDraw>& inoutDraws)
{
#if DEBUG_DRAW_ENABLED
    PrimitiveComponent::GatherProxyDraws(inoutDraws);

    if (GetType() == BoxComponent::GetStaticType())
    {
        glm::vec3 extentScale = { mExtents.x / sDefaultExtent, mExtents.y / sDefaultExtent, mExtents.z / sDefaultExtent };
        glm::vec4 color = 
            (Renderer::Get()->GetDebugMode() == DEBUG_COLLISION) ?
            GetCollisionDebugColor() :
            glm::vec4(0.8f, 0.2f, 0.8f, 1.0f);

        DebugDraw debugDraw;
        debugDraw.mMesh = LoadAsset<StaticMesh>("SM_Cube");
        debugDraw.mActor = GetOwner();
        debugDraw.mComponent = this;
        debugDraw.mColor = color;
        debugDraw.mTransform = glm::scale(mTransform, extentScale);
        inoutDraws.push_back(debugDraw);
    }
#endif
}

void BoxComponent::Create()
{
    PrimitiveComponent::Create();

    btVector3 halfExtents = btVector3(mExtents.x, mExtents.y, mExtents.z) / 2.0f;
    btBoxShape* boxShape = new btBoxShape(halfExtents);
    SetCollisionShape(boxShape);

    UpdateRigidBody();
}

void BoxComponent::SaveStream(Stream& stream)
{
    PrimitiveComponent::SaveStream(stream);
    stream.WriteVec3(mExtents);
}

void BoxComponent::LoadStream(Stream& stream)
{
    PrimitiveComponent::LoadStream(stream);
    SetExtents(stream.ReadVec3());
}

glm::vec3 BoxComponent::GetExtents() const
{
    return mExtents;
}

void BoxComponent::SetExtents(glm::vec3 extents)
{
    if (mExtents != extents)
    {
        mExtents = extents;
        UpdateRigidBody();
    }
}

void BoxComponent::UpdateRigidBody()
{
    EnableRigidBody(false);

    btBoxShape* boxShape = static_cast<btBoxShape*>(mCollisionShape);

    // Unfortunately there isn't an easy way to reset the safe margins when 
    // setting the implicit safe dimensions, so I copied the code from the constructor 
    // basically to ensure the safe margin gets updated.
    btVector3 halfExtents = btVector3(mExtents.x, mExtents.y, mExtents.z) / 2.0f;
    boxShape->setSafeMargin(halfExtents);
    btVector3 margin(boxShape->getMargin(), boxShape->getMargin(), boxShape->getMargin());
    boxShape->setImplicitShapeDimensions((halfExtents * boxShape->getLocalScaling()) - margin);

    EnableRigidBody(true);
}