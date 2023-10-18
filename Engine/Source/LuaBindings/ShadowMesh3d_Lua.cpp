#include "LuaBindings/ShadowMesh3d_Lua.h"
#include "LuaBindings/StaticMesh3d_Lua.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

void ShadowMesh3D_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        SHADOW_MESH_COMPONENT_LUA_NAME,
        SHADOW_MESH_COMPONENT_LUA_FLAG,
        STATIC_MESH_COMPONENT_LUA_NAME);

    Component_Lua::BindCommon(L, mtIndex);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
