#include "Timeline/Tracks/AudioTrack.h"
#include "Timeline/Tracks/AudioClip.h"
#include "Timeline/TimelineInstance.h"
#include "Nodes/3D/Audio3d.h"
#include "AudioManager.h"
#include "Utilities.h"

FORCE_LINK_DEF(AudioTrack);
DEFINE_TRACK(AudioTrack);

AudioTrack::AudioTrack()
{
}

AudioTrack::~AudioTrack()
{
}

void AudioTrack::Evaluate(float time, Node* target, TimelineInstance* inst)
{
    if (target == nullptr)
        return;

    Audio3D* audioNode = target->As<Audio3D>();
    if (audioNode == nullptr)
        return;

    bool anyClipActive = false;

    for (uint32_t i = 0; i < mClips.size(); ++i)
    {
        if (mClips[i]->GetType() != AudioClip::GetStaticType())
            continue;

        AudioClip* clip = static_cast<AudioClip*>(mClips[i]);

        if (clip->ContainsTime(time))
        {
            SoundWave* soundWave = clip->GetSoundWave().Get<SoundWave>();
            if (soundWave == nullptr)
                continue;

            anyClipActive = true;
            audioNode->SetSoundWave(soundWave);
            audioNode->SetLoop(clip->GetLoop());

            float volume = clip->GetVolume();

            // Apply fade envelope
            float localTime = clip->GetLocalTime(time);
            float fadeDur = clip->GetFadeInDuration();
            if (fadeDur > 0.0f && localTime < fadeDur)
            {
                volume *= (localTime / fadeDur);
            }

            float fadeOut = clip->GetFadeOutDuration();
            float timeToEnd = clip->GetEndTime() - time;
            if (fadeOut > 0.0f && timeToEnd < fadeOut)
            {
                volume *= (timeToEnd / fadeOut);
            }

            audioNode->SetVolume(volume);
            audioNode->SetPitch(clip->GetPitch());

            if (!audioNode->IsPlaying())
            {
                audioNode->PlayAudio();
            }
        }
    }

    if (!anyClipActive && audioNode->IsPlaying())
    {
        bool shouldStop = true;

        for (uint32_t i = 0; i < mClips.size(); ++i)
        {
            if (mClips[i]->GetType() != AudioClip::GetStaticType())
                continue;

            AudioClip* clip = static_cast<AudioClip*>(mClips[i]);

            if (time > clip->GetEndTime() && clip->GetEndMode() == AudioClipEndMode::Continue)
            {
                shouldStop = false;
                break;
            }
        }

        if (shouldStop)
        {
            audioNode->StopAudio();
        }
    }
}

void AudioTrack::Reset(Node* target, TimelineInstance* inst)
{
    if (target == nullptr)
        return;

    Audio3D* audioNode = target->As<Audio3D>();
    if (audioNode != nullptr)
    {
        audioNode->StopAudio();
    }
}

glm::vec4 AudioTrack::GetTrackColor() const
{
    return glm::vec4(0.3f, 0.8f, 0.3f, 1.0f);
}

TypeId AudioTrack::GetDefaultClipType() const
{
    return AudioClip::GetStaticType();
}
