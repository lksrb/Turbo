#pragma once

#include "Audio.h"

namespace Turbo
{
    class AudioBackend
    {
    public:
        AudioBackend();
        virtual ~AudioBackend();

        virtual void RegisterAudioClip(Ref<AudioClip> audioClip) = 0;

        static Ref<AudioBackend> Create();
    };
}
