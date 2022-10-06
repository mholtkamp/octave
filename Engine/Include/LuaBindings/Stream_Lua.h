#pragma once

#include "Engine.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define STREAM_LUA_NAME "Stream"
#define CHECK_STREAM(L, arg) CheckLuaType<Stream_Lua>(L, arg, STREAM_LUA_NAME)->mStream;

struct Stream_Lua
{
    Stream mStream;

    static int Create(lua_State* L);
    static int Destroy(lua_State* L);

    static int GetSize(lua_State* L);
    static int GetPos(lua_State* L);
    static int SetPos(lua_State* L);

    static int ReadFile(lua_State* L);
    static int WriteFile(lua_State* L);

    static int ReadAsset(lua_State* L);
    static int WriteAsset(lua_State* L);

    static int ReadString(lua_State* L);
    static int WriteString(lua_State* L);

    static int ReadInt32(lua_State* L);
    static int ReadInt16(lua_State* L);
    static int ReadInt8(lua_State* L);
    static int ReadFloat(lua_State* L);
    static int ReadBool(lua_State* L);
    static int ReadVec2(lua_State* L);
    static int ReadVec3(lua_State* L);
    static int ReadVec4(lua_State* L);

    static int WriteInt32(lua_State* L);
    static int WriteInt16(lua_State* L);
    static int WriteInt8(lua_State* L);
    static int WriteFloat(lua_State* L);
    static int WriteBool(lua_State* L);
    static int WriteVec2(lua_State* L);
    static int WriteVec3(lua_State* L);
    static int WriteVec4(lua_State* L);

    static void Bind();
};

#endif
