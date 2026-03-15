#pragma once

/**
 * @file RuntimePluginManager.h
 * @brief Lightweight plugin manager for runtime (non-editor) builds.
 *
 * This manager handles plugins that are compiled directly into the game.
 * Unlike NativeAddonManager (editor-only), this doesn't handle building
 * or hot-reloading - it just manages the plugin lifecycle and ticking.
 */

#include "OctavePluginAPI.h"
#include "OctaveEngineAPI.h"

#include <vector>
#include <string>

/**
 * @brief Runtime state for a registered plugin.
 */
struct RuntimePluginState
{
    std::string mPluginId;
    OctavePluginDesc mDesc = {};
    bool mLoaded = false;
};

/**
 * @brief Singleton manager for runtime plugins.
 *
 * Plugins register themselves during static initialization or
 * explicit registration. The engine calls TickAllPlugins each frame.
 */
class RuntimePluginManager
{
public:
    static void Create();
    static void Destroy();
    static RuntimePluginManager* Get();

    /**
     * @brief Initialize the engine API and call OnLoad for all registered plugins.
     *
     * Call this after the engine is initialized but before the main loop.
     */
    void Initialize();

    /**
     * @brief Shutdown all plugins (calls OnUnload).
     *
     * Call this before engine shutdown.
     */
    void Shutdown();

    /**
     * @brief Register a plugin descriptor.
     *
     * Can be called during static initialization or explicitly.
     *
     * @param desc Plugin descriptor
     * @param pluginId Unique identifier for the plugin
     */
    void RegisterPlugin(const OctavePluginDesc& desc, const std::string& pluginId);

    /**
     * @brief Tick all loaded plugins.
     *
     * Called once per frame from the main loop.
     *
     * @param deltaTime Time elapsed since last frame in seconds
     */
    void TickAllPlugins(float deltaTime);

    /**
     * @brief Get the engine API struct for plugins.
     */
    OctaveEngineAPI* GetEngineAPI() { return &mEngineAPI; }

    /**
     * @brief Check if a plugin is registered.
     */
    bool IsRegistered(const std::string& pluginId) const;

    /**
     * @brief Get number of registered plugins.
     */
    size_t GetPluginCount() const { return mPlugins.size(); }

private:
    static RuntimePluginManager* sInstance;
    RuntimePluginManager();
    ~RuntimePluginManager();

    void InitializeEngineAPI();

    std::vector<RuntimePluginState> mPlugins;
    OctaveEngineAPI mEngineAPI;
    bool mInitialized = false;
};

/**
 * @brief Queue a plugin descriptor for registration.
 *
 * This can be called before RuntimePluginManager::Create() - the descriptors
 * are stored in a pending queue and processed when Create() is called.
 *
 * @param getDescFunc Function that fills in OctavePluginDesc
 * @param pluginId Unique identifier for the plugin
 */
void QueuePluginRegistration(int (*getDescFunc)(OctavePluginDesc*), const char* pluginId);

/**
 * @brief Helper macro for static plugin registration.
 *
 * Usage in plugin source file:
 *   OCTAVE_REGISTER_PLUGIN(MyPlugin, GetMyPluginDesc);
 *
 * Where GetMyPluginDesc is a function that fills in OctavePluginDesc.
 */
#define OCTAVE_REGISTER_PLUGIN(pluginId, getDescFunc) \
    namespace { \
        struct PluginRegistrar_##pluginId { \
            PluginRegistrar_##pluginId() { \
                QueuePluginRegistration(getDescFunc, #pluginId); \
            } \
        }; \
        static PluginRegistrar_##pluginId sRegistrar_##pluginId; \
    }
