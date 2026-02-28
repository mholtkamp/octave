#pragma once

#include "Datum.h"
#include "Maths.h"

#include <string>
#include <unordered_map>
#include <vector>

class PointCloud
{
public:

    struct AttribArray
    {
        DatumType mType = DatumType::Count;
        std::vector<float> mFloats;
        std::vector<int32_t> mInts;
        std::vector<glm::vec3> mVectors;
        std::vector<glm::vec4> mColors;
        std::vector<std::string> mStrings;
    };

    PointCloud() = default;
    ~PointCloud() = default;

    uint32_t GetNumPoints() const;
    void Resize(uint32_t count);
    void AddPoint(const glm::vec3& pos);
    void RemovePoint(uint32_t index);
    void Clear();

    // Position (always present)
    const glm::vec3& GetPosition(uint32_t i) const;
    void SetPosition(uint32_t i, const glm::vec3& p);
    const std::vector<glm::vec3>& GetPositions() const;

    // Generic attribute CRUD
    void AddAttribute(const std::string& name, DatumType type);
    bool HasAttribute(const std::string& name) const;
    DatumType GetAttributeType(const std::string& name) const;
    float GetAttributeFloat(uint32_t pointIndex, const std::string& name) const;
    int32_t GetAttributeInt(uint32_t pointIndex, const std::string& name) const;
    glm::vec3 GetAttributeVector(uint32_t pointIndex, const std::string& name) const;
    glm::vec4 GetAttributeColor(uint32_t pointIndex, const std::string& name) const;
    const std::string& GetAttributeString(uint32_t pointIndex, const std::string& name) const;
    void SetAttributeFloat(uint32_t pointIndex, const std::string& name, float value);
    void SetAttributeInt(uint32_t pointIndex, const std::string& name, int32_t value);
    void SetAttributeVector(uint32_t pointIndex, const std::string& name, const glm::vec3& value);
    void SetAttributeColor(uint32_t pointIndex, const std::string& name, const glm::vec4& value);
    void SetAttributeString(uint32_t pointIndex, const std::string& name, const std::string& value);

    // Well-known attribute shortcuts
    float GetPScale(uint32_t i) const;
    glm::vec4 GetCd(uint32_t i) const;
    glm::vec3 GetNormal(uint32_t i) const;
    glm::vec3 GetVelocity(uint32_t i) const;

    // Merge another cloud (appends points + matching attributes)
    void Merge(const PointCloud& other);

    // Deep copy
    PointCloud* Clone() const;

    const std::unordered_map<std::string, AttribArray>& GetAttributes() const { return mAttributes; }

private:

    std::vector<glm::vec3> mPositions;
    std::unordered_map<std::string, AttribArray> mAttributes;
};
