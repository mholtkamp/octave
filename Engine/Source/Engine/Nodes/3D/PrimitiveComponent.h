#pragma once
#include "TransformComponent.h"
#include "Vertex.h"
#include "Assets/StaticMesh.h"

#include <Bullet/btBulletDynamicsCommon.h>
#include "Maths.h"

#include <vector>

//typedef void(*BeginOverlapHandlerFP)(PrimitiveComponent* thisPrim, PrimitiveComponent* otherPrim);
//typedef void(*EndOverlapHandlerFP)(PrimitiveComponent* thisPrim, PrimitiveComponent* otherPrim);
//typedef void(*CollisionHandlerFP)(PrimitiveComponent* thisPrim, PrimitiveComponent* otherPrim, btPersistentManifold* manifold);

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

class PrimitiveComponent : public TransformComponent
{
public:

    DECLARE_RTTI(PrimitiveComponent, TransformComponent);

    PrimitiveComponent();
    virtual ~PrimitiveComponent();

    virtual void Create() override;
    virtual void Destroy() override;

    virtual const char* GetTypeName() const override;
    virtual bool IsPrimitiveComponent() const override;
    virtual void Tick(float deltaTime) override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;

    virtual void LoadStream(Stream& stream) override;
    virtual void SaveStream(Stream& stream) override;

    virtual void SetOwner(Actor* owner) override;
    virtual void Render();

    virtual void UpdateTransform(bool updateChildren) override;
    virtual void SetTransform(const glm::mat4& transform) override;

    void EnablePhysics(bool enable);
    void EnableCollision(bool enable);
    void EnableOverlaps(bool enable);
    bool IsPhysicsEnabled() const;
    bool IsCollisionEnabled() const;
    bool AreOverlapsEnabled() const;

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

    virtual VertexType GetVertexType() const;

    btRigidBody* GetRigidBody();
    btCollisionShape* GetCollisionShape();
    void SetCollisionShape(btCollisionShape* newShape);

    // When passing in the mask as 0, it means use the primitive's collision mask
    bool SweepToWorldPosition(glm::vec3 position, SweepTestResult& outSweepResult, uint8_t mask = 0);

    Bounds GetBounds() const;
    virtual Bounds GetLocalBounds() const;

    virtual void GatherProxyDraws(std::vector<DebugDraw>& inoutDraws) override;

protected:

    static btCollisionShape* GetEmptyCollisionShape();
    glm::vec4 GetCollisionDebugColor();

    bool IsRigidBodyInWorld() const;
    void EnableRigidBody(bool enable);
    void DestroyComponentCollisionShape();

    btRigidBody* mRigidBody;
    OctaveMotionState* mMotionState;
    btCollisionShape* mCollisionShape;

    // Physics Properties
    float mMass;
    float mRestitution;
    float mFriction;
    float mRollingFriction;
    float mLinearDamping;
    float mAngularDamping;
    glm::vec3 mLinearFactor;
    glm::vec3 mAngularFactor;
    uint8_t mCollisionGroup;
    uint8_t mCollisionMask;

    bool mPhysicsEnabled;
    bool mCollisionEnabled;
    bool mOverlapsEnabled;
    bool mCastShadows;
    bool mReceiveShadows;
    bool mReceiveSimpleShadows;
    //BeginOverlapHandlerFP mBeginOverlapHandler;
    //EndOverlapHandlerFP mEndOverlapHandler;
    //CollisionHandlerFP mCollisionHandler;
};