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

    REGISTER_TABLE_FUNC(L, mtIndex, GetVolumeMultiplier);

    REGISTER_TABLE_FUNC(L, mtIndex, SetVolumeMultiplier);

    REGISTER_TABLE_FUNC(L, mtIndex, GetPitchMultiplier);

    REGISTER_TABLE_FUNC(L, mtIndex, SetPitchMultiplier);

    REGISTER_TABLE_FUNC(L, mtIndex, GetWaveDataSize);

    REGISTER_TABLE_FUNC(L, mtIndex, GetNumChannels);

    REGISTER_TABLE_FUNC(L, mtIndex, GetBitsPerSample);

    REGISTER_TABLE_FUNC(L, mtIndex, GetSampleRate);

    REGISTER_TABLE_FUNC(L, mtIndex, GetNumSamples);

    REGISTER_TABLE_FUNC(L, mtIndex, GetByteRate);

    REGISTER_TABLE_FUNC(L, mtIndex, GetDuration);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
