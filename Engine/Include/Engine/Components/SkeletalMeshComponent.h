#pragma once

#include "Components/MeshComponent.h"
#include "Components/ScriptComponent.h"
#include "AssetRef.h"
#include "Vertex.h"

enum class BoneInfluenceMode
{
    One,
    Four,
    Num
};

enum class AnimationUpdateMode
{
    AlwaysUpdateTimeAndBones,
    AlwaysUpdateTime,
    OnlyUpdateWhenRendered,

    Count
};

class SkeletalMesh;
struct AnimEvent;
struct Channel;
struct Animation;

struct ActiveAnimation
{
    std::string mName;
    float mTime = 0.0f;
    float mSpeed = 1.0f;;
    float mWeight = 0.0f;
    bool mLoop = false;
};

typedef void(*AnimEventHandlerFP)(const AnimEvent& animEvent);

class SkeletalMeshComponent : public MeshComponent
{
public:

    DECLARE_COMPONENT(SkeletalMeshComponent, MeshComponent);

    SkeletalMeshComponent();
    ~SkeletalMeshComponent();

    virtual const char* GetTypeName() const override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;

    virtual void Create() override;
    virtual void Destroy() override;
    SkeletalMeshCompResource* GetResource();

    virtual void SaveStream(Stream& stream) override;
    virtual void LoadStream(Stream& stream) override;

    virtual void Tick(float deltaTime) override;

    virtual bool IsStaticMeshComponent() const override;
    virtual bool IsSkeletalMeshComponent() const override;

    void SetSkeletalMesh(SkeletalMesh* skeletalMesh);
    SkeletalMesh* GetSkeletalMesh();

    void PlayAnimation(const char* animName, bool loop, float speed = 1.0f, float weight = 1.0f, uint8_t priority = 255);
    void StopAnimation(const char* animName);
    void StopAllAnimations();
    bool IsAnimationPlaying(const char* animName);
    void ResetAnimation();
    float GetAnimationSpeed() const;
    void SetAnimationSpeed(float speed);

    void SetAnimationPaused(bool paused);
    bool IsAnimationPaused() const;

    bool HasAnimatedThisFrame() const;

    ActiveAnimation* FindActiveAnimation(const char* animName);
    std::vector<ActiveAnimation>& GetActiveAnimations();

    glm::mat4 GetBoneTransform(const std::string& name) const;
    glm::vec3 GetBonePosition(const std::string& name) const;
    glm::vec3 GetBoneRotation(const std::string& name) const;
    glm::vec3 GetBoneScale(const std::string& name) const;

    glm::mat4 GetBoneTransform(int32_t index) const;
    glm::vec3 GetBonePosition(int32_t boneIndex) const;
    glm::vec3 GetBoneRotation(int32_t boneIndex) const;
    glm::vec3 GetBoneScale(int32_t boneIndex) const;

    void SetBoneTransform(int32_t boneIndex, const glm::mat4& transform);
    void SetBonePosition(int32_t boneIndex, glm::vec3 position);
    void SetBoneRotation(int32_t boneIndex, glm::vec3 rotation);
    void SetBoneScale(int32_t boneIndex, glm::vec2 scale);

    uint32_t GetNumBones() const;
    BoneInfluenceMode GetBoneInfluenceMode() const;

    AnimationUpdateMode GetAnimationUpdateMode() const;
    void SetAnimationUpdateMode(AnimationUpdateMode mode);

    Vertex* GetSkinnedVertices();
    uint32_t GetNumSkinnedVertices();

    virtual Material* GetMaterial() override;
    virtual void Render() override;

    void UpdateAnimation(float deltaTime, bool updateBones);

    virtual Bounds GetLocalBounds() const override;

    int32_t FindBoneIndex(const std::string& name) const;

    void SetAnimEventHandler(AnimEventHandlerFP handler);
    AnimEventHandlerFP GetAnimEventHandler();
    void SetScriptAnimEventHandler(
        const char* tableName,
        const char* funcName);

protected:

    static bool HandlePropChange(Datum* datum, const void* newValue);

    glm::vec3 InterpolateScale(float time, const Channel& channel);
    glm::quat InterpolateRotation(float time, const Channel& channel);
    glm::vec3 InterpolatePosition(float time, const Channel& channel);
    void DetectTriggeredAnimEvents(
        const Animation& animation,
        float prevTickTime,
        float tickTime,
        float animationSpeed,
        std::vector<AnimEvent>& outEvents);

    uint32_t FindScaleIndex(float time, const Channel& channel);
    uint32_t FindRotationIndex(float time, const Channel& channel);
    uint32_t FindPositionIndex(float time, const Channel& channel);

    void UpdateAttachedChildren(float deltaTime);
    void CpuSkinVertices();

    SkeletalMeshRef mSkeletalMesh;
    std::vector<glm::mat4> mBoneMatrices;
    std::vector<Vertex> mSkinnedVertices; // Used by CPU skinning only.

    ScriptableFP<AnimEventHandlerFP> mAnimEventHandler;
    std::string mDefaultAnimation;
    float mAnimationSpeed = 1.0f;
    std::vector<ActiveAnimation> mActiveAnimations;
    bool mAnimationPaused;
    bool mRevertToBindPose;
    bool mHasAnimatedThisFrame;

    BoneInfluenceMode mBoneInfluenceMode;
    AnimationUpdateMode mAnimationUpdateMode;

    // Graphics Resource
    SkeletalMeshCompResource mResource;
};