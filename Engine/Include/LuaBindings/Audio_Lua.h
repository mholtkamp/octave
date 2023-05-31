#pragma once

#include "AudioManager.h"
#include "EngineTypes.h"

#if LUA_ENABLED

#define AUDIO_LUA_NAME "Audio"

struct Audio_Lua
{
    static int PlaySound2D(lua_State* L);
    static int PlaySound3D(lua_State* L);
    static int StopSounds(lua_State* L);
    static int StopAllSounds(lua_State* L);

    static int SetAudioClassVolume(lua_State* L);
    static int GetAudioClassVolume(lua_State* L);
    static int SetAudioClassPitch(lua_State* L);
    static int GetAudioClassPitch(lua_State* L);

    static int SetMasterVolume(lua_State* L);
    static int GetMasterVolume(lua_State* L);
    static int SetMasterPitch(lua_State* L);
    static int GetMasterPitch(lua_State* L);

    static void Bind();
};

#endif
