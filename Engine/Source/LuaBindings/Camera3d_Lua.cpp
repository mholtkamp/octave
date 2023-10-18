#include "LuaBindings/Camera3d_Lua.h"
#include "LuaBindings/Node3d_Lua.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/LuaUtils.h"

#include "Nodes/3D/Primitive3d.h"

#if LUA_ENABLED

int Camera3D_Lua::SetPerspective(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_COMPONENT(L, 1);
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
    Camera3D* comp = CHECK_CAMERA_COMPONENT(L, 1);
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
    Camera3D* comp = CHECK_CAMERA_COMPONENT(L, 1);

    bool ret = comp->GetProjectionMode() == ProjectionMode::PERSPECTIVE;

    lua_pushboolean(L, ret);
    return 1;
}

int Camera3D_Lua::IsOrtho(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_COMPONENT(L, 1);

    bool ret = comp->GetProjectionMode() == ProjectionMode::ORTHOGRAPHIC;

    lua_pushboolean(L, ret);
    return 1;
}

int Camera3D_Lua::GetNear(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_COMPONENT(L, 1);

    float ret = comp->GetNearZ();

    lua_pushnumber(L, ret);
    return 1;
}

int Camera3D_Lua::GetFar(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_COMPONENT(L, 1);

    float ret = comp->GetFarZ();

    lua_pushnumber(L, ret);
    return 1;
}

int Camera3D_Lua::GetFieldOfView(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_COMPONENT(L, 1);

    float ret = comp->GetPerspectiveSettings().mFovY;

    lua_pushnumber(L, ret);
    return 1;
}

int Camera3D_Lua::GetAspectRatio(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_COMPONENT(L, 1);

    float ret = comp->GetPerspectiveSettings().mAspectRatio;

    lua_pushnumber(L, ret);
    return 1;
}

int Camera3D_Lua::GetWidth(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_COMPONENT(L, 1);

    float ret = comp->GetOrthoSettings().mWidth;

    lua_pushnumber(L, ret);
    return 1;
}

int Camera3D_Lua::GetHeight(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_COMPONENT(L, 1);

    float ret = comp->GetOrthoSettings().mHeight;

    lua_pushnumber(L, ret);
    return 1;
}

int Camera3D_Lua::SetNear(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_COMPONENT(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetNearZ(value);

    return 0;
}

int Camera3D_Lua::SetFar(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_COMPONENT(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetFarZ(value);

    return 0;
}

int Camera3D_Lua::SetFieldOfView(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_COMPONENT(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetFieldOfView(value);

    return 0;
}

int Camera3D_Lua::SetAspectRatio(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_COMPONENT(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetAspectRatio(value);

    return 0;
}

int Camera3D_Lua::SetWidth(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_COMPONENT(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetWidth(value);

    return 0;
}

int Camera3D_Lua::SetHeight(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_COMPONENT(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetHeight(value);

    return 0;
}

int Camera3D_Lua::WorldToScreenPosition(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_COMPONENT(L, 1);
    glm::vec3 worldPos = CHECK_VECTOR(L, 2);

    glm::vec3 ret = comp->WorldToScreenPosition(worldPos);

    Vector_Lua::Create(L, ret);
    return 1;
}

int Camera3D_Lua::ScreenToWorldPosition(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_COMPONENT(L, 1);
    int32_t x = CHECK_INTEGER(L, 2);
    int32_t y = CHECK_INTEGER(L, 3);

    glm::vec3 ret = comp->ScreenToWorldPosition(x, y);

    Vector_Lua::Create(L, ret);
    return 1;
}

int Camera3D_Lua::TraceScreenToWorld(lua_State* L)
{
    Camera3D* comp = CHECK_CAMERA_COMPONENT(L, 1);
    int32_t x = CHECK_INTEGER(L, 2);
    int32_t y = CHECK_INTEGER(L, 3);
    uint8_t colMask = 0xff;
    if (!lua_isnone(L, 4)) { colMask = (uint8_t)CHECK_INTEGER(L, 4); }

    Primitive3D* hitComp = nullptr;
    glm::vec3 worldPos = comp->TraceScreenToWorld(x, y, colMask, &hitComp);

    Vector_Lua::Create(L, worldPos);
    Component_Lua::Create(L, hitComp);
    return 2;
}

void Camera3D_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        CAMERA_COMPONENT_LUA_NAME,
        CAMERA_COMPONENT_LUA_FLAG,
        TRANSFORM_COMPONENT_LUA_NAME);

    Component_Lua::BindCommon(L, mtIndex);

    lua_pushcfunction(L, SetPerspective);
    lua_setfield(L, mtIndex, "SetPerspective");

    lua_pushcfunction(L, SetOrtho);
    lua_setfield(L, mtIndex, "SetOrtho");

    lua_pushcfunction(L, IsPerspective);
    lua_setfield(L, mtIndex, "IsPerspective");

    lua_pushcfunction(L, IsOrtho);
    lua_setfield(L, mtIndex, "IsOrtho");

    lua_pushcfunction(L, GetNear);
    lua_setfield(L, mtIndex, "GetNear");

    lua_pushcfunction(L, GetFar);
    lua_setfield(L, mtIndex, "GetFar");

    lua_pushcfunction(L, GetFieldOfView);
    lua_setfield(L, mtIndex, "GetFieldOfView");

    lua_pushcfunction(L, GetAspectRatio);
    lua_setfield(L, mtIndex, "GetAspectRatio");

    lua_pushcfunction(L, GetWidth);
    lua_setfield(L, mtIndex, "GetWidth");

    lua_pushcfunction(L, GetHeight);
    lua_setfield(L, mtIndex, "GetHeight");

    lua_pushcfunction(L, SetNear);
    lua_setfield(L, mtIndex, "SetNear");

    lua_pushcfunction(L, SetFar);
    lua_setfield(L, mtIndex, "SetFar");

    lua_pushcfunction(L, SetFieldOfView);
    lua_setfield(L, mtIndex, "SetFieldOfView");

    lua_pushcfunction(L, SetAspectRatio);
    lua_setfield(L, mtIndex, "SetAspectRatio");

    lua_pushcfunction(L, SetWidth);
    lua_setfield(L, mtIndex, "SetWidth");

    lua_pushcfunction(L, SetHeight);
    lua_setfield(L, mtIndex, "SetHeight");

    lua_pushcfunction(L, WorldToScreenPosition);
    lua_setfield(L, mtIndex, "WorldToScreenPosition");

    lua_pushcfunction(L, ScreenToWorldPosition);
    lua_setfield(L, mtIndex, "ScreenToWorldPosition");

    lua_pushcfunction(L, TraceScreenToWorld);
    lua_setfield(L, mtIndex, "TraceScreenToWorld");

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
