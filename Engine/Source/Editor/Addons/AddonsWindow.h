#pragma once

#if EDITOR

#include <string>
#include <vector>
#include <unordered_map>

#include "imgui.h"

struct Addon;
class Image;

/**
 * @brief Window for browsing and installing addons.
 *
 * Provides:
 * - Browse addons from configured repositories
 * - View installed addons
 * - Manage addon repositories
 */
class AddonsWindow
{
public:
    AddonsWindow();
    ~AddonsWindow();

    void Open();
    void Close();
    void Draw();
    bool IsOpen() const { return mIsOpen; }

private:
    void DrawAddonBrowser();
    void DrawInstalledAddons();
    void DrawRepositoryManager();
    void DrawAddonCard(const Addon& addon, float cardWidth);
    void DrawAddonDetailsPopup();
    void DrawAddRepoPopup();

    void OnDownloadAddon(const std::string& addonId);
    void OnViewMore(const std::string& addonId);
    void OnUninstallAddon(const std::string& addonId);
    void OnAddRepository();
    void OnRemoveRepository(const std::string& url);
    void OnRefreshRepositories();

    // Native addon operations
    void OnBuildNativeAddon(const std::string& addonId);
    void OnReloadNativeAddon(const std::string& addonId);
    void OnToggleNativeEnabled(const std::string& addonId);

    bool mIsOpen = false;
    int mSelectedTab = 0;  // 0=Browse, 1=Installed, 2=Repositories

    // Popup state
    bool mShowAddonDetails = false;
    bool mShowAddRepoPopup = false;
    std::string mSelectedAddonId;
    char mRepoUrlBuffer[512] = {};
    std::string mErrorMessage;
    std::string mStatusMessage;

    // Filter
    char mSearchBuffer[256] = {};
    std::vector<std::string> mSelectedTags;

    // Available tags (collected from addons)
    std::vector<std::string> mAvailableTags;

    // Refresh state
    bool mNeedsRefresh = true;
    bool mIsRefreshing = false;

    // Uninstall confirmation
    bool mShowUninstallConfirm = false;
    std::string mUninstallAddonId;
    void DrawUninstallConfirmPopup();

    // Build log popup
    bool mShowBuildLog = false;
    std::string mBuildLogAddonId;

    // Thumbnail cache
    struct ThumbnailEntry
    {
        ImTextureID mTexId = 0;
        Image* mImage = nullptr;
    };
    ImTextureID GetAddonThumbnail(const std::string& addonId);
    void ClearThumbnailCache();
    std::unordered_map<std::string, ThumbnailEntry> mThumbnailCache;
};

AddonsWindow* GetAddonsWindow();

#endif
