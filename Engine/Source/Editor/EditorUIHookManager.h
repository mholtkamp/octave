#pragma once

/**
 * @file EditorUIHookManager.h
 * @brief Manages registered UI hooks from plugins and Lua scripts.
 *
 * This entire file is wrapped in #if EDITOR to ensure editor code
 * does not leak into game builds.
 */

#if EDITOR

#include "Plugins/EditorUIHooks.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

/**
 * @brief Registered menu item.
 */
struct RegisteredMenuItem
{
    HookId mHookId;
    std::string mMenuPath;
    std::string mItemPath;
    MenuCallback mCallback;
    void* mUserData;
    std::string mShortcut;
    bool mIsSeparator = false;
};

/**
 * @brief Registered custom window.
 */
struct RegisteredWindow
{
    HookId mHookId;
    std::string mWindowName;
    std::string mWindowId;
    WindowDrawCallback mDrawFunc;
    void* mUserData;
    bool mIsOpen = false;
};

/**
 * @brief Registered custom inspector.
 */
struct RegisteredInspector
{
    HookId mHookId;
    std::string mNodeTypeName;
    InspectorDrawCallback mDrawFunc;
    void* mUserData;
};

/**
 * @brief Registered context menu item.
 */
struct RegisteredContextItem
{
    HookId mHookId;
    std::string mItemPath;
    std::string mAssetTypeFilter;  // Empty for node context, or asset type for asset context
    MenuCallback mCallback;
    void* mUserData;
    bool mIsNodeContext;  // true for node context, false for asset context
};

/**
 * @brief Registered event callback (generic, no extra params).
 */
struct RegisteredEventCallback
{
    HookId mHookId;
    EventCallback mCallback;
    void* mUserData;
};

/**
 * @brief Registered string event callback.
 */
struct RegisteredStringEventCallback
{
    HookId mHookId;
    StringEventCallback mCallback;
    void* mUserData;
};

/**
 * @brief Registered platform event callback.
 */
struct RegisteredPlatformEventCallback
{
    HookId mHookId;
    PlatformEventCallback mCallback;
    void* mUserData;
};

/**
 * @brief Registered package finished callback.
 */
struct RegisteredPackageFinishedCallback
{
    HookId mHookId;
    PackageFinishedCallback mCallback;
    void* mUserData;
};

/**
 * @brief Registered play mode callback.
 */
struct RegisteredPlayModeCallback
{
    HookId mHookId;
    PlayModeCallback mCallback;
    void* mUserData;
};

/**
 * @brief Registered top-level menu.
 */
struct RegisteredTopLevelMenu
{
    HookId mHookId;
    std::string mMenuName;
    TopLevelMenuDrawCallback mDrawFunc;
    void* mUserData;
    int32_t mPosition = -1;  // -1=append after all, 0=after File, 1=after Edit, etc.
};

/**
 * @brief Registered toolbar item.
 */
struct RegisteredToolbarItem
{
    HookId mHookId;
    std::string mItemName;
    ToolbarDrawCallback mDrawFunc;
    void* mUserData;
};

/**
 * @brief Registered menu item with validation.
 */
struct RegisteredMenuItemEx
{
    HookId mHookId;
    std::string mMenuPath;
    std::string mItemPath;
    MenuCallback mCallback;
    void* mUserData;
    std::string mShortcut;
    MenuValidationCallback mValidateFunc;
};

/**
 * @brief Registered node menu extension.
 */
struct RegisteredNodeMenuItems
{
    HookId mHookId;
    std::string mCategory;
    MenuSectionDrawCallback mDrawFunc;
    void* mUserData;
};

/**
 * @brief Registered create asset menu extension.
 */
struct RegisteredCreateAssetItems
{
    HookId mHookId;
    MenuSectionDrawCallback mDrawFunc;
    void* mUserData;
};

/**
 * @brief Registered spawn menu extension.
 */
struct RegisteredSpawnItems
{
    HookId mHookId;
    MenuSectionDrawCallback mDrawFunc;
    void* mUserData;
};

/**
 * @brief Registered viewport context menu item.
 */
struct RegisteredViewportContextItem
{
    HookId mHookId;
    std::string mItemPath;
    MenuCallback mCallback;
    void* mUserData;
};

/**
 * @brief Registered viewport overlay.
 */
struct RegisteredViewportOverlay
{
    HookId mHookId;
    std::string mOverlayName;
    ViewportOverlayCallback mDrawFunc;
    void* mUserData;
};

/**
 * @brief Registered preferences panel.
 */
struct RegisteredPreferencesPanel
{
    HookId mHookId;
    std::string mPanelName;
    std::string mPanelCategory;
    PreferencesPanelDrawCallback mDrawFunc;
    PreferencesLoadCallback mLoadFunc;
    PreferencesSaveCallback mSaveFunc;
    void* mUserData;
};

/**
 * @brief Registered keyboard shortcut.
 */
struct RegisteredShortcut
{
    HookId mHookId;
    std::string mShortcutId;
    std::string mDisplayName;
    std::string mDefaultBinding;
    ShortcutCallback mCallback;
    void* mUserData;
    // Parsed key info
    bool mCtrl = false;
    bool mShift = false;
    bool mAlt = false;
    int32_t mKeyCode = -1;
};

/**
 * @brief Registered property drawer.
 */
struct RegisteredPropertyDrawer
{
    HookId mHookId;
    std::string mPropertyTypeName;
    PropertyDrawCallback mDrawFunc;
    void* mUserData;
};

/**
 * @brief Registered hierarchy item GUI overlay.
 */
struct RegisteredHierarchyItemGUI
{
    HookId mHookId;
    HierarchyItemGUICallback mDrawFunc;
    void* mUserData;
};

/**
 * @brief Registered asset browser item GUI overlay.
 */
struct RegisteredAssetItemGUI
{
    HookId mHookId;
    AssetItemGUICallback mDrawFunc;
    void* mUserData;
};

/**
 * @brief Registered hierarchy changed callback.
 */
struct RegisteredHierarchyChangedCallback
{
    HookId mHookId;
    HierarchyChangedCallback mCallback;
    void* mUserData;
};

/**
 * @brief Registered simple context menu item (used by scene tabs, debug log, import, addons menus).
 */
struct RegisteredSimpleContextItem
{
    HookId mHookId;
    std::string mItemPath;
    MenuCallback mCallback;
    void* mUserData;
};

/**
 * @brief Registered play target.
 */
struct RegisteredPlayTarget
{
    HookId mHookId;
    std::string mTargetName;
    std::string mIconText;
    PlayTargetCallback mCallback;
    void* mUserData;
};

/**
 * @brief Registered drag-drop handler.
 */
struct RegisteredDragDropHandler
{
    HookId mHookId;
    std::string mTargetArea;
    DragDropHandlerCallback mHandler;
    void* mUserData;
};

/**
 * @brief Registered asset importer.
 */
struct RegisteredAssetImporter
{
    HookId mHookId;
    std::string mExtension;
    AssetImportCallback mImportFunc;
    void* mUserData;
};

/**
 * @brief Registered pre-import callback.
 */
struct RegisteredPreImportCallback
{
    HookId mHookId;
    PreImportCallback mCallback;
    void* mUserData;
};

/**
 * @brief Registered pre-build callback.
 */
struct RegisteredPreBuildCallback
{
    HookId mHookId;
    PreBuildCallback mCallback;
    void* mUserData;
};

/**
 * @brief Registered editor mode callback.
 */
struct RegisteredEditorModeCallback
{
    HookId mHookId;
    EditorModeCallback mCallback;
    void* mUserData;
};

/**
 * @brief Registered gizmo tool.
 */
struct RegisteredGizmoTool
{
    HookId mHookId;
    std::string mToolName;
    std::string mIconText;
    std::string mTooltip;
    GizmoToolDrawCallback mDrawFunc;
    void* mUserData;
    bool mIsActive = false;
};

/**
 * @brief Singleton manager for editor UI hooks.
 *
 * Stores all registered hooks and provides rendering helpers.
 */
class EditorUIHookManager
{
public:
    static void Create();
    static void Destroy();
    static EditorUIHookManager* Get();

    /**
     * @brief Get the EditorUIHooks struct for plugins.
     */
    EditorUIHooks* GetHooks() { return &mHooks; }

    // ===== Menu Items =====

    /**
     * @brief Get menu items for a specific menu path.
     */
    const std::vector<RegisteredMenuItem>& GetMenuItems(const std::string& menuPath) const;

    /**
     * @brief Draw plugin menu items for a menu.
     * Call this inside ImGui::BeginMenu/EndMenu.
     */
    void DrawMenuItems(const std::string& menuPath);

    // ===== Custom Windows =====

    /**
     * @brief Get all registered windows.
     */
    const std::vector<RegisteredWindow>& GetWindows() const { return mWindows; }

    /**
     * @brief Draw all open custom windows.
     */
    void DrawWindows();

    /**
     * @brief Open a window by ID.
     */
    void OpenWindow(const std::string& windowId);

    /**
     * @brief Close a window by ID.
     */
    void CloseWindow(const std::string& windowId);

    /**
     * @brief Check if a window is open.
     */
    bool IsWindowOpen(const std::string& windowId) const;

    // ===== Inspectors =====

    /**
     * @brief Get inspector for a node type.
     * @return Pointer to inspector, or nullptr if none registered.
     */
    const RegisteredInspector* GetInspector(const std::string& nodeTypeName) const;

    /**
     * @brief Draw custom inspector for a node.
     * @return true if a custom inspector was drawn.
     */
    bool DrawInspector(const std::string& nodeTypeName, void* node);

    // ===== Context Menus =====

    /**
     * @brief Draw node context menu items.
     */
    void DrawNodeContextItems();

    /**
     * @brief Draw asset context menu items.
     * @param assetType The type of asset being right-clicked
     */
    void DrawAssetContextItems(const std::string& assetType);

    // ===== Top-Level Menus =====

    /**
     * @brief Draw all registered top-level menus in the viewport bar (legacy, appends at end).
     */
    void DrawTopLevelMenus();

    /**
     * @brief Draw addon top-level menus positioned after a built-in menu.
     * @param builtinPosition The position index of the built-in menu that just finished drawing.
     *        0=File, 1=Edit, 2=View, 3=World, 4=Developer, 5=Addons, 6=Extra
     */
    void DrawTopLevelMenusAtPosition(int32_t builtinPosition);

    // ===== Toolbar =====

    void DrawToolbarItems();

    // ===== Batch 2: Create/Spawn Menu Extensions =====

    void DrawNodeMenuItems(const char* category, void* parentNode);
    void DrawCustomNodeCategories(void* parentNode);
    void DrawCreateAssetItems();
    void DrawSpawnBasic3dItems(void* parentNode);
    void DrawSpawnBasicWidgetItems(void* parentNode);

    // ===== Batch 3: Viewport Context Menu & Overlay Drawing =====

    void DrawViewportContextItems();
    void DrawViewportOverlays(float viewportX, float viewportY, float viewportW, float viewportH);
    bool HasViewportContextItems() const;

    // ===== Batch 4: Custom Preferences =====

    const std::vector<RegisteredPreferencesPanel>& GetPreferencesPanels() const { return mPreferencesPanels; }
    void LoadAddonPreferences();
    void SaveAddonPreferences();

    // ===== Batch 5: Keyboard Shortcuts =====

    void ProcessShortcuts();

    // ===== Batch 6: Property Drawers =====

    bool DrawPropertyDrawer(const char* propertyTypeName, const char* propertyName, void* propertyOwner, int32_t propertyType);

    // ===== Batch 7: Hierarchy & Asset Browser Extensions =====

    void DrawHierarchyItemGUI(void* node, float rowX, float rowY, float rowW, float rowH);
    void DrawAssetItemGUI(const char* assetName, const char* assetType, float rowX, float rowY, float rowW, float rowH);
    void FireOnHierarchyChanged(int32_t changeType, void* node);

    // ===== Batch 8: Additional Context Menus =====

    void DrawSceneTabContextItems();
    void DrawDebugLogContextItems();
    void DrawImportMenuItems();
    void DrawAddonsMenuItems();
    void DrawPlayTargets();
    bool HasPlayTargets() const;

    // ===== Batch 9: Drag-Drop & Asset Pipeline =====

    bool HandleDragDrop(const char* targetArea, const char* payloadType, const void* payloadData, int32_t payloadSize);
    bool HandleAssetImport(const char* filePath, const char* extension);
    bool FireOnPreAssetImport(const char* filePath);
    void FireOnPostAssetImport(const char* assetPath);

    // ===== Batch 10: Build Pipeline & Editor State =====

    bool FireOnPreBuild(int32_t platform);
    void FireOnPostBuild(int32_t platform, bool success);
    void FireOnEditorModeChanged(int32_t newMode);
    void DrawGizmoTools(void* selectedNode);
    const std::vector<RegisteredGizmoTool>& GetGizmoTools() const { return mGizmoTools; }

    // ===== Event Dispatchers =====

    void FireOnProjectOpen(const char* projectPath);
    void FireOnProjectClose(const char* projectPath);
    void FireOnProjectSave(const char* filePath);
    void FireOnSceneOpen(const char* scenePath);
    void FireOnSceneClose(const char* scenePath);
    void FireOnPackageStarted(int32_t platform);
    void FireOnPackageFinished(int32_t platform, bool success);
    void FireOnSelectionChanged();
    void FireOnPlayModeChanged(int32_t state);
    void FireOnEditorShutdown();
    void FireOnAssetImported(const char* assetPath);
    void FireOnAssetDeleted(const char* assetPath);
    void FireOnAssetSaved(const char* assetPath);
    void FireOnAssetOpen(const char* assetName);
    void FireOnAssetOpened(const char* assetName);
    void FireOnUndoRedo();
    void FireOnAssetDropHierarchy(const char* assetName);
    void FireOnAssetDropViewport(const char* assetName);

    // ===== Cleanup =====

    /**
     * @brief Remove all hooks registered by a specific hook ID.
     */
    void RemoveAllHooks(HookId hookId);

private:
    static EditorUIHookManager* sInstance;
    EditorUIHookManager();
    ~EditorUIHookManager();

    void InitializeHooks();

    // Hook storage
    std::unordered_map<std::string, std::vector<RegisteredMenuItem>> mMenuItems;
    std::vector<RegisteredWindow> mWindows;
    std::vector<RegisteredInspector> mInspectors;
    std::vector<RegisteredContextItem> mContextItems;

    // New hook storage
    std::vector<RegisteredTopLevelMenu> mTopLevelMenus;
    std::vector<RegisteredToolbarItem> mToolbarItems;
    std::vector<RegisteredStringEventCallback> mOnProjectOpen;
    std::vector<RegisteredStringEventCallback> mOnProjectClose;
    std::vector<RegisteredStringEventCallback> mOnProjectSave;
    std::vector<RegisteredStringEventCallback> mOnSceneOpen;
    std::vector<RegisteredStringEventCallback> mOnSceneClose;
    std::vector<RegisteredPlatformEventCallback> mOnPackageStarted;
    std::vector<RegisteredPackageFinishedCallback> mOnPackageFinished;
    std::vector<RegisteredEventCallback> mOnSelectionChanged;
    std::vector<RegisteredPlayModeCallback> mOnPlayModeChanged;
    std::vector<RegisteredEventCallback> mOnEditorShutdown;
    std::vector<RegisteredStringEventCallback> mOnAssetImported;
    std::vector<RegisteredStringEventCallback> mOnAssetDeleted;
    std::vector<RegisteredStringEventCallback> mOnAssetSaved;
    std::vector<RegisteredStringEventCallback> mOnAssetOpen;
    std::vector<RegisteredStringEventCallback> mOnAssetOpened;
    std::vector<RegisteredEventCallback> mOnUndoRedo;
    std::vector<RegisteredStringEventCallback> mOnAssetDropHierarchy;
    std::vector<RegisteredStringEventCallback> mOnAssetDropViewport;

    // Batch 1: Menu items with validation
    std::vector<RegisteredMenuItemEx> mMenuItemsEx;

    // Batch 2: Create/Spawn menu extensions
    std::vector<RegisteredNodeMenuItems> mNodeMenuItems;
    std::vector<RegisteredCreateAssetItems> mCreateAssetItems;
    std::vector<RegisteredSpawnItems> mSpawnBasic3dItems;
    std::vector<RegisteredSpawnItems> mSpawnBasicWidgetItems;

    // Batch 3: Viewport context + overlays
    std::vector<RegisteredViewportContextItem> mViewportContextItems;
    std::vector<RegisteredViewportOverlay> mViewportOverlays;

    // Batch 4: Preferences panels
    std::vector<RegisteredPreferencesPanel> mPreferencesPanels;

    // Batch 5: Keyboard shortcuts
    std::vector<RegisteredShortcut> mShortcuts;

    // Batch 6: Property drawers
    std::vector<RegisteredPropertyDrawer> mPropertyDrawers;

    // Batch 7: Hierarchy & asset browser
    std::vector<RegisteredHierarchyItemGUI> mHierarchyItemGUI;
    std::vector<RegisteredAssetItemGUI> mAssetItemGUI;
    std::vector<RegisteredHierarchyChangedCallback> mOnHierarchyChanged;

    // Batch 8: Additional context menus
    std::vector<RegisteredSimpleContextItem> mSceneTabContextItems;
    std::vector<RegisteredSimpleContextItem> mDebugLogContextItems;
    std::vector<RegisteredSimpleContextItem> mImportMenuItems;
    std::vector<RegisteredSimpleContextItem> mAddonsMenuItems;
    std::vector<RegisteredPlayTarget> mPlayTargets;

    // Batch 9: Drag-drop & asset pipeline
    std::vector<RegisteredDragDropHandler> mDragDropHandlers;
    std::vector<RegisteredAssetImporter> mAssetImporters;
    std::vector<RegisteredPreImportCallback> mOnPreAssetImport;
    std::vector<RegisteredStringEventCallback> mOnPostAssetImport;

    // Batch 10: Build pipeline & editor state
    std::vector<RegisteredPreBuildCallback> mOnPreBuild;
    std::vector<RegisteredPackageFinishedCallback> mOnPostBuild;
    std::vector<RegisteredEditorModeCallback> mOnEditorModeChanged;
    std::vector<RegisteredGizmoTool> mGizmoTools;

    // Empty vector for returning when menu not found
    std::vector<RegisteredMenuItem> mEmptyMenuItems;

    // Hooks struct passed to plugins
    EditorUIHooks mHooks;

    // Helper: Parse key binding string to shortcut fields
    void ParseKeyBinding(RegisteredShortcut& shortcut);
};

/**
 * @brief Generate a HookId from a string identifier.
 */
HookId GenerateHookId(const char* identifier);

#endif // EDITOR
