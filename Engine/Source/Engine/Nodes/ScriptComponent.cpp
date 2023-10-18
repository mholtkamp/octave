#include "Nodes/3D/ScriptComponent.h"
#include "Nodes/3D/Primitive3d.h"
#include "Nodes/3D/SkeletalMesh3d.h"
#include "Constants.h"
#include "Assets/SkeletalMesh.h"
#include "Engine.h"
#include "Log.h"

#include "Widgets/Button.h"
#include "Widgets/Selector.h"
#include "Widgets/TextField.h"

#include "LuaBindings/LuaUtils.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Actor_Lua.h"
#include "LuaBindings/Component_Lua.h"
#include "LuaBindings/Network_Lua.h"
#include "LuaBindings/Widget_Lua.h"

DEFINE_NODE(ScriptComponent)

std::unordered_map<std::string, ScriptComponent*> ScriptComponent::sTableToCompMap;
std::unordered_map<std::string, ScriptNetFuncMap> ScriptComponent::sScriptNetFuncMap;

bool ScriptComponent::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{

    Property* prop = static_cast<Property*>(datum);

    OCT_ASSERT(prop != nullptr);
    ScriptComponent* scriptComp = static_cast<ScriptComponent*>(prop->mOwner);
    bool success = false;

    if (prop->mName == "Filename")
    {
        const std::string* newFileName = ((const std::string*)newValue);

        if (scriptComp->mFileName != *newFileName)
        {
            scriptComp->SetFile(newFileName->c_str());
            scriptComp->RestartScript();
        }

        success = true;
    }
#if EDITOR
    if (prop->mName == "Restart Script")
    {
        scriptComp->RestartScript();
        success = true;
    }
    else if (prop->mName == "Reload Script File")
    {
        scriptComp->ReloadScriptFile(scriptComp->mFileName, true);
        success = true;
    }
#endif

    return success;
}

bool ScriptComponent::HandleScriptPropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);

    OCT_ASSERT(prop != nullptr);
    OCT_ASSERT(!prop->mExternal);
    ScriptComponent* comp = static_cast<ScriptComponent*>(prop->mOwner);

    prop->SetValueRaw(newValue, index);

    // Now that the value has been updated (prop->SetValue()),
    // We want to propagate that new value to the script instance table.
    comp->UploadDatum(*prop, prop->mName.c_str());

    return true;
}

bool ScriptComponent::HandleForeignScriptPropChange(Datum* datum, uint32_t index, const void* newValue)
{
    // This is ridiculous, but for the Property's that are passed to the editor, we need
    // to propagate the changes made to them so that they reflect the ScriptComponent's properties.
    Property* prop = static_cast<Property*>(datum);

    OCT_ASSERT(prop != nullptr);
    OCT_ASSERT(!prop->mExternal);
    ScriptComponent* comp = static_cast<ScriptComponent*>(prop->mOwner);

    prop->SetValueRaw(newValue, index);

    Property* compProp = nullptr;
    for (uint32_t i = 0; i < comp->mScriptProps.size(); ++i)
    {
        if (comp->mScriptProps[i].mName == prop->mName)
        {
            compProp = &comp->mScriptProps[i];
            break;
        }
    }

    if (compProp != nullptr)
    {
        compProp->SetValue(newValue, index);
    }

    return true;
}


ScriptComponent::ScriptComponent()
{
    mName = "Script";
}

ScriptComponent::~ScriptComponent()
{

}

void ScriptComponent::SaveStream(Stream& stream)
{
    Component::SaveStream(stream);
    stream.WriteString(mFileName);

    // Save script properties
    uint32_t numProps = (uint32_t)mScriptProps.size();
    stream.WriteUint32(numProps);

    for (uint32_t i = 0; i < numProps; ++i)
    {
        mScriptProps[i].WriteStream(stream);
    }
}

void ScriptComponent::LoadStream(Stream& stream)
{
    Component::LoadStream(stream);
    stream.ReadString(mFileName);

    // Load script properties
    uint32_t numProps = stream.ReadUint32();
    std::vector<Property> savedProps;
    savedProps.resize(numProps);

    // Start the script before updating the saved props
    // (because the props won't exist until we start the script)
    StartScript();

    for (uint32_t i = 0; i < numProps; ++i)
    {
        savedProps[i].ReadStream(stream, false);

        for (uint32_t j = 0; j < mScriptProps.size(); ++j)
        {
            if (mScriptProps[j].mName == savedProps[i].mName &&
                mScriptProps[j].mType == savedProps[i].mType)
            {
                if (mScriptProps[i].mCount < savedProps[i].mCount)
                {
                    mScriptProps[i].SetCount(savedProps[i].mCount);
                }

                uint32_t count = savedProps[i].mCount;
                mScriptProps[j].SetValue(savedProps[i].mData.vp, 0, count);
                break;
            }
        }
    }
}

void ScriptComponent::Create()
{
    Component::Create();
    StartScript();
}

void ScriptComponent::Destroy()
{
    Component::Destroy();
    StopScript();
}

void ScriptComponent::Start()
{
    Component::Start();
    CallFunction("Start");
}

void ScriptComponent::EndPlay()
{
    Component::EndPlay();
    CallFunction("EndPlay");
}

void ScriptComponent::Tick(float deltaTime)
{
    Component::Tick(deltaTime);
    CallTick(deltaTime);

    if (NetIsServer())
    {
        DownloadReplicatedData();
    }
}

void ScriptComponent::SetOwner(Actor* owner)
{
    if (GetOwner() != nullptr &&
        ShouldHandleEvents())
    {
        GetOwner()->RemoveScriptEventHandler(this);
    }

    Component::SetOwner(owner);

    if (owner != nullptr &&
        ShouldHandleEvents())
    {
        owner->AddScriptEventHandler(this);
    }
}

const char* ScriptComponent::GetTypeName() const
{
    return "Script";
}

void ScriptComponent::GatherProperties(std::vector<Property>& outProps)
{
    Component::GatherProperties(outProps);

    outProps.push_back(Property(DatumType::String, "Filename", this, &mFileName, 1, HandlePropChange));

#if EDITOR
    static bool sFakeBool = false;
    outProps.push_back(Property(DatumType::Bool, "Restart Script", this, &sFakeBool, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Bool, "Reload Script File", this, &sFakeBool, 1, HandlePropChange));
#endif

    AppendScriptProperties(outProps);
}

void ScriptComponent::AppendScriptProperties(std::vector<Property>& outProps)
{
    for (uint32_t i = 0; i < mScriptProps.size(); ++i)
    {
        outProps.push_back(mScriptProps[i]);

        // Assign our tunneling change handler to the appended properties to make sure they
        // update the properties that are stored in the mScriptProps vector.
        outProps.back().mChangeHandler = HandleForeignScriptPropChange;
    }
}

void ScriptComponent::UploadScriptProperties()
{
    for (uint32_t i = 0; i < mScriptProps.size(); ++i)
    {
        UploadDatum(mScriptProps[i], mScriptProps[i].mName.c_str());
    }
}

void ScriptComponent::SetArrayScriptPropCount(const std::string& name, uint32_t count)
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

void ScriptComponent::GatherScriptProperties()
{
#if LUA_ENABLED
    if (mTableName != "")
    {
        // Even if we had valid ScriptProps already (loaded from blueprint or level file),
        // we still want to re-gather the properties because some may have been added or deleted.
        mScriptProps.clear();

        lua_State* L = GetLua();
        lua_getglobal(L, mTableName.c_str());
        if (lua_istable(L, -1))
        {
            int scriptIdx = lua_gettop(L);

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
                                    lua_getfield(L, scriptIdx, name);

                                    if (!lua_istable(L, -1))
                                    {
                                        lua_pop(L, 1);
                                        lua_newtable(L);
                                        tableIdx = lua_gettop(L);

                                        // Push a copy of the arraytable on the stack so it can be used later.
                                        lua_pushvalue(L, -1);

                                        lua_setfield(L, scriptIdx, name);
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
                                    lua_getfield(L, isArray ? tableIdx : scriptIdx, name);

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
                                            lua_setfield(L, scriptIdx, name);
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

        // Pop script instance Table
        lua_pop(L, 1);
    }
#endif
}

const std::vector<Property>& ScriptComponent::GetScriptProperties() const
{
    return mScriptProps;
}

void ScriptComponent::SetScriptProperties(const std::vector<Property>& srcProps)
{
    CopyPropertyValues(mScriptProps, srcProps);
}

void ScriptComponent::GatherReplicatedData()
{
#if LUA_ENABLED
    if (mTableName != "")
    {
        // Even if we had valid ScriptProps already (loaded from blueprint or level file),
        // we still want to re-gather the properties because some may have been added or deleted.
        mReplicatedData.clear();

        bool isServer = NetIsServer();

        lua_State* L = GetLua();
        lua_getglobal(L, mTableName.c_str());
        if (lua_istable(L, -1))
        {
            int scriptIdx = lua_gettop(L);

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
                                lua_getfield(L, scriptIdx, name);

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

                                    lua_setfield(L, scriptIdx, name);
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
                                    Actor* actorPointer = CHECK_ACTOR(L, -1);
                                    newDatum.PushBack(actorPointer);
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

        // Pop script instance Table
        lua_pop(L, 1);
    }
#endif
}

void ScriptComponent::RegisterNetFuncs()
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

void ScriptComponent::GatherNetFuncs(std::vector<ScriptNetFunc>& outFuncs)
{
#if LUA_ENABLED
    if (mTableName != "")
    {
        lua_State* L = GetLua();
        lua_getglobal(L, mTableName.c_str());
        if (lua_istable(L, -1))
        {
            int scriptIdx = lua_gettop(L);

            lua_getfield(L, -1, "GatherNetFuncs");

            if (lua_isfunction(L, -1))
            {
                lua_pushvalue(L, scriptIdx);   // arg1 - self
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

void ScriptComponent::DownloadReplicatedData()
{
#if LUA_ENABLED
    lua_State* L = GetLua();

    if (mTableName != "")
    {
        lua_getglobal(L, mTableName.c_str());
        OCT_ASSERT(lua_istable(L, -1));
        int tableIdx = lua_gettop(L);

        for (uint32_t i = 0; i < mReplicatedData.size(); ++i)
        {
            DownloadDatum(L, mReplicatedData[i], tableIdx, mReplicatedData[i].mVarName.c_str());
        }

        // Pop script instance table
        lua_pop(L, 1);
    }
#endif
}

ScriptNetFunc* ScriptComponent::FindNetFunc(const char* funcName)
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

ScriptNetFunc* ScriptComponent::FindNetFunc(uint16_t index)
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

void ScriptComponent::ExecuteNetFunc(uint16_t index, uint32_t numParams, std::vector<Datum>& params)
{
#if LUA_ENABLED
    if (mTableName != "")
    {
        lua_State* L = GetLua();
        ScriptNetFunc* netFunc = FindNetFunc(index);

        if (netFunc != nullptr)
        {
            lua_getglobal(L, mTableName.c_str());

            if (lua_istable(L, -1))
            {
                int tableIdx = lua_gettop(L);
                lua_getfield(L, tableIdx, netFunc->mName.c_str());

                if (lua_isfunction(L, -1))
                {
                    lua_pushvalue(L, tableIdx);     // arg1 - self

                    for (uint32_t i = 0; i < numParams; ++i)
                    {
                        LuaPushDatum(L, params[i]);
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

            // Pop instance table
            lua_pop(L, 1);
        }
        else
        {
            LogWarning("Can't find NetFunc[%d] on script %s", index, mClassName.c_str());
        }
    }
#endif
}

bool ScriptComponent::DownloadDatum(lua_State* L, Datum& datum, int tableIdx, const char* varName)
{
    bool success = true;

#if LUA_ENABLED
    lua_getfield(L, tableIdx, varName);

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

void ScriptComponent::UploadDatum(Datum& datum, const char* varName)
{
#if LUA_ENABLED
    lua_State* L = GetLua();
    lua_getglobal(L, mTableName.c_str());
    if (lua_istable(L, -1))
    {
        int instTableIdx = lua_gettop(L);
        int arrayTableIdx = -1;
        uint32_t count = datum.GetCount();

        if (datum.IsProperty() &&
            static_cast<Property&>(datum).IsVector())
        {
            lua_getfield(L, instTableIdx, varName);
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
                lua_setfield(L, instTableIdx, varName);
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

void ScriptComponent::SetFile(const char* filename)
{
    mFileName = filename;
}

const std::string& ScriptComponent::GetFile() const
{
    return mFileName;
}

const std::string& ScriptComponent::GetScriptClassName() const
{
    return mClassName;
}

const std::string& ScriptComponent::GetTableName() const
{
    return mTableName;
}

void ScriptComponent::StartScript()
{
    if (mTableName == "")
    {
        CreateScriptInstance();
    }
}

void ScriptComponent::RestartScript()
{
    DestroyScriptInstance();
    CreateScriptInstance();
}

void ScriptComponent::StopScript()
{
    DestroyScriptInstance();
}

bool ScriptComponent::ReloadScriptFile(const std::string& fileName, bool restartScript)
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

ScriptComponent* ScriptComponent::FindScriptCompFromTableName(const std::string& tableName)
{
    ScriptComponent* scriptComp = nullptr;

    auto it = sTableToCompMap.find(tableName);

    if (it != sTableToCompMap.end())
    {
        scriptComp = it->second;
    }

    return scriptComp;
}

bool ScriptComponent::ShouldHandleEvents() const
{
    return (mHandleBeginOverlap || mHandleEndOverlap || mHandleOnCollision);
}

std::vector<ScriptNetDatum>& ScriptComponent::GetReplicatedData()
{
    return mReplicatedData;
}

void ScriptComponent::InvokeNetFunc(const char* name, std::vector<Datum>& params)
{
    uint32_t numParams = (uint32_t)params.size();
    ScriptNetFunc* netFunc = FindNetFunc(name);
    Actor* actor = GetOwner();

    OCT_ASSERT(numParams <= MAX_NET_FUNC_PARAMS);
    Datum* paramArray[MAX_NET_FUNC_PARAMS];
    for (uint32_t i = 0; i < MAX_NET_FUNC_PARAMS; ++i)
    {
        if (i < numParams)
        {
            paramArray[i] = &params[i];
        }
        else
        {
            paramArray[i] = nullptr;
        }
    }

    if (ShouldSendNetFunc(netFunc->mType, actor))
    {
        // Local execution is handled in SendInvokeScriptMsg
        NetworkManager::Get()->SendInvokeScriptMsg(this, netFunc, numParams, paramArray);
    }

    if (ShouldExecuteNetFunc(netFunc->mType, actor))
    {
        ExecuteNetFunc(netFunc->mIndex, numParams, params);
    }
}

void ScriptComponent::BeginOverlap(Primitive3D* thisComp, Primitive3D* otherComp)
{
#if LUA_ENABLED
    if (mHandleBeginOverlap && mTableName != "")
    {
        lua_State* L = GetLua();

        // Grab the table
        lua_getglobal(L, mTableName.c_str());
        OCT_ASSERT(lua_istable(L, -1));
        int tableIdx = lua_gettop(L);
        lua_getfield(L, tableIdx, "BeginOverlap");

        if (lua_isfunction(L, -1))
        {
            lua_pushvalue(L, tableIdx);
            Component_Lua::Create(L, thisComp);
            Component_Lua::Create(L, otherComp);

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

void ScriptComponent::EndOverlap(Primitive3D* thisComp, Primitive3D* otherComp)
{
#if LUA_ENABLED
    if (mHandleEndOverlap && mTableName != "")
    {
        lua_State* L = GetLua();

        // Grab the table
        lua_getglobal(L, mTableName.c_str());
        OCT_ASSERT(lua_istable(L, -1));
        int tableIdx = lua_gettop(L);
        lua_getfield(L, tableIdx, "EndOverlap");

        if (lua_isfunction(L, -1))
        {
            lua_pushvalue(L, tableIdx);
            Component_Lua::Create(L, thisComp);
            Component_Lua::Create(L, otherComp);

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

void ScriptComponent::OnCollision(
    Primitive3D* thisComp,
    Primitive3D* otherComp,
    glm::vec3 impactPoint,
    glm::vec3 impactNormal,
    btPersistentManifold* manifold)
{
#if LUA_ENABLED
    if (mHandleOnCollision && mTableName != "")
    {
        lua_State* L = GetLua();

        // Grab the table
        lua_getglobal(L, mTableName.c_str());
        OCT_ASSERT(lua_istable(L, -1));
        int tableIdx = lua_gettop(L);
        lua_getfield(L, tableIdx, "OnCollision");

        if (lua_isfunction(L, -1))
        {
            lua_pushvalue(L, tableIdx);                             // arg1 - self
            Component_Lua::Create(L, thisComp);                     // arg2 - thisComp
            Component_Lua::Create(L, otherComp);                    // arg3 - otherComp
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

bool ScriptComponent::HasFunction(const char* name) const
{
    bool ret = false;

    if (mTableName != "")
    {
        lua_State* L = GetLua();

        // Grab the script instance table
        lua_getglobal(L, mTableName.c_str());
        OCT_ASSERT(lua_istable(L, -1));
        lua_getfield(L, -1, name);

        // Only call the function if it has been defined.
        if (lua_isfunction(L, -1))
        {
            ret = true;
        }

        // Pop table and function
        lua_pop(L, 2);
    }

    return ret;
}

// These functions are kinda nasty, but it's a nice convenience for the game programmer
void ScriptComponent::CallFunction(const char* name)
{
    CallFunction(name, 0, nullptr, nullptr);
}

void ScriptComponent::CallFunction(const char* name, const Datum& param0)
{
    const Datum* params[] = { &param0 };
    CallFunction(name, 1, params, nullptr);
}

void ScriptComponent::CallFunction(const char* name, const Datum& param0, const Datum& param1)
{
    const Datum* params[] = { &param0, &param1 };
    CallFunction(name, 2, params, nullptr);
}

void ScriptComponent::CallFunction(const char* name, const Datum& param0, const Datum& param1, const Datum& param2)
{
    const Datum* params[] = { &param0, &param1, &param2 };
    CallFunction(name, 3, params, nullptr);
}

void ScriptComponent::CallFunction(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3)
{
    const Datum* params[] = { &param0, &param1, &param2, &param3 };
    CallFunction(name, 4, params, nullptr);
}

void ScriptComponent::CallFunction(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3, const Datum& param4)
{
    const Datum* params[] = { &param0, &param1, &param2, &param3, &param4 };
    CallFunction(name, 5, params, nullptr);
}

void ScriptComponent::CallFunction(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3, const Datum& param4, const Datum& param5)
{
    const Datum* params[] = { &param0, &param1, &param2, &param3, &param4, &param5 };
    CallFunction(name, 6, params, nullptr);
}

void ScriptComponent::CallFunction(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3, const Datum& param4, const Datum& param5, const Datum& param6)
{
    const Datum* params[] = { &param0, &param1, &param2, &param3, &param4, &param5, &param6 };
    CallFunction(name, 7, params, nullptr);
}

void ScriptComponent::CallFunction(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3, const Datum& param4, const Datum& param5, const Datum& param6, const Datum& param7)
{
    const Datum* params[] = { &param0, &param1, &param2, &param3, &param4, &param5, &param6, &param7 };
    CallFunction(name, 8, params, nullptr);
}

Datum ScriptComponent::CallFunctionR(const char* name)
{
    Datum retDatum;
    CallFunction(name, 0, nullptr, &retDatum);
    return retDatum;
}

Datum ScriptComponent::CallFunctionR(const char* name, const Datum& param0)
{
    Datum retDatum;
    const Datum* params[] = { &param0 };
    CallFunction(name, 1, params, &retDatum);
    return retDatum;
}

Datum ScriptComponent::CallFunctionR(const char* name, const Datum& param0, const Datum& param1)
{
    Datum retDatum;
    const Datum* params[] = { &param0, &param1 };
    CallFunction(name, 2, params, &retDatum);
    return retDatum;
}

Datum ScriptComponent::CallFunctionR(const char* name, const Datum& param0, const Datum& param1, const Datum& param2)
{
    Datum retDatum;
    const Datum* params[] = { &param0, &param1, &param2 };
    CallFunction(name, 3, params, &retDatum);
    return retDatum;
}

Datum ScriptComponent::CallFunctionR(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3)
{
    Datum retDatum;
    const Datum* params[] = { &param0, &param1, &param2, &param3 };
    CallFunction(name, 4, params, &retDatum);
    return retDatum;
}

Datum ScriptComponent::CallFunctionR(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3, const Datum& param4)
{
    Datum retDatum;
    const Datum* params[] = { &param0, &param1, &param2, &param3, &param4 };
    CallFunction(name, 5, params, &retDatum);
    return retDatum;
}

Datum ScriptComponent::CallFunctionR(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3, const Datum& param4, const Datum& param5)
{
    Datum retDatum;
    const Datum* params[] = { &param0, &param1, &param2, &param3, &param4, &param5 };
    CallFunction(name, 6, params, &retDatum);
    return retDatum;
}

Datum ScriptComponent::CallFunctionR(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3, const Datum& param4, const Datum& param5, const Datum& param6)
{
    Datum retDatum;
    const Datum* params[] = { &param0, &param1, &param2, &param3, &param4, &param5, &param6 };
    CallFunction(name, 7, params, &retDatum);
    return retDatum;
}

Datum ScriptComponent::CallFunctionR(const char* name, const Datum& param0, const Datum& param1, const Datum& param2, const Datum& param3, const Datum& param4, const Datum& param5, const Datum& param6, const Datum& param7)
{
    Datum retDatum;
    const Datum* params[] = { &param0, &param1, &param2, &param3, &param4, &param5, &param6, &param7 };
    CallFunction(name, 8, params, &retDatum);
    return retDatum;
}

void ScriptComponent::CallFunction(const char* name, uint32_t numParams, const Datum** params, Datum* ret)
{
    if (mTableName != "")
    {
        ScriptUtils::CallMethod(mTableName.c_str(), name, numParams, params, ret);
    }
}

Datum ScriptComponent::GetField(const char* key)
{
    Datum ret;

    if (mTableName != "")
    {
        ret = ScriptUtils::GetField(mTableName.c_str(), key);
    }

    return ret;
}


void ScriptComponent::SetField(const char* key, const Datum& value)
{
    if (mTableName != "")
    {
        ScriptUtils::SetField(mTableName.c_str(), key, value);
    }
}


bool ScriptComponent::OnRepHandler(Datum* datum, uint32_t index, const void* newValue)
{
#if LUA_ENABLED
    ScriptNetDatum* netDatum = (ScriptNetDatum*)datum;
    ScriptComponent* comp = static_cast<ScriptComponent*>(netDatum->mOwner);

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
    comp->UploadDatum(*netDatum, netDatum->mVarName.c_str());

    if (onRepFunc)
    {
        // Grab the table
        lua_getglobal(L, comp->mTableName.c_str());
        OCT_ASSERT(lua_istable(L, -1));
        int tableIdx = lua_gettop(L);
        lua_getfield(L, tableIdx, netDatum->mOnRepFuncName.c_str());

        if (lua_isfunction(L, -1))
        {
            // (2) If the func is valid, call it and pass the new value
            lua_pushvalue(L, tableIdx);         // arg1 - self
            lua_pushvalue(L, oldValueIdx);      // arg2 - old value

            netDatum->SetValueRaw(newValue);

            comp->LuaFuncCall(2, 0);
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

void ScriptComponent::CreateScriptInstance()
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
                LuaFuncCall(2);
            }
            else
            {
                lua_pop(L, 1);
            }

            // Assign the new table's metatable to the class table
            lua_setmetatable(L, instanceTableIdx);

            Actor_Lua::Create(L, GetOwner());
            lua_setfield(L, instanceTableIdx, "actor");

            Component_Lua::Create(L, this);
            lua_setfield(L, instanceTableIdx, "component");

            mTableName = mClassName + "_" + std::to_string(ScriptUtils::GetNextScriptInstanceNumber());

            // Register the global name on to the script itself, so that native functions can
            // identify what script they are working with.
            lua_pushstring(L, mTableName.c_str());
            lua_setfield(L, instanceTableIdx, "tableName");

            // Save the new table as a global so it doesnt get GCed.
            lua_setglobal(L, mTableName.c_str());

            mTickEnabled = CheckIfFunctionExists("Tick");
            mHandleBeginOverlap = CheckIfFunctionExists("BeginOverlap");
            mHandleEndOverlap = CheckIfFunctionExists("EndOverlap");
            mHandleOnCollision = CheckIfFunctionExists("OnCollision");

            if (ShouldHandleEvents() && GetOwner() != nullptr)
            {
                GetOwner()->AddScriptEventHandler(this);
            }

            UploadScriptProperties();
            GatherScriptProperties();

            if (GetOwner()->IsReplicated())
            {
                GatherReplicatedData();
                RegisterNetFuncs();
            }

            // Register the table to comp map entry so that the script component can be found from a table name.
            sTableToCompMap[mTableName] = this;

            CallFunction("Create");
        }
        else
        {
            LogError("Failed to create script instance. Class has not been loaded.");
        }
    }
#endif
}

void ScriptComponent::DestroyScriptInstance()
{
#if LUA_ENABLED
    if (mTableName != "")
    {
        // Erase this instance so that it gets garbage collected.
        lua_State* L = GetLua();
        if (L != nullptr)
        {
            CallFunction("Destroy");

            // Clear the actor and component fields of the table in case anything else tries to access it.
            // Also set a destroyed field that can be queried.
            lua_getglobal(L, mTableName.c_str());
            if (lua_istable(L, -1))
            {
                int tableIdx = lua_gettop(L);

                lua_pushnil(L);
                lua_setfield(L, tableIdx, "actor");

                lua_pushnil(L);
                lua_setfield(L, tableIdx, "component");

                lua_pushboolean(L, true);
                lua_setfield(L, tableIdx, "destroyed");
            }
            lua_pop(L, 1);

            // Erase this global. It will eventually be garbage collected when nothing else references it.
            lua_pushnil(L);
            lua_setglobal(L, mTableName.c_str());
        }

        sTableToCompMap.erase(mTableName);

        mTableName = "";
        mClassName = "";

        mScriptProps.clear();
        mReplicatedData.clear();
    }

    if (ShouldHandleEvents() && GetOwner() != nullptr)
    {
        GetOwner()->RemoveScriptEventHandler(this);
    }

    mTickEnabled = false;
    mHandleBeginOverlap = false;
    mHandleEndOverlap = false;
    mHandleOnCollision = false;
#endif
}

bool ScriptComponent::LuaFuncCall(int numArgs, int numResults)
{
    bool success = true;
    success = ScriptUtils::CallLuaFunc(numArgs, numResults);
    return success;
}

void ScriptComponent::CallTick(float deltaTime)
{
#if LUA_ENABLED

#if EDITOR
    if (mTableName != "")
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
    if (mTableName != "" && mTickEnabled)
    {
        lua_State* L = GetLua();

        // Grab the table
        lua_getglobal(L, mTableName.c_str());
        OCT_ASSERT(lua_istable(L, -1));
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

bool ScriptComponent::CheckIfFunctionExists(const char* funcName)
{
    bool exists = false;

#if LUA_ENABLED
    if (mTableName != "")
    {
        lua_State* L = GetLua();
        lua_getglobal(L, mTableName.c_str());
        OCT_ASSERT(lua_istable(L, -1));
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


