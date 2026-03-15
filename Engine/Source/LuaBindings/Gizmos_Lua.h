#pragma once

#include "Engine.h"

#if LUA_ENABLED

#define GIZMOS_LUA_NAME "Gizmos"

struct Gizmos_Lua
{
    static int SetColor(lua_State* L);
    static int GetColor(lua_State* L);
    static int SetMatrix(lua_State* L);
    static int GetMatrix(lua_State* L);
    static int ResetState(lua_State* L);

    static int DrawCube(lua_State* L);
    static int DrawSphere(lua_State* L);
    static int DrawMesh(lua_State* L);
    static int DrawWireCube(lua_State* L);
    static int DrawWireSphere(lua_State* L);
    static int DrawWireMesh(lua_State* L);

    static int DrawLine(lua_State* L);
    static int DrawRay(lua_State* L);
    static int DrawFrustum(lua_State* L);

    static void Bind();
};

#endif
