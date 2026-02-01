#include "LuaBindings/Spline3d_Lua.h"
#include "LuaBindings/Node3d_Lua.h"
#include "LuaBindings/Vector_Lua.h"

#if LUA_ENABLED

int Spline3D_Lua::AddPoint(lua_State* L)
{
    Spline3D* spline = CHECK_SPLINE_3D(L, 1);
    glm::vec3 p = CHECK_VECTOR(L, 2);
    spline->AddPoint(p);
    return 0;
}

int Spline3D_Lua::ClearPoints(lua_State* L)
{
    Spline3D* spline = CHECK_SPLINE_3D(L, 1);
    spline->ClearPoints();
    return 0;
}

int Spline3D_Lua::GetPointCount(lua_State* L)
{
    Spline3D* spline = CHECK_SPLINE_3D(L, 1);
    lua_pushinteger(L, (int)spline->GetPointCount());
    return 1;
}

int Spline3D_Lua::GetPoint(lua_State* L)
{
    Spline3D* spline = CHECK_SPLINE_3D(L, 1);
    int32_t index = CHECK_INTEGER(L, 2);
    index--;
    glm::vec3 p = spline->GetPoint(index);
    Vector_Lua::Create(L, p);
    return 1;
}

int Spline3D_Lua::SetPoint(lua_State* L)
{
    Spline3D* spline = CHECK_SPLINE_3D(L, 1);
    int32_t index = CHECK_INTEGER(L, 2);
    index--;
    glm::vec3 p = CHECK_VECTOR(L, 3);
    spline->SetPoint(index, p);
    return 0;
}

int Spline3D_Lua::GetPositionAt(lua_State* L)
{
    Spline3D* spline = CHECK_SPLINE_3D(L, 1);
    float t = CHECK_NUMBER(L, 2);
    glm::vec3 p = spline->GetPositionAt(t);
    Vector_Lua::Create(L, p);
    return 1;
}

int Spline3D_Lua::GetTangentAt(lua_State* L)
{
    Spline3D* spline = CHECK_SPLINE_3D(L, 1);
    float t = CHECK_NUMBER(L, 2);
    glm::vec3 p = spline->GetTangentAt(t);
    Vector_Lua::Create(L, p);
    return 1;
}

int Spline3D_Lua::Play(lua_State* L)
{
    Spline3D* spline = CHECK_SPLINE_3D(L, 1);
    spline->Play();
    return 0;
}

int Spline3D_Lua::Stop(lua_State* L)
{
    Spline3D* spline = CHECK_SPLINE_3D(L, 1);
    spline->StopPlayback();
    return 0;
}

void Spline3D_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        SPLINE_3D_LUA_NAME,
        SPLINE_3D_LUA_FLAG,
        NODE_3D_LUA_NAME);

    Node_Lua::BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, AddPoint);
    REGISTER_TABLE_FUNC(L, mtIndex, ClearPoints);
    REGISTER_TABLE_FUNC(L, mtIndex, GetPointCount);
    REGISTER_TABLE_FUNC(L, mtIndex, GetPoint);
    REGISTER_TABLE_FUNC(L, mtIndex, SetPoint);
    REGISTER_TABLE_FUNC(L, mtIndex, GetPositionAt);
    REGISTER_TABLE_FUNC(L, mtIndex, GetTangentAt);
    REGISTER_TABLE_FUNC(L, mtIndex, Play);
    REGISTER_TABLE_FUNC(L, mtIndex, Stop);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
