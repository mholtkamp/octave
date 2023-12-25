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

int Asset_Lua::Equals(lua_State* L)
{
    Asset* assetA = CHECK_ASSET(L, 1);
    Asset* assetB = nullptr;

    if (lua_isuserdata(L, 2))
    {
        assetB = CHECK_ASSET(L, 2);
    }

    bool ret = (assetA == assetB);

    lua_pushboolean(L, ret);
    return 1;
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

void Asset_Lua::BindCommon(lua_State* L, int mtIndex)
{
    REGISTER_TABLE_FUNC_EX(L, mtIndex, Destroy, "__gc");

    REGISTER_TABLE_FUNC_EX(L, mtIndex, Equals, "__eq");
}

void Asset_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        ASSET_LUA_NAME,
        ASSET_LUA_FLAG,
        nullptr);

    BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, Equals);

    REGISTER_TABLE_FUNC(L, mtIndex, GetName);

    REGISTER_TABLE_FUNC(L, mtIndex, IsRefCounted);

    REGISTER_TABLE_FUNC(L, mtIndex, GetRefCount);

    REGISTER_TABLE_FUNC(L, mtIndex, GetTypeName);

    REGISTER_TABLE_FUNC(L, mtIndex, IsTransient);

    REGISTER_TABLE_FUNC(L, mtIndex, IsLoaded);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}
