#pragma once

#include "NodeGraph/GraphDomain.h"

class AnimationDomain : public GraphDomain
{
public:

    virtual const char* GetDomainName() const override { return "Animation"; }
    virtual void RegisterNodeTypes() override;
    virtual void OnGraphEvaluated(NodeGraph* graph) override;
    virtual TypeId GetDefaultOutputNodeType() const override;
};
