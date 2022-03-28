#pragma once

#include <string>

#include "Assets/Material.h"
#include "Asset.h"
#include "Vertex.h"
#include "Constants.h"

#include "Graphics/Graphics.h"
#include "Graphics/GraphicsTypes.h"
#include "Graphics/GraphicsConstants.h"

#include <glm/gtc/quaternion.hpp>

#if EDITOR
#include <assimp/scene.h>
#endif

struct Bone
{
    std::string mName;
    int32_t mIndex = -1;
    int32_t mParentIndex = -1;
    glm::mat4 mOffsetMatrix = { };
    glm::mat4 mInvOffsetMatrix = { };
};

struct PositionKey
{
    float mTime = 0.0f;
    glm::vec3 mValue = {};
};

struct RotationKey
{
    float mTime = 0.0f;
    glm::quat mValue = {};
};

struct ScaleKey
{
    float mTime = 0.0f;
    glm::vec3 mValue = {};
};

struct AnimEventKey
{
    float mTime = 0.0f;
    glm::vec3 mValue = {};
};

struct AnimEventTrack
{
    std::string mName;
    std::vector<AnimEventKey> mEventKeys;
};

struct AnimEvent
{
    Actor* mActor = nullptr;
    SkeletalMeshComponent* mComponent = nullptr;
    std::string mName;
    std::string mAnimation;
    float mTime = 0.0f;
    glm::vec3 mValue = {};
};

struct Channel
{
    int32_t mBoneIndex = -1;
    std::vector<PositionKey> mPositionKeys;
    std::vector<RotationKey> mRotationKeys;
    std::vector<ScaleKey> mScaleKeys;
};

struct Animation
{
    std::string mName;
    float mDuration = 0.0f;
    float mTicksPerSecond = 0.0f;
    std::vector<Channel> mChannels;
    std::vector<AnimEventTrack> mEventTracks;
};

class SkeletalMesh : public Asset
{
public:

    DECLARE_ASSET(SkeletalMesh, Asset);

    SkeletalMesh();
    ~SkeletalMesh();

    SkeletalMeshResource* GetResource();

    // Asset Interface
    virtual void LoadStream(Stream& stream, Platform platform) override;
    virtual void SaveStream(Stream& stream, Platform platform) override;
    virtual void Create() override;
    virtual void Destroy() override;
    virtual void Import(const std::string& path) override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual glm::vec4 GetTypeColor() override;
    virtual const char* GetTypeName() override;
    virtual const char* GetTypeImportExt() override;

    class Material* GetMaterial();
    void SetMaterial(class Material* newMaterial);

    uint32_t GetNumIndices();
    uint32_t GetNumFaces();
    uint32_t GetNumVertices();

    const IndexType* GetIndices() const;

    int32_t FindBoneIndex(const std::string& name) const;
    const std::vector<Bone>& GetBones() const;
    const Bone& GetBone(int32_t index) const;
    uint32_t GetNumBones() const;

    glm::mat4 GetInvRootTransform() const;

    const std::vector<Animation>& GetAnimations() const;
    const Animation* GetAnimation(const char* name);

    // Get length of animation in seconds
    float GetAnimationDuration(const char* name);

    const std::vector<VertexSkinned>& GetVertices() const;

    void FinalizeBoneTransforms(std::vector<glm::mat4>& inoutTransforms);

    void CopyBindPose(std::vector<glm::mat4>& outTransforms);

    Bounds GetBounds() const;

    const glm::mat4 GetBindPoseMatrix(int32_t boneIndex) const;

private:

    void InitBindPose();
    void ComputeBounds();

    MaterialRef mMaterial;
    uint32_t mNumVertices;
    uint32_t mNumIndices;

    std::vector<Bone> mBones;
    std::vector<Animation> mAnimations;
    std::vector<VertexSkinned> mVertices;

    glm::mat4 mInvRootTransform;
    std::vector<glm::mat4> mBindPoseMatrices;
    std::vector<IndexType> mIndices;

    Bounds mBounds;
    float mBoundsScale = 1.1f;

    // Graphics Resource
    SkeletalMeshResource mResource;

#if EDITOR
public:
    void Create(const aiScene& scene,
        const aiMesh& meshData,
        std::vector<Material>* materials = nullptr);

    void SetupBoneHierarchy(const aiNode& node, int32_t parentBoneIndex);
    void SetupAnimations(const aiScene& scene);
    void SetupResource(const aiMesh& meshData,
        const std::vector<float>& boneWeights,
        const std::vector<uint8_t>& boneIndices);
#endif // EDITOR
};
