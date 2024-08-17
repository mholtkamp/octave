#include "Utilities.h"
#include "Log.h"
#include "Constants.h"
#include "Renderer.h"
#include "Maths.h"
#include "Engine.h"
#include "TableDatum.h"
#include "Assets/Scene.h"
#include "Nodes/3D/StaticMesh3d.h"

#include <iostream>
#include <fstream>
#include <ctype.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "System/System.h"
#include "Input/Input.h"

#include <btBulletDynamicsCommon.h>

#if LUA_ENABLED
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Node_Lua.h"
#include "LuaBindings/Widget_Lua.h"
#endif

using namespace std;

Platform GetPlatform()
{
#if PLATFORM_WINDOWS
    return Platform::Windows;
#elif PLATFORM_LINUX
    return Platform::Linux;
#elif PLATFORM_ANDROID
    return Platform::Android;
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

std::string StringToLower(const std::string& str)
{
    std::string ret;
    for (uint32_t i = 0; i < str.size(); ++i)
    {
        ret.push_back(tolower(str[i]));
    }

    return ret;
}

std::string StringToUpper(const std::string& str)
{
    std::string ret;
    for (uint32_t i = 0; i < str.size(); ++i)
    {
        ret.push_back(toupper(str[i]));
    }

    return ret;
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

std::string GetDirShortName(const std::string& dirName)
{
    std::string shortName = dirName;
    size_t slash1 = shortName.find_last_of("/\\", shortName.size() - 2);
    size_t slash2 = shortName.find_last_of("/\\");

    if (slash1 != std::string::npos &&
        slash2 != std::string::npos)
    {
        shortName = shortName.substr(slash1 + 1, (slash2 - slash1 - 1));
    }

    return shortName;
}

void CreateSymLink(const std::string& original, const std::string& link)
{
#if PLATFORM_WINDOWS
    std::string absOriginal = SYS_GetAbsolutePath(original);
    std::string linkOriginal = SYS_GetAbsolutePath(link);
    SYS_Exec(std::string("mklink /J " + linkOriginal + " " + absOriginal).c_str());
#else
    SYS_Exec(std::string("ln -s " + original + " " + link).c_str());
#endif
}

btCollisionShape* CloneCollisionShape(const btCollisionShape* srcShape)
{
    btCollisionShape* retShape = nullptr;

    if (srcShape != nullptr)
    {
        uint32_t numShapes = 0;
        std::vector<btCollisionShape*> dstShapes;
        std::vector<const btCollisionShape*> srcShapes;

        const btCompoundShape* compoundShape = nullptr;

        // Determine number of shapes we need to copy
        if (srcShape->isCompound())
        {
            compoundShape = static_cast<const btCompoundShape*>(srcShape);
            numShapes = compoundShape->getNumChildShapes();

            for (uint32_t i = 0; i < numShapes; ++i)
            {
                srcShapes.push_back(compoundShape->getChildShape(i));
            }
        }
        else
        {
            numShapes = 1;
            srcShapes.push_back(srcShape);
        }

        OCT_ASSERT(srcShapes.size() == numShapes);
        dstShapes.resize(numShapes);

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

            case COMPOUND_SHAPE_PROXYTYPE:
            {
                dstShapes[i] = CloneCollisionShape(srcShapes[i]);
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
                DestroyCollisionShape(compoundShape->getChildShape(i));
            }
        }
        
        delete shape;
    }
}

static int32_t sDebugConvexCollisionMeshIndex = 0;

void DebugDrawCollisionShape(btCollisionShape* collisionShape, Node3D* node, const glm::mat4& parentTransform, std::vector<DebugDraw>* inoutDraws)
{
    uint32_t numCollisionShapes = 0;
    std::vector<btCollisionShape*> collisionShapes;
    std::vector<glm::mat4> collisionTransforms;

    Primitive3D* prim3d = node ? node->As<Primitive3D>() : nullptr;
    StaticMesh3D* mesh3d = node ? node->As<StaticMesh3D>() : nullptr;
    StaticMesh* staticMesh = mesh3d ? mesh3d->GetStaticMesh() : nullptr;

    glm::vec3 invScale = 1.0f / BulletToGlm(collisionShape->getLocalScaling()); // GetWorldScale();

    int shapeType = collisionShape->getShapeType();

    if (shapeType == EMPTY_SHAPE_PROXYTYPE)
        return;

    if (shapeType == COMPOUND_SHAPE_PROXYTYPE)
    {
        sDebugConvexCollisionMeshIndex = 0;
    }

    DebugDraw debugDraw;
    glm::mat4 shapeTransform = glm::mat4(1);

    switch (shapeType)
    {
    case BOX_SHAPE_PROXYTYPE:
    {
        // Assuming that default cube mesh has half extents equal to 1,1,1
        btBoxShape* boxShape = static_cast<btBoxShape*>(collisionShape);
        btVector3 halfExtents = boxShape->getHalfExtentsWithMargin();

        shapeTransform = glm::scale(shapeTransform, invScale);
        shapeTransform = glm::scale(shapeTransform, { halfExtents.x(), halfExtents.y(), halfExtents.z() });

        debugDraw.mMesh = LoadAsset<StaticMesh>("SM_Cube");
        break;
    }
    case SPHERE_SHAPE_PROXYTYPE:
    {
        // Assuming that default sphere mesh has a radius of 1
        btSphereShape* sphereShape = static_cast<btSphereShape*>(collisionShape);
        float radius = sphereShape->getRadius();

        shapeTransform = glm::scale(shapeTransform, invScale);
        shapeTransform = glm::scale(shapeTransform, { radius, radius, radius });

        debugDraw.mMesh = LoadAsset<StaticMesh>("SM_Sphere");
        break;
    }
    case CONVEX_HULL_SHAPE_PROXYTYPE:
    {
#if CREATE_CONVEX_COLLISION_MESH
        if (sDebugConvexCollisionMeshIndex < staticMesh->mCollisionMeshes.size())
        {
            // We only create StaticMesh objects for convex hulls when in editor.
            debugDraw.mMesh = staticMesh->mCollisionMeshes[sDebugConvexCollisionMeshIndex];
            staticMesh->mCollisionMeshes[sDebugConvexCollisionMeshIndex]->GetBounds();
            ++sDebugConvexCollisionMeshIndex;
        }
#endif
        break;
    }
    case SCALED_TRIANGLE_MESH_SHAPE_PROXYTYPE:
    {
        debugDraw.mMesh = staticMesh;

        break;
    }
    case COMPOUND_SHAPE_PROXYTYPE:
    {
        btCompoundShape* compoundShape = static_cast<btCompoundShape*>(collisionShape);

        for (int32_t i = 0; i < compoundShape->getNumChildShapes(); ++i)
        {
            btCollisionShape* childShape = compoundShape->getChildShape(i);
            collisionShapes.push_back(childShape);
            const btTransform& bTransform = compoundShape->getChildTransform(i);
            btQuaternion bRotation = bTransform.getRotation();
            btVector3 bPosition = bTransform.getOrigin();

            glm::quat rotation = glm::quat(bRotation.w(), bRotation.x(), bRotation.y(), bRotation.z());
            glm::vec3 position = { bPosition.x(), bPosition.y(), bPosition.z() };
            glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);

            // Compound shapes scale their children immediately??
            // I mean, setLocalScale() on a compound shape will iterate through children and 
            // call setLocalScale() on them... This is not what I would expect.
            //if (childShape->getShapeType() == SCALED_TRIANGLE_MESH_SHAPE_PROXYTYPE)
            {
                scale = BulletToGlm(childShape->getLocalScaling());
            }

            glm::mat4 colTransform = glm::mat4(1);
            colTransform = glm::scale(invScale);
            colTransform = glm::translate(colTransform, position);
            colTransform *= glm::toMat4(rotation);
            colTransform = glm::scale(colTransform, scale);
            collisionTransforms.push_back(colTransform);

            DebugDrawCollisionShape(childShape, node, parentTransform * colTransform, inoutDraws);
        }
        break;
    }
    }

    if (shapeType != COMPOUND_SHAPE_PROXYTYPE)
    {
        debugDraw.mTransform = parentTransform * shapeTransform;
        debugDraw.mColor = prim3d->GetCollisionDebugColor();

        if (inoutDraws)
        {
            inoutDraws->push_back(debugDraw);
        }
        else
        {
            Renderer::Get()->AddDebugDraw(debugDraw);
        }
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

void GatherAllNodeNames(std::vector<std::string>& outNames)
{
    outNames.clear();
    const std::vector<Factory*>& nodeFactories = Node::GetFactoryList();

    for (uint32_t i = 0; i < nodeFactories.size(); ++i)
    {
        outNames.push_back(nodeFactories[i]->GetClassName());
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
    case Platform::Android: retString = "Android"; break;
    case Platform::GameCube: retString = "GameCube"; break;
    case Platform::Wii: retString = "Wii"; break;
    case Platform::N3DS: retString = "3DS"; break;
    default: break;
    }

    return retString;
}

uint8_t ConvertKeyCodeToChar(uint8_t keyCode, bool shiftDown)
{
    uint8_t retChar = 0;
    retChar = INP_ConvertKeyCodeToChar(keyCode);

    if (retChar >= 'A' &&
        retChar <= 'Z' &&
        !shiftDown)
    {
        // If not shifted, make the character lower-case.
        // TODO: handle caps lock state.
        retChar += 32;
    }
    else if (shiftDown)
    {
        switch (retChar)
        {
        case '`': retChar = '~'; break;
        case '1': retChar = '!'; break;
        case '2': retChar = '@'; break;
        case '3': retChar = '#'; break;
        case '4': retChar = '$'; break;
        case '5': retChar = '%'; break;
        case '6': retChar = '^'; break;
        case '7': retChar = '&'; break;
        case '8': retChar = '*'; break;
        case '9': retChar = '('; break;
        case '0': retChar = ')'; break;
        case '-': retChar = '_'; break;
        case '=': retChar = '+'; break;
        case '[': retChar = '{'; break;
        case ']': retChar = '}'; break;
        case '\\': retChar = '|'; break;
        case ';': retChar = ':'; break;
        case '\'': retChar = '\"'; break;
        case ',': retChar = '<'; break;
        case '.': retChar = '>'; break;
        case '/': retChar = '?'; break;
        }
    }

    return retChar;
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

btTransform MakeBulletTransform(glm::vec3 translation, glm::vec3 rotationDegrees)
{
    btTransform retTransform;

    glm::quat rotQuat = glm::quat(rotationDegrees * DEGREES_TO_RADIANS);
    glm::vec3 pos = translation;

    btQuaternion bRotation = btQuaternion(rotQuat.x, rotQuat.y, rotQuat.z, rotQuat.w);
    btVector3 bPosition = { pos.x, pos.y, pos.z };
    retTransform = btTransform(bRotation, bPosition);

    return retTransform;
}

void GatherNonDefaultProperties(Node* node, std::vector<Property>& props)
{
    std::vector<Property> extProps;
    node->GatherProperties(extProps);

    Scene* scene = node->IsSceneLinked() ? node->GetScene() : nullptr;

    {
        // For native nodes, determine which properties are different than the defaults
        // and only save those to reduce storage/memory of the scene.
        Node* defaultNode = scene ? scene->Instantiate() : Node::Construct(node->GetType());
        std::vector<Property> defaultProps;
        defaultNode->GatherProperties(defaultProps);

        props.reserve(extProps.size());
        for (uint32_t i = 0; i < extProps.size(); ++i)
        {
            Property* defaultProp = FindProperty(defaultProps, extProps[i].mName);

            if (defaultProp == nullptr ||
                (extProps[i].mType == DatumType::Asset && scene == nullptr) || 
                extProps[i] != *defaultProp)
            {
                props.push_back(Property());
                props.back().DeepCopy(extProps[i], true);
            }
        }

        Node::Destruct(defaultNode);
        defaultNode = nullptr;
    }
}

void RemoveSpacesFromString(std::string& str)
{
    for (int32_t i = int32_t(str.size()) - 1; i >= 0; --i)
    {
        if (str[i] == ' ')
        {
            str.erase(str.begin() + i);
        }
    }
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
    uint32_t count = arg.GetCount();

    if (arg.GetType() == DatumType::Table)
    {
        CreateTableLua(L, arg);
        return;
    }

    if (count == 0 && !arg.mForceScriptArray)
    {
        lua_pushnil(L);
        return;
    }

    int32_t arrIdx = -1;

    if (count > 1 || arg.mForceScriptArray)
    {
        lua_newtable(L);
        arrIdx = lua_gettop(L);
    }

    for (uint32_t i = 0; i < count; ++i)
    {
        switch (arg.mType)
        {
        case DatumType::Integer: lua_pushinteger(L, arg.GetInteger(i)); break;
        case DatumType::Short: lua_pushinteger(L, (int)arg.GetShort(i)); break;
        case DatumType::Byte: lua_pushinteger(L, (int)arg.GetByte(i)); break;
        case DatumType::Float: lua_pushnumber(L, arg.GetFloat(i)); break;
        case DatumType::Bool: lua_pushboolean(L, arg.GetBool(i)); break;
        case DatumType::String: lua_pushstring(L, arg.GetString(i).c_str()); break;
        case DatumType::Vector2D: Vector_Lua::Create(L, arg.GetVector2D(i)); break;
        case DatumType::Vector: Vector_Lua::Create(L, arg.GetVector(i)); break;
        case DatumType::Color: Vector_Lua::Create(L, arg.GetColor(i)); break;
        case DatumType::Asset: Asset_Lua::Create(L, arg.GetAsset(i)); break;
        case DatumType::Table: 
            // This should be handled by CreateTableLua().
            OCT_ASSERT(0);
            break;
        case DatumType::Function:
            arg.GetFunction(i).Push(L);
            break;
        case DatumType::Pointer:
        {
            RTTI* pointer = arg.GetPointer(i);
            if (pointer == nullptr)
            {
                lua_pushnil(L);
            }
            else if (pointer->Is(Node::ClassRuntimeId()))
            {
                Node_Lua::Create(L, (Node*)pointer);
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

    // Need to insert elements into array table
    if (arrIdx != -1)
    {
        // The last element in the array is at the top of the stack, 
        // so go backwards to set the the integer indexes (otherwise the vector will be reversed).
        for (int32_t i = count - 1; i >= 0; --i)
        {
            lua_seti(L, arrIdx, i + 1);
        }

        // The array table should be top on the stack, now that has lua_seti has removed count elements.
        OCT_ASSERT(lua_gettop(L) == arrIdx);
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
        // 3 main possibilities: Vector, Node, Asset
        if (luaL_testudata(L, idx, VECTOR_LUA_NAME))
        {
            glm::vec4 vect = CHECK_VECTOR(L, idx);
            datum.PushBack(vect);
        }
        else if (CheckClassFlag(L, idx, NODE_LUA_FLAG))
        {
            Node* node = CHECK_NODE(L, idx);
            datum.PushBack(node);
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
    case LUA_TFUNCTION:
        datum.PushBack(ScriptFunc(L, idx));
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
