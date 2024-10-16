#pragma once
#include "Node3d.h"
#include "Vertex.h"
#include "Assets/StaticMesh.h"

#include <Bullet/btBulletDynamicsCommon.h>
#include "Maths.h"

#include <vector>

//typedef void(*BeginOverlapHandlerFP)(Primitive3D* thisPrim, Primitive3D* otherPrim);
//typedef void(*EndOverlapHandlerFP)(Primitive3D* thisPrim, Primitive3D* otherPrim);
//typedef void(*CollisionHandlerFP)(Primitive3D* thisPrim, Primitive3D* otherPrim, btPersistentManifold* manifold);

ATTRIBUTE_ALIGNED16(struct) OctaveMotionState : public btMotionState
{
    btTransform mInterpolatedTransform;
    //void* mUserPointer;

    BT_DECLARE_ALIGNED_ALLOCATOR();

    OctaveMotionState(glm::mat4 startTransform = glm::mat4(1))
    {
        mInterpolatedTransform.setFromOpenGLMatrix(glm::value_ptr(startTransform));
    }

    virtual void getWorldTransform(btTransform& transform) const override
    {
        transform = mInterpolatedTransform;
    }

    virtual void setWorldTransform(const btTransform& transform) override
    {
        mInterpolatedTransform = transform;
    }

    glm::mat4 GetTransform() const
    {
        glm::mat4 retMat;
        mInterpolatedTransform.getOpenGLMatrix(glm::value_ptr(retMat));
        return retMat;
    }
};

class Primitive3D : public Node3D
{
public:

    DECLARE_RTTI(Primitive3D, Node3D);

    Primitive3D();
    virtual ~Primitive3D();

    virtual void Create() override;
    virtual void Destroy() override;

    virtual const char* GetTypeName() const override;
    virtual bool IsPrimitive3D() const override;
    virtual void Tick(float deltaTime) override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;

    virtual void SetWorld(World* world) override;
    virtual void Render() override;

    virtual void UpdateTransform(bool updateChildren) override;
    virtual void SetTransform(const glm::mat4& transform) override;

    void EnablePhysics(bool enable);
    void EnableCollision(bool enable);
    void EnableOverlaps(bool enable);
    bool IsPhysicsEnabled() const;
    bool IsCollisionEnabled() const;
    bool AreOverlapsEnabled() const;

    float GetCullDistance() const;
    void SetCullDistance(float cullDistance);

    float GetMass() const;
    float GetLinearDamping() const;
    float GetAngularDamping() const;
    float GetRestitution() const;
    float GetFriction() const;
    float GetRollingFriction();
    glm::vec3 GetLinearFactor() const;
    glm::vec3 GetAngularFactor() const;
    uint8_t GetCollisionGroup() const;
    uint8_t GetCollisionMask() const;

    void SetMass(float mass);
    void SetLinearDamping(float linearDamping);
    void SetAngularDamping(float angularDamping);
    void SetRestitution(float restitution);
    void SetFriction(float friction);
    void SetRollingFriction(float rollingFriction);
    void SetLinearFactor(glm::vec3 linearFactor);
    void SetAngularFactor(glm::vec3 angularFactor);
    void SetCollisionGroup(uint8_t group);
    void SetCollisionMask(uint8_t mask);

    glm::vec3 GetLinearVelocity() const;
    glm::vec3 GetAngularVelocity() const;

    void AddLinearVelocity(glm::vec3 deltaVelocity);
    void AddAngularVelocity(glm::vec3 deltaVelocity);
    void SetLinearVelocity(glm::vec3 linearVelocity);
    void SetAngularVelocity(glm::vec3 angularVelocity);
    void AddForce(glm::vec3 force);
    void AddImpulse(glm::vec3 impulse);
    void ClearForces();

    void FullSyncRigidBodyTransform();

    void SyncRigidBodyTransform();
    void SyncRigidBodyMass();
    void SyncCollisionFlags();

    void EnableCastShadows(bool enable);
    bool ShouldCastShadows() const;

    void EnableReceiveShadows(bool enable);
    bool ShouldReceiveShadows() const;

    void EnableReceiveSimpleShadows(bool enable);
    bool ShouldReceiveSimpleShadows() const;

    uint8_t GetLightingChannels() const;
    void SetLightingChannels(uint8_t channels);

    virtual VertexType GetVertexType() const override;

    btRigidBody* GetRigidBody();
    btCollisionShape* GetCollisionShape();
    void SetCollisionShape(btCollisionShape* newShape);

    // When passing in the mask as 0, it means use the primitive's collision mask
    bool SweepToWorldPosition(glm::vec3 position, SweepTestResult& outSweepResult, uint8_t mask = 0);

    Bounds GetBounds() const;
    virtual Bounds GetLocalBounds() const;

    virtual void GatherProxyDraws(std::vector<DebugDraw>& inoutDraws) override;

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

    glm::vec4 GetCollisionDebugColor();

protected:

    static btCollisionShape* GetEmptyCollisionShape();

    bool IsRigidBodyInWorld() const;
    void EnableRigidBody(bool enable);
    void DestroyComponentCollisionShape();

    btRigidBody* mRigidBody = nullptr;
    OctaveMotionState* mMotionState = nullptr;
    btCollisionShape* mCollisionShape = nullptr;

    float mCullDistance = 0.0f;

    // Physics Properties
    float mMass = 1.0f;
    float mRestitution = 0.0f;
    float mFriction = 0.5f;
    uint8_t mCollisionGroup = ColGroup0;
    uint8_t mCollisionMask = ColGroupAll;
    uint8_t mLightingChannels = 0x01;

    bool mPhysicsEnabled = false;
    bool mCollisionEnabled = false;
    bool mOverlapsEnabled = false;
    bool mCastShadows = false;
    bool mReceiveShadows = true;
    bool mReceiveSimpleShadows = true;
    //BeginOverlapHandlerFP mBeginOverlapHandler;
    //EndOverlapHandlerFP mEndOverlapHandler;
    //CollisionHandlerFP mCollisionHandler;
};