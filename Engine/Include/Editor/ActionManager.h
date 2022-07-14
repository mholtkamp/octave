#pragma once

#include <string>
#include <vector>

#include "EngineTypes.h"
#include "Asset.h"

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
    void ImportAsset();
    void ImportScene();
    void ShowBuildDataPrompt();
    void BuildData(Platform platform, bool embedded);
    void ClearWorld();
    void RecaptureAndSaveAllLevels();
    void DeleteAsset(AssetStub* stub);
    void DeleteAssetDir(AssetDir* dir);
    void DuplicateActor(Actor* actor);
};
