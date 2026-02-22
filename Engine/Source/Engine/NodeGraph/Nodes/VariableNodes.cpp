#include "NodeGraph/Nodes/VariableNodes.h"
#include "NodeGraph/NodeGraph.h"
#include "NodeGraph/GraphVariable.h"
#include "Assets/NodeGraphAsset.h"
#include "Stream.h"
#include "Log.h"

FORCE_LINK_DEF(VariableNodes);

// Only DEFINE (for deserialization), NOT registered with any domain.
// Created dynamically from sidebar drag, context menu, etc.
DEFINE_GRAPH_NODE(GetVariableNode);
DEFINE_GRAPH_NODE(SetVariableNode);

// =============================================================================
// GetVariableNode
// =============================================================================
void GetVariableNode::SetupPins()
{
    // Starts with zero pins — rebuilt from variable
}

void GetVariableNode::Evaluate()
{
    if (mOwnerAsset == nullptr)
        return;

    int32_t idx = mOwnerAsset->FindVariableIndex(mVariableName);
    if (idx < 0)
        return;

    const GraphVariable* var = mOwnerAsset->GetVariable((uint32_t)idx);
    if (var != nullptr && mOutputPins.size() > 0)
    {
        SetOutputValue(0, var->mRuntimeValue);
    }
}

void GetVariableNode::SaveStream(Stream& stream)
{
    GraphNode::SaveStream(stream);
    stream.WriteString(mVariableName);
    stream.WriteUint8((uint8_t)mCachedType);
}

void GetVariableNode::LoadStream(Stream& stream, uint32_t version)
{
    // Override base LoadStream entirely because we have dynamic pins.
    mId = stream.ReadUint32();
    mEditorPosition = stream.ReadVec2();

    // Read base input pins (should be 0 for GetVariableNode)
    uint32_t numInputPins = stream.ReadUint32();
    for (uint32_t i = 0; i < numInputPins; ++i)
    {
        GraphPinId pinId = stream.ReadUint32();
        Datum defaultValue = ReadDatumFromStream(stream);

        if (i >= mInputPins.size())
        {
            AddInputPin("", DatumType::Float, Datum(0.0f));
        }
        mInputPins[i].mId = pinId;
        mInputPins[i].mOwnerNodeId = mId;
        mInputPins[i].mDefaultValue = defaultValue;
        mInputPins[i].mValue = defaultValue;
    }

    // Read base output pins — pre-create pin
    uint32_t numOutputPins = stream.ReadUint32();
    for (uint32_t i = 0; i < numOutputPins; ++i)
    {
        GraphPinId pinId = stream.ReadUint32();

        if (i >= mOutputPins.size())
        {
            AddOutputPin("", DatumType::Float);
        }
        mOutputPins[i].mId = pinId;
        mOutputPins[i].mOwnerNodeId = mId;
    }

    mNextPinId = stream.ReadUint32();

    // Read variable-specific data
    if (version >= ASSET_VERSION_NODE_GRAPH_VARIABLES)
    {
        stream.ReadString(mVariableName);
        mCachedType = (DatumType)stream.ReadUint8();

        // Apply cached type to the output pin
        if (mOutputPins.size() > 0)
        {
            mOutputPins[0].mName = mVariableName;
            mOutputPins[0].mDataType = mCachedType;
        }
    }
}

const char* GetVariableNode::GetNodeTypeName() const
{
    if (mVariableName.empty())
        return "Get Variable";

    mDisplayName = "Get: " + mVariableName;
    return mDisplayName.c_str();
}

void GetVariableNode::CopyCustomData(const GraphNode* src)
{
    if (src->GetType() != GetVariableNode::GetStaticType())
        return;

    const GetVariableNode* srcVar = static_cast<const GetVariableNode*>(src);
    mVariableName = srcVar->mVariableName;
    mOwnerAsset = srcVar->mOwnerAsset;
    mCachedType = srcVar->mCachedType;

    // Recreate dynamic pins to match the source
    mOutputPins.clear();
    mNextPinId = 1;

    for (uint32_t i = 0; i < src->GetNumOutputPins(); ++i)
    {
        const GraphPin& srcPin = src->GetOutputPins()[i];
        GraphPin& newPin = AddOutputPin(srcPin.mName.c_str(), srcPin.mDataType);
        newPin.mId = srcPin.mId;
        newPin.mOwnerNodeId = src->GetId();
        newPin.mValue = srcPin.mValue;
    }
}

void GetVariableNode::RebuildPinFromVariable(NodeGraphAsset* asset)
{
    if (asset == nullptr)
        return;

    mOwnerAsset = asset;

    int32_t idx = asset->FindVariableIndex(mVariableName);
    if (idx < 0)
        return;

    const GraphVariable* var = asset->GetVariable((uint32_t)idx);
    if (var == nullptr)
        return;

    mCachedType = var->mType;

    // Clear existing pins
    mOutputPins.clear();
    mNextPinId = 1;

    AddOutputPin(var->mName.c_str(), var->mType);

    // Reassign pin IDs using graph-global counter to avoid collisions
    if (mGraph != nullptr)
    {
        for (uint32_t i = 0; i < mOutputPins.size(); ++i)
        {
            mOutputPins[i].mId = mGraph->AllocPinId();
            mOutputPins[i].mOwnerNodeId = mId;
        }
    }
}

// =============================================================================
// SetVariableNode
// =============================================================================
void SetVariableNode::SetupPins()
{
    // Starts with zero pins — rebuilt from variable
}

void SetVariableNode::Evaluate()
{
    if (mOwnerAsset == nullptr)
        return;

    int32_t idx = mOwnerAsset->FindVariableIndex(mVariableName);
    if (idx < 0)
        return;

    GraphVariable* var = mOwnerAsset->GetVariable((uint32_t)idx);
    if (var == nullptr)
        return;

    // Pins: [0] Exec In, [1] Value input
    // Output pins: [0] Exec Out, [1] Value output (pass-through)
    if (mInputPins.size() >= 2)
    {
        var->mRuntimeValue = mInputPins[1].mValue;

        // Pass-through output
        if (mOutputPins.size() >= 2)
        {
            SetOutputValue(1, var->mRuntimeValue);
        }
    }

    // Trigger exec out
    TriggerExecutionPin(0);
}

void SetVariableNode::SaveStream(Stream& stream)
{
    GraphNode::SaveStream(stream);
    stream.WriteString(mVariableName);
    stream.WriteUint8((uint8_t)mCachedType);
}

void SetVariableNode::LoadStream(Stream& stream, uint32_t version)
{
    // Override base LoadStream entirely because we have dynamic pins.
    mId = stream.ReadUint32();
    mEditorPosition = stream.ReadVec2();

    // Read base input pins — pre-create pins as needed
    uint32_t numInputPins = stream.ReadUint32();
    for (uint32_t i = 0; i < numInputPins; ++i)
    {
        GraphPinId pinId = stream.ReadUint32();
        Datum defaultValue = ReadDatumFromStream(stream);

        if (i >= mInputPins.size())
        {
            AddInputPin("", DatumType::Float, Datum(0.0f));
        }
        mInputPins[i].mId = pinId;
        mInputPins[i].mOwnerNodeId = mId;
        mInputPins[i].mDefaultValue = defaultValue;
        mInputPins[i].mValue = defaultValue;
    }

    // Read base output pins — pre-create pins as needed
    uint32_t numOutputPins = stream.ReadUint32();
    for (uint32_t i = 0; i < numOutputPins; ++i)
    {
        GraphPinId pinId = stream.ReadUint32();

        if (i >= mOutputPins.size())
        {
            AddOutputPin("", DatumType::Float);
        }
        mOutputPins[i].mId = pinId;
        mOutputPins[i].mOwnerNodeId = mId;
    }

    mNextPinId = stream.ReadUint32();

    // Read variable-specific data
    if (version >= ASSET_VERSION_NODE_GRAPH_VARIABLES)
    {
        stream.ReadString(mVariableName);
        mCachedType = (DatumType)stream.ReadUint8();

        // Apply types to pins
        if (mInputPins.size() >= 1)
        {
            mInputPins[0].mName = "    ";
            mInputPins[0].mDataType = DatumType::Execution;
        }
        if (mInputPins.size() >= 2)
        {
            mInputPins[1].mName = mVariableName;
            mInputPins[1].mDataType = mCachedType;
        }
        if (mOutputPins.size() >= 1)
        {
            mOutputPins[0].mName = "    ";
            mOutputPins[0].mDataType = DatumType::Execution;
        }
        if (mOutputPins.size() >= 2)
        {
            mOutputPins[1].mName = mVariableName;
            mOutputPins[1].mDataType = mCachedType;
        }
    }
}

const char* SetVariableNode::GetNodeTypeName() const
{
    if (mVariableName.empty())
        return "Set Variable";

    mDisplayName = "Set: " + mVariableName;
    return mDisplayName.c_str();
}

void SetVariableNode::CopyCustomData(const GraphNode* src)
{
    if (src->GetType() != SetVariableNode::GetStaticType())
        return;

    const SetVariableNode* srcVar = static_cast<const SetVariableNode*>(src);
    mVariableName = srcVar->mVariableName;
    mOwnerAsset = srcVar->mOwnerAsset;
    mCachedType = srcVar->mCachedType;

    // Recreate dynamic pins to match the source
    mInputPins.clear();
    mOutputPins.clear();
    mNextPinId = 1;

    for (uint32_t i = 0; i < src->GetNumInputPins(); ++i)
    {
        const GraphPin& srcPin = src->GetInputPins()[i];
        GraphPin& newPin = AddInputPin(srcPin.mName.c_str(), srcPin.mDataType, srcPin.mDefaultValue);
        newPin.mId = srcPin.mId;
        newPin.mOwnerNodeId = src->GetId();
        newPin.mValue = srcPin.mValue;
    }

    for (uint32_t i = 0; i < src->GetNumOutputPins(); ++i)
    {
        const GraphPin& srcPin = src->GetOutputPins()[i];
        GraphPin& newPin = AddOutputPin(srcPin.mName.c_str(), srcPin.mDataType);
        newPin.mId = srcPin.mId;
        newPin.mOwnerNodeId = src->GetId();
        newPin.mValue = srcPin.mValue;
    }
}

void SetVariableNode::RebuildPinsFromVariable(NodeGraphAsset* asset)
{
    if (asset == nullptr)
        return;

    mOwnerAsset = asset;

    int32_t idx = asset->FindVariableIndex(mVariableName);
    if (idx < 0)
        return;

    const GraphVariable* var = asset->GetVariable((uint32_t)idx);
    if (var == nullptr)
        return;

    mCachedType = var->mType;

    // Clear existing pins
    mInputPins.clear();
    mOutputPins.clear();
    mNextPinId = 1;

    Datum defaultValue;
    switch (var->mType)
    {
    case DatumType::Integer:  defaultValue = Datum(0); break;
    case DatumType::Float:    defaultValue = Datum(0.0f); break;
    case DatumType::Bool:     defaultValue = Datum(false); break;
    case DatumType::String:   defaultValue = Datum(std::string("")); break;
    case DatumType::Vector2D: defaultValue = Datum(glm::vec2(0.0f)); break;
    case DatumType::Vector:   defaultValue = Datum(glm::vec3(0.0f)); break;
    case DatumType::Color:    defaultValue = Datum(glm::vec4(0.0f)); break;
    default: break;
    }

    // Input pins: Exec In, Value
    AddInputPin("    ", DatumType::Execution);
    AddInputPin(var->mName.c_str(), var->mType, defaultValue);

    // Output pins: Exec Out, Value pass-through
    AddOutputPin("    ", DatumType::Execution);
    AddOutputPin(var->mName.c_str(), var->mType);

    // Reassign pin IDs using graph-global counter to avoid collisions
    if (mGraph != nullptr)
    {
        for (uint32_t i = 0; i < mInputPins.size(); ++i)
        {
            mInputPins[i].mId = mGraph->AllocPinId();
            mInputPins[i].mOwnerNodeId = mId;
        }
        for (uint32_t i = 0; i < mOutputPins.size(); ++i)
        {
            mOutputPins[i].mId = mGraph->AllocPinId();
            mOutputPins[i].mOwnerNodeId = mId;
        }
    }
}
