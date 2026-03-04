#pragma once

#include "NodeGraph/GraphDomain.h"

class MaterialDomain : public GraphDomain
{
public:

    virtual const char* GetDomainName() const override { return "Material"; }
    virtual void RegisterNodeTypes() override;
    virtual void OnGraphEvaluated(NodeGraph* graph) override;
    virtual TypeId GetDefaultOutputNodeType() const override;
};
