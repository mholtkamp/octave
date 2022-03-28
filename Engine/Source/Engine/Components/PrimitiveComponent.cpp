#include "Components/PrimitiveComponent.h"
#include "World.h"
#include "Utilities.h"
#include "Log.h"
#include "Renderer.h"
#include "AssetManager.h"
#include "Assets/StaticMesh.h"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

#include <btBulletDynamicsCommon.h>

DEFINE_RTTI(PrimitiveComponent);

static btEmptyShape* sEmptyCollisionShape = nullptr;

#define UPDATE_RIGID_BODY_PROPERTY(primVariable, newValue, rigidBodyUpdate)    \
    {                                                                          \
        if (primVariable != newValue)                                          \
        {                                                                      \
            EnableRigidBody(false);                                            \
            primVariable = newValue;                                           \
            if (mRigidBody) rigidBodyUpdate;                                   \
            EnableRigidBody(true);                                             \
        }                                                                      \
    }

bool HandlePropChange(Datum* datum, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);
    assert(prop != nullptr);
    PrimitiveComponent* primComponent = static_cast<PrimitiveComponent*>(prop->mOwner);
    bool success = false;

    if (prop->mName == "Mass")
    {
        primComponent->SetMass(*static_cast<const float*>(newValue));
        success = true;
    }
    else if (prop->mName == "Restitution")
    {
        primComponent->SetRestitution(*static_cast<const float*>(newValue));
        success = true;
    }
    else if (prop->mName == "Friction")
    {
        primComponent->SetFriction(*static_cast<const float*>(newValue));
        success = true;
    }
    else if (prop->mName == "Rolling Friction")
    {
        primComponent->SetRollingFriction(*static_cast<const float*>(newValue));
        success = true;
    }
    else if (prop->mName == "Linear Damping")
    {
        primComponent->SetLinearDamping(*static_cast<const float*>(newValue));
        success = true;
    }
    else if (prop->mName == "Angular Damping")
    {
        primComponent->SetAngularDamping(*static_cast<const float*>(newValue));
        success = true;
    }
    else if (prop->mName == "Linear Factor")
    {
        primComponent->SetLinearFactor(*static_cast<const glm::vec3*>(newValue));
        success = true;
    }
    else if (prop->mName == "Angular Factor")
    {
        primComponent->SetAngularFactor(*static_cast<const glm::vec3*>(newValue));
        success = true;
    }
    else if (prop->mName == "Collision Group")
    {
        primComponent->SetCollisionGroup(*static_cast<const uint8_t*>(newValue));
        success = true;
    }
    else if (prop->mName == "Collision Mask")
    {
        primComponent->SetCollisionMask(*static_cast<const uint8_t*>(newValue));
        success = true;
    }
    else if (prop->mName == "Overlaps")
    {
        primComponent->EnableOverlaps(*static_cast<const bool*>(newValue));
        success = true;
    }
    else if (prop->mName == "Physics")
    {
        primComponent->EnablePhysics(*static_cast<const bool*>(newValue));
        success = true;
    }
    else if (prop->mName == "Collision")
    {
        primComponent->EnableCollision(*static_cast<const bool*>(newValue));
        success = true;
    }

    return success;
}

PrimitiveComponent::PrimitiveComponent() :
    mRigidBody(nullptr),
    mMotionState(nullptr),
    mCollisionShape(nullptr),
    mMass(1.0f),
    mRestitution(0.0f),
    mFriction(0.5f),
    mRollingFriction(0.0f),
    mLinearDamping(0.0f),
    mAngularDamping(0.0f),
    mLinearFactor(1.0f, 1.0f, 1.0f),
    mAngularFactor(1.0f, 1.0f, 1.0f),
    mCollisionGroup(ColGroup0),
    mCollisionMask(ColGroupAll),
    mPhysicsEnabled(false),
    mCollisionEnabled(false),
    mOverlapsEnabled(false),
    mCastShadows(false),
    mReceiveShadows(true),
    mReceiveSimpleShadows(true)
    //mBeginOverlapHandler(nullptr),
    //mEndOverlapHandler(nullptr),
    //mCollisionHandler(nullptr)
{

}

PrimitiveComponent::~PrimitiveComponent()
{

}

void PrimitiveComponent::Create()
{
    TransformComponent::Create();

    if (mCollisionShape == nullptr)
    {
        mCollisionShape = GetEmptyCollisionShape();
    }

    EnableRigidBody(true);
}

void PrimitiveComponent::Destroy()
{
    TransformComponent::Destroy();

    if (GetWorld() && IsRigidBodyInWorld())
    {
        GetWorld()->GetDynamicsWorld()->removeRigidBody(mRigidBody);
    }

    if (mRigidBody != nullptr)
    {
        delete mRigidBody;
        mRigidBody = nullptr;
    }

    if (mMotionState != nullptr)
    {
        delete mMotionState;
        mMotionState = nullptr;
    }

    DestroyComponentCollisionShape();
}

const char* PrimitiveComponent::GetTypeName() const
{
    return "Primitive";
}

bool PrimitiveComponent::IsPrimitiveComponent() const
{
    return true;
}

void PrimitiveComponent::Tick(float deltaTime)
{
    TransformComponent::Tick(deltaTime);

    if (mPhysicsEnabled)
    {
        if (mTransformDirty)
        {
            UpdateTransform(false);
        }
        else
        {
            // Sync the component transform with the physics transform
            glm::vec3 worldScale = GetAbsoluteScale();
            glm::mat4 physTransform(1);

            if (mPhysicsEnabled)
            {
                physTransform = mMotionState->GetTransform();
            }
            else
            {
                mRigidBody->getWorldTransform().getOpenGLMatrix(glm::value_ptr(physTransform));
            }

            physTransform = glm::scale(physTransform, worldScale);
            SetTransform(physTransform);
        }
    }
}

void PrimitiveComponent::GatherProperties(std::vector<Property>& outProps)
{
    TransformComponent::GatherProperties(outProps);

    outProps.push_back(Property(DatumType::Bool, "Physics", this, &mPhysicsEnabled, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Bool, "Collision", this, &mCollisionEnabled, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Bool, "Overlaps", this, &mOverlapsEnabled, 1, HandlePropChange));
    
    outProps.push_back(Property(DatumType::Bool, "Cast Shadows", this, &mCastShadows));
    outProps.push_back(Property(DatumType::Bool, "Receive Projected Shadows", this, &mReceiveShadows));
    outProps.push_back(Property(DatumType::Bool, "Receive Simple Shadows", this, &mReceiveSimpleShadows));

    outProps.push_back(Property(DatumType::Float, "Mass", this, &mMass, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Float, "Restitution", this, &mRestitution, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Float, "Friction", this, &mFriction, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Float, "Rolling Friction", this, &mRollingFriction, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Float, "Linear Damping", this, &mLinearDamping, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Float, "Angular Damping", this, &mAngularDamping, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Vector, "Linear Factor", this, &mLinearFactor, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Vector, "Angular Factor", this, &mAngularFactor, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Byte, "Collision Group", this, &mCollisionGroup, 1, HandlePropChange, (int32_t)ByteExtra::FlagWidget));
    outProps.push_back(Property(DatumType::Byte, "Collision Mask", this, &mCollisionMask, 1, HandlePropChange, (int32_t)ByteExtra::FlagWidget));
}

void PrimitiveComponent::LoadStream(Stream& stream)
{
    TransformComponent::LoadStream(stream);
    EnablePhysics(stream.ReadBool());
    EnableCollision(stream.ReadBool());
    EnableOverlaps(stream.ReadBool());
    mCastShadows = stream.ReadBool();
    mReceiveShadows = stream.ReadBool();
    mReceiveSimpleShadows = stream.ReadBool();
    SetMass(stream.ReadFloat());
    SetRestitution(stream.ReadFloat());
    SetFriction(stream.ReadFloat());
    SetRollingFriction(stream.ReadFloat());
    SetLinearDamping(stream.ReadFloat());
    SetAngularDamping(stream.ReadFloat());
    SetLinearFactor(stream.ReadVec3());
    SetAngularFactor(stream.ReadVec3());
    SetCollisionGroup(stream.ReadUint8());
    SetCollisionMask(stream.ReadUint8());
}

void PrimitiveComponent::SaveStream(Stream& stream)
{
    TransformComponent::SaveStream(stream);
    stream.WriteBool(mPhysicsEnabled);
    stream.WriteBool(mCollisionEnabled);
    stream.WriteBool(mOverlapsEnabled);
    stream.WriteBool(mCastShadows);
    stream.WriteBool(mReceiveShadows);
    stream.WriteBool(mReceiveSimpleShadows);
    stream.WriteFloat(mMass);
    stream.WriteFloat(mRestitution);
    stream.WriteFloat(mFriction);
    stream.WriteFloat(mRollingFriction);
    stream.WriteFloat(mLinearDamping);
    stream.WriteFloat(mAngularDamping);
    stream.WriteVec3(mLinearFactor);
    stream.WriteVec3(mAngularFactor);
    stream.WriteUint8(mCollisionGroup);
    stream.WriteUint8(mCollisionMask);
}

void PrimitiveComponent::SetOwner(Actor* owner)
{
    bool rigidBodyInWorld = IsRigidBodyInWorld();

    if (rigidBodyInWorld)
    {
        // Before switching to a new world, disable physics to remove component from dynamics world
        EnableRigidBody(false);
    }

    TransformComponent::SetOwner(owner);

    if (rigidBodyInWorld)
    {
        EnableRigidBody(true);
    }
}

void PrimitiveComponent::Render()
{

}

void PrimitiveComponent::UpdateTransform(bool updateChildren)
{
    bool updateRigidBody = (mPhysicsEnabled || mCollisionEnabled || mOverlapsEnabled) && mTransformDirty && IsRigidBodyInWorld();
    TransformComponent::UpdateTransform(updateChildren);
    
    if (updateRigidBody)
    {
        // Because updating transform is something that might happen very often
        // We only sync transform instead of calling Enable/DisableRigidBody
        btDynamicsWorld* dynamicsWorld = GetWorld()->GetDynamicsWorld();
        dynamicsWorld->removeRigidBody(mRigidBody);
        SyncRigidBodyTransform();
        mRigidBody->activate(true);
        dynamicsWorld->addRigidBody(mRigidBody, mCollisionGroup, mCollisionMask);
    }
}

void PrimitiveComponent::EnablePhysics(bool enable)
{
    if (mPhysicsEnabled != enable)
    {
        EnableRigidBody(false);
        mPhysicsEnabled = enable;

        if (enable)
        {
            // Lazily allocate the motion state the first time physics is enabled.
            mMotionState = new OctaveMotionState();

            if (mRigidBody != nullptr)
            {
                mRigidBody->setLinearVelocity(btVector3(0, 0, 0));
                mRigidBody->setAngularVelocity(btVector3(0, 0, 0));
                mRigidBody->setMotionState(mMotionState);
            }
        }

        EnableRigidBody(true);
    }
}

void PrimitiveComponent::EnableCollision(bool enable)
{
    if (mCollisionEnabled != enable)
    {
        EnableRigidBody(false);
        mCollisionEnabled = enable;
        EnableRigidBody(true);
    }
}

void PrimitiveComponent::EnableOverlaps(bool enable)
{
    if (mOverlapsEnabled != enable)
    {
        EnableRigidBody(false);
        mOverlapsEnabled = enable;
        EnableRigidBody(true);
    }
}

bool PrimitiveComponent::IsPhysicsEnabled() const
{
    return mPhysicsEnabled;
}

bool PrimitiveComponent::IsCollisionEnabled() const
{
    return mCollisionEnabled;
}

bool PrimitiveComponent::AreOverlapsEnabled() const
{
    return mOverlapsEnabled;
}

float PrimitiveComponent::GetMass() const
{
    return mMass;
}

float PrimitiveComponent::GetLinearDamping() const
{
    return mLinearDamping;
}

float PrimitiveComponent::GetAngularDamping() const
{
    return mAngularDamping;
}

float PrimitiveComponent::GetRestitution() const
{
    return mRestitution;
}

float PrimitiveComponent::GetFriction() const
{
    return mFriction;
}

float PrimitiveComponent::GetRollingFriction()
{
    return mRollingFriction;
}

glm::vec3 PrimitiveComponent::GetLinearFactor() const
{
    return mLinearFactor;
}

glm::vec3 PrimitiveComponent::GetAngularFactor() const
{
    return mAngularFactor;
}

uint8_t PrimitiveComponent::GetCollisionGroup() const
{
    return mCollisionGroup;
}

uint8_t PrimitiveComponent::GetCollisionMask() const
{
    return mCollisionMask;
}

void PrimitiveComponent::SetMass(float mass)
{
    if (mass != mMass)
    {
        // When changing mass, disable physics, then re-enable physics
        // to properly update the rigid body. According to this article: 
        // https://studiofreya.com/game-maker/bullet-physics/bullet-physics-how-to-change-body-mass/
        EnableRigidBody(false);
        mMass = mass;
        EnableRigidBody(true);
    }
}

void PrimitiveComponent::SetLinearDamping(float linearDamping)
{
    UPDATE_RIGID_BODY_PROPERTY
    (
        mLinearDamping,
        linearDamping,
        mRigidBody->setDamping(mLinearDamping, mAngularDamping)
    );
}

void PrimitiveComponent::SetAngularDamping(float angularDamping)
{
    UPDATE_RIGID_BODY_PROPERTY
    (
        mAngularDamping,
        angularDamping,
        mRigidBody->setDamping(mLinearDamping, mAngularDamping)
    );
}

void PrimitiveComponent::SetRestitution(float restitution)
{
    UPDATE_RIGID_BODY_PROPERTY
    (
        mRestitution,
        restitution,
        mRigidBody->setRestitution(restitution)
    );
}

void PrimitiveComponent::SetFriction(float friction)
{
    UPDATE_RIGID_BODY_PROPERTY
    (
        mFriction,
        friction,
        mRigidBody->setFriction(friction)
    );
}

void PrimitiveComponent::SetRollingFriction(float rollingFriction)
{
    UPDATE_RIGID_BODY_PROPERTY
    (
        mRollingFriction,
        rollingFriction,
        mRigidBody->setRollingFriction(rollingFriction)
    );
}

void PrimitiveComponent::SetLinearFactor(glm::vec3 linearFactor)
{
    UPDATE_RIGID_BODY_PROPERTY
    (
        mLinearFactor,
        linearFactor,
        mRigidBody->setLinearFactor({ linearFactor.x, linearFactor.y, linearFactor.z })
    );
}

void PrimitiveComponent::SetAngularFactor(glm::vec3 angularFactor)
{
    UPDATE_RIGID_BODY_PROPERTY
    (
        mAngularFactor,
        angularFactor,
        mRigidBody->setAngularFactor({ angularFactor.x, angularFactor.y, angularFactor.z })
    );
}

void PrimitiveComponent::SetCollisionGroup(uint8_t group)
{
    EnableRigidBody(false);
    mCollisionGroup = group;
    EnableRigidBody(true);
}

void PrimitiveComponent::SetCollisionMask(uint8_t mask)
{
    EnableRigidBody(false);
    mCollisionMask = mask;
    EnableRigidBody(true);
}

glm::vec3 PrimitiveComponent::GetLinearVelocity() const
{
    btVector3 linearVelocity;
    linearVelocity = mRigidBody->getLinearVelocity();
    return { linearVelocity.x(), linearVelocity.y(), linearVelocity.z() };
}

glm::vec3 PrimitiveComponent::GetAngularVelocity() const
{
    btVector3 angularVelocity;
    angularVelocity = mRigidBody->getAngularVelocity();
    return { angularVelocity.x(), angularVelocity.y(), angularVelocity.z() };
}

void PrimitiveComponent::AddLinearVelocity(glm::vec3 deltaVelocity)
{
    if (mRigidBody)
    {
        btVector3 delta = { deltaVelocity.x, deltaVelocity.y, deltaVelocity.z };
        mRigidBody->setLinearVelocity(mRigidBody->getLinearVelocity() + delta);
        mRigidBody->activate();
    }
}

void PrimitiveComponent::AddAngularVelocity(glm::vec3 deltaVelocity)
{
    if (mRigidBody)
    {
        btVector3 delta = { deltaVelocity.x, deltaVelocity.y, deltaVelocity.z };
        mRigidBody->setAngularVelocity(mRigidBody->getAngularVelocity() + delta);
        mRigidBody->activate();
    }
}

void PrimitiveComponent::SetLinearVelocity(glm::vec3 linearVelocity)
{
    if (mRigidBody)
    {
        btVector3 velocity = { linearVelocity.x, linearVelocity.y, linearVelocity.z };
        mRigidBody->setLinearVelocity(velocity);
        mRigidBody->activate();
    }
}

void PrimitiveComponent::SetAngularVelocity(glm::vec3 angularVelocity)
{
    if (mRigidBody)
    {
        btVector3 velocity = { angularVelocity.x, angularVelocity.y, angularVelocity.z };
        mRigidBody->setAngularVelocity(velocity);
        mRigidBody->activate();
    }
}

void PrimitiveComponent::AddForce(glm::vec3 force)
{
    if (mRigidBody)
    {
        btVector3 forceBt = { force.x, force.y, force.z };
        mRigidBody->applyCentralForce(forceBt);
        mRigidBody->activate();
    }
}

void PrimitiveComponent::AddImpulse(glm::vec3 impulse)
{
    if (mRigidBody)
    {
        btVector3 impulseBt = { impulse.x, impulse.y, impulse.z };
        mRigidBody->applyCentralImpulse(impulseBt);
        mRigidBody->activate();
    }
}

void PrimitiveComponent::ClearForces()
{
    if (mRigidBody)
    {
        mRigidBody->clearForces();
    }
}

void PrimitiveComponent::SyncRigidBodyTransform()
{
    if (mOwner != nullptr &&
        GetWorld() != nullptr)
    {
        if (mRigidBody != nullptr)
        {
            btTransform worldTransform;
            glm::vec3 worldPos;
            glm::quat worldRot;

            if (mParent != nullptr)
            {
                worldPos = GetAbsolutePosition();
                worldRot = GetAbsoluteRotationQuat();

                //worldPos = glm::vec3(mParent->GetTransform() * glm::vec4(mPosition.x, mPosition.y, mPosition.z, 1.0f));
                //worldRot = toquat(GetTransform) * mRotationQuat;
            }
            else
            {
                worldPos = mPosition;
                worldRot = mRotationQuat;
            }

            worldTransform.setOrigin(btVector3(worldPos.x, worldPos.y, worldPos.z));
            worldTransform.setRotation(btQuaternion(worldRot.x, worldRot.y, worldRot.z, worldRot.w));

            if (mPhysicsEnabled)
            {
                assert(mMotionState != nullptr);
                mMotionState->setWorldTransform(worldTransform);
            }

            mRigidBody->setWorldTransform(worldTransform);
        }

        if (mCollisionShape != nullptr)
        {
            glm::vec3 worldScale = GetAbsoluteScale();
            mCollisionShape->setLocalScaling(btVector3(worldScale.x, worldScale.y, worldScale.z));
        }
    }
}

void PrimitiveComponent::SyncRigidBodyMass()
{
    if (!mRigidBody)
        return;

    btCollisionShape* shape = GetCollisionShape();
    btVector3 localInertia(0, 0, 0);

    float rigidBodyMass = mPhysicsEnabled ? mMass : 0.0f;

    if (shape && shape->getShapeType() != EMPTY_SHAPE_PROXYTYPE)
    {
        shape->calculateLocalInertia(rigidBodyMass, localInertia);
    }

    mRigidBody->setMassProps(rigidBodyMass, localInertia);
}

void PrimitiveComponent::SyncCollisionFlags()
{
    if (!mRigidBody)
        return;

    int flags = mRigidBody->getCollisionFlags();

    if (mCollisionEnabled)
    {
        flags &= (~btCollisionObject::CF_NO_CONTACT_RESPONSE);
    }
    else
    {
        flags |= btCollisionObject::CF_NO_CONTACT_RESPONSE;
    }

    if (mOverlapsEnabled || mPhysicsEnabled)
    {
        flags &= (~btCollisionObject::CF_STATIC_OBJECT);
    }
    else
    {
        flags |= btCollisionObject::CF_STATIC_OBJECT;
    }

    // Need a custom material callback to handle internal edges.
    // See ContactAddedHandler in World.cpp
    if (mCollisionShape->getShapeType() == SCALED_TRIANGLE_MESH_SHAPE_PROXYTYPE)
    {
        flags |= btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK;
    }
    else
    {
        flags &= ~btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK;
    }

    mRigidBody->setCollisionFlags(flags);
}

void PrimitiveComponent::EnableCastShadows(bool enable)
{
    mCastShadows = enable;
}

bool PrimitiveComponent::ShouldCastShadows() const
{
    return mCastShadows;
}

void PrimitiveComponent::EnableReceiveShadows(bool enable)
{
    mReceiveShadows = enable;
}

bool PrimitiveComponent::ShouldReceiveShadows() const
{
    return mReceiveShadows;
}

void PrimitiveComponent::EnableReceiveSimpleShadows(bool enable)
{
    mReceiveSimpleShadows = enable;
}

bool PrimitiveComponent::ShouldReceiveSimpleShadows() const
{
    return mReceiveSimpleShadows;
}

VertexType PrimitiveComponent::GetVertexType() const
{
    return VertexType::Vertex;
}

btRigidBody* PrimitiveComponent::GetRigidBody()
{
    return mRigidBody;
}

btCollisionShape* PrimitiveComponent::GetCollisionShape()
{
    return mCollisionShape;
}

void PrimitiveComponent::SetCollisionShape(btCollisionShape* newShape)
{
    EnableRigidBody(false);

    DestroyComponentCollisionShape();
    mCollisionShape = newShape;

    if (mRigidBody)
    {
        mRigidBody->setCollisionShape(mCollisionShape);
    }

    EnableRigidBody(true);
}

bool PrimitiveComponent::SweepToWorldPosition(glm::vec3 position, SweepTestResult& outSweepResult, uint8_t mask)
{
    bool hit = false;
    glm::vec3 startPos = GetAbsolutePosition();

    if (mask == 0)
    {
        mask = GetCollisionMask();
    }

    GetWorld()->SweepTest(
        this,
        startPos,
        position,
        mask,
        outSweepResult);

    if (outSweepResult.mHitFraction < 1.0f)
    {
        hit = true;
        glm::vec3 delta = position - startPos;

        // Convex sweep solver uses 0.001f as an epsilon value. Do a little more to be safe.
        const float padding = 0.00101f;

        glm::vec3 fracDelta = outSweepResult.mHitFraction * delta;

        SetPosition(startPos + fracDelta + outSweepResult.mHitNormal * padding);

        GetOwner()->OnCollision(this, outSweepResult.mHitComponent, outSweepResult.mHitPosition, outSweepResult.mHitNormal, nullptr);
    }
    else
    {
        SetPosition(position);
    }

    return hit;
}

Bounds PrimitiveComponent::GetBounds() const
{
    // Transform the local bounds into world bounds.
    Bounds localBounds = GetLocalBounds();
    Bounds worldBounds;
    worldBounds.mCenter = mTransform * glm::vec4(localBounds.mCenter, 1.0f);

    glm::vec3 absScale = Maths::ExtractScale(mTransform);
    float maxScale = glm::max(glm::max(absScale.x, absScale.y), absScale.z);
    worldBounds.mRadius = maxScale * localBounds.mRadius;

    return worldBounds;
}

Bounds PrimitiveComponent::GetLocalBounds() const
{
    // Derived classes should implement a way of getting their local bounds.
    // By default PrimitiveComponent will create a huge bounding sphere so that it never really gets culled.
    Bounds retBounds;
    retBounds.mCenter = { 0.0f, 0.0f, 0.0f };
    retBounds.mRadius = LARGE_BOUNDS;
    return retBounds;
}

void PrimitiveComponent::GatherProxyDraws(std::vector<DebugDraw>& inoutDraws)
{
#if DEBUG_DRAW_ENABLED
    BoundsDebugMode boundsMode = Renderer::Get()->GetBoundsDebugMode();

    if (boundsMode == BoundsDebugMode::All
#if EDITOR
        || (boundsMode == BoundsDebugMode::Selected && GetWorld()->GetSelectedComponent() == this)
#endif
        )
    {
        Bounds worldBounds = GetBounds();
        
        AddDebugDraw(
            LoadAsset<StaticMesh>("SM_Sphere"),
            worldBounds.mCenter,
            glm::vec3(0.0f, 0.0f, 0.0f),
            { worldBounds.mRadius, worldBounds.mRadius ,worldBounds.mRadius },
            {0.21f, 0.63f, 0.37, 1.0f});
    }
#endif
}

btCollisionShape* PrimitiveComponent::GetEmptyCollisionShape()
{
    if (sEmptyCollisionShape == nullptr)
    {
        sEmptyCollisionShape = new btEmptyShape();
    }

    return sEmptyCollisionShape;
}

glm::vec4 PrimitiveComponent::GetCollisionDebugColor()
{
    // Override color based on overlaps/collision/physics
    glm::vec4 collisionColor = { 0.0f, 0.0f, 0.0f, 1.0f };

    if (IsCollisionEnabled())
    {
        collisionColor += glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
    }
    if (IsPhysicsEnabled())
    {
        collisionColor += glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
    }
    if (AreOverlapsEnabled())
    {
        collisionColor += glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    }

    return collisionColor;
}

bool PrimitiveComponent::IsRigidBodyInWorld() const
{
    return mRigidBody && mRigidBody->isInWorld();
}

void PrimitiveComponent::EnableRigidBody(bool enable)
{
    World* world = GetWorld();
    if (world == nullptr)
        return;

    if (!enable && IsRigidBodyInWorld())
    {
        world->GetDynamicsWorld()->removeRigidBody(mRigidBody);
    }
    
    if (enable && (mPhysicsEnabled || mCollisionEnabled || mOverlapsEnabled))
    {
        if (mRigidBody == nullptr)
        {
            // This primitive component requires a rigidbody.
            btVector3 localInertia(0, 0, 0);
            assert(mCollisionShape != nullptr);

            float rigidBodyMass = mPhysicsEnabled ? mMass : 0.0f;

            if (mPhysicsEnabled &&
                mCollisionShape->getShapeType() != EMPTY_SHAPE_PROXYTYPE)
            {
                mCollisionShape->calculateLocalInertia(rigidBodyMass, localInertia);
            }

            // A motion state should be created when physics is enabled.
            assert(mMotionState || !mPhysicsEnabled);

            btRigidBody::btRigidBodyConstructionInfo rbInfo(rigidBodyMass, mMotionState, mCollisionShape, localInertia);
            mRigidBody = new btRigidBody(rbInfo);
            mRigidBody->setUserPointer(this);
        }

        if (!IsRigidBodyInWorld())
        {
            SyncRigidBodyMass();
            SyncCollisionFlags();
            SyncRigidBodyTransform();

            mRigidBody->activate(true);

            btDynamicsWorld* dynamicsWorld = world->GetDynamicsWorld();
            dynamicsWorld->addRigidBody(mRigidBody, mCollisionGroup, mCollisionMask);
        }
    }
}

void PrimitiveComponent::DestroyComponentCollisionShape()
{
    if (mCollisionShape != nullptr &&
        mCollisionShape != GetEmptyCollisionShape())
    {
        DestroyCollisionShape(mCollisionShape);
        mCollisionShape = nullptr;
    }
}
