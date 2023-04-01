#pragma once

#include "Turbo/Core/PrimitiveTypes.h"

#include "Turbo/Audio/AudioClip.h"

namespace Turbo
{
    class Entity;

    class Audio
    {
    public:
        struct Data;

        enum class BackendType : u32
        {
            XAudio2 = 0
        };

        static void Init();
        static void Shutdown();

        static void RegisterAudioClip(Ref<AudioClip> audioClip);

        static BackendType GetAudioBackend();
        static void PlayClip(Entity entity);
    private:
        static Audio::Data* GetAudioContext();
    };
}
