#pragma once

#include <string>
#include <vector>

#include "EngineTypes.h"
#include "EditorTypes.h"
#include "Asset.h"
#include "AssetRef.h"
#include "Nodes/Widgets/Widget.h"

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

    void OnSelectedActorChanged();
    void OnSelectedComponentChanged();

    Actor* SpawnActor(TypeId actorType, glm::vec3 position);
    Actor* SpawnBasicActor(const std::string& name, glm::vec3 position, Asset* srcAsset = nullptr);

    void ExecuteAction(Action* action);
    void Undo();
    void Redo();

    // Actions
    void EXE_EditProperty(void* owner, PropertyOwnerType ownerType, const std::string& name, uint32_t index, Datum newValue);
    void EXE_EditTransform(Node3D* transComp, const glm::mat4& transform);
    void EXE_EditTransforms(const std::vector<Node3D*>& transComps, const std::vector<glm::mat4>& newTransforms);
    void EXE_SpawnActor(Actor* actor);
    void EXE_DeleteActor(Actor* actor);
    void EXE_SpawnActors(const std::vector<Actor*>& actors);
    void EXE_DeleteActors(const std::vector<Actor*>& actors);
    void EXE_AddComponent(Component* comp);
    void EXE_RemoveComponent(Component* comp);
    void EXE_AttachComponent(Node3D* comp, Node3D* newParent, int32_t boneIndex);
    void EXE_SetRootComponent(Node3D* newRoot);
    void EXE_SetAbsoluteRotation(Node3D* comp, glm::quat rot);
    void EXE_SetAbsolutePosition(Node3D* comp, glm::vec3 pos);
    void EXE_SetAbsoluteScale(Node3D* comp, glm::vec3 scale);
    void EXE_AddWidget(Widget* widget);
    void EXE_RemoveWidget(Widget* widget);
    void EXE_AttachWidget(Widget* widget, Widget* newParent, int32_t index = -1);
    void EXE_SetRootWidget(Widget* newRoot);

    void ClearActionHistory();
    void ClearActionFuture();
    void ResetUndoRedo();
    void ExileActor(Actor* actor);
    void RestoreExiledActor(Actor* actor);
    void ExileComponent(Component* comp);
    void RestoreExiledComponent(Component* comp);
    void ExileWidget(Widget* widget);
    void RestoreExiledWidget(Widget* widget);

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
    std::vector<Actor*> mExiledActors;
    std::vector<Component*> mExiledComponents;
    std::vector<Widget*> mExiledWidgets;

public:

    // Actions
    void CreateNewProject();
    void OpenProject(const char* path = nullptr);
    void OpenLevel();
    void OpenLevel(Level* level);
    void SaveLevel(bool saveAs);
    void SaveSelectedAsset();
    void DeleteSelectedActors();
    void DeleteActor(Actor* actor);
    Asset* ImportAsset();
    Asset* ImportAsset(const std::string& path);
    void ImportScene(const SceneImportOptions& options);
    void ShowBuildDataPrompt();
    void BuildData(Platform platform, bool embedded);
    void ClearWorld();
    void DeleteAllActors();
    void RecaptureAndSaveAllLevels();
    void RecaptureAndSaveAllBlueprints();
    void ResaveAllAssets();
    void DeleteAsset(AssetStub* stub);
    void DeleteAssetDir(AssetDir* dir);
    void DuplicateActor(Actor* actor);
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
        const std::vector<Node3D*>& transComps,
        const std::vector<glm::mat4>& newTransforms);

protected:
    std::vector<Node3D*> mTransComps;
    std::vector<glm::mat4> mNewTransforms;
    std::vector<glm::mat4> mPrevTransforms;
};

class ActionSpawnActors : public Action
{
public:
    DECLARE_ACTION_INTERFACE(SpawnActors)
    ActionSpawnActors(const std::vector<Actor*>& actors);
protected:
    std::vector<Actor*> mActors;
};

class ActionDeleteActors : public Action
{
public:
    DECLARE_ACTION_INTERFACE(DeleteActors)
    ActionDeleteActors(const std::vector<Actor*>& actors);
protected:
    std::vector<Actor*> mActors;
};

class ActionAddComponent : public Action
{
public:
    DECLARE_ACTION_INTERFACE(AddComponent)
    ActionAddComponent(Component* comp);
protected:
    Component* mComponent = nullptr;
    Actor* mOwner = nullptr;
    Node3D* mParent = nullptr;
};

class ActionRemoveComponent : public Action
{
public:
    DECLARE_ACTION_INTERFACE(RemoveComponent)
    ActionRemoveComponent(Component* comp);
protected:
    Component* mComponent = nullptr;
    Actor* mOwner = nullptr;
    Node3D* mParent = nullptr;
};

class ActionAttachComponent : public Action
{
public:
    DECLARE_ACTION_INTERFACE(AttachComponent)
    ActionAttachComponent(Node3D* comp, Node3D* newParent, int32_t boneIndex);
protected:
    Node3D* mComponent = nullptr;
    Node3D* mNewParent = nullptr;
    Node3D* mPrevParent = nullptr;
    int32_t mBoneIndex = -1;
    int32_t mPrevBoneIndex = -1;
};

class ActionSetRootComponent : public Action
{
public:
    DECLARE_ACTION_INTERFACE(SetRootComponent)
    ActionSetRootComponent(Node3D* newRoot);
protected:
    Node3D* mNewRoot = nullptr;
    Node3D* mOldRoot = nullptr;
};

class ActionSetAbsoluteRotation : public Action
{
public:
    DECLARE_ACTION_INTERFACE(SetAbsoluteRotation)
    ActionSetAbsoluteRotation(Node3D* comp, glm::quat rot);
protected:
    Node3D* mComponent = nullptr;
    glm::quat mNewRotation;
    glm::quat mPrevRotation;
};

class ActionSetAbsolutePosition : public Action
{
public:
    DECLARE_ACTION_INTERFACE(SetAbsolutePosition)
    ActionSetAbsolutePosition(Node3D* comp, glm::vec3 pos);
protected:
    Node3D* mComponent = nullptr;
    glm::vec3 mNewPosition;
    glm::vec3 mPrevPosition;
};

class ActionSetAbsoluteScale : public Action
{
public:
    DECLARE_ACTION_INTERFACE(SetAbsoluteScale)
    ActionSetAbsoluteScale(Node3D* comp, glm::vec3 scale);
protected:
    Node3D* mComponent = nullptr;
    glm::vec3 mNewScale;
    glm::vec3 mPrevScale;
};

class ActionAddWidget : public Action
{
public:
    DECLARE_ACTION_INTERFACE(AddWidget)
    ActionAddWidget(Widget* widget);
protected:
    Widget* mWidget = nullptr;
    Widget* mParent = nullptr;
    bool mHasExecuted = false;
};

class ActionRemoveWidget : public Action
{
public:
    DECLARE_ACTION_INTERFACE(RemoveWidget)
    ActionRemoveWidget(Widget* widget);
protected:
    Widget* mWidget = nullptr;
    Widget* mParent = nullptr;
    int32_t mPrevIndex = -1;
    bool mWasRoot = false;
};

class ActionAttachWidget : public Action
{
public:
    DECLARE_ACTION_INTERFACE(AttachWidget)
    ActionAttachWidget(Widget* widget, Widget* newParent, int32_t index);
protected:
    Widget* mWidget = nullptr;
    Widget* mNewParent = nullptr;
    int32_t mNewIndex = -1;
    Widget* mPrevParent = nullptr;
    int32_t mPrevIndex = -1;
};

class ActionSetRootWidget : public Action
{
public:
    DECLARE_ACTION_INTERFACE(SetRootWidget)
    ActionSetRootWidget(Widget* newRoot);
protected:
    Widget* mNewRoot = nullptr;
    Widget* mOldRoot = nullptr;
};
