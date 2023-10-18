#include "EngineTypes.h"
#include "Log.h"
#include "Engine.h"

#include "Nodes/Node.h"

#include "LuaBindings/LuaUtils.h"
#include "LuaBindings/Component_Lua.h"
#include "LuaBindings/Node3D_Lua.h"
#include "LuaBindings/Actor_Lua.h"
#include "LuaBindings/World_Lua.h"

#if LUA_ENABLED

int Component_Lua::Create(lua_State* L, Component* component)
{
    if (component != nullptr)
    {
        Component_Lua* compLua = (Component_Lua*)lua_newuserdata(L, sizeof(Component_Lua));
        new (compLua) Component_Lua();
        compLua->mComponent = component;

        int udIndex = lua_gettop(L);

        luaL_getmetatable(L, component->GetClassName());
        if (lua_isnil(L, -1))
        {
            LogWarning("Could not find object's metatable, so the top-level metatable will be used.");

            // Could not find this type's metatable, so just use Component
            lua_pop(L, 1);
            luaL_getmetatable(L, COMPONENT_LUA_NAME);
        }

        OCT_ASSERT(lua_istable(L, -1));
        lua_setmetatable(L, udIndex);
    }
    else
    {
        lua_pushnil(L);
    }

    return 1;
}

int Component_Lua::Destroy(lua_State* L)
{
    CHECK_COMPONENT(L, 1);
    Component_Lua* compLua = (Component_Lua*)lua_touserdata(L, 1);
    compLua->~Component_Lua();
    return 0;
}

int Component_Lua::IsValid(lua_State* L)
{
#if LUA_SAFE_COMPONENT
    Component_Lua* luaObj = static_cast<Component_Lua*>(CheckHierarchyLuaType<Component_Lua>(L, 1, COMPONENT_LUA_NAME, COMPONENT_LUA_FLAG));

    bool ret = (luaObj->mNode.Get() != nullptr);

    lua_pushboolean(L, ret);
    return 1;
#else
    lua_pushboolean(L, true);
    return 1;
#endif
}

int Component_Lua::GetOwner(lua_State* L)
{
    Component* component = CHECK_COMPONENT(L, 1);

    Actor* actor = component->GetOwner();

    Actor_Lua::Create(L, actor);
    return 1;
}

int Component_Lua::GetName(lua_State* L)
{
    Component* component = CHECK_COMPONENT(L, 1);

    const std::string& name = component->GetName();

    lua_pushstring(L, name.c_str());
    return 1;
}

int Component_Lua::SetName(lua_State* L)
{
    Component* component = CHECK_COMPONENT(L, 1);
    const char* name = CHECK_STRING(L, 2);

    component->SetName(name);

    return 0;
}

int Component_Lua::SetActive(lua_State* L)
{
    Component* component = CHECK_COMPONENT(L, 1);
    bool active = CHECK_BOOLEAN(L, 2);

    component->SetActive(active);

    return 0;
}

int Component_Lua::IsActive(lua_State* L)
{
    Component* component = CHECK_COMPONENT(L, 1);

    bool active = component->IsActive();

    lua_pushboolean(L, active);
    return 1;
}

int Component_Lua::SetVisible(lua_State* L)
{
    Component* component = CHECK_COMPONENT(L, 1);
    bool visible = CHECK_BOOLEAN(L, 2);

    component->SetVisible(visible);

    return 0;
}

int Component_Lua::IsVisible(lua_State* L)
{
    Component* component = CHECK_COMPONENT(L, 1);

    bool isVisible = component->IsVisible();

    lua_pushboolean(L, isVisible);
    return 1;
}

int Component_Lua::GetWorld(lua_State* L)
{
    Component* component = CHECK_COMPONENT(L, 1);

    World* world = component->GetWorld();

    World_Lua::Create(L, world);
    return 1;
}

int Component_Lua::Equals(lua_State* L)
{
    Component* compA = CHECK_COMPONENT(L, 1);
    Component* compB = nullptr;

    if (lua_isuserdata(L, 2))
    {
        compB = CHECK_COMPONENT(L, 2);
    }

    bool ret = (compA == compB);

    lua_pushboolean(L, ret);
    return 1;
}

int Component_Lua::CheckType(lua_State* L)
{
    bool ret = false;
    CHECK_COMPONENT(L, 1);
    const char* typeName = CHECK_STRING(L, 2);

    if (lua_getmetatable(L, 1))
    {
        char classFlag[64];
        snprintf(classFlag, 64, "cf%s", typeName);
        lua_getfield(L, 1, classFlag);

        if (!lua_isnil(L, -1))
        {
            ret = true;
        }
    }

    return ret;
}

void Component_Lua::BindCommon(lua_State* L, int mtIndex)
{
    lua_pushcfunction(L, Destroy);
    lua_setfield(L, mtIndex, "__gc");

    lua_pushcfunction(L, Equals);
    lua_setfield(L, mtIndex, "__eq");
}

void Component_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        COMPONENT_LUA_NAME,
        COMPONENT_LUA_FLAG,
        nullptr);

    BindCommon(L, mtIndex);

    lua_pushcfunction(L, IsValid);
    lua_setfield(L, mtIndex, "IsValid");

    lua_pushcfunction(L, GetOwner);
    lua_setfield(L, mtIndex, "GetOwner");

    lua_pushcfunction(L, GetName);
    lua_setfield(L, mtIndex, "GetName");

    lua_pushcfunction(L, SetName);
    lua_setfield(L, mtIndex, "SetName");

    lua_pushcfunction(L, SetActive);
    lua_setfield(L, mtIndex, "SetActive");

    lua_pushcfunction(L, IsActive);
    lua_setfield(L, mtIndex, "IsActive");

    lua_pushcfunction(L, SetVisible);
    lua_setfield(L, mtIndex, "SetVisible");

    lua_pushcfunction(L, IsVisible);
    lua_setfield(L, mtIndex, "IsVisible");

    lua_pushcfunction(L, GetWorld);
    lua_setfield(L, mtIndex, "GetWorld");

    lua_pushcfunction(L, Equals);
    lua_setfield(L, mtIndex, "Equals");

    lua_pushcfunction(L, CheckType);
    lua_pushcfunction(L, CheckType);
    lua_pushcfunction(L, CheckType);
    lua_setfield(L, mtIndex, "CheckType");
    lua_setfield(L, mtIndex, "Is");
    lua_setfield(L, mtIndex, "IsA");

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
