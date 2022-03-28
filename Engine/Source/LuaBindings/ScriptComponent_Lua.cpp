#include "LuaBindings/ScriptComponent_Lua.h"
#include "LuaBindings/Component_Lua.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

void PullDatumValue(lua_State* L, Datum& datum, int idx)
{
#if LUA_ENABLED
    int luaType = lua_type(L, idx);

    switch (luaType)
    {
    case LUA_TNUMBER:
        if (lua_isinteger(L, idx))
        {
            datum.PushBack((int32_t)lua_tointeger(L, idx));
        }
        else
        {
            datum.PushBack(lua_tonumber(L, idx));
        }
        break;

    case LUA_TBOOLEAN:
        datum.PushBack((bool)lua_toboolean(L, idx));
        break;

    case LUA_TSTRING:
        datum.PushBack(lua_tostring(L, idx));
        break;

    case LUA_TUSERDATA:
        if (luaL_testudata(L, idx, VECTOR_LUA_NAME))
        {
            glm::vec4 value = CHECK_VECTOR(L, idx);
            datum.PushBack(value);
        }
        else
        {
            Asset* value = CHECK_ASSET(L, idx);
            datum.PushBack(value);
        }
        //else
        //{
        //    LogError("Passed bad data type in Invoke() arg %d", idx);
        //    datum.PushBack(false);
        //}
        break;

    default:
        LogError("Passed bad data type in Invoke() arg %d", idx);
        break;
    }
#endif
}

int ScriptComponent_Lua::SetFile(lua_State* L)
{
    ScriptComponent* comp = CHECK_SCRIPT_COMPONENT(L, 1);
    const char* value = CHECK_STRING(L, 2);

    comp->SetFile(value);

    return 0;
}

int ScriptComponent_Lua::GetFile(lua_State* L)
{
    ScriptComponent* comp = CHECK_SCRIPT_COMPONENT(L, 1);

    const std::string& file = comp->GetFile();

    lua_pushstring(L, file.c_str());
    return 1;
}

int ScriptComponent_Lua::StartScript(lua_State* L)
{
    ScriptComponent* comp = CHECK_SCRIPT_COMPONENT(L, 1);

    comp->StartScript();

    return 0;
}

int ScriptComponent_Lua::RestartScript(lua_State* L)
{
    ScriptComponent* comp = CHECK_SCRIPT_COMPONENT(L, 1);

    comp->RestartScript();

    return 0;
}

int ScriptComponent_Lua::StopScript(lua_State* L)
{
    ScriptComponent* comp = CHECK_SCRIPT_COMPONENT(L, 1);

    comp->StopScript();

    return 0;
}

int ScriptComponent_Lua::ReloadScriptFile(lua_State* L)
{
    ScriptComponent* comp = CHECK_SCRIPT_COMPONENT(L, 1);

    comp->ReloadScriptFile(comp->GetFile());

    return 0;
}

int ScriptComponent_Lua::InvokeNetFunc(lua_State* L)
{
    ScriptComponent* comp = nullptr;
    const char* funcName = CHECK_STRING(L, 2);
    int numArgs = lua_gettop(L) - 2;
    std::vector<Datum> params;

    // Find the component if invoked on script table.
    // Otherwise arg1 should be the component.
    if (numArgs >= 0)
    {
        if (lua_istable(L, 1))
        {
            lua_getfield(L, 1, "component");
            comp = CHECK_SCRIPT_COMPONENT(L, -1);
            lua_pop(L, 1); // pop the component
        }
        else
        {
            comp = CHECK_SCRIPT_COMPONENT(L, 1);
        }
    }

    for (int32_t i = 0; i < numArgs; ++i)
    {
        params.push_back(Datum());
        // Invoke param Idx starts at arg 3 (after self and funcName)
        PullDatumValue(L, params.back(), i + 3);
    }

    comp->InvokeNetFunc(funcName, params);

    return 0;
}

void ScriptComponent_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        SCRIPT_COMPONENT_LUA_NAME,
        SCRIPT_COMPONENT_LUA_FLAG,
        COMPONENT_LUA_NAME);

    lua_pushcfunction(L, SetFile);
    lua_setfield(L, mtIndex, "SetFile");

    lua_pushcfunction(L, GetFile);
    lua_setfield(L, mtIndex, "GetFile");

    lua_pushcfunction(L, StartScript);
    lua_setfield(L, mtIndex, "StartScript");

    lua_pushcfunction(L, RestartScript);
    lua_setfield(L, mtIndex, "RestartScript");

    lua_pushcfunction(L, StopScript);
    lua_setfield(L, mtIndex, "StopScript");

    lua_pushcfunction(L, ReloadScriptFile);
    lua_setfield(L, mtIndex, "ReloadScriptFile");

    lua_pushcfunction(L, InvokeNetFunc);
    lua_setfield(L, mtIndex, "InvokeNetFunc");

    lua_pop(L, 1);
    assert(lua_gettop(L) == 0);
}

#endif
