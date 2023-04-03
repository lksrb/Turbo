#pragma once

#include "AudioClip.h"
#include <glm/glm.hpp>

namespace Turbo
{
    class AudioBackend
    {
    public:
        AudioBackend();
        virtual ~AudioBackend();

        virtual void OnRuntimeStart() = 0;
        virtual void OnRuntimeStop() = 0;

        virtual void RegisterAudioClip(Ref<AudioClip> audioClip) = 0;
        virtual void Play(Ref<AudioClip> audioClip, bool loop) = 0;
        virtual void Pause(Ref<AudioClip> audioClip) = 0;
        virtual void StopAndClear(Ref<AudioClip> audioClip) = 0;
        virtual void SetGain(Ref<AudioClip> audioClip, f32 gain) = 0;
        virtual void UpdateAudioListener(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& velocity) = 0;
        virtual void CalculateSpatial(Ref<AudioClip> audioClip, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& velocity) = 0;

        static Ref<AudioBackend> Create();
    };
}
