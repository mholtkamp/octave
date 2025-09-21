#pragma once

#include "Engine.h"
#include "Property.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define PROPERTY_LUA_NAME "Property"

struct AutoPropertyInfo
{
    std::string mVarName;
    std::string mDisplayName;
    DatumType mType;
    Datum mDefaultValue;
};

struct Property_Lua
{
    static int Create(lua_State* L);
    static int CreateArray(lua_State* L);
    
    static void Bind();
    
    // Static function to get the current script that's being initialized
    static class Script* GetCurrentInitializingScript();
    static void SetCurrentInitializingScript(class Script* script);
    
    // Function to process pending auto properties
    static void ProcessPendingAutoProperties(class Script* script);
    
    // Clear pending properties
    static void ClearPendingProperties();
    
    // Add a pending auto property
    static void AddPendingAutoProperty(const std::string& varName, const std::string& displayName, DatumType type, const Datum& defaultValue);

private:
    static class Script* sCurrentInitializingScript;
    static std::vector<AutoPropertyInfo> sPendingAutoProperties;
};

#endif
