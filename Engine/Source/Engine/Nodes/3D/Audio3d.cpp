#include "Nodes/3D/Audio3d.h"

#include "Engine.h"
#include "World.h"
#include "Log.h"

#include "AudioManager.h"
#include "AssetManager.h"

#if EDITOR
#include "EditorState.h"
#endif

static const char* sAttenuationFuncStrings[] =
{
    "Constant",
    "Linear"
};
static_assert(int32_t(AttenuationFunc::Count) == 2, "Need to update string conversion table");

FORCE_LINK_DEF(Audio3D);
DEFINE_NODE(Audio3D, Node3D);

bool Audio3D::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);
    OCT_ASSERT(prop != nullptr);
    Audio3D* comp = static_cast<Audio3D*>(prop->mOwner);
    bool success = false;

    if (prop->mName == "Play")
    {
        if (*(bool*)newValue)
        {
            comp->PlayAudio();
        }
        else
        {
            comp->StopAudio();
        }
        
        success = true;
    }

    return success;
}

Audio3D::Audio3D()
{
    mName = "Audio";
}

Audio3D::~Audio3D()
{

}

const char* Audio3D::GetTypeName() const
{
    return "Audio";
}

void Audio3D::GatherProperties(std::vector<Property>& outProps)
{
    Node3D::GatherProperties(outProps);

    SCOPED_CATEGORY("Audio");

    // Dummy Play property
    outProps.push_back(Property(DatumType::Bool, "Play", this, &mPlaying, 1, HandlePropChange));

    outProps.push_back(Property(DatumType::Asset, "Sound Wave", this, &mSoundWave, 1, nullptr, int32_t(SoundWave::GetStaticType())));
    outProps.push_back(Property(DatumType::Float, "Inner Radius", this, &mInnerRadius));
    outProps.push_back(Property(DatumType::Float, "Outer Radius", this, &mOuterRadius));
    outProps.push_back(Property(DatumType::Float, "Volume", this, &mVolume));
    outProps.push_back(Property(DatumType::Float, "Pitch", this, &mPitch));
    outProps.push_back(Property(DatumType::Float, "Start Offset", this, &mStartOffset));
    outProps.push_back(Property(DatumType::Integer, "Priority", this, &mPriority));
    outProps.push_back(Property(DatumType::Integer, "Attenuation Func", this, &mAttenuationFunc, 1, nullptr, 0, int32_t(AttenuationFunc::Count), sAttenuationFuncStrings));
    outProps.push_back(Property(DatumType::Byte, "Audio Class", this, &mAudioClass));
    outProps.push_back(Property(DatumType::Bool, "Loop", this, &mLoop));
    outProps.push_back(Property(DatumType::Bool, "Auto Play", this, &mAutoPlay));
}

void Audio3D::GatherProxyDraws(std::vector<DebugDraw>& inoutDraws)
{
#if DEBUG_DRAW_ENABLED

    if (GetType() == Audio3D::GetStaticType())
    {
        {
            DebugDraw debugDraw;
            debugDraw.mMesh = LoadAsset<StaticMesh>("SM_Sphere");
            debugDraw.mNode = this;
            debugDraw.mColor = glm::vec4(0.3f, 0.8f, 0.8f, 1.0f);
            debugDraw.mTransform = glm::scale(mTransform, { 0.2f, 0.2f, 0.2f });
            inoutDraws.push_back(debugDraw);
        }

#if EDITOR
        if (GetEditorState()->GetSelectedNode() == this)
        {
            {
                // Inner Radius
                DebugDraw debugDraw;
                debugDraw.mMesh = LoadAsset<StaticMesh>("SM_Sphere");
                debugDraw.mNode = this;
                debugDraw.mColor = glm::vec4(0.3f, 0.8f, 0.8f, 1.0f);
                debugDraw.mTransform = glm::scale(mTransform, { mInnerRadius, mInnerRadius, mInnerRadius });
                inoutDraws.push_back(debugDraw);
            }

            {
                // Outer Radius
                DebugDraw debugDraw;
                debugDraw.mMesh = LoadAsset<StaticMesh>("SM_Sphere");
                debugDraw.mNode = this;
                debugDraw.mColor = glm::vec4(0.3f, 0.8f, 0.8f, 1.0f);
                debugDraw.mTransform = glm::scale(mTransform, { mOuterRadius, mOuterRadius, mOuterRadius });
                inoutDraws.push_back(debugDraw);
            }
        }
#endif // EDITOR
    }
#endif // DEBUG_DRAW_ENABLED
}

void Audio3D::Create()
{
    Node3D::Create();
}

void Audio3D::Destroy()
{
    AudioManager::StopComponent(this);
    Node3D::Destroy();
}

void Audio3D::Start()
{
    Node3D::Start();

    if (mAutoPlay)
    {
        PlayAudio();
    }
}

void Audio3D::Tick(float deltaTime)
{
    Node3D::Tick(deltaTime);
    TickCommon(deltaTime);
}

void Audio3D::EditorTick(float deltaTime)
{
    Node3D::EditorTick(deltaTime);
    TickCommon(deltaTime);
}

void Audio3D::TickCommon(float deltaTime)
{
    if (mPlaying)
    {
        mPlayTime += deltaTime;
    }
}

void Audio3D::SetSoundWave(SoundWave* soundWave)
{
    mSoundWave = soundWave;
}

SoundWave* Audio3D::GetSoundWave()
{
    return mSoundWave.Get<SoundWave>();
}

void Audio3D::SetInnerRadius(float innerRadius)
{
    mInnerRadius = innerRadius;
}

float Audio3D::GetInnerRadius() const
{
    return mInnerRadius;
}

void Audio3D::SetOuterRadius(float outerRadius)
{
    mOuterRadius = outerRadius;
}

float Audio3D::GetOuterRadius() const
{
    return mOuterRadius;
}

void Audio3D::SetVolume(float volume)
{
    mVolume = volume;
}

float Audio3D::GetVolume() const
{
    return mVolume;
}

void Audio3D::SetPitch(float pitch)
{
    mPitch = pitch;
}

float Audio3D::GetPitch() const
{
    return mPitch;
}

void Audio3D::SetStartOffset(float startOffset)
{
    mStartOffset = startOffset;
}

float Audio3D::GetStartOffset() const
{
    return mStartOffset;
}

void Audio3D::SetPriority(int32_t priority)
{
    mPriority = priority;
}

int32_t Audio3D::GetPriority() const
{
    return mPriority;
}

void Audio3D::SetAttenuationFunc(AttenuationFunc func)
{
    mAttenuationFunc = func;
}

AttenuationFunc Audio3D::GetAttenuationFunc() const
{
    return mAttenuationFunc;
}

void Audio3D::SetAudioClass(int8_t audioClass)
{
    mAudioClass = audioClass;
}

int8_t Audio3D::GetAudioClass() const
{
    int8_t audioClass = 0;
    if (mAudioClass < 0)
    {
        // Defer to sound wave property
        if (mSoundWave != nullptr)
        {
            audioClass = mSoundWave.Get<SoundWave>()->GetAudioClass();
        }
    }
    else
    {
        audioClass = mAudioClass;
    }

    return audioClass;
}

void Audio3D::SetLoop(bool loop)
{
    mLoop = loop;
}

bool Audio3D::GetLoop() const
{
    return mLoop;
}


void Audio3D::SetAutoPlay(bool autoPlay)
{
    mAutoPlay = autoPlay;
}

bool Audio3D::GetAutoPlay() const
{
    return mAutoPlay;
}

float Audio3D::GetPlayTime() const
{
    return mPlayTime;
}

bool Audio3D::IsPlaying() const
{
    return mPlaying;
}

bool Audio3D::IsAudible() const
{
    return mAudible;
}

void Audio3D::PlayAudio()
{
    mPlaying = true;
}

void Audio3D::PauseAudio()
{
    mPlaying = false;
}

void Audio3D::ResetAudio()
{
    // Forcibly remove the audio
    AudioManager::StopComponent(this);
    mPlayTime = 0.0f;
}

void Audio3D::StopAudio()
{
    mPlaying = false;
    mPlayTime = 0.0f;
}

void Audio3D::NotifyAudible(bool audible)
{
    mAudible = audible;
}
