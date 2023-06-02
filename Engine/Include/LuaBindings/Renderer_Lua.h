#pragma once

#include "Renderer.h"
#include "EngineTypes.h"

#if LUA_ENABLED

#define RENDERER_LUA_NAME "Renderer"

struct Renderer_Lua
{
    static int AddWidget(lua_State* L);
    static int RemoveWidget(lua_State* L);
    static int RemoveAllWidgets(lua_State* L);
    static int EnableStatsOverlay(lua_State* L);
    static int SetModalWidget(lua_State* L);
    static int GetModalWidget(lua_State* L);
    static int IsInModalWidgetUpdate(lua_State* L);
    static int DirtyAllWidgets(lua_State* L);
    static int GetFrameNumber(lua_State* L);
    static int GetFrameIndex(lua_State* L);
    static int GetScreenIndex(lua_State* L);
    static int GetScreenResolution(lua_State* L);
    static int GetActiveScreenResolution(lua_State* L);
    static int GetGlobalUiScale(lua_State* L);
    static int SetGlobalUiScale(lua_State* L);
    static int SetDebugMode(lua_State* L);
    static int GetDebugMode(lua_State* L);
    static int EnableProxyRendering(lua_State* L);
    static int IsProxyRenderingEnabled(lua_State* L);
    static int SetBoundsDebugMode(lua_State* L);
    static int GetBoundsDebugMode(lua_State* L);
    static int EnableFrustumCulling(lua_State* L);
    static int IsFrustumCullingEnabled(lua_State* L);
    static int AddDebugDraw(lua_State* L);
    static int AddDebugLine(lua_State* L);
    static int EnableWorldRendering(lua_State* L);
    static int IsWorldRenderingEnabled(lua_State* L);
    static int IsLightFadeEnabled(lua_State* L);
    static int EnableLightFade(lua_State* L);
    static int SetLightFadeLimit(lua_State* L);
    static int GetLightFadeLimit(lua_State* L);

    static void Bind();
};

#endif
