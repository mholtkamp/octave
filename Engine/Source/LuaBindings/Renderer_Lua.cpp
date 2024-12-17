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
    if (!lua_isnone(L, 1)) { screenIndex = CHECK_INDEX(L, 1); }

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

    GetWorld(0)->AddLine(line);

    return 0;
}

int Renderer_Lua::Enable3dRendering(lua_State* L)
{
    bool enable = CHECK_BOOLEAN(L, 1);

    Renderer::Get()->Enable3dRendering(enable);

    return 0;
}

int Renderer_Lua::Is3dRenderingEnabled(lua_State* L)
{
    bool ret = Renderer::Get()->Is3dRenderingEnabled();

    lua_pushboolean(L, ret);
    return 1;
}

int Renderer_Lua::Enable2dRendering(lua_State* L)
{
    bool enable = CHECK_BOOLEAN(L, 1);

    Renderer::Get()->Enable2dRendering(enable);

    return 0;
}

int Renderer_Lua::Is2dRenderingEnabled(lua_State* L)
{
    bool ret = Renderer::Get()->Is2dRenderingEnabled();

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

    REGISTER_TABLE_FUNC(L, tableIdx, EnableStatsOverlay);

    REGISTER_TABLE_FUNC(L, tableIdx, EnableConsole);

    REGISTER_TABLE_FUNC(L, tableIdx, SetModalWidget);

    REGISTER_TABLE_FUNC(L, tableIdx, GetModalWidget);

    REGISTER_TABLE_FUNC(L, tableIdx, IsInModalWidgetUpdate);

    REGISTER_TABLE_FUNC(L, tableIdx, DirtyAllWidgets);

    REGISTER_TABLE_FUNC(L, tableIdx, GetFrameNumber);

    REGISTER_TABLE_FUNC(L, tableIdx, GetFrameIndex);

    REGISTER_TABLE_FUNC(L, tableIdx, GetScreenIndex);

    REGISTER_TABLE_FUNC(L, tableIdx, GetScreenResolution);

    REGISTER_TABLE_FUNC(L, tableIdx, GetActiveScreenResolution);

    REGISTER_TABLE_FUNC(L, tableIdx, GetGlobalUiScale);

    REGISTER_TABLE_FUNC(L, tableIdx, SetGlobalUiScale);

    REGISTER_TABLE_FUNC(L, tableIdx, SetDebugMode);

    REGISTER_TABLE_FUNC(L, tableIdx, GetDebugMode);

    REGISTER_TABLE_FUNC(L, tableIdx, EnableProxyRendering);

    REGISTER_TABLE_FUNC(L, tableIdx, IsProxyRenderingEnabled);

    REGISTER_TABLE_FUNC(L, tableIdx, SetBoundsDebugMode);

    REGISTER_TABLE_FUNC(L, tableIdx, GetBoundsDebugMode);

    REGISTER_TABLE_FUNC(L, tableIdx, EnableFrustumCulling);

    REGISTER_TABLE_FUNC(L, tableIdx, IsFrustumCullingEnabled);

    REGISTER_TABLE_FUNC(L, tableIdx, AddDebugDraw);

    REGISTER_TABLE_FUNC(L, tableIdx, AddDebugLine);

    REGISTER_TABLE_FUNC(L, tableIdx, Enable3dRendering);

    REGISTER_TABLE_FUNC(L, tableIdx, Is3dRenderingEnabled);

    REGISTER_TABLE_FUNC(L, tableIdx, Enable2dRendering);

    REGISTER_TABLE_FUNC(L, tableIdx, Is2dRenderingEnabled);

    REGISTER_TABLE_FUNC(L, tableIdx, IsLightFadeEnabled);

    REGISTER_TABLE_FUNC(L, tableIdx, EnableLightFade);

    REGISTER_TABLE_FUNC(L, tableIdx, SetLightFadeLimit);

    REGISTER_TABLE_FUNC(L, tableIdx, GetLightFadeLimit);

    REGISTER_TABLE_FUNC(L, tableIdx, SetLightFadeSpeed);

    REGISTER_TABLE_FUNC(L, tableIdx, GetLightFadeSpeed);

    REGISTER_TABLE_FUNC(L, tableIdx, SetResolutionScale);

    REGISTER_TABLE_FUNC(L, tableIdx, GetResolutionScale);

    lua_setglobal(L, RENDERER_LUA_NAME);

    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
