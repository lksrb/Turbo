#include "tbopch.h"
#include "AudioClip.h"

#include "AudioEngine.h"

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

}
