#pragma once

#include "RTTI.h"
#include "Assets/StaticMesh.h"
#include "Maths.h"
#include "Nodes/Node.h"
#include "Property.h"
#include "Factory.h"
#include "NetDatum.h"
#include "NetFunc.h"
#include "ScriptUtils.h"
#include "ScriptAutoReg.h"

#include "Bullet/btBulletCollisionCommon.h"

#include <unordered_map>
#include <unordered_set>

class World;
class Level;
class TransformComponent;
class ScriptComponent;
class Stream;
class Blueprint;
class ActorFactory;

class Actor : public RTTI
{
public:

protected:

    void DestroyAllComponents();

private:
    TransformComponent* mRootComponent;

protected:
    std::string mName;
    World* mWorld;
    Level* mLevel;
    std::vector<Component*> mComponents;
    std::vector<ScriptComponent*> mEventHandlerScripts;
    std::vector<NetDatum> mReplicatedData;
    std::vector<std::string> mTags;
    AssetRef mBlueprintSource;
    NetId mNetId;
    uint32_t mHitCheckId;
    NetHostId mOwningHost;
    bool mReplicate;
    bool mReplicateTransform;
    bool mForceReplicate;
    bool mBegunPlay;
    bool mPendingDestroy;
    bool mTickEnabled;
    bool mTransient;
    bool mPersistent;
    bool mVisible;
    ReplicationRate mReplicationRate;
    uint8_t mNumScriptComps;
};
