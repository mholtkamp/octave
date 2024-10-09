#include "EngineTypes.h"
#include "NetworkManager.h"

#include <Bullet/btBulletDynamicsCommon.h>

IgnoreRayResultCallback::IgnoreRayResultCallback(const btVector3& rayFromWorld, const btVector3& rayToWorld) :
    ClosestRayResultCallback(rayFromWorld, rayToWorld)
{

}

btScalar IgnoreRayResultCallback::addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
{
    for (uint32_t i = 0; i < mNumIgnoreObjects; ++i)
    {
        if (mIgnoreObjects[i] == rayResult.m_collisionObject)
        {
            // What are we supposed to return here?
            return rayResult.m_hitFraction;
        }
    }

    return btCollisionWorld::ClosestRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);
}

IgnoreConvexResultCallback::IgnoreConvexResultCallback(
    const btVector3& convexFromWorld,
    const btVector3& convexToWorld) : 
    ClosestConvexResultCallback(convexFromWorld, convexToWorld)

{

}

bool IgnoreConvexResultCallback::needsCollision(btBroadphaseProxy* proxy0) const
{
    bool collides = (proxy0->m_collisionFilterGroup & m_collisionFilterMask) != 0;
    collides = collides && (m_collisionFilterGroup & proxy0->m_collisionFilterMask);

    int flags = reinterpret_cast<btRigidBody*>(proxy0->m_clientObject)->getCollisionFlags();
    collides = collides && !(flags & btCollisionObject::CF_NO_CONTACT_RESPONSE);

    if (collides)
    {
        for (uint32_t i = 0; i < mNumIgnoreObjects; ++i)
        {
            if (proxy0->m_clientObject == (void*)mIgnoreObjects[i])
            {
                collides = false;
                break;
            }
        }
    }

    return collides;
}

ReliablePacket::ReliablePacket(uint16_t seqNum, const char* data, uint32_t size)
{
    OCT_ASSERT(size <= OCT_MAX_MSG_SIZE);
    mSeq = seqNum;
    mData.resize(size);
    memcpy(mData.data(), data, size);
}
