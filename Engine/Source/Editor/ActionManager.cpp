#if EDITOR

#include "ActionManager.h"

#if PLATFORM_WINDOWS
#include <Windows.h>
#include <ShlObj.h>
#endif

#include <stdint.h>

#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <algorithm>

#include "Log.h"
#include "EditorConstants.h"
#include "Constants.h"
#include "World.h"
#include "Engine.h"
#include "Assets/Level.h"
#include "AssetManager.h"
#include "EditorState.h"
#include "PanelManager.h"
#include "Widgets/AssetsPanel.h"
#include "Widgets/HierarchyPanel.h"
#include "Widgets/ActionList.h"
#include "Assets/Texture.h"
#include "Assets/StaticMesh.h"
#include "Assets/SkeletalMesh.h"
#include "Assets/SoundWave.h"
#include "Assets/Blueprint.h"
#include "Assets/Font.h"
#include "AssetDir.h"
#include "EmbeddedFile.h"
#include "Utilities.h"
#include "EditorUtils.h"
#include "Log.h"
#include "StaticMeshActor.h"

#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/TransformComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/ParticleComponent.h"
#include "Components/AudioComponent.h"
#include "Components/ScriptComponent.h"
#include "Components/ShadowMeshComponent.h"
#include "Components/TextMeshComponent.h"

#include "System/System.h"

#if EDITOR
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#endif

#define STANDALONE_RELEASE 0

ActionManager* ActionManager::sInstance = nullptr;

TypeId CheckDaeAssetType(const char* path)
{
    TypeId retType = 0;

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_FlipUVs);

    if (scene != nullptr &&
        scene->mNumMeshes >= 1)
    {
        retType = scene->mMeshes[0]->HasBones() ? SkeletalMesh::GetStaticType() : StaticMesh::GetStaticType();
    }

    return retType;
}

void ActionManager::Create()
{
    Destroy();
    sInstance = new ActionManager();
}

void ActionManager::Destroy()
{
    if (sInstance != nullptr)
    {
        delete sInstance;
        sInstance = nullptr;
    }
}

ActionManager* ActionManager::Get()
{
    return sInstance;
}

ActionManager::ActionManager()
{

}

ActionManager::~ActionManager()
{

}

void ActionManager::Update()
{

}

void HandleBuildButtonPressed(Button* button)
{
    std::string buttonText = button->GetTextString();
    ActionManager* am = ActionManager::Get();

    if (buttonText == "Windows")
    {
        am->BuildData(Platform::Windows, false);

    }
    else if (buttonText == "Linux")
    {
        am->BuildData(Platform::Linux, false);
    }
    else if (buttonText == "GameCube")
    {
        am->BuildData(Platform::GameCube, false);
    }
    else if (buttonText == "Wii")
    {
        am->BuildData(Platform::Wii, false);
    }
    else if (buttonText == "3DS")
    {
        am->BuildData(Platform::N3DS, false);
    }
    else if (buttonText == "GameCube Embedded")
    {
        am->BuildData(Platform::GameCube, true);
    }
    else if (buttonText == "Wii Embedded")
    {
        am->BuildData(Platform::Wii, true);
    }
    else if (buttonText == "3DS Embedded")
    {
        am->BuildData(Platform::N3DS, true);
    }

    GetActionList()->Hide();
}

void ActionManager::ShowBuildDataPrompt()
{
    std::vector<std::string> actions;
#if PLATFORM_WINDOWS
    actions.push_back("Windows");
#elif PLATFORM_LINUX
    actions.push_back("Linux");
#endif
    actions.push_back("GameCube");
    actions.push_back("Wii");
    actions.push_back("3DS");
    actions.push_back("GameCube Embedded");
    actions.push_back("Wii Embedded");
    actions.push_back("3DS Embedded");
    GetActionList()->SetActions(actions, HandleBuildButtonPressed);
}

void ActionManager::BuildData(Platform platform, bool embedded)
{
    const EngineState* engineState = GetEngineState();
    bool standalone = engineState->mStandalone;
    const std::string& projectDir = engineState->mProjectDirectory;
    const std::string& projectName = engineState->mProjectName;

    std::vector<std::pair<AssetStub*, std::string> > embeddedAssets;

    if (projectDir == "")
    {
        LogError("Project directory not set?");
        return;
    }

    // Build Data is responsible for 3 things
    // (1) Create a Packaged directory in ProjectDir/Packaged. Erase previous packaged first.
    std::string packagedDir = projectDir + "Packaged/";

    // Create top level Packaged dir first.
    if (!DoesDirExist(packagedDir.c_str()))
    {
        CreateDir(packagedDir.c_str());
    }

    // Create platform-specific packaged dir. Delete old platform dir if it exists.
    packagedDir += GetPlatformString(platform);
    packagedDir += "/";
    if (DoesDirExist(packagedDir.c_str()))
    {
        RemoveDir(packagedDir.c_str());
    }

    CreateDir(packagedDir.c_str());

    // (2) Iterate over AssetDirs and save each file (platform-specific save) to the Packaged folder.
    std::function<void(AssetDir*, bool)> saveDir = [&](AssetDir* dir, bool engine)
    {
        std::string packDir;
        if (engine)
        {
            packDir = packagedDir + dir->mPath + "/";
        }
        else
        {
            packDir = dir->mPath;
            packDir = packDir.substr(projectDir.length());
            packDir = packagedDir + projectName + "/" + packDir;
        }

        if (!DoesDirExist(packDir.c_str()))
        {
            CreateDir(packDir.c_str());
        }

        // Cook the assets into our packaged folder
        for (uint32_t i = 0; i < dir->mAssetStubs.size(); ++i)
        {
            AssetStub* stub = dir->mAssetStubs[i];
            bool alreadyLoaded = (stub->mAsset != nullptr);

            if (!alreadyLoaded)
            {
                AssetManager::Get()->LoadAsset(*stub);
            }

            std::string packFile = packDir + stub->mAsset->GetName() + ".oct";
            stub->mAsset->SaveFile(packFile.c_str(), platform);

            if (true)
            {
                // Save the asset in the src location. There is probably a better time and place for this.
                AssetManager::Get()->SaveAsset(*stub);
            }

            // Currently either embed everything or embed nothing...
            // Embed flag on Asset does nothing, but if we want to keep that feature, then 
            // we need to load the asset if it's not loaded, add to embedded list if it's flagged and then probably unload it after.
            if (embedded)
            {
                embeddedAssets.push_back({ stub, packFile });
            }

            if (!alreadyLoaded)
            {
                AssetManager::Get()->UnloadAsset(*stub);
            }
        }

        // Cook child dirs
        for (uint32_t i = 0; i < dir->mChildDirs.size(); ++i)
        {
            saveDir(dir->mChildDirs[i], engine);
        }
    };

    AssetDir* engineAssetDir = AssetManager::Get()->FindEngineDirectory();
    AssetDir* projectAssetDir = AssetManager::Get()->FindProjectDirectory();
    std::string packEngineDir = packagedDir + engineAssetDir->mName + "/";
    std::string packProjectDir = packagedDir + projectAssetDir->mName + "/";
    CreateDir(packEngineDir.c_str());
    CreateDir(packProjectDir.c_str());

    saveDir(engineAssetDir, true);
    saveDir(projectAssetDir, false);

    // (3) Generate .cpp / .h files (empty if not embedded) using the .oct files in the Packaged folder.
    // (4) Create and save an asset registry file with simple list of asset paths into Packaged folder.
    std::unordered_map<std::string, AssetStub*>& assetMap = AssetManager::Get()->GetAssetMap();
    FILE* registryFile = nullptr;

    std::string registryFileName = packagedDir + projectName + "/AssetRegistry.txt";
    registryFile = fopen(registryFileName.c_str(), "w");

    for (auto pair : assetMap)
    {
        if (pair.second->mAsset != nullptr &&
            pair.second->mEngineAsset)
        {
            AssetManager::Get()->SaveAsset(*pair.second);
        }

        if (registryFile != nullptr)
        {
            std::string regPath = pair.second->mPath.c_str();
            if (!pair.second->mEngineAsset &&
                regPath.find(projectDir) != std::string::npos)
            {
                regPath = regPath.substr(projectDir.length());
                regPath = projectName + "/" + regPath;
            }

            fprintf(registryFile, "%s\n", regPath.c_str());
        }
    }

    if (registryFile != nullptr)
    {
        fclose(registryFile);
        registryFile = nullptr;
    }

    // Create a Generated folder inside the project folder if it doesn't exist
    if (!DoesDirExist((projectDir + "Generated").c_str()))
    {
        CreateDir((projectDir + "Generated").c_str());
    }

    std::string embeddedHeaderPath = projectDir + "Generated/EmbeddedAssets.h";
    std::string embeddedSourcePath = projectDir + "Generated/EmbeddedAssets.cpp";
    GenerateEmbeddedAssetFiles(embeddedAssets, embeddedHeaderPath.c_str(), embeddedSourcePath.c_str());

    // Generate embedded script source files. If not doing an embedded build, copy over the script folders.
    std::vector<std::string> scriptFiles;

    if (embedded)
    {
        GatherScriptFiles("Engine/Scripts/", scriptFiles);
        GatherScriptFiles(projectDir + "/Scripts/", scriptFiles);
    }
    else
    {
        SYS_Exec(std::string("cp -R Engine/Scripts " + packagedDir + "Engine/Scripts").c_str());
        SYS_Exec(std::string("cp -R " + projectDir + "Scripts " + packagedDir + projectName + "/Scripts").c_str());
    }

    // Generate embedded script source files, even if not doing an embedded build. 
    // So we don't need to worry about whether we include code that links to the embedded script array / script count.
    std::string scriptHeaderPath = projectDir + "Generated/EmbeddedScripts.h";
    std::string scriptSourcePath = projectDir + "Generated/EmbeddedScripts.cpp";
    GenerateEmbeddedScriptFiles(scriptFiles, scriptHeaderPath.c_str(), scriptSourcePath.c_str());

    if (standalone)
    {
        std::string copyGeneratedFolder = "cp -R " + projectDir + "Generated " + "Standalone";
        SYS_Exec(copyGeneratedFolder.c_str());
    }

    // ( ) Maybe copy Project .octp file into the Packaged folder? Are we actually using it?
    {
        std::string copyOctpCmd = "cp " + projectDir + projectName + ".octp " + packagedDir + projectName;
        SYS_Exec(copyOctpCmd.c_str());
    }

    // ( ) Run the makefile to compile the game.
#if STANDALONE_RELEASE
    bool needCompile = !standalone || embedded;
#else
    bool needCompile = true;
#endif
    std::string buildProjName = standalone ? "Standalone" : projectName;

    if (needCompile)
    {
        if (platform == Platform::Windows)
        {
            // If devenv can't be found, add it to your PATH
            // My devenv for VS2017 community was found here:
            // C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\IDE

            std::string solutionPath = "Octave.sln";
            if (engineState->mSolutionPath != "")
            {
                solutionPath = engineState->mSolutionPath;
            }

            std::string devenvCmd = std::string("devenv ") + solutionPath + " /Build \"Release|x64\" /Project " + buildProjName;

            SYS_Exec(devenvCmd.c_str());
        }
        else
        {
            std::string makefilePath = "Makefile_";

            switch (platform)
            {
            case Platform::Linux: makefilePath += "Linux_Game"; break;
            case Platform::GameCube: makefilePath += "GCN"; break;
            case Platform::Wii: makefilePath += "Wii"; break;
            case Platform::N3DS: makefilePath += "3DS"; break;
            default: assert(0); break;
            }

            std::string makeCmd = std::string("make -C ") + (standalone ? buildProjName : projectDir) + " -f " + makefilePath + " -j 6";
            SYS_Exec(makeCmd.c_str());
        }
    }
    else
    {
        // When running the standalone editor, we don't need to compile 
        // (unless we are embeddeding assets into the executable).
        // But we do need to copy over the Octave.exe and ideally rename it to the project name to make it more official :)
    }

    // ( ) Copy the executable into the Packaged folder.
    std::string exeSrc = (standalone ? buildProjName : projectDir) + "/Build/";

    if (!needCompile)
    {
        // Override exe path for uncompiled standalone builds
#if STANDALONE_RELEASE
        exeSrc = "Standalone/Binaries/";
#else
        exeSrc = "Standalone/Build/";
#endif
    }

    switch (platform)
    {
    case Platform::Windows: exeSrc += "Windows/x64/Release/"; break;
    case Platform::Linux: exeSrc += "Linux/"; break;
    case Platform::GameCube: exeSrc += "GCN/"; break;
    case Platform::Wii: exeSrc += "Wii/"; break;
    case Platform::N3DS: exeSrc += "3DS/"; break;
    default: assert(0); break;
    }

    exeSrc += standalone ? "Octave" : projectName;

    std::string extension = ".exe";

    switch (platform)
    {
    case Platform::Windows: extension = ".exe"; break;
    case Platform::Linux: extension = ".out"; break;
    case Platform::GameCube: extension = ".dol"; break;
    case Platform::Wii: extension = ".dol"; break;
    case Platform::N3DS: extension = ".3dsx"; break;
    default: assert(0); break;
    }

    exeSrc += extension;

    std::string exeCopyCmd = std::string("cp ") + exeSrc + " " + packagedDir;
    SYS_Exec(exeCopyCmd.c_str());

    if (standalone)
    {
        // Rename the executable to the project name
        std::string renameCmd = std::string("mv ") + packagedDir + "Octave" + extension + " " + packagedDir + projectName + extension;
        SYS_Exec(renameCmd.c_str());
    }

    // Write out an Engine.ini file which is used by Standalone game exe.
    FILE* engineIni = fopen(std::string(packagedDir + "Engine.ini").c_str(), "w");
    if (engineIni != nullptr)
    {
        fprintf(engineIni, "project=%s", projectName.c_str());

        fclose(engineIni);
        engineIni = nullptr;
    }

    // Handle SpirV shaders on Vulkan platforms
    if (platform == Platform::Windows ||
        platform == Platform::Linux)
    {
        // Compile shaders
        if (platform == Platform::Windows)
        {
            SYS_Exec("cd Engine/Shaders/GLSL && \"./compile.bat\"");
        }
        else
        {
            SYS_Exec("cd Engine/Shaders/GLSL && \"./compile.sh\"");
        }

        // Then copy over the binaries.
        CreateDir((packagedDir + "Engine/Shaders/").c_str());
        CreateDir((packagedDir + "Engine/Shaders/GLSL/").c_str());

        SYS_Exec(std::string("cp -R Engine/Shaders/GLSL/bin " + packagedDir + "Engine/Shaders/GLSL/bin").c_str());
    }

    LogDebug("Build Finished");
}

void ActionManager::OnSelectedActorChanged()
{

}

void ActionManager::OnSelectedComponentChanged()
{

}

void ActionManager::SpawnActor(TypeId actorType, glm::vec3 position)
{
    Actor* spawnedActor = GetWorld()->SpawnActor(actorType);

    if (spawnedActor->GetRootComponent() == nullptr)
    {
        TransformComponent* defaultRoot = spawnedActor->CreateComponent<TransformComponent>();
        defaultRoot->SetName("Default Root");
        spawnedActor->SetRootComponent(defaultRoot);
    }

    spawnedActor->SetPosition(position);
    SetSelectedActor(spawnedActor);
    EXE_SpawnActor(spawnedActor);
}

void ActionManager::SpawnBasicActor(const std::string& name, glm::vec3 position, Asset* srcAsset)
{
    Actor* spawnedActor = nullptr;

    if (srcAsset == nullptr)
    {
        srcAsset = GetSelectedAsset();
    }

    if (name == BASIC_STATIC_MESH)
    {
        spawnedActor = GetWorld()->SpawnActor<StaticMeshActor>();

        StaticMesh* mesh = (StaticMesh*) LoadAsset("SM_Cube");

        if (srcAsset != nullptr &&
            srcAsset->GetType() == StaticMesh::GetStaticType())
        {
            mesh = static_cast<StaticMesh*>(srcAsset);
        }

        static_cast<StaticMeshActor*>(spawnedActor)->GetStaticMeshComponent()->SetStaticMesh(mesh);
    }
    else if (name == BASIC_POINT_LIGHT)
    {
        // Spawn point light actor
        spawnedActor = GetWorld()->SpawnActor<Actor>();
        PointLightComponent* pointLightComp = spawnedActor->CreateComponent<PointLightComponent>();
        spawnedActor->SetRootComponent(pointLightComp);
        pointLightComp->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        pointLightComp->SetRadius(10.0f);
    }
    else if (name == BASIC_TRANSFORM)
    {
        spawnedActor = GetWorld()->SpawnActor<Actor>();
        TransformComponent* transformComp = spawnedActor->CreateComponent<TransformComponent>();
        spawnedActor->SetRootComponent(transformComp);
    }
    else if (name == BASIC_DIRECTIONAL_LIGHT)
    {
        spawnedActor = GetWorld()->SpawnActor<Actor>();
        DirectionalLightComponent* dirLightComp = spawnedActor->CreateComponent<DirectionalLightComponent>();
        spawnedActor->SetRootComponent(dirLightComp);
        GetWorld()->SetDirectionalLight(dirLightComp);
    }
    else if (name == BASIC_SKELETAL_MESH)
    {
        // TODO: Add a default SkeletalMesh to Engine assets
        SkeletalMesh* mesh = nullptr;

        if (srcAsset != nullptr &&
            srcAsset->GetType() == SkeletalMesh::GetStaticType())
        {
            mesh = static_cast<SkeletalMesh*>(srcAsset);

            // Spawn skeletal mesh actor.
            spawnedActor = GetWorld()->SpawnActor<Actor>();
            SkeletalMeshComponent* skeletalMeshComp = spawnedActor->CreateComponent<SkeletalMeshComponent>();
            spawnedActor->SetRootComponent(skeletalMeshComp);
            skeletalMeshComp->SetSkeletalMesh(mesh);
        }
    }
    else if (name == BASIC_BOX)
    {
        spawnedActor = GetWorld()->SpawnActor<Actor>();
        BoxComponent* boxComponent = spawnedActor->CreateComponent<BoxComponent>();
        spawnedActor->SetRootComponent(boxComponent);
    }
    else if (name == BASIC_SPHERE)
    {
        spawnedActor = GetWorld()->SpawnActor<Actor>();
        SphereComponent* sphereComponent = spawnedActor->CreateComponent<SphereComponent>();
        spawnedActor->SetRootComponent(sphereComponent);
    }
    else if (name == BASIC_CAPSULE)
    {
        spawnedActor = GetWorld()->SpawnActor<Actor>();
        CapsuleComponent* capsuleComponent = spawnedActor->CreateComponent<CapsuleComponent>();
        spawnedActor->SetRootComponent(capsuleComponent);
    }
    else if (name == BASIC_PARTICLE)
    {
        ParticleSystem* particleSystem = (ParticleSystem*) LoadAsset("P_DefaultParticle");

        if (srcAsset != nullptr &&
            srcAsset->GetType() == ParticleSystem::GetStaticType())
        {
            particleSystem = static_cast<ParticleSystem*>(srcAsset);
        }

        // Spawn a Particle actor
        spawnedActor = GetWorld()->SpawnActor<Actor>();
        ParticleComponent* particleComponent = spawnedActor->CreateComponent<ParticleComponent>();
        spawnedActor->SetRootComponent(particleComponent);
        particleComponent->SetParticleSystem(particleSystem);
    }
    else if (name == BASIC_AUDIO)
    {
        // TODO: Add a default sound in Engine assets
        SoundWave* soundWave = nullptr;

        if (srcAsset != nullptr &&
            srcAsset->GetType() == SoundWave::GetStaticType())
        {
            soundWave = static_cast<SoundWave*>(srcAsset);
        }

        if (soundWave != nullptr)
        {
            // Spawn an Audio actor
            spawnedActor = GetWorld()->SpawnActor<Actor>();
            AudioComponent* audioComponent = spawnedActor->CreateComponent<AudioComponent>();
            spawnedActor->SetRootComponent(audioComponent);
            audioComponent->SetSoundWave(soundWave);
            audioComponent->SetLoop(true);
            audioComponent->SetAutoPlay(true);
        }
    }
    else if (name == BASIC_BLUEPRINT)
    {
        Blueprint* bp = nullptr;

        if (srcAsset != nullptr &&
            srcAsset->GetType() == Blueprint::GetStaticType())
        {
            bp = static_cast<Blueprint*>(srcAsset);
        }

        if (bp != nullptr)
        {
            spawnedActor = bp->Instantiate(GetWorld());
        }
    }
    else if (name == BASIC_CAMERA)
    {
        spawnedActor = GetWorld()->SpawnActor<Actor>();
        CameraComponent* cameraComponent = spawnedActor->CreateComponent<CameraComponent>();
        spawnedActor->SetRootComponent(cameraComponent);
    }
    else if (name == BASIC_TEXT_MESH)
    {
        spawnedActor = GetWorld()->SpawnActor<Actor>();
        TextMeshComponent* textComponent = spawnedActor->CreateComponent<TextMeshComponent>();
        spawnedActor->SetRootComponent(textComponent);
    }

    if (spawnedActor != nullptr)
    {
        spawnedActor->SetPosition(position);
        SetSelectedActor(spawnedActor);
        EXE_SpawnActor(spawnedActor);
    }
    else
    {
        LogError("Failed to spawn basic actor: %s", name.c_str());
    }
}

void ActionManager::ExecuteAction(Action* action)
{
    assert(std::find(mActionHistory.begin(), mActionHistory.end(), action) == mActionHistory.end());
    assert(std::find(mActionFuture.begin(), mActionFuture.end(), action) == mActionFuture.end());

    action->Execute();

    // Don't record action history while playing in editor. Too chaotic for undo/redo
    if (IsPlayingInEditor())
    {
        delete action;
        action = nullptr;
    }
    else
    {
        // Limit max number of history?
        const uint32_t MaxActionHistoryCount = 100;
        if (mActionHistory.size() >= MaxActionHistoryCount)
        {
            mActionHistory.erase(mActionHistory.begin());
        }

        mActionHistory.push_back(action);
        ClearActionFuture();
    }
}

void ActionManager::Undo()
{
    if (mActionHistory.size() > 0)
    {
        Action* action = mActionHistory.back();
        mActionHistory.pop_back();

        LogDebug("Undo %s", action->GetName());
        action->Reverse();

        mActionFuture.push_back(action);
    }
}

void ActionManager::Redo()
{
    if (mActionFuture.size() > 0)
    {
        Action* action = mActionFuture.back();
        mActionFuture.pop_back();

        LogDebug("Redo %s", action->GetName());
        action->Execute();

        mActionHistory.push_back(action);
    }
}

void ActionManager::EXE_EditProperty(void* owner, PropertyOwnerType ownerType, const std::string& name, uint32_t index, Datum newValue)
{
    ActionEditProperty* action = new ActionEditProperty(owner, ownerType, name, index, newValue);
    ActionManager::Get()->ExecuteAction(action);
}

void ActionManager::EXE_EditTransforms(const std::vector<TransformComponent*>& transComps, const std::vector<glm::mat4>& newTransforms)
{
    ActionEditTransforms* action = new ActionEditTransforms(transComps, newTransforms);
    ActionManager::Get()->ExecuteAction(action);
}

void ActionManager::EXE_SpawnActor(Actor* actor)
{
    std::vector<Actor*> actors;
    actors.push_back(actor);

    ActionSpawnActors* action = new ActionSpawnActors(actors);
    ActionManager::Get()->ExecuteAction(action);
}

void ActionManager::EXE_DeleteActor(Actor* actor)
{
    std::vector<Actor*> actors;
    actors.push_back(actor);

    ActionDeleteActors* action = new ActionDeleteActors(actors);
    ActionManager::Get()->ExecuteAction(action);
}

void ActionManager::EXE_SpawnActors(const std::vector<Actor*>& actors)
{
    ActionSpawnActors* action = new ActionSpawnActors(actors);
    ActionManager::Get()->ExecuteAction(action);
}

void ActionManager::EXE_DeleteActors(const std::vector<Actor*>& actors)
{
    ActionDeleteActors* action = new ActionDeleteActors(actors);
    ActionManager::Get()->ExecuteAction(action);
}

void ActionManager::EXE_AddComponent(Component* comp)
{
    ActionAddComponent* action = new ActionAddComponent(comp);
    ActionManager::Get()->ExecuteAction(action);
}

void ActionManager::EXE_RemoveComponent(Component* comp)
{
    ActionRemoveComponent* action = new ActionRemoveComponent(comp);
    ActionManager::Get()->ExecuteAction(action);
}

void ActionManager::EXE_AttachComponent(TransformComponent* comp, TransformComponent* newParent)
{
    ActionAttachComponent* action = new ActionAttachComponent(comp, newParent);
    ActionManager::Get()->ExecuteAction(action);
}

void ActionManager::ClearActionHistory()
{
    for (uint32_t i = 0; i < mActionHistory.size(); ++i)
    {
        delete mActionHistory[i];
        mActionHistory[i] = nullptr;
    }

    mActionHistory.clear();
}

void ActionManager::ClearActionFuture()
{
    for (uint32_t i = 0; i < mActionFuture.size(); ++i)
    {
        delete mActionFuture[i];
        mActionFuture[i] = nullptr;
    }

    mActionFuture.clear();
}

void ActionManager::ResetUndoRedo()
{
    ClearActionHistory();
    ClearActionFuture();

    for (int32_t i = (int32_t)mExiledActors.size() - 1; i >= 0; --i)
    {
        GetWorld()->AddActor(mExiledActors[i]);
        GetWorld()->DestroyActor(mExiledActors[i]);
        mExiledActors.erase(mExiledActors.begin() + i);
    }

    for (int32_t i = (int32_t)mExiledComponents.size() - 1; i >= 0; --i)
    {
        mExiledComponents[i]->Destroy();
        mExiledComponents.erase(mExiledComponents.begin() + i);
    }
}

void ActionManager::ExileActor(Actor* actor)
{
    assert(std::find(mExiledActors.begin(), mExiledActors.end(), actor) == mExiledActors.end());
    assert(actor->GetWorld());
    GetWorld()->RemoveActor(actor);
    mExiledActors.push_back(actor);

    if (IsActorSelected(actor))
    {
        SetSelectedActor(nullptr);
    }
}

void ActionManager::RestoreExiledActor(Actor* actor)
{
    bool restored = false;

    for (uint32_t i = 0; i < mExiledActors.size(); ++i)
    {
        if (mExiledActors[i] == actor)
        {
            GetWorld()->AddActor(actor);
            mExiledActors.erase(mExiledActors.begin() + i);
            restored = true;
            break;
        }
    }

    assert(restored);
}

void ActionManager::ExileComponent(Component* comp)
{
    assert(std::find(mExiledComponents.begin(), mExiledComponents.end(), comp) == mExiledComponents.end());
    mExiledComponents.push_back(comp);

    if (IsComponentSelected(comp))
    {
        SetSelectedComponent(nullptr);
    }
}

void ActionManager::RestoreExiledComponent(Component* comp)
{
    bool restored = false;

    for (uint32_t i = 0; i < mExiledComponents.size(); ++i)
    {
        if (mExiledComponents[i] == comp)
        {
            mExiledComponents.erase(mExiledComponents.begin() + i);
            restored = true;
            break;
        }
    }

    assert(restored);
}

void ActionManager::CreateNewProject()
{
    std::string newProjDir = SYS_SelectFolderDialog();
    std::replace(newProjDir.begin(), newProjDir.end(), '\\', '/');

    std::string newProjName = newProjDir;
    size_t slashLoc = newProjName.find_last_of('/');
    if (slashLoc != std::string::npos)
    {
        newProjName = newProjName.substr(slashLoc + 1);
    }

    LogDebug("CreateNewProject: %s @ %s", newProjName.c_str(), newProjDir.c_str());

    // Now that we have the folder, we need to populate it with an Assets and Scripts folder
    std::string assetsFolder = newProjDir + "/Assets";
    std::string scriptsFolder = newProjDir + "/Scripts";
    SYS_CreateDirectory(assetsFolder.c_str());
    SYS_CreateDirectory(scriptsFolder.c_str());

    // Also we need to create an octp so that user can open the project with Ctrl+P
    std::string projectFile = newProjDir + "/" + newProjName.c_str() + ".octp";
    FILE* octpFile = fopen(projectFile.c_str(), "w");
    if (octpFile != nullptr)
    {
        fprintf(octpFile, "name=%s", newProjName.c_str());

        fclose(octpFile);
        octpFile = nullptr;
    }

    // Finally, open the project
    OpenProject(projectFile.c_str());
}

void ActionManager::OpenProject(const char* path)
{
    const char* projectPath = path;
    std::string openPath;

    if (projectPath == nullptr)
    {
        openPath = SYS_OpenFileDialog();

        // Display the Open dialog box. 
        if (openPath != "")
        {
            projectPath = openPath.c_str();
            LogDebug("ProjectDirectory = %s", projectPath);
        }
    }

    if (projectPath != nullptr)
    {
        LoadProject(projectPath);
    }

    // Handle new project directory
    PanelManager::Get()->GetAssetsPanel()->OnProjectDirectorySet();
}

void ActionManager::OpenLevel()
{
    if (GetEngineState()->mProjectPath == "")
        return;

    std::string openPath = SYS_OpenFileDialog();

    // Display the Open dialog box. 
    if (openPath != "")
    {
        std::string filename = strrchr(openPath.c_str(), '/') + 1;
        filename = filename.substr(0, filename.find_last_of('.'));
        AssetStub* stub = FetchAssetStub(filename);

        if (stub != nullptr &&
            stub->mType == Level::GetStaticType())
        {
            AssetManager::Get()->LoadAsset(*stub);
            Level* loadedLevel = (Level*)stub->mAsset;

            OpenLevel(loadedLevel);
        }
        else
        {
            LogError("Failed to fetch Level from AssetManager");
        }
    }
}

void ActionManager::OpenLevel(Level* level)
{
    ClearWorld();

    ResetUndoRedo();

    if (level != nullptr)
    {
        level->LoadIntoWorld(GetWorld());
    }

    SetActiveLevel(level);
}

void ActionManager::SaveLevel(bool saveAs)
{
    if (GetEngineState()->mProjectPath == "")
        return;

    if (saveAs || GetActiveLevel() == nullptr)
    {
        std::string savePath = SYS_SaveFileDialog();

        // Display the Open dialog box. 
        if (savePath != "")
        {
            std::replace(savePath.begin(), savePath.end(), '\\', '/');
            std::string newLevelName = Asset::GetNameFromPath(savePath);
            AssetStub* stub = AssetManager::Get()->CreateAndRegisterAsset(Level::GetStaticType(), nullptr, newLevelName, false);

            if (stub != nullptr)
            {
                Level* newLevel = (Level*)stub->mAsset;
                newLevel->SetName(newLevelName);
                SetActiveLevel(newLevel);
            }
        }
    }
    
    if (GetActiveLevel() != nullptr)
    {
        GetActiveLevel()->CaptureWorld(GetWorld());
        AssetManager::Get()->SaveAsset(GetActiveLevel()->GetName());
    }
}

void ActionManager::SaveSelectedAsset()
{
    AssetStub* selectedStub = GetSelectedAssetStub();
    if (selectedStub != nullptr &&
        selectedStub->mAsset != nullptr)
    {
        AssetManager::Get()->SaveAsset(*selectedStub);
    }
}

void ActionManager::DeleteSelectedActors()
{
    std::vector<Actor*> actors = GetSelectedActors();

    for (uint32_t i = 0; i < actors.size(); ++i)
    {
        if (actors[i] == nullptr ||
            actors[i] == GetWorld()->GetActiveCamera()->GetOwner())
        {
            actors.erase(actors.begin() + i);
            --i;
        }
    }

    EXE_DeleteActors(actors);
    SetSelectedActor(nullptr);
}

void ActionManager::DeleteActor(Actor* actor)
{
    if (actor != nullptr &&
        actor != GetWorld()->GetActiveCamera()->GetOwner())
    {
        EXE_DeleteActor(actor);
    }
}

Asset* ActionManager::ImportAsset()
{
    Asset* retAsset = nullptr;

    if (GetEngineState()->mProjectPath != "")
    {
        std::string openPath = SYS_OpenFileDialog();

        // Display the Open dialog box. 
        if (openPath != "")
        {
            retAsset = ImportAsset(openPath);
        }
    }
    else
    {
        LogWarning("Cannot import asset. No project loaded.");
    }

    return retAsset;
}

Asset* ActionManager::ImportAsset(const std::string& path)
{
    Asset* retAsset = nullptr;

    std::string filename = strrchr(path.c_str(), '/') + 1;
    int32_t dotIndex = int32_t(filename.find_last_of('.'));
    std::string extension = filename.substr(dotIndex, filename.size() - dotIndex);

    TypeId newType = INVALID_TYPE_ID;

    if (extension == ".png")
    {
        newType = Texture::GetStaticType();
    }
    else if (extension == ".dae" ||
        extension == ".fbx" ||
        extension == ".glb")
    {
        newType = CheckDaeAssetType(path.c_str());
    }
    else if (extension == ".wav")
    {
        newType = SoundWave::GetStaticType();
    }
    else if (extension == ".xml")
    {
        newType = Font::GetStaticType();
    }

    if (newType != INVALID_TYPE_ID)
    {
        Asset* newAsset = Asset::CreateInstance(newType);
        newAsset->Import(path);

        AssetDir* assetDir = PanelManager::Get()->GetAssetsPanel()->GetDirectory();
        std::string assetName = filename.substr(0, dotIndex);
        filename = assetName + ".oct";

#if ASSET_REF_VECTOR
        // If this asset already exists, then we are about to delete it and replace it.
        // So let's fix up any references now or else they will be lost (replaced with nullptr).
        Asset* oldAsset = FetchAsset(assetName.c_str());
        if (oldAsset != nullptr)
        {
            AssetRef::ReplaceReferencesToAsset(oldAsset, newAsset);
        }
#endif

        // If asset already exists, overwrite it. So delete existing asset.
        bool purged = AssetManager::Get()->PurgeAsset(assetName.c_str());
        if (purged)
        {
            LogWarning("Reimporting asset");
        }

        AssetStub* stub = AssetManager::Get()->RegisterAsset(filename, newAsset->GetType(), assetDir, nullptr, false);
        stub->mAsset = newAsset;
        newAsset->SetName(stub->mName);
        AssetManager::Get()->SaveAsset(*stub);

        retAsset = newAsset;
    }
    else
    {
        LogError("Failed to import Asset. Unrecognized source asset extension.");
    }

    return retAsset;
}

static std::string GetFixedFilename(const char* name, const char* prefix)
{
    std::string nameStr = name;
    size_t extPeriod = nameStr.find_last_of('.');
    if (extPeriod != std::string::npos)
    {
        nameStr = nameStr.substr(0, extPeriod);
    }

    if (nameStr.substr(0, 2) != prefix)
    {
        nameStr = prefix + nameStr;
    }

    return nameStr;
}

static void SpawnNode(aiNode* node, const glm::mat4& parentTransform, const std::vector<StaticMesh*>& meshList)
{
    World* world = GetWorld();

    if (node != nullptr)
    {
        glm::mat4 transform = parentTransform * glm::transpose(glm::make_mat4(&node->mTransformation.a1));

        for (uint32_t i = 0; i < node->mNumMeshes; ++i)
        {
            uint32_t meshIndex = node->mMeshes[i];
            StaticMeshActor* newActor = world->SpawnActor<StaticMeshActor>();
            newActor->GetStaticMeshComponent()->SetStaticMesh(meshList[meshIndex]);
            newActor->GetRootComponent()->SetTransform(transform);
            newActor->SetName(node->mName.C_Str());
            newActor->AddTag("Scene");
        }
    }

    for (uint32_t i = 0; i < node->mNumChildren; ++i)
    {
        SpawnNode(node->mChildren[i], parentTransform, meshList);
    }
}

static glm::mat4 GetNodeTransform(aiNode* node)
{
    aiMatrix4x4 transform;

    while (node)
    {
        transform = transform * node->mTransformation;
        node = node->mParent;
    }

    glm::mat4 gMat = glm::transpose(glm::make_mat4(&transform.a1));
    return gMat;
}

void ActionManager::ImportScene(const SceneImportOptions& options)
{
    if (GetEngineState()->mProjectPath == "")
        return;

    std::string openPath = options.mFilePath;

    // Display the Open dialog box. 
    if (openPath != "")
    {
        std::string filename = openPath;
        int32_t dotIndex = int32_t(filename.find_last_of('.'));
        std::string extension = filename.substr(dotIndex, filename.size() - dotIndex);

        std::string importDir;
        size_t slashPos = openPath.find_last_of("/\\");
        if (slashPos != std::string::npos)
        {
            importDir = openPath.substr(0, slashPos + 1);
        }
        else
        {
            importDir = "./";
        }

        if (extension == ".glb" ||
            extension == ".gltf" ||
            extension == ".dae")
        {
            LogDebug("Begin scene import...");
            Assimp::Importer importer;
            const aiScene* scene = importer.ReadFile(filename, aiProcess_FlipUVs);

            if (scene == nullptr)
            {
                LogError("Failed to load scene file");
                return;
            }

            // Get the current directory in the asset panel (all assets will be saved there)
            AssetDir* dir = PanelManager::Get()->GetAssetsPanel()->GetDirectory();

            if (dir == nullptr ||
                dir->mParentDir == nullptr)
            {
                LogError("Invalid directory. Use the asset panel to navigate to a valid directory");
                return;
            }

            // Destroy all actors with a Scene tag.
            // Kinda hacky, but for now, to mark anything spawned as part of a scene,
            // I'm adding a Scene tag. This is to make reimporting scenes easier.
            const std::vector<Actor*>& actors = GetWorld()->GetActors();
            for (int32_t i = int32_t(actors.size()) - 1; i >= 0; --i)
            {
                if (actors[i]->HasTag("Scene"))
                {
                    GetWorld()->DestroyActor(i);
                }
            }

            std::vector<Texture*> textureList;
            std::vector<Material*> materialList;
            std::vector<StaticMesh*> meshList;
            std::unordered_map<std::string, Texture*> textureMap;

            uint32_t numMaterials = scene->mNumMaterials;
            for (uint32_t i = 0; i < numMaterials; ++i)
            {
                aiMaterial* aMaterial = scene->mMaterials[i];
                std::string materialFileName = GetFixedFilename(aMaterial->GetName().C_Str(), "M_");

                AssetStub* materialStub = EditorAddUniqueAsset(materialFileName.c_str(), dir, Material::GetStaticType(), true);
                Material* newMaterial = static_cast<Material*>(materialStub->mAsset);
                newMaterial->SetShadingModel(ShadingModel::Unlit);

                uint32_t numBaseTextures = aMaterial->GetTextureCount(aiTextureType_DIFFUSE);
                numBaseTextures = glm::clamp(numBaseTextures, 0u, 4u);

                for (uint32_t t = 0; t < numBaseTextures; ++t)
                {
                    aiString path;
                    aiReturn ret = aMaterial->GetTexture(aiTextureType::aiTextureType_DIFFUSE, t, &path);

                    if (ret == aiReturn_SUCCESS)
                    {
                        std::string texturePath = path.C_Str();
                        Texture* textureToAssign = nullptr;
                        LogDebug("Scene Texture: %s", texturePath.c_str());
                        
                        if (textureMap.find(texturePath) != textureMap.end())
                        {
                            // Case 1 - Texture has already been loaded by a previous material
                            textureToAssign = textureMap[texturePath];
                        }
                        else
                        {
                            // Case 2 - Texture needs to be loaded.
                            // To make texturing sharing simpler, we only import the texture if
                            //  - There is no texture registered
                            //  - There is a texture registered, and it resides in the current AssetDir

                            bool importTexture = false;

                            std::string assetName = EditorGetAssetNameFromPath(texturePath);

                            AssetStub* existingStub = AssetManager::Get()->GetAssetStub(assetName);
                            if (existingStub && existingStub->mDirectory != dir)
                            {
                                textureToAssign = LoadAsset<Texture>(assetName);
                            }

                            if (textureToAssign == nullptr)
                            {
                                Asset* importedAsset = ImportAsset(importDir + texturePath);
                                assert(importedAsset == nullptr || importedAsset->GetType() == Texture::GetStaticType());

                                if (importedAsset == nullptr || importedAsset->GetType() == Texture::GetStaticType())
                                {
                                    textureToAssign = (Texture*)importedAsset;
                                }
                            }

                            textureMap.insert({ texturePath, textureToAssign });
                        }

                        newMaterial->SetTexture(TextureSlot(TEXTURE_0 + t), textureToAssign);
                    }
                }

                AssetManager::Get()->SaveAsset(*materialStub);
                materialList.push_back(newMaterial);
            }

            // Create static mesh assets (assign corresponding material)
            uint32_t numMeshes = scene->mNumMeshes;
            for (uint32_t i = 0; i < numMeshes; ++i)
            {
                aiMesh* aMesh = scene->mMeshes[i];
                std::string meshName = aMesh->mName.C_Str();

                StaticMesh* newMesh = (StaticMesh*)Asset::CreateInstance(StaticMesh::GetStaticType());
                newMesh->Create(scene, *aMesh, 0, nullptr); // Collision meshes currently not supported for scene import?

                AssetStub* meshStub = EditorAddUniqueAsset(meshName.c_str(), dir, StaticMesh::GetStaticType(), false);
                meshStub->mAsset = newMesh;
                newMesh->SetName(meshName);

                // Find material to use...
                uint32_t materialIndex = aMesh->mMaterialIndex;
                assert(materialIndex < materialList.size());
                newMesh->SetMaterial(materialList[materialIndex]);

                AssetManager::Get()->SaveAsset(*meshStub);
                meshList.push_back(newMesh);
            }

            // Create Lights
            uint32_t numLights = scene->mNumLights;
            for (uint32_t i = 0; i < numLights; ++i)
            {
                aiLight* aLight = scene->mLights[i];

                if (aLight->mType == aiLightSource_POINT)
                {
                    Actor* lightActor = GetWorld()->SpawnActor<Actor>();
                    PointLightComponent* pointLight = lightActor->CreateComponent<PointLightComponent>();

                    glm::vec3 lightColor;
                    lightColor.r = aLight->mColorDiffuse.r;
                    lightColor.g = aLight->mColorDiffuse.g;
                    lightColor.b = aLight->mColorDiffuse.b;
                    lightColor = Maths::SafeNormalize(lightColor);
                    pointLight->SetColor(glm::vec4(lightColor, 1.0f));

                    // For now, set lights to a default radius to 50.0f.
                    // Not sure how to convert attenutation data into a radius.
                    // Maybe I need to rethink how light attenuation is specified.
                    // Using the constant/linear/quadratic coefficients is how GCN and 3DS do it IIRC.
                    pointLight->SetRadius(50.0f);

                    glm::mat4 lightTransform(1);
                    aiNode* lightNode = scene->mRootNode->FindNode(aLight->mName.C_Str());

                    if (lightNode)
                    {
                        lightTransform = GetNodeTransform(lightNode);
                    }

                    pointLight->SetTransform(lightTransform);
                    lightActor->UpdateComponentTransforms();

                    lightActor->SetName(aLight->mName.C_Str());
                    lightActor->AddTag("Scene");
                }
            }

            aiNode* node = scene->mRootNode;
            SpawnNode(node, glm::mat4(1), meshList);
        }
        else
        {
            LogError("Failed to import scene. File format must be .glb or .gltf");
        }
    }
}

void ActionManager::GenerateEmbeddedAssetFiles(std::vector<std::pair<AssetStub*, std::string> >& assets,
    const char* headerPath,
    const char* sourcePath)
{
    FILE* headerFile = fopen(headerPath, "w");
    FILE* sourceFile = fopen(sourcePath, "w");

    assert(headerFile != nullptr && sourceFile != nullptr);

    if (headerFile != nullptr && sourceFile != nullptr)
    {
        fprintf(headerFile, "#include <stdint.h>\n");
        fprintf(headerFile, "#include \"EmbeddedFile.h\"\n\n");

        fprintf(headerFile, "extern uint32_t gNumEmbeddedAssets;\n");
        fprintf(headerFile, "extern EmbeddedFile gEmbeddedAssets[];\n\n");

        fprintf(sourceFile, "#include <stdint.h>\n");
        fprintf(sourceFile, "#include \"EmbeddedFile.h\"\n\n");

        std::string initializer;

        for (int32_t i = 0; i < int32_t(assets.size()); ++i)
        {
            AssetStub* stub = assets[i].first;
            const std::string& packPath = assets[i].second;

            Stream stream;
            
            // Handle special case for level
            stream.ReadFile(packPath.c_str());
            uint32_t size = uint32_t(stream.GetSize());
            char* data = stream.GetData();

            std::string sourceString;
            sourceString.reserve(2048);

            std::string assetDataVar = stub->mName + "_Data";
            sourceString += "const char ";
            sourceString += assetDataVar;
            sourceString += "[] = \n{\n";

            for (uint32_t byte = 0; byte < size; ++byte)
            {
                char byteString[8] = {};
                sprintf(byteString, "'\\x%02X',", uint8_t(data[byte]));

                sourceString += byteString;

                if (byte % 8 == 7)
                {
                    sourceString += "\n";
                }
            }

            sourceString += "\n};\n\n";

            fprintf(sourceFile, "%s", sourceString.c_str());

            initializer += "{" + ("\"" + stub->mName + "\",") +
                                 (assetDataVar + ",") +
                                 (std::to_string(size) + ",") +
                                 (stub->mEngineAsset ? "true" : "false") +
                                 "}, \n";
        }

        fprintf(sourceFile, "\n\nuint32_t gNumEmbeddedAssets = %d;\n", uint32_t(assets.size()));

        if (assets.size() > 0)
        {
            fprintf(sourceFile, "\n\nEmbeddedFile gEmbeddedAssets[] = \n{\n");
            fprintf(sourceFile, "%s", initializer.c_str());
            fprintf(sourceFile, "\n};\n");
        }
        else
        {
            fprintf(sourceFile, "\n\nEmbeddedFile gEmbeddedAssets[] = { {} };\n");
        }

        fclose(headerFile);
        headerFile = nullptr;
        fclose(sourceFile);
        sourceFile = nullptr;
    }
}

void ActionManager::GenerateEmbeddedScriptFiles(
    std::vector<std::string> files,
    const char* headerPath,
    const char* sourcePath)
{
    FILE* headerFile = fopen(headerPath, "w");
    FILE* sourceFile = fopen(sourcePath, "w");

    assert(headerFile != nullptr && sourceFile != nullptr);

    if (headerFile != nullptr && sourceFile != nullptr)
    {
        fprintf(headerFile, "#include <stdint.h>\n");
        fprintf(headerFile, "#include \"EmbeddedFile.h\"\n\n");

        fprintf(headerFile, "extern uint32_t gNumEmbeddedScripts;\n");
        fprintf(headerFile, "extern EmbeddedFile gEmbeddedScripts[];\n\n");

        fprintf(sourceFile, "#include <stdint.h>\n");
        fprintf(sourceFile, "#include \"EmbeddedFile.h\"\n\n");

        std::string initializer;

        for (int32_t i = 0; i < int32_t(files.size()); ++i)
        {
            std::string luaFile = files[i];
            std::string luaClass = ScriptComponent::GetClassNameFromFileName(luaFile);

            Stream stream;

            // Handle special case for level
            stream.ReadFile(luaFile.c_str());
            uint32_t size = uint32_t(stream.GetSize());
            char* data = stream.GetData();

            std::string sourceString;
            sourceString.reserve(2048);

            std::string fileDataVar = luaClass + "_Data";
            sourceString += "const char ";
            sourceString += fileDataVar;
            sourceString += "[] = \n{\n";

            for (uint32_t byte = 0; byte < size; ++byte)
            {
                char byteString[8] = {};
                sprintf(byteString, "'\\x%02X',", uint8_t(data[byte]));

                sourceString += byteString;

                if (byte % 8 == 7)
                {
                    sourceString += "\n";
                }
            }

            sourceString += "\n};\n\n";

            fprintf(sourceFile, "%s", sourceString.c_str());

            initializer += "{" + ("\"" + luaClass + "\",") +
                (fileDataVar + ",") +
                (std::to_string(size) + ",") +
                (/*engineFile ? "true" : */"false") +
                "}, \n";
        }

        fprintf(sourceFile, "\n\nuint32_t gNumEmbeddedScripts = %d;\n", uint32_t(files.size()));

        if (files.size() > 0)
        {
            fprintf(sourceFile, "\n\nEmbeddedFile gEmbeddedScripts[] = \n{\n");
            fprintf(sourceFile, "%s", initializer.c_str());
            fprintf(sourceFile, "\n};\n");
        }
        else
        {
            fprintf(sourceFile, "\n\nEmbeddedFile gEmbeddedScripts[] = { {} };\n");
        }

        fclose(headerFile);
        headerFile = nullptr;
        fclose(sourceFile);
        sourceFile = nullptr;
    }
}

void ActionManager::GatherScriptFiles(const std::string& dir, std::vector<std::string>& outFiles)
{
    // Recursively iterate through the Script directory and find .lua files.
    std::function<void(std::string)> searchDirectory = [&](std::string dirPath)
    {
        std::vector<std::string> subDirectories;
        DirEntry dirEntry = { };

        SYS_OpenDirectory(dirPath, dirEntry);

        while (dirEntry.mValid)
        {
            if (dirEntry.mDirectory)
            {
                // Ignore this directory and parent directory.
                if (dirEntry.mFilename[0] != '.')
                {
                    subDirectories.push_back(dirEntry.mFilename);
                }
            }
            else
            {
                const char* extension = strrchr(dirEntry.mFilename, '.');

                if (extension != nullptr &&
                    strcmp(extension, ".lua") == 0)
                {
                    std::string path = dirPath + dirEntry.mFilename;
                    outFiles.push_back(path);
                }
            }

            SYS_IterateDirectory(dirEntry);
        }

        SYS_CloseDirectory(dirEntry);

        // Discover files of subdirectories.
        for (uint32_t i = 0; i < subDirectories.size(); ++i)
        {
            std::string subDirPath = dirPath + subDirectories[i] + "/";
            searchDirectory(subDirPath);
        }
    };

    searchDirectory(dir);
}

void ActionManager::ClearWorld()
{
    SetSelectedComponent(nullptr);
    SetActiveLevel(nullptr);
    GetWorld()->DestroyAllActors();

    ResetUndoRedo();
}

void ActionManager::DeleteAllActors()
{
    if (!IsPlayingInEditor())
    {
        SetSelectedComponent(nullptr);

        const std::vector<Actor*>& worldActors = GetWorld()->GetActors();
        std::vector<Actor*> deleteActors;

        Actor* cameraActor = GetWorld()->GetActiveCamera() ? GetWorld()->GetActiveCamera()->GetOwner() : nullptr;

        for (uint32_t i = 0; i < worldActors.size(); ++i)
        {
            if (cameraActor == nullptr ||
                worldActors[i] != cameraActor)
            {
                deleteActors.push_back(worldActors[i]);
            }
        }

        EXE_DeleteActors(deleteActors);
    }

}

void ActionManager::RecaptureAndSaveAllLevels()
{
    std::unordered_map<std::string, AssetStub*>& assetMap = AssetManager::Get()->GetAssetMap();

    // This will load all assets! 
    for (auto& pair : assetMap)
    {
        if (pair.second->mType == Level::GetStaticType())
        {
            Asset* asset = AssetManager::Get()->LoadAsset(*pair.second);
            Level* level = static_cast<Level*>(asset);
            assert(level != nullptr);

            OpenLevel(level);

            if (GetActiveLevel() == level &&
                level != nullptr)
            {
                level->CaptureWorld(GetWorld());
                AssetManager::Get()->SaveAsset(*pair.second);
            }
        }
    }

    ClearWorld();
}

void ActionManager::RecaptureAndSaveAllBlueprints()
{
    std::unordered_map<std::string, AssetStub*>& assetMap = AssetManager::Get()->GetAssetMap();

    // This will load all assets! 
    for (auto& pair : assetMap)
    {
        if (pair.second->mType == Blueprint::GetStaticType())
        {
            Asset* asset = AssetManager::Get()->LoadAsset(*pair.second);
            Blueprint* bp = static_cast<Blueprint*>(asset);
            assert(bp != nullptr);

            Actor* spawnedBp = bp->Instantiate(GetWorld());

            if (spawnedBp != nullptr)
            {
                spawnedBp->UpdateComponentTransforms();
                bp->Create(spawnedBp);
                AssetManager::Get()->SaveAsset(*pair.second);
            }

            GetWorld()->DestroyActor(spawnedBp);
        }
    }
}

void ActionManager::ResaveAllAssets()
{
    std::unordered_map<std::string, AssetStub*>& assetMap = AssetManager::Get()->GetAssetMap();

    // This action is really meant for doing project-wide data updates when adding new serialized data to an asset.
    // It is important that assets are only loaded once if the LoadStream() / WriteStream() funcs are different
    for (auto& pair : assetMap)
    {
        Asset* asset = AssetManager::Get()->LoadAsset(*pair.second);
        AssetManager::Get()->SaveAsset(*pair.second);
    }

    // Refsweep afterwards to 
    AssetManager::Get()->RefSweep();
}

void ActionManager::DeleteAsset(AssetStub* stub)
{
    if (stub != nullptr)
    {
        std::string path = stub->mPath;
        AssetManager::Get()->PurgeAsset(stub->mName.c_str());
        SYS_RemoveFile(path.c_str());
    }
    else
    {
        LogWarning("Can't delete null asset stub.");
    }
}

void ActionManager::DeleteAssetDir(AssetDir* dir)
{
    if (dir != nullptr &&
        dir->mParentDir != nullptr &&
        dir->mParentDir != AssetManager::Get()->GetRootDirectory())
    {
        std::string path = dir->mPath;
        dir->mParentDir->DeleteSubdirectory(dir->mName.c_str());
        SYS_RemoveDirectory(path.c_str());
    }
    else
    {
        LogWarning("Can't delete null asset dir.");
    }
}

void ActionManager::DuplicateActor(Actor* actor)
{
    Actor* newActor = GetWorld()->CloneActor(actor);
    EXE_SpawnActor(newActor);
    SetSelectedActor(newActor);
}

// ---------------------------
// --------- ACTIONS ---------
// ---------------------------

ActionEditProperty::ActionEditProperty(
    void* owner,
    PropertyOwnerType ownerType,
    const std::string& propName,
    uint32_t index,
    Datum value)
{
    mOwner = owner;
    mOwnerType = ownerType;
    mPropertyName = propName;
    mIndex = index;
    mValue = value;

    // Keep a reference
    if (mOwnerType == PropertyOwnerType::Asset)
    {
        mReferencedAsset = (Asset*)mOwner;
    }
}

void ActionEditProperty::GatherProps(std::vector<Property>& props)
{
    if (mOwnerType == PropertyOwnerType::Component)
    {
        Component* comp = (Component*)mOwner;
        comp->GatherProperties(props);
    }
    else if (mOwnerType == PropertyOwnerType::Actor)
    {
        Actor* actor = (Actor*)mOwner;
        actor->GatherProperties(props);
    }
    else if (mOwnerType == PropertyOwnerType::Asset)
    {
        Asset* asset = (Asset*)mReferencedAsset.Get<Asset>();
        if (asset)
        {
            asset->GatherProperties(props);
        }
    }
}

Property* ActionEditProperty::FindProp(std::vector<Property>& props, const std::string& name)
{
    Property* prop = nullptr;

    for (uint32_t i = 0; i < props.size(); ++i)
    {
        if (props[i].mName == name)
        {
            prop = &props[i];
            break;
        }
    }

    return prop;
}

void ActionEditProperty::Execute()
{
    std::vector<Property> sProps;
    GatherProps(sProps);

    Property* prop = FindProp(sProps, mPropertyName);

    if (prop != nullptr)
    {
        mPreviousValue.Destroy();
        mPreviousValue.SetType(prop->GetType());
        mPreviousValue.SetCount(1);
        mPreviousValue.SetValue(prop->GetValue(mIndex));

        prop->SetValue(mValue.mData.vp, mIndex, 1);
    }
}

void ActionEditProperty::Reverse()
{
    std::vector<Property> sProps;
    GatherProps(sProps);

    Property* prop = FindProp(sProps, mPropertyName);

    if (prop != nullptr)
    {
        prop->SetValue(mPreviousValue.GetValue(0), mIndex, 1);
    }
}

ActionEditTransforms::ActionEditTransforms(
    const std::vector<TransformComponent*>& transComps,
    const std::vector<glm::mat4>& newTransforms)
{
    mTransComps = transComps;
    mNewTransforms = newTransforms;

    assert(mTransComps.size() == mNewTransforms.size());
}

void ActionEditTransforms::Execute()
{
    mPrevTransforms.clear();

    for (uint32_t i = 0; i < mTransComps.size(); ++i)
    {
        mPrevTransforms.push_back(mTransComps[i]->GetTransform());
        mTransComps[i]->SetTransform(mNewTransforms[i]);
    }
}

void ActionEditTransforms::Reverse()
{
    assert(mPrevTransforms.size() == mTransComps.size());

    for (uint32_t i = 0; i < mTransComps.size(); ++i)
    {
        mTransComps[i]->SetTransform(mPrevTransforms[i]);
    }
}

ActionSpawnActors::ActionSpawnActors(const std::vector<Actor*>& actors)
{
    mActors = actors;
}

void ActionSpawnActors::Execute()
{
    // Actor is already spawned at this point.
    for (uint32_t i = 0; i < mActors.size(); ++i)
    {
        if (mActors[i]->GetWorld() == nullptr)
        {
            ActionManager::Get()->RestoreExiledActor(mActors[i]);
        }
    }
}

void ActionSpawnActors::Reverse()
{
    for (uint32_t i = 0; i < mActors.size(); ++i)
    {
        ActionManager::Get()->ExileActor(mActors[i]);
    }
}

ActionDeleteActors::ActionDeleteActors(const std::vector<Actor*>& actors)
{
    mActors = actors;
}

void ActionDeleteActors::Execute()
{
    for (uint32_t i = 0; i < mActors.size(); ++i)
    {
        // Actor is already spawned at this point.
        assert(mActors[i]->GetWorld() != nullptr);

        if (IsPlayingInEditor())
        {
            mActors[i]->SetPendingDestroy(true);
        }
        else
        {
            ActionManager::Get()->ExileActor(mActors[i]);
        }
    }
}

void ActionDeleteActors::Reverse()
{
    for (uint32_t i = 0; i < mActors.size(); ++i)
    {
        ActionManager::Get()->RestoreExiledActor(mActors[i]);
    }
}

ActionAddComponent::ActionAddComponent(Component* comp)
{
    mComponent = comp;
    mOwner = comp->GetOwner();
    mParent = comp->IsTransformComponent() ? ((TransformComponent*)comp)->GetParent() : nullptr;
    assert(mComponent);
    assert(mOwner);
}

void ActionAddComponent::Execute()
{
    if (mComponent->GetOwner() == nullptr)
    {
        ActionManager::Get()->RestoreExiledComponent(mComponent);
        mOwner->AddComponent(mComponent);

        if (mParent != nullptr)
        {
            assert(mComponent->IsTransformComponent());
            ((TransformComponent*)mComponent)->Attach(mParent);
        }

        PanelManager::Get()->GetHierarchyPanel()->RefreshCompButtons();
    }
}

void ActionAddComponent::Reverse()
{
    assert(mComponent->GetOwner());
    mOwner->RemoveComponent(mComponent);
    ActionManager::Get()->ExileComponent(mComponent);
}

ActionRemoveComponent::ActionRemoveComponent(Component* comp)
{
    mComponent = comp;
    mOwner = comp->GetOwner();
    mParent = comp->IsTransformComponent() ? ((TransformComponent*)comp)->GetParent() : nullptr;
    assert(mComponent);
    assert(mOwner);
}

void ActionRemoveComponent::Execute()
{
    assert(mComponent->GetOwner());

    if (IsPlayingInEditor())
    {
        mOwner->DestroyComponent(mComponent);
    }
    else
    {
        mOwner->RemoveComponent(mComponent);
        ActionManager::Get()->ExileComponent(mComponent);
    }

    PanelManager::Get()->GetHierarchyPanel()->RefreshCompButtons();
}

void ActionRemoveComponent::Reverse()
{
    assert(mComponent->GetOwner() == nullptr);
    ActionManager::Get()->RestoreExiledComponent(mComponent);
    mOwner->AddComponent(mComponent);

    if (mParent != nullptr)
    {
        assert(mComponent->IsTransformComponent());
        ((TransformComponent*)mComponent)->Attach(mParent);
    }

    PanelManager::Get()->GetHierarchyPanel()->RefreshCompButtons();
}

ActionAttachComponent::ActionAttachComponent(TransformComponent* comp, TransformComponent* newParent)
{
    mComponent = comp;
    mNewParent = newParent;
    mPrevParent = comp->GetParent();
    assert(mComponent);
    assert(mNewParent);
}

void ActionAttachComponent::Execute()
{
    mComponent->Attach(mNewParent);
    PanelManager::Get()->GetHierarchyPanel()->RefreshCompButtons();
}

void ActionAttachComponent::Reverse()
{
    mComponent->Attach(mPrevParent);
    PanelManager::Get()->GetHierarchyPanel()->RefreshCompButtons();
}

#endif
