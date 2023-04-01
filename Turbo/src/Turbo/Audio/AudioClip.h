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
        
        static Ref<AudioClip> Create(const std::string& filepath);
    private:
        AudioFile m_AudioFile;
        std::string m_Filepath;
        f32 m_AudioLength = 0;

        friend class XAudio2AudioBackend; // NOTE: Maybe not ideal
    };
}
