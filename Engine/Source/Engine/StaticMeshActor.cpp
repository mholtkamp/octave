#include "StaticMeshActor.h"

DEFINE_ACTOR(StaticMeshActor, Actor);
FORCE_LINK_DEF(StaticMeshActor);


StaticMeshActor::StaticMeshActor() :
    mStaticMesh3D(nullptr)
{

}

void StaticMeshActor::Create()
{
    Actor::Create();

    mStaticMesh3D = CreateComponent<StaticMesh3D>();
    SetRootComponent(mStaticMesh3D);
    mStaticMesh3D->EnablePhysics(false);
    mStaticMesh3D->EnableCollision(true);
    mStaticMesh3D->EnableOverlaps(false);
    mStaticMesh3D->SetCollisionGroup(ColGroup1);
    mStaticMesh3D->SetCollisionMask(~ColGroup1);

    SetName("StaticMesh");
}

StaticMesh3D* StaticMeshActor::GetStaticMesh3D()
{
    return mStaticMesh3D;
}

#if EDITOR
void StaticMeshActor::InitFromAiNode(const aiNode& node, std::vector<StaticMesh>& meshes)
{
    mName = node.mName.C_Str();
    aiMatrix4x4 invTransform = node.mTransformation;
    invTransform.Transpose();
    glm::mat4 transform;
    memcpy(&transform, &invTransform, sizeof(aiMatrix4x4));
    mStaticMesh3D->SetTransform(transform);

    if (node.mNumMeshes > 0)
    {
        if (node.mNumMeshes != 1)
        {
            printf("Multiple meshes received by one actor.\n");
        }

        mStaticMesh3D->SetStaticMesh(&meshes[node.mMeshes[0]]);
    }
}
#endif