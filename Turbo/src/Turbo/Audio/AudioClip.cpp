#include "tbopch.h"
#include "AudioClip.h"

#include "Audio.h"

namespace Turbo
{
    AudioClip::AudioClip(const std::string& filepath)
        : m_Filepath(filepath)
    {
        m_AudioFile.Load(filepath);
        if (!m_AudioFile)
            TBO_ENGINE_ERROR("Could not load the file!");
    }

    AudioClip::~AudioClip()
    {
    }

    Ref<AudioClip> AudioClip::Create(const std::string& filepath)
    {
        Ref<AudioClip> audioClip = Ref<AudioClip>::Create(filepath);
        if (audioClip->m_AudioFile)
            Audio::RegisterAudioClip(audioClip);
        return audioClip;
    }

}
