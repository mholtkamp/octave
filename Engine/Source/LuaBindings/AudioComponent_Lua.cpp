#include "LuaBindings/Audio3D_Lua.h"
#include "LuaBindings/Node3D_Lua.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/SoundWave_Lua.h"
#include "LuaBindings/LuaUtils.h"

#include "AssetManager.h"

#if LUA_ENABLED

int Audio3D_Lua::SetSoundWave(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_COMPONENT(L, 1);
    SoundWave* wave = nullptr;
    if (!lua_isnil(L, 2)) { wave = CHECK_SOUND_WAVE(L, 2); }

    comp->SetSoundWave(wave);

    return 0;
}

int Audio3D_Lua::GetSoundWave(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_COMPONENT(L, 1);

    SoundWave* ret = comp->GetSoundWave();

    Asset_Lua::Create(L, ret);
    return 1;
}

int Audio3D_Lua::SetInnerRadius(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_COMPONENT(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetInnerRadius(value);

    return 0;
}

int Audio3D_Lua::GetInnerRadius(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_COMPONENT(L, 1);

    float ret = comp->GetInnerRadius();

    lua_pushnumber(L, ret);
    return 1;
}

int Audio3D_Lua::SetOuterRadius(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_COMPONENT(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetOuterRadius(value);

    return 0;
}

int Audio3D_Lua::GetOuterRadius(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_COMPONENT(L, 1);

    float ret = comp->GetOuterRadius();

    lua_pushnumber(L, ret);
    return 1;
}

int Audio3D_Lua::SetVolume(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_COMPONENT(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetVolume(value);

    return 0;
}

int Audio3D_Lua::GetVolume(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_COMPONENT(L, 1);

    float ret = comp->GetVolume();

    lua_pushnumber(L, ret);
    return 1;
}

int Audio3D_Lua::SetPitch(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_COMPONENT(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetPitch(value);

    return 0;
}

int Audio3D_Lua::GetPitch(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_COMPONENT(L, 1);

    float ret = comp->GetPitch();

    lua_pushnumber(L, ret);
    return 1;
}

int Audio3D_Lua::SetStartOffset(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_COMPONENT(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetStartOffset(value);

    return 0;
}

int Audio3D_Lua::GetStartOffset(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_COMPONENT(L, 1);

    float ret = comp->GetStartOffset();

    lua_pushnumber(L, ret);
    return 1;
}

int Audio3D_Lua::SetPriority(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_COMPONENT(L, 1);
    int32_t value = (int32_t) CHECK_INTEGER(L, 2);

    comp->SetPriority(value);

    return 0;
}

int Audio3D_Lua::GetPriority(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_COMPONENT(L, 1);

    int32_t ret = comp->GetPriority();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int Audio3D_Lua::SetAttenuationFunc(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_COMPONENT(L, 1);
    AttenuationFunc value = (AttenuationFunc) CHECK_INTEGER(L, 2);

    comp->SetAttenuationFunc(value);

    return 0;
}

int Audio3D_Lua::GetAttenuationFunc(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_COMPONENT(L, 1);

    AttenuationFunc ret = comp->GetAttenuationFunc();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int Audio3D_Lua::SetLoop(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_COMPONENT(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    comp->SetLoop(value);

    return 0;
}

int Audio3D_Lua::GetLoop(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_COMPONENT(L, 1);

    bool ret = comp->GetLoop();

    lua_pushboolean(L, ret);
    return 1;
}

int Audio3D_Lua::SetAutoPlay(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_COMPONENT(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    comp->SetAutoPlay(value);

    return 0;
}

int Audio3D_Lua::GetAutoPlay(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_COMPONENT(L, 1);

    bool ret = comp->GetAutoPlay();

    lua_pushboolean(L, ret);
    return 1;
}

int Audio3D_Lua::GetPlayTime(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_COMPONENT(L, 1);

    float ret = comp->GetPlayTime();

    lua_pushnumber(L, ret);
    return 1;
}

int Audio3D_Lua::IsPlaying(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_COMPONENT(L, 1);

    bool ret = comp->IsPlaying();

    lua_pushboolean(L, ret);
    return 1;
}

int Audio3D_Lua::IsAudible(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_COMPONENT(L, 1);

    bool ret = comp->IsAudible();

    lua_pushboolean(L, ret);
    return 1;
}

int Audio3D_Lua::Play(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_COMPONENT(L, 1);

    comp->Play();

    return 0;
}

int Audio3D_Lua::Pause(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_COMPONENT(L, 1);

    comp->Pause();

    return 0;
}

int Audio3D_Lua::Stop(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_COMPONENT(L, 1);

    comp->Stop();

    return 0;
}

int Audio3D_Lua::Reset(lua_State* L)
{
    Audio3D* comp = CHECK_AUDIO_COMPONENT(L, 1);

    comp->Reset();

    return 0;
}

void Audio3D_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        AUDIO_COMPONENT_LUA_NAME,
        AUDIO_COMPONENT_LUA_FLAG,
        TRANSFORM_COMPONENT_LUA_NAME);

    Component_Lua::BindCommon(L, mtIndex);

    lua_pushcfunction(L, SetSoundWave);
    lua_setfield(L, mtIndex, "SetSoundWave");

    lua_pushcfunction(L, GetSoundWave);
    lua_setfield(L, mtIndex, "GetSoundWave");

    lua_pushcfunction(L, SetInnerRadius);
    lua_setfield(L, mtIndex, "SetInnerRadius");

    lua_pushcfunction(L, GetInnerRadius);
    lua_setfield(L, mtIndex, "GetInnerRadius");

    lua_pushcfunction(L, SetOuterRadius);
    lua_setfield(L, mtIndex, "SetOuterRadius");

    lua_pushcfunction(L, GetOuterRadius);
    lua_setfield(L, mtIndex, "GetOuterRadius");

    lua_pushcfunction(L, SetVolume);
    lua_setfield(L, mtIndex, "SetVolume");

    lua_pushcfunction(L, GetVolume);
    lua_setfield(L, mtIndex, "GetVolume");

    lua_pushcfunction(L, SetPitch);
    lua_setfield(L, mtIndex, "SetPitch");

    lua_pushcfunction(L, GetPitch);
    lua_setfield(L, mtIndex, "GetPitch");

    lua_pushcfunction(L, SetStartOffset);
    lua_setfield(L, mtIndex, "SetStartOffset");

    lua_pushcfunction(L, GetStartOffset);
    lua_setfield(L, mtIndex, "GetStartOffset");

    lua_pushcfunction(L, SetPriority);
    lua_setfield(L, mtIndex, "SetPriority");

    lua_pushcfunction(L, GetPriority);
    lua_setfield(L, mtIndex, "GetPriority");

    lua_pushcfunction(L, SetAttenuationFunc);
    lua_setfield(L, mtIndex, "SetAttenuationFunc");

    lua_pushcfunction(L, GetAttenuationFunc);
    lua_setfield(L, mtIndex, "GetAttenuationFunc");

    lua_pushcfunction(L, SetLoop);
    lua_setfield(L, mtIndex, "SetLoop");

    lua_pushcfunction(L, GetLoop);
    lua_setfield(L, mtIndex, "GetLoop");

    lua_pushcfunction(L, SetAutoPlay);
    lua_setfield(L, mtIndex, "SetAutoPlay");

    lua_pushcfunction(L, GetAutoPlay);
    lua_setfield(L, mtIndex, "GetAutoPlay");

    lua_pushcfunction(L, GetPlayTime);
    lua_setfield(L, mtIndex, "GetPlayTime");

    lua_pushcfunction(L, IsPlaying);
    lua_setfield(L, mtIndex, "IsPlaying");

    lua_pushcfunction(L, IsAudible);
    lua_setfield(L, mtIndex, "IsAudible");

    lua_pushcfunction(L, Play);
    lua_setfield(L, mtIndex, "Play");

    lua_pushcfunction(L, Pause);
    lua_setfield(L, mtIndex, "Pause");

    lua_pushcfunction(L, Stop);
    lua_setfield(L, mtIndex, "Stop");

    lua_pushcfunction(L, Reset);
    lua_setfield(L, mtIndex, "Reset");

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
