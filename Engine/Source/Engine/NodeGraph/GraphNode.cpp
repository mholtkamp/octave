#include "NodeGraph/GraphNode.h"
#include "NodeGraph/NodeGraph.h"
#include "NodeGraph/GraphLink.h"
#include "Stream.h"
#include "Log.h"

DEFINE_FACTORY_MANAGER(GraphNode);
DEFINE_FACTORY(GraphNode, GraphNode);
DEFINE_OBJECT(GraphNode);

GraphNode::GraphNode()
{
}

GraphNode::~GraphNode()
{
}

void GraphNode::SetupPins()
{
}

void GraphNode::Evaluate()
{
}

glm::vec4 GraphNode::GetNodeColor() const
{
    return glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
}

const std::string& GraphNode::GetInputName() const
{
    static std::string sEmpty;
    return sEmpty;
}

void GraphNode::WriteDatumToStream(Stream& stream, const Datum& datum)
{
    stream.WriteUint8((uint8_t)datum.mType);

    switch (datum.mType)
    {
    case DatumType::Integer:  stream.WriteInt32(datum.GetInteger()); break;
    case DatumType::Float:    stream.WriteFloat(datum.GetFloat()); break;
    case DatumType::Bool:     stream.WriteBool(datum.GetBool()); break;
    case DatumType::String:   stream.WriteString(datum.GetString()); break;
    case DatumType::Vector2D: stream.WriteVec2(datum.GetVector2D()); break;
    case DatumType::Vector:   stream.WriteVec3(datum.GetVector()); break;
    case DatumType::Color:    stream.WriteVec4(datum.GetColor()); break;
    default: break;
    }
}

Datum GraphNode::ReadDatumFromStream(Stream& stream)
{
    DatumType type = (DatumType)stream.ReadUint8();

    switch (type)
    {
    case DatumType::Integer:  return Datum(stream.ReadInt32());
    case DatumType::Float:    return Datum(stream.ReadFloat());
    case DatumType::Bool:     return Datum(stream.ReadBool());
    case DatumType::String:   { std::string s; stream.ReadString(s); return Datum(s); }
    case DatumType::Vector2D: return Datum(stream.ReadVec2());
    case DatumType::Vector:   return Datum(stream.ReadVec3());
    case DatumType::Color:    return Datum(stream.ReadVec4());
    default: return Datum();
    }
}

void GraphNode::SaveStream(Stream& stream)
{
    stream.WriteUint32(mId);
    stream.WriteVec2(mEditorPosition);

    stream.WriteUint32((uint32_t)mInputPins.size());
    for (uint32_t i = 0; i < mInputPins.size(); ++i)
    {
        stream.WriteUint32(mInputPins[i].mId);
        WriteDatumToStream(stream, mInputPins[i].mDefaultValue);
    }

    stream.WriteUint32((uint32_t)mOutputPins.size());
    for (uint32_t i = 0; i < mOutputPins.size(); ++i)
    {
        stream.WriteUint32(mOutputPins[i].mId);
    }

    stream.WriteUint32(mNextPinId);
}

void GraphNode::LoadStream(Stream& stream, uint32_t version)
{
    mId = stream.ReadUint32();
    mEditorPosition = stream.ReadVec2();

    // SetupPins should have been called before LoadStream
    uint32_t numInputPins = stream.ReadUint32();
    for (uint32_t i = 0; i < numInputPins; ++i)
    {
        GraphPinId pinId = stream.ReadUint32();
        Datum defaultValue = ReadDatumFromStream(stream);

        if (i < mInputPins.size())
        {
            mInputPins[i].mId = pinId;
            mInputPins[i].mDefaultValue = defaultValue;
            mInputPins[i].mValue = defaultValue;
        }
    }

    uint32_t numOutputPins = stream.ReadUint32();
    for (uint32_t i = 0; i < numOutputPins; ++i)
    {
        GraphPinId pinId = stream.ReadUint32();

        if (i < mOutputPins.size())
        {
            mOutputPins[i].mId = pinId;
        }
    }

    mNextPinId = stream.ReadUint32();
}

GraphPin& GraphNode::AddInputPin(const char* name, DatumType type, const Datum& defaultValue)
{
    GraphPin pin;
    pin.mId = mNextPinId++;
    pin.mOwnerNodeId = mId;
    pin.mName = name;
    pin.mDataType = type;
    pin.mDirection = GraphPinDirection::Input;
    pin.mDefaultValue = defaultValue;
    pin.mValue = defaultValue;

    mInputPins.push_back(pin);
    return mInputPins.back();
}

GraphPin& GraphNode::AddOutputPin(const char* name, DatumType type)
{
    GraphPin pin;
    pin.mId = mNextPinId++;
    pin.mOwnerNodeId = mId;
    pin.mName = name;
    pin.mDataType = type;
    pin.mDirection = GraphPinDirection::Output;

    // Initialize default output value
    switch (type)
    {
    case DatumType::Integer:  pin.mValue = Datum(0); break;
    case DatumType::Float:    pin.mValue = Datum(0.0f); break;
    case DatumType::Bool:     pin.mValue = Datum(false); break;
    case DatumType::String:   pin.mValue = Datum(""); break;
    case DatumType::Vector2D: pin.mValue = Datum(glm::vec2(0.0f)); break;
    case DatumType::Vector:   pin.mValue = Datum(glm::vec3(0.0f)); break;
    case DatumType::Color:    pin.mValue = Datum(glm::vec4(0.0f)); break;
    case DatumType::Node:     pin.mValue = Datum((Node*)nullptr); break;
    case DatumType::Node3D:   pin.mValue = Datum((Node*)nullptr); break;
    case DatumType::Widget:   pin.mValue = Datum((Node*)nullptr); break;
    case DatumType::Text:     pin.mValue = Datum((Node*)nullptr); break;
    case DatumType::Quad:     pin.mValue = Datum((Node*)nullptr); break;
    case DatumType::Audio3D:  pin.mValue = Datum((Node*)nullptr); break;
    case DatumType::Scene:    pin.mValue = Datum((Asset*)nullptr); break;
    case DatumType::Execution: pin.mName = "    "; break;
    default: break;
    }

    mOutputPins.push_back(pin);
    return mOutputPins.back();
}

GraphPinId GraphNode::GetInputPinId(uint32_t index) const
{
    if (index < mInputPins.size())
    {
        return mInputPins[index].mId;
    }
    return INVALID_GRAPH_PIN_ID;
}

GraphPinId GraphNode::GetOutputPinId(uint32_t index) const
{
    if (index < mOutputPins.size())
    {
        return mOutputPins[index].mId;
    }
    return INVALID_GRAPH_PIN_ID;
}

GraphPin* GraphNode::GetInputPin(uint32_t index)
{
    if (index < mInputPins.size())
    {
        return &mInputPins[index];
    }
    return nullptr;
}

GraphPin* GraphNode::GetOutputPin(uint32_t index)
{
    if (index < mOutputPins.size())
    {
        return &mOutputPins[index];
    }
    return nullptr;
}

const Datum& GraphNode::GetInputValue(uint32_t index) const
{
    if (index < mInputPins.size())
    {
        return mInputPins[index].mValue;
    }

    static Datum sNullDatum;
    return sNullDatum;
}

void GraphNode::SetOutputValue(uint32_t index, const Datum& value)
{
    if (index < mOutputPins.size())
    {
        mOutputPins[index].mValue = value;
    }
}

void GraphNode::TriggerExecutionPin(uint32_t outputPinIndex)
{
    if (outputPinIndex >= mOutputPins.size())
        return;

    GraphPin& outPin = mOutputPins[outputPinIndex];
    if (outPin.mDataType != DatumType::Execution)
        return;

    outPin.mExecutionTriggered = true;

    if (mGraph == nullptr)
        return;

    // Propagate to connected input execution pins
    const std::vector<GraphLink>& links = mGraph->GetLinks();
    for (uint32_t i = 0; i < links.size(); ++i)
    {
        if (links[i].mOutputPinId == outPin.mId)
        {
            GraphPin* inputPin = mGraph->FindPin(links[i].mInputPinId);
            if (inputPin != nullptr && inputPin->mDataType == DatumType::Execution)
            {
                inputPin->mExecutionTriggered = true;
            }
        }
    }
}

bool GraphNode::WasExecutionTriggered(uint32_t inputPinIndex) const
{
    if (inputPinIndex < mInputPins.size() &&
        mInputPins[inputPinIndex].mDataType == DatumType::Execution)
    {
        return mInputPins[inputPinIndex].mExecutionTriggered;
    }
    return false;
}
