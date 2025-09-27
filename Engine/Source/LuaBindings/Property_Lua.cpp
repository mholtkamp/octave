#include "LuaBindings/Property_Lua.h"
#include "LuaBindings/LuaUtils.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Node_Lua.h"
#include "Engine/Script.h"
#include "Log.h"
#include <vector>

#if LUA_ENABLED

// Static members
Script* Property_Lua::sCurrentInitializingScript = nullptr;
std::vector<AutoPropertyInfo> Property_Lua::sPendingAutoProperties;

// Custom userdata type that tracks property information
struct PropertyTracker
{
    DatumType mType;
    Datum mValue;
    std::string mDisplayName;
    bool mIsArray = false;
    std::vector<Datum> mArrayValues;
};

#define PROPERTY_TRACKER_LUA_NAME "PropertyTracker"

int Property_Lua::Create(lua_State* L)
{
    int numArgs = lua_gettop(L);
    
    if (numArgs < 2)
    {
        return 0;
    }
    
    // Get the type (first argument)
    DatumType type = DatumType::Count;
    if (lua_isinteger(L, 1))
    {
        type = (DatumType)lua_tointeger(L, 1);
    }
    else
    {
        luaL_error(L, "Property.Create() first argument must be a DatumType");
        return 0;
    }
    
    // Get the default value (second argument)
    Datum defaultValue;
    switch (type)
    {
    case DatumType::Integer:
        if (lua_isinteger(L, 2))
        {
            defaultValue = Datum((int32_t)lua_tointeger(L, 2));
        }
        else
        {
            defaultValue = Datum((int32_t)0);
        }
        break;
    case DatumType::Float:
        if (lua_isnumber(L, 2))
        {
            defaultValue = Datum((float)lua_tonumber(L, 2));
        }
        else
        {
            defaultValue = Datum(0.0f);
        }
        break;
    case DatumType::Bool:
        if (lua_isboolean(L, 2))
        {
            defaultValue = Datum((bool)lua_toboolean(L, 2));
        }
        else
        {
            defaultValue = Datum(false);
        }
        break;
    case DatumType::String:
        if (lua_isstring(L, 2))
        {
            defaultValue = Datum(lua_tostring(L, 2));
        }
        else
        {
            defaultValue = Datum("");
        }
        break;
    case DatumType::Vector2D:
        if (lua_isuserdata(L, 2))
        {
            glm::vec2 vec = CHECK_VECTOR(L, 2);
            defaultValue = Datum(vec);
        }
        else
        {
            defaultValue = Datum(glm::vec2(0.0f));
        }
        break;
    case DatumType::Vector:
        if (lua_isuserdata(L, 2))
        {
            glm::vec3 vec = CHECK_VECTOR(L, 2);
            defaultValue = Datum(vec);
        }
        else
        {
            defaultValue = Datum(glm::vec3(0.0f));
        }
        break;
    case DatumType::Color:
        if (lua_isuserdata(L, 2))
        {
            glm::vec4 vec = CHECK_VECTOR(L, 2);
            defaultValue = Datum(vec);
        }
        else
        {
            defaultValue = Datum(glm::vec4(0.0f));
        }
        break;
    case DatumType::Asset:
        if (lua_isuserdata(L, 2))
        {
            Asset* asset = CHECK_ASSET(L, 2);
            defaultValue = Datum(asset);
        }
        else
        {
            defaultValue = Datum((Asset*)nullptr);
        }
        break;
    case DatumType::Node:
        if (lua_isuserdata(L, 2))
        {
            Node* node = CHECK_NODE(L, 2);
            defaultValue = Datum(node);
        }
        else
        {
            defaultValue = Datum((Node*)nullptr);
        }
        break;
    case DatumType::Byte:
        if (lua_isinteger(L, 2))
        {
            defaultValue = Datum((uint8_t)lua_tointeger(L, 2));
        }
        else
        {
            defaultValue = Datum((uint8_t)0);
        }
        break;
    case DatumType::Short:
        if (lua_isinteger(L, 2))
        {
            defaultValue = Datum((int16_t)lua_tointeger(L, 2));
        }
        else
        {
            defaultValue = Datum((int16_t)0);
        }
        break;
    default:
        luaL_error(L, "Property.Create() unsupported type: %d", (int)type);
        return 0;
    }
    
    // Get optional display name (third argument)
    std::string displayName = "";
    if (numArgs >= 3 && lua_isstring(L, 3))
    {
        displayName = lua_tostring(L, 3);
    }
    
    // Create a PropertyTracker userdata that we can detect later
    PropertyTracker* tracker = (PropertyTracker*)lua_newuserdata(L, sizeof(PropertyTracker));
    new (tracker) PropertyTracker();
    tracker->mType = type;
    tracker->mValue = defaultValue;
    tracker->mDisplayName = displayName;
    
    // Set the metatable for the PropertyTracker
    luaL_getmetatable(L, PROPERTY_TRACKER_LUA_NAME);
    if (lua_isnil(L, -1))
    {
        // Create the metatable if it doesn't exist
        lua_pop(L, 1); // pop nil
        luaL_newmetatable(L, PROPERTY_TRACKER_LUA_NAME);
        
        // Add a __gc metamethod to clean up the tracker
        lua_pushstring(L, "__gc");
        lua_pushcfunction(L, [](lua_State* L) -> int {
            PropertyTracker* tracker = (PropertyTracker*)lua_touserdata(L, 1);
            tracker->~PropertyTracker();
            return 0;
        });
        lua_rawset(L, -3);
        
        // Add __tostring for debugging
        lua_pushstring(L, "__tostring");
        lua_pushcfunction(L, [](lua_State* L) -> int {
            PropertyTracker* tracker = (PropertyTracker*)lua_touserdata(L, 1);
            std::string str = "PropertyTracker(type=" + std::to_string((int)tracker->mType) + ")";
            lua_pushstring(L, str.c_str());
            return 1;
        });
        lua_rawset(L, -3);
    }
    
    lua_setmetatable(L, -2);
    
    return 1;
}

int Property_Lua::CreateArray(lua_State* L)
{
    int numArgs = lua_gettop(L);
    
    if (numArgs < 2)
    {
        return 0;
    }
    
    // Get the type (first argument)
    DatumType type = DatumType::Count;
    if (lua_isinteger(L, 1))
    {
        type = (DatumType)lua_tointeger(L, 1);
    }
    else
    {
        luaL_error(L, "Property.CreateArray() first argument must be a DatumType");
        return 0;
    }
    
    // Get the array values (second argument)
    std::vector<Datum> arrayValues;
    if (lua_istable(L, 2))
    {
        int len = (int)lua_rawlen(L, 2);
        for (int i = 1; i <= len; ++i)
        {
            lua_rawgeti(L, 2, i);
            
            Datum datum;
            switch (type)
            {
            case DatumType::Integer:
                if (lua_isinteger(L, -1))
                    datum = Datum((int32_t)lua_tointeger(L, -1));
                else
                    datum = Datum((int32_t)0);
                break;
            case DatumType::Float:
                if (lua_isnumber(L, -1))
                    datum = Datum((float)lua_tonumber(L, -1));
                else
                    datum = Datum(0.0f);
                break;
            case DatumType::Bool:
                if (lua_isboolean(L, -1))
                    datum = Datum((bool)lua_toboolean(L, -1));
                else
                    datum = Datum(false);
                break;
            case DatumType::String:
                if (lua_isstring(L, -1))
                    datum = Datum(lua_tostring(L, -1));
                else
                    datum = Datum("");
                break;
            case DatumType::Vector2D:
                if (lua_isuserdata(L, -1))
                {
                    glm::vec2 vec = CHECK_VECTOR(L, -1);
                    datum = Datum(vec);
                }
                else
                    datum = Datum(glm::vec2(0.0f));
                break;
            case DatumType::Vector:
                if (lua_isuserdata(L, -1))
                {
                    glm::vec3 vec = CHECK_VECTOR(L, -1);
                    datum = Datum(vec);
                }
                else
                    datum = Datum(glm::vec3(0.0f));
                break;
            case DatumType::Color:
                if (lua_isuserdata(L, -1))
                {
                    glm::vec4 vec = CHECK_VECTOR(L, -1);
                    datum = Datum(vec);
                }
                else
                    datum = Datum(glm::vec4(0.0f));
                break;
            case DatumType::Asset:
                if (lua_isuserdata(L, -1))
                {
                    Asset* asset = CHECK_ASSET(L, -1);
                    datum = Datum(asset);
                }
                else
                    datum = Datum((Asset*)nullptr);
                break;
            case DatumType::Node:
                if (lua_isuserdata(L, -1))
                {
                    Node* node = CHECK_NODE(L, -1);
                    datum = Datum(node);
                }
                else
                    datum = Datum((Node*)nullptr);
                break;
            case DatumType::Byte:
                if (lua_isinteger(L, -1))
                    datum = Datum((uint8_t)lua_tointeger(L, -1));
                else
                    datum = Datum((uint8_t)0);
                break;
            case DatumType::Short:
                if (lua_isinteger(L, -1))
                    datum = Datum((int16_t)lua_tointeger(L, -1));
                else
                    datum = Datum((int16_t)0);
                break;
            default:
                luaL_error(L, "Property.CreateArray() unsupported type: %d", (int)type);
                return 0;
            }
            
            arrayValues.push_back(datum);
            lua_pop(L, 1);
        }
    }
    else
    {
        luaL_error(L, "Property.CreateArray() second argument must be a table");
        return 0;
    }
    
    // Get optional display name (third argument)
    std::string displayName = "";
    if (numArgs >= 3 && lua_isstring(L, 3))
    {
        displayName = lua_tostring(L, 3);
    }
    
    // Create a PropertyTracker userdata for arrays
    PropertyTracker* tracker = (PropertyTracker*)lua_newuserdata(L, sizeof(PropertyTracker));
    new (tracker) PropertyTracker();
    tracker->mType = type;
    tracker->mValue = arrayValues.empty() ? Datum() : arrayValues[0]; // Default value for new entries
    tracker->mDisplayName = displayName;
    tracker->mIsArray = true;
    tracker->mArrayValues = arrayValues;
    
    // Set the metatable for the PropertyTracker
    luaL_getmetatable(L, PROPERTY_TRACKER_LUA_NAME);
    if (lua_isnil(L, -1))
    {
        // Create the metatable if it doesn't exist
        lua_pop(L, 1); // pop nil
        luaL_newmetatable(L, PROPERTY_TRACKER_LUA_NAME);
        
        // Add a __gc metamethod to clean up the tracker
        lua_pushstring(L, "__gc");
        lua_pushcfunction(L, [](lua_State* L) -> int {
            PropertyTracker* tracker = (PropertyTracker*)lua_touserdata(L, 1);
            tracker->~PropertyTracker();
            return 0;
        });
        lua_rawset(L, -3);
        
        // Add __tostring for debugging
        lua_pushstring(L, "__tostring");
        lua_pushcfunction(L, [](lua_State* L) -> int {
            PropertyTracker* tracker = (PropertyTracker*)lua_touserdata(L, 1);
            std::string str = "PropertyTracker(type=" + std::to_string((int)tracker->mType) + ", isArray=" + (tracker->mIsArray ? "true" : "false") + ")";
            lua_pushstring(L, str.c_str());
            return 1;
        });
        lua_rawset(L, -3);
    }
    
    lua_setmetatable(L, -2);
    
    return 1;
}

void Property_Lua::Bind()
{
    lua_State* L = GetLua();

    lua_newtable(L);
    int tableIdx = lua_gettop(L);

    REGISTER_TABLE_FUNC(L, tableIdx, Create);
    REGISTER_TABLE_FUNC(L, tableIdx, CreateArray);

    lua_setglobal(L, PROPERTY_LUA_NAME);

    OCT_ASSERT(lua_gettop(L) == 0);
}

Script* Property_Lua::GetCurrentInitializingScript()
{
    return sCurrentInitializingScript;
}

void Property_Lua::SetCurrentInitializingScript(Script* script)
{
    sCurrentInitializingScript = script;
}

void Property_Lua::ProcessPendingAutoProperties(Script* script)
{
    if (!script)
        return;
        
    lua_State* L = GetLua();
    
    // Get the script instance userdata
    Node_Lua::Create(L, script->GetOwner());
    
    if (lua_isuserdata(L, -1))
    {
        int udIdx = lua_gettop(L);
        
        // Get the uservalue table (this is where script variables are stored)
        lua_getuservalue(L, udIdx);
        if (lua_istable(L, -1))
        {
            int uvIdx = lua_gettop(L);
            
            // Collect all PropertyTracker objects first to avoid modification during iteration
            std::vector<std::string> trackerKeys;
            std::vector<PropertyTracker> trackerData;
            
            // Use lua_next to iterate through the uservalue table
            lua_pushnil(L);  // First key
            while (lua_next(L, uvIdx) != 0)
            {
                // Stack: ... uservalue key value
                if (lua_isstring(L, -2) && lua_isuserdata(L, -1))
                {
                    const char* varName = lua_tostring(L, -2);
                    
                    // Check if this is a PropertyTracker
                    if (luaL_testudata(L, -1, PROPERTY_TRACKER_LUA_NAME))
                    {
                        PropertyTracker* tracker = (PropertyTracker*)lua_touserdata(L, -1);
                        
                        // Store the key and tracker data
                        trackerKeys.push_back(varName);
                        trackerData.push_back(*tracker);
                    }
                }
                
                lua_pop(L, 1); // pop value, keep key for next iteration
            }
            
            // Now process all the collected PropertyTracker objects
            for (size_t i = 0; i < trackerKeys.size(); ++i)
            {
                const std::string& varName = trackerKeys[i];
                const PropertyTracker& tracker = trackerData[i];
                
                // Add this as an auto property
                std::string displayName = tracker.mDisplayName.empty() ? varName : tracker.mDisplayName;
                if (tracker.mIsArray)
                {
                    script->AddAutoPropertyArray(varName, displayName, tracker.mType, tracker.mArrayValues);
                }
                else
                {
                    script->AddAutoProperty(varName, displayName, tracker.mType, tracker.mValue);
                }
                
                // Replace the tracker with the actual value
                if (tracker.mIsArray)
                {
                    // Create a Lua table for array values
                    lua_newtable(L);
                    for (size_t j = 0; j < tracker.mArrayValues.size(); ++j)
                    {
                        const Datum& value = tracker.mArrayValues[j];
                        switch (tracker.mType)
                        {
                        case DatumType::Integer:
                            lua_pushinteger(L, value.GetInteger());
                            break;
                        case DatumType::Float:
                            lua_pushnumber(L, value.GetFloat());
                            break;
                        case DatumType::Bool:
                            lua_pushboolean(L, value.GetBool());
                            break;
                        case DatumType::String:
                            lua_pushstring(L, value.GetString().c_str());
                            break;
                        case DatumType::Vector2D:
                            Vector_Lua::Create(L, value.GetVector2D());
                            break;
                        case DatumType::Vector:
                            Vector_Lua::Create(L, value.GetVector());
                            break;
                        case DatumType::Color:
                            Vector_Lua::Create(L, value.GetColor());
                            break;
                        case DatumType::Asset:
                            Asset_Lua::Create(L, value.GetAsset(), true);
                            break;
                        case DatumType::Node:
                            Node_Lua::Create(L, value.GetNode().Get());
                            break;
                        case DatumType::Byte:
                            lua_pushinteger(L, (int32_t)value.GetByte());
                            break;
                        case DatumType::Short:
                            lua_pushinteger(L, (int32_t)value.GetShort());
                            break;
                        default:
                            lua_pushnil(L);
                            break;
                        }
                        lua_rawseti(L, -2, (int)(j + 1)); // Lua tables are 1-indexed
                    }
                }
                else
                {
                    // Single value
                    switch (tracker.mType)
                    {
                    case DatumType::Integer:
                        lua_pushinteger(L, tracker.mValue.GetInteger());
                        break;
                    case DatumType::Float:
                        lua_pushnumber(L, tracker.mValue.GetFloat());
                        break;
                    case DatumType::Bool:
                        lua_pushboolean(L, tracker.mValue.GetBool());
                        break;
                    case DatumType::String:
                        lua_pushstring(L, tracker.mValue.GetString().c_str());
                        break;
                    case DatumType::Vector2D:
                        Vector_Lua::Create(L, tracker.mValue.GetVector2D());
                        break;
                    case DatumType::Vector:
                        Vector_Lua::Create(L, tracker.mValue.GetVector());
                        break;
                    case DatumType::Color:
                        Vector_Lua::Create(L, tracker.mValue.GetColor());
                        break;
                    case DatumType::Asset:
                        Asset_Lua::Create(L, tracker.mValue.GetAsset(), true);
                        break;
                    case DatumType::Node:
                        Node_Lua::Create(L, tracker.mValue.GetNode().Get());
                        break;
                    case DatumType::Byte:
                        lua_pushinteger(L, (int32_t)tracker.mValue.GetByte());
                        break;
                    case DatumType::Short:
                        lua_pushinteger(L, (int32_t)tracker.mValue.GetShort());
                        break;
                    default:
                        lua_pushnil(L);
                        break;
                    }
                }
                
                // Set the field with the actual value in the uservalue table
                lua_setfield(L, uvIdx, varName.c_str());
            }
            
            lua_pop(L, 1); // pop uservalue table
        }
        else
        {
            lua_pop(L, 1); // pop non-table uservalue
        }
    }
    
    lua_pop(L, 1); // pop userdata
}

void Property_Lua::ClearPendingProperties()
{
    sPendingAutoProperties.clear();
}

void Property_Lua::AddPendingAutoProperty(const std::string& varName, const std::string& displayName, DatumType type, const Datum& defaultValue)
{
    AutoPropertyInfo info;
    info.mVarName = varName;
    info.mDisplayName = displayName;
    info.mType = type;
    info.mDefaultValue = defaultValue;
    sPendingAutoProperties.push_back(info);
}

#endif
