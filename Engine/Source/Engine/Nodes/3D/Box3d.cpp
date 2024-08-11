#include "Nodes/3D/Box3d.h"

#include "AssetManager.h"
#include "Renderer.h"

#include "BulletCollision/CollisionShapes/btBoxShape.h"

FORCE_LINK_DEF(Box3D);
DEFINE_NODE(Box3D, Primitive3D);

bool Box3D::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);
    OCT_ASSERT(prop != nullptr);
    Box3D* boxComp = static_cast<Box3D*>(prop->mOwner);
    bool success = false;

    if (prop->mName == "Extents")
    {
        boxComp->SetExtents(*(glm::vec3*)newValue);
        success = true;
    }

    return success;
}

const float Box3D::sDefaultExtent = 2.0f;

Box3D::Box3D()
{
    mName = "Box";
    mExtents = { sDefaultExtent, sDefaultExtent, sDefaultExtent };
}

Box3D::~Box3D()
{

}

const char* Box3D::GetTypeName() const
{
    return "Box";
}

void Box3D::GatherProperties(std::vector<Property>& outProps)
{
    Primitive3D::GatherProperties(outProps);

    SCOPED_CATEGORY("Box");

    outProps.push_back(Property(DatumType::Vector, "Extents", this, &mExtents, 1, HandlePropChange));
}

void Box3D::GatherProxyDraws(std::vector<DebugDraw>& inoutDraws)
{
#if DEBUG_DRAW_ENABLED
    Primitive3D::GatherProxyDraws(inoutDraws);

    glm::vec3 extentScale = { mExtents.x / sDefaultExtent, mExtents.y / sDefaultExtent, mExtents.z / sDefaultExtent };
    glm::vec4 color = 
        (Renderer::Get()->GetDebugMode() == DEBUG_COLLISION) ?
        GetCollisionDebugColor() :
        glm::vec4(0.8f, 0.2f, 0.8f, 1.0f);

    DebugDraw debugDraw;
    debugDraw.mMesh = LoadAsset<StaticMesh>("SM_Cube");
    debugDraw.mNode = this;
    debugDraw.mColor = color;
    debugDraw.mTransform = glm::scale(mTransform, extentScale);
    inoutDraws.push_back(debugDraw);

#endif
}

void Box3D::Create()
{
    Primitive3D::Create();

    btVector3 halfExtents = btVector3(mExtents.x, mExtents.y, mExtents.z) / 2.0f;
    btBoxShape* boxShape = new btBoxShape(halfExtents);
    SetCollisionShape(boxShape);

    UpdateRigidBody();
}

glm::vec3 Box3D::GetExtents() const
{
    return mExtents;
}

void Box3D::SetExtents(glm::vec3 extents)
{
    if (mExtents != extents)
    {
        mExtents = extents;
        UpdateRigidBody();
    }
}

void Box3D::UpdateRigidBody()
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