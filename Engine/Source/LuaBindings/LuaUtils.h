#pragma once

#include "EngineTypes.h"
#include "Nodes/Node.h"

#include "AssetManager.h"

#include "Engine.h"

#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/LuaTypeCheck.h"

#if LUA_ENABLED

inline glm::quat LuaVectorToQuat(glm::vec4 vec)
{
    glm::quat retQuat;
    retQuat.x = vec.x;
    retQuat.y = vec.y;
    retQuat.z = vec.z;
    retQuat.w = vec.w;
    return retQuat;
}

inline glm::vec4 LuaQuatToVector(glm::quat quat)
{
    glm::vec4 retVec;
    retVec.x = quat.x;
    retVec.y = quat.y;
    retVec.z = quat.z;
    retVec.w = quat.w;
    return retVec;
}

int CreateClassMetatable(
    const char* className,
    const char* classFlag,
    const char* parentClassName);

#define REGISTER_TABLE_FUNC(L, TableIdx, Func) \
    OCT_ASSERT(TableIdx >= 1); \
    lua_pushstring(L, #Func); \
    lua_pushcfunction(L, Func); \
    lua_rawset(L, TableIdx);

#define REGISTER_TABLE_FUNC_EX(L, TableIdx, Func, Name) \
    OCT_ASSERT(TableIdx >= 1); \
    lua_pushstring(L, Name); \
    lua_pushcfunction(L, Func); \
    lua_rawset(L, TableIdx);

#endif
