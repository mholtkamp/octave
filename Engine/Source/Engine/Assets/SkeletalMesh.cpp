#include "Assets/SkeletalMesh.h"
#include "Renderer.h"
#include "Vertex.h"
#include "AssetManager.h"
#include "Log.h"
#include "Maths.h"

#include "Graphics/Graphics.h"

#if EDITOR
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#endif

using namespace std;

FORCE_LINK_DEF(SkeletalMesh);
DEFINE_ASSET(SkeletalMesh);

SkeletalMesh::SkeletalMesh() :
    mMaterial(nullptr),
    mNumVertices(0),
    mNumIndices(0),
    mNumUvMaps(2)
{
    mType = SkeletalMesh::GetStaticType();
}

SkeletalMesh::~SkeletalMesh()
{

}

SkeletalMeshResource* SkeletalMesh::GetResource()
{
    return &mResource;
}

Material* SkeletalMesh::GetMaterial()
{
    return mMaterial.Get<Material>();
}

void SkeletalMesh::SetMaterial(class Material* newMaterial)
{
    mMaterial = newMaterial;
}

void SkeletalMesh::LoadStream(Stream& stream, Platform platform)
{
    Asset::LoadStream(stream, platform);

    mNumVertices = stream.ReadUint32();
    mNumIndices = stream.ReadUint32();
    // TODO: Handle multiple uv maps
    //mNumUvMaps = stream.ReadUint32();

    stream.ReadAsset(mMaterial);
    stream.ReadAsset(mAnimationLookupMesh);
    if (mMaterial.Get() == nullptr)
    {
        mMaterial = Renderer::Get()->GetDefaultMaterial();
    }

    mInvRootTransform = stream.ReadMatrix();

    uint32_t numBones = stream.ReadUint32();
    mBones.resize(numBones);

    for (uint32_t i = 0; i < numBones; ++i)
    {
        stream.ReadString(mBones[i].mName);
        mBones[i].mIndex = stream.ReadInt32();
        mBones[i].mParentIndex = stream.ReadInt32();
        mBones[i].mOffsetMatrix = stream.ReadMatrix();
        mBones[i].mInvOffsetMatrix = glm::inverse(mBones[i].mOffsetMatrix);
    }

    uint32_t numAnimations = stream.ReadUint32();
    mAnimations.resize(numAnimations);

    for (uint32_t animIndex = 0; animIndex < numAnimations; ++animIndex)
    {
        Animation& animation = mAnimations[animIndex];
        stream.ReadString(animation.mName);
        animation.mDuration = stream.ReadFloat();
        animation.mTicksPerSecond = stream.ReadFloat();
        uint32_t numChannels = stream.ReadUint32();
        animation.mChannels.resize(numChannels);

        for (uint32_t chanIndex = 0; chanIndex < numChannels; ++chanIndex)
        {
            Channel& channel = animation.mChannels[chanIndex];
            channel.mBoneIndex = stream.ReadInt32();

            uint32_t numPositionKeys = stream.ReadUint32();
            channel.mPositionKeys.resize(numPositionKeys);
            for (uint32_t i = 0; i < numPositionKeys; ++i)
            {
                channel.mPositionKeys[i].mTime = stream.ReadFloat();
                channel.mPositionKeys[i].mValue = stream.ReadVec3();
            }

            uint32_t numRotationKeys = stream.ReadUint32();
            channel.mRotationKeys.resize(numRotationKeys);
            for (uint32_t i = 0; i < numRotationKeys; ++i)
            {
                channel.mRotationKeys[i].mTime = stream.ReadFloat();
                channel.mRotationKeys[i].mValue = stream.ReadQuat();
            }

            uint32_t numScaleKeys = stream.ReadUint32();
            channel.mScaleKeys.resize(numScaleKeys);
            for (uint32_t i = 0; i < numScaleKeys; ++i)
            {
                channel.mScaleKeys[i].mTime = stream.ReadFloat();
                channel.mScaleKeys[i].mValue = stream.ReadVec3();
            }
        }

        uint32_t numEventTracks = stream.ReadUint32();
        animation.mEventTracks.resize(numEventTracks);

        for (uint32_t i = 0; i < numEventTracks; ++i)
        {
            AnimEventTrack& track = animation.mEventTracks[i];
            stream.ReadString(track.mName);

            uint32_t numEventKeys = stream.ReadUint32();
            track.mEventKeys.resize(numEventKeys);

            for (uint32_t k = 0; k < numEventKeys; ++k)
            {
                AnimEventKey& key = track.mEventKeys[k];
                key.mTime = stream.ReadFloat();
                key.mValue = stream.ReadVec3();
            }
        }
    }

    mVertices.resize(mNumVertices);
    for (uint32_t i = 0; i < mNumVertices; ++i)
    {
        mVertices[i].mPosition = stream.ReadVec3();
        mVertices[i].mTexcoord0 = stream.ReadVec2();
        mVertices[i].mTexcoord1 = { 0.0f, 0.0f };// stream.ReadVec2();
        mVertices[i].mNormal = stream.ReadVec3();
        mVertices[i].mBoneIndices[0] = stream.ReadUint8();
        mVertices[i].mBoneIndices[1] = stream.ReadUint8();
        mVertices[i].mBoneIndices[2] = stream.ReadUint8();
        mVertices[i].mBoneIndices[3] = stream.ReadUint8();
        mVertices[i].mBoneWeights[0] = stream.ReadFloat();
        mVertices[i].mBoneWeights[1] = stream.ReadFloat();
        mVertices[i].mBoneWeights[2] = stream.ReadFloat();
        mVertices[i].mBoneWeights[3] = stream.ReadFloat();
    }

    mIndices.resize(mNumIndices);
    for (uint32_t i = 0; i < mNumIndices; ++i)
    {
        mIndices[i] = (IndexType) stream.ReadUint32();
    }

    mBounds.mCenter = stream.ReadVec3();
    mBounds.mRadius = stream.ReadFloat();
    mBoundsScale = stream.ReadFloat();
}

void SkeletalMesh::SaveStream(Stream& stream, Platform platform)
{
    Asset::SaveStream(stream, platform);
    
#if EDITOR
    stream.WriteUint32(mNumVertices);
    stream.WriteUint32(mNumIndices);

    stream.WriteAsset(mMaterial);
    stream.WriteAsset(mAnimationLookupMesh);
    stream.WriteMatrix(mInvRootTransform);

    stream.WriteUint32(uint32_t(mBones.size()));
    for (uint32_t i = 0; i < uint32_t(mBones.size()); ++i)
    {
        stream.WriteString(mBones[i].mName);
        stream.WriteInt32(mBones[i].mIndex);
        stream.WriteInt32(mBones[i].mParentIndex);
        stream.WriteMatrix(mBones[i].mOffsetMatrix);
    }

    stream.WriteUint32(uint32_t(mAnimations.size()));
    for (uint32_t animIndex = 0; animIndex < uint32_t(mAnimations.size()); ++animIndex)
    {
        Animation& animation = mAnimations[animIndex];
        stream.WriteString(animation.mName);
        stream.WriteFloat(animation.mDuration);
        stream.WriteFloat(animation.mTicksPerSecond);
        stream.WriteUint32(uint32_t(animation.mChannels.size()));

        uint32_t numChannels = uint32_t(animation.mChannels.size());
        for (uint32_t chanIndex = 0; chanIndex < numChannels; ++chanIndex)
        {
            Channel& channel = animation.mChannels[chanIndex];
            stream.WriteInt32(channel.mBoneIndex);


            stream.WriteUint32((uint32_t)channel.mPositionKeys.size());
            for (uint32_t i = 0; i < uint32_t(channel.mPositionKeys.size()); ++i)
            {
                stream.WriteFloat(channel.mPositionKeys[i].mTime);
                stream.WriteVec3(channel.mPositionKeys[i].mValue);
            }

            stream.WriteUint32((uint32_t)channel.mRotationKeys.size());
            for (uint32_t i = 0; i < uint32_t(channel.mRotationKeys.size()); ++i)
            {
                stream.WriteFloat(channel.mRotationKeys[i].mTime);
                stream.WriteQuat(channel.mRotationKeys[i].mValue);
            }

            stream.WriteUint32((uint32_t)channel.mScaleKeys.size());
            for (uint32_t i = 0; i < uint32_t(channel.mScaleKeys.size()); ++i)
            {
                stream.WriteFloat(channel.mScaleKeys[i].mTime);
                stream.WriteVec3(channel.mScaleKeys[i].mValue);
            }
        }

        uint32_t numEventTracks = (uint32_t)animation.mEventTracks.size();
        stream.WriteUint32(numEventTracks);

        for (uint32_t i = 0; i < numEventTracks; ++i)
        {
            AnimEventTrack& track = animation.mEventTracks[i];
            stream.WriteString(track.mName);

            uint32_t numEventKeys = (uint32_t)track.mEventKeys.size();
            stream.WriteUint32(numEventKeys);

            for (uint32_t k = 0; k < numEventKeys; ++k)
            {
                AnimEventKey& key = track.mEventKeys[k];
                stream.WriteFloat(key.mTime);
                stream.WriteVec3(key.mValue);
            }
        }
    }

    OCT_ASSERT(mNumVertices == mVertices.size());
    for (uint32_t i = 0; i < mNumVertices; ++i)
    {
        stream.WriteVec3(mVertices[i].mPosition);
        stream.WriteVec2(mVertices[i].mTexcoord0);
        //stream.WriteVec2(mVertices[i].mTexcoord1);
        stream.WriteVec3(mVertices[i].mNormal);
        stream.WriteUint8(mVertices[i].mBoneIndices[0]);
        stream.WriteUint8(mVertices[i].mBoneIndices[1]);
        stream.WriteUint8(mVertices[i].mBoneIndices[2]);
        stream.WriteUint8(mVertices[i].mBoneIndices[3]);
        stream.WriteFloat(mVertices[i].mBoneWeights[0]);
        stream.WriteFloat(mVertices[i].mBoneWeights[1]);
        stream.WriteFloat(mVertices[i].mBoneWeights[2]);
        stream.WriteFloat(mVertices[i].mBoneWeights[3]);
    }

    OCT_ASSERT(mNumIndices == mIndices.size());
    for (uint32_t i = 0; i < mNumIndices; ++i)
    {
        stream.WriteUint32(mIndices[i]);
    }

    stream.WriteVec3(mBounds.mCenter);
    stream.WriteFloat(mBounds.mRadius);
    stream.WriteFloat(mBoundsScale);
#endif
}

void SkeletalMesh::Create()
{
    Asset::Create();

    OCT_ASSERT(mNumVertices <= MAX_MESH_VERTEX_COUNT); // Vertex index must fit into IndexType width.

    GFX_CreateSkeletalMeshResource(this, mNumVertices, mVertices.data(), mNumIndices, mIndices.data());

    InitBindPose();
}

void SkeletalMesh::Destroy()
{
    Asset::Destroy();

    GFX_DestroySkeletalMeshResource(this);
    mMaterial = nullptr;
}


void SkeletalMesh::Import(const std::string& path, ImportOptions* options)
{
#if EDITOR
    Asset::Import(path, options);

    // Loads a .DAE file and loads the first mesh in the mesh library.
    if (mResource.mVertexBuffer == VK_NULL_HANDLE)
    {
        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(path, aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

        if (scene == nullptr)
        {
            LogError("Failed to load dae/gltf file");
            OCT_ASSERT(0);
        }

        if (scene->mNumMeshes < 1)
        {
            LogError("Failed to find any meshes in dae/gltf file");
            OCT_ASSERT(0);
        }

        if (!scene->mMeshes[0]->HasBones())
        {
            LogError("Skeletal mesh has no bones");
            OCT_ASSERT(0);
        }

        Create(*scene, *scene->mMeshes[0]);
    }
#endif
}

void SkeletalMesh::GatherProperties(std::vector<Property>& outProps)
{
    Asset::GatherProperties(outProps);
    outProps.push_back(Property(DatumType::Asset, "Material", this, &mMaterial, 1, nullptr, int32_t(Material::GetStaticType())));
    outProps.push_back(Property(DatumType::Asset, "Animation Lookup", this, &mAnimationLookupMesh, 1, nullptr, int32_t(SkeletalMesh::GetStaticType())));
    outProps.push_back(Property(DatumType::Float, "Bounds Scale", this, &mBoundsScale));

    // TODO: Do we want default animations?
    //outProps.push_back(Property(DatumType::String, "Default Animation", this, &mDefaultAnimation));
}

glm::vec4 SkeletalMesh::GetTypeColor()
{
    return glm::vec4(0.8f, 0.0f, 0.4f, 1.0f);
}

const char* SkeletalMesh::GetTypeName()
{
    return "SkeletalMesh";
}

const char* SkeletalMesh::GetTypeImportExt()
{
    return ".glb";
}


uint32_t SkeletalMesh::GetNumIndices()
{
    return mNumIndices;
}

uint32_t SkeletalMesh::GetNumFaces()
{
    return mNumIndices / 3;
}

uint32_t SkeletalMesh::GetNumVertices()
{
    return mNumVertices;
}

const IndexType* SkeletalMesh::GetIndices() const
{
    return mIndices.data();
}

int32_t SkeletalMesh::FindBoneIndex(const std::string& name) const
{
    int32_t boneIndex = -1;

    for (uint32_t i = 0; i < mBones.size(); ++i)
    {
        if (mBones[i].mName == name)
        {
            boneIndex = i;
            break;
        }
    }

    return boneIndex;
}

const std::vector<Bone>& SkeletalMesh::GetBones() const
{
    return mBones;
}

const Bone& SkeletalMesh::GetBone(int32_t index) const
{
    return mBones[index];
}

uint32_t SkeletalMesh::GetNumBones() const
{
    return uint32_t(mBones.size());
}

glm::mat4 SkeletalMesh::GetInvRootTransform() const
{
    return mInvRootTransform;
}

const std::vector<Animation>& SkeletalMesh::GetAnimations() const
{
    return mAnimations;
}

const Animation* SkeletalMesh::GetAnimation(const char* name)
{
    const Animation* retAnim = nullptr;

    for (uint32_t i = 0; i < mAnimations.size(); ++i)
    {
        if (mAnimations[i].mName == name)
        {
            retAnim = &mAnimations[i];
            break;
        }
    }

    if (mAnimationLookupMesh != nullptr)
    {
        SkeletalMesh* animLookup = mAnimationLookupMesh.Get<SkeletalMesh>();
        retAnim = animLookup->GetAnimation(name);
    }

    return retAnim;
}

float SkeletalMesh::GetAnimationDuration(const char* name)
{
    float duration = 0.0f;
    
    const Animation* anim = GetAnimation(name);
    if (anim != nullptr)
    {
        duration = (anim->mDuration / anim->mTicksPerSecond);
    }

    return duration;
}

const std::vector<VertexSkinned>& SkeletalMesh::GetVertices() const
{
    return mVertices;
}

void SkeletalMesh::FinalizeBoneTransforms(std::vector<glm::mat4>& inoutTransforms)
{
    // Iterate through each bone and if it has a parent bone, concatenate 
    // the bone's transform matrix with its parent bone matrix.
    for (uint32_t i = 0; i < mBones.size(); ++i)
    {
        int32_t parentIndex = mBones[i].mParentIndex;
        if (parentIndex != -1)
        {
            inoutTransforms[i] = inoutTransforms[parentIndex] * inoutTransforms[i];
        }
    }

    // One last iteration to multiply the transforms by the bone offset matrix
    // to calculate the final bone transform matrix that will be passed to vertex shader or cpu skinner.
    for (uint32_t i = 0; i < mBones.size(); ++i)
    {
        inoutTransforms[i] = mInvRootTransform * inoutTransforms[i] * mBones[i].mOffsetMatrix;
    }
}

void SkeletalMesh::CopyBindPose(std::vector<glm::mat4>& outTransforms)
{
    outTransforms.resize(mBones.size());

    if (mBindPoseMatrices.size() == mBones.size())
    {
        memcpy(outTransforms.data(), mBindPoseMatrices.data(), sizeof(glm::mat4) * mBones.size());
    }
    else
    {
        // Reset all out transform matrices to identity
        for (uint32_t i = 0; i < mBones.size(); ++i)
        {
            outTransforms[i] = glm::mat4(1.0f);
        }
    }
}

Bounds SkeletalMesh::GetBounds() const
{
    Bounds retBounds;
    retBounds.mCenter = mBounds.mCenter;
    retBounds.mRadius = mBounds.mRadius * mBoundsScale;
    return retBounds;
}

const glm::mat4 SkeletalMesh::GetBindPoseMatrix(int32_t boneIndex) const
{
    return mBindPoseMatrices[boneIndex];
}

SkeletalMesh* SkeletalMesh::GetAnimationLookupMesh()
{
    return mAnimationLookupMesh.Get<SkeletalMesh>();
}

void SkeletalMesh::SetAnimationLookupMesh(SkeletalMesh* lookupMesh)
{
    mAnimationLookupMesh = lookupMesh;
}

void SkeletalMesh::InitBindPose()
{
    mBindPoseMatrices.clear();
    mBindPoseMatrices.resize(mBones.size());

    for (uint32_t i = 0; i < mBones.size(); ++i)
    {
        mBindPoseMatrices[i] = mBones[i].mInvOffsetMatrix;

        int32_t parentIndex = mBones[i].mParentIndex;
        if (parentIndex != -1)
        {
            mBindPoseMatrices[i] = mBones[parentIndex].mOffsetMatrix * mBindPoseMatrices[i];
        }
    }

#if 0
    // If there is a bind pose, save it off.
    for (uint32_t i = 0; i < mAnimations.size(); ++i)
    {
        if (mAnimations[i].mName == "BindPose")
        {
            float time = 0.0f;
            mBindPoseMatrices.resize(mBones.size());
            AnimateBones("BindPose", time, mBindPoseMatrices);
            break;
        }
    }
#endif
}

void SkeletalMesh::ComputeBounds()
{
    if (mNumVertices == 0)
    {
        mBounds.mCenter = { 0.0f, 0.0f, 0.0f };
        mBounds.mRadius = 0.0f;
        return;
    }

    glm::vec3 boxMin = { FLT_MAX, FLT_MAX, FLT_MAX };
    glm::vec3 boxMax = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

    for (uint32_t i = 0; i < mNumVertices; ++i)
    {
        glm::vec3 pos = mVertices[i].mPosition;

        boxMin = glm::min(boxMin, pos);
        boxMax = glm::max(boxMax, pos);
    }

    mBounds.mCenter = (boxMin + boxMax) / 2.0f;

    float maxDist = 0.0f;

    for (uint32_t i = 0; i < mNumVertices; ++i)
    {
        glm::vec3 pos = mVertices[i].mPosition;

        float dist = glm::distance(pos, mBounds.mCenter);
        maxDist = glm::max(maxDist, dist);
    }

    mBounds.mRadius = maxDist;
}

#if EDITOR
void SkeletalMesh::Create(const aiScene& scene,
    const aiMesh& meshData,
    std::vector<Material>* materials)
{
    if (meshData.mNumVertices == 0 ||
        meshData.mNumFaces == 0 ||
        meshData.mNumBones == 0)
    {
        return;
    }

    mInvRootTransform = glm::transpose(glm::make_mat4(&scene.mRootNode->mTransformation.a1));
    mInvRootTransform = glm::inverse(mInvRootTransform);

    // Read bone data and accumulate bone indices / weights for each vertex.
    mBones.clear();
    std::vector<uint8_t> boneIndices;
    std::vector<float> boneWeights;
    boneIndices.resize(MAX_BONE_INFLUENCES * meshData.mNumVertices);
    boneWeights.resize(MAX_BONE_INFLUENCES * meshData.mNumVertices);

    // Setup bone hierarchy
    SetupBoneHierarchy(*scene.mRootNode, meshData, boneIndices, boneWeights, -1);

    // Register any "bones" used for animation events. These are prefixed with "Event_".
    for (int32_t i = 0; i < (int32_t)meshData.mNumBones; ++i)
    {
        aiBone* bone = meshData.mBones[i];
        if (strncmp(bone->mName.C_Str(), "Event_", strlen("Event_")) == 0)
        {
            Bone boneData;
            boneData.mName = bone->mName.C_Str();
            boneData.mOffsetMatrix = glm::transpose(glm::make_mat4(&bone->mOffsetMatrix.a1));
            boneData.mInvOffsetMatrix = glm::inverse(boneData.mOffsetMatrix);
            boneData.mIndex = (int32_t)mBones.size();
            mBones.push_back(boneData);
        }
    }

    // Verify that all bones now have parent indices.
    //for (uint32_t i = 0; i < mBones.size(); ++i)
    //{
    //    if (i == 0)
    //    {
    //        OCT_ASSERT(mBones[i].mParentIndex == -1);
    //    }
    //    else
    //    {
    //        OCT_ASSERT(mBones[i].mParentIndex >= 0 && 
    //               mBones[i].mParentIndex < mBones.size() &&
    //               mBones[i].mParentIndex != i);
    //    }
    //}

    // Import animations
    SetupAnimations(scene);

    // Setup vertex/index buffers
    SetupResource(meshData, boneWeights, boneIndices);

    // Remove event bones. Those are a work around for exporting animation events.
    for (int32_t i = int32_t(mBones.size()) - 1; i >= 0; --i)
    {
        if (mBones[i].mName.substr(0, 6) == "Event_")
        {
            mBones.erase(mBones.begin() + i);
        }
    }

    mMaterial = Renderer::Get()->GetDefaultMaterial();

    ComputeBounds();
}

void SkeletalMesh::SetupBoneHierarchy(
    const aiNode& node,
    const aiMesh& meshData,
    std::vector<uint8_t>& boneIndices,
    std::vector<float>& boneWeights,
    int32_t parentBoneIndex)
{
    std::string nodeName = node.mName.C_Str();
    int32_t boneIndex = -1;

    aiBone* bone = nullptr;

    for (boneIndex = 0; boneIndex < (int32_t)meshData.mNumBones; ++boneIndex)
    {
        if (meshData.mBones[boneIndex]->mName == node.mName)
        {
            bone = meshData.mBones[boneIndex];
            break;
        }
    }

    if (bone && nodeName.substr(0, 6) == "Event_")
    {
        // Event bones are handled separately from the main bone hierarchy.
        return;
    }

    if (bone == nullptr)
    {
        boneIndex = -1;
    }


    if (bone != nullptr)
    {
        Bone boneData;
        boneData.mName = bone->mName.C_Str();
        boneData.mOffsetMatrix = glm::transpose(glm::make_mat4(&bone->mOffsetMatrix.a1));
        boneData.mInvOffsetMatrix = glm::inverse(boneData.mOffsetMatrix);
        boneData.mIndex = (int32_t)mBones.size();
        boneData.mParentIndex = parentBoneIndex;
        mBones.push_back(boneData);

        for (uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex)
        {
            aiVertexWeight weight = bone->mWeights[weightIndex];
            uint8_t* vertIndices = &boneIndices[weight.mVertexId * MAX_BONE_INFLUENCES];
            float* vertWeights = &boneWeights[weight.mVertexId * MAX_BONE_INFLUENCES];

            // Insert the new weight into the weight array.
            // Keep the weights ordered by their value (largest weight first) so that
            // we can know which bone id/weight to use in the 1-bone influence case.
            int32_t insertIndex = 0;
            for (insertIndex = 0; insertIndex < MAX_BONE_INFLUENCES; ++insertIndex)
            {
                if (vertWeights[insertIndex] == 0.0f ||
                    vertWeights[insertIndex] < weight.mWeight)
                {
                    break;
                }
            }

            if (insertIndex < MAX_BONE_INFLUENCES)
            {
                OCT_ASSERT(insertIndex >= 0);

                // Shift all weights forward one index to make room for the new weight/index.
                for (int32_t i = MAX_BONE_INFLUENCES - 1; i > insertIndex; --i)
                {
                    vertIndices[i] = vertIndices[i - 1];
                    vertWeights[i] = vertWeights[i - 1];
                }

                vertIndices[insertIndex] = boneData.mIndex;
                vertWeights[insertIndex] = weight.mWeight;
            }
        }
    }

    int32_t newParentIndex = (bone != nullptr) ? boneIndex : parentBoneIndex;
    for (uint32_t i = 0; i < node.mNumChildren; ++i)
    {
        SetupBoneHierarchy(*node.mChildren[i], meshData, boneIndices, boneWeights, newParentIndex);
    }
}

void SkeletalMesh::SetupAnimations(const aiScene& scene)
{
    // Read in all animations first, afterwards we will read in animation events.
    for (uint32_t animIndex = 0; animIndex < scene.mNumAnimations; ++animIndex)
    {
        const aiAnimation* srcAnim = scene.mAnimations[animIndex];
        OCT_ASSERT(srcAnim);

        mAnimations.push_back(Animation());
        Animation& dstAnim = mAnimations.back();
        dstAnim.mName = srcAnim->mName.C_Str();
        dstAnim.mDuration = (float) srcAnim->mDuration;
        // GLTF is not being imported/exported properly from assimp/blender. Hardcode to 1000. Use 60 fps in blender.
        //dstAnim.mTicksPerSecond = (float) srcAnim->mTicksPerSecond
        dstAnim.mTicksPerSecond = (float) 1000.0f;

        for (uint32_t channelIndex = 0; channelIndex < srcAnim->mNumChannels; ++channelIndex)
        {
            const aiNodeAnim* srcChannel = srcAnim->mChannels[channelIndex];
            OCT_ASSERT(srcChannel);

            // Find which bone index matches the node name.
            int32_t boneIndex = -1;
            for (uint32_t i = 0; i < mBones.size(); ++i)
            {
                if (mBones[i].mName == srcChannel->mNodeName.C_Str())
                {
                    boneIndex = (int32_t)i;
                    break;
                }
            }

            bool eventBone = strncmp(srcChannel->mNodeName.C_Str(), "Event_", strlen("Event_")) == 0;

            if (eventBone)
            {
                // This is a special bone that issues events throughout animation.
                std::string eventName = srcChannel->mNodeName.C_Str();
                eventName = eventName.substr(6); // Cut off "Event_"

                AnimEventTrack newTrack;
                newTrack.mName = eventName;

                OCT_ASSERT(srcChannel->mNumPositionKeys > 0);

                if (srcChannel->mNumPositionKeys > 0)
                {
                    for (uint32_t i = 0; i < srcChannel->mNumPositionKeys; ++i)
                    {
                        AnimEventKey newEvent;
                        newEvent.mTime = (float)srcChannel->mPositionKeys[i].mTime;

                        // Events use position keys? Idk seems safest but maybe we could switch to rotation.
                        // I don't know how to hack in events to blender's gltf export, so that's why I'm repurposing
                        // the position channel.
                        newEvent.mValue.x = (float)srcChannel->mPositionKeys[i].mValue.x;
                        newEvent.mValue.y = (float)srcChannel->mPositionKeys[i].mValue.y;
                        newEvent.mValue.z = (float)srcChannel->mPositionKeys[i].mValue.z;

                        newTrack.mEventKeys.push_back(newEvent);
                    }

                    for (AnimEventTrack& track : dstAnim.mEventTracks)
                    {
                        // Ensure there is not somehow a duplicate track. Shouldn't happen...
                        OCT_ASSERT(track.mName != newTrack.mName);
                    }

                    dstAnim.mEventTracks.push_back(newTrack);
                }
            }
            else if (boneIndex != -1)
            {
                // Normal non-event bone that deforms the mesh.
                dstAnim.mChannels.push_back(Channel());
                Channel& dstChannel = dstAnim.mChannels.back();
                dstChannel.mBoneIndex = boneIndex;
                dstChannel.mPositionKeys.resize(srcChannel->mNumPositionKeys);
                dstChannel.mRotationKeys.resize(srcChannel->mNumRotationKeys);
                dstChannel.mScaleKeys.resize(srcChannel->mNumScalingKeys);

                // Copy position keys
                for (uint32_t i = 0; i < srcChannel->mNumPositionKeys; ++i)
                {
                    dstChannel.mPositionKeys[i].mTime = (float)srcChannel->mPositionKeys[i].mTime;
                    dstChannel.mPositionKeys[i].mValue.x = (float)srcChannel->mPositionKeys[i].mValue.x;
                    dstChannel.mPositionKeys[i].mValue.y = (float)srcChannel->mPositionKeys[i].mValue.y;
                    dstChannel.mPositionKeys[i].mValue.z = (float)srcChannel->mPositionKeys[i].mValue.z;
                }

                // Copy rotation keys
                for (uint32_t i = 0; i < srcChannel->mNumRotationKeys; ++i)
                {
                    dstChannel.mRotationKeys[i].mTime = (float)srcChannel->mRotationKeys[i].mTime;
                    dstChannel.mRotationKeys[i].mValue.x = (float)srcChannel->mRotationKeys[i].mValue.x;
                    dstChannel.mRotationKeys[i].mValue.y = (float)srcChannel->mRotationKeys[i].mValue.y;
                    dstChannel.mRotationKeys[i].mValue.z = (float)srcChannel->mRotationKeys[i].mValue.z;
                    dstChannel.mRotationKeys[i].mValue.w = (float)srcChannel->mRotationKeys[i].mValue.w;
                }

                // Copy scale keys
                for (uint32_t i = 0; i < srcChannel->mNumScalingKeys; ++i)
                {
                    dstChannel.mScaleKeys[i].mTime = (float)srcChannel->mScalingKeys[i].mTime;
                    dstChannel.mScaleKeys[i].mValue.x = (float)srcChannel->mScalingKeys[i].mValue.x;
                    dstChannel.mScaleKeys[i].mValue.y = (float)srcChannel->mScalingKeys[i].mValue.y;
                    dstChannel.mScaleKeys[i].mValue.z = (float)srcChannel->mScalingKeys[i].mValue.z;
                }

                // Cut out unnecessary keyframes.
                // Remove keyframes that don't change transform.
                {
                    std::vector<PositionKey>& keys = dstChannel.mPositionKeys;
                    int32_t keyCount = (int32_t)keys.size();
                    for (int32_t i = keyCount - 2; i > 0; --i)
                    {
                        if (keys[i - 1].mValue == keys[i].mValue &&
                            keys[i + 1].mValue == keys[i].mValue)
                        {
                            keys.erase(keys.begin() + i);
                        }
                    }
                }

                {
                    std::vector<RotationKey>& keys = dstChannel.mRotationKeys;
                    int32_t keyCount = (int32_t)keys.size();
                    for (int32_t i = keyCount - 2; i > 0; --i)
                    {
                        if (keys[i - 1].mValue == keys[i].mValue &&
                            keys[i + 1].mValue == keys[i].mValue)
                        {
                            keys.erase(keys.begin() + i);
                        }
                    }
                }

                {
                    std::vector<ScaleKey>& keys = dstChannel.mScaleKeys;
                    int32_t keyCount = (int32_t)keys.size();
                    for (int32_t i = keyCount - 2; i > 0; --i)
                    {
                        if (keys[i - 1].mValue == keys[i].mValue &&
                            keys[i + 1].mValue == keys[i].mValue)
                        {
                            keys.erase(keys.begin() + i);
                        }
                    }
                }
            }
            else
            {
                LogWarning("Unknown bone channel found in animation: %s", srcChannel->mNodeName.C_Str());
            }
        }
    }

    InitBindPose();
}

void SkeletalMesh::SetupResource(const aiMesh& meshData,
    const std::vector<float>& boneWeights,
    const std::vector<uint8_t>& boneIndices)
{
    // Get pointers to vertex attributes
    glm::vec3* positions = reinterpret_cast<glm::vec3*>(meshData.mVertices);
    glm::vec3* texcoords0 = meshData.HasTextureCoords(0) ? reinterpret_cast<glm::vec3*>(meshData.mTextureCoords[0]) : nullptr;
    glm::vec3* texcoords1 = meshData.HasTextureCoords(1) ? reinterpret_cast<glm::vec3*>(meshData.mTextureCoords[1]) : texcoords0;
    glm::vec3* normals = reinterpret_cast<glm::vec3*>(meshData.mNormals);

    aiFace* faces = meshData.mFaces;
    mNumVertices = meshData.mNumVertices;
    mNumIndices = meshData.mNumFaces * 3;
    mVertices.resize(mNumVertices);
    mIndices.resize(mNumIndices);

    // Always set the num uv maps to 2 since we are storing two sets of UVs no matter what.
    mNumUvMaps = 2;
    //mNumUvMaps = glm::clamp(meshData.GetNumUVChannels(), 0u, MAX_UV_MAPS - 1u);

    // Create an interleaved VBO
    for (uint32_t i = 0; i < meshData.mNumVertices; ++i)
    {
        mVertices[i].mPosition = glm::vec3(positions[i].x, positions[i].y, positions[i].z);
        mVertices[i].mTexcoord0 = texcoords0 ? glm::vec2(texcoords0[i].x, texcoords0[i].y) : glm::vec2(0.0f, 0.0f);
        mVertices[i].mTexcoord1 = texcoords1 ? glm::vec2(texcoords1[i].x, texcoords1[i].y) : glm::vec2(0.0f, 0.0f);
        mVertices[i].mNormal = glm::vec3(normals[i].x, normals[i].y, normals[i].z);

        mVertices[i].mBoneIndices[0] = boneIndices[i * MAX_BONE_INFLUENCES + 0];
        mVertices[i].mBoneIndices[1] = boneIndices[i * MAX_BONE_INFLUENCES + 1];
        mVertices[i].mBoneIndices[2] = boneIndices[i * MAX_BONE_INFLUENCES + 2];
        mVertices[i].mBoneIndices[3] = boneIndices[i * MAX_BONE_INFLUENCES + 3];

        mVertices[i].mBoneWeights[0] = boneWeights[i * MAX_BONE_INFLUENCES + 0];
        mVertices[i].mBoneWeights[1] = boneWeights[i * MAX_BONE_INFLUENCES + 1];
        mVertices[i].mBoneWeights[2] = boneWeights[i * MAX_BONE_INFLUENCES + 2];
        mVertices[i].mBoneWeights[3] = boneWeights[i * MAX_BONE_INFLUENCES + 3];
    }

    for (uint32_t i = 0; i < meshData.mNumFaces; ++i)
    {
        // Enforce triangulated faces
        OCT_ASSERT(faces[i].mNumIndices == 3);
        mIndices[i * 3 + 0] = faces[i].mIndices[0];
        mIndices[i * 3 + 1] = faces[i].mIndices[1];
        mIndices[i * 3 + 2] = faces[i].mIndices[2];
    }

    Create();
}
#endif // EDITOR
