#include "LuaBindings/ScriptWidget_Lua.h"
#include "LuaBindings/Widget_Lua.h"

#if LUA_ENABLED

int ScriptWidget_Lua::CreateNew(lua_State* L)
{
    const char* fileName = nullptr;
    if (lua_isstring(L, 1))
    {
        fileName = lua_tostring(L, 1);
    }

    Widget* newWidget = nullptr;
    Widget_Lua::CreateNew(L, SCRIPT_WIDGET_LUA_NAME, &newWidget);

    OCT_ASSERT(newWidget);
    OCT_ASSERT(newWidget->GetType() == ScriptWidget::GetStaticType());

    if (fileName != nullptr)
    {
        ScriptWidget* scriptWidget = (ScriptWidget*)newWidget;
        scriptWidget->SetFile(fileName);
    }

    return 1;
}

int ScriptWidget_Lua::CustomIndex(lua_State* L)
{
    int luaType = lua_type(L, 1);
    const char* key = lua_tostring(L, 2);

    if (luaType == LUA_TUSERDATA)
    {
        ScriptWidget* scriptWidget = (ScriptWidget*)((Widget_Lua*)lua_touserdata(L, 1))->mWidget;

        bool fieldFound = false;

        if (scriptWidget->GetTableName() != "")
        {
            // Grab the instance table first
            lua_getglobal(L, scriptWidget->GetTableName().c_str());

            // Then try to grab the field we are looking for.
            lua_getfield(L, -1, key);

            if (!lua_isnil(L, -1))
            {
                fieldFound = true;
            }
            else
            {
                lua_pop(L, 1);
            }
        }

        if (!fieldFound)
        {
            lua_getglobal(L, scriptWidget->RuntimeName());
            OCT_ASSERT(!lua_isnil(L, -1));
            lua_getfield(L, -1, key);
        }
    }
    else
    {
        OCT_ASSERT(luaType == LUA_TTABLE);

        lua_pushstring(L, key);
        lua_rawget(L, 1);

        if (lua_isnil(L, -1)) 
        {
            // Pop the nil value
            lua_pop(L, 1);

            // Get the field from the table's metatable.
            lua_getmetatable(L, 1);
            OCT_ASSERT(lua_type(L, -1) == LUA_TTABLE);
            lua_getfield(L, -1, key);
        }

        // The indexed value should be on the top of the stack now.
    }

    return 1;
}

int ScriptWidget_Lua::CustomNewIndex(lua_State* L)
{
    int luaType = lua_type(L, 1);
    //OCT_ASSERT(lua_isstring(L, 2) || lua_isinteger(L, 2));
    OCT_ASSERT(!lua_isnone(L, 3));

    if (luaType == LUA_TUSERDATA)
    {
        ScriptWidget* scriptWidget = (ScriptWidget*)((Widget_Lua*)lua_touserdata(L, 1))->mWidget;

        if (scriptWidget->GetTableName() != "")
        {
            // Grab the instance table first
            lua_getglobal(L, scriptWidget->GetTableName().c_str());

            // Then try to grab the field we are looking for.
            lua_pushvalue(L, 2); // key
            lua_pushvalue(L, 3); // value
            lua_rawset(L, -3);

            lua_pop(L, 1);
        }
    }
    else
    {
        OCT_ASSERT(luaType == LUA_TTABLE);

        // Just do a normal set
        lua_pushvalue(L, 2); // key
        lua_pushvalue(L, 3); // value
        lua_rawset(L, 1);
    }

    return 0;
}

int ScriptWidget_Lua::GetScript(lua_State* L)
{
    ScriptWidget* scriptWidget = CHECK_SCRIPT_WIDGET(L, 1);

    if (scriptWidget->GetTableName() != "")
    {
        lua_getglobal(L, scriptWidget->GetTableName().c_str());
    }
    else
    {
        lua_pushnil(L);
    }

    return 1;
}

int ScriptWidget_Lua::SetFile(lua_State* L)
{
    ScriptWidget* scriptWidget = CHECK_SCRIPT_WIDGET(L, 1);
    const char* fileName = CHECK_STRING(L, 2);

    scriptWidget->SetFile(fileName);

    return 0;
}

int ScriptWidget_Lua::StartScript(lua_State* L)
{
    ScriptWidget* scriptWidget = CHECK_SCRIPT_WIDGET(L, 1);

    scriptWidget->StartScript();

    return 0;
}

int ScriptWidget_Lua::RestartScript(lua_State* L)
{
    ScriptWidget* scriptWidget = CHECK_SCRIPT_WIDGET(L, 1);

    scriptWidget->RestartScript();

    return 0;
}

int ScriptWidget_Lua::StopScript(lua_State* L)
{
    ScriptWidget* scriptWidget = CHECK_SCRIPT_WIDGET(L, 1);

    scriptWidget->StopScript();

    return 0;
}

void ScriptWidget_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        SCRIPT_WIDGET_LUA_NAME,
        SCRIPT_WIDGET_LUA_FLAG,
        WIDGET_LUA_NAME);

    Widget_Lua::BindCommon(L, mtIndex);

    lua_pushcfunction(L, CreateNew);
    lua_setfield(L, mtIndex, "Create");

    lua_pushcfunction(L, GetScript);
    lua_setfield(L, mtIndex, "GetScript");

    lua_pushcfunction(L, SetFile);
    lua_setfield(L, mtIndex, "SetFile");

    lua_pushcfunction(L, StartScript);
    lua_setfield(L, mtIndex, "StartScript");

    lua_pushcfunction(L, RestartScript);
    lua_setfield(L, mtIndex, "RestartScript");

    lua_pushcfunction(L, StopScript);
    lua_setfield(L, mtIndex, "StopScript");

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
