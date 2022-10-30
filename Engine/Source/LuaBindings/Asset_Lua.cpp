#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/LuaUtils.h"

int Asset_Lua::Create(lua_State* L, const Asset* asset, bool allowNull)
{
    if (asset != nullptr || allowNull)
    {
        Asset_Lua* assetLua = (Asset_Lua*)lua_newuserdata(L, sizeof(Asset_Lua));
        new (assetLua) Asset_Lua(); // This should call asset ref constructor I think. Needed to zero refcount.
        assetLua->mAsset = asset;

        int udIndex = lua_gettop(L);

        luaL_getmetatable(L, asset ? asset->GetClassName() : ASSET_LUA_NAME);
        if (lua_isnil(L, -1))
        {
            // Could not find this type's metatable, so just use Component
            lua_pop(L, 1);
            luaL_getmetatable(L, ASSET_LUA_NAME);
        }

        OCT_ASSERT(lua_istable(L, -1));
        lua_setmetatable(L, udIndex);
    }
    else
    {
        lua_pushnil(L);
    }

    return 1;
}

int Asset_Lua::Destroy(lua_State* L)
{
    CHECK_ASSET_USERDATA(L, 1);
    Asset_Lua* assetLua = (Asset_Lua*)lua_touserdata(L, 1);
    assetLua->~Asset_Lua(); // Should decement refcount
    return 0;
}

int Asset_Lua::GetName(lua_State* L)
{
    Asset* asset = CHECK_ASSET(L, 1);

    const std::string& name = asset->GetName();

    lua_pushstring(L, name.c_str());
    return 1;
}

int Asset_Lua::IsRefCounted(lua_State* L)
{
    Asset* asset = CHECK_ASSET(L, 1);

    bool ret = asset->IsRefCounted();

    lua_pushboolean(L, ret);
    return 1;
}

int Asset_Lua::GetRefCount(lua_State* L)
{
    Asset* asset = CHECK_ASSET(L, 1);

    int ret = asset->GetRefCount();

    lua_pushinteger(L, ret);
    return 1;
}

int Asset_Lua::GetTypeName(lua_State* L)
{
    Asset* asset = CHECK_ASSET(L, 1);

    const char* className = asset->GetClassName();

    lua_pushstring(L, className);
    return 1;
}

int Asset_Lua::IsTransient(lua_State* L)
{
    Asset* asset = CHECK_ASSET(L, 1);

    bool ret = asset->IsTransient();

    lua_pushboolean(L, ret);
    return 1;
}

int Asset_Lua::IsLoaded(lua_State* L)
{
    // This is really only useful for checking async loads I think.
    // After calling AsyncLoad() in script, make sure you call this func
    // before using the returned asset.
    // Maybe in the future we can add safety checks in all the Asset functions
    // to ensure that mAsset != nullptr.
    Asset* asset = CHECK_ASSET(L, 1);

    bool ret = asset && asset->IsLoaded();

    lua_pushboolean(L, ret);
    return 1;
}

void Asset_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        ASSET_LUA_NAME,
        ASSET_LUA_FLAG,
        nullptr);

    lua_pushcfunction(L, Destroy);
    lua_setfield(L, mtIndex, "__gc");

    lua_pushcfunction(L, GetName);
    lua_setfield(L, mtIndex, "GetName");

    lua_pushcfunction(L, IsRefCounted);
    lua_setfield(L, mtIndex, "IsRefCounted");

    lua_pushcfunction(L, GetRefCount);
    lua_setfield(L, mtIndex, "GetRefCount");

    lua_pushcfunction(L, GetTypeName);
    lua_setfield(L, mtIndex, "GetTypeName");

    lua_pushcfunction(L, IsTransient);
    lua_setfield(L, mtIndex, "IsTransient");

    lua_pushcfunction(L, IsLoaded);
    lua_setfield(L, mtIndex, "IsLoaded");

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}
