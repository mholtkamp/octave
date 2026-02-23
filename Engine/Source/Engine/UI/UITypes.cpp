#include "UI/UITypes.h"

#include "Nodes/Widgets/Widget.h"
#include "Nodes/Widgets/Canvas.h"
#include "Nodes/Widgets/Quad.h"
#include "Nodes/Widgets/Text.h"
#include "Nodes/Widgets/Button.h"
#include "Nodes/Widgets/ArrayWidget.h"
#include "Nodes/Widgets/Poly.h"
#include "Nodes/Widgets/PolyRect.h"
#include "AssetManager.h"
#include "Assets/Texture.h"
#include "Assets/Font.h"
#include "Log.h"

#include <cstring>
#include <cstdlib>
#include <cctype>
#include <cmath>

//------------------------------------------------------------
// CSSValue factories
//------------------------------------------------------------

CSSValue CSSValue::MakeNumber(float val)
{
    CSSValue v;
    v.mType = CSSValueType::Number;
    v.mNumber = val;
    return v;
}

CSSValue CSSValue::MakePercent(float val)
{
    CSSValue v;
    v.mType = CSSValueType::Percent;
    v.mNumber = val;
    return v;
}

CSSValue CSSValue::MakeColor(glm::vec4 col)
{
    CSSValue v;
    v.mType = CSSValueType::Color;
    v.mColor = col;
    return v;
}

CSSValue CSSValue::MakeString(const std::string& str)
{
    CSSValue v;
    v.mType = CSSValueType::String;
    v.mString = str;
    return v;
}

CSSValue CSSValue::MakeEnum(const std::string& str)
{
    CSSValue v;
    v.mType = CSSValueType::Enum;
    v.mString = str;
    return v;
}

//------------------------------------------------------------
// Element → Widget type mapping
//------------------------------------------------------------

const char* UIElementToWidgetType(const char* elementName)
{
    if (!elementName) return nullptr;

    if (strcmp(elementName, "div") == 0)       return "Canvas";
    if (strcmp(elementName, "flex") == 0)      return "ArrayWidget";
    if (strcmp(elementName, "img") == 0)       return "Quad";
    if (strcmp(elementName, "text") == 0)      return "Text";
    if (strcmp(elementName, "p") == 0)         return "Text";
    if (strcmp(elementName, "span") == 0)      return "Text";
    if (strcmp(elementName, "button") == 0)    return "Button";
    if (strcmp(elementName, "canvas") == 0)    return "Canvas";
    if (strcmp(elementName, "poly") == 0)      return "Poly";
    if (strcmp(elementName, "polyrect") == 0)  return "PolyRect";

    // Allow direct widget type names (PascalCase)
    if (strcmp(elementName, "Canvas") == 0)       return "Canvas";
    if (strcmp(elementName, "Quad") == 0)          return "Quad";
    if (strcmp(elementName, "Text") == 0)          return "Text";
    if (strcmp(elementName, "Button") == 0)        return "Button";
    if (strcmp(elementName, "ArrayWidget") == 0)   return "ArrayWidget";
    if (strcmp(elementName, "Poly") == 0)          return "Poly";
    if (strcmp(elementName, "PolyRect") == 0)      return "PolyRect";

    return nullptr;
}

//------------------------------------------------------------
// CSS color parsing
//------------------------------------------------------------

static uint8_t HexCharToNibble(char c)
{
    if (c >= '0' && c <= '9') return (uint8_t)(c - '0');
    if (c >= 'a' && c <= 'f') return (uint8_t)(c - 'a' + 10);
    if (c >= 'A' && c <= 'F') return (uint8_t)(c - 'A' + 10);
    return 0;
}

CSSValue ParseCSSColor(const char* str)
{
    if (!str) return {};

    while (isspace((unsigned char)*str)) str++;

    // #RGB, #RRGGBB, #RRGGBBAA
    if (str[0] == '#')
    {
        str++;
        size_t len = strlen(str);

        glm::vec4 color(0.0f, 0.0f, 0.0f, 1.0f);

        if (len == 3)
        {
            color.r = HexCharToNibble(str[0]) / 15.0f;
            color.g = HexCharToNibble(str[1]) / 15.0f;
            color.b = HexCharToNibble(str[2]) / 15.0f;
        }
        else if (len >= 6)
        {
            color.r = (HexCharToNibble(str[0]) * 16 + HexCharToNibble(str[1])) / 255.0f;
            color.g = (HexCharToNibble(str[2]) * 16 + HexCharToNibble(str[3])) / 255.0f;
            color.b = (HexCharToNibble(str[4]) * 16 + HexCharToNibble(str[5])) / 255.0f;

            if (len >= 8)
            {
                color.a = (HexCharToNibble(str[6]) * 16 + HexCharToNibble(str[7])) / 255.0f;
            }
        }

        return CSSValue::MakeColor(color);
    }

    // rgba(r, g, b, a) or rgb(r, g, b)
    if (strncmp(str, "rgba(", 5) == 0 || strncmp(str, "rgb(", 4) == 0)
    {
        bool hasAlpha = (str[3] == 'a');
        const char* p = str + (hasAlpha ? 5 : 4);

        glm::vec4 color(0.0f, 0.0f, 0.0f, 1.0f);
        color.r = (float)atof(p) / 255.0f;

        p = strchr(p, ',');
        if (p) { p++; color.g = (float)atof(p) / 255.0f; }

        p = p ? strchr(p, ',') : nullptr;
        if (p) { p++; color.b = (float)atof(p) / 255.0f; }

        if (hasAlpha)
        {
            p = p ? strchr(p, ',') : nullptr;
            if (p) { p++; color.a = (float)atof(p); } // alpha is 0-1
        }

        return CSSValue::MakeColor(color);
    }

    // Named colors (common subset)
    if (strcmp(str, "white") == 0)       return CSSValue::MakeColor({ 1, 1, 1, 1 });
    if (strcmp(str, "black") == 0)       return CSSValue::MakeColor({ 0, 0, 0, 1 });
    if (strcmp(str, "red") == 0)         return CSSValue::MakeColor({ 1, 0, 0, 1 });
    if (strcmp(str, "green") == 0)       return CSSValue::MakeColor({ 0, 1, 0, 1 });
    if (strcmp(str, "blue") == 0)        return CSSValue::MakeColor({ 0, 0, 1, 1 });
    if (strcmp(str, "yellow") == 0)      return CSSValue::MakeColor({ 1, 1, 0, 1 });
    if (strcmp(str, "transparent") == 0) return CSSValue::MakeColor({ 0, 0, 0, 0 });
    if (strcmp(str, "gray") == 0)        return CSSValue::MakeColor({ 0.5f, 0.5f, 0.5f, 1 });
    if (strcmp(str, "grey") == 0)        return CSSValue::MakeColor({ 0.5f, 0.5f, 0.5f, 1 });

    return {};
}

//------------------------------------------------------------
// CSS length parsing
//------------------------------------------------------------

CSSValue ParseCSSLength(const char* str)
{
    if (!str) return {};

    while (isspace((unsigned char)*str)) str++;

    if (strcmp(str, "auto") == 0)
    {
        CSSValue v;
        v.mType = CSSValueType::Auto;
        return v;
    }

    char* end = nullptr;
    float val = strtof(str, &end);

    if (end == str) return {};

    // Check suffix
    if (end && *end == '%')
    {
        return CSSValue::MakePercent(val);
    }

    // "px" suffix or bare number → pixels
    return CSSValue::MakeNumber(val);
}

//------------------------------------------------------------
// Generic CSS value parsing
//------------------------------------------------------------

CSSValue ParseCSSGenericValue(const char* str)
{
    if (!str || *str == '\0') return {};

    while (isspace((unsigned char)*str)) str++;

    // Try color first
    if (str[0] == '#' || strncmp(str, "rgb", 3) == 0)
    {
        CSSValue col = ParseCSSColor(str);
        if (col.IsValid()) return col;
    }

    // Named colors
    {
        CSSValue col = ParseCSSColor(str);
        if (col.IsValid()) return col;
    }

    // url("...")
    if (strncmp(str, "url(", 4) == 0)
    {
        const char* start = str + 4;
        while (*start == '"' || *start == '\'') start++;
        const char* end = start;
        while (*end && *end != '"' && *end != '\'' && *end != ')') end++;
        return CSSValue::MakeString(std::string(start, end));
    }

    // Try number/length
    {
        char* end = nullptr;
        float val = strtof(str, &end);
        if (end != str)
        {
            if (end && *end == '%')
                return CSSValue::MakePercent(val);
            else
                return CSSValue::MakeNumber(val);
        }
    }

    // Keywords / enum / string
    if (strcmp(str, "none") == 0 ||
        strcmp(str, "flex") == 0 ||
        strcmp(str, "hidden") == 0 ||
        strcmp(str, "left") == 0 ||
        strcmp(str, "center") == 0 ||
        strcmp(str, "right") == 0 ||
        strcmp(str, "top") == 0 ||
        strcmp(str, "bottom") == 0 ||
        strcmp(str, "row") == 0 ||
        strcmp(str, "column") == 0 ||
        strcmp(str, "normal") == 0 ||
        strcmp(str, "break-word") == 0 ||
        strcmp(str, "fill") == 0 ||
        strcmp(str, "contain") == 0 ||
        strcmp(str, "cover") == 0 ||
        strcmp(str, "true") == 0 ||
        strcmp(str, "false") == 0 ||
        strncmp(str, "top-", 4) == 0 ||
        strncmp(str, "center-", 7) == 0 ||
        strncmp(str, "bottom-", 7) == 0 ||
        strncmp(str, "left-", 5) == 0 ||
        strncmp(str, "right-", 6) == 0 ||
        strcmp(str, "full-stretch") == 0)
    {
        return CSSValue::MakeEnum(str);
    }

    // Fall back to string
    return CSSValue::MakeString(str);
}

//------------------------------------------------------------
// Inline style parsing
//------------------------------------------------------------

void ParseInlineStyle(
    const char* styleStr,
    std::vector<std::pair<std::string, CSSValue>>& outProps)
{
    if (!styleStr) return;

    const char* p = styleStr;

    while (*p)
    {
        // Skip whitespace
        while (*p && isspace((unsigned char)*p)) p++;
        if (!*p) break;

        // Read property name
        const char* nameStart = p;
        while (*p && *p != ':' && *p != ';') p++;
        if (*p != ':') { if (*p) p++; continue; }

        std::string name(nameStart, p);
        // Trim trailing whitespace from name
        while (!name.empty() && isspace((unsigned char)name.back()))
            name.pop_back();

        p++; // skip ':'

        // Read value
        while (*p && isspace((unsigned char)*p)) p++;
        const char* valStart = p;
        while (*p && *p != ';') p++;

        std::string valStr(valStart, p);
        // Trim trailing whitespace
        while (!valStr.empty() && isspace((unsigned char)valStr.back()))
            valStr.pop_back();

        if (*p == ';') p++;

        if (!name.empty() && !valStr.empty())
        {
            CSSValue value = ParseCSSGenericValue(valStr.c_str());
            if (value.IsValid())
            {
                outProps.push_back({ name, value });
            }
        }
    }
}

//------------------------------------------------------------
// AnchorMode mapping
//------------------------------------------------------------

static AnchorMode ParseAnchorMode(const std::string& str)
{
    if (str == "top-left")           return AnchorMode::TopLeft;
    if (str == "top-center")         return AnchorMode::TopMid;
    if (str == "top-right")          return AnchorMode::TopRight;
    if (str == "center-left")        return AnchorMode::MidLeft;
    if (str == "center")             return AnchorMode::Mid;
    if (str == "center-right")       return AnchorMode::MidRight;
    if (str == "bottom-left")        return AnchorMode::BottomLeft;
    if (str == "bottom-center")      return AnchorMode::BottomMid;
    if (str == "bottom-right")       return AnchorMode::BottomRight;
    if (str == "top-stretch")        return AnchorMode::TopStretch;
    if (str == "center-h-stretch")   return AnchorMode::MidHorizontalStretch;
    if (str == "bottom-stretch")     return AnchorMode::BottomStretch;
    if (str == "left-stretch")       return AnchorMode::LeftStretch;
    if (str == "center-v-stretch")   return AnchorMode::MidVerticalStretch;
    if (str == "right-stretch")      return AnchorMode::RightStretch;
    if (str == "full-stretch")       return AnchorMode::FullStretch;
    return AnchorMode::TopLeft;
}

//------------------------------------------------------------
// Apply a single CSS property to a widget
//------------------------------------------------------------

void ApplyCSSProperty(
    Widget* widget,
    const char* elementType,
    const std::string& propName,
    const CSSValue& value)
{
    if (!widget) return;

    bool isButton = widget->Is("Button");
    bool isText = widget->Is("Text");
    bool isQuad = widget->Is("Quad");
    bool isArray = widget->Is("ArrayWidget");

    //--- Layout properties ---

    if (propName == "width")
    {
        if (value.mType == CSSValueType::Number)
            widget->SetWidth(value.mNumber);
        else if (value.mType == CSSValueType::Percent)
            widget->SetWidthRatio(value.mNumber / 100.0f);
        return;
    }
    if (propName == "height")
    {
        if (value.mType == CSSValueType::Number)
            widget->SetHeight(value.mNumber);
        else if (value.mType == CSSValueType::Percent)
            widget->SetHeightRatio(value.mNumber / 100.0f);
        return;
    }
    if (propName == "left")
    {
        if (value.mType == CSSValueType::Number)
            widget->SetX(value.mNumber);
        else if (value.mType == CSSValueType::Percent)
            widget->SetXRatio(value.mNumber / 100.0f);
        return;
    }
    if (propName == "top")
    {
        if (value.mType == CSSValueType::Number)
            widget->SetY(value.mNumber);
        else if (value.mType == CSSValueType::Percent)
            widget->SetYRatio(value.mNumber / 100.0f);
        return;
    }
    if (propName == "right")
    {
        if (value.mType == CSSValueType::Number)
            widget->SetRightMargin(value.mNumber);
        return;
    }
    if (propName == "bottom")
    {
        if (value.mType == CSSValueType::Number)
            widget->SetBottomMargin(value.mNumber);
        return;
    }
    if (propName == "margin")
    {
        if (value.mType == CSSValueType::Number)
            widget->SetMargins(value.mNumber, value.mNumber, value.mNumber, value.mNumber);
        return;
    }
    if (propName == "margin-left")
    {
        if (value.mType == CSSValueType::Number)
            widget->SetLeftMargin(value.mNumber);
        return;
    }
    if (propName == "margin-top")
    {
        if (value.mType == CSSValueType::Number)
            widget->SetTopMargin(value.mNumber);
        return;
    }
    if (propName == "margin-right")
    {
        if (value.mType == CSSValueType::Number)
            widget->SetRightMargin(value.mNumber);
        return;
    }
    if (propName == "margin-bottom")
    {
        if (value.mType == CSSValueType::Number)
            widget->SetBottomMargin(value.mNumber);
        return;
    }

    //--- Anchor ---

    if (propName == "anchor")
    {
        if (value.mType == CSSValueType::Enum)
        {
            AnchorMode mode = ParseAnchorMode(value.mString);
            widget->SetAnchorMode(mode);

            // When CSS sets a stretch mode, default to 100% of parent.
            // SetAnchorMode converts the old pixel size to a ratio, but the
            // default 100px becomes ~0.09 ratio instead of 1.0. Explicit CSS
            // width/height properties applied afterward will override this.
            bool stretchX = (mode == AnchorMode::TopStretch ||
                             mode == AnchorMode::MidHorizontalStretch ||
                             mode == AnchorMode::BottomStretch ||
                             mode == AnchorMode::FullStretch);
            bool stretchY = (mode == AnchorMode::LeftStretch ||
                             mode == AnchorMode::MidVerticalStretch ||
                             mode == AnchorMode::RightStretch ||
                             mode == AnchorMode::FullStretch);
            if (stretchX)
                widget->SetWidthRatio(1.0f);
            if (stretchY)
                widget->SetHeightRatio(1.0f);
        }
        return;
    }

    //--- Display / Visibility ---

    if (propName == "display")
    {
        if (value.mType == CSSValueType::Enum && value.mString == "none")
            widget->SetVisible(false);
        return;
    }
    if (propName == "visibility")
    {
        if (value.mType == CSSValueType::Enum && value.mString == "hidden")
            widget->SetVisible(false);
        return;
    }

    //--- Overflow / Scissor ---

    if (propName == "overflow")
    {
        if (value.mType == CSSValueType::Enum && value.mString == "hidden")
            widget->EnableScissor(true);
        return;
    }

    //--- Opacity ---

    if (propName == "opacity")
    {
        if (value.mType == CSSValueType::Number)
            widget->SetOpacityFloat(value.mNumber);
        return;
    }

    //--- Transform ---

    if (propName == "rotation" || propName == "rotate")
    {
        if (value.mType == CSSValueType::Number)
            widget->SetRotation(value.mNumber);
        return;
    }
    if (propName == "scale")
    {
        if (value.mType == CSSValueType::Number)
            widget->SetScale(glm::vec2(value.mNumber));
        return;
    }
    if (propName == "pivot" || propName == "transform-origin")
    {
        if (value.mType == CSSValueType::Number)
            widget->SetPivot(glm::vec2(value.mNumber));
        return;
    }

    //--- Offset / Size (engine-specific shorthand) ---

    if (propName == "offset-x")
    {
        glm::vec2 off = widget->GetOffset();
        off.x = value.mNumber;
        widget->SetOffset(off.x, off.y);
        return;
    }
    if (propName == "offset-y")
    {
        glm::vec2 off = widget->GetOffset();
        off.y = value.mNumber;
        widget->SetOffset(off.x, off.y);
        return;
    }

    //--- ArrayWidget properties ---

    if (isArray)
    {
        ArrayWidget* arr = static_cast<ArrayWidget*>(widget);

        if (propName == "flex-direction")
        {
            if (value.mType == CSSValueType::Enum)
            {
                std::vector<Property> props;
                arr->GatherProperties(props);
                for (Property& p : props)
                {
                    if (p.mName == "Orientation")
                    {
                        uint8_t ori = (value.mString == "row")
                            ? (uint8_t)ArrayOrientation::Horizontal
                            : (uint8_t)ArrayOrientation::Vertical;
                        p.SetValue(&ori, 0);
                        break;
                    }
                }
            }
            return;
        }
        if (propName == "gap")
        {
            if (value.mType == CSSValueType::Number)
            {
                std::vector<Property> props;
                arr->GatherProperties(props);
                for (Property& p : props)
                {
                    if (p.mName == "Spacing")
                    {
                        float spacing = value.mNumber;
                        p.SetValue(&spacing, 0);
                        break;
                    }
                }
            }
            return;
        }
        if (propName == "align-items")
        {
            if (value.mType == CSSValueType::Enum && value.mString == "center")
                arr->SetCentered(true);
            return;
        }
    }

    //--- Color properties ---

    if (propName == "color")
    {
        if (value.mType == CSSValueType::Color)
        {
            if (isButton)
            {
                // "color" on a button sets the text color
                Button* btn = static_cast<Button*>(widget);
                Text* text = btn->GetText();
                if (text) text->SetColor(value.mColor);
            }
            else if (isText)
            {
                widget->SetColor(value.mColor);
            }
            else
            {
                widget->SetColor(value.mColor);
            }
        }
        return;
    }

    if (propName == "background-color")
    {
        if (value.mType == CSSValueType::Color)
        {
            if (isButton)
            {
                Button* btn = static_cast<Button*>(widget);
                btn->SetNormalColor(value.mColor);
                btn->SetUseQuadStateColor(true);
            }
            else
            {
                widget->SetColor(value.mColor);
            }
        }
        return;
    }

    if (propName == "background-image")
    {
        if (value.mType == CSSValueType::String && !value.mString.empty())
        {
            Texture* tex = LoadAsset<Texture>(value.mString);
            if (tex)
            {
                if (isButton)
                {
                    static_cast<Button*>(widget)->SetNormalTexture(tex);
                }
                else if (isQuad)
                {
                    static_cast<Quad*>(widget)->SetTexture(tex);
                }
            }
        }
        return;
    }

    //--- Button state colors ---

    if (propName == "--normal-color" && isButton)
    {
        if (value.mType == CSSValueType::Color)
            static_cast<Button*>(widget)->SetNormalColor(value.mColor);
        return;
    }
    if (propName == "--hovered-color" && isButton)
    {
        if (value.mType == CSSValueType::Color)
            static_cast<Button*>(widget)->SetHoveredColor(value.mColor);
        return;
    }
    if (propName == "--pressed-color" && isButton)
    {
        if (value.mType == CSSValueType::Color)
            static_cast<Button*>(widget)->SetPressedColor(value.mColor);
        return;
    }
    if (propName == "--locked-color" && isButton)
    {
        if (value.mType == CSSValueType::Color)
            static_cast<Button*>(widget)->SetLockedColor(value.mColor);
        return;
    }

    //--- Button state textures ---

    if (propName == "--normal-texture" && isButton)
    {
        if (value.mType == CSSValueType::String)
        {
            Texture* tex = LoadAsset<Texture>(value.mString);
            if (tex) static_cast<Button*>(widget)->SetNormalTexture(tex);
        }
        return;
    }
    if (propName == "--hovered-texture" && isButton)
    {
        if (value.mType == CSSValueType::String)
        {
            Texture* tex = LoadAsset<Texture>(value.mString);
            if (tex) static_cast<Button*>(widget)->SetHoveredTexture(tex);
        }
        return;
    }
    if (propName == "--pressed-texture" && isButton)
    {
        if (value.mType == CSSValueType::String)
        {
            Texture* tex = LoadAsset<Texture>(value.mString);
            if (tex) static_cast<Button*>(widget)->SetPressedTexture(tex);
        }
        return;
    }
    if (propName == "--locked-texture" && isButton)
    {
        if (value.mType == CSSValueType::String)
        {
            Texture* tex = LoadAsset<Texture>(value.mString);
            if (tex) static_cast<Button*>(widget)->SetLockedTexture(tex);
        }
        return;
    }

    //--- Text properties ---

    if (propName == "font-size")
    {
        if (value.mType == CSSValueType::Number)
        {
            if (isButton)
            {
                Text* text = static_cast<Button*>(widget)->GetText();
                if (text) text->SetTextSize(value.mNumber);
            }
            else if (isText)
            {
                static_cast<Text*>(widget)->SetTextSize(value.mNumber);
            }
        }
        return;
    }

    if (propName == "font-family")
    {
        std::string fontName = (value.mType == CSSValueType::String) ? value.mString : value.mString;
        if (!fontName.empty())
        {
            Font* font = LoadAsset<Font>(fontName);
            if (font)
            {
                if (isButton)
                {
                    Text* text = static_cast<Button*>(widget)->GetText();
                    if (text) text->SetFont(font);
                }
                else if (isText)
                {
                    static_cast<Text*>(widget)->SetFont(font);
                }
            }
        }
        return;
    }

    if (propName == "text-align")
    {
        if (value.mType == CSSValueType::Enum)
        {
            Justification just = Justification::Left;
            if (value.mString == "center") just = Justification::Center;
            else if (value.mString == "right") just = Justification::Right;

            if (isButton)
            {
                Text* text = static_cast<Button*>(widget)->GetText();
                if (text) text->SetHorizontalJustification(just);
            }
            else if (isText)
            {
                static_cast<Text*>(widget)->SetHorizontalJustification(just);
            }
        }
        return;
    }

    if (propName == "vertical-align")
    {
        if (value.mType == CSSValueType::Enum)
        {
            Justification just = Justification::Top;
            if (value.mString == "center") just = Justification::Center;
            else if (value.mString == "bottom") just = Justification::Bottom;

            if (isButton)
            {
                Text* text = static_cast<Button*>(widget)->GetText();
                if (text) text->SetVerticalJustification(just);
            }
            else if (isText)
            {
                static_cast<Text*>(widget)->SetVerticalJustification(just);
            }
        }
        return;
    }

    if (propName == "word-wrap" || propName == "overflow-wrap")
    {
        if (value.mType == CSSValueType::Enum && value.mString == "break-word")
        {
            if (isText)
                static_cast<Text*>(widget)->EnableWordWrap(true);
        }
        return;
    }

    //--- Object-fit (Quad / Button) ---

    if (propName == "object-fit")
    {
        if (value.mType == CSSValueType::Enum)
        {
            ObjectFit fit = ObjectFit::Fill;
            if (value.mString == "contain") fit = ObjectFit::Contain;
            else if (value.mString == "cover") fit = ObjectFit::Cover;
            else if (value.mString == "none") fit = ObjectFit::None;

            if (isQuad)
                static_cast<Quad*>(widget)->SetObjectFit(fit);
            else if (isButton)
                static_cast<Button*>(widget)->GetQuad()->SetObjectFit(fit);
        }
        return;
    }

    //--- Game resolution ---

    if (propName == "game-resolution")
    {
        if (value.mType == CSSValueType::Enum && value.mString == "true")
        {
            widget->SetUseGameResolution(true);
        }
        else if (value.mType == CSSValueType::String && value.mString == "true")
        {
            widget->SetUseGameResolution(true);
        }
        return;
    }
}
