#include "Utilities.h"
#include "Log.h"
#include "Constants.h"
#include "Renderer.h"
#include "Maths.h"
#include "Engine.h"

#include <iostream>
#include <fstream>

#include <sys/types.h>
#include <sys/stat.h>

#include "System/System.h"

#include <btBulletDynamicsCommon.h>

#if LUA_ENABLED
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#endif

using namespace std;

Platform GetPlatform()
{
#if PLATFORM_WINDOWS
    return Platform::Windows;
#elif PLATFORM_LINUX
    return Platform::Linux;
#elif PLATFORM_GAMECUBE
    return Platform::GameCube;
#elif PLATFORM_WII
    return Platform::Wii;
#elif PLATFORM_3DS
    return Platform::N3DS;
#else
    return Platform::Count;
#endif
}

bool DoesFileExist(const char* filename)
{
    struct stat info;
    bool exists = false;

    int32_t retStatus = stat(filename, &info);

    if (retStatus == 0)
    {
        // If the file is actually a directory, than return false.
        exists = !(info.st_mode & S_IFDIR);
    }

    return exists;
}

std::vector<char> ReadFile(const std::string& filename)
{
    ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        LogError("Failed to open file %s", filename.c_str());
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);
    file.seekg(0);

    file.read(buffer.data(), fileSize);

    return buffer;
}

std::string GetCurrentDirectoryPath()
{
    return SYS_GetCurrentDirectoryPath();
}

void SetWorkingDirectory(const std::string& dirPath)
{
    SYS_SetWorkingDirectory(dirPath);
}

bool DoesDirExist(const char* dirPath)
{
    struct stat info;
    bool isDir = false;

    int32_t retStatus = stat(dirPath, &info);

    if (retStatus == 0)
    {
        isDir = info.st_mode & S_IFDIR;
    }

    return isDir;
}

void CreateDir(const char* dirPath)
{
    SYS_CreateDirectory(dirPath);
}

void RemoveDir(const char* dirPath)
{
    SYS_RemoveDirectory(dirPath);
}

btCollisionShape* CloneCollisionShape(btCollisionShape* srcShape)
{
    btCollisionShape* retShape = nullptr;

    if (srcShape != nullptr)
    {
        uint32_t numShapes = 0;
        btCollisionShape* dstShapes[MAX_COLLISION_SHAPES] = {};
        const btCollisionShape* srcShapes[MAX_COLLISION_SHAPES] = {};

        const btCompoundShape* compoundShape = nullptr;

        // Determine number of shapes we need to copy
        if (srcShape->isCompound())
        {
            compoundShape = static_cast<const btCompoundShape*>(srcShape);
            numShapes = compoundShape->getNumChildShapes();

            for (uint32_t i = 0; i < numShapes; ++i)
            {
                srcShapes[i] = compoundShape->getChildShape(i);
            }
        }
        else
        {
            numShapes = 1;
            srcShapes[0] = srcShape;
        }

        // Copy shapes
        for (uint32_t i = 0; i < numShapes; ++i)
        {
            switch (srcShapes[i]->getShapeType())
            {
            case BOX_SHAPE_PROXYTYPE:
            {
                const btBoxShape* srcBox = (const btBoxShape*)srcShapes[i];
                dstShapes[i] = new btBoxShape(srcBox->getHalfExtentsWithMargin());
                break;
            }
            case SPHERE_SHAPE_PROXYTYPE:
            {
                const btSphereShape* srcSphere = (const btSphereShape*)srcShapes[i];
                dstShapes[i] = new btSphereShape(srcSphere->getRadius());
                break;
            }
            case CONVEX_HULL_SHAPE_PROXYTYPE:
            {
                const btConvexHullShape* srcHull = (const btConvexHullShape*)srcShapes[i];
                dstShapes[i] = new btConvexHullShape((const btScalar*)srcHull->getPoints(), srcHull->getNumPoints(), sizeof(btVector3));
                break;
            }
            case EMPTY_SHAPE_PROXYTYPE:
            {
                dstShapes[i] = new btEmptyShape();
                break;
            }
            }
        }

        // If num shapes > 1, create a new compound shape and add all shapes to it.
        if (compoundShape != nullptr)
        {
            retShape = new btCompoundShape();
            for (uint32_t i = 0; i < numShapes; ++i)
            {
                static_cast<btCompoundShape*>(retShape)->addChildShape(compoundShape->getChildTransform(i), dstShapes[i]);
            }
        }
        else
        {
            retShape = dstShapes[0];
        }
    }
    else
    {
        retShape = new btEmptyShape();
    }

    return retShape;
}

void DestroyCollisionShape(btCollisionShape* shape)
{
    if (shape != nullptr)
    {
        if (shape->isCompound())
        {
            btCompoundShape* compoundShape = static_cast<btCompoundShape*>(shape);
            uint32_t numShape = compoundShape->getNumChildShapes();


            for (uint32_t i = 0; i < numShape; ++i)
            {
                delete compoundShape->getChildShape(i);
            }
        }
        
        delete shape;
    }
}

uint32_t OctHashString(const char* key)
{
    // Using the "CRC" variant from this website:
    // https://www.cs.hmc.edu/~geoff/classes/hmc.cs070.200101/homework10/hashfuncs.html
    assert(key != nullptr);

    uint32_t h = 0;
    const char* keyChar = key;

    while (*keyChar != 0)
    {
        uint32_t ki = uint32_t(*keyChar);
        uint32_t highOrder = h & 0xf8000000;
        h = h << 5;
        h = h ^ (highOrder >> 27);
        h = h ^ ki;

        ++keyChar;
    }

    return h;
}

void CopyPropertyValues(std::vector<Property>& dstProps, const std::vector<Property>& srcProps)
{
    for (uint32_t i = 0; i < srcProps.size(); ++i)
    {
        const Property* srcProp = &srcProps[i];
        Property* dstProp = nullptr;

        for (uint32_t j = 0; j < dstProps.size(); ++j)
        {
            if (dstProps[j].mName == srcProp->mName &&
                dstProps[j].mType == srcProp->mType)
            {
                dstProp = &dstProps[j];
                break;
            }
        }

        if (dstProp != nullptr)
        {
            if (dstProp->IsVector())
            {
                dstProp->ResizeVector(srcProp->GetCount());
            }
            else
            {
                assert(dstProp->mCount == srcProp->mCount);
            }
            
            dstProp->SetValue(srcProp->mData.vp, 0, srcProp->mCount);
        }
    }
}

uint32_t GetStringSerializationSize(const std::string& str)
{
    return uint32_t(STREAM_STRING_LEN_BYTES + str.length());
}

const char* GetPlatformString(Platform platform)
{
    const char* retString = "Unknown";

    switch (platform)
    {
    case Platform::Windows: retString = "Windows"; break;
    case Platform::Linux: retString = "Linux"; break;
    case Platform::GameCube: retString = "GameCube"; break;
    case Platform::Wii: retString = "Wii"; break;
    case Platform::N3DS: retString = "3DS"; break;
    default: break;
    }

    return retString;
}

glm::mat4 MakeTransform(glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale)
{
    glm::mat4 transform = glm::mat4(1);

    glm::quat rotQuat = glm::quat(rotation * DEGREES_TO_RADIANS);

    transform = glm::translate(transform, translation);
    transform *= glm::toMat4(rotQuat);
    transform = glm::scale(transform, scale);

    return transform;
}

void AddDebugDraw(const DebugDraw& draw)
{
#if DEBUG_DRAW_ENABLED
    Renderer::Get()->AddDebugDraw(draw);
#endif
}

void AddDebugDraw(
    StaticMesh* mesh,
    glm::vec3 position,
    glm::quat rotation,
    glm::vec3 scale,
    glm::vec4 color ,
    float life,
    Material* material)
{
#if DEBUG_DRAW_ENABLED
    DebugDraw debugDraw;
    debugDraw.mMesh = mesh;
    debugDraw.mMaterial = material;
    debugDraw.mLife = life;
    debugDraw.mColor = color;

    // Compute transform
    debugDraw.mTransform = glm::translate(debugDraw.mTransform, position);
    debugDraw.mTransform *= glm::toMat4(rotation);
    debugDraw.mTransform = glm::scale(debugDraw.mTransform, scale);

    AddDebugDraw(debugDraw);
#endif
}

void AddDebugDraw(
    StaticMesh* mesh,
    glm::vec3 position,
    glm::vec3 rotation,
    glm::vec3 scale,
    glm::vec4 color,
    float life,
    Material* material)
{
#if DEBUG_DRAW_ENABLED
    glm::quat quat = glm::quat(rotation * DEGREES_TO_RADIANS);
    AddDebugDraw(mesh, position, quat, scale, color, life, material);
#endif
}

float RotateYawTowardDirection(float srcYaw, glm::vec3 dir, float speed, float deltaTime)
{
    float targetYaw = RADIANS_TO_DEGREES * atan2f(-dir.x, -dir.z);
    return Maths::ApproachAngle(srcYaw, targetYaw, speed, deltaTime);
}

#if LUA_ENABLED

bool PreFuncCall(lua_State* L, const char* funcName, const char* selfName)
{
    int top = lua_gettop(L);

    bool funcPushed = false;
    if (selfName)
    {
        lua_getglobal(L, selfName);
        if (lua_istable(L, -1))
        {
            lua_getfield(L, -1, funcName);
            if (lua_isfunction(L, -1))
            {
                funcPushed = true;
                // Push arg1 as self param.
                lua_pushvalue(L, -2);
            }
        }
    }
    else
    {
        lua_getglobal(L, funcName);
        if (lua_isfunction(L, -1))
        {
            funcPushed = true;
        }
    }

    if (!funcPushed)
    {
        lua_settop(L, top);
    }

    return funcPushed;
}

void DoFuncCall(lua_State* L, const char* selfName, int argCount, int retCount)
{
    int totalArgCount = selfName ? argCount + 1 : argCount;
    lua_pcall(L, totalArgCount, retCount, 0);
}

void PostFuncCall(lua_State* L, const char* funcName, const char* selfName)
{
    if (selfName)
    {
        // PreFuncCall pushed 
        lua_pop(L, 1);
    }
}

void PushArgDatum(lua_State* L, const Datum& arg)
{
    switch (arg.mType)
    {
    case DatumType::Integer: lua_pushinteger(L, arg.GetInteger()); break;
    case DatumType::Float: lua_pushnumber(L, arg.GetFloat()); break;
    case DatumType::Bool: lua_pushboolean(L, arg.GetBool()); break;
    case DatumType::String: lua_pushstring(L, arg.GetString().c_str()); break;
    case DatumType::Vector2D: Vector_Lua::Create(L, arg.GetVector2D()); break;
    case DatumType::Vector: Vector_Lua::Create(L, arg.GetVector()); break;
    case DatumType::Color: Vector_Lua::Create(L, arg.GetColor()); break;
    case DatumType::Asset: Asset_Lua::Create(L, arg.GetAsset()); break;
    default: lua_pushnil(L); assert(0); break;
    }
}

void ConvertReturnDatum(lua_State* L, int retIdx, Datum& ret)
{
    int luaType = lua_type(L, retIdx);

    switch (luaType)
    {
    case LUA_TNUMBER:
        if (lua_isinteger(L, retIdx))
        {
            ret.PushBack((int32_t)lua_tointeger(L, retIdx));
        }
        else
        {
            ret.PushBack(lua_tonumber(L, retIdx));
        }
        break;
    case LUA_TBOOLEAN:
        ret.PushBack((bool)lua_toboolean(L, retIdx));
        break;
    case LUA_TSTRING:
        ret.PushBack(lua_tostring(L, retIdx));
        break;
    case LUA_TUSERDATA:
        // 2 main possibilities, Vector or Asset.
        // For Vector types, always return color for all 4 float components.
        // Caller just needs to be aware.
        if (luaL_testudata(L, retIdx, VECTOR_LUA_NAME))
        {
            glm::vec4 vect = CHECK_VECTOR(L, retIdx);
            ret.PushBack(vect);
        }
        else
        {
            Asset* asset = CHECK_ASSET(L, retIdx);
            ret.PushBack(asset);
        }
        break;
    }

    // If nil, then the ret datum is left uninitialized.
    // Caller can check if (mType == DatumType::Count) or (mCount == 0)
}

void CallLuaFunc0(const char* funcName, const char* selfName)
{
    lua_State* L = GetLua();
    if (PreFuncCall(L, funcName, selfName))
    {
        DoFuncCall(L, selfName, 0, 0);
        PostFuncCall(L, funcName, selfName);
    }
}

void CallLuaFunc1(const char* funcName, const char* selfName, Datum arg1)
{
    lua_State* L = GetLua();
    if (PreFuncCall(L, funcName, selfName))
    {
        PushArgDatum(L, arg1);
        DoFuncCall(L, selfName, 1, 0);
        PostFuncCall(L, funcName, selfName);
    }

    //Datum ret;
    //ConvertReturnDatum(L, -1, ret);
    //return ret;
}

void CallLuaFunc2(const char* funcName, const char* selfTable, Datum arg1, Datum arg2)
{

}

void CallLuaFunc3(const char* funcName, const char* selfTable, Datum arg1, Datum arg2, Datum arg3)
{

}

void CallLuaFunc4(const char* funcName, const char* selfTable, Datum arg1, Datum arg2, Datum arg3, Datum arg4)
{

}

void CallLuaFunc5(const char* funcName, const char* selfTable, Datum arg1, Datum arg2, Datum arg3, Datum arg4, Datum arg5)
{

}

#endif
