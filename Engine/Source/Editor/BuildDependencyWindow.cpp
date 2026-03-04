#if EDITOR

#include "BuildDependencyWindow.h"
#include "EditorUtils.h"
#include "Preferences/PreferencesManager.h"
#include "Preferences/General/GeneralModule.h"
#include "Preferences/External/ExternalModule.h"

#include "Log.h"
#include "System/System.h"

#include "imgui.h"

static BuildDependencyWindow sBuildDependencyWindow;

BuildDependencyWindow* GetBuildDependencyWindow()
{
    return &sBuildDependencyWindow;
}

void BuildDependencyWindow::CheckMake()
{
    BuildDependency dep;
    dep.mName = "make";
    dep.mDescription = "Required for GameCube / Wii / 3DS builds";
#if PLATFORM_WINDOWS
    dep.mInstallHint = "Install MSYS2 and run: pacman -S make";
    dep.mInstallUrl = "https://www.msys2.org/";
#else
    dep.mInstallHint = "Install via: sudo apt install build-essential";
    dep.mInstallUrl = "https://www.gnu.org/software/make/";
#endif

    std::string output;
    SYS_Exec("make --version", &output);

    if (!output.empty() && output.find("Make") != std::string::npos)
    {
        dep.mStatus = DependencyStatus::Found;
        // Extract first line as version
        size_t newline = output.find('\n');
        dep.mVersion = (newline != std::string::npos) ? output.substr(0, newline) : output;
    }
    else
    {
        dep.mStatus = DependencyStatus::NotFound;
    }

    mDependencies.push_back(dep);
}

void BuildDependencyWindow::CheckDevkitPro()
{
    BuildDependency dep;
    dep.mName = "devkitPro";
    dep.mDescription = "Required for GameCube / Wii / 3DS builds";
    dep.mInstallHint = "Download the devkitPro installer";
    dep.mInstallUrl = "https://devkitpro.org/wiki/Getting_Started";

    std::string dkpPath = GetDevkitproPath();

    if (!dkpPath.empty())
    {
        dep.mStatus = DependencyStatus::Found;
        dep.mVersion = dkpPath;
    }
    else
    {
        dep.mStatus = DependencyStatus::NotFound;
    }

    mDependencies.push_back(dep);
}

void BuildDependencyWindow::CheckDocker()
{
    BuildDependency dep;
    dep.mName = "Docker";
    dep.mDescription = "Optional for cross-compilation (e.g. Linux builds)";
    dep.mInstallHint = "Download and install Docker Desktop";
    dep.mInstallUrl = "https://docs.docker.com/get-docker/";

    ExternalModule* ext = static_cast<ExternalModule*>(
        PreferencesManager::Get()->FindModule("External"));
    std::string dockerCmd = ext ? ext->GetDockerCommand() : "docker";
    dockerCmd += " --version";

    std::string output;
    SYS_Exec(dockerCmd.c_str(), &output);

    if (!output.empty() && output.find("Docker") != std::string::npos)
    {
        dep.mStatus = DependencyStatus::Found;
        size_t newline = output.find('\n');
        dep.mVersion = (newline != std::string::npos) ? output.substr(0, newline) : output;
    }
    else
    {
        dep.mStatus = DependencyStatus::NotFound;
    }

    mDependencies.push_back(dep);
}

void BuildDependencyWindow::CheckVisualStudio()
{
    BuildDependency dep;
    dep.mName = "Visual Studio";
    dep.mDescription = "Required for Windows builds";
    dep.mInstallUrl = "https://visualstudio.microsoft.com/";

#if PLATFORM_WINDOWS
    dep.mInstallHint = "Download and install Visual Studio with C++ workload";

    std::string devenvPath = GetDevenvPath();

    if (!devenvPath.empty())
    {
        dep.mStatus = DependencyStatus::Found;
        dep.mVersion = devenvPath;
    }
    else
    {
        dep.mStatus = DependencyStatus::NotFound;
    }
#else
    dep.mStatus = DependencyStatus::Skipped;
    dep.mInstallHint = "Windows only";
#endif

    mDependencies.push_back(dep);
}

void BuildDependencyWindow::CheckGradle()
{
    BuildDependency dep;
    dep.mName = "Gradle";
    dep.mDescription = "Required for Android builds";
    dep.mInstallHint = "Install Android Studio (includes Gradle)";
    dep.mInstallUrl = "https://developer.android.com/studio";

    ExternalModule* extModule = static_cast<ExternalModule*>(
        PreferencesManager::Get()->FindModule("External"));
    std::string gradleCmd = extModule ? extModule->GetGradleCommand() : "gradle";
    gradleCmd += " --version";

    std::string output;
    SYS_Exec(gradleCmd.c_str(), &output);

    if (!output.empty() && output.find("Gradle") != std::string::npos)
    {
        dep.mStatus = DependencyStatus::Found;
        // Find the "Gradle X.Y.Z" line
        size_t pos = output.find("Gradle ");
        if (pos != std::string::npos)
        {
            size_t newline = output.find('\n', pos);
            dep.mVersion = (newline != std::string::npos) ? output.substr(pos, newline - pos) : output.substr(pos);
        }
    }
    else
    {
        dep.mStatus = DependencyStatus::NotFound;
    }

    mDependencies.push_back(dep);
}

void BuildDependencyWindow::RunChecks()
{
    mDependencies.clear();

    CheckMake();
    CheckDevkitPro();
    CheckDocker();
    CheckVisualStudio();
    CheckGradle();

    // Log results
    for (const BuildDependency& dep : mDependencies)
    {
        switch (dep.mStatus)
        {
        case DependencyStatus::Found:
            LogDebug("[BuildDeps] %s: Found (%s)", dep.mName.c_str(), dep.mVersion.c_str());
            break;
        case DependencyStatus::NotFound:
            LogWarning("[BuildDeps] %s: Not found - install from %s", dep.mName.c_str(), dep.mInstallUrl.c_str());
            break;
        case DependencyStatus::Skipped:
            LogDebug("[BuildDeps] %s: Skipped (not applicable on this platform)", dep.mName.c_str());
            break;
        }
    }
}

bool BuildDependencyWindow::HasMissing() const
{
    for (const BuildDependency& dep : mDependencies)
    {
        if (dep.mStatus == DependencyStatus::NotFound)
        {
            return true;
        }
    }
    return false;
}

void BuildDependencyWindow::Open()
{
    RunChecks();
    mIsOpen = true;
}

bool BuildDependencyWindow::IsOpen() const
{
    return mIsOpen;
}

void BuildDependencyWindow::Draw()
{
    if (!mIsOpen)
    {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 windowSize(600.0f, 400.0f);
    ImVec2 windowPos((io.DisplaySize.x - windowSize.x) * 0.5f, (io.DisplaySize.y - windowSize.y) * 0.5f);
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoCollapse;

    if (ImGui::Begin("Build Dependencies", &mIsOpen, windowFlags))
    {
        ImGui::TextWrapped("Status of external tools required for building to various platforms.");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::BeginTable("DepsTable", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 24.0f);
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 110.0f);
            ImGui::TableSetupColumn("Details", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableHeadersRow();

            for (size_t i = 0; i < mDependencies.size(); ++i)
            {
                const BuildDependency& dep = mDependencies[i];
                ImGui::TableNextRow();
                ImGui::PushID((int)i);

                // Status indicator
                ImGui::TableNextColumn();
                ImVec4 statusColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
                const char* statusIcon = "-";
                switch (dep.mStatus)
                {
                case DependencyStatus::Found:
                    statusColor = ImVec4(0.2f, 0.8f, 0.2f, 1.0f);
                    statusIcon = "O";
                    break;
                case DependencyStatus::NotFound:
                    statusColor = ImVec4(0.9f, 0.2f, 0.2f, 1.0f);
                    statusIcon = "X";
                    break;
                case DependencyStatus::Skipped:
                    statusColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
                    statusIcon = "-";
                    break;
                }
                ImGui::TextColored(statusColor, "%s", statusIcon);

                // Name
                ImGui::TableNextColumn();
                ImGui::Text("%s", dep.mName.c_str());

                // Details
                ImGui::TableNextColumn();
                if (dep.mStatus == DependencyStatus::Found)
                {
                    ImGui::TextWrapped("%s", dep.mVersion.c_str());
                }
                else if (dep.mStatus == DependencyStatus::NotFound)
                {
                    ImGui::TextWrapped("%s", dep.mInstallHint.c_str());
                }
                else
                {
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s", dep.mDescription.c_str());
                }

                // Action
                ImGui::TableNextColumn();
                if (dep.mStatus == DependencyStatus::NotFound && !dep.mInstallUrl.empty())
                {
                    if (ImGui::SmallButton("Install..."))
                    {
#if PLATFORM_WINDOWS
                        std::string cmd = "start " + dep.mInstallUrl;
#else
                        std::string cmd = "xdg-open " + dep.mInstallUrl + " &";
#endif
                        SYS_Exec(cmd.c_str());
                    }
                }

                ImGui::PopID();
            }

            ImGui::EndTable();
        }

        // Re-check button at the bottom
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Re-check"))
        {
            RunChecks();
        }

        ImGui::SameLine();

        GeneralModule* generalModule = static_cast<GeneralModule*>(
            PreferencesManager::Get()->FindModule("General"));
        if (generalModule != nullptr)
        {
            bool dontShow = !generalModule->GetCheckBuildDepsOnStartup();
            if (ImGui::Checkbox("Don't show on startup", &dontShow))
            {
                generalModule->SetCheckBuildDepsOnStartup(!dontShow);
                PreferencesManager::Get()->SaveModule(generalModule);
            }
        }

        int foundCount = 0;
        int checkedCount = 0;
        for (const BuildDependency& dep : mDependencies)
        {
            if (dep.mStatus != DependencyStatus::Skipped)
            {
                checkedCount++;
                if (dep.mStatus == DependencyStatus::Found)
                    foundCount++;
            }
        }
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%d of %d dependencies found",
            foundCount, checkedCount);
    }
    ImGui::End();
}

#endif
