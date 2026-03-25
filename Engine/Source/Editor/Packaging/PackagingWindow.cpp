#if EDITOR

#include "PackagingWindow.h"
#include "PackagingSettings.h"
#include "Preferences/PreferencesWindow.h"
#include "Preferences/PreferencesManager.h"
#include "Preferences/External/LaunchersModule.h"
#include "Preferences/Packaging/DockerModule.h"
#include "Preferences/External/ExternalModule.h"
#include "ActionManager.h"

#include "Engine.h"
#include "Log.h"
#include "System/System.h"
#include "Utilities.h"

#include "imgui.h"

#include <cstring>
#include <cstdio>
#include <thread>
#include <mutex>

#if PLATFORM_LINUX
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#elif PLATFORM_WINDOWS
#include <Windows.h>
#endif

static PackagingWindow sPackagingWindow;

PackagingWindow* GetPackagingWindow()
{
    return &sPackagingWindow;
}

PackagingWindow::PackagingWindow()
{
}

PackagingWindow::~PackagingWindow()
{
    // Clean up build thread if still running
    if (mBuildState.mRunning.load())
    {
        CancelDockerBuild();
    }
    if (mBuildState.mBuildThread.joinable())
    {
        mBuildState.mBuildThread.join();
    }
}

void PackagingWindow::Open()
{
    mIsOpen = true;
    mShowDockerWarning = false;
    mShow3dsLinkWarning = false;
    mShowWiiloadWarning = false;

    // Initialize buffers with selected profile data
    PackagingSettings* settings = PackagingSettings::Get();
    if (settings != nullptr)
    {
        BuildProfile* profile = settings->GetSelectedProfile();
        if (profile != nullptr)
        {
            strncpy(mNameBuffer, profile->mName.c_str(), sizeof(mNameBuffer) - 1);
            mNameBuffer[sizeof(mNameBuffer) - 1] = '\0';

            strncpy(mOutputDirBuffer, profile->mOutputDirectory.c_str(), sizeof(mOutputDirBuffer) - 1);
            mOutputDirBuffer[sizeof(mOutputDirBuffer) - 1] = '\0';
        }
    }
}

void PackagingWindow::Close()
{
    mIsOpen = false;
}

void PackagingWindow::Draw()
{
    if (!mIsOpen)
    {
        return;
    }

    PackagingSettings* settings = PackagingSettings::Get();
    if (settings == nullptr)
    {
        return;
    }

    // Center the window
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 windowSize(700.0f, 500.0f);
    ImVec2 windowPos((io.DisplaySize.x - windowSize.x) * 0.5f, (io.DisplaySize.y - windowSize.y) * 0.5f);
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse;

    if (ImGui::Begin("Packaging", &mIsOpen, windowFlags))
    {
        // Calculate layout dimensions
        float listWidth = 180.0f;
        float buttonHeight = 40.0f;
        float contentHeight = ImGui::GetContentRegionAvail().y - buttonHeight - 16.0f;
        float settingsWidth = ImGui::GetContentRegionAvail().x - listWidth - 8.0f;

        // Left panel - Profile list
        ImGui::BeginChild("ProfileList", ImVec2(listWidth, contentHeight), true);
        DrawProfileList();
        ImGui::EndChild();

        ImGui::SameLine();

        // Right panel - Profile settings
        ImGui::BeginChild("ProfileSettings", ImVec2(settingsWidth, contentHeight), true);
        DrawProfileSettings();
        ImGui::EndChild();

        // Bottom - Build buttons
        ImGui::Spacing();
        DrawBuildButtons();
    }
    ImGui::End();

    // Draw popups
    DrawDockerWarningPopup();
    Draw3dsLinkWarningPopup();
    DrawWiiloadWarningPopup();
    DrawBuildOutputModal();

    // Handle window close
    if (!mIsOpen)
    {
        Close();
    }
}

void PackagingWindow::DrawProfileList()
{
    PackagingSettings* settings = PackagingSettings::Get();
    if (settings == nullptr)
    {
        return;
    }

    // Add/Remove buttons
    if (ImGui::Button("+", ImVec2(24, 0)))
    {
        settings->CreateProfile("New Profile");
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Add new profile");
    }

    ImGui::SameLine();

    BuildProfile* selectedProfile = settings->GetSelectedProfile();
    bool canDelete = selectedProfile != nullptr && settings->GetProfiles().size() > 1;

    if (!canDelete)
    {
        ImGui::BeginDisabled();
    }

    if (ImGui::Button("-", ImVec2(24, 0)))
    {
        if (selectedProfile != nullptr)
        {
            settings->DeleteProfile(selectedProfile->mId);
        }
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Delete selected profile");
    }

    if (!canDelete)
    {
        ImGui::EndDisabled();
    }

    ImGui::Separator();

    // Profile list
    std::vector<BuildProfile>& profiles = settings->GetProfiles();
    int32_t selectedIndex = settings->GetSelectedProfileIndex();

    uint32_t currentTargetId = settings->GetCurrentTargetProfileId();

    for (int32_t i = 0; i < static_cast<int32_t>(profiles.size()); ++i)
    {
        BuildProfile& profile = profiles[i];
        bool isSelected = (i == selectedIndex);
        bool isCurrentTarget = (profile.mId == currentTargetId);

        // Use profile ID for unique ImGui ID to prevent crash with empty names
        ImGui::PushID(static_cast<int>(profile.mId));

        // Build display name with star prefix for current target
        char displayBuffer[512];
        const char* name = profile.mName.empty() ? " " : profile.mName.c_str();
        if (isCurrentTarget)
        {
            snprintf(displayBuffer, sizeof(displayBuffer), "* %s", name);
        }
        else
        {
            snprintf(displayBuffer, sizeof(displayBuffer), "  %s", name);
        }

        if (ImGui::Selectable(displayBuffer, isSelected))
        {
            settings->SetSelectedProfileIndex(i);

            // Update buffers for editing
            strncpy(mNameBuffer, profile.mName.c_str(), sizeof(mNameBuffer) - 1);
            mNameBuffer[sizeof(mNameBuffer) - 1] = '\0';

            strncpy(mOutputDirBuffer, profile.mOutputDirectory.c_str(), sizeof(mOutputDirBuffer) - 1);
            mOutputDirBuffer[sizeof(mOutputDirBuffer) - 1] = '\0';
        }

        // Right-click context menu
        // The PushID above makes "ProfileContextMenu" unique per profile
        ImGui::OpenPopupOnItemClick("ProfileContextMenu", ImGuiPopupFlags_MouseButtonRight);
        if (ImGui::BeginPopup("ProfileContextMenu"))
        {
            if (isCurrentTarget)
            {
                if (ImGui::MenuItem("Clear as Current Target"))
                {
                    settings->SetCurrentTargetProfileId(0);
                }
            }
            else
            {
                if (ImGui::MenuItem("Set as Current Target"))
                {
                    settings->SetCurrentTargetProfileId(profile.mId);
                }
            }
            ImGui::EndPopup();
        }

        ImGui::PopID();
    }
}

void PackagingWindow::DrawProfileSettings()
{
    PackagingSettings* settings = PackagingSettings::Get();
    if (settings == nullptr)
    {
        return;
    }

    BuildProfile* profile = settings->GetSelectedProfile();
    if (profile == nullptr)
    {
        ImGui::TextDisabled("Select a profile from the list.");
        return;
    }

    ImGui::Text("Profile Settings");
    ImGui::Separator();
    ImGui::Spacing();

    bool changed = false;

    // Name
    ImGui::Text("Name:");
    ImGui::SetNextItemWidth(-1);
    if (ImGui::InputText("##Name", mNameBuffer, sizeof(mNameBuffer)))
    {
        profile->mName = mNameBuffer;
        changed = true;
    }

    ImGui::Spacing();

    // Platform
    ImGui::Text("Platform:");
    const char* platformNames[] = { "Windows", "Linux", "Android", "GameCube", "Wii", "3DS" };
    int platformIndex = static_cast<int>(profile->mTargetPlatform);
    ImGui::SetNextItemWidth(-1);
    if (ImGui::Combo("##Platform", &platformIndex, platformNames, IM_ARRAYSIZE(platformNames)))
    {
        profile->mTargetPlatform = static_cast<Platform>(platformIndex);
        changed = true;
    }

    ImGui::Spacing();

    // Embedded mode
    if (ImGui::Checkbox("Embedded Mode", &profile->mEmbedded))
    {
        changed = true;
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Embed assets into the executable");
    }

    ImGui::Spacing();

    // Output directory
    ImGui::Text("Output Directory:");
    float browseWidth = 70.0f;
    float spacing = ImGui::GetStyle().ItemSpacing.x;
    float inputWidth = ImGui::GetContentRegionAvail().x - browseWidth - spacing;

    ImGui::SetNextItemWidth(inputWidth);
    if (ImGui::InputText("##OutputDir", mOutputDirBuffer, sizeof(mOutputDirBuffer)))
    {
        profile->mOutputDirectory = mOutputDirBuffer;
        changed = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Browse...", ImVec2(browseWidth, 0)))
    {
        std::string folder = SYS_SelectFolderDialog();
        if (!folder.empty())
        {
            strncpy(mOutputDirBuffer, folder.c_str(), sizeof(mOutputDirBuffer) - 1);
            mOutputDirBuffer[sizeof(mOutputDirBuffer) - 1] = '\0';
            profile->mOutputDirectory = folder;
            changed = true;
        }
    }
    ImGui::TextDisabled("Leave empty for default: Packaged/{Platform}/");

    ImGui::Spacing();

    // Use Docker checkbox (optional on all platforms — Windows builds GCN/Wii/3DS natively)
    if (ImGui::Checkbox("Use Docker", &profile->mUseDocker))
    {
        changed = true;
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Use Docker for building instead of local tools");
    }

    ImGui::Spacing();

    // Open directory on finish
    if (ImGui::Checkbox("Open Directory On Finish", &profile->mOpenDirectoryOnFinish))
    {
        changed = true;
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Open the output directory when build completes");
    }

    if (changed)
    {
        settings->SaveSettings();
    }
}

void PackagingWindow::DrawBuildButtons()
{
    PackagingSettings* settings = PackagingSettings::Get();
    BuildProfile* profile = settings ? settings->GetSelectedProfile() : nullptr;

    bool canBuild = (profile != nullptr) && !mBuildInProgress;

    if (!canBuild)
    {
        ImGui::BeginDisabled();
    }

    float buttonWidth = 100.0f;
    float dropdownButtonWidth = 130.0f;
    float deviceButtonWidth = 150.0f;
    float arrowWidth = 20.0f;
    float gearWidth = 30.0f;

    if (ImGui::Button("Build", ImVec2(buttonWidth, 0)))
    {
        OnBuild();
    }

    ImGui::SameLine();

    bool supportsRun = profile && PlatformSupportsRun(profile->mTargetPlatform);
    bool is3DS = profile && profile->mTargetPlatform == Platform::N3DS;
    bool isWii = profile && profile->mTargetPlatform == Platform::Wii;

    if (!supportsRun)
    {
        ImGui::BeginDisabled();
    }

    // For Wii: Show dropdown with Dolphin and Wii LAN options
    if (isWii)
    {
        // Main button
        if (ImGui::Button("Build & Run", ImVec2(dropdownButtonWidth, 0)))
        {
            ImGui::OpenPopup("WiiBuildRunMenu");
        }

        // Dropdown arrow
        ImGui::SameLine(0, 0);
        if (ImGui::ArrowButton("##WiiRunArrow", ImGuiDir_Down))
        {
            ImGui::OpenPopup("WiiBuildRunMenu");
        }

        // Popup menu
        if (ImGui::BeginPopup("WiiBuildRunMenu"))
        {
            if (ImGui::MenuItem("Dolphin", nullptr, false))
            {
                OnBuildAndRun();
            }
            if (ImGui::MenuItem("Wii LAN", nullptr, false))
            {
                OnBuildAndRunOnDevice();
            }
            ImGui::EndPopup();
        }
    }
    else
    {
        // Standard Build & Run button for other platforms
        if (ImGui::Button("Build & Run", ImVec2(buttonWidth, 0)))
        {
            OnBuildAndRun();
        }
    }

    if (!supportsRun)
    {
        ImGui::EndDisabled();
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        {
            ImGui::SetTooltip("Build & Run is only available for GameCube, Wii, and 3DS");
        }
    }

    // Show "Build & Run On Device" button only for 3DS
    if (is3DS)
    {
        ImGui::SameLine();

        if (ImGui::Button("Build & Run On Device", ImVec2(deviceButtonWidth, 0)))
        {
            OnBuildAndRunOnDevice();
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Build and send to 3DS hardware via 3dslink");
        }
    }

    ImGui::SameLine();

    // Gear icon for launcher settings
    if (ImGui::Button("...", ImVec2(gearWidth, 0)))
    {
        OpenLauncherSettings();
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Configure emulator paths");
    }

    if (!canBuild)
    {
        ImGui::EndDisabled();
    }

    if (mBuildInProgress)
    {
        ImGui::SameLine();
        ImGui::TextDisabled("Building...");
    }
}

void PackagingWindow::DrawDockerWarningPopup()
{
    if (mShowDockerWarning)
    {
        ImGui::OpenPopup("Docker Required");
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Docker Required", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Docker is not running or not installed.");
        ImGui::Spacing();

#if PLATFORM_WINDOWS
        ImGui::TextWrapped("Please ensure Docker Desktop is installed and running to build for this platform.");
#else
        ImGui::TextWrapped("Please install Docker and ensure the daemon is running.");
#endif

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        float buttonWidth = 80.0f;
        float windowWidth = ImGui::GetWindowSize().x;
        ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);

        if (ImGui::Button("OK", ImVec2(buttonWidth, 0)))
        {
            mShowDockerWarning = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void PackagingWindow::Draw3dsLinkWarningPopup()
{
    if (mShow3dsLinkWarning)
    {
        ImGui::OpenPopup("3DS Hardware Transfer");
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("3DS Hardware Transfer", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Please make sure your 3DS has Homebrew");
        ImGui::Text("Launcher open and is ready to receive");
        ImGui::Text("files via 3dslink.");
        ImGui::Spacing();
        ImGui::Text("Both devices must be on the same network.");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        float buttonWidth = 80.0f;

        if (ImGui::Button("Send", ImVec2(buttonWidth, 0)))
        {
            mShow3dsLinkWarning = false;
            ImGui::CloseCurrentPopup();

            // Execute 3dslink
            Launch3dsLink(mPendingOutputPath);
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0)))
        {
            mShow3dsLinkWarning = false;
            mPendingOutputPath.clear();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void PackagingWindow::DrawWiiloadWarningPopup()
{
    if (mShowWiiloadWarning)
    {
        ImGui::OpenPopup("Wii Hardware Transfer");
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Wii Hardware Transfer", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Please make sure your Wii has Homebrew");
        ImGui::Text("Channel open and is ready to receive");
        ImGui::Text("files via wiiload.");
        ImGui::Spacing();
        ImGui::Text("Both devices must be on the same network.");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        float buttonWidth = 80.0f;

        if (ImGui::Button("Send", ImVec2(buttonWidth, 0)))
        {
            mShowWiiloadWarning = false;
            ImGui::CloseCurrentPopup();

            // Execute wiiload
            LaunchWiiload(mPendingOutputPath);
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0)))
        {
            mShowWiiloadWarning = false;
            mPendingOutputPath.clear();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void PackagingWindow::OpenLauncherSettings()
{
    PreferencesWindow* prefsWindow = GetPreferencesWindow();
    if (prefsWindow != nullptr)
    {
        prefsWindow->Open();
        prefsWindow->SelectModule("External/Launchers");
    }
}

void PackagingWindow::OnBuild()
{
    ExecuteBuild(false, false);
}

void PackagingWindow::OnBuildAndRun()
{
    ExecuteBuild(true, false);
}

void PackagingWindow::OnBuildAndRunOnDevice()
{
    ExecuteBuild(true, true);
}

void PackagingWindow::ExecuteBuild(bool runAfterBuild, bool runOnDevice)
{
    PackagingSettings* settings = PackagingSettings::Get();
    if (settings == nullptr)
    {
        return;
    }

    BuildProfile* profile = settings->GetSelectedProfile();
    if (profile == nullptr)
    {
        LogError("No build profile selected");
        return;
    }

    bool useDocker = profile->mUseDocker;

    if (useDocker)
    {
        if (!CheckDockerAvailable())
        {
            mShowDockerWarning = true;
            return;
        }
        ExecuteDockerBuild(*profile, runAfterBuild, runOnDevice);
    }
    else
    {
        ExecuteLocalBuild(*profile, runAfterBuild, runOnDevice);
    }
}

void PackagingWindow::BuildAndRunWithProfile(Platform platform, bool embedded, bool runOnDevice)
{
    // Try to find an existing profile for this platform
    PackagingSettings* settings = PackagingSettings::Get();
    BuildProfile* existingProfile = nullptr;

    if (settings != nullptr)
    {
        std::vector<BuildProfile>& profiles = settings->GetProfiles();
        for (BuildProfile& p : profiles)
        {
            if (p.mTargetPlatform == platform)
            {
                existingProfile = &p;
                break;
            }
        }
    }

    BuildProfile tempProfile;
    const BuildProfile& profile = existingProfile ? *existingProfile : tempProfile;

    if (!existingProfile)
    {
        tempProfile.mName = "Quick Play";
        tempProfile.mTargetPlatform = platform;
        tempProfile.mEmbedded = embedded;
        tempProfile.mOpenDirectoryOnFinish = false;
    }

    bool useDocker = profile.mUseDocker;

    if (useDocker)
    {
        if (!CheckDockerAvailable())
        {
            mShowDockerWarning = true;
            return;
        }
        ExecuteDockerBuild(profile, true, runOnDevice);
    }
    else
    {
        ExecuteLocalBuild(profile, true, runOnDevice);
    }
}

void PackagingWindow::ExecuteDockerBuild(const BuildProfile& profile, bool runAfterBuild, bool runOnDevice)
{
    LogDebug("Starting Docker build for platform: %s", GetPlatformString(profile.mTargetPlatform));

    std::string dockerCmd = BuildDockerCommand(profile);
    if (dockerCmd.empty())
    {
        LogError("Failed to build Docker command");
        return;
    }

    // Start async build with modal
    StartAsyncDockerBuild(profile, runAfterBuild, runOnDevice);
}

void PackagingWindow::ExecuteLocalBuild(const BuildProfile& profile, bool runAfterBuild, bool runOnDevice)
{
    LogDebug("Starting local build for platform: %s", GetPlatformString(profile.mTargetPlatform));

    mBuildInProgress = true;

    // Delegate to ActionManager's BuildData (now non-blocking)
    ActionManager* am = ActionManager::Get();
    if (am != nullptr)
    {
        am->BuildData(profile.mTargetPlatform, profile.mEmbedded);
        // Set run-after-build flags after BuildData (which resets state for non-headless)
        am->GetBuildState().mRunAfterBuild = runAfterBuild;
        am->GetBuildState().mRunOnDevice = runOnDevice;
    }

    mBuildInProgress = false;
}

void PackagingWindow::LaunchEmulator(const BuildProfile& profile, const std::string& outputPath)
{
    if (!PlatformSupportsRun(profile.mTargetPlatform))
    {
        return;
    }

    // Get launcher settings from Preferences module
    LaunchersModule* launchers = static_cast<LaunchersModule*>(
        PreferencesManager::Get()->FindModule("External/Launchers"));

    if (launchers == nullptr)
    {
        LogError("Launchers module not found");
        return;
    }

    // Validate emulator is configured
    if (!launchers->IsEmulatorConfigured(profile.mTargetPlatform))
    {
        LogError("Emulator not configured for %s. Open Preferences > External > Launchers to configure.",
                 GetPlatformString(profile.mTargetPlatform));
        return;
    }

    // Build and execute the command using user's custom args
    std::string cmd = launchers->BuildLaunchCommand(profile.mTargetPlatform, outputPath);

    LogDebug("Launching emulator: %s", cmd.c_str());
    SYS_Exec(cmd.c_str());
}

void PackagingWindow::Launch3dsLink(const std::string& outputPath)
{
    // Get launcher settings from Preferences module
    LaunchersModule* launchers = static_cast<LaunchersModule*>(
        PreferencesManager::Get()->FindModule("External/Launchers"));

    if (launchers == nullptr)
    {
        LogError("Launchers module not found");
        return;
    }

    // Check if 3dslink is available
    if (!launchers->Is3dsLinkConfigured())
    {
        LogError("3dslink not available. Please ensure devkitPro is installed.");
        return;
    }

    // Build and execute the 3dslink command
    std::string cmd = launchers->Build3dsLinkCommand(outputPath);

    if (cmd.empty())
    {
        LogError("Failed to build 3dslink command");
        return;
    }

    LogDebug("Launching 3dslink: %s", cmd.c_str());
    SYS_Exec(cmd.c_str());
}

void PackagingWindow::LaunchWiiload(const std::string& outputPath)
{
    // Get launcher settings from Preferences module
    LaunchersModule* launchers = static_cast<LaunchersModule*>(
        PreferencesManager::Get()->FindModule("External/Launchers"));

    if (launchers == nullptr)
    {
        LogError("Launchers module not found");
        return;
    }

    // Check if wiiload is available
    if (!launchers->IsWiiloadConfigured())
    {
        LogError("wiiload not available. Please ensure devkitPro is installed and Wii IP is configured in Preferences > External > Launchers.");
        return;
    }

    // Build and execute the wiiload command
    std::string cmd = launchers->BuildWiiloadCommand(outputPath);

    if (cmd.empty())
    {
        LogError("Failed to build wiiload command");
        return;
    }

    LogDebug("Launching wiiload: %s", cmd.c_str());
    SYS_Exec(cmd.c_str());
}

bool PackagingWindow::CheckDockerAvailable()
{
    ExternalModule* ext = static_cast<ExternalModule*>(
        PreferencesManager::Get()->FindModule("External"));
    std::string cmd = ext ? ext->GetDockerCommand() : "docker";
    cmd += " --version";

    std::string output;
    SYS_Exec(cmd.c_str(), &output);
    return !output.empty() && output.find("Docker") != std::string::npos;
}

std::string PackagingWindow::BuildDockerCommand(const BuildProfile& profile)
{
    const EngineState* engine = GetEngineState();
    std::string projectDir = engine->mProjectDirectory;

    std::string outputDir = GetOutputDirectory(profile);

    std::string buildCmd;
    switch (profile.mTargetPlatform)
    {
        case Platform::Linux:    buildCmd = "build-linux"; break;
        case Platform::GameCube: buildCmd = "build-gcn";   break;
        case Platform::Wii:      buildCmd = "build-wii";   break;
        case Platform::N3DS:     buildCmd = "build-3ds";   break;
        default:
            LogError("Docker build not supported for platform: %s", GetPlatformString(profile.mTargetPlatform));
            return "";
    }

    // Get Docker image from preferences
    DockerModule* dockerModule = static_cast<DockerModule*>(
        PreferencesManager::Get()->FindModule("Packaging/Docker"));
    std::string dockerImage = dockerModule ? dockerModule->GetDockerImage() : "vltmedia/octavegameengine-linux:dev";

    // Get Docker executable from External preferences
    ExternalModule* ext = static_cast<ExternalModule*>(
        PreferencesManager::Get()->FindModule("External"));
    std::string dockerExe = ext ? ext->GetDockerCommand() : "docker";

    std::string cmd = dockerExe + " run --rm "
                      "-v \"" + outputDir + ":/game\" "
                      "-v \"" + projectDir + ":/project\" "
                      "\"" + dockerImage + "\" " + buildCmd;

    return cmd;
}

std::string PackagingWindow::GetOutputDirectory(const BuildProfile& profile)
{
    if (!profile.mOutputDirectory.empty())
    {
        std::string dir = profile.mOutputDirectory;
        // Ensure trailing slash
        if (dir.back() != '/' && dir.back() != '\\')
        {
            dir += '/';
        }
        return dir;
    }

    const EngineState* engine = GetEngineState();
    return engine->mProjectDirectory + "Packaged/" + GetPlatformString(profile.mTargetPlatform) + "/";
}

void PackagingWindow::StartAsyncDockerBuild(const BuildProfile& profile, bool runAfterBuild, bool runOnDevice)
{
    // Wait for any previous build to complete
    if (mBuildState.mBuildThread.joinable())
    {
        mBuildState.mBuildThread.join();
    }

    // Reset state
    mBuildState.mRunning.store(true);
    mBuildState.mCancelRequested.store(false);
    mBuildState.mComplete.store(false);
    mBuildState.mSuccess.store(false);
    mBuildState.mExitCode.store(0);
#if PLATFORM_LINUX
    mBuildState.mProcessId = 0;
#elif PLATFORM_WINDOWS
    mBuildState.mProcessHandle = nullptr;
#endif

    {
        std::lock_guard<std::mutex> lock(mBuildState.mOutputMutex);
        mBuildState.mOutput.clear();
        mBuildState.mOutputDirty = false;
    }

    // Store build configuration
    mBuildState.mCommand = BuildDockerCommand(profile);
    mBuildState.mRunAfterBuild = runAfterBuild;
    mBuildState.mRunOnDevice = runOnDevice;
    mBuildState.mOpenDirectoryOnFinish = profile.mOpenDirectoryOnFinish;
    mBuildState.mTargetPlatform = profile.mTargetPlatform;

    // Compute output path
    std::string outputDir = GetOutputDirectory(profile);
    const EngineState* engine = GetEngineState();
    std::string projectName = engine->mProjectName;
    std::string extension = GetPlatformOutputExtension(profile.mTargetPlatform);
    mBuildState.mOutputPath = outputDir + projectName + extension;

    // Reset display state
    mDisplayOutput.clear();
    mAutoScroll = true;

    // Show modal and mark build in progress
    mShowBuildModal = true;
    mBuildInProgress = true;

    // Add command to output
    {
        std::lock_guard<std::mutex> lock(mBuildState.mOutputMutex);
        mBuildState.mOutput = "[CMD] " + mBuildState.mCommand + "\n\n";
        mBuildState.mOutputDirty = true;
    }

    LogDebug("Executing async: %s", mBuildState.mCommand.c_str());

    // Launch build thread
    mBuildState.mBuildThread = std::thread(&PackagingWindow::DockerBuildThreadFunc, this);
}

void PackagingWindow::DockerBuildThreadFunc()
{
#if PLATFORM_LINUX
    // Create pipe for capturing output
    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
        std::lock_guard<std::mutex> lock(mBuildState.mOutputMutex);
        mBuildState.mOutput += "[ERROR] Failed to create pipe\n";
        mBuildState.mOutputDirty = true;
        mBuildState.mSuccess.store(false);
        mBuildState.mExitCode.store(-1);
        mBuildState.mComplete.store(true);
        mBuildState.mRunning.store(false);
        return;
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        close(pipefd[0]);
        close(pipefd[1]);
        std::lock_guard<std::mutex> lock(mBuildState.mOutputMutex);
        mBuildState.mOutput += "[ERROR] Failed to fork process\n";
        mBuildState.mOutputDirty = true;
        mBuildState.mSuccess.store(false);
        mBuildState.mExitCode.store(-1);
        mBuildState.mComplete.store(true);
        mBuildState.mRunning.store(false);
        return;
    }

    if (pid == 0)
    {
        // Child process
        close(pipefd[0]); // Close read end

        // Redirect stdout and stderr to pipe
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);

        // Execute command via shell
        execl("/bin/sh", "sh", "-c", mBuildState.mCommand.c_str(), nullptr);
        _exit(127); // exec failed
    }

    // Parent process
    mBuildState.mProcessId = pid;
    close(pipefd[1]); // Close write end

    // Set non-blocking read
    int flags = fcntl(pipefd[0], F_GETFL, 0);
    fcntl(pipefd[0], F_SETFL, flags | O_NONBLOCK);

    char buffer[4096];
    bool processRunning = true;

    while (processRunning && !mBuildState.mCancelRequested.load())
    {
        // Try to read output
        ssize_t bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1);
        if (bytesRead > 0)
        {
            buffer[bytesRead] = '\0';
            std::lock_guard<std::mutex> lock(mBuildState.mOutputMutex);
            mBuildState.mOutput += buffer;
            mBuildState.mOutputDirty = true;
        }

        // Check if process is still running
        int status;
        pid_t result = waitpid(pid, &status, WNOHANG);
        if (result == pid)
        {
            // Process finished - read any remaining output
            while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0)
            {
                buffer[bytesRead] = '\0';
                std::lock_guard<std::mutex> lock(mBuildState.mOutputMutex);
                mBuildState.mOutput += buffer;
                mBuildState.mOutputDirty = true;
            }

            if (WIFEXITED(status))
            {
                int exitCode = WEXITSTATUS(status);
                mBuildState.mExitCode.store(exitCode);
                mBuildState.mSuccess.store(exitCode == 0);
            }
            else
            {
                mBuildState.mExitCode.store(-1);
                mBuildState.mSuccess.store(false);
            }
            processRunning = false;
        }
        else if (result == -1)
        {
            // Error checking process
            processRunning = false;
            mBuildState.mSuccess.store(false);
        }
        else
        {
            // Process still running, sleep briefly
            usleep(50000); // 50ms
        }
    }

    close(pipefd[0]);

    // Handle cancellation
    if (mBuildState.mCancelRequested.load() && processRunning)
    {
        kill(pid, SIGTERM);
        usleep(100000); // 100ms grace period
        kill(pid, SIGKILL);
        waitpid(pid, nullptr, 0);

        std::lock_guard<std::mutex> lock(mBuildState.mOutputMutex);
        mBuildState.mOutput += "\n[CANCELLED] Build was cancelled by user.\n";
        mBuildState.mOutputDirty = true;
        mBuildState.mSuccess.store(false);
    }

#elif PLATFORM_WINDOWS
    // Create pipes for stdout/stderr
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = nullptr;

    HANDLE hStdOutRead, hStdOutWrite;
    if (!CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 0))
    {
        std::lock_guard<std::mutex> lock(mBuildState.mOutputMutex);
        mBuildState.mOutput += "[ERROR] Failed to create pipe\n";
        mBuildState.mOutputDirty = true;
        mBuildState.mSuccess.store(false);
        mBuildState.mComplete.store(true);
        mBuildState.mRunning.store(false);
        return;
    }

    SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOA si = {};
    si.cb = sizeof(si);
    si.hStdOutput = hStdOutWrite;
    si.hStdError = hStdOutWrite;
    si.dwFlags |= STARTF_USESTDHANDLES;

    PROCESS_INFORMATION pi = {};

    std::string cmdLine = "cmd /c " + mBuildState.mCommand;
    if (!CreateProcessA(nullptr, const_cast<char*>(cmdLine.c_str()), nullptr, nullptr,
                        TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi))
    {
        CloseHandle(hStdOutRead);
        CloseHandle(hStdOutWrite);

        std::lock_guard<std::mutex> lock(mBuildState.mOutputMutex);
        mBuildState.mOutput += "[ERROR] Failed to create process\n";
        mBuildState.mOutputDirty = true;
        mBuildState.mSuccess.store(false);
        mBuildState.mComplete.store(true);
        mBuildState.mRunning.store(false);
        return;
    }

    mBuildState.mProcessHandle = pi.hProcess;
    CloseHandle(hStdOutWrite);
    CloseHandle(pi.hThread);

    char buffer[4096];
    DWORD bytesRead;
    DWORD bytesAvailable;

    while (!mBuildState.mCancelRequested.load())
    {
        // Check if data available
        if (PeekNamedPipe(hStdOutRead, nullptr, 0, nullptr, &bytesAvailable, nullptr) && bytesAvailable > 0)
        {
            if (ReadFile(hStdOutRead, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) && bytesRead > 0)
            {
                buffer[bytesRead] = '\0';
                std::lock_guard<std::mutex> lock(mBuildState.mOutputMutex);
                mBuildState.mOutput += buffer;
                mBuildState.mOutputDirty = true;
            }
        }

        // Check if process finished
        DWORD exitCode;
        if (GetExitCodeProcess(pi.hProcess, &exitCode))
        {
            if (exitCode != STILL_ACTIVE)
            {
                // Read any remaining output
                while (PeekNamedPipe(hStdOutRead, nullptr, 0, nullptr, &bytesAvailable, nullptr) && bytesAvailable > 0)
                {
                    if (ReadFile(hStdOutRead, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) && bytesRead > 0)
                    {
                        buffer[bytesRead] = '\0';
                        std::lock_guard<std::mutex> lock(mBuildState.mOutputMutex);
                        mBuildState.mOutput += buffer;
                        mBuildState.mOutputDirty = true;
                    }
                }

                mBuildState.mExitCode.store(static_cast<int>(exitCode));
                mBuildState.mSuccess.store(exitCode == 0);
                break;
            }
        }

        Sleep(50); // 50ms
    }

    // Handle cancellation
    if (mBuildState.mCancelRequested.load())
    {
        TerminateProcess(pi.hProcess, 1);
        std::lock_guard<std::mutex> lock(mBuildState.mOutputMutex);
        mBuildState.mOutput += "\n[CANCELLED] Build was cancelled by user.\n";
        mBuildState.mOutputDirty = true;
        mBuildState.mSuccess.store(false);
    }

    CloseHandle(hStdOutRead);
    CloseHandle(pi.hProcess);
    mBuildState.mProcessHandle = nullptr;
#endif

    mBuildState.mComplete.store(true);
    mBuildState.mRunning.store(false);
}

void PackagingWindow::CancelDockerBuild()
{
    if (!mBuildState.mRunning.load())
    {
        return;
    }

    mBuildState.mCancelRequested.store(true);

#if PLATFORM_LINUX
    if (mBuildState.mProcessId > 0)
    {
        kill(mBuildState.mProcessId, SIGTERM);
    }
#elif PLATFORM_WINDOWS
    if (mBuildState.mProcessHandle != nullptr)
    {
        TerminateProcess(static_cast<HANDLE>(mBuildState.mProcessHandle), 1);
    }
#endif
}

void PackagingWindow::FinalizeBuild()
{
    // Join the build thread
    if (mBuildState.mBuildThread.joinable())
    {
        mBuildState.mBuildThread.join();
    }

    mBuildInProgress = false;

    // Handle post-build actions
    if (mBuildState.mSuccess.load() && mBuildState.mRunAfterBuild)
    {
        if (mBuildState.mRunOnDevice && mBuildState.mTargetPlatform == Platform::N3DS)
        {
            // Show 3dslink warning popup
            mPendingOutputPath = mBuildState.mOutputPath;
            mShow3dsLinkWarning = true;
        }
        else if (mBuildState.mRunOnDevice && mBuildState.mTargetPlatform == Platform::Wii)
        {
            // Show wiiload warning popup
            mPendingOutputPath = mBuildState.mOutputPath;
            mShowWiiloadWarning = true;
        }
        else if (PlatformSupportsRun(mBuildState.mTargetPlatform))
        {
            // Launch emulator
            LaunchersModule* launchers = static_cast<LaunchersModule*>(
                PreferencesManager::Get()->FindModule("External/Launchers"));

            if (launchers != nullptr && launchers->IsEmulatorConfigured(mBuildState.mTargetPlatform))
            {
                std::string cmd = launchers->BuildLaunchCommand(mBuildState.mTargetPlatform, mBuildState.mOutputPath);
                LogDebug("Launching emulator: %s", cmd.c_str());
                SYS_Exec(cmd.c_str());
            }
            else
            {
                LogError("Emulator not configured for %s", GetPlatformString(mBuildState.mTargetPlatform));
            }
        }
    }
    else if (mBuildState.mSuccess.load() && !mBuildState.mRunAfterBuild && mBuildState.mOpenDirectoryOnFinish)
    {
        // Open output directory on successful build without run
        std::string outputDir = mBuildState.mOutputPath;
        size_t lastSlash = outputDir.find_last_of("/\\");
        if (lastSlash != std::string::npos)
        {
            outputDir = outputDir.substr(0, lastSlash + 1);
        }
        SYS_ExplorerOpenDirectory(outputDir.c_str());
    }
}

void PackagingWindow::DrawBuildOutputModal()
{
    if (!mShowBuildModal)
    {
        return;
    }

    // Update display output from shared buffer
    {
        std::lock_guard<std::mutex> lock(mBuildState.mOutputMutex);
        if (mBuildState.mOutputDirty)
        {
            mDisplayOutput = mBuildState.mOutput;
            mBuildState.mOutputDirty = false;
        }
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(720, 500), ImGuiCond_FirstUseEver);

    ImGuiWindowFlags modalFlags = ImGuiWindowFlags_NoCollapse;
    bool finalized = false;

    if (ImGui::Begin("Docker Build", &mShowBuildModal, modalFlags))
    {
        // Status header
        bool isComplete = mBuildState.mComplete.load();
        bool isSuccess = mBuildState.mSuccess.load();
        bool isCancelled = mBuildState.mCancelRequested.load();

        if (!isComplete)
        {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Building...");
        }
        else if (isCancelled)
        {
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Build Cancelled");
        }
        else if (isSuccess)
        {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Build Successful!");
        }
        else
        {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Build Failed (exit code: %d)", mBuildState.mExitCode.load());
        }

        ImGui::Separator();

        // Output window
        float footerHeight = ImGui::GetFrameHeightWithSpacing() + 8.0f;
        ImVec2 outputSize(0, -footerHeight);

        ImGui::BeginChild("BuildOutput", outputSize, true, ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::TextUnformatted(mDisplayOutput.c_str());

        // Auto-scroll to bottom
        if (mAutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 10.0f)
        {
            ImGui::SetScrollHereY(1.0f);
        }
        ImGui::EndChild();

        // Footer with checkboxes and button
        ImGui::Checkbox("Auto-scroll", &mAutoScroll);
        ImGui::SameLine();
        ImGui::Checkbox("Auto-close when finished", &mAutoCloseOnFinish);

        ImGui::SameLine();
        float buttonWidth = 80.0f;
        float availWidth = ImGui::GetContentRegionAvail().x;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + availWidth - buttonWidth);

        // Auto-close on successful completion
        if (isComplete && mAutoCloseOnFinish && isSuccess && !isCancelled)
        {
            mShowBuildModal = false;
            FinalizeBuild();
            finalized = true;
        }

        if (!finalized && isComplete)
        {
            if (ImGui::Button("Close", ImVec2(buttonWidth, 0)))
            {
                mShowBuildModal = false;
                FinalizeBuild();
                finalized = true;
            }
        }
        else if (!isComplete)
        {
            if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0)))
            {
                CancelDockerBuild();
            }
        }
    }
    ImGui::End();

    // Handle window close via X button (skip if already finalized above)
    if (!mShowBuildModal && !finalized)
    {
        if (mBuildState.mRunning.load())
        {
            CancelDockerBuild();
        }
        // Wait briefly for thread to finish before finalizing
        if (mBuildState.mComplete.load())
        {
            FinalizeBuild();
        }
        else
        {
            // Re-show modal until build completes
            mShowBuildModal = true;
        }
    }
}

#endif
