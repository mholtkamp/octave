#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Nodes/Widgets/Sprite.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define SPRITE_LUA_NAME "Sprite"
#define SPRITE_LUA_FLAG "cfSprite"
#define CHECK_SPRITE(L, arg) (Sprite*)CheckNodeLuaType(L, arg, SPRITE_LUA_NAME, SPRITE_LUA_FLAG);

struct Sprite_Lua
{


    static int AddAnimation(lua_State* L);
    static int AddFrame(lua_State* L);
    static int AddEmptyFrame(lua_State* L);
    static int RemoveAnimation(lua_State* L);
    static int RemoveFrame(lua_State* L);
    static int SetAnimation(lua_State* L);
    static int SetFrame(lua_State* L);
    static int SetFPS(lua_State* L);
    static int SetPlay(lua_State* L);
    static int SetLoop(lua_State* L);
    static int SetAnimationName(lua_State* L);
    static int GetFrame(lua_State* L);
    static int GetAnimationLength(lua_State* L);
    static int GetAnimationName(lua_State* L);
    static int IsPlaying(lua_State* L);
    static int GetLoop(lua_State* L);
    static int GetFrameIndex(lua_State* L);
    static int GetNumAnimations(lua_State* L);
    static int GetAnimationIndex(lua_State* L);
    static int GetNumFrames(lua_State* L);
    static int GetFPS(lua_State* L);

    static void Bind();
};

#endif
