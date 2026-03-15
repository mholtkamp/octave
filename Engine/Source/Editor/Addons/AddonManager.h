#pragma once

#if EDITOR

#include "../ProjectSelect/TemplateData.h"
#include <string>
#include <vector>

/**
 * @brief Singleton manager for addon repositories and installed addons.
 *
 * Manages addon discovery from configured repositories, download/installation
 * to the current project, and tracking of installed addons for updates.
 */
class AddonManager
{
public:
    static void Create();
    static void Destroy();
    static AddonManager* Get();

    /** @brief Directory for addon cache: {AppData}/OctaveEditor/AddonCache/ */
    std::string GetAddonCacheDirectory();

    /** @brief Settings file: {AppData}/OctaveEditor/addons.json */
    std::string GetSettingsPath();

    /** @brief Installed addons file in project: {ProjectDir}/Settings/installed_addons.json */
    std::string GetInstalledAddonsPath();

    // Repository management
    void LoadSettings();
    void SaveSettings();
    void AddRepository(const std::string& url);
    void RemoveRepository(const std::string& url);
    const std::vector<AddonRepository>& GetRepositories() const { return mRepositories; }

    // Addon discovery
    void RefreshAllRepositories();
    void RefreshRepository(const std::string& url);
    const std::vector<Addon>& GetAvailableAddons() const { return mAvailableAddons; }

    // Installation
    bool DownloadAddon(const Addon& addon, std::string& outError);
    bool InstallAddon(const std::string& addonCachePath, const std::string& addonId, std::string& outError);
    bool UninstallAddon(const std::string& addonId);

    // Tracking
    void LoadInstalledAddons();
    void SaveInstalledAddons();
    const std::vector<InstalledAddon>& GetInstalledAddons() const { return mInstalledAddons; }
    bool IsAddonInstalled(const std::string& addonId) const;
    bool HasUpdate(const std::string& addonId) const;
    std::string GetInstalledVersion(const std::string& addonId) const;

    // Find addon by ID
    const Addon* FindAddon(const std::string& addonId) const;

private:
    static AddonManager* sInstance;
    AddonManager();
    ~AddonManager();

    /** @brief Ensure cache directory exists */
    void EnsureCacheDirectory();
    bool FetchRepositoryManifest(const std::string& url, AddonRepository& outRepo, const std::string& branch);
    /** @brief Fetch addon metadata from addon's package.json */
    bool FetchAddonMetadata(const std::string& repoUrl, const std::string& addonId, Addon& outAddon, const std::string& branch);

    /** @brief Merge addon files into current project */
    bool MergeAddonIntoProject(const std::string& addonPath, std::string& outError);

    /** @brief Download a file from URL */
    bool DownloadFile(const std::string& url, const std::string& destPath, std::string& outError);

    /** @brief Extract a zip file */
    bool ExtractZip(const std::string& zipPath, const std::string& destDir, std::string& outError);

    std::string NormalizePath(const std::string& in);

    /** @brief Convert GitHub URL to raw content URL */
    std::string ConvertToRawUrl(const std::string& gitHubUrl, const std::string& filePath, const std::string& branch);

    /** @brief Convert GitHub URL to download URL */
    std::string ConvertToDownloadUrl(const std::string& gitHubUrl, const std::string& branch);

    /** @brief Get current timestamp as ISO string */
    std::string GetCurrentTimestamp();

    std::vector<AddonRepository> mRepositories;
    std::vector<Addon> mAvailableAddons;
    std::vector<InstalledAddon> mInstalledAddons;
};

#endif
