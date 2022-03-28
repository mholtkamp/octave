#include "LuaBindings/CameraComponent_Lua.h"
#include "LuaBindings/TransformComponent_Lua.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

int CameraComponent_Lua::SetPerspective(lua_State* L)
{
    CameraComponent* comp = CHECK_CAMERA_COMPONENT(L, 1);
    float fovY = CHECK_NUMBER(L, 2);
    float aspectRatio = CHECK_NUMBER(L, 3);
    float zNear = CHECK_NUMBER(L, 4);
    float zFar = CHECK_NUMBER(L, 5);

    comp->SetPerspectiveSettings(fovY, aspectRatio, zNear, zFar);
    comp->SetProjectionMode(ProjectionMode::PERSPECTIVE);

    return 0;
}

int CameraComponent_Lua::SetOrtho(lua_State* L)
{
    CameraComponent* comp = CHECK_CAMERA_COMPONENT(L, 1);
    float width = CHECK_NUMBER(L, 2);
    float height = CHECK_NUMBER(L, 3);
    float zNear = CHECK_NUMBER(L, 4);
    float zFar = CHECK_NUMBER(L, 5);

    comp->SetOrthoSettings(width, height, zNear, zFar);
    comp->SetProjectionMode(ProjectionMode::ORTHOGRAPHIC);

    return 0;
}

int CameraComponent_Lua::IsPerspective(lua_State* L)
{
    CameraComponent* comp = CHECK_CAMERA_COMPONENT(L, 1);

    bool ret = comp->GetProjectionMode() == ProjectionMode::PERSPECTIVE;

    lua_pushboolean(L, ret);
    return 1;
}

int CameraComponent_Lua::IsOrtho(lua_State* L)
{
    CameraComponent* comp = CHECK_CAMERA_COMPONENT(L, 1);

    bool ret = comp->GetProjectionMode() == ProjectionMode::ORTHOGRAPHIC;

    lua_pushboolean(L, ret);
    return 1;
}

int CameraComponent_Lua::GetNear(lua_State* L)
{
    CameraComponent* comp = CHECK_CAMERA_COMPONENT(L, 1);

    float ret = comp->GetNearZ();

    lua_pushnumber(L, ret);
    return 1;
}

int CameraComponent_Lua::GetFar(lua_State* L)
{
    CameraComponent* comp = CHECK_CAMERA_COMPONENT(L, 1);

    float ret = comp->GetFarZ();

    lua_pushnumber(L, ret);
    return 1;
}

int CameraComponent_Lua::GetFieldOfView(lua_State* L)
{
    CameraComponent* comp = CHECK_CAMERA_COMPONENT(L, 1);

    float ret = comp->GetPerspectiveSettings().mFovY;

    lua_pushnumber(L, ret);
    return 1;
}

int CameraComponent_Lua::GetAspectRatio(lua_State* L)
{
    CameraComponent* comp = CHECK_CAMERA_COMPONENT(L, 1);

    float ret = comp->GetPerspectiveSettings().mAspectRatio;

    lua_pushnumber(L, ret);
    return 1;
}

int CameraComponent_Lua::GetWidth(lua_State* L)
{
    CameraComponent* comp = CHECK_CAMERA_COMPONENT(L, 1);

    float ret = comp->GetOrthoSettings().mWidth;

    lua_pushnumber(L, ret);
    return 1;
}

int CameraComponent_Lua::GetHeight(lua_State* L)
{
    CameraComponent* comp = CHECK_CAMERA_COMPONENT(L, 1);

    float ret = comp->GetOrthoSettings().mHeight;

    lua_pushnumber(L, ret);
    return 1;
}

int CameraComponent_Lua::SetNear(lua_State* L)
{
    CameraComponent* comp = CHECK_CAMERA_COMPONENT(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetNearZ(value);

    return 0;
}

int CameraComponent_Lua::SetFar(lua_State* L)
{
    CameraComponent* comp = CHECK_CAMERA_COMPONENT(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetFarZ(value);

    return 0;
}

int CameraComponent_Lua::SetFieldOfView(lua_State* L)
{
    CameraComponent* comp = CHECK_CAMERA_COMPONENT(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetFieldOfView(value);

    return 0;
}

int CameraComponent_Lua::SetAspectRatio(lua_State* L)
{
    CameraComponent* comp = CHECK_CAMERA_COMPONENT(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetAspectRatio(value);

    return 0;
}

int CameraComponent_Lua::SetWidth(lua_State* L)
{
    CameraComponent* comp = CHECK_CAMERA_COMPONENT(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetWidth(value);

    return 0;
}

int CameraComponent_Lua::SetHeight(lua_State* L)
{
    CameraComponent* comp = CHECK_CAMERA_COMPONENT(L, 1);
    float value = CHECK_NUMBER(L, 2);

    comp->SetHeight(value);

    return 0;
}

void CameraComponent_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        CAMERA_COMPONENT_LUA_NAME,
        CAMERA_COMPONENT_LUA_FLAG,
        TRANSFORM_COMPONENT_LUA_NAME);

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

    lua_pop(L, 1);
    assert(lua_gettop(L) == 0);
}

#endif
