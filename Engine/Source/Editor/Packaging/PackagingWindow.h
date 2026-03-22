#pragma once

#if EDITOR

#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#if PLATFORM_LINUX
#include <sys/types.h>
#endif
#include "BuildProfile.h"

/**
 * @brief State for async Docker build operations.
 */
struct DockerBuildState
{
    std::thread mBuildThread;
    std::atomic<bool> mRunning{false};
    std::atomic<bool> mCancelRequested{false};
    std::atomic<bool> mComplete{false};
    std::atomic<bool> mSuccess{false};
    std::atomic<int> mExitCode{0};

    std::mutex mOutputMutex;
    std::string mOutput;
    bool mOutputDirty{false};

    std::string mCommand;
    bool mRunAfterBuild{false};
    bool mRunOnDevice{false};
    bool mOpenDirectoryOnFinish{true};
    Platform mTargetPlatform{Platform::Linux};
    std::string mOutputPath;

#if PLATFORM_LINUX
    pid_t mProcessId{0};
#elif PLATFORM_WINDOWS
    void* mProcessHandle{nullptr};
#endif
};

/**
 * @brief Window for managing build profiles and packaging projects.
 *
 * The Packaging Window provides a UI for creating, editing, and managing
 * build profiles. It also handles build execution with Docker integration
 * for console platforms and emulator launching for testing.
 */
class PackagingWindow
{
public:
    PackagingWindow();
    ~PackagingWindow();

    /**
     * @brief Opens the packaging window.
     */
    void Open();

    /**
     * @brief Closes the packaging window.
     */
    void Close();

    /**
     * @brief Draws the packaging window UI.
     * Should be called each frame from the main render loop.
     */
    void Draw();

    /**
     * @brief Checks if the window is currently open.
     * @return True if the window is open
     */
    bool IsOpen() const { return mIsOpen; }

    /**
     * @brief Triggers a build+run for a given platform without requiring a saved profile.
     *
     * If a matching profile exists in PackagingSettings, it will be used.
     * Otherwise a temporary embedded profile is created.
     *
     * @param platform Target platform to build for
     * @param embedded Whether to embed assets into the executable
     * @param runOnDevice If true, uses 3dslink instead of emulator (3DS only)
     */
    void BuildAndRunWithProfile(Platform platform, bool embedded, bool runOnDevice = false);

private:
    /**
     * @brief Draws the left panel with profile list.
     */
    void DrawProfileList();

    /**
     * @brief Draws the right panel with profile settings.
     */
    void DrawProfileSettings();

    /**
     * @brief Draws the build buttons at the bottom.
     */
    void DrawBuildButtons();

    /**
     * @brief Draws the Docker warning popup.
     */
    void DrawDockerWarningPopup();

    /**
     * @brief Draws the 3dslink warning popup.
     */
    void Draw3dsLinkWarningPopup();

    /**
     * @brief Draws the wiiload warning popup.
     */
    void DrawWiiloadWarningPopup();

    /**
     * @brief Draws the Docker build output modal.
     */
    void DrawBuildOutputModal();

    /**
     * @brief Opens the Preferences window and navigates to Launchers.
     */
    void OpenLauncherSettings();

    /**
     * @brief Initiates a build without running.
     */
    void OnBuild();

    /**
     * @brief Initiates a build and runs the result in emulator.
     */
    void OnBuildAndRun();

    /**
     * @brief Initiates a build and runs on 3DS hardware via 3dslink.
     */
    void OnBuildAndRunOnDevice();

    /**
     * @brief Executes a build with the selected profile.
     * @param runAfterBuild If true, launches emulator after successful build
     * @param runOnDevice If true, uses 3dslink instead of emulator (3DS only)
     */
    void ExecuteBuild(bool runAfterBuild, bool runOnDevice = false);

    /**
     * @brief Executes a Docker-based build.
     * @param profile The build profile to use
     * @param runAfterBuild If true, launches emulator after successful build
     * @param runOnDevice If true, uses 3dslink instead of emulator (3DS only)
     */
    void ExecuteDockerBuild(const BuildProfile& profile, bool runAfterBuild, bool runOnDevice = false);

    /**
     * @brief Starts an asynchronous Docker build in a background thread.
     * @param profile The build profile to use
     * @param runAfterBuild If true, launches emulator after successful build
     * @param runOnDevice If true, uses 3dslink instead of emulator (3DS only)
     */
    void StartAsyncDockerBuild(const BuildProfile& profile, bool runAfterBuild, bool runOnDevice);

    /**
     * @brief Thread function that executes the Docker build process.
     */
    void DockerBuildThreadFunc();

    /**
     * @brief Cancels the current Docker build.
     */
    void CancelDockerBuild();

    /**
     * @brief Finalizes the build after thread completion.
     */
    void FinalizeBuild();

    /**
     * @brief Executes a local build (non-Docker).
     * @param profile The build profile to use
     * @param runAfterBuild If true, launches emulator after successful build
     * @param runOnDevice If true, uses 3dslink instead of emulator (3DS only)
     */
    void ExecuteLocalBuild(const BuildProfile& profile, bool runAfterBuild, bool runOnDevice = false);

    /**
     * @brief Launches the emulator for the given profile.
     * @param profile The build profile that was built
     * @param outputPath Path to the built executable
     */
    void LaunchEmulator(const BuildProfile& profile, const std::string& outputPath);

    /**
     * @brief Sends build to 3DS hardware via 3dslink.
     * @param outputPath Path to the built .3dsx file
     */
    void Launch3dsLink(const std::string& outputPath);

    /**
     * @brief Sends build to Wii hardware via wiiload.
     * @param outputPath Path to the built .dol/.elf file
     */
    void LaunchWiiload(const std::string& outputPath);

    /**
     * @brief Checks if Docker is available on the system.
     * @return True if Docker is installed and running
     */
    bool CheckDockerAvailable();

    /**
     * @brief Builds the Docker command for a given profile.
     * @param profile The build profile
     * @return The Docker command string
     */
    std::string BuildDockerCommand(const BuildProfile& profile);

    /**
     * @brief Gets the output directory for a build profile.
     * @param profile The build profile
     * @return The output directory path
     */
    std::string GetOutputDirectory(const BuildProfile& profile);

    /** @brief Whether the window is currently open */
    bool mIsOpen = false;

    /** @brief Whether to show the Docker warning popup */
    bool mShowDockerWarning = false;

    /** @brief Whether to show the 3dslink warning popup */
    bool mShow3dsLinkWarning = false;

    /** @brief Whether to show the wiiload warning popup */
    bool mShowWiiloadWarning = false;

    /** @brief Whether a build is currently in progress */
    bool mBuildInProgress = false;

    /** @brief Pending output path for 3dslink after warning confirmation */
    std::string mPendingOutputPath;

    /** @brief Buffer for profile name editing */
    char mNameBuffer[256] = {};

    /** @brief Buffer for output directory editing */
    char mOutputDirBuffer[512] = {};

    /** @brief Whether to show the Docker build modal */
    bool mShowBuildModal = false;

    /** @brief State for async Docker build */
    DockerBuildState mBuildState;

    /** @brief Display copy of build output (main thread only) */
    std::string mDisplayOutput;

    /** @brief Whether to auto-scroll the build output */
    bool mAutoScroll = true;

    /** @brief Whether to auto-close the modal when build finishes */
    bool mAutoCloseOnFinish = false;
};

/**
 * @brief Gets the global PackagingWindow instance.
 * @return Pointer to the singleton PackagingWindow
 */
PackagingWindow* GetPackagingWindow();

#endif
