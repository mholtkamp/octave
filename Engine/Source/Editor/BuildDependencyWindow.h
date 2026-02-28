#pragma once

#if EDITOR

#include <string>
#include <vector>

enum class DependencyStatus
{
    Found,
    NotFound,
    Skipped
};

struct BuildDependency
{
    std::string mName;
    std::string mDescription;
    DependencyStatus mStatus = DependencyStatus::NotFound;
    std::string mVersion;
    std::string mInstallHint;
    std::string mInstallUrl;
};

class BuildDependencyWindow
{
public:
    void RunChecks();
    bool HasMissing() const;
    void Open();
    void Draw();
    bool IsOpen() const;

private:
    void CheckMake();
    void CheckDevkitPro();
    void CheckDocker();
    void CheckVisualStudio();
    void CheckGradle();

    bool mIsOpen = false;
    std::vector<BuildDependency> mDependencies;
};

BuildDependencyWindow* GetBuildDependencyWindow();

#endif
