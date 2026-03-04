#include "Constants.h"

#if LUA_ENABLED

#include "Gizmos_Lua.h"
#include "Gizmos.h"

#include "LuaBindings/LuaUtils.h"
#include "LuaBindings/LuaTypeCheck.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/StaticMesh_Lua.h"

int Gizmos_Lua::SetColor(lua_State* L)
{
    glm::vec4 color = CHECK_VECTOR(L, 1);
    Gizmos::SetColor(color);
    return 0;
}

int Gizmos_Lua::GetColor(lua_State* L)
{
    glm::vec4 color = Gizmos::GetColor();
    Vector_Lua::Create(L, color);
    return 1;
}

int Gizmos_Lua::SetMatrix(lua_State* L)
{
    // Accept a Node3D and use its transform, or identity if nil
    if (lua_isnil(L, 1))
    {
        Gizmos::SetMatrix(glm::mat4(1.0f));
    }
    else
    {
        // For now, accept 16 numbers as a flat matrix
        glm::mat4 mat(1.0f);
        for (int i = 0; i < 16; ++i)
        {
            if (!lua_isnone(L, i + 1))
            {
                float val = (float)lua_tonumber(L, i + 1);
                glm::value_ptr(mat)[i] = val;
            }
        }
        Gizmos::SetMatrix(mat);
    }
    return 0;
}

int Gizmos_Lua::GetMatrix(lua_State* L)
{
    const glm::mat4& mat = Gizmos::GetMatrix();
    const float* ptr = glm::value_ptr(mat);
    for (int i = 0; i < 16; ++i)
    {
        lua_pushnumber(L, ptr[i]);
    }
    return 16;
}

int Gizmos_Lua::ResetState(lua_State* L)
{
    Gizmos::ResetState();
    return 0;
}

int Gizmos_Lua::DrawCube(lua_State* L)
{
    glm::vec4 centerV = CHECK_VECTOR(L, 1);
    glm::vec4 sizeV = CHECK_VECTOR(L, 2);
    Gizmos::DrawCube(glm::vec3(centerV), glm::vec3(sizeV));
    return 0;
}

int Gizmos_Lua::DrawSphere(lua_State* L)
{
    glm::vec4 centerV = CHECK_VECTOR(L, 1);
    float radius = CHECK_NUMBER(L, 2);
    Gizmos::DrawSphere(glm::vec3(centerV), radius);
    return 0;
}

int Gizmos_Lua::DrawMesh(lua_State* L)
{
    StaticMesh* mesh = CHECK_STATIC_MESH(L, 1);
    glm::vec4 posV = CHECK_VECTOR(L, 2);
    glm::vec3 rot = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    if (!lua_isnone(L, 3)) { glm::vec4 rv = CHECK_VECTOR(L, 3); rot = glm::vec3(rv); }
    if (!lua_isnone(L, 4)) { glm::vec4 sv = CHECK_VECTOR(L, 4); scale = glm::vec3(sv); }
    Gizmos::DrawMesh(mesh, glm::vec3(posV), rot, scale);
    return 0;
}

int Gizmos_Lua::DrawWireCube(lua_State* L)
{
    glm::vec4 centerV = CHECK_VECTOR(L, 1);
    glm::vec4 sizeV = CHECK_VECTOR(L, 2);
    Gizmos::DrawWireCube(glm::vec3(centerV), glm::vec3(sizeV));
    return 0;
}

int Gizmos_Lua::DrawWireSphere(lua_State* L)
{
    glm::vec4 centerV = CHECK_VECTOR(L, 1);
    float radius = CHECK_NUMBER(L, 2);
    Gizmos::DrawWireSphere(glm::vec3(centerV), radius);
    return 0;
}

int Gizmos_Lua::DrawWireMesh(lua_State* L)
{
    StaticMesh* mesh = CHECK_STATIC_MESH(L, 1);
    glm::vec4 posV = CHECK_VECTOR(L, 2);
    glm::vec3 rot = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    if (!lua_isnone(L, 3)) { glm::vec4 rv = CHECK_VECTOR(L, 3); rot = glm::vec3(rv); }
    if (!lua_isnone(L, 4)) { glm::vec4 sv = CHECK_VECTOR(L, 4); scale = glm::vec3(sv); }
    Gizmos::DrawWireMesh(mesh, glm::vec3(posV), rot, scale);
    return 0;
}

int Gizmos_Lua::DrawLine(lua_State* L)
{
    glm::vec4 fromV = CHECK_VECTOR(L, 1);
    glm::vec4 toV = CHECK_VECTOR(L, 2);
    Gizmos::DrawLine(glm::vec3(fromV), glm::vec3(toV));
    return 0;
}

int Gizmos_Lua::DrawRay(lua_State* L)
{
    glm::vec4 originV = CHECK_VECTOR(L, 1);
    glm::vec4 directionV = CHECK_VECTOR(L, 2);
    Gizmos::DrawRay(glm::vec3(originV), glm::vec3(directionV));
    return 0;
}

int Gizmos_Lua::DrawFrustum(lua_State* L)
{
    // Accept 16 numbers as a flat view-projection matrix
    glm::mat4 mat(1.0f);
    for (int i = 0; i < 16; ++i)
    {
        float val = CHECK_NUMBER(L, i + 1);
        glm::value_ptr(mat)[i] = val;
    }
    Gizmos::DrawFrustum(mat);
    return 0;
}

void Gizmos_Lua::Bind()
{
    lua_State* L = GetLua();

    lua_newtable(L);
    int tableIdx = lua_gettop(L);

    REGISTER_TABLE_FUNC(L, tableIdx, SetColor);
    REGISTER_TABLE_FUNC(L, tableIdx, GetColor);
    REGISTER_TABLE_FUNC(L, tableIdx, SetMatrix);
    REGISTER_TABLE_FUNC(L, tableIdx, GetMatrix);
    REGISTER_TABLE_FUNC(L, tableIdx, ResetState);

    REGISTER_TABLE_FUNC(L, tableIdx, DrawCube);
    REGISTER_TABLE_FUNC(L, tableIdx, DrawSphere);
    REGISTER_TABLE_FUNC(L, tableIdx, DrawMesh);
    REGISTER_TABLE_FUNC(L, tableIdx, DrawWireCube);
    REGISTER_TABLE_FUNC(L, tableIdx, DrawWireSphere);
    REGISTER_TABLE_FUNC(L, tableIdx, DrawWireMesh);

    REGISTER_TABLE_FUNC(L, tableIdx, DrawLine);
    REGISTER_TABLE_FUNC(L, tableIdx, DrawRay);
    REGISTER_TABLE_FUNC(L, tableIdx, DrawFrustum);

    lua_setglobal(L, GIZMOS_LUA_NAME);

    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
