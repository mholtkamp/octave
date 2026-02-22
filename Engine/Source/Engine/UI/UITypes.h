#pragma once

#include "glm/glm.hpp"

#include <string>
#include <vector>
#include <utility>

class Widget;

//------------------------------------------------------------
// CSS Value
//------------------------------------------------------------

enum class CSSValueType : uint8_t
{
    None,
    Number,
    Percent,
    Color,
    String,
    Enum,
    Auto
};

struct CSSValue
{
    CSSValueType mType = CSSValueType::None;
    float mNumber = 0.0f;
    glm::vec4 mColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    std::string mString;

    bool IsValid() const { return mType != CSSValueType::None; }

    static CSSValue MakeNumber(float val);
    static CSSValue MakePercent(float val);
    static CSSValue MakeColor(glm::vec4 col);
    static CSSValue MakeString(const std::string& str);
    static CSSValue MakeEnum(const std::string& str);
};

//------------------------------------------------------------
// Element → Widget type mapping
//------------------------------------------------------------

const char* UIElementToWidgetType(const char* elementName);

//------------------------------------------------------------
// CSS parsing utilities
//------------------------------------------------------------

CSSValue ParseCSSColor(const char* str);
CSSValue ParseCSSLength(const char* str);
CSSValue ParseCSSGenericValue(const char* str);

void ParseInlineStyle(
    const char* styleStr,
    std::vector<std::pair<std::string, CSSValue>>& outProps);

//------------------------------------------------------------
// CSS → Widget property application
//------------------------------------------------------------

void ApplyCSSProperty(
    Widget* widget,
    const char* elementType,
    const std::string& propName,
    const CSSValue& value);
