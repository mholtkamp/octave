#pragma once

#include <vector>
#include <string>

#include <Bullet/btBulletDynamicsCommon.h>

#include "Property.h"
#include "EngineTypes.h"
#include "Maths.h"

class Actor;
class Component;

#define OCT_ARRAY_SIZE(array) (int(sizeof(array) / sizeof(array[0])))

#define OCT_UNUSED(var) (void)var

#define FORCE_LINK_DEF(x) int gForceLink_##x = 0;
#define FORCE_LINK_CALL(x) { extern int gForceLink_##x; gForceLink_##x = 1; }

Platform GetPlatform();

bool DoesFileExist(const char* filename);
std::vector<char> ReadFile(const std::string& filename);

std::string GetCurrentDirectoryPath();
void SetWorkingDirectory(const std::string& dirPath);
bool DoesDirExist(const char* dirPath);
void CreateDir(const char* dirPath);
void RemoveDir(const char* dirPath);

btCollisionShape* CloneCollisionShape(btCollisionShape* srcShape);
void DestroyCollisionShape(btCollisionShape* shape);

uint32_t OctHashString(const char* key);
void GatherAllClassNames(std::vector<std::string>& outNames);

Property* FindProperty(std::vector<Property>& props, const std::string& name);
void CopyPropertyValues(std::vector<Property>& dstProps, const std::vector<Property>& srcProps);

uint32_t GetStringSerializationSize(const std::string& str);

const char* GetPlatformString(Platform platform);

glm::mat4 MakeTransform(glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale);

void AddDebugDraw(const DebugDraw& draw);
void AddDebugDraw(
    StaticMesh* mesh,
    glm::vec3 position,
    glm::quat rotation,
    glm::vec3 scale,
    glm::vec4 color = { 0.25f, 0.25f, 1.0f, 1.0f },
    float life = 0.0f,
    Material* material = nullptr);
void AddDebugDraw(
    StaticMesh* mesh,
    glm::vec3 position,
    glm::vec3 rotation,
    glm::vec3 scale,
    glm::vec4 color = { 0.25f, 0.25f, 1.0f, 1.0f },
    float life = 0.0f,
    Material* material = nullptr);

#if LUA_ENABLED
void CreateTableLua(lua_State* L, const Datum& datum);
void CreateTableCpp(lua_State* L, int tableIdx, Datum& datum);
void LuaPushDatum(lua_State* L, const Datum& arg);
Datum LuaObjectToDatum(lua_State* L, int idx);
void LuaObjectToDatum(lua_State* L, int idx, Datum& datum);
#endif

glm::vec3 EnforceEulerRange(const glm::vec3& eulerAngles);

inline glm::vec3 BulletToGlm(const btVector3& vector3)
{
    return glm::vec3(vector3.x(), vector3.y(), vector3.z());
}

inline glm::quat BulletToGlm(const btQuaternion& quat)
{
    return glm::quat(quat.w(), quat.x(), quat.y(), quat.z());
}

inline btVector3 GlmToBullet(const glm::vec3& vector3)
{
    return btVector3(vector3.x, vector3.y, vector3.z);
}

inline btQuaternion GlmToBullet(const glm::quat& quat)
{
    return btQuaternion(quat.x, quat.y, quat.z, quat.w);
}

inline uint32_t ColorFloat4ToUint32(glm::vec4 color)
{
    uint8_t colors[4] =
    {
        uint8_t(glm::clamp(color.r * 255.0f, 0.0f, 255.0f)),
        uint8_t(glm::clamp(color.g * 255.0f, 0.0f, 255.0f)),
        uint8_t(glm::clamp(color.b * 255.0f, 0.0f, 255.0f)),
        uint8_t(glm::clamp(color.a * 255.0f, 0.0f, 255.0f))
    };

    uint32_t color32 =
        (colors[0]) |
        (colors[1] << 8) |
        (colors[2] << 16) |
        (colors[3] << 24);

    return color32;
}

inline glm::vec4 ColorUint32ToFloat4(uint32_t color)
{
    float r = float(color & 0x000000ff) / 255.0f;
    float g = float((color & 0x0000ff00) >> 8) / 255.0f;
    float b = float((color & 0x00ff0000) >> 16) / 255.0f;
    float a = float((color & 0xff000000) >> 24) / 255.0f;

    return glm::vec4(r, g, b, a);
}

template<typename T>
Bounds ComputeBounds(const std::vector<T>& vertices)
{
    Bounds retBounds;

    if (vertices.size() > 0)
    {
        glm::vec3 boxMin = { FLT_MAX, FLT_MAX, FLT_MAX };
        glm::vec3 boxMax = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

        for (uint32_t i = 0; i < vertices.size(); ++i)
        {
            glm::vec3 pos = vertices[i].mPosition;

            boxMin = glm::min(boxMin, pos);
            boxMax = glm::max(boxMax, pos);
        }

        retBounds.mCenter = (boxMin + boxMax) / 2.0f;

        float maxDist = 0.0f;

        for (uint32_t i = 0; i < vertices.size(); ++i)
        {
            glm::vec3 pos = vertices[i].mPosition;

            float dist = glm::distance(pos, retBounds.mCenter);
            maxDist = glm::max(maxDist, dist);
        }

        retBounds.mRadius = maxDist;
    }

    return retBounds;
}

template<typename T>
void ShuffleVector(std::vector<T>& vect)
{
    uint32_t vectSize = (uint32_t)vect.size();

    if (vectSize > 1)
    {
        for (uint32_t i = 0; i < vectSize; ++i)
        {
            uint32_t p = Maths::RandRange<uint32_t>(0u, vectSize - 1);
            T temp = vect[p];
            vect[p] = vect[i];
            vect[i] = temp;
        }
    }
}
