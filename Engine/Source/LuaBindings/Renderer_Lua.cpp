#include "Renderer.h"
#include "Engine.h"
#include "AssetManager.h"
#include "Utilities.h"

#include "LuaBindings/Renderer_Lua.h"
#include "LuaBindings/LuaUtils.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Material_Lua.h"
#include "LuaBindings/StaticMesh_Lua.h"
#include "LuaBindings/Widget_Lua.h"


#if LUA_ENABLED

int Renderer_Lua::AddWidget(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);
    int32_t arrayIndex = -1;
    int32_t screenIndex = 0;
    if (!lua_isnone(L, 2)) { arrayIndex = CHECK_INTEGER(L, 2); }
    if (!lua_isnone(L, 3)) { screenIndex = CHECK_INTEGER(L, 3); }

    Renderer::Get()->AddWidget(widget, arrayIndex, screenIndex);

    return 0;
}

int Renderer_Lua::RemoveWidget(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);
    int32_t screenIndex = 0;
    if (!lua_isnone(L, 2)) { screenIndex = CHECK_INTEGER(L, 2); }

    Renderer::Get()->RemoveWidget(widget, screenIndex);

    return 0;
}

int Renderer_Lua::RemoveAllWidgets(lua_State* L)
{
    int32_t screenIndex = -1;
    if (!lua_isnone(L, 1)) { screenIndex = CHECK_INTEGER(L, 1); }

    Renderer::Get()->RemoveAllWidgets(screenIndex);

    return 0;
}

int Renderer_Lua::EnableStatsOverlay(lua_State* L)
{
    bool value = CHECK_BOOLEAN(L, 1);

    Renderer::Get()->EnableStatsOverlay(value);

    return 0;
}

int Renderer_Lua::EnableConsole(lua_State* L)
{
    bool value = CHECK_BOOLEAN(L, 1);

    Renderer::Get()->EnableConsole(value);

    return 0;
}

int Renderer_Lua::SetModalWidget(lua_State* L)
{
    Widget* widget = CHECK_WIDGET(L, 1);

    Renderer::Get()->SetModalWidget(widget);

    return 0;
}

int Renderer_Lua::GetModalWidget(lua_State* L)
{
    Widget* modalWidget = Renderer::Get()->GetModalWidget();

    Node_Lua::Create(L, modalWidget);
    return 1;
}

int Renderer_Lua::IsInModalWidgetUpdate(lua_State* L)
{
    bool ret = Renderer::Get()->IsInModalWidgetUpdate();

    lua_pushboolean(L, ret);
    return 1;
}

int Renderer_Lua::DirtyAllWidgets(lua_State* L)
{
    Renderer::Get()->DirtyAllWidgets();

    return 0;
}

int Renderer_Lua::GetFrameNumber(lua_State* L)
{
    uint32_t ret = Renderer::Get()->GetFrameNumber();

    lua_pushinteger(L, ret);
    return 1;
}

int Renderer_Lua::GetFrameIndex(lua_State* L)
{
    uint32_t ret = Renderer::Get()->GetFrameIndex();

    lua_pushinteger(L, ret);
    return 1;
}

int Renderer_Lua::GetScreenIndex(lua_State* L)
{
    uint32_t ret = Renderer::Get()->GetScreenIndex();

    lua_pushinteger(L, ret);
    return 1;
}

int Renderer_Lua::GetScreenResolution(lua_State* L)
{
    int32_t screenIndex = -1;
    if (!lua_isnone(L, 1)) { screenIndex = CHECK_INTEGER(L, 1); }

    glm::vec2 res = Renderer::Get()->GetScreenResolution(screenIndex);

    Vector_Lua::Create(L, res);
    return 1;
}

int Renderer_Lua::GetActiveScreenResolution(lua_State* L)
{
    glm::vec2 res = Renderer::Get()->GetActiveScreenResolution();

    Vector_Lua::Create(L, res);
    return 1;
}

int Renderer_Lua::GetGlobalUiScale(lua_State* L)
{
    float uiScale = Renderer::Get()->GetGlobalUiScale();

    lua_pushnumber(L, uiScale);
    return 1;
}

int Renderer_Lua::SetGlobalUiScale(lua_State* L)
{
    float scale = CHECK_NUMBER(L, 1);

    Renderer::Get()->SetGlobalUiScale(scale);

    return 0;
}

int Renderer_Lua::SetDebugMode(lua_State* L)
{
    DebugMode debugMode = (DebugMode)CHECK_INTEGER(L, 1);

    Renderer::Get()->SetDebugMode(debugMode);

    return 0;
}

int Renderer_Lua::GetDebugMode(lua_State* L)
{
    DebugMode mode = Renderer::Get()->GetDebugMode();

    lua_pushinteger(L, mode);
    return 1;
}

int Renderer_Lua::EnableProxyRendering(lua_State* L)
{
    bool value = CHECK_BOOLEAN(L, 1);

    Renderer::Get()->EnableProxyRendering(value);

    return 0;
}

int Renderer_Lua::IsProxyRenderingEnabled(lua_State* L)
{
    bool ret = Renderer::Get()->IsProxyRenderingEnabled();

    lua_pushboolean(L, ret);
    return 1;
}

int Renderer_Lua::SetBoundsDebugMode(lua_State* L)
{
    BoundsDebugMode value = (BoundsDebugMode)CHECK_INTEGER(L, 1);

    Renderer::Get()->SetBoundsDebugMode(value);

    return 0;
}

int Renderer_Lua::GetBoundsDebugMode(lua_State* L)
{
    BoundsDebugMode ret = Renderer::Get()->GetBoundsDebugMode();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int Renderer_Lua::EnableFrustumCulling(lua_State* L)
{
    bool value = CHECK_BOOLEAN(L, 1);

    Renderer::Get()->EnableFrustumCulling(value);

    return 0;
}

int Renderer_Lua::IsFrustumCullingEnabled(lua_State* L)
{
    bool ret = Renderer::Get()->IsFrustumCullingEnabled();

    lua_pushboolean(L, ret);
    return 1;
}

int Renderer_Lua::AddDebugDraw(lua_State* L)
{
    DebugDraw draw;
    StaticMesh* mesh = CHECK_STATIC_MESH(L, 1);
    glm::vec3 pos = CHECK_VECTOR(L, 2);
    glm::vec3 rot = { 0.0f, 0.0f, 0.0f };
    glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
    glm::vec4 color = { 0.25f, 0.25f, 1.0f, 1.0f };
    float life = 0.0f;
    Material* material = nullptr;
    if (!lua_isnone(L, 3)) { rot = CHECK_VECTOR(L, 3); }
    if (!lua_isnone(L, 4)) { scale = CHECK_VECTOR(L, 4); }
    if (!lua_isnone(L, 5)) { color = CHECK_VECTOR(L, 5); }
    if (!lua_isnone(L, 6)) { life = CHECK_NUMBER(L, 6); }
    if (!lua_isnone(L, 7)) { material = CHECK_MATERIAL(L, 7); }

    ::AddDebugDraw(mesh, pos, rot, scale, color, life, material);

    return 0;
}

int Renderer_Lua::AddDebugLine(lua_State* L)
{
    glm::vec3 start = CHECK_VECTOR(L, 1);
    glm::vec3 end = CHECK_VECTOR(L, 2);
    glm::vec4 color = CHECK_VECTOR(L, 3);
    float life = CHECK_NUMBER(L, 4);

    Line line;
    line.mStart = start;
    line.mEnd = end;
    line.mColor = color;
    line.mLifetime = life;

    GetWorld()->AddLine(line);

    return 0;
}

int Renderer_Lua::EnableWorldRendering(lua_State* L)
{
    bool enable = CHECK_BOOLEAN(L, 1);

    Renderer::Get()->EnableWorldRendering(enable);

    return 0;
}

int Renderer_Lua::IsWorldRenderingEnabled(lua_State* L)
{
    bool ret = Renderer::Get()->IsWorldRenderingEnabled();

    lua_pushboolean(L, ret);
    return 1;
}

int Renderer_Lua::IsLightFadeEnabled(lua_State* L)
{
    bool ret = Renderer::Get()->IsLightFadeEnabled();

    lua_pushboolean(L, ret);
    return 1;
}

int Renderer_Lua::EnableLightFade(lua_State* L)
{
    bool value = CHECK_BOOLEAN(L, 1);

    Renderer::Get()->EnableLightFade(value);

    return 0;
}

int Renderer_Lua::SetLightFadeLimit(lua_State* L)
{
    int32_t value = CHECK_INTEGER(L, 1);

    Renderer::Get()->SetLightFadeLimit((uint32_t)value);

    return 0;
}

int Renderer_Lua::GetLightFadeLimit(lua_State* L)
{
    int32_t ret = (int32_t) Renderer::Get()->GetLightFadeLimit();

    lua_pushinteger(L, ret);
    return 1;
}

int Renderer_Lua::SetLightFadeSpeed(lua_State* L)
{
    float value = CHECK_NUMBER(L, 1);

    Renderer::Get()->SetLightFadeSpeed(value);

    return 0;
}

int Renderer_Lua::GetLightFadeSpeed(lua_State* L)
{
    float ret = Renderer::Get()->GetLightFadeSpeed();

    lua_pushnumber(L, ret);
    return 1;
}

int Renderer_Lua::SetResolutionScale(lua_State* L)
{
    float value = CHECK_NUMBER(L, 1);

    Renderer::Get()->SetResolutionScale(value);

    return 0;
}

int Renderer_Lua::GetResolutionScale(lua_State* L)
{
    float ret = Renderer::Get()->GetResolutionScale();

    lua_pushnumber(L, ret);
    return 1;
}

void Renderer_Lua::Bind()
{
    lua_State* L = GetLua();

    lua_newtable(L);
    int tableIdx = lua_gettop(L);

    lua_pushcfunction(L, AddWidget);
    lua_setfield(L, tableIdx, "AddWidget");

    lua_pushcfunction(L, RemoveWidget);
    lua_setfield(L, tableIdx, "RemoveWidget");

    lua_pushcfunction(L, RemoveAllWidgets);
    lua_setfield(L, tableIdx, "RemoveAllWidgets");

    lua_pushcfunction(L, EnableStatsOverlay);
    lua_setfield(L, tableIdx, "EnableStatsOverlay");

    lua_pushcfunction(L, EnableConsole);
    lua_setfield(L, tableIdx, "EnableConsole");

    lua_pushcfunction(L, SetModalWidget);
    lua_setfield(L, tableIdx, "SetModalWidget");

    lua_pushcfunction(L, GetModalWidget);
    lua_setfield(L, tableIdx, "GetModalWidget");

    lua_pushcfunction(L, IsInModalWidgetUpdate);
    lua_setfield(L, tableIdx, "IsInModalWidgetUpdate");

    lua_pushcfunction(L, DirtyAllWidgets);
    lua_setfield(L, tableIdx, "DirtyAllWidgets");

    lua_pushcfunction(L, GetFrameNumber);
    lua_setfield(L, tableIdx, "GetFrameNumber");

    lua_pushcfunction(L, GetFrameIndex);
    lua_setfield(L, tableIdx, "GetFrameIndex");

    lua_pushcfunction(L, GetScreenIndex);
    lua_setfield(L, tableIdx, "GetScreenIndex");

    lua_pushcfunction(L, GetScreenResolution);
    lua_setfield(L, tableIdx, "GetScreenResolution");

    lua_pushcfunction(L, GetActiveScreenResolution);
    lua_setfield(L, tableIdx, "GetActiveScreenResolution");

    lua_pushcfunction(L, GetGlobalUiScale);
    lua_setfield(L, tableIdx, "GetGlobalUiScale");

    lua_pushcfunction(L, SetGlobalUiScale);
    lua_setfield(L, tableIdx, "SetGlobalUiScale");

    lua_pushcfunction(L, SetDebugMode);
    lua_setfield(L, tableIdx, "SetDebugMode");

    lua_pushcfunction(L, GetDebugMode);
    lua_setfield(L, tableIdx, "GetDebugMode");

    lua_pushcfunction(L, EnableProxyRendering);
    lua_setfield(L, tableIdx, "EnableProxyRendering");

    lua_pushcfunction(L, IsProxyRenderingEnabled);
    lua_setfield(L, tableIdx, "IsProxyRenderingEnabled");

    lua_pushcfunction(L, SetBoundsDebugMode);
    lua_setfield(L, tableIdx, "SetBoundsDebugMode");

    lua_pushcfunction(L, GetBoundsDebugMode);
    lua_setfield(L, tableIdx, "GetBoundsDebugMode");

    lua_pushcfunction(L, EnableFrustumCulling);
    lua_setfield(L, tableIdx, "EnableFrustumCulling");

    lua_pushcfunction(L, IsFrustumCullingEnabled);
    lua_setfield(L, tableIdx, "IsFrustumCullingEnabled");

    lua_pushcfunction(L, AddDebugDraw);
    lua_setfield(L, tableIdx, "AddDebugDraw");

    lua_pushcfunction(L, AddDebugLine);
    lua_setfield(L, tableIdx, "AddDebugLine");

    lua_pushcfunction(L, EnableWorldRendering);
    lua_setfield(L, tableIdx, "EnableWorldRendering");

    lua_pushcfunction(L, IsWorldRenderingEnabled);
    lua_setfield(L, tableIdx, "IsWorldRenderingEnabled");

    lua_pushcfunction(L, IsLightFadeEnabled);
    lua_setfield(L, tableIdx, "IsLightFadeEnabled");

    lua_pushcfunction(L, EnableLightFade);
    lua_setfield(L, tableIdx, "EnableLightFade");

    lua_pushcfunction(L, SetLightFadeLimit);
    lua_setfield(L, tableIdx, "SetLightFadeLimit");

    lua_pushcfunction(L, GetLightFadeLimit);
    lua_setfield(L, tableIdx, "GetLightFadeLimit");

    lua_pushcfunction(L, SetLightFadeSpeed);
    lua_setfield(L, tableIdx, "SetLightFadeSpeed");

    lua_pushcfunction(L, GetLightFadeSpeed);
    lua_setfield(L, tableIdx, "GetLightFadeSpeed");

    lua_pushcfunction(L, SetResolutionScale);
    lua_setfield(L, tableIdx, "SetResolutionScale");

    lua_pushcfunction(L, GetResolutionScale);
    lua_setfield(L, tableIdx, "GetResolutionScale");

    lua_setglobal(L, RENDERER_LUA_NAME);

    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
