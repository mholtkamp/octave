#include "StaticMeshActor.h"

DEFINE_ACTOR(StaticMeshActor, Actor);
FORCE_LINK_DEF(StaticMeshActor);


StaticMeshActor::StaticMeshActor() :
    mStaticMeshComponent(nullptr)
{

}

void StaticMeshActor::Create()
{
    Actor::Create();

    mStaticMeshComponent = CreateComponent<StaticMesh3D>();
    SetRootComponent(mStaticMeshComponent);
    mStaticMeshComponent->EnablePhysics(false);
    mStaticMeshComponent->EnableCollision(true);
    mStaticMeshComponent->EnableOverlaps(false);
    mStaticMeshComponent->SetCollisionGroup(ColGroup1);
    mStaticMeshComponent->SetCollisionMask(~ColGroup1);

    SetName("StaticMesh");
}

StaticMesh3D* StaticMeshActor::GetStaticMeshComponent()
{
    return mStaticMeshComponent;
}

#if EDITOR
void StaticMeshActor::InitFromAiNode(const aiNode& node, std::vector<StaticMesh>& meshes)
{
    mName = node.mName.C_Str();
    aiMatrix4x4 invTransform = node.mTransformation;
    invTransform.Transpose();
    glm::mat4 transform;
    memcpy(&transform, &invTransform, sizeof(aiMatrix4x4));
    mStaticMeshComponent->SetTransform(transform);

    if (node.mNumMeshes > 0)
    {
        if (node.mNumMeshes != 1)
        {
            printf("Multiple meshes received by one actor.\n");
        }

        mStaticMeshComponent->SetStaticMesh(&meshes[node.mMeshes[0]]);
    }
}
#endif