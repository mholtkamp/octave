#include "Utilities.h"
#include "Log.h"
#include "Constants.h"
#include "Renderer.h"
#include "Maths.h"
#include "Engine.h"
#include "TableDatum.h"
#include "Components/ScriptComponent.h"

#include <iostream>
#include <fstream>

#include <sys/types.h>
#include <sys/stat.h>

#include "System/System.h"

#include <btBulletDynamicsCommon.h>

#if LUA_ENABLED
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Actor_Lua.h"
#include "LuaBindings/Component_Lua.h"
#include "LuaBindings/Widget_Lua.h"
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
    OCT_ASSERT(key != nullptr);

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

void GatherAllClassNames(std::vector<std::string>& outNames)
{
    outNames.clear();
    const std::vector<Factory*>& actorFactories = Actor::GetFactoryList();

    for (uint32_t i = 0; i < actorFactories.size(); ++i)
    {
        outNames.push_back(actorFactories[i]->GetClassName());
    }
}

Property* FindProperty(std::vector<Property>& props, const std::string& name)
{
    Property* prop = nullptr;

    for (uint32_t i = 0; i < props.size(); ++i)
    {
        if (props[i].mName == name)
        {
            prop = &props[i];
            break;
        }
    }

    return prop;
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
                OCT_ASSERT(dstProp->mCount == srcProp->mCount);
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

#if LUA_ENABLED
void CreateTableLua(lua_State* L, const Datum& datum)
{
    OCT_ASSERT(datum.GetType() == DatumType::Table);
    if (datum.GetType() == DatumType::Table)
    {
        lua_newtable(L);

        // Add all children to the table 
        for (uint32_t i = 0; i < datum.GetCount(); ++i)
        {
            const TableDatum& tableDatum = datum.GetTableDatum(i);

            // Push key first
            if (tableDatum.IsStringKey())
            {
                lua_pushstring(L, tableDatum.GetStringKey());
            }
            else
            {
                lua_pushinteger(L, tableDatum.GetIntegerKey());
            }

            // Then push value
            LuaPushDatum(L, tableDatum);

            // [-1] Value
            // [-2] Key
            // [-3] Table
            lua_settable(L, -3);
        }
    }
    else
    {
        lua_pushnil(L);
    }
}

void CreateTableCpp(lua_State* L, int tableIdx, Datum& datum)
{
    OCT_ASSERT(datum.GetType() == DatumType::Count ||
           datum.GetType() == DatumType::Table);

    if (lua_istable(L, tableIdx))
    {
        // Push a copy so that we can accept positive/negative indices for tableIdx
        lua_pushvalue(L, tableIdx); // -1 = table

        // Update tableIdx to be a positive index (needed for recursion)
        tableIdx = lua_gettop(L);

        // Push the first key to start iterating at, in this case nil
        lua_pushnil(L); // -1 = nil, -2 = table

        datum.SetType(DatumType::Table);

        while (lua_next(L, -2))
        {
            // Stack is now: -1 = value, -2 = key, -3 = table
            
            datum.PushBackTableDatum(TableDatum());
            TableDatum& tableDatum = datum.GetTableDatum(datum.GetCount() - 1);

            // Initialize key first
            if (lua_isinteger(L, -2))
            {
                tableDatum.SetIntegerKey(lua_tointeger(L, -2));
            }
            else if (lua_isstring(L, -2))
            {
                tableDatum.SetStringKey(lua_tostring(L, -2));
            }

            // Initialize value
            LuaObjectToDatum(L, -1, tableDatum);

            // Pop the value, but leave the key on top of the stack, so it can be
            // used as the argument for the next lua_next() call.
            lua_pop(L, 1);
        }

        // Pop the copy of table
        lua_pop(L, 1);
    }
    else
    {
        LogError("CreateTableCpp(): tableIdx is not a table");
    }
}

void LuaPushDatum(lua_State* L, const Datum& arg)
{
    switch (arg.mType)
    {
    case DatumType::Integer: lua_pushinteger(L, arg.GetInteger()); break;
    case DatumType::Short: lua_pushinteger(L, (int) arg.GetShort()); break;
    case DatumType::Byte: lua_pushinteger(L, (int) arg.GetByte()); break;
    case DatumType::Float: lua_pushnumber(L, arg.GetFloat()); break;
    case DatumType::Bool: lua_pushboolean(L, arg.GetBool()); break;
    case DatumType::String: lua_pushstring(L, arg.GetString().c_str()); break;
    case DatumType::Vector2D: Vector_Lua::Create(L, arg.GetVector2D()); break;
    case DatumType::Vector: Vector_Lua::Create(L, arg.GetVector()); break;
    case DatumType::Color: Vector_Lua::Create(L, arg.GetColor()); break;
    case DatumType::Asset: Asset_Lua::Create(L, arg.GetAsset()); break;
    case DatumType::Table: CreateTableLua(L, arg); break;
    case DatumType::Pointer:
    {
        RTTI* pointer = arg.GetPointer();
        if (pointer == nullptr)
        {
            lua_pushnil(L);
        }
        else if (pointer->Is(Actor::ClassRuntimeId()))
        {
            Actor_Lua::Create(L, (Actor*)pointer);
        }
        else if (pointer->Is(Component::ClassRuntimeId()))
        {
            Component_Lua::Create(L, (Component*)pointer);
        }
        else if (pointer->Is(Widget::ClassRuntimeId()))
        {
            Widget_Lua::Create(L, (Widget*)pointer);
        }
        else
        {
            LogError("Unsupported pointer type in LuaPushDatum.");
            lua_pushnil(L);
        }
        break;
    }

    default: lua_pushnil(L); break;
    }
}

Datum LuaObjectToDatum(lua_State* L, int idx)
{
    // Hope return value optimization saves unnecessary copies/alloc/free?
    Datum ret;
    LuaObjectToDatum(L, idx, ret);
    return ret;
}

void LuaObjectToDatum(lua_State* L, int idx, Datum& datum)
{
    int luaType = lua_type(L, idx);

    switch (luaType)
    {
    case LUA_TNUMBER:
        if (lua_isinteger(L, idx))
        {
            datum.PushBack((int32_t)lua_tointeger(L, idx));
        }
        else
        {
            datum.PushBack(lua_tonumber(L, idx));
        }
        break;
    case LUA_TBOOLEAN:
        datum.PushBack((bool)lua_toboolean(L, idx));
        break;
    case LUA_TSTRING:
        datum.PushBack(lua_tostring(L, idx));
        break;
    case LUA_TUSERDATA:
        // 5 main possibilities: Vector, Actor, Component, Widget, Asset
        if (luaL_testudata(L, idx, VECTOR_LUA_NAME))
        {
            glm::vec4 vect = CHECK_VECTOR(L, idx);
            datum.PushBack(vect);
        }
        else if (CheckClassFlag(L, idx, ACTOR_LUA_FLAG))
        {
            Actor* actor = CHECK_ACTOR(L, idx);
            datum.PushBack(actor);
        }
        else if (CheckClassFlag(L, idx, COMPONENT_LUA_FLAG))
        {
            Component* comp = CHECK_COMPONENT(L, idx);
            datum.PushBack(comp);
        }
        else if (CheckClassFlag(L, idx, WIDGET_LUA_FLAG))
        {
            Widget* widget = CHECK_WIDGET(L, idx);
            datum.PushBack(widget);
        }
        else //if (CheckClassFlag(L, idx, ASSET_LUA_FLAG))
        {
            // If the type check failed here, then an unsupported userdata is being received.
            Asset* asset = CHECK_ASSET(L, idx);
            datum.PushBack(asset);
        }
        break;
    case LUA_TTABLE:
        CreateTableCpp(L, idx, datum);
        break;

    default:
        // If nil, then the datum is initialized as a Byte type with 0
        // Actually... Leave as DatumType::Count?
        //datum.PushBack((uint8_t)0);
        break;
    }
}
#endif

glm::vec3 EnforceEulerRange(const glm::vec3& eulerAngles)
{
    glm::vec3 retAngles = eulerAngles;
    if (retAngles.x > 180.0f)
    {
        retAngles.x -= 360.0f;
    }
    else if (retAngles.x < -180.0f)
    {
        retAngles.x += 360.0f;
    }

    if (retAngles.x > 90.0f)
    {
        retAngles.x -= 180.f;
        retAngles.y = 180.f - retAngles.y;
        retAngles.z += 180.f;
    }
    else if (retAngles.x < -90.0f)
    {
        retAngles.x += 180.f;
        retAngles.y = 180.f - retAngles.y;
        retAngles.z += 180.f;
    }

    return retAngles;
}
