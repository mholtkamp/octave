#pragma once

#include <string>
#include <vector>

#include "EngineTypes.h"
#include "EditorTypes.h"
#include "Asset.h"
#include "AssetRef.h"

class TransformComponent;

class Action
{
public:
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

    void SpawnActor(TypeId actorType, glm::vec3 position);
    void SpawnBasicActor(const std::string& name, glm::vec3 position, Asset* srcAsset = nullptr);

    void ExecuteAction(Action* action);
    void Undo();
    void Redo();

    // Actions
    void EXE_EditProperty(void* owner, PropertyOwnerType ownerType, const std::string& name, uint32_t index, Datum newValue);
    void EXE_EditTransforms(const std::vector<TransformComponent*>& transComps, const std::vector<glm::mat4>& newTransforms);
    void EXE_SpawnActor(Actor* actor);
    void EXE_DeleteActor(Actor* actor);
    void EXE_SpawnActors(const std::vector<Actor*>& actors);
    void EXE_DeleteActors(const std::vector<Actor*>& actors);
    void EXE_AddComponent(Component* comp);
    void EXE_RemoveComponent(Component* comp);
    void EXE_AttachComponent(TransformComponent* comp, TransformComponent* newParent);

    void ClearActionHistory();
    void ClearActionFuture();
    void ResetUndoRedo();
    void ExileActor(Actor* actor);
    void RestoreExiledActor(Actor* actor);
    void ExileComponent(Component* comp);
    void RestoreExiledComponent(Component* comp);

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
        const std::vector<TransformComponent*>& transComps,
        const std::vector<glm::mat4>& newTransforms);

protected:
    std::vector<TransformComponent*> mTransComps;
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
    TransformComponent* mParent = nullptr;
};

class ActionRemoveComponent : public Action
{
public:
    DECLARE_ACTION_INTERFACE(RemoveComponent)
    ActionRemoveComponent(Component* comp);
protected:
    Component* mComponent = nullptr;
    Actor* mOwner = nullptr;
    TransformComponent* mParent = nullptr;
};

class ActionAttachComponent : public Action
{
public:
    DECLARE_ACTION_INTERFACE(AttachComponent)
    ActionAttachComponent(TransformComponent* comp, TransformComponent* newParent);
protected:
    TransformComponent* mComponent = nullptr;
    TransformComponent* mNewParent = nullptr;
    TransformComponent* mPrevParent = nullptr;
};
