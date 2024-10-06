#include "AssetManager.h"
#include "Asset.h"
#include "AssetDir.h"
#include "Engine.h"
#include "Stream.h"
#include "Log.h"
#include "Constants.h"
#include "Utilities.h"
#include "EmbeddedFile.h"
#include "Renderer.h"

#include "Assets/Scene.h"
#include "Assets/Texture.h"
#include "Assets/StaticMesh.h"
#include "Assets/SkeletalMesh.h"
#include "Assets/ParticleSystem.h"
#include "Assets/ParticleSystemInstance.h"
#include "Assets/SoundWave.h"
#include "Assets/Material.h"
#include "Assets/MaterialLite.h"
#include "Assets/MaterialInstance.h"
#include "Assets/Font.h"

#include "System/System.h"

#include <string>
#include <functional>

#if EDITOR
#include "Editor/EditorState.h"
#endif

#define ASYNC_REQUEUE_LIMIT 30

AssetManager* AssetManager::sInstance = nullptr;

Asset* FetchAsset(const std::string& name)
{
    return AssetManager::Get()->GetAsset(name);
}

Asset* LoadAsset(const std::string& name)
{
	return AssetManager::Get()->LoadAsset(name);
}

void UnloadAsset(const std::string& name)
{
    AssetManager::Get()->UnloadAsset(name);
}

void AsyncLoadAsset(const std::string& name, AssetRef* targetRef)
{
    AssetManager::Get()->AsyncLoadAsset(name, targetRef);
}

AssetStub* FetchAssetStub(const std::string& name)
{
    return AssetManager::Get()->GetAssetStub(name);
}

void AssetManager::Create()
{
    Destroy();
    sInstance = new AssetManager();
}

void AssetManager::Destroy()
{
    if (sInstance != nullptr)
    {
        delete sInstance;
        sInstance = nullptr;
    }
}

AssetManager* AssetManager::Get()
{
    return sInstance;
}

AssetManager::AssetManager()
{
#if EDITOR
    InitAssetColorMap();
#endif
}

AssetManager::~AssetManager()
{
    Purge(true);

    SYS_LockMutex(mMutex);
    // Flag that we are destructing so that the async load thread can exit.
    mDestructing = true;
    SYS_UnlockMutex(mMutex);

    SYS_JoinThread(mAsyncLoadThread);
    SYS_DestroyThread(mAsyncLoadThread);
    mAsyncLoadThread = nullptr;

    SYS_DestroyMutex(mMutex);
    mMutex = nullptr;
}

void AssetManager::Initialize()
{
    mRootDirectory = new AssetDir("Root", "", nullptr);

    mMutex = SYS_CreateMutex();
    mAsyncLoadThread = SYS_CreateThread(AsyncLoadThreadFunc, this);
}

void AssetManager::Update(float deltaTime)
{
    UpdateEndLoadQueue();
}

AssetStub* AssetManager::RegisterAsset(const std::string& filename, TypeId type, AssetDir* directory, EmbeddedFile* embeddedAsset, bool engineAsset)
{
    std::string fixedFilename = filename;

    if (filename.size() < 4 ||
        filename.substr(filename.size() - 4, 4) != ".oct")
    {
        fixedFilename += ".oct";
    }

    std::string name = Asset::GetNameFromPath(fixedFilename);
    std::string path = directory ? directory->mPath + fixedFilename : "";

#if EDITOR || _DEBUG
    auto foundAsset = mAssetMap.find(name);
    if (foundAsset != mAssetMap.end())
    {
        // Asset with same name found. Asset names must be unique for now :[
        // However, if the path is the same, then we may be rediscovering an asset.
        if (foundAsset->second->mPath != path &&
            foundAsset->second->mEmbeddedData == embeddedAsset)
        {
            LogDebug("Asset name conflict: %s", name.c_str());
            OCT_ASSERT(false);
        }
        else
        {
            LogDebug("Register asset skipped. Asset already exists: %s", name.c_str());
        }

        return nullptr;
    }
#endif

    AssetStub* stub = new AssetStub();
#if EDITOR
    stub->mName = name;
    stub->mDirectory = directory;
#endif
    stub->mAsset = nullptr;
    stub->mType = type;
    stub->mEmbeddedData = embeddedAsset;
    stub->mEngineAsset = directory ? directory->mEngineDir : false;
    stub->mPath = path;

    mAssetMap.insert(std::pair<std::string, AssetStub*>(name, stub));

    if (directory != nullptr)
    {
        directory->mAssetStubs.push_back(stub);
    }

    return stub;
}

AssetStub* AssetManager::CreateAndRegisterAsset(TypeId type, AssetDir* directory, const std::string& filename, bool engineAsset)
{
    Asset* newAsset = Asset::CreateInstance(type);
    newAsset->Create();
    newAsset->SetEngineAsset(engineAsset);

    AssetStub* stub = RegisterAsset(filename, type, directory, nullptr, engineAsset);
    
    if (stub != nullptr)
    {
        stub->mAsset = newAsset;
#if EDITOR
        newAsset->SetName(stub->mName);
#else
        newAsset->SetName(Asset::GetNameFromPath(filename));
#endif
    }
    else
    {
        delete newAsset;
        newAsset = nullptr;
    }

    return stub;
}

AssetDir* AssetManager::GetAssetDirFromPath(const std::string& dirPath)
{
    // Path should be in format like:
    // Levels/Zone1/Cells/
    AssetDir* retDir = AssetManager::FindProjectDirectory();
    if (retDir == nullptr)
    {
        LogError("Couldn't find project directory in GetAssetDirFromPath()");
        return nullptr;
    }

    if (dirPath.size() == 0)
    {
        LogError("Empty path received in GetAssetDirFromPath()");
        return nullptr;
    }

    // Make a mutable copy of the dir path
    std::string path = dirPath;

    if (path.front() == '/')
    {
        path.erase(0, 1);
    }

    if (path.back() != '/')
    {
        path.push_back('/');
    }

    std::string delimiter = "/";

    size_t pos = 0;
    std::string token;
    while (retDir != nullptr &&
        (pos = path.find(delimiter)) != std::string::npos)
    {
        token = path.substr(0, pos);

        AssetDir* subDir = nullptr;
        for (uint32_t i = 0; i < retDir->mChildDirs.size(); ++i)
        {
            if (retDir->mChildDirs[i]->mName == token)
            {
                subDir = retDir->mChildDirs[i];
                break;
            }
        }

        retDir = subDir;
        path.erase(0, pos + delimiter.length());
    }

    return retDir;
}

bool AssetManager::IsPurging() const
{
    return mPurging;
}

void AssetManager::Discover(const char* directoryName, const char* directoryPath)
{
    SCOPED_STAT("DiscoverAssets")

    // Make sure directory path ends with 
    std::string dirPath = directoryPath;
    if (dirPath.size() > 0 && dirPath[dirPath.size() - 1] != '/')
    {
        dirPath += '/';
    }

    AssetDir* newDir = new AssetDir(directoryName, dirPath, mRootDirectory);
    bool isEngineDir = (strcmp(directoryName, "Engine") == 0);
    newDir->mEngineDir = isEngineDir;

    // Recursively iterate through Asset directory and find any .oct asset
    // and register an Asset to the map. At this point, we also want to read the oct 
    // header and determine the asset type so we can instantiate the correct Asset derived class.

    std::function<void(AssetDir*, bool)> searchDirectory = [&](AssetDir* directory, bool engineDir)
    {

        std::vector<std::string> subDirectories;
        DirEntry dirEntry = { };

        SYS_OpenDirectory(directory->mPath, dirEntry);

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
                // TODO: Read the asset header to check the asset type.
                const char* extension = strrchr(dirEntry.mFilename, '.');

                if (extension != nullptr &&
                    strcmp(extension, ".oct") == 0)
                {
                    Stream stream;
                    std::string path = directory->mPath + dirEntry.mFilename;
                    stream.ReadFile(path.c_str(), true, sizeof(AssetHeader));

                    AssetHeader header = Asset::ReadHeader(stream);
                    RegisterAsset(dirEntry.mFilename, header.mType, directory, nullptr, engineDir);
                }
            }

            SYS_IterateDirectory(dirEntry);
        }

        SYS_CloseDirectory(dirEntry);

        // Discover assets of subdirectories.
        for (uint32_t i = 0; i < subDirectories.size(); ++i)
        {
            std::string dirPath = directory->mPath + subDirectories[i] + "/";
            AssetDir* subDir = new AssetDir(subDirectories[i], dirPath, directory);
            searchDirectory(subDir, engineDir);
        }
    };

    searchDirectory(newDir, isEngineDir);
}

void AssetManager::DiscoverAssetRegistry(const char* registryPath)
{
    SCOPED_STAT("DiscoverAssetRegistry");
    // For each line, attempt to open that file.
    // If file is found, read asset header and call RegisterAsset() from header.
    Stream regStream;
    regStream.ReadFile(registryPath, true);

    if (regStream.GetData() != nullptr)
    {
        constexpr uint32_t kLineSize = MAX_PATH_SIZE + 42;
        char line[kLineSize];
        char typeString[32];
        char filename[MAX_PATH_SIZE];

        std::string lineStr;
        while (true)
        {
            lineStr = regStream.GetLine();
            if (lineStr == "")
            {
                break;
            }

            strncpy(line, lineStr.c_str(), kLineSize);

            char* comma = strchr(line, ',');
            OCT_ASSERT(comma != nullptr);
            *comma = '\0';

            strncpy(typeString, line, 31);
            strncpy(filename, comma + 1, MAX_PATH_SIZE - 1);

            // Replace newline with null terminator
            filename[strcspn(filename, "\r\n")] = 0;

            TypeId assetType = Asset::GetTypeIdFromName(typeString);
            RegisterAsset(filename, assetType, mRootDirectory, nullptr, false);
        }
    }
    else
    {
        LogError("Asset Registry file not found");
    }
}

void AssetManager::DiscoverEmbeddedAssets(EmbeddedFile* assets, uint32_t numAssets)
{
    SCOPED_STAT("DiscoverEmbeddedAssets");
    if (numAssets > 0)
    {
        for (uint32_t i = 0; i < numAssets; ++i)
        {
            EmbeddedFile* embeddedAsset = &assets[i];

            Stream stream(embeddedAsset->mData, sizeof(AssetHeader));

            AssetHeader header = Asset::ReadHeader(stream);
            AssetStub* stub = GetAssetStub(embeddedAsset->mName);

            if (stub == nullptr)
            {
                stub = RegisterAsset(embeddedAsset->mName, header.mType, nullptr, embeddedAsset, embeddedAsset->mEngine);
            }
        }
    }
}

void AssetManager::Purge(bool purgeEngineAssets)
{
    // Destroy all assets in the map and empty the map.
    // Caller needs to ensure that no assets are being referenced.

    mPurging = true;

    if (purgeEngineAssets)
    {
        for (auto it = mAssetMap.begin(); it != mAssetMap.end(); ++it)
        {
            UnloadAsset(*it->second);
            delete it->second;
        }

        mAssetMap.clear();
    }
    else
    {
        std::vector<std::string> assetsToPurge;

        for (auto it = mAssetMap.begin(); it != mAssetMap.end(); ++it)
        {
            if (!it->second->mEngineAsset)
            {
                assetsToPurge.push_back(it->first);
            }
        }

        while (assetsToPurge.size() > 0)
        {
            PurgeAsset(assetsToPurge.back().c_str());
            assetsToPurge.pop_back();
        }
    }

    for (int32_t i = int32_t(mTransientAssets.size()) - 1; i >= 0; --i)
    {
        if (purgeEngineAssets || !mTransientAssets[i]->IsEngineAsset())
        {
            mTransientAssets[i]->Destroy();
            delete mTransientAssets[i];
        }
    }
    mTransientAssets.clear();

    mPurging = false;
}

bool AssetManager::PurgeAsset(const char* name)
{
    bool purged = false;

    for (auto it = mAssetMap.begin(); it != mAssetMap.end(); ++it)
    {
        if (it->first == name)
        {
            AssetStub* delStub = it->second;
            mAssetMap.erase(it);

#if EDITOR
            if (delStub->mDirectory != nullptr)
            {
                delStub->mDirectory->RemoveAssetStub(delStub);
            }

            if (GetEditorState()->GetSelectedAssetStub() == delStub)
            {
                GetEditorState()->SetSelectedAssetStub(nullptr);
            }

            if (GetEditorState()->GetInspectedObject() == delStub->mAsset)
            {
                GetEditorState()->InspectObject(nullptr, true);
            }

            GetEditorState()->RemoveFilteredAssetStub(delStub);
#endif

            if (delStub->mAsset != nullptr)
            {
                //delStub->mAsset->Destroy();
                //delete delStub->mAsset;

                UnloadAsset(*delStub);
            }

            delete delStub;
            delStub = nullptr;

            purged = true;
            break;
        }
    }

    return purged;
}

void AssetManager::RefSweep()
{
    // Iterate several times until no assets are unloaded.
    uint32_t iter = 0;
    uint32_t numAssetsUnloaded = 0;
    uint32_t totalAssetsUnloaded = 0;

    while (iter == 0 || (iter < 10 && numAssetsUnloaded != 0))
    {
        totalAssetsUnloaded += numAssetsUnloaded;
        numAssetsUnloaded = 0;

        for (auto it = mAssetMap.begin(); it != mAssetMap.end(); ++it)
        {
#if EDITOR
            // Don't ref sweep engine assets. They might not be saved as an OCT file yet.
            if (it->second->mEngineAsset)
                continue;
#endif

            if (it->second->mAsset != nullptr &&
                it->second->mAsset->IsLoaded() &&
                it->second->mAsset->GetRefCount() == 0)
            {
                it->second->mAsset->Destroy();
                delete it->second->mAsset;
                it->second->mAsset = nullptr;
                ++numAssetsUnloaded;
            }
        }

        for (int32_t i = int32_t(mTransientAssets.size()) - 1; i >= 0; --i)
        {
            if (mTransientAssets[i]->GetRefCount() == 0)
            {
                mTransientAssets[i]->Destroy();
                delete mTransientAssets[i];

                mTransientAssets.erase(mTransientAssets.begin() + i);
            }
        }

        iter++;
    }

    totalAssetsUnloaded += numAssetsUnloaded;
    LogDebug("%d assets swept", totalAssetsUnloaded);
}

void AssetManager::LoadAll()
{
    for (auto it = mAssetMap.begin(); it != mAssetMap.end(); ++it)
    {
        AssetStub* stub = it->second;
        if (stub->mAsset == nullptr)
        {
            stub->mAsset = Asset::CreateInstance(stub->mType);
            stub->mAsset->LoadFile(stub->mPath.c_str());
        }
    }
}

void AssetManager::RegisterTransientAsset(Asset* asset)
{
    mTransientAssets.push_back(asset);
    asset->SetTransient(true);
}

Asset* AssetManager::ImportEngineAsset(TypeId type, AssetDir* dir, const std::string& filename, ImportOptions* options)
{
    Asset* newAsset = Asset::CreateInstance(type);
    std::string importPath = dir->mPath + filename + newAsset->GetTypeImportExt();
    newAsset->Import(importPath, options);
    newAsset->SetName(filename);
    newAsset->SetEngineAsset(true);
    AssetStub* stub = RegisterAsset(filename + ".oct", newAsset->GetType(), dir, nullptr, true);

    if (stub == nullptr)
    {
        delete newAsset;
        newAsset = nullptr;
    }
    else
    {
        stub->mAsset = newAsset;

        if (type == StaticMesh::GetStaticType())
        {
            bool noTriangleCollision = (filename == "SM_Cube") || (filename == "SM_Sphere");

            if (noTriangleCollision)
            {
                static_cast<StaticMesh*>(newAsset)->SetGenerateTriangleCollisionMesh(false);
            }
        }

        // Was saving right on startup but in Debug config this is very slow for some reason.
        // Moving the call to SaveAsset into the Package code.
        //AssetManager::Get()->SaveAsset(*stub);
    }

    return newAsset;
}

void AssetManager::ImportEngineAssets()
{
    static bool imported = false;
    OCT_ASSERT(!imported);

    if (!imported)
    {
        AssetDir* engineDir = FindEngineDirectory();
        if (engineDir == nullptr)
        {
            engineDir = new AssetDir("Engine", "Engine/Assets/", mRootDirectory);
            engineDir->mEngineDir = true;
        }

        AssetDir* engineTextures = engineDir->CreateSubdirectory("Textures");
        AssetDir* engineMaterials = engineDir->CreateSubdirectory("Materials");
        AssetDir* engineMeshes = engineDir->CreateSubdirectory("Meshes");
        AssetDir* engineParticles = engineDir->CreateSubdirectory("Particles");
        AssetDir* engineFonts = engineDir->CreateSubdirectory("Fonts");

        ImportEngineAsset(Texture::GetStaticType(), engineTextures, "T_White");
        ImportEngineAsset(Texture::GetStaticType(), engineTextures, "T_Black");
        ImportEngineAsset(Texture::GetStaticType(), engineTextures, "T_DefaultColor");
        ImportEngineAsset(Texture::GetStaticType(), engineTextures, "T_DefaultColorAlpha");
        ImportEngineAsset(Texture::GetStaticType(), engineTextures, "T_DefaultNormal");
        ImportEngineAsset(Texture::GetStaticType(), engineTextures, "T_DefaultORM");
        ImportEngineAsset(Texture::GetStaticType(), engineTextures, "T_Radial");
        ImportEngineAsset(Texture::GetStaticType(), engineTextures, "T_Ring");
        ImportEngineAsset(Texture::GetStaticType(), engineTextures, "T_Circle");
        ImportEngineAsset(Texture::GetStaticType(), engineTextures, "T_TriShape");
        ImportEngineAsset(Texture::GetStaticType(), engineTextures, "T_MiniArrow");

        ImportOptions fontOptions;
        // Do we want to disable mip maps? Mipmaps can cause artifacts on edges of characters
        // But if we disable mipmaps, then large fonts look coarse and aliased at smaller sizes.
        //fontOptions.SetOptionValue("mipmapped", false);
        Texture* roboto16Tex = (Texture*) ImportEngineAsset(Texture::GetStaticType(), engineTextures, "T_Roboto16", &fontOptions);
        roboto16Tex->SetMipmapped(false);
        Texture* roboto32Tex = (Texture*) ImportEngineAsset(Texture::GetStaticType(), engineTextures, "T_Roboto32", &fontOptions);
        roboto32Tex->SetMipmapped(false);
        Texture* robotoMono8Tex = (Texture*) ImportEngineAsset(Texture::GetStaticType(), engineTextures, "T_RobotoMono8", &fontOptions);
        robotoMono8Tex->SetMipmapped(false);
        Texture* robotoMono16Tex = (Texture*) ImportEngineAsset(Texture::GetStaticType(), engineTextures, "T_RobotoMono16", &fontOptions);
        robotoMono16Tex->SetMipmapped(false);

        // Need to fetch these loaded textures so that the default materials can use them.
        Renderer::Get()->LoadDefaultTextures();

        // Create materials (not actually imported from any source files)
        CreateAndRegisterAsset(MaterialLite::GetStaticType(), engineMaterials, "M_Default", true);
        AssetStub* defaultUnlitStub = CreateAndRegisterAsset(MaterialLite::GetStaticType(), engineMaterials, "M_DefaultUnlit", true);
        MaterialLite* defaultUnlitMaterial = (MaterialLite*)defaultUnlitStub->mAsset;
        defaultUnlitMaterial->SetShadingModel(ShadingModel::Unlit);

        AssetStub* paintMatStub = CreateAndRegisterAsset(MaterialLite::GetStaticType(), engineMaterials, "M_PaintSphere", true);
        MaterialLite* paintMat = (MaterialLite*)paintMatStub->mAsset;
        paintMat->SetShadingModel(ShadingModel::Unlit);
        paintMat->SetBlendMode(BlendMode::Additive);
        paintMat->SetColor({ 0.0f, 0.0f, 0.0f, 1.0f });
        paintMat->SetFresnelEnabled(true);
        paintMat->SetFresnelColor(glm::vec4(1.0f, 0.27f, 0.12f, 1.0f));
        paintMat->SetFresnelPower(3.0f);
        paintMat->SetApplyFog(false);

        Renderer::Get()->LoadDefaultMaterials();

        ImportEngineAsset(StaticMesh::GetStaticType(), engineMeshes, "SM_Cone");
        ImportEngineAsset(StaticMesh::GetStaticType(), engineMeshes, "SM_Cube");
        ImportEngineAsset(StaticMesh::GetStaticType(), engineMeshes, "SM_Cylinder");
        ImportEngineAsset(StaticMesh::GetStaticType(), engineMeshes, "SM_Plane");
        ImportEngineAsset(StaticMesh::GetStaticType(), engineMeshes, "SM_Sphere");
        ImportEngineAsset(StaticMesh::GetStaticType(), engineMeshes, "SM_Sphere_112");
        ImportEngineAsset(StaticMesh::GetStaticType(), engineMeshes, "SM_Torus");
        ImportEngineAsset(StaticMesh::GetStaticType(), engineMeshes, "SM_CapsuleCylinder");
        ImportEngineAsset(StaticMesh::GetStaticType(), engineMeshes, "SM_CapsuleCap");
        ImportEngineAsset(SkeletalMesh::GetStaticType(), engineMeshes, "SK_EighthNote");
        Renderer::Get()->LoadDefaultMeshes();

        // Create a default particle system
        AssetStub* defParticleStub = CreateAndRegisterAsset(ParticleSystem::GetStaticType(), engineParticles, "P_DefaultParticle", true);
        ParticleSystem* defaultParticle = (ParticleSystem*)defParticleStub->mAsset;
        defaultParticle->GetParams().mPositionMin = { -0.1f, -0.1f, -0.1f };
        defaultParticle->GetParams().mPositionMax = { 0.1f, 0.1f, 0.1f };
        defaultParticle->GetParams().mVelocityMin = { 3.0f, 0.0f, 0.0f };
        defaultParticle->GetParams().mVelocityMax = { 5.0f, 0.0f, 0.0f };
        defaultParticle->GetParams().mLifetimeMin = 0.3f;
        defaultParticle->GetParams().mLifetimeMax = 0.5f;
        defaultParticle->GetParams().mAcceleration = { 0.0f, 0.0f, 0.0f };
        defaultParticle->GetParams().mAlphaEase = 0.1f;
        defaultParticle->GetParams().mSizeMin = { 0.5f, 0.5f };
        defaultParticle->GetParams().mSizeMax = { 0.5f, 0.5f };
        defaultParticle->SetRadialVelocity(true);
        defaultParticle->SetSpawnRate(100.0f);
        defaultParticle->SetMaterial(defaultUnlitMaterial);

        // Import fonts
        Font* fontRoboto32 = (Font*) ImportEngineAsset(Font::GetStaticType(), engineFonts, "F_Roboto32");
        Font* fontRobotoMono16 = (Font*)ImportEngineAsset(Font::GetStaticType(), engineFonts, "F_RobotoMono16");

        // Mark any transient assets that exist at this point as engine assets
        for (uint32_t i = 0; i < mTransientAssets.size(); ++i)
        {
            mTransientAssets[i]->SetEngineAsset(true);
        }

        imported = true;
    }
}

AssetStub* AssetManager::GetAssetStub(const std::string& name)
{
    AssetStub* stub = nullptr;
    auto itr = mAssetMap.find(name);
    stub = (itr != mAssetMap.end()) ? itr->second : nullptr;
    return stub;
}

Asset* AssetManager::GetAsset(const std::string& name)
{
    Asset* asset = nullptr;
    auto itr = mAssetMap.find(name);
    asset = (itr != mAssetMap.end()) ? itr->second->mAsset : nullptr;
    return asset;
}

Asset* AssetManager::LoadAsset(const std::string& name)
{
    Asset* retAsset = nullptr;
    auto it = mAssetMap.find(name);

    if (it != mAssetMap.end())
    {
        retAsset = LoadAsset(*it->second);
    }

    return retAsset;
}

Asset* AssetManager::LoadAsset(AssetStub& stub)
{
    if (stub.mAsset == nullptr)
    {
        stub.mAsset = Asset::CreateInstance(stub.mType);

        if (stub.mEmbeddedData != nullptr)
        {
            stub.mAsset->LoadEmbedded(stub.mEmbeddedData);
        }
        else
        {
            stub.mAsset->LoadFile(stub.mPath.c_str());
        }
    }

    return stub.mAsset;
}

void AssetManager::AsyncLoadAsset(const std::string& name, AssetRef* targetRef)
{
    SCOPED_LOCK(mMutex);
    // (1) Check to see if an asset stub exists at all, if not, then log an error and return.
    AssetStub* stub = GetAssetStub(name);
    if (stub == nullptr)
    {
        LogError("AsyncLoadAsset failed, asset %s does not exist in map", name.c_str());
        return;
    }

    // Erase ref from current pending request
    if (targetRef != nullptr &&
        targetRef->mLoadRequest != nullptr)
    {
        EraseAsyncLoadRef(*targetRef);
    }

    // (2) Check to see if the asset is already loaded. If so, assign the target ref immediately.
    if (stub->mAsset != nullptr)
    {
        if (targetRef != nullptr)
        {
            *targetRef = stub->mAsset;
        }

        return;
    }

    // (3) Check to see if an AsyncLoadRequest is already in flight (in begin/end queues) and if so, add this ref to the list.
    for (uint32_t i = 0; i < mBeginLoadQueue.size(); ++i)
    {
        if (mBeginLoadQueue[i]->mName == name)
        {
            mBeginLoadQueue[i]->mTargetRefs.push_back(targetRef);
            return;
        }
    }

    for (uint32_t i = 0; i < mEndLoadQueue.size(); ++i)
    {
        if (mEndLoadQueue[i]->mName == name)
        {
            mEndLoadQueue[i]->mTargetRefs.push_back(targetRef);
            return;
        }
    }
    
    // (4) Otherwise, malloc and enqueue a new AsyncLoadRequest to the BeginLoadQueue
    AsyncLoadRequest* newRequest = new AsyncLoadRequest();
    mBeginLoadQueue.push_back(newRequest);

    // (5) Set the data on the request, including the targetRef.
    newRequest->mName = name;
    newRequest->mPath = stub->mPath;
    newRequest->mType = stub->mType;
    newRequest->mEmbeddedData = stub->mEmbeddedData;

    if (targetRef != nullptr)
    {
        newRequest->mTargetRefs.push_back(targetRef);

        // (6) Set the request pointer on the AssetRef.
        targetRef->mLoadRequest = newRequest;
    }
}

void AssetManager::SaveAsset(const std::string& name)
{
    auto it = mAssetMap.find(name);

    if (it != mAssetMap.end())
    {
        SaveAsset(*it->second);
    }
}

void AssetManager::SaveAsset(AssetStub& stub)
{
#if EDITOR
    // Don't attempt to save an unloaded asset.
    if (stub.mAsset != nullptr)
    {
        // If the Asset's mName doesn't match its Path's file name,
        // then that means the file has been renamed. So we need to
        // delete the old file and update mPath before saving.
        std::string filename = Asset::GetNameFromPath(stub.mPath);
        if (stub.mAsset->GetName() != filename)
        {
            remove(stub.mPath.c_str());

            std::string newPath = stub.mDirectory ? stub.mDirectory->mPath : Asset::GetDirectoryFromPath(stub.mPath);
            newPath += stub.mAsset->GetName();
            newPath += ".oct";

            stub.mPath = newPath;
        }

        if (stub.mDirectory && !DoesDirExist(stub.mDirectory->mPath.c_str()))
        {
            CreateDir(stub.mDirectory->mPath.c_str());
        }

        stub.mAsset->SaveFile(stub.mPath.c_str(), Platform::Count);
    }
    else
    {
        LogWarning("Asset is not loaded, so SaveAsset() will do nothing.");
    }
#endif
}

bool AssetManager::UnloadAsset(const std::string& name)
{
    bool unloaded = true;
    auto it = mAssetMap.find(name);

    if (it != mAssetMap.end())
    {
        unloaded = UnloadAsset(*it->second);
    }

    return unloaded;
}

bool AssetManager::UnloadAsset(AssetStub& stub)
{
    bool unloaded = false;

    if (stub.mAsset != nullptr)
    {
        if (!stub.mAsset->IsRefCounted() ||
            stub.mAsset->GetRefCount() == 0 ||
            IsPurging())
        {
            LogWarning("Unloading %s", stub.mAsset->GetName().c_str());

            stub.mAsset->Destroy();
            delete stub.mAsset;
            stub.mAsset = nullptr;
            unloaded = true;
        }
        else
        {
            LogWarning("Cannot unload asset %s, it still has %d refs", stub.mAsset->GetName().c_str(), stub.mAsset->GetRefCount());
        }
    }
    else
    {
        unloaded = true;
    }

    return unloaded;
}

void AssetManager::EraseAsyncLoadRef(AssetRef& assetRef)
{
    SCOPED_LOCK(mMutex);

    auto eraseRef = [](std::deque<AsyncLoadRequest*>& queue, AssetRef& ref)
    {
        for (uint32_t i = 0; i < queue.size(); ++i)
        {
            std::vector<AssetRef*>& refs = queue[i]->mTargetRefs;

            for (int32_t r = int32_t(refs.size()) - 1; r >= 0; --r)
            {
                if (refs[r] == &ref)
                {
                    refs.erase(refs.begin() + r);
                }
            }
        }
    };

    eraseRef(mBeginLoadQueue, assetRef);
    eraseRef(mEndLoadQueue, assetRef);

    assetRef.mLoadRequest = nullptr;
}

bool AssetManager::DoesAssetExist(const std::string& name)
{
	return mAssetMap.find(name) != mAssetMap.end();
}

bool AssetManager::RenameAsset(Asset* asset, const std::string& newName)
{
    bool success = false;
    OCT_ASSERT(asset != nullptr);
    OCT_ASSERT(newName.size() > 0);

    if (asset != nullptr &&
        newName.size() > 0)
    {
        auto oldItr = mAssetMap.find(asset->GetName());
        auto newItr = mAssetMap.find(newName);

        if (newItr != mAssetMap.end())
        {
            LogError("Failed to rename file. Another asset with the same name already exists.");
        }
        else if (oldItr->second->mAsset != asset)
        {
            LogError("Pointer mismatch when attempting to rename asset. Is there a duplicate asset?");
        }
        else
        {
            AssetStub* stub = mAssetMap[asset->GetName()];
            mAssetMap.erase(asset->GetName());

#if EDITOR
            stub->mName = newName;
#endif
            asset->SetName(newName);
            
            mAssetMap.insert(std::pair<std::string, AssetStub*>(newName, stub));

            // Do not adjust mPath, as the asset still refers to an old file.
            // When saving the asset, the old file will be destroyed first before saving to the new location.
            success = true;
        }
    }

    return success;
}

bool AssetManager::RenameDirectory(AssetDir* dir, const std::string& newName)
{
    bool success = false;

    if (newName != "" &&
        newName != dir->mName)
    {
        std::string oldPath = dir->mPath;

        if (oldPath.size() > 0 &&
            oldPath[oldPath.size() - 1] == '/')
        {
            oldPath = oldPath.substr(0, oldPath.size() - 1);
        }

        std::string newPath = oldPath;

        size_t slashLoc = oldPath.find_last_of('/');
        if (slashLoc != std::string::npos)
        {
            newPath = oldPath.substr(0, slashLoc + 1);
            newPath += newName;
        }

        if (SYS_Rename(oldPath.c_str(), newPath.c_str()))
        {
            dir->mPath = newPath;
            dir->mName = newName;
            success = true;
        }
    }

    return success;
}

AssetDir* AssetManager::FindProjectDirectory()
{
    AssetDir* retDir = nullptr;
    uint32_t numChildDirs = uint32_t(mRootDirectory ? mRootDirectory->mChildDirs.size() : 0);
    for (uint32_t i = 0; i < numChildDirs; ++i)
    {
        if (mRootDirectory->mChildDirs[i]->mName == GetEngineState()->mProjectName)
        {
            retDir = mRootDirectory->mChildDirs[i];
            break;
        }
    }

    return retDir;
}

AssetDir* AssetManager::FindEngineDirectory()
{
    AssetDir* retDir = nullptr;
    uint32_t numChildDirs = uint32_t(mRootDirectory ? mRootDirectory->mChildDirs.size() : 0);
    for (uint32_t i = 0; i < numChildDirs; ++i)
    {
        if (mRootDirectory->mChildDirs[i]->mName == "Engine")
        {
            retDir = mRootDirectory->mChildDirs[i];
            break;
        }
    }

    return retDir;
}

AssetDir* AssetManager::GetRootDirectory()
{
    return mRootDirectory;
}

void AssetManager::UnloadProjectDirectory()
{
    const std::string& projName = GetEngineState()->mProjectName;

    if (projName != "")
    {
        std::vector<AssetDir*>& dirs = mRootDirectory->mChildDirs;
        uint32_t numChildDirs = uint32_t(mRootDirectory ? dirs.size() : 0);
        for (uint32_t i = 0; i < numChildDirs; ++i)
        {
            if (dirs[i]->mName == projName)
            {
                delete dirs[i];
                dirs.erase(dirs.begin() + i);
                break;
            }
        }
    }
}

std::unordered_map<std::string, AssetStub*>& AssetManager::GetAssetMap()
{
    return mAssetMap;
}

ThreadFuncRet AssetManager::AsyncLoadThreadFunc(void* in)
{
    AssetManager& am = *((AssetManager*)in);
    bool exit = false;

    while (!exit)
    {
        AsyncLoadRequest* request = nullptr;

        // Pop off the next request from the queue.
        SYS_LockMutex(am.mMutex);
        bool exit = am.mDestructing;
        if (am.mBeginLoadQueue.size() > 0)
        {
            request = am.mBeginLoadQueue.front();
            am.mBeginLoadQueue.pop_front();
        }
        SYS_UnlockMutex(am.mMutex);

        if (exit)
        {
            break;
        }

        if (request != nullptr)
        {
            // We have a request, so we need to
            // (1) Create the Asset type
            Asset* newAsset = Asset::CreateInstance(request->mType);
            OCT_ASSERT(newAsset);

            // (2) Load the file into a stream
            // (3) Call asset->LoadStream()
            // The call to Asset::Create() is made on the main thread, that's why we queue it up on the EndLoadQueue
            if (request->mEmbeddedData != nullptr)
            {
                newAsset->LoadEmbedded(request->mEmbeddedData, request);
            }
            else
            {
                newAsset->LoadFile(request->mPath.c_str(), request);
            }

            request->mAsset = newAsset;

            // (4) Add the request to the EndLoadQueue
            {
                SCOPED_LOCK(am.mMutex);
                am.mEndLoadQueue.push_back(request);
            }
        }
        else
        {
            // If nothing to process then sleep, for a bit so we don't was cpu cycles checking.
            SYS_Sleep(2);
        }
    }

    THREAD_RETURN();
}

void AssetManager::UpdateEndLoadQueue()
{
    SCOPED_LOCK(mMutex);

    const uint32_t maxIterations = 3;
    uint32_t numIterations = 0;

    bool handled = false;

    do
    {
        handled = false;
        AsyncLoadRequest* loadRequest = (mEndLoadQueue.size() > 0) ? mEndLoadQueue.front() : nullptr;

        if (mEndLoadQueue.size() > 0)
        {
            mEndLoadQueue.pop_front();
        }

        if (loadRequest != nullptr)
        {
            numIterations++;

            // Check load dependencies before finish the load
            bool allDependenciesLoaded = true;

            for (uint32_t i = 0; i < loadRequest->mDependentAssets.size(); ++i)
            {
                if (loadRequest->mDependentAssets[i]->mAsset == nullptr)
                {
                    allDependenciesLoaded = false;
                    break;
                }
            }

            if (allDependenciesLoaded)
            {
                handled = true;

                AssetStub* stub = GetAssetStub(loadRequest->mName);

                if (stub == nullptr)
                {
                    LogError("Cannot find asset for async load request");
                }
                else if (stub->mAsset != nullptr)
                {
                    LogWarning("AsyncLoadRequest not finished because the asset has already been loaded");
                }
                else
                {
                    LogDebug("Finished Async Loading: %s", loadRequest->mName.c_str());

                    // Finish the load on the main thread and assign the stub's mAsset so that it is officially "Loaded"
                    OCT_ASSERT(loadRequest->mAsset != nullptr);
                    loadRequest->mAsset->Create();
                    stub->mAsset = loadRequest->mAsset;

                    // Now assign the asset to all of the refs that had requested the load
                    for (uint32_t i = 0; i < loadRequest->mTargetRefs.size(); ++i)
                    {
                        if (loadRequest->mTargetRefs[i] != nullptr)
                        {
                            // The load request of the target ref should match this load request but...
                            // We need to make sure we handle the case where an AssetRef is assigned twice to an async load
                            // before the first one finishes. Might mean Canceling the request if one already exists in AsyncLoadAsset()
                            OCT_ASSERT(loadRequest->mTargetRefs[i]->mLoadRequest == nullptr ||
                                loadRequest->mTargetRefs[i]->mLoadRequest == loadRequest);

                            (*loadRequest->mTargetRefs[i]) = loadRequest->mAsset;
                            loadRequest->mTargetRefs[i]->mLoadRequest = nullptr;
                        }
                    }
                }

                delete loadRequest;
                loadRequest = nullptr;
            }
            else
            {
                // Still waiting on some dependent assets, so push this on the back of the queue
                // This might happen several times before the dependent assests make their way to the front
                // of the mEndLoadQueue.
                LogWarning("Async load for %s is still waiting on dependencies", loadRequest->mName.c_str());
                mEndLoadQueue.push_back(loadRequest);
                loadRequest->mRequeueCount++;

                if (loadRequest->mRequeueCount >= ASYNC_REQUEUE_LIMIT)
                {
                    LogWarning("Exceeded requeue limit for %s, possible cyclical dependency. Forcing load.", loadRequest->mName.c_str());
                    LoadAsset(loadRequest->mName);
                }
            }
        }
    } while (handled && numIterations < maxIterations);
}

#if EDITOR
glm::vec4 AssetManager::GetEditorAssetColor(TypeId type)
{
    glm::vec4 retColor = { 0.5f, 0.5f, 0.5f, 1.0f };

    if (mAssetColorMap.find(type) != mAssetColorMap.end())
    {
        retColor = mAssetColorMap[type];
    }

    return retColor;
}

void AssetManager::InitAssetColorMap()
{
    std::vector<Factory*>& factoryList = Asset::GetFactoryList();

    for (uint32_t i = 0; i < factoryList.size(); ++i)
    {
        Asset* asset = (Asset*)factoryList[i]->Create();
        mAssetColorMap.insert(std::pair<TypeId, glm::vec4>(factoryList[i]->GetType(), asset->GetTypeColor()));
        delete asset;
    }
}
#endif
