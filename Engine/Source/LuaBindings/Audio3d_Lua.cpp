#include "LuaBindings/Audio3d_Lua.h"
#include "LuaBindings/Node3d_Lua.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/SoundWave_Lua.h"
#include "LuaBindings/LuaUtils.h"

#include "AssetManager.h"

#if LUA_ENABLED

int Audio3D_Lua::SetSoundWave(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_3D(L, 1);
    SoundWave* wave = nullptr;
    if (!lua_isnil(L, 2)) { wave = CHECK_SOUND_WAVE(L, 2); }

    comp->SetSoundWave(wave);

    return 0;
}

int Audio3D_Lua::GetSoundWave(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_3D(L, 1);

    SoundWave* ret = comp->GetSoundWave();

    Asset_Lua::Create(L, ret);
    return 1;
}

int Audio3D_Lua::SetInnerRadius(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_3D(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetInnerRadius(value);

    return 0;
}

int Audio3D_Lua::GetInnerRadius(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_3D(L, 1);

    float ret = comp->GetInnerRadius();

    lua_pushnumber(L, ret);
    return 1;
}

int Audio3D_Lua::SetOuterRadius(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_3D(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetOuterRadius(value);

    return 0;
}

int Audio3D_Lua::GetOuterRadius(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_3D(L, 1);

    float ret = comp->GetOuterRadius();

    lua_pushnumber(L, ret);
    return 1;
}

int Audio3D_Lua::SetVolume(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_3D(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetVolume(value);

    return 0;
}

int Audio3D_Lua::GetVolume(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_3D(L, 1);

    float ret = comp->GetVolume();

    lua_pushnumber(L, ret);
    return 1;
}

int Audio3D_Lua::SetPitch(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_3D(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetPitch(value);

    return 0;
}

int Audio3D_Lua::GetPitch(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_3D(L, 1);

    float ret = comp->GetPitch();

    lua_pushnumber(L, ret);
    return 1;
}

int Audio3D_Lua::SetStartOffset(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_3D(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetStartOffset(value);

    return 0;
}

int Audio3D_Lua::GetStartOffset(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_3D(L, 1);

    float ret = comp->GetStartOffset();

    lua_pushnumber(L, ret);
    return 1;
}

int Audio3D_Lua::SetPriority(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_3D(L, 1);
    int32_t value = (int32_t) CHECK_INTEGER(L, 2);

    comp->SetPriority(value);

    return 0;
}

int Audio3D_Lua::GetPriority(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_3D(L, 1);

    int32_t ret = comp->GetPriority();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int Audio3D_Lua::SetAttenuationFunc(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_3D(L, 1);
    AttenuationFunc value = (AttenuationFunc) CHECK_INTEGER(L, 2);

    comp->SetAttenuationFunc(value);

    return 0;
}

int Audio3D_Lua::GetAttenuationFunc(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_3D(L, 1);

    AttenuationFunc ret = comp->GetAttenuationFunc();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int Audio3D_Lua::SetLoop(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_3D(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    comp->SetLoop(value);

    return 0;
}

int Audio3D_Lua::GetLoop(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_3D(L, 1);

    bool ret = comp->GetLoop();

    lua_pushboolean(L, ret);
    return 1;
}

int Audio3D_Lua::SetAutoPlay(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_3D(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    comp->SetAutoPlay(value);

    return 0;
}

int Audio3D_Lua::GetAutoPlay(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_3D(L, 1);

    bool ret = comp->GetAutoPlay();

    lua_pushboolean(L, ret);
    return 1;
}

int Audio3D_Lua::GetPlayTime(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_3D(L, 1);

    float ret = comp->GetPlayTime();

    lua_pushnumber(L, ret);
    return 1;
}

int Audio3D_Lua::IsPlaying(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_3D(L, 1);

    bool ret = comp->IsPlaying();

    lua_pushboolean(L, ret);
    return 1;
}

int Audio3D_Lua::IsAudible(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_3D(L, 1);

    bool ret = comp->IsAudible();

    lua_pushboolean(L, ret);
    return 1;
}

int Audio3D_Lua::PlayAudio(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_3D(L, 1);

    comp->PlayAudio();

    return 0;
}

int Audio3D_Lua::PauseAudio(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_3D(L, 1);

    comp->PauseAudio();

    return 0;
}

int Audio3D_Lua::StopAudio(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_3D(L, 1);

    comp->StopAudio();

    return 0;
}

int Audio3D_Lua::ResetAudio(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_3D(L, 1);

    comp->ResetAudio();

    return 0;
}

void Audio3D_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        AUDIO_3D_LUA_NAME,
        AUDIO_3D_LUA_FLAG,
        NODE_3D_LUA_NAME);

    Node_Lua::BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, SetSoundWave);

    REGISTER_TABLE_FUNC(L, mtIndex, GetSoundWave);

    REGISTER_TABLE_FUNC(L, mtIndex, SetInnerRadius);

    REGISTER_TABLE_FUNC(L, mtIndex, GetInnerRadius);

    REGISTER_TABLE_FUNC(L, mtIndex, SetOuterRadius);

    REGISTER_TABLE_FUNC(L, mtIndex, GetOuterRadius);

    REGISTER_TABLE_FUNC(L, mtIndex, SetVolume);

    REGISTER_TABLE_FUNC(L, mtIndex, GetVolume);

    REGISTER_TABLE_FUNC(L, mtIndex, SetPitch);

    REGISTER_TABLE_FUNC(L, mtIndex, GetPitch);

    REGISTER_TABLE_FUNC(L, mtIndex, SetStartOffset);

    REGISTER_TABLE_FUNC(L, mtIndex, GetStartOffset);

    REGISTER_TABLE_FUNC(L, mtIndex, SetPriority);

    REGISTER_TABLE_FUNC(L, mtIndex, GetPriority);

    REGISTER_TABLE_FUNC(L, mtIndex, SetAttenuationFunc);

    REGISTER_TABLE_FUNC(L, mtIndex, GetAttenuationFunc);

    REGISTER_TABLE_FUNC(L, mtIndex, SetLoop);

    REGISTER_TABLE_FUNC(L, mtIndex, GetLoop);

    REGISTER_TABLE_FUNC(L, mtIndex, SetAutoPlay);

    REGISTER_TABLE_FUNC(L, mtIndex, GetAutoPlay);

    REGISTER_TABLE_FUNC(L, mtIndex, GetPlayTime);

    REGISTER_TABLE_FUNC(L, mtIndex, IsPlaying);

    REGISTER_TABLE_FUNC(L, mtIndex, IsAudible);

    REGISTER_TABLE_FUNC(L, mtIndex, PlayAudio);

    REGISTER_TABLE_FUNC(L, mtIndex, PauseAudio);

    REGISTER_TABLE_FUNC(L, mtIndex, StopAudio);

    REGISTER_TABLE_FUNC(L, mtIndex, ResetAudio);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
