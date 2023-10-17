#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Assets/SoundWave.h"

#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define SOUND_WAVE_LUA_NAME "SoundWave"
#define SOUND_WAVE_LUA_FLAG "cfSoundWave"
#define CHECK_SOUND_WAVE(L, arg) CheckAssetLuaType<SoundWave>(L, arg, SOUND_WAVE_LUA_NAME, SOUND_WAVE_LUA_FLAG)

struct SoundWave_Lua
{
    static int GetVolumeMultiplier(lua_State* L);
    static int SetVolumeMultiplier(lua_State* L);
    static int GetPitchMultiplier(lua_State* L);
    static int SetPitchMultiplier(lua_State* L);
    static int GetWaveDataSize(lua_State* L);
    static int GetNumChannels(lua_State* L);
    static int GetBitsPerSample(lua_State* L);
    static int GetSampleRate(lua_State* L);
    static int GetNumSamples(lua_State* L);
    static int GetByteRate(lua_State* L);
    static int GetDuration(lua_State* L);

    static void Bind();
};

#endif
