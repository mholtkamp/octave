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
#include "AssetManager.h"
#include "EditorState.h"
#include "EditorUtils.h"
#include "Assets/Scene.h"
#include "Assets/Texture.h"
#include "Assets/StaticMesh.h"
#include "Assets/SkeletalMesh.h"
#include "Assets/SoundWave.h"
#include "Assets/MaterialLite.h"
#include "Assets/Font.h"
#include "AssetDir.h"
#include "EmbeddedFile.h"
#include "Utilities.h"
#include "EditorUtils.h"
#include "EditorImgui.h"
#include "Log.h"

#include "Nodes/3D/StaticMesh3d.h"
#include "Nodes/3D/PointLight3d.h"
#include "Nodes/3D/DirectionalLight3d.h"
#include "Nodes/3D/Node3d.h"
#include "Nodes/3D/SkeletalMesh3d.h"
#include "Nodes/3D/Box3d.h"
#include "Nodes/3D/Sphere3d.h"
#include "Nodes/3D/Capsule3d.h"
#include "Nodes/3D/Particle3d.h"
#include "Nodes/3D/Audio3d.h"
#include "Nodes/3D/ShadowMesh3d.h"
#include "Nodes/3D/InstancedMesh3d.h"
#include "Nodes/3D/TextMesh3d.h"

#include "System/System.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define USE_IMGUI_FILE_BROWSER (!PLATFORM_WINDOWS)

#define SUB_SCENE_HIER_WARN_TEXT "Cannot modify sub-scene hierarchy. Must unlink scene first."

ActionManager* ActionManager::sInstance = nullptr;

static void GatherMeshImportTypes(const char* path, std::vector<TypeId>& outImportTypes, std::vector<int32_t>& meshIndices)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_FlipUVs);
    
    bool hasCollisionMeshes = false;

    for (uint32_t m = 0; m < scene->mNumMeshes; ++m)
    {
        if (IsAiCollisionMesh(scene->mMeshes[m]))
        {
            hasCollisionMeshes = true;
        }
        else
        {
            TypeId typeId = scene->mMeshes[m]->HasBones() ? SkeletalMesh::GetStaticType() : StaticMesh::GetStaticType();
            outImportTypes.push_back(typeId);
            meshIndices.push_back(int32_t(m));
        }
    }

    if (outImportTypes.size() > 1 && hasCollisionMeshes)
    {
        LogWarning("File has collision meshes but more than one non-collision mesh. Simple collision importing will be ignored.");
    }
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

void ReplaceAllStrings(std::string& str, const std::string& from, const std::string& to)
{
    if (from.empty())
        return;

    size_t startPos = 0;
    while ((startPos = str.find(from, startPos)) != std::string::npos)
    {
        str.replace(startPos, from.length(), to);
        startPos += to.length();
    }
}

void ReplaceStringInFile(const std::string& file, const std::string& srcString, const std::string& dstString)
{
    Stream fileStream;
    fileStream.ReadFile(file.c_str(), false);
    std::string fileString = std::string(fileStream.GetData(), fileStream.GetSize());
    ReplaceAllStrings(fileString, srcString, dstString);

    Stream outStream(fileString.c_str(), (uint32_t)fileString.size());
    outStream.WriteFile(file.c_str());
}

void ActionManager::BuildData(Platform platform, bool embedded)
{
    LogDebug("Begin packaging...");

    const EngineState* engineState = GetEngineState();
    bool standalone = engineState->mStandalone;
    const std::string& projectDir = engineState->mProjectDirectory;
    const std::string& projectName = engineState->mProjectName;
    bool useRomfs = (platform == Platform::N3DS) && embedded;

    std::vector<std::pair<AssetStub*, std::string> > embeddedAssets;

    if (projectDir == "")
    {
        LogError("Project directory not set?");
        return;
    }

    std::string dkpPath = GetDevkitproPath();
    bool dkpInstalled = (dkpPath != "");

    LogDebug("DevkitPro is %s.", dkpInstalled ? "installed" : "not installed");

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
            if (embedded && !useRomfs)
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
            const char* regType = Asset::GetNameFromTypeId(pair.second->mType);

            std::string regPath = pair.second->mPath.c_str();
            if (!pair.second->mEngineAsset &&
                regPath.find(projectDir) != std::string::npos)
            {
                regPath = regPath.substr(projectDir.length());
                regPath = projectName + "/" + regPath;
            }

            fprintf(registryFile, "%s,%s\n", regType, regPath.c_str());
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
    GenerateEmbeddedAssetFiles(embedded, embeddedAssets, embeddedHeaderPath.c_str(), embeddedSourcePath.c_str());

    // Generate embedded script source files. If not doing an embedded build, copy over the script folders.
    std::vector<std::string> scriptFiles;

    if (embedded && !useRomfs)
    {
        GatherScriptFiles("Engine/Scripts/", scriptFiles);
        GatherScriptFiles(projectDir + "/Scripts/", scriptFiles);
    }
    else
    {
        SYS_Exec(std::string("cp -R Engine/Scripts " + packagedDir + "Engine/Scripts").c_str());
        SYS_Exec(std::string("cp -R " + projectDir + "Scripts " + packagedDir + projectName + "/Scripts").c_str());

        if (platform != Platform::Windows && platform != Platform::Linux)
        {
            // Remove LuaPanda on consoles (it's a 148 KB file)
            SYS_Exec(std::string("rm " + packagedDir + "Engine/Scripts" + "/LuaPanda.lua").c_str());
        }
    }

    // Generate embedded script source files, even if not doing an embedded build. 
    // So we don't need to worry about whether we include code that links to the embedded script array / script count.
    std::string scriptHeaderPath = projectDir + "Generated/EmbeddedScripts.h";
    std::string scriptSourcePath = projectDir + "Generated/EmbeddedScripts.cpp";
    GenerateEmbeddedScriptFiles(embedded, scriptFiles, scriptHeaderPath.c_str(), scriptSourcePath.c_str());

    if (standalone)
    {
        std::string copyGeneratedFolder = "cp -R " + projectDir + "Generated " + "Standalone";
        SYS_Exec(copyGeneratedFolder.c_str());
    }

    // Copy .octp and Config.ini
    {
        std::string copyOctpCmd = "cp " + projectDir + projectName + ".octp " + packagedDir + projectName;
        SYS_Exec(copyOctpCmd.c_str());

        std::string copyConfigCmd = "cp " + projectDir + "Config.ini " + packagedDir;
        SYS_Exec(copyConfigCmd.c_str());
    }

    // Handle SpirV shaders on Vulkan platforms
    // Make sure to do this before copying everything to "assets/" directory in the Android build.
    if (platform == Platform::Windows ||
        platform == Platform::Linux ||
        platform == Platform::Android)
    {
        // Compile shaders
#if PLATFORM_WINDOWS
        SYS_Exec("cd Engine/Shaders/GLSL && \"./compile.bat\"");
#else
        SYS_Exec("cd Engine/Shaders/GLSL && \"./compile.sh\"");
#endif

        // Then copy over the binaries.
        CreateDir((packagedDir + "Engine/Shaders/").c_str());
        CreateDir((packagedDir + "Engine/Shaders/GLSL/").c_str());

        SYS_Exec(std::string("cp -R Engine/Shaders/GLSL/bin " + packagedDir + "Engine/Shaders/GLSL/bin").c_str());
    }

    // If we are running a 3DS build, copy all the packaged data to the
    // Intermediate/Romfs directory.
    // Clear existing Romfs directory first.
    std::string intermediateDir = standalone ? "Standalone/Intermediate" : (projectDir + "/Intermediate");
    std::string romfsDir = intermediateDir + "/Romfs";
    RemoveDir(romfsDir.c_str());
    CreateDir(intermediateDir.c_str());
    CreateDir(romfsDir.c_str());

    if (useRomfs)
    {
        LogDebug("Copying packaged data to Romfs staging directory.");
        SYS_Exec(std::string("cp -R " + packagedDir + "/* " + romfsDir).c_str());
    }

    // ( ) Run the makefile to compile the game.
    bool needCompile = true;
    std::string prebuiltExeName = (platform == Platform::Windows) ? "Octave.exe" : "Octave.elf";

    // If packaging for Windows or Linux in standalone editor, we can use the existing octave executables.
    if (standalone &&
        (platform == Platform::Windows || platform == Platform::Linux))
    {
        needCompile = !SYS_DoesFileExist(prebuiltExeName.c_str(), false);
    }

    std::string buildProjName = standalone ? "Standalone" : projectName;
    std::string buildProjDir = standalone ? "Standalone/" : projectDir;
    std::string buildDstExeName = standalone ? "Octave" : projectName;
    bool useSteam = GetEngineConfig()->mPackageForSteam;

    if (needCompile)
    {
        LogDebug("Compiling game executable.");

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

            std::string devenvCmd = "\"" + GetDevenvPath();
            if (useSteam)
            {
                devenvCmd += std::string(" ") + solutionPath + " /Build \"ReleaseSteam|x64\" /Project " + buildProjName;
            }
            else
            {
                devenvCmd += std::string(" ") + solutionPath + " /Build \"Release|x64\" /Project " + buildProjName;
            }
            devenvCmd += "\"";

            SYS_Exec(devenvCmd.c_str());
        }
        else if (platform == Platform::Android)
        {
            // Copy contents of Packaged/Android folder into Android/app/src/main/assets folder so they get put into the APK.
            std::string androidAssetsDir = buildProjDir + "Android/app/src/main/assets/";
            if (!DoesDirExist(androidAssetsDir.c_str()))
            {
                CreateDir(androidAssetsDir.c_str());
            }

            SYS_Exec(std::string("cp -R " + packagedDir + "/* " + androidAssetsDir).c_str());

            // Invoke the gradle build
            std::string gradleDir = buildProjDir + "Android/";
#if PLATFORM_WINDOWS
            std::string gradleCmd = "cd " + gradleDir + " && gradlew.bat assembleRelease";
#else
            std::string gradleCmd = "cd " + gradleDir + " && \"./gradlew assembleRelease\"";
#endif
            SYS_Exec(gradleCmd.c_str());

            // Rename the executable
            std::string srcExeName = StringToLower(buildProjName);
            srcExeName += "-release.apk";

            std::string dstExeName = buildDstExeName + ".apk";

            std::string renameCmd = std::string("mv ") + buildProjDir + "/Android/app/build/outputs/apk/release/" + srcExeName + " " + buildProjDir + "/Android/app/build/outputs/apk/release/" + dstExeName;
            SYS_Exec(renameCmd.c_str());
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
            default: OCT_ASSERT(0); break;
            }

            std::string srcMakefile = buildProjDir + "/" + makefilePath;
            std::string tmpMakefile = buildProjDir + "/Makefile_TEMP";
            // Make a copy of the makefile so we can change the app name and things like that
            SYS_Exec(std::string("cp " + srcMakefile + " " + tmpMakefile).c_str());

            // This is fragile, but we're going to modify the temp makefile to compile
            // the way we need to for a given platform.
            if (platform == Platform::N3DS)
            {
                ReplaceStringInFile(tmpMakefile, "OctaveApp", projectName);
                ReplaceStringInFile(tmpMakefile, "$(CURDIR)/Makefile_3DS", "$(CURDIR)/Makefile_TEMP");

                //if (!useRomfs)
                //{
                //    ReplaceStringInFile(tmpMakefile, "ROMFS :=", "#ROMFS :=");
                //}
            }

            std::string makeCmd = std::string("make -C ") + (buildProjDir) + " -f Makefile_TEMP -j 12";
            SYS_Exec(makeCmd.c_str());

            // Delete the temp makefile
            SYS_Exec(std::string("rm " + tmpMakefile).c_str());

            if (platform == Platform::Linux)
            {
                // Strip out debug symbols to keep the file size small.
                std::string exeName = standalone ? "Octave" : projectName;
                std::string stripCmd = std::string("strip --strip-debug ") + buildProjDir + "Build/Linux/" + exeName + ".elf";
                SYS_Exec(stripCmd.c_str());
            }
        }
    }
    else
    {
        LogDebug("Reusing pre-compiled game executable.");
        // When running the standalone editor, we don't need to compile 
        // (unless we are embeddeding assets into the executable).
        // But we do need to copy over the Octave.exe and ideally rename it to the project name to make it more official :)
    }

    // ( ) Copy the executable into the Packaged folder.
    std::string exeSrc = buildProjDir + "/Build/";

    if (platform == Platform::Android)
    {
        exeSrc = buildProjDir;
    }

    switch (platform)
    {
    case Platform::Windows:exeSrc += (useSteam ? "Windows/x64/ReleaseSteam/" : "Windows/x64/Release/"); break;
    case Platform::Linux: exeSrc += "Linux/"; break;
    case Platform::Android: exeSrc += "Android/app/build/outputs/apk/release/"; break;
    case Platform::GameCube: exeSrc += "GCN/"; break;
    case Platform::Wii: exeSrc += "Wii/"; break;
    case Platform::N3DS: exeSrc += "3DS/"; break;
    default: OCT_ASSERT(0); break;
    }

    std::string exeNameBase = standalone ? "Octave" : projectName;
    std::string exeName = exeNameBase;

    std::string extension = ".exe";

    switch (platform)
    {
    case Platform::Windows: extension = ".exe"; break;
    case Platform::Linux: extension = ".elf"; break;
    case Platform::Android: extension = ".apk"; break;
    case Platform::GameCube: extension = ".dol"; break;
    case Platform::Wii: extension = ".dol"; break;
    case Platform::N3DS: extension = ".3dsx"; break;
    default: OCT_ASSERT(0); break;
    }

    exeName += extension;
    exeSrc += exeName;

    if (!needCompile)
    {
        // Override exe path for uncompiled standalone builds
        exeSrc = prebuiltExeName;
    }

    std::string exeCopyCmd = std::string("cp ") + exeSrc + " " + packagedDir;
    SYS_Exec(exeCopyCmd.c_str());

    if (standalone)
    {
        // Rename the executable to the project name
        std::string renameCmd = std::string("mv ") + packagedDir + "Octave" + extension + " " + packagedDir + projectName + extension;
        SYS_Exec(renameCmd.c_str());
    }

    if (platform == Platform::Windows && useSteam)
    {
        // (1) Copy over redistributable DLL
        std::string cpSteamDllCmd1 = std::string("cp ") + projectDir + (standalone ? "../" : "../Octave/") + "External/Steam/redistributable_bin/win64/steam_api64.dll " + packagedDir;
        SYS_Exec(cpSteamDllCmd1.c_str());

        // (2) Create a steam_appid.txt with 480
        Stream idStream;
        const char* txtId = "480";
        idStream.WriteBytes((uint8_t*)txtId, (uint32_t)strlen(txtId));
        idStream.WriteFile((packagedDir + "steam_appid.txt").c_str());
    }

    // ISO building for GameCube and Wii, non-embedded builds
    if (!embedded && platform == Platform::GameCube)
    {
        // TODO: Check if we have the tools available for building an iso

        // (1) Run dollz3 on the .dol file (I
        std::string dollzPath = "External/ISO/dollz3";
#if PLATFORM_WINDOWS
        dollzPath += ".exe";
#endif

        std::string dollzCmd = dollzPath + " " + packagedDir + projectName + ".dol " + packagedDir + "bootldr.dol -m";
        SYS_Exec(dollzCmd.c_str());

        // (2) Run mkisofs using the provided gbi.hdr file
        std::string gbiPath = "External/ISO/gbi.hdr";
        std::string mkisofsPath = "mkisofs";
#if PLATFORM_WINDOWS
        mkisofsPath = "External/ISO/mkisofs.exe";
#endif
        std::string isoCmd = mkisofsPath + " -R -J -G " + gbiPath + " -no-emul-boot -b bootldr.dol -o "  + packagedDir + projectName + ".iso " + packagedDir;
        SYS_Exec(isoCmd.c_str());
    }

    LogDebug("Finished packaging!");
}

void ActionManager::PrepareRelease()
{
    // Make sure to change the OCTAVE_VERSION definition in Constants.h first.

    // [ ] Package the project for current platform
    std::string platformName = "";
#if PLATFORM_WINDOWS
    ActionManager::Get()->BuildData(Platform::Windows, false);
    platformName = "Windows";
#else
    ActionManager::Get()->BuildData(Platform::Linux, false);
    platformName = "Linux";

    // On Linux, make the editor too, in case we are using a cmake-build
    SYS_Exec("make -j 12 -C Standalone -f Makefile_Linux_Editor");
#endif

    // [ ] Destroy/Create directory in CWD for release
    const char* homePath = nullptr;
#if PLATFORM_WINDOWS
    homePath = getenv("USERPROFILE");
#else
    homePath = getenv("HOME");
#endif
    OCT_ASSERT(homePath != nullptr);
    LogDebug("Home path = %s", homePath);

    std::string stagingDir = std::string(homePath) + "/OctaveRelease/";
    SYS_RemoveDirectory(stagingDir.c_str());
    SYS_CreateDirectory(stagingDir.c_str());

    // [ ] Copy specific directories. See if we can use rsync on windows?
    std::string cpCmd = std::string("cp -R . ") + stagingDir;
    SYS_Exec(cpCmd.c_str());
    std::string cleanCmd = std::string("cd ") + stagingDir + " && git clean -xdf";
    SYS_Exec(cleanCmd.c_str());

    // [ ] Copy this exectubable (Release Editor) to the staging directory, rename to OctaveEditor
#if PLATFORM_WINDOWS
    std::string cpEditorCmd = std::string("cp Standalone/Build/Windows/x64/ReleaseEditor/Octave.exe ") + stagingDir + "/OctaveEditor.exe";
#else
    std::string cpEditorCmd = std::string("cp Standalone/Build/Linux/OctaveEditor.elf ") + stagingDir;// + std::string("OctaveEditor");
#endif

    SYS_Exec(cpEditorCmd.c_str());

    // [ ] Copy the packaged platform's Octave exe to the staging directory
#if PLATFORM_WINDOWS
    std::string cpGameCmd = std::string("cp Standalone/Build/Windows/x64/Release/Octave.exe ") + stagingDir;
#else
    std::string cpGameCmd = std::string("cp Standalone/Build/Linux/Octave.elf ") + stagingDir;// + std::string("Octave");
#endif

    SYS_Exec(cpGameCmd.c_str());

#if PLATFORM_LINUX
    // Strip out debug symbols to keep the file size small.
    std::string stripEditorCmd = std::string("strip --strip-debug ") + stagingDir + "OctaveEditor.elf";
    SYS_Exec(stripEditorCmd.c_str());
    std::string stripCmd = std::string("strip --strip-debug ") + stagingDir + "Octave.elf";
    SYS_Exec(stripCmd.c_str());
#endif

    // Compile shaders
#if PLATFORM_WINDOWS
    SYS_Exec("cd Engine/Shaders/GLSL && \"./compile.bat\"");
    std::string shadersCmd = "cd " + stagingDir + "/Engine/Shaders/GLSL && \"./compile.bat\"";
#else
    std::string shadersCmd = "cd " + stagingDir + "/Engine/Shaders/GLSL && \"./compile.sh\"";
#endif
    SYS_Exec(shadersCmd.c_str());

    // Remove git submodules
    std::string rmBulletSubmodule = std::string("rm -rf ") + stagingDir + "/External/bullet3";
    SYS_Exec(rmBulletSubmodule.c_str());
    std::string rmAssimpSubmodule = std::string("rm -rf ") + stagingDir + "/External/assimp";
    SYS_Exec(rmAssimpSubmodule.c_str());

    // Delete all dot files
    std::string deleteGitCmd = std::string("find ") + stagingDir + " -name '.*' -exec rm -rf \"{}\" \\;";
    SYS_Exec(deleteGitCmd.c_str());
}

void ActionManager::OnSelectedNodeChanged()
{

}

Node* ActionManager::SpawnNode(TypeId nodeType, Node* parent)
{
    if (parent && parent->IsSceneLinked())
    {
        LogWarning("Cannot add child to scene-linked node. Unlink the scene first.");
        return nullptr;
    }

    Node* spawnedNode = EXE_SpawnNode(nodeType);

    OCT_ASSERT(spawnedNode != nullptr);
    if (spawnedNode != nullptr)
    {
        parent = parent ? parent : GetWorld(0)->GetRootNode();
        if (parent != nullptr)
        {
            parent->AddChild(spawnedNode);
        }
        else
        {
            GetWorld(0)->SetRootNode(spawnedNode);
        }

        GetEditorState()->SetSelectedNode(spawnedNode);
    }

    return spawnedNode;
}

Node* ActionManager::SpawnNode(TypeId nodeType, glm::vec3 position)
{
    Node* node = SpawnNode(nodeType, nullptr);
    Node3D* node3d = node ? node->As<Node3D>() : nullptr;

    if (node3d)
    {
        node3d->SetWorldPosition(position);
    }

    return node;
}

Node* ActionManager::SpawnBasicNode(const std::string& name, Node* parent, Asset* srcAsset, bool setWorldPos, glm::vec3 worldPos)
{
    Node* spawnedNode = nullptr;

    if (parent && parent->IsSceneLinked())
    {
        LogWarning("Cannot add child to scene-linked node. Unlink the scene first.");
        return nullptr;
    }

    if (srcAsset == nullptr)
    {
        srcAsset = GetEditorState()->GetSelectedAsset();
    }

    if (name == BASIC_STATIC_MESH)
    {
        StaticMesh3D* meshNode = EXE_SpawnNode(StaticMesh3D::GetStaticType())->As<StaticMesh3D>();

        StaticMesh* mesh = (StaticMesh*) LoadAsset("SM_Cube");

        if (srcAsset != nullptr &&
            srcAsset->GetType() == StaticMesh::GetStaticType())
        {
            mesh = static_cast<StaticMesh*>(srcAsset);
            meshNode->SetName(mesh->GetName());
        }

        // When spawned by the editor, static meshes have collision enabled on colgroup1
        meshNode->SetStaticMesh(mesh);
        meshNode->EnableOverlaps(false);
        meshNode->EnableCollision(true);
        meshNode->EnablePhysics(false);
        meshNode->SetCollisionGroup(ColGroup1);
        meshNode->SetCollisionMask(~ColGroup1);
        meshNode->SetBakeLighting(true);

        spawnedNode = meshNode;
    }
    else if (name == BASIC_POINT_LIGHT)
    {
        // Spawn point light actor
        PointLight3D* pointLight = EXE_SpawnNode(PointLight3D::GetStaticType())->As<PointLight3D>();

        pointLight->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        pointLight->SetRadius(10.0f);
        pointLight->SetLightingDomain(LightingDomain::All);
        
        spawnedNode = pointLight;
    }
    else if (name == BASIC_NODE_3D)
    {
        spawnedNode = EXE_SpawnNode(Node3D::GetStaticType())->As<Node3D>();
    }
    else if (name == BASIC_DIRECTIONAL_LIGHT)
    {
        DirectionalLight3D* dirLight = EXE_SpawnNode(DirectionalLight3D::GetStaticType())->As<DirectionalLight3D>();
        dirLight->SetLightingDomain(LightingDomain::All);

        spawnedNode = dirLight;
    }
    else if (name == BASIC_SKELETAL_MESH)
    {
        SkeletalMesh3D* skNode = EXE_SpawnNode(SkeletalMesh3D::GetStaticType())->As<SkeletalMesh3D>();

        SkeletalMesh* mesh = LoadAsset<SkeletalMesh>("SK_EighthNote");

        if (srcAsset != nullptr &&
            srcAsset->GetType() == SkeletalMesh::GetStaticType())
        {
            mesh = static_cast<SkeletalMesh*>(srcAsset);

            // Spawn skeletal mesh actor.
            skNode->SetSkeletalMesh(mesh);
        }

        spawnedNode = skNode;
    }
    else if (name == BASIC_BOX)
    {
        spawnedNode = EXE_SpawnNode(Box3D::GetStaticType())->As<Box3D>();
    }
    else if (name == BASIC_SPHERE)
    {
        spawnedNode = EXE_SpawnNode(Sphere3D::GetStaticType())->As<Sphere3D>();
    }
    else if (name == BASIC_CAPSULE)
    {
        spawnedNode = EXE_SpawnNode(Capsule3D::GetStaticType())->As<Capsule3D>();
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
        Particle3D* particleNode = EXE_SpawnNode(Particle3D::GetStaticType())->As<Particle3D>();
        particleNode->SetParticleSystem(particleSystem);

        spawnedNode = particleNode;
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

        // Spawn an Audio actor
        Audio3D* audioNode = EXE_SpawnNode(Audio3D::GetStaticType())->As<Audio3D>();
        audioNode->SetSoundWave(soundWave);
        audioNode->SetLoop(true);
        audioNode->SetAutoPlay(true);

        spawnedNode = audioNode;
    }
    else if (name == BASIC_SCENE)
    {
        Scene* scene = nullptr;

        if (srcAsset != nullptr &&
            srcAsset->GetType() == Scene::GetStaticType())
        {
            scene = static_cast<Scene*>(srcAsset);
        }

        if (scene != nullptr)
        {
            spawnedNode = EXE_SpawnNode(scene);
        }
    }
    else if (name == BASIC_CAMERA)
    {
        spawnedNode = EXE_SpawnNode(Camera3D::GetStaticType())->As<Camera3D>();
    }
    else if (name == BASIC_TEXT_MESH)
    {
        spawnedNode = EXE_SpawnNode(TextMesh3D::GetStaticType())->As<TextMesh3D>();
    }
    else if (name == BASIC_INSTANCED_MESH)
    {
        InstancedMesh3D* instMeshNode = EXE_SpawnNode(InstancedMesh3D::GetStaticType())->As<InstancedMesh3D>();
        spawnedNode = instMeshNode;

        StaticMesh* mesh = (StaticMesh*)LoadAsset("SM_Cube");

        if (srcAsset != nullptr &&
            srcAsset->GetType() == StaticMesh::GetStaticType())
        {
            mesh = static_cast<StaticMesh*>(srcAsset);
            instMeshNode->SetName(mesh->GetName());
        }

        instMeshNode->SetStaticMesh(mesh);
        instMeshNode->EnableOverlaps(false);
        instMeshNode->EnableCollision(false);
        instMeshNode->EnablePhysics(false);
        instMeshNode->SetCollisionGroup(ColGroup1);
        instMeshNode->SetCollisionMask(~ColGroup1);
        instMeshNode->AddInstanceData(MeshInstanceData());
    }

    if (spawnedNode != nullptr)
    {
        parent = parent ? parent : GetWorld(0)->GetRootNode();
        if (parent != nullptr)
        {
            parent->AddChild(spawnedNode);
        }
        else
        {
            GetWorld(0)->SetRootNode(spawnedNode);
        }

        GetEditorState()->SetSelectedNode(spawnedNode);

        Node3D* node3d = spawnedNode ? spawnedNode->As<Node3D>() : nullptr;

        if (setWorldPos && node3d)
        {
            node3d->SetWorldPosition(worldPos);
        }
    }
    else
    {
        LogError("Failed to spawn basic actor: %s", name.c_str());
    }

    return spawnedNode;
}

void ActionManager::ExecuteAction(Action* action)
{
    OCT_ASSERT(std::find(mActionHistory.begin(), mActionHistory.end(), action) == mActionHistory.end());
    OCT_ASSERT(std::find(mActionFuture.begin(), mActionFuture.end(), action) == mActionFuture.end());

    action->Execute();

    // Limit max number of history?
    const uint32_t MaxActionHistoryCount = 100;
    if (mActionHistory.size() >= MaxActionHistoryCount)
    {
        mActionHistory.erase(mActionHistory.begin());
    }

    mActionHistory.push_back(action);
    ClearActionFuture();
}

void ActionManager::Undo()
{
    if (!IsPlaying() &&
        mActionHistory.size() > 0 &&
        !ImGui::GetIO().WantTextInput)
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
    if (!IsPlaying() &&
        mActionFuture.size() > 0 &&
        !ImGui::GetIO().WantTextInput)
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

void ActionManager::EXE_EditTransform(Node3D* transComp, const glm::mat4& transform)
{
    std::vector<Node3D*> transComps;
    transComps.push_back(transComp);

    std::vector<glm::mat4> transforms;
    transforms.push_back(transform);

    ActionEditTransforms* action = new ActionEditTransforms(transComps, transforms);
    ActionManager::Get()->ExecuteAction(action);
}

void ActionManager::EXE_EditTransforms(const std::vector<Node3D*>& transComps, const std::vector<glm::mat4>& newTransforms)
{
    ActionEditTransforms* action = new ActionEditTransforms(transComps, newTransforms);
    ActionManager::Get()->ExecuteAction(action);
}

Node* ActionManager::EXE_SpawnNode(TypeId srcType)
{
    std::vector<TypeId> srcTypes;
    srcTypes.push_back(srcType);

    GetEditorState()->EnsureActiveScene();

    ActionSpawnNodes* action = new ActionSpawnNodes(srcTypes);
    ActionManager::Get()->ExecuteAction(action);

    OCT_ASSERT(action->GetNodes().size() == 1);
    Node* retNode = action->GetNodes()[0].Get();
    return retNode;
}

Node* ActionManager::EXE_SpawnNode(const char* srcTypeName)
{
    std::vector<const char*> srcTypeNames;
    srcTypeNames.push_back(srcTypeName);

    GetEditorState()->EnsureActiveScene();

    ActionSpawnNodes* action = new ActionSpawnNodes(srcTypeNames);
    ActionManager::Get()->ExecuteAction(action);

    OCT_ASSERT(action->GetNodes().size() == 1);
    Node* retNode = action->GetNodes()[0].Get();
    return retNode;
}

Node* ActionManager::EXE_SpawnNode(Scene* srcScene)
{
    std::vector<SceneRef> srcScenes;
    srcScenes.push_back(srcScene);

    GetEditorState()->EnsureActiveScene();

    ActionSpawnNodes* action = new ActionSpawnNodes(srcScenes);
    ActionManager::Get()->ExecuteAction(action);

    OCT_ASSERT(action->GetNodes().size() == 1);
    Node* retNode = action->GetNodes()[0].Get();
    return retNode;
}

Node* ActionManager::EXE_SpawnNode(Node* srcNode)
{
    if (srcNode->IsSceneLinkedChild())
    {
        LogError(SUB_SCENE_HIER_WARN_TEXT);
        return nullptr;
    }

    std::vector<Node*> srcNodes;
    srcNodes.push_back(srcNode);

    GetEditorState()->EnsureActiveScene();

    ActionSpawnNodes* action = new ActionSpawnNodes(srcNodes);
    ActionManager::Get()->ExecuteAction(action);

    OCT_ASSERT(action->GetNodes().size() == 1);
    Node* retNode = action->GetNodes()[0].Get();
    return retNode;
}

void ActionManager::EXE_DeleteNode(Node* node)
{
    if (node->IsSceneLinkedChild())
    {
        LogError(SUB_SCENE_HIER_WARN_TEXT);
        return;
    }

    std::vector<Node*> nodes;
    nodes.push_back(node);

    ActionDeleteNodes* action = new ActionDeleteNodes(nodes);
    ActionManager::Get()->ExecuteAction(action);
}

std::vector<Node*> ActionManager::EXE_SpawnNodes(const std::vector<Node*>& srcNodes)
{
    OCT_ASSERT(srcNodes.size() > 0);
    std::vector<Node*> retNodes;

    std::vector<Node*> trimmedSrcNodes = srcNodes;
    RemoveRedundantDescendants(trimmedSrcNodes);

    for (auto node : trimmedSrcNodes)
    {
        if (node->IsSceneLinkedChild())
        {
            LogError(SUB_SCENE_HIER_WARN_TEXT);
            return retNodes;
        }
    }

    GetEditorState()->EnsureActiveScene();

    ActionSpawnNodes* action = new ActionSpawnNodes(trimmedSrcNodes);
    ActionManager::Get()->ExecuteAction(action);

    OCT_ASSERT(action->GetNodes().size() > 0);

    retNodes.resize(action->GetNodes().size());

    for (uint32_t i = 0; i < retNodes.size(); ++i)
    {
        retNodes[i] = action->GetNodes()[i].Get();
    }

    return retNodes;
}

void ActionManager::EXE_DeleteNodes(const std::vector<Node*>& nodes)
{
    std::vector<Node*> trimmedNodes = nodes;
    RemoveRedundantDescendants(trimmedNodes);

    for (auto node : trimmedNodes)
    {
        if (node == nullptr)
        {
            LogError("Delete Node: Invalid node(s)");
            return;
        }

        if (node->IsSceneLinkedChild())
        {
            LogError(SUB_SCENE_HIER_WARN_TEXT);
            return;
        }
    }

    ActionDeleteNodes* action = new ActionDeleteNodes(trimmedNodes);
    ActionManager::Get()->ExecuteAction(action);
}

void ActionManager::EXE_AttachNode(Node* node, Node* newParent, int32_t childIndex, int32_t boneIndex)
{
    if (node->IsSceneLinkedChild() || newParent->IsSceneLinkedChild())
    {
        LogError(SUB_SCENE_HIER_WARN_TEXT);
        return;
    }

    ActionAttachNode* action = new ActionAttachNode(node, newParent, childIndex, boneIndex);
    ActionManager::Get()->ExecuteAction(action);
}

void ActionManager::EXE_SetRootNode(Node* newRoot)
{
    if (newRoot->IsSceneLinkedChild())
    {
        LogError(SUB_SCENE_HIER_WARN_TEXT);
        return;
    }

    ActionSetRootNode* action = new ActionSetRootNode(newRoot);
    ActionManager::Get()->ExecuteAction(action);
}

void ActionManager::EXE_SetWorldRotation(Node3D* comp, glm::quat rot)
{
    ActionSetWorldRotation* action = new ActionSetWorldRotation(comp, rot);
    ActionManager::Get()->ExecuteAction(action);
}

void ActionManager::EXE_SetWorldPosition(Node3D* comp, glm::vec3 pos)
{
    ActionSetWorldPosition* action = new ActionSetWorldPosition(comp, pos);
    ActionManager::Get()->ExecuteAction(action);
}

void ActionManager::EXE_SetWorldScale(Node3D* comp, glm::vec3 scale)
{
    ActionSetWorldScale* action = new ActionSetWorldScale(comp, scale);
    ActionManager::Get()->ExecuteAction(action);
}

void ActionManager::EXE_UnlinkScene(Node* node)
{
    if (node->IsSceneLinked())
    {
        ActionUnlinkScene* action = new ActionUnlinkScene(node);
        ActionManager::Get()->ExecuteAction(action);
    }
}

void ActionManager::EXE_SetInstanceColors(const std::vector<ActionSetInstanceColorsData>& data)
{
    ActionSetInstanceColors* action = new ActionSetInstanceColors(data);
    ActionManager::Get()->ExecuteAction(action);
}

void ActionManager::EXE_SetInstanceData(InstancedMesh3D* instMesh, int32_t startIndex, const std::vector<MeshInstanceData>& data)
{
    ActionSetInstanceData* action = new ActionSetInstanceData(instMesh, startIndex, data);
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

    for (int32_t i = (int32_t)mExiledNodes.size() - 1; i >= 0; --i)
    {
        mExiledNodes[i]->Destroy();
        mExiledNodes.erase(mExiledNodes.begin() + i);
    }

    // Clear property inspection history also.
    GetEditorState()->ClearInspectHistory();
}

void ActionManager::ExileNode(NodePtr node)
{
    OCT_ASSERT(std::find(mExiledNodes.begin(), mExiledNodes.end(), node) == mExiledNodes.end());
    OCT_ASSERT(node->GetParent() == nullptr);

    mExiledNodes.push_back(node);

    if (GetEditorState()->IsNodeSelected(node.Get()))
    {
        GetEditorState()->SetSelectedNode(nullptr);
    }

    if (GetEditorState()->GetInspectedObject() == node.Get())
    {
        GetEditorState()->InspectObject(nullptr, true);
    }
}

void ActionManager::RestoreExiledNode(NodePtr node)
{
    bool restored = false;

    for (uint32_t i = 0; i < mExiledNodes.size(); ++i)
    {
        if (mExiledNodes[i] == node)
        {
            mExiledNodes.erase(mExiledNodes.begin() + i);
            restored = true;
            break;
        }
    }

    OCT_ASSERT(restored);
}

static bool sHandleNewProjectCallbackCpp = false;
static void HandleNewProjectCallback(const std::vector<std::string>& folderPaths)
{
    if (folderPaths.size() > 0 && folderPaths[0] != "")
    {
        ActionManager::Get()->CreateNewProject(folderPaths[0].c_str(), sHandleNewProjectCallbackCpp);
    }
    else
    {
        LogError("Bad folder for CreateNewProject.");
    }
}

void CpyFile(const std::string& srcFile, const std::string& dstFile)
{
    SYS_Exec((std::string("cp ") + srcFile + " " + dstFile).c_str());
}

void CpyDir(const std::string& srcFile, const std::string& dstFile)
{
    SYS_Exec((std::string("cp -r ") + srcFile + " " + dstFile).c_str());
}

void CopyFileAndReplaceString(const std::string& srcFile, const std::string& dstFile, const std::string& srcString, const std::string& dstString)
{
    CpyFile(srcFile, dstFile);
    ReplaceStringInFile(dstFile, srcString, dstString);
}

void CpyDirWithExclusions(const std::string& srcDir, const std::string& dstDir, const std::vector<std::string> exclusions)
{
    DirEntry dirEntry;
    SYS_OpenDirectory(srcDir, dirEntry);

    // Create dir if it doesn't exist.
    CreateDir(dstDir.c_str());

    std::vector<std::string> subDirs;

    while (dirEntry.mValid)
    {
        std::string fileName = dirEntry.mFilename;
        if (std::find(exclusions.begin(), exclusions.end(), fileName) == exclusions.end())
        {
            if (dirEntry.mDirectory)
            {
                if (fileName != "." && fileName != "..")
                {
                    subDirs.push_back(fileName);
                }
            }
            else
            {
                CpyFile(srcDir + fileName, dstDir + fileName);
            }
        }

        SYS_IterateDirectory(dirEntry);
    }

    SYS_CloseDirectory(dirEntry);

    for (uint32_t i = 0; i < subDirs.size(); ++i)
    {
        CpyDirWithExclusions(srcDir + subDirs[i] + "/", dstDir + subDirs[i] + "/", exclusions);
    }
}

static void CreateConfigIni(const std::string& projDir, const std::string projName)
{
    // Write out an Engine.ini file which is used by Standalone game exe.
    FILE* engineIni = fopen(std::string(projDir + "Config.ini").c_str(), "w");
    if (engineIni != nullptr)
    {
        fprintf(engineIni, "project=%s", projName.c_str());

        fclose(engineIni);
        engineIni = nullptr;
    }
}

void ActionManager::CreateNewProject(const char* folderPath, bool cpp)
{
    std::string folderPathStr = folderPath ? folderPath : "";

    if (folderPathStr == "")
    {
#if USE_IMGUI_FILE_BROWSER
        sHandleNewProjectCallbackCpp = cpp;
        EditorOpenFileBrowser(HandleNewProjectCallback, true);
#else
        folderPathStr = SYS_SelectFolderDialog();
#endif
    }

    if (folderPathStr != "")
    {
        std::string newProjDir = folderPathStr;
        std::replace(newProjDir.begin(), newProjDir.end(), '\\', '/');

        // Remove trailing slash
        if (newProjDir.size() > 0 && newProjDir.back() == '/')
        {
            newProjDir.pop_back();
        }

        std::string newProjName = newProjDir;
        size_t slashLoc = newProjName.find_last_of('/');
        if (slashLoc != std::string::npos)
        {
            newProjName = newProjName.substr(slashLoc + 1);
        }

        LogDebug("CreateNewProject: %s @ %s", newProjName.c_str(), newProjDir.c_str());
        {
            if (!DoesDirExist(newProjDir.c_str()))
            {
                LogDebug("The specified folder was not found, it will now be created @ %s",newProjDir.c_str());
                SYS_CreateDirectory(newProjDir.c_str());
            }
        }

        newProjDir += "/";

        // Now that we have the folder, we need to populate it with an Assets and Scripts folder
        std::string assetsFolder = newProjDir + "Assets";
        std::string scriptsFolder = newProjDir + "Scripts";
        SYS_CreateDirectory(assetsFolder.c_str());
        SYS_CreateDirectory(scriptsFolder.c_str());

        // Also we need to create an octp so that user can open the project with Ctrl+P
        std::string projectFile = newProjDir + newProjName.c_str() + ".octp";
        FILE* octpFile = fopen(projectFile.c_str(), "w");
        if (octpFile != nullptr)
        {
            fprintf(octpFile, "name=%s", newProjName.c_str());

            fclose(octpFile);
            octpFile = nullptr;
        }

        if (cpp)
        {
            std::string standaloneDir = "Standalone/";

            std::string subProjFolder = newProjDir + newProjName.c_str();
            SYS_CreateDirectory(subProjFolder.c_str());
            subProjFolder += "/";

            // Copy .octp folder into the subfolder
            SYS_Exec(("mv " + newProjDir + newProjName.c_str() + ".octp" + " " + subProjFolder + newProjName.c_str() + ".octp").c_str());

            // Create Proj subfolder (Assets and Scripts folders will need to be moved in the subfolder)
            SYS_Exec((std::string("mv ") + assetsFolder.c_str() + " " + subProjFolder + "Assets").c_str());
            SYS_Exec((std::string("mv ") + scriptsFolder.c_str() + " " + subProjFolder + "Scripts").c_str());
            assetsFolder = subProjFolder + "Assets";
            scriptsFolder = subProjFolder + "Scripts";

            // Copy Makefiles and replace "Octave"
            CopyFileAndReplaceString(standaloneDir + "Makefile_3DS", subProjFolder + "Makefile_3DS", "Octave", newProjName);
            ReplaceStringInFile(subProjFolder + "Makefile_3DS", "../Engine", "../Octave/Engine");
            ReplaceStringInFile(subProjFolder + "Makefile_3DS", "../External", "../Octave/External");
            CopyFileAndReplaceString(standaloneDir + "Makefile_GCN", subProjFolder + "Makefile_GCN", "Octave", newProjName);
            ReplaceStringInFile(subProjFolder + "Makefile_GCN", "../Engine", "../Octave/Engine");
            ReplaceStringInFile(subProjFolder + "Makefile_GCN", "../External", "../Octave/External");
            CopyFileAndReplaceString(standaloneDir + "Makefile_Wii", subProjFolder + "Makefile_Wii", "Octave", newProjName);
            ReplaceStringInFile(subProjFolder + "Makefile_Wii", "../Engine", "../Octave/Engine");
            ReplaceStringInFile(subProjFolder + "Makefile_Wii", "../External", "../Octave/External");
            CopyFileAndReplaceString(standaloneDir + "Makefile_Linux_Game", subProjFolder + "Makefile_Linux_Game", "Octave", newProjName);
            ReplaceStringInFile(subProjFolder + "Makefile_Linux_Game", "../Engine", "../Octave/Engine");
            ReplaceStringInFile(subProjFolder + "Makefile_Linux_Game", "../External", "../Octave/External");
            CopyFileAndReplaceString(standaloneDir + "Makefile_Linux_Editor", subProjFolder + "Makefile_Linux_Editor", "Octave", newProjName);
            ReplaceStringInFile(subProjFolder + "Makefile_Linux_Editor", "../Engine", "../Octave/Engine");
            ReplaceStringInFile(subProjFolder + "Makefile_Linux_Editor", "../External", "../Octave/External");

            // Copy and replace Standalone.vcxproj/filters and replace "Standalone" 
            CopyFileAndReplaceString(standaloneDir + "Standalone.vcxproj", subProjFolder + newProjName + ".vcxproj", "Standlone", newProjName);
            ReplaceStringInFile(subProjFolder + newProjName + ".vcxproj", "Octave", newProjName);
            ReplaceStringInFile(subProjFolder + newProjName + ".vcxproj", "../", "../Octave/");
            ReplaceStringInFile(subProjFolder + newProjName + ".vcxproj", "$(SolutionDir)Engine", "$(SolutionDir)Octave\\Engine");
            ReplaceStringInFile(subProjFolder + newProjName + ".vcxproj", "$(SolutionDir)External", "$(SolutionDir)Octave\\External");
            ReplaceStringInFile(subProjFolder + newProjName + ".vcxproj", "..\\Engine\\", "..\\Octave\\Engine\\");
            CpyFile(standaloneDir + "Standalone.vcxproj.filters", subProjFolder + newProjName + ".vcxproj.filters");

            // Copy Source folder
            CpyDir(standaloneDir + "Source", subProjFolder + "Source");
            ReplaceStringInFile(subProjFolder + "Source/Main.cpp", "Octave", newProjName);
            ReplaceStringInFile(subProjFolder + "Source/Main.cpp", "config.mStandalone = true", "config.mStandalone = false");

            // Copy android files (whatevers in git, exclude .cxx and build)
            CpyDirWithExclusions(standaloneDir + "Android/", subProjFolder + "Android/", { "Build", "build", "Intermediate", ".gradle", ".cxx", ".vs", ".git", "imgui.ini", "assets"});

#if 0
            // Create a symlink to the Octave directory
            CreateSymLink(standaloneDir + "../../Octave", newProjDir + "Octave");
#else
            // Copy Engine, External, to Proj folder (EXCLUDE Build and Intermediate folders)
            CpyDirWithExclusions(standaloneDir + "../../Octave/", newProjDir + "Octave/", {"Build", "build", "Intermediate", ".gradle", ".cxx", ".vs", ".git", "imgui.ini"});
#endif

            // Copy Octave.sln  - Replace "Standalone" with Proj Name
            CpyFile(standaloneDir + "../Octave.sln", newProjDir + newProjName + ".sln");
            ReplaceStringInFile(newProjDir + newProjName + ".sln", "Engine\\Engine.vcxproj", "Octave\\Engine\\Engine.vcxproj");
            ReplaceStringInFile(newProjDir + newProjName + ".sln", "External\\", "Octave\\External\\");
            ReplaceStringInFile(newProjDir + newProjName + ".sln", "Standalone", newProjName);
            ReplaceStringInFile(newProjDir + newProjName + ".sln", "\"External\", \"External\",", "\"External\", \"Octave\\External\",");

            // Copy .vscode folder from Octave to SubProjDir
            CpyDir(standaloneDir + "../.vscode", newProjDir + ".vscode");
            ReplaceStringInFile(newProjDir + ".vscode/tasks.json", "Standalone", newProjName);
            ReplaceStringInFile(newProjDir + ".vscode/launch.json", "Standalone", newProjName);
            ReplaceStringInFile(newProjDir + ".vscode/launch.json", "\"name\": \"Octave" , "\"name\": \"" + newProjName);
        }

        ResetEngineConfig();
        GetMutableEngineConfig()->mProjectName = newProjName;
        WriteEngineConfig(newProjDir + "/Config.ini");

        // Finally, open the project
        OpenProject(projectFile.c_str());
    }
}

static void HandleOpenProjectCallback(const std::vector<std::string>& filePaths)
{
    if (filePaths.size() > 0 && filePaths[0] != "")
    {
        ActionManager::Get()->OpenProject(filePaths[0].c_str());
    }
    else
    {
        LogError("Bad file for OpenProject.");
    }
}

void ActionManager::OpenProject(const char* path)
{
    std::string pathStr = path ? path : "";

    if (pathStr == "")
    {
#if USE_IMGUI_FILE_BROWSER
        EditorOpenFileBrowser(HandleOpenProjectCallback, false);
#else
        std::vector<std::string> paths = SYS_OpenFileDialog();

        if (paths.size() > 0)
        {
            pathStr = paths[0];
        }
#endif
    }

    if (pathStr != "")
    {
        LoadProject(pathStr);

        // Handle new project directory
        GetEditorState()->ClearAssetDirHistory();
        GetEditorState()->SetAssetDirectory(AssetManager::Get()->FindProjectDirectory(), true);
        GetEditorState()->SetSelectedAssetStub(nullptr);
    }
}

void ActionManager::OpenScene()
{
    // TODO-NODE: This uses SYS_OpenFileDialog() and I think we should just get rid of this function.
    if (GetEngineState()->mProjectPath == "")
        return;

    std::vector<std::string> openPaths = SYS_OpenFileDialog();

    // Display the Open dialog box. 
    if (openPaths.size() > 0)
    {
        std::string filename = strrchr(openPaths[0].c_str(), '/') + 1;
        filename = filename.substr(0, filename.find_last_of('.'));
        AssetStub* stub = FetchAssetStub(filename);

        if (stub != nullptr &&
            stub->mType == Scene::GetStaticType())
        {
            AssetManager::Get()->LoadAsset(*stub);
            Scene* loadedScene = (Scene*)stub->mAsset;

            OpenScene(loadedScene);
        }
        else
        {
            LogError("Failed to fetch Level from AssetManager");
        }
    }
}

void ActionManager::OpenScene(Scene* scene)
{
    GetEditorState()->OpenEditScene(scene);
}

void ActionManager::SaveScene(bool saveAs)
{
    EditScene* editScene = GetEditorState()->GetEditScene();

    if (GetEngineState()->mProjectPath == "" ||
        editScene == nullptr)
        return;

    if (saveAs || editScene->mSceneAsset == nullptr)
    {
        GetEditorState()->mRequestSaveSceneAs = true;

#if 0
        // Old method of bringing up OS file browser
        std::string savePath = SYS_SaveFileDialog();

        // Display the Open dialog box. 
        if (savePath != "")
        {
            std::replace(savePath.begin(), savePath.end(), '\\', '/');
            std::string newSceneName = Asset::GetNameFromPath(savePath);
            AssetStub* stub = AssetManager::Get()->CreateAndRegisterAsset(Scene::GetStaticType(), nullptr, newSceneName, false);

            if (stub != nullptr)
            {
                Scene* newScene = (Scene*)stub->mAsset;
                newScene->SetName(newSceneName);
                editScene->mSceneAsset = newScene;
            }
        }
#endif
    }
    else if (editScene->mSceneAsset != nullptr)
    {
        Scene* scene = editScene->mSceneAsset.Get<Scene>();
        scene->Capture(GetWorld(0)->GetRootNode());
        AssetManager::Get()->SaveAsset(scene->GetName());
    }
}

void ActionManager::SaveSelectedAsset()
{
    AssetStub* selectedStub = GetEditorState()->GetSelectedAssetStub();
    if (selectedStub != nullptr &&
        selectedStub->mAsset != nullptr)
    {
        AssetManager::Get()->SaveAsset(*selectedStub);
    }
}

void ActionManager::DeleteSelectedNodes()
{
    std::vector<Node*> nodes = GetEditorState()->GetSelectedNodes();

    for (uint32_t i = 0; i < nodes.size(); ++i)
    {
        // TODO-NODE: Previously we checked here if the node was the World's active camera.
        //   Now that we use a totally separate editor camera in EditorState, I removed the check.
        //   Verify editor cam works as expected.
        if (nodes[i] == nullptr)
        {
            nodes.erase(nodes.begin() + i);
            --i;
        }
    }

    EXE_DeleteNodes(nodes);
    GetEditorState()->SetSelectedNode(nullptr);
}

void ActionManager::DeleteNode(Node* node)
{
    if (node != nullptr &&
        node != GetEditorState()->GetEditorCamera())
    {
        EXE_DeleteNode(node);
    }
}

static void HandleRunScriptCallback(const std::vector<std::string>& filePaths)
{
    lua_State* L = GetLua();

    for (uint32_t i = 0; i < filePaths.size(); ++i)
    {
        // Don't use ScriptUtils::RunScript(). Allow script to exist outside of the project.
        if (luaL_dofile(L, filePaths[i].c_str()))
        {
            LogError("Lua Error: %s\n", lua_tostring(L, -1));
        }
    }
}

void ActionManager::RunScript()
{
#if USE_IMGUI_FILE_BROWSER
    EditorOpenFileBrowser(HandleRunScriptCallback, false);
#else
    std::vector<std::string> filePaths = SYS_OpenFileDialog();
    HandleRunScriptCallback(filePaths);
#endif
}

static void HandleImportCallback(const std::vector<std::string>& filePaths)
{
    for (uint32_t i = 0; i < filePaths.size(); ++i)
    {
        if (filePaths[i] != "")
        {
            ActionManager::Get()->ImportAsset(filePaths[i].c_str());
        }
        else
        {
            LogError("Bad file for ImportAsset.");
        }
    }
}

static void HandleImportSceneCallback(const std::vector<std::string>& filePaths)
{
    if (filePaths.size() > 1)
    {
        LogWarning("Can only import one scene at a time. Ignoring extra scenes.");
    }

    if (filePaths.size() >= 1)
    {
        GetEditorState()->mPendingSceneImportPath = filePaths[0];
    }
}

void ActionManager::ImportAsset()
{
    if (GetEngineState()->mProjectPath != "")
    {
#if USE_IMGUI_FILE_BROWSER
        EditorOpenFileBrowser(HandleImportCallback, false);
#else
        std::vector<std::string> filePaths = SYS_OpenFileDialog();
        HandleImportCallback(filePaths);
#endif
    }
    else
    {
        LogWarning("Cannot import asset. No project loaded.");
    }
}

Asset* ActionManager::ImportAsset(const std::string& path)
{
    Asset* retAsset = nullptr;

    std::string filename = path;
    size_t lastSlashIdx = filename.find_last_of("/\\");
    std::vector<TypeId> importTypes;
    std::vector<int32_t> meshIndices;

    if (lastSlashIdx != std::string::npos)
    {
        filename = filename.substr(lastSlashIdx + 1);
    }

    int32_t dotIndex = int32_t(filename.find_last_of('.'));
    std::string extension = filename.substr(dotIndex, filename.size() - dotIndex);

    if (extension == ".png" ||
        extension == ".bmp" ||
        extension == ".jpeg" ||
        extension == ".jpg" ||
        extension == ".tga")
    {
        importTypes.push_back(Texture::GetStaticType());
    }
    else if (extension == ".dae" ||
        extension == ".fbx" ||
        extension == ".glb" ||
        extension == ".gltf" ||
        extension == ".obj")
    {
        GatherMeshImportTypes(path.c_str(), importTypes, meshIndices);
    }
    else if (extension == ".wav")
    {
        importTypes.push_back(SoundWave::GetStaticType());
    }
    else if (extension == ".ttf" || extension == ".xml")
    {
        importTypes.push_back(Font::GetStaticType());
    }

    if (importTypes.size() == 0)
    {
        LogError("Failed to import Asset. Unrecognized source asset extension.");
    }

    for (uint32_t i = 0; i < importTypes.size(); ++i)
    {
        Asset* newAsset = nullptr;
        bool success = false;
        TypeId typeId = importTypes[i];
        ImportOptions options;

        if (typeId == StaticMesh::GetStaticType() ||
            typeId == SkeletalMesh::GetStaticType())
        {
            OCT_ASSERT(meshIndices.size() == importTypes.size());
            if (importTypes.size() > 1)
            {
                options.SetOptionValue("meshIndex", meshIndices[i]);
            }
        }

        newAsset = Asset::CreateInstance(typeId);

        std::string assetName = filename.substr(0, dotIndex);
        newAsset->SetName(assetName);

        success = newAsset->Import(path, &options);

        if (success)
        {
            // Update the asset name in case it was changed by the Import() function.
            // This happens when importing multiple meshes within the same file
            assetName = newAsset->GetName();

            AssetDir* assetDir = GetEditorState()->GetAssetDirectory();
            filename = assetName + ".oct";

            // Clear inspected asset if we are reimporting that same asset.
            Asset* oldAsset = FetchAsset(assetName.c_str());

            if (oldAsset != nullptr)
            {
                if (GetEditorState()->GetInspectedObject() == oldAsset)
                {
                    GetEditorState()->InspectObject(nullptr, true);
                }
            }

#if ASSET_LIVE_REF_TRACKING
            // If this asset already exists, then we are about to delete it and replace it.
            // So let's fix up any references now or else they will be lost (replaced with nullptr).
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

            // If a StaticMesh/SkeletalMesh is being imported, and there is a selected material, then assign
            // the material to that static mesh.
            if (newAsset != nullptr &&
                (newAsset->Is(StaticMesh::ClassRuntimeId()) || newAsset->Is(SkeletalMesh::ClassRuntimeId())) &&
                GetEditorState()->GetSelectedAsset() != nullptr &&
                GetEditorState()->GetSelectedAsset()->Is(Material::ClassRuntimeId()))
            {
                Material* material = GetEditorState()->GetSelectedAsset()->As<Material>();

                if (newAsset->Is(StaticMesh::ClassRuntimeId()))
                {
                    StaticMesh* mesh = newAsset->As<StaticMesh>();
                    mesh->SetMaterial(material);
                }
                else if (newAsset->Is(SkeletalMesh::ClassRuntimeId()))
                {
                    SkeletalMesh* mesh = newAsset->As<SkeletalMesh>();
                    mesh->SetMaterial(material);
                }
            }

            AssetManager::Get()->SaveAsset(*stub);

            retAsset = newAsset;
        }
        else
        {
            LogError("Failed to import asset.");

            delete newAsset;
            newAsset = nullptr;
        }
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

static void ConvertFileToByteString(
    const std::string& filePath,
    const std::string& name,
    std::string& outString,
    uint32_t& outSize)
{
    Stream stream;

    stream.ReadFile(filePath.c_str(), false);
    outSize = uint32_t(stream.GetSize());
    char* data = stream.GetData();

    outString.clear();
    outString.reserve(2048);

    std::string assetDataVar = name;
    outString += "extern const char ";
    outString += assetDataVar;
    outString += "[] = \n{\n";

    for (uint32_t byte = 0; byte < outSize; ++byte)
    {
        char byteString[8] = {};
        sprintf(byteString, "'\\x%02X',", uint8_t(data[byte]));

        outString += byteString;

        if (byte % 8 == 7)
        {
            outString += "\n";
        }
    }

    outString += "\n};\n\n";
}

static void SpawnAiNode(aiNode* node, Node* root, const glm::mat4& parentTransform, const std::vector<StaticMesh*>& meshList, const SceneImportOptions& options)
{
    if (node == nullptr || root == nullptr)
        return;

    World* world = GetWorld(0);

    glm::mat4 transform = parentTransform * glm::transpose(glm::make_mat4(&node->mTransformation.a1));

    for (uint32_t i = 0; i < node->mNumMeshes; ++i)
    {
        uint32_t meshIndex = node->mMeshes[i];
        StaticMesh3D* newMesh = root->CreateChild<StaticMesh3D>();

        newMesh->SetStaticMesh(meshList[meshIndex]);
        newMesh->SetUseTriangleCollision(meshList[meshIndex]->IsTriangleCollisionMeshEnabled());
        newMesh->SetTransform(transform);
        newMesh->SetName(/*options.mPrefix + */node->mName.C_Str());
        newMesh->EnableCastShadows(true);
        newMesh->SetBakeLighting(true);
        newMesh->SetUseTriangleCollision(true);
        newMesh->EnableCollision(options.mEnableCollision);
    }

    for (uint32_t i = 0; i < node->mNumChildren; ++i)
    {
        SpawnAiNode(node->mChildren[i], root, parentTransform, meshList, options);
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

    World* world = GetWorld(0);
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
            AssetDir* curDir = GetEditorState()->GetAssetDirectory();

            const std::string& sceneName = options.mSceneName;
            AssetDir* sceneDir = curDir->CreateSubdirectory(sceneName);

            std::string fullSceneName = "SC_" + sceneName;
            for (uint32_t i = 0; i < GetEditorState()->mEditScenes.size(); ++i)
            {
                Scene* scene = GetEditorState()->mEditScenes[i].mSceneAsset.Get<Scene>();
                if (scene && scene->GetName() == fullSceneName)
                {
                    GetEditorState()->CloseEditScene((int32_t)i);
                    break;
                }
            }

            GarbageCollect();

            if (sceneDir == nullptr ||
                sceneDir->mParentDir == nullptr)
            {
                LogError("Invalid directory. Use the asset panel to navigate to a valid directory");
                return;
            }

            sceneDir->Purge();
            GetEditorState()->SetAssetDirectory(sceneDir, true);

            SharedPtr<Node3D> rootNode = Node::Construct<Node3D>();
            rootNode->SetName(sceneName);

            std::vector<Texture*> textureList;
            std::vector<Material*> materialList;
            std::vector<StaticMesh*> meshList;
            std::unordered_map<std::string, Texture*> textureMap;

            uint32_t numMaterials = scene->mNumMaterials;
            for (uint32_t i = 0; i < numMaterials; ++i)
            {
                aiMaterial* aMaterial = scene->mMaterials[i];
                std::string materialName = options.mPrefix + aMaterial->GetName().C_Str();

                if (materialName.size() < 2 || (materialName.substr(0, 2) != "M_"))
                {
                    materialName = std::string("M_") + materialName;
                }

                AssetStub* materialStub = nullptr;
                MaterialLite* newMaterial = nullptr;
                if (options.mImportMaterials)
                {
                    materialStub = EditorAddUniqueAsset(materialName.c_str(), sceneDir, MaterialLite::GetStaticType(), true);
                    newMaterial = static_cast<MaterialLite*>(materialStub->mAsset);
                    newMaterial->SetShadingModel(options.mDefaultShadingModel);
                    newMaterial->SetVertexColorMode(options.mDefaultVertexColorMode);
                }

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
                        else if (options.mImportTextures)
                        {
                            // Case 2 - Texture needs to be loaded.
                            // To make texturing sharing simpler, we only import the texture if
                            //  - There is no texture registered
                            //  - There is a texture registered, and it resides in the current AssetDir

                            bool importTexture = false;

                            std::string assetName = EditorGetAssetNameFromPath(texturePath);
                            if (assetName.size() >= 2 && (strncmp(assetName.c_str(), "T_", 2) == 0))
                            {
                                // Remove the T_ prefix, reapply later.
                                assetName = assetName.substr(2);
                            }

                            assetName = options.mPrefix + assetName;
                            assetName = GetFixedFilename(assetName.c_str(), "T_");

                            AssetStub* existingStub = AssetManager::Get()->GetAssetStub(assetName);
                            if (existingStub && existingStub->mDirectory != sceneDir)
                            {
                                textureToAssign = LoadAsset<Texture>(assetName);
                            }

                            bool embeddedTexturePath = (texturePath.size() > 1 && texturePath[0] == '*');
                            if (textureToAssign == nullptr && !embeddedTexturePath)
                            {
                                Asset* importedAsset = ImportAsset(importDir + texturePath);
                                OCT_ASSERT(importedAsset == nullptr || importedAsset->GetType() == Texture::GetStaticType());

                                if (importedAsset == nullptr || importedAsset->GetType() == Texture::GetStaticType())
                                {
                                    textureToAssign = (Texture*)importedAsset;
                                }

                                if (importedAsset != nullptr)
                                {
                                    AssetManager::Get()->RenameAsset(importedAsset, assetName);
                                    AssetManager::Get()->SaveAsset(assetName);
                                }
                            }

                            if (textureToAssign != nullptr)
                            {
                                textureMap.insert({ texturePath, textureToAssign });
                            }
                        }

                        if (newMaterial != nullptr && textureToAssign != nullptr)
                        {
                            newMaterial->SetTexture(t, textureToAssign);
                        }
                    }
                }

                if (materialStub != nullptr)
                {
                    AssetManager::Get()->SaveAsset(*materialStub);
                    materialList.push_back(newMaterial);
                }
            }

            // Create static mesh assets (assign corresponding material)
            uint32_t numMeshes = scene->mNumMeshes;

            for (uint32_t i = 0; i < numMeshes; ++i)
            {
                aiMesh* aMesh = scene->mMeshes[i];
                std::string meshName = options.mPrefix + aMesh->mName.C_Str();

                if (meshName.size() < 3 || (meshName.substr(0, 3) != "SM_"))
                {
                    meshName = std::string("SM_") + meshName;
                }

                // Ensure unique name (this normally happens when model has multiple materials).
                std::string uniqueName = meshName;
                int32_t uniqueNum = 1;
                for (int32_t u = 0; u < (int32_t)meshList.size(); ++u)
                {
                    if (meshList[u]->GetName() == uniqueName)
                    {
                        uniqueName = meshName + "_" + std::to_string(uniqueNum);
                        uniqueNum++;
                        u = -1;
                    }
                }
                meshName = uniqueName;

                StaticMesh* existingMesh = LoadAsset<StaticMesh>(meshName);
                StaticMesh* meshToAddToList = existingMesh;

                if (options.mImportMeshes)
                {
                    StaticMesh* newMesh = (StaticMesh*)Asset::CreateInstance(StaticMesh::GetStaticType());
                    newMesh->Create(scene, *aMesh, 0, nullptr); // Collision meshes currently not supported for scene import?
                    meshToAddToList = newMesh;

                    if (existingMesh)
                    {
                        newMesh->SetGenerateTriangleCollisionMesh(existingMesh->IsTriangleCollisionMeshEnabled());
                        newMesh->SetMaterial(existingMesh->GetMaterial());
                        AssetManager::Get()->PurgeAsset(existingMesh->GetName().c_str());
                        existingMesh = nullptr;
                    }

                    if (options.mImportMaterials)
                    {
                        // Find material to use...
                        uint32_t materialIndex = aMesh->mMaterialIndex;
                        OCT_ASSERT(materialIndex < materialList.size());
                        newMesh->SetMaterial(materialList[materialIndex]);
                    }

                    AssetStub* meshStub = EditorAddUniqueAsset(meshName.c_str(), sceneDir, StaticMesh::GetStaticType(), false);
                    meshStub->mAsset = newMesh;
                    newMesh->SetName(meshName);
                    newMesh->SetGenerateTriangleCollisionMesh(true);
                    AssetManager::Get()->SaveAsset(*meshStub);
                }

                meshList.push_back(meshToAddToList);
            }

            // Create Lights
            if (options.mImportLights)
            {
                uint32_t numLights = scene->mNumLights;
                for (uint32_t i = 0; i < numLights; ++i)
                {
                    aiLight* aLight = scene->mLights[i];

                    if (aLight->mType == aiLightSource_POINT)
                    {
                        PointLight3D* pointLight = rootNode->CreateChild<PointLight3D>();

                        glm::vec3 lightColor;
                        lightColor.r = aLight->mColorDiffuse.r;
                        lightColor.g = aLight->mColorDiffuse.g;
                        lightColor.b = aLight->mColorDiffuse.b;
                        lightColor = Maths::SafeNormalize(lightColor);
                        pointLight->SetColor(glm::vec4(lightColor, 1.0f));

                        pointLight->SetLightingDomain(LightingDomain::All);

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
                        pointLight->UpdateTransform(true);

                        pointLight->SetName(aLight->mName.C_Str());
                    }
                }
            }

            aiNode* node = scene->mRootNode;
            SpawnAiNode(node, rootNode.Get(), glm::mat4(1), meshList, options);

            AssetStub* sceneStub = EditorAddUniqueAsset(fullSceneName.c_str(), sceneDir, Scene::GetStaticType(), true);
            Scene* newScene = sceneStub->mAsset ? sceneStub->mAsset->As<Scene>() : nullptr;

            if (newScene)
            {
                newScene->Capture(rootNode.Get());
                AssetManager::Get()->SaveAsset(*sceneStub);

                GetEditorState()->OpenEditScene(newScene);
            }
            else
            {
                LogError("Failed to create new scene asset for scene import");
            }
        }
        else
        {
            LogError("Failed to import scene. File format must be .glb or .gltf");
        }
    }
}

void ActionManager::BeginImportScene()
{
    if (GetEngineState()->mProjectPath != "")
    {
#if USE_IMGUI_FILE_BROWSER
        EditorOpenFileBrowser(HandleImportSceneCallback, false);
#else
        std::vector<std::string> filePaths = SYS_OpenFileDialog();
        HandleImportSceneCallback(filePaths);
#endif
    }
    else
    {
        LogWarning("Cannot import scene. No project loaded.");
    }

}

void ActionManager::GenerateEmbeddedAssetFiles(
    bool embeddedBuild,
    std::vector<std::pair<AssetStub*, std::string> >& assets,
    const char* headerPath,
    const char* sourcePath)
{
    FILE* headerFile = fopen(headerPath, "w");
    FILE* sourceFile = fopen(sourcePath, "w");

    OCT_ASSERT(headerFile != nullptr && sourceFile != nullptr);

    if (headerFile != nullptr && sourceFile != nullptr)
    {
        fprintf(headerFile, "#include <stdint.h>\n");
        fprintf(headerFile, "#include \"EmbeddedFile.h\"\n\n");

        fprintf(headerFile, "extern uint32_t gNumEmbeddedAssets;\n");
        fprintf(headerFile, "extern EmbeddedFile gEmbeddedAssets[];\n\n");
        fprintf(headerFile, "extern const char gEmbeddedConfig_Data[];\n\n");
        fprintf(headerFile, "extern uint32_t gEmbeddedConfig_Size;\n\n");

        fprintf(sourceFile, "#include <stdint.h>\n");
        fprintf(sourceFile, "#include \"EmbeddedFile.h\"\n\n");

        std::string initializer;

        for (int32_t i = 0; i < int32_t(assets.size()); ++i)
        {
            AssetStub* stub = assets[i].first;
            const std::string& packPath = assets[i].second;
            std::string dataVarName = stub->mName + "_Data";
            uint32_t dataSize = 0;

            std::string sourceString;
            ConvertFileToByteString(
                packPath,
                dataVarName,
                sourceString,
                dataSize);

            fprintf(sourceFile, "%s", sourceString.c_str());

            initializer += "{" + ("\"" + stub->mName + "\",") +
                                 (dataVarName + ",") +
                                 (std::to_string(dataSize) + ",") +
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


        {
            std::string sourceString;
            uint32_t dataSize = 0;

            if (embeddedBuild)
            {
                ConvertFileToByteString(
                    GetEngineState()->mProjectDirectory + "Config.ini",
                    "gEmbeddedConfig_Data",
                    sourceString,
                    dataSize);

                fprintf(sourceFile, "%s", sourceString.c_str());
            }
            else
            {
                fprintf(sourceFile, "extern const char gEmbeddedConfig_Data[] = {};\n");
            }

            fprintf(sourceFile, "extern const uint32_t gEmbeddedConfig_Size = %d;\n", dataSize);

        }

        fclose(headerFile);
        headerFile = nullptr;
        fclose(sourceFile);
        sourceFile = nullptr;
    }
}

void ActionManager::GenerateEmbeddedScriptFiles(
    bool embeddedBuild,
    std::vector<std::string> files,
    const char* headerPath,
    const char* sourcePath)
{
    FILE* headerFile = fopen(headerPath, "w");
    FILE* sourceFile = fopen(sourcePath, "w");

    OCT_ASSERT(headerFile != nullptr && sourceFile != nullptr);

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
            std::string luaClass = ScriptUtils::GetClassNameFromFileName(luaFile);

            Stream stream;

            // Handle special case for level
            stream.ReadFile(luaFile.c_str(), false);
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

                if (strcmp(dirEntry.mFilename, "LuaPanda.lua") != 0 &&
                    extension != nullptr &&
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
    GetEditorState()->SetSelectedNode(nullptr);
    //SetActiveLevel(nullptr);
    GetWorld(0)->Clear();

    ResetUndoRedo();
}

void ActionManager::DeleteAllNodes()
{
    if (!IsPlayingInEditor())
    {
        GetEditorState()->SetSelectedNode(nullptr);

        if (GetWorld(0)->GetRootNode() != nullptr)
        {
            EXE_DeleteNode(GetWorld(0)->GetRootNode());
        }
    }

}

void ActionManager::RecaptureAndSaveAllScenes()
{
    std::unordered_map<std::string, AssetStub*>& assetMap = AssetManager::Get()->GetAssetMap();

    // This will load all assets!
    for (auto& pair : assetMap)
    {
        if (pair.second->mType == Scene::GetStaticType())
        {
            Asset* asset = AssetManager::Get()->LoadAsset(*pair.second);
            Scene* scene = static_cast<Scene*>(asset);
            OCT_ASSERT(scene != nullptr);

            NodePtr temp = scene->Instantiate();
            scene->Capture(temp.Get());

            temp->Destroy();
            temp = nullptr;

            AssetManager::Get()->SaveAsset(*pair.second);
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
        if (GetEditorState()->GetSelectedAssetStub() == stub)
        {
            GetEditorState()->SetSelectedAssetStub(nullptr);
        }

        if (GetEditorState()->GetInspectedObject() == stub->mAsset)
        {
            GetEditorState()->InspectObject(nullptr);
        }

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

bool ActionManager::DuplicateNodes(std::vector<Node*> srcNodes)
{
    bool duplicated = false;

    // Don't use a vector reference for nodes param because we are going to modify the vector anyway.
    std::vector<Node*> dupedNodes;
    RemoveRedundantDescendants(srcNodes);

    OCT_ASSERT(srcNodes.size() > 0);
    dupedNodes = EXE_SpawnNodes(srcNodes);

    if (dupedNodes.size() > 0 &&
        dupedNodes.size() == srcNodes.size())
    {
        duplicated = true;

        for (uint32_t i = 0; i < srcNodes.size(); ++i)
        {
            Node* srcNode = srcNodes[i];
            Node* newNode = dupedNodes[i];
            Node* parent = srcNode->GetParent();
            if (parent == nullptr)
            {
                parent = srcNode;
            }

            parent->AddChild(newNode);
        }

        GetEditorState()->SetSelectedNode(nullptr);

        for (uint32_t i = 0; i < dupedNodes.size(); ++i)
        {
            GetEditorState()->AddSelectedNode(dupedNodes[i], false);
        }
    }

    return duplicated;
}

void ActionManager::AttachSelectedNodes(Node* newParent, int32_t boneIdx)
{
    if (newParent == nullptr)
        return;

    if (newParent->IsSceneLinked())
    {
        LogWarning("Cannot add child to scene-linked node. Unlink the scene first.");
        return;
    }

    std::vector<Node*> selNodes = GetEditorState()->GetSelectedNodes();

    if (selNodes.size() == 0)
        return;

    for (uint32_t i = 0; i < selNodes.size(); ++i)
    {
        Node* child = selNodes[i];
        Node* parent = newParent;

        if (child == parent)
            continue;

        if (parent->GetParent() == child)
            continue;

        int32_t boneIndex = -1;

        if (child->As<Node3D>())
        {
            boneIndex = child->As<Node3D>()->GetParentBoneIndex();
        }

        if (child->GetParent() != parent ||
            boneIdx != boneIndex)
        {
            // TODO: Do one EXE for all nodes so user doesn't have to CTRL+Z N times.
            ActionManager::Get()->EXE_AttachNode(child, parent, -1, boneIdx);

            // Reparenting components should break the scene link.
            // For now, you cannot override scene instance children
            if (newParent->GetParent() != GetWorld(0)->GetRootNode())
            {
                newParent->SetScene(nullptr);
            }
        }
    }

}

// ---------------------------
// --------- ACTIONS ---------
// ---------------------------

static void MarkEditSceneDirty()
{
    Scene* sceneToDirty = nullptr;
    EditScene* editScene = GetEditorState()->GetEditScene();
    if (editScene)
    {
        sceneToDirty = editScene->mSceneAsset.Get<Scene>();
    }

    if (sceneToDirty)
    {
        sceneToDirty->SetDirtyFlag();
    }
}
void Action::Execute()
{
    MarkEditSceneDirty();
}

void Action::Reverse()
{
    MarkEditSceneDirty();
}

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
    if (mOwnerType == PropertyOwnerType::Node)
    {
        Node* node = (Node*)mOwner;
        node->GatherProperties(props);
    }
    else if (mOwnerType == PropertyOwnerType::Asset)
    {
        Asset* asset = (Asset*)mReferencedAsset.Get<Asset>();
        if (asset)
        {
            asset->GatherProperties(props);
        }
    }
    else if (mOwnerType == PropertyOwnerType::Global)
    {
        GatherGlobalProperties(props);
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
    Action::Execute();

    std::vector<Property> sProps;
    GatherProps(sProps);

    Property* prop = FindProp(sProps, mPropertyName);

    if (prop != nullptr)
    {
        if (prop->IsVector() && !prop->IsExternal() && mIndex >= prop->GetCount())
        {
            // In the case of Asset properties, they will be nil if not set, thus
            // when we regather script properties at the start of this function, the count will not include
            // those nil values.
            mIndex = prop->GetCount();
            prop->SetCount(prop->GetCount() + 1);
        }

        // TODO-NODE: Support undo/redo for AddPropertyArray. Until then, don't set value on OOB elements.
        if (mIndex < prop->GetCount())
        {
            mPreviousValue.Destroy();
            mPreviousValue.SetType(prop->GetType());
            mPreviousValue.SetCount(1);
            mPreviousValue.SetValue(prop->GetValue(mIndex));

            prop->SetValue(mValue.mData.vp, mIndex, 1);

            // TODO-NODE: Delete this? Shouldn't be necessary anymore now that we are using Imgui
            //   and gathering properties every frame.
#if 0
            // Script properties are stored internally and propagated to scripts
            // so after setting the value we need to refresh property widgets.
                if (!prop->IsExternal())
                {
                    PanelManager::Get()->GetPropertiesPanel()->RefreshProperties();
            }
#endif
        }
    }
}

void ActionEditProperty::Reverse()
{
    Action::Reverse();

    std::vector<Property> sProps;
    GatherProps(sProps);

    Property* prop = FindProp(sProps, mPropertyName);

    if (prop != nullptr)
    {
        // TODO-NODE: Support undo/redo for AddPropertyArray. Until then, don't set value on OOB elements.
        if (prop->GetCount() > mIndex)
        {
            prop->SetValue(mPreviousValue.GetValue(0), mIndex, 1);
        }

        // TODO-NODE: Delete this? Shouldn't be necessary anymore now that we are using Imgui
        //   and gathering properties every frame.
#if 0
        // Script properties are stored internally and propagated to scripts
        // so after setting the value we need to refresh property widgets.
        if (!prop->IsExternal())
        {
            PanelManager::Get()->GetPropertiesPanel()->RefreshProperties();
        }
#endif
    }
}

ActionEditTransforms::ActionEditTransforms(
    const std::vector<Node3D*>& nodes,
    const std::vector<glm::mat4>& newTransforms)
{
    mNodes = nodes;
    mNewTransforms = newTransforms;

    OCT_ASSERT(mNodes.size() == mNewTransforms.size());
}

void ActionEditTransforms::Execute()
{
    Action::Execute();

    mPrevTransforms.clear();

    for (uint32_t i = 0; i < mNodes.size(); ++i)
    {
        mPrevTransforms.push_back(mNodes[i]->GetTransform());
        mNodes[i]->SetTransform(mNewTransforms[i]);
    }
}

void ActionEditTransforms::Reverse()
{
    Action::Reverse();

    OCT_ASSERT(mPrevTransforms.size() == mNodes.size());

    for (uint32_t i = 0; i < mNodes.size(); ++i)
    {
        mNodes[i]->SetTransform(mPrevTransforms[i]);
    }
}

ActionSpawnNodes::ActionSpawnNodes(const std::vector<TypeId>& types)
{
    mSrcTypes = types;
}

ActionSpawnNodes::ActionSpawnNodes(const std::vector<const char*>& typeNames)
{
    mSrcTypeNames = typeNames;
}

ActionSpawnNodes::ActionSpawnNodes(const std::vector<SceneRef>& scenes)
{
    mSrcScenes = scenes;
}

ActionSpawnNodes::ActionSpawnNodes(const std::vector<Node*>& srcNodes)
{
    std::vector<Node*> trimmedSrcNodes = srcNodes;
    RemoveRedundantDescendants(trimmedSrcNodes);

    mSrcNodes.resize(trimmedSrcNodes.size());

    for (uint32_t i = 0; i < mSrcNodes.size(); ++i)
    {
        mSrcNodes[i] = ResolvePtr(trimmedSrcNodes[i]);
    }
}

void ActionSpawnNodes::Execute()
{
    Action::Execute();

    if (mNodes.size() == 0)
    {
        // First time executing this action. We need to create the nodes from the src data.
        if (mSrcTypes.size() > 0)
        {
            for (uint32_t i = 0; i < mSrcTypes.size(); ++i)
            {
                NodePtr newNode = Node::Construct(mSrcTypes[i]);
                OCT_ASSERT(newNode);
                mNodes.push_back(newNode);
            }
        }
        else if (mSrcTypeNames.size() > 0)
        {
            for (uint32_t i = 0; i < mSrcTypeNames.size(); ++i)
            {
                NodePtr newNode = Node::Construct(mSrcTypeNames[i]);
                OCT_ASSERT(newNode);
                mNodes.push_back(newNode);
            }
        }
        else if (mSrcScenes.size() > 0)
        {
            for (uint32_t i = 0; i < mSrcScenes.size(); ++i)
            {
                Scene* scene = mSrcScenes[i].Get<Scene>();
                if (scene != nullptr)
                {
                    NodePtr newNode = scene->Instantiate();
                    OCT_ASSERT(newNode);
                    mNodes.push_back(newNode);
                }
                else
                {
                    LogError("Null scene in ActionSpawnNodes::Execute()");
                }
            }
        }
        else if (mSrcNodes.size() > 0)
        {
            for (uint32_t i = 0; i < mSrcNodes.size(); ++i)
            {
                OCT_ASSERT(mSrcNodes[i] != nullptr);

                NodePtr newNode = mSrcNodes[i]->Clone(true, true, true);
                OCT_ASSERT(newNode);
                mNodes.push_back(newNode);
            }
        }
        else
        {
            LogError("Invalid src data for ActionSpawnNodes!");
        }
    }
    else 
    {
        // Second time and beyond. Restore exiled nodes and attach to correct parents.
        for (uint32_t i = 0; i < mNodes.size(); ++i)
        {
            ActionManager::Get()->RestoreExiledNode(mNodes[i]);

            if (mParents[i] != nullptr)
            {
                mNodes[i]->Attach(mParents[i].Get());
            }
            else
            {
                // This must have been the root node?
                OCT_ASSERT(mNodes.size() == 1);
                OCT_ASSERT(GetWorld(0)->GetRootNode() == nullptr);
                GetWorld(0)->SetRootNode(mNodes[i].Get());
            }
        }
    }
}

void ActionSpawnNodes::Reverse()
{
    Action::Reverse();

    if (mParents.size() == 0)
    {
        // First time reversing, track the parents and parent scene-links.
        for (uint32_t i = 0; i < mNodes.size(); ++i)
        {
            Node* parent = mNodes[i]->GetParent();
            mParents.push_back(ResolvePtr(parent));
        }
    }

    for (uint32_t i = 0; i < mNodes.size(); ++i)
    {
        if (mParents[i] != nullptr)
        {
            mNodes[i]->Detach();
        }
        else
        {
            OCT_ASSERT(mNodes.size() == 1);
            OCT_ASSERT(mNodes[i] == GetWorld(0)->GetRootNode());
            GetWorld(0)->SetRootNode(nullptr);
        }

        ActionManager::Get()->ExileNode(mNodes[i]);
    }
}

ActionDeleteNodes::ActionDeleteNodes(const std::vector<Node*>& nodes)
{
    std::vector<Node*> trimmedNodes = nodes;
    RemoveRedundantDescendants(trimmedNodes);

    mNodes.resize(trimmedNodes.size());
    for (uint32_t i = 0; i < mNodes.size(); ++i)
    {
        mNodes[i] = ResolvePtr(nodes[i]);
    }

    for (uint32_t i = 0; i < mNodes.size(); ++i)
    {
        mParents.push_back(ResolvePtr(mNodes[i]->GetParent()));
        if (mParents[i] != nullptr)
        {
            int32_t childIdx = mParents[i]->FindChildIndex(mNodes[i].Get());
            OCT_ASSERT(childIdx != -1);
            mChildIndices.push_back(childIdx);

            Node3D* node3d = mNodes[i]->As<Node3D>();
            if (node3d != nullptr)
            {
                mBoneIndices.push_back(node3d->GetParentBoneIndex());
            }
            else
            {
                mBoneIndices.push_back(-1);
            }
        }
        else
        {
            // This must be the root node being deleted.
            OCT_ASSERT(mNodes.size() == 1);

            mChildIndices.push_back(-1);
            mBoneIndices.push_back(-1);
        }
    }
}

void ActionDeleteNodes::Execute()
{
    Action::Execute();

    for (uint32_t i = 0; i < mNodes.size(); ++i)
    {
        // Actor is already spawned at this point.
        OCT_ASSERT(mNodes[i]->GetWorld() != nullptr);

        if (IsPlayingInEditor())
        {
            mNodes[i]->Destroy();
        }
        else
        {
            if (mParents[i] != nullptr)
            {
                mNodes[i]->Detach();
            }
            else
            {
                // We must be deleting the root node
                OCT_ASSERT(mNodes.size() == 1);
                OCT_ASSERT(mNodes[i] == GetWorld(0)->GetRootNode());
                GetWorld(0)->SetRootNode(nullptr);
            }

            ActionManager::Get()->ExileNode(mNodes[i]);
        }
    }
}

void ActionDeleteNodes::Reverse()
{
    Action::Reverse();

    for (uint32_t i = 0; i < mNodes.size(); ++i)
    {
        ActionManager::Get()->RestoreExiledNode(mNodes[i]);

        if (mParents[i] != nullptr)
        {
            if (mBoneIndices[i] >= 0 &&
                mParents[i]->As<SkeletalMesh3D>() &&
                mNodes[i]->As<Node3D>())
            {
                // This node was attached to a bone!
                mNodes[i]->As<Node3D>()->AttachToBone(
                    mParents[i]->As<SkeletalMesh3D>(),
                    mBoneIndices[i], 
                    false, 
                    mChildIndices[i]);
            }
            else
            {
                // Normal attachment
                mParents[i]->AddChild(mNodes[i].Get(), mChildIndices[i]);
            }
            // TODO: Support attaching to the correct bone. 
            // Probably need to add extra parameter to Attach() to include child index.
        }
        else
        {
            // Must have deleted the root node.
            OCT_ASSERT(mNodes.size() == 1);
            OCT_ASSERT(GetWorld(0)->GetRootNode() == nullptr);
            GetWorld(0)->SetRootNode(mNodes[i].Get());
        }
    }
}

ActionAttachNode::ActionAttachNode(Node* node, Node* newParent, int32_t childIndex, int32_t boneIndex)
{
    mNode = ResolvePtr(node);
    mNewParent = ResolvePtr(newParent);
    mPrevParent = ResolvePtr(node->GetParent());
    mChildIndex = childIndex;
    mPrevChildIndex = node->GetParent() ? node->GetParent()->FindChildIndex(node) : -1;
    mBoneIndex = boneIndex;
    mPrevBoneIndex = node->IsNode3D() ? node->As<Node3D>()->GetParentBoneIndex() : -1;
    OCT_ASSERT(mNode);
    OCT_ASSERT(mNewParent);
}

void ActionAttachNode::Execute()
{
    Action::Execute();

    if (mBoneIndex >= 0 &&
        mNewParent != nullptr &&
        mNewParent->As<SkeletalMesh3D>() &&
        mNode->As<Node3D>())
    {
        Node3D* node3d = mNode->As<Node3D>();
        SkeletalMesh3D* skParent = mNewParent->As<SkeletalMesh3D>();

        node3d->AttachToBone(skParent, mBoneIndex, true, mChildIndex);
    }
    else
    {
       mNode->Attach(mNewParent.Get(), true, mChildIndex);
    }
}

void ActionAttachNode::Reverse()
{
    Action::Reverse();

    if (mPrevBoneIndex >= 0 &&
        mPrevParent != nullptr &&
        mPrevParent->As<SkeletalMesh3D>() &&
        mNode->As<Node3D>())
    {
        Node3D* node3d = mNode->As<Node3D>();
        SkeletalMesh3D* skParent = mPrevParent->As<SkeletalMesh3D>();
        node3d->AttachToBone(skParent, mPrevBoneIndex, true, mPrevChildIndex);
    }
    else
    {
        mNode->Attach(mPrevParent.Get(), true, mPrevChildIndex);
    }
}

ActionSetRootNode::ActionSetRootNode(Node* newRoot)
{
    mNewRoot = ResolvePtr(newRoot);
    mOldRoot = ResolvePtr(GetWorld(0)->GetRootNode());
    mNewRootParent = ResolvePtr(mNewRoot->GetParent());
    mNewRootChildIndex = mNewRootParent ? mNewRootParent->FindChildIndex(mNewRoot.Get()) : -1;

    OCT_ASSERT(mNewRoot != mOldRoot);
    OCT_ASSERT(mNewRoot != nullptr);
    OCT_ASSERT(mOldRoot != nullptr);
    OCT_ASSERT(mNewRootParent != nullptr && mNewRootChildIndex != -1);
}

void ActionSetRootNode::Execute()
{
    Action::Execute();

    mNewRoot->Detach(true);
    GetWorld(0)->SetRootNode(mNewRoot.Get());
    mOldRoot->Attach(mNewRoot.Get(), true);
    mOldRoot->SetScene(nullptr);
}

void ActionSetRootNode::Reverse()
{
    Action::Reverse();

    mOldRoot->Detach(true);
    GetWorld(0)->SetRootNode(mOldRoot.Get());
    mNewRoot->Attach(mNewRootParent.Get(), true, mNewRootChildIndex);
    mNewRoot->SetScene(nullptr);
}

ActionSetWorldRotation::ActionSetWorldRotation(Node3D* node, glm::quat rot)
{
    mNode = node;
    mNewRotation = rot;
    mPrevRotation = node->GetWorldRotationQuat();
    OCT_ASSERT(mNode);
}

void ActionSetWorldRotation::Execute()
{
    Action::Execute();

    mNode->SetWorldRotation(mNewRotation);
}

void ActionSetWorldRotation::Reverse()
{
    Action::Reverse();

    mNode->SetWorldRotation(mPrevRotation);
}

ActionSetWorldPosition::ActionSetWorldPosition(Node3D* node, glm::vec3 pos)
{
    mNode = node;
    mNewPosition = pos;
    mPrevPosition = node->GetWorldPosition();
    OCT_ASSERT(mNode);
}

void ActionSetWorldPosition::Execute()
{
    Action::Execute();

    mNode->SetWorldPosition(mNewPosition);
}

void ActionSetWorldPosition::Reverse()
{
    Action::Reverse();

    mNode->SetWorldPosition(mPrevPosition);
}

ActionSetWorldScale::ActionSetWorldScale(Node3D* node, glm::vec3 scale)
{
    mNode = node;
    mNewScale = scale;
    mPrevScale = node->GetWorldScale();
    OCT_ASSERT(mNode);
}

void ActionSetWorldScale::Execute()
{
    Action::Execute();

    mNode->SetWorldScale(mNewScale);
}

void ActionSetWorldScale::Reverse()
{
    Action::Reverse();

    mNode->SetWorldScale(mPrevScale);
}

ActionUnlinkScene::ActionUnlinkScene(Node* node)
{
    mNode = node;
    mScene = node->GetScene();
    OCT_ASSERT(mNode);
}

void ActionUnlinkScene::Execute()
{
    Action::Execute();

    mNode->BreakSceneLink();
}

void ActionUnlinkScene::Reverse()
{
    Action::Reverse();

    mNode->SetScene(mScene.Get<Scene>());
}

ActionSetInstanceColors::ActionSetInstanceColors(const std::vector<ActionSetInstanceColorsData>& data)
{
    // Don't call this action with no color data to change.
    OCT_ASSERT(data.size() > 0);

    mData = data;

    mPrevData.resize(mData.size());
    for (uint32_t i = 0; i < data.size(); ++i)
    {
        mPrevData[i].mMesh3d = mData[i].mMesh3d;
        mPrevData[i].mColors = mData[i].mMesh3d->GetInstanceColors();
        mPrevData[i].mBakedLight = mData[i].mMesh3d->HasBakedLighting();
    }
}

void ActionSetInstanceColors::Execute()
{
    Action::Execute();

    for (uint32_t i = 0; i < mData.size(); ++i)
    {
        mData[i].mMesh3d->SetInstanceColors(mData[i].mColors, mData[i].mBakedLight);
    }
}

void ActionSetInstanceColors::Reverse()
{
    Action::Reverse();

    for (uint32_t i = 0; i < mPrevData.size(); ++i)
    {
        mPrevData[i].mMesh3d->SetInstanceColors(mPrevData[i].mColors, mPrevData[i].mBakedLight);
    }
}

ActionSetInstanceData::ActionSetInstanceData(InstancedMesh3D* instMesh, int32_t startIndex, const std::vector<MeshInstanceData>& data)
{
    mInstMesh = instMesh;
    mData = data;
    mStartIndex = startIndex;
    mData = data;

    if (mStartIndex < 0)
    {
        // Negative start index means set entire array of instance data
        mPrevData = mInstMesh->GetInstanceData();
    }
    else
    {
        // We are only setting a subset of the data
        for (int32_t i = mStartIndex; (i < (int32_t)mInstMesh->GetNumInstances()) && (i < mStartIndex + data.size()); ++i)
        {
            mPrevData.push_back(mInstMesh->GetInstanceData(i));
        }
    }
}

void ActionSetInstanceData::Execute()
{
    Action::Execute();

    if (mStartIndex < 0)
    {
        mInstMesh->SetInstanceData(mData);
    }
    else
    {
        // Remove X num of instance data
        int32_t removeEnd = int32_t(mStartIndex + mData.size());
        removeEnd = glm::min<uint32_t>(removeEnd, (int32_t)mInstMesh->GetNumInstances());
        for (int32_t i = removeEnd - 1; i >= mStartIndex; --i)
        {
            mInstMesh->RemoveInstanceData(i);
        }

        // Add X num of new instance data
        for (int32_t i = 0; i < mData.size(); ++i)
        {
            mInstMesh->AddInstanceData(mData[i], mStartIndex + i);
        }
    }
}

void ActionSetInstanceData::Reverse()
{
    Action::Reverse();

    if (mStartIndex < 0)
    {
        mInstMesh->SetInstanceData(mPrevData);
    }
    else
    {
        // Remove X num of instance data
        int32_t removeEnd = int32_t(mStartIndex + mPrevData.size());
        removeEnd = glm::min<uint32_t>(removeEnd, (int32_t)mInstMesh->GetNumInstances());
        for (int32_t i = removeEnd - 1; i >= mStartIndex; --i)
        {
            mInstMesh->RemoveInstanceData(i);
        }

        // Add X num of new instance data
        for (int32_t i = 0; i < mPrevData.size(); ++i)
        {
            mInstMesh->AddInstanceData(mPrevData[i], mStartIndex + i);
        }
    }
}

#endif
