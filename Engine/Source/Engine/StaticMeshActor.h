#pragma once

#include "Actor.h"
#include "Components/StaticMeshComponent.h"

class StaticMeshComponent;

class StaticMeshActor : public Actor
{
public:

    DECLARE_ACTOR(StaticMeshActor, Actor);

    StaticMeshActor();
    virtual void Create() override;
    StaticMeshComponent* GetStaticMeshComponent();

protected:

    StaticMeshComponent* mStaticMeshComponent;

#if EDITOR
public:
    void InitFromAiNode(const aiNode& node, std::vector<StaticMesh>& meshes);
#endif
};