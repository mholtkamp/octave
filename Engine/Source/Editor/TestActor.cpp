#if 1 //EDITOR

#include "TestActor.h"

#include "Nodes/3D/StaticMesh3d.h"
#include "Nodes/3D/PointLight3d.h"
#include "Assets/Texture.h"
#include "Assets/StaticMesh.h"
#include "Clock.h"
#include "Engine.h"
#include "AssetManager.h"
#include "Log.h"
#include "Nodes/Widgets/Widget.h"

DEFINE_NODE(TestActor, Node3D);

void TestActor::Create()
{
    StaticMesh3D::Create();

    SetName("TestActor");

    mLeftMesh1 = CreateChild<StaticMesh3D>("Left Mesh 1");
    mRightMesh1 = CreateChild<StaticMesh3D>("Right Mesh 1");
    mRightMesh2 = mRightMesh1->CreateChild<StaticMesh3D>("Right Mesh 2");
    mLeftLight1 = mLeftSpin1->CreateChild<PointLight3D>("Left Light 1");
    mLeftLight2 = mLeftSpin1->CreateChild<PointLight3D>("Left Light 2");
    mRightLight1 = mRightMesh1->CreateChild<PointLight3D>("Right Light 1");
    mLeftSpin1 = mLeftMesh1->CreateChild<Node3D>("Left Spin Root");

    mLeftMesh1->SetPosition(glm::vec3(-5, 0, 0));
    mRightMesh1->SetPosition(glm::vec3(5, 0, 0));
    mRightMesh2->SetPosition(glm::vec3(0, 5, 0));
    mLeftLight1->SetPosition(glm::vec3(-2, 0, 0));
    mLeftLight2->SetPosition(glm::vec3(2, 0, 0));
    mRightLight1->SetPosition(glm::vec3(0, -2.5f, 0));

    StaticMeshRef ConeMesh = LoadAsset("SM_Cone");
    StaticMeshRef CubeMesh = LoadAsset("SM_Cube");

    SetStaticMesh(ConeMesh.Get<StaticMesh>());
    mLeftMesh1->SetStaticMesh(CubeMesh.Get<StaticMesh>());
    mRightMesh1->SetStaticMesh(CubeMesh.Get<StaticMesh>());
    mRightMesh2->SetStaticMesh(ConeMesh.Get<StaticMesh>());

    mLeftLight1->SetColor(glm::vec4(0.5f, 1.0f, 0.5f, 1.0f));
    mLeftLight2->SetColor(glm::vec4(1.0f, 1.0f, 0.5f, 1.0f));
    mRightLight1->SetColor(glm::vec4(0.5f, 1.0f, 1.0f, 1.0f));

    mLeftLight1->SetRadius(15.0f);
    mLeftLight2->SetRadius(10.0f);
    mRightLight1->SetRadius(10.0f);


    // Print out all Actor factories.
    {
        LogDebug("----Node Types----");
        std::vector<Factory*>& factoryList = Node::GetFactoryList();
        
        for (uint32_t i = 0; i < factoryList.size(); ++i)
        {
            LogDebug("NodeClass[%d] %s - %lu", i, factoryList[i]->GetClassName(), factoryList[i]->GetType());
        }
    }

    {
        LogDebug("----Asset Types----");
        std::vector<Factory*>& factoryList = Asset::GetFactoryList();
        
        for (uint32_t i = 0; i < factoryList.size(); ++i)
        {
            LogDebug("AssetClass[%d] %s - %lu", i, factoryList[i]->GetClassName(), factoryList[i]->GetType());
        }
    }

    //SetScriptFile("Engine/Scripts/Test.lua");

    // Test actor only exists in the EDITOR builds. Don't allow saving it to a level.
    SetTransient(true);
}

void TestActor::Destroy()
{
    StaticMesh3D::Destroy();
}

void TestActor::Tick(float deltaTime)
{
    StaticMesh3D::Tick(deltaTime);

    static bool rotate = true;
    static bool translate = false;
    static bool scale = false;

    if (rotate)
    {
        glm::vec3 rootRot = GetRotationEuler();
        rootRot.y += 30 * deltaTime;
        SetRotation(rootRot);

        glm::vec3 right2Rot = mRightMesh2->GetRotationEuler();
        right2Rot.y += 470 * deltaTime;
        mRightMesh2->SetRotation(right2Rot);

        glm::vec3 right1Rot = mRightMesh1->GetRotationEuler();
        right1Rot.x += 40 * deltaTime;
        mRightMesh1->SetRotation(right1Rot);

        glm::vec3 left1Rot = mLeftMesh1->GetRotationEuler();
        left1Rot.y += 20 * deltaTime;
        mLeftMesh1->SetRotation(left1Rot);

        glm::vec3 leftSpin1Rot = mLeftSpin1->GetRotationEuler();
        leftSpin1Rot.y += 200 * deltaTime;
        mLeftSpin1->SetRotation(leftSpin1Rot);
    }

    if (translate)
    {
        glm::vec3 rootPos = GetPosition();
        rootPos.z = 20.0f * sinf(0.5f * GetAppClock()->GetTime());
        SetPosition(rootPos);

        glm::vec3 right2Pos = mRightMesh2->GetPosition();
        right2Pos.y = 5 + 2.5f * sinf(4.0f * GetAppClock()->GetTime());
        mRightMesh2->SetPosition(right2Pos);
    }

    if (scale)
    {
        float newScale = 2 + 1 * sinf(2 * GetAppClock()->GetTime());
        SetScale(glm::vec3(newScale, newScale, newScale));

        float right2Scale = 1 + 2 * fabs(sinf(5 * GetAppClock()->GetTime()));
        mRightMesh2->SetScale(glm::vec3(right2Scale, right2Scale, right2Scale));
    }
}

void TestActor::EditorTick(float deltaTime)
{
    Tick(deltaTime);
}

#endif
