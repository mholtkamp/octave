#if EDITOR

#include "Grid.h"
#include "Line.h"
#include "Engine.h"
#include "World.h"

static bool sGridEnabled = false;

static const float sGridResolution = 1.0f;
static const int32_t sGridSize = 20;
static const float sGridSpan = sGridSize / sGridResolution;
static const float sGridHalfSpan = sGridSpan / 2.0f;
static const int32_t sNumGridLines = sGridSize + 1;
static const glm::vec4 sGridColor = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
static Line sGridLinesX[sNumGridLines] = {};
static Line sGridLinesZ[sNumGridLines] = {};

void InitializeGrid()
{
    float pos = -sGridHalfSpan;
    for (int32_t i = 0; i < sNumGridLines; ++i)
    {
        if (i == sGridSize / 2)
        {
            sGridLinesX[i].mColor = glm::vec4(0.7f, 0.3f, 0.3f, 1.0f);
            sGridLinesZ[i].mColor = glm::vec4(0.3f, 0.3f, 0.7f, 1.0f);
        }
        else
        {
            sGridLinesX[i].mColor = sGridColor;
            sGridLinesZ[i].mColor = sGridColor;
        }

        sGridLinesX[i].mStart = glm::vec3(-sGridHalfSpan, 0, pos);
        sGridLinesX[i].mEnd = glm::vec3(sGridHalfSpan, 0, pos);
        sGridLinesX[i].mLifetime = -1.0f;

        sGridLinesZ[i].mStart = glm::vec3(pos, 0, -sGridHalfSpan);
        sGridLinesZ[i].mEnd = glm::vec3(pos, 0, sGridHalfSpan);
        sGridLinesZ[i].mLifetime = -1.0f;

        pos += sGridResolution;
    }
}

void EnableGrid(bool enable)
{
    World* world = GetWorld(0);
    if (world == nullptr)
        return;

    sGridEnabled = enable;

    if (enable)
    {
        for (int32_t i = 0; i < sNumGridLines; ++i)
        {
            world->AddLine(sGridLinesX[i]);
            world->AddLine(sGridLinesZ[i]);
        }
    }
    else
    {
        for (int32_t i = 0; i < sNumGridLines; ++i)
        {
            world->RemoveLine(sGridLinesX[i]);
            world->RemoveLine(sGridLinesZ[i]);
        }
    }
}

bool IsGridEnabled()
{
    return sGridEnabled;
}

void ToggleGrid()
{
    EnableGrid(!IsGridEnabled());
}

#endif
