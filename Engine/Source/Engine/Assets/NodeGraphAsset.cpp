#include "Assets/NodeGraphAsset.h"
#include "NodeGraph/GraphDomainManager.h"
#include "NodeGraph/GraphDomain.h"
#include "NodeGraph/GraphNode.h"
#include "NodeGraph/Nodes/FunctionNodes.h"
#include "NodeGraph/Nodes/VariableNodes.h"
#include "Log.h"
#include "Property.h"

FORCE_LINK_DEF(NodeGraphAsset);
DEFINE_ASSET(NodeGraphAsset);

NodeGraphAsset::NodeGraphAsset()
{
    mType = NodeGraphAsset::GetStaticType();
}

NodeGraphAsset::~NodeGraphAsset()
{
}

void NodeGraphAsset::LoadStream(Stream& stream, Platform platform)
{
    Asset::LoadStream(stream, platform);
    mGraph.LoadStream(stream, mVersion);

    // Load function graphs
    if (mVersion >= ASSET_VERSION_NODE_GRAPH_FUNCTIONS)
    {
        uint32_t numFuncs = stream.ReadUint32();
        for (uint32_t i = 0; i < numFuncs; ++i)
        {
            NodeGraph* fg = new NodeGraph();
            fg->LoadStream(stream, mVersion);
            mFunctionGraphs.push_back(fg);
        }
    }

    // Resolve mDomainIndex from the loaded domain name
    GraphDomainManager* mgr = GraphDomainManager::Get();
    if (mgr != nullptr)
    {
        const std::vector<GraphDomain*>& domains = mgr->GetDomains();
        for (uint32_t i = 0; i < domains.size(); ++i)
        {
            if (mGraph.GetDomainName() == domains[i]->GetDomainName())
            {
                mDomainIndex = (int32_t)i;
                break;
            }
        }
    }

    // Load variables
    if (mVersion >= ASSET_VERSION_NODE_GRAPH_VARIABLES)
    {
        uint32_t numVars = stream.ReadUint32();
        for (uint32_t i = 0; i < numVars; ++i)
        {
            GraphVariable var;
            stream.ReadString(var.mName);
            var.mType = (DatumType)stream.ReadUint8();
            var.mDefaultValue = GraphNode::ReadDatumFromStream(stream);
            var.mRuntimeValue = var.mDefaultValue;
            mVariables.push_back(var);
        }
    }

    // Wire FunctionCallNodes in all graphs to this asset
    ResolveFunctionCallNodes();
    ResolveVariableNodes();
}

void NodeGraphAsset::SaveStream(Stream& stream, Platform platform)
{
    Asset::SaveStream(stream, platform);
    mGraph.SaveStream(stream);

    // Save function graphs
    stream.WriteUint32((uint32_t)mFunctionGraphs.size());
    for (auto* fg : mFunctionGraphs)
    {
        fg->SaveStream(stream);
    }

    // Save variables
    stream.WriteUint32((uint32_t)mVariables.size());
    for (const auto& var : mVariables)
    {
        stream.WriteString(var.mName);
        stream.WriteUint8((uint8_t)var.mType);
        GraphNode::WriteDatumToStream(stream, var.mDefaultValue);
    }
}

void NodeGraphAsset::Create()
{
    Asset::Create();

    // If the graph was already loaded from a stream, don't add a duplicate output node.
    if (mGraph.GetNumNodes() > 0)
    {
        return;
    }

    GraphDomainManager* mgr = GraphDomainManager::Get();
    if (mgr != nullptr)
    {
        const std::vector<GraphDomain*>& domains = mgr->GetDomains();
        if (mDomainIndex >= 0 && mDomainIndex < (int32_t)domains.size())
        {
            GraphDomain* domain = domains[mDomainIndex];
            mGraph.SetDomainName(domain->GetDomainName());

            TypeId outputType = domain->GetDefaultOutputNodeType();
            if (outputType != 0)
            {
                GraphNode* outputNode = mGraph.AddNode(outputType);
                if (outputNode != nullptr)
                {
                    outputNode->SetEditorPosition(glm::vec2(400.0f, 200.0f));
                }
            }
        }
        else
        {
            // Fallback to first domain
            if (!domains.empty())
            {
                mGraph.SetDomainName(domains[0]->GetDomainName());
            }
        }
    }
}

void NodeGraphAsset::Destroy()
{
    mGraph.Clear();

    for (auto* fg : mFunctionGraphs)
    {
        delete fg;
    }
    mFunctionGraphs.clear();
    mVariables.clear();

    Asset::Destroy();
}

bool NodeGraphAsset::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);
    OCT_ASSERT(prop != nullptr);

    NodeGraphAsset* asset = static_cast<NodeGraphAsset*>(prop->mOwner);
    bool success = false;

    if (prop->mName == "Graph Type" && newValue != nullptr)
    {
        int32_t newIndex = *static_cast<const int32_t*>(newValue);
        GraphDomainManager* mgr = GraphDomainManager::Get();

        if (mgr != nullptr)
        {
            const std::vector<GraphDomain*>& domains = mgr->GetDomains();
            if (newIndex >= 0 && newIndex < (int32_t)domains.size())
            {
                asset->mDomainIndex = newIndex;
                asset->mGraph.Clear();

                // Clear function graphs and variables on domain change
                for (auto* fg : asset->mFunctionGraphs)
                {
                    delete fg;
                }
                asset->mFunctionGraphs.clear();
                asset->mVariables.clear();

                GraphDomain* domain = domains[newIndex];
                asset->mGraph.SetDomainName(domain->GetDomainName());

                TypeId outputType = domain->GetDefaultOutputNodeType();
                if (outputType != 0)
                {
                    GraphNode* outputNode = asset->mGraph.AddNode(outputType);
                    if (outputNode != nullptr)
                    {
                        outputNode->SetEditorPosition(glm::vec2(400.0f, 200.0f));
                    }
                }

                success = true;
            }
        }
    }

    return success;
}

void NodeGraphAsset::GatherProperties(std::vector<Property>& outProps)
{
    Asset::GatherProperties(outProps);

    GraphDomainManager* mgr = GraphDomainManager::Get();
    if (mgr != nullptr)
    {
        const std::vector<GraphDomain*>& domains = mgr->GetDomains();
        int32_t count = (int32_t)domains.size();

        static const char* sDomainNames[32] = {};
        for (int32_t i = 0; i < count && i < 32; ++i)
        {
            sDomainNames[i] = domains[i]->GetDomainName();
        }

        outProps.push_back(Property(DatumType::Integer, "Graph Type", this, &mDomainIndex, 1, HandlePropChange, NULL_DATUM, count, sDomainNames));
    }
}

NodeGraph* NodeGraphAsset::AddFunctionGraph(const std::string& name)
{
    // Check for duplicate name
    if (FindFunctionGraph(name) != nullptr)
    {
        LogWarning("NodeGraphAsset::AddFunctionGraph - Function '%s' already exists", name.c_str());
        return nullptr;
    }

    NodeGraph* fg = new NodeGraph();
    fg->SetDomainName(mGraph.GetDomainName());
    fg->SetGraphName(name);
    mFunctionGraphs.push_back(fg);
    return fg;
}

void NodeGraphAsset::RemoveFunctionGraph(uint32_t index)
{
    if (index < mFunctionGraphs.size())
    {
        delete mFunctionGraphs[index];
        mFunctionGraphs.erase(mFunctionGraphs.begin() + index);
    }
}

NodeGraph* NodeGraphAsset::FindFunctionGraph(const std::string& name) const
{
    for (auto* fg : mFunctionGraphs)
    {
        if (fg->GetGraphName() == name)
        {
            return fg;
        }
    }
    return nullptr;
}

NodeGraph* NodeGraphAsset::GetFunctionGraph(uint32_t index)
{
    if (index < mFunctionGraphs.size())
    {
        return mFunctionGraphs[index];
    }
    return nullptr;
}

void NodeGraphAsset::RenameFunctionGraph(uint32_t index, const std::string& newName)
{
    if (index >= mFunctionGraphs.size())
        return;

    // Check for duplicate name
    for (uint32_t i = 0; i < mFunctionGraphs.size(); ++i)
    {
        if (i != index && mFunctionGraphs[i]->GetGraphName() == newName)
        {
            LogWarning("NodeGraphAsset::RenameFunctionGraph - Name '%s' already in use", newName.c_str());
            return;
        }
    }

    std::string oldName = mFunctionGraphs[index]->GetGraphName();
    mFunctionGraphs[index]->SetGraphName(newName);

    // Update FunctionCallNodes referencing the old name in all graphs
    auto updateCallNodes = [&](NodeGraph* graph)
    {
        for (GraphNode* node : graph->GetNodes())
        {
            if (node->GetType() == FunctionCallNode::GetStaticType())
            {
                FunctionCallNode* callNode = static_cast<FunctionCallNode*>(node);
                if (callNode->GetFunctionName() == oldName)
                {
                    callNode->SetFunctionName(newName);
                }
            }
        }
    };

    updateCallNodes(&mGraph);
    for (auto* fg : mFunctionGraphs)
    {
        updateCallNodes(fg);
    }
}

void NodeGraphAsset::ResolveFunctionCallNodes()
{
    auto wireGraph = [this](NodeGraph* graph)
    {
        for (GraphNode* node : graph->GetNodes())
        {
            if (node->GetType() == FunctionCallNode::GetStaticType())
            {
                static_cast<FunctionCallNode*>(node)->SetOwnerAsset(this);
            }
        }
    };

    wireGraph(&mGraph);
    for (auto* fg : mFunctionGraphs)
    {
        wireGraph(fg);
    }
}

glm::vec4 NodeGraphAsset::GetTypeColor()
{
    return glm::vec4(0.6f, 0.3f, 0.8f, 1.0f);
}

const char* NodeGraphAsset::GetTypeName()
{
    return "Node Graph";
}

int32_t NodeGraphAsset::AddVariable(const std::string& name, DatumType type)
{
    // Check for duplicate name
    if (FindVariableIndex(name) >= 0)
    {
        LogWarning("NodeGraphAsset::AddVariable - Variable '%s' already exists", name.c_str());
        return -1;
    }

    GraphVariable var;
    var.mName = name;
    var.mType = type;

    switch (type)
    {
    case DatumType::Integer:  var.mDefaultValue = Datum((int32_t)0); break;
    case DatumType::Float:    var.mDefaultValue = Datum(0.0f); break;
    case DatumType::Bool:     var.mDefaultValue = Datum(false); break;
    case DatumType::String:   var.mDefaultValue = Datum(std::string("")); break;
    case DatumType::Vector2D: var.mDefaultValue = Datum(glm::vec2(0.0f)); break;
    case DatumType::Vector:   var.mDefaultValue = Datum(glm::vec3(0.0f)); break;
    case DatumType::Color:    var.mDefaultValue = Datum(glm::vec4(1.0f)); break;
    default: var.mDefaultValue = Datum(0.0f); break;
    }

    var.mRuntimeValue = var.mDefaultValue;
    mVariables.push_back(var);
    return (int32_t)(mVariables.size() - 1);
}

void NodeGraphAsset::RemoveVariable(uint32_t index)
{
    if (index < mVariables.size())
    {
        mVariables.erase(mVariables.begin() + index);
    }
}

void NodeGraphAsset::RenameVariable(uint32_t index, const std::string& newName)
{
    if (index >= mVariables.size())
        return;

    // Check for duplicate name
    for (uint32_t i = 0; i < mVariables.size(); ++i)
    {
        if (i != index && mVariables[i].mName == newName)
        {
            LogWarning("NodeGraphAsset::RenameVariable - Name '%s' already in use", newName.c_str());
            return;
        }
    }

    std::string oldName = mVariables[index].mName;
    mVariables[index].mName = newName;

    // Update GetVariableNode/SetVariableNode instances referencing the old name
    auto updateVarNodes = [&](NodeGraph* graph)
    {
        for (GraphNode* node : graph->GetNodes())
        {
            if (node->GetType() == GetVariableNode::GetStaticType())
            {
                GetVariableNode* varNode = static_cast<GetVariableNode*>(node);
                if (varNode->GetVariableName() == oldName)
                {
                    varNode->SetVariableName(newName);
                }
            }
            else if (node->GetType() == SetVariableNode::GetStaticType())
            {
                SetVariableNode* varNode = static_cast<SetVariableNode*>(node);
                if (varNode->GetVariableName() == oldName)
                {
                    varNode->SetVariableName(newName);
                }
            }
        }
    };

    updateVarNodes(&mGraph);
    for (auto* fg : mFunctionGraphs)
    {
        updateVarNodes(fg);
    }
}

int32_t NodeGraphAsset::FindVariableIndex(const std::string& name) const
{
    for (uint32_t i = 0; i < mVariables.size(); ++i)
    {
        if (mVariables[i].mName == name)
        {
            return (int32_t)i;
        }
    }
    return -1;
}

GraphVariable* NodeGraphAsset::GetVariable(uint32_t index)
{
    if (index < mVariables.size())
    {
        return &mVariables[index];
    }
    return nullptr;
}

const GraphVariable* NodeGraphAsset::GetVariable(uint32_t index) const
{
    if (index < mVariables.size())
    {
        return &mVariables[index];
    }
    return nullptr;
}

void NodeGraphAsset::ResetVariablesToDefaults()
{
    for (auto& var : mVariables)
    {
        var.mRuntimeValue = var.mDefaultValue;
    }
}

void NodeGraphAsset::ResolveVariableNodes()
{
    auto wireGraph = [this](NodeGraph* graph)
    {
        for (GraphNode* node : graph->GetNodes())
        {
            if (node->GetType() == GetVariableNode::GetStaticType())
            {
                static_cast<GetVariableNode*>(node)->SetOwnerAsset(this);
            }
            else if (node->GetType() == SetVariableNode::GetStaticType())
            {
                static_cast<SetVariableNode*>(node)->SetOwnerAsset(this);
            }
        }
    };

    wireGraph(&mGraph);
    for (auto* fg : mFunctionGraphs)
    {
        wireGraph(fg);
    }
}
