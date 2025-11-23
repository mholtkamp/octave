#pragma once

#include <vector>
#include <string>

class Asset;
class AssetDir;
struct AssetStub;

class AssetDir
{
public:
    std::string mName;
    std::string mPath;
    std::vector<AssetStub*> mAssetStubs;
    std::vector<AssetDir*> mChildDirs;
    AssetDir* mParentDir;
    bool mEngineDir;

    ~AssetDir();
    AssetDir(const std::string& name,
            const std::string& localPath,
            AssetDir* parent);
    AssetDir();

    AssetDir* GetSubdirectory(const std::string& name);
    AssetDir* CreateSubdirectory(const std::string& name);
    void DeleteSubdirectory(const char* name);
    bool RemoveAssetStub(AssetStub* stub);
    void Purge();
    void SortChildrenAlphabetically();
};