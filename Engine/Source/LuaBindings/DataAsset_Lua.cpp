#include "LuaBindings/DataAsset_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Node_Lua.h"

#if LUA_ENABLED

int DataAsset_Lua::Get(lua_State* L)
{
    DataAsset* dataAsset = CHECK_DATA_ASSET(L, 1);
    const char* propName = CHECK_STRING(L, 2);

    Datum* datum = dataAsset->GetPropertyValue(propName);

    if (datum == nullptr)
    {
        lua_pushnil(L);
        return 1;
    }

    // Push the value based on type
    switch (datum->GetType())
    {
    case DatumType::Integer:
        lua_pushinteger(L, datum->GetInteger());
        break;
    case DatumType::Float:
        lua_pushnumber(L, datum->GetFloat());
        break;
    case DatumType::Bool:
        lua_pushboolean(L, datum->GetBool());
        break;
    case DatumType::String:
        lua_pushstring(L, datum->GetString().c_str());
        break;
    case DatumType::Vector2D:
        Vector_Lua::Create(L, datum->GetVector2D());
        break;
    case DatumType::Vector:
        Vector_Lua::Create(L, datum->GetVector());
        break;
    case DatumType::Color:
        Vector_Lua::Create(L, datum->GetColor());
        break;
    case DatumType::Asset:
        Asset_Lua::Create(L, datum->GetAsset(), true);
        break;
    case DatumType::Byte:
        lua_pushinteger(L, (int32_t)datum->GetByte());
        break;
    case DatumType::Short:
        lua_pushinteger(L, (int32_t)datum->GetShort());
        break;
    default:
        lua_pushnil(L);
        break;
    }

    return 1;
}

int DataAsset_Lua::Set(lua_State* L)
{
    DataAsset* dataAsset = CHECK_DATA_ASSET(L, 1);
    const char* propName = CHECK_STRING(L, 2);

    Datum* datum = dataAsset->GetPropertyValue(propName);

    if (datum == nullptr)
    {
        LogWarning("DataAsset:Set() - Property '%s' not found", propName);
        return 0;
    }

    // Set the value based on type
    switch (datum->GetType())
    {
    case DatumType::Integer:
    {
        int32_t val = CHECK_INTEGER(L, 3);
        datum->SetInteger(val);
        break;
    }
    case DatumType::Float:
    {
        float val = CHECK_NUMBER(L, 3);
        datum->SetFloat(val);
        break;
    }
    case DatumType::Bool:
    {
        bool val = CHECK_BOOLEAN(L, 3);
        datum->SetBool(val);
        break;
    }
    case DatumType::String:
    {
        const char* val = CHECK_STRING(L, 3);
        datum->SetString(val);
        break;
    }
    case DatumType::Vector2D:
    {
        glm::vec2 vec = CHECK_VECTOR(L, 3);
        datum->SetVector2D(vec);
        break;
    }
    case DatumType::Vector:
    {
        glm::vec3 vec = CHECK_VECTOR(L, 3);
        datum->SetVector(vec);
        break;
    }
    case DatumType::Color:
    {
        glm::vec4 vec = CHECK_VECTOR(L, 3);
        datum->SetColor(vec);
        break;
    }
    case DatumType::Asset:
    {
        Asset* asset = nullptr;
        if (lua_isuserdata(L, 3))
        {
            asset = CHECK_ASSET(L, 3);
        }
        datum->SetAsset(asset);
        break;
    }
    case DatumType::Byte:
    {
        int32_t val = CHECK_INTEGER(L, 3);
        datum->SetByte((uint8_t)val);
        break;
    }
    case DatumType::Short:
    {
        int32_t val = CHECK_INTEGER(L, 3);
        datum->SetShort((int16_t)val);
        break;
    }
    default:
        LogWarning("DataAsset:Set() - Unsupported property type for '%s'", propName);
        break;
    }

    return 0;
}

int DataAsset_Lua::GetScriptFile(lua_State* L)
{
    DataAsset* dataAsset = CHECK_DATA_ASSET(L, 1);

    const std::string& scriptFile = dataAsset->GetScriptFile();

    lua_pushstring(L, scriptFile.c_str());
    return 1;
}

int DataAsset_Lua::SetScriptFile(lua_State* L)
{
    DataAsset* dataAsset = CHECK_DATA_ASSET(L, 1);
    const char* scriptFile = CHECK_STRING(L, 2);

    dataAsset->SetScriptFile(scriptFile);

    return 0;
}

int DataAsset_Lua::ReloadProperties(lua_State* L)
{
    DataAsset* dataAsset = CHECK_DATA_ASSET(L, 1);

    dataAsset->ReloadPropertiesFromScript();

    return 0;
}

void DataAsset_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        DATA_ASSET_LUA_NAME,
        DATA_ASSET_LUA_FLAG,
        ASSET_LUA_NAME);

    Asset_Lua::BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, Get);

    REGISTER_TABLE_FUNC(L, mtIndex, Set);

    REGISTER_TABLE_FUNC(L, mtIndex, GetScriptFile);

    REGISTER_TABLE_FUNC(L, mtIndex, SetScriptFile);

    REGISTER_TABLE_FUNC(L, mtIndex, ReloadProperties);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
