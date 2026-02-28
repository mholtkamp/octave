#pragma once

struct lua_State;

struct UIDocument_Lua
{
    // UI module functions
    static int Load(lua_State* L);
    static int LoadFromString(lua_State* L);

    // UIDocument instance methods
    static int Mount(lua_State* L);
    static int Unmount(lua_State* L);
    static int Instantiate(lua_State* L);
    static int FindById(lua_State* L);
    static int FindByClass(lua_State* L);
    static int GetRootWidget(lua_State* L);
    static int SetData(lua_State* L);
    static int SetCallback(lua_State* L);
    static int Tick(lua_State* L);
    static int Destroy(lua_State* L);

    static void Bind();
};
