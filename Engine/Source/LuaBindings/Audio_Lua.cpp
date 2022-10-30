#include "AudioManager.h"
#include "AssetManager.h"
#include "Engine.h"
#include "Asset.h"
#include "Assets/SoundWave.h"

#include "LuaBindings/LuaUtils.h"
#include "LuaBindings/Audio_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/SoundWave_Lua.h"

#if LUA_ENABLED

int Audio_Lua::PlaySound2D(lua_State* L)
{
    SoundWave* soundWave = CHECK_SOUND_WAVE(L, 1);
    float volume = 1.0f;
    float pitch = 1.0f;
    float startTime = 0.0f;
    bool loop = false;
    int32_t priority = 0;
    if (!lua_isnone(L, 2)) { volume = CHECK_NUMBER(L, 2); }
    if (!lua_isnone(L, 3)) { pitch = CHECK_NUMBER(L, 3); }
    if (!lua_isnone(L, 4)) { startTime = CHECK_NUMBER(L, 4); }
    if (!lua_isnone(L, 5)) { loop = CHECK_BOOLEAN(L, 5); }
    if (!lua_isnone(L, 6)) { priority = (int32_t) CHECK_INTEGER(L, 6); }

    if (soundWave != nullptr)
    {
        AudioManager::PlaySound2D(soundWave, volume, pitch, startTime, loop, priority);
    }

    return 0;
}

int Audio_Lua::PlaySound3D(lua_State* L)
{
    SoundWave* soundWave = CHECK_SOUND_WAVE(L, 1);
    glm::vec3 pos = CHECK_VECTOR(L, 2);
    float innerRadius = CHECK_NUMBER(L, 3);
    float outerRadius = CHECK_NUMBER(L, 4);
    AttenuationFunc attenFunc = AttenuationFunc::Linear;
    float volume = 1.0f;
    float pitch = 1.0f;
    float startTime = 0.0f;
    bool loop = false;
    int32_t priority = 0;
    if (!lua_isnone(L, 5)) { attenFunc = (AttenuationFunc) CHECK_INTEGER(L, 5); }
    if (!lua_isnone(L, 6)) { volume = CHECK_NUMBER(L, 6); }
    if (!lua_isnone(L, 7)) { pitch = CHECK_NUMBER(L, 7); }
    if (!lua_isnone(L, 8)) { startTime = CHECK_NUMBER(L, 8); }
    if (!lua_isnone(L, 9)) { loop = CHECK_BOOLEAN(L, 9); }
    if (!lua_isnone(L, 10)) { priority = (int32_t)CHECK_INTEGER(L, 10); }

    if (soundWave != nullptr)
    {
        AudioManager::PlaySound3D(
            soundWave, 
            pos,
            innerRadius,
            outerRadius,
            attenFunc,
            volume,
            pitch,
            startTime,
            loop,
            priority);
    }

    return 0;
}

int Audio_Lua::StopSounds(lua_State* L)
{
    SoundWave* soundWave = CHECK_SOUND_WAVE(L, 1);

    if (soundWave != nullptr)
    {
        AudioManager::StopSounds(soundWave);
    }

    return 0;
}

int Audio_Lua::StopAllSounds(lua_State* L)
{
    AudioManager::StopAllSounds();
    return 0;
}

void Audio_Lua::Bind()
{
    lua_State* L = GetLua();

    lua_newtable(L);
    int tableIdx = lua_gettop(L);

    lua_pushcfunction(L, PlaySound2D);
    lua_setfield(L, tableIdx, "PlaySound2D");

    lua_pushcfunction(L, PlaySound3D);
    lua_setfield(L, tableIdx, "PlaySound3D");

    lua_pushcfunction(L, StopSounds);
    lua_setfield(L, tableIdx, "StopSounds");

    lua_pushcfunction(L, StopAllSounds);
    lua_setfield(L, tableIdx, "StopAllSounds");

    lua_setglobal(L, AUDIO_LUA_NAME);

    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
