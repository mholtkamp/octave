#include "LuaBindings/Scene_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Node_Lua.h"

#include "LuaBindings/Vector_Lua.h"

#if LUA_ENABLED

int Scene_Lua::Capture(lua_State* L)
{
    Scene* scene = CHECK_SCENE(L, 1);
    Node* root = CHECK_NODE(L, 2);

    scene->Capture(root);

    return 0;
}

int Scene_Lua::Instantiate(lua_State* L)
{
    Scene* scene = CHECK_SCENE(L, 1);

    Node* ret = scene->Instantiate();

    Node_Lua::Create(L, ret);
    return 1;
}

void Scene_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        SCENE_LUA_NAME,
        SCENE_LUA_FLAG,
        ASSET_LUA_NAME);

    Asset_Lua::BindCommon(L, mtIndex);

    lua_pushcfunction(L, Capture);
    lua_setfield(L, mtIndex, "Capture");

    lua_pushcfunction(L, Instantiate);
    lua_setfield(L, mtIndex, "Instantiate");

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
