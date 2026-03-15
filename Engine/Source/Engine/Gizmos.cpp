#include "Gizmos.h"
#include "Engine.h"
#include "AssetManager.h"
#include "Assets/StaticMesh.h"

#if EDITOR
#include "EditorState.h"
#endif

glm::vec4 Gizmos::sColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
glm::mat4 Gizmos::sMatrix = glm::mat4(1.0f);
std::vector<DebugDraw> Gizmos::sSolidDraws;
std::vector<DebugDraw> Gizmos::sWireDraws;
std::vector<Line> Gizmos::sLines;

void Gizmos::SetColor(glm::vec4 color)
{
    sColor = color;
}

glm::vec4 Gizmos::GetColor()
{
    return sColor;
}

void Gizmos::SetMatrix(const glm::mat4& matrix)
{
    sMatrix = matrix;
}

const glm::mat4& Gizmos::GetMatrix()
{
    return sMatrix;
}

void Gizmos::ResetState()
{
    sColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    sMatrix = glm::mat4(1.0f);
}

static glm::mat4 BuildTransform(const glm::mat4& baseMatrix, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
{
    glm::mat4 transform = glm::translate(baseMatrix, position);
    if (rotation.x != 0.0f) transform = glm::rotate(transform, rotation.x * DEGREES_TO_RADIANS, glm::vec3(1, 0, 0));
    if (rotation.y != 0.0f) transform = glm::rotate(transform, rotation.y * DEGREES_TO_RADIANS, glm::vec3(0, 1, 0));
    if (rotation.z != 0.0f) transform = glm::rotate(transform, rotation.z * DEGREES_TO_RADIANS, glm::vec3(0, 0, 1));
    transform = glm::scale(transform, scale);
    return transform;
}

void Gizmos::DrawCube(glm::vec3 center, glm::vec3 size)
{
    if (!IsEnabled()) return;

    DebugDraw draw;
    draw.mMesh = LoadAsset<StaticMesh>("SM_Cube");
    draw.mColor = sColor;
    draw.mTransform = BuildTransform(sMatrix, center, glm::vec3(0.0f), size);
    sSolidDraws.push_back(draw);
}

void Gizmos::DrawSphere(glm::vec3 center, float radius)
{
    if (!IsEnabled()) return;

    DebugDraw draw;
    draw.mMesh = LoadAsset<StaticMesh>("SM_Sphere");
    draw.mColor = sColor;
    draw.mTransform = BuildTransform(sMatrix, center, glm::vec3(0.0f), glm::vec3(radius));
    sSolidDraws.push_back(draw);
}

void Gizmos::DrawMesh(StaticMesh* mesh, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
{
    if (!IsEnabled() || mesh == nullptr) return;

    DebugDraw draw;
    draw.mMesh = mesh;
    draw.mColor = sColor;
    draw.mTransform = BuildTransform(sMatrix, position, rotation, scale);
    sSolidDraws.push_back(draw);
}

void Gizmos::DrawWireCube(glm::vec3 center, glm::vec3 size)
{
    if (!IsEnabled()) return;

    DebugDraw draw;
    draw.mMesh = LoadAsset<StaticMesh>("SM_Cube");
    draw.mColor = sColor;
    draw.mTransform = BuildTransform(sMatrix, center, glm::vec3(0.0f), size);
    sWireDraws.push_back(draw);
}

void Gizmos::DrawWireSphere(glm::vec3 center, float radius)
{
    if (!IsEnabled()) return;

    DebugDraw draw;
    draw.mMesh = LoadAsset<StaticMesh>("SM_Sphere");
    draw.mColor = sColor;
    draw.mTransform = BuildTransform(sMatrix, center, glm::vec3(0.0f), glm::vec3(radius));
    sWireDraws.push_back(draw);
}

void Gizmos::DrawWireMesh(StaticMesh* mesh, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
{
    if (!IsEnabled() || mesh == nullptr) return;

    DebugDraw draw;
    draw.mMesh = mesh;
    draw.mColor = sColor;
    draw.mTransform = BuildTransform(sMatrix, position, rotation, scale);
    sWireDraws.push_back(draw);
}

void Gizmos::DrawLine(glm::vec3 from, glm::vec3 to)
{
    if (!IsEnabled()) return;

    glm::vec3 worldFrom = glm::vec3(sMatrix * glm::vec4(from, 1.0f));
    glm::vec3 worldTo = glm::vec3(sMatrix * glm::vec4(to, 1.0f));
    sLines.push_back(Line(worldFrom, worldTo, sColor, 0.0f));
}

void Gizmos::DrawRay(glm::vec3 origin, glm::vec3 direction)
{
    DrawLine(origin, origin + direction);
}

void Gizmos::DrawFrustum(const glm::mat4& viewProjectionMatrix)
{
    if (!IsEnabled()) return;

    glm::mat4 inv = glm::inverse(viewProjectionMatrix);

    // NDC corners
    glm::vec4 ndcCorners[8] = {
        { -1, -1, -1, 1 }, // near bottom-left
        {  1, -1, -1, 1 }, // near bottom-right
        {  1,  1, -1, 1 }, // near top-right
        { -1,  1, -1, 1 }, // near top-left
        { -1, -1,  1, 1 }, // far bottom-left
        {  1, -1,  1, 1 }, // far bottom-right
        {  1,  1,  1, 1 }, // far top-right
        { -1,  1,  1, 1 }, // far top-left
    };

    glm::vec3 corners[8];
    for (int i = 0; i < 8; ++i)
    {
        glm::vec4 world = inv * ndcCorners[i];
        corners[i] = glm::vec3(world) / world.w;
    }

    // Near face
    DrawLine(corners[0], corners[1]);
    DrawLine(corners[1], corners[2]);
    DrawLine(corners[2], corners[3]);
    DrawLine(corners[3], corners[0]);

    // Far face
    DrawLine(corners[4], corners[5]);
    DrawLine(corners[5], corners[6]);
    DrawLine(corners[6], corners[7]);
    DrawLine(corners[7], corners[4]);

    // Connecting edges
    DrawLine(corners[0], corners[4]);
    DrawLine(corners[1], corners[5]);
    DrawLine(corners[2], corners[6]);
    DrawLine(corners[3], corners[7]);
}

void Gizmos::DrawLineList(const glm::vec3* points, uint32_t count)
{
    if (!IsEnabled() || points == nullptr) return;

    for (uint32_t i = 0; i + 1 < count; i += 2)
    {
        DrawLine(points[i], points[i + 1]);
    }
}

void Gizmos::DrawLineStrip(const glm::vec3* points, uint32_t count)
{
    if (!IsEnabled() || points == nullptr) return;

    for (uint32_t i = 0; i + 1 < count; ++i)
    {
        DrawLine(points[i], points[i + 1]);
    }
}

void Gizmos::BeginFrame()
{
    sSolidDraws.clear();
    sWireDraws.clear();
    sLines.clear();
}

const std::vector<DebugDraw>& Gizmos::GetSolidDraws()
{
    return sSolidDraws;
}

const std::vector<DebugDraw>& Gizmos::GetWireDraws()
{
    return sWireDraws;
}

const std::vector<Line>& Gizmos::GetLines()
{
    return sLines;
}

bool Gizmos::IsEnabled()
{
#if EDITOR
    return true;
#else
    return false;
#endif
}
