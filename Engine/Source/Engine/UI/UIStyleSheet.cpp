#include "UI/UIStyleSheet.h"
#include "Nodes/Widgets/Widget.h"
#include "Log.h"

#include <cstring>
#include <cctype>
#include <algorithm>

//------------------------------------------------------------
// CSSSelector
//------------------------------------------------------------

int32_t CSSSelector::GetSpecificity() const
{
    switch (mType)
    {
    case CSSSelectorType::Id:      return 100;
    case CSSSelectorType::Class:   return 10;
    case CSSSelectorType::Element: return 1;
    default: return 0;
    }
}

//------------------------------------------------------------
// UIStyleSheet - parsing helpers
//------------------------------------------------------------

void UIStyleSheet::SkipWhitespace(const char*& cursor)
{
    while (*cursor && isspace((unsigned char)*cursor))
        cursor++;
}

void UIStyleSheet::SkipComments(const char*& cursor)
{
    if (cursor[0] == '/' && cursor[1] == '*')
    {
        cursor += 2;
        while (*cursor && !(cursor[0] == '*' && cursor[1] == '/'))
            cursor++;
        if (*cursor) cursor += 2; // skip */
    }
}

void UIStyleSheet::SkipWhitespaceAndComments(const char*& cursor)
{
    while (*cursor)
    {
        SkipWhitespace(cursor);
        if (cursor[0] == '/' && cursor[1] == '*')
            SkipComments(cursor);
        else
            break;
    }
}

std::string UIStyleSheet::ReadIdentifier(const char*& cursor)
{
    std::string result;
    while (*cursor && (isalnum((unsigned char)*cursor) || *cursor == '-' || *cursor == '_'))
    {
        result += *cursor;
        cursor++;
    }
    return result;
}

std::string UIStyleSheet::ReadUntil(const char*& cursor, char delim)
{
    std::string result;
    while (*cursor && *cursor != delim)
    {
        result += *cursor;
        cursor++;
    }
    return result;
}

//------------------------------------------------------------
// UIStyleSheet - parsing
//------------------------------------------------------------

void UIStyleSheet::Parse(const std::string& cssSource)
{
    Clear();
    ParseAndAppend(cssSource);
}

void UIStyleSheet::ParseAndAppend(const std::string& cssSource)
{
    const char* cursor = cssSource.c_str();

    while (*cursor)
    {
        SkipWhitespaceAndComments(cursor);
        if (!*cursor) break;

        // Parse selector
        CSSSelector selector;

        if (*cursor == '#')
        {
            cursor++; // skip #
            selector.mType = CSSSelectorType::Id;
            selector.mValue = ReadIdentifier(cursor);
        }
        else if (*cursor == '.')
        {
            cursor++; // skip .
            selector.mType = CSSSelectorType::Class;
            selector.mValue = ReadIdentifier(cursor);
        }
        else if (isalpha((unsigned char)*cursor) || *cursor == '_')
        {
            selector.mType = CSSSelectorType::Element;
            selector.mValue = ReadIdentifier(cursor);
        }
        else
        {
            // Unexpected character, skip it
            cursor++;
            continue;
        }

        if (selector.mValue.empty())
        {
            cursor++;
            continue;
        }

        SkipWhitespaceAndComments(cursor);

        // Expect '{'
        if (*cursor != '{')
        {
            // Skip to next '{' or give up on this rule
            while (*cursor && *cursor != '{') cursor++;
            if (!*cursor) break;
        }
        cursor++; // skip '{'

        // Parse property block
        CSSRule rule;
        rule.mSelector = selector;

        while (*cursor)
        {
            SkipWhitespaceAndComments(cursor);
            if (*cursor == '}') { cursor++; break; }
            if (!*cursor) break;

            // Read property name (may start with -- for custom props)
            std::string propName;
            if (cursor[0] == '-' && cursor[1] == '-')
            {
                propName += '-'; cursor++;
                propName += '-'; cursor++;
                propName += ReadIdentifier(cursor);
            }
            else
            {
                propName = ReadIdentifier(cursor);
            }

            SkipWhitespace(cursor);

            // Expect ':'
            if (*cursor != ':')
            {
                // Skip to next ';' or '}'
                while (*cursor && *cursor != ';' && *cursor != '}') cursor++;
                if (*cursor == ';') cursor++;
                continue;
            }
            cursor++; // skip ':'

            SkipWhitespace(cursor);

            // Read value (everything until ';' or '}')
            std::string valueStr;
            while (*cursor && *cursor != ';' && *cursor != '}')
            {
                valueStr += *cursor;
                cursor++;
            }
            if (*cursor == ';') cursor++;

            // Trim trailing whitespace from value
            while (!valueStr.empty() && isspace((unsigned char)valueStr.back()))
                valueStr.pop_back();

            if (!propName.empty() && !valueStr.empty())
            {
                CSSProperty prop;
                prop.mName = propName;
                prop.mValue = ParseCSSGenericValue(valueStr.c_str());

                if (prop.mValue.IsValid())
                {
                    rule.mProperties.push_back(prop);
                }
            }
        }

        if (!rule.mProperties.empty())
        {
            mRules.push_back(std::move(rule));
        }
    }
}

void UIStyleSheet::Clear()
{
    mRules.clear();
}

//------------------------------------------------------------
// UIStyleSheet - matching
//------------------------------------------------------------

static bool ClassListContains(const char* classList, const std::string& target)
{
    if (!classList || classList[0] == '\0') return false;

    const char* p = classList;
    while (*p)
    {
        while (*p && isspace((unsigned char)*p)) p++;
        const char* start = p;
        while (*p && !isspace((unsigned char)*p)) p++;

        if ((size_t)(p - start) == target.size() &&
            strncmp(start, target.c_str(), target.size()) == 0)
        {
            return true;
        }
    }
    return false;
}

void UIStyleSheet::GetMatchingRules(
    const char* elementType,
    const char* className,
    const char* id,
    std::vector<const CSSRule*>& outRules) const
{
    for (const CSSRule& rule : mRules)
    {
        bool match = false;

        switch (rule.mSelector.mType)
        {
        case CSSSelectorType::Element:
            if (elementType && strcmp(elementType, rule.mSelector.mValue.c_str()) == 0)
                match = true;
            break;

        case CSSSelectorType::Class:
            if (ClassListContains(className, rule.mSelector.mValue))
                match = true;
            break;

        case CSSSelectorType::Id:
            if (id && strcmp(id, rule.mSelector.mValue.c_str()) == 0)
                match = true;
            break;
        }

        if (match)
        {
            outRules.push_back(&rule);
        }
    }

    // Sort by specificity (lowest first, so later = higher priority)
    std::sort(outRules.begin(), outRules.end(),
        [](const CSSRule* a, const CSSRule* b)
        {
            return a->mSelector.GetSpecificity() < b->mSelector.GetSpecificity();
        });
}

void UIStyleSheet::ApplyStyles(
    Widget* widget,
    const char* elementType,
    const char* className,
    const char* id) const
{
    std::vector<const CSSRule*> matchingRules;
    GetMatchingRules(elementType, className, id, matchingRules);

    for (const CSSRule* rule : matchingRules)
    {
        for (const CSSProperty& prop : rule->mProperties)
        {
            ApplyCSSProperty(widget, elementType, prop.mName, prop.mValue);
        }
    }
}
