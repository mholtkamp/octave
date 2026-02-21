#pragma once

#include "NodeGraph/GraphDomain.h"

class ShaderDomain : public GraphDomain
{
public:

    virtual const char* GetDomainName() const override { return "Shader"; }
    virtual void RegisterNodeTypes() override;
    virtual void OnGraphEvaluated(NodeGraph* graph) override;
    virtual TypeId GetDefaultOutputNodeType() const override;
};
