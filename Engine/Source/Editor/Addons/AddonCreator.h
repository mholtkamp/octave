#pragma once

/**
 * @file AddonCreator.h
 * @brief Business logic for addon creation, initialization, publishing, and package.json editing.
 */

#if EDITOR

#include "NativeAddonManager.h"

#include <string>
#include <vector>

/**
 * @brief Parameters for creating a template or script addon.
 */
struct AddonCreateInfo
{
    std::string mName;
    std::string mId;           // Auto-generated from name if empty
    std::string mAuthor;
    std::string mDescription;
    std::string mVersion = "1.0.0";
    std::string mType;         // "template" or "addon"
};

/**
 * @brief Full package.json data for editing.
 */
struct PackageJsonData
{
    std::string mName;
    std::string mAuthor;
    std::string mDescription;
    std::string mVersion;
    std::string mUrl;
    std::string mType;         // "template", "addon", or empty
    std::string mTags;         // Comma-separated

    // Native section
    bool mHasNative = false;
    std::string mNativeTarget; // "editor" or "engine"
    std::string mSourceDir = "Source";
    std::string mBinaryName;
    std::string mEntrySymbol = "OctavePlugin_GetDesc";
    int mApiVersion = 1;
};

/**
 * @brief Info about a user-created addon for selector UI.
 */
struct UserAddonInfo
{
    std::string mId;
    std::string mName;
    std::string mPath;
    std::string mType;         // "template", "addon", "native", or empty
};

/**
 * @brief Business logic for all addon operations (non-UI).
 */
namespace AddonCreator
{
    // ===== Creation =====

    /**
     * @brief Create a template package (package.json + Assets/ + Scripts/).
     */
    bool CreateTemplate(const AddonCreateInfo& info, const std::string& targetDir, std::string& outError);

    /**
     * @brief Create a script addon package (package.json + Scripts/).
     */
    bool CreateAddon(const AddonCreateInfo& info, const std::string& targetDir, std::string& outError);

    /**
     * @brief Create a native C++ addon at a custom target directory.
     * Delegates to NativeAddonManager::CreateNativeAddonAtPath().
     */
    bool CreateNativeAddon(const NativeAddonCreateInfo& info, const std::string& targetDir,
                           std::string& outError, std::string* outPath = nullptr);

    // ===== Initialization =====

    /**
     * @brief Initialize current project as a template (writes package.json at project root).
     */
    bool InitializeAsTemplate(const std::string& projectDir, std::string& outError);

    /**
     * @brief Initialize current project as a script addon.
     */
    bool InitializeAsAddon(const std::string& projectDir, std::string& outError);

    /**
     * @brief Initialize current project as a native C++ addon.
     */
    bool InitializeAsNativeAddon(const std::string& projectDir, std::string& outError);

    // ===== Publishing =====

    /**
     * @brief Publish addon via git (add, commit, optionally push).
     */
    bool PublishViaGit(const std::string& addonDir, const std::string& commitMessage,
                       bool push, std::string& outError, std::string& outLog);

    /**
     * @brief Package addon as a zip file.
     */
    bool PublishAsZip(const std::string& addonDir, const std::string& outputPath, std::string& outError);

    // ===== Package.json Operations =====

    /**
     * @brief Read package.json into editable data.
     */
    bool ReadPackageJson(const std::string& path, PackageJsonData& outData, std::string& outError);

    /**
     * @brief Write editable data back to package.json, preserving unknown fields.
     */
    bool WritePackageJson(const std::string& path, const PackageJsonData& data, std::string& outError);

    /**
     * @brief Open a file with the system default editor.
     */
    void OpenInExternalEditor(const std::string& path);

    // ===== Utilities =====

    /**
     * @brief Generate kebab-case ID from display name.
     */
    std::string GenerateIdFromName(const std::string& name);

    /**
     * @brief Get list of user-created addons (not downloaded from repos).
     *
     * Scans {ProjectDir}/Packages/ for package.json files and excludes
     * any addon whose ID appears in installed_addons.json.
     */
    std::vector<UserAddonInfo> GetUserCreatedAddons(const std::string& projectDir);

    /**
     * @brief Check if git is available on the system.
     */
    bool IsGitAvailable();

    /**
     * @brief Check if a directory has a .git folder.
     */
    bool HasGitRepo(const std::string& dir);

    /**
     * @brief Initialize a git repository in a directory.
     */
    bool InitGitRepo(const std::string& dir, std::string& outError);

    /**
     * @brief Check if a git repo has a remote configured.
     */
    bool HasGitRemote(const std::string& dir);
};

#endif // EDITOR
