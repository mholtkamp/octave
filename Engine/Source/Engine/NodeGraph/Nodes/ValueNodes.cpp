#include "NodeGraph/Nodes/ValueNodes.h"
#include "Utilities.h"
#include "Clock.h"
#include "Engine.h"
#include "Log.h"
#include "TableDatum.h"
#include "Nodes/Node.h"
#include "Nodes/3D/Node3d.h"
#include "Nodes/Widgets/Text.h"

#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <string>

FORCE_LINK_DEF(ValueNodes);

static const glm::vec4 kValueNodeColor = glm::vec4(0.2f, 0.5f, 0.7f, 1.0f);

// =============================================================================
// FloatConstantNode
// =============================================================================
DEFINE_GRAPH_NODE(FloatConstantNode);

void FloatConstantNode::SetupPins()
{
    AddInputPin("Value", DatumType::Float, Datum(0.0f));
    AddOutputPin("Out", DatumType::Float);
}

void FloatConstantNode::Evaluate()
{
    SetOutputValue(0, GetInputValue(0));
}

glm::vec4 FloatConstantNode::GetNodeColor() const { return kValueNodeColor; }

// =============================================================================
// IntConstantNode
// =============================================================================
DEFINE_GRAPH_NODE(IntConstantNode);

void IntConstantNode::SetupPins()
{
    AddInputPin("Value", DatumType::Integer, Datum(0));
    AddOutputPin("Out", DatumType::Integer);
}

void IntConstantNode::Evaluate()
{
    SetOutputValue(0, GetInputValue(0));
}

glm::vec4 IntConstantNode::GetNodeColor() const { return kValueNodeColor; }

// =============================================================================
// VectorConstantNode
// =============================================================================
DEFINE_GRAPH_NODE(VectorConstantNode);

void VectorConstantNode::SetupPins()
{
    AddInputPin("Value", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddOutputPin("Out", DatumType::Vector);
}

void VectorConstantNode::Evaluate()
{
    SetOutputValue(0, GetInputValue(0));
}

glm::vec4 VectorConstantNode::GetNodeColor() const { return kValueNodeColor; }

// =============================================================================
// ColorConstantNode
// =============================================================================
DEFINE_GRAPH_NODE(ColorConstantNode);

void ColorConstantNode::SetupPins()
{
    AddInputPin("Value", DatumType::Color, Datum(glm::vec4(1.0f)));
    AddOutputPin("Out", DatumType::Color);
}

void ColorConstantNode::Evaluate()
{
    SetOutputValue(0, GetInputValue(0));
}

glm::vec4 ColorConstantNode::GetNodeColor() const { return kValueNodeColor; }

// =============================================================================
// TimeNode
// =============================================================================
DEFINE_GRAPH_NODE(TimeNode);

void TimeNode::SetupPins()
{
    AddOutputPin("Time", DatumType::Float);
}

void TimeNode::Evaluate()
{
    float time = GetAppClock() ? GetAppClock()->GetTime() : 0.0f;
    SetOutputValue(0, Datum(time));
}

glm::vec4 TimeNode::GetNodeColor() const { return kValueNodeColor; }

// =============================================================================
// ViewerNode
// =============================================================================
DEFINE_GRAPH_NODE(ViewerNode);

void ViewerNode::SetupPins()
{
    AddInputPin("Float", DatumType::Float, Datum(0.0f));
    AddInputPin("Color", DatumType::Color, Datum(glm::vec4(0.0f)));
    AddInputPin("Vector", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("Integer", DatumType::Integer, Datum(0));
}

void ViewerNode::Evaluate()
{
    // Viewer is a display-only sink node.
    // Input values are propagated by the processor and shown in the editor.
}

glm::vec4 ViewerNode::GetNodeColor() const { return glm::vec4(0.2f, 0.6f, 0.3f, 1.0f); }

// =============================================================================
// DebugLogNode
// =============================================================================
DEFINE_GRAPH_NODE(DebugLogNode);

void DebugLogNode::SetupPins()
{
    AddInputPin("Label", DatumType::String, Datum(std::string("Debug")));
    AddInputPin("Float", DatumType::Float, Datum(0.0f));
    AddInputPin("Color", DatumType::Color, Datum(glm::vec4(0.0f)));
    AddInputPin("Vector", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("Integer", DatumType::Integer, Datum(0));
    AddInputPin("String", DatumType::String, Datum(std::string("")));
}

void DebugLogNode::Evaluate()
{
    const std::string& label = GetInputValue(0).GetString();

    float f = GetInputValue(1).GetFloat();
    glm::vec4 c = GetInputValue(2).GetColor();
    glm::vec3 v = GetInputValue(3).GetVector();
    int32_t i = GetInputValue(4).GetInteger();
    const std::string& s = GetInputValue(5).GetString();

    LogDebug("[%s] Float=%.4f Int=%d Vec=(%.2f, %.2f, %.2f) Color=(%.2f, %.2f, %.2f, %.2f) Str=%s",
        label.c_str(),
        f, i,
        v.x, v.y, v.z,
        c.r, c.g, c.b, c.a,
        s.c_str());
}

glm::vec4 DebugLogNode::GetNodeColor() const { return glm::vec4(0.2f, 0.6f, 0.3f, 1.0f); }

static const glm::vec4 kConversionNodeColor = glm::vec4(0.6f, 0.4f, 0.7f, 1.0f);
static const glm::vec4 kColorNodeColor = glm::vec4(0.1f, 0.5f, 0.9f, 1.0f);
static const glm::vec4 kDateTimeNodeColor = glm::vec4(0.7f, 0.5f, 0.2f, 1.0f);
static const glm::vec4 kDictionaryNodeColor = glm::vec4(0.5f, 0.7f, 0.3f, 1.0f);
static const glm::vec4 kStringNodeColor = glm::vec4(0.7f, 0.3f, 0.5f, 1.0f);
static const glm::vec4 kUtilityNodeColor = glm::vec4(0.2f, 0.6f, 0.3f, 1.0f);

// =============================================================================
// FloatToStringNode
// =============================================================================
DEFINE_GRAPH_NODE(FloatToStringNode);

void FloatToStringNode::SetupPins()
{
    AddInputPin("Value", DatumType::Float, Datum(0.0f));
    AddInputPin("Precision", DatumType::Integer, Datum(4));
    AddOutputPin("Out", DatumType::String);
}

void FloatToStringNode::Evaluate()
{
    float value = GetInputValue(0).GetFloat();
    int32_t precision = GetInputValue(1).GetInteger();
    if (precision < 0) precision = 0;
    if (precision > 10) precision = 10;

    char buf[64];
    snprintf(buf, sizeof(buf), "%.*f", precision, value);
    SetOutputValue(0, Datum(std::string(buf)));
}

glm::vec4 FloatToStringNode::GetNodeColor() const { return kConversionNodeColor; }

// =============================================================================
// IntToStringNode
// =============================================================================
DEFINE_GRAPH_NODE(IntToStringNode);

void IntToStringNode::SetupPins()
{
    AddInputPin("Value", DatumType::Integer, Datum(0));
    AddOutputPin("Out", DatumType::String);
}

void IntToStringNode::Evaluate()
{
    int32_t value = GetInputValue(0).GetInteger();
    SetOutputValue(0, Datum(std::to_string(value)));
}

glm::vec4 IntToStringNode::GetNodeColor() const { return kConversionNodeColor; }

// =============================================================================
// ColorToStringNode
// =============================================================================
DEFINE_GRAPH_NODE(ColorToStringNode);

void ColorToStringNode::SetupPins()
{
    AddInputPin("Value", DatumType::Color, Datum(glm::vec4(1.0f)));
    AddOutputPin("Out", DatumType::String);
}

void ColorToStringNode::Evaluate()
{
    glm::vec4 c = GetInputValue(0).GetColor();
    char buf[128];
    snprintf(buf, sizeof(buf), "(%.4f, %.4f, %.4f, %.4f)", c.r, c.g, c.b, c.a);
    SetOutputValue(0, Datum(std::string(buf)));
}

glm::vec4 ColorToStringNode::GetNodeColor() const { return kConversionNodeColor; }

// =============================================================================
// VectorToStringNode
// =============================================================================
DEFINE_GRAPH_NODE(VectorToStringNode);

void VectorToStringNode::SetupPins()
{
    AddInputPin("Value", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddOutputPin("Out", DatumType::String);
}

void VectorToStringNode::Evaluate()
{
    glm::vec3 v = GetInputValue(0).GetVector();
    char buf[128];
    snprintf(buf, sizeof(buf), "(%.4f, %.4f, %.4f)", v.x, v.y, v.z);
    SetOutputValue(0, Datum(std::string(buf)));
}

glm::vec4 VectorToStringNode::GetNodeColor() const { return kConversionNodeColor; }

// =============================================================================
// BoolToStringNode
// =============================================================================
DEFINE_GRAPH_NODE(BoolToStringNode);

void BoolToStringNode::SetupPins()
{
    AddInputPin("Value", DatumType::Bool, Datum(false));
    AddOutputPin("Out", DatumType::String);
}

void BoolToStringNode::Evaluate()
{
    bool value = GetInputValue(0).GetBool();
    SetOutputValue(0, Datum(std::string(value ? "true" : "false")));
}

glm::vec4 BoolToStringNode::GetNodeColor() const { return kConversionNodeColor; }

// =============================================================================
// NodeToStringNode
// =============================================================================
DEFINE_GRAPH_NODE(NodeToStringNode);

void NodeToStringNode::SetupPins()
{
    AddInputPin("Node", DatumType::Node);
    AddOutputPin("Out", DatumType::String);
}

void NodeToStringNode::Evaluate()
{
    WeakPtr<Node> nodeRef = GetInputValue(0).GetNode();
    Node* node = nodeRef.Get();
    if (node != nullptr)
    {
        SetOutputValue(0, Datum(node->GetName()));
    }
    else
    {
        SetOutputValue(0, Datum(std::string("<null>")));
    }
}

glm::vec4 NodeToStringNode::GetNodeColor() const { return kConversionNodeColor; }

// =============================================================================
// Node3DToStringNode
// =============================================================================
DEFINE_GRAPH_NODE(Node3DToStringNode);

void Node3DToStringNode::SetupPins()
{
    AddInputPin("Node3D", DatumType::Node3D);
    AddOutputPin("Out", DatumType::String);
}

void Node3DToStringNode::Evaluate()
{
    WeakPtr<Node> nodeRef = GetInputValue(0).GetNode();
    Node* node = nodeRef.Get();
    if (node != nullptr)
    {
        SetOutputValue(0, Datum(node->GetName()));
    }
    else
    {
        SetOutputValue(0, Datum(std::string("<null>")));
    }
}

glm::vec4 Node3DToStringNode::GetNodeColor() const { return kConversionNodeColor; }

// =============================================================================
// StringToFloatNode
// =============================================================================
DEFINE_GRAPH_NODE(StringToFloatNode);

void StringToFloatNode::SetupPins()
{
    AddInputPin("Value", DatumType::String, Datum(std::string("0")));
    AddOutputPin("Out", DatumType::Float);
}

void StringToFloatNode::Evaluate()
{
    const std::string& str = GetInputValue(0).GetString();
    float result = 0.0f;
    try { result = std::stof(str); } catch (...) {}
    SetOutputValue(0, Datum(result));
}

glm::vec4 StringToFloatNode::GetNodeColor() const { return kConversionNodeColor; }

// =============================================================================
// StringToIntNode
// =============================================================================
DEFINE_GRAPH_NODE(StringToIntNode);

void StringToIntNode::SetupPins()
{
    AddInputPin("Value", DatumType::String, Datum(std::string("0")));
    AddOutputPin("Out", DatumType::Integer);
}

void StringToIntNode::Evaluate()
{
    const std::string& str = GetInputValue(0).GetString();
    int32_t result = 0;
    try { result = std::stoi(str); } catch (...) {}
    SetOutputValue(0, Datum(result));
}

glm::vec4 StringToIntNode::GetNodeColor() const { return kConversionNodeColor; }

// =============================================================================
// StringToBoolNode
// =============================================================================
DEFINE_GRAPH_NODE(StringToBoolNode);

void StringToBoolNode::SetupPins()
{
    AddInputPin("Value", DatumType::String, Datum(std::string("false")));
    AddOutputPin("Out", DatumType::Bool);
}

void StringToBoolNode::Evaluate()
{
    std::string str = GetInputValue(0).GetString();
    str = StringToLower(str);
    bool result = (str == "true" || str == "1" || str == "yes");
    SetOutputValue(0, Datum(result));
}

glm::vec4 StringToBoolNode::GetNodeColor() const { return kConversionNodeColor; }

// =============================================================================
// StringToVectorNode
// =============================================================================
DEFINE_GRAPH_NODE(StringToVectorNode);

void StringToVectorNode::SetupPins()
{
    AddInputPin("Value", DatumType::String, Datum(std::string("0, 0, 0")));
    AddOutputPin("Out", DatumType::Vector);
}

void StringToVectorNode::Evaluate()
{
    const std::string& str = GetInputValue(0).GetString();
    glm::vec3 result(0.0f);
    // Parse "x, y, z" format (with optional parentheses)
    if (sscanf(str.c_str(), " ( %f , %f , %f )", &result.x, &result.y, &result.z) != 3)
    {
        sscanf(str.c_str(), " %f , %f , %f", &result.x, &result.y, &result.z);
    }
    SetOutputValue(0, Datum(result));
}

glm::vec4 StringToVectorNode::GetNodeColor() const { return kConversionNodeColor; }

// =============================================================================
// StringToColorNode
// =============================================================================
DEFINE_GRAPH_NODE(StringToColorNode);

void StringToColorNode::SetupPins()
{
    AddInputPin("Value", DatumType::String, Datum(std::string("1, 1, 1, 1")));
    AddOutputPin("Out", DatumType::Color);
}

void StringToColorNode::Evaluate()
{
    const std::string& str = GetInputValue(0).GetString();
    glm::vec4 result(0.0f);
    // Parse "r, g, b, a" format (with optional parentheses)
    if (sscanf(str.c_str(), " ( %f , %f , %f , %f )", &result.r, &result.g, &result.b, &result.a) != 4)
    {
        if (sscanf(str.c_str(), " %f , %f , %f , %f", &result.r, &result.g, &result.b, &result.a) != 4)
        {
            // Try 3-component (default alpha = 1)
            result.a = 1.0f;
            if (sscanf(str.c_str(), " ( %f , %f , %f )", &result.r, &result.g, &result.b) != 3)
            {
                sscanf(str.c_str(), " %f , %f , %f", &result.r, &result.g, &result.b);
            }
        }
    }
    SetOutputValue(0, Datum(result));
}

glm::vec4 StringToColorNode::GetNodeColor() const { return kConversionNodeColor; }

// =============================================================================
// VectorToColorNode
// =============================================================================
DEFINE_GRAPH_NODE(VectorToColorNode);

void VectorToColorNode::SetupPins()
{
    AddInputPin("Vector", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("Alpha", DatumType::Float, Datum(1.0f));
    AddOutputPin("Out", DatumType::Color);
}

void VectorToColorNode::Evaluate()
{
    glm::vec3 v = GetInputValue(0).GetVector();
    float a = GetInputValue(1).GetFloat();
    SetOutputValue(0, Datum(glm::vec4(v.x, v.y, v.z, a)));
}

glm::vec4 VectorToColorNode::GetNodeColor() const { return kConversionNodeColor; }

// =============================================================================
// ColorToVectorNode
// =============================================================================
DEFINE_GRAPH_NODE(ColorToVectorNode);

void ColorToVectorNode::SetupPins()
{
    AddInputPin("Color", DatumType::Color, Datum(glm::vec4(1.0f)));
    AddOutputPin("Vector", DatumType::Vector);
    AddOutputPin("Alpha", DatumType::Float);
}

void ColorToVectorNode::Evaluate()
{
    glm::vec4 c = GetInputValue(0).GetColor();
    SetOutputValue(0, Datum(glm::vec3(c.r, c.g, c.b)));
    SetOutputValue(1, Datum(c.a));
}

glm::vec4 ColorToVectorNode::GetNodeColor() const { return kConversionNodeColor; }

// =============================================================================
// SplitVectorNode
// =============================================================================
DEFINE_GRAPH_NODE(SplitVectorNode);

void SplitVectorNode::SetupPins()
{
    AddInputPin("Vector", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddOutputPin("X", DatumType::Float);
    AddOutputPin("Y", DatumType::Float);
    AddOutputPin("Z", DatumType::Float);
}

void SplitVectorNode::Evaluate()
{
    glm::vec3 v = GetInputValue(0).GetVector();
    SetOutputValue(0, Datum(v.x));
    SetOutputValue(1, Datum(v.y));
    SetOutputValue(2, Datum(v.z));
}

glm::vec4 SplitVectorNode::GetNodeColor() const { return kConversionNodeColor; }

// =============================================================================
// SplitColorNode
// =============================================================================
DEFINE_GRAPH_NODE(SplitColorNode);

void SplitColorNode::SetupPins()
{
    AddInputPin("Color", DatumType::Color, Datum(glm::vec4(1.0f)));
    AddOutputPin("R", DatumType::Float);
    AddOutputPin("G", DatumType::Float);
    AddOutputPin("B", DatumType::Float);
    AddOutputPin("A", DatumType::Float);
}

void SplitColorNode::Evaluate()
{
    glm::vec4 c = GetInputValue(0).GetColor();
    SetOutputValue(0, Datum(c.r));
    SetOutputValue(1, Datum(c.g));
    SetOutputValue(2, Datum(c.b));
    SetOutputValue(3, Datum(c.a));
}

glm::vec4 SplitColorNode::GetNodeColor() const { return kConversionNodeColor; }

// =============================================================================
// HexToColorNode
// =============================================================================
DEFINE_GRAPH_NODE(HexToColorNode);

void HexToColorNode::SetupPins()
{
    AddInputPin("Hex", DatumType::String, Datum(std::string("#FFFFFF")));
    AddOutputPin("Out", DatumType::Color);
}

void HexToColorNode::Evaluate()
{
    std::string hex = GetInputValue(0).GetString();
    glm::vec4 result(0.0f, 0.0f, 0.0f, 1.0f);

    // Strip leading '#'
    if (!hex.empty() && hex[0] == '#')
        hex = hex.substr(1);

    unsigned int hexVal = 0;
    if (sscanf(hex.c_str(), "%x", &hexVal) == 1)
    {
        if (hex.size() == 8) // RRGGBBAA
        {
            result.r = ((hexVal >> 24) & 0xFF) / 255.0f;
            result.g = ((hexVal >> 16) & 0xFF) / 255.0f;
            result.b = ((hexVal >> 8) & 0xFF) / 255.0f;
            result.a = (hexVal & 0xFF) / 255.0f;
        }
        else if (hex.size() == 6) // RRGGBB
        {
            result.r = ((hexVal >> 16) & 0xFF) / 255.0f;
            result.g = ((hexVal >> 8) & 0xFF) / 255.0f;
            result.b = (hexVal & 0xFF) / 255.0f;
            result.a = 1.0f;
        }
        else if (hex.size() == 3) // RGB shorthand
        {
            int r = (hexVal >> 8) & 0xF;
            int g = (hexVal >> 4) & 0xF;
            int b = hexVal & 0xF;
            result.r = (r * 17) / 255.0f;
            result.g = (g * 17) / 255.0f;
            result.b = (b * 17) / 255.0f;
            result.a = 1.0f;
        }
    }

    SetOutputValue(0, Datum(result));
}

glm::vec4 HexToColorNode::GetNodeColor() const { return kColorNodeColor; }

// =============================================================================
// RGBAColorNode
// =============================================================================
DEFINE_GRAPH_NODE(RGBAColorNode);

void RGBAColorNode::SetupPins()
{
    AddInputPin("R", DatumType::Float, Datum(1.0f));
    AddInputPin("G", DatumType::Float, Datum(1.0f));
    AddInputPin("B", DatumType::Float, Datum(1.0f));
    AddInputPin("A", DatumType::Float, Datum(1.0f));
    AddOutputPin("Out", DatumType::Color);
}

void RGBAColorNode::Evaluate()
{
    float r = GetInputValue(0).GetFloat();
    float g = GetInputValue(1).GetFloat();
    float b = GetInputValue(2).GetFloat();
    float a = GetInputValue(3).GetFloat();
    SetOutputValue(0, Datum(glm::vec4(r, g, b, a)));
}

glm::vec4 RGBAColorNode::GetNodeColor() const { return kColorNodeColor; }

// =============================================================================
// RGBColorNode
// =============================================================================
DEFINE_GRAPH_NODE(RGBColorNode);

void RGBColorNode::SetupPins()
{
    AddInputPin("R", DatumType::Float, Datum(1.0f));
    AddInputPin("G", DatumType::Float, Datum(1.0f));
    AddInputPin("B", DatumType::Float, Datum(1.0f));
    AddOutputPin("Out", DatumType::Color);
}

void RGBColorNode::Evaluate()
{
    float r = GetInputValue(0).GetFloat();
    float g = GetInputValue(1).GetFloat();
    float b = GetInputValue(2).GetFloat();
    SetOutputValue(0, Datum(glm::vec4(r, g, b, 1.0f)));
}

glm::vec4 RGBColorNode::GetNodeColor() const { return kColorNodeColor; }

// =============================================================================
// LinearColorNode
// =============================================================================
DEFINE_GRAPH_NODE(LinearColorNode);

void LinearColorNode::SetupPins()
{
    AddInputPin("sRGB", DatumType::Color, Datum(glm::vec4(1.0f)));
    AddOutputPin("Linear", DatumType::Color);
}

static float SrgbToLinear(float srgb)
{
    if (srgb <= 0.04045f)
        return srgb / 12.92f;
    else
        return powf((srgb + 0.055f) / 1.055f, 2.4f);
}

void LinearColorNode::Evaluate()
{
    glm::vec4 srgb = GetInputValue(0).GetColor();
    glm::vec4 linear;
    linear.r = SrgbToLinear(srgb.r);
    linear.g = SrgbToLinear(srgb.g);
    linear.b = SrgbToLinear(srgb.b);
    linear.a = srgb.a; // Alpha is not gamma-corrected
    SetOutputValue(0, Datum(linear));
}

glm::vec4 LinearColorNode::GetNodeColor() const { return kColorNodeColor; }

// =============================================================================
// DateTimeToStringNode
// =============================================================================
DEFINE_GRAPH_NODE(DateTimeToStringNode);

void DateTimeToStringNode::SetupPins()
{
    AddInputPin("Timestamp", DatumType::Float, Datum(0.0f));
    AddInputPin("Format", DatumType::String, Datum(std::string("%Y-%m-%d %H:%M:%S")));
    AddOutputPin("Out", DatumType::String);
}

void DateTimeToStringNode::Evaluate()
{
    float timestamp = GetInputValue(0).GetFloat();
    const std::string& format = GetInputValue(1).GetString();

    time_t t = (time_t)timestamp;
    struct tm* tmInfo = localtime(&t);

    char buf[256];
    buf[0] = '\0';
    if (tmInfo != nullptr)
    {
        strftime(buf, sizeof(buf), format.c_str(), tmInfo);
    }
    SetOutputValue(0, Datum(std::string(buf)));
}

glm::vec4 DateTimeToStringNode::GetNodeColor() const { return kDateTimeNodeColor; }

// =============================================================================
// DateTimeNowNode
// =============================================================================
DEFINE_GRAPH_NODE(DateTimeNowNode);

void DateTimeNowNode::SetupPins()
{
    AddOutputPin("Timestamp", DatumType::Float);
}

void DateTimeNowNode::Evaluate()
{
    time_t now = time(nullptr);
    SetOutputValue(0, Datum((float)now));
}

glm::vec4 DateTimeNowNode::GetNodeColor() const { return kDateTimeNodeColor; }

// =============================================================================
// DateTimeToFloatNode
// =============================================================================
DEFINE_GRAPH_NODE(DateTimeToFloatNode);

void DateTimeToFloatNode::SetupPins()
{
    AddInputPin("DateTime", DatumType::String, Datum(std::string("2025-01-01 00:00:00")));
    AddOutputPin("Timestamp", DatumType::Float);
}

void DateTimeToFloatNode::Evaluate()
{
    const std::string& str = GetInputValue(0).GetString();
    struct tm tmInfo = {};
    float result = 0.0f;

    // Parse "YYYY-MM-DD HH:MM:SS" format
    if (sscanf(str.c_str(), "%d-%d-%d %d:%d:%d",
        &tmInfo.tm_year, &tmInfo.tm_mon, &tmInfo.tm_mday,
        &tmInfo.tm_hour, &tmInfo.tm_min, &tmInfo.tm_sec) >= 3)
    {
        tmInfo.tm_year -= 1900;
        tmInfo.tm_mon -= 1;
        tmInfo.tm_isdst = -1;
        time_t t = mktime(&tmInfo);
        if (t != (time_t)-1)
        {
            result = (float)t;
        }
    }

    SetOutputValue(0, Datum(result));
}

glm::vec4 DateTimeToFloatNode::GetNodeColor() const { return kDateTimeNodeColor; }

// =============================================================================
// FloatToDateTimeNode
// =============================================================================
DEFINE_GRAPH_NODE(FloatToDateTimeNode);

void FloatToDateTimeNode::SetupPins()
{
    AddInputPin("Timestamp", DatumType::Float, Datum(0.0f));
    AddOutputPin("Out", DatumType::String);
}

void FloatToDateTimeNode::Evaluate()
{
    float timestamp = GetInputValue(0).GetFloat();
    time_t t = (time_t)timestamp;
    struct tm* tmInfo = localtime(&t);

    char buf[64];
    buf[0] = '\0';
    if (tmInfo != nullptr)
    {
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tmInfo);
    }
    SetOutputValue(0, Datum(std::string(buf)));
}

glm::vec4 FloatToDateTimeNode::GetNodeColor() const { return kDateTimeNodeColor; }

// =============================================================================
// AddSecondsNode
// =============================================================================
DEFINE_GRAPH_NODE(AddSecondsNode);

void AddSecondsNode::SetupPins()
{
    AddInputPin("Timestamp", DatumType::Float, Datum(0.0f));
    AddInputPin("Seconds", DatumType::Float, Datum(0.0f));
    AddOutputPin("Out", DatumType::Float);
}

void AddSecondsNode::Evaluate()
{
    float timestamp = GetInputValue(0).GetFloat();
    float seconds = GetInputValue(1).GetFloat();
    SetOutputValue(0, Datum(timestamp + seconds));
}

glm::vec4 AddSecondsNode::GetNodeColor() const { return kDateTimeNodeColor; }

// =============================================================================
// AddMinutesNode
// =============================================================================
DEFINE_GRAPH_NODE(AddMinutesNode);

void AddMinutesNode::SetupPins()
{
    AddInputPin("Timestamp", DatumType::Float, Datum(0.0f));
    AddInputPin("Minutes", DatumType::Float, Datum(0.0f));
    AddOutputPin("Out", DatumType::Float);
}

void AddMinutesNode::Evaluate()
{
    float timestamp = GetInputValue(0).GetFloat();
    float minutes = GetInputValue(1).GetFloat();
    SetOutputValue(0, Datum(timestamp + minutes * 60.0f));
}

glm::vec4 AddMinutesNode::GetNodeColor() const { return kDateTimeNodeColor; }

// =============================================================================
// AddHoursNode
// =============================================================================
DEFINE_GRAPH_NODE(AddHoursNode);

void AddHoursNode::SetupPins()
{
    AddInputPin("Timestamp", DatumType::Float, Datum(0.0f));
    AddInputPin("Hours", DatumType::Float, Datum(0.0f));
    AddOutputPin("Out", DatumType::Float);
}

void AddHoursNode::Evaluate()
{
    float timestamp = GetInputValue(0).GetFloat();
    float hours = GetInputValue(1).GetFloat();
    SetOutputValue(0, Datum(timestamp + hours * 3600.0f));
}

glm::vec4 AddHoursNode::GetNodeColor() const { return kDateTimeNodeColor; }

// =============================================================================
// StringToDictionaryNode
// =============================================================================
DEFINE_GRAPH_NODE(StringToDictionaryNode);

void StringToDictionaryNode::SetupPins()
{
    AddInputPin("Value", DatumType::String, Datum(std::string("key1=value1;key2=value2")));
    AddInputPin("Pair Delimiter", DatumType::String, Datum(std::string(";")));
    AddInputPin("Key Delimiter", DatumType::String, Datum(std::string("=")));
    AddOutputPin("Out", DatumType::Table);
}

void StringToDictionaryNode::Evaluate()
{
    const std::string& input = GetInputValue(0).GetString();
    const std::string& pairDelim = GetInputValue(1).GetString();
    const std::string& keyDelim = GetInputValue(2).GetString();

    Datum table;
    table.SetType(DatumType::Table);

    if (!input.empty() && !pairDelim.empty() && !keyDelim.empty())
    {
        size_t pos = 0;
        std::string remaining = input;

        while (!remaining.empty())
        {
            size_t pairEnd = remaining.find(pairDelim);
            std::string pair = (pairEnd != std::string::npos) ? remaining.substr(0, pairEnd) : remaining;

            size_t keyEnd = pair.find(keyDelim);
            if (keyEnd != std::string::npos)
            {
                std::string key = pair.substr(0, keyEnd);
                std::string value = pair.substr(keyEnd + keyDelim.size());
                table.PushBackTableDatum(TableDatum(key.c_str(), value));
            }

            if (pairEnd == std::string::npos)
                break;
            remaining = remaining.substr(pairEnd + pairDelim.size());
        }
    }

    SetOutputValue(0, table);
}

glm::vec4 StringToDictionaryNode::GetNodeColor() const { return kDictionaryNodeColor; }

// =============================================================================
// ParseJsonToDictionaryNode
// =============================================================================
DEFINE_GRAPH_NODE(ParseJsonToDictionaryNode);

void ParseJsonToDictionaryNode::SetupPins()
{
    AddInputPin("JSON", DatumType::String, Datum(std::string("{}")));
    AddOutputPin("Out", DatumType::Table);
}

// Simple JSON string value parser helper
static std::string ParseJsonStringValue(const std::string& json, size_t& pos)
{
    std::string result;
    if (pos < json.size() && json[pos] == '"')
    {
        pos++; // skip opening quote
        while (pos < json.size() && json[pos] != '"')
        {
            if (json[pos] == '\\' && pos + 1 < json.size())
            {
                pos++;
                switch (json[pos])
                {
                    case '"': result += '"'; break;
                    case '\\': result += '\\'; break;
                    case 'n': result += '\n'; break;
                    case 't': result += '\t'; break;
                    case 'r': result += '\r'; break;
                    default: result += json[pos]; break;
                }
            }
            else
            {
                result += json[pos];
            }
            pos++;
        }
        if (pos < json.size()) pos++; // skip closing quote
    }
    return result;
}

static void SkipWhitespace(const std::string& json, size_t& pos)
{
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\n' || json[pos] == '\r'))
        pos++;
}

void ParseJsonToDictionaryNode::Evaluate()
{
    const std::string& json = GetInputValue(0).GetString();
    Datum table;
    table.SetType(DatumType::Table);

    size_t pos = 0;
    SkipWhitespace(json, pos);

    if (pos < json.size() && json[pos] == '{')
    {
        pos++; // skip '{'

        while (pos < json.size())
        {
            SkipWhitespace(json, pos);
            if (pos >= json.size() || json[pos] == '}')
                break;

            // Parse key
            std::string key = ParseJsonStringValue(json, pos);
            SkipWhitespace(json, pos);

            // Skip ':'
            if (pos < json.size() && json[pos] == ':')
                pos++;

            SkipWhitespace(json, pos);

            // Parse value (strings, numbers, bools)
            if (pos < json.size())
            {
                if (json[pos] == '"')
                {
                    std::string value = ParseJsonStringValue(json, pos);
                    table.PushBackTableDatum(TableDatum(key.c_str(), value));
                }
                else if (json[pos] == 't' || json[pos] == 'f')
                {
                    bool value = (json[pos] == 't');
                    // Skip "true" or "false"
                    while (pos < json.size() && json[pos] >= 'a' && json[pos] <= 'z') pos++;
                    table.PushBackTableDatum(TableDatum(key.c_str(), value));
                }
                else if (json[pos] == '-' || (json[pos] >= '0' && json[pos] <= '9'))
                {
                    size_t start = pos;
                    bool isFloat = false;
                    if (json[pos] == '-') pos++;
                    while (pos < json.size() && ((json[pos] >= '0' && json[pos] <= '9') || json[pos] == '.'))
                    {
                        if (json[pos] == '.') isFloat = true;
                        pos++;
                    }
                    std::string numStr = json.substr(start, pos - start);
                    if (isFloat)
                        table.PushBackTableDatum(TableDatum(key.c_str(), (float)atof(numStr.c_str())));
                    else
                        table.PushBackTableDatum(TableDatum(key.c_str(), (int32_t)atoi(numStr.c_str())));
                }
                else
                {
                    // Skip unknown value (null, nested objects, arrays)
                    while (pos < json.size() && json[pos] != ',' && json[pos] != '}') pos++;
                }
            }

            SkipWhitespace(json, pos);
            if (pos < json.size() && json[pos] == ',')
                pos++;
        }
    }

    SetOutputValue(0, table);
}

glm::vec4 ParseJsonToDictionaryNode::GetNodeColor() const { return kDictionaryNodeColor; }

// =============================================================================
// GetDictionaryValueNode
// =============================================================================
DEFINE_GRAPH_NODE(GetDictionaryValueNode);

void GetDictionaryValueNode::SetupPins()
{
    AddInputPin("Dictionary", DatumType::Table);
    AddInputPin("Key", DatumType::String, Datum(std::string("")));
    AddOutputPin("Value", DatumType::String);
    AddOutputPin("Found", DatumType::Bool);
}

void GetDictionaryValueNode::Evaluate()
{
    Datum dictDatum = GetInputValue(0);
    const std::string& key = GetInputValue(1).GetString();

    std::string result;
    bool found = false;

    if (dictDatum.GetType() == DatumType::Table)
    {
        TableDatum* td = dictDatum.FindTableDatum(key.c_str());
        if (td != nullptr)
        {
            found = true;
            switch (td->GetType())
            {
                case DatumType::String: result = td->GetString(); break;
                case DatumType::Integer: result = std::to_string(td->GetInteger()); break;
                case DatumType::Float:
                {
                    char buf[64];
                    snprintf(buf, sizeof(buf), "%g", td->GetFloat());
                    result = buf;
                    break;
                }
                case DatumType::Bool: result = td->GetBool() ? "true" : "false"; break;
                default: result = "<unsupported type>"; break;
            }
        }
    }

    SetOutputValue(0, Datum(result));
    SetOutputValue(1, Datum(found));
}

glm::vec4 GetDictionaryValueNode::GetNodeColor() const { return kDictionaryNodeColor; }

// =============================================================================
// GetDictionaryKeysNode
// =============================================================================
DEFINE_GRAPH_NODE(GetDictionaryKeysNode);

void GetDictionaryKeysNode::SetupPins()
{
    AddInputPin("Dictionary", DatumType::Table);
    AddOutputPin("Keys", DatumType::String);
}

void GetDictionaryKeysNode::Evaluate()
{
    Datum dictDatum = GetInputValue(0);
    std::string result;

    if (dictDatum.GetType() == DatumType::Table)
    {
        for (uint32_t i = 0; i < dictDatum.GetCount(); ++i)
        {
            const TableDatum& td = dictDatum.GetTableDatum(i);
            if (i > 0) result += ", ";
            if (td.IsStringKey())
            {
                result += td.GetStringKey();
            }
            else if (td.IsIntegerKey())
            {
                result += std::to_string(td.GetIntegerKey());
            }
        }
    }

    SetOutputValue(0, Datum(result));
}

glm::vec4 GetDictionaryKeysNode::GetNodeColor() const { return kDictionaryNodeColor; }

// =============================================================================
// DictionaryToJsonNode
// =============================================================================
DEFINE_GRAPH_NODE(DictionaryToJsonNode);

void DictionaryToJsonNode::SetupPins()
{
    AddInputPin("Dictionary", DatumType::Table);
    AddOutputPin("JSON", DatumType::String);
}

void DictionaryToJsonNode::Evaluate()
{
    Datum dictDatum = GetInputValue(0);
    std::string result = "{";

    if (dictDatum.GetType() == DatumType::Table)
    {
        for (uint32_t i = 0; i < dictDatum.GetCount(); ++i)
        {
            const TableDatum& td = dictDatum.GetTableDatum(i);
            if (i > 0) result += ", ";

            // Key
            if (td.IsStringKey())
            {
                result += "\"";
                result += td.GetStringKey();
                result += "\"";
            }
            else if (td.IsIntegerKey())
            {
                result += "\"";
                result += std::to_string(td.GetIntegerKey());
                result += "\"";
            }

            result += ": ";

            // Value
            switch (td.GetType())
            {
                case DatumType::String:
                {
                    result += "\"";
                    result += td.GetString();
                    result += "\"";
                    break;
                }
                case DatumType::Integer:
                    result += std::to_string(td.GetInteger());
                    break;
                case DatumType::Float:
                {
                    char buf[64];
                    snprintf(buf, sizeof(buf), "%g", td.GetFloat());
                    result += buf;
                    break;
                }
                case DatumType::Bool:
                    result += td.GetBool() ? "true" : "false";
                    break;
                default:
                    result += "null";
                    break;
            }
        }
    }

    result += "}";
    SetOutputValue(0, Datum(result));
}

glm::vec4 DictionaryToJsonNode::GetNodeColor() const { return kDictionaryNodeColor; }

// =============================================================================
// DictionaryToStringNode
// =============================================================================
DEFINE_GRAPH_NODE(DictionaryToStringNode);

void DictionaryToStringNode::SetupPins()
{
    AddInputPin("Dictionary", DatumType::Table);
    AddOutputPin("Out", DatumType::String);
}

void DictionaryToStringNode::Evaluate()
{
    Datum dictDatum = GetInputValue(0);
    std::string result;

    if (dictDatum.GetType() == DatumType::Table)
    {
        for (uint32_t i = 0; i < dictDatum.GetCount(); ++i)
        {
            const TableDatum& td = dictDatum.GetTableDatum(i);
            if (i > 0) result += "\n";

            // Key
            if (td.IsStringKey())
            {
                result += td.GetStringKey();
            }
            else if (td.IsIntegerKey())
            {
                result += std::to_string(td.GetIntegerKey());
            }

            result += " = ";

            // Value
            switch (td.GetType())
            {
                case DatumType::String: result += td.GetString(); break;
                case DatumType::Integer: result += std::to_string(td.GetInteger()); break;
                case DatumType::Float:
                {
                    char buf[64];
                    snprintf(buf, sizeof(buf), "%g", td.GetFloat());
                    result += buf;
                    break;
                }
                case DatumType::Bool: result += td.GetBool() ? "true" : "false"; break;
                default: result += "<unsupported>"; break;
            }
        }
    }

    SetOutputValue(0, Datum(result));
}

glm::vec4 DictionaryToStringNode::GetNodeColor() const { return kDictionaryNodeColor; }

// =============================================================================
// StringConcatenateNode
// =============================================================================
DEFINE_GRAPH_NODE(StringConcatenateNode);

void StringConcatenateNode::SetupPins()
{
    AddInputPin("A", DatumType::String, Datum(std::string("")));
    AddInputPin("B", DatumType::String, Datum(std::string("")));
    AddOutputPin("Out", DatumType::String);
}

void StringConcatenateNode::Evaluate()
{
    const std::string& a = GetInputValue(0).GetString();
    const std::string& b = GetInputValue(1).GetString();
    SetOutputValue(0, Datum(std::string(a + b)));
}

glm::vec4 StringConcatenateNode::GetNodeColor() const { return kStringNodeColor; }

// =============================================================================
// StringLengthNode
// =============================================================================
DEFINE_GRAPH_NODE(StringLengthNode);

void StringLengthNode::SetupPins()
{
    AddInputPin("Value", DatumType::String, Datum(std::string("")));
    AddOutputPin("Length", DatumType::Integer);
}

void StringLengthNode::Evaluate()
{
    const std::string& str = GetInputValue(0).GetString();
    SetOutputValue(0, Datum((int32_t)str.size()));
}

glm::vec4 StringLengthNode::GetNodeColor() const { return kStringNodeColor; }

// =============================================================================
// SubstringNode
// =============================================================================
DEFINE_GRAPH_NODE(SubstringNode);

void SubstringNode::SetupPins()
{
    AddInputPin("Value", DatumType::String, Datum(std::string("")));
    AddInputPin("Start", DatumType::Integer, Datum(0));
    AddInputPin("Length", DatumType::Integer, Datum(1));
    AddOutputPin("Out", DatumType::String);
}

void SubstringNode::Evaluate()
{
    const std::string& str = GetInputValue(0).GetString();
    int32_t start = GetInputValue(1).GetInteger();
    int32_t length = GetInputValue(2).GetInteger();

    std::string result;
    if (start >= 0 && start < (int32_t)str.size())
    {
        result = str.substr(start, length);
    }

    SetOutputValue(0, Datum(result));
}

glm::vec4 SubstringNode::GetNodeColor() const { return kStringNodeColor; }

// =============================================================================
// StringContainsNode
// =============================================================================
DEFINE_GRAPH_NODE(StringContainsNode);

void StringContainsNode::SetupPins()
{
    AddInputPin("String", DatumType::String, Datum(std::string("")));
    AddInputPin("Search", DatumType::String, Datum(std::string("")));
    AddOutputPin("Contains", DatumType::Bool);
}

void StringContainsNode::Evaluate()
{
    const std::string& str = GetInputValue(0).GetString();
    const std::string& search = GetInputValue(1).GetString();
    SetOutputValue(0, Datum(str.find(search) != std::string::npos));
}

glm::vec4 StringContainsNode::GetNodeColor() const { return kStringNodeColor; }

// =============================================================================
// StringFindNode
// =============================================================================
DEFINE_GRAPH_NODE(StringFindNode);

void StringFindNode::SetupPins()
{
    AddInputPin("String", DatumType::String, Datum(std::string("")));
    AddInputPin("Search", DatumType::String, Datum(std::string("")));
    AddOutputPin("Index", DatumType::Integer);
}

void StringFindNode::Evaluate()
{
    const std::string& str = GetInputValue(0).GetString();
    const std::string& search = GetInputValue(1).GetString();
    size_t pos = str.find(search);
    SetOutputValue(0, Datum(pos != std::string::npos ? (int32_t)pos : -1));
}

glm::vec4 StringFindNode::GetNodeColor() const { return kStringNodeColor; }

// =============================================================================
// StringReplaceNode
// =============================================================================
DEFINE_GRAPH_NODE(StringReplaceNode);

void StringReplaceNode::SetupPins()
{
    AddInputPin("String", DatumType::String, Datum(std::string("")));
    AddInputPin("Find", DatumType::String, Datum(std::string("")));
    AddInputPin("Replace", DatumType::String, Datum(std::string("")));
    AddOutputPin("Out", DatumType::String);
}

void StringReplaceNode::Evaluate()
{
    std::string str = GetInputValue(0).GetString();
    const std::string& find = GetInputValue(1).GetString();
    const std::string& replace = GetInputValue(2).GetString();

    if (!find.empty())
    {
        size_t pos = 0;
        while ((pos = str.find(find, pos)) != std::string::npos)
        {
            str.replace(pos, find.size(), replace);
            pos += replace.size();
        }
    }

    SetOutputValue(0, Datum(str));
}

glm::vec4 StringReplaceNode::GetNodeColor() const { return kStringNodeColor; }

// =============================================================================
// StringSplitNode
// =============================================================================
DEFINE_GRAPH_NODE(StringSplitNode);

void StringSplitNode::SetupPins()
{
    AddInputPin("String", DatumType::String, Datum(std::string("")));
    AddInputPin("Delimiter", DatumType::String, Datum(std::string(",")));
    AddOutputPin("First", DatumType::String);
    AddOutputPin("Rest", DatumType::String);
    AddOutputPin("Count", DatumType::Integer);
}

void StringSplitNode::Evaluate()
{
    const std::string& str = GetInputValue(0).GetString();
    const std::string& delimiter = GetInputValue(1).GetString();

    std::string first = str;
    std::string rest;
    int32_t count = 1;

    if (!delimiter.empty())
    {
        size_t pos = str.find(delimiter);
        if (pos != std::string::npos)
        {
            first = str.substr(0, pos);
            rest = str.substr(pos + delimiter.size());
        }

        // Count occurrences
        count = 1;
        size_t searchPos = 0;
        while ((searchPos = str.find(delimiter, searchPos)) != std::string::npos)
        {
            count++;
            searchPos += delimiter.size();
        }
    }

    SetOutputValue(0, Datum(first));
    SetOutputValue(1, Datum(rest));
    SetOutputValue(2, Datum(count));
}

glm::vec4 StringSplitNode::GetNodeColor() const { return kStringNodeColor; }

// =============================================================================
// StringToUpperNode
// =============================================================================
DEFINE_GRAPH_NODE(StringToUpperNode);

void StringToUpperNode::SetupPins()
{
    AddInputPin("Value", DatumType::String, Datum(std::string("")));
    AddOutputPin("Out", DatumType::String);
}

void StringToUpperNode::Evaluate()
{
    const std::string& str = GetInputValue(0).GetString();
    SetOutputValue(0, Datum(StringToUpper(str)));
}

glm::vec4 StringToUpperNode::GetNodeColor() const { return kStringNodeColor; }

// =============================================================================
// StringToLowerNode
// =============================================================================
DEFINE_GRAPH_NODE(StringToLowerNode);

void StringToLowerNode::SetupPins()
{
    AddInputPin("Value", DatumType::String, Datum(std::string("")));
    AddOutputPin("Out", DatumType::String);
}

void StringToLowerNode::Evaluate()
{
    const std::string& str = GetInputValue(0).GetString();
    SetOutputValue(0, Datum(StringToLower(str)));
}

glm::vec4 StringToLowerNode::GetNodeColor() const { return kStringNodeColor; }

// =============================================================================
// StringTrimNode
// =============================================================================
DEFINE_GRAPH_NODE(StringTrimNode);

void StringTrimNode::SetupPins()
{
    AddInputPin("Value", DatumType::String, Datum(std::string("")));
    AddOutputPin("Out", DatumType::String);
}

void StringTrimNode::Evaluate()
{
    const std::string& str = GetInputValue(0).GetString();
    std::string result;

    size_t start = str.find_first_not_of(" \t\n\r");
    if (start != std::string::npos)
    {
        size_t end = str.find_last_not_of(" \t\n\r");
        result = str.substr(start, end - start + 1);
    }

    SetOutputValue(0, Datum(result));
}

glm::vec4 StringTrimNode::GetNodeColor() const { return kStringNodeColor; }

// =============================================================================
// StringFormatNode
// =============================================================================
DEFINE_GRAPH_NODE(StringFormatNode);

void StringFormatNode::SetupPins()
{
    AddInputPin("Format", DatumType::String, Datum(std::string("%f %f")));
    AddInputPin("A", DatumType::Float, Datum(0.0f));
    AddInputPin("B", DatumType::Float, Datum(0.0f));
    AddOutputPin("Out", DatumType::String);
}

void StringFormatNode::Evaluate()
{
    const std::string& format = GetInputValue(0).GetString();
    float a = GetInputValue(1).GetFloat();
    float b = GetInputValue(2).GetFloat();

    char buf[256];
    snprintf(buf, sizeof(buf), format.c_str(), a, b);
    SetOutputValue(0, Datum(std::string(buf)));
}

glm::vec4 StringFormatNode::GetNodeColor() const { return kStringNodeColor; }

// =============================================================================
// StringStartsWithNode
// =============================================================================
DEFINE_GRAPH_NODE(StringStartsWithNode);

void StringStartsWithNode::SetupPins()
{
    AddInputPin("String", DatumType::String, Datum(std::string("")));
    AddInputPin("Prefix", DatumType::String, Datum(std::string("")));
    AddOutputPin("Result", DatumType::Bool);
}

void StringStartsWithNode::Evaluate()
{
    const std::string& str = GetInputValue(0).GetString();
    const std::string& prefix = GetInputValue(1).GetString();
    SetOutputValue(0, Datum(str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0));
}

glm::vec4 StringStartsWithNode::GetNodeColor() const { return kStringNodeColor; }

// =============================================================================
// StringEndsWithNode
// =============================================================================
DEFINE_GRAPH_NODE(StringEndsWithNode);

void StringEndsWithNode::SetupPins()
{
    AddInputPin("String", DatumType::String, Datum(std::string("")));
    AddInputPin("Suffix", DatumType::String, Datum(std::string("")));
    AddOutputPin("Result", DatumType::Bool);
}

void StringEndsWithNode::Evaluate()
{
    const std::string& str = GetInputValue(0).GetString();
    const std::string& suffix = GetInputValue(1).GetString();
    SetOutputValue(0, Datum(str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0));
}

glm::vec4 StringEndsWithNode::GetNodeColor() const { return kStringNodeColor; }

// =============================================================================
// MakeVectorNode
// =============================================================================
DEFINE_GRAPH_NODE(MakeVectorNode);

void MakeVectorNode::SetupPins()
{
    AddInputPin("X", DatumType::Float, Datum(0.0f));
    AddInputPin("Y", DatumType::Float, Datum(0.0f));
    AddInputPin("Z", DatumType::Float, Datum(0.0f));
    AddOutputPin("Vector", DatumType::Vector);
}

void MakeVectorNode::Evaluate()
{
    float x = GetInputValue(0).GetFloat();
    float y = GetInputValue(1).GetFloat();
    float z = GetInputValue(2).GetFloat();
    SetOutputValue(0, Datum(glm::vec3(x, y, z)));
}

glm::vec4 MakeVectorNode::GetNodeColor() const { return kUtilityNodeColor; }

// =============================================================================
// MakeVector2DNode
// =============================================================================
DEFINE_GRAPH_NODE(MakeVector2DNode);

void MakeVector2DNode::SetupPins()
{
    AddInputPin("X", DatumType::Float, Datum(0.0f));
    AddInputPin("Y", DatumType::Float, Datum(0.0f));
    AddOutputPin("Vector2D", DatumType::Vector2D);
}

void MakeVector2DNode::Evaluate()
{
    float x = GetInputValue(0).GetFloat();
    float y = GetInputValue(1).GetFloat();
    SetOutputValue(0, Datum(glm::vec2(x, y)));
}

glm::vec4 MakeVector2DNode::GetNodeColor() const { return kUtilityNodeColor; }

// =============================================================================
// NodeToNode3DNode
// =============================================================================
DEFINE_GRAPH_NODE(NodeToNode3DNode);

void NodeToNode3DNode::SetupPins()
{
    AddInputPin("Node", DatumType::Node);
    AddOutputPin("Node3D", DatumType::Node3D);
    AddOutputPin("Success", DatumType::Bool);
}

void NodeToNode3DNode::Evaluate()
{
    WeakPtr<Node> nodeRef = GetInputValue(0).GetNode();
    Node* node = nodeRef.Get();
    Node3D* node3d = node ? node->As<Node3D>() : nullptr;

    SetOutputValue(0, Datum(node3d));
    SetOutputValue(1, Datum(node3d != nullptr));
}

glm::vec4 NodeToNode3DNode::GetNodeColor() const { return kUtilityNodeColor; }

// =============================================================================
// Node3DToNodeNode
// =============================================================================
DEFINE_GRAPH_NODE(Node3DToNodeNode);

void Node3DToNodeNode::SetupPins()
{
    AddInputPin("Node3D", DatumType::Node3D);
    AddOutputPin("Node", DatumType::Node);
    AddOutputPin("Success", DatumType::Bool);
}

void Node3DToNodeNode::Evaluate()
{
    WeakPtr<Node> nodeRef = GetInputValue(0).GetNode();
    Node* node = nodeRef.Get();

    SetOutputValue(0, Datum(node));
    SetOutputValue(1, Datum(node != nullptr));
}

glm::vec4 Node3DToNodeNode::GetNodeColor() const { return kUtilityNodeColor; }

// =============================================================================
// RerouteNode
// =============================================================================
DEFINE_GRAPH_NODE(RerouteNode);

void RerouteNode::SetupPins()
{
    AddInputPin("In", DatumType::Float, Datum(0.0f));
    AddOutputPin("Out", DatumType::Float);
}

void RerouteNode::Evaluate()
{
    SetOutputValue(0, GetInputValue(0));
}

glm::vec4 RerouteNode::GetNodeColor() const { return kUtilityNodeColor; }



// =============================================================================
// NodeToTextNode
// =============================================================================
DEFINE_GRAPH_NODE(NodeToTextNode);

void NodeToTextNode::SetupPins()
{
    AddInputPin("Node", DatumType::Node);
    AddOutputPin("Text", DatumType::Text);
    AddOutputPin("Success", DatumType::Bool);
}

void NodeToTextNode::Evaluate()
{
    WeakPtr<Node> nodeRef = GetInputValue(0).GetNode();
    Node* node = nodeRef.Get();
    Text* text = node ? node->As<Text>() : nullptr;

    SetOutputValue(0, Datum((Node*)text));
    SetOutputValue(1, Datum(text != nullptr));
}
glm::vec4 NodeToTextNode::GetNodeColor() const { return kUtilityNodeColor; }

// =============================================================================
// NodeToTextNode
// =============================================================================
DEFINE_GRAPH_NODE(Node3DToTextNode);

void Node3DToTextNode::SetupPins()
{
    AddInputPin("Node3D", DatumType::Node3D);
    AddOutputPin("Text", DatumType::Text);
    AddOutputPin("Success", DatumType::Bool);
}

void Node3DToTextNode::Evaluate()
{
    WeakPtr<Node> nodeRef = GetInputValue(0).GetNode();
    Node* node = nodeRef.Get();
    Text* text = node ? node->As<Text>() : nullptr;

    SetOutputValue(0, Datum((Node*)text));
    SetOutputValue(1, Datum(text != nullptr));
}
glm::vec4 Node3DToTextNode::GetNodeColor() const { return kUtilityNodeColor; }
