#pragma once

#include "NodeGraph/GraphNode.h"

// --- Float Constant ---
class FloatConstantNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(FloatConstantNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Float"; }
    virtual const char* GetNodeCategory() const override { return "Value"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Int Constant ---
class IntConstantNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(IntConstantNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Integer"; }
    virtual const char* GetNodeCategory() const override { return "Value"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Vector Constant ---
class VectorConstantNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(VectorConstantNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Vector"; }
    virtual const char* GetNodeCategory() const override { return "Value"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Color Constant ---
class ColorConstantNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(ColorConstantNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Color"; }
    virtual const char* GetNodeCategory() const override { return "Value"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Time ---
class TimeNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(TimeNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Time"; }
    virtual const char* GetNodeCategory() const override { return "Value"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Viewer ---
class ViewerNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(ViewerNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Viewer"; }
    virtual const char* GetNodeCategory() const override { return "Utility"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Debug Log ---
class DebugLogNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(DebugLogNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Debug Log"; }
    virtual const char* GetNodeCategory() const override { return "Utility"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// =============================================================================
// String Conversion Nodes
// =============================================================================

// --- Float To String ---
class FloatToStringNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(FloatToStringNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Float To String"; }
    virtual const char* GetNodeCategory() const override { return "Conversion"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Int To String ---
class IntToStringNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(IntToStringNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Int To String"; }
    virtual const char* GetNodeCategory() const override { return "Conversion"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Color To String ---
class ColorToStringNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(ColorToStringNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Color To String"; }
    virtual const char* GetNodeCategory() const override { return "Conversion"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Vector To String ---
class VectorToStringNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(VectorToStringNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Vector To String"; }
    virtual const char* GetNodeCategory() const override { return "Conversion"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Bool To String ---
class BoolToStringNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(BoolToStringNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Bool To String"; }
    virtual const char* GetNodeCategory() const override { return "Conversion"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Node To String ---
class NodeToStringNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(NodeToStringNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Node To String"; }
    virtual const char* GetNodeCategory() const override { return "Conversion"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Node3D To String ---
class Node3DToStringNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(Node3DToStringNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Node3D To String"; }
    virtual const char* GetNodeCategory() const override { return "Conversion"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- String To Float ---
class StringToFloatNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(StringToFloatNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "String To Float"; }
    virtual const char* GetNodeCategory() const override { return "Conversion"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- String To Int ---
class StringToIntNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(StringToIntNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "String To Int"; }
    virtual const char* GetNodeCategory() const override { return "Conversion"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- String To Bool ---
class StringToBoolNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(StringToBoolNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "String To Bool"; }
    virtual const char* GetNodeCategory() const override { return "Conversion"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- String To Vector ---
class StringToVectorNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(StringToVectorNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "String To Vector"; }
    virtual const char* GetNodeCategory() const override { return "Conversion"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- String To Color ---
class StringToColorNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(StringToColorNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "String To Color"; }
    virtual const char* GetNodeCategory() const override { return "Conversion"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// =============================================================================
// Type Conversion Nodes
// =============================================================================

// --- Vector To Color ---
class VectorToColorNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(VectorToColorNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Vector To Color"; }
    virtual const char* GetNodeCategory() const override { return "Conversion"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Color To Vector ---
class ColorToVectorNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(ColorToVectorNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Color To Vector"; }
    virtual const char* GetNodeCategory() const override { return "Conversion"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Split Vector ---
class SplitVectorNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SplitVectorNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Split Vector"; }
    virtual const char* GetNodeCategory() const override { return "Conversion"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Split Color ---
class SplitColorNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SplitColorNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Split Color"; }
    virtual const char* GetNodeCategory() const override { return "Conversion"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// =============================================================================
// Color Construction Nodes
// =============================================================================

// --- Hex To Color ---
class HexToColorNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(HexToColorNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Hex To Color"; }
    virtual const char* GetNodeCategory() const override { return "Color"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- RGBA Color ---
class RGBAColorNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(RGBAColorNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "RGBA Color"; }
    virtual const char* GetNodeCategory() const override { return "Color"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- RGB Color ---
class RGBColorNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(RGBColorNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "RGB Color"; }
    virtual const char* GetNodeCategory() const override { return "Color"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Linear Color ---
class LinearColorNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(LinearColorNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Linear Color"; }
    virtual const char* GetNodeCategory() const override { return "Color"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// =============================================================================
// DateTime Nodes
// =============================================================================

// --- DateTime To String ---
class DateTimeToStringNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(DateTimeToStringNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "DateTime To String"; }
    virtual const char* GetNodeCategory() const override { return "DateTime"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- DateTime Now ---
class DateTimeNowNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(DateTimeNowNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "DateTime Now"; }
    virtual const char* GetNodeCategory() const override { return "DateTime"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- DateTime To Float ---
class DateTimeToFloatNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(DateTimeToFloatNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "DateTime To Float"; }
    virtual const char* GetNodeCategory() const override { return "DateTime"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Float To DateTime ---
class FloatToDateTimeNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(FloatToDateTimeNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Float To DateTime"; }
    virtual const char* GetNodeCategory() const override { return "DateTime"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Add Seconds ---
class AddSecondsNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(AddSecondsNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Add Seconds"; }
    virtual const char* GetNodeCategory() const override { return "DateTime"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Add Minutes ---
class AddMinutesNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(AddMinutesNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Add Minutes"; }
    virtual const char* GetNodeCategory() const override { return "DateTime"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Add Hours ---
class AddHoursNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(AddHoursNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Add Hours"; }
    virtual const char* GetNodeCategory() const override { return "DateTime"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// =============================================================================
// Dictionary Nodes
// =============================================================================

// --- String To Dictionary ---
class StringToDictionaryNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(StringToDictionaryNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "String To Dictionary"; }
    virtual const char* GetNodeCategory() const override { return "Dictionary"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Parse Json To Dictionary ---
class ParseJsonToDictionaryNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(ParseJsonToDictionaryNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Parse JSON To Dictionary"; }
    virtual const char* GetNodeCategory() const override { return "Dictionary"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Get Dictionary Value ---
class GetDictionaryValueNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GetDictionaryValueNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Get Dictionary Value"; }
    virtual const char* GetNodeCategory() const override { return "Dictionary"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Get Dictionary Keys ---
class GetDictionaryKeysNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GetDictionaryKeysNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Get Dictionary Keys"; }
    virtual const char* GetNodeCategory() const override { return "Dictionary"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Dictionary To Json ---
class DictionaryToJsonNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(DictionaryToJsonNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Dictionary To JSON"; }
    virtual const char* GetNodeCategory() const override { return "Dictionary"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Dictionary To String ---
class DictionaryToStringNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(DictionaryToStringNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Dictionary To String"; }
    virtual const char* GetNodeCategory() const override { return "Dictionary"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// =============================================================================
// String Operation Nodes
// =============================================================================

// --- String Concatenate ---
class StringConcatenateNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(StringConcatenateNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "String Concatenate"; }
    virtual const char* GetNodeCategory() const override { return "String"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- String Length ---
class StringLengthNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(StringLengthNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "String Length"; }
    virtual const char* GetNodeCategory() const override { return "String"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Substring ---
class SubstringNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SubstringNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Substring"; }
    virtual const char* GetNodeCategory() const override { return "String"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- String Contains ---
class StringContainsNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(StringContainsNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "String Contains"; }
    virtual const char* GetNodeCategory() const override { return "String"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- String Find ---
class StringFindNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(StringFindNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "String Find"; }
    virtual const char* GetNodeCategory() const override { return "String"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- String Replace ---
class StringReplaceNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(StringReplaceNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "String Replace"; }
    virtual const char* GetNodeCategory() const override { return "String"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- String Split ---
class StringSplitNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(StringSplitNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "String Split"; }
    virtual const char* GetNodeCategory() const override { return "String"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- String To Upper ---
class StringToUpperNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(StringToUpperNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "String To Upper"; }
    virtual const char* GetNodeCategory() const override { return "String"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- String To Lower ---
class StringToLowerNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(StringToLowerNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "String To Lower"; }
    virtual const char* GetNodeCategory() const override { return "String"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- String Trim ---
class StringTrimNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(StringTrimNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "String Trim"; }
    virtual const char* GetNodeCategory() const override { return "String"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- String Format ---
class StringFormatNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(StringFormatNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "String Format"; }
    virtual const char* GetNodeCategory() const override { return "String"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- String Starts With ---
class StringStartsWithNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(StringStartsWithNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "String Starts With"; }
    virtual const char* GetNodeCategory() const override { return "String"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- String Ends With ---
class StringEndsWithNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(StringEndsWithNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "String Ends With"; }
    virtual const char* GetNodeCategory() const override { return "String"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// =============================================================================
// Utility Nodes
// =============================================================================

// --- Make Vector ---
class MakeVectorNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(MakeVectorNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Make Vector"; }
    virtual const char* GetNodeCategory() const override { return "Utility"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Make Vector 2D ---
class MakeVector2DNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(MakeVector2DNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Make Vector 2D"; }
    virtual const char* GetNodeCategory() const override { return "Utility"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Node To Node3D ---
class NodeToNode3DNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(NodeToNode3DNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Node To Node3D"; }
    virtual const char* GetNodeCategory() const override { return "Utility"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Node3D To Node ---
class Node3DToNodeNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(Node3DToNodeNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Node3D To Node"; }
    virtual const char* GetNodeCategory() const override { return "Utility"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Reroute ---
class RerouteNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(RerouteNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Reroute"; }
    virtual const char* GetNodeCategory() const override { return "Utility"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Node to Text ---
class NodeToTextNode : public GraphNode
{
    public:
    DECLARE_GRAPH_NODE(NodeToTextNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Node To Text"; }
    virtual const char* GetNodeCategory() const override { return "Utility"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Node to Text ---
class Node3DToTextNode : public GraphNode
{
    public:
    DECLARE_GRAPH_NODE(Node3DToTextNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Node3D To Text"; }
    virtual const char* GetNodeCategory() const override { return "Utility"; }
    virtual glm::vec4 GetNodeColor() const override;
};