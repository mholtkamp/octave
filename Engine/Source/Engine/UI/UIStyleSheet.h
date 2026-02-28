#pragma once

#include "UI/UITypes.h"

#include <string>
#include <vector>

//------------------------------------------------------------
// CSS Selector
//------------------------------------------------------------

enum class CSSSelectorType : uint8_t
{
    Element,    // div, button, etc.  (specificity 1)
    Class,      // .classname         (specificity 10)
    Id          // #idname            (specificity 100)
};

struct CSSSelector
{
    CSSSelectorType mType = CSSSelectorType::Element;
    std::string mValue;

    int32_t GetSpecificity() const;
};

//------------------------------------------------------------
// CSS Rule
//------------------------------------------------------------

struct CSSProperty
{
    std::string mName;
    CSSValue mValue;
};

struct CSSRule
{
    CSSSelector mSelector;
    std::vector<CSSProperty> mProperties;
};

//------------------------------------------------------------
// UIStyleSheet
//------------------------------------------------------------

class UIStyleSheet
{
public:

    void Parse(const std::string& cssSource);
    void ParseAndAppend(const std::string& cssSource);
    void Clear();

    void GetMatchingRules(
        const char* elementType,
        const char* className,
        const char* id,
        std::vector<const CSSRule*>& outRules) const;

    void ApplyStyles(
        Widget* widget,
        const char* elementType,
        const char* className,
        const char* id) const;

private:

    std::vector<CSSRule> mRules;

    static void SkipWhitespace(const char*& cursor);
    static void SkipComments(const char*& cursor);
    static void SkipWhitespaceAndComments(const char*& cursor);
    static std::string ReadIdentifier(const char*& cursor);
    static std::string ReadUntil(const char*& cursor, char delim);
};
