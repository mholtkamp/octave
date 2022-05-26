#include "Maths.h"
#include <time.h>
#include <stdlib.h>

#define USE_GLM_MATRIX_DECOMPOSE_TRANSLATION 0
#define USE_GLM_MATRIX_DECOMPOSE_ROTATION 1
#define USE_GLM_MATRIX_DECOMPOSE_SCALE 1

bool Maths::mRandSeeded = false;

template<>
float Maths::RandRange<float>(float min, float max)
{
    float randFloat = rand() / static_cast<float>(RAND_MAX);
    float retRand = min + (max - min) * randFloat;
    return retRand;
}

template<>
glm::vec2 Maths::RandRange<glm::vec2>(glm::vec2 min, glm::vec2 max)
{
    glm::vec2 retRand;
    retRand.x = RandRange(min.x, max.x);
    retRand.y = RandRange(min.y, max.y);
    return retRand;
}

template<>
glm::vec3 Maths::RandRange<glm::vec3>(glm::vec3 min, glm::vec3 max)
{
    glm::vec3 retRand;
    retRand.x = RandRange(min.x, max.x);
    retRand.y = RandRange(min.y, max.y);
    retRand.z = RandRange(min.z, max.z);
    return retRand;
}

template<>
glm::vec4 Maths::RandRange<glm::vec4>(glm::vec4 min, glm::vec4 max)
{
    glm::vec4 retRand;
    retRand.x = RandRange(min.x, max.x);
    retRand.y = RandRange(min.y, max.y);
    retRand.z = RandRange(min.z, max.z);
    retRand.w = RandRange(min.w, max.w);
    return retRand;
}

float Maths::Damp(float source, float target, float smoothing, float dt)
{
    return glm::mix(source, target, 1.0f - glm::pow(smoothing, dt));
}

glm::vec3 Maths::Damp(glm::vec3 source, glm::vec3 target, float smoothing, float dt)
{
    glm::vec3 ret;
    ret.x = Damp(source.x, target.x, smoothing, dt);
    ret.y = Damp(source.y, target.y, smoothing, dt);
    ret.z = Damp(source.z, target.z, smoothing, dt);
    return ret;
}

glm::vec4 Maths::Damp(glm::vec4 source, glm::vec4 target, float smoothing, float dt)
{
    glm::vec4 ret;
    ret.x = Damp(source.x, target.x, smoothing, dt);
    ret.y = Damp(source.y, target.y, smoothing, dt);
    ret.z = Damp(source.z, target.z, smoothing, dt);
    ret.w = Damp(source.w, target.w, smoothing, dt);
    return ret;
}

float Maths::DampAngle(float source, float target, float smoothing, float deltaTime)
{
    float ret = source;

    // TODO: Might need to ensure source and target are in -180 to 180 degree range.

    // (Source vs Target yaws should be within 180 degrees of each other)
    if (fabs(target - source) <= 180.0f)
    {
        ret = Maths::Damp(source, target, smoothing, deltaTime);
    }
    else
    {
        // Convert to 0 to 360 range
        if (source < 0.0f)
            source += 360.0f;
        if (target < 0.0f)
            target += 360.0f;

        ret = Maths::Damp(source, target, smoothing, deltaTime);

        // Convert current yaw back into -180 to 180 range
        if (ret > 180.0f)
        {
            ret -= 360.0f;
        }
    }

    return ret;
}

float Maths::Approach(float source, float target, float speed, float deltaTime)
{
    float smoothed = source;

    if (target > source)
    {
        smoothed = source + speed * deltaTime;
        smoothed = glm::min(smoothed, target);
    }
    else if (target < source)
    {
        smoothed = source - speed * deltaTime;
        smoothed = glm::max(smoothed, target);
    }

    return smoothed;
}

float Maths::ApproachAngle(float source, float target, float speed, float deltaTime)
{
    float ret = source;

    // TODO: Might need to ensure source and target are in -180 to 180 degree range.

    // (Source vs Target yaws should be within 180 degrees of each other)
    if (fabs(target - source) <= 180.0f)
    {
        ret = Maths::Approach(source, target, speed, deltaTime);
    }
    else
    {
        // Convert to 0 to 360 range
        if (source < 0.0f)
            source += 360.0f;
        if (target < 0.0f)
            target += 360.0f;

        ret = Maths::Approach(source, target, speed, deltaTime);

        // Convert current yaw back into -180 to 180 range
        if (ret > 180.0f)
        {
            ret -= 360.0f;
        }
    }

    return ret;
}

glm::vec3 Maths::SafeNormalize(glm::vec3 vector)
{
    glm::vec3 normalizedVector = glm::vec3(0.0f, 0.0f, 0.0f);
    float length = glm::length(vector);

    if (length > 0.0f)
    {
        normalizedVector = vector / length;
    }

    return normalizedVector;
}

bool Maths::IsPowerOfTwo(uint32_t x)
{
    return (x != 0) && ((x & (x - 1)) == 0);
}

glm::vec3 Maths::ExtractPosition(const glm::mat4& mat)
{
#if USE_GLM_MATRIX_DECOMPOSE_TRANSLATION
    glm::vec3 translation;
    glm::vec3 scale;
    glm::quat orientation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(mat, scale, orientation, translation, skew, perspective);

    return translation;
#else
    glm::vec3 worldPos;
    worldPos.x = mat[3][0];
    worldPos.y = mat[3][1];
    worldPos.z = mat[3][2];
    return worldPos;
#endif
}

glm::quat Maths::ExtractRotation(const glm::mat4& mat)
{
#if USE_GLM_MATRIX_DECOMPOSE_ROTATION
    glm::vec3 translation;
    glm::vec3 scale;
    glm::quat orientation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(mat, scale, orientation, translation, skew, perspective);

    glm::quat fixedRotation = glm::conjugate(orientation);
    return fixedRotation;
#else
    // Multiply mTransform by forward vector. Then convert vector to euler angles.
    // Or.... use this snippet I found from stack overflow?
    float pitch = 0.0f;
    float yaw = 0.0f;
    float roll = 0.0f;

    glm::mat4 m = mat;
    glm::vec3 s = ExtractScale(m);

    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = 0.0f;

    m[0][0] /= s.x;
    m[0][1] /= s.x;
    m[0][2] /= s.x;

    m[1][0] /= s.y;
    m[1][1] /= s.y;
    m[1][2] /= s.y;

    m[2][0] /= s.z;
    m[2][1] /= s.z;
    m[2][2] /= s.z;

    yaw = atan2f(-m[0][2], m[0][0]);
    roll = asinf(m[0][1]);
    pitch = atan2f(-m[2][1], m[1][1]);

    return glm::vec3(pitch, yaw, roll) * RADIANS_TO_DEGREES;
#endif
}

glm::vec3 Maths::ExtractScale(const glm::mat4& mat)
{
#if USE_GLM_MATRIX_DECOMPOSE_SCALE
    glm::vec3 translation;
    glm::vec3 scale;
    glm::quat orientation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(mat, scale, orientation, translation, skew, perspective);

    return scale;
#else
    const glm::mat4& m = mat;
    float sx = glm::length(glm::vec3(m[0][0], m[0][1], m[0][2]));
    float sy = glm::length(glm::vec3(m[1][0], m[1][1], m[1][2]));
    float sz = glm::length(glm::vec3(m[2][0], m[2][1], m[2][2]));
    return glm::vec3(sx, sy, sz);
#endif
}


void Maths::SeedRand(uint32_t seed)
{
    srand(seed);
}