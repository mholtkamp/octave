#pragma once

#include "EngineTypes.h"
#include "AssetManager.h"

#include "LuaBindings/Asset_Lua.h"

class Node;

#if LUA_ENABLED

template<typename T>
T* CheckLuaType(lua_State* L, int arg, const char* typeName, bool throwError = true)
{
    return throwError ?
        (T*)luaL_checkudata(L, arg, typeName) :
        (T*)luaL_testudata(L, arg, typeName);
}

template<typename T>
T* CheckHierarchyLuaType(lua_State* L, int arg, const char* typeName, const char* classFlag, bool throwError = true)
{
    luaL_checktype(L, arg, LUA_TUSERDATA);
    T* ret = (T*)lua_touserdata(L, arg);

    if (ret != nullptr)
    {
        // Check that the userdata class inherits from the type T
        bool hasClassFlag = (lua_getfield(L, arg, classFlag) != LUA_TNIL);
        lua_pop(L, 1);

        if (!hasClassFlag)
        {
            ret = nullptr;

            if (throwError)
            {
                luaL_error(L, "Error: Arg #%d: Expected %s", arg, typeName);
            }
        }
    }

    return ret;
}

template<typename T>
T* CheckAssetLuaType(lua_State* L, int arg, const char* className, const char* classFlag, bool throwError = true)
{
    T* ret = nullptr;
    if (lua_isstring(L, arg))
    {
        const char* name = lua_tostring(L, arg);
        ret = LoadAsset<T>(name);
    }
    else
    {
        Asset_Lua* assetLua = CheckHierarchyLuaType<Asset_Lua>(L, arg, className, classFlag, throwError);
        ret = static_cast<T*>(assetLua ? assetLua->mAsset.Get() : nullptr);
    }

    return ret;
}

NodePtr& CheckNodeWrapperPtr(lua_State* L, int arg);
Node* CheckNodeWrapper(lua_State* L, int arg);
Node* CheckNodeLuaType(lua_State* L, int arg, const char* className, const char* classFlag);
Object* CheckObjectLuaType(lua_State* L, int arg);

Asset* CheckAssetOrNilLuaType(lua_State* L, int arg, const char* className, const char* classFlag);
Node* CheckNodeOrNilLuaType(lua_State* L, int arg, const char* className, const char* classFlag);
Object* CheckObjectOrNilLuaType(lua_State* L, int arg);


const char* CheckTableName(lua_State* L, int arg);

#define CHECK_STRING(L, Arg) lua_tostring(L, Arg); luaL_checktype(L, Arg, LUA_TSTRING);
#define CHECK_NUMBER(L, Arg) lua_tonumber(L, Arg); luaL_checktype(L, Arg, LUA_TNUMBER);
#define CHECK_INTEGER(L, Arg) lua_tointeger(L, Arg); luaL_checktype(L, Arg, LUA_TNUMBER);
#define CHECK_BOOLEAN(L, Arg) lua_toboolean(L, Arg); luaL_checktype(L, Arg, LUA_TBOOLEAN);
#define CHECK_TABLE(L, Arg) luaL_checktype(L, Arg, LUA_TTABLE);
#define CHECK_FUNCTION(L, Arg) luaL_checktype(L, Arg, LUA_TFUNCTION);
#define CHECK_USERDATA(L, Arg) luaL_checktype(L, Arg, LUA_TUSERDATA);
#define CHECK_NIL(L, Arg) luaL_checktype(L, Arg, LUA_TNIL);

#define CHECK_INDEX(L, Arg) (lua_tointeger(L, Arg) - 1); luaL_checktype(L, Arg, LUA_TNUMBER);

bool CheckClassFlag(lua_State* L, int arg, const char* flag);

#endif