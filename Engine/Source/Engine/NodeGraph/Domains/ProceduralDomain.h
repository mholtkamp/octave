#pragma once

#include "NodeGraph/GraphDomain.h"

class ProceduralDomain : public GraphDomain
{
public:

    virtual const char* GetDomainName() const override { return "Procedural"; }
    virtual void RegisterNodeTypes() override;
    virtual void OnGraphEvaluated(NodeGraph* graph) override;
    virtual TypeId GetDefaultOutputNodeType() const override;
};
