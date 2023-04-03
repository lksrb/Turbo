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

        std::string GetFilepath() const { return m_Filepath; }
        bool PlayOnStart() const { return m_PlayOnStart; }
        void SetPlayOnStart(bool playOnStart) { m_PlayOnStart = playOnStart; }
    private:
        std::string m_Filepath;

        bool m_PlayOnStart = true;
        AudioFile m_AudioFile;
        f32 m_AudioLength = 0;

        friend class Audio;
        friend class XAudio2AudioBackend; // NOTE: Maybe not ideal
    };
}
