#pragma once

#include "Turbo/Core/Common.h"

#include "AudioFile.h"

namespace Turbo
{
    // High-level wrapper around a single audio file
    class AudioClip
    {
    public:
        AudioClip(const std::string& filepath);
        ~AudioClip();

        const std::string& GetFilepath() const { return m_Filepath; }
        const AudioFile& GetAudioFile() const { return m_AudioFile; }
    private:
        std::string m_Filepath;

        AudioFile m_AudioFile;
        f32 m_AudioLength = 0;

        friend class Audio;
    };
}
