#if EDITOR

#include "BuildCache.h"
#include "ActionManager.h"
#include "Engine.h"
#include "AssetManager.h"
#include "System/System.h"
#include "Utilities.h"
#include "Stream.h"
#include "Log.h"

#include "document.h"
#include "prettywriter.h"
#include "stringbuffer.h"

#include <sys/stat.h>
#include <ctime>

BuildCache* BuildCache::sInstance = nullptr;

BuildCache* BuildCache::Get()
{
    return sInstance;
}

void BuildCache::Create()
{
    if (sInstance == nullptr)
    {
        sInstance = new BuildCache();
    }
}

void BuildCache::Destroy()
{
    if (sInstance != nullptr)
    {
        delete sInstance;
        sInstance = nullptr;
    }
}

BuildCache::BuildCache()
{
}

BuildCache::~BuildCache()
{
}

int64_t BuildCache::GetFileModTime(const std::string& path) const
{
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
    {
        return 0;
    }
    return static_cast<int64_t>(info.st_mtime);
}

std::string BuildCache::GetManifestPath(Platform platform, bool embedded) const
{
    const EngineState* engineState = GetEngineState();
    std::string projectDir = engineState->mProjectDirectory;

    std::string intermediateDir = projectDir + "Intermediate/";
    if (!DoesDirExist(intermediateDir.c_str()))
    {
        CreateDir(intermediateDir.c_str());
    }

    std::string mode = embedded ? "Embedded" : "External";
    return intermediateDir + "BuildManifest_" + GetPlatformString(platform) + "_" + mode + ".json";
}

void BuildCache::GatherAssetFiles(std::vector<FileEntry>& outAssets)
{
    outAssets.clear();

    AssetManager* assetMgr = AssetManager::Get();
    if (assetMgr == nullptr) return;

    auto& assetMap = assetMgr->GetAssetMap();
    for (auto& pair : assetMap)
    {
        AssetStub* stub = pair.second;
        if (stub == nullptr) continue;

        FileEntry entry;
        entry.mPath = stub->mPath;
        entry.mModTime = GetFileModTime(stub->mPath);
        outAssets.push_back(entry);
    }
}

void BuildCache::GatherScriptFiles(std::vector<FileEntry>& outScripts)
{
    outScripts.clear();

    const EngineState* engineState = GetEngineState();
    std::string octaveDir = SYS_GetOctavePath();
    std::string projectDir = engineState->mProjectDirectory;

    std::vector<std::string> scriptPaths;

    // Gather from Engine/Scripts
    ActionManager::Get()->GatherScriptFiles(octaveDir + "Engine/Scripts/", scriptPaths);

    // Gather from Project/Scripts
    ActionManager::Get()->GatherScriptFiles(projectDir + "Scripts/", scriptPaths);

    // Gather from Packages
    std::string packagesDir = projectDir + "Packages/";
    if (DoesDirExist(packagesDir.c_str()))
    {
        DirEntry dirEntry;
        SYS_OpenDirectory(packagesDir, dirEntry);
        while (dirEntry.mValid)
        {
            if (dirEntry.mDirectory && dirEntry.mFilename[0] != '.')
            {
                std::string pkgScriptsDir = packagesDir + dirEntry.mFilename + "/Scripts/";
                if (DoesDirExist(pkgScriptsDir.c_str()))
                {
                    ActionManager::Get()->GatherScriptFiles(pkgScriptsDir, scriptPaths);
                }
            }
            SYS_IterateDirectory(dirEntry);
        }
        SYS_CloseDirectory(dirEntry);
    }

    for (const auto& path : scriptPaths)
    {
        FileEntry entry;
        entry.mPath = path;
        entry.mModTime = GetFileModTime(path);
        outScripts.push_back(entry);
    }
}

void BuildCache::GatherConfigFiles(std::vector<FileEntry>& outConfigs)
{
    outConfigs.clear();

    const EngineState* engineState = GetEngineState();
    std::string projectDir = engineState->mProjectDirectory;
    std::string projectName = engineState->mProjectName;

    // Config.ini
    std::string configPath = projectDir + "Config.ini";
    if (SYS_DoesFileExist(configPath.c_str(), false))
    {
        FileEntry entry;
        entry.mPath = configPath;
        entry.mModTime = GetFileModTime(configPath);
        outConfigs.push_back(entry);
    }

    // Project file (.octp)
    std::string octpPath = projectDir + projectName + ".octp";
    if (SYS_DoesFileExist(octpPath.c_str(), false))
    {
        FileEntry entry;
        entry.mPath = octpPath;
        entry.mModTime = GetFileModTime(octpPath);
        outConfigs.push_back(entry);
    }
}

void BuildCache::BuildCurrentManifest(Platform platform, bool embedded)
{
    mCurrentManifest = BuildManifest();
    mCurrentManifest.mVersion = BuildManifest::kCurrentVersion;
    mCurrentManifest.mPlatform = platform;
    mCurrentManifest.mEmbedded = embedded;
    mCurrentManifest.mBuildTime = static_cast<int64_t>(time(nullptr));
    mCurrentManifest.mProjectName = GetEngineState()->mProjectName;

    std::string projectDir = GetEngineState()->mProjectDirectory;
    mCurrentManifest.mOutputDirectory = projectDir + "Packaged/" + GetPlatformString(platform) + "/";

    GatherAssetFiles(mCurrentManifest.mAssets);
    GatherScriptFiles(mCurrentManifest.mScripts);
    GatherConfigFiles(mCurrentManifest.mConfigFiles);
}

bool BuildCache::SaveManifest()
{
    rapidjson::Document doc;
    doc.SetObject();
    auto& alloc = doc.GetAllocator();

    doc.AddMember("version", mCurrentManifest.mVersion, alloc);
    doc.AddMember("platform", rapidjson::Value(GetPlatformString(mCurrentManifest.mPlatform), alloc), alloc);
    doc.AddMember("embedded", mCurrentManifest.mEmbedded, alloc);
    doc.AddMember("buildTime", mCurrentManifest.mBuildTime, alloc);
    doc.AddMember("projectName", rapidjson::Value(mCurrentManifest.mProjectName.c_str(), alloc), alloc);
    doc.AddMember("outputDirectory", rapidjson::Value(mCurrentManifest.mOutputDirectory.c_str(), alloc), alloc);

    auto addFileEntries = [&](const char* name, const std::vector<FileEntry>& entries)
    {
        rapidjson::Value arr(rapidjson::kArrayType);
        for (const auto& entry : entries)
        {
            rapidjson::Value obj(rapidjson::kObjectType);
            obj.AddMember("path", rapidjson::Value(entry.mPath.c_str(), alloc), alloc);
            obj.AddMember("modTime", entry.mModTime, alloc);
            arr.PushBack(obj, alloc);
        }
        doc.AddMember(rapidjson::Value(name, alloc), arr, alloc);
    };

    addFileEntries("assets", mCurrentManifest.mAssets);
    addFileEntries("scripts", mCurrentManifest.mScripts);
    addFileEntries("configFiles", mCurrentManifest.mConfigFiles);

    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    std::string path = GetManifestPath(mCurrentManifest.mPlatform, mCurrentManifest.mEmbedded);
    Stream outStream(buffer.GetString(), (uint32_t)buffer.GetSize());
    outStream.WriteFile(path.c_str());

    LogDebug("Build cache: saved manifest to %s", path.c_str());
    return true;
}

bool BuildCache::LoadManifest(Platform platform, bool embedded)
{
    std::string path = GetManifestPath(platform, embedded);
    if (!SYS_DoesFileExist(path.c_str(), false))
    {
        return false;
    }

    Stream stream;
    stream.ReadFile(path.c_str(), false);
    if (stream.GetSize() == 0)
    {
        return false;
    }

    rapidjson::Document doc;
    std::string jsonStr(stream.GetData(), stream.GetSize());
    doc.Parse(jsonStr.c_str());

    if (doc.HasParseError() || !doc.IsObject())
    {
        LogWarning("Build cache: failed to parse manifest");
        return false;
    }

    mSavedManifest = BuildManifest();

    if (doc.HasMember("version") && doc["version"].IsUint())
        mSavedManifest.mVersion = doc["version"].GetUint();

    if (doc.HasMember("platform") && doc["platform"].IsString())
    {
        std::string platformStr = doc["platform"].GetString();
        for (int i = 0; i < (int)Platform::Count; ++i)
        {
            if (platformStr == GetPlatformString((Platform)i))
            {
                mSavedManifest.mPlatform = (Platform)i;
                break;
            }
        }
    }

    if (doc.HasMember("embedded") && doc["embedded"].IsBool())
        mSavedManifest.mEmbedded = doc["embedded"].GetBool();

    if (doc.HasMember("buildTime") && doc["buildTime"].IsInt64())
        mSavedManifest.mBuildTime = doc["buildTime"].GetInt64();

    if (doc.HasMember("projectName") && doc["projectName"].IsString())
        mSavedManifest.mProjectName = doc["projectName"].GetString();

    if (doc.HasMember("outputDirectory") && doc["outputDirectory"].IsString())
        mSavedManifest.mOutputDirectory = doc["outputDirectory"].GetString();

    auto loadFileEntries = [&](const char* name, std::vector<FileEntry>& entries)
    {
        entries.clear();
        if (doc.HasMember(name) && doc[name].IsArray())
        {
            for (auto& item : doc[name].GetArray())
            {
                if (item.IsObject())
                {
                    FileEntry entry;
                    if (item.HasMember("path") && item["path"].IsString())
                        entry.mPath = item["path"].GetString();
                    if (item.HasMember("modTime") && item["modTime"].IsInt64())
                        entry.mModTime = item["modTime"].GetInt64();
                    entries.push_back(entry);
                }
            }
        }
    };

    loadFileEntries("assets", mSavedManifest.mAssets);
    loadFileEntries("scripts", mSavedManifest.mScripts);
    loadFileEntries("configFiles", mSavedManifest.mConfigFiles);

    return true;
}

bool BuildCache::CompareManifests(const BuildManifest& current, const BuildManifest& saved)
{
    // Compare file lists
    auto compareEntries = [this](const std::vector<FileEntry>& currentList,
                                  const std::vector<FileEntry>& savedList,
                                  const char* category) -> bool
    {
        // Build lookup map for saved entries
        std::unordered_map<std::string, int64_t> savedMap;
        for (const auto& entry : savedList)
        {
            savedMap[entry.mPath] = entry.mModTime;
        }

        // Check for new or modified files
        for (const auto& entry : currentList)
        {
            auto it = savedMap.find(entry.mPath);
            if (it == savedMap.end())
            {
                mRebuildReason = std::string("New ") + category + ": " + entry.mPath;
                return false;
            }
            if (entry.mModTime != it->second)
            {
                mRebuildReason = std::string("Modified ") + category + ": " + entry.mPath;
                return false;
            }
            savedMap.erase(it);
        }

        // Check for removed files
        if (!savedMap.empty())
        {
            mRebuildReason = std::string("Removed ") + category + ": " + savedMap.begin()->first;
            return false;
        }

        return true;
    };

    if (!compareEntries(current.mAssets, saved.mAssets, "asset"))
        return false;

    if (!compareEntries(current.mScripts, saved.mScripts, "script"))
        return false;

    if (!compareEntries(current.mConfigFiles, saved.mConfigFiles, "config"))
        return false;

    return true;
}

bool BuildCache::VerifyOutputDirectory() const
{
    const EngineState* engineState = GetEngineState();
    std::string outputDir = mSavedManifest.mOutputDirectory;
    std::string projectName = engineState->mProjectName;

    if (!DoesDirExist(outputDir.c_str()))
    {
        return false;
    }

    // Check for executable
    std::string extension;
    switch (mSavedManifest.mPlatform)
    {
    case Platform::Windows: extension = ".exe"; break;
    case Platform::Linux: extension = ""; break;
    case Platform::Android: extension = ".apk"; break;
    case Platform::GameCube: extension = ".dol"; break;
    case Platform::Wii: extension = ".dol"; break;
    case Platform::N3DS: extension = ".3dsx"; break;
    default: extension = ""; break;
    }

    std::string exePath = outputDir + projectName + extension;
    if (!SYS_DoesFileExist(exePath.c_str(), false))
    {
        return false;
    }

    return true;
}

BuildCacheResult BuildCache::CheckRebuildNeeded(Platform platform, bool embedded)
{
    // Load saved manifest
    if (!LoadManifest(platform, embedded))
    {
        mRebuildReason = "No previous build manifest found";
        return BuildCacheResult::ManifestMissing;
    }

    // Version check
    if (mSavedManifest.mVersion != BuildManifest::kCurrentVersion)
    {
        mRebuildReason = "Manifest version mismatch";
        return BuildCacheResult::VersionMismatch;
    }

    // Build current manifest for comparison
    BuildCurrentManifest(platform, embedded);

    // Compare
    if (!CompareManifests(mCurrentManifest, mSavedManifest))
    {
        return BuildCacheResult::NeedsRebuild;
    }

    // Verify output directory is intact
    if (!VerifyOutputDirectory())
    {
        mRebuildReason = "Output directory missing or incomplete";
        return BuildCacheResult::OutputMissing;
    }

    mRebuildReason = "All files unchanged";
    return BuildCacheResult::UpToDate;
}

#endif
