#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Assets/SkeletalMesh.h"
#include "Renderer.h"
#include "AssetManager.h"
#include "ScriptEvent.h"
#include "Log.h"
#include "Utilities.h"

#include "Graphics/Graphics.h"

#include <glm/gtx/transform.hpp>

static const char* sBoneInfluenceModeStrings[] =
{
    "One Bone",
    "Four Bones"
};
static_assert(int32_t(BoneInfluenceMode::Num) == 2, "Need to update string conversion table");

static const char* sAnimationUpdateModeStrings[] =
{
    "Always Update",
    "Always Update Time",
    "Only When Rendered"
};
static_assert(int32_t(AnimationUpdateMode::Count) == 3, "Need to update string conversion table");

FORCE_LINK_DEF(SkeletalMeshComponent);
DEFINE_COMPONENT(SkeletalMeshComponent);

bool SkeletalMeshComponent::HandlePropChange(Datum* datum, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);
    OCT_ASSERT(prop != nullptr);
    SkeletalMeshComponent* meshComp = static_cast<SkeletalMeshComponent*>(prop->mOwner);
    bool success = false;

    if (prop->mName == "Skeletal Mesh")
    {
        meshComp->SetSkeletalMesh(*(SkeletalMesh**) newValue);
        success = true;
    }
    else if (prop->mName == "Default Animation")
    {
        meshComp->StopAllAnimations();
        meshComp->mDefaultAnimation = *((std::string*) newValue);
        meshComp->PlayAnimation(meshComp->mDefaultAnimation.c_str(), true);
        success = true;
    }

    return success;
}

static SkeletalMesh* GetDefaultMesh()
{
    // TODO: Create a default skeletal mesh
    //static SkeletalMeshRef defaultMesh = LoadAsset("SK_Flower");
    //return defaultMesh.GetSkeletalMesh();

    return nullptr;
}

SkeletalMeshComponent::SkeletalMeshComponent() :
    mSkeletalMesh(nullptr),
    mAnimationSpeed(1.0f),
    mAnimationPaused(false),
    mRevertToBindPose(true),
    mInheritPose(false),
    mHasAnimatedThisFrame(false),
    mBoneInfluenceMode(BoneInfluenceMode::Four),
    mAnimationUpdateMode(AnimationUpdateMode::OnlyUpdateWhenRendered)
{
    mName = "Skeletal Mesh";
    mCollisionEnabled = false;
}

SkeletalMeshComponent::~SkeletalMeshComponent()
{

}

const char* SkeletalMeshComponent::GetTypeName() const
{
    return "SkeletalMesh";
}

void SkeletalMeshComponent::GatherProperties(std::vector<Property>& outProps)
{
    MeshComponent::GatherProperties(outProps);
    outProps.push_back(Property(DatumType::Asset, "Skeletal Mesh", this, &mSkeletalMesh, 1, HandlePropChange, int32_t(SkeletalMesh::GetStaticType())));
    outProps.push_back(Property(DatumType::String, "Default Animation", this, &mDefaultAnimation, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Float, "Animation Speed", this, &mAnimationSpeed));
    outProps.push_back(Property(DatumType::Bool, "Animation Paused", this, &mAnimationPaused));
    outProps.push_back(Property(DatumType::Bool, "Revert To Bind Pose", this, &mRevertToBindPose));
    outProps.push_back(Property(DatumType::Bool, "Inherit Pose", this, &mInheritPose));
    outProps.push_back(Property(DatumType::Integer, "Bone Influence Mode", this, &mBoneInfluenceMode, 1, nullptr, 0, (int32_t)BoneInfluenceMode::Num, sBoneInfluenceModeStrings));
    outProps.push_back(Property(DatumType::Integer, "Animation Update Mode", this, &mAnimationUpdateMode, 1, nullptr, 0, (int32_t)AnimationUpdateMode::Count, sAnimationUpdateModeStrings));
    outProps.push_back(Property(DatumType::Float, "Bounds Radius Override", this, &mBoundsRadiusOverride));
}

void SkeletalMeshComponent::Create()
{
    MeshComponent::Create();
    GFX_CreateSkeletalMeshCompResource(this);
    SetSkeletalMesh(GetDefaultMesh());

    if (mDefaultAnimation != "")
    {
        PlayAnimation(mDefaultAnimation.c_str(), true);
    }
}

void SkeletalMeshComponent::Destroy()
{
    MeshComponent::Destroy();
    GFX_DestroySkeletalMeshCompResource(this);
}

SkeletalMeshCompResource* SkeletalMeshComponent::GetResource()
{
    return &mResource;
}

void SkeletalMeshComponent::SaveStream(Stream& stream)
{
    MeshComponent::SaveStream(stream);
    stream.WriteAsset(mSkeletalMesh);
    stream.WriteString(mDefaultAnimation);
    stream.WriteFloat(mAnimationSpeed);
    stream.WriteBool(mAnimationPaused);
    //stream.WriteBool(mRevertToBindPose);
    //stream.WriteBool(mInheritPose);
    stream.WriteUint32(uint32_t(mBoneInfluenceMode));
    //stream.WriteUint32(uint32_t(mAnimationUpdateMode));
}

void SkeletalMeshComponent::LoadStream(Stream& stream)
{
    MeshComponent::LoadStream(stream);

    AssetRef meshRef;
    stream.ReadAsset(meshRef);
    if (meshRef.Get<SkeletalMesh>() == nullptr)
        meshRef = GetDefaultMesh();
    SetSkeletalMesh(meshRef.Get<SkeletalMesh>());

    stream.ReadString(mDefaultAnimation);
    mAnimationSpeed = stream.ReadFloat();
    mAnimationPaused = stream.ReadBool();
    //mRevertToBindPose = stream.ReadBool();
    //mInheritPose = stream.ReadBool();
    mBoneInfluenceMode = BoneInfluenceMode(stream.ReadUint32());
    //mAnimationUpdateMode = AnimationUpdateMode(stream.ReadUint32());

    if (mDefaultAnimation != "")
    {
        PlayAnimation(mDefaultAnimation.c_str(), true);
    }
}

void SkeletalMeshComponent::Tick(float deltaTime)
{
    MeshComponent::Tick(deltaTime);
    mHasAnimatedThisFrame = false;
}

bool SkeletalMeshComponent::IsStaticMeshComponent() const
{
    return false;
}

bool SkeletalMeshComponent::IsSkeletalMeshComponent() const
{
    return true;
}

void SkeletalMeshComponent::SetSkeletalMesh(SkeletalMesh* skeletalMesh)
{
    if (mSkeletalMesh.Get() != skeletalMesh)
    {
        mSkeletalMesh = skeletalMesh;

        if (skeletalMesh != nullptr)
        {
            if (GFX_IsCpuSkinningRequired(this))
            {
                // Initialize skinned vertex array
                mSkinnedVertices.resize(skeletalMesh->GetNumVertices());
                const std::vector<VertexSkinned> srcVerts = skeletalMesh->GetVertices();
                for (uint32_t i = 0; i < mSkinnedVertices.size(); ++i)
                {
                    mSkinnedVertices[i].mPosition = srcVerts[i].mPosition;
                    mSkinnedVertices[i].mNormal = srcVerts[i].mNormal;
                    mSkinnedVertices[i].mTexcoord0 = srcVerts[i].mTexcoord0;
                    mSkinnedVertices[i].mTexcoord1 = srcVerts[i].mTexcoord1;
                }

                GFX_ReallocateSkeletalMeshCompVertexBuffer(this, skeletalMesh->GetNumVertices());
                GFX_UpdateSkeletalMeshCompVertexBuffer(this, mSkinnedVertices);
            }

            // Resize and clear matrices to identity matrix.
            mBoneMatrices.resize(skeletalMesh->GetNumBones(), glm::mat4(1));
        }
        else
        {
            mBoneMatrices.resize(0);
        }
    }
}

SkeletalMesh* SkeletalMeshComponent::GetSkeletalMesh()
{
    return mSkeletalMesh.Get<SkeletalMesh>();
}

void SkeletalMeshComponent::PlayAnimation(const char* animName, bool loop, float speed, float weight, uint8_t priority)
{
    if (animName == nullptr ||
        animName[0] == '\0')
    {
        return;
    }

    ActiveAnimation* anim = FindActiveAnimation(animName);

    if (anim == nullptr)
    {
        // Animation isn't playing yet.
        uint32_t index = (uint32_t)priority;
        index = glm::min(index, (uint32_t)mActiveAnimations.size());
        mActiveAnimations.insert(mActiveAnimations.begin() + index, ActiveAnimation());

        anim = &mActiveAnimations[index];
    }

    OCT_ASSERT(anim != nullptr);

    anim->mName = animName;
    anim->mTime = loop ? anim->mTime : 0.0f;
    anim->mSpeed = speed;
    anim->mWeight = weight;
    anim->mLoop = loop;
}

void SkeletalMeshComponent::QueueAnimation(const char* animName, bool loop, const char* targetAnim, float speed, float weight, uint8_t priority)
{
    if (animName == nullptr ||
        animName[0] == '\0')
    {
        return;
    }

    ActiveAnimation* dependentAnim = nullptr;
    QueuedAnimation* dependentQueuedAnim = nullptr;

    if (targetAnim == nullptr)
    {
        if (mActiveAnimations.size() == 0)
        {
            // Default queued, just play it immediately.
            PlayAnimation(animName, loop, speed, weight, priority);
        }
        else
        {
            dependentAnim = &mActiveAnimations.back();
        }
    }
    else
    {
        dependentAnim = FindActiveAnimation(targetAnim);
    }

    // Check queued anims for dependent anim (so you could make a chain of animations if you needed).
    if (dependentAnim == nullptr)
    {
        dependentQueuedAnim = FindQueuedAnimation(targetAnim);
    }

    if (dependentAnim != nullptr ||
        dependentQueuedAnim != nullptr)
    {
        QueuedAnimation* queuedAnim = nullptr;

        {
            // dependName may be invalid if vector resize happens
            const char* dependName = dependentAnim ? dependentAnim->mName.c_str() : dependentQueuedAnim->mName.c_str();
            queuedAnim = FindQueuedAnimation(animName, dependName);
        }

        if (queuedAnim == nullptr)
        {
            // No queuedAnim exists yet so create a new one.
            mQueuedAnimations.push_back(QueuedAnimation());
            queuedAnim = &(mQueuedAnimations.back());
        }

        OCT_ASSERT(queuedAnim);

        queuedAnim->mName = animName;
        queuedAnim->mDependentAnim = dependentAnim ? dependentAnim->mName : dependentQueuedAnim->mName;
        queuedAnim->mLoop = loop;
        queuedAnim->mSpeed = speed;
        queuedAnim->mWeight = weight;
        queuedAnim->mPriority = priority;
    }
}

void SkeletalMeshComponent::StopAnimation(const char* animName, bool cancelQueued)
{
    for (uint32_t i = 0; i < mActiveAnimations.size(); ++i)
    {
        if (mActiveAnimations[i].mName == animName)
        {
            mActiveAnimations.erase(mActiveAnimations.begin() + i);
            break;
        }
    }

    if (cancelQueued)
    {
        // I think it's possible for multiple queued animations to exist with different dependent animations
        // So just to be safe, iterate through all queued vector.
        for (int32_t i = int32_t(mQueuedAnimations.size()) - 1; i >= 0; --i)
        {
            if (mQueuedAnimations[i].mName == animName)
            {
                mQueuedAnimations.erase(mQueuedAnimations.begin() + i);
            }
        }
    }
}

void SkeletalMeshComponent::StopAllAnimations(bool cancelQueued)
{
    mActiveAnimations.clear();

    if (cancelQueued)
    {
        mQueuedAnimations.clear();
    }
}

void SkeletalMeshComponent::CancelQueuedAnimation(const char* animName)
{
    for (int32_t i = int32_t(mQueuedAnimations.size()) - 1; i >= 0; --i)
    {
        if (mQueuedAnimations[i].mName == animName)
        {
            mQueuedAnimations.erase(mQueuedAnimations.begin() + i);
        }
    }
}

void SkeletalMeshComponent::CancelAllQueuedAnimations()
{
    mQueuedAnimations.clear();
}

bool SkeletalMeshComponent::IsAnimationPlaying(const char* animName)
{
    return (FindActiveAnimation(animName) != nullptr);
}

void SkeletalMeshComponent::ResetAnimation()
{
    // Not sure what a "Reset" function should do? Will probably remove this
    for (uint32_t i = 0; i < mActiveAnimations.size(); ++i)
    {
        mActiveAnimations[i].mTime = 0.0f;
    }
}

float SkeletalMeshComponent::GetAnimationSpeed() const
{
    return mAnimationSpeed;
}

void SkeletalMeshComponent::SetAnimationSpeed(float speed)
{
    mAnimationSpeed = speed;
}

void SkeletalMeshComponent::SetAnimationPaused(bool paused)
{
    mAnimationPaused = paused;
}

bool SkeletalMeshComponent::IsAnimationPaused() const
{
    return mAnimationPaused;
}

void SkeletalMeshComponent::SetInheritPose(bool inherit)
{
    mInheritPose = inherit;
}

bool SkeletalMeshComponent::IsInheritPoseEnabled() const
{
    return mInheritPose;
}

void SkeletalMeshComponent::SetBoundsRadiusOverride(float radius)
{
    mBoundsRadiusOverride = radius;
}

float SkeletalMeshComponent::GetBoundsRadiusOverride() const
{
    return mBoundsRadiusOverride;
}

bool SkeletalMeshComponent::HasAnimatedThisFrame() const
{
    return mHasAnimatedThisFrame;
}

ActiveAnimation* SkeletalMeshComponent::FindActiveAnimation(const char* animName)
{
    ActiveAnimation* anim = nullptr;

    for (uint32_t i = 0; i < mActiveAnimations.size(); ++i)
    {
        if (mActiveAnimations[i].mName == animName)
        {
            anim = &mActiveAnimations[i];
            break;
        }
    }

    return anim;
}

std::vector<ActiveAnimation>& SkeletalMeshComponent::GetActiveAnimations()
{
    return mActiveAnimations;
}

QueuedAnimation* SkeletalMeshComponent::FindQueuedAnimation(const char* animName, const char* dependName)
{
    QueuedAnimation* anim = nullptr;

    for (uint32_t i = 0; i < mQueuedAnimations.size(); ++i)
    {
        if (mQueuedAnimations[i].mName == animName &&
            (dependName == nullptr || mQueuedAnimations[i].mDependentAnim == dependName))
        {
            anim = &mQueuedAnimations[i];
            break;
        }
    }

    return anim;
}

std::vector<QueuedAnimation>& SkeletalMeshComponent::GetQueuedAnimations()
{
    return mQueuedAnimations;
}

int32_t SkeletalMeshComponent::FindBoneIndex(const std::string& name) const
{
    int32_t boneIndex = -1;
    SkeletalMesh* mesh = mSkeletalMesh.Get<SkeletalMesh>();

    if (mesh != nullptr)
    {
        boneIndex = mesh->FindBoneIndex(name);
    }

    return boneIndex;
}

void SkeletalMeshComponent::SetAnimEventHandler(AnimEventHandlerFP handler)
{
    mAnimEventHandler.mFuncPointer = handler;
}

AnimEventHandlerFP SkeletalMeshComponent::GetAnimEventHandler()
{
    return mAnimEventHandler.mFuncPointer;
}

void SkeletalMeshComponent::SetScriptAnimEventHandler(
    const char* tableName,
    const char* funcName)
{
    mAnimEventHandler.mScriptTableName = tableName;
    mAnimEventHandler.mScriptFuncName = funcName;
}

glm::vec3 SkeletalMeshComponent::InterpolateScale(float time, const Channel& channel)
{
    if (channel.mScaleKeys.size() == 1)
    {
        return channel.mScaleKeys[0].mValue;
    }

    uint32_t index = FindScaleIndex(time, channel);
    uint32_t nextIndex = index + 1;
    OCT_ASSERT(nextIndex < channel.mScaleKeys.size());

    float deltaTime = channel.mScaleKeys[nextIndex].mTime - channel.mScaleKeys[index].mTime;
    float factor = (time - channel.mScaleKeys[index].mTime) / deltaTime;
    factor = glm::clamp(factor, 0.0f, 1.0f);
    OCT_ASSERT(factor >= 0.0f && factor <= 1.0f);

    glm::vec3 startScale = channel.mScaleKeys[index].mValue;
    glm::vec3 endScale = channel.mScaleKeys[nextIndex].mValue;
    glm::vec3 retScale = glm::mix(startScale, endScale, factor);
    return retScale;
}

glm::quat SkeletalMeshComponent::InterpolateRotation(float time, const Channel& channel)
{
    if (channel.mRotationKeys.size() == 1)
    {
        return channel.mRotationKeys[0].mValue;
    }

    uint32_t index = FindRotationIndex(time, channel);
    uint32_t nextIndex = index + 1;
    OCT_ASSERT(nextIndex < channel.mRotationKeys.size());

    float deltaTime = channel.mRotationKeys[nextIndex].mTime - channel.mRotationKeys[index].mTime;
    float factor = (time - channel.mRotationKeys[index].mTime) / deltaTime;
    factor = glm::clamp(factor, 0.0f, 1.0f);
    OCT_ASSERT(factor >= 0.0f && factor <= 1.0f);

    glm::quat startQuat = channel.mRotationKeys[index].mValue;
    glm::quat endQuat = channel.mRotationKeys[nextIndex].mValue;
    glm::quat retQuat = glm::slerp(startQuat, endQuat, factor);
    retQuat = glm::normalize(retQuat);
    return retQuat;
}

glm::vec3 SkeletalMeshComponent::InterpolatePosition(float time, const Channel& channel)
{
    if (channel.mPositionKeys.size() == 1)
    {
        return channel.mPositionKeys[0].mValue;
    }

    uint32_t index = FindPositionIndex(time, channel);
    uint32_t nextIndex = index + 1;
    OCT_ASSERT(nextIndex < channel.mPositionKeys.size());

    float deltaTime = channel.mPositionKeys[nextIndex].mTime - channel.mPositionKeys[index].mTime;
    float factor = (time - channel.mPositionKeys[index].mTime) / deltaTime;
    factor = glm::clamp(factor, 0.0f, 1.0f);
    OCT_ASSERT(factor >= 0.0f && factor <= 1.0f);

    glm::vec3 startPos = channel.mPositionKeys[index].mValue;
    glm::vec3 endPos = channel.mPositionKeys[nextIndex].mValue;
    glm::vec3 retPos = glm::mix(startPos, endPos, factor);
    return retPos;
}

void SkeletalMeshComponent::DetectTriggeredAnimEvents(
    const Animation& animation,
    float prevTickTime,
    float tickTime,
    float animationSpeed,
    std::vector<AnimEvent>& outEvents)
{
    const std::vector<AnimEventTrack>& eventTracks = animation.mEventTracks;

    for (uint32_t trackIndex = 0; trackIndex < eventTracks.size(); ++trackIndex)
    {
        const std::vector<AnimEventKey>& keys = eventTracks[trackIndex].mEventKeys;
        for (uint32_t keyIndex = 0; keyIndex < keys.size(); ++keyIndex)
        {
            const AnimEventKey& key = keys[keyIndex];

            if ((animationSpeed > 0.0f && key.mTime > prevTickTime && key.mTime <= tickTime) ||
                (animationSpeed < 0.0f && key.mTime < prevTickTime && key.mTime >= tickTime) ||
                (animationSpeed > 0.0f && key.mTime > prevTickTime && tickTime < prevTickTime) || // Did we loop?
                (animationSpeed < 0.0f && key.mTime < prevTickTime && tickTime > prevTickTime))   // Did we loop?
            {
                AnimEvent newEvent;
                newEvent.mAnimation = animation.mName;
                newEvent.mName = eventTracks[trackIndex].mName;
                newEvent.mTime = key.mTime;
                newEvent.mValue = key.mValue;
                outEvents.push_back(newEvent);
            }
        }
    }
}

uint32_t SkeletalMeshComponent::FindScaleIndex(float time, const Channel& channel)
{
    OCT_ASSERT(channel.mScaleKeys.size() > 0);

    for (uint32_t i = 0; i < channel.mScaleKeys.size() - 1; ++i)
    {
        if (time < channel.mScaleKeys[i + 1].mTime)
        {
            return i;
        }
    }

    return uint32_t(channel.mScaleKeys.size() - 2);
}

uint32_t SkeletalMeshComponent::FindRotationIndex(float time, const Channel& channel)
{
    OCT_ASSERT(channel.mRotationKeys.size() > 0);

    for (uint32_t i = 0; i < channel.mRotationKeys.size() - 1; ++i)
    {
        if (time < channel.mRotationKeys[i + 1].mTime)
        {
            return i;
        }
    }

    return uint32_t(channel.mRotationKeys.size() - 2);
}

uint32_t SkeletalMeshComponent::FindPositionIndex(float time, const Channel& channel)
{
    OCT_ASSERT(channel.mPositionKeys.size() > 0);

    for (uint32_t i = 0; i < channel.mPositionKeys.size() - 1; ++i)
    {
        if (time < channel.mPositionKeys[i + 1].mTime)
        {
            return i;
        }
    }

    return uint32_t(channel.mPositionKeys.size() - 2);
}

glm::mat4 SkeletalMeshComponent::GetBoneTransform(const std::string& name) const
{
    int32_t index = FindBoneIndex(name);
    return GetBoneTransform(index);
}

glm::vec3 SkeletalMeshComponent::GetBonePosition(const std::string& name) const
{
    int32_t index = FindBoneIndex(name);
    return GetBonePosition(index);
}

glm::quat SkeletalMeshComponent::GetBoneRotationQuat(const std::string& name) const
{
    int32_t index = FindBoneIndex(name);
    return GetBoneRotationQuat(index);
}

glm::vec3 SkeletalMeshComponent::GetBoneRotationEuler(const std::string& name) const
{
    int32_t index = FindBoneIndex(name);
    return GetBoneRotationEuler(index);
}

glm::vec3 SkeletalMeshComponent::GetBoneScale(const std::string& name) const
{
    int32_t index = FindBoneIndex(name);
    return GetBoneScale(index);
}

glm::mat4 SkeletalMeshComponent::GetBoneTransform(int32_t index) const
{
    glm::mat4 retTransform;
    if (index >= 0 && index < (int32_t)mBoneMatrices.size())
    {
        retTransform = mBoneMatrices[index];
    }
    return retTransform;
}

glm::vec3 SkeletalMeshComponent::GetBonePosition(int32_t boneIndex) const
{
    glm::vec3 retPosition(0.0f, 0.0f, 0.0f);
    SkeletalMesh* mesh = mSkeletalMesh.Get<SkeletalMesh>();

    if (mesh != nullptr &&
        boneIndex >= 0 &&
        boneIndex < (int32_t)mBoneMatrices.size())
    {
        glm::mat4 offset = glm::inverse(mesh->GetBone(boneIndex).mOffsetMatrix);
        glm::mat4 transform = mTransform * mBoneMatrices[boneIndex] * offset;
        retPosition.x = transform[3][0];
        retPosition.y = transform[3][1];
        retPosition.z = transform[3][2];
    }

    return retPosition;
}

glm::quat SkeletalMeshComponent::GetBoneRotationQuat(int32_t boneIndex) const
{
    //TODO
    glm::quat retRotation(0.0f, 0.0f, 0.0f, 1.0f);

    SkeletalMesh* mesh = mSkeletalMesh.Get<SkeletalMesh>();
    if (mesh != nullptr &&
        boneIndex >= 0 &&
        boneIndex < (int32_t)mBoneMatrices.size())
    {
        glm::mat4 offset = glm::inverse(mesh->GetBone(boneIndex).mOffsetMatrix);
        glm::mat4 transform = mTransform * mBoneMatrices[boneIndex] * offset;

        retRotation = Maths::ExtractRotation(transform);
    }

    return retRotation;
}

glm::vec3 SkeletalMeshComponent::GetBoneRotationEuler(int32_t boneIndex) const
{
    //TODO
    glm::vec3 retRotation(0.0f, 0.0f, 0.0f);

    glm::quat retQuat = GetBoneRotationQuat(boneIndex);
    retRotation = glm::eulerAngles(retQuat) * RADIANS_TO_DEGREES;

    return retRotation;
}

glm::vec3 SkeletalMeshComponent::GetBoneScale(int32_t boneIndex) const
{
    //TODO
    LogWarning("SkeletalMeshComponent::GetBoneScale() not yet implemented");
    return glm::vec3(0, 0, 0);
}

void SkeletalMeshComponent::SetBoneTransform(int32_t boneIndex, const glm::mat4& transform)
{
    // TODO: This function probably isn't working as intended. Please fix.
#if 1
    OCT_ASSERT(boneIndex >= 0 && boneIndex < int32_t(mBoneMatrices.size()));

    SkeletalMesh* mesh = mSkeletalMesh.Get<SkeletalMesh>();
    const std::vector<Bone>& bones = mesh->GetBones();
    mBoneMatrices[boneIndex] = bones[boneIndex].mInvOffsetMatrix * transform * bones[boneIndex].mOffsetMatrix;
#else
    // I attempted to recreate the way I am creating the bone transforms in SkeletalMesh::AnimateBones
    // but this just isn't working. Not sure what I'm doing wrong.
    SkeletalMesh* mesh = mSkeletalMesh.GetSkeletalMesh();
    if (mesh != nullptr)
    {
        OCT_ASSERT(boneIndex >= 0 && boneIndex < int32_t(mBoneMatrices.size()));
        mBoneMatrices[boneIndex] = transform;

        const std::vector<Bone>& bones = mesh->GetBones();

        int32_t parentIndex = bones[boneIndex].mParentIndex;
        if (parentIndex != -1)
        {
            mBoneMatrices[boneIndex] = mBoneMatrices[parentIndex] * mBoneMatrices[boneIndex];
        }

        mBoneMatrices[boneIndex] = mesh->GetInvRootTransform() * mBoneMatrices[boneIndex] * bones[boneIndex].mOffsetMatrix;
    }
#endif
}

void SkeletalMeshComponent::SetBonePosition(int32_t boneIndex, glm::vec3 position)
{
    LogWarning("SkeletalMeshComponent::SetBonePosition() not yet implemented");
}

void SkeletalMeshComponent::SetBoneRotation(int32_t boneIndex, glm::vec3 rotation)
{
    LogWarning("SkeletalMeshComponent::SetBoneRotation() not yet implemented");
}

void SkeletalMeshComponent::SetBoneScale(int32_t boneIndex, glm::vec2 scale)
{
    LogWarning("SkeletalMeshComponent::SetBoneScale() not yet implemented");
}

uint32_t SkeletalMeshComponent::GetNumBones() const
{
    return uint32_t(mBoneMatrices.size());
}

BoneInfluenceMode SkeletalMeshComponent::GetBoneInfluenceMode() const
{
    return mBoneInfluenceMode;
}

AnimationUpdateMode SkeletalMeshComponent::GetAnimationUpdateMode() const
{
    return mAnimationUpdateMode;
}

void SkeletalMeshComponent::SetAnimationUpdateMode(AnimationUpdateMode mode)
{
    mAnimationUpdateMode = mode;
}

Vertex* SkeletalMeshComponent::GetSkinnedVertices()
{
    return mSkinnedVertices.data();
}

uint32_t SkeletalMeshComponent::GetNumSkinnedVertices()
{
    return (uint32_t) mSkinnedVertices.size();
}

Material* SkeletalMeshComponent::GetMaterial()
{
    Material* mat = mMaterialOverride.Get<Material>();

    if (!mat && mSkeletalMesh.Get())
    {
        mat = mSkeletalMesh.Get<SkeletalMesh>()->GetMaterial();
    }

    return mat;
}

void SkeletalMeshComponent::Render()
{
    GFX_DrawSkeletalMeshComp(this);
}

Bounds SkeletalMeshComponent::GetLocalBounds() const
{
    Bounds retBounds;
    if (mSkeletalMesh != nullptr)
    {
        retBounds = mSkeletalMesh.Get<SkeletalMesh>()->GetBounds();

    }
    else
    {
        retBounds = MeshComponent::GetLocalBounds();
    }

    if (mBoundsRadiusOverride > 0.0f)
    {
        retBounds.mRadius = mBoundsRadiusOverride;
    }

    return retBounds;
}

void SkeletalMeshComponent::UpdateAnimation(float deltaTime, bool updateBones)
{
    if (mHasAnimatedThisFrame)
        return;

    static std::vector<AnimEvent> sAnimEvents;
    sAnimEvents.clear();

    SkeletalMesh* mesh = mSkeletalMesh.Get<SkeletalMesh>();

    bool inheritPose = mInheritPose && 
        updateBones &&
        mParent != nullptr && 
        mParent->GetType() == SkeletalMeshComponent::GetStaticType();

    if (inheritPose &&
        mesh != nullptr)
    {
        // It's definitely possible to make this better by referencing the parent bone transforms
        // instead of copying them, but that would probably require some refactoring in the GFX layer.
        SkeletalMeshComponent* parentMesh = mParent->As<SkeletalMeshComponent>();

        if (!parentMesh->mHasAnimatedThisFrame)
        {
            parentMesh->UpdateAnimation(deltaTime, true);
        }

        uint32_t numBones = parentMesh->GetNumBones();

        // Inherited mesh should have the same skeleton.
        if (numBones == GetNumBones())
        {
            for (uint32_t i = 0; i < numBones; ++i)
            {
                mBoneMatrices[i] = parentMesh->GetBoneTransform((int32_t)i);
            }
        }
        else
        {
            LogError("Cannot inherit pose because number of bones differs between components.");
            OCT_ASSERT(0);
        }
    }

    if (mesh != nullptr &&
        !mAnimationPaused && 
        !inheritPose && 
        (mActiveAnimations.size() > 0 || mRevertToBindPose))
    {
        if (updateBones)
        {
            mesh->CopyBindPose(mBoneMatrices);
        }

        for (int32_t i = 0; i < (int32_t)mActiveAnimations.size(); ++i)
        {
            const Animation* anim = mesh->GetAnimation(mActiveAnimations[i].mName.c_str());
            bool animFinished = false;

            if (anim != nullptr)
            {
                float prevAnimTime = mActiveAnimations[i].mTime;
                float& animationTime = mActiveAnimations[i].mTime;
                float animationSpeed = mActiveAnimations[i].mSpeed * mAnimationSpeed;
                animationTime += (deltaTime * animationSpeed);

                float weight = mActiveAnimations[i].mWeight;
                weight = glm::clamp(weight, 0.0f, 1.0f);

                if (anim != nullptr)
                {
                    // Mod the animation time to fit within the animation duration
                    const float durationSeconds = anim->mDuration / anim->mTicksPerSecond;
                    const float prevTickTime = prevAnimTime * anim->mTicksPerSecond;

                    if (mActiveAnimations[i].mLoop)
                    {
                        if (animationSpeed > 0.0f &&
                            animationTime > durationSeconds)
                        {
                            animationTime = fmod(animationTime - durationSeconds, durationSeconds);
                        }
                        else if (animationSpeed < 0.0f &&
                                 animationTime < 0.0f)
                        {
                            animationTime = fmod(durationSeconds + animationTime, durationSeconds);
                        }
                    }
                    else
                    {
                        if (animationTime > durationSeconds)
                        {
                            animFinished = true;
                            animationTime = durationSeconds;
                        }
                    }

                    if (weight > 0.0f)
                    {
                        float tickTime = animationTime * anim->mTicksPerSecond;

                        if (updateBones)
                        {
                            // Go through all the channels, and update the relative transform 
                            // for each bone that exists in the animation.
                            for (uint32_t i = 0; i < anim->mChannels.size(); ++i)
                            {
                                int32_t boneIndex = anim->mChannels[i].mBoneIndex;
                                OCT_ASSERT(boneIndex != -1 &&
                                    boneIndex >= 0 &&
                                    boneIndex < (int32_t)mesh->GetBones().size());

                                if (boneIndex != -1)
                                {
                                    glm::vec3 scale = InterpolateScale(tickTime, anim->mChannels[i]);
                                    glm::quat rotation = InterpolateRotation(tickTime, anim->mChannels[i]);
                                    glm::vec3 position = InterpolatePosition(tickTime, anim->mChannels[i]);

                                    glm::mat4 transform = glm::mat4(1.0f);

                                    transform = glm::translate(transform, position);
                                    transform *= glm::toMat4(rotation);
                                    transform = glm::scale(transform, scale);

                                    mBoneMatrices[boneIndex] = (weight * transform) + ((1.0f - weight) * mBoneMatrices[boneIndex]);
                                }
                            }
                        }

                        if (anim->mEventTracks.size() > 0)
                        {
                            DetectTriggeredAnimEvents(*anim, prevTickTime, tickTime, animationSpeed, sAnimEvents);
                        }
                    }
                }
            }
            else
            {
                LogWarning("Invalid animation name \"%s\" received in SkeletalMesh::AnimateBones()", mActiveAnimations[i].mName.c_str());
                animFinished = true;
            }

            if (animFinished)
            {
                const std::string& animName = mActiveAnimations[i].mName;

                for (int32_t q = int32_t(mQueuedAnimations.size()) - 1; q >= 0; --q)
                {
                    if (mQueuedAnimations[q].mDependentAnim == animName)
                    {
                        QueuedAnimation& queuedAnim = mQueuedAnimations[q];
                        PlayAnimation(
                            queuedAnim.mName.c_str(),
                            queuedAnim.mLoop,
                            queuedAnim.mSpeed,
                            queuedAnim.mWeight,
                            queuedAnim.mPriority);

                        mQueuedAnimations.erase(mQueuedAnimations.begin() + q);
                    }
                }

                mActiveAnimations.erase(mActiveAnimations.begin() + i);
                --i;
            }
        }

        if (updateBones)
        {
            mesh->FinalizeBoneTransforms(mBoneMatrices);

            if (GFX_IsCpuSkinningRequired(this))
            {
                CpuSkinVertices();
            }
        }

        // Fire off any events that triggered.
        if (mAnimEventHandler.mFuncPointer != nullptr)
        {
            Actor* actor = GetOwner();
            for (uint32_t i = 0; i < sAnimEvents.size(); ++i)
            {
                sAnimEvents[i].mActor = actor;
                sAnimEvents[i].mComponent = this;
                mAnimEventHandler.mFuncPointer(sAnimEvents[i]);
            }
        }
        if (mAnimEventHandler.mScriptTableName != "")
        {
            Actor* actor = GetOwner();
            for (uint32_t i = 0; i < sAnimEvents.size(); ++i)
            {
                sAnimEvents[i].mActor = actor;
                sAnimEvents[i].mComponent = this;

                ScriptEvent::Animation(
                    mAnimEventHandler.mScriptTableName,
                    mAnimEventHandler.mScriptFuncName,
                    sAnimEvents[i]);
            }
        }
    }

    if (inheritPose &&
        mesh != nullptr &&
        !mAnimationPaused &&
        GFX_IsCpuSkinningRequired(this))
    {
        CpuSkinVertices();
    }

    if (updateBones)
    {
        UpdateAttachedChildren(deltaTime);
    }

    mHasAnimatedThisFrame = true;
}

void SkeletalMeshComponent::UpdateAttachedChildren(float deltaTime)
{
    bool isAnimating = !mAnimationPaused &&
        (mAnimationSpeed != 0.0f) &&
        mActiveAnimations.size() > 0;

    if (isAnimating)
    {
        for (uint32_t i = 0; i < mChildren.size(); ++i)
        {
            if (mChildren[i]->GetParentBoneIndex() != -1)
            {
                mChildren[i]->MarkTransformDirty();
            }
        }
    }
}

void SkeletalMeshComponent::CpuSkinVertices()
{
    SkeletalMesh* mesh = mSkeletalMesh.Get<SkeletalMesh>();
    if (mesh != nullptr)
    {
        mSkinnedVertices.resize(mesh->GetNumVertices());
        const std::vector<VertexSkinned>& verts = mesh->GetVertices();

        if (mBoneInfluenceMode == BoneInfluenceMode::One)
        {
            for (uint32_t i = 0; i < mesh->GetNumVertices(); ++i)
            {
                const VertexSkinned& srcVert = verts[i];
                Vertex& dstVert = mSkinnedVertices[i];

                glm::mat4 transform = mBoneMatrices[srcVert.mBoneIndices[0]];

                dstVert.mPosition = transform * glm::vec4(srcVert.mPosition, 1.0f);
                dstVert.mNormal = transform * glm::vec4(srcVert.mNormal, 0.0f);
                dstVert.mTexcoord0 = srcVert.mTexcoord0;
                dstVert.mTexcoord1 = srcVert.mTexcoord1;
            }
        }
        else
        {
            for (uint32_t i = 0; i < mesh->GetNumVertices(); ++i)
            {
                const VertexSkinned& srcVert = verts[i];
                Vertex& dstVert = mSkinnedVertices[i];

                static_assert(MAX_BONE_INFLUENCES == 4, "Need to adjust this code or convert to loop.");
                glm::mat4 transform = mBoneMatrices[srcVert.mBoneIndices[0]] * srcVert.mBoneWeights[0];
                transform += mBoneMatrices[srcVert.mBoneIndices[1]] * srcVert.mBoneWeights[1];
                transform += mBoneMatrices[srcVert.mBoneIndices[2]] * srcVert.mBoneWeights[2];
                transform += mBoneMatrices[srcVert.mBoneIndices[3]] * srcVert.mBoneWeights[3];

                dstVert.mPosition = transform * glm::vec4(srcVert.mPosition, 1.0f);
                dstVert.mNormal = transform * glm::vec4(srcVert.mNormal, 0.0f);
                dstVert.mTexcoord0 = srcVert.mTexcoord0;
                dstVert.mTexcoord1 = srcVert.mTexcoord1;
            }
        }

        GFX_UpdateSkeletalMeshCompVertexBuffer(this, mSkinnedVertices);
    }
}
