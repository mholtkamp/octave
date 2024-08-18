#include "LuaBindings/Camera3d_Lua.h"
#include "LuaBindings/Node3d_Lua.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/LuaUtils.h"

#include "Nodes/3D/Primitive3d.h"

#if LUA_ENABLED

int Camera3D_Lua::EnablePerspective(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_3D(L, 1);
    bool persp = CHECK_BOOLEAN(L, 2);

    comp->EnablePerspective(persp);

    return 0;
}

int Camera3D_Lua::IsPerspectiveEnabled(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_3D(L, 1);

    bool ret = comp->GetProjectionMode() == ProjectionMode::PERSPECTIVE;

    lua_pushboolean(L, ret);
    return 1;
}

int Camera3D_Lua::IsOrtho(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_3D(L, 1);

    bool ret = comp->GetProjectionMode() == ProjectionMode::ORTHOGRAPHIC;

    lua_pushboolean(L, ret);
    return 1;
}

int Camera3D_Lua::GetNear(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_3D(L, 1);

    float ret = comp->GetNearZ();

    lua_pushnumber(L, ret);
    return 1;
}

int Camera3D_Lua::GetFar(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_3D(L, 1);

    float ret = comp->GetFarZ();

    lua_pushnumber(L, ret);
    return 1;
}

int Camera3D_Lua::GetFieldOfView(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_3D(L, 1);

    float ret = comp->GetFieldOfViewY();

    lua_pushnumber(L, ret);
    return 1;
}

int Camera3D_Lua::GetAspectRatio(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_3D(L, 1);

    float ret = comp->GetAspectRatio();

    lua_pushnumber(L, ret);
    return 1;
}

int Camera3D_Lua::GetOrthoWidth(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_3D(L, 1);

    float ret = comp->GetOrthoWidth();

    lua_pushnumber(L, ret);
    return 1;
}

int Camera3D_Lua::GetOrthoHeight(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_3D(L, 1);

    float ret = comp->GetOrthoHeight();

    lua_pushnumber(L, ret);
    return 1;
}

int Camera3D_Lua::SetNear(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_3D(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetNearZ(value);

    return 0;
}

int Camera3D_Lua::SetFar(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_3D(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetFarZ(value);

    return 0;
}

int Camera3D_Lua::SetFieldOfView(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_3D(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetFieldOfView(value);

    return 0;
}

int Camera3D_Lua::SetOrthoWidth(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_3D(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetOrthoWidth(value);

    return 0;
}

int Camera3D_Lua::WorldToScreenPosition(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_3D(L, 1);
    glm::vec3 worldPos = CHECK_VECTOR(L, 2);

    glm::vec3 ret = comp->WorldToScreenPosition(worldPos);

    Vector_Lua::Create(L, ret);
    return 1;
}

int Camera3D_Lua::ScreenToWorldPosition(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_3D(L, 1);
    int32_t x = CHECK_INTEGER(L, 2);
    int32_t y = CHECK_INTEGER(L, 3);

    glm::vec3 ret = comp->ScreenToWorldPosition(x, y);

    Vector_Lua::Create(L, ret);
    return 1;
}

int Camera3D_Lua::TraceScreenToWorld(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_3D(L, 1);
    int32_t x = CHECK_INTEGER(L, 2);
    int32_t y = CHECK_INTEGER(L, 3);
    uint8_t colMask = 0xff;
    if (!lua_isnone(L, 4)) { colMask = (uint8_t)CHECK_INTEGER(L, 4); }

    RayTestResult rayResult;
    glm::vec3 worldPos = comp->TraceScreenToWorld(x, y, colMask, rayResult);

    // TODO: Might want to return the ray test result as a table instead

    Vector_Lua::Create(L, worldPos);
    Node_Lua::Create(L, rayResult.mHitNode);
    return 2;
}

void Camera3D_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        CAMERA_3D_LUA_NAME,
        CAMERA_3D_LUA_FLAG,
        NODE_3D_LUA_NAME);

    Node_Lua::BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, EnablePerspective);

    REGISTER_TABLE_FUNC(L, mtIndex, IsPerspectiveEnabled);

    REGISTER_TABLE_FUNC(L, mtIndex, IsOrtho);

    REGISTER_TABLE_FUNC(L, mtIndex, GetNear);

    REGISTER_TABLE_FUNC(L, mtIndex, GetFar);

    REGISTER_TABLE_FUNC(L, mtIndex, GetFieldOfView);

    REGISTER_TABLE_FUNC(L, mtIndex, GetAspectRatio);

    REGISTER_TABLE_FUNC(L, mtIndex, GetOrthoWidth);

    REGISTER_TABLE_FUNC(L, mtIndex, GetOrthoHeight);

    REGISTER_TABLE_FUNC(L, mtIndex, SetNear);

    REGISTER_TABLE_FUNC(L, mtIndex, SetFar);

    REGISTER_TABLE_FUNC(L, mtIndex, SetFieldOfView);

    REGISTER_TABLE_FUNC(L, mtIndex, SetOrthoWidth);

    REGISTER_TABLE_FUNC(L, mtIndex, WorldToScreenPosition);

    REGISTER_TABLE_FUNC(L, mtIndex, ScreenToWorldPosition);

    REGISTER_TABLE_FUNC(L, mtIndex, TraceScreenToWorld);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
