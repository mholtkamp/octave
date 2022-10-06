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
    assert(lua_istable(L, -1));
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
    assert(lua_gettop(L) == 0);

    luaL_newmetatable(L, STREAM_LUA_NAME);
    int mtIndex = lua_gettop(L);

    lua_pushcfunction(L, Create);
    lua_setfield(L, mtIndex, "Create");

    lua_pushcfunction(L, Destroy);
    lua_setfield(L, mtIndex, "__gc");

    lua_pushcfunction(L, GetSize);
    lua_setfield(L, mtIndex, "GetSize");

    lua_pushcfunction(L, GetPos);
    lua_setfield(L, mtIndex, "GetPos");

    lua_pushcfunction(L, SetPos);
    lua_setfield(L, mtIndex, "SetPos");

    lua_pushcfunction(L, ReadFile);
    lua_setfield(L, mtIndex, "ReadFile");

    lua_pushcfunction(L, WriteFile);
    lua_setfield(L, mtIndex, "WriteFile");

    lua_pushcfunction(L, ReadAsset);
    lua_setfield(L, mtIndex, "ReadAsset");

    lua_pushcfunction(L, WriteAsset);
    lua_setfield(L, mtIndex, "WriteAsset");

    lua_pushcfunction(L, ReadString);
    lua_setfield(L, mtIndex, "ReadString");

    lua_pushcfunction(L, WriteString);
    lua_setfield(L, mtIndex, "WriteString");

    lua_pushcfunction(L, ReadInt32);
    lua_setfield(L, mtIndex, "ReadInt32");

    lua_pushcfunction(L, ReadInt16);
    lua_setfield(L, mtIndex, "ReadInt16");

    lua_pushcfunction(L, ReadInt8);
    lua_setfield(L, mtIndex, "ReadInt8");

    lua_pushcfunction(L, ReadFloat);
    lua_setfield(L, mtIndex, "ReadFloat");

    lua_pushcfunction(L, ReadBool);
    lua_setfield(L, mtIndex, "ReadBool");

    lua_pushcfunction(L, ReadVec2);
    lua_setfield(L, mtIndex, "ReadVec2");

    lua_pushcfunction(L, ReadVec3);
    lua_setfield(L, mtIndex, "ReadVec3");

    lua_pushcfunction(L, ReadVec4);
    lua_setfield(L, mtIndex, "ReadVec4");

    lua_pushcfunction(L, WriteInt32);
    lua_setfield(L, mtIndex, "WriteInt32");

    lua_pushcfunction(L, WriteInt16);
    lua_setfield(L, mtIndex, "WriteInt16");

    lua_pushcfunction(L, WriteInt8);
    lua_setfield(L, mtIndex, "WriteInt8");

    lua_pushcfunction(L, WriteFloat);
    lua_setfield(L, mtIndex, "WriteFloat");

    lua_pushcfunction(L, WriteBool);
    lua_setfield(L, mtIndex, "WriteBool");

    lua_pushcfunction(L, WriteVec2);
    lua_setfield(L, mtIndex, "WriteVec2");

    lua_pushcfunction(L, WriteVec3);
    lua_setfield(L, mtIndex, "WriteVec3");

    lua_pushcfunction(L, WriteVec4);
    lua_setfield(L, mtIndex, "WriteVec4");

    // Set the __index metamethod to itself
    lua_pushvalue(L, mtIndex);
    lua_setfield(L, mtIndex, "__index");

    lua_setglobal(L, STREAM_LUA_NAME);

    assert(lua_gettop(L) == 0);
}

#endif
