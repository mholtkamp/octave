#pragma once

/**
 * @file OctavePluginAPI.h
 * @brief Stable C ABI header for native addon plugins.
 *
 * This header defines the interface that native addons must implement
 * to be loaded by the Octave engine/editor.
 */

#include <stdint.h>

// Platform-specific export/import macros
#ifdef _WIN32
    #ifdef OCTAVE_PLUGIN_EXPORT
        #define OCTAVE_PLUGIN_API __declspec(dllexport)
    #else
        #define OCTAVE_PLUGIN_API __declspec(dllimport)
    #endif
#else
    #define OCTAVE_PLUGIN_API __attribute__((visibility("default")))
#endif

// Plugin API version - increment when breaking changes are made
#define OCTAVE_PLUGIN_API_VERSION 3

// Forward declarations
struct OctaveEngineAPI;
struct EditorUIHooks;
struct lua_State;

/**
 * @brief Plugin descriptor returned by OctavePlugin_GetDesc.
 *
 * Contains plugin metadata and callback function pointers.
 * All function pointers may be nullptr if not implemented.
 */
struct OctavePluginDesc
{
    // Plugin metadata
    uint32_t apiVersion;        // Must match OCTAVE_PLUGIN_API_VERSION
    const char* pluginName;     // Human-readable plugin name
    const char* pluginVersion;  // Plugin version string (e.g., "1.0.0")

    // Lifecycle callbacks
    int (*OnLoad)(OctaveEngineAPI* api);    // Called when plugin is loaded, return 0 on success
    void (*OnUnload)();                      // Called before plugin is unloaded

    // Tick callbacks (set to nullptr if not needed)
    void (*Tick)(float deltaTime);           // Called during gameplay (PIE or built game)
    void (*TickEditor)(float deltaTime);     // Called in editor regardless of play state (editor builds only)

    // Registration callbacks (called after OnLoad)
    void (*RegisterTypes)(void* nodeFactory);           // Register custom node types
    void (*RegisterScriptFuncs)(struct lua_State* L);   // Register Lua bindings

    // Editor UI extension (editor builds only, set to nullptr for game-only plugins)
    void (*RegisterEditorUI)(EditorUIHooks* hooks, uint64_t hookId);

    /**
     * @brief Called before the editor ImGui context is fully initialized.
     *
     * Use this to configure ImGui settings, add custom fonts, or set
     * configuration flags before the UI is created. The ImGui context
     * exists but has not been fully configured yet.
     *
     * Set to nullptr if not needed.
     */
    void (*OnEditorPreInit)();

    /**
     * @brief Called after the editor is fully initialized, before the main loop starts.
     *
     * At this point all editor systems are ready: ImGui is configured,
     * the project is loaded (if any), and the UI is about to be shown.
     * Safe to query project state, open windows, etc.
     *
     * Set to nullptr if not needed.
     */
    void (*OnEditorReady)();
};

/**
 * @brief Plugin entry point function signature.
 *
 * Every plugin must export a function named "OctavePlugin_GetDesc"
 * with this signature.
 *
 * @param outDesc Pointer to descriptor struct to fill in
 * @return 0 on success, non-zero on failure
 */
typedef int (*OctavePlugin_GetDescFunc)(OctavePluginDesc* outDesc);

// Macro for declaring the plugin entry point
#define OCTAVE_PLUGIN_ENTRY() \
    extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* outDesc)
