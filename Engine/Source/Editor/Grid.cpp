#if EDITOR

#include "Grid.h"
#include "Line.h"
#include "Engine.h"
#include "World.h"
#include "Preferences/PreferencesManager.h"
#include "Preferences/Appearance/Viewport/ViewportModule.h"

static bool sGridEnabled = false;
static bool sGridInitialized = false;

static const float sGridResolution = 1.0f;
static const int32_t sGridSize = 20;
static const float sGridSpan = sGridSize / sGridResolution;
static const float sGridHalfSpan = sGridSpan / 2.0f;
static const int32_t sNumGridLines = sGridSize + 1;
static const glm::vec4 sAxisXColor = glm::vec4(0.7f, 0.3f, 0.3f, 1.0f);
static const glm::vec4 sAxisZColor = glm::vec4(0.3f, 0.3f, 0.7f, 1.0f);
static glm::vec4 sGridColor = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
static Line sGridLinesX[sNumGridLines] = {};
static Line sGridLinesZ[sNumGridLines] = {};

static void RefreshGridLineColors()
{
    const int32_t axisIdx = sGridSize / 2;
    for (int32_t i = 0; i < sNumGridLines; ++i)
    {
        if (i == axisIdx)
        {
            sGridLinesX[i].mColor = sAxisXColor;
            sGridLinesZ[i].mColor = sAxisZColor;
        }
        else
        {
            sGridLinesX[i].mColor = sGridColor;
            sGridLinesZ[i].mColor = sGridColor;
        }
    }
}

static void RemoveGridLinesFromWorld(World* world)
{
    if (world == nullptr)
        return;

    for (int32_t i = 0; i < sNumGridLines; ++i)
    {
        world->RemoveLine(sGridLinesX[i]);
        world->RemoveLine(sGridLinesZ[i]);
    }
}

static void AddGridLinesToWorld(World* world)
{
    if (world == nullptr)
        return;

    for (int32_t i = 0; i < sNumGridLines; ++i)
    {
        world->AddLine(sGridLinesX[i]);
        world->AddLine(sGridLinesZ[i]);
    }
}

void InitializeGrid()
{
    float pos = -sGridHalfSpan;
    for (int32_t i = 0; i < sNumGridLines; ++i)
    {
        sGridLinesX[i].mStart = glm::vec3(-sGridHalfSpan, 0, pos);
        sGridLinesX[i].mEnd = glm::vec3(sGridHalfSpan, 0, pos);
        sGridLinesX[i].mLifetime = -1.0f;

        sGridLinesZ[i].mStart = glm::vec3(pos, 0, -sGridHalfSpan);
        sGridLinesZ[i].mEnd = glm::vec3(pos, 0, sGridHalfSpan);
        sGridLinesZ[i].mLifetime = -1.0f;

        pos += sGridResolution;
    }

    RefreshGridLineColors();
    sGridInitialized = true;

    if (sGridEnabled)
    {
        AddGridLinesToWorld(GetWorld(0));
    }
}

void EnableGrid(bool enable)
{
    sGridEnabled = enable;

    if (!sGridInitialized)
        return;

    World* world = GetWorld(0);
    if (enable)
    {
        AddGridLinesToWorld(world);
    }
    else
    {
        RemoveGridLinesFromWorld(world);
    }

    ViewportModule::HandleExternalGridToggle(enable);
}

bool IsGridEnabled()
{
    return sGridEnabled;
}

void ToggleGrid()
{
    EnableGrid(!IsGridEnabled());
}

void SetGridColor(const glm::vec4& color)
{
    if (sGridColor == color)
        return;

    World* world = nullptr;
    if (sGridInitialized && sGridEnabled)
    {
        world = GetWorld(0);
        RemoveGridLinesFromWorld(world);
    }

    sGridColor = color;
    RefreshGridLineColors();

    if (sGridInitialized && sGridEnabled)
    {
        AddGridLinesToWorld(world);
    }
}

glm::vec4 GetGridColor()
{
    return sGridColor;
}

#endif
