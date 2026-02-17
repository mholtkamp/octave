#pragma once

#include <string>
#include <vector>
#include <map>

#include "EngineTypes.h"
#include "EditorTypes.h"
#include "Asset.h"
#include "AssetRef.h"
#include "Nodes/Node.h"
#include "Nodes/3D/StaticMesh3d.h"

class Node3D;
class Mesh3D;
class InstancedMesh3D;
class Timeline;
struct ActionSetInstanceColorsData;
struct MeshInstanceData;

class Action
{
public:
    virtual ~Action() {}
    virtual void Execute();
    virtual void Reverse();
    virtual const char* GetName() = 0;
};

class ActionManager
{
public:

    ~ActionManager();

    static void Create();
    static void Destroy();
    static ActionManager* Get();

    void Update();

    void OnSelectedNodeChanged();

    Node* SpawnNode(TypeId nodeType, Node* parent);
    Node* SpawnNode(TypeId nodeType, glm::vec3 position);

    Node* SpawnBasicNode(const std::string& name, Node* parent, Asset* srcAsset = nullptr, bool setWorldPos = false, glm::vec3 worldPos = { 0.0f, 0.0f, 0.0f });

    void ExecuteAction(Action* action);
    void Undo();
    void Redo();

    // Actions
    void EXE_EditProperty(void* owner, PropertyOwnerType ownerType, const std::string& name, uint32_t index, Datum newValue);
    void EXE_EditTransform(Node3D* node, const glm::mat4& transform);
    void EXE_EditTransforms(const std::vector<Node3D*>& nodes, const std::vector<glm::mat4>& newTransforms);
    Node* EXE_SpawnNode(TypeId srcType);
    Node* EXE_SpawnNode(const char* srcTypeName);
    Node* EXE_SpawnNode(Scene* srcScene);
    Node* EXE_SpawnNode(Node* srcNode);
    void EXE_DeleteNode(Node* node);
    std::vector<Node*> EXE_SpawnNodes(const std::vector<Node*>& nodes);
    void EXE_DeleteNodes(const std::vector<Node*>& nodes);
    void EXE_AttachNode(Node* node, Node* newParent, int32_t childIndex, int32_t boneIndex);
    void EXE_SetRootNode(Node* newRoot);
    void EXE_SetWorldRotation(Node3D* node, glm::quat rot);
    void EXE_SetWorldPosition(Node3D* node, glm::vec3 pos);
    void EXE_SetWorldScale(Node3D* node, glm::vec3 scale);
    void EXE_UnlinkScene(Node* node);
    void EXE_SetInstanceColors(const std::vector<ActionSetInstanceColorsData>& data);
    void EXE_SetInstanceData(InstancedMesh3D* instMesh, int32_t startIndex, const std::vector<MeshInstanceData>& data);

    /**
     * @brief Replace selected nodes' mesh/material with an asset, or replace with scene instances.
     * @param asset The asset to apply (StaticMesh, Material, or Scene).
     * @param nodes The nodes to operate on.
     */
    void EXE_ReplaceSelectedWithAsset(Asset* asset, const std::vector<Node*>& nodes);

    /**
     * @brief Replace selected StaticMesh3D nodes with InstancedMesh3D nodes.
     * @param nodes The selected nodes to convert.
     * @param merge If true, group by mesh into one InstancedMesh3D each. If false, convert each node 1:1.
     */
    void EXE_ReplaceWithInstancedMesh(const std::vector<Node*>& nodes, bool merge);

    /**
     * @brief Split selected InstancedMesh3D nodes into individual StaticMesh3D nodes.
     * @param nodes The selected nodes to split.
     */
    void EXE_ReplaceWithStaticMesh(const std::vector<Node*>& nodes);

    // Timeline actions
    void EXE_TimelineAddTrack(Timeline* timeline, TypeId trackType);
    void EXE_TimelineRemoveTrack(Timeline* timeline, int32_t trackIndex);
    void EXE_TimelineAddClip(Timeline* timeline, int32_t trackIndex, TypeId clipType, float startTime, float duration);
    void EXE_TimelineRemoveClip(Timeline* timeline, int32_t trackIndex, int32_t clipIndex);
    void EXE_TimelineMoveClip(Timeline* timeline, int32_t trackIndex, int32_t clipIndex, float oldStartTime, float newStartTime);
    void EXE_TimelineBindTrack(Timeline* timeline, int32_t trackIndex, uint64_t oldUuid, uint64_t newUuid, const std::string& oldName, const std::string& newName);

    void ClearActionHistory();
    void ClearActionFuture();
    void ResetUndoRedo();
    void ExileNode(NodePtr node);
    void RestoreExiledNode(NodePtr node);
    void GatherScriptFiles(const std::string& dir, std::vector<std::string>& outFiles);

protected:

    static ActionManager* sInstance;
    ActionManager();

    void GenerateEmbeddedAssetFiles(
        std::vector<std::pair<AssetStub*, std::string> >& assets,
        const char* headerPath,
        const char* sourcePath);

    void GenerateEmbeddedScriptFiles(
        std::vector<std::string> files,
        const char* headerPath,
        const char* sourcePath);


    std::vector<Action*> mActionHistory;
    std::vector<Action*> mActionFuture;
    std::vector<NodePtr> mExiledNodes;

public:

    // Actions
    void CreateNewProject(const char* folderPath = nullptr, bool cpp = false, const char* defaultSceneName = "SC_Default");
    void OpenProject(const char* path = nullptr);
    void OpenScene();
    void OpenScene(Scene* scene);
    void SaveScene(bool saveAs);
    void SaveSelectedAsset();
    void DeleteSelectedNodes();
    void DeleteNode(Node* node);
    void RunScript();
    void ImportAsset();
    Asset* ImportAsset(const std::string& path);
    void ImportCamera(const CameraImportOptions& options);
    void ImportScene(const SceneImportOptions& options);
    void BeginImportScene();
    void BeginReimportScene(AssetStub* sceneStub);
    void BeginImportCamera();
    void BuildData(Platform platform, bool embedded);
    void PrepareRelease();
    void ClearWorld();
    void DeleteAllNodes();
    void RecaptureAndSaveAllScenes();
    void ResaveAllAssets();
    void DeleteAsset(AssetStub* stub);
    void DeleteAssetDir(AssetDir* dir);
    bool DuplicateNodes(std::vector<Node*> nodes);
    void AttachSelectedNodes(Node* newParent, int32_t boneIdx);

    // Project upgrade functions
    bool CheckProjectNeedsUpgrade();
    void UpgradeProject();
};

#define DECLARE_ACTION_INTERFACE(Name) \
    virtual void Execute() override; \
    virtual void Reverse() override; \
    virtual const char* GetName() { return #Name; }

// Actions

class ActionEditProperty : public Action
{
public:
    DECLARE_ACTION_INTERFACE(EditProperty)

    ActionEditProperty(
        void* owner,
        PropertyOwnerType ownerType,
        const std::string& propName,
        uint32_t index,
        Datum value);

protected:

    void GatherProps(std::vector<Property>& props);
    Property* FindProp(std::vector<Property>& props, const std::string& name);

    void* mOwner = nullptr;
    PropertyOwnerType mOwnerType = PropertyOwnerType::Count;
    std::string mPropertyName;
    uint32_t mIndex = 0;
    Datum mValue;

    AssetRef mReferencedAsset;
    Datum mPreviousValue;
};

class ActionEditTransforms : public Action
{
public:
    DECLARE_ACTION_INTERFACE(EditTransforms)

    ActionEditTransforms(
        const std::vector<Node3D*>& nodes,
        const std::vector<glm::mat4>& newTransforms);

protected:
    std::vector<Node3D*> mNodes;
    std::vector<glm::mat4> mNewTransforms;
    std::vector<glm::mat4> mPrevTransforms;
};

class ActionSpawnNodes : public Action
{
public:
    DECLARE_ACTION_INTERFACE(SpawnNodes)
    ActionSpawnNodes(const std::vector<TypeId>& types);
    ActionSpawnNodes(const std::vector<const char*>& typeNames);
    ActionSpawnNodes(const std::vector<SceneRef>& scenes);
    ActionSpawnNodes(const std::vector<Node*>& srcNodes);

    const std::vector<NodePtr>& GetNodes() const { return mNodes; }

protected:

    // Action inputs, only one should be filled.
    std::vector<TypeId> mSrcTypes;
    std::vector<const char*> mSrcTypeNames;
    std::vector<SceneRef> mSrcScenes;
    std::vector<NodePtr> mSrcNodes;

    // Populated after first Execute()
    std::vector<NodePtr> mNodes;

    // Populated after first Reverse()
    std::vector<NodePtr> mParents;
};

class ActionDeleteNodes : public Action
{
public:
    DECLARE_ACTION_INTERFACE(DeleteNodes)
    ActionDeleteNodes(const std::vector<Node*>& nodes);
protected:
    std::vector<NodePtr> mNodes;
    std::vector<NodePtr> mParents;
    std::vector<int32_t> mChildIndices;
    std::vector<int32_t> mBoneIndices;
};

class ActionAttachNode : public Action
{
public:
    DECLARE_ACTION_INTERFACE(AttachNode)
    ActionAttachNode(Node* node, Node* newParent, int32_t childIndex, int32_t boneIndex);
protected:
    NodePtr mNode = nullptr;
    NodePtr mNewParent = nullptr;
    NodePtr mPrevParent = nullptr;
    int32_t mChildIndex = -1;
    int32_t mPrevChildIndex = -1;
    int32_t mBoneIndex = -1;
    int32_t mPrevBoneIndex = -1;
};

class ActionSetRootNode : public Action
{
public:
    DECLARE_ACTION_INTERFACE(SetRootNode)
    ActionSetRootNode(Node* newRoot);
protected:
    NodePtr mNewRoot = nullptr;
    NodePtr mOldRoot = nullptr;
    NodePtr mNewRootParent = nullptr;
    int32_t mNewRootChildIndex = -1;
};

class ActionSetWorldRotation : public Action
{
public:
    DECLARE_ACTION_INTERFACE(SetWorldRotation)
    ActionSetWorldRotation(Node3D* node, glm::quat rot);
protected:
    Node3D* mNode = nullptr;
    glm::quat mNewRotation;
    glm::quat mPrevRotation;
};

class ActionSetWorldPosition : public Action
{
public:
    DECLARE_ACTION_INTERFACE(SetWorldPosition)
    ActionSetWorldPosition(Node3D* node, glm::vec3 pos);
protected:
    Node3D* mNode = nullptr;
    glm::vec3 mNewPosition;
    glm::vec3 mPrevPosition;
};

class ActionSetWorldScale : public Action
{
public:
    DECLARE_ACTION_INTERFACE(SetWorldScale)
    ActionSetWorldScale(Node3D* node, glm::vec3 scale);
protected:
    Node3D* mNode = nullptr;
    glm::vec3 mNewScale;
    glm::vec3 mPrevScale;
};

class ActionUnlinkScene : public Action
{
public:
    DECLARE_ACTION_INTERFACE(UnlinkScene);
    ActionUnlinkScene(Node* node);

protected:

    Node* mNode = nullptr;
    SceneRef mScene;

};

struct ActionSetInstanceColorsData
{
    StaticMesh3D* mMesh3d = nullptr;
    std::vector<uint32_t> mColors;
    bool mBakedLight = false;
};

class ActionSetInstanceColors : public Action
{
public:
    DECLARE_ACTION_INTERFACE(SetInstanceColors);
    ActionSetInstanceColors(const std::vector<ActionSetInstanceColorsData>& data);

protected:

    std::vector<ActionSetInstanceColorsData> mData;
    std::vector<ActionSetInstanceColorsData> mPrevData;
};

class ActionSetInstanceData : public Action
{
public:
    DECLARE_ACTION_INTERFACE(SetInstanceData);
    ActionSetInstanceData(InstancedMesh3D* instMesh, int32_t startIndex, const std::vector<MeshInstanceData>& data);

protected:
    InstancedMesh3D* mInstMesh = nullptr;
    int32_t mStartIndex = -1;

    std::vector<MeshInstanceData> mData;
    std::vector<MeshInstanceData> mPrevData;
};

/**
 * @brief Action to replace selected nodes' mesh or material with an asset, or replace with scene instances.
 */
class ActionReplaceWithAsset : public Action
{
public:
    DECLARE_ACTION_INTERFACE(ReplaceWithAsset);
    ActionReplaceWithAsset(Asset* asset, const std::vector<Node*>& nodes);

protected:

    enum class ReplaceMode
    {
        StaticMesh,
        Material,
        Scene,
        Invalid
    };

    AssetRef mAsset;
    ReplaceMode mMode = ReplaceMode::Invalid;

    // For StaticMesh mode
    std::vector<StaticMesh3D*> mMeshNodes;
    std::vector<StaticMeshRef> mPrevMeshes;

    // For Material mode
    std::vector<Mesh3D*> mMatNodes;
    std::vector<MaterialRef> mPrevMaterials;

    // For Scene mode
    struct SceneReplaceEntry
    {
        NodePtr mOriginal;
        NodePtr mParent;
        int32_t mChildIndex = -1;
        NodePtr mSpawnedNode;
    };
    std::vector<SceneReplaceEntry> mSceneEntries;
};

/**
 * @brief Action to merge selected StaticMesh3D nodes into InstancedMesh3D nodes grouped by mesh.
 */
class ActionReplaceWithInstancedMesh : public Action
{
public:
    DECLARE_ACTION_INTERFACE(ReplaceWithInstancedMesh);
    ActionReplaceWithInstancedMesh(const std::vector<Node*>& nodes, bool merge);

protected:

    struct GroupEntry
    {
        std::vector<NodePtr> mOriginalNodes;
        std::vector<NodePtr> mOriginalParents;
        std::vector<int32_t> mOriginalChildIndices;
        NodePtr mInstancedNode;
        NodePtr mInstancedParent;
        int32_t mInstancedChildIndex = -1;
    };

    std::vector<GroupEntry> mGroups;
    bool mFirstExecute = true;
};

/**
 * @brief Action to split selected InstancedMesh3D nodes into individual StaticMesh3D nodes.
 */
class ActionReplaceWithStaticMesh : public Action
{
public:
    DECLARE_ACTION_INTERFACE(ReplaceWithStaticMesh);
    ActionReplaceWithStaticMesh(const std::vector<Node*>& nodes);

protected:

    struct SplitEntry
    {
        NodePtr mOriginalNode;
        NodePtr mOriginalParent;
        int32_t mOriginalChildIndex = -1;
        std::vector<NodePtr> mCreatedNodes;
    };

    std::vector<SplitEntry> mEntries;
    bool mFirstExecute = true;
};
