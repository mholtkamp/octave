#include "ScriptEvent.h"
#include "Engine.h"
#include "Log.h"

#include "Widgets/Widget.h"

#include "Assets/SkeletalMesh.h"
#include "Nodes/3D/SkeletalMesh3d.h"

#include "LuaBindings/Actor_Lua.h"
#include "LuaBindings/Component_Lua.h"
#include "LuaBindings/Network_Lua.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Widget_Lua.h"

bool PrepFunctionCall(lua_State* L, std::string& tableName, std::string& funcName)
{
#if LUA_ENABLED
    bool success = false;

    if (tableName != "")
    {
        // Grab the table
        lua_getglobal(L, tableName.c_str());

        if (lua_istable(L, -1))
        {
            lua_getfield(L, -1, funcName.c_str());

            // Only call the function if it has been defined.
            if (lua_isfunction(L, -1))
            {
                lua_pushvalue(L, -2); // arg1 - self
                success = true;
            }
            else
            {
                lua_pop(L, 1);
            }
        }
        else
        {
            lua_pop(L, 1);
        }
    }

    if (!success)
    {
        // Table or function doesn't exist. This is okay.
        LogDebug("Clearing script callback since the function doesn't exist.");
        tableName = "";
        funcName = "";
    }

    // On success, this function will have pushed 3 values onto the stack
    // (1) Instance table
    // (2) Function
    // (3) Instance table again (as arg1)
    // ExecFunction will restore the stack to the original location.
    // If not successful, this function will not add anything to the stack.
    return success;
#else
    return false;
#endif
}

void ExecFunctionCall(lua_State* L, const std::string& tableName, int argCount)
{
    // The self parameter is always passed as arg1
    OCT_ASSERT(argCount >= 1);
    ScriptComponent* scriptComp = ScriptComponent::FindScriptCompFromTableName(tableName);

    if (scriptComp)
    {
        scriptComp->LuaFuncCall(argCount);
    }
    else
    {
        LogError("Failed to find script comp for ScriptEvent callback");
        lua_pop(L, 1 + argCount); // Func + args
    }

    // Pop the initial table that was found in PrepFunctionCall().
    lua_pop(L, 1);
}

// SkeletalMesh3D
void ScriptEvent::Animation(std::string& tableName, std::string& funcName, const AnimEvent& animEvent)
{
    lua_State* L = GetLua();
    if (PrepFunctionCall(L, tableName, funcName))
    {
        lua_newtable(L);

        Actor_Lua::Create(L, animEvent.mActor);
        lua_setfield(L, -2, "actor");

        Component_Lua::Create(L, animEvent.mComponent);
        lua_setfield(L, -2, "component");

        lua_pushstring(L, animEvent.mName.c_str());
        lua_setfield(L, -2, "name");

        lua_pushstring(L, animEvent.mAnimation.c_str());
        lua_setfield(L, -2, "animation");

        lua_pushnumber(L, animEvent.mTime);
        lua_setfield(L, -2, "time");

        Vector_Lua::Create(L, animEvent.mValue);
        lua_setfield(L, -2, "value");

        ExecFunctionCall(L, tableName, 2);
    }
}

// Widget (Button, Selector, TextField events)
void ScriptEvent::WidgetState(std::string& tableName, std::string& funcName, Widget* widget)
{
    lua_State* L = GetLua();
    if (PrepFunctionCall(L, tableName, funcName))
    {
        Widget_Lua::Create(L, widget);     // arg2 - widget
        ExecFunctionCall(L, tableName, 2);
    }
}

// NetworkManager
void ScriptEvent::NetConnect(std::string& tableName, std::string& funcName, const NetClient& client)
{
    lua_State* L = GetLua();
    if (PrepFunctionCall(L, tableName, funcName))
    {
        PushNetHostProfile(L, client);     // arg2 - client table
        ExecFunctionCall(L, tableName, 2);
    }
}

void ScriptEvent::NetAccept(std::string& tableName, std::string& funcName)
{
    lua_State* L = GetLua();
    if (PrepFunctionCall(L, tableName, funcName))
    {
        ExecFunctionCall(L, tableName, 1);
    }
}

void ScriptEvent::NetReject(std::string& tableName, std::string& funcName, NetMsgReject::Reason reason)
{
    lua_State* L = GetLua();
    if (PrepFunctionCall(L, tableName, funcName))
    {
        lua_pushinteger(L, (int)reason);    // arg2 - reason
        ExecFunctionCall(L, tableName, 2);
    }
}

void ScriptEvent::NetDisconnect(std::string& tableName, std::string& funcName, const NetClient& client)
{
    lua_State* L = GetLua();
    if (PrepFunctionCall(L, tableName, funcName))
    {
        PushNetHostProfile(L, client);     // arg2 - client table
        ExecFunctionCall(L, tableName, 2);
    }
}

void ScriptEvent::NetKick(std::string& tableName, std::string& funcName, NetMsgKick::Reason reason)
{
    lua_State* L = GetLua();
    if (PrepFunctionCall(L, tableName, funcName))
    {
        lua_pushinteger(L, (int)reason);    // arg2 - reason
        ExecFunctionCall(L, tableName, 2);
    }
}
