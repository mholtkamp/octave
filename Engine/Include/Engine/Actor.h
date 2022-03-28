#pragma once

#include "RTTI.h"
#include "Assets/StaticMesh.h"
#include <glm/glm.hpp>
#include "Components/Component.h"
#include "Property.h"
#include "Factory.h"
#include "NetDatum.h"
#include "NetFunc.h"


#include "Bullet/btBulletCollisionCommon.h"

#include <unordered_map>

class World;
class Level;
class TransformComponent;
class ScriptComponent;
class Stream;
class Blueprint;
class ActorFactory;

#define DECLARE_ACTOR(Base, Parent) DECLARE_FACTORY(Base, Actor); DECLARE_RTTI(Base, Parent);
#define DEFINE_ACTOR(Base) DEFINE_FACTORY(Base, Actor); DEFINE_RTTI(Base);

typedef std::unordered_map<std::string, NetFunc> NetFuncMap;

class Actor : public RTTI
{
public:

    Actor();

    virtual ~Actor();

    virtual void Create();
    virtual void Destroy();
    virtual void Tick(float deltaTime);
    virtual void BeginPlay();
    virtual void EndPlay();

    virtual void SaveStream(Stream& stream);
    virtual void LoadStream(Stream& stream);
    virtual void Copy(Actor* srcActor);
    virtual void Render(PipelineId pipelineId);

    virtual void GatherProperties(std::vector<Property>& outProps);
    virtual void GatherReplicatedData(std::vector<NetDatum>& outData);
    virtual void GatherNetFuncs(std::vector<NetFunc>& outFuncs);
    void GatherPropertyOverrides(std::vector<PropertyOverride>& outOverrides);
    void ApplyPropertyOverrides(const std::vector<PropertyOverride>& overs);

    virtual void BeginOverlap(PrimitiveComponent* thisComp, PrimitiveComponent* otherComp);
    virtual void EndOverlap(PrimitiveComponent* thisComp, PrimitiveComponent* otherComp);
    virtual void OnCollision(
        PrimitiveComponent* thisComp, 
        PrimitiveComponent* otherComp, 
        glm::vec3 impactPoint,
        glm::vec3 impactNormal,
        btPersistentManifold* manifold);

    void RenderShadow();
    void RenderSelected();

    Component* CreateComponent(TypeId compType);
    Component* CreateComponent(const char* typeName);
    Component* CloneComponent(Component* srcComp);
    void DestroyComponent(Component* comp);

    template<class CompClass>
    CompClass* CreateComponent()
    {
        return (CompClass*) CreateComponent(CompClass::GetStaticType());
    }

    const std::string& GetName() const;
    void SetName(const std::string& name);

    void Attach(Actor* actor);
    void Attach(TransformComponent* comp);

    void SetPendingDestroy(bool pendingDestroy);
    bool IsPendingDestroy() const;

    bool HasBegunPlay() const;
    bool ShouldTickInEditor() const;

    void EnableTick(bool enable);
    bool IsTickEnabled() const;

    void SetTransient(bool transient);
    bool IsTransient() const;

    glm::vec3 GetPosition() const;
    glm::quat GetRotationQuat() const;
    glm::vec3 GetRotationEuler() const;
    glm::vec3 GetScale() const;

    void SetPosition(glm::vec3 position);
    void SetRotation(glm::quat rotation);
    void SetRotation(glm::vec3 rotation);
    void SetScale(glm::vec3 scale);

    glm::vec3 GetForwardVector();
    glm::vec3 GetRightVector();
    glm::vec3 GetUpVector();

    // When passing in the mask as 0, it means use the primitive's collision mask
    bool SweepToPosition(glm::vec3 position, SweepTestResult& outSweepResult, uint8_t mask = 0);

    void SetWorld(World* world);
    World* GetWorld();

    void SetLevel(Level* level);
    Level* GetLevel();

    std::vector<NetDatum>& GetReplicatedData();

    void SetNetId(NetId id);
    NetId GetNetId() const;

    NetHostId GetOwningHost() const;
    void SetOwningHost(NetHostId hostId);

    void SetReplicate(bool replicate);
    bool IsReplicated() const;

    void ForceReplication();
    void ClearForcedReplication();
    bool NeedsForcedReplication();

    ReplicationRate GetReplicationRate() const;
    //void SetReplicationRate(ReplicationRate rate);

    const std::vector<Component*> GetComponents() const;
    void AddComponent(Component* component);
    void RemoveComponent(Component* component);
    uint32_t GetNumComponents() const;
    Component* GetComponent(int32_t index);
    Component* GetComponent(const std::string& name);
    Component* GetComponent(TypeId type);
    void CopyComponents(const std::vector<Component*>& srcComps);

    void SetRootComponent(TransformComponent* component);
    TransformComponent* GetRootComponent();

    void UpdateComponentTransforms();
    void SetHitCheckId(uint32_t id);
    uint32_t GetHitCheckId() const;

    void AddScriptEventHandler(ScriptComponent* scriptComp);
    void RemoveScriptEventHandler(ScriptComponent* scriptComp);
    uint32_t GetNumScriptComponents() const;

    Blueprint* GetBlueprintSource() const;
    void SetBlueprintSource(Blueprint* bp);

    NetFunc* FindNetFunc(const char* name);
    NetFunc* FindNetFunc(uint16_t index);

    void InvokeNetFunc(const char* name);
    void InvokeNetFunc(const char* name, Datum param0);
    void InvokeNetFunc(const char* name, Datum param0, Datum param1);
    void InvokeNetFunc(const char* name, Datum param0, Datum param1, Datum param2);
    void InvokeNetFunc(const char* name, Datum param0, Datum param1, Datum param2, Datum param3);
    void InvokeNetFunc(const char* name, Datum param0, Datum param1, Datum param2, Datum param3, Datum param4);
    void InvokeNetFunc(const char* name, Datum param0, Datum param1, Datum param2, Datum param3, Datum param4, Datum param5);
    void InvokeNetFunc(const char* name, Datum param0, Datum param1, Datum param2, Datum param3, Datum param4, Datum param5, Datum param6);
    void InvokeNetFunc(const char* name, Datum param0, Datum param1, Datum param2, Datum param3, Datum param4, Datum param5, Datum param6, Datum param7);

    static bool OnRep_RootPosition(Datum* datum, const void* newValue);
    static bool OnRep_RootRotation(Datum* datum, const void* newValue);
    static bool OnRep_RootScale(Datum* datum, const void* newValue);

    static void RegisterNetFuncs(Actor* actor);

    DECLARE_FACTORY_MANAGER(Actor);
    DECLARE_FACTORY(Actor, Actor);
    DECLARE_RTTI(Actor, RTTI);

protected:

    void SendNetFunc(NetFunc* func, uint32_t numParams, Datum** params);
    void DestroyAllComponents();

    static std::unordered_map<TypeId, NetFuncMap> sTypeNetFuncMap;

private:
    TransformComponent* mRootComponent;

protected:
    std::string mName;
    World* mWorld;
    Level* mLevel;
    std::vector<Component*> mComponents;
    std::vector<ScriptComponent*> mEventHandlerScripts;
    std::vector<NetDatum> mReplicatedData;
    AssetRef mBlueprintSource;
    NetId mNetId;
    uint32_t mHitCheckId;
    NetHostId mOwningHost;
    bool mReplicate;
    bool mReplicateTransform;
    bool mForceReplicate;
    bool mBegunPlay;
    bool mPendingDestroy;
    bool mTickInEditor;
    bool mTickEnabled;
    bool mTransient;
    ReplicationRate mReplicationRate;
    uint8_t mNumScriptComps;
};
