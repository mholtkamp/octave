#pragma once

#include "ScriptMacros.h"
#include <unordered_set>

class ScriptUtils
{
public:

    static bool IsScriptLoaded(const std::string& className);
    static bool ReloadScriptFile(const std::string& fileName);
    static bool CallLuaFunc(int numArgs, int numResults = 0);
    static bool LoadScriptFile(const std::string& fileName, const std::string& className);
    static void ReloadAllScriptFiles();
    static void LoadAllScripts();
    static void LoadScriptDirectory(const std::string& dirName, bool recurse = true);

    static std::string GetClassNameFromFileName(const std::string& fileName);
    static void SetEmbeddedScripts(EmbeddedFile* embeddedScripts, uint32_t numEmbeddedScripts);
    static EmbeddedFile* FindEmbeddedScript(const std::string& className);
    static bool RunScript(const char* fileName, Datum* ret = nullptr);

    static uint32_t GetNextScriptInstanceNumber();

    static void CallMethod(const char* tableName, const char* funcName, uint32_t numParams, const Datum** params, Datum* ret);
    static void SetBreakOnScriptError(bool enableBreak);

    static void GarbageCollect();

    static Datum GetField(const char* table, const char* key);
    static void SetField(const char* table, const char* key, const Datum& value);

    static Datum GetField(const char* table, int32_t key);
    static void SetField(const char* table, int32_t key, const Datum& value);

private:

    static std::unordered_set<std::string> sLoadedLuaFiles;
    static EmbeddedFile* sEmbeddedScripts;
    static uint32_t sNumEmbeddedScripts;
    static uint32_t sNumScriptInstances;

    static bool sBreakOnScriptError;
};
