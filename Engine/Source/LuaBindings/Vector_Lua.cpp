
#include "Engine.h"
#include "EngineTypes.h"
#include "Log.h"
#include "Maths.h"

#include "LuaBindings/Vector_Lua.h"

#if LUA_ENABLED

int Vector_Lua::Create(lua_State* L)
{
    int numArgs = lua_gettop(L);

    Vector_Lua* newVector = (Vector_Lua*)lua_newuserdata(L, sizeof(Vector_Lua));
    new (newVector) Vector_Lua();
    luaL_getmetatable(L, VECTOR_LUA_NAME);
    OCT_ASSERT(lua_istable(L, -1));
    lua_setmetatable(L, -2);

    // Initialize members is args were passed
    if (numArgs == 1 &&
        lua_isuserdata(L, 1))
    {
        // Initialize from other vector
        luaL_checkudata(L, 1, VECTOR_LUA_NAME);
        Vector_Lua* src = (Vector_Lua*)lua_touserdata(L, 1);
        newVector->mVector = src->mVector;
    }
    else if (numArgs >= 1)
    {
        float x = (numArgs >= 1 && lua_isnumber(L, 1)) ? lua_tonumber(L, 1) : 0.0f;
        float y = (numArgs >= 2 && lua_isnumber(L, 2)) ? lua_tonumber(L, 2) : 0.0f;
        float z = (numArgs >= 3 && lua_isnumber(L, 3)) ? lua_tonumber(L, 3) : 0.0f;
        float w = (numArgs >= 4 && lua_isnumber(L, 4)) ? lua_tonumber(L, 4) : 0.0f;
        newVector->mVector = glm::vec4(x, y, z, w);
    }

    return 1;
}

int Vector_Lua::Create(lua_State* L, glm::vec4 value)
{
    Vector_Lua* newVector = (Vector_Lua*)lua_newuserdata(L, sizeof(Vector_Lua));
    new (newVector) Vector_Lua();
    newVector->mVector = value;
    luaL_getmetatable(L, VECTOR_LUA_NAME);
    OCT_ASSERT(lua_istable(L, -1));
    lua_setmetatable(L, -2);

    return 1;
}

int Vector_Lua::Create(lua_State* L, glm::vec3 value)
{
    return Vector_Lua::Create(L, glm::vec4(value, 0.0f));
}

int Vector_Lua::Create(lua_State* L, glm::vec2 value)
{
    return Vector_Lua::Create(L, glm::vec4(value, 0.0f, 0.0f));
}

int Vector_Lua::Destroy(lua_State* L)
{
    // This isn't needed but im keeping it for furture reference for how to hookup destructor.
    CHECK_VECTOR(L, 1);
    Vector_Lua* vec = (Vector_Lua*)lua_touserdata(L, 1);
    vec->~Vector_Lua();
    return 0;
}

int Vector_Lua::Index(lua_State* L)
{
    glm::vec4& value = CHECK_VECTOR(L, 1);
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
        case 'r':
            ret = value.x;
            propertyFound = true;
            break;
        case 'y':
        case 'g':
            ret = value.y;
            propertyFound = true;
            break;
        case 'z':
        case 'b':
            ret = value.z;
            propertyFound = true;
            break;
        case 'w':
        case 'a':
            ret = value.w;
            propertyFound = true;
            break;
        default: break;
        }
    }

    if (propertyFound)
    {
        lua_pushnumber(L, ret);
        return 1;
    }
    else
    {
        lua_getglobal(L, VECTOR_LUA_NAME);
        lua_pushstring(L, key);
        // I think this could be a normal lua_tableget() if you wanted to follow an inheritance chain.
        // But vector doesn't need that.
        lua_rawget(L, -2);
        return 1;
    }
}

int Vector_Lua::NewIndex(lua_State* L)
{
    glm::vec4& value = CHECK_VECTOR(L, 1);
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
        case 'r':
            value.x = lua_tonumber(L, 3);
            propertyFound = true;
            break;
        case 'y':
        case 'g':
            value.y = lua_tonumber(L, 3);
            propertyFound = true;
            break;
        case 'z':
        case 'b':
            value.z = lua_tonumber(L, 3);
            propertyFound = true;
            break;
        case 'w':
        case 'a':
            value.w = lua_tonumber(L, 3);
            propertyFound = true;
            break;
        default: break;
        }
    }

    if (!propertyFound)
    {
        LogError("Lua script attempted to assign a new index to Vector object.");
    }

    return 0;
}

int Vector_Lua::ToString(lua_State* L)
{
    glm::vec4& vec = CHECK_VECTOR(L, 1);

    lua_pushfstring(L, "{ %f, %f, %f, %f }", vec.x, vec.y, vec.z, vec.w);

    return 1;
}

int Vector_Lua::Set(lua_State* L)
{
    int numArgs = lua_gettop(L);
    OCT_ASSERT(numArgs >= 2);
    glm::vec4& dstVec = CHECK_VECTOR(L, 1);

    if (lua_isuserdata(L, 2))
    {
        // Copy value of arg2 to arg1
        OCT_ASSERT(numArgs == 2);
        glm::vec4& srcVec = CHECK_VECTOR(L, 2);
        dstVec = srcVec;
    }
    else
    {
        OCT_ASSERT(numArgs <= 5);
        dstVec.x = (numArgs >= 2 && lua_isnumber(L,2)) ? lua_tonumber(L, 2) : dstVec.x;
        dstVec.y = (numArgs >= 3 && lua_isnumber(L,3)) ? lua_tonumber(L, 3) : dstVec.y;
        dstVec.z = (numArgs >= 4 && lua_isnumber(L,4)) ? lua_tonumber(L, 4) : dstVec.z;
        dstVec.w = (numArgs >= 5 && lua_isnumber(L,5)) ? lua_tonumber(L, 5) : dstVec.w;
    }

    return 0;
}

int Vector_Lua::Clone(lua_State* L)
{
    glm::vec4& srcVec = CHECK_VECTOR(L, 1);

    Vector_Lua::Create(L, srcVec);

    return 1;
}

int Vector_Lua::Add(lua_State* L)
{
    glm::vec4 result = {};

    // leftIndex should always be Vector, secondary could be a number or a Vector.
    int leftIndex = lua_isuserdata(L, 1) ? 1 : 2;
    int rightIndex = (leftIndex == 1) ? 2 : 1;
    glm::vec4& left = CHECK_VECTOR(L, leftIndex);

    if (lua_isnumber(L, rightIndex))
    {
        // Scalar add
        float right = lua_tonumber(L, rightIndex);
        result = left + right;
    }
    else
    {
        // Vector add
        glm::vec4& right = CHECK_VECTOR(L, rightIndex);
        result = left + right;
    }

    Vector_Lua::Create(L, result);
    return 1;
}

int Vector_Lua::Subtract(lua_State* L)
{
    glm::vec4 result = {};

    // leftIndex should always be Vector, secondary could be a number or a Vector.
    int leftIndex = lua_isuserdata(L, 1) ? 1 : 2;
    int rightIndex = (leftIndex == 1) ? 2 : 1;
    glm::vec4& left = CHECK_VECTOR(L, leftIndex);

    if (lua_isnumber(L, rightIndex))
    {
        // Scalar subtract
        float right = lua_tonumber(L, rightIndex);
        result = left - right;
    }
    else
    {
        // Vector subtract
        glm::vec4& right = CHECK_VECTOR(L, rightIndex);
        result = left- right;
    }

    Vector_Lua::Create(L, result);
    return 1;
}

int Vector_Lua::Multiply(lua_State* L)
{
    glm::vec4 result = {};

    // leftIndex should always be Vector, secondary could be a number or a Vector.
    int leftIndex = lua_isuserdata(L, 1) ? 1 : 2;
    int rightIndex = (leftIndex == 1) ? 2 : 1;
    glm::vec4& left = CHECK_VECTOR(L, leftIndex);

    if (lua_isnumber(L, rightIndex))
    {
        // Scalar multiply
        float right = lua_tonumber(L, rightIndex);
        result = left * right;
    }
    else
    {
        // Vector multiply
        glm::vec4& right = CHECK_VECTOR(L, rightIndex);
        result = left * right;
    }

    Vector_Lua::Create(L, result);
    return 1;
}

int Vector_Lua::Divide(lua_State* L)
{
    glm::vec4 result = {};

    // leftIndex should always be Vector, secondary could be a number or a Vector.
    int leftIndex = lua_isuserdata(L, 1) ? 1 : 2;
    int rightIndex = (leftIndex == 1) ? 2 : 1;

    glm::vec4& left = CHECK_VECTOR(L, leftIndex);

    if (lua_isnumber(L, rightIndex))
    {
        // Scalar divide
        float right = lua_tonumber(L, rightIndex);
        result = left / right;
    }
    else
    {
        // Vector divide
        glm::vec4& right = CHECK_VECTOR(L, rightIndex);
        result = left / right;
    }

    Vector_Lua::Create(L, result);
    return 1;
}

int Vector_Lua::Equals(lua_State* L)
{
    glm::vec4& left =CHECK_VECTOR(L, 1);
    glm::vec4& right =CHECK_VECTOR(L, 2);

    bool result = (left == right);

    lua_pushboolean(L, result);
    return 1;
}

int Vector_Lua::Dot(lua_State* L)
{
    glm::vec4& left = CHECK_VECTOR(L, 1);
    glm::vec4& right = CHECK_VECTOR(L, 2);

    float result = glm::dot(left, right);

    lua_pushnumber(L, result);
    return 1;
}

int Vector_Lua::Dot3(lua_State* L)
{
    glm::vec4& left = CHECK_VECTOR(L, 1);
    glm::vec4& right = CHECK_VECTOR(L, 2);

    glm::vec3 l3 = glm::vec3(left);
    glm::vec3 r3 = glm::vec3(right);

    float result = glm::dot(l3, r3);

    lua_pushnumber(L, result);
    return 1;
}

int Vector_Lua::Cross(lua_State* L)
{
    glm::vec4& left = CHECK_VECTOR(L, 1);
    glm::vec4& right = CHECK_VECTOR(L, 2);

    glm::vec3 l3 = left;
    glm::vec3 r3 = right;

    glm::vec3 result = glm::cross(l3, r3);

    Vector_Lua::Create(L, glm::vec4(result, 0));
    return 1;
}

int Vector_Lua::Lerp(lua_State* L)
{
    glm::vec4& a = CHECK_VECTOR(L, 1);
    glm::vec4& b = CHECK_VECTOR(L, 2);
    float alpha = CHECK_NUMBER(L, 3);

    glm::vec4 result = glm::mix(a, b, alpha);

    Vector_Lua::Create(L, result);
    return 1;
}

int Vector_Lua::Max(lua_State* L)
{
    glm::vec4& a = CHECK_VECTOR(L, 1);
    glm::vec4& b = CHECK_VECTOR(L, 2);

    glm::vec4 result = glm::max(a, b);

    Vector_Lua::Create(L, result);
    return 1;
}

int Vector_Lua::Min(lua_State* L)
{
    glm::vec4& a = CHECK_VECTOR(L, 1);
    glm::vec4& b = CHECK_VECTOR(L, 2);

    glm::vec4 result = glm::min(a, b);

    Vector_Lua::Create(L, result);
    return 1;
}

int Vector_Lua::Clamp(lua_State* L)
{
    glm::vec4& value = CHECK_VECTOR(L, 1);
    glm::vec4& min = CHECK_VECTOR(L, 2);
    glm::vec4& max = CHECK_VECTOR(L, 3);

    glm::vec4 result = glm::clamp(value, min, max);

    Vector_Lua::Create(L, result);
    return 1;
}

int Vector_Lua::Normalize(lua_State* L)
{
    glm::vec4& v4 = CHECK_VECTOR(L, 1);

    glm::vec4 result = { 0.0f, 0.0f, 0.0f, 0.0f };

    if (glm::length(v4) > 0.0f)
    {
        result = glm::normalize(v4);
    }

    Vector_Lua::Create(L, result);
    return 1;
}

int Vector_Lua::Normalize3(lua_State* L)
{
    glm::vec3 v3 = CHECK_VECTOR(L, 1);

    glm::vec3 result = { 0.0f, 0.0f, 0.0f };

    if (glm::length(v3) > 0.0f)
    {
        result = glm::normalize(v3);
    }

    Vector_Lua::Create(L, glm::vec4(result, 0));
    return 1;
}

int Vector_Lua::Reflect(lua_State* L)
{
    glm::vec3 inc3 = CHECK_VECTOR(L, 1);
    glm::vec3 nrm3 = CHECK_VECTOR(L, 2);

    glm::vec3 result = glm::reflect(inc3, nrm3);

    Vector_Lua::Create(L, glm::vec4(result, 0));
    return 1;
}

int Vector_Lua::Damp(lua_State* L)
{
    glm::vec4& src = CHECK_VECTOR(L, 1);
    glm::vec4& dst = CHECK_VECTOR(L, 2);
    float smoothing = CHECK_NUMBER(L, 3);
    float deltaTime = CHECK_NUMBER(L, 4);

    glm::vec4 result = Maths::Damp(src, dst, smoothing, deltaTime);

    Vector_Lua::Create(L, result);
    return 1;
}

int Vector_Lua::Rotate(lua_State* L)
{
    glm::vec3 vect3 = CHECK_VECTOR(L, 1);
    float angle = CHECK_NUMBER(L, 2);
    glm::vec3 axis3 = CHECK_VECTOR(L, 3);

    glm::vec3 result = glm::rotate(vect3, angle * DEGREES_TO_RADIANS, axis3);

    Vector_Lua::Create(L, glm::vec4(result, 0));
    return 1;
}

int Vector_Lua::Length(lua_State* L)
{
    glm::vec4 vect = CHECK_VECTOR(L, 1);

    float ret = glm::length(vect);

    lua_pushnumber(L, ret);
    return 1;
}

int Vector_Lua::Distance(lua_State* L)
{
    glm::vec4 a = CHECK_VECTOR(L, 1);
    glm::vec4 b = CHECK_VECTOR(L, 2);

    float ret = glm::distance(a, b);

    lua_pushnumber(L, ret);
    return 1;
}

int Vector_Lua::Distance2(lua_State* L)
{
    glm::vec4 a = CHECK_VECTOR(L, 1);
    glm::vec4 b = CHECK_VECTOR(L, 2);

    float ret = glm::distance2(a, b);

    lua_pushnumber(L, ret);
    return 1;
}

int Vector_Lua::Angle(lua_State* L)
{
    glm::vec3 a = CHECK_VECTOR(L, 1);
    glm::vec3 b = CHECK_VECTOR(L, 2);

    float angleRad = glm::angle(a, b);
    float angleDeg = angleRad * RADIANS_TO_DEGREES;

    lua_pushnumber(L, angleDeg);
    return 1;
}

int Vector_Lua::SignedAngle(lua_State* L)
{
    glm::vec3 a = CHECK_VECTOR(L, 1);
    glm::vec3 b = CHECK_VECTOR(L, 2);
    glm::vec3 n = CHECK_VECTOR(L, 3);

    //float angleRad = atan2f(glm::dot(glm::cross(a, b), n), glm::dot(a, b));
    float angleRad = glm::orientedAngle(a, b, n);
    float angleDeg = angleRad * RADIANS_TO_DEGREES;

    lua_pushnumber(L, angleDeg);
    return 1;
}

int Vector_Lua::Negate(lua_State* L)
{
    glm::vec4 vect = CHECK_VECTOR(L, 1);

    glm::vec4 ret = vect * -1.0f;

    Vector_Lua::Create(L, ret);
    return 1;
}

void Vector_Lua::Bind()
{
    lua_State* L = GetLua();
    OCT_ASSERT(lua_gettop(L) == 0);

    luaL_newmetatable(L, VECTOR_LUA_NAME);
    int mtIndex = lua_gettop(L);

    REGISTER_TABLE_FUNC(L, mtIndex, Create);

    //lua_pushcfunction(L, Vector_Lua::Destroy);
    //lua_setfield(L, mtIndex, "__gc");

    REGISTER_TABLE_FUNC(L, mtIndex, Set);

    REGISTER_TABLE_FUNC(L, mtIndex, Clone);

    REGISTER_TABLE_FUNC(L, mtIndex, Add);
    REGISTER_TABLE_FUNC_EX(L, mtIndex, Add, "__add");

    REGISTER_TABLE_FUNC(L, mtIndex, Subtract);
    REGISTER_TABLE_FUNC_EX(L, mtIndex, Subtract, "__sub");

    REGISTER_TABLE_FUNC(L, mtIndex, Multiply);
    REGISTER_TABLE_FUNC_EX(L, mtIndex, Multiply, "__mul");

    REGISTER_TABLE_FUNC(L, mtIndex, Divide);
    REGISTER_TABLE_FUNC_EX(L, mtIndex, Divide, "__div");

    REGISTER_TABLE_FUNC(L, mtIndex, Equals);
    REGISTER_TABLE_FUNC_EX(L, mtIndex, Equals, "__eq");

    //lua_pushcfunction(L, Vector_Lua::NotEqual);
    //lua_pushvalue(L, -1);
    //lua_setfield(L, mtIndex, "NotEqual");

    REGISTER_TABLE_FUNC(L, mtIndex, Dot);

    REGISTER_TABLE_FUNC(L, mtIndex, Dot3);

    REGISTER_TABLE_FUNC(L, mtIndex, Cross);

    REGISTER_TABLE_FUNC(L, mtIndex, Lerp);

    REGISTER_TABLE_FUNC(L, mtIndex, Max);

    REGISTER_TABLE_FUNC(L, mtIndex, Min);

    REGISTER_TABLE_FUNC(L, mtIndex, Clamp);

    REGISTER_TABLE_FUNC(L, mtIndex, Normalize);

    REGISTER_TABLE_FUNC(L, mtIndex, Normalize3);

    REGISTER_TABLE_FUNC(L, mtIndex, Reflect);

    REGISTER_TABLE_FUNC(L, mtIndex, Damp);

    REGISTER_TABLE_FUNC(L, mtIndex, Rotate);

    REGISTER_TABLE_FUNC(L, mtIndex, Length);
    REGISTER_TABLE_FUNC_EX(L, mtIndex, Length, "Magnitude");

    REGISTER_TABLE_FUNC(L, mtIndex, Distance);

    REGISTER_TABLE_FUNC(L, mtIndex, Distance2);
    REGISTER_TABLE_FUNC_EX(L, mtIndex, Distance2, "DistanceSquared");

    REGISTER_TABLE_FUNC(L, mtIndex, Angle);

    REGISTER_TABLE_FUNC(L, mtIndex, SignedAngle);

    REGISTER_TABLE_FUNC_EX(L, mtIndex, Negate, "__unm");

    REGISTER_TABLE_FUNC_EX(L, mtIndex, Index, "__index");

    REGISTER_TABLE_FUNC_EX(L, mtIndex, NewIndex, "__newindex");

    REGISTER_TABLE_FUNC_EX(L, mtIndex, ToString, "__tostring");

    lua_setglobal(L, VECTOR_LUA_NAME);

    // Add a global to easily create a vector with Vec()
    lua_pushcfunction(L, Vector_Lua::Create);
    lua_setglobal(L, "Vec");

    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif