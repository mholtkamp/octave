#if EDITOR

#include "AddonManager.h"
#include "System/System.h"
#include "Engine.h"
#include "Stream.h"
#include "Utilities.h"
#include "Log.h"

#include "document.h"
#include "prettywriter.h"
#include "stringbuffer.h"

#include <cstdlib>
#include <ctime>
#include <functional>

AddonManager* AddonManager::sInstance = nullptr;

void AddonManager::Create()
{
    if (sInstance == nullptr)
    {
        sInstance = new AddonManager();
    }
}

void AddonManager::Destroy()
{
    if (sInstance != nullptr)
    {
        delete sInstance;
        sInstance = nullptr;
    }
}

AddonManager* AddonManager::Get()
{
    return sInstance;
}

AddonManager::AddonManager()
{
    LoadSettings();
}

AddonManager::~AddonManager()
{
    SaveSettings();
}

std::string AddonManager::GetAddonCacheDirectory()
{
    std::string cacheDir;

#if PLATFORM_WINDOWS
    const char* appData = getenv("APPDATA");
    if (appData != nullptr)
    {
        cacheDir = std::string(appData) + "/OctaveEditor/AddonCache";
    }
    else
    {
        const char* userProfile = getenv("USERPROFILE");
        if (userProfile != nullptr)
        {
            cacheDir = std::string(userProfile) + "/AppData/Roaming/OctaveEditor/AddonCache";
        }
    }
#else
    const char* home = getenv("HOME");
    if (home != nullptr)
    {
        cacheDir = std::string(home) + "/.config/OctaveEditor/AddonCache";
    }
#endif

    if (cacheDir.empty())
    {
        cacheDir = "Engine/Saves/AddonCache";
    }

    return cacheDir;
}

std::string AddonManager::GetSettingsPath()
{
    std::string settingsDir;

#if PLATFORM_WINDOWS
    const char* appData = getenv("APPDATA");
    if (appData != nullptr)
    {
        settingsDir = std::string(appData) + "/OctaveEditor";
    }
#else
    const char* home = getenv("HOME");
    if (home != nullptr)
    {
        settingsDir = std::string(home) + "/.config/OctaveEditor";
    }
#endif

    if (settingsDir.empty())
    {
        settingsDir = "Engine/Saves";
    }

    return settingsDir + "/addons.json";
}

std::string AddonManager::GetInstalledAddonsPath()
{
    const std::string& projDir = GetEngineState()->mProjectDirectory;
    if (projDir.empty())
    {
        return "";
    }
    return projDir + "Settings/installed_addons.json";
}

void AddonManager::EnsureCacheDirectory()
{
    std::string cacheDir = GetAddonCacheDirectory();

#if PLATFORM_WINDOWS
    const char* appData = getenv("APPDATA");
    if (appData != nullptr)
    {
        std::string octaveDir = std::string(appData) + "/OctaveEditor";
        if (!DoesDirExist(octaveDir.c_str()))
        {
            SYS_CreateDirectory(octaveDir.c_str());
        }
    }
#else
    const char* home = getenv("HOME");
    if (home != nullptr)
    {
        std::string configDir = std::string(home) + "/.config";
        if (!DoesDirExist(configDir.c_str()))
        {
            SYS_CreateDirectory(configDir.c_str());
        }
        std::string octaveDir = configDir + "/OctaveEditor";
        if (!DoesDirExist(octaveDir.c_str()))
        {
            SYS_CreateDirectory(octaveDir.c_str());
        }
    }
#endif

    if (!DoesDirExist(cacheDir.c_str()))
    {
        SYS_CreateDirectory(cacheDir.c_str());
    }
}

void AddonManager::LoadSettings()
{
    mRepositories.clear();

    std::string settingsPath = GetSettingsPath();
    if (!SYS_DoesFileExist(settingsPath.c_str(), false))
    {
        // Add default repository on first run
        AddonRepository defaultRepo;
        defaultRepo.mName = "Official Octave Addons";
        defaultRepo.mUrl = "https://github.com/OctaveEngine/addons";
        mRepositories.push_back(defaultRepo);
        SaveSettings();
        return;
    }

    Stream stream;
    if (!stream.ReadFile(settingsPath.c_str(), false))
    {
        return;
    }

    std::string jsonStr(stream.GetData(), stream.GetSize());
    rapidjson::Document doc;
    doc.Parse(jsonStr.c_str());

    if (doc.HasParseError())
    {
        LogError("Failed to parse addons.json");
        return;
    }

    if (doc.HasMember("repositories") && doc["repositories"].IsArray())
    {
        const rapidjson::Value& repos = doc["repositories"];
        for (rapidjson::SizeType i = 0; i < repos.Size(); ++i)
        {
            const rapidjson::Value& repoObj = repos[i];
            AddonRepository repo;

            if (repoObj.HasMember("name") && repoObj["name"].IsString())
            {
                repo.mName = repoObj["name"].GetString();
            }
            if (repoObj.HasMember("url") && repoObj["url"].IsString())
            {
                repo.mUrl = repoObj["url"].GetString();
            }

            if (!repo.mUrl.empty())
            {
                mRepositories.push_back(repo);
            }
        }
    }
}

void AddonManager::SaveSettings()
{
    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

    doc.AddMember("version", 1, allocator);

    rapidjson::Value reposArray(rapidjson::kArrayType);
    for (const AddonRepository& repo : mRepositories)
    {
        rapidjson::Value repoObj(rapidjson::kObjectType);
        repoObj.AddMember("name", rapidjson::Value(repo.mName.c_str(), allocator), allocator);
        repoObj.AddMember("url", rapidjson::Value(repo.mUrl.c_str(), allocator), allocator);
        reposArray.PushBack(repoObj, allocator);
    }
    doc.AddMember("repositories", reposArray, allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    // Ensure directory exists
    std::string settingsPath = GetSettingsPath();
    std::string settingsDir = settingsPath.substr(0, settingsPath.find_last_of("/\\"));
    if (!DoesDirExist(settingsDir.c_str()))
    {
        SYS_CreateDirectory(settingsDir.c_str());
    }

    Stream stream(buffer.GetString(), (uint32_t)buffer.GetSize());
    stream.WriteFile(settingsPath.c_str());
}

void AddonManager::AddRepository(const std::string& url)
{
    // Check if already exists
    for (const AddonRepository& repo : mRepositories)
    {
        if (repo.mUrl == url)
        {
            return;
        }
    }

    AddonRepository newRepo;
    newRepo.mUrl = url;
    newRepo.mName = url;  // Will be updated when fetched

    mRepositories.push_back(newRepo);
    SaveSettings();

    // Refresh to get repo info
    RefreshRepository(url);
}

void AddonManager::RemoveRepository(const std::string& url)
{
    for (auto it = mRepositories.begin(); it != mRepositories.end(); ++it)
    {
        if (it->mUrl == url)
        {
            mRepositories.erase(it);
            SaveSettings();

            // Remove addons from this repo from available list
            for (auto addonIt = mAvailableAddons.begin(); addonIt != mAvailableAddons.end();)
            {
                if (addonIt->mRepoUrl == url)
                {
                    addonIt = mAvailableAddons.erase(addonIt);
                }
                else
                {
                    ++addonIt;
                }
            }
            return;
        }
    }
}

static bool IsGitHubUrl(const std::string& str)
{
    return str.find("github.com/") != std::string::npos;
}

std::string AddonManager::ConvertToRawUrl(const std::string& gitHubUrl, const std::string& filePath, const std::string& branch = "main")
{
    // Convert: https://github.com/user/repo
    // To: https://raw.githubusercontent.com/user/repo/main/filePath

    std::string url = gitHubUrl;

    // Remove trailing slash
    while (!url.empty() && url.back() == '/')
    {
        url.pop_back();
    }

    // Replace github.com with raw.githubusercontent.com
    size_t githubPos = url.find("github.com");
    if (githubPos != std::string::npos)
    {
        url.replace(githubPos, 10, "raw.githubusercontent.com");
    }

    return url + "/"+ branch +"/" + filePath;
}

std::string AddonManager::ConvertToDownloadUrl(const std::string& gitHubUrl,  const std::string& branch = "main")
{
    std::string url = gitHubUrl;

    while (!url.empty() && url.back() == '/')
    {
        url.pop_back();
    }

    return url + "/archive/refs/heads/"+ branch +".zip";
}

bool AddonManager::DownloadFile(const std::string& url, const std::string& destPath, std::string& outError)
{
    std::string output;

#if PLATFORM_WINDOWS
    std::string cmd = "curl -L -s -o \"" + destPath + "\" \"" + url + "\" 2>&1";
    SYS_Exec(cmd.c_str(), &output);

    if (!SYS_DoesFileExist(destPath.c_str(), false))
    {
        cmd = "powershell -Command \"Invoke-WebRequest -Uri '" + url + "' -OutFile '" + destPath + "'\" 2>&1";
        SYS_Exec(cmd.c_str(), &output);
    }
#else
    std::string cmd = "curl -L -s -o \"" + destPath + "\" \"" + url + "\" 2>&1";
    SYS_Exec(cmd.c_str(), &output);
#endif

    if (!SYS_DoesFileExist(destPath.c_str(), false))
    {
        outError = "Failed to download: " + output;
        return false;
    }

    return true;
}

bool AddonManager::ExtractZip(const std::string& zipPath, const std::string& destDir, std::string& outError)
{
    std::string output;

    if (!DoesDirExist(destDir.c_str()))
    {
        SYS_CreateDirectory(destDir.c_str());
    }
    std::string zipPath_ = NormalizePath(zipPath);
    std::string extractPath = NormalizePath(destDir);
#if PLATFORM_WINDOWS
    std::string cmd = "C:\\Windows\\System32\\tar.exe -xf \"" + zipPath_ + "\" -C \"" + extractPath + "\" 2>&1";
    SYS_Exec(cmd.c_str(), &output);
#else
    std::string cmd = "unzip -o \"" + zipPath_ + "\" -d \"" + extractPath + "\" 2>&1";
    SYS_Exec(cmd.c_str(), &output);
#endif

    return true;
}
std::string AddonManager::NormalizePath(const std::string& in)
{
    std::string out = in;
    for (char& c : out)
    {
        if (c == '\\')
            c = '/';
    }
    return out;
}
bool AddonManager::FetchRepositoryManifest(const std::string& url, AddonRepository& outRepo, const std::string& branch = "main")
{
    EnsureCacheDirectory();

    std::string rawUrl = ConvertToRawUrl(url, "package.json", branch);
    std::string tempPath = GetAddonCacheDirectory() + "/_temp_manifest.json";

    std::string error;
    if (!DownloadFile(rawUrl, tempPath, error))
    {
		// test "master" branch if "main" fails
		rawUrl = ConvertToRawUrl(url, "package.json", "master");
        if (!DownloadFile(rawUrl, tempPath, error)) {

            LogWarning("Failed to fetch repository manifest from %s: %s", url.c_str(), error.c_str());
            return false;
        }
    }

    Stream stream;
    if (!stream.ReadFile(tempPath.c_str(), false))
    {
        SYS_RemoveFile(tempPath.c_str());
        return false;
    }

    std::string jsonStr(stream.GetData(), stream.GetSize());
    rapidjson::Document doc;
    doc.Parse(jsonStr.c_str());

    SYS_RemoveFile(tempPath.c_str());

    if (doc.HasParseError())
    {
        LogWarning("Failed to parse repository manifest from %s", url.c_str());
        return false;
    }

    outRepo.mUrl = url;

    if (doc.HasMember("name") && doc["name"].IsString())
    {
        outRepo.mName = doc["name"].GetString();
    }
    else
    {
        outRepo.mName = url;
    }

    outRepo.mAddonIds.clear();
    if (doc.HasMember("addons") && doc["addons"].IsArray())
    {
        const rapidjson::Value& addons = doc["addons"];
        for (rapidjson::SizeType i = 0; i < addons.Size(); ++i)
        {
            if (addons[i].IsString())
            {
                outRepo.mAddonIds.push_back(addons[i].GetString());
            }
        }
    }

    return true;
}

bool AddonManager::FetchAddonMetadata(const std::string& repoUrl, const std::string& addonId, Addon& outAddon, const std::string& branch = "main")
{
    EnsureCacheDirectory();

    std::string rawUrl;
    std::string effectiveId = addonId;

    if (addonId.empty())
    {
        // Standalone repo - package.json is at root
        rawUrl = ConvertToRawUrl(repoUrl, "package.json", branch);
        // Derive ID from URL: https://github.com/user/cool-addon -> cool-addon
        std::string trimmed = repoUrl;
        while (!trimmed.empty() && trimmed.back() == '/') trimmed.pop_back();
        size_t lastSlash = trimmed.find_last_of('/');
        effectiveId = (lastSlash != std::string::npos) ? trimmed.substr(lastSlash + 1) : trimmed;
    }
    else
    {
        rawUrl = ConvertToRawUrl(repoUrl, addonId + "/package.json", branch);
    }

    std::string tempPath = GetAddonCacheDirectory() + "/_temp_addon_meta.json";

    std::string error;
    if (!DownloadFile(rawUrl, tempPath, error))
    {
        LogWarning("Failed to fetch addon metadata for %s: %s", effectiveId.c_str(), error.c_str());
        return false;
    }

    Stream stream;
    if (!stream.ReadFile(tempPath.c_str(), false))
    {
        SYS_RemoveFile(tempPath.c_str());
        return false;
    }

    std::string jsonStr(stream.GetData(), stream.GetSize());
    rapidjson::Document doc;
    doc.Parse(jsonStr.c_str());

    SYS_RemoveFile(tempPath.c_str());

    if (doc.HasParseError())
    {
        LogWarning("Failed to parse addon metadata for %s", effectiveId.c_str());
        return false;
    }

    outAddon.mMetadata.mId = effectiveId;
    outAddon.mRepoUrl = repoUrl;

    if (doc.HasMember("name") && doc["name"].IsString())
    {
        outAddon.mMetadata.mName = doc["name"].GetString();
    }
    else
    {
        outAddon.mMetadata.mName = effectiveId;
    }

    if (doc.HasMember("author") && doc["author"].IsString())
    {
        outAddon.mMetadata.mAuthor = doc["author"].GetString();
    }

    if (doc.HasMember("description") && doc["description"].IsString())
    {
        outAddon.mMetadata.mDescription = doc["description"].GetString();
    }

    if (doc.HasMember("url") && doc["url"].IsString())
    {
        outAddon.mMetadata.mUrl = doc["url"].GetString();
    }

    if (doc.HasMember("version") && doc["version"].IsString())
    {
        outAddon.mMetadata.mVersion = doc["version"].GetString();
    }

    if (doc.HasMember("updated") && doc["updated"].IsString())
    {
        outAddon.mMetadata.mUpdated = doc["updated"].GetString();
    }

    if (doc.HasMember("tags") && doc["tags"].IsArray())
    {
        const rapidjson::Value& tags = doc["tags"];
        for (rapidjson::SizeType i = 0; i < tags.Size(); ++i)
        {
            if (tags[i].IsString())
            {
                outAddon.mMetadata.mTags.push_back(tags[i].GetString());
            }
        }
    }

    // Parse native module metadata
    if (doc.HasMember("native") && doc["native"].IsObject())
    {
        const rapidjson::Value& native = doc["native"];
        outAddon.mNative.mHasNative = true;

        // Parse target: "engine" (default) or "editor"
        if (native.HasMember("target") && native["target"].IsString())
        {
            std::string target = native["target"].GetString();
            if (target == "editor")
            {
                outAddon.mNative.mTarget = NativeAddonTarget::EditorOnly;
            }
            else
            {
                outAddon.mNative.mTarget = NativeAddonTarget::EngineAndEditor;
            }
        }

        if (native.HasMember("sourceDir") && native["sourceDir"].IsString())
        {
            outAddon.mNative.mSourceDir = native["sourceDir"].GetString();
        }

        if (native.HasMember("binaryName") && native["binaryName"].IsString())
        {
            outAddon.mNative.mBinaryName = native["binaryName"].GetString();
        }
        else
        {
            // Default binary name to addon ID (lowercase, no spaces)
            outAddon.mNative.mBinaryName = effectiveId;
        }

        if (native.HasMember("entrySymbol") && native["entrySymbol"].IsString())
        {
            outAddon.mNative.mEntrySymbol = native["entrySymbol"].GetString();
        }

        if (native.HasMember("apiVersion") && native["apiVersion"].IsUint())
        {
            outAddon.mNative.mPluginApiVersion = native["apiVersion"].GetUint();
        }
    }

    // Check if installed
    outAddon.mIsInstalled = IsAddonInstalled(effectiveId);
    if (outAddon.mIsInstalled)
    {
        outAddon.mInstalledVersion = GetInstalledVersion(effectiveId);
    }

    return true;
}

void AddonManager::RefreshAllRepositories()
{
    mAvailableAddons.clear();

    for (AddonRepository& repo : mRepositories)
    {
        RefreshRepository(repo.mUrl);
    }

    // Update installed status
    LoadInstalledAddons();
    for (Addon& addon : mAvailableAddons)
    {
        addon.mIsInstalled = IsAddonInstalled(addon.mMetadata.mId);
        if (addon.mIsInstalled)
        {
            addon.mInstalledVersion = GetInstalledVersion(addon.mMetadata.mId);
        }
    }
}

void AddonManager::RefreshRepository(const std::string& url)
{
    AddonRepository repoInfo;
    if (!FetchRepositoryManifest(url, repoInfo, "main"))
    {
        if (!FetchRepositoryManifest(url, repoInfo, "master"))
        {
            return;
        }

    }
    
  
    // Update repository in list
    for (AddonRepository& repo : mRepositories)
    {
        if (repo.mUrl == url)
        {
            repo.mName = repoInfo.mName;
            repo.mAddonIds = repoInfo.mAddonIds;
            break;
        }
    }

    // Fetch each addon's metadata
    for (const std::string& addonId : repoInfo.mAddonIds)
    {
        Addon addon;
        bool fetched = false;

        if (IsGitHubUrl(addonId))
        {
            // Standalone addon from external repo
            fetched = FetchAddonMetadata(addonId, "", addon, "main");
            if (fetched)
            {
                addon.mIsStandalone = true;
            }
            else {
                fetched = FetchAddonMetadata(addonId, "", addon, "master");
                if (fetched)
                {
                    addon.mIsStandalone = true;
                    addon.mIsMain = false;
                }
            }
        }
        else
        {
            // Local subdirectory addon
            fetched = FetchAddonMetadata(url, addonId, addon);
        }

        if (fetched)
        {
            // Check if already in list (avoid duplicates from multiple repos)
            bool exists = false;
            for (const Addon& existing : mAvailableAddons)
            {
                if (existing.mMetadata.mId == addon.mMetadata.mId)
                {
                    exists = true;
                    break;
                }
            }

            if (!exists)
            {
                mAvailableAddons.push_back(addon);
            }
        }
    }

    SaveSettings();
}

bool AddonManager::DownloadAddon(const Addon& addon, std::string& outError)
{
    EnsureCacheDirectory();

    // Download the full repo and extract just this addon folder
    std::string downloadUrl = ConvertToDownloadUrl(addon.mRepoUrl, addon.mIsMain ? "main" : "master");
	// extract repoName from URL for logging
    std::string repoName = addon.mRepoUrl.substr(addon.mRepoUrl.find_last_of('/') + 1) + "-" + (addon.mIsMain ? "main" : "master");
    std::string zipPath = GetAddonCacheDirectory() + "/_temp_repo.zip";
    std::string extractDir = GetAddonCacheDirectory() + "/_temp_extract";

    if (!DownloadFile(downloadUrl, zipPath, outError))
    {
        return false;
    }

    if (DoesDirExist(extractDir.c_str()))
    {
        SYS_RemoveDirectory(extractDir.c_str());
    }

    if (!ExtractZip(zipPath, extractDir, outError))
    {
        SYS_RemoveFile(zipPath.c_str());
        return false;
    }

    extractDir = extractDir + "/" + repoName;
    //SYS_RemoveFile(zipPath.c_str());

    // Find the extracted folder (GitHub zips have repo-name-branch structure)
    DirEntry dirEntry;
    //SYS_OpenDirectory(extractDir, dirEntry);
    std::string extractedRepoFolder = extractDir;
    
    //SYS_CloseDirectory(dirEntry);

    if (extractedRepoFolder.empty())
    {
        outError = "Could not find extracted repository folder";
        SYS_RemoveDirectory(extractDir.c_str());
        return false;
    }

    // Find the addon folder
    std::string addonPath;
    if (addon.mIsStandalone)
    {
        // Standalone: the entire extracted repo IS the addon
        addonPath = extractedRepoFolder;
    }
    else
    {
        // Subdirectory: find addon folder within repo
        addonPath = extractedRepoFolder + "/" + addon.mMetadata.mId;
        if (!DoesDirExist(addonPath.c_str()))
        {
            outError = "Addon folder not found in repository: " + addon.mMetadata.mId;
            SYS_RemoveDirectory(extractDir.c_str());
            return false;
        }
    }

    // Move addon to cache
    std::string cachedAddonPath = GetAddonCacheDirectory() + "/" + addon.mMetadata.mId;
    if (DoesDirExist(cachedAddonPath.c_str()))
    {
        //SYS_RemoveDirectory(cachedAddonPath.c_str());
    }
    SYS_CreateDirectory(cachedAddonPath.c_str());

    SYS_CopyDirectoryRecursive(addonPath.c_str(), cachedAddonPath.c_str());
    //SYS_MoveDirectory(addonPath.c_str(), cachedAddonPath.c_str());

    // Clean up
    //SYS_RemoveDirectory(extractDir.c_str());

    // Install to project
    return InstallAddon(cachedAddonPath, addon.mMetadata.mId, outError);
}

std::string AddonManager::GetCurrentTimestamp()
{
    time_t now = time(nullptr);
    struct tm* timeinfo = gmtime(&now);
    char buffer[32];
    strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", timeinfo);
    return std::string(buffer);
}

bool AddonManager::MergeAddonIntoProject(const std::string& addonPath, std::string& outError)
{
    const std::string& projectDir = GetEngineState()->mProjectDirectory;
    if (projectDir.empty())
    {
        outError = "No project loaded";
        return false;
    }

    // Get list of files in addon
    std::function<void(const std::string&, const std::string&)> copyRecursive;
    copyRecursive = [&](const std::string& srcDir, const std::string& relPath)
    {
        DirEntry dirEntry;
        SYS_OpenDirectory(srcDir, dirEntry);

        while (dirEntry.mValid)
        {
            std::string filename = dirEntry.mFilename;

            if (filename != "." && filename != "..")
            {
                std::string srcPath = srcDir + "/" + filename;
                std::string relativePath = relPath.empty() ? filename : (relPath + "/" + filename);

                // Skip project files
                if (filename.find(".octp") != std::string::npos ||
                    filename.find(".ini") != std::string::npos ||
                    filename == "package.json" ||
                    filename == "thumbnail.png")
                {
                    SYS_IterateDirectory(dirEntry);
                    continue;
                }

                std::string destPath = projectDir + relativePath;

                if (dirEntry.mDirectory)
                {
                    // Create directory if needed
                    if (!DoesDirExist(destPath.c_str()))
                    {
                        SYS_CreateDirectory(destPath.c_str());
                    }
                    // Recurse
                    copyRecursive(srcPath, relativePath);
                }
                else
                {
                    // Skip if destination already exists (don't overwrite)
                    if (!SYS_DoesFileExist(destPath.c_str(), false))
                    {
                        // Ensure parent directory exists
                        size_t lastSlash = destPath.find_last_of("/\\");
                        if (lastSlash != std::string::npos)
                        {
                            std::string parentDir = destPath.substr(0, lastSlash);
                            if (!DoesDirExist(parentDir.c_str()))
                            {
                                SYS_CreateDirectory(parentDir.c_str());
                            }
                        }

                        SYS_CopyFile(srcPath.c_str(), destPath.c_str());
                    }
                }
            }

            SYS_IterateDirectory(dirEntry);
        }
        SYS_CloseDirectory(dirEntry);
    };

    copyRecursive(addonPath, "");

    return true;
}

bool AddonManager::InstallAddon(const std::string& addonCachePath, const std::string& addonId, std::string& outError)
{
    const std::string& projectDir = GetEngineState()->mProjectDirectory;
    if (projectDir.empty())
    {
        outError = "No project loaded";
        return false;
    }

    // Install to Packages/{addonId}/
    std::string packagesDir = projectDir + "Packages";
    if (!DoesDirExist(packagesDir.c_str()))
    {
        SYS_CreateDirectory(packagesDir.c_str());
    }

    std::string destDir = packagesDir + "/" + addonId;
    if (DoesDirExist(destDir.c_str()))
    {
        SYS_RemoveDirectory(destDir.c_str());
    }
    SYS_CreateDirectory(destDir.c_str());
    SYS_CopyDirectoryRecursive(addonCachePath.c_str(), destDir.c_str());

    // Record installation
    const Addon* addon = FindAddon(addonId);
    InstalledAddon installed;
    installed.mId = addonId;
    installed.mVersion = addon ? addon->mMetadata.mVersion : "1.0.0";
    installed.mInstalledDate = GetCurrentTimestamp();
    installed.mRepoUrl = addon ? addon->mRepoUrl : "";

    // Remove if already installed (upgrade)
    for (auto it = mInstalledAddons.begin(); it != mInstalledAddons.end(); ++it)
    {
        if (it->mId == addonId)
        {
            mInstalledAddons.erase(it);
            break;
        }
    }

    mInstalledAddons.push_back(installed);
    SaveInstalledAddons();

    // Update available addons list
    for (Addon& availAddon : mAvailableAddons)
    {
        if (availAddon.mMetadata.mId == addonId)
        {
            availAddon.mIsInstalled = true;
            availAddon.mInstalledVersion = installed.mVersion;
            break;
        }
    }

    return true;
}

bool AddonManager::UninstallAddon(const std::string& addonId)
{
    for (auto it = mInstalledAddons.begin(); it != mInstalledAddons.end(); ++it)
    {
        if (it->mId == addonId)
        {
            // Remove addon files from Packages/
            const std::string& projectDir = GetEngineState()->mProjectDirectory;
            if (!projectDir.empty())
            {
                std::string addonDir = projectDir + "Packages/" + addonId;
                if (DoesDirExist(addonDir.c_str()))
                {
                    SYS_RemoveDirectory(addonDir.c_str());
                }
            }

            mInstalledAddons.erase(it);
            SaveInstalledAddons();

            // Update available addons list
            for (Addon& addon : mAvailableAddons)
            {
                if (addon.mMetadata.mId == addonId)
                {
                    addon.mIsInstalled = false;
                    addon.mInstalledVersion.clear();
                    break;
                }
            }

            return true;
        }
    }

    return false;
}

void AddonManager::LoadInstalledAddons()
{
    mInstalledAddons.clear();

    std::string installedPath = GetInstalledAddonsPath();
    if (installedPath.empty() || !SYS_DoesFileExist(installedPath.c_str(), false))
    {
        return;
    }

    Stream stream;
    if (!stream.ReadFile(installedPath.c_str(), false))
    {
        return;
    }

    std::string jsonStr(stream.GetData(), stream.GetSize());
    rapidjson::Document doc;
    doc.Parse(jsonStr.c_str());

    if (doc.HasParseError())
    {
        return;
    }

    if (doc.HasMember("addons") && doc["addons"].IsArray())
    {
        const rapidjson::Value& addons = doc["addons"];
        for (rapidjson::SizeType i = 0; i < addons.Size(); ++i)
        {
            const rapidjson::Value& addonObj = addons[i];
            InstalledAddon installed;

            if (addonObj.HasMember("id") && addonObj["id"].IsString())
            {
                installed.mId = addonObj["id"].GetString();
            }
            if (addonObj.HasMember("version") && addonObj["version"].IsString())
            {
                installed.mVersion = addonObj["version"].GetString();
            }
            if (addonObj.HasMember("installed") && addonObj["installed"].IsString())
            {
                installed.mInstalledDate = addonObj["installed"].GetString();
            }
            if (addonObj.HasMember("repoUrl") && addonObj["repoUrl"].IsString())
            {
                installed.mRepoUrl = addonObj["repoUrl"].GetString();
            }
            if (addonObj.HasMember("enabled") && addonObj["enabled"].IsBool())
            {
                installed.mEnabled = addonObj["enabled"].GetBool();
            }
            if (addonObj.HasMember("enableNative") && addonObj["enableNative"].IsBool())
            {
                installed.mEnableNative = addonObj["enableNative"].GetBool();
            }

            if (!installed.mId.empty())
            {
                mInstalledAddons.push_back(installed);
            }
        }
    }
}

void AddonManager::SaveInstalledAddons()
{
    std::string installedPath = GetInstalledAddonsPath();
    if (installedPath.empty())
    {
        return;
    }

    // Ensure Settings directory exists
    std::string settingsDir = GetEngineState()->mProjectDirectory + "Settings";
    if (!DoesDirExist(settingsDir.c_str()))
    {
        SYS_CreateDirectory(settingsDir.c_str());
    }

    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

    doc.AddMember("version", 1, allocator);

    rapidjson::Value addonsArray(rapidjson::kArrayType);
    for (const InstalledAddon& installed : mInstalledAddons)
    {
        rapidjson::Value addonObj(rapidjson::kObjectType);
        addonObj.AddMember("id", rapidjson::Value(installed.mId.c_str(), allocator), allocator);
        addonObj.AddMember("version", rapidjson::Value(installed.mVersion.c_str(), allocator), allocator);
        addonObj.AddMember("installed", rapidjson::Value(installed.mInstalledDate.c_str(), allocator), allocator);
        addonObj.AddMember("repoUrl", rapidjson::Value(installed.mRepoUrl.c_str(), allocator), allocator);
        addonObj.AddMember("enabled", installed.mEnabled, allocator);
        addonObj.AddMember("enableNative", installed.mEnableNative, allocator);
        addonsArray.PushBack(addonObj, allocator);
    }
    doc.AddMember("addons", addonsArray, allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    Stream stream(buffer.GetString(), (uint32_t)buffer.GetSize());
    stream.WriteFile(installedPath.c_str());
}

bool AddonManager::IsAddonInstalled(const std::string& addonId) const
{
    for (const InstalledAddon& installed : mInstalledAddons)
    {
        if (installed.mId == addonId)
        {
            return true;
        }
    }
    return false;
}

bool AddonManager::HasUpdate(const std::string& addonId) const
{
    const Addon* addon = FindAddon(addonId);
    if (addon == nullptr || !addon->mIsInstalled)
    {
        return false;
    }

    std::string installedVersion = GetInstalledVersion(addonId);
    return !installedVersion.empty() && installedVersion != addon->mMetadata.mVersion;
}

std::string AddonManager::GetInstalledVersion(const std::string& addonId) const
{
    for (const InstalledAddon& installed : mInstalledAddons)
    {
        if (installed.mId == addonId)
        {
            return installed.mVersion;
        }
    }
    return "";
}

const Addon* AddonManager::FindAddon(const std::string& addonId) const
{
    for (const Addon& addon : mAvailableAddons)
    {
        if (addon.mMetadata.mId == addonId)
        {
            return &addon;
        }
    }
    return nullptr;
}

#endif
