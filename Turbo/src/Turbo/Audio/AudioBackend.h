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

        virtual void RegisterAudioClip(Ref<AudioClip> audioClip) = 0;
        virtual void PlayAudioClip(Ref<AudioClip> audioClip) = 0;
        virtual void UpdateAudioListener(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& velocity) = 0;
        virtual void CalculateSpatial(Ref<AudioClip> audioClip, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& velocity) = 0;

        static Ref<AudioBackend> Create();
    };
}
