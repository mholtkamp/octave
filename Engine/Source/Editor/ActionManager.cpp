#if EDITOR

#include "ActionManager.h"

#if PLATFORM_WINDOWS
#include <Windows.h>
#include <ShlObj.h>
#endif

#include <stdint.h>

#include <vector>
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

            std::string makeCmd = std::string("make -C ") + (standalone ? buildProjName : projectDir) + " -f " + makefilePath;
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

    if (spawnedActor != nullptr)
    {
        spawnedActor->SetPosition(position);
        SetSelectedActor(spawnedActor);
    }
    else
    {
        LogError("Failed to spawn basic actor: %s", name.c_str());
    }
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
        Actor* selectedActor = actors[i];

        if (selectedActor != nullptr &&
            selectedActor != GetWorld()->GetActiveCamera()->GetOwner())
        {
            DirectionalLightComponent* dirLightComp = GetWorld()->GetDirectionalLight();
            if (dirLightComp && selectedActor == dirLightComp->GetOwner())
            {
                GetWorld()->SetDirectionalLight(nullptr);
            }

            GetWorld()->DestroyActor(selectedActor);
        }
    }

    SetSelectedActor(nullptr);
}

void ActionManager::ImportAsset()
{
    if (GetEngineState()->mProjectPath == "")
        return;

    std::string openPath = SYS_OpenFileDialog();

    // Display the Open dialog box. 
    if (openPath != "")
    {
        std::string filename = strrchr(openPath.c_str(), '/') + 1;
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
            newType = CheckDaeAssetType(openPath.c_str());
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
            newAsset->Import(openPath);

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
        }
        else
        {
            LogError("Failed to import Asset. Unrecognized source asset extension.");
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

#endif