#include "ScriptUtils.h"

std::unordered_set<std::string> ScriptUtils::sLoadedLuaFiles;
EmbeddedFile* ScriptUtils::sEmbeddedScripts = nullptr;
uint32_t ScriptUtils::sNumEmbeddedScripts = 0;
uint32_t ScriptUtils::sNumScriptInstances = 0;
bool ScriptUtils::sBreakOnScriptError = false;

bool ScriptUtils::IsScriptLoaded(const std::string& className)
{
    return sLoadedLuaFiles.find(className) != sLoadedLuaFiles.end();
}

bool ScriptUtils::ReloadScriptFile(const std::string& fileName)
{
    std::string className = GetClassNameFromFileName(fileName);

    auto it = sLoadedLuaFiles.find(className);
    if (it != sLoadedLuaFiles.end())
    {
        sLoadedLuaFiles.erase(it);
    }

    bool success = LoadScriptFile(fileName, className);

    return success;
}

bool ScriptUtils::CallLuaFunc(int numArgs, int numResults)
{
    bool success = true;

#if LUA_ENABLED
    lua_State* L = GetLua();
    if (lua_pcall(L, numArgs, numResults, 0))
    {
        LogError("Lua Error: %s\n", lua_tostring(L, -1));
        if (sBreakOnScriptError) { OCT_ASSERT(0); }
        success = false;
    }
#endif

    return success;
}

bool ScriptUtils::LoadScriptFile(const std::string& fileName, const std::string& className)
{
    bool successful = false;

#if LUA_ENABLED
    lua_State* L = GetLua();
    successful = RunScript(fileName.c_str());

    if (successful)
    {
        // Assign the __index metamethod to itself, so that tables with the class metatable
        // will have access to its methods/properties.
        lua_getglobal(L, className.c_str());
        OCT_ASSERT(lua_istable(L, -1));

        lua_pushvalue(L, -1);
        lua_setfield(L, -2, "__index");

        lua_pop(L, 1);

        sLoadedLuaFiles.insert(className);
        successful = true;
    }
#endif

    return successful;
}

void ScriptUtils::ReloadAllScriptFiles()
{
    std::vector<std::string> fileNames;

    for (const std::string& fileName : sLoadedLuaFiles)
    {
        fileNames.push_back(fileName);
    }

    sLoadedLuaFiles.clear();

    for (uint32_t i = 0; i < fileNames.size(); ++i)
    {
        std::string className = GetClassNameFromFileName(fileNames[i]);
        LoadScriptFile(fileNames[i], className);
    }

    // This doesn't re-gather the NetFuncs for this script file.
}

std::string ScriptUtils::GetClassNameFromFileName(const std::string& fileName)
{
    std::string className = fileName;
    size_t dotLoc = className.find_last_of('.');
    if (dotLoc != std::string::npos)
    {
        className = className.substr(0, dotLoc);
    }

    size_t slashLoc = className.find_last_of('/');
    if (slashLoc != std::string::npos)
    {
        className = className.substr(slashLoc + 1);
    }

    return className;
}

void ScriptUtils::SetEmbeddedScripts(EmbeddedFile* embeddedScripts, uint32_t numEmbeddedScripts)
{
    sEmbeddedScripts = embeddedScripts;
    sNumEmbeddedScripts = numEmbeddedScripts;
}

EmbeddedFile* ScriptUtils::FindEmbeddedScript(const std::string& className)
{
    EmbeddedFile* retFile = nullptr;

    for (uint32_t i = 0; i < sNumEmbeddedScripts; ++i)
    {
        if (className == sEmbeddedScripts[i].mName)
        {
            retFile = &sEmbeddedScripts[i];
            break;
        }
    }

    return retFile;
}

bool ScriptUtils::RunScript(const char* fileName, Datum* ret)
{
    bool successful = false;

#if LUA_ENABLED
    lua_State* L = GetLua();

    std::string relativeFileName = fileName;

    if (relativeFileName.length() < 4 ||
        relativeFileName.compare(relativeFileName.length() - 4, 4, ".lua") != 0)
    {
        relativeFileName.append(".lua");
    }

    bool fileExists = false;
    std::string className = GetClassNameFromFileName(fileName);
    EmbeddedFile* embeddedScript = nullptr;

    if (sEmbeddedScripts != nullptr &&
        sNumEmbeddedScripts > 0)
    {
        embeddedScript = FindEmbeddedScript(className);
        fileExists = (embeddedScript != nullptr);
    }

    std::string fullFileName = GetEngineState()->mProjectDirectory + "Scripts/" + relativeFileName;

    if (!fileExists)
    {
        fileExists = DoesFileExist(fullFileName.c_str());
    }

    if (!fileExists)
    {
        // Fall back to Engine script directory
        fullFileName = std::string("Engine/Scripts/") + relativeFileName;
        fileExists = DoesFileExist(fullFileName.c_str());
    }

    if (fileExists)
    {
        int numResults = (ret != nullptr) ? 1 : 0;

        if (embeddedScript != nullptr)
        {
            LogDebug("Loading embedded script: %s", className.c_str());

            std::string luaString;
            luaString.assign(embeddedScript->mData, embeddedScript->mSize);

            if (luaL_dostring(L, luaString.c_str()) == LUA_OK)
            {
                successful = true;
            }
            else
            {
                LogError("Lua Error: %s\n", lua_tostring(L, -1));
                if (sBreakOnScriptError) { OCT_ASSERT(0); }

                LogError("Couldn't load embedded script file %s", className.c_str());
            }
        }
        else
        {
            if (luaL_loadfile(L, fullFileName.c_str()) == LUA_OK)
            {
                if (lua_pcall(L, 0, numResults, 0) == LUA_OK)
                {
                    successful = true;
                }
                else
                {
                    LogError("Lua Error: %s\n", lua_tostring(L, -1));
                    if (sBreakOnScriptError) { OCT_ASSERT(0); }
                }
            }
            else
            {
                LogError("Lua Error: %s\n", lua_tostring(L, -1));
                if (sBreakOnScriptError) { OCT_ASSERT(0); }

                LogError("Couldn't load script file %s", fullFileName.c_str());
            }
        }

        if (successful && ret != nullptr)
        {
            LuaObjectToDatum(L, -1, *ret);
            lua_pop(L, 1);
        }
    }
#endif

    return successful;
}

uint32_t ScriptUtils::GetNextScriptInstanceNumber()
{
    uint32_t retNum = sNumScriptInstances;
    ++sNumScriptInstances;
    return retNum;
}

void ScriptUtils::CallMethod(const char* tableName, const char* funcName, uint32_t numParams, const Datum** params, Datum* ret)
{
#if LUA_ENABLED
    lua_State* L = GetLua();

    OCT_ASSERT(tableName != nullptr);
    OCT_ASSERT(*tableName != '\0');

    // Grab the script instance table
    lua_getglobal(L, tableName);
    OCT_ASSERT(lua_istable(L, -1));
    lua_getfield(L, -1, funcName);

    // Only call the function if it has been defined.
    if (lua_isfunction(L, -1))
    {
        // Push self param
        lua_pushvalue(L, -2);

        for (uint32_t i = 0; i < numParams; ++i)
        {
            LuaPushDatum(L, *params[i]);
        }

        int totalParams = numParams + 1; // Always pass self table
        int numReturns = (ret != nullptr) ? 1 : 0;
        bool success = CallLuaFunc(totalParams, numReturns);

        if (ret != nullptr && success)
        {
            LuaObjectToDatum(L, -1, *ret);
            lua_pop(L, 1);
        }
    }
    else
    {
        lua_pop(L, 1);
    }

    lua_pop(L, 1);
#endif
}

void ScriptUtils::SetBreakOnScriptError(bool enableBreak)
{
    sBreakOnScriptError = enableBreak;
}

void ScriptUtils::GarbageCollect()
{
#if LUA_ENABLED
    lua_State* L = GetLua();

    lua_getglobal(L, "collectgarbage");
    OCT_ASSERT(lua_isfunction(L, -1));

    lua_pushstring(L, "collect");

    CallLuaFunc(1);
#endif
}

Datum ScriptUtils::GetField(const char* table, const char* key)
{
    Datum ret;

#if LUA_ENABLED
    lua_State* L = GetLua();

    // Grab the script instance table
    lua_getglobal(L, table);
    OCT_ASSERT(lua_istable(L, -1));
    lua_getfield(L, -1, key);

    LuaObjectToDatum(L, -1, ret);

    // Pop field and instance table
    lua_pop(L, 2);
#endif

    return ret;
}

void ScriptUtils::SetField(const char* table, const char* key, const Datum& value)
{
#if LUA_ENABLED
    lua_State* L = GetLua();

    // Grab the script instance table
    lua_getglobal(L, table);
    OCT_ASSERT(lua_istable(L, -1));

    LuaPushDatum(L, value);
    lua_setfield(L, -2, key);
    
    // Pop instance table
    lua_pop(L, 1);
#endif
}

Datum ScriptUtils::GetField(const char* table, int32_t key)
{
    Datum ret;

#if LUA_ENABLED
    lua_State* L = GetLua();

    // Grab the script instance table
    lua_getglobal(L, table);
    OCT_ASSERT(lua_istable(L, -1));
    lua_geti(L, -1, key);

    LuaObjectToDatum(L, -1, ret);

    // Pop field and instance table
    lua_pop(L, 2);
#endif

    return ret;
}

void ScriptUtils::SetField(const char* table, int32_t key, const Datum& value)
{
#if LUA_ENABLED
    lua_State* L = GetLua();

    // Grab the script instance table
    lua_getglobal(L, table);
    OCT_ASSERT(lua_istable(L, -1));

    LuaPushDatum(L, value);
    lua_seti(L, -2, key);

    // Pop instance table
    lua_pop(L, 1);
#endif
}

