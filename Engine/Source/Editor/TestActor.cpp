#if EDITOR

#include "TestActor.h"

#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/ScriptComponent.h"
#include "Assets/Texture.h"
#include "Assets/StaticMesh.h"
#include "Clock.h"
#include "Engine.h"
#include "AssetManager.h"
#include "Log.h"

TestActor::TestActor() :
    mRootMesh(nullptr),
    mLeftMesh1(nullptr),
    mRightMesh1(nullptr),
    mRightMesh2(nullptr),
    mLeftLight1(nullptr),
    mLeftLight2(nullptr),
    mRightLight1(nullptr)
{

}

void TestActor::Create()
{
    Actor::Create();

    SetName("TestActor");

    mRootMesh = CreateComponent<StaticMeshComponent>();
    mLeftMesh1 = CreateComponent<StaticMeshComponent>();
    mRightMesh1 = CreateComponent<StaticMeshComponent>();
    mRightMesh2 = CreateComponent<StaticMeshComponent>();
    mLeftLight1 = CreateComponent<PointLightComponent>();
    mLeftLight2 = CreateComponent<PointLightComponent>();
    mRightLight1 = CreateComponent<PointLightComponent>();
    mLeftSpin1 = CreateComponent<TransformComponent>();

    mRootMesh->SetName("Root Mesh");
    mLeftMesh1->SetName("Left Mesh 1");
    mRightMesh1->SetName("Right Mesh 1");
    mRightMesh2->SetName("Right Mesh 2");
    mLeftLight1->SetName("Left Light 1");
    mLeftLight2->SetName("Left Light 2");
    mRightLight1->SetName("Right Light 1");
    mLeftSpin1->SetName("Left Spin Root");

    SetRootComponent(mRootMesh);
    mLeftMesh1->Attach(mRootMesh);
    mRightMesh1->Attach(mRootMesh);
    mRightMesh2->Attach(mRightMesh1);
    mRightLight1->Attach(mRightMesh1);
    mLeftSpin1->Attach(mLeftMesh1);
    mLeftLight1->Attach(mLeftSpin1);
    mLeftLight2->Attach(mLeftSpin1);

    mRootMesh->SetPosition(glm::vec3(0, 0, 0));
    mLeftMesh1->SetPosition(glm::vec3(-5, 0, 0));
    mRightMesh1->SetPosition(glm::vec3(5, 0, 0));
    mRightMesh2->SetPosition(glm::vec3(0, 5, 0));
    mLeftLight1->SetPosition(glm::vec3(-2, 0, 0));
    mLeftLight2->SetPosition(glm::vec3(2, 0, 0));
    mRightLight1->SetPosition(glm::vec3(0, -2.5f, 0));

    StaticMeshRef ConeMesh = LoadAsset("SM_Cone");
    StaticMeshRef CubeMesh = LoadAsset("SM_Cube");

    mRootMesh->SetStaticMesh(ConeMesh.Get<StaticMesh>());
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
        LogDebug("----Actor Types----")
        std::vector<Factory*>& factoryList = GetFactoryList();
        
        for (uint32_t i = 0; i < factoryList.size(); ++i)
        {
            LogDebug("ActorClass[%d] %s - %lu", i, factoryList[i]->GetClassName(), factoryList[i]->GetType());
        }
    }

    {
        LogDebug("----Component Types----")
        std::vector<Factory*>& factoryList = Component::GetFactoryList();
        
        for (uint32_t i = 0; i < factoryList.size(); ++i)
        {
            LogDebug("ComponentClass[%d] %s - %lu", i, factoryList[i]->GetClassName(), factoryList[i]->GetType());
        }
    }


    {
        LogDebug("----Asset Types----")
        std::vector<Factory*>& factoryList = Asset::GetFactoryList();
        
        for (uint32_t i = 0; i < factoryList.size(); ++i)
        {
            LogDebug("AssetClass[%d] %s - %lu", i, factoryList[i]->GetClassName(), factoryList[i]->GetType());
        }
    }

    mScript = CreateComponent<ScriptComponent>();
    //mScript->SetFile("Engine/Scripts/Test.lua");
    //mScript->Create();
}

void TestActor::Destroy()
{
    Actor::Destroy();
}

void TestActor::Tick(float deltaTime)
{
    Actor::Tick(deltaTime);

    static bool rotate = true;
    static bool translate = false;
    static bool scale = false;

    if (rotate)
    {
        glm::vec3 rootRot = mRootMesh->GetRotationEuler();
        rootRot.y += 30 * deltaTime;
        mRootMesh->SetRotation(rootRot);

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
        glm::vec3 rootPos = mRootMesh->GetPosition();
        rootPos.z = 20.0f * sinf(0.5f * GetAppClock()->GetTime());
        mRootMesh->SetPosition(rootPos);

        glm::vec3 right2Pos = mRightMesh2->GetPosition();
        right2Pos.y = 5 + 2.5f * sinf(4.0f * GetAppClock()->GetTime());
        mRightMesh2->SetPosition(right2Pos);
    }

    if (scale)
    {
        float newScale = 2 + 1 * sinf(2 * GetAppClock()->GetTime());
        mRootMesh->SetScale(glm::vec3(newScale, newScale, newScale));

        float right2Scale = 1 + 2 * fabs(sinf(5 * GetAppClock()->GetTime()));
        mRightMesh2->SetScale(glm::vec3(right2Scale, right2Scale, right2Scale));
    }
}

DEFINE_ACTOR(TestActor, Actor);

#endif
