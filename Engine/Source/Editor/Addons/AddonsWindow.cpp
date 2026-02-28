#if EDITOR

#include "AddonsWindow.h"
#include "AddonsMenu.h"
#include "AddonManager.h"
#include "NativeAddonManager.h"
#include "../ProjectSelect/TemplateData.h"

#include "Engine.h"
#include "Log.h"
#include "System/System.h"
#include "AssetManager.h"
#include "AssetDir.h"
#include "EditorState.h"

#include "imgui.h"

#if API_VULKAN
#include "Graphics/Vulkan/Image.h"
#include "Graphics/Vulkan/VulkanUtils.h"
#include "backends/imgui_impl_vulkan.h"
#include <stb_image.h>
#endif

#include <algorithm>

static AddonsWindow sAddonsWindow;

AddonsWindow* GetAddonsWindow()
{
    return &sAddonsWindow;
}

AddonsWindow::AddonsWindow()
{
    memset(mSearchBuffer, 0, sizeof(mSearchBuffer));
    memset(mRepoUrlBuffer, 0, sizeof(mRepoUrlBuffer));
}

AddonsWindow::~AddonsWindow()
{
    ClearThumbnailCache();
}

void AddonsWindow::ClearThumbnailCache()
{
#if API_VULKAN
    if (!mThumbnailCache.empty())
    {
        DeviceWaitIdle();
        for (auto& pair : mThumbnailCache)
        {
            if (pair.second.mTexId != 0)
            {
                ImGui_ImplVulkan_RemoveTexture((VkDescriptorSet)pair.second.mTexId);
            }
            if (pair.second.mImage != nullptr)
            {
                GetDestroyQueue()->Destroy(pair.second.mImage);
            }
        }
        mThumbnailCache.clear();
    }
#endif
}

ImTextureID AddonsWindow::GetAddonThumbnail(const std::string& addonId)
{
    // Check cache first
    auto it = mThumbnailCache.find(addonId);
    if (it != mThumbnailCache.end())
    {
        return it->second.mTexId;
    }

#if API_VULKAN
    // Try installed path first: {ProjectDir}/Packages/{addonId}/thumbnail.png
    std::string thumbPath;
    const std::string& projDir = GetEngineState()->mProjectDirectory;
    if (!projDir.empty())
    {
        thumbPath = projDir + "Packages/" + addonId + "/thumbnail.png";
        if (!SYS_DoesFileExist(thumbPath.c_str(), false))
        {
            thumbPath.clear();
        }
    }

    // Try addon cache: {CacheDir}/{addonId}/thumbnail.png
    if (thumbPath.empty())
    {
        AddonManager* am = AddonManager::Get();
        if (am != nullptr)
        {
            std::string cachePath = am->GetAddonCacheDirectory() + "/" + addonId + "/thumbnail.png";
            if (SYS_DoesFileExist(cachePath.c_str(), false))
            {
                thumbPath = cachePath;
            }
        }
    }

    if (thumbPath.empty())
    {
        mThumbnailCache[addonId] = {};
        return 0;
    }

    // Load with stb_image
    int width, height, channels;
    stbi_uc* pixels = stbi_load(thumbPath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (pixels == nullptr)
    {
        mThumbnailCache[addonId] = {};
        return 0;
    }

    // Create Vulkan image
    ImageDesc imgDesc;
    imgDesc.mWidth = width;
    imgDesc.mHeight = height;
    imgDesc.mFormat = VK_FORMAT_R8G8B8A8_UNORM;
    imgDesc.mUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imgDesc.mMipLevels = 1;
    imgDesc.mLayers = 1;

    SamplerDesc sampDesc;
    sampDesc.mMagFilter = VK_FILTER_LINEAR;
    sampDesc.mMinFilter = VK_FILTER_LINEAR;
    sampDesc.mAddressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

    Image* image = new Image(imgDesc, sampDesc, "AddonThumbnail");
    image->Update(pixels);
    image->Transition(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    stbi_image_free(pixels);

    ImTextureID texId = (ImTextureID)ImGui_ImplVulkan_AddTexture(
        image->GetSampler(),
        image->GetView(),
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    ThumbnailEntry entry;
    entry.mTexId = texId;
    entry.mImage = image;
    mThumbnailCache[addonId] = entry;
    return texId;
#else
    mThumbnailCache[addonId] = {};
    return 0;
#endif
}

void AddonsWindow::Open()
{
    mIsOpen = true;
    mSelectedTab = 0;
    mShowAddonDetails = false;
    mShowAddRepoPopup = false;
    mSelectedAddonId.clear();
    mErrorMessage.clear();
    mStatusMessage.clear();

    // Load installed addons when opening
    AddonManager* am = AddonManager::Get();
    if (am != nullptr)
    {
        am->LoadInstalledAddons();

        // Refresh on first open
        if (mNeedsRefresh)
        {
            OnRefreshRepositories();
            mNeedsRefresh = false;
        }
    }
}

void AddonsWindow::Close()
{
    mIsOpen = false;
    ClearThumbnailCache();
}

void AddonsWindow::Draw()
{
    if (!mIsOpen)
    {
        return;
    }

    // Check if project is loaded
    if (GetEngineState()->mProjectPath.empty())
    {
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 windowSize(400.0f, 150.0f);
        ImVec2 windowPos((io.DisplaySize.x - windowSize.x) * 0.5f, (io.DisplaySize.y - windowSize.y) * 0.5f);
        ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

        if (ImGui::Begin("Addons", &mIsOpen, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
        {
            ImGui::TextWrapped("Please open a project before browsing addons.");
            ImGui::TextWrapped("Addons are installed into the current project.");

            ImGui::Spacing();
            if (ImGui::Button("Close", ImVec2(80, 0)))
            {
                Close();
            }
        }
        ImGui::End();
        return;
    }

    // Center the window
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 windowSize(750.0f, 550.0f);
    ImVec2 windowPos((io.DisplaySize.x - windowSize.x) * 0.5f, (io.DisplaySize.y - windowSize.y) * 0.5f);
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar;

    if (ImGui::Begin("Addons", &mIsOpen, windowFlags))
    {
        if (ImGui::BeginMenuBar())
        {
            DrawAddonsMenuBar();
            ImGui::EndMenuBar();
        }

        // Tab bar
        if (ImGui::BeginTabBar("AddonsTabs"))
        {
            if (ImGui::BeginTabItem("Browse Addons"))
            {
                mSelectedTab = 0;
                DrawAddonBrowser();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Installed"))
            {
                mSelectedTab = 1;
                DrawInstalledAddons();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Repositories"))
            {
                mSelectedTab = 2;
                DrawRepositoryManager();
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }
    ImGui::End();

    // Draw popups
    DrawAddonDetailsPopup();
    DrawAddRepoPopup();
    DrawUninstallConfirmPopup();

    // Draw build log popup
    if (mShowBuildLog)
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                                ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(600, 400));

        if (ImGui::Begin("Build Log", &mShowBuildLog,
                         ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
        {
            NativeAddonManager* nam = NativeAddonManager::Get();
            const NativeAddonState* state = nam ? nam->GetState(mBuildLogAddonId) : nullptr;

            ImGui::Text("Build Log: %s", mBuildLogAddonId.c_str());
            ImGui::Separator();

            if (state)
            {
                if (state->mBuildSucceeded)
                {
                    ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "Build Succeeded");
                }
                else if (!state->mBuildError.empty())
                {
                    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Build Failed");
                }

                ImGui::Spacing();

                // Scrollable log content
                ImGui::BeginChild("LogContent", ImVec2(0, -30), true);
                ImGui::TextWrapped("%s", state->mBuildLog.c_str());
                ImGui::EndChild();
            }
            else
            {
                ImGui::TextDisabled("No build log available.");
            }

            if (ImGui::Button("Close", ImVec2(80, 0)))
            {
                mShowBuildLog = false;
            }
        }
        ImGui::End();
    }
}

void AddonsWindow::DrawAddonBrowser()
{
    AddonManager* am = AddonManager::Get();
    if (am == nullptr)
    {
        ImGui::TextDisabled("Addon manager not initialized.");
        return;
    }

    // Search and refresh bar
    ImGui::SetNextItemWidth(300);
    ImGui::InputTextWithHint("##Search", "Search addons...", mSearchBuffer, sizeof(mSearchBuffer));

    ImGui::SameLine(ImGui::GetWindowWidth() - 100);
    if (ImGui::Button("Refresh", ImVec2(80, 0)))
    {
        OnRefreshRepositories();
    }

    // Status message
    if (!mStatusMessage.empty())
    {
        ImGui::TextColored(ImVec4(0.5f, 0.8f, 0.5f, 1.0f), "%s", mStatusMessage.c_str());
    }

    ImGui::Separator();
    ImGui::Spacing();

    const std::vector<Addon>& addons = am->GetAvailableAddons();

    if (addons.empty())
    {
        ImGui::TextDisabled("No addons found.");
        ImGui::TextDisabled("Click 'Refresh' to fetch addons from repositories.");
        return;
    }

    // Collect available tags
    mAvailableTags.clear();
    for (const Addon& addon : addons)
    {
        for (const std::string& tag : addon.mMetadata.mTags)
        {
            if (std::find(mAvailableTags.begin(), mAvailableTags.end(), tag) == mAvailableTags.end())
            {
                mAvailableTags.push_back(tag);
            }
        }
    }
    std::sort(mAvailableTags.begin(), mAvailableTags.end());

    // Tag filter buttons
    if (!mAvailableTags.empty())
    {
        ImGui::Text("Tags:");
        ImGui::SameLine();

        for (const std::string& tag : mAvailableTags)
        {
            bool selected = std::find(mSelectedTags.begin(), mSelectedTags.end(), tag) != mSelectedTags.end();

            if (selected)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.8f, 1.0f));
            }

            if (ImGui::SmallButton(tag.c_str()))
            {
                if (selected)
                {
                    mSelectedTags.erase(std::find(mSelectedTags.begin(), mSelectedTags.end(), tag));
                }
                else
                {
                    mSelectedTags.push_back(tag);
                }
            }

            if (selected)
            {
                ImGui::PopStyleColor();
            }

            ImGui::SameLine();
        }

        if (!mSelectedTags.empty())
        {
            if (ImGui::SmallButton("Clear"))
            {
                mSelectedTags.clear();
            }
        }
        else
        {
            ImGui::NewLine();
        }

        ImGui::Spacing();
    }

    // Filter addons
    std::vector<const Addon*> filteredAddons;
    std::string searchStr = mSearchBuffer;
    std::transform(searchStr.begin(), searchStr.end(), searchStr.begin(), ::tolower);

    for (const Addon& addon : addons)
    {
        // Search filter
        if (!searchStr.empty())
        {
            std::string name = addon.mMetadata.mName;
            std::transform(name.begin(), name.end(), name.begin(), ::tolower);
            std::string desc = addon.mMetadata.mDescription;
            std::transform(desc.begin(), desc.end(), desc.begin(), ::tolower);

            if (name.find(searchStr) == std::string::npos &&
                desc.find(searchStr) == std::string::npos)
            {
                continue;
            }
        }

        // Tag filter
        if (!mSelectedTags.empty())
        {
            bool hasTag = false;
            for (const std::string& selectedTag : mSelectedTags)
            {
                if (std::find(addon.mMetadata.mTags.begin(), addon.mMetadata.mTags.end(), selectedTag) != addon.mMetadata.mTags.end())
                {
                    hasTag = true;
                    break;
                }
            }
            if (!hasTag)
            {
                continue;
            }
        }

        filteredAddons.push_back(&addon);
    }

    // Addon grid
    float cardWidth = 200.0f;
    float cardHeight = 200.0f;
    float spacing = 10.0f;
    int cardsPerRow = (int)((ImGui::GetContentRegionAvail().x + spacing) / (cardWidth + spacing));
    if (cardsPerRow < 1) cardsPerRow = 1;

    ImGui::BeginChild("AddonGrid", ImVec2(0, 0), true);

    for (int i = 0; i < (int)filteredAddons.size(); ++i)
    {
        if (i > 0 && i % cardsPerRow != 0)
        {
            ImGui::SameLine();
        }

        DrawAddonCard(*filteredAddons[i], cardWidth);
    }

    ImGui::EndChild();
}

void AddonsWindow::DrawAddonCard(const Addon& addon, float cardWidth)
{
    ImGui::PushID(addon.mMetadata.mId.c_str());

    float cardHeight = 200.0f;
    ImVec2 cardPos = ImGui::GetCursorScreenPos();

    ImGui::BeginGroup();

    // Card background (Dummy reserves space without capturing clicks)
    ImGui::Dummy(ImVec2(cardWidth, cardHeight));

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImU32 bgColor = addon.mIsInstalled ? IM_COL32(40, 60, 40, 255) : IM_COL32(50, 50, 60, 255);
    drawList->AddRectFilled(cardPos, ImVec2(cardPos.x + cardWidth, cardPos.y + cardHeight), bgColor, 4.0f);
    drawList->AddRect(cardPos, ImVec2(cardPos.x + cardWidth, cardPos.y + cardHeight), IM_COL32(80, 80, 100, 255), 4.0f);

    // Thumbnail (square)
    ImVec2 thumbPos(cardPos.x + 5, cardPos.y + 5);
    float thumbSide = cardWidth - 10;
    ImVec2 thumbSize(thumbSide, thumbSide);
    ImTextureID thumbTex = GetAddonThumbnail(addon.mMetadata.mId);
    if (thumbTex != 0)
    {
        drawList->AddImage(thumbTex, thumbPos, ImVec2(thumbPos.x + thumbSize.x, thumbPos.y + thumbSize.y));
    }
    else
    {
        drawList->AddRectFilled(thumbPos, ImVec2(thumbPos.x + thumbSize.x, thumbPos.y + thumbSize.y), IM_COL32(70, 70, 90, 255));
    }

    // Native badge
    if (addon.mNative.mHasNative)
    {
        ImVec2 badgePos(cardPos.x + cardWidth - 55, cardPos.y + 8);
        ImU32 badgeColor = (addon.mNative.mTarget == NativeAddonTarget::EditorOnly)
            ? IM_COL32(100, 100, 200, 255)
            : IM_COL32(200, 100, 100, 255);
        drawList->AddRectFilled(badgePos, ImVec2(badgePos.x + 50, badgePos.y + 16), badgeColor, 3.0f);
        drawList->AddText(ImVec2(badgePos.x + 5, badgePos.y + 1), IM_COL32(255, 255, 255, 255), "Native");
    }

    // Name
    float textStartY = cardPos.y + thumbSide + 10;
    ImGui::SetCursorScreenPos(ImVec2(cardPos.x + 5, textStartY));
    ImGui::PushTextWrapPos(cardPos.x + cardWidth - 5);
    ImGui::TextWrapped("%s", addon.mMetadata.mName.c_str());
    ImGui::PopTextWrapPos();

    // Author
    if (!addon.mMetadata.mAuthor.empty())
    {
        ImGui::SetCursorScreenPos(ImVec2(cardPos.x + 5, textStartY + 20));
        ImGui::TextDisabled("by %s", addon.mMetadata.mAuthor.c_str());
    }

    // Buttons
    ImGui::SetCursorScreenPos(ImVec2(cardPos.x + 5, cardPos.y + cardHeight - 30));

    if (addon.mIsInstalled)
    {
        ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f), "Installed");

        // Check for update
        AddonManager* am = AddonManager::Get();
        if (am && am->HasUpdate(addon.mMetadata.mId))
        {
            ImGui::SameLine();
            if (ImGui::SmallButton("Update"))
            {
                OnDownloadAddon(addon.mMetadata.mId);
            }
        }
    }
    else
    {
        if (ImGui::SmallButton("Download"))
        {
            OnDownloadAddon(addon.mMetadata.mId);
        }
    }

    ImGui::SameLine();
    if (ImGui::SmallButton("Info"))
    {
        OnViewMore(addon.mMetadata.mId);
    }

    ImGui::EndGroup();
    ImGui::PopID();
}

void AddonsWindow::DrawInstalledAddons()
{
    AddonManager* am = AddonManager::Get();
    if (am == nullptr)
    {
        ImGui::TextDisabled("Addon manager not initialized.");
        return;
    }

    const std::vector<InstalledAddon>& installed = am->GetInstalledAddons();

    if (installed.empty())
    {
        ImGui::TextDisabled("No addons installed in this project.");
        return;
    }

    ImGui::Text("Installed Addons (%d)", (int)installed.size());
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::BeginChild("InstalledList", ImVec2(0, 0), true);

    for (const InstalledAddon& inst : installed)
    {
        ImGui::PushID(inst.mId.c_str());

        // Find full addon info
        const Addon* addon = am->FindAddon(inst.mId);
        bool hasNative = addon && addon->mNative.mHasNative;

        // Get native state if available
        NativeAddonManager* nam = NativeAddonManager::Get();
        const NativeAddonState* nativeState = nam ? nam->GetState(inst.mId) : nullptr;

        ImGui::BeginGroup();

        // Name and version
        ImGui::Text("%s", addon ? addon->mMetadata.mName.c_str() : inst.mId.c_str());

        // Native badge
        if (hasNative)
        {
            ImGui::SameLine();
            const char* targetStr = (addon->mNative.mTarget == NativeAddonTarget::EditorOnly)
                ? "[Editor Only]" : "[Engine+Editor]";
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 1.0f, 1.0f), "%s", targetStr);
        }

        ImGui::SameLine(300);
        ImGui::TextDisabled("v%s", inst.mVersion.c_str());

        // Check for update
        bool hasUpdate = addon && am->HasUpdate(inst.mId);

        ImGui::SameLine(ImGui::GetWindowWidth() - 250);

        if (hasUpdate)
        {
            if (ImGui::SmallButton("Update"))
            {
                OnDownloadAddon(inst.mId);
            }
            ImGui::SameLine();
        }

        if (ImGui::SmallButton("Uninstall"))
        {
            mUninstallAddonId = inst.mId;
            mShowUninstallConfirm = true;
        }

        // Native addon controls
        if (hasNative)
        {
            ImGui::SameLine();
            if (ImGui::SmallButton("Build"))
            {
                OnBuildNativeAddon(inst.mId);
            }

            ImGui::SameLine();
            if (ImGui::SmallButton("Reload"))
            {
                OnReloadNativeAddon(inst.mId);
            }

            // Build status indicator
            if (nativeState)
            {
                ImGui::SameLine();
                if (nativeState->mBuildInProgress)
                {
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Building...");
                }
                else if (nativeState->mBuildSucceeded)
                {
                    bool isLoaded = nam->IsLoaded(inst.mId);
                    if (isLoaded)
                    {
                        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "Loaded");
                    }
                    else
                    {
                        ImGui::TextColored(ImVec4(0.5f, 0.8f, 0.5f, 1.0f), "Built");
                    }
                }
                else if (!nativeState->mBuildError.empty())
                {
                    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Failed");
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::SetTooltip("%s", nativeState->mBuildError.c_str());
                    }
                }
            }
        }

        // Installed date
        ImGui::TextDisabled("Installed: %s", inst.mInstalledDate.c_str());

        // Native enable checkbox
        if (hasNative)
        {
            ImGui::SameLine(300);
            bool enableNative = inst.mEnableNative;
            if (ImGui::Checkbox("Enable Native", &enableNative))
            {
                OnToggleNativeEnabled(inst.mId);
            }

            // Show build log button if there's a log
            if (nativeState && !nativeState->mBuildLog.empty())
            {
                ImGui::SameLine();
                if (ImGui::SmallButton("Log"))
                {
                    mShowBuildLog = true;
                    mBuildLogAddonId = inst.mId;
                }
            }
        }

        ImGui::EndGroup();

        ImGui::Separator();
        ImGui::PopID();
    }

    ImGui::EndChild();
}

void AddonsWindow::DrawRepositoryManager()
{
    AddonManager* am = AddonManager::Get();
    if (am == nullptr)
    {
        ImGui::TextDisabled("Addon manager not initialized.");
        return;
    }

    // Header with Add button
    if (ImGui::Button("+ Add Repository"))
    {
        mShowAddRepoPopup = true;
        memset(mRepoUrlBuffer, 0, sizeof(mRepoUrlBuffer));
        mErrorMessage.clear();
    }

    ImGui::Separator();
    ImGui::Spacing();

    const std::vector<AddonRepository>& repos = am->GetRepositories();

    if (repos.empty())
    {
        ImGui::TextDisabled("No repositories configured.");
        return;
    }

    ImGui::BeginChild("RepoList", ImVec2(0, 0), true);

    for (const AddonRepository& repo : repos)
    {
        ImGui::PushID(repo.mUrl.c_str());

        ImGui::BeginGroup();

        ImGui::Text("%s", repo.mName.c_str());
        ImGui::SameLine(ImGui::GetWindowWidth() - 100);
        if (ImGui::SmallButton("Remove"))
        {
            OnRemoveRepository(repo.mUrl);
        }

        ImGui::TextDisabled("%s", repo.mUrl.c_str());

        if (!repo.mAddonIds.empty())
        {
            ImGui::TextDisabled("%d addon(s)", (int)repo.mAddonIds.size());
        }

        ImGui::EndGroup();

        ImGui::Separator();
        ImGui::PopID();
    }

    ImGui::EndChild();
}

void AddonsWindow::DrawAddonDetailsPopup()
{
    if (!mShowAddonDetails)
    {
        return;
    }

    AddonManager* am = AddonManager::Get();
    if (am == nullptr)
    {
        mShowAddonDetails = false;
        return;
    }

    const Addon* addon = am->FindAddon(mSelectedAddonId);
    if (addon == nullptr)
    {
        mShowAddonDetails = false;
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(450, 450));

    if (ImGui::Begin("Addon Details", &mShowAddonDetails,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
    {
        ImGui::Text("%s", addon->mMetadata.mName.c_str());
        ImGui::Separator();
        ImGui::Spacing();

        // Thumbnail (square)
        ImVec2 thumbSize(200, 200);
        ImTextureID thumbTex = GetAddonThumbnail(addon->mMetadata.mId);
        if (thumbTex != 0)
        {
            ImGui::Image(thumbTex, thumbSize);
        }
        else
        {
            ImGui::Dummy(thumbSize);
            ImVec2 thumbPos = ImGui::GetItemRectMin();
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            drawList->AddRectFilled(thumbPos, ImVec2(thumbPos.x + thumbSize.x, thumbPos.y + thumbSize.y), IM_COL32(60, 60, 80, 255));
        }

        ImGui::Spacing();

        if (!addon->mMetadata.mAuthor.empty())
        {
            ImGui::Text("Author: %s", addon->mMetadata.mAuthor.c_str());
        }

        if (!addon->mMetadata.mVersion.empty())
        {
            ImGui::Text("Version: %s", addon->mMetadata.mVersion.c_str());
        }

        if (!addon->mMetadata.mUpdated.empty())
        {
            ImGui::Text("Updated: %s", addon->mMetadata.mUpdated.c_str());
        }

        ImGui::Spacing();

        if (!addon->mMetadata.mDescription.empty())
        {
            ImGui::TextWrapped("%s", addon->mMetadata.mDescription.c_str());
        }

        ImGui::Spacing();

        // Tags
        if (!addon->mMetadata.mTags.empty())
        {
            ImGui::Text("Tags:");
            ImGui::SameLine();
            for (const std::string& tag : addon->mMetadata.mTags)
            {
                ImGui::SmallButton(tag.c_str());
                ImGui::SameLine();
            }
            ImGui::NewLine();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Buttons
        if (addon->mIsInstalled)
        {
            ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f), "Already installed (v%s)", addon->mInstalledVersion.c_str());

            if (am->HasUpdate(addon->mMetadata.mId))
            {
                ImGui::SameLine();
                if (ImGui::Button("Update"))
                {
                    OnDownloadAddon(addon->mMetadata.mId);
                    mShowAddonDetails = false;
                }
            }
        }
        else
        {
            if (ImGui::Button("Download", ImVec2(100, 0)))
            {
                OnDownloadAddon(addon->mMetadata.mId);
                mShowAddonDetails = false;
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Close", ImVec2(80, 0)))
        {
            mShowAddonDetails = false;
        }

        if (!mErrorMessage.empty())
        {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s", mErrorMessage.c_str());
        }
    }
    ImGui::End();
}

void AddonsWindow::DrawAddRepoPopup()
{
    if (!mShowAddRepoPopup)
    {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(450, 200));

    if (ImGui::Begin("Add Repository", &mShowAddRepoPopup,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
    {
        ImGui::Text("Repository URL:");
        ImGui::SetNextItemWidth(420);
        ImGui::InputText("##RepoUrl", mRepoUrlBuffer, sizeof(mRepoUrlBuffer));

        ImGui::Spacing();
        ImGui::TextWrapped("Enter a GitHub repository URL containing addons.");
        ImGui::TextWrapped("The repository must have a package.json at its root listing available addons.");

        if (!mErrorMessage.empty())
        {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s", mErrorMessage.c_str());
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Add", ImVec2(80, 0)))
        {
            OnAddRepository();
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(80, 0)))
        {
            mShowAddRepoPopup = false;
        }
    }
    ImGui::End();
}

void AddonsWindow::OnDownloadAddon(const std::string& addonId)
{
    AddonManager* am = AddonManager::Get();
    if (am == nullptr)
    {
        return;
    }

    const Addon* addon = am->FindAddon(addonId);
    if (addon == nullptr)
    {
        mErrorMessage = "Addon not found: " + addonId;
        return;
    }

    mStatusMessage = "Downloading " + addon->mMetadata.mName + "...";

    std::string error;
    if (am->DownloadAddon(*addon, error))
    {
        mStatusMessage = addon->mMetadata.mName + " installed successfully!";
        mErrorMessage.clear();

        // Re-discover addon packages so the new addon shows up in the Assets panel
        AssetDir* rootDir = AssetManager::Get()->GetRootDirectory();
        if (rootDir != nullptr)
        {
            AssetDir* oldPackages = AssetManager::Get()->FindPackagesDirectory();
            if (oldPackages != nullptr)
            {
                rootDir->DeleteSubdirectory("Packages");
            }

            std::string packagesDir = GetEngineState()->mProjectDirectory + "Packages/";
            AssetManager::Get()->DiscoverAddonPackages(packagesDir);

            // Update Addons tab directory
            GetEditorState()->mTabCurrentDir[(int)AssetBrowserTab::Addons] = AssetManager::Get()->FindPackagesDirectory();
        }

        ClearThumbnailCache();
    }
    else
    {
        mStatusMessage.clear();
        mErrorMessage = "Failed to install: " + error;
        LogError("Failed to install addon %s: %s", addonId.c_str(), error.c_str());
    }
}

void AddonsWindow::OnViewMore(const std::string& addonId)
{
    mSelectedAddonId = addonId;
    mShowAddonDetails = true;
    mErrorMessage.clear();
}

void AddonsWindow::OnUninstallAddon(const std::string& addonId)
{
    AddonManager* am = AddonManager::Get();
    if (am == nullptr)
    {
        return;
    }

    if (am->UninstallAddon(addonId))
    {
        mStatusMessage = "Addon uninstalled successfully";
    }
    else
    {
        mErrorMessage = "Failed to uninstall addon";
    }
}

void AddonsWindow::DrawUninstallConfirmPopup()
{
    if (!mShowUninstallConfirm)
    {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(350, 0));

    if (ImGui::Begin("Uninstall Addon", &mShowUninstallConfirm,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse))
    {
        ImGui::TextWrapped("Are you sure you want to uninstall \"%s\"? This will delete the addon files from the Packages folder.", mUninstallAddonId.c_str());
        ImGui::Spacing();

        if (ImGui::Button("Uninstall", ImVec2(120, 0)))
        {
            OnUninstallAddon(mUninstallAddonId);
            mShowUninstallConfirm = false;
            mUninstallAddonId.clear();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            mShowUninstallConfirm = false;
            mUninstallAddonId.clear();
        }
    }
    ImGui::End();
}

void AddonsWindow::OnAddRepository()
{
    std::string url = mRepoUrlBuffer;
    if (url.empty())
    {
        mErrorMessage = "Please enter a repository URL.";
        return;
    }

    AddonManager* am = AddonManager::Get();
    if (am == nullptr)
    {
        mErrorMessage = "Addon manager not initialized.";
        return;
    }

    am->AddRepository(url);
    mShowAddRepoPopup = false;
    mErrorMessage.clear();
    mStatusMessage = "Repository added. Refreshing...";

    // Refresh to get addon list
    am->RefreshRepository(url);
    mStatusMessage = "Repository added successfully.";
}

void AddonsWindow::OnRemoveRepository(const std::string& url)
{
    AddonManager* am = AddonManager::Get();
    if (am == nullptr)
    {
        return;
    }

    am->RemoveRepository(url);
    mStatusMessage = "Repository removed.";
}

void AddonsWindow::OnRefreshRepositories()
{
    AddonManager* am = AddonManager::Get();
    if (am == nullptr)
    {
        return;
    }

    mStatusMessage = "Refreshing repositories...";
    mIsRefreshing = true;

    am->RefreshAllRepositories();

    mStatusMessage = "Repositories refreshed.";
    mIsRefreshing = false;
}

void AddonsWindow::OnBuildNativeAddon(const std::string& addonId)
{
    NativeAddonManager* nam = NativeAddonManager::Get();
    if (nam == nullptr)
    {
        mErrorMessage = "Native addon manager not initialized.";
        return;
    }

    mStatusMessage = "Building native addon...";

    std::string error;
    if (nam->BuildNativeAddon(addonId, error))
    {
        mStatusMessage = "Native addon built successfully!";
        mErrorMessage.clear();
    }
    else
    {
        mStatusMessage.clear();
        mErrorMessage = "Build failed: " + error;
    }
}

void AddonsWindow::OnReloadNativeAddon(const std::string& addonId)
{
    NativeAddonManager* nam = NativeAddonManager::Get();
    if (nam == nullptr)
    {
        mErrorMessage = "Native addon manager not initialized.";
        return;
    }

    mStatusMessage = "Reloading native addon...";

    std::string error;
    if (nam->ReloadNativeAddon(addonId, error))
    {
        mStatusMessage = "Native addon reloaded successfully!";
        mErrorMessage.clear();
    }
    else
    {
        mStatusMessage.clear();
        mErrorMessage = "Reload failed: " + error;
    }
}

void AddonsWindow::OnToggleNativeEnabled(const std::string& addonId)
{
    AddonManager* am = AddonManager::Get();
    if (am == nullptr)
    {
        return;
    }

    // Get mutable access to installed addons (need to add method or make this work)
    // For now, we'll reload the installed addons list
    std::vector<InstalledAddon>& installedAddons =
        const_cast<std::vector<InstalledAddon>&>(am->GetInstalledAddons());

    for (InstalledAddon& inst : installedAddons)
    {
        if (inst.mId == addonId)
        {
            inst.mEnableNative = !inst.mEnableNative;

            // Unload or load based on new state
            NativeAddonManager* nam = NativeAddonManager::Get();
            if (nam != nullptr)
            {
                if (!inst.mEnableNative)
                {
                    nam->UnloadNativeAddon(addonId);
                    mStatusMessage = "Native addon disabled and unloaded.";
                }
                else
                {
                    std::string error;
                    if (nam->LoadNativeAddon(addonId, error))
                    {
                        mStatusMessage = "Native addon enabled and loaded.";
                    }
                    else
                    {
                        mErrorMessage = "Failed to load: " + error;
                    }
                }
            }

            // Save the change
            am->SaveInstalledAddons();
            break;
        }
    }
}

#endif
