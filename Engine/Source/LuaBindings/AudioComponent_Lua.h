#pragma once

#include "EngineTypes.h"
#include "Log.h"
#include "Engine.h"

#include "Nodes/3D/AudioComponent.h"

#include "LuaBindings/Component_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define AUDIO_COMPONENT_LUA_NAME "Audio3D"
#define AUDIO_COMPONENT_LUA_FLAG "cfAudioComponent"
#define CHECK_AUDIO_COMPONENT(L, arg) static_cast<Audio3D*>(CheckComponentLuaType(L, arg, AUDIO_COMPONENT_LUA_NAME, AUDIO_COMPONENT_LUA_FLAG));

struct AudioComponent_Lua
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
    static int Play(lua_State* L);
    static int Pause(lua_State* L);
    static int Stop(lua_State* L);
    static int Reset(lua_State* L);

    static void Bind();
};

#endif
