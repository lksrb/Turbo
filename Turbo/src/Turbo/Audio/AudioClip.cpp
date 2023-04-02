#include "tbopch.h"
#include "AudioClip.h"

#include "Audio.h"

namespace Turbo
{
    AudioClip::AudioClip(const std::string& filepath, bool playOnStart)
        : m_Filepath(filepath), m_PlayOnStart(playOnStart)
    {
        m_AudioFile.Load(filepath);
        if (!m_AudioFile)
            TBO_ENGINE_ERROR("Could not load the file!");
    }

    AudioClip::~AudioClip()
    {
    }

    Ref<AudioClip> AudioClip::Create(const std::string& filepath, bool playOnStart)
    {
        Ref<AudioClip> audioClip = Ref<AudioClip>::Create(filepath, playOnStart);
        if (audioClip->m_AudioFile)
            Audio::RegisterAudioClip(audioClip);
        return audioClip;
    }

}
