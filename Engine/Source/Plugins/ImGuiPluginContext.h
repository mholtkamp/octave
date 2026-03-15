#pragma once

#if EDITOR

struct ImGuiContext;

/**
 * @brief ImGui context information for plugin DLLs.
 *
 * Plugins must call ImGui::SetCurrentContext() and ImGui::SetAllocatorFunctions()
 * with this data before using any ImGui functions.
 */
struct ImGuiPluginContext
{
    ImGuiContext* context;
    void* (*allocFunc)(size_t sz, void* user_data);
    void (*freeFunc)(void* ptr, void* user_data);
    void* allocUserData;
};

/**
 * @brief Get the editor's ImGui context for plugin use.
 * @param outCtx Pointer to ImGuiPluginContext to fill
 */
void GetImGuiPluginContext(ImGuiPluginContext* outCtx);

#endif
