#include "Widgets/ScriptWidget.h"
#include "Engine.h"
#include "ScriptUtils.h"

#include "LuaBindings/Widget_Lua.h"

#if EDITOR
#include "EditorState.h"
#endif

FORCE_LINK_DEF(ScriptWidget);
DEFINE_WIDGET(ScriptWidget, Widget);

ScriptWidget::ScriptWidget()
{

}

ScriptWidget::ScriptWidget(const char* scriptName)
{
    SetFile(scriptName);
}

void ScriptWidget::GatherProperties(std::vector<Property>& outProps)
{
    Widget::GatherProperties(outProps);

    outProps.push_back(Property(DatumType::String, "File", this, &mFileName));
}

void ScriptWidget::SetFile(const char* filename)
{
    if (mFileName != "")
    {
        StopScript();
    }

    mFileName = filename;

    if (mFileName != "")
    {
        StartScript();
    }
}

const std::string& ScriptWidget::GetFile()
{
    return mFileName;
}

const std::string& ScriptWidget::GetTableName()
{
    return mTableName;
}

void ScriptWidget::Update()
{
    Widget::Update();

    // To prevent updating in Widget editor.
    if (IsPlaying())
    {
        CallFunction("Update");
    }
}

void ScriptWidget::StartScript()
{
    if (mTableName == "")
    {
        CreateScriptInstance();
    }
}

void ScriptWidget::RestartScript()
{
    DestroyScriptInstance();
    CreateScriptInstance();
}

void ScriptWidget::StopScript()
{
    DestroyScriptInstance();
}

// These functions are kinda nasty, but it's a nice convenience for the game programmer
void ScriptWidget::CallFunction(const char* name)
{
    CallFunction(name, 0, nullptr, nullptr);
}

void ScriptWidget::CallFunction(const char* name, const Datum& param0)
{
    const Datum* params[] = { &param0 };
    CallFunction(name, 1, params, nullptr);
}

void ScriptWidget::CallFunction(const char* name, const Datum& param0, const Datum& param1)
{
    const Datum* params[] = { &param0, &param1 };
    CallFunction(name, 2, params, nullptr);
}

void ScriptWidget::CallFunction(const char* name, const Datum& param0, const Datum& param1, const Datum& param2)
{
    const Datum* params[] = { &param0, &param1, &param2 };
    CallFunction(name, 3, params, nullptr);
}

void ScriptWidget::CallFunction(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3)
{
    const Datum* params[] = { &param0, &param1, &param2, &param3 };
    CallFunction(name, 4, params, nullptr);
}

void ScriptWidget::CallFunction(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3, const Datum& param4)
{
    const Datum* params[] = { &param0, &param1, &param2, &param3, &param4 };
    CallFunction(name, 5, params, nullptr);
}

void ScriptWidget::CallFunction(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3, const Datum& param4, const Datum& param5)
{
    const Datum* params[] = { &param0, &param1, &param2, &param3, &param4, &param5 };
    CallFunction(name, 6, params, nullptr);
}

void ScriptWidget::CallFunction(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3, const Datum& param4, const Datum& param5, const Datum& param6)
{
    const Datum* params[] = { &param0, &param1, &param2, &param3, &param4, &param5, &param6 };
    CallFunction(name, 7, params, nullptr);
}

void ScriptWidget::CallFunction(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3, const Datum& param4, const Datum& param5, const Datum& param6, const Datum& param7)
{
    const Datum* params[] = { &param0, &param1, &param2, &param3, &param4, &param5, &param6, &param7 };
    CallFunction(name, 8, params, nullptr);
}

Datum ScriptWidget::CallFunctionR(const char* name)
{
    Datum retDatum;
    CallFunction(name, 0, nullptr, &retDatum);
    return retDatum;
}

Datum ScriptWidget::CallFunctionR(const char* name, const Datum& param0)
{
    Datum retDatum;
    const Datum* params[] = { &param0 };
    CallFunction(name, 1, params, &retDatum);
    return retDatum;
}

Datum ScriptWidget::CallFunctionR(const char* name, const Datum& param0, const Datum& param1)
{
    Datum retDatum;
    const Datum* params[] = { &param0, &param1 };
    CallFunction(name, 2, params, &retDatum);
    return retDatum;
}

Datum ScriptWidget::CallFunctionR(const char* name, const Datum& param0, const Datum& param1, const Datum& param2)
{
    Datum retDatum;
    const Datum* params[] = { &param0, &param1, &param2 };
    CallFunction(name, 3, params, &retDatum);
    return retDatum;
}

Datum ScriptWidget::CallFunctionR(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3)
{
    Datum retDatum;
    const Datum* params[] = { &param0, &param1, &param2, &param3 };
    CallFunction(name, 4, params, &retDatum);
    return retDatum;
}

Datum ScriptWidget::CallFunctionR(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3, const Datum& param4)
{
    Datum retDatum;
    const Datum* params[] = { &param0, &param1, &param2, &param3, &param4 };
    CallFunction(name, 5, params, &retDatum);
    return retDatum;
}

Datum ScriptWidget::CallFunctionR(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3, const Datum& param4, const Datum& param5)
{
    Datum retDatum;
    const Datum* params[] = { &param0, &param1, &param2, &param3, &param4, &param5 };
    CallFunction(name, 6, params, &retDatum);
    return retDatum;
}

Datum ScriptWidget::CallFunctionR(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3, const Datum& param4, const Datum& param5, const Datum& param6)
{
    Datum retDatum;
    const Datum* params[] = { &param0, &param1, &param2, &param3, &param4, &param5, &param6 };
    CallFunction(name, 7, params, &retDatum);
    return retDatum;
}

Datum ScriptWidget::CallFunctionR(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3, const Datum& param4, const Datum& param5, const Datum& param6, const Datum& param7)
{
    Datum retDatum;
    const Datum* params[] = { &param0, &param1, &param2, &param3, &param4, &param5, &param6, &param7 };
    CallFunction(name, 8, params, &retDatum);
    return retDatum;
}

void ScriptWidget::CallFunction(const char* name, uint32_t numParams, const Datum** params, Datum* ret)
{
    if (mTableName != "")
    {
        ScriptUtils::CallMethod(mTableName.c_str(), name, numParams, params, ret);
    }
}

Datum ScriptWidget::GetField(const char* key)
{
    Datum ret;

    if (mTableName != "")
    {
        ret = ScriptUtils::GetField(mTableName.c_str(), key);
    }

    return ret;
}

void ScriptWidget::SetField(const char* key, const Datum& value)
{
    if (mTableName != "")
    {
        ScriptUtils::SetField(mTableName.c_str(), key, value);
    }
}


void ScriptWidget::CreateScriptInstance()
{
#if LUA_ENABLED

    lua_State* L = GetLua();

    // If mTableName is set, that means CreateScriptInstance is being called
    // before the previous instance was destroyed with DestroyScriptInstance.
    if (mTableName == "" &&
        mFileName != "")
    {
        bool classLoaded = false;

        // Determine the class name the script should use
        // For instance, if the filename is Characters/Monster/Goblin.lua,
        // then the classname should be Goblin.
        mClassName = ScriptUtils::GetClassNameFromFileName(mFileName);

        classLoaded = ScriptUtils::IsScriptLoaded(mClassName);
        if (!classLoaded)
        {
            classLoaded = ScriptUtils::LoadScriptFile(mFileName, mClassName);
        }

        if (classLoaded)
        {
            // Create a new table
            lua_newtable(L);
            int instanceTableIdx = lua_gettop(L);

            // Retrieve the prototype class table
            lua_getglobal(L, mClassName.c_str());
            OCT_ASSERT(lua_istable(L, -1));
            int classTableIdx = lua_gettop(L);

            // Check if the class table has a New function, if so call it to initialize the object (needed for setting up inheritance).
            lua_getfield(L, -1, "New");
            if (lua_isfunction(L, -1))
            {
                lua_pushvalue(L, classTableIdx); // push the class table as arg1 (self)
                lua_pushvalue(L, instanceTableIdx); // push the newly created instance table as arg2 (o)
                ScriptUtils::CallLuaFunc(2);
            }
            else
            {
                lua_pop(L, 1);
            }

            // Assign the new table's metatable to the class table
            lua_setmetatable(L, instanceTableIdx);

            Widget_Lua::Create(L, this);
            lua_setfield(L, instanceTableIdx, "widget");

            mTableName = mClassName + "_" + std::to_string(ScriptUtils::GetNextScriptInstanceNumber());

            // Register the global name on to the script itself, so that native functions can
            // identify what script they are working with.
            lua_pushstring(L, mTableName.c_str());
            lua_setfield(L, instanceTableIdx, "tableName");

            // Save the new table as a global so it doesnt get GCed.
            lua_setglobal(L, mTableName.c_str());

            if (IsPlaying())
            {
                CallFunction("Create");
            }
        }
        else
        {
            LogError("Failed to create script instance. Class has not been loaded.");
        }
    }
#endif
}

void ScriptWidget::DestroyScriptInstance()
{
#if LUA_ENABLED
    if (mTableName != "")
    {
        // Erase this instance so that it gets garbage collected.
        lua_State* L = GetLua();
        if (L != nullptr)
        {
            if (IsPlaying())
            {
                CallFunction("Destroy");
            }

            // Clear the actor and component fields of the table in case anything else tries to access it.
            // Also set a destroyed field that can be queried.
            lua_getglobal(L, mTableName.c_str());
            if (lua_istable(L, -1))
            {
                int tableIdx = lua_gettop(L);

                lua_pushnil(L);
                lua_setfield(L, tableIdx, "widget");

                lua_pushboolean(L, true);
                lua_setfield(L, tableIdx, "destroyed");
            }
            lua_pop(L, 1);

            // Erase this global. It will eventually be garbage collected when nothing else references it.
            lua_pushnil(L);
            lua_setglobal(L, mTableName.c_str());
        }

        mTableName = "";
        mClassName = "";
    }
#endif
}
