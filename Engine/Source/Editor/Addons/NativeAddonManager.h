#pragma once

/**
 * @file NativeAddonManager.h
 * @brief Manages native addon lifecycle including discovery, building, loading, and unloading.
 */

#if EDITOR

#include "ProjectSelect/TemplateData.h"
#include "Plugins/OctaveEngineAPI.h"
#include "Plugins/OctavePluginAPI.h"

#include <string>
#include <unordered_map>
#include <vector>

/**
 * @brief Parameters for creating a new native addon.
 */
struct NativeAddonCreateInfo
{
    std::string mName;               // Display name (e.g., "My Addon")
    std::string mId;                 // Internal ID (e.g., "my-addon", auto-generated from name if empty)
    std::string mAuthor;
    std::string mDescription;
    std::string mVersion = "1.0.0";
    NativeAddonTarget mTarget = NativeAddonTarget::EngineAndEditor;
    std::string mBinaryName;         // Auto-generated from ID if empty
};

/**
 * @brief Options for packaging a native addon.
 */
struct NativeAddonPackageOptions
{
    std::string mAddonId;
    bool mIncludeSource = true;
    bool mIncludeAssets = true;
    bool mIncludeScripts = true;
    bool mIncludeThumbnail = true;
    std::string mOutputPath;         // Full path to output zip file
};

/**
 * @brief Runtime state for a native addon.
 */
struct NativeAddonState
{
    std::string mAddonId;
    std::string mSourcePath;      // Path to addon source (local Packages/ or cache)
    std::string mLoadedPath;      // Path to loaded DLL/SO
    void* mModuleHandle = nullptr;
    std::string mFingerprint;     // Hash for rebuild detection

    // Build state
    bool mBuildInProgress = false;
    bool mBuildSucceeded = false;
    std::string mBuildLog;
    std::string mBuildError;

    // Plugin descriptor (after load)
    OctavePluginDesc mDesc = {};
    bool mDescValid = false;

    // Native metadata from package.json
    NativeModuleMetadata mNativeMetadata;
};

/**
 * @brief Singleton manager for native addon lifecycle.
 *
 * Handles:
 * - Discovery from local Packages/ and installed addons
 * - Fingerprint computation for rebuild detection
 * - Building native addons
 * - Loading, unloading, and hot-reloading
 */
class NativeAddonManager
{
public:
    static void Create();
    static void Destroy();
    static NativeAddonManager* Get();

    // ===== Discovery =====

    /**
     * @brief Discover all native addons from Packages/ and installed addons.
     */
    void DiscoverNativeAddons();

    /**
     * @brief Get list of discovered addon IDs.
     */
    std::vector<std::string> GetDiscoveredAddonIds() const;

    // ===== Build Operations =====

    /**
     * @brief Build a native addon.
     *
     * @param addonId Addon to build
     * @param outError Error message on failure
     * @return true if build succeeded
     */
    bool BuildNativeAddon(const std::string& addonId, std::string& outError);

    /**
     * @brief Compute fingerprint hash of source files.
     *
     * @param addonId Addon to compute fingerprint for
     * @return Fingerprint string, or empty on error
     */
    std::string ComputeFingerprint(const std::string& addonId);

    /**
     * @brief Check if addon needs rebuild.
     *
     * @param addonId Addon to check
     * @return true if source has changed since last build
     */
    bool NeedsBuild(const std::string& addonId);

    // ===== Load/Unload Operations =====

    /**
     * @brief Load a native addon.
     *
     * @param addonId Addon to load
     * @param outError Error message on failure
     * @return true if load succeeded
     */
    bool LoadNativeAddon(const std::string& addonId, std::string& outError);

    /**
     * @brief Unload a native addon.
     *
     * @param addonId Addon to unload
     * @return true if unload succeeded
     */
    bool UnloadNativeAddon(const std::string& addonId);

    /**
     * @brief Reload a native addon (unload, build if needed, load).
     *
     * @param addonId Addon to reload
     * @param outError Error message on failure
     * @return true if reload succeeded
     */
    bool ReloadNativeAddon(const std::string& addonId, std::string& outError);

    /**
     * @brief Reload all native addons.
     *
     * Discovers, builds changed addons, and reloads all.
     */
    void ReloadAllNativeAddons();

    /**
     * @brief Tick all loaded plugins (gameplay tick).
     *
     * Calls the Tick callback for each loaded plugin that has one.
     * Should be called only when playing (PIE or built game).
     *
     * @param deltaTime Time elapsed since last frame in seconds
     */
    void TickAllPlugins(float deltaTime);

    /**
     * @brief Tick all loaded plugins (editor tick).
     *
     * Calls the TickEditor callback for each loaded plugin that has one.
     * Should be called every frame in the editor regardless of play state.
     *
     * @param deltaTime Time elapsed since last frame in seconds
     */
    void TickEditorAllPlugins(float deltaTime);

    /**
     * @brief Call OnEditorPreInit on all loaded plugins that implement it.
     */
    void CallOnEditorPreInit();

    /**
     * @brief Call OnEditorReady on all loaded plugins that implement it.
     */
    void CallOnEditorReady();

    // ===== State Queries =====

    /**
     * @brief Get state for an addon.
     *
     * @param addonId Addon to query
     * @return Pointer to state, or nullptr if not found
     */
    const NativeAddonState* GetState(const std::string& addonId) const;

    /**
     * @brief Check if addon is currently loaded.
     */
    bool IsLoaded(const std::string& addonId) const;

    /**
     * @brief Get source path for an addon.
     *
     * @param addonId Addon to query
     * @return Source path (local Packages/ or cache), or empty if not found
     */
    std::string GetAddonSourcePath(const std::string& addonId) const;

    /**
     * @brief Get all addons with engine target (for final build injection).
     */
    std::vector<NativeAddonState> GetEngineAddons() const;

    /**
     * @brief Get the engine API struct for plugins.
     */
    OctaveEngineAPI* GetEngineAPI() { return &mEngineAPI; }

    // ===== Creation and Packaging =====

    /**
     * @brief Create a new native addon with folder structure and template files.
     *
     * Creates:
     * - {ProjectDir}/Packages/{addonId}/
     *     - package.json
     *     - Source/{AddonName}.cpp (template)
     *     - .vscode/c_cpp_properties.json
     *
     * @param info Creation parameters
     * @param outError Error message on failure
     * @param outPath Output path to the created addon folder (optional)
     * @return true if creation succeeded
     */
    bool CreateNativeAddon(const NativeAddonCreateInfo& info, std::string& outError, std::string* outPath = nullptr);

    /**
     * @brief Create a native addon at a custom target directory.
     *
     * Same as CreateNativeAddon() but creates the addon in the specified
     * directory instead of the project's Packages/ folder.
     *
     * @param info Creation parameters
     * @param targetDir Directory where the addon folder will be created
     * @param outError Error message on failure
     * @param outPath Output path to the created addon folder (optional)
     * @return true if creation succeeded
     */
    bool CreateNativeAddonAtPath(const NativeAddonCreateInfo& info, const std::string& targetDir,
                                  std::string& outError, std::string* outPath = nullptr);

    /**
     * @brief Package a native addon for distribution.
     *
     * Creates a zip file containing the addon contents.
     *
     * @param options Packaging options
     * @param outError Error message on failure
     * @return true if packaging succeeded
     */
    bool PackageNativeAddon(const NativeAddonPackageOptions& options, std::string& outError);

    /**
     * @brief Generate IDE configuration files for an addon.
     *
     * Creates .vscode/c_cpp_properties.json and optionally CMakeLists.txt
     *
     * @param addonPath Path to addon root folder
     * @return true if generation succeeded
     */
    bool GenerateIDEConfig(const std::string& addonPath);

    /**
     * @brief Get list of local package addon IDs (in Packages/ folder).
     */
    std::vector<std::string> GetLocalPackageIds() const;

    /**
     * @brief Generate the AddonIncludes.json manifest file.
     *
     * Creates Engine/Generated/AddonIncludes.json with all include paths
     * that native addons need for compilation and IDE support.
     *
     * @return true if generation succeeded
     */
    static bool GenerateAddonIncludesManifest();

    /**
     * @brief Load the AddonIncludes.json manifest.
     *
     * @param outIncludePaths Output: list of relative include paths
     * @param outDefines Output: list of preprocessor defines
     * @return true if manifest was loaded successfully
     */
    static bool LoadAddonIncludesManifest(std::vector<std::string>& outIncludePaths,
                                           std::vector<std::string>& outDefines);

private:
    static NativeAddonManager* sInstance;
    NativeAddonManager();
    ~NativeAddonManager();

    // Discovery helpers
    void ScanLocalPackages();
    void ScanInstalledAddons();
    bool ParsePackageJson(const std::string& path, NativeModuleMetadata& outMetadata);

    // Build helpers
    std::string GetIntermediateDir(const std::string& addonId);
    std::string GetOutputPath(const std::string& addonId, const std::string& fingerprint);
    bool GenerateBuildScript(const std::string& addonId, const std::string& outputDir,
                             const std::string& outputPath, std::string& outScriptPath);
    std::vector<std::string> GatherSourceFiles(const std::string& sourceDir);

    // Engine API setup
    void InitializeEngineAPI();

    // Creation helpers
    std::string GenerateIdFromName(const std::string& name);
    bool WriteTemplateSourceFile(const std::string& path, const std::string& addonName,
                                  const std::string& binaryName);
    bool WritePackageJson(const std::string& path, const NativeAddonCreateInfo& info);
    bool WriteVSCodeConfig(const std::string& addonPath);
    bool WriteCMakeLists(const std::string& addonPath, const std::string& binaryName);
    bool WriteVSProject(const std::string& addonPath, const std::string& addonName,
                        const std::string& binaryName);

    std::unordered_map<std::string, NativeAddonState> mStates;
    OctaveEngineAPI mEngineAPI;
};

#endif // EDITOR
