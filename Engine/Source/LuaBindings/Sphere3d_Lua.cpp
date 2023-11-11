#include "LuaBindings/Sphere3d_Lua.h"
#include "LuaBindings/Primitive3d_Lua.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

int Sphere3D_Lua::GetRadius(lua_State* L)
{
    Sphere3D* comp = CHECK_SPHERE_3D(L, 1);

    float ret = comp->GetRadius();

    lua_pushnumber(L, ret);
    return 1;
}

int Sphere3D_Lua::SetRadius(lua_State* L)
{
    Sphere3D* comp = CHECK_SPHERE_3D(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetRadius(value);

    return 0;
}

void Sphere3D_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        SPHERE_3D_LUA_NAME,
        SPHERE_3D_LUA_FLAG,
        PRIMITIVE_3D_LUA_NAME);

    Node_Lua::BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, GetRadius);

    REGISTER_TABLE_FUNC(L, mtIndex, SetRadius);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
