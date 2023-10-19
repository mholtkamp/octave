#pragma once

#include "EngineTypes.h"
#include "Log.h"
#include "Engine.h"

#include "Nodes/3D/Audio3d.h"

#include "LuaBindings/Node_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define AUDIO_3D_LUA_NAME "Audio3D"
#define AUDIO_3D_LUA_FLAG "cfAudio3D"
#define CHECK_AUDIO_3D(L, arg) static_cast<Audio3D*>(CheckNodeLuaType(L, arg, AUDIO_3D_LUA_NAME, AUDIO_3D_LUA_FLAG));

struct Audio3D_Lua
{
    static int SetSoundWave(lua_State* L);
    static int GetSoundWave(lua_State* L);
    static int SetInnerRadius(lua_State* L);
    static int GetInnerRadius(lua_State* L);
    static int SetOuterRadius(lua_State* L);
    static int GetOuterRadius(lua_State* L);
    static int SetVolume(lua_State* L);
    static int GetVolume(lua_State* L);
    static int SetPitch(lua_State* L);
    static int GetPitch(lua_State* L);
    static int SetStartOffset(lua_State* L);
    static int GetStartOffset(lua_State* L);
    static int SetPriority(lua_State* L);
    static int GetPriority(lua_State* L);
    static int SetAttenuationFunc(lua_State* L);
    static int GetAttenuationFunc(lua_State* L);
    static int SetLoop(lua_State* L);
    static int GetLoop(lua_State* L);
    static int SetAutoPlay(lua_State* L);
    static int GetAutoPlay(lua_State* L);
    static int GetPlayTime(lua_State* L);
    static int IsPlaying(lua_State* L);
    static int IsAudible(lua_State* L);
    static int PlayAudio(lua_State* L);
    static int PauseAudio(lua_State* L);
    static int StopAudio(lua_State* L);
    static int ResetAudio(lua_State* L);

    static void Bind();
};

#endif
