#include "NodeGraph/GraphDomain.h"

GraphDomain::GraphDomain()
{
}

GraphDomain::~GraphDomain()
{
}

void GraphDomain::AddNodeType(TypeId typeId, const char* typeName, const char* category, const glm::vec4& color)
{
    // Skip duplicates (can happen when both Domain::RegisterNodeTypes and REGISTER_GRAPH_NODE_MULTI register the same node)
    for (uint32_t i = 0; i < mNodeTypes.size(); ++i)
    {
        if (mNodeTypes[i].mTypeId == typeId)
        {
            return;
        }
    }

    GraphNodeTypeInfo info;
    info.mTypeId = typeId;
    info.mTypeName = typeName;
    info.mCategory = category;
    info.mColor = color;
    mNodeTypes.push_back(info);
}
