#include "Assets/NodeGraphAsset.h"
#include "NodeGraph/GraphDomainManager.h"
#include "NodeGraph/GraphDomain.h"
#include "NodeGraph/GraphNode.h"
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
}

void NodeGraphAsset::SaveStream(Stream& stream, Platform platform)
{
    Asset::SaveStream(stream, platform);
    mGraph.SaveStream(stream);
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

glm::vec4 NodeGraphAsset::GetTypeColor()
{
    return glm::vec4(0.6f, 0.3f, 0.8f, 1.0f);
}

const char* NodeGraphAsset::GetTypeName()
{
    return "Node Graph";
}
