#pragma once

#if EDITOR

#include <string>
#include "Maths.h"
#include "document.h"

namespace JsonSettings
{
    // File operations
    bool LoadFromFile(const std::string& path, rapidjson::Document& doc);
    bool SaveToFile(const std::string& path, rapidjson::Document& doc);

    // Getters with defaults
    bool GetBool(const rapidjson::Document& doc, const char* key, bool defaultValue);
    int GetInt(const rapidjson::Document& doc, const char* key, int defaultValue);
    float GetFloat(const rapidjson::Document& doc, const char* key, float defaultValue);
    std::string GetString(const rapidjson::Document& doc, const char* key, const std::string& defaultValue);
    glm::vec3 GetVec3(const rapidjson::Document& doc, const char* key, const glm::vec3& defaultValue);
    glm::vec4 GetVec4(const rapidjson::Document& doc, const char* key, const glm::vec4& defaultValue);

    // Setters
    void SetBool(rapidjson::Document& doc, const char* key, bool value);
    void SetInt(rapidjson::Document& doc, const char* key, int value);
    void SetFloat(rapidjson::Document& doc, const char* key, float value);
    void SetString(rapidjson::Document& doc, const char* key, const std::string& value);
    void SetVec3(rapidjson::Document& doc, const char* key, const glm::vec3& value);
    void SetVec4(rapidjson::Document& doc, const char* key, const glm::vec4& value);

    // Utility
    std::string GetPreferencesDirectory();
    void EnsurePreferencesDirectory();
}

#endif
