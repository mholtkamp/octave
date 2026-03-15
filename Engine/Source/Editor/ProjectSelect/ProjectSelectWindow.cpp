#if EDITOR

#include "ProjectSelectWindow.h"
#include "TemplateManager.h"
#include "TemplateData.h"
#include "../Addons/AddonsMenu.h"
#include "../EditorIcons.h"
#include "EditorState.h"
#include "ActionManager.h"
#include "Engine.h"
#include "System/System.h"
#include "Log.h"

#include "imgui.h"

static ProjectSelectWindow sProjectSelectWindow;

ProjectSelectWindow* GetProjectSelectWindow()
{
    return &sProjectSelectWindow;
}

ProjectSelectWindow::ProjectSelectWindow()
{
    memset(mProjectNameBuffer, 0, sizeof(mProjectNameBuffer));
    memset(mProjectPathBuffer, 0, sizeof(mProjectPathBuffer));
    memset(mGitHubUrlBuffer, 0, sizeof(mGitHubUrlBuffer));
}

ProjectSelectWindow::~ProjectSelectWindow()
{
}

void ProjectSelectWindow::Open()
{
    mIsOpen = true;
    mSelectedTab = 0;
    mShowAddTemplatePopup = false;
    mAddTemplateError.clear();
    mSelectedTemplateIndex = -1;

    // Set default project path
    std::string defaultPath;
#if PLATFORM_WINDOWS
    const char* userProfile = getenv("USERPROFILE");
    if (userProfile != nullptr)
    {
        defaultPath = std::string(userProfile) + "/Documents/OctaveProjects";
    }
#else
    const char* home = getenv("HOME");
    if (home != nullptr)
    {
        defaultPath = std::string(home) + "/OctaveProjects";
    }
#endif
    strncpy(mProjectPathBuffer, defaultPath.c_str(), sizeof(mProjectPathBuffer) - 1);
}

void ProjectSelectWindow::Close()
{
    mIsOpen = false;
}

void ProjectSelectWindow::OpenIfNoProject()
{
    if (GetEngineState()->mProjectPath.empty())
    {
        Open();
    }
}

void ProjectSelectWindow::Draw()
{
    if (!mIsOpen)
    {
        return;
    }

    // Process any pending project removals
    for (const std::string& path : mProjectsToRemove)
    {
        std::vector<std::string>& recentProjects = GetEditorState()->mRecentProjects;
        for (auto it = recentProjects.begin(); it != recentProjects.end(); ++it)
        {
            if (*it == path)
            {
                recentProjects.erase(it);
                break;
            }
        }
    }
    mProjectsToRemove.clear();

    // Center the modal window
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 windowSize(700.0f, 500.0f);
    ImVec2 windowPos((io.DisplaySize.x - windowSize.x) * 0.5f, (io.DisplaySize.y - windowSize.y) * 0.5f);
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

    if (ImGui::Begin("Octave - Project Select", &mIsOpen, windowFlags))
    {
        // Tab bar
        if (ImGui::BeginTabBar("ProjectSelectTabs"))
        {
            if (ImGui::BeginTabItem("Recent Projects"))
            {
                mSelectedTab = 0;
                DrawRecentProjects();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Create New"))
            {
                mSelectedTab = 1;
                DrawCreateProject();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Templates"))
            {
                mSelectedTab = 2;
                DrawTemplates();
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }
    ImGui::End();

    // Draw popups
    DrawAddTemplatePopup();
}

void ProjectSelectWindow::DrawRecentProjects()
{
    const std::vector<std::string>& recentProjects = GetEditorState()->mRecentProjects;

    float contentHeight = 380.0f;
    ImGui::BeginChild("RecentProjectsList", ImVec2(0, contentHeight), true);

    if (recentProjects.empty())
    {
        ImGui::TextDisabled("No recent projects.");
    }
    else
    {
        static int selectedIndex = -1;

        for (int i = 0; i < (int)recentProjects.size(); ++i)
        {
            const std::string& path = recentProjects[i];
            std::string projectName = path;
            std::string projectDirectory = path;

            // Extract project name and directory from path
            size_t slashPos = path.find_last_of("/\\");
            if (slashPos != std::string::npos)
            {
                projectName = path.substr(slashPos + 1);
                projectDirectory = path.substr(0, slashPos);
            }
            // Remove .octp extension
            size_t dotPos = projectName.find_last_of('.');
            if (dotPos != std::string::npos)
            {
                projectName = projectName.substr(0, dotPos);
            }

            ImGui::PushID(i);

            bool isSelected = (selectedIndex == i);
            if (ImGui::Selectable(("##Project" + std::to_string(i)).c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_AllowItemOverlap, ImVec2(0, 40)))
            {
                selectedIndex = i;
                if (ImGui::IsMouseDoubleClicked(0))
                {
                    OnOpenProject(path);
                }
            }

            // Draw project info on the selectable
            ImGui::SameLine(10);
            ImGui::BeginGroup();
            ImGui::Text("%s", projectName.c_str());
            ImGui::TextDisabled("%s", path.c_str());
            ImGui::EndGroup();

            // Remove button on the right
			// take background color out to make the button more visible

            ImGui::SameLine(ImGui::GetWindowWidth() - 100);
            //ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            //ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
            if (ImGui::Button(ICON_MATERIAL_SYMBOLS_FOLDER_OPEN_SHARP, ImVec2(0,25) ))
            {
                SYS_ExplorerOpenDirectory(projectDirectory.c_str());
            }
            ImGui::SameLine(ImGui::GetWindowWidth() - 60);
            if (ImGui::Button(ICON_ZONDICONS_TRASH, ImVec2(0,25) ))
            {

                mProjectsToRemove.push_back(path);
                if (selectedIndex == i)
                {
                    selectedIndex = -1;
                }
            }
            //ImGui::PopStyleColor(2);

            ImGui::PopID();
        }
    }

    ImGui::EndChild();

    // Buttons at bottom
    ImGui::Spacing();

    if (ImGui::Button("Browse...", ImVec2(100, 0)))
    {
        OnBrowseProject();
    }

    ImGui::SameLine(ImGui::GetWindowWidth() - 120);

    bool hasSelection = false;
    int selectedIndex = -1;
    for (int i = 0; i < (int)recentProjects.size(); ++i)
    {
        // Check if this item is selected (simplified - in real code we'd track this properly)
    }

    if (ImGui::Button("Open Selected", ImVec2(100, 0)))
    {
        // Find selected and open
        // For simplicity, double-click handles opening
    }
}

void ProjectSelectWindow::DrawCreateProject()
{
    ImGui::Spacing();

    ImGui::Text("Project Name:");
    ImGui::SetNextItemWidth(400);
    ImGui::InputText("##ProjectName", mProjectNameBuffer, sizeof(mProjectNameBuffer));

    ImGui::Spacing();

    ImGui::Text("Location:");
    ImGui::SetNextItemWidth(330);
    ImGui::InputText("##ProjectPath", mProjectPathBuffer, sizeof(mProjectPathBuffer));
    ImGui::SameLine();
    if (ImGui::Button("Browse...##PathBrowse", ImVec2(60, 0)))
    {
        std::string selectedPath = SYS_SelectFolderDialog();
        if (!selectedPath.empty())
        {
            strncpy(mProjectPathBuffer, selectedPath.c_str(), sizeof(mProjectPathBuffer) - 1);
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Project Type:");
    ImGui::RadioButton("Lua", &mProjectType, 0);
    ImGui::SameLine();
    ImGui::RadioButton("C++", &mProjectType, 1);

    ImGui::Spacing();

    if (mProjectType == 1)
    {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f),
            "C++ projects create a standalone copy of the engine.");
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // Create button
    float buttonWidth = 150.0f;
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - buttonWidth) * 0.5f);

    bool canCreate = (strlen(mProjectNameBuffer) > 0 && strlen(mProjectPathBuffer) > 0);

    if (!canCreate)
    {
        ImGui::BeginDisabled();
    }

    if (ImGui::Button("Create Project", ImVec2(buttonWidth, 40)))
    {
        OnCreateNewProject();
    }

    if (!canCreate)
    {
        ImGui::EndDisabled();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Or create an addon:");
    ImGui::Spacing();
    DrawAddonsCreateItems_ProjectSelect();
}

void ProjectSelectWindow::DrawTemplates()
{
    TemplateManager* tm = TemplateManager::Get();
    if (tm == nullptr)
    {
        ImGui::TextDisabled("Template manager not initialized.");
        return;
    }

    // Header with Add Template button
    if (ImGui::Button("+ Add Template"))
    {
        mShowAddTemplatePopup = true;
        memset(mGitHubUrlBuffer, 0, sizeof(mGitHubUrlBuffer));
        mAddTemplateError.clear();
    }

    ImGui::Separator();
    ImGui::Spacing();

    const std::vector<Template>& templates = tm->GetTemplates();

    if (templates.empty())
    {
        ImGui::TextDisabled("No templates installed.");
        ImGui::TextDisabled("Click '+ Add Template' to install templates from GitHub.");
        return;
    }

    // Template grid
    float cardWidth = 150.0f;
    float cardHeight = 120.0f;
    float spacing = 10.0f;
    int cardsPerRow = (int)((ImGui::GetContentRegionAvail().x + spacing) / (cardWidth + spacing));
    if (cardsPerRow < 1) cardsPerRow = 1;

    ImGui::BeginChild("TemplateGrid", ImVec2(0, 280), true);

    for (int i = 0; i < (int)templates.size(); ++i)
    {
        const Template& tmpl = templates[i];

        if (i > 0 && i % cardsPerRow != 0)
        {
            ImGui::SameLine();
        }

        ImGui::PushID(i);

        bool isSelected = (mSelectedTemplateIndex == i);
        ImVec2 cardPos = ImGui::GetCursorScreenPos();

        // Card background
        ImGui::BeginGroup();
        if (ImGui::Selectable(("##Template" + std::to_string(i)).c_str(), isSelected,
                              ImGuiSelectableFlags_None, ImVec2(cardWidth, cardHeight)))
        {
            mSelectedTemplateIndex = i;
        }

        // Draw template info
        ImGui::SetCursorScreenPos(ImVec2(cardPos.x + 5, cardPos.y + 5));

        // Placeholder for thumbnail
        ImGui::BeginGroup();
        ImVec2 thumbSize(cardWidth - 10, 60);
        ImGui::Dummy(thumbSize);
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddRectFilled(
            ImVec2(cardPos.x + 5, cardPos.y + 5),
            ImVec2(cardPos.x + cardWidth - 5, cardPos.y + 65),
            IM_COL32(60, 60, 80, 255)
        );

        ImGui::SetCursorScreenPos(ImVec2(cardPos.x + 5, cardPos.y + 70));
        ImGui::TextWrapped("%s", tmpl.mMetadata.mName.c_str());
        ImGui::EndGroup();

        ImGui::EndGroup();
        ImGui::PopID();
    }

    ImGui::EndChild();

    // Selected template info
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (mSelectedTemplateIndex >= 0 && mSelectedTemplateIndex < (int)templates.size())
    {
        const Template& selected = templates[mSelectedTemplateIndex];

        ImGui::Text("Selected: %s", selected.mMetadata.mName.c_str());
        if (!selected.mMetadata.mAuthor.empty())
        {
            ImGui::Text("Author: %s", selected.mMetadata.mAuthor.c_str());
        }
        if (!selected.mMetadata.mDescription.empty())
        {
            ImGui::TextWrapped("%s", selected.mMetadata.mDescription.c_str());
        }

        ImGui::Spacing();

        if (ImGui::Button("Remove Template", ImVec2(120, 0)))
        {
            tm->RemoveTemplate(selected.mMetadata.mId);
            mSelectedTemplateIndex = -1;
        }

        ImGui::SameLine();

        if (ImGui::Button("Create from Template", ImVec2(150, 0)))
        {
            OnCreateFromTemplate(selected.mMetadata.mId);
        }
    }
    else
    {
        ImGui::TextDisabled("Select a template to see details.");
    }
}

void ProjectSelectWindow::DrawAddTemplatePopup()
{
    if (!mShowAddTemplatePopup)
    {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(400, 200));

    if (ImGui::Begin("Add Template", &mShowAddTemplatePopup,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
    {
        ImGui::Text("GitHub URL:");
        ImGui::SetNextItemWidth(370);
        ImGui::InputText("##GitHubUrl", mGitHubUrlBuffer, sizeof(mGitHubUrlBuffer));

        ImGui::Spacing();
        ImGui::TextWrapped("Enter a GitHub repository URL containing a project template.");
        ImGui::TextWrapped("The repository should have Assets/, Scripts/, and optionally a package.json.");

        if (!mAddTemplateError.empty())
        {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s", mAddTemplateError.c_str());
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Add", ImVec2(80, 0)))
        {
            OnAddTemplateFromGitHub();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(80, 0)))
        {
            mShowAddTemplatePopup = false;
        }
    }
    ImGui::End();
}

void ProjectSelectWindow::OnOpenProject(const std::string& path)
{
    ActionManager::Get()->OpenProject(path.c_str());
    Close();
}

void ProjectSelectWindow::OnBrowseProject()
{
    std::vector<std::string> paths = SYS_OpenFileDialog();

    if (paths.size() > 0 && !paths[0].empty())
    {
        ActionManager::Get()->OpenProject(paths[0].c_str());
    }

    // Close if a project was opened
    if (!GetEngineState()->mProjectPath.empty())
    {
        Close();
    }
}

void ProjectSelectWindow::OnCreateNewProject()
{
    std::string projectPath = std::string(mProjectPathBuffer) + "/" + mProjectNameBuffer;
    bool isCpp = (mProjectType == 1);

    ActionManager::Get()->CreateNewProject(projectPath.c_str(), isCpp);

    // Close if project was created successfully
    if (!GetEngineState()->mProjectPath.empty())
    {
        Close();
    }
}

void ProjectSelectWindow::OnCreateFromTemplate(const std::string& templateId)
{
    TemplateManager* tm = TemplateManager::Get();
    if (tm == nullptr)
    {
        return;
    }

    // Use the Create New tab settings for project name and path
    if (strlen(mProjectNameBuffer) == 0)
    {
        // Default to template name
        const Template* tmpl = tm->FindTemplate(templateId);
        if (tmpl != nullptr)
        {
            strncpy(mProjectNameBuffer, tmpl->mMetadata.mName.c_str(), sizeof(mProjectNameBuffer) - 1);
        }
    }

    std::string projectPath = std::string(mProjectPathBuffer) + "/" + mProjectNameBuffer;
    std::string error;

    if (tm->CreateProjectFromTemplate(templateId, projectPath, mProjectNameBuffer, error))
    {
        // Find the .octp file and open it
        std::string octpPath = projectPath + "/" + mProjectNameBuffer + ".octp";
        if (SYS_DoesFileExist(octpPath.c_str(), false))
        {
            ActionManager::Get()->OpenProject(octpPath.c_str());
            Close();
        }
        else
        {
            LogError("Created project but could not find .octp file: %s", octpPath.c_str());
        }
    }
    else
    {
        LogError("Failed to create project from template: %s", error.c_str());
    }
}

void ProjectSelectWindow::OnRemoveRecentProject(const std::string& path)
{
    mProjectsToRemove.push_back(path);
}

void ProjectSelectWindow::OnAddTemplateFromGitHub()
{
    std::string url = mGitHubUrlBuffer;
    if (url.empty())
    {
        mAddTemplateError = "Please enter a GitHub URL.";
        return;
    }

    TemplateManager* tm = TemplateManager::Get();
    if (tm == nullptr)
    {
        mAddTemplateError = "Template manager not initialized.";
        return;
    }

    std::string error;
    if (tm->AddTemplateFromGitHub(url, error))
    {
        mShowAddTemplatePopup = false;
        mAddTemplateError.clear();
    }
    else
    {
        mAddTemplateError = error;
    }
}

#endif
