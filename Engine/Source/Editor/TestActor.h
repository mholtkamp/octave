#pragma once

#include "Nodes/Node.h"
#include "Nodes/3D/StaticMesh3D.h"

class StaticMesh3D;
class PointLight3D;
class Node3D;
class ScriptComponent;
class Texture;
class StaticMesh;

class TestActor : public StaticMesh3D
{
public:

    DECLARE_NODE(TestActor, StaticMesh3D);

    virtual void Create() override;
    virtual void Destroy() override;
    virtual void Tick(float deltaTime) override;
    virtual void EditorTick(float deltaTime) override;

protected:

    StaticMesh3D* mLeftMesh1 = nullptr;
    StaticMesh3D* mRightMesh1 = nullptr;
    StaticMesh3D* mRightMesh2 = nullptr;
    PointLight3D* mLeftLight1 = nullptr;
    PointLight3D* mLeftLight2 = nullptr;
    PointLight3D* mRightLight1 = nullptr;
    Node3D* mLeftSpin1 = nullptr;
};
