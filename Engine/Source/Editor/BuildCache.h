#pragma once

#if EDITOR

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include "EngineTypes.h"

struct FileEntry
{
    std::string mPath;
    int64_t mModTime = 0;
};

struct BuildManifest
{
    static constexpr uint32_t kCurrentVersion = 1;

    uint32_t mVersion = kCurrentVersion;
    Platform mPlatform = Platform::Windows;
    bool mEmbedded = false;
    int64_t mBuildTime = 0;
    std::string mProjectName;
    std::string mOutputDirectory;

    std::vector<FileEntry> mAssets;
    std::vector<FileEntry> mScripts;
    std::vector<FileEntry> mConfigFiles;
};

enum class BuildCacheResult
{
    UpToDate,
    NeedsRebuild,
    ManifestMissing,
    ManifestCorrupt,
    OutputMissing,
    VersionMismatch
};

class BuildCache
{
public:
    static BuildCache* Get();
    static void Create();
    static void Destroy();

    BuildCacheResult CheckRebuildNeeded(Platform platform, bool embedded);
    std::string GetRebuildReason() const { return mRebuildReason; }

    void BuildCurrentManifest(Platform platform, bool embedded);
    bool SaveManifest();
    bool LoadManifest(Platform platform, bool embedded);

    std::string GetManifestPath(Platform platform, bool embedded) const;

private:
    BuildCache();
    ~BuildCache();

    void GatherAssetFiles(std::vector<FileEntry>& outAssets);
    void GatherScriptFiles(std::vector<FileEntry>& outScripts);
    void GatherConfigFiles(std::vector<FileEntry>& outConfigs);

    int64_t GetFileModTime(const std::string& path) const;
    bool CompareManifests(const BuildManifest& current, const BuildManifest& saved);
    bool VerifyOutputDirectory() const;

    static BuildCache* sInstance;

    BuildManifest mCurrentManifest;
    BuildManifest mSavedManifest;
    std::string mRebuildReason;
};

#endif
