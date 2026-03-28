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
#include "EditorIconRegistry.h"

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
#include "Assets/NodeGraphAsset.h"
#include "Assets/DataAsset.h"
#include "UI/UIDocument.h"
#include "UI/UITypes.h"

#include "Viewport3d.h"
#include "Viewport2d.h"
#include "ActionManager.h"
#include "EditorState.h"
#include "Preferences/PreferencesWindow.h"
#include "Preferences/Appearance/Theme/ThemeModule.h"
#include "Preferences/Appearance/Viewport/ViewportModule.h"
#include "Packaging/PackagingWindow.h"
#include "ProjectSelect/ProjectSelectWindow.h"
#include "Addons/AddonsWindow.h"
#include "Addons/NativeAddonManager.h"
#include "Addons/AddonsMenu.h"
#include "EditorUIHookManager.h"
#include "BuildDependencyWindow.h"
#include "DebugLog/DebugLogWindow.h"
#include "ScriptEditor/ScriptEditorWindow.h"
#include "ThemeEditor/ThemeEditorWindow.h"
#include "Preferences/Appearance/Theme/CssThemeParser.h"
#include "Timeline/TimelinePanel.h"
#include "NodeGraph/NodeGraphPanel.h"
#include "Profiling/ProfilingWindow.h"
#include "Preferences/General/GeneralModule.h"
#include "Preferences/PreferencesManager.h"
#include "Preferences/External/LaunchersModule.h"
#include "Preferences/External/EditorsModule.h"
#include "Packaging/PackagingSettings.h"
#include "ScriptCreator/ScriptCreatorDialog.h"
#include "ControllerServer/ControllerServer.h"
#include "Preferences/Network/NetworkModule.h"

#include <functional>
#include <algorithm>
#include <map>
#include <unordered_map>

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

// Cross-platform case-insensitive string comparison
#ifdef _WIN32
#define strcasecmp _stricmp
#endif
#include <Nodes/TimelinePlayer.h>
#include <Nodes/Widgets/Button.h>
#include <Nodes/Widgets/Quad.h>
#include <Nodes/Widgets/Canvas.h>
#include <Nodes/Widgets/Console.h>
#include <Nodes/Widgets/ArrayWidget.h>

#include "SecondScreenPreview/SecondScreenPreview.h"
#include "GamePreview/GamePreview.h"


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
    if (node->IsSceneLinked())
    {
        Scene* scene = node->GetScene();
        if (scene != nullptr)
        {
            const char* customIcon = GetSceneIconByIndex(scene->GetIconOverride());
            if (customIcon != nullptr)
                return customIcon;
        }
        return ICON_STREAMLINE_PLUMP_WORLD_REMIX;
    }
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
    if (type == NodeGraphAsset::GetStaticType())    return ICON_IC_BASELINE_SHARE;
    if (type == UIDocument::GetStaticType())         return ICON_MATERIAL_SYMBOLS_WIDGET_MEDIUM;
    return ICON_STREAMLINE_SHARP_NEW_FILE_REMIX;
}

struct FileBrowserDirEntry
{
    std::string mName;
    std::string mDirPath;
    bool mFolder = false;
    bool mSelected = false;
};

static float GetMenuBarPadding()
{
    ViewportModule* vm = ViewportModule::Get();
    return vm ? vm->GetMenuBarPadding() : 8.0f;
}

static float GetMainMenuBarHeight()
{
    float padding = GetMenuBarPadding();
    return ImGui::GetFontSize() + padding * 2.0f + 1.0f;
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

// XML to UIDocument conversion state
static std::string sConvertXmlSourcePath;
static bool sShowUIDocumentPicker = false;

static int sNewSceneType = 1;        // 0=2D, 1=3D
static bool sNewSceneCreateCamera = true;
static int32_t sZooColumns = 5;
static float sZooSpacing = 3.0f;

static bool sF2RenameAssetFocus = false;
static bool sDuplicateAssetFocus = false;
static bool sDuplicateNodeFocus = false;
static AssetStub* sDuplicateAssetStub = nullptr;
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
static std::unordered_map<std::string, std::vector<std::string>> sScenesByCategory;
static int32_t sDevModeClicks = 0;

static std::string sReplaceAssetInput;
static std::vector<std::string> sReplaceAssetSuggestions;

static Node* sCreateSceneTargetNode = nullptr;
static AssetDir* sCreateSceneTargetDir = nullptr;

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
static bool sViewportDockActive = false;

// A known dock label that must exist in a valid layout.
// If imgui.ini has dock data but this label is missing, the layout is stale.
// Update kDockLayoutVersion when dock panel names change to force a reset.
static constexpr uint32_t kDockLayoutVersion = 9;

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

static std::string ReadFileToString(const char* path)
{
    std::string result;
    FILE* f = fopen(path, "rb");
    if (f)
    {
        fseek(f, 0, SEEK_END);
        long sz = ftell(f);
        fseek(f, 0, SEEK_SET);
        if (sz > 0)
        {
            result.resize(sz);
            fread(&result[0], 1, sz, f);
        }
        fclose(f);
    }
    return result;
}

static void RefreshUIDocuments()
{
    AssetManager* assetMgr = AssetManager::Get();
    if (!assetMgr)
        return;

    auto& assetMap = assetMgr->GetAssetMap();
    TypeId uiDocType = UIDocument::GetStaticType();

    for (auto& pair : assetMap)
    {
        AssetStub* stub = pair.second;
        if (!stub || stub->mType != uiDocType)
            continue;

        // Ensure asset is loaded so we can inspect it
        if (!stub->mAsset)
            assetMgr->LoadAsset(*stub);
        if (!stub->mAsset)
            continue;

        UIDocument* doc = stub->mAsset->As<UIDocument>();
        if (!doc)
            continue;

        const std::string& srcPath = doc->GetSourceFilePath();
        if (srcPath.empty() || !SYS_DoesFileExist(srcPath.c_str(), false))
            continue;

        // Read the .xml from disk and compare with the loaded asset.
        // If they differ, reimport from the .xml (the editable source of truth).
        std::string diskXml = ReadFileToString(srcPath.c_str());

        if (!diskXml.empty() && diskXml != doc->GetXmlSource())
        {
            doc->Import(srcPath, nullptr);
            assetMgr->SaveAsset(*stub);
            LogDebug("UIDocument reimported from XML: %s", stub->mName.c_str());
        }
    }
}

// Forward declarations for panel content functions (called from DrawDockspace)
static void DrawScenePanel();
static void DrawAssetsPanel();
static void DrawPropertiesPanel();
static void DrawScriptsPanel();
static void DrawDirPickerTree(AssetDir* dir, AssetDir** selectedDir);

// Alternating row background helpers
static void BeginAlternatingRows();
static void AlternatingRowBackground();

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
    sViewportDockActive = ImGui::BeginDock(ICON_VIEWPORT3D "  Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    if (sViewportDockActive)
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
                        else if (droppedStub != nullptr && droppedStub->mType == Scene::GetStaticType())
                        {
                            if (droppedStub->mAsset == nullptr)
                                AssetManager::Get()->LoadAsset(*droppedStub);
                            if (droppedStub->mAsset != nullptr)
                            {
                                glm::vec3 spawnPos = EditorGetFocusPosition();
                                Camera3D* camera = GetEditorState()->GetEditorCamera();
                                if (camera != nullptr)
                                {
                                    ImVec2 dropPos = ImGui::GetMousePos();
                                    float scale = GetEngineConfig()->mEditorInterfaceScale;
                                    if (scale == 0.0f) scale = 1.0f;
                                    int32_t screenX = (int32_t)(dropPos.x * scale);
                                    int32_t screenY = (int32_t)(dropPos.y * scale);

                                    RayTestResult rayResult;
                                    camera->TraceScreenToWorld(screenX, screenY, ColGroupAll, rayResult);
                                    if (rayResult.mHitNode != nullptr)
                                        spawnPos = rayResult.mHitPosition;
                                }
                                ActionManager::Get()->SpawnBasicNode(BASIC_SCENE,
                                    nullptr, droppedStub->mAsset, true, spawnPos);
                            }
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
            }
        }

        // Viewport right-click context menu (Batch 3)
        {
            EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
            if (hookMgr != nullptr && hookMgr->HasViewportContextItems())
            {
                if (ImGui::BeginPopupContextWindow("ViewportContextMenu", 1))
                {
                    hookMgr->DrawViewportContextItems();
                    ImGui::EndPopup();
                }
            }
        }

        // Viewport overlays (Batch 3)
        {
            EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
            if (hookMgr != nullptr)
            {
                hookMgr->DrawViewportOverlays(
                    sViewportDockPos.x, sViewportDockPos.y,
                    sViewportDockSize.x, sViewportDockSize.y);
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
    if (ImGui::BeginDock(ICON_STREAMLINE_LOG_SOLID "  Debug Log", &debugLogOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
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

    // --- 3DS Preview dock ---
    {
        ImVec4 bg = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, bg);
    }
    if (ImGui::BeginDock(ICON_CIB_NINTENDO_3DS "  3DS Preview", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
    {
        GetSecondScreenPreview()->DrawPanel();
    }
    ImGui::EndDock();
    ImGui::PopStyleColor();

    // --- Game Preview dock ---
    {
        ImVec4 bg = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, bg);
    }
    if (ImGui::BeginDock(ICON_IX_VIDEO_CAMERA_FILLED "  Game Preview", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
    {
        GetGamePreview()->DrawPanel();
    }
    ImGui::EndDock();
    ImGui::PopStyleColor();

    // --- Node Graph dock ---
    {
        ImVec4 bg = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, bg);
    }
    if (ImGui::BeginDock(ICON_IC_BASELINE_SHARE "  Node Graph", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
    {
        DrawNodeGraphContent();
    }
    ImGui::EndDock();
    ImGui::PopStyleColor();

    // --- Profiling dock ---
    {
        ImVec4 bg = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, bg);
    }
    if (ImGui::BeginDock(ICON_CURVEGRAPH "  Profiling", nullptr, 0))
    {
        GetProfilingWindow()->DrawContent();
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
            ImGui::DockToRoot("EditorDock", ICON_INFO "  Properties", ImGuiDockSlot_Right, 0.45f);
            ImGui::DockToRoot("EditorDock",ICON_ASSETS "  Assets", ImGuiDockSlot_Bottom, 0.33f);
            ImGui::DockTo("EditorDock", ICON_IX_CODE "  Scripts", ICON_ASSETS "  Assets", ImGuiDockSlot_Tab);
            ImGui::DockTo("EditorDock", ICON_STREAMLINE_LOG_SOLID "  Debug Log", ICON_ASSETS "  Assets", ImGuiDockSlot_Tab);
            ImGui::DockTo("EditorDock", ICON_IX_CODE "  Script Editor", ICON_VIEWPORT3D "  Viewport", ImGuiDockSlot_Tab);
            ImGui::DockTo("EditorDock", ICON_CIB_NINTENDO_3DS "  3DS Preview", ICON_INFO "  Properties", ImGuiDockSlot_Left, 0.6f);
            ImGui::DockTo("EditorDock", ICON_IX_VIDEO_CAMERA_FILLED "  Game Preview",ICON_CIB_NINTENDO_3DS "  3DS Preview",  ImGuiDockSlot_Tab);
            ImGui::DockTo("EditorDock", ICON_IC_BASELINE_SHARE "  Node Graph", ICON_ASSETS "  Assets", ImGuiDockSlot_Right, 0.5f);
            ImGui::DockTo("EditorDock", ICON_CURVEGRAPH "  Profiling", ICON_IC_BASELINE_SHARE "  Node Graph", ImGuiDockSlot_Tab);

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

        if (!contextPopupOpen && IsKeyJustDown(OCTAVE_KEY_ENTER))
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

        if (IsKeyJustDown(OCTAVE_KEY_ESCAPE))
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

        if (IsKeyJustDown(OCTAVE_KEY_ESCAPE))
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

static bool MoveAssetToDirectory(AssetStub* stub, AssetDir* newDir)
{
    if (stub == nullptr || newDir == nullptr || stub->mDirectory == newDir)
        return false;
    if (stub->mEngineAsset || newDir->mEngineDir || newDir->mAddonDir)
        return false;

    AssetDir* oldDir = stub->mDirectory;

    // Build new path
    std::string newPath = newDir->mPath + stub->mName + ".oct";

    // Move file on disk
    if (!SYS_Rename(stub->mPath.c_str(), newPath.c_str()))
        return false;

    // Update in-memory structures
    oldDir->RemoveAssetStub(stub);
    newDir->mAssetStubs.push_back(stub);
    stub->mDirectory = newDir;
    stub->mPath = newPath;

    return true;
}

static bool MoveDirectoryToDirectory(AssetDir* dir, AssetDir* newParent)
{
    if (dir == nullptr || newParent == nullptr)
        return false;
    if (dir->mEngineDir || dir->mAddonDir || newParent->mEngineDir || newParent->mAddonDir)
        return false;
    if (dir->mParentDir == newParent)
        return false;
    // Prevent moving a dir into itself or a descendant
    for (AssetDir* p = newParent; p != nullptr; p = p->mParentDir)
    {
        if (p == dir)
            return false;
    }

    std::string oldPath = dir->mPath;
    std::string newPath = newParent->mPath + dir->mName + "/";

    // Move on disk
    SYS_MoveDirectory(oldPath.c_str(), newPath.c_str());

    // Update tree: remove from old parent
    AssetDir* oldParent = dir->mParentDir;
    auto& oldChildren = oldParent->mChildDirs;
    oldChildren.erase(std::find(oldChildren.begin(), oldChildren.end(), dir));

    // Add to new parent
    newParent->mChildDirs.push_back(dir);
    dir->mParentDir = newParent;

    // Recursively update paths
    std::function<void(AssetDir*)> updatePaths = [&](AssetDir* d) {
        d->mPath = d->mParentDir->mPath + d->mName + "/";
        for (AssetStub* stub : d->mAssetStubs)
            stub->mPath = d->mPath + stub->mName + ".oct";
        for (AssetDir* child : d->mChildDirs)
            updatePaths(child);
    };
    updatePaths(dir);

    return true;
}

static const char* AnchorModeToCSS(AnchorMode mode)
{
    switch (mode)
    {
    case AnchorMode::TopLeft:                return "top-left";
    case AnchorMode::TopMid:                 return "top-center";
    case AnchorMode::TopRight:               return "top-right";
    case AnchorMode::MidLeft:                return "center-left";
    case AnchorMode::Mid:                    return "center";
    case AnchorMode::MidRight:               return "center-right";
    case AnchorMode::BottomLeft:             return "bottom-left";
    case AnchorMode::BottomMid:              return "bottom-center";
    case AnchorMode::BottomRight:            return "bottom-right";
    case AnchorMode::TopStretch:             return "top-stretch";
    case AnchorMode::MidHorizontalStretch:   return "center-h-stretch";
    case AnchorMode::BottomStretch:          return "bottom-stretch";
    case AnchorMode::LeftStretch:            return "left-stretch";
    case AnchorMode::MidVerticalStretch:     return "center-v-stretch";
    case AnchorMode::RightStretch:           return "right-stretch";
    case AnchorMode::FullStretch:            return "full-stretch";
    default:                                 return "top-left";
    }
}

static void ExportWidgetTreeToXML(Widget* widget, std::string& outXml, int indent = 0)
{
    if (widget == nullptr)
        return;

    const char* typeName = widget->GetTypeName();
    const char* element = UIWidgetTypeToElement(typeName);

    std::string pad(indent * 2, ' ');
    outXml += pad + "<" + element;

    // id attribute from widget name
    const std::string& name = widget->GetName();
    if (!name.empty())
    {
        outXml += " id=\"" + name + "\"";
    }

    // Build inline style
    std::string style;

    // Anchor mode (skip default top-left)
    AnchorMode anchor = widget->GetAnchorMode();
    if (anchor != AnchorMode::TopLeft)
    {
        style += "anchor: " + std::string(AnchorModeToCSS(anchor)) + "; ";
    }

    // Position
    glm::vec2 offset = widget->GetOffset();
    if (offset.x != 0.0f || offset.y != 0.0f)
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "left: %.0fpx; top: %.0fpx; ", offset.x, offset.y);
        style += buf;
    }

    // Size
    glm::vec2 size = widget->GetSize();
    if (size.x != 0.0f || size.y != 0.0f)
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "width: %.0fpx; height: %.0fpx; ", size.x, size.y);
        style += buf;
    }

    // Color (skip default white)
    glm::vec4 color = widget->GetColor();
    if (color.r != 1.0f || color.g != 1.0f || color.b != 1.0f || color.a != 1.0f)
    {
        char buf[80];
        snprintf(buf, sizeof(buf), "color: rgba(%.0f, %.0f, %.0f, %.2f); ",
                 color.r * 255.0f, color.g * 255.0f, color.b * 255.0f, color.a);
        style += buf;
    }

    // Opacity (skip default 255)
    uint8_t opacity = widget->GetOpacity();
    if (opacity != 255)
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "opacity: %.2f; ", opacity / 255.0f);
        style += buf;
    }

    // Visibility
    if (!widget->IsVisible())
    {
        style += "display: none; ";
    }

    // Text-specific: font-size
    Text* textWidget = widget->As<Text>();
    if (textWidget)
    {
        float textSize = textWidget->GetTextSize();
        if (textSize != 32.0f) // skip default
        {
            char buf[32];
            snprintf(buf, sizeof(buf), "font-size: %.0fpx; ", textSize);
            style += buf;
        }
    }

    // Quad-specific: src attribute
    Quad* quadWidget = widget->As<Quad>();
    if (quadWidget && quadWidget->GetTexture())
    {
        outXml += " src=\"" + quadWidget->GetTexture()->GetName() + "\"";
    }

    // Trim trailing space from style
    if (!style.empty() && style.back() == ' ')
        style.pop_back();

    if (!style.empty())
    {
        outXml += " style=\"" + style + "\"";
    }

    // Check for text content and children
    std::string textContent;
    if (textWidget)
    {
        textContent = textWidget->GetText();
    }

    // Button: skip internal Quad/Text children (they're auto-created)
    bool isButton = (widget->As<Button>() != nullptr);
    uint32_t childCount = isButton ? 0 : widget->GetNumChildren();

    if (textContent.empty() && childCount == 0)
    {
        outXml += " />\n";
    }
    else
    {
        outXml += ">";

        if (!textContent.empty())
        {
            outXml += textContent;
        }

        if (childCount > 0)
        {
            outXml += "\n";
            for (uint32_t i = 0; i < widget->GetNumChildren(); ++i)
            {
                Widget* child = widget->GetChild(i)->As<Widget>();
                if (child)
                {
                    ExportWidgetTreeToXML(child, outXml, indent + 1);
                }
            }
            outXml += pad;
        }

        outXml += "</" + std::string(element) + ">\n";
    }
}

static void CreateNewAsset(TypeId assetType, const char* assetName, bool isSkybox = false, bool userProvidedName = false)
{
    AssetStub* stub = nullptr;
    AssetDir* currentDir = GetEditorState()->GetAssetDirectory();

    if (currentDir == nullptr)
        return;

    stub = EditorAddUniqueAsset(assetName, currentDir, assetType, true);

    if (isSkybox && stub != nullptr && stub->mAsset != nullptr)
    {
        MaterialLite* skyMat = stub->mAsset->As<MaterialLite>();
        if (skyMat != nullptr)
        {
            skyMat->SetShadingModel(ShadingModel::Unlit);
            skyMat->SetCullMode(CullMode::Front);
            skyMat->SetDepthTestDisabled(true);
            skyMat->SetSortPriority(-1000);
            skyMat->SetApplyFog(false);
            skyMat->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
            AssetManager::Get()->SaveAsset(*stub);
        }
    }

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

            // Only auto-rename if user didn't provide a custom name
            if (!userProvidedName)
            {
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
    }

    if (assetType == UIDocument::GetStaticType())
    {
        if (stub != nullptr && stub->mAsset != nullptr)
        {
            UIDocument* doc = stub->mAsset->As<UIDocument>();

            // Write the .xml source file alongside the .oct asset
            std::string xmlPath = currentDir->mPath + stub->mName + ".xml";
            FILE* xmlFile = fopen(xmlPath.c_str(), "w");
            if (xmlFile != nullptr)
            {
                fputs(doc->GetXmlSource().c_str(), xmlFile);
                fclose(xmlFile);
            }

            doc->Import(xmlPath, nullptr);
        }
    }

    if (stub != nullptr)
    {
        AssetManager::Get()->SaveAsset(*stub);
    }
}

static void CreateNewScene(const char* sceneName, int sceneType, bool createCamera)
{
    AssetDir* currentDir = GetEditorState()->GetAssetDirectory();
    if (currentDir == nullptr)
        return;

    // Check if this is a plugin-registered scene type
    EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
    if (hookMgr != nullptr && sceneType >= 2)
    {
        const auto& sceneTypes = hookMgr->GetSceneTypes();
        int hookIndex = sceneType - 2;
        if (hookIndex >= 0 && hookIndex < (int)sceneTypes.size())
        {
            AssetStub* stub = EditorAddUniqueAsset(sceneName, currentDir, Scene::GetStaticType(), true);
            if (stub == nullptr)
                return;

            // Create a temporary root for plugin to populate
            SharedPtr<Node3D> root = Node::Construct<Node3D>();
            root->SetName("Root");

            sceneTypes[hookIndex].mCreateFunc(sceneName, root.Get(), sceneTypes[hookIndex].mUserData);

            Scene* scene = (Scene*)stub->mAsset;
            scene->Capture(root.Get());
            AssetManager::Get()->SaveAsset(*stub);
            return;
        }
    }

    // Built-in scene types (0=2D, 1=3D)
    AssetStub* stub = EditorAddUniqueAsset(sceneName, currentDir, Scene::GetStaticType(), true);
    if (stub == nullptr)
        return;

    if (sceneType == 1) // 3D
    {
        SharedPtr<Node3D> root = Node::Construct<Node3D>();
        root->SetName("Root");

        if (createCamera)
        {
            Camera3D* cam = root->CreateChild<Camera3D>("Camera3D");
            cam->SetPosition(glm::vec3(0.0f, 0.0f, 5.0f));
        }

        Scene* scene = (Scene*)stub->mAsset;
        scene->Capture(root.Get());
    }
    else // 2D
    {
        SharedPtr<Widget> root = Node::Construct<Widget>();
        root->SetName("Root");

        if (createCamera)
        {
            root->CreateChild<Widget>("Canvas");
        }

        Scene* scene = (Scene*)stub->mAsset;
        scene->Capture(root.Get());
    }

    AssetManager::Get()->SaveAsset(*stub);
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
        if (ImGui::Button(ICON_MATERIAL_SYMBOLS_TARGET) && node)
        {
            GetEditorState()->SetSelectedNode(node);
        }
    }
    else if (IsAltDown())
    {
        if (ImGui::Button(ICON_INFO) && node)
        {
            GetEditorState()->InspectObject(node);
        }
    }
    else
    {
        if (ImGui::Button(ICON_MDI_TARGET))
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
            IsKeyJustDown(OCTAVE_KEY_DELETE))
        {
            am->EXE_EditProperty(owner, ownerType, prop.mName, index, (Node*) nullptr);
        }
    }

    ImGui::SameLine();

    static std::string sTempString;
    static ImGuiID sActiveInputId = 0;

    Node* src = owner->As<Node>();

    if (!src && owner->As<Script>())
    {
        src = owner->As<Script>()->GetOwner();
    }

    // Only update the temp string when not actively editing this field
    // Otherwise user's input gets overwritten every frame
    ImGuiID inputId = ImGui::GetID("##NodeNameStr");
    if (sActiveInputId != inputId)
    {
        sTempString = FindRelativeNodePath(src, node);
    }

    ImGui::InputText("##NodeNameStr", &sTempString);

    if (ImGui::IsItemActive())
    {
        sActiveInputId = inputId;
    }
    else if (sActiveInputId == inputId)
    {
        sActiveInputId = 0;
    }

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

    // Per-element delete button for vector properties
    if (prop.IsVector() && prop.GetCount() > prop.mMinCount)
    {
        ImGui::SameLine(0.0f, 2.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.2f, 0.2f, 1.0f));
        if (ImGui::Button(ICON_RAPHAEL_NO "##NodeDelete"))
        {
            if (prop.IsExternal())
            {
                prop.EraseVector(index);
            }
            else
            {
                prop.Erase(index);
                if (prop.GetCount() > 0)
                {
                    prop.SetValue(prop.GetValue(0), 0, prop.GetCount());
                }
                else if (prop.mChangeHandler)
                {
                    prop.mChangeHandler(&prop, 0, nullptr);
                }
            }
        }
        ImGui::PopStyleColor();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Remove element [%d]", index);
    }
}

// A reusable autocomplete dropdown function that works with various input types
// Returns true if a selection was made, and updates the input string
template<typename FilterFuncType>
static bool DrawAutocompleteDropdown(const char* dropdownId,
                                     std::string& inputText,
                                     const std::vector<std::string>& suggestions,
                                     FilterFuncType filterFunc,
                                     bool forceActive = false,
                                     ImGuiID overrideInputId = 0,
                                     ImVec2 overrideRectMin = ImVec2(0, 0),
                                     ImVec2 overrideRectMax = ImVec2(0, 0))
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

    // Use overridden input identity/rect if provided (when other widgets are drawn between InputText and this call)
    bool hasOverride = (overrideInputId != 0);
    ImGuiID inputId = hasOverride ? overrideInputId : ImGui::GetItemID();
    bool isInputActive = hasOverride ? forceActive : ImGui::IsItemActive();
    bool isInputFocused = hasOverride ? forceActive : ImGui::IsItemFocused();
    ImVec2 inputRectMin = (overrideRectMax.x > overrideRectMin.x) ? overrideRectMin : ImGui::GetItemRectMin();
    ImVec2 inputRectMax = (overrideRectMax.x > overrideRectMin.x) ? overrideRectMax : ImGui::GetItemRectMax();

    // If forceActive is true, force the dropdown to show
    // But not if a selection was just made (prevents immediate reopen)
    if (forceActive && !selectionJustMade) {
        dropdownActive = true;
    }
    // Reset the flag after one frame
    selectionJustMade = false;

    // If we have a new active item, reset state
    if (activeDropdownId != inputId && (isInputActive || isInputFocused))
    {
        activeDropdownId = inputId;
        hasSelection = false;
        dropdownActive = true; // Set to true to ensure dropdown shows
        selectedIndex = 0;
    }
    // Hide dropdown when input loses focus
    else if (!isInputActive && !isInputFocused && activeDropdownId == inputId)
    {
        dropdownActive = false;
    }

    // Close dropdown when user clicks outside the input field
    if (dropdownActive && activeDropdownId == inputId && ImGui::IsMouseClicked(0))
    {
        ImVec2 mousePos = ImGui::GetIO().MousePos;
        if (mousePos.x < inputRectMin.x || mousePos.x > inputRectMax.x ||
            mousePos.y < inputRectMin.y || mousePos.y > inputRectMax.y)
        {
            dropdownActive = false;
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
        ImVec2 inputSize = ImVec2(inputRectMax.x - inputRectMin.x, inputRectMax.y - inputRectMin.y);
        ImGui::SetNextWindowPos(ImVec2(inputRectMin.x, inputRectMin.y + inputSize.y));

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
        if (ImGui::Button(ICON_IC_BASELINE_UPLOAD) && asset)
        {
            GetEditorState()->BrowseToAsset(asset->GetName());
        }
    }
    else if (IsAltDown())
    {
        if (ImGui::Button(ICON_INFO) && asset)
        {
            GetEditorState()->InspectObject(asset);
        }
    }
    else
    {
        if (ImGui::Button(ICON_IC_BASELINE_DOWNLOAD))
        {
            Asset* selAsset = GetEditorState()->GetSelectedAsset();
            if (selAsset != nullptr)
            {
                AssignAssetToProperty(owner, ownerType, prop, index, selAsset);
            }
        }

        if (asset != nullptr &&
            ImGui::IsItemHovered() &&
            IsKeyJustDown(OCTAVE_KEY_DELETE))
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

    // Save InputText rect/ID for the autocomplete dropdown positioning
    ImGuiID inputTextId = ImGui::GetItemID();
    ImVec2 inputTextRectMin = ImGui::GetItemRectMin();
    ImVec2 inputTextRectMax = ImGui::GetItemRectMax();

    // Drag-and-drop target on the input text field
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(DRAGDROP_ASSET))
        {
            AssetStub* droppedStub = *(AssetStub**)payload->Data;
            if (droppedStub != nullptr)
            {
                Asset* droppedAsset = LoadAsset(droppedStub->mName);
                AssignAssetToProperty(owner, ownerType, prop, index, droppedAsset);
            }
        }
        ImGui::EndDragDropTarget();
    }

    // Clear button
    ImGui::SameLine(0.0f, 2.0f);
    if (ImGui::Button(ICON_DASHICONS_NO_ALT "##AssetClear"))
    {
        Asset* nullAsset = nullptr;
        if (ownerType == PropertyOwnerType::Node || ownerType == PropertyOwnerType::Asset)
        {
            am->EXE_EditProperty(owner, ownerType, prop.mName, index, nullAsset);
        }
        else
        {
            prop.SetAsset(nullAsset, index);
        }
    }

    // Inspect button - view asset properties
    if (asset != nullptr)
    {
        ImGui::SameLine(0.0f, 2.0f);
        if (ImGui::Button(ICON_INFO "##AssetInspect"))
        {
            GetEditorState()->InspectObject(asset);
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Inspect asset properties");
    }

    // Per-element delete button for vector properties
    if (prop.IsVector() && prop.GetCount() > prop.mMinCount)
    {
        ImGui::SameLine(0.0f, 2.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.2f, 0.2f, 1.0f));
        if (ImGui::Button(ICON_RAPHAEL_NO "##AssetDelete"))
        {
            if (prop.IsExternal())
            {
                prop.EraseVector(index);
            }
            else
            {
                prop.Erase(index);
                if (prop.GetCount() > 0)
                {
                    prop.SetValue(prop.GetValue(0), 0, prop.GetCount());
                }
                else if (prop.mChangeHandler)
                {
                    prop.mChangeHandler(&prop, 0, nullptr);
                }
            }
        }
        ImGui::PopStyleColor();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Remove element [%d]", index);
    }

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
    // Use saved InputText rect so the dropdown positions below the input field, not the clear button
    bool selectionMade = DrawAutocompleteDropdown("AssetAutocomplete", sTempString, assetSuggestions, assetFilter,
                                                isInputActivated || textActive || willBeActivated,
                                                inputTextId, inputTextRectMin, inputTextRectMax);
    
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

        // Check addon property drawers (Batch 6)
        if (!custom)
        {
            EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
            if (hookMgr != nullptr)
            {
                // Match by property name - addon registers for specific property names
                custom = hookMgr->DrawPropertyDrawer(
                    prop.mName.c_str(), prop.mName.c_str(),
                    owner, (int32_t)propType);
            }
        }

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

            // Visual separation between array elements (skip first)
            if (prop.IsVector() && i > 0)
            {
                ImGui::Separator();
            }

            // Index label for vector properties
            if (prop.IsVector())
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
                ImGui::Text("[%d]", i);
                ImGui::PopStyleColor();
                ImGui::SameLine();
            }

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
                if (prop.mName == "Restart Script")
                {
                    if (ImGui::Button(ICON_IC_TWOTONE_POWER_SETTINGS_NEW "##RestartScript"))
                    {
                        bool val = true;
                        am->EXE_EditProperty(owner, ownerType, prop.mName, i, val);
                    }
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Restart Script");

                    // Draw Reload button on the same line
                    ImGui::SameLine(0.0f, 2.0f);
                    if (ImGui::Button(ICON_IC_SHARP_SETTINGS_BACKUP_RESTORE "##ReloadScriptFile"))
                    {
                        bool val = true;
                        std::string reloadName = "Reload Script File";
                        am->EXE_EditProperty(owner, ownerType, reloadName, 0, val);
                    }
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Reload Script File");
                    break;
                }
                else if (prop.mName == "Reload Script File")
                {
                    // Already drawn inline with "Restart Script" above
                    break;
                }

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

                    // Drop target for scripts dragged from Scripts panel
                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(DRAGDROP_SCRIPT))
                        {
                            std::string scriptName((const char*)payload->Data, payload->DataSize - 1);
                            am->EXE_EditProperty(owner, ownerType, prop.mName, i, scriptName);
                        }
                        ImGui::EndDragDropTarget();
                    }

                    // Capture input state IMMEDIATELY after InputText
                    bool isInputFocused = ImGui::IsItemFocused();
                    bool isInputActivated = ImGui::IsItemActivated();
                    ImGuiID scriptInputId = ImGui::GetItemID();
                    ImVec2 scriptInputMin = ImGui::GetItemRectMin();
                    ImVec2 scriptInputMax = ImGui::GetItemRectMax();

                    // Clear button
                    ImGui::SameLine(0.0f, 2.0f);
                    if (ImGui::Button(ICON_DASHICONS_NO_ALT "##ScriptClear"))
                    {
                        std::string emptyStr;
                        am->EXE_EditProperty(owner, ownerType, prop.mName, i, emptyStr);
                    }

                    // Then draw the autocomplete dropdown with the correct state
                    // Pass the InputText's saved rect so the dropdown positions below the input, not the clear button
                    bool selectionMade = DrawAutocompleteDropdown("ScriptAutocomplete", sTempString, scriptSuggestions, scriptFilter,
                        isInputActivated || textActive || isInputFocused, scriptInputId, scriptInputMin, scriptInputMax);

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
                    bool isMultiline = (prop.mName == "Options");

                    if (isMultiline)
                    {
                        ImGui::InputTextMultiline("", &sTempString, ImVec2(-1, 80));
                    }
                    else
                    {
                        ImGui::InputText("", &sTempString);
                    }

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
            case DatumType::Node3D:
            case DatumType::Audio3D:
            case DatumType::Widget:
            case DatumType::Text:
            case DatumType::Quad:
            case DatumType::Spline3D:
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

        AlternatingRowBackground();
        ImGui::PopID();
    }

    if (catIndented)
    {
        ImGui::Unindent(kCategoryIndentWidth);
    }
}

static void SpawnSceneAsNode(const std::string& sceneName, Node* parentNode)
{
    Scene* scene = LoadAsset<Scene>(sceneName);
    if (scene != nullptr)
    {
        Node* spawnedNode = ActionManager::Get()->EXE_SpawnNode(scene);
        if (spawnedNode)
        {
            Node* selNode = GetEditorState()->GetSelectedNode();
            Node* parent = parentNode ? parentNode : (selNode ? selNode : GetWorld(0)->GetRootNode());

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

static void BuildSceneCategoryMap()
{
    sScenesByCategory.clear();

    auto& assetMap = AssetManager::Get()->GetAssetMap();
    for (auto it : assetMap)
    {
        if (it.second &&
            it.second->mType == Scene::GetStaticType() &&
            !it.second->mEngineAsset)
        {
            Scene* scene = LoadAsset<Scene>(it.second->mName);
            if (scene)
            {
                const std::string& category = scene->GetMenuOverride();
                std::string key = category.empty() ? "Scene" : category;
                sScenesByCategory[key].push_back(it.second->mName);
            }
        }
    }

    // Sort each category alphabetically
    for (auto& pair : sScenesByCategory)
    {
        std::sort(pair.second.begin(), pair.second.end());
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

    // Build scene category map if needed
    if (sScenesByCategory.empty())
    {
        BuildSceneCategoryMap();
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

        ImGui::Separator();
        if (ImGui::BeginMenu("Skybox"))
        {
            if (ImGui::MenuItem(BASIC_SKYBOX_TEXTURED))
                am->SpawnBasicNode(BASIC_SKYBOX_TEXTURED, node, nullptr, false, {});
            if (ImGui::MenuItem(BASIC_SKYBOX_VERTEX_COLOR))
                am->SpawnBasicNode(BASIC_SKYBOX_VERTEX_COLOR, node, nullptr, false, {});
            ImGui::EndMenu();
        }

        // Draw scenes with "3D" menu override
        if (sScenesByCategory.count("3D") > 0)
        {
            ImGui::Separator();
            for (const auto& sceneName : sScenesByCategory["3D"])
            {
                if (ImGui::MenuItem(sceneName.c_str()))
                {
                    SpawnSceneAsNode(sceneName, node);
                }
            }
        }

        // Draw addon node menu items for "3D" category
        {
            EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
            if (hookMgr != nullptr) hookMgr->DrawNodeMenuItems("3D", node);
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

        // Draw scenes with "Widget" menu override
        if (sScenesByCategory.count("Widget") > 0)
        {
            ImGui::Separator();
            for (const auto& sceneName : sScenesByCategory["Widget"])
            {
                if (ImGui::MenuItem(sceneName.c_str()))
                {
                    SpawnSceneAsNode(sceneName, node);
                }
            }
        }

        // Draw addon node menu items for "Widget" category
        {
            EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
            if (hookMgr != nullptr) hookMgr->DrawNodeMenuItems("Widget", node);
        }

        ImGui::EndMenu();
    }

    // Default "Scene" menu - shows scenes with no menu override
    if (sScenesByCategory.count("Scene") > 0 && ImGui::BeginMenu("Scene"))
    {
        for (const auto& sceneName : sScenesByCategory["Scene"])
        {
            if (ImGui::MenuItem(sceneName.c_str()))
            {
                SpawnSceneAsNode(sceneName, node);
            }
        }
        ImGui::EndMenu();
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

        // Draw addon node menu items for "Other" category
        {
            EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
            if (hookMgr != nullptr) hookMgr->DrawNodeMenuItems("Other", node);
        }

        ImGui::EndMenu();
    }

    // Draw custom scene category menus (categories other than Scene, 3D, Widget, Other)
    // Supports nested paths like "ShootingGallery/Targets"
    {
        // Build a tree structure from category paths
        struct MenuNode
        {
            std::map<std::string, MenuNode> children;
            std::vector<std::string> scenes;
        };
        MenuNode rootMenu;

        for (const auto& pair : sScenesByCategory)
        {
            if (pair.first == "Scene" || pair.first == "3D" || pair.first == "Widget" || pair.first == "Other")
                continue;

            // Parse path segments (e.g., "ShootingGallery/Targets" -> ["ShootingGallery", "Targets"])
            std::vector<std::string> segments;
            std::string remaining = pair.first;
            size_t pos;
            while ((pos = remaining.find('/')) != std::string::npos)
            {
                std::string segment = remaining.substr(0, pos);
                if (!segment.empty())
                    segments.push_back(segment);
                remaining = remaining.substr(pos + 1);
            }
            if (!remaining.empty())
                segments.push_back(remaining);

            // Navigate/create the tree path
            MenuNode* current = &rootMenu;
            for (const auto& segment : segments)
            {
                current = &current->children[segment];
            }
            // Add scenes at this level
            for (const auto& sceneName : pair.second)
            {
                current->scenes.push_back(sceneName);
            }
        }

        // Recursive lambda to draw menu tree
        std::function<void(MenuNode&)> drawMenuTree = [&](MenuNode& menuNode)
        {
            // Draw child submenus first (sorted)
            for (auto& childPair : menuNode.children)
            {
                if (ImGui::BeginMenu(childPair.first.c_str()))
                {
                    drawMenuTree(childPair.second);
                    ImGui::EndMenu();
                }
            }
            // Draw scenes at this level
            for (const auto& sceneName : menuNode.scenes)
            {
                if (ImGui::MenuItem(sceneName.c_str()))
                {
                    SpawnSceneAsNode(sceneName, node);
                }
            }
        };

        // Draw top-level custom categories
        for (auto& topPair : rootMenu.children)
        {
            if (ImGui::BeginMenu(topPair.first.c_str()))
            {
                drawMenuTree(topPair.second);
                ImGui::EndMenu();
            }
        }
    }

    // Draw addon custom node categories
    {
        EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
        if (hookMgr != nullptr) hookMgr->DrawCustomNodeCategories(node);
    }

    // Clear scene category cache at end so it refreshes next time menu opens
    sScenesByCategory.clear();
}

static void DrawImportMenu(Node* node)
{
    ActionManager* am = ActionManager::Get();
    if (ImGui::MenuItem("Camera"))
    {
        am->BeginImportCamera();
    }

    // Draw addon import menu items (Batch 8)
    {
        EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
        if (hookMgr != nullptr) hookMgr->DrawImportMenuItems();
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

    if (ImGui::BeginMenu("Skybox"))
    {
        if (ImGui::MenuItem(BASIC_SKYBOX_TEXTURED))
            am->SpawnBasicNode(BASIC_SKYBOX_TEXTURED, node, selAsset, setFocusPos, spawnPos);
        if (ImGui::MenuItem(BASIC_SKYBOX_VERTEX_COLOR))
            am->SpawnBasicNode(BASIC_SKYBOX_VERTEX_COLOR, node, selAsset, setFocusPos, spawnPos);
        ImGui::EndMenu();
    }

    // Draw addon spawn basic 3D items
    {
        EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
        if (hookMgr != nullptr) hookMgr->DrawSpawnBasic3dItems(node);
    }
    if (ImGui::MenuItem(BASIC_NAV_MESH))
        am->SpawnBasicNode(BASIC_NAV_MESH, node, selAsset, setFocusPos, spawnPos);
    if (ImGui::MenuItem(BASIC_SPLINE))
        am->SpawnBasicNode(BASIC_SPLINE, node, selAsset, setFocusPos, spawnPos);
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

    // Draw addon spawn basic widget items
    {
        EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
        if (hookMgr != nullptr) hookMgr->DrawSpawnBasicWidgetItems(node);
    }
}

static void BuildHierarchyJson(Node* node, std::string& out, int depth)
{
    if (node == nullptr)
        return;

    auto indent = [&out](int d) {
        for (int i = 0; i < d; ++i)
            out += "  ";
    };

    indent(depth);
    out += "{\n";

    // Name
    indent(depth + 1);
    out += "\"name\": \"";
    out += node->GetName();
    out += "\",\n";

    // Type
    indent(depth + 1);
    out += "\"type\": \"";
    out += node->GetTypeName();
    out += "\"";

    // Script (if any)
    Script* script = node->GetScript();
    if (script != nullptr && !script->GetFile().empty())
    {
        out += ",\n";
        indent(depth + 1);
        out += "\"script\": \"";
        out += script->GetFile();
        out += "\"";
    }

    // Scene reference (if linked)
    if (node->IsSceneLinked() && node->GetScene() != nullptr)
    {
        out += ",\n";
        indent(depth + 1);
        out += "\"linkedScene\": \"";
        out += node->GetScene()->GetName();
        out += "\"";
    }

    // Children
    if (node->GetNumChildren() > 0)
    {
        out += ",\n";
        indent(depth + 1);
        out += "\"children\": [\n";

        for (uint32_t i = 0; i < node->GetNumChildren(); ++i)
        {
            BuildHierarchyJson(node->GetChild(i), out, depth + 2);
            if (i < node->GetNumChildren() - 1)
                out += ",";
            out += "\n";
        }

        indent(depth + 1);
        out += "]";
    }

    out += "\n";
    indent(depth);
    out += "}";
}

static void CopyNodeHierarchyToClipboard(Node* node)
{
    if (node == nullptr)
        return;

    std::string json;
    BuildHierarchyJson(node, json, 0);
    json += "\n";

    ImGui::SetClipboardText(json.c_str());
    LogDebug("Copied hierarchy JSON to clipboard (%d characters)", (int)json.size());
}

static void DrawPackageMenu()
{
    ActionManager* am = ActionManager::Get();
    bool buildRunning = am->IsBuildRunning();

    //if (ImGui::BeginPopup("PackagePopup"))
    //{
#if PLATFORM_WINDOWS
    if (ImGui::MenuItem("Windows", nullptr, false, !buildRunning))
        am->BuildData(Platform::Windows, false);
#elif PLATFORM_LINUX
    if (ImGui::MenuItem("Linux", nullptr, false, !buildRunning))
        am->BuildData(Platform::Linux, false);
#endif
    if (ImGui::MenuItem("Android", nullptr, false, !buildRunning))
        am->BuildData(Platform::Android, false);
    if (ImGui::MenuItem("GameCube", nullptr, false, !buildRunning))
        am->BuildData(Platform::GameCube, false);
    if (ImGui::MenuItem("Wii", nullptr, false, !buildRunning))
        am->BuildData(Platform::Wii, false);
    if (ImGui::MenuItem("3DS", nullptr, false, !buildRunning))
        am->BuildData(Platform::N3DS, false);
    if (ImGui::MenuItem("GameCube Embedded", nullptr, false, !buildRunning))
        am->BuildData(Platform::GameCube, true);
    if (ImGui::MenuItem("Wii Embedded", nullptr, false, !buildRunning))
        am->BuildData(Platform::Wii, true);
    if (ImGui::MenuItem("3DS Embedded", nullptr, false, !buildRunning))
        am->BuildData(Platform::N3DS, true);

    //    ImGui::EndPopup();
    //}
}

static int sAltRowIndex = 0;
static ImU32 sAltRowColor = 0;

static void BeginAlternatingRows()
{
    sAltRowIndex = 0;
    const auto& style = ImGui::GetStyle();
    ImVec4 windowBg = style.Colors[ImGuiCol_WindowBg];
    float luminance = windowBg.x * 0.299f + windowBg.y * 0.587f + windowBg.z * 0.114f;
    if (luminance < 0.5f)
        sAltRowColor = ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 0.02f));
    else
        sAltRowColor = ImGui::ColorConvertFloat4ToU32(ImVec4(0.0f, 0.0f, 0.0f, 0.03f));
}

static void AlternatingRowBackground()
{
    if (sAltRowIndex % 2 == 1)
    {
        ImVec2 min = ImGui::GetItemRectMin();
        ImVec2 max = ImGui::GetItemRectMax();
        min.x = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMin().x;
        max.x = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
        ImGui::GetWindowDrawList()->AddRectFilled(min, max, sAltRowColor);
    }
    sAltRowIndex++;
}

static void DrawScenePanel()
{
    ActionManager* am = ActionManager::Get();

    // Drag-and-drop reparenting/reordering state
    enum class SceneDropZone { None, Above, Into, Below };
    static SceneDropZone sDropZone = SceneDropZone::None;
    static Node* sDropTargetNode = nullptr;
    static ImVec2 sDropLineP1, sDropLineP2;
    static ImVec2 sDropHighlightMin, sDropHighlightMax;
    sDropZone = SceneDropZone::None;
    sDropTargetNode = nullptr;

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

    // Screen filter combo for 3DS dual-screen filtering
    {
        static const char* sScreenFilterNames[] = { "All Screens", "Top Screen", "Bottom Screen" };
        int screenFilterIdx = GetEditorState()->mSceneScreenFilter + 1; // -1->0, 0->1, 1->2
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::Combo("##ScreenFilter", &screenFilterIdx, sScreenFilterNames, 3))
        {
            GetEditorState()->mSceneScreenFilter = screenFilterIdx - 1; // 0->-1, 1->0, 2->1
        }
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

        // Screen filter: hide subtrees targeting a different screen
        if (GetEditorState()->mSceneScreenFilter >= 0 && node->GetParent() == rootNode)
        {
            if (node->GetTargetScreen() != (uint8_t)GetEditorState()->mSceneScreenFilter)
            {
                return; // Skip this subtree entirely in the hierarchy
            }
        }

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
            AlternatingRowBackground();
            bool nodeClicked = ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen();
            bool nodeMiddleClicked = ImGui::IsItemClicked(ImGuiMouseButton_Middle);
            bool expandChildren = trackingNode || (nodeMiddleClicked && IsControlDown());
            bool collapseChildren = !expandChildren && nodeMiddleClicked;

            // Hierarchy item GUI overlay (Batch 7)
            {
                ImVec2 rowMin = ImGui::GetItemRectMin();
                ImVec2 rowMax = ImGui::GetItemRectMax();
                EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
                if (hookMgr != nullptr)
                {
                    hookMgr->DrawHierarchyItemGUI(node, rowMin.x, rowMin.y,
                        rowMax.x - rowMin.x, rowMax.y - rowMin.y);
                }
            }

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

            // Drop target for node reparenting/reordering and asset drag-and-drop
            if (ImGui::BeginDragDropTarget())
            {
                // --- Node reparent / reorder ---
                if (const ImGuiPayload* peekPayload = ImGui::AcceptDragDropPayload(DRAGDROP_NODE, ImGuiDragDropFlags_AcceptPeekOnly))
                {
                    Node* dragNode = *(Node**)peekPayload->Data;

                    // Validate: not self, not dropping onto descendant, not scene-linked target, not world root
                    bool validDrop = (dragNode != nullptr && dragNode != node &&
                                      !node->HasAncestor(dragNode) &&
                                      !node->IsSceneLinkedChild() &&
                                      node != rootNode);

                    if (validDrop)
                    {
                        ImVec2 itemMin = ImGui::GetItemRectMin();
                        ImVec2 itemMax = ImGui::GetItemRectMax();
                        float itemHeight = itemMax.y - itemMin.y;
                        float mouseY = ImGui::GetMousePos().y;
                        float relY = mouseY - itemMin.y;

                        SceneDropZone zone = SceneDropZone::Into;
                        if (relY < itemHeight * 0.25f)
                            zone = SceneDropZone::Above;
                        else if (relY > itemHeight * 0.75f)
                            zone = SceneDropZone::Below;

                        // For Above/Below, the target parent must exist (not root-level reorder with no parent)
                        if ((zone == SceneDropZone::Above || zone == SceneDropZone::Below) && node->GetParent() == nullptr)
                            zone = SceneDropZone::Into;

                        sDropZone = zone;
                        sDropTargetNode = node;

                        if (zone == SceneDropZone::Into)
                        {
                            sDropHighlightMin = itemMin;
                            sDropHighlightMax = itemMax;
                        }
                        else if (zone == SceneDropZone::Above)
                        {
                            sDropLineP1 = ImVec2(itemMin.x, itemMin.y);
                            sDropLineP2 = ImVec2(itemMax.x, itemMin.y);
                        }
                        else // Below
                        {
                            sDropLineP1 = ImVec2(itemMin.x, itemMax.y);
                            sDropLineP2 = ImVec2(itemMax.x, itemMax.y);
                        }

                        // Actual delivery
                        if (const ImGuiPayload* deliverPayload = ImGui::AcceptDragDropPayload(DRAGDROP_NODE))
                        {
                            Node* droppedNode = *(Node**)deliverPayload->Data;

                            if (zone == SceneDropZone::Into)
                            {
                                am->EXE_AttachNode(droppedNode, node, -1, -1);
                            }
                            else
                            {
                                Node* targetParent = node->GetParent();
                                int32_t siblingIndex = targetParent->FindChildIndex(node);
                                int32_t insertIndex = (zone == SceneDropZone::Below) ? siblingIndex + 1 : siblingIndex;

                                // If dragging within the same parent, account for removal shift
                                if (droppedNode->GetParent() == targetParent)
                                {
                                    int32_t dragIndex = targetParent->FindChildIndex(droppedNode);
                                    if (dragIndex < insertIndex)
                                        insertIndex--;
                                }

                                am->EXE_AttachNode(droppedNode, targetParent, insertIndex, -1);
                            }

                            sDropZone = SceneDropZone::None;
                            sDropTargetNode = nullptr;
                        }
                    }
                }

                // --- Asset drag-and-drop (StaticMesh → hierarchy node) ---
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
                    else if (droppedStub != nullptr && droppedStub->mType == Scene::GetStaticType())
                    {
                        if (droppedStub->mAsset == nullptr)
                            AssetManager::Get()->LoadAsset(*droppedStub);
                        if (droppedStub->mAsset != nullptr)
                        {
                            ActionManager::Get()->SpawnBasicNode(BASIC_SCENE,
                                node, droppedStub->mAsset, false, glm::vec3(0.0f));
                        }
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
                if (!inSubScene && ImGui::Selectable("Duplicate", false, ImGuiSelectableFlags_DontClosePopups))
                {
                    ImGui::OpenPopup("Duplicate Node (Context)");
                    std::string defaultName = node->GetName() + "_00";
                    strncpy(sPopupInputBuffer, defaultName.c_str(), kPopupInputBufferSize - 1);
                    sPopupInputBuffer[kPopupInputBufferSize - 1] = '\0';
                    setTextInputFocus = true;
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
                if ((nodeSceneLinked || inSubScene) && ImGui::Selectable("Reset Scene"))
                {
                    am->EXE_ResetScene(node->GetSubRoot());
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

                if (!nodeSceneLinked && !inSubScene && node->GetNumChildren() > 0 && ImGui::Selectable("Create Scene...", false, ImGuiSelectableFlags_DontClosePopups))
                {
                    ImGui::OpenPopup("Create Scene From Node");
                    std::string defaultName = "SC_" + node->GetName();
                    strncpy(sPopupInputBuffer, defaultName.c_str(), kPopupInputBufferSize - 1);
                    sPopupInputBuffer[kPopupInputBufferSize - 1] = '\0';
                    sCreateSceneTargetNode = node;
                    sCreateSceneTargetDir = GetEditorState()->GetAssetDirectory();
                    setTextInputFocus = true;
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

                // Widget XML export options
                if (node->As<Widget>())
                {
                    ImGui::Separator();
                    if (ImGui::Selectable("Export XML..."))
                    {
                        std::string xml;
                        ExportWidgetTreeToXML(node->As<Widget>(), xml);

                        std::string savePath = SYS_SaveFileDialog();
                        if (!savePath.empty())
                        {
                            // Ensure .xml extension
                            if (savePath.size() < 4 || savePath.substr(savePath.size() - 4) != ".xml")
                                savePath += ".xml";

                            FILE* f = fopen(savePath.c_str(), "w");
                            if (f)
                            {
                                fwrite(xml.c_str(), 1, xml.size(), f);
                                fclose(f);
                                LogDebug("Exported XML to: %s", savePath.c_str());
                            }
                            else
                            {
                                LogError("Failed to write XML file: %s", savePath.c_str());
                            }
                        }
                    }
                    if (ImGui::Selectable("Create UIDocument From Hierarchy"))
                    {
                        std::string xml;
                        ExportWidgetTreeToXML(node->As<Widget>(), xml);

                        AssetDir* currentDir = GetEditorState()->GetAssetDirectory();
                        if (currentDir)
                        {
                            std::string assetName = "UI_" + node->GetName();
                            AssetStub* stub = EditorAddUniqueAsset(assetName.c_str(), currentDir, UIDocument::GetStaticType(), true);
                            if (stub && stub->mAsset)
                            {
                                UIDocument* uiDoc = stub->mAsset->As<UIDocument>();
                                if (uiDoc)
                                {
                                    uiDoc->SetXmlSource(xml);
                                    AssetManager::Get()->SaveAsset(*stub);
                                    LogDebug("Created UIDocument '%s' from widget hierarchy.", assetName.c_str());
                                }
                            }
                        }
                    }
                }

                ImGui::Separator();
                if (ImGui::Selectable("Copy Hierarchy"))
                {
                    CopyNodeHierarchyToClipboard(node);
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

                if (!inSubScene && ImGui::BeginPopup("Duplicate Node (Context)"))
                {
                    if (setTextInputFocus)
                    {
                        ImGui::SetKeyboardFocusHere();
                    }

                    if (ImGui::InputText("New Name##DupNodeCtx", sPopupInputBuffer, kPopupInputBufferSize, ImGuiInputTextFlags_EnterReturnsTrue))
                    {
                        am->DuplicateNodes({ node });

                        const std::vector<Node*>& newSelNodes = GetEditorState()->GetSelectedNodes();
                        if (!newSelNodes.empty())
                        {
                            std::string newName = sPopupInputBuffer;
                            am->EXE_EditProperty(newSelNodes[0], PropertyOwnerType::Node, "Name", 0, newName);
                        }

                        ImGui::CloseCurrentPopup();
                        closeContextPopup = true;
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

                if (ImGui::BeginPopup("Create Scene From Node"))
                {
                    if (setTextInputFocus)
                    {
                        ImGui::SetKeyboardFocusHere();
                    }

                    ImGui::Text("Scene name:");
                    ImGui::InputText("##CreateSceneName", sPopupInputBuffer, kPopupInputBufferSize);

                    ImGui::Spacing();
                    ImGui::Text("Output directory:");
                    if (sCreateSceneTargetDir != nullptr)
                    {
                        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", sCreateSceneTargetDir->mPath.c_str());
                    }

                    ImGui::Spacing();
                    if (ImGui::TreeNode("Select Directory"))
                    {
                        AssetDir* rootDir = AssetManager::Get()->GetRootDirectory();
                        if (rootDir != nullptr)
                        {
                            ImGui::BeginChild("DirTree", ImVec2(300, 150), true);
                            for (AssetDir* child : rootDir->mChildDirs)
                            {
                                DrawDirPickerTree(child, &sCreateSceneTargetDir);
                            }
                            ImGui::EndChild();
                        }
                        ImGui::TreePop();
                    }

                    ImGui::Spacing();
                    bool canCreate = (sCreateSceneTargetNode != nullptr && strlen(sPopupInputBuffer) > 0 && sCreateSceneTargetDir != nullptr);
                    if (!canCreate)
                        ImGui::BeginDisabled();
                    if (ImGui::Button("Create"))
                    {
                        AssetStub* stub = EditorAddUniqueAsset(sPopupInputBuffer, sCreateSceneTargetDir, Scene::GetStaticType(), true);
                        GetEditorState()->CaptureAndSaveScene(stub, sCreateSceneTargetNode);
                        sCreateSceneTargetNode = nullptr;
                        sCreateSceneTargetDir = nullptr;
                        ImGui::CloseCurrentPopup();
                        closeContextPopup = true;
                    }
                    if (!canCreate)
                        ImGui::EndDisabled();

                    ImGui::SameLine();
                    if (ImGui::Button("Cancel"))
                    {
                        sCreateSceneTargetNode = nullptr;
                        sCreateSceneTargetDir = nullptr;
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

    BeginAlternatingRows();

    if (rootNode != nullptr)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 6.0f);
        drawTree(rootNode, nullptr);
        ImGui::PopStyleVar();
    }

    // Draw drag-and-drop visual indicators
    if (sDropZone != SceneDropZone::None && sDropTargetNode != nullptr)
    {
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const ImU32 dropColorFill = IM_COL32(60, 120, 220, 40);
        const ImU32 dropColorBorder = IM_COL32(60, 120, 220, 180);
        const ImU32 dropLineColor = IM_COL32(60, 120, 220, 255);

        if (sDropZone == SceneDropZone::Into)
        {
            drawList->AddRectFilled(sDropHighlightMin, sDropHighlightMax, dropColorFill);
            drawList->AddRect(sDropHighlightMin, sDropHighlightMax, dropColorBorder);
        }
        else
        {
            drawList->AddLine(sDropLineP1, sDropLineP2, dropLineColor, 2.0f);
            drawList->AddCircleFilled(sDropLineP1, 3.0f, dropLineColor);
        }
    }

    // Root-level drop target for node unparenting and asset drag-and-drop (empty space below tree)
    ImVec2 avail = ImGui::GetContentRegionAvail();
    if (avail.y > 0.0f)
    {
        ImGui::InvisibleButton("##SceneAssetDropTarget", ImVec2(avail.x, avail.y));
        if (ImGui::BeginDragDropTarget())
        {
            // Node drop → unparent to root
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(DRAGDROP_NODE))
            {
                Node* droppedNode = *(Node**)payload->Data;
                if (droppedNode != nullptr &&
                    droppedNode != rootNode &&
                    !droppedNode->IsSceneLinkedChild())
                {
                    am->EXE_AttachNode(droppedNode, rootNode, -1, -1);
                }
            }

            // Asset drop
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
                else if (droppedStub != nullptr && droppedStub->mType == Scene::GetStaticType())
                {
                    if (droppedStub->mAsset == nullptr)
                        AssetManager::Get()->LoadAsset(*droppedStub);
                    if (droppedStub->mAsset != nullptr)
                    {
                        ActionManager::Get()->SpawnBasicNode(BASIC_SCENE,
                            nullptr, droppedStub->mAsset, false, glm::vec3(0.0f));
                    }
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

            if (IsKeyJustDown(OCTAVE_KEY_MINUS))
            {
                am->EXE_AttachNode(node, parent, glm::max<int32_t>(childIndex - 1, 0), -1);
            }
            else if (IsKeyJustDown(OCTAVE_KEY_PLUS))
            {
                am->EXE_AttachNode(node, parent, childIndex + 1, -1);
            }
        }

        if (selNodes.size() > 0)
        {
            if (IsKeyJustDown(OCTAVE_KEY_DELETE))
            {
                am->EXE_DeleteNodes(selNodes);
            }
            else if (ctrlDown && IsKeyJustDown(OCTAVE_KEY_D))
            {
                if (selNodes.size() == 1)
                {
                    ImGui::OpenPopup("Duplicate Node Name");
                    std::string defaultName = selNodes[0]->GetName() + "_00";
                    strncpy(sPopupInputBuffer, defaultName.c_str(), kPopupInputBufferSize - 1);
                    sPopupInputBuffer[kPopupInputBufferSize - 1] = '\0';
                    sDuplicateNodeFocus = true;
                }
                else
                {
                    am->DuplicateNodes(selNodes);
                }
            }
            else if (!ctrlDown && IsKeyJustDown(OCTAVE_KEY_F2))
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

    if (ImGui::BeginPopup("Duplicate Node Name"))
    {
        if (sDuplicateNodeFocus)
        {
            ImGui::SetKeyboardFocusHere();
            sDuplicateNodeFocus = false;
        }

        if (ImGui::InputText("New Name", sPopupInputBuffer, kPopupInputBufferSize, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            const std::vector<Node*>& selNodes = GetEditorState()->GetSelectedNodes();
            if (selNodes.size() == 1)
            {
                am->DuplicateNodes({ selNodes[0] });

                const std::vector<Node*>& newSelNodes = GetEditorState()->GetSelectedNodes();
                if (!newSelNodes.empty())
                {
                    std::string newName = sPopupInputBuffer;
                    am->EXE_EditProperty(newSelNodes[0], PropertyOwnerType::Node, "Name", 0, newName);
                }
            }
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
    static bool sNewAssetIsSkybox = false;

    ActionManager* actMan = ActionManager::Get();
    AssetManager* assMan = AssetManager::Get();

    AssetDir* curDir = GetEditorState()->GetAssetDirectory();

    // When right-clicking empty space (no dir/asset specified), default to root directory
    if (dir == nullptr && stub == nullptr)
    {
        if (GetEditorState()->mActiveAssetTab == AssetBrowserTab::Project)
            curDir = AssetManager::Get()->FindProjectDirectory();
        else
            curDir = AssetManager::Get()->FindPackagesDirectory();
    }

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

    if (stub && stub->mType == UIDocument::GetStaticType())
    {
        if (ImGui::Selectable("Edit"))
        {
            if (!stub->mAsset)
                AssetManager::Get()->LoadAsset(*stub);

            UIDocument* doc = stub->mAsset ? stub->mAsset->As<UIDocument>() : nullptr;
            if (doc)
            {
                const std::string& srcPath = doc->GetSourceFilePath();
                if (!srcPath.empty() && SYS_DoesFileExist(srcPath.c_str(), false))
                {
                    PreferencesModule* mod = PreferencesManager::Get()->FindModule("External/Editors");
                    EditorsModule* editors = mod ? static_cast<EditorsModule*>(mod) : nullptr;
                    if (editors)
                        editors->OpenLuaScript(srcPath);
                }
            }
        }
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

    // Make Zoo — available when multiple StaticMesh assets are selected
    {
        const auto& multiStubs = GetEditorState()->GetSelectedAssetStubs();
        bool hasStaticMeshes = false;
        for (AssetStub* s : multiStubs)
            if (s && s->mType == StaticMesh::GetStaticType()) hasStaticMeshes = true;

        if (hasStaticMeshes && ImGui::Selectable("Make Zoo", false, ImGuiSelectableFlags_DontClosePopups))
        {
            ImGui::OpenPopup("Make Zoo");
            strncpy(sPopupInputBuffer, "SC_Zoo", kPopupInputBufferSize - 1);
            sPopupInputBuffer[kPopupInputBufferSize - 1] = '\0';
            setTextInputFocus = true;
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
                const auto& selectedStubs = GetEditorState()->GetSelectedAssetStubs();
                if (selectedStubs.size() > 1 && GetEditorState()->IsAssetStubSelected(stub))
                {
                    std::vector<AssetStub*> toDelete = selectedStubs;
                    for (AssetStub* s : toDelete)
                    {
                        actMan->DeleteAsset(s);
                    }
                }
                else
                {
                    actMan->DeleteAsset(stub);
                }
            }
            else if (dir)
            {
                actMan->DeleteAssetDir(dir);
                GetEditorState()->ClearAssetDirHistory();
            }
        }

        if (stub && ImGui::Selectable("Duplicate", false, ImGuiSelectableFlags_DontClosePopups))
        {
            sDuplicateAssetStub = stub;
            ImGui::OpenPopup("Duplicate Asset (Context)");
            std::string defaultName = stub->mName + "_00";
            strncpy(sPopupInputBuffer, defaultName.c_str(), kPopupInputBufferSize - 1);
            sPopupInputBuffer[kPopupInputBufferSize - 1] = '\0';
            setTextInputFocus = true;
        }
    }

    // Reveal in Explorer — available for both assets and directories regardless of read-only
    if (stub || dir)
    {
        if (ImGui::Selectable("Reveal in Explorer"))
        {
            if (stub)
            {
                // Reveal the asset file, selecting it in the explorer
                std::string absPath = SYS_GetAbsolutePath(stub->mPath);
#if PLATFORM_WINDOWS
                // Replace forward slashes with backslashes for Windows explorer
                for (char& c : absPath) { if (c == '/') c = '\\'; }
                SYS_Exec(("explorer /select,\"" + absPath + "\"").c_str());
#elif PLATFORM_LINUX
                // Open the containing directory
                std::string dirPath = absPath.substr(0, absPath.find_last_of('/'));
                SYS_Exec(("xdg-open \"" + dirPath + "\" &").c_str());
#endif
            }
            else if (dir)
            {
                std::string absPath = SYS_GetAbsolutePath(dir->mPath);
#if PLATFORM_WINDOWS
                for (char& c : absPath) { if (c == '/') c = '\\'; }
                SYS_Exec(("explorer \"" + absPath + "\"").c_str());
#elif PLATFORM_LINUX
                SYS_Exec(("xdg-open \"" + absPath + "\" &").c_str());
#endif
            }
        }

        if (stub && ImGui::Selectable("Copy Path"))
        {
            // Build the relative path from project root to the asset
            std::string relativePath;
            AssetDir* projDir = AssetManager::Get()->FindProjectDirectory();
            AssetDir* dir = stub->mDirectory;

            // Walk up the directory tree to build the path
            std::vector<std::string> pathParts;
            while (dir != nullptr && dir != projDir)
            {
                pathParts.push_back(dir->mName);
                dir = dir->mParentDir;
            }

            // Build path from root to asset
            for (int i = (int)pathParts.size() - 1; i >= 0; --i)
            {
                relativePath += pathParts[i] + "/";
            }
            relativePath += stub->mName;

            ImGui::SetClipboardText(relativePath.c_str());
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

        if (ImGui::Selectable("Import TinyLLM Model"))
        {
            actMan->ImportTinyLLMModel();
        }

        if (ImGui::Selectable("Import TinyLLM Tokenizer"))
        {
            actMan->ImportTinyLLMTokenizer();
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
            if (ImGui::Selectable("Skybox Material", false, ImGuiSelectableFlags_DontClosePopups))
            {
                sNewAssetType = MaterialLite::GetStaticType();
                sNewAssetIsSkybox = true;
                showPopup = true;
            }
            if (ImGui::Selectable("Particle System", false, ImGuiSelectableFlags_DontClosePopups))
            {
                sNewAssetType = ParticleSystem::GetStaticType();
                showPopup = true;
            }
            bool showScenePopup = false;
            if (ImGui::Selectable("Scene", false, ImGuiSelectableFlags_DontClosePopups))
            {
                showScenePopup = true;
            }
            if (ImGui::Selectable("Timeline", false, ImGuiSelectableFlags_DontClosePopups))
            {
                sNewAssetType = Timeline::GetStaticType();
                showPopup = true;
            }
            if (ImGui::Selectable("Node Graph", false, ImGuiSelectableFlags_DontClosePopups))
            {
                sNewAssetType = NodeGraphAsset::GetStaticType();
                showPopup = true;
            }
            if (ImGui::Selectable("UI Document", false, ImGuiSelectableFlags_DontClosePopups))
            {
                sNewAssetType = UIDocument::GetStaticType();
                showPopup = true;
            }
            if (ImGui::Selectable("Data Asset", false, ImGuiSelectableFlags_DontClosePopups))
            {
                sNewAssetType = DataAsset::GetStaticType();
                showPopup = true;
            }

            ImGui::Separator();

            bool showCssPopup = false;
            if (ImGui::Selectable("CSS Stylesheet", false, ImGuiSelectableFlags_DontClosePopups))
            {
                showCssPopup = true;
            }

            // Draw addon create asset items
            {
                EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
                if (hookMgr != nullptr) hookMgr->DrawCreateAssetItems();
            }

            ImGui::EndMenu();

            if (showPopup)
            {
                ImGui::OpenPopup("New Asset Name");
                sPopupInputBuffer[0] = '\0';
                setTextInputFocus = true;
            }

            if (showScenePopup)
            {
                ImGui::OpenPopup("New Scene");
                sPopupInputBuffer[0] = '\0';
                sNewSceneType = 1;  // Default to 3D
                sNewSceneCreateCamera = true;
                setTextInputFocus = true;
            }

            if (showCssPopup)
            {
                ImGui::OpenPopup("New CSS Stylesheet");
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

    if (ImGui::BeginPopup("Duplicate Asset (Context)"))
    {
        if (setTextInputFocus)
        {
            ImGui::SetKeyboardFocusHere();
        }

        if (ImGui::InputText("New Name##DupAssetCtx", sPopupInputBuffer, kPopupInputBufferSize, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            if (sDuplicateAssetStub != nullptr)
            {
                GetEditorState()->DuplicateAsset(sDuplicateAssetStub, sPopupInputBuffer);
                sDuplicateAssetStub = nullptr;
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

    if (ImGui::BeginPopup("New CSS Stylesheet"))
    {
        if (setTextInputFocus)
        {
            ImGui::SetKeyboardFocusHere();
        }

        if (ImGui::InputText("Name", sPopupInputBuffer, kPopupInputBufferSize, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            std::string cssName = sPopupInputBuffer;
            if (cssName.empty())
                cssName = "style";

            std::string cssPath = curDir->mPath + cssName + ".css";
            FILE* cssFile = fopen(cssPath.c_str(), "w");
            if (cssFile != nullptr)
            {
                fputs("/* Stylesheet */\n\n", cssFile);
                fclose(cssFile);
                LogDebug("Created CSS file: %s", cssPath.c_str());
            }
            else
            {
                LogError("Failed to create CSS file: %s", cssPath.c_str());
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
            bool userProvidedName = (assetName != "");

            if (assetName == "")
            {
                if (sNewAssetIsSkybox)
                {
                    assetName = "M_Skybox";
                }
                else if (sNewAssetType == MaterialLite::GetStaticType())
                {
                    Asset* selAsset = GetEditorState()->GetSelectedAsset();
                    if (selAsset != nullptr && selAsset->GetType() == Texture::GetStaticType())
                    {
                        std::string texName = selAsset->GetName();
                        if (texName.length() >= 2 && texName[0] == 'T' && texName[1] == '_')
                            texName[0] = 'M';
                        else
                            texName = std::string("M_") + texName;
                        assetName = texName;
                    }
                    else
                    {
                        assetName = "M_Material";
                    }
                }
                else if (sNewAssetType == MaterialBase::GetStaticType())
                    assetName = "MB_Material";
                else if (sNewAssetType == MaterialInstance::GetStaticType())
                    assetName = "MI_Material";
                else if (sNewAssetType == ParticleSystem::GetStaticType())
                    assetName = "P_Particle";
                else if (sNewAssetType == Timeline::GetStaticType())
                    assetName = "TL_Timeline";
                else if (sNewAssetType == NodeGraphAsset::GetStaticType())
                    assetName = "NG_NodeGraph";
                else if (sNewAssetType == UIDocument::GetStaticType())
                    assetName = "UI_Document";
            }

            if (assetName != "" && sNewAssetType != INVALID_TYPE_ID)
            {
                CreateNewAsset(sNewAssetType, assetName.c_str(), sNewAssetIsSkybox, userProvidedName);
            }

            sNewAssetIsSkybox = false;
            ImGui::CloseCurrentPopup();
            closeContextPopup = true;
        }

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("New Scene"))
    {
        if (setTextInputFocus)
        {
            ImGui::SetKeyboardFocusHere();
        }

        ImGui::InputText("Name", sPopupInputBuffer, kPopupInputBufferSize);

        ImGui::RadioButton("2D", &sNewSceneType, 0);
        ImGui::SameLine();
        ImGui::RadioButton("3D", &sNewSceneType, 1);

        // Plugin-registered scene types
        {
            EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
            if (hookMgr != nullptr)
            {
                const auto& sceneTypes = hookMgr->GetSceneTypes();
                for (int i = 0; i < (int)sceneTypes.size(); ++i)
                {
                    ImGui::SameLine();
                    ImGui::RadioButton(sceneTypes[i].mTypeName.c_str(), &sNewSceneType, 2 + i);
                }
            }
        }

        ImGui::Checkbox("Create Camera", &sNewSceneCreateCamera);

        if (ImGui::Button("Create"))
        {
            std::string sceneName = sPopupInputBuffer;
            if (sceneName.empty())
                sceneName = "SC_Scene";

            CreateNewScene(sceneName.c_str(), sNewSceneType, sNewSceneCreateCamera);

            ImGui::CloseCurrentPopup();
            closeContextPopup = true;
        }

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Make Zoo"))
    {
        if (setTextInputFocus)
        {
            ImGui::SetKeyboardFocusHere();
        }

        ImGui::InputText("Scene Name", sPopupInputBuffer, kPopupInputBufferSize);
        ImGui::DragInt("Columns", &sZooColumns, 1, 1, 100);
        ImGui::DragFloat("Spacing", &sZooSpacing, 0.1f, 0.5f, 100.0f);

        if (ImGui::Button("Create"))
        {
            const auto& multiStubs = GetEditorState()->GetSelectedAssetStubs();
            std::vector<StaticMesh*> meshes;
            for (AssetStub* s : multiStubs)
            {
                if (s && s->mType == StaticMesh::GetStaticType())
                {
                    if (!s->mAsset) AssetManager::Get()->LoadAsset(*s);
                    StaticMesh* m = s->mAsset ? s->mAsset->As<StaticMesh>() : nullptr;
                    if (m) meshes.push_back(m);
                }
            }

            if (!meshes.empty())
            {
                std::string sceneName = sPopupInputBuffer[0] ? sPopupInputBuffer : "SC_Zoo";
                AssetStub* sceneStub = EditorAddUniqueAsset(sceneName.c_str(), curDir, Scene::GetStaticType(), true);
                if (sceneStub)
                {
                    SharedPtr<Node3D> root = Node::Construct<Node3D>();
                    root->SetName("Root");

                    int cols = glm::max(sZooColumns, 1);
                    int rows = ((int)meshes.size() + cols - 1) / cols;

                    for (int i = 0; i < (int)meshes.size(); ++i)
                    {
                        int col = i % cols;
                        int row = i / cols;
                        StaticMesh3D* node = root->CreateChild<StaticMesh3D>();
                        node->SetName(meshes[i]->GetName());
                        node->SetStaticMesh(meshes[i]);
                        node->SetPosition(glm::vec3(col * sZooSpacing, 0.0f, row * sZooSpacing));
                    }

                    float gridWidth = (cols - 1) * sZooSpacing;
                    float gridDepth = (rows - 1) * sZooSpacing;
                    float padding = sZooSpacing;
                    Asset* planeAsset = AssetManager::Get()->GetAsset("SC_Plane");
                    StaticMesh* planeMesh = planeAsset ? planeAsset->As<StaticMesh>() : nullptr;
                    if (planeMesh)
                    {
                        StaticMesh3D* floor = root->CreateChild<StaticMesh3D>("Floor");
                        floor->SetStaticMesh(planeMesh);
                        floor->SetPosition(glm::vec3(gridWidth * 0.5f, -0.05f, gridDepth * 0.5f));
                        float scaleX = (gridWidth + padding * 2) * 0.5f;
                        float scaleZ = (gridDepth + padding * 2) * 0.5f;
                        floor->SetScale(glm::vec3(scaleX, 1.0f, scaleZ));
                    }

                    Scene* scene = (Scene*)sceneStub->mAsset;
                    scene->Capture(root.Get());
                    AssetManager::Get()->SaveAsset(*sceneStub);
                    GetEditorState()->OpenEditScene(scene);
                }
            }

            ImGui::CloseCurrentPopup();
            closeContextPopup = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
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

// Flat directory picker for dialogs (e.g., Save Scene As)
static void DrawDirPickerTree(AssetDir* dir, AssetDir** selectedDir)
{
    if (dir == nullptr || selectedDir == nullptr)
        return;

    // Skip engine/addon directories
    if (dir->mEngineDir || dir->mAddonDir)
        return;

    ImGuiTreeNodeFlags dirFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

    // Check if this is the selected directory
    if (*selectedDir == dir)
        dirFlags |= ImGuiTreeNodeFlags_Selected;

    // Check if directory has no subdirectories
    bool hasSubdirs = false;
    for (AssetDir* child : dir->mChildDirs)
    {
        if (!child->mEngineDir && !child->mAddonDir)
        {
            hasSubdirs = true;
            break;
        }
    }
    if (!hasSubdirs)
        dirFlags |= ImGuiTreeNodeFlags_Leaf;

    std::string dirLabel = std::string(ICON_MATERIAL_SYMBOLS_FOLDER_SHARP) + " " + dir->mName;
    ImGui::PushID(dir);
    bool nodeOpen = ImGui::TreeNodeEx(dirLabel.c_str(), dirFlags);

    // Click to select
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
    {
        *selectedDir = dir;
    }

    if (nodeOpen)
    {
        for (AssetDir* child : dir->mChildDirs)
        {
            DrawDirPickerTree(child, selectedDir);
        }
        ImGui::TreePop();
    }
    ImGui::PopID();
}

static void DrawDirPicker()
{
    AssetDir* currentDir = GetEditorState()->GetAssetDirectory();
    if (currentDir == nullptr)
        return;

    ImGui::PushStyleColor(ImGuiCol_Header, kBgInactive);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, kBgHover);
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, kBgInactive);

    if (currentDir->mParentDir != nullptr)
    {
        if (ImGui::Selectable(ICON_MATERIAL_SYMBOLS_FOLDER_OPEN_SHARP "...", false))
        {
            GetEditorState()->SetAssetDirectory(currentDir->mParentDir, true);
        }
    }

    for (uint32_t i = 0; i < currentDir->mChildDirs.size(); ++i)
    {
        AssetDir* childDir = currentDir->mChildDirs[i];
        std::string dirLabel = std::string(ICON_MATERIAL_SYMBOLS_FOLDER_SHARP) + "  " + childDir->mName;
        if (ImGui::Selectable(dirLabel.c_str(), false))
        {
            GetEditorState()->SetAssetDirectory(childDir, true);
        }
    }

    ImGui::PopStyleColor(3);
}

static bool DirMatchesFilter(AssetDir* dir, const std::string& filterLower)
{
    if (dir == nullptr)
        return false;

    // Check if directory name matches
    {
        std::string nameLower = dir->mName;
        std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
        if (nameLower.find(filterLower) != std::string::npos)
            return true;
    }

    // Check if any asset in this directory matches
    for (AssetStub* stub : dir->mAssetStubs)
    {
        std::string nameLower = stub->mName;
        std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
        if (nameLower.find(filterLower) != std::string::npos)
            return true;
    }

    // Check if any loose file in this directory matches
    for (const std::string& looseFile : dir->mLooseFiles)
    {
        std::string nameLower = looseFile;
        std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
        if (nameLower.find(filterLower) != std::string::npos)
            return true;
    }

    // Check if any descendant directory matches
    for (AssetDir* child : dir->mChildDirs)
    {
        if (DirMatchesFilter(child, filterLower))
            return true;
    }

    return false;
}

static std::string sSelectedLooseFile; // Full path of selected loose file

static void DrawAssetItems(AssetDir* dir, const std::string& filterLower)
{
    AssetStub* selStub = GetEditorState()->GetSelectedAssetStub();

    for (uint32_t i = 0; i < dir->mAssetStubs.size(); ++i)
    {
        AssetStub* stub = dir->mAssetStubs[i];

        // Filter check
        if (!filterLower.empty())
        {
            std::string nameLower = stub->mName;
            std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
            if (nameLower.find(filterLower) == std::string::npos)
                continue;
        }

        bool isSelectedStub = GetEditorState()->IsAssetStubSelected(stub);
        if (isSelectedStub)
        {
            ImGui::PushStyleColor(ImGuiCol_Header, kSelectedColor);
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, kSelectedColor);
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, kSelectedColor);
        }

        glm::vec4 assetColor = AssetManager::Get()->GetEditorAssetColor(stub->mType);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(assetColor.r, assetColor.g, assetColor.b, assetColor.a));

        std::string assetDispText = std::string(GetAssetIcon(stub->mType)) + "  " + stub->mName;
        if (stub->mAsset && stub->mAsset->GetDirtyFlag())
        {
            assetDispText = "*" + assetDispText;
        }

        ImGui::PushID(stub);
        bool assetClicked = ImGui::Selectable(assetDispText.c_str(), isSelectedStub, ImGuiSelectableFlags_AllowDoubleClick);
        AlternatingRowBackground();
        if (assetClicked)
        {
            if (IsControlDown())
            {
                // Ctrl+Click: toggle in multi-selection
                if (GetEditorState()->IsAssetStubSelected(stub))
                {
                    GetEditorState()->RemoveSelectedAssetStub(stub);

                    // Update primary to another selected stub, or nullptr
                    const auto& stubs = GetEditorState()->GetSelectedAssetStubs();
                    GetEditorState()->mSelectedAssetStub = stubs.empty() ? nullptr : stubs.back();
                }
                else
                {
                    GetEditorState()->AddSelectedAssetStub(stub);
                    GetEditorState()->mSelectedAssetStub = stub;
                }
                sSelectedLooseFile.clear();

                if (stub != nullptr &&
                    stub->mAsset == nullptr)
                    AssetManager::Get()->LoadAsset(*stub);
                if (stub != nullptr &&
                    stub->mAsset != nullptr)
                    GetEditorState()->InspectObject(stub->mAsset);
            }
            else if (IsShiftDown() && selStub != nullptr)
            {
                // Shift+Click: range selection from primary to clicked item
                GetEditorState()->ClearSelectedAssetStubs();

                bool inRange = false;
                for (uint32_t j = 0; j < dir->mAssetStubs.size(); ++j)
                {
                    AssetStub* s = dir->mAssetStubs[j];

                    // Apply same filter so indices match what's visible
                    if (!filterLower.empty())
                    {
                        std::string nameLower = s->mName;
                        std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
                        if (nameLower.find(filterLower) == std::string::npos)
                            continue;
                    }

                    if (s == selStub || s == stub)
                    {
                        inRange = !inRange;
                        GetEditorState()->AddSelectedAssetStub(s);
                        if (!inRange)
                            break; // We just hit the second endpoint
                    }
                    else if (inRange)
                    {
                        GetEditorState()->AddSelectedAssetStub(s);
                    }
                }

                GetEditorState()->mSelectedAssetStub = stub;
                sSelectedLooseFile.clear();
            }
            else if (selStub != stub)
            {
                GetEditorState()->SetSelectedAssetStub(stub);
                sSelectedLooseFile.clear();

                if (stub != nullptr &&
                    stub->mType == NodeGraphAsset::GetStaticType())
                {
                    if (stub->mAsset == nullptr)
                        AssetManager::Get()->LoadAsset(*stub);
                    if (stub->mAsset != nullptr)
                        GetEditorState()->InspectObject(stub->mAsset);
                }
            }
            else
            {
                GetEditorState()->SetSelectedAssetStub(nullptr);
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
                else if (stub->mType == NodeGraphAsset::GetStaticType())
                {
                    NodeGraphAsset* nodeGraph = stub->mAsset ? stub->mAsset->As<NodeGraphAsset>() : nullptr;
                    if (nodeGraph)
                    {
                        OpenNodeGraphForEditing(nodeGraph);
                    }
                }
                else if (stub->mType == UIDocument::GetStaticType())
                {
                    if (stub->mAsset)
                    {
                        UIDocument* doc = stub->mAsset->As<UIDocument>();
                        const std::string& srcPath = doc->GetSourceFilePath();

                        if (!srcPath.empty() && SYS_DoesFileExist(srcPath.c_str(), false))
                        {
                            PreferencesModule* mod = PreferencesManager::Get()->FindModule("External/Editors");
                            EditorsModule* editors = mod ? static_cast<EditorsModule*>(mod) : nullptr;
                            if (editors)
                            {
                                editors->OpenLuaScript(srcPath);
                            }
                        }
                        else
                        {
                            GetEditorState()->InspectObject(doc);
                        }
                    }
                }
                else if (stub->mAsset && stub->mAsset->Is(Material::ClassRuntimeId()))
                {
                    Material* mat = stub->mAsset->As<Material>();
                    if (mat->HasNodeGraph())
                    {
                        OpenNodeGraphForEditing(mat->GetNodeGraph(), mat);
                    }
                    else
                    {
                        GetEditorState()->InspectObject(mat);
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

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
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

        // Asset browser item GUI overlay
        {
            ImVec2 rowMin = ImGui::GetItemRectMin();
            ImVec2 rowMax = ImGui::GetItemRectMax();
            EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
            if (hookMgr != nullptr)
            {
                const char* typeName = (stub->mAsset != nullptr) ?
                    stub->mAsset->GetTypeName() : "";
                hookMgr->DrawAssetItemGUI(stub->mName.c_str(), typeName,
                    rowMin.x, rowMin.y, rowMax.x - rowMin.x, rowMax.y - rowMin.y);
            }
        }

        if (isSelectedStub)
        {
            ImGui::PopStyleColor(3);
        }

        if (ImGui::BeginPopupContextItem())
        {
            // Set current directory so context popup operations target this asset's directory
            if (stub->mDirectory)
                GetEditorState()->mTabCurrentDir[GetEditorState()->ActiveTab()] = stub->mDirectory;
            DrawAssetsContextPopup(stub, nullptr);
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }

    // Draw loose files (non-.oct files)
    static const ImVec4 kLooseFileColor = ImVec4(0.65f, 0.65f, 0.70f, 1.0f);

    auto IsTextFileExtension = [](const char* ext) -> bool
    {
        if (ext == nullptr) return false;
        static const char* textExts[] = {
            ".lua", ".txt", ".json", ".xml", ".md", ".css", ".yaml", ".yml",
            ".ini", ".cfg", ".sh", ".bat", ".html", ".js", ".py", ".c", ".cpp",
            ".h", ".hpp", ".cs", ".glsl", ".hlsl", ".vert", ".frag", ".toml", ".log"
        };
        for (const char* textExt : textExts)
        {
            if (strcasecmp(ext, textExt) == 0) return true;
        }
        return false;
    };

    auto GetLooseFileIcon = [](const char* ext) -> const char*
    {
        if (ext == nullptr) return ICON_STREAMLINE_SHARP_NEW_FILE_REMIX;
        if (strcasecmp(ext, ".lua") == 0) return ICON_LUA;
        if (strcasecmp(ext, ".json") == 0) return ICON_JSON;
        if (strcasecmp(ext, ".png") == 0 || strcasecmp(ext, ".jpg") == 0 || strcasecmp(ext, ".jpeg") == 0) return ICON_TDESIGN_IMAGE_FILLED;
        if (strcasecmp(ext, ".wav") == 0 || strcasecmp(ext, ".mp3") == 0 || strcasecmp(ext, ".ogg") == 0) return ICON_RIVET_ICONS_AUDIO_SOLID;
        if (strcasecmp(ext, ".css") == 0) return ICON_IX_CODE;
        return ICON_STREAMLINE_SHARP_NEW_FILE_REMIX;
    };

    for (uint32_t i = 0; i < dir->mLooseFiles.size(); ++i)
    {
        const std::string& filename = dir->mLooseFiles[i];

        // Filter check
        if (!filterLower.empty())
        {
            std::string nameLower = filename;
            std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
            if (nameLower.find(filterLower) == std::string::npos)
                continue;
        }

        std::string fullPath = dir->mPath + filename;
        bool isSelected = (sSelectedLooseFile == fullPath);

        if (isSelected)
        {
            ImGui::PushStyleColor(ImGuiCol_Header, kSelectedColor);
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, kSelectedColor);
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, kSelectedColor);
        }

        ImGui::PushStyleColor(ImGuiCol_Text, kLooseFileColor);

        const char* ext = strrchr(filename.c_str(), '.');
        std::string displayText = std::string(GetLooseFileIcon(ext)) + "  " + filename;

        ImGui::PushID(fullPath.c_str());
        bool looseClicked = ImGui::Selectable(displayText.c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick);
        AlternatingRowBackground();
        if (looseClicked)
        {
            if (sSelectedLooseFile != fullPath)
            {
                sSelectedLooseFile = fullPath;
                GetEditorState()->SetSelectedAssetStub(nullptr);
            }
            else if (!IsControlDown())
            {
                sSelectedLooseFile.clear();
            }

            if (ImGui::IsMouseDoubleClicked(0))
            {
                if (IsTextFileExtension(ext))
                {
                    PreferencesModule* mod = PreferencesManager::Get()->FindModule("External/Editors");
                    EditorsModule* editors = mod ? static_cast<EditorsModule*>(mod) : nullptr;
                    if (editors)
                    {
                        editors->OpenLuaScript(fullPath);
                    }
                }
                else
                {
                    SYS_OpenFileWithDefaultApp(fullPath);
                }
            }
        }

        ImGui::PopStyleColor(); // kLooseFileColor

        if (isSelected)
        {
            ImGui::PopStyleColor(3);
        }

        if (ImGui::BeginPopupContextItem())
        {
            bool isTextFile = IsTextFileExtension(ext);

            if (isTextFile && ImGui::MenuItem("Edit"))
            {
                PreferencesModule* mod = PreferencesManager::Get()->FindModule("External/Editors");
                EditorsModule* editors = mod ? static_cast<EditorsModule*>(mod) : nullptr;
                if (editors)
                {
                    editors->OpenLuaScript(fullPath);
                }
            }

            if (ImGui::MenuItem("Open with Default App"))
            {
                SYS_OpenFileWithDefaultApp(fullPath);
            }

            if (ImGui::MenuItem("Show in Explorer"))
            {
                std::string dirPath = dir->mPath;
                SYS_ExplorerOpenDirectory(dirPath);
            }

            // Convert submenu for XML files
            if (ext && strcasecmp(ext, ".xml") == 0)
            {
                if (ImGui::BeginMenu("Convert"))
                {
                    if (ImGui::MenuItem("New UIDocument"))
                    {
                        sConvertXmlSourcePath = fullPath;
                        std::string baseName = Asset::GetNameFromPath(fullPath);
                        if (baseName.length() < 3 || baseName.substr(0, 3) != "UI_")
                            baseName = "UI_" + baseName;
                        strncpy(sPopupInputBuffer, baseName.c_str(), kPopupInputBufferSize - 1);
                        sPopupInputBuffer[kPopupInputBufferSize - 1] = '\0';
                    }

                    if (ImGui::MenuItem("Refresh UIDocument"))
                    {
                        sConvertXmlSourcePath = fullPath;
                        sShowUIDocumentPicker = true;
                    }

                    ImGui::EndMenu();
                }
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Delete"))
            {
                SYS_RemoveFile(fullPath.c_str());
                if (sSelectedLooseFile == fullPath)
                    sSelectedLooseFile.clear();
                dir->mLooseFiles.erase(dir->mLooseFiles.begin() + i);
                --i;
                ImGui::EndPopup();
                ImGui::PopID();
                continue;
            }

            ImGui::EndPopup();
        }
        ImGui::PopID();
    }
}

static void DrawAssetDirTree(AssetDir* dir, const std::string& filterLower, bool isRoot)
{
    if (dir == nullptr)
        return;

    // When filtering, skip directories that don't match
    if (!filterLower.empty() && !DirMatchesFilter(dir, filterLower))
        return;

    ImGuiTreeNodeFlags dirFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
    if (!filterLower.empty())
        dirFlags |= ImGuiTreeNodeFlags_DefaultOpen;
    if (isRoot)
        dirFlags |= ImGuiTreeNodeFlags_DefaultOpen;

    // Check if directory has no children (leaf directory with no subdirs)
    bool hasChildren = !dir->mChildDirs.empty() || !dir->mAssetStubs.empty() || !dir->mLooseFiles.empty();
    if (!hasChildren)
        dirFlags |= ImGuiTreeNodeFlags_Leaf;

    std::string dirLabel = std::string(ICON_MATERIAL_SYMBOLS_FOLDER_SHARP) + " " + dir->mName;
    ImGui::PushID(dir);
    bool nodeOpen = ImGui::TreeNodeEx(dirLabel.c_str(), dirFlags);
    AlternatingRowBackground();

    // Drag source for directory (skip engine/addon dirs)
    if (!dir->mEngineDir && !dir->mAddonDir && ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        AssetDir* dragDir = dir;
        ImGui::SetDragDropPayload(DRAGDROP_DIR, &dragDir, sizeof(AssetDir*));
        ImGui::Text("%s", dir->mName.c_str());
        ImGui::EndDragDropSource();
    }

    // Drop target for assets and directories
    if (!dir->mEngineDir && !dir->mAddonDir && ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(DRAGDROP_ASSET))
        {
            AssetStub* droppedStub = *(AssetStub**)payload->Data;
            MoveAssetToDirectory(droppedStub, dir);
        }
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(DRAGDROP_DIR))
        {
            AssetDir* droppedDir = *(AssetDir**)payload->Data;
            MoveDirectoryToDirectory(droppedDir, dir);
        }
        ImGui::EndDragDropTarget();
    }

    if (ImGui::BeginPopupContextItem())
    {
        // Set current directory so context popup operations target this directory
        GetEditorState()->mTabCurrentDir[GetEditorState()->ActiveTab()] = dir;
        DrawAssetsContextPopup(nullptr, dir);
        ImGui::EndPopup();
    }

    if (nodeOpen)
    {
        // Recurse into child directories
        for (AssetDir* child : dir->mChildDirs)
        {
            DrawAssetDirTree(child, filterLower, false);
        }

        // Draw assets in this directory
        DrawAssetItems(dir, filterLower);

        ImGui::TreePop();
    }
    ImGui::PopID();
}

static void DrawAssetBrowser(AssetDir* rootDir, const std::string& filterLower, bool showRootChildren)
{
    if (rootDir == nullptr)
        return;

    if (showRootChildren)
    {
        // Show root's children directly (don't wrap in a root tree node)
        for (AssetDir* child : rootDir->mChildDirs)
        {
            DrawAssetDirTree(child, filterLower, true);
        }
        // Draw assets directly in the root directory
        DrawAssetItems(rootDir, filterLower);
    }
    else
    {
        // Show the directory itself as a tree node
        DrawAssetDirTree(rootDir, filterLower, true);
    }

    // If no popup is open and we aren't inputting text...
    if (!ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup) &&
        ImGui::IsWindowHovered() &&
        !ImGui::GetIO().WantTextInput)
    {
        const bool ctrlDown = IsControlDown();

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
        {
            ImGui::OpenPopup("Null Context");
        }

        AssetDir* currentDir = GetEditorState()->GetAssetDirectory();
        if (currentDir != nullptr && !currentDir->mAddonDir)
        {
            if (ctrlDown && IsKeyJustDown(OCTAVE_KEY_N))
            {
                CreateNewAsset(Scene::GetStaticType(), "SC_Scene");
            }

            if (ctrlDown && IsKeyJustDown(OCTAVE_KEY_M))
            {
                CreateNewAsset(MaterialLite::GetStaticType(), "M_Material");
            }

            if (ctrlDown && IsKeyJustDown(OCTAVE_KEY_P))
            {
                CreateNewAsset(ParticleSystem::GetStaticType(), "P_Particle");
            }

            if (ctrlDown && IsKeyJustDown(OCTAVE_KEY_D))
            {
                AssetStub* srcStub = GetEditorState()->GetSelectedAssetStub();

                if (srcStub != nullptr)
                {
                    sDuplicateAssetStub = srcStub;
                    ImGui::OpenPopup("Duplicate Asset Name");
                    std::string defaultName = srcStub->mName + "_00";
                    strncpy(sPopupInputBuffer, defaultName.c_str(), kPopupInputBufferSize - 1);
                    sPopupInputBuffer[kPopupInputBufferSize - 1] = '\0';
                    sDuplicateAssetFocus = true;
                }
            }

            if (IsKeyJustDown(OCTAVE_KEY_DELETE))
            {
                const auto& selectedStubs = GetEditorState()->GetSelectedAssetStubs();
                if (!selectedStubs.empty())
                {
                    // Copy the vector since deletion will modify it
                    std::vector<AssetStub*> toDelete = selectedStubs;
                    for (AssetStub* s : toDelete)
                    {
                        ActionManager::Get()->DeleteAsset(s);
                    }
                }
            }

            if (!ctrlDown && IsKeyJustDown(OCTAVE_KEY_F2))
            {
                AssetStub* selStub = GetEditorState()->GetSelectedAssetStub();
                if (selStub != nullptr && !selStub->mEngineAsset)
                {
                    ImGui::OpenPopup("Rename Asset F2");
                    strncpy(sPopupInputBuffer, selStub->mName.c_str(), kPopupInputBufferSize - 1);
                    sPopupInputBuffer[kPopupInputBufferSize - 1] = '\0';
                    sF2RenameAssetFocus = true;
                }
            }
        }
    }

    if (ImGui::BeginPopup("Rename Asset F2"))
    {
        if (sF2RenameAssetFocus)
        {
            ImGui::SetKeyboardFocusHere();
            sF2RenameAssetFocus = false;
        }

        if (ImGui::InputText("Name", sPopupInputBuffer, kPopupInputBufferSize, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            AssetStub* selStub = GetEditorState()->GetSelectedAssetStub();
            if (selStub)
            {
                Asset* asset = AssetManager::Get()->LoadAsset(*selStub);
                AssetManager::Get()->RenameAsset(asset, sPopupInputBuffer);
                AssetManager::Get()->SaveAsset(*selStub);
            }
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Duplicate Asset Name"))
    {
        if (sDuplicateAssetFocus)
        {
            ImGui::SetKeyboardFocusHere();
            sDuplicateAssetFocus = false;
        }

        if (ImGui::InputText("New Name##DupAssetF2", sPopupInputBuffer, kPopupInputBufferSize, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            if (sDuplicateAssetStub != nullptr)
            {
                GetEditorState()->DuplicateAsset(sDuplicateAssetStub, sPopupInputBuffer);
                sDuplicateAssetStub = nullptr;
            }
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Null Context"))
    {
        DrawAssetsContextPopup(nullptr, nullptr);
        ImGui::EndPopup();
    }
}

static void DrawAssetsPanel()
{
    const float kAssetPanelIndent = 4.0f;

    // Shared search bar above tab bar
    static char sAssetsSearchBuffer[256] = "";
    ImGui::SetNextItemWidth(-1);
    ImGui::InputTextWithHint("##AssetsSearch", "Search assets...", sAssetsSearchBuffer, sizeof(sAssetsSearchBuffer));
    std::string filterLower;
    if (sAssetsSearchBuffer[0] != '\0')
    {
        filterLower = sAssetsSearchBuffer;
        std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(), ::tolower);
    }

    ImGui::Spacing();

    if (ImGui::BeginTabBar("AssetBrowserTabs"))
    {
        if (ImGui::BeginTabItem("Project"))
        {
            if (GetEditorState()->mActiveAssetTab != AssetBrowserTab::Project)
                GetEditorState()->mActiveAssetTab = AssetBrowserTab::Project;

            // Toolbar
            {
                bool hasProject = !GetEngineState()->mProjectDirectory.empty();
                if (!hasProject) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

                if (ImGui::Button(ICON_BXS_ARROW_TO_BOTTOM "##ImportAsset"))
                {
                    if (hasProject)
                        ActionManager::Get()->ImportAsset();
                }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                    ImGui::SetTooltip("Import Asset");

                ImGui::SameLine();
                if (ImGui::Button(ICON_STREAMLINE_SHARP_NEW_FILE_REMIX "##NewFolder"))
                {
                    if (hasProject)
                        ImGui::OpenPopup("NewFolderPopup");
                }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                    ImGui::SetTooltip("New Folder");

                ImGui::SameLine();
                if (ImGui::Button(ICON_MATERIAL_SYMBOLS_FOLDER_OPEN_SHARP "##OpenProjectExplorer"))
                {
                    if (hasProject)
                    {
                        AssetDir* projDir = AssetManager::Get()->FindProjectDirectory();
                        if (projDir)
                            SYS_ExplorerOpenDirectory(projDir->mPath);
                    }
                }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                    ImGui::SetTooltip("Open in Explorer");

                ImGui::SameLine();
                if (ImGui::Button(ICON_UNDO "##RefreshAssets"))
                {
                    if (hasProject)
                    {
                        AssetDir* projDir = AssetManager::Get()->FindProjectDirectory();
                        if (projDir)
                            AssetManager::Get()->RefreshDirectory(projDir);
                    }
                }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                    ImGui::SetTooltip("Refresh");

                if (!hasProject) ImGui::PopStyleVar();

                // New Folder popup
                static char sNewFolderBuffer[256] = "";
                static bool sNewFolderFocus = false;
                if (ImGui::IsPopupOpen("NewFolderPopup"))
                    sNewFolderFocus = true;
                if (ImGui::BeginPopup("NewFolderPopup"))
                {
                    if (sNewFolderFocus)
                    {
                        ImGui::SetKeyboardFocusHere();
                        sNewFolderFocus = false;
                    }
                    if (ImGui::InputText("Folder Name", sNewFolderBuffer, sizeof(sNewFolderBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
                    {
                        std::string folderName = sNewFolderBuffer;
                        if (!folderName.empty())
                        {
                            AssetDir* curDir = GetEditorState()->GetAssetDirectory();
                            if (curDir == nullptr)
                                curDir = AssetManager::Get()->FindProjectDirectory();
                            if (curDir)
                            {
                                if (SYS_CreateDirectory((curDir->mPath + folderName).c_str()))
                                    curDir->CreateSubdirectory(folderName);
                                else
                                    LogError("Failed to create folder");
                            }
                        }
                        sNewFolderBuffer[0] = '\0';
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }

                ImGui::Spacing();
            }

            BeginAlternatingRows();
            ImGui::Indent(kAssetPanelIndent);
            AssetDir* projectDir = AssetManager::Get()->FindProjectDirectory();
            // Initialize current dir to project root if not set
            if (GetEditorState()->mTabCurrentDir[(int)AssetBrowserTab::Project] == nullptr && projectDir != nullptr)
                GetEditorState()->mTabCurrentDir[(int)AssetBrowserTab::Project] = projectDir;
            DrawAssetBrowser(projectDir, filterLower, true);
            ImGui::Unindent(kAssetPanelIndent);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Addons"))
        {
            if (GetEditorState()->mActiveAssetTab != AssetBrowserTab::Addons)
            {
                GetEditorState()->mActiveAssetTab = AssetBrowserTab::Addons;
                if (GetEditorState()->mTabCurrentDir[(int)AssetBrowserTab::Addons] == nullptr)
                    GetEditorState()->mTabCurrentDir[(int)AssetBrowserTab::Addons] = AssetManager::Get()->FindPackagesDirectory();
            }

            // Toolbar
            {
                if (ImGui::Button(ICON_MATERIAL_SYMBOLS_FOLDER_OPEN_SHARP "##OpenAddonsExplorer"))
                {
                    AssetDir* pkgDir = AssetManager::Get()->FindPackagesDirectory();
                    if (pkgDir)
                        SYS_ExplorerOpenDirectory(pkgDir->mPath);
                }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                    ImGui::SetTooltip("Open in Explorer");

                ImGui::SameLine();
                if (ImGui::Button(ICON_UNDO "##RefreshAddons"))
                {
                    AssetDir* pkgDir = AssetManager::Get()->FindPackagesDirectory();
                    if (pkgDir)
                        AssetManager::Get()->RefreshDirectory(pkgDir);
                }
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Refresh");

                ImGui::Spacing();
            }

            BeginAlternatingRows();
            ImGui::Indent(kAssetPanelIndent);
            AssetDir* packagesDir = AssetManager::Get()->FindPackagesDirectory();
            DrawAssetBrowser(packagesDir, filterLower, true);
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
            BeginAlternatingRows();
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

                    if (asset->As<UIDocument>())
                    {
                        ImGui::SameLine();
                        if (ImGui::Button("Edit"))
                        {
                            UIDocument* doc = asset->As<UIDocument>();
                            const std::string& srcPath = doc->GetSourceFilePath();
                            if (!srcPath.empty() && SYS_DoesFileExist(srcPath.c_str(), false))
                            {
                                PreferencesModule* mod = PreferencesManager::Get()->FindModule("External/Editors");
                                EditorsModule* editors = mod ? static_cast<EditorsModule*>(mod) : nullptr;
                                if (editors)
                                    editors->OpenLuaScript(srcPath);
                            }
                        }
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
            BeginAlternatingRows();
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
            BeginAlternatingRows();
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
        if (IsKeyJustDown(OCTAVE_KEY_L))
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
                    if (hasProject && editorConfigured) {
                        editors->OpenLuaScript(GetEngineState()->mProjectDirectory);
                    }
                }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                {
                    if (editorConfigured)
                    {
                        std::string editorName = editors->mLuaEditorPath;
                        size_t lastSlash = editorName.find_last_of("/\\");
                        if (lastSlash != std::string::npos)
                            editorName = editorName.substr(lastSlash + 1);
                        ImGui::SetTooltip("Open in %s", editorName.c_str());
                    }
                    else
                    {
                        ImGui::SetTooltip("Open in External Editor (not configured)");
                    }
                }
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

                ImGui::SameLine();
                if (ImGui::Button(ICON_UNDO "##RefreshLuaScripts"))
                {
                    sLuaLastUpdate = 0.0;
                }
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Refresh");

                if (!hasProject) ImGui::PopStyleVar();

                ImGui::Spacing();
            }

            // Cache the engine root directory on first use so relative paths survive cwd changes
            static std::string sEngineRoot;
            if (sEngineRoot.empty())
            {
                sEngineRoot = AssetManager::Get()->GetOctaveDirectory();
            }

            // Refresh every 2 seconds
            static bool sLuaInitialized = false;
            if (!sLuaInitialized || currentTime - sLuaLastUpdate > 2.0)
            {
                std::vector<ScriptFileEntry> newScripts;
                const std::string& projectDir = GetEngineState()->mProjectDirectory;

                // Engine scripts (use cached absolute path)
                {
                    std::string engineScriptsDir = sEngineRoot + "Engine/Scripts/";
                    std::vector<std::string> files;
                    AssetManager::Get()->GatherScriptFiles(engineScriptsDir, files);
                    for (const std::string& f : files)
                    {
                        ScriptFileEntry entry;
                        entry.mFullPath = f;
                        entry.mOrigin = "Engine";
                        entry.mDisplayName = (f.length() > engineScriptsDir.length() && f.substr(0, engineScriptsDir.length()) == engineScriptsDir)
                            ? f.substr(engineScriptsDir.length()) : f;
                        newScripts.push_back(entry);
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
                        newScripts.push_back(entry);
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
                                newScripts.push_back(entry);
                            }
                        }
                        SYS_IterateDirectory(pkgDirEntry);
                    }
                    SYS_CloseDirectory(pkgDirEntry);
                }

                sLuaScripts = std::move(newScripts);
                sLuaLastUpdate = currentTime;
                sLuaInitialized = true;
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

                // Group by origin (Engine, Project, package name) at the top level
                TreeNode* current = &root.children[entry.mOrigin];
                current->name = entry.mOrigin;

                // Split display path into subdirectory components
                std::string path = entry.mDisplayName;
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

                    std::string dirLabel = std::string(ICON_MATERIAL_SYMBOLS_FOLDER_SHARP) + " " + pair.first;
                    bool dirOpen = ImGui::TreeNodeEx(dirLabel.c_str(), dirFlags);
                    AlternatingRowBackground();

                    // Context menu for script directory
                    if (ImGui::BeginPopupContextItem())
                    {
                        if (ImGui::Selectable("Reveal in Explorer"))
                        {
                            // Find the first file in this subtree to derive the directory path
                            std::function<const ScriptFileEntry*(const TreeNode&)> findFirst;
                            findFirst = [&](const TreeNode& n) -> const ScriptFileEntry* {
                                if (!n.files.empty()) return n.files[0];
                                for (auto& child : n.children) {
                                    const ScriptFileEntry* f = findFirst(child.second);
                                    if (f) return f;
                                }
                                return nullptr;
                            };
                            const ScriptFileEntry* firstFile = findFirst(pair.second);
                            if (firstFile)
                            {
                                std::string dirPath = firstFile->mFullPath;
                                size_t lastSlash = dirPath.find_last_of("/\\");
                                if (lastSlash != std::string::npos)
                                    dirPath = dirPath.substr(0, lastSlash);
                                std::string absPath = SYS_GetAbsolutePath(dirPath);
#if PLATFORM_WINDOWS
                                for (char& c : absPath) { if (c == '/') c = '\\'; }
                                SYS_Exec(("explorer \"" + absPath + "\"").c_str());
#elif PLATFORM_LINUX
                                SYS_Exec(("xdg-open \"" + absPath + "\" &").c_str());
#endif
                            }
                        }
                        if (ImGui::Selectable("Copy Path"))
                        {
                            // Copy the directory path (relative to Scripts folder)
                            // Find the first file and extract the directory portion of its display name
                            std::function<const ScriptFileEntry*(const TreeNode&)> findFirst;
                            findFirst = [&](const TreeNode& n) -> const ScriptFileEntry* {
                                if (!n.files.empty()) return n.files[0];
                                for (auto& child : n.children) {
                                    const ScriptFileEntry* f = findFirst(child.second);
                                    if (f) return f;
                                }
                                return nullptr;
                            };
                            const ScriptFileEntry* firstFile = findFirst(pair.second);
                            if (firstFile)
                            {
                                std::string dirPath = firstFile->mDisplayName;
                                size_t lastSlash = dirPath.find_last_of('/');
                                if (lastSlash != std::string::npos)
                                    dirPath = dirPath.substr(0, lastSlash);
                                ImGui::SetClipboardText(dirPath.c_str());
                            }
                        }
                        ImGui::EndPopup();
                    }

                    if (dirOpen)
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
                    std::string labelWithIcon = std::string(ICON_LUA) + " " + fileName;
                    ImGui::TreeNodeEx(labelWithIcon.c_str(), leafFlags);
                    AlternatingRowBackground();

                    // Drag source for script files
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
                    {
                        ImGui::SetDragDropPayload(DRAGDROP_SCRIPT, entry->mDisplayName.c_str(), entry->mDisplayName.size() + 1);
                        ImGui::Text("%s", fileName.c_str());
                        ImGui::EndDragDropSource();
                    }

                    // Context menu for script file
                    if (ImGui::BeginPopupContextItem())
                    {
                        if (ImGui::Selectable("Reveal in Explorer"))
                        {
                            std::string absPath = SYS_GetAbsolutePath(entry->mFullPath);
#if PLATFORM_WINDOWS
                            for (char& c : absPath) { if (c == '/') c = '\\'; }
                            SYS_Exec(("explorer /select,\"" + absPath + "\"").c_str());
#elif PLATFORM_LINUX
                            std::string dirPath = absPath.substr(0, absPath.find_last_of('/'));
                            SYS_Exec(("xdg-open \"" + dirPath + "\" &").c_str());
#endif
                        }
                        if (ImGui::Selectable("Copy Path"))
                        {
                            // Copy the script reference path (relative to Scripts folder)
                            ImGui::SetClipboardText(entry->mDisplayName.c_str());
                        }
                        ImGui::EndPopup();
                    }

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
            BeginAlternatingRows();
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
                        // Open first addon's vcxproj in IDE
                        std::vector<std::string> addonIds = nam->GetDiscoveredAddonIds();
                        for (const std::string& id : addonIds)
                        {
                            const NativeAddonState* state = nam->GetState(id);
                            if (state && state->mNativeMetadata.mHasNative)
                            {
                                std::string binaryName = state->mNativeMetadata.mBinaryName.empty() ? id : state->mNativeMetadata.mBinaryName;
                                std::string vcxproj = state->mSourcePath + binaryName + ".vcxproj";
                                editors->OpenCppFile(vcxproj, vcxproj);
                                break;
                            }
                        }
                    }
                }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                {
                    if (cppConfigured)
                    {
                        std::string editorName = editors->mCppEditorPath;
                        size_t lastSlash = editorName.find_last_of("/\\");
                        if (lastSlash != std::string::npos)
                            editorName = editorName.substr(lastSlash + 1);
                        ImGui::SetTooltip("Open in %s", editorName.c_str());
                    }
                    else
                    {
                        ImGui::SetTooltip("Open in External Editor (not configured)");
                    }
                }
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

                ImGui::SameLine();
                if (ImGui::Button(ICON_UNDO "##RefreshCppAddons"))
                {
                    sCppLastUpdate = 0.0;
                }
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Refresh");

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
            BeginAlternatingRows();

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

                    bool addonOpen = ImGui::TreeNodeEx(addon.mAddonId.c_str(), addonFlags, "%s %s", ICON_MATERIAL_SYMBOLS_FOLDER_SHARP, addon.mAddonName.c_str());
                    AlternatingRowBackground();
                    if (addonOpen)
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
                            const char* fileIcon = ICON_MINGCUTE_PAPER_FILL ;
                            {
                                size_t dot = fileName.rfind('.');
                                if (dot != std::string::npos)
                                {
                                    std::string ext = fileName.substr(dot);
                                    if (ext == ".cpp" || ext == ".c" || ext == ".h" || ext == ".hpp")
                                        fileIcon = ICON_CPP;
                                    else if (ext == ".lua")
                                        fileIcon = ICON_LUA;
                                    else if (ext == ".json")
                                        fileIcon = ICON_JSON;
                                    else if (ext == ".cs")
                                        fileIcon = ICON_CS;
                                }
                            }
                            ImGui::TreeNodeEx(srcFile.c_str(), leafFlags, "%s %s", fileIcon, fileName.c_str());
                            AlternatingRowBackground();

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

    float menuPadding = GetMenuBarPadding();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, menuPadding));

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
            if (ImGui::BeginMenu("Recent Scenes"))
            {
                const std::vector<RecentScene>& recentScenes = GetEditorState()->mRecentScenes;
                AssetManager* assetMgr = AssetManager::Get();
                bool hasValid = false;

                for (const RecentScene& r : recentScenes)
                {
                    if (assetMgr && assetMgr->DoesAssetExist(r.mSceneName))
                    {
                        hasValid = true;
                        if (ImGui::MenuItem(r.mSceneName.c_str()))
                        {
                            AssetStub* stub = assetMgr->GetAssetStub(r.mSceneName);
                            if (stub)
                            {
                                if (!stub->mAsset)
                                    assetMgr->LoadAsset(*stub);
                                Scene* scene = stub->mAsset ? stub->mAsset->As<Scene>() : nullptr;
                                if (scene)
                                    GetEditorState()->OpenEditScene(scene);
                            }
                        }
                    }
                }

                if (!hasValid)
                    ImGui::TextDisabled("(No recent scenes)");

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

        // Draw addon menus positioned after File (position=0)
        {
            EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
            if (hookMgr != nullptr) hookMgr->DrawTopLevelMenusAtPosition(0);
        }

        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo"))
                am->Undo();
            if (ImGui::MenuItem("Redo"))
                am->Redo();

            ImGui::Separator();

            if (ImGui::MenuItem("Preferences..."))
            {
                GetPreferencesWindow()->Open();
            }

            // Draw plugin menu items for Edit menu
            {
                EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
                if (hookMgr != nullptr) hookMgr->DrawMenuItems("Edit");
            }

            ImGui::EndMenu();
        }

        // Draw addon menus positioned after Edit (position=1)
        {
            EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
            if (hookMgr != nullptr) hookMgr->DrawTopLevelMenusAtPosition(1);
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

            // Preferences moved to Edit menu

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

            if (ImGui::MenuItem("3DS Preview"))
                GetEditorState()->mShow3DSPreview = !GetEditorState()->mShow3DSPreview;

            if (ImGui::MenuItem("Game Preview"))
                GetEditorState()->mShowGamePreview = !GetEditorState()->mShowGamePreview;

            if (ImGui::MenuItem("Node Graph"))
                GetEditorState()->mShowNodeGraphPanel = !GetEditorState()->mShowNodeGraphPanel;

            if (ImGui::MenuItem("Profiling"))
                GetEditorState()->mShowProfilingPanel = !GetEditorState()->mShowProfilingPanel;

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

        // Draw addon menus positioned after View (position=2)
        {
            EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
            if (hookMgr != nullptr) hookMgr->DrawTopLevelMenusAtPosition(2);
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

        // Draw addon menus positioned after World (position=3)
        {
            EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
            if (hookMgr != nullptr) hookMgr->DrawTopLevelMenusAtPosition(3);
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

            // Directory paths
            const std::string& projectDir = GetEngineState()->mProjectDirectory;
            bool hasProject = !projectDir.empty();
            std::string assetsDir = projectDir + "Assets/";
            std::string scriptsDir = projectDir + "Scripts/";
            std::string addonsDir = projectDir + "Packages/";
            std::string octaveDir = SYS_GetOctavePath();

            if (ImGui::BeginMenu("Reveal in Explorer"))
            {
                auto revealDir = [](const std::string& dir) {
                    std::string absPath = SYS_GetAbsolutePath(dir);
#if PLATFORM_WINDOWS
                    for (char& c : absPath) { if (c == '/') c = '\\'; }
                    SYS_Exec(("explorer \"" + absPath + "\"").c_str());
#elif PLATFORM_LINUX
                    SYS_Exec(("xdg-open \"" + absPath + "\" &").c_str());
#endif
                };

                if (ImGui::MenuItem("Project Directory", nullptr, false, hasProject))
                    revealDir(projectDir);
                if (ImGui::MenuItem("Project Assets Directory", nullptr, false, hasProject))
                    revealDir(assetsDir);
                if (ImGui::MenuItem("Project Scripts Directory", nullptr, false, hasProject))
                    revealDir(scriptsDir);
                if (ImGui::MenuItem("Project Addons Directory", nullptr, false, hasProject))
                    revealDir(addonsDir);
                if (ImGui::MenuItem("Octave Engine Directory"))
                    revealDir(octaveDir);

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Open in Code Editor"))
            {
                PreferencesModule* mod = PreferencesManager::Get()->FindModule("External/Editors");
                EditorsModule* editors = mod ? static_cast<EditorsModule*>(mod) : nullptr;
                bool hasEditor = editors && editors->IsLuaEditorConfigured();

                auto openInEditor = [&](const std::string& dir) {
                    if (editors)
                    {
                        std::string absPath = SYS_GetAbsolutePath(dir);
                        std::string cmd = editors->BuildLuaOpenCommand(absPath);
                        SYS_Exec(cmd.c_str());
                    }
                };

                if (ImGui::MenuItem("Project Directory", nullptr, false, hasProject && hasEditor))
                    openInEditor(projectDir);
                if (ImGui::MenuItem("Project Assets Directory", nullptr, false, hasProject && hasEditor))
                    openInEditor(assetsDir);
                if (ImGui::MenuItem("Project Scripts Directory", nullptr, false, hasProject && hasEditor))
                    openInEditor(scriptsDir);
                if (ImGui::MenuItem("Project Addons Directory", nullptr, false, hasProject && hasEditor))
                    openInEditor(addonsDir);
                if (ImGui::MenuItem("Octave Engine Directory", nullptr, false, hasEditor))
                    openInEditor(octaveDir);

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Open in VS Code"))
            {
                auto openInVSCode = [](const std::string& dir) {
                    std::string absPath = SYS_GetAbsolutePath(dir);
#if PLATFORM_WINDOWS
                    SYS_Exec(("code \"" + absPath + "\"").c_str());
#elif PLATFORM_LINUX
                    SYS_Exec(("code \"" + absPath + "\" &").c_str());
#endif
                };

                if (ImGui::MenuItem("Project Directory", nullptr, false, hasProject))
                    openInVSCode(projectDir);
                if (ImGui::MenuItem("Project Assets Directory", nullptr, false, hasProject))
                    openInVSCode(assetsDir);
                if (ImGui::MenuItem("Project Scripts Directory", nullptr, false, hasProject))
                    openInVSCode(scriptsDir);
                if (ImGui::MenuItem("Project Addons Directory", nullptr, false, hasProject))
                    openInVSCode(addonsDir);
                if (ImGui::MenuItem("Octave Engine Directory"))
                    openInVSCode(octaveDir);

                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Check Build Dependencies"))
            {
                GetBuildDependencyWindow()->Open();
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

        // Draw addon menus positioned after Developer (position=4)
        {
            EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
            if (hookMgr != nullptr) hookMgr->DrawTopLevelMenusAtPosition(4);
        }

        if (ImGui::BeginMenu("Addons"))
        {
            DrawAddonsPopupContent();

            // Draw addon-registered Addons menu items (Batch 8)
            {
                EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
                if (hookMgr != nullptr) hookMgr->DrawAddonsMenuItems();
            }

            ImGui::EndMenu();
        }

        // Draw addon menus positioned after Addons (position=5)
        {
            EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
            if (hookMgr != nullptr) hookMgr->DrawTopLevelMenusAtPosition(5);
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

        // Draw addon menus positioned after Extra (position=6)
        {
            EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
            if (hookMgr != nullptr) hookMgr->DrawTopLevelMenusAtPosition(6);
        }

        // Draw addon top-level menus as main menu bar entries (legacy append, position=-1)
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

        {
            EditorMode prevMode = GetEditorState()->mMode;
            GetEditorState()->SetEditorMode((EditorMode)curMode);
            GetEditorState()->SetPaintMode(paintMode);

            // Fire editor mode changed hook (Batch 10)
            if ((int)prevMode != curMode)
            {
                EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
                if (hookMgr != nullptr) hookMgr->FireOnEditorModeChanged(curMode);
            }
        }

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

        // Draw addon gizmo tools (Batch 10)
        {
            EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
            if (hookMgr != nullptr)
            {
                Node* selNode = GetEditorState()->GetSelectedNode();
                hookMgr->DrawGizmoTools(selNode);
            }
        }

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

                // Scene tab context menu (Batch 8)
                if (ImGui::BeginPopupContextItem())
                {
                    EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
                    if (hookMgr != nullptr) hookMgr->DrawSceneTabContextItems();
                    ImGui::EndPopup();
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
            enum class PlayTarget { PlayInEditor, PlayFullScreen, Dolphin, Azahar, Standalone, Send3dsLink, Count };
            int32_t& playTargetRef = GetEditorState()->mPlayTarget;
            PlayTarget currentPlayTarget = (PlayTarget)playTargetRef;

            const char* playTargetLabels[] = {
                "Play In Editor",
                "Play Full Screen",
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
            const char* targetLabel = playTargetLabels[(int)currentPlayTarget];
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
                    switch (currentPlayTarget)
                    {
                    case PlayTarget::PlayInEditor:
                        GetEditorState()->mPlayInGameWindow = true;
                        GetEditorState()->BeginPlayInEditor();
                        break;
                    case PlayTarget::PlayFullScreen:
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
                    { PlayTarget::PlayFullScreen, "Play Full Screen", true,       nullptr },
                    { PlayTarget::Dolphin,      "Play on Dolphin", dolphinOk,     "Configure Dolphin path in Preferences > External > Launchers" },
                    { PlayTarget::Azahar,       "Play on Azahar",  azaharOk,      "Configure Azahar path in Preferences > External > Launchers" },
                    { PlayTarget::Standalone,   "Play Standalone",  true,          nullptr },
                    { PlayTarget::Send3dsLink,  "Send 3dsLink",    threeDsLinkOk, "Configure 3dslink in Preferences > External > Launchers" },
                };

                for (const PlayTargetInfo& item : items)
                {
                    bool isSelected = (currentPlayTarget == item.target);

                    if (!item.enabled)
                    {
                        ImGui::BeginDisabled();
                    }

                    if (ImGui::Selectable(item.label, isSelected))
                    {
                        playTargetRef = (int32_t)item.target;
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

                // Draw addon play targets (Batch 8)
                {
                    EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
                    if (hookMgr != nullptr && hookMgr->HasPlayTargets())
                    {
                        ImGui::Separator();
                        hookMgr->DrawPlayTargets();
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

            if (shiftDown && IsKeyJustDown(OCTAVE_KEY_Q))
            {
                ImGui::OpenPopup("Spawn Basic 3D");
            }

            if (shiftDown && IsKeyJustDown(OCTAVE_KEY_W))
            {
                ImGui::OpenPopup("Spawn Basic Widget");
            }

            if (shiftDown && IsKeyJustDown(OCTAVE_KEY_A))
            {
                ImGui::OpenPopup("Spawn Node");
            }

            if (ctrlDown && IsKeyJustDown(OCTAVE_KEY_N))
            {
                GetEditorState()->OpenEditScene(nullptr);
            }

            if (ctrlDown && IsKeyJustDown(OCTAVE_KEY_R))
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

                // Refresh asset directories
                AssetDir* projDir = AssetManager::Get()->FindProjectDirectory();
                if (projDir)
                    AssetManager::Get()->RefreshDirectory(projDir);
                AssetDir* pkgDir = AssetManager::Get()->FindPackagesDirectory();
                if (pkgDir)
                    AssetManager::Get()->RefreshDirectory(pkgDir);

                // Sync UIDocument .xml <-> .oct files
                RefreshUIDocuments();
            }
        }

        // Process addon keyboard shortcuts (Batch 5)
        {
            EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
            if (hookMgr != nullptr) hookMgr->ProcessShortcuts();
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

    ImGui::PopStyleVar();

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
            DrawDirPicker();

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

    // Convert XML to New UIDocument popup
    if (!sConvertXmlSourcePath.empty() && !sShowUIDocumentPicker)
    {
        ImGui::OpenPopup("Convert to UIDocument");
    }

    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    }

    if (ImGui::BeginPopupModal("Convert to UIDocument", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
    {
        ImGui::Text("Create UIDocument from XML:");
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", sConvertXmlSourcePath.c_str());
        ImGui::Separator();

        bool create = ImGui::InputText("Asset Name", sPopupInputBuffer, kPopupInputBufferSize,
                                        ImGuiInputTextFlags_EnterReturnsTrue);

        ImGui::SameLine();
        if (ImGui::Button("Create"))
            create = true;

        ImGui::SameLine();
        if (ImGui::Button("Cancel##ConvertCancel"))
        {
            sConvertXmlSourcePath.clear();
            ImGui::CloseCurrentPopup();
        }

        if (create && sPopupInputBuffer[0] != '\0')
        {
            std::string assetName = sPopupInputBuffer;
            AssetDir* currentDir = GetEditorState()->GetAssetDirectory();

            if (currentDir != nullptr)
            {
                AssetStub* stub = EditorAddUniqueAsset(assetName.c_str(), currentDir,
                                                        UIDocument::GetStaticType(), false);
                if (stub != nullptr && stub->mAsset != nullptr)
                {
                    UIDocument* doc = stub->mAsset->As<UIDocument>();
                    if (doc != nullptr && doc->Import(sConvertXmlSourcePath, nullptr))
                    {
                        AssetManager::Get()->SaveAsset(*stub);
                        LogDebug("Created UIDocument '%s' from: %s", assetName.c_str(),
                                 sConvertXmlSourcePath.c_str());
                    }
                    else
                    {
                        LogError("Failed to import XML: %s", sConvertXmlSourcePath.c_str());
                    }
                }
            }

            sConvertXmlSourcePath.clear();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    // Refresh UIDocument picker popup
    if (sShowUIDocumentPicker)
    {
        ImGui::OpenPopup("Refresh UIDocument");
        sShowUIDocumentPicker = false;
    }

    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    }

    if (ImGui::BeginPopupModal("Refresh UIDocument", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
    {
        ImGui::Text("Select UIDocument to refresh with XML:");
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", sConvertXmlSourcePath.c_str());
        ImGui::Separator();

        static char filterBuffer[128] = {};
        ImGui::InputText("Search", filterBuffer, sizeof(filterBuffer));

        std::string filterLower = filterBuffer;
        std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(), ::tolower);

        if (ImGui::BeginListBox("##UIDocList", ImVec2(350, 250)))
        {
            TypeId uiDocType = UIDocument::GetStaticType();
            const auto& assetMap = AssetManager::Get()->GetAssetMap();

            for (const auto& pair : assetMap)
            {
                AssetStub* stub = pair.second;
                if (stub == nullptr || stub->mType != uiDocType)
                    continue;

                // Filter by search
                if (!filterLower.empty())
                {
                    std::string nameLower = stub->mName;
                    std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
                    if (nameLower.find(filterLower) == std::string::npos)
                        continue;
                }

                if (ImGui::Selectable(stub->mName.c_str()))
                {
                    // Load if needed
                    if (stub->mAsset == nullptr)
                        AssetManager::Get()->LoadAsset(*stub);

                    if (stub->mAsset != nullptr)
                    {
                        UIDocument* doc = stub->mAsset->As<UIDocument>();
                        if (doc != nullptr && doc->Import(sConvertXmlSourcePath, nullptr))
                        {
                            AssetManager::Get()->SaveAsset(*stub);
                            LogDebug("Refreshed UIDocument '%s' from: %s", stub->mName.c_str(),
                                     sConvertXmlSourcePath.c_str());
                        }
                        else
                        {
                            LogError("Failed to refresh UIDocument: %s", stub->mName.c_str());
                        }
                    }

                    filterBuffer[0] = '\0';
                    sConvertXmlSourcePath.clear();
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::EndListBox();
        }

        if (ImGui::Button("Cancel##RefreshCancel"))
        {
            filterBuffer[0] = '\0';
            sConvertXmlSourcePath.clear();
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
    const std::string relativePath = "Engine/Assets/Fonts/OctaveEngineIcons.ttf";
    const std::string absolutePath = SYS_GetAbsolutePath(relativePath.c_str());

    if (!SYS_DoesFileExist(absolutePath.c_str(), false))
    {
        LogWarning("Editor Icon font '%s' not found at %s.", relativePath.c_str(), absolutePath.c_str());
        return "";
    }

    LogDebug("Using editor icon font at %s", absolutePath.c_str());
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


    std::string iconFontPath = ResolveEditorIconFontPath();
    if (!iconFontPath.empty())
    {
        MergeOctaveIcons(io.Fonts, 14.0f, iconFontPath.c_str());
    }

    //ImGui::StyleColorsLight();

    // Override theme
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Tab] = kBgInactive;
    colors[ImGuiCol_TabHovered] = kBgHover;
    colors[ImGuiCol_TabActive] = kSelectedColor;



    // Set unactive window title bg equal to active title.
    colors[ImGuiCol_TitleBg] = colors[ImGuiCol_TitleBgActive];

    RegisterLogCallback(DebugLogWindow::LogCallback);

    ControllerServer::Create();

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
        GetBuildDependencyWindow()->Draw();
        ActionManager::Get()->DrawBuildModal();
        GetProjectSelectWindow()->Draw();
        GetAddonsWindow()->Draw();
        GetThemeEditorWindow()->Draw();

        if (GetThemeEditorWindow()->IsInspectModeActive())
        {
            GetThemeEditorWindow()->DrawInspectOverlay();
        }
    }

    if (ControllerServer::Get())
    {
        ControllerServer::Get()->Tick();
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
    ControllerServer::Destroy();

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
    if (!sViewportDockActive)
        return false;

    ImVec2 mp = ImGui::GetIO().MousePos;
    return (mp.x >= sViewportDockPos.x &&
            mp.x <  sViewportDockPos.x + sViewportDockSize.x &&
            mp.y >= sViewportDockPos.y &&
            mp.y <  sViewportDockPos.y + sViewportDockSize.y);
}

bool EditorIsInterfaceVisible()
{
    EditorState* es = GetEditorState();
    return es->mShowInterface && (!IsPlaying() || es->mEjected || es->mPlayInGameWindow);
}

#endif


