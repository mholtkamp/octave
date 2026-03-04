#pragma once

#if EDITOR

#include <string>
#include <vector>

/**
 * @brief Window for selecting, creating, or opening projects.
 *
 * Displays on editor startup when no project is loaded. Provides:
 * - Recent projects list
 * - Create new project (Lua or C++)
 * - Create from template
 * - Template management
 */
class ProjectSelectWindow
{
public:
    ProjectSelectWindow();
    ~ProjectSelectWindow();

    void Open();
    void Close();
    void Draw();
    bool IsOpen() const { return mIsOpen; }

    /** @brief Open the window if no project is currently loaded */
    void OpenIfNoProject();

private:
    void DrawRecentProjects();
    void DrawCreateProject();
    void DrawTemplates();
    void DrawAddTemplatePopup();

    void OnOpenProject(const std::string& path);
    void OnBrowseProject();
    void OnCreateNewProject();
    void OnCreateFromTemplate(const std::string& templateId);
    void OnRemoveRecentProject(const std::string& path);
    void OnAddTemplateFromGitHub();

    bool mIsOpen = false;
    int mSelectedTab = 0;

    // Add Template popup state
    bool mShowAddTemplatePopup = false;
    char mGitHubUrlBuffer[512] = {};
    std::string mAddTemplateError;

    // Create New Project state
    char mProjectNameBuffer[256] = {};
    char mProjectPathBuffer[512] = {};
    int mProjectType = 0;  // 0 = Lua, 1 = C++
    int mSelectedTemplateIndex = -1;

    // Recent projects removal tracking
    std::vector<std::string> mProjectsToRemove;
};

ProjectSelectWindow* GetProjectSelectWindow();

#endif
