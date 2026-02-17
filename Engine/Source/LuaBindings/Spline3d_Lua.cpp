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

int Spline3D_Lua::SetPaused(lua_State* L)
{
    Spline3D* spline = CHECK_SPLINE_3D(L, 1);
    bool paused = CHECK_BOOLEAN(L, 2);
    spline->SetPaused(paused);
    return 0;
}

int Spline3D_Lua::IsPaused(lua_State* L)
{
    Spline3D* spline = CHECK_SPLINE_3D(L, 1);
    lua_pushboolean(L, spline->IsPaused());
    return 1;
}

int Spline3D_Lua::SetFollowLinkEnabled(lua_State* L)
{
    Spline3D* spline = CHECK_SPLINE_3D(L, 1);
    int32_t index = CHECK_INTEGER(L, 2);
    bool enabled = CHECK_BOOLEAN(L, 3);

    if (index < 1 || index > 64)
    {
        return luaL_error(L, "SetFollowLinkEnabled index must be in range [1, 64]");
    }

    spline->SetFollowLinkEnabled((uint32_t)index, enabled);
    return 0;
}

int Spline3D_Lua::IsFollowLinkEnabled(lua_State* L)
{
    Spline3D* spline = CHECK_SPLINE_3D(L, 1);
    int32_t index = CHECK_INTEGER(L, 2);

    if (index < 1 || index > 64)
    {
        return luaL_error(L, "IsFollowLinkEnabled index must be in range [1, 64]");
    }

    lua_pushboolean(L, spline->IsFollowLinkEnabled((uint32_t)index));
    return 1;
}

int Spline3D_Lua::IsNearLinkFrom(lua_State* L)
{
    Spline3D* spline = CHECK_SPLINE_3D(L, 1);
    int32_t index = CHECK_INTEGER(L, 2);
    float epsilon = 0.05f;
    if (!lua_isnone(L, 3))
    {
        epsilon = CHECK_NUMBER(L, 3);
    }

    if (index < 1 || index > 64)
    {
        return luaL_error(L, "IsNearLinkFrom index must be in range [1, 64]");
    }

    lua_pushboolean(L, spline->IsNearLinkFrom((uint32_t)index, epsilon));
    return 1;
}

int Spline3D_Lua::IsNearLinkTo(lua_State* L)
{
    Spline3D* spline = CHECK_SPLINE_3D(L, 1);
    int32_t index = CHECK_INTEGER(L, 2);
    float epsilon = 0.05f;
    if (!lua_isnone(L, 3))
    {
        epsilon = CHECK_NUMBER(L, 3);
    }

    if (index < 1 || index > 64)
    {
        return luaL_error(L, "IsNearLinkTo index must be in range [1, 64]");
    }

    lua_pushboolean(L, spline->IsNearLinkTo((uint32_t)index, epsilon));
    return 1;
}

int Spline3D_Lua::IsLinkDirectionForward(lua_State* L)
{
    Spline3D* spline = CHECK_SPLINE_3D(L, 1);
    int32_t index = CHECK_INTEGER(L, 2);
    float threshold = 0.0f;
    if (!lua_isnone(L, 3))
    {
        threshold = CHECK_NUMBER(L, 3);
    }

    if (index < 1 || index > 64)
    {
        return luaL_error(L, "IsLinkDirectionForward index must be in range [1, 64]");
    }

    lua_pushboolean(L, spline->IsLinkDirectionForward((uint32_t)index, threshold));
    return 1;
}

int Spline3D_Lua::TriggerLink(lua_State* L)
{
    Spline3D* spline = CHECK_SPLINE_3D(L, 1);
    int32_t index = CHECK_INTEGER(L, 2);

    if (index < 1 || index > 64)
    {
        return luaL_error(L, "TriggerLink index must be in range [1, 64]");
    }

    lua_pushboolean(L, spline->TriggerLink((uint32_t)index));
    return 1;
}

int Spline3D_Lua::CancelActiveLink(lua_State* L)
{
    Spline3D* spline = CHECK_SPLINE_3D(L, 1);
    spline->CancelActiveLink();
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
    REGISTER_TABLE_FUNC(L, mtIndex, SetPaused);
    REGISTER_TABLE_FUNC(L, mtIndex, IsPaused);
    REGISTER_TABLE_FUNC(L, mtIndex, SetFollowLinkEnabled);
    REGISTER_TABLE_FUNC(L, mtIndex, IsFollowLinkEnabled);
    REGISTER_TABLE_FUNC(L, mtIndex, IsNearLinkFrom);
    REGISTER_TABLE_FUNC(L, mtIndex, IsNearLinkTo);
    REGISTER_TABLE_FUNC(L, mtIndex, IsLinkDirectionForward);
    REGISTER_TABLE_FUNC(L, mtIndex, TriggerLink);
    REGISTER_TABLE_FUNC(L, mtIndex, CancelActiveLink);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif



