#pragma once

#include <string>
#include <vector>

#include "EngineTypes.h"
#include "Asset.h"
#include "AssetRef.h"

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

    void ClearActionHistory();
    void ClearActionFuture();
    void ResetUndoRedo();

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
    void ImportScene();
    void ShowBuildDataPrompt();
    void BuildData(Platform platform, bool embedded);
    void ClearWorld();
    void RecaptureAndSaveAllLevels();
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
class ActionSelectComponent : public Action
{
public:
    DECLARE_ACTION_INTERFACE(SelectComponent)
    Component* mComponent = nullptr;
protected:
    std::vector<Component*> mPrevComponents;
};

class ActionEditProperty : public Action
{
public:
    DECLARE_ACTION_INTERFACE(EditProperty)

    ActionEditProperty(void* owner, PropertyOwnerType ownerType, const std::string& propName, uint32_t index, Datum value);

    void* mOwner = nullptr;
    PropertyOwnerType mOwnerType = PropertyOwnerType::Count;
    std::string mPropertyName;
    uint32_t mIndex = 0;
    Datum mValue;
protected:
    void GatherProps(std::vector<Property>& props);
    Property* FindProp(std::vector<Property>& props, const std::string& name);

    AssetRef mReferencedAsset;
    Datum mPreviousValue;
};
