#include "NodeGraph/Nodes/FunctionNodes.h"
#include "NodeGraph/Nodes/InputNodes.h"
#include "NodeGraph/Nodes/VariableNodes.h"
#include "NodeGraph/NodeGraph.h"
#include "NodeGraph/GraphProcessor.h"
#include "Assets/NodeGraphAsset.h"
#include "Stream.h"
#include "Log.h"

FORCE_LINK_DEF(FunctionNodes);

// FunctionOutputNode: registered with all domains so it appears in function graphs
DEFINE_GRAPH_NODE(FunctionOutputNode);
REGISTER_GRAPH_NODE_MULTI(FunctionOutputNode, "Function Output", "Function",
    glm::vec4(0.2f, 0.7f, 0.4f, 1.0f),
    "Material", "Shader", "Procedural", "Animation", "FSM", "SceneGraph");

// FunctionCallNode: only DEFINE (for deserialization), NOT registered with any domain.
// Created only via sidebar drag, context menu, or subgraph extraction.
DEFINE_GRAPH_NODE(FunctionCallNode);

// =============================================================================
// FunctionOutputNode
// =============================================================================
void FunctionOutputNode::SetupPins()
{
    // Starts with zero pins — user adds them dynamically
}

void FunctionOutputNode::Evaluate()
{
    // Sink node — values are read from input pins directly after evaluation
}

void FunctionOutputNode::SaveStream(Stream& stream)
{
    GraphNode::SaveStream(stream);

    // Write dynamic pin layout so we can reconstruct on load
    stream.WriteUint32((uint32_t)mInputPins.size());
    for (uint32_t i = 0; i < mInputPins.size(); ++i)
    {
        stream.WriteString(mInputPins[i].mName);
        stream.WriteUint8((uint8_t)mInputPins[i].mDataType);
    }
}

void FunctionOutputNode::LoadStream(Stream& stream, uint32_t version)
{
    // Override base LoadStream entirely because we have dynamic pins.
    // SetupPins() creates 0 pins, so we must pre-create them before reading IDs.
    mId = stream.ReadUint32();
    mEditorPosition = stream.ReadVec2();

    uint32_t numInputPins = stream.ReadUint32();
    for (uint32_t i = 0; i < numInputPins; ++i)
    {
        GraphPinId pinId = stream.ReadUint32();
        Datum defaultValue = ReadDatumFromStream(stream);

        // Create pin if it doesn't exist yet
        if (i >= mInputPins.size())
        {
            AddInputPin("", DatumType::Float, Datum(0.0f));
        }
        mInputPins[i].mId = pinId;
        mInputPins[i].mOwnerNodeId = mId;
        mInputPins[i].mDefaultValue = defaultValue;
        mInputPins[i].mValue = defaultValue;
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

    // Read the dynamic pin layout (names + types)
    if (version >= ASSET_VERSION_NODE_GRAPH_FUNCTIONS)
    {
        uint32_t numDynPins = stream.ReadUint32();
        for (uint32_t i = 0; i < numDynPins; ++i)
        {
            std::string name;
            stream.ReadString(name);
            DatumType type = (DatumType)stream.ReadUint8();

            if (i < mInputPins.size())
            {
                mInputPins[i].mName = name;
                mInputPins[i].mDataType = type;
            }
        }
    }
}

void FunctionOutputNode::CopyCustomData(const GraphNode* src)
{
    if (src->GetType() != FunctionOutputNode::GetStaticType())
        return;

    // Recreate dynamic pins to match the source
    mInputPins.clear();
    mNextPinId = 1;

    for (uint32_t i = 0; i < src->GetNumInputPins(); ++i)
    {
        const GraphPin& srcPin = src->GetInputPins()[i];
        GraphPin& newPin = AddInputPin(srcPin.mName.c_str(), srcPin.mDataType, srcPin.mDefaultValue);
        newPin.mId = srcPin.mId;
        newPin.mOwnerNodeId = src->GetId();
        newPin.mValue = srcPin.mValue;
    }
}

void FunctionOutputNode::AddOutputField(const std::string& name, DatumType type)
{
    Datum defaultValue;
    switch (type)
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

    GraphPin& pin = AddInputPin(name.c_str(), type, defaultValue);

    // Re-assign pin owner ID if the node is already in a graph
    pin.mOwnerNodeId = mId;

    // Update pin ID to use graph-global counter if in a graph
    if (mGraph != nullptr)
    {
        // We need to get a globally unique ID — use the graph's AddNode approach.
        // Since we're adding pins after initial setup, we use the node's internal counter.
        // The graph will reconcile mNextPinId on save/load.
    }
}

void FunctionOutputNode::RemoveOutputField(uint32_t index)
{
    if (index >= mInputPins.size())
        return;

    GraphPinId pinId = mInputPins[index].mId;

    // Remove any links connected to this pin
    if (mGraph != nullptr)
    {
        const std::vector<GraphLink>& links = mGraph->GetLinks();
        for (int32_t i = (int32_t)links.size() - 1; i >= 0; --i)
        {
            if (links[i].mInputPinId == pinId)
            {
                mGraph->RemoveLink(links[i].mId);
            }
        }
    }

    mInputPins.erase(mInputPins.begin() + index);
}

// =============================================================================
// FunctionCallNode
// =============================================================================
void FunctionCallNode::SetupPins()
{
    // Starts with zero pins — rebuilt from function signature
}

void FunctionCallNode::Evaluate()
{
    if (mOwnerAsset == nullptr)
        return;

    NodeGraph* funcGraph = mOwnerAsset->FindFunctionGraph(mFunctionName);
    if (funcGraph == nullptr)
        return;

    // Create temp copy of the function graph
    NodeGraph tempGraph;
    tempGraph.CopyFrom(*funcGraph);

    // Wire variable nodes in temp graph to same owner asset
    for (GraphNode* node : tempGraph.GetNodes())
    {
        if (node->GetType() == GetVariableNode::GetStaticType())
        {
            static_cast<GetVariableNode*>(node)->SetOwnerAsset(mOwnerAsset);
        }
        else if (node->GetType() == SetVariableNode::GetStaticType())
        {
            static_cast<SetVariableNode*>(node)->SetOwnerAsset(mOwnerAsset);
        }
    }

    // Map FunctionCallNode input pin values → function graph's InputNode values
    uint32_t inputIdx = 0;
    for (GraphNode* node : tempGraph.GetNodes())
    {
        if (node->IsInputNode())
        {
            if (inputIdx < mInputPins.size())
            {
                // Set the input node's value from our input pin
                if (node->GetNumInputPins() > 0)
                {
                    node->GetInputPins()[0].mDefaultValue = mInputPins[inputIdx].mValue;
                    node->GetInputPins()[0].mValue = mInputPins[inputIdx].mValue;
                }
            }
            ++inputIdx;
        }
    }

    // Evaluate the temp graph
    GraphProcessor processor;
    processor.Evaluate(&tempGraph);

    // Read FunctionOutputNode input pin values → set our output pin values
    for (GraphNode* node : tempGraph.GetNodes())
    {
        if (node->GetType() == FunctionOutputNode::GetStaticType())
        {
            for (uint32_t i = 0; i < node->GetNumInputPins() && i < mOutputPins.size(); ++i)
            {
                SetOutputValue(i, node->GetInputValue(i));
            }
            break;
        }
    }
}

void FunctionCallNode::SaveStream(Stream& stream)
{
    GraphNode::SaveStream(stream);
    stream.WriteString(mFunctionName);

    // Write redundant pin layout (names + types) so we can reconstruct without the function graph
    stream.WriteUint32((uint32_t)mInputPins.size());
    for (uint32_t i = 0; i < mInputPins.size(); ++i)
    {
        stream.WriteString(mInputPins[i].mName);
        stream.WriteUint8((uint8_t)mInputPins[i].mDataType);
        WriteDatumToStream(stream, mInputPins[i].mDefaultValue);
    }

    stream.WriteUint32((uint32_t)mOutputPins.size());
    for (uint32_t i = 0; i < mOutputPins.size(); ++i)
    {
        stream.WriteString(mOutputPins[i].mName);
        stream.WriteUint8((uint8_t)mOutputPins[i].mDataType);
    }
}

void FunctionCallNode::LoadStream(Stream& stream, uint32_t version)
{
    // Override base LoadStream entirely because we have dynamic pins.
    mId = stream.ReadUint32();
    mEditorPosition = stream.ReadVec2();

    // Read base input pins — pre-create pins as needed
    uint32_t numBaseInputPins = stream.ReadUint32();
    for (uint32_t i = 0; i < numBaseInputPins; ++i)
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
    uint32_t numBaseOutputPins = stream.ReadUint32();
    for (uint32_t i = 0; i < numBaseOutputPins; ++i)
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

    if (version >= ASSET_VERSION_NODE_GRAPH_FUNCTIONS)
    {
        stream.ReadString(mFunctionName);

        // Read input pin layout (names + types + defaults)
        uint32_t numInputs = stream.ReadUint32();
        for (uint32_t i = 0; i < numInputs; ++i)
        {
            std::string name;
            stream.ReadString(name);
            DatumType type = (DatumType)stream.ReadUint8();
            Datum defaultValue = ReadDatumFromStream(stream);

            if (i < mInputPins.size())
            {
                mInputPins[i].mName = name;
                mInputPins[i].mDataType = type;
                mInputPins[i].mDefaultValue = defaultValue;
                mInputPins[i].mValue = defaultValue;
            }
        }

        // Read output pin layout (names + types)
        uint32_t numOutputs = stream.ReadUint32();
        for (uint32_t i = 0; i < numOutputs; ++i)
        {
            std::string name;
            stream.ReadString(name);
            DatumType type = (DatumType)stream.ReadUint8();

            if (i < mOutputPins.size())
            {
                mOutputPins[i].mName = name;
                mOutputPins[i].mDataType = type;
            }
        }
    }
}

const char* FunctionCallNode::GetNodeTypeName() const
{
    return mFunctionName.empty() ? "Function Call" : mFunctionName.c_str();
}

void FunctionCallNode::SetFunctionName(const std::string& name)
{
    mFunctionName = name;
}

void FunctionCallNode::CopyCustomData(const GraphNode* src)
{
    if (src->GetType() != FunctionCallNode::GetStaticType())
        return;

    const FunctionCallNode* srcCall = static_cast<const FunctionCallNode*>(src);
    mFunctionName = srcCall->mFunctionName;
    mOwnerAsset = srcCall->mOwnerAsset;

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

void FunctionCallNode::RebuildPinsFromFunction(NodeGraphAsset* asset)
{
    if (asset == nullptr)
        return;

    NodeGraph* funcGraph = asset->FindFunctionGraph(mFunctionName);
    if (funcGraph == nullptr)
        return;

    mOwnerAsset = asset;

    // Clear existing pins
    mInputPins.clear();
    mOutputPins.clear();
    mNextPinId = 1;

    // Build input pins from the function graph's InputNodes
    for (GraphNode* node : funcGraph->GetNodes())
    {
        if (node->IsInputNode())
        {
            const std::string& inputName = node->GetInputName();
            DatumType type = DatumType::Float;  // default

            if (node->GetNumOutputPins() > 0)
            {
                type = node->GetOutputPins()[0].mDataType;
            }

            Datum defaultValue;
            if (node->GetNumInputPins() > 0)
            {
                defaultValue = node->GetInputPins()[0].mDefaultValue;
            }

            AddInputPin(inputName.c_str(), type, defaultValue);
        }
    }

    // Build output pins from the FunctionOutputNode's input pins
    for (GraphNode* node : funcGraph->GetNodes())
    {
        if (node->GetType() == FunctionOutputNode::GetStaticType())
        {
            for (uint32_t i = 0; i < node->GetNumInputPins(); ++i)
            {
                const GraphPin& funcOutPin = node->GetInputPins()[i];
                AddOutputPin(funcOutPin.mName.c_str(), funcOutPin.mDataType);
            }
            break;
        }
    }

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
