#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "Assets/StaticMesh.h"
#include "Assets/Material.h"
#include "Assets/Texture.h"
#include "Actor.h"
#include "Clock.h"
#include "Line.h"
#include "EngineTypes.h"
#include "ObjectRef.h"
#include "Components/CameraComponent.h"
#include "Components/DirectionalLightComponent.h"

class Component;
class AudioComponent;

struct QueuedLevel
{
    LevelRef mLevel;
    glm::vec3 mOffset = {};
    glm::vec3 mRotation = {};
};

class World
{
public:

    World();

    void Destroy();

    void Update(float deltaTime);

    CameraComponent* GetActiveCamera();
    CameraComponent* GetDefaultCamera();
    TransformComponent* GetAudioReceiver();

    void SetActiveCamera(CameraComponent* activeCamera);
    void SetAudioReceiver(TransformComponent* newReceiver);

    void AddActor(Actor* actor);
    void RemoveActor(Actor* actor);

    Actor* SpawnActor(TypeId actorType, bool addNetwork = true);
    Actor* SpawnActor(const char* typeName);
    Actor* CloneActor(Actor* srcActor);

    template<class ActorClass>
    ActorClass* SpawnActor()
    {
        return (ActorClass*) SpawnActor(ActorClass::GetStaticType());
    }

    void DestroyActor(Actor* actor);
    void DestroyActor(uint32_t index);
    void DestroyAllActors();
    void FlushPendingDestroys();
    const std::vector<Actor*>& GetActors() const;
    Actor* FindActor(const std::string& name);
    Actor* FindActor(NetId netId);
    std::vector<Actor*> FindActorsByTag(const char* tag);
    std::vector<Actor*> FindActorsByName(const char* name);
    Component* FindComponent(const std::string& name);
    void PrioritizeActorTick(Actor* actor);
    void AddNetActor(Actor* actor, NetId netId);
    const std::unordered_map<NetId, Actor*>& GetNetActorMap() const;

    void Clear(bool clearPersistent = false);

    void AddLine(const Line& line);
    void RemoveLine(const Line& line);
    void RemoveAllLines();
    const std::vector<Line>& GetLines() const;

    const std::vector<class LightComponent*>& GetLightComponents();

    void SetAmbientLightColor(glm::vec4 color);
    glm::vec4 GetAmbientLightColor() const;

    void SetShadowColor(glm::vec4 shadowColor);
    glm::vec4 GetShadowColor() const;

    void SetFogSettings(const FogSettings& settings);
    const FogSettings& GetFogSettings() const;

    void SetGravity(glm::vec3 gravity);
    glm::vec3 GetGravity() const;

    btDynamicsWorld* GetDynamicsWorld();
    btDbvtBroadphase* GetBroadphase();
    void PurgeOverlaps(PrimitiveComponent* prim);

    void RayTest(glm::vec3 start, glm::vec3 end, uint8_t collisionMask, RayTestResult& outResult);
    void RayTestMulti(glm::vec3 start, glm::vec3 end, uint8_t collisionMask, RayTestMultiResult& outResult);
    void SweepTest(PrimitiveComponent* primComp, glm::vec3 start, glm::vec3 end, uint8_t collisionMask, SweepTestResult& outResult);
    void SweepTest(
        btConvexShape* convexShape, 
        glm::vec3 start,
        glm::vec3 end,
        glm::quat rotation,
        uint8_t collisionMask,
        SweepTestResult& outResult,
        uint32_t numIgnoreObjects = 0,
        btCollisionObject** ignoreObjects = nullptr);

    void RegisterComponent(Component* comp);
    void UnregisterComponent(Component* comp);
    const std::vector<AudioComponent*>& GetAudioComponents() const;

    std::vector<Actor*>& GetReplicatedActorVector(ReplicationRate rate);
    uint32_t& GetReplicatedActorIndex(ReplicationRate rate);
    uint32_t& GetIncrementalRepTier();
    uint32_t& GetIncrementalRepIndex();

    std::vector<LevelRef>& GetLoadedLevels();
    void UnloadAllLevels();

    Actor* SpawnBlueprint(const char* name);
    void LoadLevel(
        const char* name,
        bool clear,
        glm::vec3 offset = { 0.0f, 0.0f, 0.0f },
        glm::vec3 rotation = { 0.0f, 0.0f, 0.0f });
    void QueueLevelLoad(
        const char* name,
        bool clearWorld,
        glm::vec3 offset = { 0.0f, 0.0f, 0.0f },
        glm::vec3 rotation = { 0.0f, 0.0f, 0.0f });
    void UnloadLevel(const char* name);
    bool IsLevelLoaded(const char* levelName);

    void EnableInternalEdgeSmoothing(bool enable);
    bool IsInternalEdgeSmoothingEnabled() const;

    template<typename T>
    T* FindActor()
    {
        for (uint32_t i = 0; i < mActors.size(); ++i)
        {
            if (mActors[i]->Is(T::ClassRuntimeId()))
            {
                return static_cast<T*>(mActors[i]);
            }
        }

        return nullptr;
    }

    template<typename T>
    void FindActors(std::vector<T*>& outActors)
    {
        for (uint32_t i = 0; i < mActors.size(); ++i)
        {
            if (mActors[i]->Is(T::ClassRuntimeId()))
            {
                outActors.push_back(static_cast<T*>(mActors[i]));
            }
        }
    }

private:

    void UpdateLines(float deltaTime);
    void SetTestDirectionalLight();
    void SpawnDefaultCamera();

private:

    std::vector<Actor*> mActors;
    std::unordered_map<NetId, Actor*> mNetActorMap;
    std::vector<Line> mLines;
    std::vector<class LightComponent*> mLightComponents;
    std::vector<class AudioComponent*> mAudioComponents;
    std::vector<LevelRef> mLoadedLevels;
    std::vector<QueuedLevel> mQueuedLevels;
    DirectionalLightComponent* mDirectionalLight;
    glm::vec4 mAmbientLightColor;
    glm::vec4 mShadowColor;
    FogSettings mFogSettings;
    CameraComponent* mActiveCamera;
    TransformComponent* mAudioReceiver;
    ComponentRef mDefaultCamera;
    NetId mNextNetId;
    bool mPendingClear = false;

    // Replication tiers
    std::vector<Actor*> mRepActors[(uint32_t)ReplicationRate::Count];
    uint32_t mRepIndices[(uint32_t)ReplicationRate::Count] = {};
    uint32_t mIncrementalRepTier = 0;
    uint32_t mIncrementalRepIndex = 0;

    // Physics
    btDefaultCollisionConfiguration* mCollisionConfig;
    btCollisionDispatcher* mCollisionDispatcher;
    btDbvtBroadphase* mBroadphase;
    btSequentialImpulseConstraintSolver* mSolver;
    btDiscreteDynamicsWorld* mDynamicsWorld;
    std::vector<ComponentPair> mCurrentOverlaps;
    std::vector<ComponentPair> mPreviousOverlaps;

#if EDITOR
public:

    bool IsComponentSelected(Component* comp) const;
    Component* GetSelectedComponent();
    const std::vector<Component*>& GetSelectedComponents();
    std::vector<Actor*> GetSelectedActors();
    void DeselectComponent(Component* comp);

#endif
};