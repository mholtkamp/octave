
#include "Engine.h"
#include "EngineTypes.h"
#include "Log.h"
#include "Maths.h"

#include "LuaBindings/Rect_Lua.h"

#if LUA_ENABLED

int Rect_Lua::Create(lua_State* L)
{
    int numArgs = lua_gettop(L);

    Rect_Lua* newRect = (Rect_Lua*)lua_newuserdata(L, sizeof(Rect_Lua));
    new (newRect) Rect_Lua();
    luaL_getmetatable(L, RECT_LUA_NAME);
    OCT_ASSERT(lua_istable(L, -1));
    lua_setmetatable(L, -2);

    // Initialize members is args were passed
    if (numArgs == 1 &&
        lua_isuserdata(L, 1))
    {
        // Initialize from other rect
        luaL_checkudata(L, 1, RECT_LUA_NAME);
        Rect_Lua* src = (Rect_Lua*)lua_touserdata(L, 1);
        newRect->mRect = src->mRect;
    }
    else if (numArgs >= 1)
    {
        float x = (numArgs >= 1 && lua_isnumber(L, 1)) ? lua_tonumber(L, 1) : 0.0f;
        float y = (numArgs >= 2 && lua_isnumber(L, 2)) ? lua_tonumber(L, 2) : 0.0f;
        float w = (numArgs >= 3 && lua_isnumber(L, 3)) ? lua_tonumber(L, 3) : 1.0f;
        float h = (numArgs >= 4 && lua_isnumber(L, 4)) ? lua_tonumber(L, 4) : 1.0f;
        newRect->mRect.mX = x;
        newRect->mRect.mY = y;
        newRect->mRect.mWidth = w;
        newRect->mRect.mHeight = h;
    }

    return 1;
}

int Rect_Lua::Create(lua_State* L, Rect value)
{
    Rect_Lua* newRect = (Rect_Lua*)lua_newuserdata(L, sizeof(Rect_Lua));
    new (newRect) Rect_Lua();
    newRect->mRect = value;
    luaL_getmetatable(L, RECT_LUA_NAME);
    OCT_ASSERT(lua_istable(L, -1));
    lua_setmetatable(L, -2);

    return 1;
}

int Rect_Lua::Destroy(lua_State* L)
{
    // This isn't needed but im keeping it for furture reference for how to hookup destructor.
    CHECK_RECT(L, 1);
    Rect_Lua* rect = (Rect_Lua*)lua_touserdata(L, 1);
    rect->~Rect_Lua();
    return 0;
}

int Rect_Lua::Index(lua_State* L)
{
    Rect& value = CHECK_RECT(L, 1);
    const char* key = CHECK_STRING(L, 2);

    bool propertyFound = false;
    float ret = 0.0f;

    uint8_t keyChar = key[0];
    bool singleChar = key[0] != '\0' && key[1] == '\0';

    if (singleChar)
    {
        switch (keyChar)
        {
        case 'x':
            ret = value.mX;
            propertyFound = true;
            break;
        case 'y':
            ret = value.mY;
            propertyFound = true;
            break;
        case 'w':
            ret = value.mWidth;
            propertyFound = true;
            break;
        case 'h':
            ret = value.mHeight;
            propertyFound = true;
            break;
        default: break;
        }
    }
    else
    {
        if (strcmp(key, "width") == 0)
        {
            ret = value.mWidth;
            propertyFound = true;
        }
        else if (strcmp(key, "height") == 0)
        {
            ret = value.mHeight;
            propertyFound = true;
        }
    }

    if (propertyFound)
    {
        lua_pushnumber(L, ret);
        return 1;
    }
    else
    {
        lua_getglobal(L, RECT_LUA_NAME);
        lua_pushstring(L, key);
        lua_rawget(L, -2);
        return 1;
    }
}

int Rect_Lua::NewIndex(lua_State* L)
{
    Rect& value = CHECK_RECT(L, 1);
    const char* key = CHECK_STRING(L, 2);
    // Arg3 is value we want to assign

    bool propertyFound = false;
    uint8_t keyChar = key[0];
    bool singleChar = key[0] != '\0' && key[1] == '\0';

    if (singleChar)
    {
        switch (keyChar)
        {
        case 'x':
            value.mX = lua_tonumber(L, 3);
            propertyFound = true;
            break;
        case 'y':
            value.mY = lua_tonumber(L, 3);
            propertyFound = true;
            break;
        case 'w':
            value.mWidth = lua_tonumber(L, 3);
            propertyFound = true;
            break;
        case 'h':
            value.mHeight = lua_tonumber(L, 3);
            propertyFound = true;
            break;
        default: break;
        }
    }
    else
    {
        if (strcmp(key, "width") == 0)
        {
            value.mWidth = lua_tonumber(L, 3);
            propertyFound = true;
        }
        else if (strcmp(key, "height") == 0)
        {
            value.mHeight = lua_tonumber(L, 3);
            propertyFound = true;
        }
    }

    if (!propertyFound)
    {
        LogError("Lua script attempted to assign a new index to Rect object.");
    }

    return 0;
}

int Rect_Lua::ToString(lua_State* L)
{
    Rect& rect = CHECK_RECT(L, 1);

    lua_pushfstring(L, "{ %f, %f, %f, %f }", rect.mX, rect.mY, rect.mWidth, rect.mHeight);

    return 1;
}

int Rect_Lua::ContainsPoint(lua_State* L)
{
    Rect& rect = CHECK_RECT(L, 1);
    float x = CHECK_NUMBER(L, 2);
    float y = CHECK_NUMBER(L, 3);

    bool ret = rect.ContainsPoint(x, y);

    lua_pushboolean(L, ret);
    return 1;
}

int Rect_Lua::OverlapsRect(lua_State* L)
{
    Rect& a = CHECK_RECT(L, 1);
    Rect& b = CHECK_RECT(L, 2);

    bool ret = a.OverlapsRect(b);

    lua_pushboolean(L, ret);
    return 1;
}

int Rect_Lua::Clamp(lua_State* L)
{
    Rect& a = CHECK_RECT(L, 1);
    Rect& b = CHECK_RECT(L, 2);

    a.Clamp(b);

    return 0;
}

int Rect_Lua::Top(lua_State* L)
{
    Rect& rect = CHECK_RECT(L, 1);

    float ret = rect.Top();

    lua_pushnumber(L, ret);
    return 1;
}

int Rect_Lua::Bottom(lua_State* L)
{
    Rect& rect = CHECK_RECT(L, 1);

    float ret = rect.Bottom();

    lua_pushnumber(L, ret);
    return 1;
}

int Rect_Lua::Left(lua_State* L)
{
    Rect& rect = CHECK_RECT(L, 1);

    float ret = rect.Left();

    lua_pushnumber(L, ret);
    return 1;
}

int Rect_Lua::Right(lua_State* L)
{
    Rect& rect = CHECK_RECT(L, 1);

    float ret = rect.Right();

    lua_pushnumber(L, ret);
    return 1;
}

void Rect_Lua::Bind()
{
    lua_State* L = GetLua();
    OCT_ASSERT(lua_gettop(L) == 0);

    luaL_newmetatable(L, RECT_LUA_NAME);
    int mtIndex = lua_gettop(L);

    lua_pushcfunction(L, Create);
    lua_setfield(L, mtIndex, "Create");

    lua_pushcfunction(L, Index);
    lua_setfield(L, mtIndex, "__index");

    lua_pushcfunction(L, NewIndex);
    lua_setfield(L, mtIndex, "__newindex");

    lua_pushcfunction(L, ToString);
    lua_setfield(L, mtIndex, "__tostring");

    lua_pushcfunction(L, ContainsPoint);
    lua_setfield(L, mtIndex, "ContainsPoint");

    lua_pushcfunction(L, OverlapsRect);
    lua_setfield(L, mtIndex, "OverlapsRect");

    lua_pushcfunction(L, Clamp);
    lua_setfield(L, mtIndex, "Clamp");

    lua_pushcfunction(L, Top);
    lua_setfield(L, mtIndex, "Top");

    lua_pushcfunction(L, Bottom);
    lua_setfield(L, mtIndex, "Bottom");

    lua_pushcfunction(L, Left);
    lua_setfield(L, mtIndex, "Left");

    lua_pushcfunction(L, Right);
    lua_setfield(L, mtIndex, "Right");

    lua_setglobal(L, RECT_LUA_NAME);

    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif