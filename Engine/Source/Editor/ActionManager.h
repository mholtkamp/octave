#pragma once

#include <string>
#include <vector>

#include "EngineTypes.h"
#include "EditorTypes.h"
#include "Asset.h"
#include "AssetRef.h"
#include "Nodes/Node.h"
#include "Nodes/3D/StaticMesh3d.h"

class Node3D;
class InstancedMesh3D;
struct ActionSetInstanceColorsData;
struct MeshInstanceData;

class Action
{
public:
    virtual ~Action() {}
    virtual void Execute() = 0;
    virtual void Reverse() = 0;
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

    void ClearActionHistory();
    void ClearActionFuture();
    void ResetUndoRedo();
    void ExileNode(Node* node);
    void RestoreExiledNode(Node* node);

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

    void GatherScriptFiles(const std::string& dir, std::vector<std::string>& outFiles);

    std::vector<Action*> mActionHistory;
    std::vector<Action*> mActionFuture;
    std::vector<Node*> mExiledNodes;

public:

    // Actions
    void CreateNewProject(const char* folderPath = nullptr, bool cpp = false);
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
    void BuildData(Platform platform, bool embedded);
    void ClearWorld();
    void DeleteAllNodes();
    void RecaptureAndSaveAllScenes();
    void ResaveAllAssets();
    void DeleteAsset(AssetStub* stub);
    void DeleteAssetDir(AssetDir* dir);
    void DuplicateNodes(std::vector<Node*> nodes);
    void AttachSelectedNodes(Node* newParent, int32_t boneIdx);
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

    const std::vector<Node*>& GetNodes() const { return mNodes; }

protected:

    // Action inputs, only one should be filled.
    std::vector<TypeId> mSrcTypes;
    std::vector<const char*> mSrcTypeNames;
    std::vector<SceneRef> mSrcScenes;
    std::vector<Node*> mSrcNodes;

    // Populated after first Execute()
    std::vector<Node*> mNodes;

    // Populated after first Reverse()
    std::vector<Node*> mParents;
};

class ActionDeleteNodes : public Action
{
public:
    DECLARE_ACTION_INTERFACE(DeleteNodes)
    ActionDeleteNodes(const std::vector<Node*>& nodes);
protected:
    std::vector<Node*> mNodes;
    std::vector<Node*> mParents;
    std::vector<int32_t> mChildIndices;
    std::vector<int32_t> mBoneIndices;
};

class ActionAttachNode : public Action
{
public:
    DECLARE_ACTION_INTERFACE(AttachNode)
    ActionAttachNode(Node* node, Node* newParent, int32_t childIndex, int32_t boneIndex);
protected:
    Node* mNode = nullptr;
    Node* mNewParent = nullptr;
    Node* mPrevParent = nullptr;
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
    Node* mNewRoot = nullptr;
    Node* mOldRoot = nullptr;
    Node* mNewRootParent = nullptr;
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
