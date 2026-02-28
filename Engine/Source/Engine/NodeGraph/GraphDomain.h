#pragma once

#include "EngineTypes.h"
#include "Maths.h"

#include <string>
#include <vector>

class NodeGraph;

struct GraphNodeTypeInfo
{
    TypeId mTypeId = 0;
    std::string mTypeName;
    std::string mCategory;
    glm::vec4 mColor = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
};

class GraphDomain
{
public:

    GraphDomain();
    virtual ~GraphDomain();

    virtual const char* GetDomainName() const = 0;
    virtual void RegisterNodeTypes() = 0;
    virtual void OnGraphEvaluated(NodeGraph* graph) {}
    virtual TypeId GetDefaultOutputNodeType() const { return 0; }

    void AddNodeType(TypeId typeId, const char* typeName, const char* category, const glm::vec4& color);
    const std::vector<GraphNodeTypeInfo>& GetNodeTypes() const { return mNodeTypes; }

protected:

    std::vector<GraphNodeTypeInfo> mNodeTypes;
};
