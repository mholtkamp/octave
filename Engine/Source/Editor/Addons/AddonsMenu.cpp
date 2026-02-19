#if EDITOR

#include "AddonsMenu.h"
#include "AddonCreator.h"
#include "AddonManager.h"
#include "NativeAddonManager.h"
#include "ActionManager.h"
#include "ProjectSelect/ProjectSelectWindow.h"
#include "System/System.h"
#include "Engine.h"
#include "InputDevices.h"
#include "Log.h"

#include "imgui.h"

#include <cstring>
#include <vector>
#include <string>

// ===== Dialog State =====

// Create Template/Addon dialog
static bool sShowCreateAddonDialog = false;
static bool sCreateAddonIsTemplate = false; // true = template, false = addon
static bool sCreateAddonCustomLocation = false;
static char sCreateAddonName[256] = {};
static char sCreateAddonAuthor[256] = {};
static char sCreateAddonDescription[512] = {};
static char sCreateAddonVersion[64] = "1.0.0";
static bool sCreateAddonWithProject = false;
static char sCreateAddonLocation[512] = {};
static std::string sCreateAddonError;
static std::string sCreateAddonSuccess;

// Create Native Addon dialog (moved from EditorImgui.cpp)
static bool sShowCreateNativeAddonDialog = false;
static bool sCreateNativeCustomLocation = false;
static char sCreateNativeName[256] = {};
static char sCreateNativeAuthor[256] = {};
static char sCreateNativeDescription[512] = {};
static char sCreateNativeVersion[64] = "1.0.0";
static int sCreateNativeTarget = 0;  // 0 = Engine + Editor, 1 = Editor Only
static char sCreateNativeLocation[512] = {};
static std::string sCreateNativeError;
static std::string sCreateNativeSuccess;

// Package Native Addon dialog (moved from EditorImgui.cpp)
static bool sShowPackageNativeAddonDialog = false;
static int sPackageSelectedAddon = 0;
static bool sPackageIncludeSource = true;
static bool sPackageIncludeAssets = true;
static bool sPackageIncludeScripts = true;
static bool sPackageIncludeThumbnail = true;
static char sPackageOutputPath[512] = {};
static std::string sPackageError;
static std::string sPackageSuccess;
static std::vector<std::string> sPackageAddonList;

// Publish Git dialog
static bool sShowPublishGitDialog = false;
static int sGitSelectedAddon = 0;
static char sGitCommitMessage[1024] = {};
static bool sGitPush = true;
static std::string sGitLog;
static std::string sGitError;
static std::string sGitStatus;
static std::vector<UserAddonInfo> sGitUserAddons;
static bool sGitHasRepo = false;
static bool sGitHasRemote = false;

// Publish Zip dialog
static bool sShowPublishZipDialog = false;
static int sZipSelectedAddon = 0;
static char sZipOutputDir[512] = {};
static std::string sZipError;
static std::string sZipSuccess;
static std::vector<UserAddonInfo> sZipUserAddons;

// Edit package.json dialog
static bool sShowEditPackageJsonDialog = false;
static int sEditSelectedAddon = 0;
static int sEditPrevSelectedAddon = -1;
static std::vector<UserAddonInfo> sEditUserAddons;
static PackageJsonData sEditData;
static char sEditName[256] = {};
static char sEditAuthor[256] = {};
static char sEditDescription[512] = {};
static char sEditVersion[64] = {};
static char sEditUrl[512] = {};
static char sEditTags[512] = {};
static int sEditType = 0;  // 0=template, 1=addon, 2=(other)
static bool sEditHasNative = false;
static int sEditNativeTarget = 0; // 0=engine, 1=editor
static char sEditSourceDir[256] = {};
static char sEditBinaryName[256] = {};
static char sEditEntrySymbol[256] = {};
static int sEditApiVersion = 1;
static std::string sEditError;
static std::string sEditSuccess;

// ===== Helper: Check if project is loaded =====

static bool IsProjectLoaded()
{
    return !GetEngineState()->mProjectPath.empty();
}

static std::string GetProjectDir()
{
    return GetEngineState()->mProjectDirectory;
}

static std::string GetPackagesDir()
{
    return GetProjectDir() + "Packages/";
}

// ===== Helper: Reset dialog states =====

static void ResetCreateAddonDialog(bool isTemplate, bool customLocation)
{
    sShowCreateAddonDialog = true;
    sCreateAddonIsTemplate = isTemplate;
    sCreateAddonCustomLocation = customLocation;
    sCreateAddonWithProject = false;
    memset(sCreateAddonName, 0, sizeof(sCreateAddonName));
    memset(sCreateAddonAuthor, 0, sizeof(sCreateAddonAuthor));
    memset(sCreateAddonDescription, 0, sizeof(sCreateAddonDescription));
    strncpy(sCreateAddonVersion, "1.0.0", sizeof(sCreateAddonVersion) - 1);
    memset(sCreateAddonLocation, 0, sizeof(sCreateAddonLocation));
    sCreateAddonError.clear();
    sCreateAddonSuccess.clear();
}

static void ResetCreateNativeDialog(bool customLocation)
{
    sShowCreateNativeAddonDialog = true;
    sCreateNativeCustomLocation = customLocation;
    memset(sCreateNativeName, 0, sizeof(sCreateNativeName));
    memset(sCreateNativeAuthor, 0, sizeof(sCreateNativeAuthor));
    memset(sCreateNativeDescription, 0, sizeof(sCreateNativeDescription));
    strncpy(sCreateNativeVersion, "1.0.0", sizeof(sCreateNativeVersion) - 1);
    sCreateNativeTarget = 0;
    memset(sCreateNativeLocation, 0, sizeof(sCreateNativeLocation));
    sCreateNativeError.clear();
    sCreateNativeSuccess.clear();
}

// ===== Helper: Load edit fields from PackageJsonData =====

static void LoadEditFields(const PackageJsonData& data)
{
    strncpy(sEditName, data.mName.c_str(), sizeof(sEditName) - 1);
    strncpy(sEditAuthor, data.mAuthor.c_str(), sizeof(sEditAuthor) - 1);
    strncpy(sEditDescription, data.mDescription.c_str(), sizeof(sEditDescription) - 1);
    strncpy(sEditVersion, data.mVersion.c_str(), sizeof(sEditVersion) - 1);
    strncpy(sEditUrl, data.mUrl.c_str(), sizeof(sEditUrl) - 1);
    strncpy(sEditTags, data.mTags.c_str(), sizeof(sEditTags) - 1);

    if (data.mType == "template") sEditType = 0;
    else if (data.mType == "addon") sEditType = 1;
    else sEditType = 2;

    sEditHasNative = data.mHasNative;
    sEditNativeTarget = (data.mNativeTarget == "editor") ? 1 : 0;
    strncpy(sEditSourceDir, data.mSourceDir.c_str(), sizeof(sEditSourceDir) - 1);
    strncpy(sEditBinaryName, data.mBinaryName.c_str(), sizeof(sEditBinaryName) - 1);
    strncpy(sEditEntrySymbol, data.mEntrySymbol.c_str(), sizeof(sEditEntrySymbol) - 1);
    sEditApiVersion = data.mApiVersion;
}

static PackageJsonData CollectEditFields()
{
    PackageJsonData data;
    data.mName = sEditName;
    data.mAuthor = sEditAuthor;
    data.mDescription = sEditDescription;
    data.mVersion = sEditVersion;
    data.mUrl = sEditUrl;
    data.mTags = sEditTags;

    if (sEditType == 0) data.mType = "template";
    else if (sEditType == 1) data.mType = "addon";
    else data.mType = "";

    data.mHasNative = sEditHasNative;
    data.mNativeTarget = (sEditNativeTarget == 1) ? "editor" : "engine";
    data.mSourceDir = sEditSourceDir;
    data.mBinaryName = sEditBinaryName;
    data.mEntrySymbol = sEditEntrySymbol;
    data.mApiVersion = sEditApiVersion;
    return data;
}

// ===== Menu Content Drawing =====

static void DrawCreateSubmenu()
{
    if (ImGui::BeginMenu("Create"))
    {
        bool projectLoaded = IsProjectLoaded();

        if (!projectLoaded) ImGui::BeginDisabled();

        if (ImGui::MenuItem("Create Template In Project"))
        {
            ResetCreateAddonDialog(true, false);
        }

        if (!projectLoaded) ImGui::EndDisabled();

        if (ImGui::MenuItem("Create Template..."))
        {
            ResetCreateAddonDialog(true, true);
        }

        ImGui::Separator();

        if (!projectLoaded) ImGui::BeginDisabled();

        if (ImGui::MenuItem("Create Addon In Project"))
        {
            ResetCreateAddonDialog(false, false);
        }

        if (!projectLoaded) ImGui::EndDisabled();

        if (ImGui::MenuItem("Create Addon..."))
        {
            ResetCreateAddonDialog(false, true);
        }

        ImGui::Separator();

        if (!projectLoaded) ImGui::BeginDisabled();

        if (ImGui::MenuItem("Create Native C++ Addon In Project"))
        {
            ResetCreateNativeDialog(false);
        }

        if (!projectLoaded) ImGui::EndDisabled();

        if (ImGui::MenuItem("Create Native C++ Addon..."))
        {
            ResetCreateNativeDialog(true);
        }

        ImGui::EndMenu();
    }
}

static void DrawInitializeSubmenu()
{
    bool projectLoaded = IsProjectLoaded();

    if (!projectLoaded) ImGui::BeginDisabled();

    if (ImGui::BeginMenu("Initialize"))
    {
        if (ImGui::MenuItem("Template"))
        {
            std::string error;
            if (AddonCreator::InitializeAsTemplate(GetProjectDir(), error))
            {
                LogDebug("Project initialized as template.");
            }
            else
            {
                LogError("Initialize failed: %s", error.c_str());
            }
        }

        if (ImGui::MenuItem("Addon"))
        {
            std::string error;
            if (AddonCreator::InitializeAsAddon(GetProjectDir(), error))
            {
                LogDebug("Project initialized as addon.");
            }
            else
            {
                LogError("Initialize failed: %s", error.c_str());
            }
        }

        if (ImGui::MenuItem("Native C++ Addon"))
        {
            std::string error;
            if (AddonCreator::InitializeAsNativeAddon(GetProjectDir(), error))
            {
                LogDebug("Project initialized as native addon.");
            }
            else
            {
                LogError("Initialize failed: %s", error.c_str());
            }
        }

        ImGui::EndMenu();
    }

    if (!projectLoaded) ImGui::EndDisabled();
}

static void DrawPublishSubmenu()
{
    bool projectLoaded = IsProjectLoaded();

    if (!projectLoaded) ImGui::BeginDisabled();

    if (ImGui::BeginMenu("Publish"))
    {
        if (ImGui::MenuItem("Git..."))
        {
            sShowPublishGitDialog = true;
            sGitSelectedAddon = 0;
            memset(sGitCommitMessage, 0, sizeof(sGitCommitMessage));
            sGitPush = true;
            sGitLog.clear();
            sGitError.clear();
            sGitStatus.clear();
            sGitUserAddons = AddonCreator::GetUserCreatedAddons(GetProjectDir());
            sGitHasRepo = false;
            sGitHasRemote = false;
        }

        if (ImGui::MenuItem("Zip..."))
        {
            sShowPublishZipDialog = true;
            sZipSelectedAddon = 0;
            memset(sZipOutputDir, 0, sizeof(sZipOutputDir));
            sZipError.clear();
            sZipSuccess.clear();
            sZipUserAddons = AddonCreator::GetUserCreatedAddons(GetProjectDir());
        }

        ImGui::EndMenu();
    }

    if (!projectLoaded) ImGui::EndDisabled();
}

static void DrawEditPackageJsonItem()
{
    bool projectLoaded = IsProjectLoaded();

    if (!projectLoaded) ImGui::BeginDisabled();

    if (ImGui::MenuItem("Edit package.json..."))
    {
        sShowEditPackageJsonDialog = true;
        sEditSelectedAddon = 0;
        sEditPrevSelectedAddon = -1;
        sEditError.clear();
        sEditSuccess.clear();
        sEditUserAddons = AddonCreator::GetUserCreatedAddons(GetProjectDir());

        // Clear edit fields
        memset(sEditName, 0, sizeof(sEditName));
        memset(sEditAuthor, 0, sizeof(sEditAuthor));
        memset(sEditDescription, 0, sizeof(sEditDescription));
        memset(sEditVersion, 0, sizeof(sEditVersion));
        memset(sEditUrl, 0, sizeof(sEditUrl));
        memset(sEditTags, 0, sizeof(sEditTags));
        memset(sEditSourceDir, 0, sizeof(sEditSourceDir));
        memset(sEditBinaryName, 0, sizeof(sEditBinaryName));
        memset(sEditEntrySymbol, 0, sizeof(sEditEntrySymbol));
    }

    if (!projectLoaded) ImGui::EndDisabled();
}

static void DrawAddonsMenuItems()
{
    DrawCreateSubmenu();
    DrawInitializeSubmenu();
    DrawPublishSubmenu();
    ImGui::Separator();
    DrawEditPackageJsonItem();
}

void DrawAddonsPopupContent()
{
    DrawAddonsMenuItems();
}

void DrawAddonsMenuBar()
{
    if (ImGui::BeginMenu("Addons"))
    {
        DrawAddonsMenuItems();
        ImGui::EndMenu();
    }
}

void DrawAddonsCreateItems_ProjectSelect()
{
    if (ImGui::Button("Create Template..."))
    {
        ResetCreateAddonDialog(true, true);
    }

    ImGui::SameLine();
    if (ImGui::Button("Create Addon..."))
    {
        ResetCreateAddonDialog(false, true);
    }

    ImGui::SameLine();
    if (ImGui::Button("Create Native C++ Addon..."))
    {
        ResetCreateNativeDialog(true);
    }
}

// ===== Dialog Drawing =====

static void DrawCreateAddonDialog()
{
    if (sShowCreateAddonDialog)
    {
        const char* title = sCreateAddonIsTemplate ? "Create Template" : "Create Addon";
        ImGui::OpenPopup(title);
    }

    const char* title = sCreateAddonIsTemplate ? "Create Template" : "Create Addon";

    if (ImGui::IsPopupOpen(title))
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                                ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(450, 400));
    }

    if (ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
    {
        const char* desc = sCreateAddonIsTemplate ?
            "Create a new template package." :
            "Create a new script addon package.";
        ImGui::Text("%s", desc);
        ImGui::Separator();
        ImGui::Spacing();

        // Name
        ImGui::Text("Name *");
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##CreateAddonName", sCreateAddonName, sizeof(sCreateAddonName));
        ImGui::Spacing();

        // Author
        ImGui::Text("Author");
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##CreateAddonAuthor", sCreateAddonAuthor, sizeof(sCreateAddonAuthor));
        ImGui::Spacing();

        // Description
        ImGui::Text("Description");
        ImGui::SetNextItemWidth(-1);
        ImGui::InputTextMultiline("##CreateAddonDesc", sCreateAddonDescription, sizeof(sCreateAddonDescription), ImVec2(-1, 60));
        ImGui::Spacing();

        // Version
        ImGui::Text("Version");
        ImGui::SetNextItemWidth(100);
        ImGui::InputText("##CreateAddonVersion", sCreateAddonVersion, sizeof(sCreateAddonVersion));
        ImGui::Spacing();

        // Create project file option
        if (sCreateAddonIsTemplate)
        {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "A project file will be created and opened.");
        }
        else
        {
            ImGui::Checkbox("Create project file", &sCreateAddonWithProject);
        }
        ImGui::Spacing();

        // Location (custom only)
        if (sCreateAddonCustomLocation)
        {
            ImGui::Text("Location");
            ImGui::SetNextItemWidth(-70);
            ImGui::InputText("##CreateAddonLoc", sCreateAddonLocation, sizeof(sCreateAddonLocation));
            ImGui::SameLine();
            if (ImGui::Button("Browse...##AddonLoc"))
            {
                std::string selected = SYS_SelectFolderDialog();
                if (!selected.empty())
                {
                    strncpy(sCreateAddonLocation, selected.c_str(), sizeof(sCreateAddonLocation) - 1);
                }
            }
        }
        else
        {
            std::string id = AddonCreator::GenerateIdFromName(sCreateAddonName);
            if (!id.empty())
            {
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Location: %sPackages/%s/",
                    GetProjectDir().c_str(), id.c_str());
            }
        }

        // Error/Success
        if (!sCreateAddonError.empty())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
            ImGui::TextWrapped("%s", sCreateAddonError.c_str());
            ImGui::PopStyleColor();
        }
        if (!sCreateAddonSuccess.empty())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.3f, 1.0f));
            ImGui::TextWrapped("%s", sCreateAddonSuccess.c_str());
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        bool closePopup = false;

        if (ImGui::Button("Create", ImVec2(100, 0)))
        {
            if (strlen(sCreateAddonName) == 0)
            {
                sCreateAddonError = "Name is required.";
            }
            else
            {
                AddonCreateInfo info;
                info.mName = sCreateAddonName;
                info.mAuthor = sCreateAddonAuthor;
                info.mDescription = sCreateAddonDescription;
                info.mVersion = sCreateAddonVersion;

                std::string targetDir;
                if (sCreateAddonCustomLocation)
                {
                    if (strlen(sCreateAddonLocation) == 0)
                    {
                        sCreateAddonError = "Location is required.";
                        ImGui::EndPopup();
                        return;
                    }
                    targetDir = sCreateAddonLocation;
                }
                else
                {
                    targetDir = GetPackagesDir();
                }

                std::string error;
                bool success = sCreateAddonIsTemplate ?
                    AddonCreator::CreateTemplate(info, targetDir, error) :
                    AddonCreator::CreateAddon(info, targetDir, error);

                if (success)
                {
                    sCreateAddonSuccess = sCreateAddonIsTemplate ?
                        "Template created successfully!" : "Addon created successfully!";
                    sCreateAddonError.clear();

                    std::string id = AddonCreator::GenerateIdFromName(sCreateAddonName);
                    std::string normalizedTarget = targetDir;
                    if (!normalizedTarget.empty() && normalizedTarget.back() != '/' && normalizedTarget.back() != '\\')
                        normalizedTarget += '/';
                    std::string createdPath = normalizedTarget + id + "/";

                    bool shouldCreateProject = sCreateAddonIsTemplate || sCreateAddonWithProject;
                    if (shouldCreateProject)
                    {
                        // Create project with .octp, Config.ini, and default scene
                        const char* sceneName = sCreateAddonIsTemplate ? "SC_Default" : "SC_Addon";
                        ActionManager::Get()->CreateNewProject(createdPath.c_str(), false, sceneName);
                        GetProjectSelectWindow()->Close();
                    }
                    else
                    {
                        // Just open the created folder in explorer
                        SYS_ExplorerOpenDirectory(createdPath);
                    }

                    sShowCreateAddonDialog = false;
                    ImGui::CloseCurrentPopup();
                }
                else
                {
                    sCreateAddonError = error;
                    sCreateAddonSuccess.clear();
                }
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(100, 0)))
        {
            closePopup = true;
        }

        if (IsKeyJustDown(KEY_ESCAPE))
        {
            closePopup = true;
        }

        if (closePopup)
        {
            sShowCreateAddonDialog = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

static void DrawCreateNativeAddonDialog()
{
    if (sShowCreateNativeAddonDialog)
    {
        ImGui::OpenPopup("Create Native Addon");
    }

    if (ImGui::IsPopupOpen("Create Native Addon"))
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                                ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(450, 420));
    }

    if (ImGui::BeginPopupModal("Create Native Addon", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
    {
        ImGui::Text("Create a new native addon with C++ source code.");
        ImGui::Separator();
        ImGui::Spacing();

        // Name
        ImGui::Text("Addon Name *");
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##NativeAddonName", sCreateNativeName, sizeof(sCreateNativeName));
        ImGui::Spacing();

        // Author
        ImGui::Text("Author");
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##NativeAddonAuthor", sCreateNativeAuthor, sizeof(sCreateNativeAuthor));
        ImGui::Spacing();

        // Description
        ImGui::Text("Description");
        ImGui::SetNextItemWidth(-1);
        ImGui::InputTextMultiline("##NativeAddonDesc", sCreateNativeDescription, sizeof(sCreateNativeDescription), ImVec2(-1, 60));
        ImGui::Spacing();

        // Version
        ImGui::Text("Version");
        ImGui::SetNextItemWidth(100);
        ImGui::InputText("##NativeAddonVersion", sCreateNativeVersion, sizeof(sCreateNativeVersion));
        ImGui::Spacing();

        // Target
        ImGui::Text("Target");
        const char* targetItems[] = { "Engine + Editor (Recommended)", "Editor Only" };
        ImGui::SetNextItemWidth(-1);
        ImGui::Combo("##NativeAddonTarget", &sCreateNativeTarget, targetItems, 2);
        ImGui::Spacing();

        // Location (custom only)
        if (sCreateNativeCustomLocation)
        {
            ImGui::Text("Location");
            ImGui::SetNextItemWidth(-70);
            ImGui::InputText("##NativeAddonLoc", sCreateNativeLocation, sizeof(sCreateNativeLocation));
            ImGui::SameLine();
            if (ImGui::Button("Browse...##NativeLoc"))
            {
                std::string selected = SYS_SelectFolderDialog();
                if (!selected.empty())
                {
                    strncpy(sCreateNativeLocation, selected.c_str(), sizeof(sCreateNativeLocation) - 1);
                }
            }
        }
        else
        {
            std::string id = AddonCreator::GenerateIdFromName(sCreateNativeName);
            if (!id.empty())
            {
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Location: %sPackages/%s/",
                    GetProjectDir().c_str(), id.c_str());
            }
        }

        // Error/Success
        if (!sCreateNativeError.empty())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
            ImGui::TextWrapped("%s", sCreateNativeError.c_str());
            ImGui::PopStyleColor();
        }
        if (!sCreateNativeSuccess.empty())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.3f, 1.0f));
            ImGui::TextWrapped("%s", sCreateNativeSuccess.c_str());
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        bool closePopup = false;

        if (ImGui::Button("Create", ImVec2(100, 0)))
        {
            if (strlen(sCreateNativeName) == 0)
            {
                sCreateNativeError = "Addon name is required.";
            }
            else
            {
                NativeAddonCreateInfo info;
                info.mName = sCreateNativeName;
                info.mAuthor = sCreateNativeAuthor;
                info.mDescription = sCreateNativeDescription;
                info.mVersion = sCreateNativeVersion;
                info.mTarget = (sCreateNativeTarget == 0) ?
                    NativeAddonTarget::EngineAndEditor : NativeAddonTarget::EditorOnly;

                std::string targetDir;
                if (sCreateNativeCustomLocation)
                {
                    if (strlen(sCreateNativeLocation) == 0)
                    {
                        sCreateNativeError = "Location is required.";
                        ImGui::EndPopup();
                        return;
                    }
                    targetDir = sCreateNativeLocation;
                }
                else
                {
                    targetDir = GetPackagesDir();
                }

                std::string error;
                std::string createdPath;
                bool success;

                if (sCreateNativeCustomLocation)
                {
                    success = AddonCreator::CreateNativeAddon(info, targetDir, error, &createdPath);
                }
                else
                {
                    NativeAddonManager* nam = NativeAddonManager::Get();
                    if (nam != nullptr)
                    {
                        success = nam->CreateNativeAddon(info, error, &createdPath);
                    }
                    else
                    {
                        error = "NativeAddonManager not initialized.";
                        success = false;
                    }
                }

                if (success)
                {
                    sCreateNativeSuccess = "Native addon created successfully!";
                    sCreateNativeError.clear();

                    if (!createdPath.empty())
                    {
                        SYS_ExplorerOpenDirectory(createdPath);
                    }

                    sShowCreateNativeAddonDialog = false;
                    ImGui::CloseCurrentPopup();
                }
                else
                {
                    sCreateNativeError = error;
                    sCreateNativeSuccess.clear();
                }
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(100, 0)))
        {
            closePopup = true;
        }

        if (IsKeyJustDown(KEY_ESCAPE))
        {
            closePopup = true;
        }

        if (closePopup)
        {
            sShowCreateNativeAddonDialog = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

static void DrawPackageNativeAddonDialog()
{
    if (sShowPackageNativeAddonDialog)
    {
        ImGui::OpenPopup("Package Native Addon");
    }

    if (ImGui::IsPopupOpen("Package Native Addon"))
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                                ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(450, 350));
    }

    if (ImGui::BeginPopupModal("Package Native Addon", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
    {
        ImGui::Text("Package a native addon for distribution.");
        ImGui::Separator();
        ImGui::Spacing();

        // Addon selection
        ImGui::Text("Select Addon");
        ImGui::SetNextItemWidth(-1);

        if (sPackageAddonList.empty())
        {
            ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), "No native addons found in Packages/ folder.");
        }
        else
        {
            std::vector<const char*> addonNames;
            for (const auto& id : sPackageAddonList)
            {
                addonNames.push_back(id.c_str());
            }

            ImGui::Combo("##SelectAddon", &sPackageSelectedAddon, addonNames.data(), (int)addonNames.size());
        }
        ImGui::Spacing();

        // Include options
        ImGui::Text("Include in Package:");
        ImGui::Checkbox("Source Code", &sPackageIncludeSource);
        ImGui::Checkbox("Assets", &sPackageIncludeAssets);
        ImGui::Checkbox("Scripts", &sPackageIncludeScripts);
        ImGui::Checkbox("Thumbnail", &sPackageIncludeThumbnail);
        ImGui::Spacing();

        // Output path
        ImGui::Text("Output Path (optional, leave empty for default)");
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##OutputPath", sPackageOutputPath, sizeof(sPackageOutputPath));
        ImGui::Spacing();

        // Error/Success
        if (!sPackageError.empty())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
            ImGui::TextWrapped("%s", sPackageError.c_str());
            ImGui::PopStyleColor();
        }
        if (!sPackageSuccess.empty())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.3f, 1.0f));
            ImGui::TextWrapped("%s", sPackageSuccess.c_str());
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        bool closePopup = false;

        bool canPackage = !sPackageAddonList.empty() &&
                          sPackageSelectedAddon >= 0 &&
                          sPackageSelectedAddon < (int)sPackageAddonList.size();

        if (!canPackage) ImGui::BeginDisabled();

        if (ImGui::Button("Package", ImVec2(100, 0)))
        {
            NativeAddonManager* nam = NativeAddonManager::Get();
            if (nam != nullptr && canPackage)
            {
                NativeAddonPackageOptions options;
                options.mAddonId = sPackageAddonList[sPackageSelectedAddon];
                options.mIncludeSource = sPackageIncludeSource;
                options.mIncludeAssets = sPackageIncludeAssets;
                options.mIncludeScripts = sPackageIncludeScripts;
                options.mIncludeThumbnail = sPackageIncludeThumbnail;
                options.mOutputPath = sPackageOutputPath;

                std::string error;
                if (nam->PackageNativeAddon(options, error))
                {
                    sPackageSuccess = "Addon packaged successfully! Check Packaged/ folder.";
                    sPackageError.clear();
                }
                else
                {
                    sPackageError = error;
                    sPackageSuccess.clear();
                }
            }
        }

        if (!canPackage) ImGui::EndDisabled();

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(100, 0)))
        {
            closePopup = true;
        }

        if (IsKeyJustDown(KEY_ESCAPE))
        {
            closePopup = true;
        }

        if (closePopup)
        {
            sShowPackageNativeAddonDialog = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

static void DrawPublishGitDialog()
{
    if (sShowPublishGitDialog)
    {
        ImGui::OpenPopup("Publish via Git");
    }

    if (ImGui::IsPopupOpen("Publish via Git"))
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                                ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(500, 450));
    }

    if (ImGui::BeginPopupModal("Publish via Git", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
    {
        ImGui::Text("Publish addon via Git.");
        ImGui::Separator();
        ImGui::Spacing();

        if (sGitUserAddons.empty())
        {
            ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), "No user-created addons found.");
            ImGui::Spacing();
        }
        else
        {
            // Addon selector
            ImGui::Text("Select Addon");
            ImGui::SetNextItemWidth(-1);
            std::vector<const char*> names;
            for (const auto& addon : sGitUserAddons)
            {
                names.push_back(addon.mName.c_str());
            }
            if (ImGui::Combo("##GitAddon", &sGitSelectedAddon, names.data(), (int)names.size()))
            {
                // Selection changed - refresh git status
                sGitLog.clear();
                sGitError.clear();
                sGitStatus.clear();
            }

            if (sGitSelectedAddon >= 0 && sGitSelectedAddon < (int)sGitUserAddons.size())
            {
                const UserAddonInfo& selected = sGitUserAddons[sGitSelectedAddon];
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Path: %s", selected.mPath.c_str());

                // Check git status
                sGitHasRepo = AddonCreator::HasGitRepo(selected.mPath);
                sGitHasRemote = sGitHasRepo ? AddonCreator::HasGitRemote(selected.mPath) : false;

                if (!sGitHasRepo)
                {
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), "No git repository found.");
                    if (ImGui::Button("Initialize Git Repository"))
                    {
                        std::string error;
                        if (AddonCreator::InitGitRepo(selected.mPath, error))
                        {
                            sGitHasRepo = true;
                            sGitLog += "Git repository initialized.\n";
                        }
                        else
                        {
                            sGitError = error;
                        }
                    }
                }
                else
                {
                    // Refresh git status button
                    ImGui::Spacing();
                    if (ImGui::Button("Refresh Status"))
                    {
                        std::string cmd = "git -C \"" + selected.mPath + "\" status --porcelain";
                        int exitCode = -1;
                        std::string stdOut;
                        SYS_ExecFull(cmd.c_str(), &stdOut, nullptr, &exitCode);
                        sGitStatus = stdOut.empty() ? "Working tree clean." : stdOut;
                    }

                    if (!sGitStatus.empty())
                    {
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Status:");
                        ImGui::BeginChild("##GitStatus", ImVec2(-1, 60), true);
                        ImGui::TextWrapped("%s", sGitStatus.c_str());
                        ImGui::EndChild();
                    }

                    ImGui::Spacing();

                    // Commit message
                    ImGui::Text("Commit Message *");
                    ImGui::SetNextItemWidth(-1);
                    ImGui::InputTextMultiline("##GitCommit", sGitCommitMessage, sizeof(sGitCommitMessage), ImVec2(-1, 60));
                    ImGui::Spacing();

                    // Push checkbox
                    if (!sGitHasRemote) ImGui::BeginDisabled();
                    ImGui::Checkbox("Push to remote", &sGitPush);
                    if (!sGitHasRemote)
                    {
                        ImGui::EndDisabled();
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), "(no remote configured)");
                    }
                }
            }
        }

        // Error/Log
        if (!sGitError.empty())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
            ImGui::TextWrapped("%s", sGitError.c_str());
            ImGui::PopStyleColor();
        }
        if (!sGitLog.empty())
        {
            ImGui::BeginChild("##GitLog", ImVec2(-1, 60), true);
            ImGui::TextWrapped("%s", sGitLog.c_str());
            ImGui::EndChild();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        bool closePopup = false;
        bool canPublish = !sGitUserAddons.empty() && sGitHasRepo &&
                          sGitSelectedAddon >= 0 && sGitSelectedAddon < (int)sGitUserAddons.size();

        if (!canPublish) ImGui::BeginDisabled();

        if (ImGui::Button("Publish", ImVec2(100, 0)))
        {
            if (strlen(sGitCommitMessage) == 0)
            {
                sGitError = "Commit message is required.";
            }
            else
            {
                const UserAddonInfo& selected = sGitUserAddons[sGitSelectedAddon];
                std::string error;
                std::string log;
                if (AddonCreator::PublishViaGit(selected.mPath, sGitCommitMessage, sGitPush, error, log))
                {
                    sGitLog = log;
                    sGitError.clear();
                }
                else
                {
                    sGitError = error;
                    sGitLog = log;
                }
            }
        }

        if (!canPublish) ImGui::EndDisabled();

        ImGui::SameLine();
        if (ImGui::Button("Close", ImVec2(100, 0)))
        {
            closePopup = true;
        }

        if (IsKeyJustDown(KEY_ESCAPE))
        {
            closePopup = true;
        }

        if (closePopup)
        {
            sShowPublishGitDialog = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

static void DrawPublishZipDialog()
{
    if (sShowPublishZipDialog)
    {
        ImGui::OpenPopup("Publish as Zip");
    }

    if (ImGui::IsPopupOpen("Publish as Zip"))
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                                ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(450, 300));
    }

    if (ImGui::BeginPopupModal("Publish as Zip", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
    {
        ImGui::Text("Package addon as a zip file.");
        ImGui::Separator();
        ImGui::Spacing();

        if (sZipUserAddons.empty())
        {
            ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), "No user-created addons found.");
        }
        else
        {
            // Addon selector
            ImGui::Text("Select Addon");
            ImGui::SetNextItemWidth(-1);
            std::vector<const char*> names;
            for (const auto& addon : sZipUserAddons)
            {
                names.push_back(addon.mName.c_str());
            }
            ImGui::Combo("##ZipAddon", &sZipSelectedAddon, names.data(), (int)names.size());
            ImGui::Spacing();

            // Output directory
            ImGui::Text("Output Directory");
            ImGui::SetNextItemWidth(-70);
            ImGui::InputText("##ZipOutputDir", sZipOutputDir, sizeof(sZipOutputDir));
            ImGui::SameLine();
            if (ImGui::Button("Browse...##ZipDir"))
            {
                std::string selected = SYS_SelectFolderDialog();
                if (!selected.empty())
                {
                    strncpy(sZipOutputDir, selected.c_str(), sizeof(sZipOutputDir) - 1);
                }
            }

            if (sZipSelectedAddon >= 0 && sZipSelectedAddon < (int)sZipUserAddons.size())
            {
                const UserAddonInfo& addon = sZipUserAddons[sZipSelectedAddon];
                std::string autoName = addon.mId + ".zip";
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Output: %s/%s",
                    strlen(sZipOutputDir) > 0 ? sZipOutputDir : "(select directory)", autoName.c_str());
            }
        }

        // Error/Success
        if (!sZipError.empty())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
            ImGui::TextWrapped("%s", sZipError.c_str());
            ImGui::PopStyleColor();
        }
        if (!sZipSuccess.empty())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.3f, 1.0f));
            ImGui::TextWrapped("%s", sZipSuccess.c_str());
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        bool closePopup = false;
        bool canZip = !sZipUserAddons.empty() &&
                      sZipSelectedAddon >= 0 && sZipSelectedAddon < (int)sZipUserAddons.size() &&
                      strlen(sZipOutputDir) > 0;

        if (!canZip) ImGui::BeginDisabled();

        if (ImGui::Button("Package", ImVec2(100, 0)))
        {
            const UserAddonInfo& addon = sZipUserAddons[sZipSelectedAddon];
            std::string outputDir = sZipOutputDir;
            if (!outputDir.empty() && outputDir.back() != '/' && outputDir.back() != '\\')
            {
                outputDir += '/';
            }
            std::string outputPath = outputDir + addon.mId + ".zip";

            std::string error;
            if (AddonCreator::PublishAsZip(addon.mPath, outputPath, error))
            {
                sZipSuccess = "Addon packaged to: " + outputPath;
                sZipError.clear();
            }
            else
            {
                sZipError = error;
                sZipSuccess.clear();
            }
        }

        if (!canZip) ImGui::EndDisabled();

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(100, 0)))
        {
            closePopup = true;
        }

        if (IsKeyJustDown(KEY_ESCAPE))
        {
            closePopup = true;
        }

        if (closePopup)
        {
            sShowPublishZipDialog = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

static void DrawEditPackageJsonDialog()
{
    if (sShowEditPackageJsonDialog)
    {
        ImGui::OpenPopup("Edit package.json");
    }

    if (ImGui::IsPopupOpen("Edit package.json"))
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                                ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(550, 500));
    }

    if (ImGui::BeginPopupModal("Edit package.json", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
    {
        if (sEditUserAddons.empty())
        {
            ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), "No user-created addons found. Create one first.");
            ImGui::Spacing();
        }
        else
        {
            // Addon selector
            ImGui::Text("Select Addon");
            ImGui::SetNextItemWidth(-1);
            std::vector<const char*> names;
            for (const auto& addon : sEditUserAddons)
            {
                names.push_back(addon.mName.c_str());
            }
            ImGui::Combo("##EditAddon", &sEditSelectedAddon, names.data(), (int)names.size());

            // Reload data when selection changes
            if (sEditSelectedAddon != sEditPrevSelectedAddon)
            {
                sEditPrevSelectedAddon = sEditSelectedAddon;
                sEditError.clear();
                sEditSuccess.clear();

                if (sEditSelectedAddon >= 0 && sEditSelectedAddon < (int)sEditUserAddons.size())
                {
                    const UserAddonInfo& addon = sEditUserAddons[sEditSelectedAddon];
                    std::string packagePath = addon.mPath + "package.json";

                    PackageJsonData data;
                    std::string error;
                    if (AddonCreator::ReadPackageJson(packagePath, data, error))
                    {
                        sEditData = data;
                        LoadEditFields(data);
                    }
                    else
                    {
                        sEditError = "Failed to load: " + error;
                    }
                }
            }

            if (sEditSelectedAddon >= 0 && sEditSelectedAddon < (int)sEditUserAddons.size())
            {
                const UserAddonInfo& addon = sEditUserAddons[sEditSelectedAddon];
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%spackage.json", addon.mPath.c_str());
                ImGui::Separator();
                ImGui::Spacing();

                // Scrollable edit area
                ImGui::BeginChild("##EditFields", ImVec2(-1, -80), false);

                // Name
                ImGui::Text("Name");
                ImGui::SetNextItemWidth(-1);
                ImGui::InputText("##EditName", sEditName, sizeof(sEditName));

                // Author
                ImGui::Text("Author");
                ImGui::SetNextItemWidth(-1);
                ImGui::InputText("##EditAuthor", sEditAuthor, sizeof(sEditAuthor));

                // Description
                ImGui::Text("Description");
                ImGui::SetNextItemWidth(-1);
                ImGui::InputTextMultiline("##EditDesc", sEditDescription, sizeof(sEditDescription), ImVec2(-1, 50));

                // Version
                ImGui::Text("Version");
                ImGui::SetNextItemWidth(100);
                ImGui::InputText("##EditVersion", sEditVersion, sizeof(sEditVersion));

                // URL
                ImGui::Text("URL");
                ImGui::SetNextItemWidth(-1);
                ImGui::InputText("##EditUrl", sEditUrl, sizeof(sEditUrl));

                // Type
                ImGui::Text("Type");
                const char* typeItems[] = { "template", "addon", "(none)" };
                ImGui::SetNextItemWidth(150);
                ImGui::Combo("##EditType", &sEditType, typeItems, 3);

                // Tags
                ImGui::Text("Tags (comma-separated)");
                ImGui::SetNextItemWidth(-1);
                ImGui::InputText("##EditTags", sEditTags, sizeof(sEditTags));

                ImGui::Spacing();

                // Native section (collapsible)
                if (ImGui::CollapsingHeader("Native C++ Configuration"))
                {
                    ImGui::Checkbox("Enable Native Code", &sEditHasNative);

                    if (sEditHasNative)
                    {
                        ImGui::Spacing();

                        ImGui::Text("Target");
                        const char* nativeTargetItems[] = { "Engine + Editor", "Editor Only" };
                        ImGui::SetNextItemWidth(200);
                        ImGui::Combo("##EditNativeTarget", &sEditNativeTarget, nativeTargetItems, 2);

                        ImGui::Text("Source Dir");
                        ImGui::SetNextItemWidth(-1);
                        ImGui::InputText("##EditSourceDir", sEditSourceDir, sizeof(sEditSourceDir));

                        ImGui::Text("Binary Name");
                        ImGui::SetNextItemWidth(-1);
                        ImGui::InputText("##EditBinaryName", sEditBinaryName, sizeof(sEditBinaryName));

                        ImGui::Text("Entry Symbol");
                        ImGui::SetNextItemWidth(-1);
                        ImGui::InputText("##EditEntrySymbol", sEditEntrySymbol, sizeof(sEditEntrySymbol));

                        ImGui::Text("API Version");
                        ImGui::SetNextItemWidth(80);
                        ImGui::InputInt("##EditApiVersion", &sEditApiVersion);
                    }
                }

                ImGui::EndChild();
            }
        }

        // Error/Success
        if (!sEditError.empty())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
            ImGui::TextWrapped("%s", sEditError.c_str());
            ImGui::PopStyleColor();
        }
        if (!sEditSuccess.empty())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.3f, 1.0f));
            ImGui::TextWrapped("%s", sEditSuccess.c_str());
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        bool closePopup = false;
        bool hasAddon = !sEditUserAddons.empty() &&
                        sEditSelectedAddon >= 0 && sEditSelectedAddon < (int)sEditUserAddons.size();

        if (!hasAddon) ImGui::BeginDisabled();

        if (ImGui::Button("Save", ImVec2(80, 0)))
        {
            const UserAddonInfo& addon = sEditUserAddons[sEditSelectedAddon];
            std::string packagePath = addon.mPath + "package.json";

            PackageJsonData data = CollectEditFields();
            std::string error;
            if (AddonCreator::WritePackageJson(packagePath, data, error))
            {
                sEditSuccess = "Saved successfully!";
                sEditError.clear();
            }
            else
            {
                sEditError = error;
                sEditSuccess.clear();
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Open Externally", ImVec2(120, 0)))
        {
            const UserAddonInfo& addon = sEditUserAddons[sEditSelectedAddon];
            AddonCreator::OpenInExternalEditor(addon.mPath + "package.json");
        }

        if (!hasAddon) ImGui::EndDisabled();

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(80, 0)))
        {
            closePopup = true;
        }

        if (IsKeyJustDown(KEY_ESCAPE))
        {
            closePopup = true;
        }

        if (closePopup)
        {
            sShowEditPackageJsonDialog = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void DrawAddonsDialogs()
{
    DrawCreateAddonDialog();
    DrawCreateNativeAddonDialog();
    DrawPackageNativeAddonDialog();
    DrawPublishGitDialog();
    DrawPublishZipDialog();
    DrawEditPackageJsonDialog();
}

void OpenCreateNativeAddonDialog()
{
    ResetCreateNativeDialog(false);
}

#endif // EDITOR
