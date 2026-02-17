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

    // ===== Undo/Redo =====

    /** @brief Register callback for when an undo or redo operation is performed. */
    void (*RegisterOnUndoRedo)(HookId hookId, EventCallback cb, void* userData);

    // ===== Drag-and-Drop Events =====

    /** @brief Register callback for when an asset is dropped onto the scene hierarchy. Receives asset name. */
    void (*RegisterOnAssetDropHierarchy)(HookId hookId, StringEventCallback cb, void* userData);

    /** @brief Register callback for when an asset is dropped onto the viewport. Receives asset name. */
    void (*RegisterOnAssetDropViewport)(HookId hookId, StringEventCallback cb, void* userData);

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
