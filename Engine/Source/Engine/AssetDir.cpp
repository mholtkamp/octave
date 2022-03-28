#include "AssetDir.h"
#include "Engine.h"
#include "AssetManager.h"

#include <algorithm>

AssetDir::AssetDir() :
    mParentDir(nullptr)
{

}

AssetDir::AssetDir(const std::string& name,
                   const std::string& localPath,
                   AssetDir* parent) :
    mName(name),
    mPath(localPath),
    mParentDir(parent),
    mEngineDir(false)
{
    if (parent != nullptr)
    {
        mEngineDir = parent->mEngineDir;
        parent->mChildDirs.push_back(this);
    }
}

AssetDir::~AssetDir()
{
    for (uint32_t i = 0; i < mChildDirs.size(); ++i)
    {
         delete mChildDirs[i];
    }
}

bool AssetDir::RemoveAssetStub(AssetStub* stub)
{
    bool removed = false;
    auto it = std::find(mAssetStubs.begin(), mAssetStubs.end(), stub);
    
    if (it != mAssetStubs.end())
    {
        mAssetStubs.erase(it);
        removed = true;
    }

    return removed;
}

AssetDir* AssetDir::CreateSubdirectory(const std::string& name)
{
    AssetDir* subdir = new AssetDir(name, mPath + name + "/", this);
    return subdir;
}

void AssetDir::DeleteSubdirectory(const char* name)
{
    for (uint32_t i = 0; i < mChildDirs.size(); ++i)
    {
        if (mChildDirs[i]->mName == name)
        {
            mChildDirs[i]->Purge();
            delete mChildDirs[i];
            mChildDirs.erase(mChildDirs.begin() + i);
            break;
        }
    }
}

void AssetDir::Purge()
{
#if EDITOR
    // Purge all assets within
    for (uint32_t i = 0; i < mAssetStubs.size(); ++i)
    {
        AssetManager::Get()->PurgeAsset(mAssetStubs[i]->mName.c_str());
    }

    mAssetStubs.clear();

    // Purge all subdirectories and delete the AssetDir objects
    for (int32_t d = int32_t(mChildDirs.size()) - 1; d >= 0; --d)
    {
        mChildDirs[d]->Purge();
        delete mChildDirs[d];
        mChildDirs.erase(mChildDirs.begin() + d);
    }

    mChildDirs.clear();
#endif
}
