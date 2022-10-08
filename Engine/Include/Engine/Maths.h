#pragma once

#include <stdint.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/vector_angle.hpp>

#define PI 3.14159265359f
#define DEGREES_TO_RADIANS (PI / 180)
#define RADIANS_TO_DEGREES (180 / PI)

class Maths
{
public:

    template<typename T>
    static T RandRange(T min, T max)
    {
        int32_t retRand = min;
        if (max >= min)
        {
            retRand = min + rand() % (max + 1 - min);
        }
        return retRand;
    }

    static void SeedRand(uint32_t seed);

    static float Damp(float source, float target, float smoothing, float deltaTime);
    static glm::vec3 Damp(glm::vec3 source, glm::vec3 target, float smoothing, float deltaTime);
    static glm::vec4 Damp(glm::vec4 source, glm::vec4 target, float smoothing, float deltaTime);
    static float DampAngle(float source, float target, float smoothing, float deltaTime);

    static float Approach(float source, float target, float speed, float deltaTime);
    static float ApproachAngle(float source, float target, float speed, float deltaTime);

    static glm::vec3 SafeNormalize(glm::vec3 vector);

    static bool IsPowerOfTwo(uint32_t number);

    static glm::vec3 ExtractPosition(const glm::mat4& mat);
    static glm::quat ExtractRotation(const glm::mat4& mat);
    static glm::vec3 ExtractScale(const glm::mat4& mat);

    static float RotateYawTowardDirection(float srcYaw, glm::vec3 dir, float speed, float deltaTime);

private:

    static bool mRandSeeded;
};

template<>
float Maths::RandRange<float>(float min, float max);

template<>
glm::vec2 Maths::RandRange<glm::vec2>(glm::vec2 min, glm::vec2 max);

template<>
glm::vec3 Maths::RandRange<glm::vec3>(glm::vec3 min, glm::vec3 max);

template<>
glm::vec4 Maths::RandRange<glm::vec4>(glm::vec4 min, glm::vec4 max);
