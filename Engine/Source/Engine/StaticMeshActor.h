#pragma once

#include "Nodes/Node.h"
#include "Nodes/3D/StaticMesh3d.h"

class StaticMesh3D;

class StaticMeshActor : public Actor
{
public:

    DECLARE_ACTOR(StaticMeshActor, Actor);

    StaticMeshActor();
    virtual void Create() override;
    StaticMesh3D* GetStaticMesh3D();

protected:

    StaticMesh3D* mStaticMesh3D;

#if EDITOR
public:
    void InitFromAiNode(const aiNode& node, std::vector<StaticMesh>& meshes);
#endif
};