#include "Nodes/3D/Primitive3d.h"
#include "World.h"
#include "Utilities.h"
#include "Log.h"
#include "Maths.h"
#include "Renderer.h"
#include "AssetManager.h"
#include "Assets/StaticMesh.h"

#include <btBulletDynamicsCommon.h>

#if EDITOR
#include "EditorState.h"
#endif

DEFINE_RTTI(Primitive3D);

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

bool Primitive3D::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);
    OCT_ASSERT(prop != nullptr);
    Primitive3D* primComponent = static_cast<Primitive3D*>(prop->mOwner);
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

Primitive3D::Primitive3D()
{

}

Primitive3D::~Primitive3D()
{

}

void Primitive3D::Create()
{
    Node3D::Create();

    if (mCollisionShape == nullptr)
    {
        mCollisionShape = GetEmptyCollisionShape();
    }

    EnableRigidBody(true);
}

void Primitive3D::Destroy()
{
    Node3D::Destroy();

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

const char* Primitive3D::GetTypeName() const
{
    return "Primitive";
}

bool Primitive3D::IsPrimitive3D() const
{
    return true;
}

void Primitive3D::Tick(float deltaTime)
{
    Node3D::Tick(deltaTime);

    bool gameTickEnabled = IsGameTickEnabled();

    if (gameTickEnabled && mPhysicsEnabled)
    {
        if (mTransformDirty)
        {
            UpdateTransform(false);
        }
        else
        {
            // Sync the component transform with the physics transform
            glm::vec3 worldScale = GetWorldScale();
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

            // Do not call Primitive3D's SetTransform, because it will
            // remove / add the rigidbody to the world, which will mess up its velocity/acceleration.
            // In this case, we just want to update our position/rotation/scale from the new transform
            // and also dirty child transforms.
            Node3D::SetTransform(physTransform);
        }
    }
}

void Primitive3D::GatherProperties(std::vector<Property>& outProps)
{
    Node3D::GatherProperties(outProps);

    SCOPED_CATEGORY("Primitive");

    outProps.push_back(Property(DatumType::Bool, "Physics", this, &mPhysicsEnabled, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Bool, "Collision", this, &mCollisionEnabled, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Bool, "Overlaps", this, &mOverlapsEnabled, 1, HandlePropChange));
    
    outProps.push_back(Property(DatumType::Bool, "Cast Shadows", this, &mCastShadows));
    outProps.push_back(Property(DatumType::Bool, "Receive Projected Shadows", this, &mReceiveShadows));
    outProps.push_back(Property(DatumType::Bool, "Receive Simple Shadows", this, &mReceiveSimpleShadows));

    outProps.push_back(Property(DatumType::Float, "Cull Distance", this, &mCullDistance));

    outProps.push_back(Property(DatumType::Float, "Mass", this, &mMass, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Float, "Restitution", this, &mRestitution, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Float, "Friction", this, &mFriction, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Byte, "Collision Group", this, &mCollisionGroup, 1, HandlePropChange, (int32_t)ByteExtra::FlagWidget));
    outProps.push_back(Property(DatumType::Byte, "Collision Mask", this, &mCollisionMask, 1, HandlePropChange, (int32_t)ByteExtra::FlagWidget));
    outProps.push_back(Property(DatumType::Byte, "Lighting Channels", this, &mLightingChannels, 1, nullptr, (int32_t)ByteExtra::FlagWidget));
}

void Primitive3D::SetWorld(World* world)
{
    // TODO-NODE: I am attempting to simplify this code. Does it still work?
#if 1
    EnableRigidBody(false);
    Node3D::SetWorld(world);
    EnableRigidBody(true);
#else
    bool rigidBodyInWorld = IsRigidBodyInWorld();

    if (rigidBodyInWorld)
    {
        // Before switching to a new world, disable physics to remove component from dynamics world
        EnableRigidBody(false);
    }

    Node3D::SetWorld(world);

    if (rigidBodyInWorld)
    {
        EnableRigidBody(true);
    }
#endif
}

void Primitive3D::Render()
{

}

void Primitive3D::UpdateTransform(bool updateChildren)
{
    bool updateRigidBody = (mPhysicsEnabled || mCollisionEnabled || mOverlapsEnabled) && mTransformDirty && IsRigidBodyInWorld();
    Node3D::UpdateTransform(updateChildren);
    
    if (updateRigidBody)
    {
        FullSyncRigidBodyTransform();
    }
}

void Primitive3D::SetTransform(const glm::mat4& transform)
{
    Node3D::SetTransform(transform);

    if (IsRigidBodyInWorld())
    {
        FullSyncRigidBodyTransform();
    }
}

void Primitive3D::EnablePhysics(bool enable)
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

void Primitive3D::EnableCollision(bool enable)
{
    if (mCollisionEnabled != enable)
    {
        EnableRigidBody(false);
        mCollisionEnabled = enable;
        EnableRigidBody(true);
    }
}

void Primitive3D::EnableOverlaps(bool enable)
{
    if (mOverlapsEnabled != enable)
    {
        EnableRigidBody(false);
        mOverlapsEnabled = enable;
        EnableRigidBody(true);
    }
}

bool Primitive3D::IsPhysicsEnabled() const
{
    return mPhysicsEnabled;
}

bool Primitive3D::IsCollisionEnabled() const
{
    return mCollisionEnabled;
}

bool Primitive3D::AreOverlapsEnabled() const
{
    return mOverlapsEnabled;
}

float Primitive3D::GetCullDistance() const
{
    return mCullDistance;
}

void Primitive3D::SetCullDistance(float cullDistance)
{
    mCullDistance = cullDistance;
}

float Primitive3D::GetMass() const
{
    return mMass;
}

float Primitive3D::GetLinearDamping() const
{
    return mRigidBody ? mRigidBody->getLinearDamping() : 0.0f;
}

float Primitive3D::GetAngularDamping() const
{
    return mRigidBody ? mRigidBody->getAngularDamping() : 0.0f;
}

float Primitive3D::GetRestitution() const
{
    return mRestitution;
}

float Primitive3D::GetFriction() const
{
    return mFriction;
}

float Primitive3D::GetRollingFriction()
{
    return mRigidBody ? mRigidBody->getRollingFriction() : 0.0f;
}

glm::vec3 Primitive3D::GetLinearFactor() const
{
    return mRigidBody ? BulletToGlm(mRigidBody->getLinearFactor()) : glm::vec3(1.0f, 1.0f, 1.0f);
}

glm::vec3 Primitive3D::GetAngularFactor() const
{
    return mRigidBody ? BulletToGlm(mRigidBody->getAngularFactor()) : glm::vec3(1.0f, 1.0f, 1.0f);
}

uint8_t Primitive3D::GetCollisionGroup() const
{
    return mCollisionGroup;
}

uint8_t Primitive3D::GetCollisionMask() const
{
    return mCollisionMask;
}

void Primitive3D::SetMass(float mass)
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

void Primitive3D::SetLinearDamping(float linearDamping)
{
    EnableRigidBody(false);
    if (mRigidBody) mRigidBody->setDamping(linearDamping, mRigidBody->getAngularDamping());
    EnableRigidBody(true);
}

void Primitive3D::SetAngularDamping(float angularDamping)
{
    EnableRigidBody(false);
    if (mRigidBody) mRigidBody->setDamping(mRigidBody->getLinearDamping(), angularDamping);
    EnableRigidBody(true);
}

void Primitive3D::SetRestitution(float restitution)
{
    UPDATE_RIGID_BODY_PROPERTY
    (
        mRestitution,
        restitution,
        mRigidBody->setRestitution(restitution)
    );
}

void Primitive3D::SetFriction(float friction)
{
    UPDATE_RIGID_BODY_PROPERTY
    (
        mFriction,
        friction,
        mRigidBody->setFriction(friction)
    );
}

void Primitive3D::SetRollingFriction(float rollingFriction)
{
        EnableRigidBody(false);
        if (mRigidBody) mRigidBody->setRollingFriction(rollingFriction);
        EnableRigidBody(true);
}

void Primitive3D::SetLinearFactor(glm::vec3 linearFactor)
{
    EnableRigidBody(false);
    if (mRigidBody) mRigidBody->setLinearFactor(GlmToBullet(linearFactor));
    EnableRigidBody(true);
}

void Primitive3D::SetAngularFactor(glm::vec3 angularFactor)
{
    EnableRigidBody(false);
    if (mRigidBody) mRigidBody->setAngularFactor(GlmToBullet(angularFactor));
    EnableRigidBody(true);
}

void Primitive3D::SetCollisionGroup(uint8_t group)
{
    EnableRigidBody(false);
    mCollisionGroup = group;
    EnableRigidBody(true);
}

void Primitive3D::SetCollisionMask(uint8_t mask)
{
    EnableRigidBody(false);
    mCollisionMask = mask;
    EnableRigidBody(true);
}

glm::vec3 Primitive3D::GetLinearVelocity() const
{
    btVector3 linearVelocity;
    linearVelocity = mRigidBody->getLinearVelocity();
    return { linearVelocity.x(), linearVelocity.y(), linearVelocity.z() };
}

glm::vec3 Primitive3D::GetAngularVelocity() const
{
    btVector3 angularVelocity;
    angularVelocity = mRigidBody->getAngularVelocity();
    return { angularVelocity.x(), angularVelocity.y(), angularVelocity.z() };
}

void Primitive3D::AddLinearVelocity(glm::vec3 deltaVelocity)
{
    if (mRigidBody)
    {
        btVector3 delta = { deltaVelocity.x, deltaVelocity.y, deltaVelocity.z };
        mRigidBody->setLinearVelocity(mRigidBody->getLinearVelocity() + delta);
        mRigidBody->activate();
    }
}

void Primitive3D::AddAngularVelocity(glm::vec3 deltaVelocity)
{
    if (mRigidBody)
    {
        btVector3 delta = { deltaVelocity.x, deltaVelocity.y, deltaVelocity.z };
        mRigidBody->setAngularVelocity(mRigidBody->getAngularVelocity() + delta);
        mRigidBody->activate();
    }
}

void Primitive3D::SetLinearVelocity(glm::vec3 linearVelocity)
{
    if (mRigidBody)
    {
        btVector3 velocity = { linearVelocity.x, linearVelocity.y, linearVelocity.z };
        mRigidBody->setLinearVelocity(velocity);
        mRigidBody->activate();
    }
}

void Primitive3D::SetAngularVelocity(glm::vec3 angularVelocity)
{
    if (mRigidBody)
    {
        btVector3 velocity = { angularVelocity.x, angularVelocity.y, angularVelocity.z };
        mRigidBody->setAngularVelocity(velocity);
        mRigidBody->activate();
    }
}

void Primitive3D::AddForce(glm::vec3 force)
{
    if (mRigidBody)
    {
        btVector3 forceBt = { force.x, force.y, force.z };
        mRigidBody->applyCentralForce(forceBt);
        mRigidBody->activate();
    }
}

void Primitive3D::AddImpulse(glm::vec3 impulse)
{
    if (mRigidBody)
    {
        btVector3 impulseBt = { impulse.x, impulse.y, impulse.z };
        mRigidBody->applyCentralImpulse(impulseBt);
        mRigidBody->activate();
    }
}

void Primitive3D::ClearForces()
{
    if (mRigidBody)
    {
        mRigidBody->clearForces();
    }
}

void Primitive3D::FullSyncRigidBodyTransform()
{
    // Because updating transform is something that might happen very often
    // We only sync transform instead of calling Enable/DisableRigidBody
    btDynamicsWorld* dynamicsWorld = GetWorld()->GetDynamicsWorld();
    dynamicsWorld->removeRigidBody(mRigidBody);
    SyncRigidBodyTransform();
    mRigidBody->activate(true);
    dynamicsWorld->addRigidBody(mRigidBody, mCollisionGroup, mCollisionMask);
}

void Primitive3D::SyncRigidBodyTransform()
{
    if (GetWorld() != nullptr)
    {
        if (mRigidBody != nullptr)
        {
            btTransform worldTransform;
            glm::vec3 worldPos;
            glm::quat worldRot;

            if (mParent != nullptr)
            {
                worldPos = GetWorldPosition();
                worldRot = GetWorldRotationQuat();

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
                OCT_ASSERT(mMotionState != nullptr);
                mMotionState->setWorldTransform(worldTransform);
            }

            mRigidBody->setWorldTransform(worldTransform);
        }

        if (mCollisionShape != nullptr)
        {
            glm::vec3 worldScale = GetWorldScale();
            mCollisionShape->setLocalScaling(btVector3(worldScale.x, worldScale.y, worldScale.z));
        }
    }
}

void Primitive3D::SyncRigidBodyMass()
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

void Primitive3D::SyncCollisionFlags()
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

void Primitive3D::EnableCastShadows(bool enable)
{
    mCastShadows = enable;
}

bool Primitive3D::ShouldCastShadows() const
{
    return mCastShadows;
}

void Primitive3D::EnableReceiveShadows(bool enable)
{
    mReceiveShadows = enable;
}

bool Primitive3D::ShouldReceiveShadows() const
{
    return mReceiveShadows;
}

void Primitive3D::EnableReceiveSimpleShadows(bool enable)
{
    mReceiveSimpleShadows = enable;
}

bool Primitive3D::ShouldReceiveSimpleShadows() const
{
    return mReceiveSimpleShadows;
}

uint8_t Primitive3D::GetLightingChannels() const
{
    return mLightingChannels;
}

void Primitive3D::SetLightingChannels(uint8_t channels)
{
    mLightingChannels = channels;
}

VertexType Primitive3D::GetVertexType() const
{
    return VertexType::Vertex;
}

btRigidBody* Primitive3D::GetRigidBody()
{
    return mRigidBody;
}

btCollisionShape* Primitive3D::GetCollisionShape()
{
    return mCollisionShape;
}

void Primitive3D::SetCollisionShape(btCollisionShape* newShape)
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

bool Primitive3D::SweepToWorldPosition(glm::vec3 position, SweepTestResult& outSweepResult, uint8_t mask)
{
    bool hit = false;
    glm::vec3 startPos = GetWorldPosition();

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

        SetWorldPosition(startPos + fracDelta + outSweepResult.mHitNormal * padding);

        if (GetParent() != nullptr)
        {
            GetParent()->OnCollision(this, outSweepResult.mHitNode, outSweepResult.mHitPosition, outSweepResult.mHitNormal, nullptr);
        }
    }
    else
    {
        SetWorldPosition(position);
    }

    return hit;
}

Bounds Primitive3D::GetBounds() const
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

Bounds Primitive3D::GetLocalBounds() const
{
    // Derived classes should implement a way of getting their local bounds.
    // By default Primitive3D will create a huge bounding sphere so that it never really gets culled.
    Bounds retBounds;
    retBounds.mCenter = { 0.0f, 0.0f, 0.0f };
    retBounds.mRadius = LARGE_BOUNDS;
    return retBounds;
}

void Primitive3D::GatherProxyDraws(std::vector<DebugDraw>& inoutDraws)
{
#if DEBUG_DRAW_ENABLED
    BoundsDebugMode boundsMode = Renderer::Get()->GetBoundsDebugMode();

    if (boundsMode == BoundsDebugMode::All
#if EDITOR
        || (boundsMode == BoundsDebugMode::Selected && GetEditorState()->GetSelectedNode() == this)
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

btCollisionShape* Primitive3D::GetEmptyCollisionShape()
{
    if (sEmptyCollisionShape == nullptr)
    {
        sEmptyCollisionShape = new btEmptyShape();
    }

    return sEmptyCollisionShape;
}

glm::vec4 Primitive3D::GetCollisionDebugColor()
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

bool Primitive3D::IsRigidBodyInWorld() const
{
    return mRigidBody && mRigidBody->isInWorld();
}

void Primitive3D::EnableRigidBody(bool enable)
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
            OCT_ASSERT(mCollisionShape != nullptr);

            float rigidBodyMass = mPhysicsEnabled ? mMass : 0.0f;

            if (mPhysicsEnabled &&
                mCollisionShape->getShapeType() != EMPTY_SHAPE_PROXYTYPE)
            {
                mCollisionShape->calculateLocalInertia(rigidBodyMass, localInertia);
            }

            // A motion state should be created when physics is enabled.
            OCT_ASSERT(mMotionState || !mPhysicsEnabled);

            btRigidBody::btRigidBodyConstructionInfo rbInfo(rigidBodyMass, mMotionState, mCollisionShape, localInertia);
            mRigidBody = new btRigidBody(rbInfo);
            mRigidBody->setUserPointer(this);

            // These values might have been set before the primitive had a valid mWorld, 
            // so initialize them here. Afterwards calls to the respective Primitive3D functions
            // will relay the changes to the mRigidbody.
            mRigidBody->setRestitution(mRestitution);
            mRigidBody->setFriction(mFriction);
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

void Primitive3D::DestroyComponentCollisionShape()
{
    if (mCollisionShape != nullptr &&
        mCollisionShape != GetEmptyCollisionShape())
    {
        DestroyCollisionShape(mCollisionShape);
        mCollisionShape = nullptr;
    }
}
