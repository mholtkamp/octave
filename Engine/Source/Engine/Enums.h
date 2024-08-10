#pragma once

#include <stdint.h>

#define INDEX(x) static_cast<uint32_t>(x)

enum TextureSlot
{
    TEXTURE_0,
    TEXTURE_1,
    TEXTURE_2,
    TEXTURE_3
};

enum GlobalDescriptor
{
    GLD_UNIFORM_BUFFER,
    GLD_SHADOW_MAP
};

enum GeometryDescriptor
{
    GD_UNIFORM_BUFFER,
    GD_INSTANCE_DATA_BUFFER,
};

enum MaterialDescriptor
{
    MD_UNIFORM_BUFFER,
    MD_TEXTURE_START
};

enum class ProjectionMode : uint8_t
{
    ORTHOGRAPHIC,
    PERSPECTIVE
};

enum DebugMode
{
    DEBUG_NONE,
    DEBUG_SHADOW_MAP,
    DEBUG_WIREFRAME,
    DEBUG_COLLISION
};

enum class BoundsDebugMode : uint8_t
{
    Off,
    All,
    Selected,

    Count
};
