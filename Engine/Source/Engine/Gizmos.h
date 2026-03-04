#pragma once

#include "EngineTypes.h"
#include "Line.h"
#include "Maths.h"

#include <vector>

class StaticMesh;

class Gizmos
{
public:

    // State
    static void SetColor(glm::vec4 color);
    static glm::vec4 GetColor();
    static void SetMatrix(const glm::mat4& matrix);
    static const glm::mat4& GetMatrix();
    static void ResetState();

    // Solid draws (Forward pipeline)
    static void DrawCube(glm::vec3 center, glm::vec3 size);
    static void DrawSphere(glm::vec3 center, float radius);
    static void DrawMesh(StaticMesh* mesh, glm::vec3 position, glm::vec3 rotation = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f));

    // Wire draws (Wireframe pipeline)
    static void DrawWireCube(glm::vec3 center, glm::vec3 size);
    static void DrawWireSphere(glm::vec3 center, float radius);
    static void DrawWireMesh(StaticMesh* mesh, glm::vec3 position, glm::vec3 rotation = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f));

    // Lines (Line pipeline)
    static void DrawLine(glm::vec3 from, glm::vec3 to);
    static void DrawRay(glm::vec3 origin, glm::vec3 direction);
    static void DrawFrustum(const glm::mat4& viewProjectionMatrix);
    static void DrawLineList(const glm::vec3* points, uint32_t count);
    static void DrawLineStrip(const glm::vec3* points, uint32_t count);

    // Internal (called by Renderer)
    static void BeginFrame();
    static const std::vector<DebugDraw>& GetSolidDraws();
    static const std::vector<DebugDraw>& GetWireDraws();
    static const std::vector<Line>& GetLines();
    static bool IsEnabled();

private:

    static glm::vec4 sColor;
    static glm::mat4 sMatrix;
    static std::vector<DebugDraw> sSolidDraws;
    static std::vector<DebugDraw> sWireDraws;
    static std::vector<Line> sLines;
};
