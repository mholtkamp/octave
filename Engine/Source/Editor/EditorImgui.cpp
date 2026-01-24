#if EDITOR

#include "EditorImgui.h"
#include "System/System.h"
#include "Engine.h"
#include "EditorUtils.h"
#include "EditorConstants.h"
#include "Renderer.h"
#include "InputDevices.h"
#include "Log.h"
#include "AssetDir.h"
#include "Grid.h"
#include "Script.h"
#include "PaintManager.h"
#include "NodePath.h"

#include "Nodes/3D/StaticMesh3d.h"
#include "Nodes/3D/InstancedMesh3d.h"
#include "Nodes/3D/SkeletalMesh3d.h"
#include "Nodes/3D/Camera3d.h"
#include "World.h"

#include <glm/gtc/type_ptr.hpp>

#include "Assets/Scene.h"
#include "Assets/SoundWave.h"
#include "Assets/ParticleSystem.h"
#include "Assets/StaticMesh.h"
#include "Assets/SkeletalMesh.h"
#include "Assets/MaterialBase.h"
#include "Assets/MaterialInstance.h"
#include "Assets/MaterialLite.h"

#include "Viewport3d.h"
#include "Viewport2d.h"
#include "ActionManager.h"
#include "EditorState.h"

#include <functional>
#include <algorithm>

// TODO: If we ever support an OpenGL backend, gotta change this.
#include "backends/imgui_impl_vulkan.cpp"
#include "Graphics/Vulkan/VulkanContext.h"
#include "Graphics/Vulkan/VulkanUtils.h"

#include "CustomImgui.h"
#include "ImGuizmo/ImGuizmo.h"

#if PLATFORM_WINDOWS
#include "backends/imgui_impl_win32.cpp"
#elif PLATFORM_LINUX
#include "imgui_impl_xcb.h"
#endif

struct FileBrowserDirEntry
{
    std::string mName;
    std::string mDirPath;
    bool mFolder = false;
    bool mSelected = false;
};

static const float kSidePaneWidth = 200.0f;
static const float kViewportBarHeight = 40.0f;
static const ImGuiWindowFlags kPaneWindowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove;

static const ImVec4 kSelectedColor = ImVec4(0.12f, 0.50f, 0.47f, 1.00f);
static const ImVec4 kBgInactive = ImVec4(0.20f, 0.20f, 0.68f, 1.00f);
static const ImVec4 kBgHover = ImVec4(0.26f, 0.61f, 0.98f, 0.80f);
static const ImVec4 kToggledColor = ImVec4(0.7f, 0.2f, 0.2f, 1.0f);

constexpr const uint32_t kPopupInputBufferSize = 256;
static char sPopupInputBuffer[kPopupInputBufferSize] = {};

static bool sNodesDiscovered = false;
static std::vector<std::string> sNode3dNames;
static std::vector<std::string> sNodeWidgetNames;
static std::vector<std::string> sNodeOtherNames;

static ImTextureID sInspectTexId = 0;
static Texture* sPrevInspectTexture = nullptr;

static bool sFileBrowserOpen = false;
static bool sFileBrowserFolderMode = false;
static FileBrowserCallbackFP sFileBrowserCallback = nullptr;
static std::vector<std::string> sFileBrowserPaths;
static std::string sFileBrowserCurDir;
static std::vector<FileBrowserDirEntry> sFileBrowserEntries;
static float sFileBrowserDoubleClickBlock = 0.0f;
static bool sFileBrowserNeedsRefresh = false;

static bool sObjectTabOpen = false;

static SceneImportOptions sSceneImportOptions;

static std::vector<AssetStub*> sUnsavedAssets;
static std::vector<bool> sUnsavedAssetsSelected;
static bool sUnsavedModalActive = false;

static std::vector<std::string> sSceneList;
static int32_t sDevModeClicks = 0;

static void PopulateFileBrowserDirs()
{
    sFileBrowserDoubleClickBlock = 0.2f;

    sFileBrowserEntries.clear();

    DirEntry dirEntry;
    SYS_OpenDirectory(sFileBrowserCurDir, dirEntry);

    if (!dirEntry.mValid)
    {
        // Invalid directory. Renamed or no longer exists?
        // Remove favorite in case it was favorited. Will do nothing if not favorited.
        GetEditorState()->RemoveFavoriteDir(sFileBrowserCurDir);

        // Default to project directory.
        sFileBrowserCurDir = SYS_GetAbsolutePath(GetEngineState()->mProjectDirectory);
        SYS_OpenDirectory(sFileBrowserCurDir, dirEntry);
    }

    while (dirEntry.mValid)
    {
        FileBrowserDirEntry entry;
        entry.mName = dirEntry.mFilename;
        entry.mDirPath = sFileBrowserCurDir;
        entry.mFolder = dirEntry.mDirectory;
        sFileBrowserEntries.push_back(entry);

        SYS_IterateDirectory(dirEntry);
    }

    SYS_CloseDirectory(dirEntry);

    // Sort entries alphabetically. Perhaps add different sorting methods in the future.
    auto alphaComp = [&](const FileBrowserDirEntry& l, const FileBrowserDirEntry& r)
    {
        if (l.mName == "..")
            return true;
        else if (r.mName == "..")
            return false;
        return l.mName < r.mName;
    };

    std::sort(sFileBrowserEntries.begin(), sFileBrowserEntries.end(), alphaComp);
}

void EditorOpenFileBrowser(FileBrowserCallbackFP callback, bool folderMode)
{
    if (!sFileBrowserOpen)
    {
        sFileBrowserOpen = true;
        sFileBrowserFolderMode = folderMode;
        sFileBrowserCallback = callback;
        sFileBrowserPaths.clear();

        if (sFileBrowserCurDir == "")
        {
            if (GetEngineState()->mProjectDirectory != "")
            {
                sFileBrowserCurDir = GetEngineState()->mProjectDirectory;
            }
            else
            {
                sFileBrowserCurDir = "./";
            }
        }

        sFileBrowserCurDir = SYS_GetAbsolutePath(sFileBrowserCurDir);

        PopulateFileBrowserDirs();

        if (sFileBrowserEntries.size() == 0)
        {
            LogWarning("No directory entries found. Reseting to working dir.");
            sFileBrowserCurDir = "./";
            PopulateFileBrowserDirs();

            if (sFileBrowserEntries.size() == 0)
            {
                LogError("Still couldn't find directory entries...");
            }
        }
    }
    else
    {
        LogWarning("Failed to open file browser. It is already open.");
    }
}

void EditorSetFileBrowserDir(const std::string& dir)
{
    sFileBrowserCurDir = dir;
}

void EditorShowUnsavedAssetsModal(const std::vector<AssetStub*>& unsavedStubs)
{
    if (!sUnsavedModalActive)
    {
        sUnsavedModalActive = true;
        sUnsavedAssets = unsavedStubs;
        sUnsavedAssetsSelected.clear();
        sUnsavedAssetsSelected.resize(sUnsavedAssets.size());
    }
}

static void DrawFileBrowserContextPopup(FileBrowserDirEntry* entry)
{
    // New Folder
    // Rename 
    // Delete

    bool closeContextPopup = false;
    bool setTextInputFocus = false;
    bool isParentDir = (entry && entry->mName == "..");

    if (entry && !isParentDir && ImGui::Selectable("Rename", false, ImGuiSelectableFlags_DontClosePopups))
    {
        ImGui::OpenPopup("Rename File");
        strncpy(sPopupInputBuffer, entry->mName.c_str(), kPopupInputBufferSize);
        setTextInputFocus = true;
    }
    if (ImGui::Selectable("New Folder", false, ImGuiSelectableFlags_DontClosePopups))
    {
        ImGui::OpenPopup("New Folder");
        strncpy(sPopupInputBuffer, "", kPopupInputBufferSize);
        setTextInputFocus = true;
    }
    if (entry && !isParentDir && ImGui::Selectable("Delete"))
    {
        if (entry->mFolder)
        {
            std::string dirPath = sFileBrowserCurDir + entry->mName;
            SYS_RemoveDirectory(dirPath.c_str());
            LogDebug("Deleted directory: %s", dirPath.c_str());
        }
        else
        {
            std::string filePath = sFileBrowserCurDir + entry->mName;
            SYS_RemoveFile(filePath.c_str());
            LogDebug("Deleted file: %s", filePath.c_str());
        }

        sFileBrowserNeedsRefresh = true;
    }

    if (ImGui::BeginPopup("Rename File"))
    {
        if (setTextInputFocus)
            ImGui::SetKeyboardFocusHere();

        if (ImGui::InputText("Name", sPopupInputBuffer, kPopupInputBufferSize, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            // Rename!
            std::string newName = sPopupInputBuffer;
            std::string oldFilePath = entry->mDirPath + entry->mName;
            std::string newFilePath = entry->mDirPath + newName;
            SYS_Rename(oldFilePath.c_str(), newFilePath.c_str());
            sFileBrowserNeedsRefresh = true;

            ImGui::CloseCurrentPopup();
            closeContextPopup = true;
        }

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("New Folder"))
    {
        if (setTextInputFocus)
            ImGui::SetKeyboardFocusHere();

        if (ImGui::InputText("Folder Name", sPopupInputBuffer, kPopupInputBufferSize, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            const std::string folderName = sPopupInputBuffer;

            if (folderName != "")
            {
                if (!SYS_CreateDirectory((sFileBrowserCurDir + folderName).c_str()))
                {
                    LogError("Failed to create folder");
                }
            }

            sFileBrowserNeedsRefresh = true;

            ImGui::CloseCurrentPopup();
            closeContextPopup = true;
        }

        ImGui::EndPopup();
    }

    if (closeContextPopup)
    {
        ImGui::CloseCurrentPopup();
    }
}

static void DrawFileBrowser()
{
    bool contextPopupOpen = false;

    if (sFileBrowserNeedsRefresh)
    {
        PopulateFileBrowserDirs();
        sFileBrowserNeedsRefresh = false;
    }

    sFileBrowserDoubleClickBlock -= GetEngineState()->mRealDeltaTime;
    sFileBrowserDoubleClickBlock = glm::max(sFileBrowserDoubleClickBlock, 0.0f);

    if (sFileBrowserOpen)
    {
        ImGui::OpenPopup("File Browser");
    }

    // Center the modal
    if (ImGui::IsPopupOpen("File Browser"))
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(465, 490));
    }

    if (ImGui::BeginPopupModal("File Browser", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
    {
        bool confirmOpen = false;

        // Text that shows current directory.
        ImGui::Text(sFileBrowserCurDir.c_str());

        bool dirFaved = GetEditorState()->IsDirFavorited(sFileBrowserCurDir);
        if (dirFaved)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, kToggledColor);
        }

        if (ImGui::Button("F", ImVec2(20, 20)))
        {
            if (dirFaved)
                GetEditorState()->RemoveFavoriteDir(sFileBrowserCurDir);
            else
                GetEditorState()->AddFavoriteDir(sFileBrowserCurDir);
        }

        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone))
            ImGui::SetTooltip("Favorite");

        if (dirFaved)
        {
            ImGui::PopStyleColor();
        }

        ImGui::SameLine();
        std::string curDirShortName = GetDirShortName(sFileBrowserCurDir);

        if (ImGui::BeginCombo("Favorites", dirFaved ? curDirShortName.c_str() : ""))
        {
            const std::vector<std::string>& favedDirs = GetEditorState()->mFavoritedDirs;
            for (uint32_t i = 0; i < favedDirs.size(); ++i)
            {
                std::string dispName = GetDirShortName(favedDirs[i]);
                dispName += " --- " + favedDirs[i];

                if (ImGui::Selectable(dispName.c_str()))
                {
                    sFileBrowserCurDir = favedDirs[i];
                    sFileBrowserNeedsRefresh = true;
                }
            }

            ImGui::EndCombo();
        }

        // List of selectables that let you navigate/open/delete.
        {
            ImGuiWindowFlags childFlags = ImGuiWindowFlags_None;
            ImGui::BeginChild("File List", ImVec2(450, 350), true, childFlags);

            bool changedDir = false;

            // Show folders
            for (uint32_t i = 0; i < sFileBrowserEntries.size(); ++i)
            {
                if (sFileBrowserEntries[i].mFolder &&
                    sFileBrowserEntries[i].mName != ".")
                {
                    if (ImGui::Selectable(sFileBrowserEntries[i].mName.c_str(), true))
                    {
                        // If a folder is selected, then we need to switch to that directory,
                        // and also set the sFileBrowserPath if in folder mode.
                        sFileBrowserCurDir = SYS_GetAbsolutePath(sFileBrowserCurDir + sFileBrowserEntries[i].mName + "/");
                        changedDir = true;

                        if (sFileBrowserFolderMode)
                        {
                            sFileBrowserPaths.clear();
                            sFileBrowserPaths.push_back(sFileBrowserCurDir);
                        }
                    }

                    if (ImGui::BeginPopupContextItem())
                    {
                        contextPopupOpen = true;
                        DrawFileBrowserContextPopup(&sFileBrowserEntries[i]);
                        ImGui::EndPopup();
                    }
                }
            }

            // Show files
            bool doubleClicked = ImGui::IsMouseDoubleClicked(0) && sFileBrowserDoubleClickBlock <= 0.0f;

            for (uint32_t i = 0; i < sFileBrowserEntries.size(); ++i)
            {
                if (!sFileBrowserEntries[i].mFolder)
                {
                    bool selected = sFileBrowserEntries[i].mSelected;

                    if (selected)
                    {
                        ImGui::PushStyleColor(ImGuiCol_Header, kToggledColor);
                        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, kToggledColor);
                        ImGui::PushStyleColor(ImGuiCol_HeaderActive, kToggledColor);
                    }

                    if (ImGui::Selectable(sFileBrowserEntries[i].mName.c_str(), selected, ImGuiSelectableFlags_AllowDoubleClick))
                    {
                        if (!sFileBrowserFolderMode)
                        {
                            if (IsControlDown())
                            {
                                if (selected)
                                {
                                    sFileBrowserEntries[i].mSelected = false;
                                    sFileBrowserPaths.erase(std::find(sFileBrowserPaths.begin(), sFileBrowserPaths.end(), sFileBrowserCurDir + sFileBrowserEntries[i].mName));
                                }
                                else
                                {
                                    sFileBrowserEntries[i].mSelected = true;
                                    sFileBrowserPaths.push_back(sFileBrowserCurDir + sFileBrowserEntries[i].mName);
                                }
                            }
                            else
                            {
                                if (doubleClicked)
                                {
                                    confirmOpen = true;
                                }

                                sFileBrowserPaths.clear();
                                sFileBrowserPaths.push_back(sFileBrowserCurDir + sFileBrowserEntries[i].mName);

                                for (uint32_t x = 0; x < sFileBrowserEntries.size(); ++x)
                                {
                                    sFileBrowserEntries[x].mSelected = false;
                                }

                                sFileBrowserEntries[i].mSelected = true;
                            }
                        }
                    }

                    if (selected)
                    {
                        ImGui::PopStyleColor(3);
                    }

                    if (ImGui::BeginPopupContextItem())
                    {
                        contextPopupOpen = true;
                        DrawFileBrowserContextPopup(&sFileBrowserEntries[i]);
                        ImGui::EndPopup();
                    }
                }
            }

            ImGui::EndChild();

            // If no popup is open and we aren't inputting text...
            if (!contextPopupOpen &&
                ImGui::IsItemHovered() &&
                !ImGui::GetIO().WantTextInput)
            {
                if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
                {
                    ImGui::OpenPopup("Null Context");
                }
            }

            if (ImGui::BeginPopup("Null Context"))
            {
                contextPopupOpen = true;
                DrawFileBrowserContextPopup(nullptr);
                ImGui::EndPopup();
            }

            if (changedDir)
            {
                PopulateFileBrowserDirs();
            }
        }

        std::string fileFolderName = "";
        if (sFileBrowserPaths.size() > 0)
        {
            fileFolderName = sFileBrowserPaths.back();
        }

        if (sFileBrowserFolderMode && 
            fileFolderName.size() > 0 &&
            (fileFolderName.back() == '/' || fileFolderName.back() == '\\'))
        {
            fileFolderName.pop_back();
        }

        size_t lastSlashIdx = fileFolderName.find_last_of("/\\");
        if (lastSlashIdx != std::string::npos)
        {
            fileFolderName = fileFolderName.substr(lastSlashIdx + 1);
        }

        ImGui::SetNextItemWidth(400);
        if (ImGui::InputText(sFileBrowserFolderMode ? "Folder" : "File", &fileFolderName))
        {
            sFileBrowserPaths.clear();
            sFileBrowserPaths.push_back(sFileBrowserCurDir + fileFolderName);

            // Add the trailing slash if its a folder.
            if (sFileBrowserFolderMode &&
                sFileBrowserPaths[0].size() > 0 &&
                (sFileBrowserPaths[0].back() != '/' && sFileBrowserPaths[0].back() != '\\'))
            {
#if PLATFORM_WINDOWS
                sFileBrowserPaths[0].push_back('\\');
#else
                sFileBrowserPaths[0].push_back('/');
#endif
            }
        }

        {
            std::string curPath = "";
            if (sFileBrowserPaths.size() > 0)
            {
                curPath = sFileBrowserPaths[0];
            }
            ImGui::Text(curPath.c_str());
        }

        if (ImGui::Button("Open"))
        {
            confirmOpen = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            sFileBrowserOpen = false;
            ImGui::CloseCurrentPopup();
        }

        if (!contextPopupOpen && IsKeyJustDown(KEY_ENTER))
        {
            confirmOpen = true;
        }

        if (confirmOpen)
        {
            if (sFileBrowserCallback != nullptr)
            {
                sFileBrowserCallback(sFileBrowserPaths);
            }

            sFileBrowserOpen = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

static void DrawUnsavedCheck()
{
    if (sUnsavedModalActive)
    {
        ImGui::OpenPopup("Unsaved Changes");
    }

    // Center the modal
    if (ImGui::IsPopupOpen("Unsaved Changes"))
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(400,260));
    }

    if (ImGui::BeginPopupModal("Unsaved Changes", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
    {
        // Scrolling list
        if (ImGui::BeginListBox("##UnsavedAssetListBox", ImVec2(390, 200)))
        {
            for (uint32_t i = 0; i < sUnsavedAssets.size(); ++i)
            {
                if (sUnsavedAssets[i] == nullptr ||
                    sUnsavedAssets[i]->mAsset == nullptr)
                {
                    continue;
                }

                glm::vec4 assetColor = AssetManager::Get()->GetEditorAssetColor(sUnsavedAssets[i]->mType);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(assetColor.r, assetColor.g, assetColor.b, assetColor.a));

                if (ImGui::Selectable(sUnsavedAssets[i]->mAsset->GetName().c_str(), sUnsavedAssetsSelected[i]))
                {
                    sUnsavedAssetsSelected[i] = !sUnsavedAssetsSelected[i];
                }

                ImGui::PopStyleColor();
            }
            ImGui::EndListBox();
        }

        bool closePopup = false;

        if (ImGui::Button("Save All"))
        {
            for (uint32_t i = 0; i < sUnsavedAssets.size(); ++i)
            {
                if (sUnsavedAssets[i] != nullptr)
                {
                    AssetManager::Get()->SaveAsset(*sUnsavedAssets[i]);
                }
            }

            GetEditorState()->mShutdownUnsavedCheck = true;
            GetEngineState()->mQuit = true;
            closePopup = true;
        }

        ImGui::SameLine();
        if (ImGui::Button("Save Selected"))
        {
            for (uint32_t i = 0; i < sUnsavedAssets.size(); ++i)
            {
                if (sUnsavedAssets[i] != nullptr &&
                    sUnsavedAssetsSelected[i])
                {
                    AssetManager::Get()->SaveAsset(*sUnsavedAssets[i]);
                }
            }
            GetEditorState()->mShutdownUnsavedCheck = true;
            GetEngineState()->mQuit = true;
            closePopup = true;
        }

        ImGui::SameLine();
        if (ImGui::Button("Don't Save"))
        {
            GetEditorState()->mShutdownUnsavedCheck = true;
            GetEngineState()->mQuit = true;
            closePopup = true;
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            GetEditorState()->mShutdownUnsavedCheck = false;
            GetEngineState()->mQuit = false;
            closePopup = true;
        }

        if (IsKeyJustDown(KEY_ESCAPE))
        {
            GetEditorState()->mShutdownUnsavedCheck = false;
            GetEngineState()->mQuit = false;
            closePopup = true;
        }

        if (closePopup)
        {
            sUnsavedModalActive = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

static void DiscoverNodeClasses()
{
    sNode3dNames.clear();
    sNodeWidgetNames.clear();

    const std::vector<Factory*>& nodeFactories = Node::GetFactoryList();
    for (uint32_t i = 0; i < nodeFactories.size(); ++i)
    {
        Node* node = Node::CreateInstance(nodeFactories[i]->GetType());
        if (node->As<Node3D>())
        {
            if (strcmp(node->GetClassName(), "Node3D") == 0)
            {
                sNode3dNames.insert(sNode3dNames.begin(), node->GetClassName());
            }
            else
            {
                sNode3dNames.push_back(nodeFactories[i]->GetClassName());
            }
        }
        else if (node->As<Widget>())
        {
            if (strcmp(node->GetClassName(), "Widget") == 0)
            {
                sNodeWidgetNames.insert(sNodeWidgetNames.begin(), node->GetClassName());
            }
            else
            {
                sNodeWidgetNames.push_back(nodeFactories[i]->GetClassName());
            }
        }
        else if (strcmp(node->GetClassName(), "Node") != 0)
        {
            sNodeOtherNames.push_back(nodeFactories[i]->GetClassName());
        }

        delete node;
    }
}

static void CreateNewAsset(TypeId assetType, const char* assetName)
{
    AssetStub* stub = nullptr;
    AssetDir* currentDir = GetEditorState()->GetAssetDirectory();

    if (currentDir == nullptr)
        return;

    stub = EditorAddUniqueAsset(assetName, currentDir, assetType, true);

    if (assetType == MaterialLite::GetStaticType())
    {
        Asset* selAsset = GetEditorState()->GetSelectedAsset();

        if (stub != nullptr &&
            stub->mAsset != nullptr &&
            selAsset != nullptr &&
            selAsset->GetType() == Texture::GetStaticType())
        {
            MaterialLite* matLite = stub->mAsset->As<MaterialLite>();
            Texture* texture = selAsset->As<Texture>();

            // Auto assign the selected texture to index 0
            matLite->SetTexture(0, texture);

            std::string newMatName = texture->GetName();

            if (newMatName.length() >= 2 && newMatName[0] == 'T' && newMatName[1] == '_')
            {
                newMatName[0] = 'M';
            }
            else
            {
                newMatName = std::string("M_") + newMatName;
            }

            AssetManager::Get()->RenameAsset(matLite, newMatName);
        }
    }

    if (stub != nullptr)
    {
        AssetManager::Get()->SaveAsset(*stub);
    }
}

 static void SetupImGuiStyle()
{
	// Future Dark style by rewrking from ImThemes
	ImGuiStyle& style = ImGui::GetStyle();
	
	style.Alpha = 1.0f;
	style.DisabledAlpha = 1.0f;
	style.WindowPadding = ImVec2(12.0f, 12.0f);
	style.WindowRounding = 0.0f;
	style.WindowBorderSize = 0.0f;
	style.WindowMinSize = ImVec2(20.0f, 20.0f);
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
	style.WindowMenuButtonPosition = ImGuiDir_None;
	style.ChildRounding = 0.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupRounding = 0.0f;
	style.PopupBorderSize = 1.0f;
	style.FramePadding = ImVec2(6.0f, 6.0f);
	style.FrameRounding = 0.0f;
	style.FrameBorderSize = 0.0f;
	style.ItemSpacing = ImVec2(12.0f, 6.0f);
	style.ItemInnerSpacing = ImVec2(6.0f, 3.0f);
	style.CellPadding = ImVec2(12.0f, 6.0f);
	style.IndentSpacing = 20.0f;
	style.ColumnsMinSpacing = 6.0f;
	style.ScrollbarSize = 12.0f;
	style.ScrollbarRounding = 0.0f;
	style.GrabMinSize = 12.0f;
	style.GrabRounding = 0.0f;
	style.TabRounding = 0.0f;
	style.TabBorderSize = 0.0f;
	style.TabMinWidthForCloseButton = 0.0f;
	style.ColorButtonPosition = ImGuiDir_Right;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.SelectableTextAlign = ImVec2(0.0f, 0.0f);
	
	style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.27450982f, 0.31764707f, 0.4509804f, 1.0f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.078431375f, 0.08627451f, 0.101960786f, 1.0f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.078431375f, 0.08627451f, 0.101960786f, 1.0f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.078431375f, 0.08627451f, 0.101960786f, 1.0f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.15686275f, 0.16862746f, 0.19215687f, 1.0f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.078431375f, 0.08627451f, 0.101960786f, 1.0f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.11764706f, 0.13333334f, 0.14901961f, 1.0f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.15686275f, 0.16862746f, 0.19215687f, 1.0f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.23529412f, 0.21568628f, 0.59607846f, 1.0f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.047058824f, 0.05490196f, 0.07058824f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.047058824f, 0.05490196f, 0.07058824f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.078431375f, 0.08627451f, 0.101960786f, 1.0f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.09803922f, 0.105882354f, 0.12156863f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.047058824f, 0.05490196f, 0.07058824f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.11764706f, 0.13333334f, 0.14901961f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.15686275f, 0.16862746f, 0.19215687f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.11764706f, 0.13333334f, 0.14901961f, 1.0f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.49803922f, 0.5137255f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.49803922f, 0.5137255f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.5372549f, 0.5529412f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.11764706f, 0.13333334f, 0.14901961f, 1.0f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.19607843f, 0.1764706f, 0.54509807f, 1.0f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.23529412f, 0.21568628f, 0.59607846f, 1.0f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.11764706f, 0.13333334f, 0.14901961f, 1.0f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.19607843f, 0.1764706f, 0.54509807f, 1.0f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.23529412f, 0.21568628f, 0.59607846f, 1.0f);
	style.Colors[ImGuiCol_Separator] = ImVec4(0.15686275f, 0.18431373f, 0.2509804f, 1.0f);
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.15686275f, 0.18431373f, 0.2509804f, 1.0f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.15686275f, 0.18431373f, 0.2509804f, 1.0f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.11764706f, 0.13333334f, 0.14901961f, 1.0f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.19607843f, 0.1764706f, 0.54509807f, 1.0f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.23529412f, 0.21568628f, 0.59607846f, 1.0f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.047058824f, 0.05490196f, 0.07058824f, 1.0f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.11764706f, 0.13333334f, 0.14901961f, 1.0f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.09803922f, 0.105882354f, 0.12156863f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.047058824f, 0.05490196f, 0.07058824f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.078431375f, 0.08627451f, 0.101960786f, 1.0f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.52156866f, 0.6f, 0.7019608f, 1.0f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.039215688f, 0.98039216f, 0.98039216f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(1.0f, 0.2901961f, 0.59607846f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.99607843f, 0.4745098f, 0.69803923f, 1.0f);
	style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.047058824f, 0.05490196f, 0.07058824f, 1.0f);
	style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.047058824f, 0.05490196f, 0.07058824f, 1.0f);
	style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.11764706f, 0.13333334f, 0.14901961f, 1.0f);
	style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.09803922f, 0.105882354f, 0.12156863f, 1.0f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.23529412f, 0.21568628f, 0.59607846f, 1.0f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.49803922f, 0.5137255f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.49803922f, 0.5137255f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.49803922f, 0.5137255f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.19607843f, 0.1764706f, 0.54509807f, 0.5019608f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.19607843f, 0.1764706f, 0.54509807f, 0.5019608f);
}

static void AssignAssetToProperty(Object* owner, PropertyOwnerType ownerType, Property& prop, uint32_t index, Asset* newAsset)
{
    if (newAsset != nullptr &&
        newAsset != prop.GetAsset())
    {
        TypeId newType = newAsset->GetType();
        TypeId propId = prop.mExtra ? (TypeId)prop.mExtra->GetInteger() : 0;

        bool matchingType = (prop.mExtra == nullptr || newType == TypeId(prop.mExtra->GetInteger()));

        // HACK: Handle derived class types.
        if (propId == Material::GetStaticType() &&
            (newType == MaterialBase::GetStaticType() ||
             newType == MaterialInstance::GetStaticType() ||
             newType == MaterialLite::GetStaticType()))
        {
            matchingType = true;
        }

        if (matchingType)
        {
            if (ownerType != PropertyOwnerType::Count)
            {
                ActionManager::Get()->EXE_EditProperty(owner, ownerType, prop.mName, index, newAsset);
            }
            else
            {
                // Skip undo/redo (for shader parameters?)
                prop.SetAsset(newAsset, index);
            }
        }
    }
}

static void DrawNodeProperty(Property& prop, uint32_t index, Object* owner, PropertyOwnerType ownerType)
{
    Node* node = prop.GetNode(index).Get();
    ActionManager* am = ActionManager::Get();
 
    if (IsControlDown())
    {
        if (ImGui::Button("<*") && node)
        {
            GetEditorState()->SetSelectedNode(node);
        }
    }
    else if (IsAltDown())
    {
        if (ImGui::Button("^*") && node)
        {
            GetEditorState()->InspectObject(node);
        }
    }
    else
    {
        if (ImGui::Button("*"))
        {
            if (GetEditorState()->mNodePropertySelect)
            {
                GetEditorState()->ClearNodePropertySelect();
            }
            else
            {
                GetEditorState()->SetNodePropertySelect(true, index, prop.mName);
            }
        }

        if (node != nullptr &&
            ImGui::IsItemHovered() &&
            IsKeyJustDown(KEY_DELETE))
        {
            am->EXE_EditProperty(owner, ownerType, prop.mName, index, (Node*) nullptr);
        }
    }

    ImGui::SameLine();

    static std::string sTempString;

    Node* src = owner->As<Node>();

    if (!src && owner->As<Script>())
    {
        src = owner->As<Script>()->GetOwner();
    }

    sTempString = FindRelativeNodePath(src, node);

    ImGui::InputText("##NodeNameStr", &sTempString);

    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        if (sTempString == "null" ||
            sTempString == "NULL" ||
            sTempString == "Null")
        {
            Node* nullNode = nullptr;
            if (ownerType != PropertyOwnerType::Count)
            {
                am->EXE_EditProperty(owner, ownerType, prop.mName, index, nullNode);
            }
        }
        else
        {
            Node* newNode = ResolveNodePath(src, sTempString);
            am->EXE_EditProperty(owner, ownerType, prop.mName, index, newNode);
        }
    }
}

// A reusable autocomplete dropdown function that works with various input types
// Returns true if a selection was made, and updates the input string
template<typename FilterFuncType>
static bool DrawAutocompleteDropdown(const char* dropdownId, 
                                     std::string& inputText, 
                                     const std::vector<std::string>& suggestions, 
                                     FilterFuncType filterFunc,
                                     bool forceActive = false)
{
    bool selectionMade = false;
    
    // Use static variables with the ID to handle multiple instances correctly
    static ImGuiID activeDropdownId = 0;
    static size_t selectedIndex = 0;
    static bool hasSelection = false;
    static bool dropdownActive = false;
    static std::string lastInputText = "";
    static std::vector<std::string> filteredItems;
    static bool selectionJustMade = false;

    // If forceActive is true, force the dropdown to show
    // But not if a selection was just made (prevents immediate reopen)
    if (forceActive && !selectionJustMade) {
        dropdownActive = true;
    }
    // Reset the flag after one frame
    selectionJustMade = false;
    
    // Check if this is the active dropdown
    ImGuiID inputId = ImGui::GetItemID();
    bool isInputActive = ImGui::IsItemActive();
    bool isInputFocused = ImGui::IsItemFocused();
    
    // If we have a new active item, reset state
    if (activeDropdownId != inputId && (isInputActive || isInputFocused))
    {
        activeDropdownId = inputId;
        hasSelection = false;
        dropdownActive = true; // Set to true to ensure dropdown shows
        selectedIndex = 0;
    }
    // Only hide dropdown when input completely loses focus AND mouse is not over dropdown
    // This prevents the dropdown from disappearing when clicking on it
    else if (!isInputActive && !isInputFocused && activeDropdownId == inputId )
    {
        // Use a small delay to allow interaction with the dropdown itself
        static float hideTimer = 0.0f;
        if (hideTimer > 0.1f) {
            dropdownActive = false;
            hideTimer = 0.0f;
        } else {
            hideTimer += ImGui::GetIO().DeltaTime;
        }
    }
    
    // Always update filtered items, don't just update when text changes
    // This ensures navigation works even with text entered
    filteredItems.clear();
    for (const auto& suggestion : suggestions)
    {
        if (filterFunc(suggestion, inputText))
        {
            filteredItems.push_back(suggestion);
        }
    }
    
    // Only show dropdown when we have filtered items and the dropdown is active
    // Either the input should be active/focused OR we're in the process of selecting an item
    if (!filteredItems.empty() && dropdownActive && (isInputActive || isInputFocused || activeDropdownId == inputId ))
    {
        activeDropdownId = inputId; // Keep track of which dropdown is active

        // Calculate popup position below the input field
        ImVec2 inputPos = ImGui::GetItemRectMin();
        ImVec2 inputSize = ImGui::GetItemRectSize();
        ImGui::SetNextWindowPos(ImVec2(inputPos.x, inputPos.y + inputSize.y));

        // Set maximum height for 4 items plus a bit of padding
        const float itemHeight = ImGui::GetTextLineHeightWithSpacing();
        const float maxHeight = itemHeight * 4 + ImGui::GetStyle().WindowPadding.y * 2;
        ImGui::SetNextWindowSizeConstraints(ImVec2(inputSize.x, 0), ImVec2(inputSize.x, maxHeight));

        // Make sure the dropdown appears above other elements
        ImGui::SetNextWindowBgAlpha(1.0f); // Fully opaque background
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f); // Add a border for better visibility

        // Use flags to ensure the dropdown stays on top
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_Tooltip | // Use tooltip flag to ensure it draws on top
            ImGuiWindowFlags_NoMouseInputs |
            ImGuiWindowFlags_NoScrollbar;
        
        if (ImGui::Begin(dropdownId, nullptr, flags))
        {
            // Track if mouse is hovering over this window
            // mouseOverDropdown = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

            // Get current key state
            bool upArrowPressed = ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow));
            bool downArrowPressed = ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow));
            bool tabPressed = ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Tab));
            bool enterPressed = ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter));
            bool escapePressed = ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape));
            bool backspacePressed = ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Backspace));
            
            // Handle keyboard navigation
            if (upArrowPressed)
            {
                if (!hasSelection && !filteredItems.empty())
                {
                    selectedIndex = filteredItems.size() - 1;
                    hasSelection = true;
                }
                else if (hasSelection)
                {
                    selectedIndex = (selectedIndex > 0) ? selectedIndex - 1 : filteredItems.size() - 1;
                }
                
                // Consume the event
                ImGui::GetIO().KeysDown[ImGui::GetKeyIndex(ImGuiKey_UpArrow)] = false;
            }
            else if (downArrowPressed || tabPressed)
            {
                if (!hasSelection && !filteredItems.empty())
                {
                    selectedIndex = 0;
                    hasSelection = true;
                }
                else if (hasSelection)
                {
                    selectedIndex = (selectedIndex + 1) % filteredItems.size();
                }
                
                // Consume the events
                if (downArrowPressed)
                    ImGui::GetIO().KeysDown[ImGui::GetKeyIndex(ImGuiKey_DownArrow)] = false;
                if (tabPressed)
                    ImGui::GetIO().KeysDown[ImGui::GetKeyIndex(ImGuiKey_Tab)] = false;
            }
            else if (enterPressed)
            {
                // Always hide dropdown when Enter is pressed
                dropdownActive = false;

                if (hasSelection && selectedIndex < filteredItems.size())
                {
                    inputText = filteredItems[selectedIndex];
                    selectionMade = true;
                }
                else if (inputText.empty())
                {
                    // If Enter is pressed with empty input, set to null
                    inputText = "null";
                    selectionMade = true;
                }

                ImGui::CloseCurrentPopup();

                // Consume the event
                ImGui::GetIO().KeysDown[ImGui::GetKeyIndex(ImGuiKey_Enter)] = false;
            }
            else if (escapePressed)
            {
                dropdownActive = false;
                ImGui::CloseCurrentPopup();
                
                // Consume the event
                ImGui::GetIO().KeysDown[ImGui::GetKeyIndex(ImGuiKey_Escape)] = false;
            }

            if (inputText == "" && backspacePressed)
            {
                selectedIndex = -1;
            }
            
            // Display filtered items
            // Ensure the selected item is always visible by scrolling to it
            if (hasSelection && selectedIndex < filteredItems.size())
            {
                // Calculate where we need to scroll to make the selected item visible
                float itemHeight = ImGui::GetTextLineHeightWithSpacing();
                float targetY = itemHeight * selectedIndex;
                
                // ImGui will handle the scrolling if we set focus on the selected item
                // We'll also make sure to scroll into view if arrow keys are pressed
                if (upArrowPressed || downArrowPressed)
                {
                    ImGui::SetScrollY(targetY - itemHeight); // Position selected item one row from top
                }
            }

            // Display all filtered items in a scrollable area
            for (size_t i = 0; i < filteredItems.size(); i++)
            {
                bool isSelected = (hasSelection && i == selectedIndex);

                if (isSelected)
                    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));

                // Only make selection on click, don't autofill
                bool clicked = ImGui::Selectable(filteredItems[i].c_str(), isSelected);


                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus(); // This also scrolls to make the item visible
                    ImGui::PopStyleColor();
                }

            }
        }
        ImGui::End();
        ImGui::PopStyleVar(); // Pop the window border style
    }
    else
    {

        if (filteredItems.empty() && isInputActive)
        {
            // Only hide the dropdown if there are no filtered items AND the input is active
            // This way we don't hide it when the user is trying to interact with it
            dropdownActive = false;
        }
    }

    return selectionMade;
}


void DrawAssetProperty(Property& prop, uint32_t index, Object* owner, PropertyOwnerType ownerType)
{
    Asset* asset = prop.GetAsset(index);
    ActionManager* am = ActionManager::Get();
 
    bool useAssetColor = (prop.mExtra != 0);
    if (useAssetColor)
    {
        glm::vec4 assetColor = AssetManager::Get()->GetEditorAssetColor(prop.mExtra ? (TypeId)prop.mExtra->GetInteger() : 0);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(assetColor.r, assetColor.g, assetColor.b, assetColor.a));
    }

    if (IsControlDown())
    {
        if (ImGui::Button("<<") && asset)
        {
            GetEditorState()->BrowseToAsset(asset->GetName());
        }
    }
    else if (IsAltDown())
    {
        if (ImGui::Button("^^") && asset)
        {
            GetEditorState()->InspectObject(asset);
        }
    }
    else
    {
        if (ImGui::Button(">>"))
        {
            Asset* selAsset = GetEditorState()->GetSelectedAsset();
            if (selAsset != nullptr)
            {
                AssignAssetToProperty(owner, ownerType, prop, index, selAsset);
            }
        }

        if (asset != nullptr &&
            ImGui::IsItemHovered() &&
            IsKeyJustDown(KEY_DELETE))
        {
            if (ownerType != PropertyOwnerType::Count)
            {
                am->EXE_EditProperty(owner, ownerType, prop.mName, index, (Asset*) nullptr);
            }
            else
            {
                prop.SetAsset(nullptr, index);
            }
        }
    }

    ImGui::SameLine();

    static std::string sTempString;
    sTempString = asset ? asset->GetName() : "";

    // Create a unique ID for this input
    ImGui::PushID((prop.mName + std::to_string(index)).c_str());
    
    // Use ImGui::InputText and get the active status for the dropdown
    // Capture keys to prevent input field from intercepting arrow keys
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_None;
    if (ImGui::IsItemFocused() || ImGui::IsItemActive()) {
        // If dropdown is about to be shown, don't allow up/down keys to affect the input
        flags |= ImGuiInputTextFlags_CharsNoBlank;
    }
    
    // Check if input will be activated this frame
    bool willBeActivated = ImGui::IsItemHovered() && ImGui::IsMouseClicked(0);
    
    bool textActive = ImGui::InputText("##AssetNameStr", &sTempString, flags);
    bool isInputFocused = ImGui::IsItemFocused();
    bool isInputActivated = ImGui::IsItemActivated();
    
    // If we have an extra property with type information, use it to filter assets
    TypeId assetTypeFilter = 0;
    if (prop.mExtra)
    {
        assetTypeFilter = (TypeId)prop.mExtra->GetInteger();
    }

    // Get asset suggestions based on type
    static std::vector<std::string> assetSuggestions;
    static TypeId lastAssetTypeFilter = 0;
    static double lastUpdateTime = 0.0;
    double currentTime = ImGui::GetTime();

    // Only refresh the suggestions list occasionally to avoid performance issues
    if (assetTypeFilter != lastAssetTypeFilter || currentTime - lastUpdateTime > 2.0)
    {
        assetSuggestions.clear();
        const auto& assetMap = AssetManager::Get()->GetAssetMap();
        
        for (const auto& pair : assetMap)
        {
            AssetStub* stub = pair.second;
            // If assetTypeFilter is 0 or matches the asset type, add it to suggestions
            bool typeMatches = false;
            
            if (assetTypeFilter == 0)
            {
                typeMatches = true; // Accept all types when no filter
            }
            else if (stub)
            {
                // Check for exact type match
                if (stub->mType == assetTypeFilter)
                {
                    typeMatches = true;
                }
                // Special handling for Material types
                else if (assetTypeFilter == Material::GetStaticType() && 
                         (stub->mType == MaterialBase::GetStaticType() ||
                          stub->mType == MaterialInstance::GetStaticType() ||
                          stub->mType == MaterialLite::GetStaticType()))
                {
                    typeMatches = true;
                }
            }
            
            if (stub && typeMatches)
            {
                assetSuggestions.push_back(stub->mName);
            }
        }
        
        lastAssetTypeFilter = assetTypeFilter;
        lastUpdateTime = currentTime;
    }

    // Define a filter function for our assets
    auto assetFilter = [](const std::string& suggestion, const std::string& input) {
        if (input.empty()) return true;
        // Case-insensitive substring search
        std::string lowerSuggestion = suggestion;
        std::string lowerInput = input;
        std::transform(lowerSuggestion.begin(), lowerSuggestion.end(), lowerSuggestion.begin(), ::tolower);
        std::transform(lowerInput.begin(), lowerInput.end(), lowerInput.begin(), ::tolower);
        return lowerSuggestion.find(lowerInput) != std::string::npos;
    };

    // Draw the autocomplete dropdown for asset selection
    // Force the dropdown to be active when the input is activated or text changes
    bool selectionMade = DrawAutocompleteDropdown("AssetAutocomplete", sTempString, assetSuggestions, assetFilter, 
                                                isInputActivated || textActive || willBeActivated);
    
    // Pop the ID we pushed earlier
    ImGui::PopID();

    // If a selection was made or the input was deactivated after editing, apply the change
    if (selectionMade || ImGui::IsItemDeactivatedAfterEdit())
    {
        if (sTempString == "null" ||
            sTempString == "NULL" ||
            sTempString == "Null")
        {
            Asset* nullAsset = nullptr;
            if (ownerType != PropertyOwnerType::Count)
            {
                am->EXE_EditProperty(owner, ownerType, prop.mName, index, nullAsset);
            }
            else
            {
                prop.SetAsset(nullAsset, index);
            }
        }
        else
        {
            Asset* newAsset = LoadAsset(sTempString);
            AssignAssetToProperty(owner, ownerType, prop, index, newAsset);
        }
    }

    if (useAssetColor)
    {
        ImGui::PopStyleColor();
    }
}

static bool HandleScriptSelection( std::string sTempString,
 std::string sOrigVal) {
    // Get available script files
    static std::vector<std::string> scriptSuggestions;
    static double lastUpdateTime = 0.0;
    double currentTime = ImGui::GetTime();



    // Case-insensitive filter
    auto scriptFilter = [](const std::string& suggestion, const std::string& input) {
        if (input.empty()) return true;
        std::string lowerSuggestion = suggestion, lowerInput = input;
        std::transform(lowerSuggestion.begin(), lowerSuggestion.end(), lowerSuggestion.begin(), ::tolower);
        std::transform(lowerInput.begin(), lowerInput.end(), lowerInput.begin(), ::tolower);
        return lowerSuggestion.find(lowerInput) != std::string::npos;
        };

    // Draw the input field first
    bool textActive = ImGui::InputText("##ScriptInput", &sTempString);

    // Capture input state IMMEDIATELY after InputText
    bool isInputFocused = ImGui::IsItemFocused();
    bool isInputActivated = ImGui::IsItemActivated();

    // Then draw the autocomplete dropdown with the correct state
    bool selectionMade = DrawAutocompleteDropdown("ScriptAutocomplete", sTempString, scriptSuggestions, scriptFilter,
        isInputActivated || textActive || isInputFocused);

    // Capture activation for sOrigVal AFTER the dropdown
    if (isInputActivated)
    {
        // Refresh script list every 2 seconds
        if (scriptSuggestions.empty() || currentTime - lastUpdateTime > 2.0)
        {
            scriptSuggestions = AssetManager::Get()->GetAvailableScriptFiles();
            lastUpdateTime = currentTime;
        }
        sOrigVal = sTempString;
    }
	return selectionMade;
}

static void DrawPropertyList(Object* owner, std::vector<Property>& props)
{
    ActionManager* am = ActionManager::Get();
    const float kIndentWidth = 0.0f;
    const bool ctrlDown = IsControlDown();
    const bool altDown = IsAltDown();
    const bool shiftDown = IsShiftDown();

    const char* curCategory = "";
    bool catOpen = true;

    PropertyOwnerType ownerType = PropertyOwnerType::Global;
    if (owner != nullptr)
    {
        if (owner->As<Node>())
        {
            ownerType = PropertyOwnerType::Node;
        }
        else if (owner->As<Asset>())
        {
            ownerType = PropertyOwnerType::Asset;
        }
    }

    for (uint32_t p = 0; p < props.size(); ++p)
    {
        Property& prop = props[p];
        DatumType propType = prop.GetType();

        if (strcmp(curCategory, prop.mCategory) != 0)
        {
            curCategory = prop.mCategory;

            if (strcmp(prop.mCategory, "") != 0)
            {
                catOpen = ImGui::CollapsingHeader(curCategory, ImGuiTreeNodeFlags_DefaultOpen);
            }
        }

        if (!catOpen)
            continue;

        ImGui::PushID(p);

        // Allow custom property drawing
        bool custom = owner ? owner->DrawCustomProperty(prop) : false;

        if (custom)
        {
            ImGui::PopID();
            continue;
        }

        // Bools handle name on same line after checkbox
        if (propType != DatumType::Bool || prop.GetCount() > 1)
        {
            const char* displayText = prop.mDisplayName.empty() ? prop.mName.c_str() : prop.mDisplayName.c_str();
            ImGui::Text(displayText);

            if (kIndentWidth > 0.0f)
            {
                ImGui::Indent(kIndentWidth);
            }

            if (prop.IsVector())
            {
                ImGui::SameLine();
                if (ImGui::Button("+"))
                {
                    if (prop.IsExternal())
                    {
                        prop.PushBackVector();
                    }
                    else
                    {
                        // Script properties use internal storage
                        prop.SetCount(prop.GetCount() + 1);

                        // HACK: Re-assign values so that we trigger the script prop change handler to propagate changes to script.
                        prop.SetValue(prop.GetValue(0), 0, prop.GetCount());
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("-"))
                {
                    if (prop.GetCount() > 0)
                    {
                        if (prop.IsExternal())
                        {
                            prop.EraseVector(prop.GetCount() - 1);
                        }
                        else
                        {
                            // Script properties use internal storage
                            prop.Erase(prop.GetCount() - 1);

                            // We need to trigger the script prop change handler to propagate changes to script.
                            uint32_t currentCount = prop.GetCount();
                            if (currentCount > 0)
                            {
                                // For non-empty arrays, use the normal approach
                                prop.SetValue(prop.GetValue(0), 0, currentCount);
                            }
                            else
                            {
                                // For empty arrays, we need to ensure the change propagates to the script property
                                // Call the change handler directly to sync the empty state
                                if (prop.mChangeHandler)
                                {
                                    // The change handler expects the property to already be modified, so call it
                                    // with a null pointer to indicate this is a count change, not a value change
                                    prop.mChangeHandler(&prop, 0, nullptr);
                                }
                            }
                        }
                    }
                }
            }
        }

        for (uint32_t i = 0; i < prop.GetCount(); ++i)
        {
            ImGui::PushID(i);

            switch (propType)
            {
            case DatumType::Integer:
            {
                static int32_t sOrigVal = 0;
                int32_t propVal = prop.GetInteger(i);
                int32_t preVal = propVal;

                if (prop.mEnumCount > 0)
                {
                    if (ImGui::Combo("", &propVal, prop.mEnumStrings, prop.mEnumCount))
                    {
                        am->EXE_EditProperty(owner, ownerType, prop.mName, i, propVal);
                    }
                }
                else
                {
                    ImGui::DragInt("", &propVal);

                    if (ImGui::IsItemActivated())
                    {
                        sOrigVal = preVal;
                    }

                    if (ImGui::IsItemDeactivatedAfterEdit())
                    {
                        prop.SetInteger(sOrigVal, i);
                        am->EXE_EditProperty(owner, ownerType, prop.mName, i, propVal);
                    }
                    else if (propVal != preVal)
                    {
                        prop.SetInteger(propVal, i);
                    }
                }
                break;
            }
            case DatumType::Float:
            {
                static float sOrigVal = 0.0f;
                float propVal = prop.GetFloat(i);
                float preVal = propVal;

                ImGui::DragFloat("", &propVal);

                if (ImGui::IsItemActivated())
                {
                    sOrigVal = preVal;
                }

                if (ImGui::IsItemDeactivatedAfterEdit())
                {
                    prop.SetFloat(sOrigVal, i);
                    am->EXE_EditProperty(owner, ownerType, prop.mName, i, propVal);
                }
                else if (propVal != preVal)
                {
                    prop.SetFloat(propVal, i);
                }
                break;
            }
            case DatumType::Bool:
            {
                bool propVal = prop.GetBool(i);
                if (ImGui::Checkbox("", &propVal))
                {
                    am->EXE_EditProperty(owner, ownerType, prop.mName, i, propVal);
                }

                ImGui::SameLine();
                const char* displayText = prop.mDisplayName.empty() ? prop.mName.c_str() : prop.mDisplayName.c_str();
                ImGui::Text(displayText);
                break;
            }
            case DatumType::String:
            {
                static std::string sTempString;
                static std::string sOrigVal;
                sTempString = prop.GetString(i);

                if (prop.mName == "Script")
                {
                    // Get available script files
                    // Then draw the autocomplete dropdown with the correct state
                  /*  bool selectionMade = HandleScriptSelection( sTempString,
						sOrigVal);*/

                    static std::vector<std::string> scriptSuggestions;
                    static double lastUpdateTime = 0.0;
                    double currentTime = ImGui::GetTime();



                    // Case-insensitive filter
                    auto scriptFilter = [](const std::string& suggestion, const std::string& input) {
                        if (input.empty()) return true;
                        std::string lowerSuggestion = suggestion, lowerInput = input;
                        std::transform(lowerSuggestion.begin(), lowerSuggestion.end(), lowerSuggestion.begin(), ::tolower);
                        std::transform(lowerInput.begin(), lowerInput.end(), lowerInput.begin(), ::tolower);
                        return lowerSuggestion.find(lowerInput) != std::string::npos;
                        };

                    // Draw the input field first
                    bool textActive = ImGui::InputText("##ScriptInput", &sTempString);

                    // Capture input state IMMEDIATELY after InputText
                    bool isInputFocused = ImGui::IsItemFocused();
                    bool isInputActivated = ImGui::IsItemActivated();

                    // Then draw the autocomplete dropdown with the correct state
                    bool selectionMade = DrawAutocompleteDropdown("ScriptAutocomplete", sTempString, scriptSuggestions, scriptFilter,
                        isInputActivated || textActive || isInputFocused);

                    // Capture activation for sOrigVal AFTER the dropdown
                    if (isInputActivated)
                    {
                        // Refresh script list every 2 seconds
                        if (scriptSuggestions.empty() || currentTime - lastUpdateTime > 2.0)
                        {
                            scriptSuggestions = AssetManager::Get()->GetAvailableScriptFiles();
                            lastUpdateTime = currentTime;
                        }
                        sOrigVal = sTempString;
                    }

                    if (selectionMade || ImGui::IsItemDeactivatedAfterEdit())
                    {
                        if (sTempString != sOrigVal)
                        {
                            am->EXE_EditProperty(owner, ownerType, prop.mName, i, sTempString);
                        }
                    }
                }
                else
                {

                    ImGui::InputTextMultiline("", &sTempString, ImVec2(ImGui::CalcItemWidth(), 19.0f), ImGuiInputTextFlags_CtrlEnterForNewLine);

                    if (ImGui::IsItemActivated())
                    {
                        sOrigVal = sTempString;
                    }

                    if (ImGui::IsItemDeactivatedAfterEdit())
                    {
                        if (sTempString != sOrigVal)
                        {
                            am->EXE_EditProperty(owner, ownerType, prop.mName, i, sTempString);
                        }
                    }
                }
                break;
            }
            case DatumType::Vector2D:
            {
                static glm::vec2 sOrigVal = {};
                glm::vec2 propVal = prop.GetVector2D(i);
                glm::vec2 preVal = propVal;

                ImGui::DragFloat2("", &propVal[0], 1.0f, 0.0f, 0.0f, "%.2f");

                if (ImGui::IsItemActivated())
                {
                    sOrigVal = preVal;
                }

                if (ImGui::IsItemDeactivatedAfterEdit())
                {
                    prop.SetVector2D(sOrigVal, i);
                    am->EXE_EditProperty(owner, ownerType, prop.mName, i, propVal);
                }
                else if (propVal != preVal)
                {
                    prop.SetVector2D(propVal, i);
                }
                break;
            }
            case DatumType::Vector:
            {
                static glm::vec3 sOrigVal = {};
                glm::vec3 propVal = prop.GetVector(i);
                glm::vec3 preVal = propVal;

                ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.85f);
                //ImGui::DragFloat3("", &propVal[0], 1.0f, 0.0f, 0.0f, "%.2f");
                float vMin = 0.0f;
                float vMax = 0.0f;
                ImGui::OctDragScalarN("", ImGuiDataType_Float, &propVal[0], 3, 1.0f, &vMin, &vMax, "%.2f", 0);
                ImGui::PopItemWidth();

                if (ImGui::IsItemActivated())
                {
                    sOrigVal = preVal;
                }

                if (ImGui::IsItemDeactivatedAfterEdit())
                {
                    prop.SetVector(sOrigVal, i);

                    // Handle edge case where Rotation property is reset so we need to recompute transform to update mRotationEuler.
                    if (owner->As<Node3D>())
                    {
                        owner->As<Node3D>()->UpdateTransform(false);
                    }

                    am->EXE_EditProperty(owner, ownerType, prop.mName, i, propVal);
                }
                else if (propVal != preVal)
                {
                    prop.SetVector(propVal, i);
                }
                break;
            }
            case DatumType::Color:
            {
                static glm::vec4 sOrigVal = {};
                glm::vec4 propVal = prop.GetColor(i);
                glm::vec4 preVal = propVal;

                ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.85f);
                ImGui::OctColorEdit4("", &propVal[0], 0);
                ImGui::PopItemWidth();

                if (ImGui::IsItemActivated())
                {
                    sOrigVal = preVal;
                }

                if (ImGui::IsItemDeactivatedAfterEdit())
                {
                    prop.SetColor(sOrigVal, i);
                    am->EXE_EditProperty(owner, ownerType, prop.mName, i, propVal);
                }
                else if (propVal != preVal)
                {
                    prop.SetColor(propVal, i);
                }
                break;
            }
            case DatumType::Node:
            {
                DrawNodeProperty(prop, i, owner, ownerType);
                break;
            }
            case DatumType::Asset:
            {
                DrawAssetProperty(prop, i, owner, ownerType);
                break;
            }
            case DatumType::Byte:
            {
                static int32_t sOrigVal = 0;
                int32_t propVal = prop.GetByte(i);
                int32_t preVal = propVal;

                if (prop.mEnumCount > 0)
                {
                    if (ImGui::Combo("", &propVal, prop.mEnumStrings, prop.mEnumCount))
                    {
                        am->EXE_EditProperty(owner, ownerType, prop.mName, i, (uint8_t)propVal);
                    }
                }
                else if (prop.mExtra &&
                    (prop.mExtra->GetInteger() == int32_t(ByteExtra::FlagWidget) ||
                    prop.mExtra->GetInteger() == int32_t(ByteExtra::ExclusiveFlagWidget))) // Should these be bitwise checks?
                {
                    ImVec2 itemSpacing = ImGui::GetStyle().ItemSpacing;
                    itemSpacing.x = 2.0f;
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, itemSpacing);

                    for (uint32_t f = 0; f < 8; ++f)
                    {
                        if (f > 0)
                            ImGui::SameLine();

                        ImGui::PushID(f);

                        int32_t bit = 7 - int32_t(f);
                        bool bitSet = (propVal >> bit) & 1;

                        ImVec4* imColors = ImGui::GetStyle().Colors;

                        if (bitSet)
                        {
                            ImGui::PushStyleColor(ImGuiCol_Button, kSelectedColor);
                        }
                        else
                        {
                            ImGui::PushStyleColor(ImGuiCol_Button, imColors[ImGuiCol_Button]);
                        }

                        if (ImGui::Button("", ImVec2(16.0f, 16.0f)))
                        {
                            bitSet = !bitSet;
                            uint8_t newBitMask = propVal;
                            if (bitSet)
                            {
                                newBitMask = newBitMask | (1 << bit);
                            }
                            else
                            {
                                newBitMask = newBitMask & (~(1 << bit));
                            }

                            propVal = newBitMask;

                            am->EXE_EditProperty(owner, ownerType, prop.mName, i, uint8_t(propVal));
                        }

                        ImGui::PopStyleColor();

                        ImGui::PopID();
                    }

                    ImGui::PopStyleVar();
                }
                else
                {
                    ImGui::DragInt("", &propVal);

                    if (ImGui::IsItemActivated())
                    {
                        sOrigVal = preVal;
                    }

                    if (ImGui::IsItemDeactivatedAfterEdit())
                    {
                        prop.SetByte((uint8_t)sOrigVal, i);
                        am->EXE_EditProperty(owner, ownerType, prop.mName, i, uint8_t(propVal));
                    }
                    else if (propVal != preVal)
                    {
                        prop.SetByte((uint8_t)propVal, i);
                    }
                }
                break;
            }
            case DatumType::Short:
            {
                static int32_t sOrigVal = 0;
                int32_t propVal = prop.GetShort(i);
                int32_t preVal = propVal;

                if (prop.mEnumCount > 0)
                {
                    if (ImGui::Combo("", &propVal, prop.mEnumStrings, prop.mEnumCount))
                    {
                        am->EXE_EditProperty(owner, ownerType, prop.mName, i, (int16_t)propVal);
                    }
                }
                else
                {
                    ImGui::DragInt("", &propVal);

                    if (ImGui::IsItemActivated())
                    {
                        sOrigVal = preVal;
                    }

                    if (ImGui::IsItemDeactivatedAfterEdit())
                    {
                        prop.SetShort((int16_t)sOrigVal, i);
                        am->EXE_EditProperty(owner, ownerType, prop.mName, i, int16_t(propVal));
                    }
                    else if (propVal != preVal)
                    {
                        prop.SetShort((int16_t)propVal, i);
                    }
                }
                break;
            }
            default: break;
            }

            ImGui::PopID();
        }

        if (propType != DatumType::Bool)
        {
            if (kIndentWidth > 0.0f)
            {
                ImGui::Unindent(kIndentWidth);
            }
        }

        ImGui::PopID();
    }
}

static void DrawAddNodeMenu(Node* node)
{
    ActionManager* am = ActionManager::Get();

    if (!sNodesDiscovered)
    {
        DiscoverNodeClasses();
        sNodesDiscovered = true;
    }

    if (ImGui::MenuItem("Node"))
    {
        Node* newNode = am->EXE_SpawnNode(Node::GetStaticType());
        if (node)
        {
            node->AddChild(newNode);
        }
        else
        {
            GetWorld(0)->PlaceNewlySpawnedNode(ResolvePtr(newNode), {});
        }
        
        GetEditorState()->SetSelectedNode(newNode);
    }

    if (ImGui::BeginMenu("3D"))
    {
        for (uint32_t i = 0; i < sNode3dNames.size(); ++i)
        {
            if (ImGui::MenuItem(sNode3dNames[i].c_str()))
            {
                const char* nodeName = sNode3dNames[i].c_str();

                Node* newNode = am->EXE_SpawnNode(nodeName);

                if (node)
                    node->AddChild(newNode);
                else
                    GetWorld(0)->PlaceNewlySpawnedNode(ResolvePtr(newNode), {});

                GetEditorState()->SetSelectedNode(newNode);
            }
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Widget"))
    {
        for (uint32_t i = 0; i < sNodeWidgetNames.size(); ++i)
        {
            if (ImGui::MenuItem(sNodeWidgetNames[i].c_str()))
            {
                const char* nodeName = sNodeWidgetNames[i].c_str();

                Node* newNode = am->EXE_SpawnNode(nodeName);

                if (node)
                    node->AddChild(newNode);
                else
                    GetWorld(0)->PlaceNewlySpawnedNode(ResolvePtr(newNode), {});

                GetEditorState()->SetSelectedNode(newNode);
            }
        }

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Scene"))
    {
        // Populate the scene list if needed
        if (sSceneList.size() == 0)
        {
            auto& assetMap = AssetManager::Get()->GetAssetMap();
            for (auto it : assetMap)
            {
                if (it.second &&
                    it.second->mType == Scene::GetStaticType() &&
                    !it.second->mEngineAsset)
                {
                    sSceneList.push_back(it.second->mName);
                }
            }

            std::sort(sSceneList.begin(), sSceneList.end());
        }

        for (uint32_t i = 0; i < sSceneList.size(); ++i)
        {
            if (ImGui::MenuItem(sSceneList[i].c_str()))
            {
                Scene* scene = LoadAsset<Scene>(sSceneList[i]);
                if (scene != nullptr)
                {
                    Node* spawnedNode = ActionManager::Get()->EXE_SpawnNode(scene);

                    if (spawnedNode)
                    {
                        Node* selNode = GetEditorState()->GetSelectedNode();
                        Node* parent = selNode ? selNode : GetWorld(0)->GetRootNode();

                        if (parent != nullptr)
                        {
                            parent->AddChild(spawnedNode);
                        }
                        else
                        {
                            GetWorld(0)->SetRootNode(spawnedNode);
                        }

                        GetEditorState()->SetSelectedNode(spawnedNode);
                    }
                }
            }
        }

        ImGui::EndMenu();
    }
    else
    {
        sSceneList.clear();
    }

    if (sNodeOtherNames.size() > 0 &&
        ImGui::BeginMenu("Other"))
    {
        for (uint32_t i = 0; i < sNodeOtherNames.size(); ++i)
        {
            if (ImGui::MenuItem(sNodeOtherNames[i].c_str()))
            {
                const char* nodeName = sNodeOtherNames[i].c_str();
                Node* newNode = am->EXE_SpawnNode(nodeName);

                if (node)
                    node->AddChild(newNode);
                else
                    GetWorld(0)->PlaceNewlySpawnedNode(ResolvePtr(newNode), {});

                GetEditorState()->SetSelectedNode(newNode);
            }
        }

        ImGui::EndMenu();
    }
}

static void DrawSpawnBasic3dMenu(Node* node, bool setFocusPos)
{
    ActionManager* am = ActionManager::Get();
    glm::vec3 spawnPos = EditorGetFocusPosition();
    Asset* selAsset = GetEditorState()->GetSelectedAsset();

    if (ImGui::MenuItem(BASIC_NODE_3D))
        am->SpawnBasicNode(BASIC_NODE_3D, node, selAsset, setFocusPos, spawnPos);
    if (ImGui::MenuItem(BASIC_STATIC_MESH))
        am->SpawnBasicNode(BASIC_STATIC_MESH, node, selAsset, setFocusPos, spawnPos);
    if (ImGui::MenuItem(BASIC_SKELETAL_MESH))
        am->SpawnBasicNode(BASIC_SKELETAL_MESH, node, selAsset, setFocusPos, spawnPos);
    if (ImGui::MenuItem(BASIC_POINT_LIGHT))
        am->SpawnBasicNode(BASIC_POINT_LIGHT, node, selAsset, setFocusPos, spawnPos);
    if (ImGui::MenuItem(BASIC_DIRECTIONAL_LIGHT))
        am->SpawnBasicNode(BASIC_DIRECTIONAL_LIGHT, node, selAsset, setFocusPos, spawnPos);
    if (ImGui::MenuItem(BASIC_PARTICLE))
        am->SpawnBasicNode(BASIC_PARTICLE, node, selAsset, setFocusPos, spawnPos);
    if (ImGui::MenuItem(BASIC_AUDIO))
        am->SpawnBasicNode(BASIC_AUDIO, node, selAsset, setFocusPos, spawnPos);
    if (ImGui::MenuItem(BASIC_BOX))
        am->SpawnBasicNode(BASIC_BOX, node, selAsset, setFocusPos, spawnPos);
    if (ImGui::MenuItem(BASIC_SPHERE))
        am->SpawnBasicNode(BASIC_SPHERE, node, selAsset, setFocusPos, spawnPos);
    if (ImGui::MenuItem(BASIC_CAPSULE))
        am->SpawnBasicNode(BASIC_CAPSULE, node, selAsset, setFocusPos, spawnPos);
    if (ImGui::MenuItem(BASIC_SCENE))
        am->SpawnBasicNode(BASIC_SCENE, node, selAsset, setFocusPos, spawnPos);
    if (ImGui::MenuItem(BASIC_CAMERA))
        am->SpawnBasicNode(BASIC_CAMERA, node, selAsset, setFocusPos, spawnPos);
    if (ImGui::MenuItem(BASIC_TEXT_MESH))
        am->SpawnBasicNode(BASIC_TEXT_MESH, node, selAsset, setFocusPos, spawnPos);
    if (ImGui::MenuItem(BASIC_INSTANCED_MESH))
        am->SpawnBasicNode(BASIC_INSTANCED_MESH, node, selAsset, setFocusPos, spawnPos);
}
static void DrawSpawnBasicWidgetMenu(Node* node)
{
    ActionManager* am = ActionManager::Get();
    Asset* selAsset = GetEditorState()->GetSelectedAsset();

    const char* widgetTypeName = nullptr;

    if (ImGui::MenuItem("Widget"))
        widgetTypeName = "Widget";
    if (ImGui::MenuItem("Quad"))
        widgetTypeName = "Quad";
    if (ImGui::MenuItem("Text"))
        widgetTypeName = "Text";
    if (ImGui::MenuItem("Button"))
        widgetTypeName = "Button";

    if (widgetTypeName != nullptr)
    {
        Node* newWidget = am->EXE_SpawnNode(widgetTypeName);

        if (node == nullptr)
        {
            GetWorld(0)->PlaceNewlySpawnedNode(ResolvePtr(newWidget), {});
        }
        else
        {
            node->AddChild(newWidget);
        }

        OCT_ASSERT(newWidget);
        if (newWidget)
        {
            GetEditorState()->SetSelectedNode(newWidget);
        }
    }
}

static void DrawPackageMenu()
{
    ActionManager* am = ActionManager::Get();

    //if (ImGui::BeginPopup("PackagePopup"))
    //{
#if PLATFORM_WINDOWS
    if (ImGui::MenuItem("Windows"))
        am->BuildData(Platform::Windows, false);
#elif PLATFORM_LINUX
    if (ImGui::MenuItem("Linux"))
        am->BuildData(Platform::Linux, false);
#endif
    if (ImGui::MenuItem("Android"))
        am->BuildData(Platform::Android, false);
    if (ImGui::MenuItem("GameCube"))
        am->BuildData(Platform::GameCube, false);
    if (ImGui::MenuItem("Wii"))
        am->BuildData(Platform::Wii, false);
    if (ImGui::MenuItem("3DS"))
        am->BuildData(Platform::N3DS, false);
    if (ImGui::MenuItem("GameCube Embedded"))
        am->BuildData(Platform::GameCube, true);
    if (ImGui::MenuItem("Wii Embedded"))
        am->BuildData(Platform::Wii, true);
    if (ImGui::MenuItem("3DS Embedded"))
        am->BuildData(Platform::N3DS, true);

    //    ImGui::EndPopup();
    //}
}

static void DrawScenePanel()
{
    ActionManager* am = ActionManager::Get();

    const float halfHeight = ImGui::GetIO().DisplaySize.y / 2.0f;

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(kSidePaneWidth, halfHeight));

    ImGui::Begin("Scene", nullptr, kPaneWindowFlags);

    static std::string sFilterStrTemp;
    static std::string sFilterStr;
    bool filterCleared = false;

    if (ImGui::InputText("Filter", &sFilterStrTemp, ImGuiInputTextFlags_EnterReturnsTrue))
    {
        sFilterStr = sFilterStrTemp;
        if (sFilterStr == "")
        {
            filterCleared = true;
        }
    }

    std::string filterStrUpper = sFilterStr;
    for (uint32_t c = 0; c < filterStrUpper.size(); ++c)
    {
        filterStrUpper[c] = toupper(filterStrUpper[c]);
    }

    if (filterCleared && GetEditorState()->GetSelectedNode() != nullptr)
    {
        GetEditorState()->mTrackSelectedNode = true;
    }

    ImGuiTreeNodeFlags treeNodeFlags =
        ImGuiTreeNodeFlags_OpenOnArrow
        | ImGuiTreeNodeFlags_OpenOnDoubleClick
        | ImGuiTreeNodeFlags_SpanAvailWidth
        | ImGuiTreeNodeFlags_DefaultOpen;

    World* world = GetWorld(0);
    Node* rootNode = world ? world->GetRootNode() : nullptr;
    bool sNodeContextActive = false;
    bool trackingNode = GetEditorState()->mTrackSelectedNode;

    glm::vec4 sceneColor = AssetManager::Get()->GetEditorAssetColor(Scene::GetStaticType());
    ImVec4 sceneColorIm = ImVec4(sceneColor.r, sceneColor.g, sceneColor.b, sceneColor.a);
    ImVec4 subSceneColorIm = ImVec4(0.5f, 0.8f, 0.2f, 1.0f);

    std::function<void(Node*, Scene*)> drawTree = [&](Node* node, Scene* subScene)
    {
        bool nodeSelected = GetEditorState()->IsNodeSelected(node);
        bool nodeSceneLinked = node->IsSceneLinked();
        bool inSubScene = (subScene != nullptr);
        bool nodeHasScene = (node->GetScene() != nullptr);

        bool drawTreeNode = true;

        if (filterStrUpper != "")
        {
            std::string upperName = node->GetName();
            for (uint32_t c = 0; c < upperName.size(); ++c)
            {
                upperName[c] = toupper(upperName[c]);
            }

            drawTreeNode = (upperName.find(filterStrUpper) != std::string::npos);
        }

        if (drawTreeNode)
        {
            ImGuiTreeNodeFlags nodeFlags = treeNodeFlags;
            if (nodeSelected)
            {
                nodeFlags |= ImGuiTreeNodeFlags_Selected;
            }

            if (node->GetNumChildren() == 0 ||
                node->AreAllChildrenHiddenInTree() ||
                filterStrUpper != "")
            {
                nodeFlags |= ImGuiTreeNodeFlags_Leaf;
            }

            if (inSubScene)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, subSceneColorIm);
            }
            else if (nodeHasScene)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, sceneColorIm);
            }

            bool nodeOpen = ImGui::TreeNodeEx(node->GetName().c_str(), nodeFlags);
            bool nodeClicked = ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen();
            bool nodeMiddleClicked = ImGui::IsItemClicked(ImGuiMouseButton_Middle);
            bool expandChildren = trackingNode || (nodeMiddleClicked && IsControlDown());
            bool collapseChildren = !expandChildren && nodeMiddleClicked;

            if (inSubScene || nodeHasScene)
            {
                ImGui::PopStyleColor();
            }

            if (nodeSelected && GetEditorState()->mTrackSelectedNode)
            {
                ImGui::SetScrollHereY(0.5f);
                GetEditorState()->mTrackSelectedNode = false;
            }

            if (ImGui::BeginPopupContextItem())
            {
                bool setTextInputFocus = false;
                bool closeContextPopup = false;

                sNodeContextActive = true;

                if (node->IsSceneLinked() && ImGui::Selectable("Open Scene"))
                {
                    GetEditorState()->OpenEditScene(node->GetScene());
                }
                if (node->GetParent() != nullptr &&
                    !inSubScene &&
                    ImGui::BeginMenu("Move"))
                {
                    Node* parent = node->GetParent();
                    int32_t childSlot = parent->FindChildIndex(node);

                    if (ImGui::Selectable("Top"))
                        am->EXE_AttachNode(node, parent, 0, -1);
                    if (ImGui::Selectable("Up"))
                        am->EXE_AttachNode(node, parent, glm::max<int32_t>(childSlot - 1, 0), -1);
                    if (ImGui::Selectable("Down"))
                        am->EXE_AttachNode(node, parent, childSlot + 1, -1);
                    if (ImGui::Selectable("Bottom"))
                        am->EXE_AttachNode(node, parent, -1, -1);

                    ImGui::EndMenu();
                }
                if (ImGui::Selectable("Rename", false, ImGuiSelectableFlags_DontClosePopups))
                {
                    ImGui::OpenPopup("Rename Node");
                    strncpy(sPopupInputBuffer, node->GetName().c_str(), kPopupInputBufferSize);
                    setTextInputFocus = true;
                }
                if (!inSubScene && ImGui::Selectable("Duplicate"))
                {
                    am->DuplicateNodes({ node });
                }
                if (!nodeSceneLinked && !inSubScene && ImGui::Selectable("Attach Selected"))
                {
                    am->AttachSelectedNodes(node, -1);
                }
                if (!nodeSceneLinked && !inSubScene && node->As<SkeletalMesh3D>())
                {
                    if (ImGui::Selectable("Attach Selected To Bone", false, ImGuiSelectableFlags_DontClosePopups))
                    {
                        ImGui::OpenPopup("Attach Selected To Bone");
                        setTextInputFocus = true;
                    }
                }
                if (!node->IsWorldRoot() && !inSubScene && ImGui::Selectable("Set Root Node"))
                {
                    am->EXE_SetRootNode(node);
                }
                if ((nodeSceneLinked || inSubScene) && ImGui::Selectable("Unlink Scene"))
                {
                    am->EXE_UnlinkScene(node->GetSubRoot());
                }
                if (!inSubScene && ImGui::Selectable("Delete"))
                {
                    am->EXE_DeleteNode(node);
                }
                if (!inSubScene &&
                    node->As<StaticMesh3D>() &&
                    ImGui::Selectable("Merge"))
                {
                    LogDebug("TODO: Implement Merge for static meshes.");
                }
                if (!nodeSceneLinked && !inSubScene && ImGui::BeginMenu("Add Node"))
                {
                    DrawAddNodeMenu(node);
                    ImGui::EndMenu();
                }
                if (!nodeSceneLinked && !inSubScene && ImGui::BeginMenu("Add Basic 3D"))
                {
                    DrawSpawnBasic3dMenu(node, false);
                    ImGui::EndMenu();
                }
                if (!nodeSceneLinked && !inSubScene && ImGui::BeginMenu("Add Basic Widget"))
                {
                    DrawSpawnBasicWidgetMenu(node);
                    ImGui::EndMenu();
                }
                //if (ImGui::Selectable("Add Scene..."))
                //{

                //}

                // Sub Popups

                if (ImGui::BeginPopup("Rename Node"))
                {
                    if (setTextInputFocus)
                    {
                        ImGui::SetKeyboardFocusHere();
                    }

                    if (ImGui::InputText("Node Name", sPopupInputBuffer, kPopupInputBufferSize, ImGuiInputTextFlags_EnterReturnsTrue))
                    {
                        std::string newName = sPopupInputBuffer;
                        am->EXE_EditProperty(node, PropertyOwnerType::Node, "Name", 0, newName);
                    }

                    ImGui::EndPopup();
                }

                if (!inSubScene && ImGui::BeginPopup("Attach Selected To Bone"))
                {
                    if (setTextInputFocus)
                    {
                        ImGui::SetKeyboardFocusHere();
                    }

                    if (ImGui::InputText("Bone Name", sPopupInputBuffer, kPopupInputBufferSize, ImGuiInputTextFlags_EnterReturnsTrue))
                    {
                        SkeletalMesh3D* skNode = node->As<SkeletalMesh3D>();
                        if (skNode)
                        {
                            int32_t boneIdx = skNode->FindBoneIndex(sPopupInputBuffer);
                            am->AttachSelectedNodes(skNode, boneIdx);
                            ImGui::CloseCurrentPopup();
                            closeContextPopup = true;
                        }
                    }

                    if (ImGui::BeginMenu("Bones"))
                    {
                        SkeletalMesh3D* skNode = node->As<SkeletalMesh3D>();
                        SkeletalMesh* skMesh = skNode ? skNode->GetSkeletalMesh() : nullptr;

                        if (skMesh)
                        {
                            for (uint32_t i = 0; i < skMesh->GetNumBones(); ++i)
                            {
                                ImGui::PushID(i);
                                if (ImGui::MenuItem(skMesh->GetBone(i).mName.c_str()))
                                {
                                    am->AttachSelectedNodes(skNode, (int32_t)i);
                                    ImGui::CloseCurrentPopup();
                                    closeContextPopup = true;
                                }
                                ImGui::PopID();
                            }
                        }

                        ImGui::EndMenu();
                    }

                    ImGui::EndPopup();
                }

                if (closeContextPopup)
                {
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }

            if (nodeOpen)
            {
                if (filterStrUpper != "")
                {
                    ImGui::TreePop();
                }

                for (uint32_t i = 0; i < node->GetNumChildren(); ++i)
                {
                    Scene* newSubScene = nodeSceneLinked ? node->GetScene() : subScene;

                    Node* child = node->GetChild(i);
                    if (!child->mHiddenInTree)
                    {
                        if (collapseChildren)
                        {
                            ImGui::SetNextItemOpen(false);
                        }
                        else if (expandChildren)
                        {
                            ImGui::SetNextItemOpen(true);
                        }

                        drawTree(child, newSubScene);
                    }
                }

                if (filterStrUpper == "")
                {
                    ImGui::TreePop();
                }
            }

            if (nodeClicked)
            {
                if (GetEditorState()->mNodePropertySelect)
                {
                    GetEditorState()->AssignNodePropertySelect(node);
                }
                else
                {
                    if (nodeSelected)
                    {
                        GetEditorState()->DeselectNode(node);
                    }
                    else
                    {
                        if (ImGui::GetIO().KeyCtrl)
                        {
                            GetEditorState()->AddSelectedNode(node, false);
                        }
                        else
                        {
                            GetEditorState()->SetSelectedNode(node);
                        }
                    }
                }
            }
        }
        else if (filterStrUpper != "")
        {
            // If we have a filter string but didnt draw the node,
            // we still need to traverse the tree to see if there are matching children.
            Scene* newSubScene = nodeSceneLinked ? node->GetScene() : subScene;

            for (uint32_t i = 0; i < node->GetNumChildren(); ++i)
            {
                drawTree(node->GetChild(i), newSubScene);
            }
        }
    };

    if (rootNode != nullptr)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 6.0f);
        drawTree(rootNode, nullptr);
        ImGui::PopStyleVar();
    }

    bool setKeyboardFocus = false;

    // If no popup is open and we aren't inputting text...
    if (!ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup) &&
        ImGui::IsWindowHovered() &&
        !ImGui::GetIO().WantTextInput && 
        !sNodeContextActive)
    {
        const bool ctrlDown = IsControlDown();
        const bool shiftDown = IsShiftDown();
        const bool altDown = IsAltDown();

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
        {
            ImGui::OpenPopup("Null Node Context");
        }

        const std::vector<Node*>& selNodes = GetEditorState()->GetSelectedNodes();

        // Move Up/Down selected node.
        if (selNodes.size() == 1 &&
            selNodes[0]->GetParent() != nullptr)
        {
            Node* node = selNodes[0];
            Node* parent = node->GetParent();
            int32_t childIndex = parent->FindChildIndex(node);

            if (IsKeyJustDown(KEY_MINUS))
            {
                am->EXE_AttachNode(node, parent, glm::max<int32_t>(childIndex - 1, 0), -1);
            }
            else if (IsKeyJustDown(KEY_PLUS))
            {
                am->EXE_AttachNode(node, parent, childIndex + 1, -1);
            }
        }

        if (selNodes.size() > 0)
        {
            if (IsKeyJustDown(KEY_DELETE))
            {
                am->EXE_DeleteNodes(selNodes);
            }
            else if (ctrlDown && IsKeyJustDown(KEY_D))
            {
                am->DuplicateNodes(selNodes);
            }
            else if (!ctrlDown && IsKeyJustDown(KEY_F2))
            {
                ImGui::OpenPopup("Rename Node F2");
                strncpy(sPopupInputBuffer, selNodes[0]->GetName().c_str(), kPopupInputBufferSize);
                setKeyboardFocus = true;
            }
        }
    }

    if (ImGui::BeginPopup("Rename Node F2"))
    {
        if (setKeyboardFocus)
        {
            ImGui::SetKeyboardFocusHere();
        }

        if (ImGui::InputText("Node Name F2", sPopupInputBuffer, kPopupInputBufferSize, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            const std::vector<Node*>& selNodes = GetEditorState()->GetSelectedNodes();
            std::string newName = sPopupInputBuffer;
            am->EXE_EditProperty(selNodes[0], PropertyOwnerType::Node, "Name", 0, newName);
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Null Node Context"))
    {
        if (ImGui::BeginMenu("Spawn Node"))
        {
            DrawAddNodeMenu(nullptr);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Spawn Basic 3D"))
        {
            DrawSpawnBasic3dMenu(nullptr, false);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Spawn Basic Widget"))
        {
            DrawSpawnBasicWidgetMenu(nullptr);
            ImGui::EndMenu();
        }

        ImGui::EndPopup();
    }

    ImGui::End();
}

static void DrawAssetsContextPopup(AssetStub* stub, AssetDir* dir)
{
    bool setTextInputFocus = false;
    bool closeContextPopup = false;
    static TypeId sNewAssetType = INVALID_TYPE_ID;

    ActionManager* actMan = ActionManager::Get();
    AssetManager* assMan = AssetManager::Get();

    AssetDir* curDir = GetEditorState()->GetAssetDirectory();

    bool engineFile = false;
    if ((stub && stub->mEngineAsset) ||
        (dir && dir->mEngineDir))
    {
        engineFile = true;
    }

    bool canInstantiate = false;
    if (stub &&
        (stub->mType == Scene::GetStaticType() ||
            stub->mType == SoundWave::GetStaticType() ||
            stub->mType == StaticMesh::GetStaticType() ||
            stub->mType == SkeletalMesh::GetStaticType() ||
            stub->mType == ParticleSystem::GetStaticType()))
    {
        canInstantiate = true;
    }

    if (stub && ImGui::Selectable("Properties"))
    {
        if (stub->mAsset == nullptr)
            AssetManager::Get()->LoadAsset(*stub);

        GetEditorState()->InspectObject(stub->mAsset);
    }

    if (stub && stub->mType == Scene::GetStaticType())
    {
        if (ImGui::Selectable("Open Scene"))
        {
            if (stub->mAsset == nullptr)
                AssetManager::Get()->LoadAsset(*stub);

            Scene* scene = stub->mAsset ? stub->mAsset->As<Scene>() : nullptr;
            if (scene != nullptr)
            {
                GetEditorState()->OpenEditScene(scene);
            }
            else
            {
                LogError("Failed to load scene asset?");
            }
        }


        if (ImGui::Selectable("Set Default Game Scene"))
        {
            GetMutableEngineConfig()->mDefaultScene = stub->mName;
            WriteEngineConfig();
        }

        if (ImGui::Selectable("Set Default Editor Scene"))
        {
            GetMutableEngineConfig()->mDefaultEditorScene = stub->mName;
            WriteEngineConfig();
        }
    }

    if (canInstantiate && ImGui::Selectable("Instantiate"))
    {
        if (stub->mAsset == nullptr)
            assMan->LoadAsset(*stub);

        if (stub->mAsset != nullptr)
        {
            Asset* srcAsset = stub->mAsset;
            glm::vec3 spawnPos = EditorGetFocusPosition();
            Node* selNode = GetEditorState()->GetSelectedNode();

            if (stub->mType == Scene::GetStaticType())
                actMan->SpawnBasicNode(BASIC_SCENE, selNode, srcAsset, selNode == nullptr, spawnPos);
            else if (stub->mType == SoundWave::GetStaticType())
                actMan->SpawnBasicNode(BASIC_AUDIO, selNode, srcAsset, selNode == nullptr, spawnPos);
            else if (stub->mType == StaticMesh::GetStaticType())
                actMan->SpawnBasicNode(BASIC_STATIC_MESH, selNode, srcAsset, selNode == nullptr, spawnPos);
            else if (stub->mType == SkeletalMesh::GetStaticType())
                actMan->SpawnBasicNode(BASIC_SKELETAL_MESH, selNode, srcAsset, selNode == nullptr, spawnPos);
            else if (stub->mType == ParticleSystem::GetStaticType())
                actMan->SpawnBasicNode(BASIC_PARTICLE, selNode, srcAsset, selNode == nullptr, spawnPos);
        }
    }


    if (!engineFile && (stub || dir))
    {
        if (stub && ImGui::Selectable("Save"))
        {
            if (stub->mAsset == nullptr)
                AssetManager::Get()->LoadAsset(*stub);

            assMan->SaveAsset(*stub);
        }
        if (ImGui::Selectable("Rename", false, ImGuiSelectableFlags_DontClosePopups))
        {
            ImGui::OpenPopup("Rename Asset");
            strncpy(sPopupInputBuffer, stub ? stub->mName.c_str() : dir->mName.c_str(), kPopupInputBufferSize);
            setTextInputFocus = true;
        }
        if (ImGui::Selectable("Delete"))
        {
            if (stub)
            {
                actMan->DeleteAsset(stub);
            }
            else if (dir)
            {
                actMan->DeleteAssetDir(dir);
                GetEditorState()->ClearAssetDirHistory();
            }

        }

        if (stub && ImGui::Selectable("Duplicate"))
        {
            GetEditorState()->DuplicateAsset(stub);
        }
    }

    if (curDir && !curDir->mEngineDir)
    {
        if (ImGui::Selectable("Import Asset"))
        {
            actMan->ImportAsset();
        }

        if (ImGui::Selectable("Import Scene"))
        {
            actMan->BeginImportScene();
        }

        if (ImGui::BeginMenu("Create Asset"))
        {
            bool showPopup = false;

            if (ImGui::Selectable("Material (Lite)", false, ImGuiSelectableFlags_DontClosePopups))
            {
                sNewAssetType = MaterialLite::GetStaticType();
                showPopup = true;
            }
            if (ImGui::Selectable("Material (Base)", false, ImGuiSelectableFlags_DontClosePopups))
            {
                sNewAssetType = MaterialBase::GetStaticType();
                showPopup = true;
            }
            if (ImGui::Selectable("Material (Instance)", false, ImGuiSelectableFlags_DontClosePopups))
            {
                sNewAssetType = MaterialInstance::GetStaticType();
                showPopup = true;
            }
            if (ImGui::Selectable("Particle System", false, ImGuiSelectableFlags_DontClosePopups))
            {
                sNewAssetType = ParticleSystem::GetStaticType();
                showPopup = true;
            }
            if (ImGui::Selectable("Scene", false, ImGuiSelectableFlags_DontClosePopups))
            {
                sNewAssetType = Scene::GetStaticType();
                showPopup = true;
            }

            ImGui::EndMenu();

            if (showPopup)
            {
                ImGui::OpenPopup("New Asset Name");
                strncpy(sPopupInputBuffer, "", kPopupInputBufferSize);
                setTextInputFocus = true;
            }
        }

        if (ImGui::Selectable("New Folder", false, ImGuiSelectableFlags_DontClosePopups))
        {
            ImGui::OpenPopup("New Folder");
            strncpy(sPopupInputBuffer, "", kPopupInputBufferSize);
            setTextInputFocus = true;
        }

        if (ImGui::Selectable("Capture Active Scene", false, ImGuiSelectableFlags_DontClosePopups))
        {
            AssetStub* saveStub = nullptr;
            if (stub && stub->mType == Scene::GetStaticType())
            {
                saveStub = stub;
            }

            if (saveStub == nullptr)
            {
                ImGui::OpenPopup("Capture To New Scene");
                strncpy(sPopupInputBuffer, "", kPopupInputBufferSize);
                setTextInputFocus = true;
            }
            else
            {
                GetEditorState()->CaptureAndSaveScene(saveStub, nullptr);
                closeContextPopup = true;
            }
        }

        const std::vector<Node*>& selNodes = GetEditorState()->GetSelectedNodes();
        if (selNodes.size() == 1 && ImGui::Selectable("Capture Selected Node"))
        {
            AssetStub* saveStub = nullptr;
            if (stub && stub->mType == Scene::GetStaticType())
            {
                saveStub = stub;
            }

            GetEditorState()->CaptureAndSaveScene(saveStub, selNodes[0]);
        }
    }

    if (ImGui::BeginPopup("Rename Asset"))
    {
        if (setTextInputFocus)
        {
            ImGui::SetKeyboardFocusHere();
        }

        if (ImGui::InputText("Name", sPopupInputBuffer, kPopupInputBufferSize, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            if (stub)
            {
                Asset* asset = AssetManager::Get()->LoadAsset(*stub);
                AssetManager::Get()->RenameAsset(asset, sPopupInputBuffer);
                AssetManager::Get()->SaveAsset(*stub);
            }
            else if (dir)
            {
                AssetManager::Get()->RenameDirectory(dir, sPopupInputBuffer);
            }

            ImGui::CloseCurrentPopup();
            closeContextPopup = true;
        }

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("New Folder"))
    {
        if (setTextInputFocus)
        {
            ImGui::SetKeyboardFocusHere();
        }

        if (ImGui::InputText("Folder Name", sPopupInputBuffer, kPopupInputBufferSize, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            const std::string folderName = sPopupInputBuffer;

            if (folderName != "")
            {
                if (SYS_CreateDirectory((curDir->mPath + folderName).c_str()))
                {
                    curDir->CreateSubdirectory(folderName);
                }
                else
                {
                    LogError("Failed to create folder");
                }
            }

            ImGui::CloseCurrentPopup();
            closeContextPopup = true;
        }

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Capture To New Scene"))
    {
        if (setTextInputFocus)
        {
            ImGui::SetKeyboardFocusHere();
        }

        if (ImGui::InputText("Name", sPopupInputBuffer, kPopupInputBufferSize, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            std::string sceneName = sPopupInputBuffer;

            if (sceneName == "")
            {
                sceneName = "SC_Scene";
            }

            AssetStub* saveStub = EditorAddUniqueAsset(sceneName.c_str(), curDir, Scene::GetStaticType(), true);
            GetEditorState()->CaptureAndSaveScene(saveStub, nullptr);

            ImGui::CloseCurrentPopup();
            closeContextPopup = true;
        }

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("New Asset Name"))
    {
        if (setTextInputFocus)
        {
            ImGui::SetKeyboardFocusHere();
        }

        if (ImGui::InputText("Name", sPopupInputBuffer, kPopupInputBufferSize, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            std::string assetName = sPopupInputBuffer;

            if (assetName == "")
            {
                if (sNewAssetType == MaterialLite::GetStaticType())
                    assetName = "M_Material";
                else if (sNewAssetType == MaterialBase::GetStaticType())
                    assetName = "MB_Material";
                else if (sNewAssetType == MaterialInstance::GetStaticType())
                    assetName = "MI_Material";
                else if (sNewAssetType == ParticleSystem::GetStaticType())
                    assetName = "P_Particle";
                else if (sNewAssetType == Scene::GetStaticType())
                    assetName = "SC_Scene";
            }

            if (assetName != "" && sNewAssetType != INVALID_TYPE_ID)
            {
                CreateNewAsset(sNewAssetType, assetName.c_str());
            }

            ImGui::CloseCurrentPopup();
            closeContextPopup = true;
        }

        ImGui::EndPopup();
    }

    if (closeContextPopup)
    {
        ImGui::CloseCurrentPopup();
    }
}

static void DrawAssetBrowser(bool showFilter, bool interactive)
{
    AssetDir* currentDir = GetEditorState()->GetAssetDirectory();

    static std::string sUpperAssetName;
    std::string& filterStr = GetEditorState()->mAssetFilterStr;
    std::vector<AssetStub*>& filteredStubs = GetEditorState()->mFilteredAssetStubs;

    if (showFilter && ImGui::InputText("Filter", &filterStr, ImGuiInputTextFlags_EnterReturnsTrue))
    {
        filteredStubs.clear();

        if (filterStr != "")
        {
            // Convert filter string to all upper case
            std::string filterStrUpper = filterStr;
            for (uint32_t c = 0; c < filterStrUpper.size(); ++c)
            {
                filterStrUpper[c] = toupper(filterStrUpper[c]);
            }

            // Iterate through all matching asset names
            const auto& assetMap = AssetManager::Get()->GetAssetMap();
            for (auto element : assetMap)
            {
                // Get the upper case version of asset name
                sUpperAssetName = element.second->mName;
                for (uint32_t c = 0; c < sUpperAssetName.size(); ++c)
                {
                    sUpperAssetName[c] = toupper(sUpperAssetName[c]);
                }

                if (sUpperAssetName.find(filterStrUpper) != std::string::npos)
                {
                    filteredStubs.push_back(element.second);
                }
            }
        }
    }

    if (filterStr == "")
    {
        filteredStubs.clear();
    }

    if (currentDir != nullptr)
    {
        if (!showFilter || filterStr == "")
        {
            // Directories first
            ImGui::PushStyleColor(ImGuiCol_Header, kBgInactive);
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, kBgHover);
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, kBgInactive);

            // Parent Dir (..)
            if (currentDir->mParentDir != nullptr)
            {
                if (ImGui::Selectable("..", true))
                {
                    GetEditorState()->SetAssetDirectory(currentDir->mParentDir, true);
                }
            }

            // Child Dirs
            for (uint32_t i = 0; i < currentDir->mChildDirs.size(); ++i)
            {
                AssetDir* childDir = currentDir->mChildDirs[i];

                if (ImGui::Selectable(childDir->mName.c_str(), true))
                {
                    GetEditorState()->SetAssetDirectory(childDir, true);
                }

                if (ImGui::BeginPopupContextItem())
                {
                    DrawAssetsContextPopup(nullptr, childDir);
                    ImGui::EndPopup();
                }
            }

            ImGui::PopStyleColor(3); // Pop Directory Colors
        }

        std::vector<AssetStub*>* stubs = &(currentDir->mAssetStubs);
        if (showFilter && filterStr != "")
        {
            stubs = &filteredStubs;
        }

        // Assets
        AssetStub* selStub = GetEditorState()->GetSelectedAssetStub();
        for (uint32_t i = 0; i < stubs->size(); ++i)
        {
            AssetStub* stub = (*stubs)[i];

            bool isSelectedStub = (stub == selStub);
            if (isSelectedStub)
            {
                ImGui::PushStyleColor(ImGuiCol_Header, kSelectedColor);
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, kSelectedColor);
                ImGui::PushStyleColor(ImGuiCol_HeaderActive, kSelectedColor);
            }

            glm::vec4 assetColor = AssetManager::Get()->GetEditorAssetColor(stub->mType);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(assetColor.r, assetColor.g, assetColor.b, assetColor.a));

            std::string assetDispText = stub->mName;
            if (stub && stub->mAsset && stub->mAsset->GetDirtyFlag())
            {
                assetDispText = "*" + assetDispText;
            }

            if (ImGui::Selectable(assetDispText.c_str(), isSelectedStub, ImGuiSelectableFlags_AllowDoubleClick))
            {
                if (selStub != stub)
                {
                    GetEditorState()->SetSelectedAssetStub(stub);
                }
                else if (!IsControlDown())
                {
                    GetEditorState()->SetSelectedAssetStub(nullptr);
                }

                if (IsControlDown() &&
                    stub != nullptr &&
                    stub->mAsset != nullptr)
                {
                    GetEditorState()->InspectObject(stub->mAsset);
                }

                if (ImGui::IsMouseDoubleClicked(0))
                {
                    if (stub->mAsset == nullptr)
                        AssetManager::Get()->LoadAsset(*stub);

                    if (stub->mType == Scene::GetStaticType())
                    {
                        Scene* scene = stub->mAsset ? stub->mAsset->As<Scene>() : nullptr;
                        if (scene)
                            GetEditorState()->OpenEditScene(scene);
                    }
                    else
                    {
                        if (stub->mAsset)
                            GetEditorState()->InspectObject(stub->mAsset);
                    }
                }
            }

            if (GetEditorState()->mTrackSelectedAsset &&
                stub == GetEditorState()->mSelectedAssetStub)
            {
                ImGui::SetScrollHereY(0.5f);
                GetEditorState()->mTrackSelectedAsset = false;
            }

            ImGui::PopStyleColor(); // Pop asset color

            if (isSelectedStub)
            {
                ImGui::PopStyleColor(3);
            }

            if (interactive && ImGui::BeginPopupContextItem())
            {
                DrawAssetsContextPopup(stub, nullptr);
                ImGui::EndPopup();
            }
        }
    }

    // If no popup is open and we aren't inputting text...
    if (!ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup) &&
        ImGui::IsWindowHovered() &&
        !ImGui::GetIO().WantTextInput &&
        interactive)
    {
        const bool ctrlDown = IsControlDown();
        const bool shiftDown = IsShiftDown();
        const bool altDown = IsAltDown();

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
        {
            ImGui::OpenPopup("Null Context");
        }

        if (filterStr != "")
        {
            if (IsMouseButtonJustDown(MOUSE_X1))
            {
                filterStr = "";
                filteredStubs.clear();
            }
        }
        else
        {
            if (IsMouseButtonJustDown(MOUSE_X1))
            {
                GetEditorState()->RegressDirPast();
            }
            else if (IsMouseButtonJustDown(MOUSE_X2))
            {
                GetEditorState()->ProgressDirFuture();
            }
        }

        if (currentDir != nullptr)
        {
            if (ctrlDown && IsKeyJustDown(KEY_N))
            {
                CreateNewAsset(Scene::GetStaticType(), "SC_Scene");
            }

            if (ctrlDown && IsKeyJustDown(KEY_M))
            {
                CreateNewAsset(MaterialLite::GetStaticType(), "M_Material");
            }

            if (ctrlDown && IsKeyJustDown(KEY_P))
            {
                CreateNewAsset(ParticleSystem::GetStaticType(), "P_Particle");
            }
        }

        if (ctrlDown && IsKeyJustDown(KEY_D))
        {
            AssetStub* srcStub = GetEditorState()->GetSelectedAssetStub();

            if (srcStub != nullptr)
            {
                GetEditorState()->DuplicateAsset(srcStub);
            }
        }

        if (IsKeyJustDown(KEY_DELETE))
        {
            AssetStub* selStub = GetEditorState()->GetSelectedAssetStub();

            if (selStub != nullptr)
            {
                ActionManager::Get()->DeleteAsset(selStub);
            }
        }
    }

    if (interactive && ImGui::BeginPopup("Null Context"))
    {
        DrawAssetsContextPopup(nullptr, nullptr);
        ImGui::EndPopup();
    }
}

static void DrawAssetsPanel()
{
    const float halfHeight = ImGui::GetIO().DisplaySize.y / 2.0f;

    ImGui::SetNextWindowPos(ImVec2(0.0f, halfHeight));
    ImGui::SetNextWindowSize(ImVec2(kSidePaneWidth, halfHeight));

    ImGui::Begin("Assets", nullptr, kPaneWindowFlags);

    DrawAssetBrowser(true, true);

    ImGui::End();
}

static void DrawMaterialShaderParams(Material* mat)
{
    std::vector<ShaderParameter>& params = mat->GetParameters();

    if (params.size() == 0)
        return;

    bool catOpen = ImGui::CollapsingHeader("Shader Parameters", ImGuiTreeNodeFlags_DefaultOpen);

    if (!catOpen)
        return;

    ImGui::PushID("ShaderParams");

    for (uint32_t i = 0; i < params.size(); ++i)
    {
        ShaderParameter& param = params[i];

        // This is a hidden parameter needed for ensuring that the material descriptor set is compiled.
        if (param.mName == "ForceMaterialSet")
        {
            continue;
        }

        ImGui::Text(param.mName.c_str());

        ImGui::PushID(i);

        if (param.mType == ShaderParameterType::Scalar)
        {
            ImGui::DragFloat("", &param.mFloatValue[0]);
        }
        else if (param.mType == ShaderParameterType::Vector)
        {
            ImGui::OctDragScalarN("", ImGuiDataType_Float, &param.mFloatValue[0], 3, 1.0f, nullptr, nullptr, "%.2f", 0);
        }
        else if (param.mType == ShaderParameterType::Texture)
        {
            Property prop(DatumType::Asset, "ShaderParam", nullptr, &param.mTextureValue, 1, nullptr, (int32_t)Texture::GetStaticType());
            DrawAssetProperty(prop, 0, nullptr, PropertyOwnerType::Count);
        }

        ImGui::PopID();
    }

    ImGui::PopID();
}

static void DrawInstancedMeshExtra(InstancedMesh3D* instMesh)
{
    static int32_t sActiveInstance = 0;

    int32_t selInstance = GetEditorState()->GetSelectedInstance();
    if (selInstance != -1)
    {
        sActiveInstance = selInstance;
    }

    if (ImGui::CollapsingHeader("Instance Data", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::PushID(0);

        int32_t numInstances = (int32_t)instMesh->GetNumInstances();
        char instCountStr[32];
        snprintf(instCountStr, 32, "Instances: %d", numInstances);
        ImGui::Text(instCountStr);

        if (ImGui::Button("-"))
        {
            if (instMesh->GetNumInstances() > 0)
            {
                sActiveInstance = glm::clamp<int32_t>(sActiveInstance, 0, numInstances - 1);
                std::vector<MeshInstanceData> newInstData = instMesh->GetInstanceData();
                newInstData.erase(newInstData.begin() + sActiveInstance);
                ActionManager::Get()->EXE_SetInstanceData(instMesh, -1, newInstData);

                sActiveInstance = int32_t(instMesh->GetNumInstances()) - 1;
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("+"))
        {
            // Not efficient, but lets use the same EXE_SetInstanceData() action
            std::vector<MeshInstanceData> newInstData = instMesh->GetInstanceData();
            newInstData.push_back(MeshInstanceData());
            ActionManager::Get()->EXE_SetInstanceData(instMesh, -1, newInstData);

            sActiveInstance = int32_t(instMesh->GetNumInstances()) - 1;
        }

        numInstances = (int32_t)instMesh->GetNumInstances();

        if (numInstances > 0)
        {
            sActiveInstance = glm::clamp<int32_t>(sActiveInstance, 0, numInstances - 1);
            if (ImGui::SliderInt("Active Instance", &sActiveInstance, 0, numInstances - 1))
            {
                if (selInstance != -1)
                {
                    GetEditorState()->SetSelectedInstance(sActiveInstance);
                }
            }
        }

        if (sActiveInstance >= 0 &&
            sActiveInstance < numInstances)
        {
            MeshInstanceData instData = instMesh->GetInstanceData(sActiveInstance);
            bool positionChanged = false;
            bool rotationChanged = false;
            bool scaleChanged = false;

            ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.85f);
            //ImGui::DragFloat3("", &propVal[0], 1.0f, 0.0f, 0.0f, "%.2f");
            float vMin = 0.0f;
            float vMax = 0.0f;

            static MeshInstanceData sOrigVal;
            MeshInstanceData preVal = instData;
            bool itemActivated = false;
            bool itemDeactivated = false;
            
            ImGui::Text("Position");
            positionChanged = ImGui::OctDragScalarN("Position", ImGuiDataType_Float, &instData.mPosition, 3, 1.0f, &vMin, &vMax, "%.2f", 0);
            itemActivated = itemActivated || ImGui::IsItemActivated();
            itemDeactivated = itemDeactivated || ImGui::IsItemDeactivatedAfterEdit();

            ImGui::Text("Rotation");
            rotationChanged = ImGui::OctDragScalarN("Rotation", ImGuiDataType_Float, &instData.mRotation, 3, 1.0f, &vMin, &vMax, "%.2f", 0);
            itemActivated = itemActivated || ImGui::IsItemActivated();
            itemDeactivated = itemDeactivated || ImGui::IsItemDeactivatedAfterEdit();

            ImGui::Text("Scale");
            scaleChanged = ImGui::OctDragScalarN("Scale", ImGuiDataType_Float, &instData.mScale, 3, 1.0f, &vMin, &vMax, "%.2f", 0);
            itemActivated = itemActivated || ImGui::IsItemActivated();
            itemDeactivated = itemDeactivated || ImGui::IsItemDeactivatedAfterEdit();

            if (itemActivated)
            {
                sOrigVal = preVal;
            }

            if (itemDeactivated && (memcmp(&sOrigVal, &instData, sizeof(MeshInstanceData)) != 0))
            {
                instMesh->SetInstanceData(sActiveInstance, sOrigVal);
                ActionManager::Get()->EXE_SetInstanceData(instMesh, sActiveInstance, { instData });
            }
            else if (positionChanged ||
                rotationChanged ||
                scaleChanged)
            {
                instMesh->SetInstanceData(sActiveInstance, instData);
            }

            ImGui::PopItemWidth();
        }

        ImGui::PopID();
    }
}


static void DrawPropertiesPanel()
{
    const float dispWidth = ImGui::GetIO().DisplaySize.x;
    const float dispHeight = ImGui::GetIO().DisplaySize.y;

    ImGui::SetNextWindowPos(ImVec2(dispWidth - kSidePaneWidth, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(kSidePaneWidth, dispHeight));

    // The locked BG color will be delayed by a frame.
    bool lockedProperties = sObjectTabOpen && GetEditorState()->IsInspectLocked();
    if (lockedProperties)
    {
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.4f, 0.0f, 0.0f, 1.0f));
    }

    sObjectTabOpen = false;

    ImGui::Begin("Properties", nullptr, kPaneWindowFlags);

    if (ImGui::BeginTabBar("PropertyModeTabs"))
    {
        if (ImGui::BeginTabItem("Object"))
        {
            sObjectTabOpen = true;
            Object* obj = GetEditorState()->GetInspectedObject();

            if (obj != nullptr)
            {
                bool inspectLocked = GetEditorState()->IsInspectLocked();

                if (obj->As<Asset>())
                {
                    Asset* asset = obj->As<Asset>();

                    if (ImGui::Button("<<"))
                    {
                        GetEditorState()->BrowseToAsset(asset->GetName());
                    }

                    ImGui::SameLine();

                    if (ImGui::Button("Save"))
                    {
                        AssetManager::Get()->SaveAsset(asset->GetName());
                    }

                    if (asset->GetDirtyFlag())
                    {
                        ImGui::SameLine();
                        ImGui::Text("*");
                    }
                }

                // Display object name (e.g. StaticMesh3D)
                std::string objName = obj->RuntimeName();
                ImVec2 sz = ImVec2(-FLT_MIN, 0.0f);
                ImVec4 headerColor = kSelectedColor;
                headerColor.w = 0.0f;
                ImVec4 headerTextColor = ImVec4(0.18f, 0.75f, 0.70f, 1.00f);
                ImGui::PushStyleColor(ImGuiCol_Text, headerTextColor);
                ImGui::PushStyleColor(ImGuiCol_Button, headerColor);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, headerColor);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, headerColor);
                ImGui::Button(objName.c_str(), sz);
                ImGui::PopStyleColor(4);

                Texture* texObj = obj->As<Texture>();
                if (texObj != nullptr &&
                    texObj->GetResource()->mImage != nullptr)
                {
                    // Dealloc prev tex descriptor
                    if (sPrevInspectTexture != texObj)
                    {
                        DeviceWaitIdle();

                        if (sInspectTexId != 0)
                        {
                            ImGui_ImplVulkan_RemoveTexture((VkDescriptorSet)sInspectTexId);
                            sInspectTexId = 0;
                        }

                        sInspectTexId = (VkDescriptorSet)ImGui_ImplVulkan_AddTexture(
                            texObj->GetResource()->mImage->GetSampler(),
                            texObj->GetResource()->mImage->GetView(),
                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

                        sPrevInspectTexture = texObj;
                    }

                    if (sInspectTexId != 0)
                    {
                        ImGui::Image(sInspectTexId, ImVec2(128, 128), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1,1,1,1), ImVec4(0.5f, 0.2f, 0.2f, 1.0f));
                    }

                    ImGui::Text("%d x %d", texObj->GetWidth(), texObj->GetHeight());
                    ImGui::NewLine();
                }

                std::vector<Property> props;
                obj->GatherProperties(props);

                DrawPropertyList(obj, props);

                // Custom imgui drawing
                if (obj->As<Material>())
                {
                    Material* mat = obj->As<Material>();
                    DrawMaterialShaderParams(mat);
                }
                else if (obj->As<InstancedMesh3D>())
                {
                    InstancedMesh3D* instMesh = obj->As<InstancedMesh3D>();
                    DrawInstancedMeshExtra(instMesh);
                }
                else if (obj->As<StaticMesh>())
                {
                    StaticMesh* staticMesh = obj->As<StaticMesh>();
                    bool hasVertColor = staticMesh->HasVertexColor();
                    ImGui::Text("Vertex Color: %s", hasVertColor ? "Yes" : "No");
                }
                else if (obj->As<SkeletalMesh>())
                {
                    SkeletalMesh* skelMesh = obj->As<SkeletalMesh>();
                    ImGui::Text("Animations");
                    ImGui::Indent();
                    const std::vector<Animation>& animations = skelMesh->GetAnimations();
                    for (uint32_t a = 0; a < animations.size(); ++a)
                    {
                        ImGui::Text(animations[a].mName.c_str());
                    }
                    ImGui::Unindent();
                }
                else if (obj->As<SoundWave>())
                {
                    SoundWave* soundWave = obj->As<SoundWave>();
                    ImGui::Text("Num Channels: %d", soundWave->GetNumChannels());
                    ImGui::Text("Bits Per Sample: %d", soundWave->GetBitsPerSample());
                    ImGui::Text("Sample Rate: %d", soundWave->GetSampleRate());
                }
            }

            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Scene"))
        {
            EditScene* editScene = GetEditorState()->GetEditScene();
            Scene* scene = editScene ? editScene->mSceneAsset.Get<Scene>() : nullptr;

            if (scene != nullptr)
            {
                std::vector<Property> sceneProps;
                scene->GatherProperties(sceneProps);

                DrawPropertyList(scene, sceneProps);
            }

            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Global"))
        {
            std::vector<Property> globalProps;
            GatherGlobalProperties(globalProps);

            DrawPropertyList(nullptr, globalProps);

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    if (!ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup) &&
        ImGui::IsWindowHovered() &&
        !ImGui::GetIO().WantTextInput)
    {
        bool ctrlDown = IsControlDown();

        // Hotkey for toggling lock.
        if (IsKeyJustDown(KEY_L))
        {
            GetEditorState()->LockInspect(!GetEditorState()->IsInspectLocked());
        }

        // Navigate inspection history.
        if (IsMouseButtonJustDown(MOUSE_X1))
        {
            GetEditorState()->RegressInspectPast();
        }
        else if (IsMouseButtonJustDown(MOUSE_X2))
        {
            GetEditorState()->ProgressInspectFuture();
        }
    }

    ImGui::End();

    if (lockedProperties)
    {
        ImGui::PopStyleColor();
    }
}

static void DrawViewportPanel()
{
    Renderer* renderer = Renderer::Get();
    ActionManager* am = ActionManager::Get();

    float viewportBarX = GetEditorState()->mShowLeftPane ? kSidePaneWidth : 0.0f;
    float viewportBarWidth = ImGui::GetIO().DisplaySize.x;
    if (GetEditorState()->mShowLeftPane)
        viewportBarWidth -= kSidePaneWidth;
    if (GetEditorState()->mShowRightPane)
        viewportBarWidth -= kSidePaneWidth;

    const ImGuiWindowFlags kViewportWindowFlags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoSavedSettings; /* | ImGuiWindowFlags_AlwaysAutoResize*/ /*| ImGuiWindowFlags_NoBackground*/;

    ImGui::SetNextWindowPos(ImVec2(viewportBarX, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(viewportBarWidth, kViewportBarHeight));

    ImGui::Begin("Viewport", nullptr, kViewportWindowFlags);

    // (1) Draw File / View / World / Play buttons below
    if (ImGui::Button("File"))
        ImGui::OpenPopup("FilePopup");

    ImGui::SameLine();
    if (ImGui::Button("Edit"))
        ImGui::OpenPopup("EditPopup");

    ImGui::SameLine();
    if (ImGui::Button("View"))
        ImGui::OpenPopup("ViewPopup");

    ImGui::SameLine();
    if (ImGui::Button("World"))
        ImGui::OpenPopup("WorldPopup");

    ImGui::SameLine();
    if (ImGui::Button("Extra"))
        ImGui::OpenPopup("ExtraPopup");

    ImGui::SameLine();
    bool inPie = IsPlayingInEditor();
    if (ImGui::Button(inPie ? "Stop" : "Play"))
    {
        if (inPie)
        {
            GetEditorState()->EndPlayInEditor();
        }
        else
        {
            GetEditorState()->BeginPlayInEditor();
        }
    }

    ImGui::SameLine();
    int curMode = (int)GetEditorState()->mMode;
    PaintMode paintMode = (GetEditorState()->mMode == EditorMode::Scene3D) ? GetEditorState()->mPaintMode : PaintMode::None;

    if (paintMode != PaintMode::None && paintMode != PaintMode::Count)
    {
        curMode = int(EditorMode::Count) + int(paintMode) - 1;
    }

    const char* modeStrings[] = { "Scene", "2D", "3D", "Paint Colors", "Paint Instances"};
    ImGui::SetNextItemWidth(70);
    ImGui::Combo("##EditorMode", &curMode, modeStrings, 5);

    if (curMode == 3)
    {
        curMode = (int)EditorMode::Scene3D;
        paintMode = PaintMode::Color;
    }
    else if (curMode == 4)
    {
        curMode = (int)EditorMode::Scene3D;
        paintMode = PaintMode::Instance;
    }
    else
    {
        paintMode = PaintMode::None;
    }

    GetEditorState()->SetEditorMode((EditorMode)curMode);
    GetEditorState()->SetPaintMode(paintMode);

    // Gizmo Operation Buttons (Translate/Rotate/Scale)
    ImGui::SameLine();
    ImGui::Separator();
    ImGui::SameLine();

    EditorState* edState = GetEditorState();
    ImGuizmo::OPERATION& gizmoOp = edState->mGizmoOperation;
    ImGuizmo::MODE& gizmoMode = edState->mGizmoMode;

    // Translate button
    bool isTranslate = (gizmoOp == ImGuizmo::TRANSLATE);
    if (isTranslate) ImGui::PushStyleColor(ImGuiCol_Button, kSelectedColor);
    if (ImGui::Button("T##Translate"))
        gizmoOp = ImGuizmo::TRANSLATE;
    if (isTranslate) ImGui::PopStyleColor();
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Translate (Space+G)");

    // Rotate button
    ImGui::SameLine();
    bool isRotate = (gizmoOp == ImGuizmo::ROTATE);
    if (isRotate) ImGui::PushStyleColor(ImGuiCol_Button, kSelectedColor);
    if (ImGui::Button("R##Rotate"))
        gizmoOp = ImGuizmo::ROTATE;
    if (isRotate) ImGui::PopStyleColor();
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Rotate (Space+R)");

    // Scale button
    ImGui::SameLine();
    bool isScale = (gizmoOp == ImGuizmo::SCALE);
    if (isScale) ImGui::PushStyleColor(ImGuiCol_Button, kSelectedColor);
    if (ImGui::Button("S##Scale"))
        gizmoOp = ImGuizmo::SCALE;
    if (isScale) ImGui::PopStyleColor();
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Scale (Space+S)");

    // Local/World mode toggle
    ImGui::SameLine();
    ImGui::Separator();
    ImGui::SameLine();

    bool isLocal = (gizmoMode == ImGuizmo::LOCAL);
    if (isLocal) ImGui::PushStyleColor(ImGuiCol_Button, kToggledColor);
    if (ImGui::Button(isLocal ? "Local" : "World"))
        gizmoMode = isLocal ? ImGuizmo::WORLD : ImGuizmo::LOCAL;
    if (isLocal) ImGui::PopStyleColor();
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Toggle Local/World (Ctrl+T)");

    bool openSaveSceneAsModal = false;

    if (ImGui::BeginPopup("FilePopup"))
    {
        EditScene* editScene = GetEditorState()->GetEditScene();

        if (ImGui::Selectable("Open Project"))
            am->OpenProject();
        if (ImGui::BeginMenu("Open Recent Project"))
        {
            const std::vector<std::string>& recentProjects = GetEditorState()->mRecentProjects;
            for (uint32_t i = 0; i < recentProjects.size(); ++i)
            {
                if (recentProjects[i] != "" &&
                    ImGui::MenuItem(recentProjects[i].c_str()))
                {
                    am->OpenProject(recentProjects[i].c_str());
                }
            }
            ImGui::EndMenu();
        }
        if (ImGui::Selectable("New Project"))
            am->CreateNewProject();
        if (ImGui::Selectable("New C++ Project"))
            am->CreateNewProject(nullptr, true);
        if (ImGui::Selectable("New Scene"))
            GetEditorState()->OpenEditScene(nullptr);
        if (editScene && ImGui::Selectable("Save Scene"))
        {
            Scene* scene = editScene->mSceneAsset.Get<Scene>();
            AssetStub* sceneStub = scene ? AssetManager::Get()->GetAssetStub(scene->GetName()) : nullptr;
            if (sceneStub != nullptr)
            {
                GetEditorState()->CaptureAndSaveScene(sceneStub, nullptr);
            }
            else
            {
                // Need to request name and create asset.
                openSaveSceneAsModal = true;
                strncpy(sPopupInputBuffer, "", kPopupInputBufferSize);
            }
        }
        if (editScene && ImGui::Selectable("Save Scene As..."))
        {
            // Need to request name and create asset.
            openSaveSceneAsModal = true;
            strncpy(sPopupInputBuffer, "", kPopupInputBufferSize);
        }
        if (ImGui::Selectable("Import Asset"))
            am->ImportAsset();
        if (ImGui::Selectable("Import Scene"))
            am->BeginImportScene();
        if (ImGui::Selectable("Run Script"))
            am->RunScript();
        if (ImGui::Selectable("Recapture All Scenes"))
            am->RecaptureAndSaveAllScenes();
        if (ImGui::Selectable("Resave All Assets"))
            am->ResaveAllAssets();
        if (ImGui::Selectable("Reload All Scripts"))
            ReloadAllScripts();
        
        // Script Hot-Reload toggle
        bool hotReloadEnabled = IsScriptHotReloadEnabled();
        std::string hotReloadText = hotReloadEnabled ? "Disable Script Hot-Reload" : "Enable Script Hot-Reload";
        if (ImGui::Selectable(hotReloadText.c_str()))
        {
            SetScriptHotReloadEnabled(!hotReloadEnabled);
            WriteEngineConfig();
        }
        
        if (ImGui::Selectable("Write Config"))
            WriteEngineConfig();
        //if (ImGui::Selectable("Import Scene"))
        //    YYY;
        if (ImGui::BeginMenu("Package Project"))
        {
            DrawPackageMenu();
            ImGui::EndMenu();
        }

        ImGui::EndPopup();
    }

    if (GetEditorState()->mRequestSaveSceneAs)
    {
        openSaveSceneAsModal = true;
    }

    if (openSaveSceneAsModal)
    {
        ImGui::OpenPopup("Save Scene As");
    }

    // Center the "Save Scene As" modal
    if (ImGui::IsPopupOpen("Save Scene As"))
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    }

    if (ImGui::BeginPopupModal("Save Scene As", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
    {
        GetEditorState()->mRequestSaveSceneAs = false;

        AssetDir* curDir = GetEditorState()->GetAssetDirectory();

        if (curDir == nullptr || curDir->mEngineDir || curDir == AssetManager::Get()->GetRootDirectory())
        {
            GetEditorState()->SetAssetDirectory(AssetManager::Get()->FindProjectDirectory(), true);
        }

        {
            ImGuiWindowFlags childFlags = ImGuiWindowFlags_None;
            ImGui::BeginChild("Dir Browser", ImVec2(250, 250), true, childFlags);
            DrawAssetBrowser(false, false);

            ImGui::EndChild();
        }


        if (curDir != nullptr && !curDir->mEngineDir)
        {
            AssetDir* dir = curDir;
            std::string dirString = dir->mName + "/";
            dir = dir->mParentDir;

            while (dir != nullptr)
            {
                if (dir->mParentDir == nullptr)
                    dirString = "/" + dirString;
                else
                    dirString = dir->mName + "/" + dirString;

                dir = dir->mParentDir;
            }

            ImGui::Text("Save scene to directory...");
            ImGui::Indent(10);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 1.0f, 0.7f, 1.0f));
            ImGui::Text(dirString.c_str());
            ImGui::PopStyleColor();
            ImGui::Unindent(10);

            bool save = false;
            if (ImGui::InputText("Scene Name", sPopupInputBuffer, kPopupInputBufferSize, ImGuiInputTextFlags_EnterReturnsTrue))
            {
                save = true;
            }

            if (ImGui::Button("Save"))
            {
                save = true;
            }

            ImGui::SameLine();
            if (ImGui::Button("Cancel"))
            {
                ImGui::CloseCurrentPopup();
            }

            if (save)
            {
                std::string sceneName = sPopupInputBuffer;

                if (sceneName == "")
                {
                    sceneName = "SC_Scene";
                }

                AssetStub* stub = EditorAddUniqueAsset(sceneName.c_str(), curDir, Scene::GetStaticType(), true);
                OCT_ASSERT(stub != nullptr);

                GetEditorState()->CaptureAndSaveScene(stub, nullptr);

                ImGui::CloseCurrentPopup();
            }
        }
        else
        {
            ImGui::Text("Invalid asset directory. Please navigate to a project directory.");
            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::EndPopup();
    }

    if (!ImGui::IsPopupOpen("Import Scene") && GetEditorState()->mPendingSceneImportPath != "")
    {
        std::string sceneName = GetFileNameFromPath(GetEditorState()->mPendingSceneImportPath);
        sSceneImportOptions.mSceneName = sceneName;
        ImGui::OpenPopup("Import Scene");
    }

    if (ImGui::IsPopupOpen("Import Scene"))
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    }

    if (ImGui::BeginPopupModal("Import Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
    {
        // File path
        sSceneImportOptions.mFilePath = GetEditorState()->mPendingSceneImportPath;
        ImGui::Text(GetEditorState()->mPendingSceneImportPath.c_str());
        ImGui::InputText("Scene Name", &sSceneImportOptions.mSceneName);
        ImGui::InputText("Prefix", &sSceneImportOptions.mPrefix);
        ImGui::Checkbox("Import Meshes", &sSceneImportOptions.mImportMeshes);
        ImGui::Checkbox("Import Materials", &sSceneImportOptions.mImportMaterials);
        ImGui::Checkbox("Import Textures", &sSceneImportOptions.mImportTextures);
        ImGui::Checkbox("Import Lights", &sSceneImportOptions.mImportLights);
        ImGui::Checkbox("Enable Collision", &sSceneImportOptions.mEnableCollision);

        int32_t shadingModelCount = int32_t(ShadingModel::Count);
        ImGui::Combo("Shading Model", (int*)&(sSceneImportOptions.mDefaultShadingModel), gShadingModelStrings, shadingModelCount);

        int32_t vertColorMode = int32_t(VertexColorMode::Count);
        ImGui::Combo("Vertex Color Mode", (int*)&(sSceneImportOptions.mDefaultVertexColorMode), gVertexColorModeStrings, vertColorMode);

        if (ImGui::Button("Import"))
        {
            am->ImportScene(sSceneImportOptions);
            GetEditorState()->mPendingSceneImportPath = "";
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            GetEditorState()->mPendingSceneImportPath = "";
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("EditPopup"))
    {
        if (ImGui::Selectable("Undo"))
            am->Undo();
        if (ImGui::Selectable("Redo"))
            am->Redo();

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("ViewPopup"))
    {
        Camera3D* cam = GetEditorState()->GetEditorCamera();
        ProjectionMode projMode = cam ? cam->GetProjectionMode() : ProjectionMode::PERSPECTIVE;
        if ((projMode == ProjectionMode::PERSPECTIVE && ImGui::Selectable("Orthographic")) ||
            (projMode == ProjectionMode::ORTHOGRAPHIC && ImGui::Selectable("Perspective")))
        {
            GetEditorState()->ToggleEditorCameraProjection();
        }
        if (ImGui::Selectable("Wireframe"))
            renderer->SetDebugMode(renderer->GetDebugMode() == DEBUG_WIREFRAME ? DEBUG_NONE : DEBUG_WIREFRAME);
        if (ImGui::Selectable("Collision"))
            renderer->SetDebugMode(renderer->GetDebugMode() == DEBUG_COLLISION ? DEBUG_NONE : DEBUG_COLLISION);
        if (ImGui::Selectable("Proxy"))
            renderer->EnableProxyRendering(!renderer->IsProxyRenderingEnabled());
        if (ImGui::Selectable("Bounds"))
        {
            uint32_t newMode = (uint32_t(renderer->GetBoundsDebugMode()) + 1) % uint32_t(BoundsDebugMode::Count);
            renderer->SetBoundsDebugMode((BoundsDebugMode)newMode);
        }
        if (ImGui::Selectable("Grid"))
            ToggleGrid();
        if (ImGui::Selectable("Stats"))
            renderer->EnableStatsOverlay(!renderer->IsStatsOverlayEnabled());
        if (ImGui::Selectable("Preview Lighting"))
        {
            GetEditorState()->mPreviewLighting = !GetEditorState()->mPreviewLighting;
            LogDebug("Preview lighting %s", GetEditorState()->mPreviewLighting ? "enabled." : "disabled.");
        }

        if (GetEditorState()->GetEditorMode() == EditorMode::Scene2D)
        {
            if (ImGui::Selectable("Reset 2D Viewport"))
            {
                GetEditorState()->GetViewport2D()->ResetViewport();
            }
        }

        if (ImGui::BeginMenu("Interface Scale"))
        {
            static float sInterfaceScale = GetEngineConfig()->mEditorInterfaceScale;
            ImGui::SliderFloat("IntScale", &sInterfaceScale, 0.5f, 3.0f);
            if (ImGui::Button("Apply"))
            {
                GetMutableEngineConfig()->mEditorInterfaceScale = sInterfaceScale;
                WriteEngineConfig();
            }
            ImGui::EndMenu();
        }

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("WorldPopup"))
    {
        if (ImGui::BeginMenu("Spawn Node"))
        {
            DrawAddNodeMenu(nullptr);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Spawn Basic 3D"))
        {
            DrawSpawnBasic3dMenu(nullptr, true);
            ImGui::EndMenu();
        }
        if (ImGui::Selectable("Clear World"))
            am->DeleteAllNodes();
        if (ImGui::Selectable("Bake Lighting"))
            renderer->BeginLightBake();
        if (ImGui::Selectable("Clear Baked Lighting"))
        {
            const std::vector<Node*>& nodes = GetWorld(0)->GatherNodes();
            for (uint32_t a = 0; a < nodes.size(); ++a)
            {
                StaticMesh3D* meshNode = nodes[a]->As<StaticMesh3D>();
                if (meshNode != nullptr && meshNode->GetBakeLighting())
                {
                    meshNode->ClearInstanceColors();
                }
            }
        }
        if (ImGui::Selectable("Toggle Transform Mode"))
            GetEditorState()->GetViewport3D()->ToggleTransformMode();

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("ExtraPopup"))
    {
        char versionStr[32];
        snprintf(versionStr, 31, "Version: %d", OCTAVE_VERSION);
        ImGui::Selectable(versionStr);

        if (ImGui::IsItemHovered() && IsMouseButtonJustUp(MOUSE_RIGHT))
        {
            sDevModeClicks++;
            if (sDevModeClicks >= 5)
            {
                GetEditorState()->mDevMode = true;
            }
        }

        if (GetEditorState()->mDevMode &&
            GetEngineState()->mStandalone &&
            ImGui::Selectable("Prepare Release"))
        {
            ActionManager::Get()->PrepareRelease();
        }

        ImGui::EndPopup();
    }

    // (2) Draw Scene tabs on top

    static int32_t sPrevActiveSceneIdx = 0;
    std::vector<EditScene>& scenes = GetEditorState()->mEditScenes;
    int32_t activeSceneIdx = GetEditorState()->mEditSceneIndex;
    bool sceneJustChanged = sPrevActiveSceneIdx != activeSceneIdx;

    const ImGuiTabBarFlags kSceneTabBarFlags = ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_FittingPolicyScroll;
    ImGui::SameLine(0.0f, 20.0f);

    if (scenes.size() > 0 &&
        ImGui::BeginTabBar("SceneTabBar", kSceneTabBarFlags))
    {
        int32_t openedTab = activeSceneIdx;

        for (int32_t n = 0; n < scenes.size(); n++)
        {
            // Push a unique ID in case we have scenes with duplicate names.
            ImGui::PushID(n);

            const EditScene& scene = scenes[n];

            bool opened = true;
            std::string sceneName = "*New";

            if (scene.mSceneAsset != nullptr)
            {
                Scene* sceneAsset = scene.mSceneAsset.Get<Scene>();
                sceneName = sceneAsset->GetName();
                if (sceneAsset->GetDirtyFlag())
                {
                    sceneName = "*" + sceneName;
                }
            }

            static char sTabId[128];
            snprintf(sTabId, 127, "%s###%d", sceneName.c_str(), n);

            ImGuiTabItemFlags tabFlags = ImGuiTabItemFlags_None;
            if (sceneJustChanged && n == activeSceneIdx)
            {
                tabFlags = ImGuiTabItemFlags_SetSelected;
            }

            if (ImGui::BeginTabItem(sTabId, &opened, tabFlags))
            {
                if (n != activeSceneIdx)
                {
                    openedTab = n;
                }

                ImGui::EndTabItem();
            }

            if (!opened)
            {
                GetEditorState()->CloseEditScene(n);
            }

            ImGui::PopID();
        }

        // Did we switch tabs? 
        if (!sceneJustChanged && 
            openedTab != activeSceneIdx)
        {
            GetEditorState()->OpenEditScene(openedTab);
        }

        ImGui::EndTabBar();
    }

    sPrevActiveSceneIdx = activeSceneIdx;


    // Draw 3D / 2D / Material combo box on top right corner.


    // Hotkey Menus
    if (GetEditorState()->GetViewport3D()->ShouldHandleInput())
    {
        const bool ctrlDown = IsControlDown();
        bool shiftDown = IsShiftDown();
        const bool altDown = IsAltDown();

        //ImGuiIO& io = ImGui::GetIO();
        //ImGui::SetNextWindowPos(io.MousePos);

        if (shiftDown && IsKeyJustDown(KEY_Q))
        {
            ImGui::OpenPopup("Spawn Basic 3D");
        }

        if (shiftDown && IsKeyJustDown(KEY_W))
        {
            ImGui::OpenPopup("Spawn Basic Widget");
        }

        if (shiftDown && IsKeyJustDown(KEY_A))
        {
            ImGui::OpenPopup("Spawn Node");
        }

        if (ctrlDown && IsKeyJustDown(KEY_N))
        {
            GetEditorState()->OpenEditScene(nullptr);
        }
    }

    if (ImGui::BeginPopup("Spawn Basic 3D"))
    {
        DrawSpawnBasic3dMenu(nullptr, true);
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Spawn Basic Widget"))
    {
        DrawSpawnBasicWidgetMenu(GetEditorState()->GetSelectedWidget());
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Spawn Node"))
    {
        DrawAddNodeMenu(nullptr);
        ImGui::EndPopup();
    }

    ImGui::End();

    // Set up ImGuizmo rect for the viewport area
    // edState was already declared earlier in this function
    ImGuizmo::SetRect(
        (float)edState->mViewportX,
        (float)edState->mViewportY,
        (float)edState->mViewportWidth,
        (float)edState->mViewportHeight
    );
}

static void DrawPaintColorsPanel()
{
    const float dispWidth = ImGui::GetIO().DisplaySize.x;
    const float dispHeight = ImGui::GetIO().DisplaySize.y;

    ImGui::SetNextWindowPos(ImVec2(kSidePaneWidth + 10.0f, 40.0f));
    ImGui::SetNextWindowSize(ImVec2(250.0f, 170.0f));

    ImGui::Begin("Paint Colors", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    PaintManager* pm = GetEditorState()->mPaintManager;
    ImGui::DragFloat("Radius", &pm->mRadius, 0.1f, 0.0f, 100.0f);
    ImGui::DragFloat("Opacity", &pm->mOpacity, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("Spacing", &pm->mSpacing, 0.5f, 0.0f, 500.0f);
    ImGui::OctColorEdit4("Color", &pm->mColorOptions.mColor[0], 0);
    
    const char* blendModeStrings[] = {"Mix", "Add", "Subtract", "Multiply", "+Alpha", "-Alpha"};
    int32_t blendModeCount = OCT_ARRAY_SIZE(blendModeStrings);
    ImGui::Combo("Blend Mode", (int*)&(pm->mColorOptions.mBlendMode), blendModeStrings, blendModeCount);

    ImGui::Checkbox("Only Facing Normals", &pm->mColorOptions.mOnlyFacingNormals);
    ImGui::Checkbox("Only Render Selected", &pm->mOnlyRenderSelected);

    ImGui::End();
}

static void DrawPaintInstancesPanel()
{
    const float dispWidth = ImGui::GetIO().DisplaySize.x;
    const float dispHeight = ImGui::GetIO().DisplaySize.y;

    ImGui::SetNextWindowPos(ImVec2(kSidePaneWidth + 10.0f, 40.0f));
    ImGui::SetNextWindowSize(ImVec2(250.0f, 210.0f));

    ImGui::Begin("Paint Instances", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    
    ImGui::PushItemWidth(120.0f);

    PaintManager* pm = GetEditorState()->mPaintManager;

    Property prop(DatumType::Asset, "Instance Mesh", nullptr, &(pm->mInstanceOptions.mMesh), 1, nullptr, (int32_t)StaticMesh::GetStaticType());
    DrawAssetProperty(prop, 0, nullptr, PropertyOwnerType::Count);
    ImGui::SameLine();
    ImGui::Text("Mesh");

    ImGui::DragFloat("Radius", &pm->mRadius, 0.1f, 0.0f, 100.0f);
    ImGui::DragFloat("Spacing", &pm->mSpacing, 0.5f, 0.0f, 500.0f);
    ImGui::DragFloat("Density", &pm->mInstanceOptions.mDensity, 0.05f, 0.0f, 100.0f);
    ImGui::DragFloat("Min Separation", &pm->mInstanceOptions.mMinSeparation, 0.05f, 0.0f, 100.0f);
    ImGui::OctDragScalarN("Min Position", ImGuiDataType_Float, &pm->mInstanceOptions.mMinPosition[0], 3, 1.0f, nullptr, nullptr, "%.2f", 0);
    ImGui::OctDragScalarN("Max Position", ImGuiDataType_Float, &pm->mInstanceOptions.mMaxPosition[0], 3, 1.0f, nullptr, nullptr, "%.2f", 0);
    ImGui::OctDragScalarN("Min Rotation", ImGuiDataType_Float, &pm->mInstanceOptions.mMinRotation[0], 3, 1.0f, nullptr, nullptr, "%.2f", 0);
    ImGui::OctDragScalarN("Max Rotation", ImGuiDataType_Float, &pm->mInstanceOptions.mMaxRotation[0], 3, 1.0f, nullptr, nullptr, "%.2f", 0);
    ImGui::DragFloat("Min Scale", &pm->mInstanceOptions.mMinScale, 0.05f, 0.0f, 100.0f);
    ImGui::DragFloat("Max Scale", &pm->mInstanceOptions.mMaxScale, 0.05f, 0.0f, 100.0f);
    ImGui::Checkbox("Align With Normal", &pm->mInstanceOptions.mAlignWithNormal);
    ImGui::Checkbox("Only Render Selected", &pm->mOnlyRenderSelected);
    ImGui::Checkbox("Erase", &pm->mInstanceOptions.mErase);

    ImGui::PopItemWidth();

    ImGui::End();
}

static void Draw2dSelections()
{
    const std::vector<Node*>& selNodes = GetEditorState()->GetSelectedNodes();

    ImDrawList* draw_list = ImGui::GetForegroundDrawList();
    ImColor mutliSelColor(0.7f, 1.0f, 0.0f, 1.0f);
    ImColor selColor(0.0f, 1.0f, 0.0f, 1.0f);
    ImColor hoverColor(0.0f, 1.0f, 1.0f, 1.0f);
    float thickness = 3.0f;
    glm::vec4 vp = Renderer::Get()->GetViewport(0);

    float interfaceScale = GetEngineConfig()->mEditorInterfaceScale;
    if (interfaceScale == 0.0f)
    {
        interfaceScale = 1.0f;
    }
    float invInterfaceScale = 1.0f / interfaceScale;

    Rect boundsRect;
    boundsRect.mX = 0.0f;
    boundsRect.mY = 0.0f;
    boundsRect.mWidth = vp.z;
    boundsRect.mHeight = vp.w;

    // Draw multiselected widget rects.
    for (int32_t i = 0; i < int32_t(selNodes.size()) - 1; ++i)
    {
        Widget* widget = selNodes[i]->As<Widget>();

        if (widget)
        {
            Rect rect = widget->GetRect();

            if (rect.OverlapsRect(boundsRect))
            {
                rect.Clamp(boundsRect);

                float x = invInterfaceScale * (rect.mX + vp.x);
                float y = invInterfaceScale * (rect.mY + vp.y);
                float w = invInterfaceScale * rect.mWidth;
                float h = invInterfaceScale * rect.mHeight;
                draw_list->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), mutliSelColor, 0.0f, ImDrawFlags_None, thickness);
            }
        }
    }

    // Draw the most recent selection differently
    if (selNodes.size() > 0)
    {
        Widget* widget = selNodes.back()->As<Widget>();

        if (widget)
        {
            Rect rect = widget->GetRect();

            if (rect.OverlapsRect(boundsRect))
            {
                rect.Clamp(boundsRect);

                float x = invInterfaceScale * (rect.mX + vp.x);
                float y = invInterfaceScale * (rect.mY + vp.y);
                float w = invInterfaceScale * (rect.mWidth);
                float h = invInterfaceScale * (rect.mHeight);
                draw_list->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), selColor, 0.0f, ImDrawFlags_None, thickness);
            }
        }
    }

    // Draw the hovered rect
    Widget* hoveredWidget = GetEditorState()->GetViewport2D()->GetHoveredWidget();
    if (hoveredWidget != nullptr)
    {
        Rect rect = hoveredWidget->GetRect();

        if (rect.OverlapsRect(boundsRect))
        {
            rect.Clamp(boundsRect);

            float x = invInterfaceScale * (rect.mX + vp.x);
            float y = invInterfaceScale * (rect.mY + vp.y);
            float w = invInterfaceScale * (rect.mWidth);
            float h = invInterfaceScale * (rect.mHeight);
            draw_list->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), hoverColor, 0.0f, ImDrawFlags_None, thickness);
        }
    }
}

static void DrawNodePropertySelectOverlay()
{
    int32_t mX;
    int32_t mY;
    GetMousePosition(mX, mY);

    float interfaceScale = GetEngineConfig()->mEditorInterfaceScale;
    if (interfaceScale == 0.0f)
    {
        interfaceScale = 1.0f;
    }
    float invInterfaceScale = 1.0f / interfaceScale;

    float rot = GetEngineState()->mRealElapsedTime * 3.0f;

    const float lineLength = 40.0f;
    const float lineWidth = 3.0f;
    glm::vec2 point1a = glm::vec2(-lineLength * 0.5f, 0.0f);
    glm::vec2 point1b = glm::vec2( lineLength * 0.5f, 0.0f);
    glm::vec2 point2a = glm::vec2(0.0f, -lineLength * 0.5f);
    glm::vec2 point2b = glm::vec2(0.0f,  lineLength * 0.5f);

    point1a = glm::rotate(point1a, rot);
    point1b = glm::rotate(point1b, rot);
    point2a = glm::rotate(point2a, rot);
    point2b = glm::rotate(point2b, rot);

    glm::vec2 offset = glm::vec2((float)mX * invInterfaceScale, (float)mY * invInterfaceScale);

    point1a += offset;
    point1b += offset;
    point2a += offset;
    point2b += offset;

    ImGui::GetForegroundDrawList()->AddLine({ point1a.x, point1a.y }, { point1b.x, point1b.y }, IM_COL32(255, 0, 0, 200), lineWidth);
    ImGui::GetForegroundDrawList()->AddLine({ point2a.x, point2a.y }, { point2b.x, point2b.y }, IM_COL32(255, 0, 0, 200), lineWidth);
}

static void DrawImGuizmo()
{
    EditorState* edState = GetEditorState();

    // Only draw gizmos in 3D mode when not playing in editor
    if (edState->GetEditorMode() != EditorMode::Scene3D &&
        edState->GetEditorMode() != EditorMode::Scene)
    {
        return;
    }

    if (IsPlayingInEditor() && !edState->mEjected)
    {
        return;
    }

    // Get the selected node
    Node* selectedNode = edState->GetSelectedNode();
    if (selectedNode == nullptr || !selectedNode->IsNode3D())
    {
        return;
    }

    Node3D* node3d = static_cast<Node3D*>(selectedNode);

    // Get the editor camera
    Camera3D* camera = edState->GetEditorCamera();
    if (camera == nullptr)
    {
        return;
    }

    // Get view and projection matrices
    glm::mat4 viewMatrix = camera->GetViewMatrix();
    glm::mat4 projMatrix = camera->GetProjectionMatrix();

    // The engine's projection matrix has Y flipped for Vulkan (projMatrix[1][1] is negated).
    // ImGuizmo expects OpenGL-style projection, so we need to un-flip Y.
    glm::mat4 imguizmoProjMatrix = projMatrix;
    imguizmoProjMatrix[1][1] *= -1.0f;

    // Get the node's world transform
    glm::mat4 modelMatrix = node3d->GetTransform();

    // Set orthographic mode if needed
    ImGuizmo::SetOrthographic(camera->GetProjectionMode() == ProjectionMode::ORTHOGRAPHIC);

    // Track undo state: cache the original transform when manipulation starts
    static bool wasUsing = false;
    static glm::mat4 originalMatrix;
    static Node3D* lastManipulatedNode = nullptr;

    bool isUsing = ImGuizmo::IsUsing();

    // Gizmo manipulation started - cache the original transform
    if (isUsing && !wasUsing)
    {
        originalMatrix = node3d->GetTransform();
        lastManipulatedNode = node3d;
    }

    // Store the delta matrix so we can apply incremental changes
    glm::mat4 deltaMatrix = glm::mat4(1.0f);

    // Call Manipulate with delta matrix
    bool manipulated = ImGuizmo::Manipulate(
        glm::value_ptr(viewMatrix),
        glm::value_ptr(imguizmoProjMatrix),
        edState->mGizmoOperation,
        edState->mGizmoMode,
        glm::value_ptr(modelMatrix),
        glm::value_ptr(deltaMatrix),
        nullptr   // snap
    );

    // If the gizmo was manipulated, apply the new transform using SetTransform
    // which uses the engine's own decomposition logic
    if (manipulated)
    {
        node3d->SetTransform(modelMatrix);
    }

    // Gizmo manipulation ended - commit to undo system
    if (!isUsing && wasUsing && lastManipulatedNode != nullptr)
    {
        glm::mat4 newMatrix = lastManipulatedNode->GetTransform();
        
        // Only commit if the transform actually changed
        if (originalMatrix != newMatrix)
        {
            // Temporarily restore original transform
            lastManipulatedNode->SetTransform(originalMatrix);
            lastManipulatedNode->UpdateTransform(false);
            
            // Commit the change through ActionManager for undo support
            ActionManager::Get()->EXE_EditTransform(lastManipulatedNode, newMatrix);
        }
        
        lastManipulatedNode = nullptr;
    }

    wasUsing = isUsing;
}

static void DrawImGuizmo2D()
{
    EditorState* edState = GetEditorState();

    // Only draw gizmos in 2D mode when not playing in editor
    if (edState->GetEditorMode() != EditorMode::Scene2D)
    {
        return;
    }

    if (IsPlayingInEditor() && !edState->mEjected)
    {
        return;
    }

    // Get the selected widget
    Widget* widget = edState->GetSelectedWidget();
    if (widget == nullptr)
    {
        return;
    }

    // Get viewport info
    Viewport2D* vp2d = edState->GetViewport2D();
    float zoom = vp2d->GetZoom();

    float vpWidth = (float)edState->mViewportWidth;
    float vpHeight = (float)edState->mViewportHeight;

    // Build orthographic projection matrix (Y down for screen coords)
    glm::mat4 projMatrix = glm::ortho(0.0f, vpWidth, vpHeight, 0.0f, -1.0f, 1.0f);

    // View matrix is identity since GetRect() already returns screen-space coordinates
    // (zoom and rootOffset are already applied via the wrapper widget)
    glm::mat4 viewMatrix = glm::mat4(1.0f);

    // Get widget's rect (already in screen space after zoom/offset transform)
    Rect rect = widget->GetRect();
    glm::vec2 pivot = widget->GetPivot();

    // Calculate pivot point in screen space
    float pivotX = rect.mX + rect.mWidth * pivot.x;
    float pivotY = rect.mY + rect.mHeight * pivot.y;

    // For scale mode, include current size in the model matrix so ImGuizmo can scale it
    // For translate/rotate, just use position and rotation
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(pivotX, pivotY, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(widget->GetRotation()), glm::vec3(0.0f, 0.0f, 1.0f));

    // For scale, encode the widget's screen-space size into the matrix
    if (edState->mGizmoOperation == ImGuizmo::SCALE)
    {
        modelMatrix = glm::scale(modelMatrix, glm::vec3(rect.mWidth, rect.mHeight, 1.0f));
    }

    // Set orthographic mode
    ImGuizmo::SetOrthographic(true);

    // Track undo state: cache the original transform when manipulation starts
    static bool wasUsing2D = false;
    static glm::vec2 originalOffset;
    static glm::vec2 originalSize;
    static float originalRotation;
    static glm::vec2 startSize;  // Size when gizmo manipulation started (for scale)
    static Widget* lastManipulatedWidget = nullptr;

    bool isUsing = ImGuizmo::IsUsing();

    // Gizmo manipulation started - cache the original transform
    if (isUsing && !wasUsing2D)
    {
        originalOffset = widget->GetOffset();
        originalSize = widget->GetSize();
        originalRotation = widget->GetRotation();
        startSize = widget->GetSize();
        lastManipulatedWidget = widget;
    }

    // Store the delta matrix
    glm::mat4 deltaMatrix = glm::mat4(1.0f);

    // Call Manipulate
    bool manipulated = ImGuizmo::Manipulate(
        glm::value_ptr(viewMatrix),
        glm::value_ptr(projMatrix),
        edState->mGizmoOperation,
        ImGuizmo::LOCAL,  // Use local mode for 2D
        glm::value_ptr(modelMatrix),
        glm::value_ptr(deltaMatrix),
        nullptr   // snap
    );

    // Apply transform changes
    if (manipulated)
    {
        if (edState->mGizmoOperation == ImGuizmo::TRANSLATE)
        {
            // Extract translation delta from deltaMatrix
            glm::vec3 deltaTrans(deltaMatrix[3]);

            // Convert screen delta to widget offset units (account for zoom)
            glm::vec2 offsetDelta = glm::vec2(deltaTrans.x, deltaTrans.y) / zoom;

            // Handle stretch mode - convert to ratio if needed
            if (widget->StretchX())
            {
                offsetDelta.x *= 0.002f;
            }
            if (widget->StretchY())
            {
                offsetDelta.y *= 0.002f;
            }

            glm::vec2 newOffset = widget->GetOffset() + offsetDelta;
            widget->SetOffset(newOffset.x, newOffset.y);
        }
        else if (edState->mGizmoOperation == ImGuizmo::ROTATE)
        {
            // Extract rotation delta from deltaMatrix
            // For 2D, we only care about Z rotation
            glm::vec3 deltaScale, deltaTrans, deltaSkew;
            glm::vec4 deltaPerspective;
            glm::quat deltaRot;
            glm::decompose(deltaMatrix, deltaScale, deltaRot, deltaTrans, deltaSkew, deltaPerspective);

            // Convert quaternion to euler and extract Z rotation
            // Negate because screen space Y is down, inverting rotation direction
            glm::vec3 eulerDelta = glm::eulerAngles(deltaRot);
            float deltaRotDeg = -glm::degrees(eulerDelta.z);

            float newRotation = widget->GetRotation() + deltaRotDeg;
            widget->SetRotation(newRotation);
        }
        else if (edState->mGizmoOperation == ImGuizmo::SCALE)
        {
            // Extract the new scale from the modified modelMatrix
            glm::vec3 newScale, trans, skew;
            glm::vec4 perspective;
            glm::quat rot;
            glm::decompose(modelMatrix, newScale, rot, trans, skew, perspective);

            // The modelMatrix was initialized with (rect.mWidth, rect.mHeight, 1)
            // After manipulation, newScale contains the scaled dimensions
            // Convert back to widget size units by dividing by zoom
            glm::vec2 newScreenSize(newScale.x, newScale.y);
            glm::vec2 newSize = newScreenSize / zoom;

            // Handle stretch mode
            if (widget->StretchX())
            {
                // For stretch, convert pixel change to ratio
                float pixelChange = newScreenSize.x - rect.mWidth;
                newSize.x = widget->GetSize().x + pixelChange * 0.00002f;
            }
            if (widget->StretchY())
            {
                float pixelChange = newScreenSize.y - rect.mHeight;
                newSize.y = widget->GetSize().y + pixelChange * 0.00002f;
            }

            widget->SetSize(newSize.x, newSize.y);
        }
    }

    // Gizmo manipulation ended - commit to undo system
    if (!isUsing && wasUsing2D && lastManipulatedWidget != nullptr)
    {
        glm::vec2 newOffset = lastManipulatedWidget->GetOffset();
        glm::vec2 newSize = lastManipulatedWidget->GetSize();
        float newRotation = lastManipulatedWidget->GetRotation();

        // Only commit if the transform actually changed
        bool offsetChanged = (originalOffset != newOffset);
        bool sizeChanged = (originalSize != newSize);
        bool rotationChanged = (originalRotation != newRotation);

        if (offsetChanged || sizeChanged || rotationChanged)
        {
            // Temporarily restore original transform
            lastManipulatedWidget->SetOffset(originalOffset.x, originalOffset.y);
            lastManipulatedWidget->SetSize(originalSize.x, originalSize.y);
            lastManipulatedWidget->SetRotation(originalRotation);

            // Commit changes through ActionManager for undo support
            if (offsetChanged)
            {
                ActionManager::Get()->EXE_EditProperty(lastManipulatedWidget, PropertyOwnerType::Node, "Offset", 0, newOffset);
            }
            if (sizeChanged)
            {
                ActionManager::Get()->EXE_EditProperty(lastManipulatedWidget, PropertyOwnerType::Node, "Size", 0, newSize);
            }
            if (rotationChanged)
            {
                ActionManager::Get()->EXE_EditProperty(lastManipulatedWidget, PropertyOwnerType::Node, "Rotation", 0, newRotation);
            }
        }

        lastManipulatedWidget = nullptr;
    }

    wasUsing2D = isUsing;
}

void EditorImguiInit()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuizmo::SetImGuiContext(ImGui::GetCurrentContext());
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    
    // Disabling keyboard controls because it interferes with Alt hotkeys.
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Dear ImGui style
    // ImGui::StyleColorsDark();
        SetupImGuiStyle();
        std::string fontPath = SYS_GetAbsolutePath("Engine/Assets/Fonts/F_InterRegular18.ttf");
        ImFont* myFont = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 15.0f);


    //ImGui::StyleColorsLight();

    // Override theme
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Tab] = ImVec4(0.20f, 0.20f, 0.68f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.61f, 0.98f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.12f, 0.50f, 0.47f, 1.00f);



    // Set unactive window title bg equal to active title.
    colors[ImGuiCol_TitleBg] = colors[ImGuiCol_TitleBgActive];
}

void EditorImguiDraw()
{
    EngineState* engState = GetEngineState();

    ImGuiIO& io = ImGui::GetIO();
    float interfaceScale = GetEngineConfig()->mEditorInterfaceScale;
    if (interfaceScale == 0.0f)
    {
        interfaceScale = 1.0f;
    }

    io.OctaveInterfaceScale = interfaceScale;
    io.DisplaySize = ImVec2(engState->mWindowWidth / interfaceScale, engState->mWindowHeight / interfaceScale);
    io.DisplayFramebufferScale = ImVec2(interfaceScale, interfaceScale);

    ImGui::NewFrame();
    ImGuizmo::BeginFrame();

    if (EditorIsInterfaceVisible())
    {
        if (GetEditorState()->mShowLeftPane)
        {
            DrawScenePanel();
            DrawAssetsPanel();
        }

        if (GetEditorState()->mShowRightPane)
        {
            DrawPropertiesPanel();
        }

        DrawViewportPanel();

        // Draw ImGuizmo gizmos for selected 3D nodes
        DrawImGuizmo();
        // Draw ImGuizmo gizmos for selected 2D widgets
        DrawImGuizmo2D();

        PaintMode paintMode = GetEditorState()->GetPaintMode();
        if (paintMode == PaintMode::Color)
        {
            DrawPaintColorsPanel();
        }
        else if (paintMode == PaintMode::Instance)
        {
            DrawPaintInstancesPanel();
        }

        if (GetEditorState()->GetEditorMode() == EditorMode::Scene2D)
        {
            Draw2dSelections();
        }

        if (GetEditorState()->mNodePropertySelect)
        {
            DrawNodePropertySelectOverlay();
        }

        DrawFileBrowser();

        DrawUnsavedCheck();
    }

    ImGui::Render();
}

void EditorImguiShutdown()
{
    ImGui::DestroyContext();
}

void EditorImguiPreShutdown()
{
    if (sInspectTexId != 0)
    {
        DeviceWaitIdle();
        ImGui_ImplVulkan_RemoveTexture((VkDescriptorSet)sInspectTexId);
        sInspectTexId = 0;
    }
}

void EditorImguiGetViewport(uint32_t& x, uint32_t& y, uint32_t& width, uint32_t& height)
{
    if (EditorIsInterfaceVisible())
    {
        ImGuiIO& io = ImGui::GetIO();

        float scale = io.OctaveInterfaceScale;
        if (scale == 0.0f)
        {
            scale = 1.0f;
        }
        float invScale = 1.0f / scale;

        x = 0;
        y = uint32_t(kViewportBarHeight * scale + 0.5f);
        int32_t iWidth = (int32_t)io.DisplaySize.x;
        int32_t iHeight = int32_t(io.DisplaySize.y - kViewportBarHeight * scale + 0.5f);

        if (GetEditorState()->mShowLeftPane)
        {
            x = int32_t(kSidePaneWidth * scale + 0.5f);
            iWidth -= int32_t(kSidePaneWidth * scale + 0.5f);
        }
        if (GetEditorState()->mShowRightPane)
        {
            iWidth -= int32_t(kSidePaneWidth * scale + 0.5f);
        }

        iWidth = glm::clamp<int32_t>(iWidth, 100, int32_t(ImGui::GetIO().DisplaySize.x + 0.5f));
        iHeight = glm::clamp<int32_t>(iHeight, 100, int32_t(ImGui::GetIO().DisplaySize.y + 0.5f));

        width = uint32_t(iWidth);
        height = uint32_t(iHeight);
    }
    else
    {
        x = 0;
        y = 0;
        width = uint32_t(ImGui::GetIO().DisplaySize.x + 0.5f);
        height = uint32_t(ImGui::GetIO().DisplaySize.y + 0.5f);
    }
}

bool EditorIsInterfaceVisible()
{
    return GetEditorState()->mShowInterface && (!IsPlaying() || GetEditorState()->mEjected);
}

#endif
