#pragma once

#include "Nodes/Node.h"
#include "Nodes/3D/StaticMeshComponent.h"

class StaticMesh3D;

class StaticMeshActor : public Actor
{
public:

    DECLARE_ACTOR(StaticMeshActor, Actor);

    StaticMeshActor();
    virtual void Create() override;
    StaticMesh3D* GetStaticMeshComponent();

protected:

    StaticMesh3D* mStaticMeshComponent;

#if EDITOR
public:
    void InitFromAiNode(const aiNode& node, std::vector<StaticMesh>& meshes);
#endif
};