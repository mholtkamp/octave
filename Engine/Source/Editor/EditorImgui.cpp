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
#include "FeatureFlags.h"
#include "EditorIcons.h"

#include "Nodes/3D/StaticMesh3d.h"
#include "Nodes/3D/InstancedMesh3d.h"
#include "Nodes/3D/SkeletalMesh3d.h"
#include "Nodes/3D/Camera3d.h"
#include "Nodes/3D/Spline3d.h"
#include "Nodes/3D/PointLight3d.h"
#include "Nodes/3D/DirectionalLight3d.h"
#include "Nodes/3D/Particle3d.h"
#include "Nodes/3D/Audio3d.h"
#include "Nodes/3D/Box3d.h"
#include "Nodes/3D/Sphere3d.h"
#include "Nodes/3D/Capsule3d.h"
#include "Nodes/3D/ShadowMesh3d.h"
#include "Nodes/3D/TextMesh3d.h"
#include "World.h"

#include "Assets/Scene.h"
#include "Assets/SoundWave.h"
#include "Assets/ParticleSystem.h"
#include "Assets/StaticMesh.h"
#include "Assets/SkeletalMesh.h"
#include "Assets/MaterialBase.h"
#include "Assets/MaterialInstance.h"
#include "Assets/MaterialLite.h"
#include "Assets/Timeline.h"
#include "Assets/Font.h"

#include "Viewport3d.h"
#include "Viewport2d.h"
#include "ActionManager.h"
#include "EditorState.h"
#include "Preferences/PreferencesWindow.h"
#include "Preferences/Appearance/Theme/ThemeModule.h"
#include "Packaging/PackagingWindow.h"
#include "ProjectSelect/ProjectSelectWindow.h"
#include "Addons/AddonsWindow.h"
#include "Addons/NativeAddonManager.h"
#include "Addons/AddonsMenu.h"
#include "EditorUIHookManager.h"
#include "DebugLog/DebugLogWindow.h"
#include "ScriptEditor/ScriptEditorWindow.h"
#include "ThemeEditor/ThemeEditorWindow.h"
#include "Preferences/Appearance/Theme/CssThemeParser.h"
#include "Timeline/TimelinePanel.h"
#include "Preferences/General/GeneralModule.h"
#include "Preferences/PreferencesManager.h"
#include "Preferences/External/LaunchersModule.h"
#include "Preferences/External/EditorsModule.h"
#include "Packaging/PackagingSettings.h"
#include "ScriptCreator/ScriptCreatorDialog.h"

#include <functional>
#include <algorithm>
#include <map>

// TODO: If we ever support an OpenGL backend, gotta change this.
#include "backends/imgui_impl_vulkan.cpp"
#include "Graphics/Vulkan/VulkanContext.h"
#include "Graphics/Vulkan/VulkanUtils.h"

#include "CustomImgui.h"
#include "./ImGuizmo/ImGuizmo.h"
#include "imgui_dock.h"
#include "imgui_internal.h"

#if PLATFORM_WINDOWS
#include "backends/imgui_impl_win32.cpp"
#elif PLATFORM_LINUX
#include "imgui_impl_xcb.h"
#endif
#include <Nodes/TimelinePlayer.h>
#include <Nodes/Widgets/Button.h>
#include <Nodes/Widgets/Quad.h>
#include <Nodes/Widgets/Canvas.h>
#include <Nodes/Widgets/Console.h>
#include <Nodes/Widgets/ArrayWidget.h>


static const char* GetNodeIcon(Node* node)
{
    if (node->As<InstancedMesh3D>())          return ICON_INSTANCE_MESH;
    if (node->As<ShadowMesh3D>())             return ICON_SHADOW;
    if (node->As<SkeletalMesh3D>())           return ICON_SKELETON;
    if (node->As<TextMesh3D>())               return ICON_TEXTMESH;
    if (node->As<StaticMesh3D>())             return ICON_STATIC_MESH;
    if (node->As<PointLight3D>())             return ICON_LIGHTBULB;
    if (node->As<DirectionalLight3D>())       return ICON_SUN;
    if (node->As<Camera3D>())                 return ICON_IX_VIDEO_CAMERA_FILLED;
    if (node->As<Particle3D>())               return ICON_FIREWORK;
    if (node->As<Audio3D>())                  return ICON_RIVET_ICONS_AUDIO_SOLID;
    if (node->As<Sphere3D>())                 return ICON_PH_SPHERE;
    if (node->As<Box3D>())                    return ICON_FLUENT_MDL2_CUBE_SHAPE;
    if (node->As<Capsule3D>())                return ICON_CAPSULE;
    if (node->As<Spline3D>())                 return ICON_IC_BASELINE_LINK;
    if (node->As<Text>())                     return ICON_TEXTWIDGET;
    if (node->As<Button>())                   return ICON_BUTTON;
    if (node->As<Console>())                  return ICON_CONSOLE;
    if (node->As<Canvas>())                   return ICON_CANVAS;
    if (node->As<Quad>())                     return ICON_RECT;
    if (node->As<ArrayWidget>())              return ICON_LAYERS;
    if (node->As<TimelinePlayer>())           return ICON_CLAPPERBOARD_CLOSE;
    if (node->IsSceneLinked())                return ICON_STREAMLINE_PLUMP_WORLD_REMIX;
    if (node->IsNode3D())                     return ICON_PH_SPHERE;
    if (node->IsWidget())                     return ICON_UIWIDGET;
    return ICON_STREAMLINE_PLUMP_WORLD_REMIX;
}

static const char* GetAssetIcon(TypeId type)
{
    if (type == StaticMesh::GetStaticType())       return ICON_STATIC_MESH;
    if (type == SkeletalMesh::GetStaticType())      return ICON_SKELETON;
    if (type == Texture::GetStaticType())            return ICON_TDESIGN_IMAGE_FILLED;
    if (type == Scene::GetStaticType())              return ICON_STREAMLINE_PLUMP_WORLD_REMIX;
    if (type == SoundWave::GetStaticType())          return ICON_RIVET_ICONS_AUDIO_SOLID;
    if (type == MaterialLite::GetStaticType())       return ICON_HUGEICONS_MATERIAL_AND_TEXTURE;
    if (type == MaterialBase::GetStaticType())       return ICON_HUGEICONS_MATERIAL_AND_TEXTURE;
    if (type == MaterialInstance::GetStaticType())   return ICON_HUGEICONS_MATERIAL_AND_TEXTURE;
    if (type == ParticleSystem::GetStaticType())     return ICON_FIREWORK;
    if (type == Timeline::GetStaticType())           return ICON_TIMELINE;
    if (type == Font::GetStaticType())               return ICON_TEXTMESH;
    return ICON_STREAMLINE_SHARP_NEW_FILE_REMIX;
}

struct FileBrowserDirEntry
{
    std::string mName;
    std::string mDirPath;
    bool mFolder = false;
    bool mSelected = false;
};

static float GetMainMenuBarHeight()
{
    return ImGui::GetFrameHeight();
}
static float GetTopBarHeight()
{
    return GetMainMenuBarHeight();
}
static const ImVec4 kSelectedColor = ImVec4(0.12f, 0.50f, 0.47f, 1.00f);
static const ImVec4 kBgInactive = ImVec4(0.20f, 0.20f, 0.68f, 1.00f);
static const ImVec4 kBgHover = ImVec4(0.26f, 0.61f, 0.98f, 0.80f);
static const ImVec4 kToggledColor = ImVec4(0.7f, 0.2f, 0.2f, 1.0f);

constexpr const uint32_t kPopupInputBufferSize = 256;
static char sPopupInputBuffer[kPopupInputBufferSize] = {};

static bool sNodesDiscovered = false;
static bool showTheming = false;
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
static SceneImportOptions sReimportSceneOptions;
static CameraImportOptions sCameraImportOptions;

static std::vector<AssetStub*> sUnsavedAssets;
static std::vector<bool> sUnsavedAssetsSelected;
static bool sUnsavedModalActive = false;

static std::vector<std::string> sSceneList;
static int32_t sDevModeClicks = 0;

static std::string sReplaceAssetInput;
static std::vector<std::string> sReplaceAssetSuggestions;

static AssetStub* sAssetDropStub = nullptr;
static Node* sAssetDropParentNode = nullptr;
static bool sAssetDropInViewport = false;
static ImVec2 sAssetDropPopupPos = ImVec2(0.0f, 0.0f);
static bool sAssetDropPopupPending = false;
static int32_t sAssetDropScreenX = 0;
static int32_t sAssetDropScreenY = 0;

static bool IsBottomPaneVisible()
{
    if (!GetEditorState()->mShowBottomPane) return false;
    PreferencesManager* pm = PreferencesManager::Get();
    if (pm)
    {
        GeneralModule* gm = static_cast<GeneralModule*>(pm->FindModule("General"));
        if (gm && !gm->GetShowDebugInEditor()) return false;
    }
    return true;
}

static bool sDockInitialized = false;
static bool sDockResetRequested = false;
static int  sDockActivateViewportFrames = 0;
static ImVec2 sViewportDockPos = ImVec2(0, 0);
static ImVec2 sViewportDockSize = ImVec2(800, 600);
static ImGuiWindow* sViewportDockWindow = nullptr;

// A known dock label that must exist in a valid layout.
// If imgui.ini has dock data but this label is missing, the layout is stale.
// Update kDockLayoutVersion when dock panel names change to force a reset.
static constexpr uint32_t kDockLayoutVersion = 4;

static void ValidateDockLayoutIni()
{
    const char* iniPath = ImGui::GetIO().IniFilename;
    if (!iniPath)
        return;

    FILE* f = fopen(iniPath, "rb");
    if (!f)
        return; // No INI yet — first launch, nothing to validate.

    fseek(f, 0, SEEK_END);
    long fileSize = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (fileSize <= 0 || fileSize > 1024 * 1024)
    {
        fclose(f);
        return;
    }

    std::string contents(fileSize, '\0');
    fread(&contents[0], 1, fileSize, f);
    fclose(f);

    // Check if the file has a dock section at all.
    if (contents.find("[Dock]") == std::string::npos)
        return; // No dock data saved — nothing to invalidate.

    // Check for a version marker we embed: "##DockLayoutVersion=N"
    char versionTag[64];
    snprintf(versionTag, sizeof(versionTag), "##DockLayoutVersion=%u", kDockLayoutVersion);

    if (contents.find(versionTag) != std::string::npos)
        return; // Version matches — layout is current.

    // Layout is stale (version missing or wrong). Delete the file so ImGui
    // starts fresh.  The default layout will be built by DrawDockspace()
    // and saved on shutdown.
    LogDebug("Dock layout version mismatch -- resetting imgui.ini");
    remove(iniPath);
}

// Append the dock layout version marker to imgui.ini after ImGui saves it.
static void StampDockLayoutVersion()
{
    const char* iniPath = ImGui::GetIO().IniFilename;
    if (!iniPath)
        return;

    FILE* f = fopen(iniPath, "a");
    if (!f)
        return;

    fprintf(f, "\n##DockLayoutVersion=%u\n", kDockLayoutVersion);
    fclose(f);
}

// Forward declarations for panel content functions (called from DrawDockspace)
static void DrawScenePanel();
static void DrawAssetsPanel();
static void DrawPropertiesPanel();
static void DrawScriptsPanel();

static void DrawDockspace()
{
    float topBarH = GetTopBarHeight();
    float displayW = ImGui::GetIO().DisplaySize.x;
    float displayH = ImGui::GetIO().DisplaySize.y;

    ImGui::SetNextWindowPos(ImVec2(0.0f, topBarH));
    ImGui::SetNextWindowSize(ImVec2(displayW, displayH - topBarH));

    ImGuiWindowFlags dockHostFlags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("EditorDock", nullptr, dockHostFlags);
    ImGui::PopStyleVar();

    // Make dockspace child transparent so 3D render shows through Viewport area
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
    ImGui::BeginDockspace();
    ImGui::PopStyleColor();

    // Visibility bools synced with EditorState
    bool sceneOpen = GetEditorState()->mShowLeftPane;
    bool assetsOpen = GetEditorState()->mShowLeftPane;
    bool propsOpen = GetEditorState()->mShowRightPane;
    bool debugLogOpen = IsBottomPaneVisible();

    // --- Viewport dock (transparent so 3D render shows through) ---
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
    if (ImGui::BeginDock(ICON_VIEWPORT3D "  Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
    {
        sViewportDockPos = ImGui::GetWindowPos();
        sViewportDockSize = ImGui::GetWindowSize();
        sViewportDockWindow = ImGui::GetCurrentWindow();

        // Drop target for asset drag-and-drop into the 3D viewport
        const ImGuiPayload* activePayload = ImGui::GetDragDropPayload();
        if (activePayload != nullptr && activePayload->IsDataType(DRAGDROP_ASSET))
        {
            ImVec2 avail = ImGui::GetContentRegionAvail();
            if (avail.x > 0.0f && avail.y > 0.0f)
            {
                ImGui::InvisibleButton("##ViewportDropTarget", avail);
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(DRAGDROP_ASSET))
                    {
                        AssetStub* droppedStub = *(AssetStub**)payload->Data;
                        if (droppedStub != nullptr && droppedStub->mType == StaticMesh::GetStaticType())
                        {
                            sAssetDropStub = droppedStub;
                            sAssetDropParentNode = nullptr;
                            sAssetDropInViewport = true;
                            sAssetDropPopupPos = ImGui::GetMousePos();
                            sAssetDropPopupPending = true;

                            ImVec2 dropPos = ImGui::GetMousePos();
                            float scale = GetEngineConfig()->mEditorInterfaceScale;
                            if (scale == 0.0f) scale = 1.0f;
                            sAssetDropScreenX = (int32_t)(dropPos.x * scale);
                            sAssetDropScreenY = (int32_t)(dropPos.y * scale);
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
            }
        }
    }
    ImGui::EndDock();
    ImGui::PopStyleColor();

    // --- Assets dock ---
    {
        ImVec4 assetsBg;
        ImVec4 bg = CssThemeParser::GetPanelAssetsBg(assetsBg) ? assetsBg : ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, bg);
    }
    if (ImGui::BeginDock(ICON_ASSETS "  Assets", &assetsOpen))
    {
        DrawAssetsPanel();
    }
    ImGui::EndDock();
    ImGui::PopStyleColor();

    // --- Scene dock ---
    {
        ImVec4 sceneBg;
        ImVec4 bg = CssThemeParser::GetPanelSceneBg(sceneBg) ? sceneBg : ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, bg);
    }
    if (ImGui::BeginDock(ICON_HIERARCHY "  Scene", &sceneOpen))
    {
        DrawScenePanel();
    }
    ImGui::EndDock();
    ImGui::PopStyleColor();

    // --- Properties dock ---
    {
        bool lockedProperties = sObjectTabOpen && GetEditorState()->IsInspectLocked();
        ImVec4 propsPanelBg;
        ImVec4 propsBg = lockedProperties ? ImVec4(0.4f, 0.0f, 0.0f, 1.0f)
            : (CssThemeParser::GetPanelPropertiesBg(propsPanelBg) ? propsPanelBg : ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, propsBg);
    }
    if (ImGui::BeginDock(ICON_INFO "  Properties", &propsOpen))
    {
        DrawPropertiesPanel();
    }
    ImGui::EndDock();
    ImGui::PopStyleColor();

    // --- Debug Log dock ---
    {
        ImVec4 debugBg;
        ImVec4 bg = CssThemeParser::GetPanelDebugLogBg(debugBg) ? debugBg : ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, bg);
    }
    if (ImGui::BeginDock(ICON_STREAMLINE_LOG_SOLID "  Debug Log", &debugLogOpen, ImGuiWindowFlags_NoCollapse))
    {
        GetDebugLogWindow()->DrawContent();
    }
    ImGui::EndDock();
    ImGui::PopStyleColor();

    // --- Scripts dock ---
    {
        ImVec4 bg = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, bg);
    }
    if (ImGui::BeginDock(ICON_IX_CODE "  Scripts", nullptr))
    {
        DrawScriptsPanel();
    }
    ImGui::EndDock();
    ImGui::PopStyleColor();

    // --- Script Editor dock ---
    {
        ImVec4 bg = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, bg);
    }
    if (ImGui::BeginDock(ICON_IX_CODE "  Script Editor", nullptr))
    {
        GetScriptEditorWindow()->DrawContent();
    }
    ImGui::EndDock();
    ImGui::PopStyleColor();

    // Draw plugin/addon dockable windows
    EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
    if (hookMgr)
    {
        hookMgr->DrawWindows();
    }

    ImGui::EndDockspace();
    ImGui::End();

    // Explicit layout setup on first frame or when reset is requested
    if (!sDockInitialized || sDockResetRequested)
    {
        bool isReset = sDockResetRequested;
        sDockInitialized = true;
        sDockResetRequested = false;

        if (isReset || !ImGui::HasDockLayout("EditorDock"))
        {
            ImGui::UndockAll("EditorDock");
            ImGui::DockToRoot("EditorDock", ICON_HIERARCHY "  Scene", ImGuiDockSlot_Left, 0.2f);
            ImGui::DockToRoot("EditorDock", ICON_INFO "  Properties", ImGuiDockSlot_Right, 0.15f);
            ImGui::DockToRoot("EditorDock",ICON_ASSETS "  Assets", ImGuiDockSlot_Bottom, 0.33f);
            ImGui::DockTo("EditorDock", ICON_IX_CODE "  Scripts", ICON_ASSETS "  Assets", ImGuiDockSlot_Tab);
            ImGui::DockTo("EditorDock", ICON_STREAMLINE_LOG_SOLID "  Debug Log", ICON_ASSETS "  Assets", ImGuiDockSlot_Tab);
            ImGui::DockTo("EditorDock", ICON_IX_CODE "  Script Editor", ICON_VIEWPORT3D "  Viewport", ImGuiDockSlot_Tab);

            // Defer activating the Viewport tab — docks call setActive() on their
            // first BeginDock frame, so we need to wait a couple frames for all
            // docks to finish their first-time setup before overriding.
            sDockActivateViewportFrames = 3;

            // Persist the fresh layout immediately so a crash won't leave a stale INI.
            if (ImGui::GetIO().IniFilename)
            {
                ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
                StampDockLayoutVersion();
            }

            if (isReset)
            {
                GetEditorState()->mShowLeftPane = true;
                GetEditorState()->mShowRightPane = true;
                GetEditorState()->mShowBottomPane = true;
            }
        }
    }

    // After layout reset, wait for all docks to finish first-frame setup,
    // then force the Viewport as the active tab in its group.
    if (sDockActivateViewportFrames > 0)
    {
        --sDockActivateViewportFrames;
        if (sDockActivateViewportFrames == 0)
        {
            ImGui::SetDockActive("EditorDock", ICON_VIEWPORT3D "  Viewport");
        }
    }

    // Sync visibility flags back from dock close buttons
    if (!sceneOpen)
        GetEditorState()->mShowLeftPane = false;
    if (!propsOpen)
        GetEditorState()->mShowRightPane = false;
    if (!debugLogOpen)
        GetEditorState()->mShowBottomPane = false;
}

static void PopulateFileBrowserDirs()
{
    sFileBrowserDoubleClickBlock = 0.2f;

    sFileBrowserEntries.clear();

    DirEntry dirEntry;
    SYS_OpenDirectory(sFileBrowserCurDir, dirEntry);

    if (!dirEntry.mValid)
    {

        GetEditorState()->RemoveFavoriteDir(sFileBrowserCurDir);

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
        strncpy(sPopupInputBuffer, entry->mName.c_str(), kPopupInputBufferSize - 1);
        sPopupInputBuffer[kPopupInputBufferSize - 1] = '\0';
        setTextInputFocus = true;
    }
    if (ImGui::Selectable("New Folder", false, ImGuiSelectableFlags_DontClosePopups))
    {
        ImGui::OpenPopup("New Folder");
        sPopupInputBuffer[0] = '\0';
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
        ImGui::SetNextWindowSize(ImVec2(400,300));
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

static void DrawProjectUpgradeModal()
{
    EditorState* editorState = GetEditorState();

    if (editorState->mShowProjectUpgradeModal)
    {
        ImGui::OpenPopup("Project Upgrade Required");
    }

    // Center the modal
    if (ImGui::IsPopupOpen("Project Upgrade Required"))
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(500, 350));
    }

    if (ImGui::BeginPopupModal("Project Upgrade Required", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
    {
        ImGui::TextWrapped("This project contains assets that were created with an older version of the engine.");
        ImGui::Spacing();
        ImGui::TextWrapped("The new version includes improved asset reference handling with UUID + name fallback, which prevents broken references in packaged builds.");
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Assets needing upgrade: %d", (int)editorState->mAssetsNeedingUpgrade.size());
        ImGui::Spacing();

        // Show list of assets needing upgrade (scrollable)
        if (ImGui::BeginListBox("##UpgradeAssetList", ImVec2(480, 150)))
        {
            for (uint32_t i = 0; i < editorState->mAssetsNeedingUpgrade.size(); ++i)
            {
                AssetStub* stub = editorState->mAssetsNeedingUpgrade[i];
                if (stub == nullptr)
                    continue;

                glm::vec4 assetColor = AssetManager::Get()->GetEditorAssetColor(stub->mType);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(assetColor.r, assetColor.g, assetColor.b, assetColor.a));
                ImGui::Text("%s", stub->mName.c_str());
                ImGui::PopStyleColor();
            }
            ImGui::EndListBox();
        }

        ImGui::Spacing();
        ImGui::TextWrapped("Click 'Upgrade Now' to automatically resave all assets with the new format.");
        ImGui::Spacing();

        bool closePopup = false;

        if (ImGui::Button("Upgrade Now", ImVec2(120, 0)))
        {
            closePopup = true;
            ActionManager::Get()->UpgradeProject();
            LogDebug("Project upgrade completed successfully!");
        }

        ImGui::SameLine();
        if (ImGui::Button("Skip", ImVec2(120, 0)))
        {
            closePopup = true;
            editorState->mAssetsNeedingUpgrade.clear();
            LogWarning("Project upgrade skipped. Packaged builds may have issues with asset references.");
        }

        if (IsKeyJustDown(KEY_ESCAPE))
        {
            closePopup = true;
            editorState->mAssetsNeedingUpgrade.clear();
        }

        if (closePopup)
        {
            editorState->mShowProjectUpgradeModal = false;
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
            if (ownerType == PropertyOwnerType::Node || ownerType == PropertyOwnerType::Asset)
            {
                ActionManager::Get()->EXE_EditProperty(owner, ownerType, prop.mName, index, newAsset);
            }
            else
            {
                prop.SetAsset(newAsset, index);
            }
        }
    }
}

static bool NodeMatchesProperty(const Property& prop, Node* node, Object* owner)
{
    if (node == nullptr)
        return true;

    // Spline3D attachment filters
    if (owner && owner->Is("Spline3D"))
    {
        if (prop.mName == "Camera") return node->Is("Camera3D");
        if (prop.mName == "Static Mesh") return node->Is("StaticMesh3D");
        if (prop.mName == "Skeletal Mesh") return node->Is("SkeletalMesh3D");
        if (prop.mName == "Particle") return node->Is("Particle3D");
        if (prop.mName == "Point Light") return node->Is("PointLight3D");
        if (prop.mName == "Audio") return node->Is("Audio3D");
        if (prop.mName == "Point Speed Target") return node->GetName().rfind("point", 0) == 0;
    }

    if (prop.mExtra && prop.mExtra->GetType() == DatumType::String)
    {
        const std::string& typeName = prop.mExtra->GetString();
        if (!typeName.empty())
        {
            return node->Is(typeName.c_str());
        }
    }

    return true;
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
            if (NodeMatchesProperty(prop, newNode, owner))
            {
                am->EXE_EditProperty(owner, ownerType, prop.mName, index, newNode);
            }
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
            if (ownerType == PropertyOwnerType::Node || ownerType == PropertyOwnerType::Asset)
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
    const float kCategoryIndentWidth = 8.0f;
    const bool ctrlDown = IsControlDown();
    const bool altDown = IsAltDown();
    const bool shiftDown = IsShiftDown();

    const char* curCategory = "";
    bool catOpen = true;
    bool catIndented = false;

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
            if (catIndented)
            {
                ImGui::Unindent(kCategoryIndentWidth);
                catIndented = false;
            }

            curCategory = prop.mCategory;

            if (strcmp(prop.mCategory, "") != 0)
            {
                catOpen = ImGui::CollapsingHeader(curCategory, ImGuiTreeNodeFlags_DefaultOpen);

                if (catOpen)
                {
                    ImGui::Indent(kCategoryIndentWidth);
                    catIndented = true;
                }
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
                        if (ownerType == PropertyOwnerType::Node || ownerType == PropertyOwnerType::Asset)
                        {
                            am->EXE_EditProperty(owner, ownerType, prop.mName, i, propVal);
                        }
                        else
                        {
                            prop.SetInteger(propVal, i);
                        }
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
                        if (ownerType == PropertyOwnerType::Node || ownerType == PropertyOwnerType::Asset)
                        {
                            prop.SetInteger(sOrigVal, i);
                            am->EXE_EditProperty(owner, ownerType, prop.mName, i, propVal);
                        }
                        else
                        {
                            prop.SetInteger(propVal, i);
                        }
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
                    if (ownerType == PropertyOwnerType::Node || ownerType == PropertyOwnerType::Asset)
                    {
                        prop.SetFloat(sOrigVal, i);
                        am->EXE_EditProperty(owner, ownerType, prop.mName, i, propVal);
                    }
                    else
                    {
                        prop.SetFloat(propVal, i);
                    }
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
                    if (ownerType == PropertyOwnerType::Node || ownerType == PropertyOwnerType::Asset)
                    {
                        am->EXE_EditProperty(owner, ownerType, prop.mName, i, propVal);
                    }
                    else
                    {
                        prop.SetBool(propVal, i);
                    }
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
                            if (ownerType == PropertyOwnerType::Node || ownerType == PropertyOwnerType::Asset)
                            {
                                am->EXE_EditProperty(owner, ownerType, prop.mName, i, sTempString);
                            }
                            else
                            {
                                prop.SetString(sTempString, i);
                            }
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
                    if (ownerType == PropertyOwnerType::Node || ownerType == PropertyOwnerType::Asset)
                    {
                        prop.SetVector2D(sOrigVal, i);
                        am->EXE_EditProperty(owner, ownerType, prop.mName, i, propVal);
                    }
                    else
                    {
                        prop.SetVector2D(propVal, i);
                    }
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
                    if (ownerType == PropertyOwnerType::Node || ownerType == PropertyOwnerType::Asset)
                    {
                        prop.SetVector(sOrigVal, i);

                        // Handle edge case where Rotation property is reset so we need to recompute transform to update mRotationEuler.
                        if (owner->As<Node3D>())
                        {
                            owner->As<Node3D>()->UpdateTransform(false);
                        }

                        am->EXE_EditProperty(owner, ownerType, prop.mName, i, propVal);
                    }
                    else
                    {
                        prop.SetVector(propVal, i);
                    }
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
                    if (ownerType == PropertyOwnerType::Node || ownerType == PropertyOwnerType::Asset)
                    {
                        prop.SetColor(sOrigVal, i);
                        am->EXE_EditProperty(owner, ownerType, prop.mName, i, propVal);
                    }
                    else
                    {
                        prop.SetColor(propVal, i);
                    }
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
                        if (ownerType == PropertyOwnerType::Node || ownerType == PropertyOwnerType::Asset)
                        {
                            am->EXE_EditProperty(owner, ownerType, prop.mName, i, (uint8_t)propVal);
                        }
                        else
                        {
                            prop.SetByte((uint8_t)propVal, i);
                        }
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

                            if (ownerType == PropertyOwnerType::Node || ownerType == PropertyOwnerType::Asset)
                            {
                                am->EXE_EditProperty(owner, ownerType, prop.mName, i, uint8_t(propVal));
                            }
                            else
                            {
                                prop.SetByte((uint8_t)propVal, i);
                            }
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
                        if (ownerType == PropertyOwnerType::Node || ownerType == PropertyOwnerType::Asset)
                        {
                            prop.SetByte((uint8_t)sOrigVal, i);
                            am->EXE_EditProperty(owner, ownerType, prop.mName, i, uint8_t(propVal));
                        }
                        else
                        {
                            prop.SetByte((uint8_t)propVal, i);
                        }
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

    if (catIndented)
    {
        ImGui::Unindent(kCategoryIndentWidth);
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

static void DrawImportMenu(Node* node)
{
    ActionManager* am = ActionManager::Get();
    if (ImGui::MenuItem("Camera"))
    {
        am->BeginImportCamera();

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

static void ItemRowsBackground(float lineHeight = -1.0f)
{
    auto* drawList = ImGui::GetWindowDrawList();
    const auto& style = ImGui::GetStyle();

    // Derive row color from theme's WindowBg — slightly lighter or darker for contrast
    ImVec4 windowBg = style.Colors[ImGuiCol_WindowBg];
    float luminance = windowBg.x * 0.299f + windowBg.y * 0.587f + windowBg.z * 0.114f;
    ImU32 rowColor;
    if (luminance < 0.5f)
        rowColor = ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 0.04f));
    else
        rowColor = ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 0.0f, 0.0f, 0.06f));

    if (lineHeight < 0)
    {
        lineHeight = ImGui::GetFrameHeight();
    }
    lineHeight += style.ItemSpacing.y;

    float scrollOffsetH = ImGui::GetScrollX();
    float scrollOffsetV = ImGui::GetScrollY();
    float scrolledOutLines = floorf(scrollOffsetV / lineHeight);
    scrollOffsetV -= lineHeight * scrolledOutLines;

    ImVec2 clipRectMin(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
    ImVec2 clipRectMax(clipRectMin.x + ImGui::GetWindowWidth(), clipRectMin.y + ImGui::GetWindowHeight());

    if (ImGui::GetScrollMaxX() > 0)
    {
        clipRectMax.y -= style.ScrollbarSize;
    }

    drawList->PushClipRect(clipRectMin, clipRectMax);

    bool isOdd = (static_cast<int>(scrolledOutLines) % 2) == 0;

    float yMin = clipRectMin.y - scrollOffsetV + ImGui::GetCursorPosY();
    float yMax = clipRectMax.y - scrollOffsetV + lineHeight;
    float xMin = clipRectMin.x + scrollOffsetH + ImGui::GetWindowContentRegionMin().x;
    float xMax = clipRectMin.x + scrollOffsetH + ImGui::GetWindowContentRegionMax().x;

    for (float y = yMin; y < yMax; y += lineHeight, isOdd = !isOdd)
    {
        if (isOdd)
        {
            drawList->AddRectFilled({ xMin, y - style.ItemSpacing.y }, { xMax, y + lineHeight }, rowColor);
        }
    }

    drawList->PopClipRect();
}

static void DrawScenePanel()
{
    ActionManager* am = ActionManager::Get();


    static std::string sFilterStrTemp;
    static std::string sFilterStr;
    bool filterCleared = false;

    if (ImGui::InputText(ICON_FILTER, &sFilterStrTemp, ImGuiInputTextFlags_EnterReturnsTrue))
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

            const char* nodeIcon = GetNodeIcon(node);
            std::string nodeLabel = std::string(nodeIcon) + " " + node->GetName();
            bool nodeOpen = ImGui::TreeNodeEx(nodeLabel.c_str(), nodeFlags);
            bool nodeClicked = ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen();
            bool nodeMiddleClicked = ImGui::IsItemClicked(ImGuiMouseButton_Middle);
            bool expandChildren = trackingNode || (nodeMiddleClicked && IsControlDown());
            bool collapseChildren = !expandChildren && nodeMiddleClicked;

            if (inSubScene || nodeHasScene)
            {
                ImGui::PopStyleColor();
            }

            // Drag source for node references (e.g. timeline track targets)
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
            {
                Node* dragNode = node;
                ImGui::SetDragDropPayload(DRAGDROP_NODE, &dragNode, sizeof(Node*));
                ImGui::Text("%s", node->GetName().c_str());
                ImGui::EndDragDropSource();
            }

            // Drop target for asset drag-and-drop (StaticMesh → hierarchy node)
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(DRAGDROP_ASSET))
                {
                    AssetStub* droppedStub = *(AssetStub**)payload->Data;
                    if (droppedStub != nullptr && droppedStub->mType == StaticMesh::GetStaticType())
                    {
                        sAssetDropStub = droppedStub;
                        sAssetDropParentNode = node;
                        sAssetDropInViewport = false;
                        sAssetDropPopupPos = ImGui::GetMousePos();
                        sAssetDropPopupPending = true;
                    }
                }
                ImGui::EndDragDropTarget();
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
                    strncpy(sPopupInputBuffer, node->GetName().c_str(), kPopupInputBufferSize - 1);
                    sPopupInputBuffer[kPopupInputBufferSize - 1] = '\0';
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
                if (!inSubScene && ImGui::Selectable("Replace Selected With Asset", false, ImGuiSelectableFlags_DontClosePopups))
                {
                    sReplaceAssetInput = "";
                    sReplaceAssetSuggestions.clear();

                    // Populate suggestions from asset map
                    std::unordered_map<std::string, AssetStub*>& assetMap = AssetManager::Get()->GetAssetMap();
                    for (auto& pair : assetMap)
                    {
                        AssetStub* s = pair.second;
                        if (s && (s->mType == StaticMesh::GetStaticType() ||
                                  s->mType == MaterialLite::GetStaticType() ||
                                  s->mType == MaterialBase::GetStaticType() ||
                                  s->mType == MaterialInstance::GetStaticType() ||
                                  s->mType == Scene::GetStaticType()))
                        {
                            sReplaceAssetSuggestions.push_back(s->mName);
                        }
                    }
                    std::sort(sReplaceAssetSuggestions.begin(), sReplaceAssetSuggestions.end());

                    ImGui::OpenPopup("Replace Selected With Asset");
                }
                {
                    // Check selection flags for instanced/static mesh items
                    const std::vector<Node*>& selNodes = GetEditorState()->GetSelectedNodes();
                    bool hasSelectedStaticMeshes = false;
                    bool hasSelectedInstancedMeshes = false;
                    for (Node* n : selNodes)
                    {
                        if (n->As<StaticMesh3D>() && !n->As<InstancedMesh3D>()) hasSelectedStaticMeshes = true;
                        if (n->As<InstancedMesh3D>()) hasSelectedInstancedMeshes = true;
                    }

                    if (!inSubScene && hasSelectedStaticMeshes &&
                        ImGui::Selectable("Merge Selected Into Instanced Mesh"))
                    {
                        am->EXE_ReplaceWithInstancedMesh(selNodes, true);
                    }
                    if (!inSubScene && hasSelectedStaticMeshes &&
                        ImGui::Selectable("Convert Selected To Instanced Mesh"))
                    {
                        am->EXE_ReplaceWithInstancedMesh(selNodes, false);
                    }
                    if (!inSubScene && hasSelectedInstancedMeshes &&
                        ImGui::Selectable("Replace Selected With Static Mesh"))
                    {
                        am->EXE_ReplaceWithStaticMesh(selNodes);
                    }
                }

                // Plugin/addon context menu items
                ImGui::Separator();
                {
                    EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
                    if (hookMgr)
                    {
                        hookMgr->DrawNodeContextItems();
                    }
                }

                if (!nodeSceneLinked && !inSubScene && ImGui::BeginMenu("Import"))
                {
                    DrawImportMenu(node);
                    ImGui::EndMenu();
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

                if (ImGui::BeginPopup("Replace Selected With Asset"))
                {
                    ImGui::Text("Enter asset name:");
                    bool inputEntered = ImGui::InputText("##ReplaceAsset", &sReplaceAssetInput, ImGuiInputTextFlags_EnterReturnsTrue);

                    // Autocomplete dropdown
                    auto filterFunc = [](const std::string& suggestion, const std::string& input) -> bool
                    {
                        if (input.empty()) return true;
                        std::string upperSugg = suggestion;
                        std::string upperInput = input;
                        for (char& c : upperSugg) c = (char)toupper(c);
                        for (char& c : upperInput) c = (char)toupper(c);
                        return upperSugg.find(upperInput) != std::string::npos;
                    };

                    bool autocompleteSelected = DrawAutocompleteDropdown(
                        "ReplaceAssetAutocomplete",
                        sReplaceAssetInput,
                        sReplaceAssetSuggestions,
                        filterFunc,
                        !sReplaceAssetInput.empty());

                    if (inputEntered || autocompleteSelected)
                    {
                        Asset* asset = AssetManager::Get()->LoadAsset(sReplaceAssetInput);
                        if (asset)
                        {
                            const std::vector<Node*>& selNodes = GetEditorState()->GetSelectedNodes();
                            am->EXE_ReplaceSelectedWithAsset(asset, selNodes);
                            ImGui::CloseCurrentPopup();
                            closeContextPopup = true;
                        }
                        else
                        {
                            LogWarning("Could not load asset: %s", sReplaceAssetInput.c_str());
                        }
                    }

                    if (ImGui::Button("Cancel"))
                    {
                        ImGui::CloseCurrentPopup();
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

    // Root-level drop target for asset drag-and-drop (empty space below tree)
    ImVec2 avail = ImGui::GetContentRegionAvail();
    if (avail.y > 0.0f)
    {
        ImGui::InvisibleButton("##SceneAssetDropTarget", ImVec2(avail.x, avail.y));
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(DRAGDROP_ASSET))
            {
                AssetStub* droppedStub = *(AssetStub**)payload->Data;
                if (droppedStub != nullptr && droppedStub->mType == StaticMesh::GetStaticType())
                {
                    sAssetDropStub = droppedStub;
                    sAssetDropParentNode = nullptr;
                    sAssetDropInViewport = false;
                    sAssetDropPopupPos = ImGui::GetMousePos();
                    sAssetDropPopupPending = true;
                }
            }
            ImGui::EndDragDropTarget();
        }
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
                strncpy(sPopupInputBuffer, selNodes[0]->GetName().c_str(), kPopupInputBufferSize - 1);
                sPopupInputBuffer[kPopupInputBufferSize - 1] = '\0';
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
}

static void DrawAssetsContextPopup(AssetStub* stub, AssetDir* dir)
{
    bool setTextInputFocus = false;
    bool closeContextPopup = false;
    static TypeId sNewAssetType = INVALID_TYPE_ID;

    ActionManager* actMan = ActionManager::Get();
    AssetManager* assMan = AssetManager::Get();

    AssetDir* curDir = GetEditorState()->GetAssetDirectory();

    bool readOnly = false;
    if ((stub && stub->mEngineAsset) ||
        (dir && dir->mEngineDir) ||
        (curDir && curDir->mAddonDir))
    {
        readOnly = true;
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

        if (ImGui::Selectable("Reimport Scene"))
        {
            actMan->BeginReimportScene(stub);
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


    if (!readOnly && (stub || dir))
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
            strncpy(sPopupInputBuffer, stub ? stub->mName.c_str() : dir->mName.c_str(), kPopupInputBufferSize - 1);
            sPopupInputBuffer[kPopupInputBufferSize - 1] = '\0';
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

    if (curDir && !curDir->mEngineDir && !curDir->mAddonDir)
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
            if (ImGui::Selectable("Timeline", false, ImGuiSelectableFlags_DontClosePopups))
            {
                sNewAssetType = Timeline::GetStaticType();
                showPopup = true;
            }

            ImGui::EndMenu();

            if (showPopup)
            {
                ImGui::OpenPopup("New Asset Name");
                sPopupInputBuffer[0] = '\0';
                setTextInputFocus = true;
            }
        }

        if (ImGui::Selectable("New Folder", false, ImGuiSelectableFlags_DontClosePopups))
        {
            ImGui::OpenPopup("New Folder");
            sPopupInputBuffer[0] = '\0';
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
                sPopupInputBuffer[0] = '\0';
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
                else if (sNewAssetType == Timeline::GetStaticType())
                    assetName = "TL_Timeline";
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

    // Plugin/addon context menu items
    ImGui::Separator();
    {
        EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
        if (hookMgr && stub)
        {
            const char* assetTypeName = Asset::GetNameFromTypeId(stub->mType);
            hookMgr->DrawAssetContextItems(assetTypeName ? assetTypeName : "");
        }
        else if (hookMgr)
        {
            hookMgr->DrawAssetContextItems("*");
        }
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
    std::string& filterStr = GetEditorState()->mTabFilterStr[GetEditorState()->ActiveTab()];
    std::vector<AssetStub*>& filteredStubs = GetEditorState()->mTabFilteredStubs[GetEditorState()->ActiveTab()];

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
                if (ImGui::Selectable(ICON_MATERIAL_SYMBOLS_FOLDER_OPEN_SHARP "...", false))
                {
                    GetEditorState()->SetAssetDirectory(currentDir->mParentDir, true);
                }
            }

            // Child Dirs
            for (uint32_t i = 0; i < currentDir->mChildDirs.size(); ++i)
            {
                AssetDir* childDir = currentDir->mChildDirs[i];

                std::string dirLabel = std::string(ICON_MATERIAL_SYMBOLS_FOLDER_SHARP) + "  " + childDir->mName;
                if (ImGui::Selectable(dirLabel.c_str(), false))
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

            std::string assetDispText = std::string(GetAssetIcon(stub->mType)) + "  " + stub->mName;
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
                    EditorUIHookManager::Get()->FireOnAssetOpen(stub->mName.c_str());

                    if (stub->mAsset == nullptr)
                        AssetManager::Get()->LoadAsset(*stub);

                    if (stub->mType == Scene::GetStaticType())
                    {
                        Scene* scene = stub->mAsset ? stub->mAsset->As<Scene>() : nullptr;
                        if (scene)
                            ActionManager::Get()->OpenScene(scene);
                    }
                    else if (stub->mType == Timeline::GetStaticType())
                    {
                        Timeline* timeline = stub->mAsset ? stub->mAsset->As<Timeline>() : nullptr;
                        if (timeline)
                        {
                            OpenTimelineForEditing(timeline);
                        }
                    }
                    else
                    {
                        if (stub->mAsset)
                            GetEditorState()->InspectObject(stub->mAsset);
                    }

                    EditorUIHookManager::Get()->FireOnAssetOpened(stub->mName.c_str());
                }
            }

            if (stub->mType == StaticMesh::GetStaticType() &&
                ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
            {
                AssetStub* dragStub = stub;
                ImGui::SetDragDropPayload(DRAGDROP_ASSET, &dragStub, sizeof(AssetStub*));
                ImGui::Text("%s", stub->mName.c_str());
                ImGui::EndDragDropSource();
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

        if (currentDir != nullptr && !currentDir->mAddonDir)
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
    }

    if (interactive && ImGui::BeginPopup("Null Context"))
    {
        DrawAssetsContextPopup(nullptr, nullptr);
        ImGui::EndPopup();
    }
}

static void DrawAssetsPanel()
{
    const float kAssetPanelIndent = 4.0f;

    if (ImGui::BeginTabBar("AssetBrowserTabs"))
    {
        if (ImGui::BeginTabItem("Project"))
        {
            if (GetEditorState()->mActiveAssetTab != AssetBrowserTab::Project)
                GetEditorState()->mActiveAssetTab = AssetBrowserTab::Project;
            ImGui::Indent(kAssetPanelIndent);
            DrawAssetBrowser(true, true);
            ImGui::Unindent(kAssetPanelIndent);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Addons"))
        {
            if (GetEditorState()->mActiveAssetTab != AssetBrowserTab::Addons)
            {
                GetEditorState()->mActiveAssetTab = AssetBrowserTab::Addons;
                // Init addons tab to Packages root if not set
                if (GetEditorState()->mTabCurrentDir[(int)AssetBrowserTab::Addons] == nullptr)
                    GetEditorState()->mTabCurrentDir[(int)AssetBrowserTab::Addons] = AssetManager::Get()->FindPackagesDirectory();
            }
            ImGui::Indent(kAssetPanelIndent);
            DrawAssetBrowser(true, true);
            ImGui::Unindent(kAssetPanelIndent);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
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
    sObjectTabOpen = false;

    if (ImGui::BeginTabBar("PropertyModeTabs"))
    {
        if (ImGui::BeginTabItem(ICON_FLUENT_MDL2_CUBE_SHAPE "  Object"))
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
                Node* objNode = obj->As<Node>();
                if (objNode)
                    objName = std::string(GetNodeIcon(objNode)) + " " + objName;
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
        if (ImGui::BeginTabItem(ICON_SCENE "  Scene"))
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
        if (ImGui::BeginTabItem(ICON_GLOBE "  Global"))
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

}

// ===== Scripts Panel =====

struct ScriptFileEntry
{
    std::string mDisplayName; // relative name for display (e.g. "MyDir/Script.lua")
    std::string mFullPath;   // full path for opening
    std::string mOrigin;     // "Engine", "Project", or package name
};

struct CppAddonEntry
{
    std::string mAddonId;
    std::string mAddonName;
    std::string mSourcePath;
    std::string mVcxprojPath;
    std::vector<std::string> mSourceFiles; // full paths
};

static void GatherCppSourceFiles(const std::string& dir, std::vector<std::string>& outFiles)
{
    DirEntry dirEntry;
    SYS_OpenDirectory(dir, dirEntry);
    if (!dirEntry.mValid)
        return;

    std::vector<std::string> subDirs;
    while (dirEntry.mValid)
    {
        if (strcmp(dirEntry.mFilename, ".") != 0 && strcmp(dirEntry.mFilename, "..") != 0)
        {
            std::string path = dir + dirEntry.mFilename;
            if (dirEntry.mDirectory)
            {
                subDirs.push_back(path + "/");
            }
            else
            {
                const char* ext = strrchr(dirEntry.mFilename, '.');
                if (ext != nullptr &&
                    (strcmp(ext, ".cpp") == 0 || strcmp(ext, ".c") == 0 ||
                     strcmp(ext, ".h") == 0 || strcmp(ext, ".hpp") == 0))
                {
                    outFiles.push_back(path);
                }
            }
        }
        SYS_IterateDirectory(dirEntry);
    }
    SYS_CloseDirectory(dirEntry);

    for (const std::string& sub : subDirs)
    {
        GatherCppSourceFiles(sub, outFiles);
    }
}

static void DrawScriptsPanel()
{
    // --- Cached Lua scripts ---
    static std::vector<ScriptFileEntry> sLuaScripts;
    static double sLuaLastUpdate = 0.0;

    // --- Cached C++ addons ---
    static std::vector<CppAddonEntry> sCppAddons;
    static double sCppLastUpdate = 0.0;

    // --- Search filter ---
    static char sSearchBuffer[256] = "";

    double currentTime = ImGui::GetTime();

    // Search bar at top
    ImGui::SetNextItemWidth(-1);
    ImGui::InputTextWithHint("##ScriptsSearch", "Search scripts...", sSearchBuffer, sizeof(sSearchBuffer));
    std::string filterLower;
    if (sSearchBuffer[0] != '\0')
    {
        filterLower = sSearchBuffer;
        std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(), ::tolower);
    }

    ImGui::Spacing();

    if (ImGui::BeginTabBar("ScriptBrowserTabs"))
    {
        // ===== Lua Scripts Tab =====
        if (ImGui::BeginTabItem("Lua Scripts"))
        {
            // Toolbar buttons
            {
                bool hasProject = !GetEngineState()->mProjectDirectory.empty();
                PreferencesModule* mod = PreferencesManager::Get()->FindModule("External/Editors");
                EditorsModule* editors = static_cast<EditorsModule*>(mod);

                if (!hasProject) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
                if (ImGui::Button(ICON_STREAMLINE_SHARP_NEW_FILE_REMIX "##NewScript"))
                {
                    if (hasProject)
                        OpenCreateScriptDialog();
                }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                    ImGui::SetTooltip("New Script");

                ImGui::SameLine();
                bool editorConfigured = editors && editors->IsLuaEditorConfigured();
                if (!editorConfigured) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
                if (ImGui::Button(ICON_IX_CODE "##OpenLuaEditor"))
                {
                    if (hasProject && editorConfigured)
                        editors->OpenLuaScript(GetEngineState()->mProjectDirectory + "Scripts/");
                }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                    ImGui::SetTooltip("Open in Editor");
                if (!editorConfigured) ImGui::PopStyleVar();

                ImGui::SameLine();
                if (ImGui::Button(ICON_MATERIAL_SYMBOLS_FOLDER_OPEN_SHARP "##OpenLuaExplorer"))
                {
                    if (hasProject)
                    {
                        std::string scriptsDir = GetEngineState()->mProjectDirectory + "Scripts/";
                        SYS_CreateDirectory(scriptsDir.c_str());
#if PLATFORM_WINDOWS
                        SYS_Exec(("start \"\" \"" + scriptsDir + "\"").c_str());
#elif PLATFORM_LINUX
                        SYS_Exec(("xdg-open \"" + scriptsDir + "\" &").c_str());
#endif
                    }
                }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                    ImGui::SetTooltip("Open in Explorer");
                if (!hasProject) ImGui::PopStyleVar();

                ImGui::Spacing();
            }

            // Refresh every 2 seconds
            if (sLuaScripts.empty() || currentTime - sLuaLastUpdate > 2.0)
            {
                sLuaScripts.clear();
                const std::string& projectDir = GetEngineState()->mProjectDirectory;

                // Engine scripts
                {
                    std::vector<std::string> files;
                    AssetManager::Get()->GatherScriptFiles("Engine/Scripts/", files);
                    const std::string prefix = "Engine/Scripts/";
                    for (const std::string& f : files)
                    {
                        ScriptFileEntry entry;
                        entry.mFullPath = f;
                        entry.mOrigin = "Engine";
                        entry.mDisplayName = (f.length() > prefix.length() && f.substr(0, prefix.length()) == prefix)
                            ? f.substr(prefix.length()) : f;
                        sLuaScripts.push_back(entry);
                    }
                }

                // Project scripts
                if (!projectDir.empty())
                {
                    std::string projScriptsDir = projectDir + "Scripts/";
                    std::vector<std::string> files;
                    AssetManager::Get()->GatherScriptFiles(projScriptsDir, files);
                    for (const std::string& f : files)
                    {
                        ScriptFileEntry entry;
                        entry.mFullPath = f;
                        entry.mOrigin = "Project";
                        entry.mDisplayName = (f.length() > projScriptsDir.length() && f.substr(0, projScriptsDir.length()) == projScriptsDir)
                            ? f.substr(projScriptsDir.length()) : f;
                        sLuaScripts.push_back(entry);
                    }

                    // Package scripts
                    std::string packagesDir = projectDir + "Packages/";
                    DirEntry pkgDirEntry;
                    SYS_OpenDirectory(packagesDir, pkgDirEntry);
                    while (pkgDirEntry.mValid)
                    {
                        if (pkgDirEntry.mDirectory &&
                            strcmp(pkgDirEntry.mFilename, ".") != 0 &&
                            strcmp(pkgDirEntry.mFilename, "..") != 0)
                        {
                            std::string pkgName = pkgDirEntry.mFilename;
                            std::string pkgScriptsDir = packagesDir + pkgName + "/Scripts/";
                            std::vector<std::string> pkgFiles;
                            AssetManager::Get()->GatherScriptFiles(pkgScriptsDir, pkgFiles);
                            for (const std::string& f : pkgFiles)
                            {
                                ScriptFileEntry entry;
                                entry.mFullPath = f;
                                entry.mOrigin = pkgName;
                                entry.mDisplayName = (f.length() > pkgScriptsDir.length() && f.substr(0, pkgScriptsDir.length()) == pkgScriptsDir)
                                    ? "Packages/" + pkgName + "/" + f.substr(pkgScriptsDir.length()) : f;
                                sLuaScripts.push_back(entry);
                            }
                        }
                        SYS_IterateDirectory(pkgDirEntry);
                    }
                    SYS_CloseDirectory(pkgDirEntry);
                }

                sLuaLastUpdate = currentTime;
            }

            // Build directory tree
            // Map: directory path -> list of entries
            struct TreeNode
            {
                std::string name;
                std::map<std::string, TreeNode> children;
                std::vector<const ScriptFileEntry*> files;
            };

            TreeNode root;
            bool hasFilter = !filterLower.empty();

            for (const ScriptFileEntry& entry : sLuaScripts)
            {
                // Filter check
                if (hasFilter)
                {
                    std::string nameLower = entry.mDisplayName;
                    std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
                    if (nameLower.find(filterLower) == std::string::npos)
                        continue;
                }

                // Split path into components
                std::string path = entry.mDisplayName;
                TreeNode* current = &root;
                size_t pos = 0;
                size_t slash;
                while ((slash = path.find('/', pos)) != std::string::npos)
                {
                    std::string dir = path.substr(pos, slash - pos);
                    if (!dir.empty())
                    {
                        current = &current->children[dir];
                        current->name = dir;
                    }
                    pos = slash + 1;
                }
                // Remaining part is the file name
                current->files.push_back(&entry);
            }

            // Recursive draw function
            std::function<void(const TreeNode&)> drawTree;
            drawTree = [&](const TreeNode& node)
            {
                // Draw subdirectories
                for (auto& pair : node.children)
                {
                    ImGuiTreeNodeFlags dirFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
                    if (hasFilter)
                        dirFlags |= ImGuiTreeNodeFlags_DefaultOpen;

                    if (ImGui::TreeNodeEx(pair.first.c_str(), dirFlags))
                    {
                        drawTree(pair.second);
                        ImGui::TreePop();
                    }
                }

                // Draw files
                for (const ScriptFileEntry* entry : node.files)
                {
                    // Extract just the filename
                    std::string fileName = entry->mDisplayName;
                    size_t lastSlash = fileName.find_last_of('/');
                    if (lastSlash != std::string::npos)
                        fileName = fileName.substr(lastSlash + 1);

                    ImGuiTreeNodeFlags leafFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                    ImGui::TreeNodeEx(fileName.c_str(), leafFlags);

                    if (ImGui::IsItemHovered())
                    {
                        ImGui::SetTooltip("%s\n(%s)", entry->mDisplayName.c_str(), entry->mOrigin.c_str());
                    }

                    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
                    {
                        PreferencesModule* mod = PreferencesManager::Get()->FindModule("External/Editors");
                        EditorsModule* editors = static_cast<EditorsModule*>(mod);
                        if (editors)
                        {
                            editors->OpenLuaScript(entry->mFullPath);
                        }
                    }
                }
            };

            ImGui::BeginChild("##LuaScriptsList", ImVec2(0, 0), false);
            drawTree(root);
            ImGui::EndChild();

            ImGui::EndTabItem();
        }

        // ===== C++ Addons Tab =====
        if (ImGui::BeginTabItem("C++ Addons"))
        {
            NativeAddonManager* nam = NativeAddonManager::Get();

            // Toolbar buttons
            {
                bool hasProject = !GetEngineState()->mProjectDirectory.empty();
                bool hasAddons = nam && !nam->GetDiscoveredAddonIds().empty();
                PreferencesModule* mod = PreferencesManager::Get()->FindModule("External/Editors");
                EditorsModule* editors = static_cast<EditorsModule*>(mod);

                if (!hasProject) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
                if (ImGui::Button(ICON_IC_SHARP_EXTENSION "##NewNativeAddon"))
                {
                    if (hasProject)
                        OpenCreateNativeAddonDialog();
                }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                    ImGui::SetTooltip("New Native Addon");

                ImGui::SameLine();
                if (!hasAddons) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
                if (ImGui::Button(ICON_STREAMLINE_SHARP_NEW_FILE_REMIX "##NewCppFile"))
                {
                    if (hasProject && hasAddons)
                        OpenCreateCppFileDialog();
                }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                    ImGui::SetTooltip("New C++ File");
                if (!hasAddons) ImGui::PopStyleVar();

                ImGui::SameLine();
                bool cppConfigured = editors && editors->IsCppEditorConfigured();
                if (!cppConfigured || !hasAddons) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
                if (ImGui::Button(ICON_IX_CODE "##OpenCppEditor"))
                {
                    if (hasProject && hasAddons && cppConfigured)
                    {
                        // Open first addon's source in IDE
                        std::vector<std::string> addonIds = nam->GetDiscoveredAddonIds();
                        for (const std::string& id : addonIds)
                        {
                            const NativeAddonState* state = nam->GetState(id);
                            if (state && state->mNativeMetadata.mHasNative)
                            {
                                std::string sourceDir = state->mSourcePath + state->mNativeMetadata.mSourceDir + "/";
                                std::string binaryName = state->mNativeMetadata.mBinaryName.empty() ? id : state->mNativeMetadata.mBinaryName;
                                std::string vcxproj = state->mSourcePath + binaryName + ".vcxproj";
                                editors->OpenCppFile(sourceDir, vcxproj);
                                break;
                            }
                        }
                    }
                }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                    ImGui::SetTooltip("Open in IDE");
                if (!cppConfigured || !hasAddons) ImGui::PopStyleVar();

                ImGui::SameLine();
                if (ImGui::Button(ICON_MATERIAL_SYMBOLS_FOLDER_OPEN_SHARP "##OpenCppExplorer"))
                {
                    if (hasProject)
                    {
                        std::string packagesDir = GetEngineState()->mProjectDirectory + "Packages/";
#if PLATFORM_WINDOWS
                        SYS_Exec(("start \"\" \"" + packagesDir + "\"").c_str());
#elif PLATFORM_LINUX
                        SYS_Exec(("xdg-open \"" + packagesDir + "\" &").c_str());
#endif
                    }
                }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                    ImGui::SetTooltip("Open in Explorer");
                if (!hasProject) ImGui::PopStyleVar();

                ImGui::Spacing();
            }

            // Refresh every 2 seconds
            if (nam && (sCppAddons.empty() || currentTime - sCppLastUpdate > 2.0))
            {
                sCppAddons.clear();
                std::vector<std::string> addonIds = nam->GetDiscoveredAddonIds();

                for (const std::string& id : addonIds)
                {
                    const NativeAddonState* state = nam->GetState(id);
                    if (!state || !state->mNativeMetadata.mHasNative)
                        continue;

                    CppAddonEntry addon;
                    addon.mAddonId = id;
                    addon.mAddonName = state->mNativeMetadata.mBinaryName.empty() ? id : state->mNativeMetadata.mBinaryName;
                    addon.mSourcePath = state->mSourcePath;
                    addon.mVcxprojPath = state->mSourcePath + addon.mAddonName + ".vcxproj";

                    // Gather source files
                    std::string sourceDir = state->mSourcePath + state->mNativeMetadata.mSourceDir + "/";
                    GatherCppSourceFiles(sourceDir, addon.mSourceFiles);
                    std::sort(addon.mSourceFiles.begin(), addon.mSourceFiles.end());

                    sCppAddons.push_back(addon);
                }

                sCppLastUpdate = currentTime;
            }

            ImGui::BeginChild("##CppAddonsList", ImVec2(0, 0), false);

            if (!nam)
            {
                ImGui::TextDisabled("Native addon manager not available.");
            }
            else if (sCppAddons.empty())
            {
                ImGui::TextDisabled("No native addons discovered.");
            }
            else
            {
                for (const CppAddonEntry& addon : sCppAddons)
                {
                    ImGuiTreeNodeFlags addonFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

                    bool addonVisible = true;
                    if (!filterLower.empty())
                    {
                        // Check if addon name or any source file matches
                        addonVisible = false;
                        std::string nameLower = addon.mAddonName;
                        std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
                        if (nameLower.find(filterLower) != std::string::npos)
                        {
                            addonVisible = true;
                        }
                        else
                        {
                            for (const std::string& sf : addon.mSourceFiles)
                            {
                                std::string sfLower = sf;
                                std::transform(sfLower.begin(), sfLower.end(), sfLower.begin(), ::tolower);
                                if (sfLower.find(filterLower) != std::string::npos)
                                {
                                    addonVisible = true;
                                    break;
                                }
                            }
                        }
                    }

                    if (!addonVisible)
                        continue;

                    if (!filterLower.empty())
                        addonFlags |= ImGuiTreeNodeFlags_DefaultOpen;

                    if (ImGui::TreeNodeEx(addon.mAddonId.c_str(), addonFlags, "%s", addon.mAddonName.c_str()))
                    {
                        for (const std::string& srcFile : addon.mSourceFiles)
                        {
                            // Extract just the filename
                            std::string fileName = srcFile;
                            size_t lastSlash = fileName.find_last_of("/\\");
                            if (lastSlash != std::string::npos)
                                fileName = fileName.substr(lastSlash + 1);

                            // Filter individual files
                            if (!filterLower.empty())
                            {
                                std::string fileLower = fileName;
                                std::transform(fileLower.begin(), fileLower.end(), fileLower.begin(), ::tolower);
                                if (fileLower.find(filterLower) == std::string::npos)
                                    continue;
                            }

                            ImGuiTreeNodeFlags leafFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                            ImGui::TreeNodeEx(srcFile.c_str(), leafFlags, "%s", fileName.c_str());

                            if (ImGui::IsItemHovered())
                            {
                                ImGui::SetTooltip("%s", srcFile.c_str());
                            }

                            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
                            {
                                PreferencesModule* mod = PreferencesManager::Get()->FindModule("External/Editors");
                                EditorsModule* editors = static_cast<EditorsModule*>(mod);
                                if (editors)
                                {
                                    editors->OpenCppFile(srcFile, addon.mVcxprojPath);
                                }
                            }
                        }
                        ImGui::TreePop();
                    }
                }
            }

            ImGui::EndChild();

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}

static void DrawMainMenuBar()
{
    Renderer* renderer = Renderer::Get();
    ActionManager* am = ActionManager::Get();

    bool openSaveSceneAsModal = false;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            EditScene* editScene = GetEditorState()->GetEditScene();

            if (ImGui::MenuItem("Project Select..."))
                GetProjectSelectWindow()->Open();
            ImGui::Separator();
            if (ImGui::MenuItem("Open Project"))
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
            if (ImGui::MenuItem("New Project"))
                am->CreateNewProject();
            if (ImGui::MenuItem("New C++ Project"))
                am->CreateNewProject(nullptr, true);
            if (ImGui::MenuItem("New Scene"))
                GetEditorState()->OpenEditScene(nullptr);
            if (editScene && ImGui::MenuItem("Save Scene"))
            {
                Scene* scene = editScene->mSceneAsset.Get<Scene>();
                AssetStub* sceneStub = scene ? AssetManager::Get()->GetAssetStub(scene->GetName()) : nullptr;
                if (sceneStub != nullptr)
                {
                    GetEditorState()->CaptureAndSaveScene(sceneStub, nullptr);
                }
                else
                {
                    openSaveSceneAsModal = true;
                    sPopupInputBuffer[0] = '\0';
                }
            }
            if (editScene && ImGui::MenuItem("Save Scene As..."))
            {
                openSaveSceneAsModal = true;
                sPopupInputBuffer[0] = '\0';
            }
            if (ImGui::MenuItem("Import Asset"))
                am->ImportAsset();
            if (ImGui::MenuItem("Import Scene"))
                am->BeginImportScene();
            if (ImGui::MenuItem("Run Script"))
                am->RunScript();
            if (ImGui::MenuItem("Recapture All Scenes"))
                am->RecaptureAndSaveAllScenes();
            if (ImGui::MenuItem("Resave All Assets"))
                am->ResaveAllAssets();
            if (ImGui::MenuItem("Reload All Scripts"))
            {
                ReloadAllScripts();

                NativeAddonManager* nam = NativeAddonManager::Get();
                if (nam != nullptr)
                {
                    std::vector<std::string> localIds = nam->GetLocalPackageIds();
                    for (const std::string& id : localIds)
                    {
                        std::string addonPath = nam->GetAddonSourcePath(id);
                        if (!addonPath.empty())
                        {
                            nam->GenerateIDEConfig(addonPath);
                        }
                    }

                    nam->ReloadAllNativeAddons();
                    LogDebug("Native addon dependencies regenerated and addons reloaded.");
                }
            }

            // Script Hot-Reload toggle
            bool hotReloadEnabled = IsScriptHotReloadEnabled();
            std::string hotReloadText = hotReloadEnabled ? "Disable Script Hot-Reload" : "Enable Script Hot-Reload";
            if (ImGui::MenuItem(hotReloadText.c_str()))
            {
                SetScriptHotReloadEnabled(!hotReloadEnabled);
                WriteEngineConfig();
            }

            if (ImGui::MenuItem("Write Config"))
                WriteEngineConfig();
            if (ImGui::MenuItem("Packaging..."))
            {
                GetPackagingWindow()->Open();
            }
            if (ImGui::BeginMenu("Package Project"))
            {
                DrawPackageMenu();
                ImGui::EndMenu();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Addons..."))
            {
                GetAddonsWindow()->Open();
            }

            // Draw plugin menu items for File menu
            {
                EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
                if (hookMgr != nullptr) hookMgr->DrawMenuItems("File");
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo"))
                am->Undo();
            if (ImGui::MenuItem("Redo"))
                am->Redo();

            // Draw plugin menu items for Edit menu
            {
                EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
                if (hookMgr != nullptr) hookMgr->DrawMenuItems("Edit");
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            Camera3D* cam = GetEditorState()->GetEditorCamera();
            ProjectionMode projMode = cam ? cam->GetProjectionMode() : ProjectionMode::PERSPECTIVE;
            if ((projMode == ProjectionMode::PERSPECTIVE && ImGui::MenuItem("Orthographic")) ||
                (projMode == ProjectionMode::ORTHOGRAPHIC && ImGui::MenuItem("Perspective")))
            {
                GetEditorState()->ToggleEditorCameraProjection();
            }
            if (ImGui::MenuItem("Wireframe"))
                renderer->SetDebugMode(renderer->GetDebugMode() == DEBUG_WIREFRAME ? DEBUG_NONE : DEBUG_WIREFRAME);
            if (ImGui::MenuItem("Collision"))
                renderer->SetDebugMode(renderer->GetDebugMode() == DEBUG_COLLISION ? DEBUG_NONE : DEBUG_COLLISION);
            if (ImGui::MenuItem("Proxy"))
                renderer->EnableProxyRendering(!renderer->IsProxyRenderingEnabled());
            if (ImGui::MenuItem("Spline Lines"))
                Spline3D::SetSplineLinesVisible(!Spline3D::IsSplineLinesVisible());
            if (ImGui::MenuItem("Bounds"))
            {
                uint32_t newMode = (uint32_t(renderer->GetBoundsDebugMode()) + 1) % uint32_t(BoundsDebugMode::Count);
                renderer->SetBoundsDebugMode((BoundsDebugMode)newMode);
            }
            if (ImGui::MenuItem("Grid"))
                ToggleGrid();
            if (ImGui::MenuItem("Stats"))
                renderer->EnableStatsOverlay(!renderer->IsStatsOverlayEnabled());
            if (ImGui::MenuItem("Preview Lighting"))
            {
                GetEditorState()->mPreviewLighting = !GetEditorState()->mPreviewLighting;
                LogDebug("Preview lighting %s", GetEditorState()->mPreviewLighting ? "enabled." : "disabled.");
            }

            if (GetEditorState()->GetEditorMode() == EditorMode::Scene2D)
            {
                if (ImGui::MenuItem("Reset 2D Viewport"))
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

            // TODO: Uncomment to show Preferences
            if (GetFeatureFlagsEditor().mShowPreferences == true) {
                ImGui::Separator();
                if (ImGui::MenuItem("Preferences..."))
                {
                    GetPreferencesWindow()->Open();
                }
            }

            if (GetFeatureFlagsEditor().mShowTheming == true) {
                if (ImGui::MenuItem("Theme Editor..."))
                {
                    GetThemeEditorWindow()->Open();
                }
            }

            ImGui::Separator();
            if (ImGui::MenuItem("Scene"))
                GetEditorState()->mShowLeftPane = !GetEditorState()->mShowLeftPane;
            if (ImGui::MenuItem("Assets"))
                GetEditorState()->mShowLeftPane = !GetEditorState()->mShowLeftPane;
            if (ImGui::MenuItem("Properties"))
                GetEditorState()->mShowRightPane = !GetEditorState()->mShowRightPane;
            if (ImGui::MenuItem("Debug Log"))
                GetEditorState()->mShowBottomPane = !GetEditorState()->mShowBottomPane;

            if (ImGui::MenuItem("Timeline"))
                GetEditorState()->mShowTimelinePanel = !GetEditorState()->mShowTimelinePanel;

            ImGui::Separator();
            if (ImGui::MenuItem("Reset Layout"))
                sDockResetRequested = true;

            // Draw plugin menu items for View menu
            {
                EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
                if (hookMgr != nullptr) hookMgr->DrawMenuItems("View");
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("World"))
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
            if (ImGui::MenuItem("Clear World"))
                am->DeleteAllNodes();
            if (ImGui::MenuItem("Bake Lighting"))
                renderer->BeginLightBake();
            if (ImGui::MenuItem("Clear Baked Lighting"))
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
            if (ImGui::MenuItem("Toggle Transform Mode"))
                GetEditorState()->GetViewport3D()->ToggleTransformMode();

            // Draw plugin menu items for World menu
            {
                EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
                if (hookMgr != nullptr) hookMgr->DrawMenuItems("World");
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Developer"))
        {
            if (ImGui::MenuItem("Reload Native Addons"))
            {
                NativeAddonManager* nam = NativeAddonManager::Get();
                if (nam != nullptr)
                {
                    nam->ReloadAllNativeAddons();
                    LogDebug("Native addons reloaded.");
                }
            }

            if (ImGui::MenuItem("Discover Native Addons"))
            {
                NativeAddonManager* nam = NativeAddonManager::Get();
                if (nam != nullptr)
                {
                    nam->DiscoverNativeAddons();
                    LogDebug("Native addons discovered.");
                }
            }

            if (ImGui::MenuItem("Regenerate Native Addon Dependencies"))
            {
                NativeAddonManager* nam = NativeAddonManager::Get();
                if (nam != nullptr)
                {
                    std::vector<std::string> localIds = nam->GetLocalPackageIds();
                    for (const std::string& id : localIds)
                    {
                        std::string addonPath = nam->GetAddonSourcePath(id);
                        if (!addonPath.empty())
                        {
                            nam->GenerateIDEConfig(addonPath);
                        }
                    }
                    LogDebug("Native addon dependencies regenerated for %d addon(s).", (int)localIds.size());
                }
            }

            ImGui::Separator();

            // Draw plugin menu items for Developer menu
            EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
            if (hookMgr != nullptr)
            {
                hookMgr->DrawMenuItems("Developer");
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Addons"))
        {
            DrawAddonsPopupContent();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Extra"))
        {
            char versionStr[32];
            snprintf(versionStr, 31, "Version: %d", OCTAVE_VERSION);
            ImGui::MenuItem(versionStr, nullptr, false, false);

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
                ImGui::MenuItem("Prepare Release"))
            {
                ActionManager::Get()->PrepareRelease();
            }

            // Draw plugin menu items for Extra menu
            {
                EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
                if (hookMgr != nullptr) hookMgr->DrawMenuItems("Extra");
            }

            ImGui::EndMenu();
        }

        // Draw addon top-level menus as main menu bar entries
        {
            EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
            if (hookMgr != nullptr)
            {
                hookMgr->DrawTopLevelMenus();
            }
        }

        // -- Toolbar items (merged into menu bar) --
        ImGui::SameLine(0.0f, 20.0f);
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine(0.0f, 20.0f);

        // Editor mode combo
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
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

        EditorState* edState = GetEditorState();
        ImGuizmo::OPERATION& gizmoOp = edState->mGizmoOperation;
        ImGuizmo::MODE& gizmoMode = edState->mGizmoMode;

        // Translate button
        bool isTranslate = (gizmoOp == ImGuizmo::TRANSLATE);
        if (isTranslate) ImGui::PushStyleColor(ImGuiCol_Button, kSelectedColor);
        if (ImGui::Button(ICON_BX_MOVE))
            gizmoOp = ImGuizmo::TRANSLATE;
        if (isTranslate) ImGui::PopStyleColor();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Translate (Space+G)");

        // Rotate button
        ImGui::SameLine();
        bool isRotate = (gizmoOp == ImGuizmo::ROTATE);
        if (isRotate) ImGui::PushStyleColor(ImGuiCol_Button, kSelectedColor);
        if (ImGui::Button(ICON_LUCIDE_ROTATE_3D))
            gizmoOp = ImGuizmo::ROTATE;
        if (isRotate) ImGui::PopStyleColor();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Rotate (Space+R)");

        // Scale button
        ImGui::SameLine();
        bool isScale = (gizmoOp == ImGuizmo::SCALE);
        if (isScale) ImGui::PushStyleColor(ImGuiCol_Button, kSelectedColor);
        if (ImGui::Button(ICON_MAGE_SCALE_UP))
            gizmoOp = ImGuizmo::SCALE;
        if (isScale) ImGui::PopStyleColor();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Scale (Space+S)");

        // Local/World mode toggle
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

        bool isLocal = (gizmoMode == ImGuizmo::LOCAL);
        if (isLocal) ImGui::PushStyleColor(ImGuiCol_Button, kToggledColor);
        if (ImGui::Button(isLocal ? ICON_MDI_AXIS_ARROW "  Local" : ICON_GLOBE "  World"))
            gizmoMode = isLocal ? ImGuizmo::WORLD : ImGuizmo::LOCAL;
        if (isLocal) ImGui::PopStyleColor();
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Toggle Local/World (Ctrl+T)");

        // Scene tabs
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

            for (int32_t n = 0; n < (int32_t)scenes.size(); n++)
            {
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

            if (!sceneJustChanged &&
                openedTab != activeSceneIdx)
            {
                GetEditorState()->OpenEditScene(openedTab);
            }

            ImGui::EndTabBar();
        }

        sPrevActiveSceneIdx = activeSceneIdx;

        // -- Right-aligned buttons: [Addon Toolbar Items] [Hammer] [Play ▾] --
        {
            enum class PlayTarget { PlayInEditor, Dolphin, Azahar, Standalone, Send3dsLink, Count };
            static PlayTarget sPlayTarget = PlayTarget::PlayInEditor;

            const char* playTargetLabels[] = {
                "Play In Editor",
                "Play on Dolphin",
                "Play on Azahar",
                "Play Standalone",
                "Send 3dsLink",
            };

            const ImGuiStyle& style = ImGui::GetStyle();
            float buttonSize = ImGui::GetFrameHeight();
            float spacing = style.ItemSpacing.x;
            bool inPie = IsPlayingInEditor();

            // Build the play button label: "icon TargetName"
            const char* targetLabel = playTargetLabels[(int)sPlayTarget];
            char playBtnLabel[128];
            snprintf(playBtnLabel, sizeof(playBtnLabel), "%s %s", inPie ? ICON_IC_BASELINE_STOP : ICON_MDI_PLAY, inPie ? "Stop" : targetLabel);

            float playBtnWidth = ImGui::CalcTextSize(playBtnLabel).x + style.FramePadding.x * 2.0f;
            float arrowBtnWidth = ImGui::CalcTextSize(ICON_DASHICONS_ARROW_DOWN).x + style.FramePadding.x * 2.0f;
            float playGroupWidth = inPie ? playBtnWidth : (playBtnWidth + arrowBtnWidth);
            float rightGroupWidth = buttonSize + spacing + playGroupWidth + 20.0f;
            float targetX = ImGui::GetWindowWidth() - rightGroupWidth;
            float cursorX = ImGui::GetCursorPosX();
            if (targetX > cursorX)
                ImGui::SameLine(targetX);

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

            // Hammer button
            if (ImGui::Button(ICON_ION_HAMMER_SHARP))
            {
                ReloadAllScripts();
                NativeAddonManager* nam = NativeAddonManager::Get();
                if (nam != nullptr)
                {
                    std::vector<std::string> localIds = nam->GetLocalPackageIds();
                    for (const std::string& id : localIds)
                    {
                        std::string addonPath = nam->GetAddonSourcePath(id);
                        if (!addonPath.empty())
                        {
                            nam->GenerateIDEConfig(addonPath);
                        }
                    }

                    nam->ReloadAllNativeAddons();
                    LogDebug("Native addon dependencies regenerated and addons reloaded.");
                }
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Refresh Scripts (Ctrl+R)");
            }

            // Play/Stop button with target label
            ImGui::SameLine();
            if (ImGui::Button(playBtnLabel))
            {
                if (inPie)
                {
                    GetEditorState()->EndPlayInEditor();
                }
                else
                {
                    switch (sPlayTarget)
                    {
                    case PlayTarget::PlayInEditor:
                        GetEditorState()->BeginPlayInEditor();
                        break;
                    case PlayTarget::Dolphin:
                        GetPackagingWindow()->BuildAndRunWithProfile(Platform::GameCube, true, false);
                        break;
                    case PlayTarget::Azahar:
                        GetPackagingWindow()->BuildAndRunWithProfile(Platform::N3DS, true, false);
                        break;
                    case PlayTarget::Standalone:
                        GetPackagingWindow()->BuildAndRunWithProfile(Platform::Windows, false, false);
                        break;
                    case PlayTarget::Send3dsLink:
                        GetPackagingWindow()->BuildAndRunWithProfile(Platform::N3DS, true, true);
                        break;
                    default:
                        break;
                    }
                }
            }

            // Dropdown arrow button (hidden during PIE since stop is the only action)
            if (!inPie)
            {
                ImGui::SameLine(0, 0);
                if (ImGui::Button(ICON_DASHICONS_ARROW_DOWN "##PlayDropdown", ImVec2(arrowBtnWidth, 0)))
                {
                    ImGui::OpenPopup("PlayTargetPopup");
                }
            }

            ImGui::PopStyleColor();

            // Play target dropdown popup
            if (ImGui::BeginPopup("PlayTargetPopup"))
            {
                LaunchersModule* launchers = nullptr;
                PreferencesManager* prefsMgr = PreferencesManager::Get();
                if (prefsMgr != nullptr)
                {
                    launchers = static_cast<LaunchersModule*>(prefsMgr->FindModule("External/Launchers"));
                }

                bool dolphinOk = launchers && launchers->IsEmulatorConfigured(Platform::GameCube);
                bool azaharOk = launchers && launchers->IsEmulatorConfigured(Platform::N3DS);
                bool threeDsLinkOk = launchers && launchers->Is3dsLinkConfigured();

                struct PlayTargetInfo
                {
                    PlayTarget target;
                    const char* label;
                    bool enabled;
                    const char* disabledTooltip;
                };

                PlayTargetInfo items[] = {
                    { PlayTarget::PlayInEditor, "Play In Editor",  true,          nullptr },
                    { PlayTarget::Dolphin,      "Play on Dolphin", dolphinOk,     "Configure Dolphin path in Preferences > External > Launchers" },
                    { PlayTarget::Azahar,       "Play on Azahar",  azaharOk,      "Configure Azahar path in Preferences > External > Launchers" },
                    { PlayTarget::Standalone,   "Play Standalone",  true,          nullptr },
                    { PlayTarget::Send3dsLink,  "Send 3dsLink",    threeDsLinkOk, "Configure 3dslink in Preferences > External > Launchers" },
                };

                for (const PlayTargetInfo& item : items)
                {
                    bool isSelected = (sPlayTarget == item.target);

                    if (!item.enabled)
                    {
                        ImGui::BeginDisabled();
                    }

                    if (ImGui::Selectable(item.label, isSelected))
                    {
                        sPlayTarget = item.target;
                    }

                    if (!item.enabled)
                    {
                        ImGui::EndDisabled();
                        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && item.disabledTooltip)
                        {
                            ImGui::SetTooltip("%s", item.disabledTooltip);
                        }
                    }
                }

                ImGui::EndPopup();
            }
        }

        // Draw addon toolbar items
        {
            EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
            if (hookMgr != nullptr)
            {
                hookMgr->DrawToolbarItems();
            }
        }

        // Hotkey Menus
        if (GetEditorState()->GetViewport3D()->ShouldHandleInput())
        {
            const bool ctrlDown = IsControlDown();
            bool shiftDown = IsShiftDown();
            const bool altDown = IsAltDown();

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

            if (ctrlDown && IsKeyJustDown(KEY_R))
            {
                ReloadAllScripts();
                NativeAddonManager* nam = NativeAddonManager::Get();
                if (nam != nullptr)
                {
                    std::vector<std::string> localIds = nam->GetLocalPackageIds();
                    for (const std::string& id : localIds)
                    {
                        std::string addonPath = nam->GetAddonSourcePath(id);
                        if (!addonPath.empty())
                        {
                            nam->GenerateIDEConfig(addonPath);
                        }
                    }

                    nam->ReloadAllNativeAddons();
                    LogDebug("Native addon dependencies regenerated and addons reloaded.");
                }
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

        // Asset drop mesh type selection popup
        if (sAssetDropPopupPending)
        {
            ImGui::SetNextWindowPos(sAssetDropPopupPos);
            ImGui::OpenPopup("AssetDropMeshType");
            sAssetDropPopupPending = false;
        }

        if (ImGui::BeginPopup("AssetDropMeshType"))
        {
            // Determine spawn position: raycast for viewport drops, focal point otherwise
            auto getAssetDropSpawnPos = [&]() -> glm::vec3
            {
                if (sAssetDropInViewport)
                {
                    Camera3D* camera = GetEditorState()->GetEditorCamera();
                    if (camera != nullptr)
                    {
                        RayTestResult rayResult;
                        camera->TraceScreenToWorld(sAssetDropScreenX, sAssetDropScreenY, ColGroupAll, rayResult);
                        if (rayResult.mHitNode != nullptr)
                        {
                            return rayResult.mHitPosition;
                        }

                        Node3D* hitNode = Renderer::Get()->ProcessHitCheck(GetWorld(0), sAssetDropScreenX, sAssetDropScreenY);
                        if (hitNode != nullptr)
                        {
                            Primitive3D* prim = hitNode->As<Primitive3D>();
                            if (prim != nullptr)
                            {
                                Bounds bounds = prim->GetBounds();
                                glm::vec3 rayOrigin = camera->GetWorldPosition();
                                glm::vec3 nearPoint = camera->ScreenToWorldPosition(sAssetDropScreenX, sAssetDropScreenY);
                                glm::vec3 rayDir = Maths::SafeNormalize(nearPoint - rayOrigin);

                                glm::vec3 oc = rayOrigin - bounds.mCenter;
                                float b = 2.0f * glm::dot(oc, rayDir);
                                float c = glm::dot(oc, oc) - bounds.mRadius * bounds.mRadius;
                                float discriminant = b * b - 4.0f * c;

                                if (discriminant >= 0.0f)
                                {
                                    float t = (-b - sqrtf(discriminant)) / 2.0f;
                                    return rayOrigin + rayDir * t;
                                }

                                float t = -glm::dot(oc, rayDir);
                                return rayOrigin + rayDir * glm::max(t, 0.0f);
                            }
                        }
                    }
                }
                return EditorGetFocusPosition();
            };

            if (ImGui::Selectable(BASIC_STATIC_MESH))
            {
                if (sAssetDropStub != nullptr)
                {
                    if (sAssetDropStub->mAsset == nullptr)
                        AssetManager::Get()->LoadAsset(*sAssetDropStub);
                    if (sAssetDropStub->mAsset != nullptr)
                    {
                        glm::vec3 spawnPos = getAssetDropSpawnPos();
                        ActionManager::Get()->SpawnBasicNode(BASIC_STATIC_MESH,
                            sAssetDropParentNode, sAssetDropStub->mAsset, sAssetDropInViewport, spawnPos);

                        EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
                        if (hookMgr)
                        {
                            if (sAssetDropInViewport)
                                hookMgr->FireOnAssetDropViewport(sAssetDropStub->mName.c_str());
                            else
                                hookMgr->FireOnAssetDropHierarchy(sAssetDropStub->mName.c_str());
                        }
                    }
                }
                sAssetDropStub = nullptr;
            }
            if (ImGui::Selectable(BASIC_INSTANCED_MESH))
            {
                if (sAssetDropStub != nullptr)
                {
                    if (sAssetDropStub->mAsset == nullptr)
                        AssetManager::Get()->LoadAsset(*sAssetDropStub);
                    if (sAssetDropStub->mAsset != nullptr)
                    {
                        glm::vec3 spawnPos = getAssetDropSpawnPos();
                        ActionManager::Get()->SpawnBasicNode(BASIC_INSTANCED_MESH,
                            sAssetDropParentNode, sAssetDropStub->mAsset, sAssetDropInViewport, spawnPos);

                        EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
                        if (hookMgr)
                        {
                            if (sAssetDropInViewport)
                                hookMgr->FireOnAssetDropViewport(sAssetDropStub->mName.c_str());
                            else
                                hookMgr->FireOnAssetDropHierarchy(sAssetDropStub->mName.c_str());
                        }
                    }
                }
                sAssetDropStub = nullptr;
            }
            ImGui::EndPopup();
        }
        else if (sAssetDropStub != nullptr && !sAssetDropPopupPending)
        {
            sAssetDropStub = nullptr;
            sAssetDropParentNode = nullptr;
        }

        ImGui::EndMainMenuBar();
    }

    // Set up ImGuizmo rect for the viewport area
    {
        EditorState* edState = GetEditorState();
        float interfaceScale = GetEngineConfig()->mEditorInterfaceScale;
        if (interfaceScale == 0.0f)
        {
            interfaceScale = 1.0f;
        }
        float invInterfaceScale = 1.0f / interfaceScale;

        ImGuizmo::SetRect(
            (float)edState->mViewportX * invInterfaceScale,
            (float)edState->mViewportY * invInterfaceScale,
            (float)edState->mViewportWidth * invInterfaceScale,
            (float)edState->mViewportHeight * invInterfaceScale
        );

        ImGuizmo::SetAlternativeWindow(sViewportDockWindow);
    }

    // Modal dialogs - drawn outside menu bar scope
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
        sSceneImportOptions.mFilePath = GetEditorState()->mPendingSceneImportPath;
        ImGui::Text(GetEditorState()->mPendingSceneImportPath.c_str());
        ImGui::InputText("Scene Name", &sSceneImportOptions.mSceneName);
        ImGui::InputText("Prefix", &sSceneImportOptions.mPrefix);
        ImGui::Checkbox("Import Meshes", &sSceneImportOptions.mImportMeshes);
        ImGui::Checkbox("Import Materials", &sSceneImportOptions.mImportMaterials);
        ImGui::Checkbox("Import Textures", &sSceneImportOptions.mImportTextures);
        ImGui::Checkbox("Import Lights", &sSceneImportOptions.mImportLights);
        ImGui::Checkbox("Import Cameras", &sSceneImportOptions.mImportCameras);
        ImGui::Checkbox("Enable Collision", &sSceneImportOptions.mEnableCollision);
        ImGui::Checkbox("Apply glTF Extras", &sSceneImportOptions.mApplyGltfExtras);

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


    // Reimport Scene popup
    if (!ImGui::IsPopupOpen("Reimport Scene") &&
        GetEditorState()->mPendingReimportSceneStub != nullptr &&
        GetEditorState()->mPendingReimportScenePath != "")
    {
        sReimportSceneOptions = SceneImportOptions();
        sReimportSceneOptions.mFilePath = GetEditorState()->mPendingReimportScenePath;
        ImGui::OpenPopup("Reimport Scene");
    }

    if (ImGui::IsPopupOpen("Reimport Scene"))
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    }

    if (ImGui::BeginPopupModal("Reimport Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
    {
        AssetStub* reimportStub = GetEditorState()->mPendingReimportSceneStub;

        ImGui::Text("File: %s", GetEditorState()->mPendingReimportScenePath.c_str());
        ImGui::Text("Scene: %s", reimportStub ? reimportStub->mName.c_str() : "");
        ImGui::Separator();

        ImGui::InputText("Prefix", &sReimportSceneOptions.mPrefix);
        ImGui::Checkbox("Import Meshes", &sReimportSceneOptions.mImportMeshes);
        ImGui::Checkbox("Import Materials", &sReimportSceneOptions.mImportMaterials);
        ImGui::Checkbox("Import Textures", &sReimportSceneOptions.mImportTextures);
        ImGui::Checkbox("Import Lights", &sReimportSceneOptions.mImportLights);
        ImGui::Checkbox("Import Cameras", &sReimportSceneOptions.mImportCameras);
        ImGui::Checkbox("Enable Collision", &sReimportSceneOptions.mEnableCollision);
        ImGui::Checkbox("Textures Updated", &sReimportSceneOptions.mReimportTextures);

        int32_t shadingModelCount = int32_t(ShadingModel::Count);
        ImGui::Combo("Shading Model", (int*)&(sReimportSceneOptions.mDefaultShadingModel), gShadingModelStrings, shadingModelCount);

        int32_t vertColorMode = int32_t(VertexColorMode::Count);
        ImGui::Combo("Vertex Color Mode", (int*)&(sReimportSceneOptions.mDefaultVertexColorMode), gVertexColorModeStrings, vertColorMode);

        if (ImGui::Button("Reimport"))
        {
            if (reimportStub != nullptr)
            {
                std::string sceneName = reimportStub->mName;
                if (sceneName.size() > 3 && sceneName.substr(0, 3) == "SC_")
                {
                    sceneName = sceneName.substr(3);
                }
                sReimportSceneOptions.mSceneName = sceneName;
                sReimportSceneOptions.mReimportSceneStub = reimportStub;

                am->ImportScene(sReimportSceneOptions);
            }

            GetEditorState()->mPendingReimportSceneStub = nullptr;
            GetEditorState()->mPendingReimportScenePath = "";
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            GetEditorState()->mPendingReimportSceneStub = nullptr;
            GetEditorState()->mPendingReimportScenePath = "";
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (!ImGui::IsPopupOpen("Import Camera") && GetEditorState()->mIOAssetPath != "")
    {
        ImGui::OpenPopup("Import Camera");
    }

    if (ImGui::IsPopupOpen("Import Camera"))
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    }

    if (ImGui::BeginPopupModal("Import Camera", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
    {
        sCameraImportOptions.mFilePath = GetEditorState()->mIOAssetPath;
        ImGui::Text(GetEditorState()->mPendingSceneImportPath.c_str());
        ImGui::Checkbox("Set As Main Camera", &sCameraImportOptions.mIsMainCamera);
        ImGui::Checkbox("Override Camera Name", &sCameraImportOptions.mOverrideCameraName);
        if (sCameraImportOptions.mOverrideCameraName) {
            ImGui::InputText("Camera Name", &sCameraImportOptions.mCameraName);
        }

        if (ImGui::Button("Import"))
        {
            am->ImportCamera(sCameraImportOptions);
            GetEditorState()->mIOAssetPath = "";
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            GetEditorState()->mIOAssetPath = "";
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

static void DrawPaintColorsPanel()
{
    const float dispWidth = ImGui::GetIO().DisplaySize.x;
    const float dispHeight = ImGui::GetIO().DisplaySize.y;

    ImGui::SetNextWindowPos(ImVec2(210.0f, GetTopBarHeight()));
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

    ImGui::SetNextWindowPos(ImVec2(210.0f, GetTopBarHeight()));
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

static void DrawDesignBounds()
{
    if(GetFeatureFlagsEditor().mShow2DBorder == false){
        return;
    }
    Viewport2D* viewport2d = GetEditorState()->GetViewport2D();
    if (viewport2d == nullptr)
        return;

    ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
    glm::vec4 vp = Renderer::Get()->GetViewport(0);

    float interfaceScale = GetEngineConfig()->mEditorInterfaceScale;
    if (interfaceScale == 0.0f)
    {
        interfaceScale = 1.0f;
    }
    float invInterfaceScale = 1.0f / interfaceScale;

    // Get the design resolution from engine config
    float designWidth = (float)GetEngineConfig()->mWindowWidth;
    float designHeight = (float)GetEngineConfig()->mWindowHeight;

    // Get zoom and pan from viewport
    float zoom = viewport2d->GetZoom();
    glm::vec2 rootOffset = viewport2d->GetRootOffset();

    // Calculate the design bounds in screen space
    // The design area starts at rootOffset (pan) and is scaled by zoom
    float boundsX = rootOffset.x * zoom;
    float boundsY = rootOffset.y * zoom;
    float boundsW = designWidth * zoom;
    float boundsH = designHeight * zoom;

    // Convert to ImGui coordinates (accounting for viewport offset and interface scale)
    float x = invInterfaceScale * (boundsX + vp.x);
    float y = invInterfaceScale * (boundsY + vp.y);
    float w = invInterfaceScale * boundsW;
    float h = invInterfaceScale * boundsH;

    // Draw outer darkened regions (like Unity's letterboxing)
    ImColor dimColor(0.0f, 0.0f, 0.0f, 0.4f);
    float vpLeft = invInterfaceScale * vp.x;
    float vpTop = invInterfaceScale * vp.y;
    float vpRight = invInterfaceScale * (vp.x + vp.z);
    float vpBottom = invInterfaceScale * (vp.y + vp.w);

    // Top region (above canvas)
    if (y > vpTop)
    {
        draw_list->AddRectFilled(ImVec2(vpLeft, vpTop), ImVec2(vpRight, y), dimColor);
    }
    // Bottom region (below canvas)
    if (y + h < vpBottom)
    {
        draw_list->AddRectFilled(ImVec2(vpLeft, y + h), ImVec2(vpRight, vpBottom), dimColor);
    }
    // Left region (left of canvas, between top and bottom regions)
    float regionTop = glm::max(y, vpTop);
    float regionBottom = glm::min(y + h, vpBottom);
    if (x > vpLeft && regionTop < regionBottom)
    {
        draw_list->AddRectFilled(ImVec2(vpLeft, regionTop), ImVec2(x, regionBottom), dimColor);
    }
    // Right region (right of canvas, between top and bottom regions)
    if (x + w < vpRight && regionTop < regionBottom)
    {
        draw_list->AddRectFilled(ImVec2(x + w, regionTop), ImVec2(vpRight, regionBottom), dimColor);
    }

    // Draw the design bounds border
    ImColor boundsColor(1.0f, 0.6f, 0.0f, 0.8f);  // Orange color
    float thickness = 2.0f;
    draw_list->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), boundsColor, 0.0f, ImDrawFlags_None, thickness);
}

static void Draw2dSelections()
{
    const std::vector<Node*>& selNodes = GetEditorState()->GetSelectedNodes();

    ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
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

    // Clear gizmo block when mouse is released
    if (edState->mGizmoBlockedBySelection && !IsMouseButtonDown(MouseCode::MOUSE_LEFT))
    {
        edState->mGizmoBlockedBySelection = false;
    }

    // If gizmo is blocked by recent selection, disable interaction
    if (edState->mGizmoBlockedBySelection)
    {
        ImGuizmo::Enable(false);
    }
    else
    {
        ImGuizmo::Enable(true);
    }

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

    // Clear gizmo block when mouse is released
    if (edState->mGizmoBlockedBySelection && !IsMouseButtonDown(MouseCode::MOUSE_LEFT))
    {
        edState->mGizmoBlockedBySelection = false;
    }

    // If gizmo is blocked by recent selection, disable interaction
    if (edState->mGizmoBlockedBySelection)
    {
        ImGuizmo::Enable(false);
    }
    else
    {
        ImGuizmo::Enable(true);
    }

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

    // Account for interface scale - ImGuizmo operates in scaled ImGui coordinates
    float interfaceScale = GetEngineConfig()->mEditorInterfaceScale;
    if (interfaceScale == 0.0f)
    {
        interfaceScale = 1.0f;
    }
    float invInterfaceScale = 1.0f / interfaceScale;

    // Viewport dimensions in scaled coordinates (matching ImGuizmo::SetRect)
    float vpWidth = (float)edState->mViewportWidth * invInterfaceScale;
    float vpHeight = (float)edState->mViewportHeight * invInterfaceScale;

    // Build orthographic projection matrix (Y down for screen coords)
    glm::mat4 projMatrix = glm::ortho(0.0f, vpWidth, vpHeight, 0.0f, -1.0f, 1.0f);

    // View matrix is identity since GetRect() already returns screen-space coordinates
    // (zoom and rootOffset are already applied via the wrapper widget)
    glm::mat4 viewMatrix = glm::mat4(1.0f);

    // Get widget's rect (already in screen space after zoom/offset transform)
    // Scale to match ImGuizmo's coordinate space
    Rect rect = widget->GetRect();
    rect.mX *= invInterfaceScale;
    rect.mY *= invInterfaceScale;
    rect.mWidth *= invInterfaceScale;
    rect.mHeight *= invInterfaceScale;
    glm::vec2 pivot = widget->GetPivot();

    // Calculate pivot point in screen space (already scaled)
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
            // Extract translation delta from deltaMatrix (in scaled coordinates)
            glm::vec3 deltaTrans(deltaMatrix[3]);

            // Convert from scaled screen coordinates back to render target pixels,
            // then to widget offset units (account for zoom)
            glm::vec2 offsetDelta = glm::vec2(deltaTrans.x, deltaTrans.y) * interfaceScale / zoom;

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
            // Extract the new scale from the modified modelMatrix (in scaled coordinates)
            glm::vec3 newScale, trans, skew;
            glm::vec4 perspective;
            glm::quat rot;
            glm::decompose(modelMatrix, newScale, rot, trans, skew, perspective);

            // The modelMatrix was initialized with (rect.mWidth, rect.mHeight, 1) in scaled coordinates
            // After manipulation, newScale contains the scaled dimensions
            // Convert from scaled coordinates back to render target pixels, then to widget size units
            glm::vec2 newScreenSize(newScale.x * interfaceScale, newScale.y * interfaceScale);
            glm::vec2 newSize = newScreenSize / zoom;

            // Handle stretch mode
            // Note: rect is in scaled coordinates, so convert newScale for comparison
            if (widget->StretchX())
            {
                // For stretch, convert pixel change to ratio (in scaled coordinates)
                float pixelChange = newScale.x - rect.mWidth;
                newSize.x = widget->GetSize().x + pixelChange * interfaceScale * 0.00002f;
            }
            if (widget->StretchY())
            {
                float pixelChange = newScale.y - rect.mHeight;
                newSize.y = widget->GetSize().y + pixelChange * interfaceScale * 0.00002f;
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

static std::string GetDefaultEditorFontPath()
{

    if (!SYS_DoesFileExist("Engine/Assets/Fonts/F_InterRegular18.ttf", false)) {
        return "";
    }

    return SYS_GetAbsolutePath("Engine/Assets/Fonts/F_InterRegular18.ttf");
}

static std::string ResolveEditorFontName()
{
    const EngineConfig* config = GetEngineConfig();
    if (config != nullptr && !config->mCurrentFont.empty())
    {
        return config->mCurrentFont;
    }

    std::string savedFont = ThemeModule::LoadSavedFontPreference();
    if (savedFont.empty())
    {
        savedFont = "Default";
    }

    GetMutableEngineConfig()->mCurrentFont = savedFont;
    return savedFont;
}

static std::string ResolveEditorFontPath()
{
    const std::string defaultPath = GetDefaultEditorFontPath();
    const std::string desiredFontName = ResolveEditorFontName();

    if (desiredFontName.empty() || desiredFontName == "Default")
    {
        return defaultPath;
    }

    const std::string relativePath = "Engine/Assets/Fonts/" + desiredFontName;
    const std::string absolutePath = SYS_GetAbsolutePath(relativePath.c_str());

    if (!SYS_DoesFileExist(absolutePath.c_str(), false))
    {
        LogWarning("Editor font '%s' not found at %s. Falling back to default font.", desiredFontName.c_str(), absolutePath.c_str());
        GetMutableEngineConfig()->mCurrentFont = "Default";
        return defaultPath;
    }

    return absolutePath;
}

static std::string ResolveEditorIconFontPath()
{
    const std::string defaultPath = GetDefaultEditorFontPath();
    const std::string desiredFontName = ResolveEditorFontName();

    if (desiredFontName.empty() || desiredFontName == "Default")
    {
        return defaultPath;
    }

    const std::string relativePath = "Engine/Assets/Fonts/OctaveEngineIcons.ttf" ;
    const std::string absolutePath = SYS_GetAbsolutePath(relativePath.c_str());

    if (!SYS_DoesFileExist(absolutePath.c_str(), false))
    {
        LogWarning("Editor Icon font '%s' not found at %s. Falling back to default font.", desiredFontName.c_str(), absolutePath.c_str());
        GetMutableEngineConfig()->mCurrentFont = "Default";
        return defaultPath;
    }

    return absolutePath;
}

void EditorImguiInit()
{
    if (IsHeadless())
    {
        return;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuizmo::SetImGuiContext(ImGui::GetCurrentContext());
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    
    // Disabling keyboard controls because it interferes with Alt hotkeys.
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Dear ImGui default style
    ImGui::StyleColorsDark();

    if (GetFeatureFlagsEditor().mShowTheming == true) {
       
        std::string fontPath = ResolveEditorFontPath();
        ImFont* myFont = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 15.0f);
        if (myFont == nullptr)
        {
            const std::string defaultFontPath = GetDefaultEditorFontPath();
            myFont = io.Fonts->AddFontFromFileTTF(defaultFontPath.c_str(), 15.0f);
            if (myFont == nullptr)
            {
                LogError("Failed to load editor font from %s", defaultFontPath.c_str());
            }
        }
        }


    MergeOctaveIcons(io.Fonts, 14.0f,  ResolveEditorIconFontPath().c_str());

    //ImGui::StyleColorsLight();

    // Override theme
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Tab] = kBgInactive;
    colors[ImGuiCol_TabHovered] = kBgHover;
    colors[ImGuiCol_TabActive] = kSelectedColor;



    // Set unactive window title bg equal to active title.
    colors[ImGuiCol_TitleBg] = colors[ImGuiCol_TitleBgActive];

    RegisterLogCallback(DebugLogWindow::LogCallback);

    GetScriptEditorWindow()->Init();

    ImGui::InitDock();

    // Check imgui.ini for stale dock layout BEFORE first NewFrame() loads it.
    // If dock panel names changed (e.g. icons added), the saved layout won't
    // match and causes crashes.  Delete the file so ImGui starts fresh.
    ValidateDockLayoutIni();
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
        DrawMainMenuBar();
        DrawDockspace();

        if (GetEditorState()->mShowTimelinePanel)
        {
            DrawTimelinePanel();
        }

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
            DrawDesignBounds();
            Draw2dSelections();
        }

        if (GetEditorState()->mNodePropertySelect)
        {
            DrawNodePropertySelectOverlay();
        }

        DrawFileBrowser();

        DrawUnsavedCheck();
        DrawProjectUpgradeModal();
        DrawAddonsDialogs();
        DrawScriptCreatorDialogs();

        GetPreferencesWindow()->Draw();
        GetPackagingWindow()->Draw();
        GetProjectSelectWindow()->Draw();
        GetAddonsWindow()->Draw();
        GetThemeEditorWindow()->Draw();

        if (GetThemeEditorWindow()->IsInspectModeActive())
        {
            GetThemeEditorWindow()->DrawInspectOverlay();
        }
    }

    ImGui::Render();
}

void EditorImguiShutdown()
{
    if (IsHeadless())
    {
        return;
    }

    ImGui::DestroyContext();
}

void EditorImguiPreShutdown()
{
    if (IsHeadless())
    {
        return;
    }

    // Save INI before clearing dock data, then disable auto-save so DestroyContext
    // doesn't overwrite our save with empty dock state.
    if (ImGui::GetIO().IniFilename)
    {
        ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
        StampDockLayoutVersion();
        ImGui::GetIO().IniFilename = nullptr;
    }
    GetScriptEditorWindow()->Shutdown();
    ImGui::ShutdownDock();
    UnregisterLogCallback(DebugLogWindow::LogCallback);

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
        float scale = ImGui::GetIO().OctaveInterfaceScale;
        if (scale == 0.0f)
        {
            scale = 1.0f;
        }

        x = uint32_t(sViewportDockPos.x * scale + 0.5f);
        y = uint32_t(sViewportDockPos.y * scale + 0.5f);
        int32_t iW = int32_t(sViewportDockSize.x * scale + 0.5f);
        int32_t iH = int32_t(sViewportDockSize.y * scale + 0.5f);
        iW = glm::clamp<int32_t>(iW, 100, int32_t(ImGui::GetIO().DisplaySize.x + 0.5f));
        iH = glm::clamp<int32_t>(iH, 100, int32_t(ImGui::GetIO().DisplaySize.y + 0.5f));
        width = uint32_t(iW);
        height = uint32_t(iH);
    }
    else
    {
        x = 0;
        y = 0;
        width = uint32_t(ImGui::GetIO().DisplaySize.x + 0.5f);
        height = uint32_t(ImGui::GetIO().DisplaySize.y + 0.5f);
    }
}

bool EditorImguiIsViewportHovered()
{
    ImVec2 mp = ImGui::GetIO().MousePos;
    return (mp.x >= sViewportDockPos.x &&
            mp.x <  sViewportDockPos.x + sViewportDockSize.x &&
            mp.y >= sViewportDockPos.y &&
            mp.y <  sViewportDockPos.y + sViewportDockSize.y);
}

bool EditorIsInterfaceVisible()
{
    return GetEditorState()->mShowInterface && (!IsPlaying() || GetEditorState()->mEjected);
}

#endif
