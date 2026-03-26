#include "Assets/DataAsset.h"
#include "Engine.h"
#include "Log.h"
#include "Stream.h"
#include "Property.h"
#include "AssetManager.h"

#if LUA_ENABLED
#include "LuaBindings/LuaUtils.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#endif

FORCE_LINK_DEF(DataAsset);
DEFINE_ASSET(DataAsset);

bool DataAsset::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);
    DataAsset* dataAsset = (DataAsset*)prop->mOwner;

    if (prop->mName == "Script File")
    {
        const std::string& newPath = *(const std::string*)newValue;
        prop->SetValueRaw(newValue, index);
        dataAsset->ReloadPropertiesFromScript();
        return true;
    }

    // Default handling
    return HandleAssetPropChange(datum, index, newValue);
}

DataAsset::DataAsset()
{
    mType = DataAsset::GetStaticType();
}

DataAsset::~DataAsset()
{
}

void DataAsset::Create()
{
    Asset::Create();
}

void DataAsset::Destroy()
{
    mProperties.clear();
    Asset::Destroy();
}

void DataAsset::LoadStream(Stream& stream, Platform platform)
{
    Asset::LoadStream(stream, platform);

    // Read script file path
    stream.ReadString(mScriptFile);

    // Load properties from the script file first to get property definitions
    LoadPropertiesFromLua();

    // Read number of saved property values
    uint32_t numSavedProps = stream.ReadUint32();

    // Read property values and match them to loaded properties
    for (uint32_t i = 0; i < numSavedProps; ++i)
    {
        std::string propName;
        stream.ReadString(propName);
        DatumType propType = (DatumType)stream.ReadUint8();
        bool isVector = stream.ReadBool();
        uint32_t count = stream.ReadUint32();

        // Find the property by name
        Property* matchedProp = nullptr;
        for (Property& prop : mProperties)
        {
            if (prop.mName == propName && prop.mType == propType)
            {
                matchedProp = &prop;
                break;
            }
        }

        if (matchedProp != nullptr)
        {
            // Resize if needed for arrays
            if (isVector && matchedProp->IsVector())
            {
                matchedProp->ResizeVector(count);
            }

            // Read values
            matchedProp->ReadStream(stream, mVersion, false, false);
        }
        else
        {
            // Property not found in script, skip the data
            Datum tempDatum;
            tempDatum.mType = propType;
            tempDatum.mExternal = false;

            if (isVector)
            {
                for (uint32_t j = 0; j < count; ++j)
                {
                    tempDatum.ReadStream(stream, mVersion, false, false);
                }
            }
            else
            {
                tempDatum.SetCount(count);
                tempDatum.ReadStream(stream, mVersion, false, false);
            }
        }
    }
}

void DataAsset::SaveStream(Stream& stream, Platform platform)
{
    Asset::SaveStream(stream, platform);

    // Write script file path
    stream.WriteString(mScriptFile);

    // Write number of properties
    stream.WriteUint32((uint32_t)mProperties.size());

    // Write each property's name, type, and value
    for (const Property& prop : mProperties)
    {
        stream.WriteString(prop.mName);
        stream.WriteUint8((uint8_t)prop.mType);
        stream.WriteBool(prop.IsVector());
        stream.WriteUint32(prop.GetCount());
        prop.WriteStream(stream, false);
    }
}

void DataAsset::GatherProperties(std::vector<Property>& outProps)
{
    Asset::GatherProperties(outProps);

    outProps.push_back(Property(DatumType::String, "Script File", this, &mScriptFile, 1, HandlePropChange));

    // Add all dynamic properties from the Lua script
    // Create external-storage properties pointing to the data in mProperties
    for (Property& prop : mProperties)
    {
        if (prop.IsVector())
        {
            // Vector properties need special handling - copy the property but keep
            // a reference by not using internal storage
            // For now, just copy - vector editing in DataAssets is limited
            outProps.push_back(prop);
        }
        else
        {
            // Non-vector: create external property pointing to the original's data
            Property extProp(prop.mType, prop.mName, this, prop.GetValue(0), prop.mCount, HandleAssetPropChange);
#if EDITOR
            extProp.mCategory = prop.mCategory;
#endif
            outProps.push_back(extProp);
        }
    }
}

glm::vec4 DataAsset::GetTypeColor()
{
    return glm::vec4(0.2f, 0.8f, 0.4f, 1.0f);  // Green color for data assets
}

const char* DataAsset::GetTypeName()
{
    return "DataAsset";
}

void DataAsset::SetScriptFile(const std::string& path)
{
    mScriptFile = path;
    ReloadPropertiesFromScript();
}

Datum* DataAsset::GetPropertyValue(const std::string& name)
{
    for (Property& prop : mProperties)
    {
        if (prop.mName == name)
        {
            return &prop;
        }
    }
    return nullptr;
}

bool DataAsset::SetPropertyValue(const std::string& name, const Datum& value)
{
    for (Property& prop : mProperties)
    {
        if (prop.mName == name)
        {
            prop.DeepCopy(value, true);
            return true;
        }
    }
    return false;
}

void DataAsset::ReloadPropertiesFromScript()
{
    LoadPropertiesFromLua();
}

void DataAsset::LoadPropertiesFromLua()
{
#if LUA_ENABLED
    mProperties.clear();
    mPropertiesLoaded = false;

    if (mScriptFile.empty())
    {
        return;
    }

    lua_State* L = GetLua();
    if (L == nullptr)
    {
        return;
    }

    // Build the full path to the script file
    std::string scriptPath = GetEngineState()->mProjectDirectory + "Scripts/" + mScriptFile;

    // Try to load and run the script file
    if (luaL_dofile(L, scriptPath.c_str()) != LUA_OK)
    {
        const char* errMsg = lua_tostring(L, -1);
        LogError("DataAsset: Failed to load script '%s': %s", mScriptFile.c_str(), errMsg ? errMsg : "Unknown error");
        lua_pop(L, 1);
        return;
    }

    // Call GetProperties() function
    lua_getglobal(L, "GetProperties");
    if (!lua_isfunction(L, -1))
    {
        LogError("DataAsset: Script '%s' does not have a GetProperties() function", mScriptFile.c_str());
        lua_pop(L, 1);
        return;
    }

    if (lua_pcall(L, 0, 1, 0) != LUA_OK)
    {
        const char* errMsg = lua_tostring(L, -1);
        LogError("DataAsset: Error calling GetProperties() in '%s': %s", mScriptFile.c_str(), errMsg ? errMsg : "Unknown error");
        lua_pop(L, 1);
        return;
    }

    if (!lua_istable(L, -1))
    {
        LogError("DataAsset: GetProperties() in '%s' did not return a table", mScriptFile.c_str());
        lua_pop(L, 1);
        return;
    }

    int arrayIdx = lua_gettop(L);

    // Get number of properties
    lua_len(L, arrayIdx);
    int32_t numProps = (int32_t)lua_tointeger(L, -1);
    lua_pop(L, 1);

    // Loop through each property definition
    for (int32_t i = 1; i <= numProps; ++i)
    {
        lua_geti(L, arrayIdx, i);

        if (lua_istable(L, -1))
        {
            int propIdx = lua_gettop(L);

            // Get property name
            lua_getfield(L, propIdx, "name");
            std::string name = lua_isstring(L, -1) ? lua_tostring(L, -1) : "";
            lua_pop(L, 1);

            // Get property type
            lua_getfield(L, propIdx, "type");
            DatumType type = lua_isinteger(L, -1) ? (DatumType)lua_tointeger(L, -1) : DatumType::Count;
            lua_pop(L, 1);

            // Get array flag
            lua_getfield(L, propIdx, "array");
            bool isArray = lua_toboolean(L, -1);
            lua_pop(L, 1);

            // Validate property
            if (name.empty() || type == DatumType::Count || type == DatumType::Table)
            {
                lua_pop(L, 1);  // Pop property table
                continue;
            }

            // Create the property with internal storage
            Property newProp;
            newProp.mName = name;
            newProp.mType = type;
            newProp.mOwner = this;
            newProp.mExternal = false;
            newProp.mChangeHandler = HandleAssetPropChange;

#if EDITOR
            newProp.mCategory = "Data";
#endif

            if (isArray)
            {
                newProp.MakeVector();
            }
            else
            {
                // Initialize with default value based on type
                switch (type)
                {
                case DatumType::Integer:
                {
                    lua_getfield(L, propIdx, "default");
                    int32_t defaultVal = lua_isinteger(L, -1) ? (int32_t)lua_tointeger(L, -1) : 0;
                    lua_pop(L, 1);
                    newProp.PushBack(defaultVal);
                    break;
                }
                case DatumType::Float:
                {
                    lua_getfield(L, propIdx, "default");
                    float defaultVal = lua_isnumber(L, -1) ? (float)lua_tonumber(L, -1) : 0.0f;
                    lua_pop(L, 1);
                    newProp.PushBack(defaultVal);
                    break;
                }
                case DatumType::Bool:
                {
                    lua_getfield(L, propIdx, "default");
                    bool defaultVal = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;
                    lua_pop(L, 1);
                    newProp.PushBack(defaultVal);
                    break;
                }
                case DatumType::String:
                {
                    lua_getfield(L, propIdx, "default");
                    std::string defaultVal = lua_isstring(L, -1) ? lua_tostring(L, -1) : "";
                    lua_pop(L, 1);
                    newProp.PushBack(defaultVal);
                    break;
                }
                case DatumType::Vector2D:
                {
                    lua_getfield(L, propIdx, "default");
                    glm::vec2 defaultVal(0.0f);
                    if (lua_istable(L, -1))
                    {
                        lua_rawgeti(L, -1, 1);
                        defaultVal.x = lua_isnumber(L, -1) ? (float)lua_tonumber(L, -1) : 0.0f;
                        lua_pop(L, 1);
                        lua_rawgeti(L, -1, 2);
                        defaultVal.y = lua_isnumber(L, -1) ? (float)lua_tonumber(L, -1) : 0.0f;
                        lua_pop(L, 1);
                    }
                    else if (lua_isuserdata(L, -1))
                    {
                        defaultVal = CHECK_VECTOR(L, -1);
                    }
                    lua_pop(L, 1);
                    newProp.PushBack(defaultVal);
                    break;
                }
                case DatumType::Vector:
                {
                    lua_getfield(L, propIdx, "default");
                    glm::vec3 defaultVal(0.0f);
                    if (lua_istable(L, -1))
                    {
                        lua_rawgeti(L, -1, 1);
                        defaultVal.x = lua_isnumber(L, -1) ? (float)lua_tonumber(L, -1) : 0.0f;
                        lua_pop(L, 1);
                        lua_rawgeti(L, -1, 2);
                        defaultVal.y = lua_isnumber(L, -1) ? (float)lua_tonumber(L, -1) : 0.0f;
                        lua_pop(L, 1);
                        lua_rawgeti(L, -1, 3);
                        defaultVal.z = lua_isnumber(L, -1) ? (float)lua_tonumber(L, -1) : 0.0f;
                        lua_pop(L, 1);
                    }
                    else if (lua_isuserdata(L, -1))
                    {
                        defaultVal = CHECK_VECTOR(L, -1);
                    }
                    lua_pop(L, 1);
                    newProp.PushBack(defaultVal);
                    break;
                }
                case DatumType::Color:
                {
                    lua_getfield(L, propIdx, "default");
                    glm::vec4 defaultVal(1.0f);
                    if (lua_istable(L, -1))
                    {
                        lua_rawgeti(L, -1, 1);
                        defaultVal.x = lua_isnumber(L, -1) ? (float)lua_tonumber(L, -1) : 1.0f;
                        lua_pop(L, 1);
                        lua_rawgeti(L, -1, 2);
                        defaultVal.y = lua_isnumber(L, -1) ? (float)lua_tonumber(L, -1) : 1.0f;
                        lua_pop(L, 1);
                        lua_rawgeti(L, -1, 3);
                        defaultVal.z = lua_isnumber(L, -1) ? (float)lua_tonumber(L, -1) : 1.0f;
                        lua_pop(L, 1);
                        lua_rawgeti(L, -1, 4);
                        defaultVal.w = lua_isnumber(L, -1) ? (float)lua_tonumber(L, -1) : 1.0f;
                        lua_pop(L, 1);
                    }
                    else if (lua_isuserdata(L, -1))
                    {
                        defaultVal = CHECK_VECTOR(L, -1);
                    }
                    lua_pop(L, 1);
                    newProp.PushBack(defaultVal);
                    break;
                }
                case DatumType::Asset:
                {
                    // Assets default to nullptr
                    newProp.PushBack((Asset*)nullptr);
                    break;
                }
                case DatumType::Byte:
                {
                    lua_getfield(L, propIdx, "default");
                    uint8_t defaultVal = lua_isinteger(L, -1) ? (uint8_t)lua_tointeger(L, -1) : 0;
                    lua_pop(L, 1);
                    newProp.PushBack(defaultVal);
                    break;
                }
                case DatumType::Short:
                {
                    lua_getfield(L, propIdx, "default");
                    int16_t defaultVal = lua_isinteger(L, -1) ? (int16_t)lua_tointeger(L, -1) : 0;
                    lua_pop(L, 1);
                    newProp.PushBack(defaultVal);
                    break;
                }
                default:
                    break;
                }
            }

            mProperties.push_back(std::move(newProp));
        }

        lua_pop(L, 1);  // Pop property table
    }

    lua_pop(L, 1);  // Pop properties array

    // Clear the global GetProperties function to avoid conflicts
    lua_pushnil(L);
    lua_setglobal(L, "GetProperties");

    mPropertiesLoaded = true;
#endif
}

void DataAsset::SyncPropertyStorage()
{
    // Properties use internal storage, nothing to sync
}
