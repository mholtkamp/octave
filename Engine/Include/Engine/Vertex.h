#pragma once

#include "Constants.h"

#include <glm/glm.hpp>

enum class VertexType
{
    Vertex,
    VertexColor,
    VertexUI,
    VertexColorSimple,
    VertexSkinned,
    VertexParticle,
    Max
};

struct Vertex
{
    glm::vec3 mPosition;
    glm::vec2 mTexcoord;
    glm::vec3 mNormal;
};

struct VertexColor
{
    glm::vec3 mPosition;
    glm::vec2 mTexcoord;
    glm::vec3 mNormal;
    uint32_t mColor;
};

struct VertexUI
{
    glm::vec2 mPosition;
    glm::vec2 mTexcoord;
    uint32_t mColor;
};

struct VertexColorSimple
{
    glm::vec3 mPosition;
    uint32_t mColor;
};

struct VertexSkinned
{
    glm::vec3 mPosition;
    glm::vec2 mTexcoord;
    glm::vec3 mNormal;
    uint8_t mBoneIndices[MAX_BONE_INFLUENCES];
    float mBoneWeights[MAX_BONE_INFLUENCES];
};

struct VertexParticle
{
    glm::vec3 mPosition;
    glm::vec2 mTexcoord;
    uint32_t mColor;
};
