#include "LuaBindings/Camera3d_Lua.h"
#include "LuaBindings/Node3d_Lua.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/LuaUtils.h"

#include "Nodes/3D/Primitive3d.h"

#if LUA_ENABLED

int Camera3D_Lua::SetPerspective(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_3D(L, 1);
    float fovY = CHECK_NUMBER(L, 2);
    float aspectRatio = CHECK_NUMBER(L, 3);
    float zNear = CHECK_NUMBER(L, 4);
    float zFar = CHECK_NUMBER(L, 5);

    comp->SetPerspectiveSettings(fovY, aspectRatio, zNear, zFar);
    comp->SetProjectionMode(ProjectionMode::PERSPECTIVE);

    return 0;
}

int Camera3D_Lua::SetOrtho(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_3D(L, 1);
    float width = CHECK_NUMBER(L, 2);
    float height = CHECK_NUMBER(L, 3);
    float zNear = CHECK_NUMBER(L, 4);
    float zFar = CHECK_NUMBER(L, 5);

    comp->SetOrthoSettings(width, height, zNear, zFar);
    comp->SetProjectionMode(ProjectionMode::ORTHOGRAPHIC);

    return 0;
}

int Camera3D_Lua::IsPerspective(lua_State* L)
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

    float ret = comp->GetPerspectiveSettings().mFovY;

    lua_pushnumber(L, ret);
    return 1;
}

int Camera3D_Lua::GetAspectRatio(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_3D(L, 1);

    float ret = comp->GetPerspectiveSettings().mAspectRatio;

    lua_pushnumber(L, ret);
    return 1;
}

int Camera3D_Lua::GetWidth(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_3D(L, 1);

    float ret = comp->GetOrthoSettings().mWidth;

    lua_pushnumber(L, ret);
    return 1;
}

int Camera3D_Lua::GetHeight(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_3D(L, 1);

    float ret = comp->GetOrthoSettings().mHeight;

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

int Camera3D_Lua::SetAspectRatio(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_3D(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetAspectRatio(value);

    return 0;
}

int Camera3D_Lua::SetWidth(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_3D(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetWidth(value);

    return 0;
}

int Camera3D_Lua::SetHeight(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_3D(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetHeight(value);

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

    Primitive3D* hitComp = nullptr;
    glm::vec3 worldPos = comp->TraceScreenToWorld(x, y, colMask, &hitComp);

    Vector_Lua::Create(L, worldPos);
    Node_Lua::Create(L, hitComp);
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

    REGISTER_TABLE_FUNC(L, mtIndex, SetPerspective);

    REGISTER_TABLE_FUNC(L, mtIndex, SetOrtho);

    REGISTER_TABLE_FUNC(L, mtIndex, IsPerspective);

    REGISTER_TABLE_FUNC(L, mtIndex, IsOrtho);

    REGISTER_TABLE_FUNC(L, mtIndex, GetNear);

    REGISTER_TABLE_FUNC(L, mtIndex, GetFar);

    REGISTER_TABLE_FUNC(L, mtIndex, GetFieldOfView);

    REGISTER_TABLE_FUNC(L, mtIndex, GetAspectRatio);

    REGISTER_TABLE_FUNC(L, mtIndex, GetWidth);

    REGISTER_TABLE_FUNC(L, mtIndex, GetHeight);

    REGISTER_TABLE_FUNC(L, mtIndex, SetNear);

    REGISTER_TABLE_FUNC(L, mtIndex, SetFar);

    REGISTER_TABLE_FUNC(L, mtIndex, SetFieldOfView);

    REGISTER_TABLE_FUNC(L, mtIndex, SetAspectRatio);

    REGISTER_TABLE_FUNC(L, mtIndex, SetWidth);

    REGISTER_TABLE_FUNC(L, mtIndex, SetHeight);

    REGISTER_TABLE_FUNC(L, mtIndex, WorldToScreenPosition);

    REGISTER_TABLE_FUNC(L, mtIndex, ScreenToWorldPosition);

    REGISTER_TABLE_FUNC(L, mtIndex, TraceScreenToWorld);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
