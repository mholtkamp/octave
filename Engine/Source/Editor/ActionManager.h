#pragma once

#include <string>
#include <vector>

#include "EngineTypes.h"
#include "EditorTypes.h"
#include "Asset.h"
#include "AssetRef.h"
#include "Nodes/Node.h"

class Node3D;

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

    Node* SpawnBasicNode(const std::string& name, Node* parent, Asset* srcAsset = nullptr);
    Node* SpawnBasicNode(const std::string& name, glm::vec3 position, Asset* srcAsset = nullptr);

    void ExecuteAction(Action* action);
    void Undo();
    void Redo();

    // Actions
    void EXE_EditProperty(void* owner, PropertyOwnerType ownerType, const std::string& name, uint32_t index, Datum newValue);
    void EXE_EditTransform(Node3D* node, const glm::mat4& transform);
    void EXE_EditTransforms(const std::vector<Node3D*>& nodes, const std::vector<glm::mat4>& newTransforms);
    void EXE_SpawnNode(Node* node);
    void EXE_DeleteNode(Node* node);
    void EXE_SpawnNode(const std::vector<Node*>& nodes);
    void EXE_DeleteNode(const std::vector<Node*>& nodes);
    void EXE_AttachNode(Node* node, Node3D* newParent, int32_t boneIndex);
    void EXE_SetRootNode(Node* newRoot);
    void EXE_SetAbsoluteRotation(Node3D* node, glm::quat rot);
    void EXE_SetAbsolutePosition(Node3D* node, glm::vec3 pos);
    void EXE_SetAbsoluteScale(Node3D* node, glm::vec3 scale);

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
    void CreateNewProject();
    void OpenProject(const char* path = nullptr);
    void OpenScene();
    void OpenScene(Scene* scene);
    void SaveScene(bool saveAs);
    void SaveSelectedAsset();
    void DeleteSelectedNodes();
    void DeleteNode(Node* node);
    Asset* ImportAsset();
    Asset* ImportAsset(const std::string& path);
    void ImportScene(const SceneImportOptions& options);
    void ShowBuildDataPrompt();
    void BuildData(Platform platform, bool embedded);
    void ClearWorld();
    void DeleteAllNodes();
    void RecaptureAndSaveAllScenes();
    void ResaveAllAssets();
    void DeleteAsset(AssetStub* stub);
    void DeleteAssetDir(AssetDir* dir);
    void DuplicateNode(Node* node, bool recurse);
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
    ActionSpawnNodes(const std::vector<Node*>& nodes);
protected:
    std::vector<Node*> mNodes;
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
};

class ActionSetAbsoluteRotation : public Action
{
public:
    DECLARE_ACTION_INTERFACE(SetAbsoluteRotation)
    ActionSetAbsoluteRotation(Node3D* node, glm::quat rot);
protected:
    Node3D* mNode = nullptr;
    glm::quat mNewRotation;
    glm::quat mPrevRotation;
};

class ActionSetAbsolutePosition : public Action
{
public:
    DECLARE_ACTION_INTERFACE(SetAbsolutePosition)
    ActionSetAbsolutePosition(Node3D* node, glm::vec3 pos);
protected:
    Node3D* mNode = nullptr;
    glm::vec3 mNewPosition;
    glm::vec3 mPrevPosition;
};

class ActionSetAbsoluteScale : public Action
{
public:
    DECLARE_ACTION_INTERFACE(SetAbsoluteScale)
    ActionSetAbsoluteScale(Node3D* node, glm::vec3 scale);
protected:
    Node3D* mNode = nullptr;
    glm::vec3 mNewScale;
    glm::vec3 mPrevScale;
};
