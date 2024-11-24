#include "Script.h"
#include "Nodes/3D/Primitive3d.h"
#include "Nodes/3D/SkeletalMesh3d.h"
#include "Constants.h"
#include "Assets/SkeletalMesh.h"
#include "Engine.h"
#include "Log.h"

#include "Nodes/Widgets/Button.h"
#include "Nodes/Widgets/Selector.h"
#include "Nodes/Widgets/TextField.h"

#include "LuaBindings/LuaUtils.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Node_Lua.h"
#include "LuaBindings/Network_Lua.h"
#include "LuaBindings/Widget_Lua.h"
#include "LuaBindings/World_Lua.h"

std::unordered_map<std::string, ScriptNetFuncMap> Script::sScriptNetFuncMap;

bool Script::HandleScriptPropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);

    OCT_ASSERT(prop != nullptr);
    OCT_ASSERT(!prop->mExternal);
    Script* script = static_cast<Script*>(prop->mOwner);

    prop->SetValueRaw(newValue, index);

    // Now that the value has been updated (prop->SetValue()),
    // We want to propagate that new value to the script instance table.
    script->UploadDatum(*prop, prop->mName.c_str());

    return true;
}

bool Script::HandleForeignScriptPropChange(Datum* datum, uint32_t index, const void* newValue)
{
    // This is ridiculous, but for the Property's that are passed to the editor, we need
    // to propagate the changes made to them so that they reflect the ScriptComponent's properties.
    Property* prop = static_cast<Property*>(datum);

    OCT_ASSERT(prop != nullptr);
    OCT_ASSERT(!prop->mExternal);
    Script* script = static_cast<Script*>(prop->mOwner);

    prop->SetValueRaw(newValue, index);

    Property* scriptProp = nullptr;
    for (uint32_t i = 0; i < script->mScriptProps.size(); ++i)
    {
        if (script->mScriptProps[i].mName == prop->mName)
        {
            scriptProp = &script->mScriptProps[i];
            break;
        }
    }

    if (scriptProp != nullptr)
    {
        if (scriptProp->IsArray() &&
            scriptProp->GetCount() != prop->GetCount())
        {
            scriptProp->SetCount(prop->GetCount());
        }

        scriptProp->SetValue(newValue, index);
    }

    return true;
}


Script::Script(Node* owner)
{
    mOwner = owner;
}

Script::~Script()
{
    StopScript();
}

Node* Script::GetOwner()
{
    return mOwner;
}

void Script::Tick(float deltaTime)
{
    CallTick(deltaTime);

    if (NetIsServer())
    {
        DownloadReplicatedData();
    }
}

void Script::AppendScriptProperties(std::vector<Property>& outProps)
{
    for (uint32_t i = 0; i < mScriptProps.size(); ++i)
    {
        outProps.push_back(mScriptProps[i]);

        // Assign our tunneling change handler to the appended properties to make sure they
        // update the properties that are stored in the mScriptProps vector.
        outProps.back().mChangeHandler = HandleForeignScriptPropChange;
    }
}

void Script::UploadScriptProperties()
{
    for (uint32_t i = 0; i < mScriptProps.size(); ++i)
    {
        UploadDatum(mScriptProps[i], mScriptProps[i].mName.c_str());
    }
}

void Script::SetArrayScriptPropCount(const std::string& name, uint32_t count)
{
    for (uint32_t i = 0; i < mScriptProps.size(); ++i)
    {
        if (mScriptProps[i].mName == name)
        {
            mScriptProps[i].SetCount(count);
            break;
        }
    }

    UploadScriptProperties();
}

void Script::GatherScriptProperties()
{
#if LUA_ENABLED
    if (IsActive())
    {
        // Even if we had valid ScriptProps already (loaded from blueprint or level file),
        // we still want to re-gather the properties because some may have been added or deleted.
        mScriptProps.clear();

        lua_State* L = GetLua();
        lua_rawgeti(L, LUA_REGISTRYINDEX, mUserdataRef);

        if (lua_isuserdata(L, -1))
        {
            int udIdx = lua_gettop(L);

            lua_getfield(L, -1, "GatherProperties");

            if (lua_isfunction(L, -1))
            {
                lua_pushvalue(L, -2);   // arg1 - self
                LuaFuncCall(1, 1);

                if (lua_istable(L, -1))
                {
                    // GatherProperties func should return an array
                    // of tables. Each table in the array contains the
                    // Property name, data type, count
                    int arrayIdx = lua_gettop(L);

                    // Get number of properties.
                    lua_len(L, arrayIdx);
                    int32_t numProps = lua_tointeger(L, -1);
                    lua_pop(L, 1);

                    // Loop through each property
                    for (int32_t i = 1; i <= numProps; ++i)
                    {
                        lua_geti(L, arrayIdx, i);

                        if (lua_istable(L, -1))
                        {
                            int propIdx = lua_gettop(L);
                            Property newProp;
                            
#if EDITOR
                            newProp.mCategory = "Script";
#endif

                            lua_getfield(L, propIdx, "name");
                            const char* name = lua_isstring(L, -1) ? lua_tostring(L, -1) : "";
                            newProp.mName = name;
                            lua_pop(L, 1);

                            lua_getfield(L, propIdx, "type");
                            DatumType type = lua_isinteger(L, -1) ? (DatumType)lua_tointeger(L, -1) : DatumType::Count;
                            newProp.mType = type;
                            lua_pop(L, 1);

                            // In the future, possibly support "count", "minCount", and "maxCount"
                            //int32_t count = 1;
                            //lua_getfield(L, propIdx, "count");
                            //if (lua_isinteger(L, -1))
                            //{
                            //    count = lua_tointeger(L, -1);
                            //}
                            //lua_pop(L, 1);

                            bool isArray = false;
                            lua_getfield(L, propIdx, "array");
                            isArray = lua_toboolean(L, -1);
                            lua_pop(L, 1);

                            newProp.mOwner = this;
                            newProp.mExternal = false;
                            newProp.mChangeHandler = HandleScriptPropChange;

                            if (isArray)
                            {
                                newProp.MakeVector();
                            }

                            // Setup initial value and push it onto the outProps vector.
                            // Table and pointer datum types are not supported for script props.
                            if (newProp.mName != "" &&
                                type != DatumType::Count &&
                                type != DatumType::Table &&
                                type != DatumType::Pointer)
                            {
                                int32_t count = 1;
                                int tableIdx = -1;

                                if (isArray)
                                {
                                    lua_getfield(L, udIdx, name);

                                    if (!lua_istable(L, -1))
                                    {
                                        lua_pop(L, 1);
                                        lua_newtable(L);
                                        tableIdx = lua_gettop(L);

                                        // Push a copy of the arraytable on the stack so it can be used later.
                                        lua_pushvalue(L, -1);

                                        lua_setfield(L, udIdx, name);
                                    }
                                    else
                                    {
                                        tableIdx = lua_gettop(L);
                                    }

                                    lua_len(L, tableIdx);
                                    count = lua_tointeger(L, -1);
                                    lua_pop(L, 1);
                                }

                                for (int32_t i = 0; i < count; ++i)
                                {
                                    lua_getfield(L, isArray ? tableIdx : udIdx, name);

                                    if (lua_isnil(L, -1) &&
                                        type != DatumType::Asset)
                                    {
                                        // Pop nil
                                        lua_pop(L, 1);

                                        // Add a defaulted member to the table and leave the it on the stack
                                        // so that we can initialize the Property correctly later.
                                        switch (type)
                                        {
                                        case DatumType::Integer: lua_pushinteger(L, 0); break;
                                        case DatumType::Float: lua_pushnumber(L, 0.0f); break;
                                        case DatumType::Bool: lua_pushboolean(L, false); break;
                                        case DatumType::String: lua_pushstring(L, ""); break;
                                        case DatumType::Vector2D: Vector_Lua::Create(L, glm::vec2(0.0f, 0.0f)); break;
                                        case DatumType::Vector: Vector_Lua::Create(L, glm::vec3(0.0f, 0.0f, 0.0f)); break;
                                        case DatumType::Color: Vector_Lua::Create(L, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)); break;
                                        case DatumType::Byte: lua_pushinteger(L, 0); break;
                                        case DatumType::Short: lua_pushinteger(L, 0); break;


                                        default:
                                            lua_pushnil(L);
                                            break;
                                        }

                                        // Put a duplicate of the value on the stack so it will remain after setting the field.
                                        lua_pushvalue(L, -1);

                                        if (isArray)
                                        {
                                            lua_seti(L, tableIdx, int(i + 1));
                                        }
                                        else
                                        {
                                            lua_setfield(L, udIdx, name);
                                        }
                                    }

                                    switch (type)
                                    {
                                    case DatumType::Integer:
                                    {
                                        int32_t value = CHECK_INTEGER(L, -1);
                                        newProp.PushBack(value);
                                        break;
                                    }
                                    case DatumType::Float:
                                    {
                                        float value = CHECK_NUMBER(L, -1);
                                        newProp.PushBack(value);
                                        break;
                                    }
                                    case DatumType::Bool:
                                    {
                                        bool value = CHECK_BOOLEAN(L, -1);
                                        newProp.PushBack(value);
                                        break;
                                    }
                                    case DatumType::String:
                                    {
                                        const char* value = CHECK_STRING(L, -1);
                                        newProp.PushBack(value);
                                        break;
                                    }
                                    case DatumType::Vector2D:
                                    {
                                        glm::vec2 value = CHECK_VECTOR(L, -1);
                                        newProp.PushBack(value);
                                        break;
                                    }
                                    case DatumType::Vector:
                                    {
                                        glm::vec3 value = CHECK_VECTOR(L, -1);
                                        newProp.PushBack(value);
                                        break;
                                    }
                                    case DatumType::Color:
                                    {
                                        glm::vec4 value = CHECK_VECTOR(L, -1);
                                        newProp.PushBack(value);
                                        break;
                                    }
                                    case DatumType::Asset:
                                    {
                                        Asset* asset = nullptr;
                                        if (!lua_isnil(L, -1))
                                        {
                                            asset = CHECK_ASSET(L, -1);
                                        }
                                        newProp.PushBack(asset);
                                        break;
                                    }
                                    case DatumType::Byte:
                                    {
                                        int32_t value = CHECK_INTEGER(L, -1);
                                        newProp.PushBack((uint8_t)value);
                                        break;
                                    }

                                    case DatumType::Table:
                                    {
                                        LogError("Table script properties are not supported.");
                                        OCT_ASSERT(0);
                                        break;
                                    }

                                    case DatumType::Pointer:
                                    {
                                        LogError("Pointer script properties are not supported.");
                                        OCT_ASSERT(0);
                                        break;
                                    }

                                    case DatumType::Short:
                                    {
                                        int32_t value = CHECK_INTEGER(L, -1);
                                        newProp.PushBack((int16_t)value);
                                        break;
                                    }

                                    case DatumType::Function:
                                    {
                                        LogError("Function script properties are not supported.");
                                        OCT_ASSERT(0);
                                        break;
                                    }

                                    case DatumType::Count:
                                    {
                                        OCT_ASSERT(0); // Unreachable
                                        break;
                                    }
                                    }

                                    // Pop initial value
                                    lua_pop(L, 1);
                                }


                                mScriptProps.push_back(newProp);

                                if (tableIdx != -1)
                                {
                                    // pop the array table
                                    lua_pop(L, 1);
                                }
                            }
                            else
                            {
                                LogWarning("Invalid script property found.");
                            }
                        }

                        // Pop property object
                        lua_pop(L, 1);
                    }
                }

                // Pop GatherProperties() return value (hopefully a table)
                lua_pop(L, 1);
            }
            else
            {
                // Pop non-function value
                lua_pop(L, 1);
            }
        }

        // Pop node userdata
        lua_pop(L, 1);
    }
#endif
}

const std::vector<Property>& Script::GetScriptProperties() const
{
    return mScriptProps;
}

void Script::SetScriptProperties(const std::vector<Property>& srcProps)
{
    CopyPropertyValues(mScriptProps, srcProps);
}

void Script::GatherReplicatedData()
{
#if LUA_ENABLED
    if (IsActive())
    {
        // Even if we had valid ScriptProps already (loaded from blueprint or level file),
        // we still want to re-gather the properties because some may have been added or deleted.
        mReplicatedData.clear();

        bool isServer = NetIsServer();

        lua_State* L = GetLua();
        lua_rawgeti(L, LUA_REGISTRYINDEX, mUserdataRef);
        if (lua_isuserdata(L, -1))
        {
            int udIdx = lua_gettop(L);

            lua_getfield(L, -1, "GatherReplicatedData");

            if (lua_isfunction(L, -1))
            {
                lua_pushvalue(L, -2);   // arg1 - self
                LuaFuncCall(1, 1);

                if (lua_istable(L, -1))
                {
                    // GatherReplicatedData func should return an array
                    // of tables. Each table in the array contains the
                    // NetDatum name, data type, count
                    int arrayIdx = lua_gettop(L);

                    // Get number of properties.
                    lua_len(L, arrayIdx);
                    int32_t numProps = lua_tointeger(L, -1);
                    lua_pop(L, 1);

                    // Loop through each property
                    for (int32_t i = 1; i <= numProps; ++i)
                    {
                        lua_geti(L, arrayIdx, i);

                        if (lua_istable(L, -1))
                        {
                            int propIdx = lua_gettop(L);
                            ScriptNetDatum newDatum;

                            lua_getfield(L, propIdx, "name");
                            const char* name = lua_isstring(L, -1) ? lua_tostring(L, -1) : "";
                            newDatum.mVarName = name;
                            lua_pop(L, 1);

                            lua_getfield(L, propIdx, "type");
                            DatumType type = lua_isinteger(L, -1) ? (DatumType)lua_tointeger(L, -1) : DatumType::Count;
                            newDatum.mType = type;
                            lua_pop(L, 1);

                            lua_getfield(L, propIdx, "onRep");
                            const char* onRep = lua_isstring(L, -1) ? lua_tostring(L, -1) : "";
                            newDatum.mOnRepFuncName = onRep;
                            lua_pop(L, 1);

                            // TODO: Handle array data
                            //lua_getfield(L, propIdx, "count");
                            //int32_t count= lua_isinteger(L, -1) ? lua_tointeger(L, -1) : 1;

                            newDatum.mOwner = this;
                            newDatum.mExternal = false;
                            newDatum.mChangeHandler = isServer ? nullptr : OnRepHandler;

                            // Setup initial value and push it onto the outProps vector.
                            if (newDatum.mVarName != "" &&
                                type != DatumType::Count)
                            {
                                lua_getfield(L, udIdx, name);

                                if (lua_isnil(L, -1) &&
                                    type != DatumType::Asset)
                                {
                                    // Pop nil
                                    lua_pop(L, 1);

                                    // Add a defaulted member to the table and leave the it on the stack
                                    // so that we can initialize the Property correctly later.
                                    switch (type)
                                    {
                                    case DatumType::Integer: lua_pushinteger(L, 0); break;
                                    case DatumType::Float: lua_pushnumber(L, 0.0f); break;
                                    case DatumType::Bool: lua_pushboolean(L, false); break;
                                    case DatumType::String: lua_pushstring(L, ""); break;
                                    case DatumType::Vector2D: Vector_Lua::Create(L, glm::vec2(0.0f, 0.0f)); break;
                                    case DatumType::Vector: Vector_Lua::Create(L, glm::vec3(0.0f, 0.0f, 0.0f)); break;
                                    case DatumType::Color: Vector_Lua::Create(L, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)); break;
                                    case DatumType::Byte: lua_pushinteger(L, 0); break;
                                    case DatumType::Short: lua_pushinteger(L, 0); break;

                                    default:
                                        lua_pushnil(L);
                                        break;
                                    }

                                    // Put a duplicate of the value on the stack so it will remain after setting the field.
                                    lua_pushvalue(L, -1);

                                    lua_setfield(L, udIdx, name);
                                }

                                bool push = true;

                                switch (type)
                                {
                                case DatumType::Integer:
                                {
                                    int32_t value = CHECK_INTEGER(L, -1);
                                    newDatum.PushBack(value);
                                    break;
                                }
                                case DatumType::Float:
                                {
                                    float value = CHECK_NUMBER(L, -1);
                                    newDatum.PushBack(value);
                                    break;
                                }
                                case DatumType::Bool:
                                {
                                    bool value = CHECK_BOOLEAN(L, -1);
                                    newDatum.PushBack(value);
                                    break;
                                }
                                case DatumType::String:
                                {
                                    const char* value = CHECK_STRING(L, -1);
                                    newDatum.PushBack(value);
                                    break;
                                }
                                case DatumType::Vector2D:
                                {
                                    glm::vec2 value = CHECK_VECTOR(L, -1);
                                    newDatum.PushBack(value);
                                    break;
                                }
                                case DatumType::Vector:
                                {
                                    glm::vec3 value = CHECK_VECTOR(L, -1);
                                    newDatum.PushBack(value);
                                    break;
                                }
                                case DatumType::Color:
                                {
                                    glm::vec4 value = CHECK_VECTOR(L, -1);
                                    newDatum.PushBack(value);
                                    break;
                                }
                                case DatumType::Asset:
                                {
                                    Asset* asset = nullptr;
                                    if (!lua_isnil(L, -1))
                                    {
                                        asset = CHECK_ASSET(L, -1);
                                    }
                                    newDatum.PushBack(asset);
                                    break;
                                }
                                case DatumType::Byte:
                                {
                                    int32_t value = CHECK_INTEGER(L, -1);
                                    newDatum.PushBack((uint8_t)value);
                                    break;
                                }

                                case DatumType::Table:
                                {
                                    push = false;
                                    LogError("Table replicated data is not supported.");
                                    break;
                                }

                                case DatumType::Pointer:
                                {
                                    // Only actor pointers are supported right now.
                                    Node* nodePointer = CHECK_NODE(L, -1);
                                    newDatum.PushBack(nodePointer);
                                    break;
                                }

                                case DatumType::Short:
                                {
                                    int32_t value = CHECK_INTEGER(L, -1);
                                    newDatum.PushBack((int16_t)value);
                                    break;
                                }

                                case DatumType::Function:
                                {
                                    push = false;
                                    LogError("Function replicated data is not supported.");
                                    break;
                                }

                                case DatumType::Count:
                                {
                                    push = false;
                                    OCT_ASSERT(0); // Unreachable
                                    break;
                                }
                                }

                                if (push)
                                {
                                    mReplicatedData.push_back(newDatum);
                                }

                                // Pop initial value
                                lua_pop(L, 1);
                            }
                            else
                            {
                                LogWarning("Invalid script net datum found.");
                            }
                        }

                        // Pop property object
                        lua_pop(L, 1);
                    }
                }

                // Pop GatherReplicatedData() return value (hopefully a table or nil)
                lua_pop(L, 1);
            }
            else
            {
                // Pop non-function value
                lua_pop(L, 1);
            }
        }

        // Pop userdata
        lua_pop(L, 1);
    }
#endif
}

void Script::RegisterNetFuncs()
{
    if (sScriptNetFuncMap.find(mClassName) == sScriptNetFuncMap.end())
    {
        // This script hasn't had its NetFuncs registered yet.
        sScriptNetFuncMap.insert({ mClassName, std::unordered_map<std::string, ScriptNetFunc>() });
        std::unordered_map<std::string, ScriptNetFunc>& funcMap = sScriptNetFuncMap[mClassName];

        std::vector<ScriptNetFunc> netFuncs;
        GatherNetFuncs(netFuncs);

        for (uint32_t i = 0; i < netFuncs.size(); ++i)
        {
            netFuncs[i].mIndex = (uint16_t)i;
            funcMap.insert({ netFuncs[i].mName, netFuncs[i] });
        }

        LogDebug("Registered %d net functions for script %s", int32_t(netFuncs.size()), mClassName.c_str());
    }
}

void Script::GatherNetFuncs(std::vector<ScriptNetFunc>& outFuncs)
{
#if LUA_ENABLED
    if (IsActive())
    {
        lua_State* L = GetLua();
        lua_rawgeti(L, LUA_REGISTRYINDEX, mUserdataRef);

        if (lua_isuserdata(L, -1))
        {
            int udIdx = lua_gettop(L);

            lua_getfield(L, -1, "GatherNetFuncs");

            if (lua_isfunction(L, -1))
            {
                lua_pushvalue(L, udIdx);   // arg1 - self
                LuaFuncCall(1, 1);

                if (lua_istable(L, -1))
                {
                    // GatherNetFuncs func should return an array
                    // of tables. Each table in the array contains the
                    // function name, and net function type
                    int arrayIdx = lua_gettop(L);

                    // Get number of properties.
                    lua_len(L, arrayIdx);
                    int32_t numFuncs = lua_tointeger(L, -1);
                    lua_pop(L, 1);

                    // Loop through each NetFunc def
                    for (int32_t i = 1; i <= numFuncs; ++i)
                    {
                        lua_geti(L, arrayIdx, i);

                        if (lua_istable(L, -1))
                        {
                            int defIdx = lua_gettop(L);
                            ScriptNetFunc newFunc;

                            lua_getfield(L, defIdx, "name");
                            const char* name = lua_isstring(L, -1) ? lua_tostring(L, -1) : "";
                            newFunc.mName = name;
                            lua_pop(L, 1);

                            lua_getfield(L, defIdx, "type");
                            NetFuncType type = lua_isinteger(L, -1) ? (NetFuncType)lua_tointeger(L, -1) : NetFuncType::Count;
                            newFunc.mType = type;
                            lua_pop(L, 1);

                            lua_getfield(L, defIdx, "reliable");
                            bool reliable = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;
                            newFunc.mReliable = reliable;
                            lua_pop(L, 1);

                            if (newFunc.mName != "" &&
                                newFunc.mType != NetFuncType::Count)
                            {
                                outFuncs.push_back(newFunc);
                            }
                        }

                        // Pop table describing NetFunc
                        lua_pop(L, 1);
                    }
                }

                // Pop GatherReplicatedData() return value (hopefully a table or nil)
                lua_pop(L, 1);
            }
            else
            {
                // Pop non-function value
                lua_pop(L, 1);
            }

        }

        // Pop script instance table
        lua_pop(L, 1);
    }
#endif
}

void Script::DownloadReplicatedData()
{
#if LUA_ENABLED
    lua_State* L = GetLua();

    if (IsActive())
    {
        lua_rawgeti(L, LUA_REGISTRYINDEX, mUserdataRef);

        OCT_ASSERT(lua_isuserdata(L, -1));
        int udIdx = lua_gettop(L);

        for (uint32_t i = 0; i < mReplicatedData.size(); ++i)
        {
            DownloadDatum(L, mReplicatedData[i], udIdx, mReplicatedData[i].mVarName.c_str());
        }

        // Pop script instance table
        lua_pop(L, 1);
    }
#endif
}

ScriptNetFunc* Script::FindNetFunc(const char* funcName)
{
    ScriptNetFunc* retFunc = nullptr;

    auto mapIt = sScriptNetFuncMap.find(mClassName);
    if (mapIt != sScriptNetFuncMap.end())
    {
        auto funcIt = mapIt->second.find(funcName);
        if (funcIt != mapIt->second.end())
        {
            retFunc = &funcIt->second;
        }
        else
        {
            LogError("Can't find NetFunc %s on Script %s", funcName, mClassName.c_str());
        }
    }

    return retFunc;
}

ScriptNetFunc* Script::FindNetFunc(uint16_t index)
{
    ScriptNetFunc* retFunc = nullptr;

    auto mapIt = sScriptNetFuncMap.find(mClassName);

    if (mapIt != sScriptNetFuncMap.end())
    {
        for (auto funcIt = mapIt->second.begin(); funcIt != mapIt->second.end(); ++funcIt)
        {
            if (funcIt->second.mIndex == index)
            {
                retFunc = &funcIt->second;
                break;
            }
        }
    }

    return retFunc;
}

void Script::ExecuteNetFunc(uint16_t index, uint32_t numParams, const Datum** params)
{
#if LUA_ENABLED
    if (IsActive())
    {
        lua_State* L = GetLua();
        ScriptNetFunc* netFunc = FindNetFunc(index);

        if (netFunc != nullptr)
        {
            lua_rawgeti(L, LUA_REGISTRYINDEX, mUserdataRef);

            if (lua_isuserdata(L, -1))
            {
                int udIdx = lua_gettop(L);
                lua_getfield(L, udIdx, netFunc->mName.c_str());

                if (lua_isfunction(L, -1))
                {
                    lua_pushvalue(L, udIdx);     // arg1 - self

                    for (uint32_t i = 0; i < numParams; ++i)
                    {
                        LuaPushDatum(L, *params[i]);
                    }

                    uint32_t totalArgCount = 1 + numParams;
                    LuaFuncCall(totalArgCount);
                }
                else
                {
                    // Pop non-function value
                    lua_pop(L, 1);
                }
            }

            // Pop userdata
            lua_pop(L, 1);
        }
        else
        {
            LogWarning("Can't find NetFunc[%d] on script %s", index, mClassName.c_str());
        }
    }
#endif
}

bool Script::DownloadDatum(lua_State* L, Datum& datum, int udIdx, const char* varName)
{
    bool success = true;

#if LUA_ENABLED
    lua_getfield(L, udIdx, varName);

    if (!lua_isnil(L, -1))
    {
        switch (datum.GetType())
        {
        case DatumType::Integer:
        {
            int32_t value = CHECK_INTEGER(L, -1);
            datum.SetInteger(value);
            break;
        }
        case DatumType::Float:
        {
            float value = CHECK_NUMBER(L, -1);
            datum.SetFloat(value);
            break;
        }
        case DatumType::Bool:
        {
            bool value = CHECK_BOOLEAN(L, -1);
            datum.SetBool(value);
            break;
        }
        case DatumType::String:
        {
            const char* value = CHECK_STRING(L, -1);
            datum.SetString(value);
            break;
        }
        case DatumType::Vector2D:
        {
            glm::vec2 value = CHECK_VECTOR(L, -1);
            datum.SetVector2D(value);
            break;
        }
        case DatumType::Vector:
        {
            glm::vec3 value = CHECK_VECTOR(L, -1);
            datum.SetVector(value);
            break;
        }
        case DatumType::Color:
        {
            glm::vec4 value = CHECK_VECTOR(L, -1);
            datum.SetColor(value);
            break;
        }
        case DatumType::Asset:
        {
            Asset* asset = nullptr;
            if (!lua_isnil(L, -1))
            {
                asset = CHECK_ASSET(L, -1);
            }
            datum.SetAsset(asset);
            break;
        }
        case DatumType::Byte:
        {
            int32_t value = CHECK_INTEGER(L, -1);
            datum.SetByte((uint8_t)value);
            break;
        }

        case DatumType::Table:
        {
            success = false;
            LogError("Table script properties are not supported.");
            break;
        }

        case DatumType::Pointer:
        {
            success = false;
            LogError("Pointer script properties are not supported.");
            break;
        }

        case DatumType::Short:
        {
            int32_t value = CHECK_INTEGER(L, -1);
            datum.SetShort((int16_t)value);
            break;
        }

        case DatumType::Function:
        {
            success = false;
            LogError("Function script properties are not supported.");
            break;
        }

        case DatumType::Count:
        {
            success = false;
            OCT_ASSERT(0); // Unreachable
            break;
        }
        }
    }
    else
    {
        LogWarning("Script var [%s] not found in DownloadDatum()", varName);
    }

    // Pop the grabbed field.
    lua_pop(L, 1);
#endif

    return success;
}

void Script::UploadDatum(Datum& datum, const char* varName)
{
#if LUA_ENABLED
    lua_State* L = GetLua();
    lua_rawgeti(L, LUA_REGISTRYINDEX, mUserdataRef);

    if (lua_isuserdata(L, -1))
    {
        int udIdx = lua_gettop(L);
        int arrayTableIdx = -1;
        uint32_t count = datum.GetCount();

        if (datum.IsProperty() &&
            static_cast<Property&>(datum).IsVector())
        {
            lua_getfield(L, udIdx, varName);
            arrayTableIdx = lua_gettop(L);
        }

        for (uint32_t i = 0; i < count; ++i)
        {
            // Push the value we want to update, dependent on the datum type.
            switch (datum.mType)
            {
            case DatumType::Integer: lua_pushinteger(L, datum.GetInteger(i)); break;
            case DatumType::Float: lua_pushnumber(L, datum.GetFloat(i)); break;
            case DatumType::Bool: lua_pushboolean(L, datum.GetBool(i)); break;
            case DatumType::String: lua_pushstring(L, datum.GetString(i).c_str()); break;
            case DatumType::Vector2D: Vector_Lua::Create(L, datum.GetVector2D(i)); break;
            case DatumType::Vector: Vector_Lua::Create(L, datum.GetVector(i)); break;
            case DatumType::Color: Vector_Lua::Create(L, datum.GetColor(i)); break;
            case DatumType::Asset: Asset_Lua::Create(L, datum.GetAsset(i)); break;
            case DatumType::Byte: lua_pushinteger(L, (int32_t)datum.GetByte(i)); break;
            case DatumType::Short: lua_pushinteger(L, (int32_t)datum.GetShort(i)); break;

            case DatumType::Table:
            case DatumType::Pointer:
            case DatumType::Function:
            case DatumType::Count:
                // These datum types are not supported.
                OCT_ASSERT(0);
                break;
            }

            if (arrayTableIdx != -1)
            {
                lua_seti(L, arrayTableIdx, i + 1);
            }
            else
            {
                lua_setfield(L, udIdx, varName);
            }
        }

        if (arrayTableIdx != -1)
        {
            // Set value at count + 1 to nil
            lua_pushnil(L);
            lua_seti(L, arrayTableIdx, count + 1);

            // Pop array table.
            lua_pop(L, 1);
        }
    }

    // Pop instance table
    lua_pop(L, 1);
#endif
}

void Script::SetFile(const char* filename)
{
    mFileName = filename;
}

const std::string& Script::GetFile() const
{
    return mFileName;
}

const std::string& Script::GetScriptClassName() const
{
    return mClassName;
}

void Script::StartScript()
{
    if (!IsActive())
    {
        CreateScriptInstance();
    }
}

void Script::RestartScript()
{
    DestroyScriptInstance();
    CreateScriptInstance();
}

void Script::StopScript()
{
    DestroyScriptInstance();
}

bool Script::IsActive() const
{
    return (mUserdataRef != LUA_REFNIL);
}

void Script::SetWorld(World* world)
{
    if (IsActive())
    {
        lua_State* L = GetLua();
        lua_rawgeti(L, LUA_REGISTRYINDEX, mUserdataRef);
        World_Lua::Create(L, world);
        lua_setfield(L, -2, "world");
    }
}

bool Script::ReloadScriptFile(const std::string& fileName, bool restartScript)
{
    bool success = ScriptUtils::ReloadScriptFile(fileName);

    std::string className = ScriptUtils::GetClassNameFromFileName(fileName);

    auto it = sScriptNetFuncMap.find(className);
    if (it != sScriptNetFuncMap.end())
    {
        sScriptNetFuncMap.erase(it);
    }

    if (success && restartScript)
    {
        RestartScript();
    }

    return success;
}

std::vector<ScriptNetDatum>& Script::GetReplicatedData()
{
    return mReplicatedData;
}

bool Script::InvokeNetFunc(const char* name, uint32_t numParams, const Datum** params)
{
    ScriptNetFunc* netFunc = FindNetFunc(name);
    Node* node = GetOwner();
    bool validNetFunc = false;

    if (netFunc != nullptr)
    {
        validNetFunc = true;

        OCT_ASSERT(numParams <= MAX_NET_FUNC_PARAMS);
        const Datum* paramArray[MAX_NET_FUNC_PARAMS];
        for (uint32_t i = 0; i < MAX_NET_FUNC_PARAMS; ++i)
        {
            if (i < numParams)
            {
                // I should probably rework the rest of this code so that I don't have to const cast here.
                paramArray[i] = const_cast<Datum*>(params[i]);
            }
            else
            {
                paramArray[i] = nullptr;
            }
        }

        if (ShouldSendNetFunc(netFunc->mType, node))
        {
            // Local execution is handled in SendInvokeScriptMsg
            NetworkManager::Get()->SendInvokeScriptMsg(this, netFunc, numParams, paramArray);
        }

        if (ShouldExecuteNetFunc(netFunc->mType, node))
        {
            ExecuteNetFunc(netFunc->mIndex, numParams, params);
        }
    }

    return validNetFunc;
}

void Script::BeginOverlap(Primitive3D* thisNode, Primitive3D* otherNode)
{
#if LUA_ENABLED
    if (mHandleBeginOverlap && IsActive())
    {
        lua_State* L = GetLua();

        // Grab the ud
        lua_rawgeti(L, LUA_REGISTRYINDEX, mUserdataRef);

        OCT_ASSERT(lua_isuserdata(L, -1));
        int udIdx = lua_gettop(L);
        lua_getfield(L, udIdx, "BeginOverlap");

        if (lua_isfunction(L, -1))
        {
            lua_pushvalue(L, udIdx);
            Node_Lua::Create(L, thisNode);
            Node_Lua::Create(L, otherNode);

            // Instance table at -5
            // Func at -4
            // Instance table (as arg1) at -3
            // thisComp (as arg2) at -2
            // othercomp as (arg3) at -1
            LuaFuncCall(3);
        }
        else
        {
            lua_pop(L, 1); // pop nil function
        }

        lua_pop(L, 1); // pop instance table
    }
#endif
}

void Script::EndOverlap(Primitive3D* thisNode, Primitive3D* otherNode)
{
#if LUA_ENABLED
    if (mHandleEndOverlap && IsActive())
    {
        lua_State* L = GetLua();

        lua_rawgeti(L, LUA_REGISTRYINDEX, mUserdataRef);

        OCT_ASSERT(lua_isuserdata(L, -1));
        int udIdx = lua_gettop(L);
        lua_getfield(L, udIdx, "EndOverlap");

        if (lua_isfunction(L, -1))
        {
            lua_pushvalue(L, udIdx);
            Node_Lua::Create(L, thisNode);
            Node_Lua::Create(L, otherNode);

            // Instance table at -5
            // Func at -4
            // Instance table (as arg1) at -3
            // thisNode (as arg2) at -2
            // otherNode as (arg3) at -1
            LuaFuncCall(3);
        }
        else
        {
            lua_pop(L, 1); // pop nil function
        }

        lua_pop(L, 1); // pop instance table
    }
#endif
}

void Script::OnCollision(
    Primitive3D* thisNode,
    Primitive3D* otherNode,
    glm::vec3 impactPoint,
    glm::vec3 impactNormal,
    btPersistentManifold* manifold)
{
#if LUA_ENABLED
    if (mHandleOnCollision && IsActive())
    {
        lua_State* L = GetLua();

        lua_rawgeti(L, LUA_REGISTRYINDEX, mUserdataRef);
        OCT_ASSERT(lua_isuserdata(L, -1));
        int udIdx = lua_gettop(L);
        lua_getfield(L, udIdx, "OnCollision");

        if (lua_isfunction(L, -1))
        {
            lua_pushvalue(L, udIdx);                                // arg1 - self
            Node_Lua::Create(L, thisNode);                          // arg2 - thisNode
            Node_Lua::Create(L, otherNode);                         // arg3 - otherNode
            Vector_Lua::Create(L, glm::vec4(impactPoint, 0.0f));    // arg4 - impactPoint
            Vector_Lua::Create(L, glm::vec4(impactNormal, 0.0f));   // arg5 - impactNormal
            // TODO: Do we want to handle manifold points?

            LuaFuncCall(5);
        }
        else
        {
            lua_pop(L, 1); // pop nil function
        }

        lua_pop(L, 1); // pop instance table
    }
#endif
}

bool Script::HasFunction(const char* name) const
{
    bool ret = false;

    if (IsActive())
    {
        lua_State* L = GetLua();

        lua_rawgeti(L, LUA_REGISTRYINDEX, mUserdataRef);
        OCT_ASSERT(lua_isuserdata(L, -1));
        lua_getfield(L, -1, name);

        // Only call the function if it has been defined.
        if (lua_isfunction(L, -1))
        {
            ret = true;
        }

        // Pop userdata and function
        lua_pop(L, 2);
    }

    return ret;
}

// These functions are kinda nasty, but it's a nice convenience for the game programmer
void Script::CallFunction(const char* name)
{
    CallFunction(name, 0, nullptr, nullptr);
}

void Script::CallFunction(const char* name, const Datum& param0)
{
    const Datum* params[] = { &param0 };
    CallFunction(name, 1, params, nullptr);
}

void Script::CallFunction(const char* name, const Datum& param0, const Datum& param1)
{
    const Datum* params[] = { &param0, &param1 };
    CallFunction(name, 2, params, nullptr);
}

void Script::CallFunction(const char* name, const Datum& param0, const Datum& param1, const Datum& param2)
{
    const Datum* params[] = { &param0, &param1, &param2 };
    CallFunction(name, 3, params, nullptr);
}

void Script::CallFunction(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3)
{
    const Datum* params[] = { &param0, &param1, &param2, &param3 };
    CallFunction(name, 4, params, nullptr);
}

void Script::CallFunction(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3, const Datum& param4)
{
    const Datum* params[] = { &param0, &param1, &param2, &param3, &param4 };
    CallFunction(name, 5, params, nullptr);
}

void Script::CallFunction(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3, const Datum& param4, const Datum& param5)
{
    const Datum* params[] = { &param0, &param1, &param2, &param3, &param4, &param5 };
    CallFunction(name, 6, params, nullptr);
}

void Script::CallFunction(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3, const Datum& param4, const Datum& param5, const Datum& param6)
{
    const Datum* params[] = { &param0, &param1, &param2, &param3, &param4, &param5, &param6 };
    CallFunction(name, 7, params, nullptr);
}

void Script::CallFunction(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3, const Datum& param4, const Datum& param5, const Datum& param6, const Datum& param7)
{
    const Datum* params[] = { &param0, &param1, &param2, &param3, &param4, &param5, &param6, &param7 };
    CallFunction(name, 8, params, nullptr);
}

Datum Script::CallFunctionR(const char* name)
{
    Datum retDatum;
    CallFunction(name, 0, nullptr, &retDatum);
    return retDatum;
}

Datum Script::CallFunctionR(const char* name, const Datum& param0)
{
    Datum retDatum;
    const Datum* params[] = { &param0 };
    CallFunction(name, 1, params, &retDatum);
    return retDatum;
}

Datum Script::CallFunctionR(const char* name, const Datum& param0, const Datum& param1)
{
    Datum retDatum;
    const Datum* params[] = { &param0, &param1 };
    CallFunction(name, 2, params, &retDatum);
    return retDatum;
}

Datum Script::CallFunctionR(const char* name, const Datum& param0, const Datum& param1, const Datum& param2)
{
    Datum retDatum;
    const Datum* params[] = { &param0, &param1, &param2 };
    CallFunction(name, 3, params, &retDatum);
    return retDatum;
}

Datum Script::CallFunctionR(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3)
{
    Datum retDatum;
    const Datum* params[] = { &param0, &param1, &param2, &param3 };
    CallFunction(name, 4, params, &retDatum);
    return retDatum;
}

Datum Script::CallFunctionR(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3, const Datum& param4)
{
    Datum retDatum;
    const Datum* params[] = { &param0, &param1, &param2, &param3, &param4 };
    CallFunction(name, 5, params, &retDatum);
    return retDatum;
}

Datum Script::CallFunctionR(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3, const Datum& param4, const Datum& param5)
{
    Datum retDatum;
    const Datum* params[] = { &param0, &param1, &param2, &param3, &param4, &param5 };
    CallFunction(name, 6, params, &retDatum);
    return retDatum;
}

Datum Script::CallFunctionR(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3, const Datum& param4, const Datum& param5, const Datum& param6)
{
    Datum retDatum;
    const Datum* params[] = { &param0, &param1, &param2, &param3, &param4, &param5, &param6 };
    CallFunction(name, 7, params, &retDatum);
    return retDatum;
}

Datum Script::CallFunctionR(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3, const Datum& param4, const Datum& param5, const Datum& param6, const Datum& param7)
{
    Datum retDatum;
    const Datum* params[] = { &param0, &param1, &param2, &param3, &param4, &param5, &param6, &param7 };
    CallFunction(name, 8, params, &retDatum);
    return retDatum;
}

void Script::CallFunction(const char* name, uint32_t numParams, const Datum** params, Datum* ret)
{
    if (IsActive())
    {
        ScriptUtils::CallMethod(mUserdataRef, name, numParams, params, ret);
    }
}

Datum Script::GetField(const char* key)
{
    Datum ret;

    if (IsActive())
    {
        ret = ScriptUtils::GetField(mUserdataRef, key);
    }

    return ret;
}


void Script::SetField(const char* key, const Datum& value)
{
    if (IsActive())
    {
        ScriptUtils::SetField(mUserdataRef, key, value);
    }
}


bool Script::OnRepHandler(Datum* datum, uint32_t index, const void* newValue)
{
#if LUA_ENABLED
    ScriptNetDatum* netDatum = (ScriptNetDatum*)datum;
    Script* script = static_cast<Script*>(netDatum->mOwner);

    lua_State* L = GetLua();

    OCT_ASSERT(!NetIsAuthority());
    bool onRepFunc = (netDatum->mOnRepFuncName != "");

    int oldValueIdx = 0;

    if (onRepFunc)
    {
        LuaPushDatum(L, *netDatum);
        oldValueIdx = lua_gettop(L);
    }

    netDatum->SetValueRaw(newValue);
    script->UploadDatum(*netDatum, netDatum->mVarName.c_str());

    if (onRepFunc)
    {
        lua_rawgeti(L, LUA_REGISTRYINDEX, script->mUserdataRef);

        OCT_ASSERT(lua_isuserdata(L, -1));
        int udIdx = lua_gettop(L);
        lua_getfield(L, udIdx, netDatum->mOnRepFuncName.c_str());

        if (lua_isfunction(L, -1))
        {
            // (2) If the func is valid, call it and pass the new value
            lua_pushvalue(L, udIdx);         // arg1 - self
            lua_pushvalue(L, oldValueIdx);      // arg2 - old value

            netDatum->SetValueRaw(newValue);

            script->LuaFuncCall(2, 0);
        }
        else
        {
            // (3) If there is no onRep func or it's invalid, then upload the new datum value.
            LogWarning("OnRep function %s not found.", netDatum->mOnRepFuncName.c_str());
        }

        // Pop script instance table and oldValue
        lua_pop(L, 2);
    }
#endif

    return true;
}

void Script::CreateScriptInstance()
{
#if LUA_ENABLED

    lua_State* L = GetLua();

    // If mTableName is set, that means CreateScriptInstance is being called
    // before the previous instance was destroyed with DestroyScriptInstance.
    if (!IsActive() &&
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
            // Create a new userdata if the node previously didn't have one.
            Node_Lua::Create(L, mOwner);
            int udIdx = lua_gettop(L);

            mUserdataRef = mOwner->GetUserdataRef();
            // The userdata ref should be assigned in Node_Lua::Create().
            OCT_ASSERT(mUserdataRef != LUA_REFNIL);

            lua_getuservalue(L, udIdx);
            int uvIdx = lua_gettop(L);
            OCT_ASSERT(lua_istable(L, uvIdx)); // Should be created and assigned in Node_Lua::Create()

            // Retrieve the prototype class table
            lua_getglobal(L, mClassName.c_str());
            OCT_ASSERT(lua_istable(L, -1));
            int classTableIdx = lua_gettop(L);

            if (lua_getmetatable(L, classTableIdx) == 0)
            {
                LogDebug("Auto-parenting %s to %s.", mClassName.c_str(), mOwner->GetClassName());

                luaL_getmetatable(L, mOwner->GetClassName());
                if (lua_isnil(L, -1))
                {
                    LogError("Bad native metatable in CreateScriptInstance().");
                    lua_pop(L, 1);
                    luaL_getmetatable(L, NODE_LUA_NAME);
                }

                lua_setmetatable(L, classTableIdx); // Pops native metatable
            }
            else
            {
                // Ensure that the script class derives from the native node.
                char classFlag[64];
                snprintf(classFlag, 64, "cf%s", mOwner->GetClassName());
                lua_getfield(L, classTableIdx, classFlag);

                bool hasFlag = !lua_isnil(L, -1);
                
                if (!hasFlag)
                {
                    LogError("Bad inheritance chain! Make sure the script class table inherits (eventually) from the native node it is being used on.");
                    OCT_ASSERT(0);
                }

                lua_pop(L, 2); // Pop class flag + metatable
            }

            // Assign the new script class table to the Node Uservalue's class key
            OCT_ASSERT(lua_gettop(L) == classTableIdx);
            lua_setfield(L, uvIdx, OCT_CLASS_TABLE_KEY); // Pops script class metatable

            mTickEnabled = CheckIfFunctionExists("Tick");
            mHandleBeginOverlap = CheckIfFunctionExists("BeginOverlap");
            mHandleEndOverlap = CheckIfFunctionExists("EndOverlap");
            mHandleOnCollision = CheckIfFunctionExists("OnCollision");

            SetWorld(mOwner->GetWorld());

            // Is calling Create() here causing issues? It used to be called after gathering properties.
            // If this causes a problem, consider calling a separate function like PreCreate() or Init() or something.
            CallFunction("Create");

            UploadScriptProperties();
            GatherScriptProperties();

            if (GetOwner()->IsReplicated())
            {
                GatherReplicatedData();
                RegisterNetFuncs();
            }

            // Create used to be called here, but then we couldn't assign default values in Create().
            //CallFunction("Create");
        }
        else
        {
            LogError("Failed to create script instance. Class has not been loaded.");
        }
    }
#endif
}

void Script::DestroyScriptInstance()
{
#if LUA_ENABLED
    if (IsActive())
    {
        // Erase this instance so that it gets garbage collected.
        lua_State* L = GetLua();
        if (L != nullptr)
        {
            CallFunction("Destroy");

            // Clear the actor and component fields of the table in case anything else tries to access it.
            // Also set a destroyed field that can be queried.
            lua_rawgeti(L, LUA_REGISTRYINDEX, mUserdataRef);
            OCT_ASSERT(lua_isuserdata(L, -1));
            int udIdx = lua_gettop(L);

            lua_getuservalue(L, udIdx);
            int uvIdx = lua_gettop(L);
            OCT_ASSERT(lua_istable(L, uvIdx)); // Should be created and assigned in Node_Lua::Create()

            // Reset the uservalue class table to the native node's class.
            luaL_getmetatable(L, mOwner->GetClassName());
            if (lua_isnil(L, -1))
            {
                LogError("Bad native metatable in DestroyScriptInstance().");
                lua_pop(L, 1);
                luaL_getmetatable(L, NODE_LUA_NAME);
            }

            OCT_ASSERT(lua_istable(L, -1));
            lua_setfield(L, uvIdx, OCT_CLASS_TABLE_KEY);

            lua_pop(L, 1); // Pop userdata
        }

        mUserdataRef = LUA_REFNIL;
        mClassName = "";

        mScriptProps.clear();
        mReplicatedData.clear();
    }

    mTickEnabled = false;
    mHandleBeginOverlap = false;
    mHandleEndOverlap = false;
    mHandleOnCollision = false;
#endif
}

bool Script::LuaFuncCall(int numArgs, int numResults)
{
    bool success = true;
    success = ScriptUtils::CallLuaFunc(numArgs, numResults);
    return success;
}

void Script::CallTick(float deltaTime)
{
#if LUA_ENABLED

#if EDITOR
    if (IsActive())
    {
        if (IsGameTickEnabled())
        {
            CallFunction("Tick", deltaTime);
        }
        else
        {
            CallFunction("EditorTick", deltaTime);
        }
    }
#else
    if (IsActive() && mTickEnabled)
    {
        lua_State* L = GetLua();

        lua_rawgeti(L, LUA_REGISTRYINDEX, mUserdataRef);
        OCT_ASSERT(lua_isuserdata(L, -1));

        lua_getfield(L, -1, "Tick");

        if (lua_isfunction(L, -1))
        {
            lua_pushvalue(L, -2);
            lua_pushnumber(L, deltaTime);

            // Instance table at -4
            // Func at -3
            // Instance table (as arg0) at -2
            // deltaTime as (arg1) at -1
            LuaFuncCall(2);
        }
        else
        {
            lua_pop(L, 1);
        }

        lua_pop(L, 1);
    }
#endif // EDITOR

#endif // LUA_ENABLED

}

bool Script::CheckIfFunctionExists(const char* funcName)
{
    bool exists = false;

#if LUA_ENABLED
    if (IsActive())
    {
        lua_State* L = GetLua();

        lua_rawgeti(L, LUA_REGISTRYINDEX, mUserdataRef);
        OCT_ASSERT(lua_isuserdata(L, -1));

        lua_getfield(L, -1, funcName);

        if (lua_isfunction(L, -1))
        {
            exists = true;
        }

        lua_pop(L, 2);
    }
#endif

    return exists;
}


