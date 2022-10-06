#pragma once

#include "Actor.h"

class StaticMeshComponent;
class PointLightComponent;
class TransformComponent;
class ScriptComponent;
class Texture;
class StaticMesh;

class TestActor : public Actor
{
public:

    TestActor();
    virtual void Create() override;
    virtual void Destroy() override;
    virtual void Tick(float deltaTime) override;
    virtual void EditorTick(float deltaTime) override;

    DECLARE_ACTOR(TestActor, Actor);

protected:

    StaticMeshComponent* mRootMesh;
    StaticMeshComponent* mLeftMesh1;
    StaticMeshComponent* mRightMesh1;
    StaticMeshComponent* mRightMesh2;
    PointLightComponent* mLeftLight1;
    PointLightComponent* mLeftLight2;
    PointLightComponent* mRightLight1;
    TransformComponent* mLeftSpin1;
    ScriptComponent* mScript;
};
