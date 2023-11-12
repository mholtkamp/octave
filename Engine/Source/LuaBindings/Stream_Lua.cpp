#include "LuaBindings/Stream_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Vector_Lua.h"
#include "Log.h"

#if LUA_ENABLED

int Stream_Lua::Create(lua_State* L)
{
    Stream_Lua* newObject = (Stream_Lua*)lua_newuserdata(L, sizeof(Stream_Lua));
    new (newObject) Stream_Lua();
    luaL_getmetatable(L, STREAM_LUA_NAME);
    OCT_ASSERT(lua_istable(L, -1));
    lua_setmetatable(L, -2);

    return 1;
}

int Stream_Lua::Destroy(lua_State* L)
{
    CHECK_STREAM(L, 1);
    Stream_Lua* obj = (Stream_Lua*)lua_touserdata(L, 1);
    obj->~Stream_Lua();
    return 0;
}


int Stream_Lua::GetSize(lua_State* L)
{
    Stream& stream = CHECK_STREAM(L, 1);

    int32_t ret = stream.GetSize();

    lua_pushinteger(L, ret);
    return 1;
}

int Stream_Lua::GetPos(lua_State* L)
{
    Stream& stream = CHECK_STREAM(L, 1);

    int32_t ret = stream.GetPos();

    lua_pushinteger(L, ret);
    return 1;
}

int Stream_Lua::SetPos(lua_State* L)
{
    Stream& stream = CHECK_STREAM(L, 1);
    int32_t pos = CHECK_INTEGER(L, 2);

    stream.SetPos(pos);

    return 0;
}


int Stream_Lua::ReadFile(lua_State* L)
{
    Stream& stream = CHECK_STREAM(L, 1);
    const char* path = CHECK_STRING(L, 2);
    int32_t maxSize = 0; if (lua_isinteger(L, 3)) { maxSize = CHECK_INTEGER(L, 3); }

    stream.ReadFile(path, maxSize);

    return 0;
}

int Stream_Lua::WriteFile(lua_State* L)
{
    Stream& stream = CHECK_STREAM(L, 1);
    const char* path = CHECK_STRING(L, 2);

    stream.WriteFile(path);

    return 0;
}


int Stream_Lua::ReadAsset(lua_State* L)
{
    Stream& stream = CHECK_STREAM(L, 1);

    AssetRef ref;
    stream.ReadAsset(ref);

    Asset_Lua::Create(L, ref.Get());
    return 1;
}

int Stream_Lua::WriteAsset(lua_State* L)
{
    Stream& stream = CHECK_STREAM(L, 1);
    Asset* asset = CHECK_ASSET(L, 2);

    AssetRef assetRef = asset;
    stream.WriteAsset(assetRef);

    return 0;
}


int Stream_Lua::ReadString(lua_State* L)
{
    Stream& stream = CHECK_STREAM(L, 1);

    std::string str;
    stream.ReadString(str);

    lua_pushstring(L, str.c_str());
    return 1;
}

int Stream_Lua::WriteString(lua_State* L)
{
    Stream& stream = CHECK_STREAM(L, 1);
    const char* str = CHECK_STRING(L, 2);

    stream.WriteString(str);

    return 0;
}


int Stream_Lua::ReadInt32(lua_State* L)
{
    Stream& stream = CHECK_STREAM(L, 1);

    int32_t ret = stream.ReadInt32();

    lua_pushinteger(L, ret);
    return 1;
}

int Stream_Lua::ReadInt16(lua_State* L)
{
    Stream& stream = CHECK_STREAM(L, 1);

    int16_t ret = stream.ReadInt16();

    lua_pushinteger(L, ret);
    return 1;
}

int Stream_Lua::ReadInt8(lua_State* L)
{
    Stream& stream = CHECK_STREAM(L, 1);

    int8_t ret = stream.ReadInt8();

    lua_pushinteger(L, ret);
    return 1;
}

int Stream_Lua::ReadFloat(lua_State* L)
{
    Stream& stream = CHECK_STREAM(L, 1);

    float ret = stream.ReadFloat();

    lua_pushnumber(L, ret);
    return 1;
}

int Stream_Lua::ReadBool(lua_State* L)
{
    Stream& stream = CHECK_STREAM(L, 1);

    bool ret = stream.ReadBool();

    lua_pushboolean(L, ret);
    return 1;
}

int Stream_Lua::ReadVec2(lua_State* L)
{
    Stream& stream = CHECK_STREAM(L, 1);

    glm::vec2 ret = stream.ReadVec2();

    Vector_Lua::Create(L, ret);
    return 1;
}

int Stream_Lua::ReadVec3(lua_State* L)
{
    Stream& stream = CHECK_STREAM(L, 1);

    glm::vec3 ret = stream.ReadVec3();

    Vector_Lua::Create(L, ret);
    return 1;
}

int Stream_Lua::ReadVec4(lua_State* L)
{
    Stream& stream = CHECK_STREAM(L, 1);

    glm::vec4 ret = stream.ReadVec4();

    Vector_Lua::Create(L, ret);
    return 1;
}


int Stream_Lua::WriteInt32(lua_State* L)
{
    Stream& stream = CHECK_STREAM(L, 1);
    int32_t value = CHECK_INTEGER(L, 2);

    stream.WriteInt32(value);

    return 0;
}

int Stream_Lua::WriteInt16(lua_State* L)
{
    Stream& stream = CHECK_STREAM(L, 1);
    int16_t value = (int16_t)CHECK_INTEGER(L, 2);

    stream.WriteInt16(value);

    return 0;
}

int Stream_Lua::WriteInt8(lua_State* L)
{
    Stream& stream = CHECK_STREAM(L, 1);
    int8_t value = (int8_t)CHECK_INTEGER(L, 2);

    stream.WriteInt8(value);

    return 0;
}

int Stream_Lua::WriteFloat(lua_State* L)
{
    Stream& stream = CHECK_STREAM(L, 1);
    float value = CHECK_NUMBER(L, 2);

    stream.WriteFloat(value);

    return 0;
}

int Stream_Lua::WriteBool(lua_State* L)
{
    Stream& stream = CHECK_STREAM(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    stream.WriteBool(value);

    return 0;
}

int Stream_Lua::WriteVec2(lua_State* L)
{
    Stream& stream = CHECK_STREAM(L, 1);
    glm::vec2 value = CHECK_VECTOR(L, 2);

    stream.WriteVec2(value);

    return 0;
}

int Stream_Lua::WriteVec3(lua_State* L)
{
    Stream& stream = CHECK_STREAM(L, 1);
    glm::vec3 value = CHECK_VECTOR(L, 2);

    stream.WriteVec3(value);

    return 0;
}

int Stream_Lua::WriteVec4(lua_State* L)
{
    Stream& stream = CHECK_STREAM(L, 1);
    glm::vec4 value = CHECK_VECTOR(L, 2);

    stream.WriteVec4(value);

    return 0;
}


void Stream_Lua::Bind()
{
    lua_State* L = GetLua();
    OCT_ASSERT(lua_gettop(L) == 0);

    luaL_newmetatable(L, STREAM_LUA_NAME);
    int mtIndex = lua_gettop(L);

    REGISTER_TABLE_FUNC(L, mtIndex, Create);

    REGISTER_TABLE_FUNC_EX(L, mtIndex, Destroy, "__gc");

    REGISTER_TABLE_FUNC(L, mtIndex, GetSize);

    REGISTER_TABLE_FUNC(L, mtIndex, GetPos);

    REGISTER_TABLE_FUNC(L, mtIndex, SetPos);

    REGISTER_TABLE_FUNC(L, mtIndex, ReadFile);

    REGISTER_TABLE_FUNC(L, mtIndex, WriteFile);

    REGISTER_TABLE_FUNC(L, mtIndex, ReadAsset);

    REGISTER_TABLE_FUNC(L, mtIndex, WriteAsset);

    REGISTER_TABLE_FUNC(L, mtIndex, ReadString);

    REGISTER_TABLE_FUNC(L, mtIndex, WriteString);

    REGISTER_TABLE_FUNC(L, mtIndex, ReadInt32);

    REGISTER_TABLE_FUNC(L, mtIndex, ReadInt16);

    REGISTER_TABLE_FUNC(L, mtIndex, ReadInt8);

    REGISTER_TABLE_FUNC(L, mtIndex, ReadFloat);

    REGISTER_TABLE_FUNC(L, mtIndex, ReadBool);

    REGISTER_TABLE_FUNC(L, mtIndex, ReadVec2);

    REGISTER_TABLE_FUNC(L, mtIndex, ReadVec3);

    REGISTER_TABLE_FUNC(L, mtIndex, ReadVec4);

    REGISTER_TABLE_FUNC(L, mtIndex, WriteInt32);

    REGISTER_TABLE_FUNC(L, mtIndex, WriteInt16);

    REGISTER_TABLE_FUNC(L, mtIndex, WriteInt8);

    REGISTER_TABLE_FUNC(L, mtIndex, WriteFloat);

    REGISTER_TABLE_FUNC(L, mtIndex, WriteBool);

    REGISTER_TABLE_FUNC(L, mtIndex, WriteVec2);

    REGISTER_TABLE_FUNC(L, mtIndex, WriteVec3);

    REGISTER_TABLE_FUNC(L, mtIndex, WriteVec4);

    // Set the __index metamethod to itself
    lua_pushvalue(L, mtIndex);
    lua_setfield(L, mtIndex, "__index");

    lua_setglobal(L, STREAM_LUA_NAME);

    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
