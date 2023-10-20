#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "Assets/StaticMesh.h"
#include "Assets/Material.h"
#include "Assets/Texture.h"
#include "Nodes/Node.h"
#include "Nodes/Widgets/Widget.h"
#include "Clock.h"
#include "Line.h"
#include "EngineTypes.h"
#include "ObjectRef.h"
#include "Nodes/3D/Camera3d.h"
#include "Nodes/3D/DirectionalLight3d.h"

class Node;
class Audio3D;

class World
{
public:

    World();

    void Destroy();

    void Update(float deltaTime);

    Camera3D* GetActiveCamera();
    Node3D* GetAudioReceiver();

    void SetActiveCamera(Camera3D* activeCamera);
    void SetAudioReceiver(Node3D* newReceiver);

    Node* SpawnNode(TypeId actorType);
    Node* SpawnNode(const char* typeName);
    Node* SpawnScene(const char* sceneName);

    template<class ActorClass>
    ActorClass* SpawnActor()
    {
        return (ActorClass*) SpawnActor(ActorClass::GetStaticType());
    }

    void FlushPendingDestroys();
    Node* GetRootNode();
    void SetRootNode(Node* node);
    void DestroyRootNode();
    Node* FindNode(const std::string& name);
    Node* GetNetNode(NetId netId);
    std::vector<Node*> FindNodesByTag(const char* tag);
    std::vector<Node*> FindNodesByName(const char* name);

    void Clear();

    void AddLine(const Line& line);
    void RemoveLine(const Line& line);
    void RemoveAllLines();
    const std::vector<Line>& GetLines() const;

    const std::vector<class Light3D*>& GetLights();

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
    void PurgeOverlaps(Primitive3D* prim);

    void RayTest(glm::vec3 start, glm::vec3 end, uint8_t collisionMask, RayTestResult& outResult);
    void RayTestMulti(glm::vec3 start, glm::vec3 end, uint8_t collisionMask, RayTestMultiResult& outResult);
    void SweepTest(Primitive3D* primComp, glm::vec3 start, glm::vec3 end, uint8_t collisionMask, SweepTestResult& outResult);
    void SweepTest(
        btConvexShape* convexShape, 
        glm::vec3 start,
        glm::vec3 end,
        glm::quat rotation,
        uint8_t collisionMask,
        SweepTestResult& outResult,
        uint32_t numIgnoreObjects = 0,
        btCollisionObject** ignoreObjects = nullptr);

    void RegisterNode(Node* node);
    void UnregisterNode(Node* node);
    const std::vector<Audio3D*>& GetAudios() const;

    std::vector<Node*>& GetReplicatedNodeVector(ReplicationRate rate);
    uint32_t& GetReplicatedNodeIndex(ReplicationRate rate);
    uint32_t& GetIncrementalRepTier();
    uint32_t& GetIncrementalRepIndex();

    void QueueRootScene(const char* name);

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
    void PlaceNewlySpawnedNode(Node* node);

private:

    Node* mRootNode = nullptr;
    std::vector<Line> mLines;
    std::vector<class Light3D*> mLights;
    std::vector<class Audio3D*> mAudios;
    SceneRef mQueuedRootScene;
    glm::vec4 mAmbientLightColor;
    glm::vec4 mShadowColor;
    FogSettings mFogSettings;
    Camera3D* mActiveCamera;
    Node3D* mAudioReceiver;
    bool mPendingClear = false;

    // Replication tiers
    std::vector<Node*> mRepNodes[(uint32_t)ReplicationRate::Count];
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

    bool IsNodeSelected(Node* node) const;
    Node* GetSelectedNode();
    const std::vector<Node*>& GetSelectedNodes();
    void DeselectNode(Node* node);

#endif
};