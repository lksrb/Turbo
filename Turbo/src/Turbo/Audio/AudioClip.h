#pragma once

#include "Turbo/Core/Common.h"

#include "AudioFile.h"

namespace Turbo
{
    // High-level wrapper around a single audio file
    class AudioClip
    {
    public:
        AudioClip(const std::string& filepath, bool playOnStart = true);
        ~AudioClip();
        
        static Ref<AudioClip> Create(const std::string& filepath, bool playOnStart = true);
    private:
        std::string m_Filepath;

        bool m_PlayOnStart;
        AudioFile m_AudioFile;
        f32 m_AudioLength = 0;

        friend class XAudio2AudioBackend; // NOTE: Maybe not ideal
    };
}
