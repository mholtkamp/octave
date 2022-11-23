#include "LuaBindings/SoundWave_Lua.h"
#include "LuaBindings/Asset_Lua.h"

#include "LuaBindings/Vector_Lua.h"

#if LUA_ENABLED

int SoundWave_Lua::GetVolumeMultiplier(lua_State* L)
{
    SoundWave* wave = CHECK_SOUND_WAVE(L, 1);

    float ret = wave->GetVolumeMultiplier();

    lua_pushnumber(L, ret);
    return 1;
}

int SoundWave_Lua::SetVolumeMultiplier(lua_State* L)
{
    SoundWave* wave = CHECK_SOUND_WAVE(L, 1);
    float value = CHECK_NUMBER(L, 2);

    wave->SetVolumeMultiplier(value);

    return 0;
}

int SoundWave_Lua::GetPitchMultiplier(lua_State* L)
{
    SoundWave* wave = CHECK_SOUND_WAVE(L, 1);

    float ret = wave->GetPitchMultiplier();

    lua_pushnumber(L, ret);
    return 1;
}

int SoundWave_Lua::SetPitchMultiplier(lua_State* L)
{
    SoundWave* wave = CHECK_SOUND_WAVE(L, 1);
    float value = CHECK_NUMBER(L, 2);

    wave->SetPitchMultiplier(value);

    return 0;
}

int SoundWave_Lua::GetWaveDataSize(lua_State* L)
{
    SoundWave* wave = CHECK_SOUND_WAVE(L, 1);

    uint32_t ret = wave->GetWaveDataSize();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int SoundWave_Lua::GetNumChannels(lua_State* L)
{
    SoundWave* wave = CHECK_SOUND_WAVE(L, 1);

    uint32_t ret = wave->GetNumChannels();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int SoundWave_Lua::GetBitsPerSample(lua_State* L)
{
    SoundWave* wave = CHECK_SOUND_WAVE(L, 1);

    uint32_t ret = wave->GetBitsPerSample();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int SoundWave_Lua::GetSampleRate(lua_State* L)
{
    SoundWave* wave = CHECK_SOUND_WAVE(L, 1);

    uint32_t ret = wave->GetSampleRate();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int SoundWave_Lua::GetNumSamples(lua_State* L)
{
    SoundWave* wave = CHECK_SOUND_WAVE(L, 1);

    uint32_t ret = wave->GetNumSamples();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int SoundWave_Lua::GetByteRate(lua_State* L)
{
    SoundWave* wave = CHECK_SOUND_WAVE(L, 1);

    uint32_t ret = wave->GetByteRate();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int SoundWave_Lua::GetDuration(lua_State* L)
{
    SoundWave* wave = CHECK_SOUND_WAVE(L, 1);

    float ret = wave->GetDuration();

    lua_pushnumber(L, ret);
    return 1;
}

void SoundWave_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        SOUND_WAVE_LUA_NAME,
        SOUND_WAVE_LUA_FLAG,
        ASSET_LUA_NAME);

    Asset_Lua::BindCommon(L, mtIndex);

    lua_pushcfunction(L, GetVolumeMultiplier);
    lua_setfield(L, mtIndex, "GetVolumeMultiplier");

    lua_pushcfunction(L, SetVolumeMultiplier);
    lua_setfield(L, mtIndex, "SetVolumeMultiplier");

    lua_pushcfunction(L, GetPitchMultiplier);
    lua_setfield(L, mtIndex, "GetPitchMultiplier");

    lua_pushcfunction(L, SetPitchMultiplier);
    lua_setfield(L, mtIndex, "SetPitchMultiplier");

    lua_pushcfunction(L, GetWaveDataSize);
    lua_setfield(L, mtIndex, "GetWaveDataSize");

    lua_pushcfunction(L, GetNumChannels);
    lua_setfield(L, mtIndex, "GetNumChannels");

    lua_pushcfunction(L, GetBitsPerSample);
    lua_setfield(L, mtIndex, "GetBitsPerSample");

    lua_pushcfunction(L, GetSampleRate);
    lua_setfield(L, mtIndex, "GetSampleRate");

    lua_pushcfunction(L, GetNumSamples);
    lua_setfield(L, mtIndex, "GetNumSamples");

    lua_pushcfunction(L, GetByteRate);
    lua_setfield(L, mtIndex, "GetByteRate");

    lua_pushcfunction(L, GetDuration);
    lua_setfield(L, mtIndex, "GetDuration");

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
