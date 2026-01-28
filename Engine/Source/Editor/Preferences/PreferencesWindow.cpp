#if EDITOR

#include "PreferencesWindow.h"
#include "PreferencesManager.h"
#include "PreferencesModule.h"

#include "imgui.h"

static PreferencesWindow sPreferencesWindow;

PreferencesWindow* GetPreferencesWindow()
{
    return &sPreferencesWindow;
}

PreferencesWindow::PreferencesWindow()
{
}

PreferencesWindow::~PreferencesWindow()
{
}

void PreferencesWindow::Open()
{
    mIsOpen = true;
    mPendingClose = false;

    // Select first module if none selected
    PreferencesManager* manager = PreferencesManager::Get();
    if (manager != nullptr && mSelectedModule == nullptr)
    {
        const std::vector<PreferencesModule*>& rootModules = manager->GetRootModules();
        if (!rootModules.empty())
        {
            mSelectedModule = rootModules[0];
        }
    }
}

void PreferencesWindow::Close()
{
    mIsOpen = false;
    mSelectedModule = nullptr;
}

void PreferencesWindow::Draw()
{
    if (!mIsOpen)
    {
        return;
    }

    PreferencesManager* manager = PreferencesManager::Get();
    if (manager == nullptr)
    {
        return;
    }

    // Center the modal window
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 windowSize(700.0f, 500.0f);
    ImVec2 windowPos((io.DisplaySize.x - windowSize.x) * 0.5f, (io.DisplaySize.y - windowSize.y) * 0.5f);
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

    if (ImGui::Begin("Preferences", &mIsOpen, windowFlags))
    {
        // Calculate layout dimensions
        float sidebarWidth = 200.0f;
        float footerHeight = 40.0f;
        float contentWidth = windowSize.x - sidebarWidth - 24.0f; // Account for padding
        float contentHeight = windowSize.y - footerHeight - 60.0f; // Account for title bar and footer

        // Left sidebar
        ImGui::BeginChild("Sidebar", ImVec2(sidebarWidth, contentHeight), true);
        DrawSidebar();
        ImGui::EndChild();

        ImGui::SameLine();

        // Right content panel
        ImGui::BeginChild("Content", ImVec2(contentWidth, contentHeight), true);
        DrawContent();
        ImGui::EndChild();

        // Footer with buttons
        DrawFooter();
    }
    ImGui::End();

    // Handle pending close after modal is done
    if (mPendingClose)
    {
        Close();
    }

    // Handle window close via X button
    if (!mIsOpen)
    {
        mSelectedModule = nullptr;
    }
}

void PreferencesWindow::DrawSidebar()
{
    PreferencesManager* manager = PreferencesManager::Get();
    if (manager == nullptr)
    {
        return;
    }

    const std::vector<PreferencesModule*>& rootModules = manager->GetRootModules();
    for (PreferencesModule* module : rootModules)
    {
        DrawModuleTree(module);
    }
}

void PreferencesWindow::DrawModuleTree(PreferencesModule* module)
{
    const std::vector<PreferencesModule*>& subModules = module->GetSubModules();
    bool hasChildren = !subModules.empty();
    bool isSelected = (mSelectedModule == module);

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

    if (isSelected)
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    if (!hasChildren)
    {
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }

    bool nodeOpen = ImGui::TreeNodeEx(module->GetName(), flags);

    // Handle selection on click
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
    {
        mSelectedModule = module;
    }

    if (hasChildren && nodeOpen)
    {
        for (PreferencesModule* sub : subModules)
        {
            DrawModuleTree(sub);
        }
        ImGui::TreePop();
    }
}

void PreferencesWindow::DrawContent()
{
    if (mSelectedModule != nullptr)
    {
        ImGui::Text("%s", mSelectedModule->GetName());
        ImGui::Separator();
        ImGui::Spacing();

        mSelectedModule->Render();
    }
    else
    {
        ImGui::TextDisabled("Select a category from the left panel.");
    }
}

void PreferencesWindow::DrawFooter()
{
    ImGui::Separator();
    ImGui::Spacing();

    // Right-align the buttons
    float buttonWidth = 80.0f;
    float spacing = 8.0f;
    float totalButtonWidth = buttonWidth * 3 + spacing * 2;

    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - totalButtonWidth - 16.0f);

    if (ImGui::Button("Apply", ImVec2(buttonWidth, 0)))
    {
        PreferencesManager::Get()->SaveAllSettings();
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0)))
    {
        // Reload settings to discard changes
        PreferencesManager::Get()->LoadAllSettings();
        mPendingClose = true;
    }

    ImGui::SameLine();

    if (ImGui::Button("OK", ImVec2(buttonWidth, 0)))
    {
        PreferencesManager::Get()->SaveAllSettings();
        mPendingClose = true;
    }
}

#endif
