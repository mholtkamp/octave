#pragma once

#if EDITOR

#include <string>
#include <vector>
#include <stdint.h>

/**
 * @brief Specifies where native addon code runs.
 */
enum class NativeAddonTarget
{
    EditorOnly,     // Only loads in editor, NOT compiled into final builds
    EngineAndEditor // Loads in editor AND compiled into final builds
};

/**
 * @brief Native module configuration for addons with C++ code.
 */
struct NativeModuleMetadata
{
    bool mHasNative = false;           // Whether addon has native code
    NativeAddonTarget mTarget = NativeAddonTarget::EngineAndEditor;  // Where code runs
    std::string mSourceDir = "Source"; // Relative path to source directory
    std::string mBinaryName;           // Output binary name (without extension)
    std::string mEntrySymbol = "OctavePlugin_GetDesc";
    uint32_t mPluginApiVersion = 1;
};

/**
 * @brief Metadata for a project template or addon.
 */
struct ContentMetadata
{
    std::string mId;           // Unique ID (directory name)
    std::string mName;
    std::string mAuthor;
    std::string mDescription;
    std::string mUrl;          // Source URL (GitHub, etc.)
    std::string mVersion;
    std::string mUpdated;      // ISO date string
    std::vector<std::string> mTags;
    bool mIsCpp = false;       // C++ or Lua (templates only)
};

/**
 * @brief Represents an installed template.
 */
struct Template
{
    ContentMetadata mMetadata;
    std::string mPath;         // Full path to template directory
    bool mHasThumbnail = false;
};

/**
 * @brief Represents an addon available from a repository.
 */
struct Addon
{
    ContentMetadata mMetadata;
    std::string mRepoUrl;      // Repository URL this addon came from
    bool mHasThumbnail = false;
    bool mIsInstalled = false;
    bool mIsMain = true;
    bool mIsStandalone = false; // Addon is entire repo, not a subdirectory
    std::string mInstalledVersion;
    NativeModuleMetadata mNative;  // Native module configuration
};

/**
 * @brief Represents an addon repository.
 */
struct AddonRepository
{
    std::string mName;
    std::string mUrl;          // Base URL (GitHub repo URL)
    std::vector<std::string> mAddonIds;  // List of addon IDs in this repo
};

/**
 * @brief Represents an installed addon in a project.
 */
struct InstalledAddon
{
    std::string mId;
    std::string mVersion;
    std::string mInstalledDate;  // ISO date string
    std::string mRepoUrl;
    bool mEnabled = true;        // Whether addon is enabled
    bool mEnableNative = true;   // Whether native code should be loaded
};

#endif
