#include "Components/AudioComponent.h"

#include "Engine.h"
#include "World.h"
#include "Log.h"

#include "AudioManager.h"
#include "AssetManager.h"

static const char* sAttenuationFuncStrings[] =
{
    "Constant",
    "Linear"
};
static_assert(int32_t(AttenuationFunc::Count) == 2, "Need to update string conversion table");

FORCE_LINK_DEF(AudioComponent);
DEFINE_COMPONENT(AudioComponent);

static bool HandlePropChange(Datum* datum, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);
    assert(prop != nullptr);
    AudioComponent* comp = static_cast<AudioComponent*>(prop->mOwner);
    bool success = false;

    if (prop->mName == "Play")
    {
        if (*(bool*)newValue)
        {
            comp->Play();
        }
        else
        {
            comp->Stop();
        }
        
        success = true;
    }

    return success;
}

AudioComponent::AudioComponent()
{
    mName = "Audio Component";
}

AudioComponent::~AudioComponent()
{

}

const char* AudioComponent::GetTypeName() const
{
    return "Audio";
}

void AudioComponent::GatherProperties(std::vector<Property>& outProps)
{
    TransformComponent::GatherProperties(outProps);

    // Dummy Play property
    outProps.push_back(Property(DatumType::Bool, "Play", this, &mPlaying, 1, HandlePropChange));

    outProps.push_back(Property(DatumType::Asset, "Sound Wave", this, &mSoundWave, 1, nullptr, int32_t(SoundWave::GetStaticType())));
    outProps.push_back(Property(DatumType::Float, "Inner Radius", this, &mInnerRadius));
    outProps.push_back(Property(DatumType::Float, "Outer Radius", this, &mOuterRadius));
    outProps.push_back(Property(DatumType::Float, "Volume", this, &mVolume));
    outProps.push_back(Property(DatumType::Float, "Pitch", this, &mPitch));
    outProps.push_back(Property(DatumType::Float, "Start Offset", this, &mStartOffset));
    outProps.push_back(Property(DatumType::Integer, "Priority", this, &mPriority));
    outProps.push_back(Property(DatumType::Enum, "Attenuation Func", this, &mAttenuationFunc, 1, nullptr, 0, int32_t(AttenuationFunc::Count), sAttenuationFuncStrings));
    outProps.push_back(Property(DatumType::Bool, "Loop", this, &mLoop));
    outProps.push_back(Property(DatumType::Bool, "Auto Play", this, &mAutoPlay));
}

void AudioComponent::GatherProxyDraws(std::vector<DebugDraw>& inoutDraws)
{
#if DEBUG_DRAW_ENABLED
    TransformComponent::GatherProxyDraws(inoutDraws);

    if (GetType() == AudioComponent::GetStaticType())
    {
        DebugDraw debugDraw;
        debugDraw.mMesh = LoadAsset<StaticMesh>("SM_Sphere");
        debugDraw.mActor = GetOwner();
        debugDraw.mColor = glm::vec4(0.3f, 0.8f, 0.8f, 1.0f);
        debugDraw.mTransform = glm::scale(mTransform, { 0.2f, 0.2f, 0.2f });
        inoutDraws.push_back(debugDraw);
    }
#endif
}

void AudioComponent::Create()
{
    TransformComponent::Create();
}

void AudioComponent::Destroy()
{
    AudioManager::StopComponent(this);
    TransformComponent::Destroy();
}

void AudioComponent::Tick(float deltaTime)
{
    TransformComponent::Tick(deltaTime);

    if (mPlaying)
    {
        mPlayTime += deltaTime;
    }
}

void AudioComponent::SaveStream(Stream& stream)
{
    TransformComponent::SaveStream(stream);

    stream.WriteAsset(mSoundWave);
    stream.WriteFloat(mInnerRadius);
    stream.WriteFloat(mOuterRadius);
    stream.WriteFloat(mVolume);
    stream.WriteFloat(mPitch);
    stream.WriteFloat(mStartOffset);
    stream.WriteInt32(mPriority);
    stream.WriteUint32((uint32_t)mAttenuationFunc);
    stream.WriteBool(mLoop);
    stream.WriteBool(mAutoPlay);
}

void AudioComponent::LoadStream(Stream& stream)
{
    TransformComponent::LoadStream(stream);

    stream.ReadAsset(mSoundWave);
    mInnerRadius = stream.ReadFloat();
    mOuterRadius = stream.ReadFloat();
    mVolume = stream.ReadFloat();
    mPitch = stream.ReadFloat();
    mStartOffset = stream.ReadFloat();
    mPriority = stream.ReadInt32();
    mAttenuationFunc = (AttenuationFunc)stream.ReadUint32();
    mLoop = stream.ReadBool();
    mAutoPlay = stream.ReadBool();

    if (mAutoPlay)
    {
        Play();
    }
}

void AudioComponent::SetSoundWave(SoundWave* soundWave)
{
    mSoundWave = soundWave;
}

SoundWave* AudioComponent::GetSoundWave()
{
    return mSoundWave.Get<SoundWave>();
}

void AudioComponent::SetInnerRadius(float innerRadius)
{
    mInnerRadius = innerRadius;
}

float AudioComponent::GetInnerRadius() const
{
    return mInnerRadius;
}

void AudioComponent::SetOuterRadius(float outerRadius)
{
    mOuterRadius = outerRadius;
}

float AudioComponent::GetOuterRadius() const
{
    return mOuterRadius;
}

void AudioComponent::SetVolume(float volume)
{
    mVolume = volume;
}

float AudioComponent::GetVolume() const
{
    return mVolume;
}

void AudioComponent::SetPitch(float pitch)
{
    mPitch = pitch;
}

float AudioComponent::GetPitch() const
{
    return mPitch;
}

void AudioComponent::SetStartOffset(float startOffset)
{
    mStartOffset = startOffset;
}

float AudioComponent::GetStartOffset() const
{
    return mStartOffset;
}

void AudioComponent::SetPriority(int32_t priority)
{
    mPriority = priority;
}

int32_t AudioComponent::GetPriority() const
{
    return mPriority;
}

void AudioComponent::SetAttenuationFunc(AttenuationFunc func)
{
    mAttenuationFunc = func;
}

AttenuationFunc AudioComponent::GetAttenuationFunc() const
{
    return mAttenuationFunc;
}

void AudioComponent::SetLoop(bool loop)
{
    mLoop = loop;
}

bool AudioComponent::GetLoop() const
{
    return mLoop;
}


void AudioComponent::SetAutoPlay(bool autoPlay)
{
    mAutoPlay = autoPlay;
}

bool AudioComponent::GetAutoPlay() const
{
    return mAutoPlay;
}

float AudioComponent::GetPlayTime() const
{
    return mPlayTime;
}

bool AudioComponent::IsPlaying() const
{
    return mPlaying;
}

bool AudioComponent::IsAudible() const
{
    return mAudible;
}

void AudioComponent::Play()
{
    mPlaying = true;
}

void AudioComponent::Pause()
{
    mPlaying = false;
}

void AudioComponent::Reset()
{
    // Forcibly remove the audio
    AudioManager::StopComponent(this);
    mPlayTime = 0.0f;
}

void AudioComponent::Stop()
{
    mPlaying = false;
    mPlayTime = 0.0f;
}

void AudioComponent::NotifyAudible(bool audible)
{
    mAudible = audible;
}
