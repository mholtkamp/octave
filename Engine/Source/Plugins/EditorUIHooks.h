#pragma once

/**
 * @file EditorUIHooks.h
 * @brief Editor UI extension system for native addons.
 *
 * Provides hooks for plugins and Lua scripts to extend the editor UI
 * including menus, custom windows, inspectors, and context menus.
 *
 * This entire file is wrapped in #if EDITOR to ensure editor code
 * does not leak into game builds.
 */

#if EDITOR

#include <stdint.h>

// ===== Callback Types =====

/**
 * @brief Callback for menu item clicks.
 * @param userData User data passed during registration
 */
typedef void (*MenuCallback)(void* userData);

/**
 * @brief Callback for drawing custom window content.
 * @param userData User data passed during registration
 */
typedef void (*WindowDrawCallback)(void* userData);

/**
 * @brief Callback for drawing custom inspector content.
 * @param node Pointer to the node being inspected
 * @param userData User data passed during registration
 */
typedef void (*InspectorDrawCallback)(void* node, void* userData);

/**
 * @brief Generic event callback with no additional data.
 * @param userData User data passed during registration
 */
typedef void (*EventCallback)(void* userData);

/**
 * @brief Event callback that receives a string parameter (path, name, etc.).
 * @param str Context string (file path, project path, scene path, asset path)
 * @param userData User data passed during registration
 */
typedef void (*StringEventCallback)(const char* str, void* userData);

/**
 * @brief Callback for platform-specific packaging events.
 * @param platform Platform enum value (see Platform in SystemTypes.h)
 * @param userData User data passed during registration
 */
typedef void (*PlatformEventCallback)(int32_t platform, void* userData);

/**
 * @brief Callback for packaging completion with success/failure status.
 * @param platform Platform enum value that was packaged
 * @param success true if packaging succeeded, false if it failed
 * @param userData User data passed during registration
 */
typedef void (*PackageFinishedCallback)(int32_t platform, bool success, void* userData);

/**
 * @brief Callback for play mode state changes.
 * @param state Play mode state: 0=Enter, 1=Exit, 2=Eject
 * @param userData User data passed during registration
 */
typedef void (*PlayModeCallback)(int32_t state, void* userData);

/**
 * @brief Draw callback for custom top-level menus.
 *
 * Called inside ImGui::BeginMenu/EndMenu. Use ImGui::MenuItem() etc. inside.
 * @param userData User data passed during registration
 */
typedef void (*TopLevelMenuDrawCallback)(void* userData);

/**
 * @brief Draw callback for custom toolbar items.
 *
 * Called inside the viewport toolbar area. Use ImGui widgets to draw controls.
 * @param userData User data passed during registration
 */
typedef void (*ToolbarDrawCallback)(void* userData);

/**
 * @brief Callback for menu item validation (enable/disable).
 * @param userData User data passed during registration
 * @return true if menu item should be enabled, false to grey it out
 */
typedef bool (*MenuValidationCallback)(void* userData);

/**
 * @brief Draw callback for menu sections (called inside BeginMenu/EndMenu).
 * @param parentNode Pointer to the parent node context (can be nullptr)
 * @param userData User data passed during registration
 */
typedef void (*MenuSectionDrawCallback)(void* parentNode, void* userData);

/**
 * @brief Callback for viewport overlay drawing.
 * @param viewportX X position of viewport
 * @param viewportY Y position of viewport
 * @param viewportW Width of viewport
 * @param viewportH Height of viewport
 * @param userData User data passed during registration
 */
typedef void (*ViewportOverlayCallback)(float viewportX, float viewportY, float viewportW, float viewportH, void* userData);

/**
 * @brief Callback for preferences panel drawing.
 * @param userData User data passed during registration
 */
typedef void (*PreferencesPanelDrawCallback)(void* userData);

/**
 * @brief Callback for preferences load/save.
 * @param userData User data passed during registration
 */
typedef void (*PreferencesLoadCallback)(void* userData);
typedef void (*PreferencesSaveCallback)(void* userData);

/**
 * @brief Callback for keyboard shortcuts.
 * @param userData User data passed during registration
 */
typedef void (*ShortcutCallback)(void* userData);

/**
 * @brief Property drawer callback - return true if handled.
 * @param propertyName Name of the property being drawn
 * @param propertyOwner Pointer to the object that owns the property
 * @param propertyType Type identifier for the property
 * @param userData User data passed during registration
 * @return true if the property was drawn by this callback
 */
typedef bool (*PropertyDrawCallback)(const char* propertyName, void* propertyOwner, int32_t propertyType, void* userData);

/**
 * @brief Hierarchy item GUI overlay callback.
 * @param node Pointer to the node being drawn
 * @param rowX X position of the row
 * @param rowY Y position of the row
 * @param rowW Width of the row
 * @param rowH Height of the row
 * @param userData User data passed during registration
 */
typedef void (*HierarchyItemGUICallback)(void* node, float rowX, float rowY, float rowW, float rowH, void* userData);

/**
 * @brief Asset browser item GUI overlay callback.
 * @param assetName Name of the asset
 * @param assetType Type name of the asset
 * @param rowX X position of the row
 * @param rowY Y position of the row
 * @param rowW Width of the row
 * @param rowH Height of the row
 * @param userData User data passed during registration
 */
typedef void (*AssetItemGUICallback)(const char* assetName, const char* assetType, float rowX, float rowY, float rowW, float rowH, void* userData);

/**
 * @brief Hierarchy changed event callback.
 * @param changeType 0=NodeCreated, 1=NodeDestroyed, 2=NodeReparented, 3=NodeRenamed
 * @param node Pointer to the affected node
 * @param userData User data passed during registration
 */
typedef void (*HierarchyChangedCallback)(int32_t changeType, void* node, void* userData);

/**
 * @brief Custom drag-drop handler callback.
 * @param payloadType Type string of the drag payload
 * @param payloadData Pointer to the payload data
 * @param payloadSize Size of the payload data in bytes
 * @param userData User data passed during registration
 * @return true to consume the drop, false to pass through
 */
typedef bool (*DragDropHandlerCallback)(const char* payloadType, const void* payloadData, int32_t payloadSize, void* userData);

/**
 * @brief Custom asset import callback.
 * @param filePath Path of the file to import
 * @param extension File extension (e.g., ".fbx")
 * @param userData User data passed during registration
 * @return true if the import was handled
 */
typedef bool (*AssetImportCallback)(const char* filePath, const char* extension, void* userData);

/**
 * @brief Pre-import callback. Return false to cancel the import.
 * @param filePath Path of the file about to be imported
 * @param userData User data passed during registration
 * @return false to cancel the import
 */
typedef bool (*PreImportCallback)(const char* filePath, void* userData);

/**
 * @brief Pre-build callback. Return false to cancel the build.
 * @param platform Platform enum value
 * @param userData User data passed during registration
 * @return false to cancel the build
 */
typedef bool (*PreBuildCallback)(int32_t platform, void* userData);

/**
 * @brief Editor mode changed callback.
 * @param newMode New editor mode value
 * @param userData User data passed during registration
 */
typedef void (*EditorModeCallback)(int32_t newMode, void* userData);

/**
 * @brief Custom gizmo tool draw callback.
 * @param selectedNode Pointer to the currently selected node
 * @param userData User data passed during registration
 */
typedef void (*GizmoToolDrawCallback)(void* selectedNode, void* userData);

/**
 * @brief Play target callback.
 * @param userData User data passed during registration
 */
typedef void (*PlayTargetCallback)(void* userData);

/**
 * @brief Unique identifier for tracking hooks.
 *
 * Use GenerateHookId() to create from addon ID or Lua script UUID.
 * This allows proper cleanup when plugins are unloaded.
 */
typedef uint64_t HookId;

/**
 * @brief Editor UI extension hooks.
 *
 * Provides functions for plugins to extend the editor UI.
 * All registration functions take a HookId for tracking and cleanup.
 */
struct EditorUIHooks
{
    // ===== Menu Extensions =====

    /**
     * @brief Add a menu item to an existing menu.
     *
     * @param hookId Unique identifier for this hook (for cleanup)
     * @param menuPath Top-level menu: "File", "Edit", "View", "Developer", "Help"
     * @param itemPath Item path within menu, e.g., "My Tool" or "Submenu/My Tool"
     * @param callback Function called when item is clicked
     * @param userData User data passed to callback
     * @param shortcut Optional keyboard shortcut, e.g., "Ctrl+Shift+M" (can be nullptr)
     */
    void (*AddMenuItem)(
        HookId hookId,
        const char* menuPath,
        const char* itemPath,
        MenuCallback callback,
        void* userData,
        const char* shortcut
    );

    /**
     * @brief Add a separator in a menu.
     *
     * @param hookId Unique identifier for this hook
     * @param menuPath Menu to add separator to
     */
    void (*AddMenuSeparator)(HookId hookId, const char* menuPath);

    /**
     * @brief Remove a previously added menu item.
     *
     * @param hookId Hook identifier used during registration
     * @param menuPath Menu containing the item
     * @param itemPath Path of item to remove
     */
    void (*RemoveMenuItem)(HookId hookId, const char* menuPath, const char* itemPath);

    // ===== Custom Windows =====

    /**
     * @brief Register a custom dockable window.
     *
     * @param hookId Unique identifier for this hook
     * @param windowName Display name shown in title bar
     * @param windowId Unique ID for docking persistence
     * @param drawFunc Function called to draw window content
     * @param userData User data passed to drawFunc
     */
    void (*RegisterWindow)(
        HookId hookId,
        const char* windowName,
        const char* windowId,
        WindowDrawCallback drawFunc,
        void* userData
    );

    /**
     * @brief Unregister a custom window.
     *
     * @param hookId Hook identifier used during registration
     * @param windowId Window ID to unregister
     */
    void (*UnregisterWindow)(HookId hookId, const char* windowId);

    /**
     * @brief Open a custom window by ID.
     * @param windowId Window ID to open
     */
    void (*OpenWindow)(const char* windowId);

    /**
     * @brief Close a custom window by ID.
     * @param windowId Window ID to close
     */
    void (*CloseWindow)(const char* windowId);

    /**
     * @brief Check if a custom window is currently open.
     * @param windowId Window ID to check
     * @return true if window is open
     */
    bool (*IsWindowOpen)(const char* windowId);

    // ===== Inspector Extensions =====

    /**
     * @brief Register a custom inspector for a node type.
     *
     * @param hookId Unique identifier for this hook
     * @param nodeTypeName Type name of node, e.g., "MyCustomNode"
     * @param drawFunc Function called to draw inspector content
     * @param userData User data passed to drawFunc
     */
    void (*RegisterInspector)(
        HookId hookId,
        const char* nodeTypeName,
        InspectorDrawCallback drawFunc,
        void* userData
    );

    /**
     * @brief Unregister a custom inspector.
     *
     * @param hookId Hook identifier used during registration
     * @param nodeTypeName Node type name to unregister
     */
    void (*UnregisterInspector)(HookId hookId, const char* nodeTypeName);

    // ===== Context Menu Extensions =====

    /**
     * @brief Add item to node context menu (right-click in hierarchy).
     *
     * @param hookId Unique identifier for this hook
     * @param itemPath Item path in context menu
     * @param callback Function called when item is clicked
     * @param userData User data passed to callback
     */
    void (*AddNodeContextItem)(
        HookId hookId,
        const char* itemPath,
        MenuCallback callback,
        void* userData
    );

    /**
     * @brief Add item to asset context menu (right-click in asset browser).
     *
     * @param hookId Unique identifier for this hook
     * @param itemPath Item path in context menu
     * @param assetTypeFilter Asset type to show for, e.g., "Texture", or "*" for all
     * @param callback Function called when item is clicked
     * @param userData User data passed to callback
     */
    void (*AddAssetContextItem)(
        HookId hookId,
        const char* itemPath,
        const char* assetTypeFilter,
        MenuCallback callback,
        void* userData
    );

    // ===== Top-Level Menus =====

    /**
     * @brief Add a custom top-level menu to the editor viewport bar.
     *
     * The drawFunc is called inside ImGui::BeginMenu/EndMenu.
     * Use ImGui::MenuItem(), ImGui::Separator(), etc. inside.
     *
     * @param hookId Unique identifier for this hook (for cleanup)
     * @param menuName Display name for the top-level menu button
     * @param drawFunc Function called to draw menu contents
     * @param userData User data passed to drawFunc
     */
    void (*AddTopLevelMenuItem)(HookId hookId, const char* menuName,
                                TopLevelMenuDrawCallback drawFunc, void* userData);

    /**
     * @brief Remove a previously added top-level menu.
     * @param hookId Hook identifier used during registration
     * @param menuName Name of the menu to remove
     */
    void (*RemoveTopLevelMenuItem)(HookId hookId, const char* menuName);

    // ===== Toolbar =====

    /**
     * @brief Add a custom item to the editor viewport toolbar.
     * @param hookId Unique identifier for this hook (for cleanup)
     * @param itemName Unique name for the toolbar item
     * @param drawFunc Function called to draw toolbar content (buttons, etc.)
     * @param userData User data passed to drawFunc
     */
    void (*AddToolbarItem)(HookId hookId, const char* itemName,
                           ToolbarDrawCallback drawFunc, void* userData);

    /**
     * @brief Remove a previously added toolbar item.
     * @param hookId Hook identifier used during registration
     * @param itemName Name of the toolbar item to remove
     */
    void (*RemoveToolbarItem)(HookId hookId, const char* itemName);

    // ===== Project Lifecycle Events =====

    /** @brief Register callback for when a project is opened. Receives project path. */
    void (*RegisterOnProjectOpen)(HookId hookId, StringEventCallback cb, void* userData);

    /** @brief Register callback for when a project is about to close. Receives project path. */
    void (*RegisterOnProjectClose)(HookId hookId, StringEventCallback cb, void* userData);

    /** @brief Register callback for when the project/scene is saved. Receives file path. */
    void (*RegisterOnProjectSave)(HookId hookId, StringEventCallback cb, void* userData);

    // ===== Scene Lifecycle Events =====

    /** @brief Register callback for when a scene is opened for editing. Receives scene path. */
    void (*RegisterOnSceneOpen)(HookId hookId, StringEventCallback cb, void* userData);

    /** @brief Register callback for when a scene is closed. Receives scene path. */
    void (*RegisterOnSceneClose)(HookId hookId, StringEventCallback cb, void* userData);

    // ===== Packaging/Build Events =====

    /** @brief Register callback for when packaging starts. Receives platform enum. */
    void (*RegisterOnPackageStarted)(HookId hookId, PlatformEventCallback cb, void* userData);

    /** @brief Register callback for when packaging finishes. Receives platform and success. */
    void (*RegisterOnPackageFinished)(HookId hookId, PackageFinishedCallback cb, void* userData);

    // ===== Editor State Events =====

    /** @brief Register callback for when the selected node(s) change in the editor. */
    void (*RegisterOnSelectionChanged)(HookId hookId, EventCallback cb, void* userData);

    /** @brief Register callback for Play-In-Editor state changes (Enter/Exit/Eject). */
    void (*RegisterOnPlayModeChanged)(HookId hookId, PlayModeCallback cb, void* userData);

    /** @brief Register callback for when the editor is shutting down. Called before cleanup. */
    void (*RegisterOnEditorShutdown)(HookId hookId, EventCallback cb, void* userData);

    // ===== Asset Pipeline Events =====

    /** @brief Register callback for when an asset is imported. Receives asset path. */
    void (*RegisterOnAssetImported)(HookId hookId, StringEventCallback cb, void* userData);

    /** @brief Register callback for when an asset is deleted. Receives asset path. */
    void (*RegisterOnAssetDeleted)(HookId hookId, StringEventCallback cb, void* userData);

    /** @brief Register callback for when an asset is saved. Receives asset path. */
    void (*RegisterOnAssetSaved)(HookId hookId, StringEventCallback cb, void* userData);

    // ===== Asset Open Events =====

    /** @brief Register callback for when an asset is about to be opened (double-clicked). Receives asset name. */
    void (*RegisterOnAssetOpen)(HookId hookId, StringEventCallback cb, void* userData);

    /** @brief Register callback for after an asset has been opened (loaded and displayed). Receives asset name. */
    void (*RegisterOnAssetOpened)(HookId hookId, StringEventCallback cb, void* userData);

    // ===== Undo/Redo =====

    /** @brief Register callback for when an undo or redo operation is performed. */
    void (*RegisterOnUndoRedo)(HookId hookId, EventCallback cb, void* userData);

    // ===== Drag-and-Drop Events =====

    /** @brief Register callback for when an asset is dropped onto the scene hierarchy. Receives asset name. */
    void (*RegisterOnAssetDropHierarchy)(HookId hookId, StringEventCallback cb, void* userData);

    /** @brief Register callback for when an asset is dropped onto the viewport. Receives asset name. */
    void (*RegisterOnAssetDropViewport)(HookId hookId, StringEventCallback cb, void* userData);

    // ===== Batch 1: Menu Positioning & Top-Level Menu Control =====

    /**
     * @brief Add a custom top-level menu with position control.
     * @param hookId Unique identifier for this hook
     * @param menuName Display name for the menu
     * @param drawFunc Function called to draw menu contents
     * @param userData User data passed to drawFunc
     * @param position Position index: -1=append after all, 0=after File, 1=after Edit, 2=after View, 3=after World, 4=after Developer, 5=after Addons, 6=after Extra
     */
    void (*AddTopLevelMenuItemEx)(
        HookId hookId,
        const char* menuName,
        TopLevelMenuDrawCallback drawFunc,
        void* userData,
        int32_t position
    );

    /**
     * @brief Add a menu item with validation callback.
     * @param hookId Unique identifier for this hook
     * @param menuPath Top-level menu name
     * @param itemPath Item path within menu
     * @param callback Function called when item is clicked
     * @param userData User data passed to callback
     * @param shortcut Optional keyboard shortcut (can be nullptr)
     * @param validateFunc Validation callback - return false to grey out item (can be nullptr)
     */
    void (*AddMenuItemEx)(
        HookId hookId,
        const char* menuPath,
        const char* itemPath,
        MenuCallback callback,
        void* userData,
        const char* shortcut,
        MenuValidationCallback validateFunc
    );

    // ===== Batch 2: Create/Spawn Menu Extensions =====

    /**
     * @brief Extend the "Add Node" submenu.
     * @param hookId Unique identifier for this hook
     * @param category Category name: "3D", "Widget", "Other", or custom name for a new submenu
     * @param drawFunc Function called to draw menu items
     * @param userData User data passed to drawFunc
     */
    void (*AddNodeMenuItems)(HookId hookId, const char* category, MenuSectionDrawCallback drawFunc, void* userData);

    /** @brief Remove previously added node menu items. */
    void (*RemoveNodeMenuItems)(HookId hookId, const char* category);

    /**
     * @brief Extend the "Create Asset" submenu in asset browser context menu.
     * @param hookId Unique identifier for this hook
     * @param drawFunc Function called to draw menu items
     * @param userData User data passed to drawFunc
     */
    void (*AddCreateAssetItems)(HookId hookId, MenuSectionDrawCallback drawFunc, void* userData);

    /** @brief Remove previously added create asset items. */
    void (*RemoveCreateAssetItems)(HookId hookId);

    /**
     * @brief Extend the "Spawn Basic 3D" menu.
     * @param hookId Unique identifier for this hook
     * @param drawFunc Function called to draw additional 3D spawn items
     * @param userData User data passed to drawFunc
     */
    void (*AddSpawnBasic3dItems)(HookId hookId, MenuSectionDrawCallback drawFunc, void* userData);

    /**
     * @brief Extend the "Spawn Basic Widget" menu.
     * @param hookId Unique identifier for this hook
     * @param drawFunc Function called to draw additional widget spawn items
     * @param userData User data passed to drawFunc
     */
    void (*AddSpawnBasicWidgetItems)(HookId hookId, MenuSectionDrawCallback drawFunc, void* userData);

    // ===== Batch 3: Viewport Context Menu & Overlay Drawing =====

    /**
     * @brief Add item to viewport right-click context menu.
     * @param hookId Unique identifier for this hook
     * @param itemPath Item path in context menu
     * @param callback Function called when item is clicked
     * @param userData User data passed to callback
     */
    void (*AddViewportContextItem)(HookId hookId, const char* itemPath, MenuCallback callback, void* userData);

    /** @brief Remove a viewport context menu item. */
    void (*RemoveViewportContextItem)(HookId hookId, const char* itemPath);

    /**
     * @brief Register a viewport overlay drawn each frame.
     * @param hookId Unique identifier for this hook
     * @param overlayName Unique name for the overlay
     * @param drawFunc Function called each frame with viewport dimensions
     * @param userData User data passed to drawFunc
     */
    void (*RegisterViewportOverlay)(HookId hookId, const char* overlayName, ViewportOverlayCallback drawFunc, void* userData);

    /** @brief Unregister a viewport overlay. */
    void (*UnregisterViewportOverlay)(HookId hookId, const char* overlayName);

    // ===== Batch 4: Custom Preferences/Settings Pages =====

    /**
     * @brief Register a custom preferences panel.
     * @param hookId Unique identifier for this hook
     * @param panelName Display name in preferences sidebar
     * @param panelCategory Category path, e.g., "Addons/MyAddon"
     * @param drawFunc Function called to draw panel content
     * @param loadFunc Optional load callback (can be nullptr)
     * @param saveFunc Optional save callback (can be nullptr)
     * @param userData User data passed to callbacks
     */
    void (*RegisterPreferencesPanel)(
        HookId hookId,
        const char* panelName,
        const char* panelCategory,
        PreferencesPanelDrawCallback drawFunc,
        PreferencesLoadCallback loadFunc,
        PreferencesSaveCallback saveFunc,
        void* userData
    );

    /** @brief Unregister a custom preferences panel. */
    void (*UnregisterPreferencesPanel)(HookId hookId, const char* panelName);

    // ===== Batch 5: Custom Keyboard Shortcuts =====

    /**
     * @brief Register a keyboard shortcut.
     * @param hookId Unique identifier for this hook
     * @param shortcutId Unique ID, e.g., "myaddon.toggle_panel"
     * @param displayName Human-readable name, e.g., "My Addon: Toggle Panel"
     * @param defaultBinding Default key binding, e.g., "Ctrl+Shift+M"
     * @param callback Function called when shortcut is triggered
     * @param userData User data passed to callback
     */
    void (*RegisterShortcut)(
        HookId hookId,
        const char* shortcutId,
        const char* displayName,
        const char* defaultBinding,
        ShortcutCallback callback,
        void* userData
    );

    /** @brief Unregister a keyboard shortcut. */
    void (*UnregisterShortcut)(HookId hookId, const char* shortcutId);

    // ===== Batch 6: Property Drawer System =====

    /**
     * @brief Register a custom drawer for a property type.
     * @param hookId Unique identifier for this hook
     * @param propertyTypeName Type name to match (e.g., "glm::vec3", "Asset*")
     * @param drawFunc Function called to draw the property
     * @param userData User data passed to drawFunc
     */
    void (*RegisterPropertyDrawer)(HookId hookId, const char* propertyTypeName, PropertyDrawCallback drawFunc, void* userData);

    /** @brief Unregister a custom property drawer. */
    void (*UnregisterPropertyDrawer)(HookId hookId, const char* propertyTypeName);

    // ===== Batch 7: Hierarchy & Asset Browser Extensions =====

    /**
     * @brief Register a hierarchy item GUI overlay callback.
     * Called for each visible node in the hierarchy tree.
     */
    void (*RegisterHierarchyItemGUI)(HookId hookId, HierarchyItemGUICallback drawFunc, void* userData);

    /** @brief Unregister a hierarchy item GUI overlay. */
    void (*UnregisterHierarchyItemGUI)(HookId hookId);

    /**
     * @brief Register an asset browser item GUI overlay callback.
     * Called for each visible asset in the asset browser.
     */
    void (*RegisterAssetItemGUI)(HookId hookId, AssetItemGUICallback drawFunc, void* userData);

    /** @brief Unregister an asset browser item GUI overlay. */
    void (*UnregisterAssetItemGUI)(HookId hookId);

    /**
     * @brief Register a hierarchy changed event callback.
     * changeType: 0=NodeCreated, 1=NodeDestroyed, 2=NodeReparented, 3=NodeRenamed
     */
    void (*RegisterOnHierarchyChanged)(HookId hookId, HierarchyChangedCallback cb, void* userData);

    // ===== Batch 8: Additional Context Menus =====

    /** @brief Add item to scene tab context menu (right-click on scene tabs). */
    void (*AddSceneTabContextItem)(HookId hookId, const char* itemPath, MenuCallback callback, void* userData);

    /** @brief Add item to debug log context menu. */
    void (*AddDebugLogContextItem)(HookId hookId, const char* itemPath, MenuCallback callback, void* userData);

    /** @brief Add item to the import menu. */
    void (*AddImportMenuItem)(HookId hookId, const char* itemPath, MenuCallback callback, void* userData);

    /** @brief Add item to the addons menu. */
    void (*AddAddonsMenuItem)(HookId hookId, const char* itemPath, MenuCallback callback, void* userData);

    /**
     * @brief Add a custom play target to the play dropdown.
     * @param hookId Unique identifier for this hook
     * @param targetName Display name for the target
     * @param iconText Icon text for the target button
     * @param callback Function called when this target is selected and play is pressed
     * @param userData User data passed to callback
     */
    void (*AddPlayTarget)(HookId hookId, const char* targetName, const char* iconText, PlayTargetCallback callback, void* userData);

    /** @brief Remove a custom play target. */
    void (*RemovePlayTarget)(HookId hookId, const char* targetName);

    // ===== Batch 9: Drag-Drop Enhancement & Asset Pipeline =====

    /**
     * @brief Register a custom drag-drop handler.
     * @param hookId Unique identifier for this hook
     * @param targetArea Target area: "Viewport", "Hierarchy", "AssetBrowser", "Inspector"
     * @param handler Function called when a drop occurs
     * @param userData User data passed to handler
     */
    void (*RegisterDragDropHandler)(HookId hookId, const char* targetArea, DragDropHandlerCallback handler, void* userData);

    /**
     * @brief Register a custom asset importer for a file extension.
     * @param hookId Unique identifier for this hook
     * @param extension File extension to handle (e.g., ".fbx")
     * @param importFunc Function called to import the file
     * @param userData User data passed to importFunc
     */
    void (*RegisterAssetImporter)(HookId hookId, const char* extension, AssetImportCallback importFunc, void* userData);

    /** @brief Unregister a custom asset importer. */
    void (*UnregisterAssetImporter)(HookId hookId, const char* extension);

    /** @brief Register a pre-asset-import hook. Return false from callback to cancel. */
    void (*RegisterOnPreAssetImport)(HookId hookId, PreImportCallback cb, void* userData);

    /** @brief Register a post-asset-import hook. */
    void (*RegisterOnPostAssetImport)(HookId hookId, StringEventCallback cb, void* userData);

    // ===== Batch 10: Build Pipeline & Editor State =====

    /** @brief Register a pre-build hook. Return false from callback to cancel. */
    void (*RegisterOnPreBuild)(HookId hookId, PreBuildCallback cb, void* userData);

    /** @brief Register a post-build hook. */
    void (*RegisterOnPostBuild)(HookId hookId, PackageFinishedCallback cb, void* userData);

    /** @brief Register callback for editor mode changes (Scene/2D/3D/Paint). */
    void (*RegisterOnEditorModeChanged)(HookId hookId, EditorModeCallback cb, void* userData);

    /**
     * @brief Register a custom gizmo tool (adds to Translate/Rotate/Scale toolbar).
     * @param hookId Unique identifier for this hook
     * @param toolName Unique name for the tool
     * @param iconText Icon character for toolbar button
     * @param tooltip Tooltip text
     * @param drawFunc Function called when tool is active and a node is selected
     * @param userData User data passed to drawFunc
     */
    void (*RegisterGizmoTool)(
        HookId hookId,
        const char* toolName,
        const char* iconText,
        const char* tooltip,
        GizmoToolDrawCallback drawFunc,
        void* userData
    );

    /** @brief Unregister a custom gizmo tool. */
    void (*UnregisterGizmoTool)(HookId hookId, const char* toolName);

    // ===== Cleanup =====

    /**
     * @brief Remove ALL hooks registered by this hookId.
     *
     * Call this in OnUnload to ensure proper cleanup.
     *
     * @param hookId Hook identifier to remove all hooks for
     */
    void (*RemoveAllHooks)(HookId hookId);
};

/**
 * @brief Generate a HookId from a string identifier.
 *
 * Use the addon ID or Lua script UUID as the identifier
 * to ensure hooks can be properly tracked and cleaned up.
 *
 * @param identifier Unique string identifier
 * @return HookId for use with hook functions
 */
HookId GenerateHookId(const char* identifier);

#endif // EDITOR
