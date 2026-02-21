#include "NodeGraph/Nodes/InputNodes.h"
#include "Stream.h"
#include "Utilities.h"

FORCE_LINK_DEF(InputNodes);

// Register to all domains via REGISTER_GRAPH_NODE_MULTI
DEFINE_GRAPH_NODE(FloatInputNode);
REGISTER_GRAPH_NODE_MULTI(FloatInputNode, "Float Input", "Input", glm::vec4(0.9f, 0.6f, 0.1f, 1.0f), "Material", "Shader", "Procedural", "Animation", "FSM", "SceneGraph");

DEFINE_GRAPH_NODE(IntInputNode);
REGISTER_GRAPH_NODE_MULTI(IntInputNode, "Int Input", "Input", glm::vec4(0.9f, 0.6f, 0.1f, 1.0f), "Material", "Shader", "Procedural", "Animation", "FSM", "SceneGraph");

DEFINE_GRAPH_NODE(BoolInputNode);
REGISTER_GRAPH_NODE_MULTI(BoolInputNode, "Bool Input", "Input", glm::vec4(0.9f, 0.6f, 0.1f, 1.0f), "Material", "Shader", "Procedural", "Animation", "FSM", "SceneGraph");

DEFINE_GRAPH_NODE(StringInputNode);
REGISTER_GRAPH_NODE_MULTI(StringInputNode, "String Input", "Input", glm::vec4(0.9f, 0.6f, 0.1f, 1.0f), "Material", "Shader", "Procedural", "Animation", "FSM", "SceneGraph");

DEFINE_GRAPH_NODE(ByteInputNode);
REGISTER_GRAPH_NODE_MULTI(ByteInputNode, "Byte Input", "Input", glm::vec4(0.9f, 0.6f, 0.1f, 1.0f), "Material", "Shader", "Procedural", "Animation", "FSM", "SceneGraph");

DEFINE_GRAPH_NODE(AssetInputNode);
REGISTER_GRAPH_NODE_MULTI(AssetInputNode, "Asset Input", "Input", glm::vec4(0.9f, 0.6f, 0.1f, 1.0f), "Material", "Shader", "Procedural", "Animation", "FSM", "SceneGraph");

DEFINE_GRAPH_NODE(VectorInputNode);
REGISTER_GRAPH_NODE_MULTI(VectorInputNode, "Vector Input", "Input", glm::vec4(0.9f, 0.6f, 0.1f, 1.0f), "Material", "Shader", "Procedural", "Animation", "FSM", "SceneGraph");

DEFINE_GRAPH_NODE(ColorInputNode);
REGISTER_GRAPH_NODE_MULTI(ColorInputNode, "Color Input", "Input", glm::vec4(0.9f, 0.6f, 0.1f, 1.0f), "Material", "Shader", "Procedural", "Animation", "FSM", "SceneGraph");

// =============================================================================
// FloatInputNode
// =============================================================================
void FloatInputNode::SetupPins()
{
    AddInputPin("Value", DatumType::Float, Datum(0.0f));
    AddOutputPin("Out", DatumType::Float);
}

void FloatInputNode::Evaluate()
{
    SetOutputValue(0, GetInputValue(0));
}

void FloatInputNode::SaveStream(Stream& stream)
{
    GraphNode::SaveStream(stream);
    stream.WriteString(mInputName);
}

void FloatInputNode::LoadStream(Stream& stream, uint32_t version)
{
    GraphNode::LoadStream(stream, version);
    stream.ReadString(mInputName);
}

// =============================================================================
// IntInputNode
// =============================================================================
void IntInputNode::SetupPins()
{
    AddInputPin("Value", DatumType::Integer, Datum(0));
    AddOutputPin("Out", DatumType::Integer);
}

void IntInputNode::Evaluate()
{
    SetOutputValue(0, GetInputValue(0));
}

void IntInputNode::SaveStream(Stream& stream)
{
    GraphNode::SaveStream(stream);
    stream.WriteString(mInputName);
}

void IntInputNode::LoadStream(Stream& stream, uint32_t version)
{
    GraphNode::LoadStream(stream, version);
    stream.ReadString(mInputName);
}

// =============================================================================
// BoolInputNode
// =============================================================================
void BoolInputNode::SetupPins()
{
    AddInputPin("Value", DatumType::Bool, Datum(false));
    AddOutputPin("Out", DatumType::Bool);
}

void BoolInputNode::Evaluate()
{
    SetOutputValue(0, GetInputValue(0));
}

void BoolInputNode::SaveStream(Stream& stream)
{
    GraphNode::SaveStream(stream);
    stream.WriteString(mInputName);
}

void BoolInputNode::LoadStream(Stream& stream, uint32_t version)
{
    GraphNode::LoadStream(stream, version);
    stream.ReadString(mInputName);
}

// =============================================================================
// StringInputNode
// =============================================================================
void StringInputNode::SetupPins()
{
    AddInputPin("Value", DatumType::String, Datum(std::string("")));
    AddOutputPin("Out", DatumType::String);
}

void StringInputNode::Evaluate()
{
    SetOutputValue(0, GetInputValue(0));
}

void StringInputNode::SaveStream(Stream& stream)
{
    GraphNode::SaveStream(stream);
    stream.WriteString(mInputName);
}

void StringInputNode::LoadStream(Stream& stream, uint32_t version)
{
    GraphNode::LoadStream(stream, version);
    stream.ReadString(mInputName);
}

// =============================================================================
// ByteInputNode
// =============================================================================
void ByteInputNode::SetupPins()
{
    AddInputPin("Value", DatumType::Byte, Datum(uint8_t(0)));
    AddOutputPin("Out", DatumType::Byte);
}

void ByteInputNode::Evaluate()
{
    SetOutputValue(0, GetInputValue(0));
}

void ByteInputNode::SaveStream(Stream& stream)
{
    GraphNode::SaveStream(stream);
    stream.WriteString(mInputName);
}

void ByteInputNode::LoadStream(Stream& stream, uint32_t version)
{
    GraphNode::LoadStream(stream, version);
    stream.ReadString(mInputName);
}

// =============================================================================
// AssetInputNode
// =============================================================================
void AssetInputNode::SetupPins()
{
    AddInputPin("Value", DatumType::Asset, Datum());
    AddOutputPin("Out", DatumType::Asset);
}

void AssetInputNode::Evaluate()
{
    SetOutputValue(0, GetInputValue(0));
}

void AssetInputNode::SaveStream(Stream& stream)
{
    GraphNode::SaveStream(stream);
    stream.WriteString(mInputName);
}

void AssetInputNode::LoadStream(Stream& stream, uint32_t version)
{
    GraphNode::LoadStream(stream, version);
    stream.ReadString(mInputName);
}

// =============================================================================
// VectorInputNode
// =============================================================================
void VectorInputNode::SetupPins()
{
    AddInputPin("Value", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddOutputPin("Out", DatumType::Vector);
}

void VectorInputNode::Evaluate()
{
    SetOutputValue(0, GetInputValue(0));
}

void VectorInputNode::SaveStream(Stream& stream)
{
    GraphNode::SaveStream(stream);
    stream.WriteString(mInputName);
}

void VectorInputNode::LoadStream(Stream& stream, uint32_t version)
{
    GraphNode::LoadStream(stream, version);
    stream.ReadString(mInputName);
}

// =============================================================================
// ColorInputNode
// =============================================================================
void ColorInputNode::SetupPins()
{
    AddInputPin("Value", DatumType::Color, Datum(glm::vec4(1.0f)));
    AddOutputPin("Out", DatumType::Color);
}

void ColorInputNode::Evaluate()
{
    SetOutputValue(0, GetInputValue(0));
}

void ColorInputNode::SaveStream(Stream& stream)
{
    GraphNode::SaveStream(stream);
    stream.WriteString(mInputName);
}

void ColorInputNode::LoadStream(Stream& stream, uint32_t version)
{
    GraphNode::LoadStream(stream, version);
    stream.ReadString(mInputName);
}
