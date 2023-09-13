#pragma once

#include "Turbo/Core/UUID.h"

#include <string>

namespace Turbo {

    class AudioBackend : public RefCounted
    {
    public:
        virtual ~AudioBackend() = default;

        virtual void OnRuntimeStart() = 0;
        virtual void OnRuntimeStop() = 0;

        virtual void Register(UUID uuid, const std::string& filepath) = 0;
        virtual void UnRegister(UUID uuid) = 0;
        virtual void Play(UUID uuid, bool loop) = 0;
        virtual void Pause(UUID uuid) = 0;
        virtual void Resume(UUID uuid) = 0;
        virtual void Stop(UUID uuid) = 0;
        virtual bool IsPlaying(UUID uuid) = 0;
        virtual void SetGain(UUID uuid, f32 gain) = 0;
        virtual void UpdateAudioListener(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& velocity) = 0;
        virtual void CalculateSpatial(UUID uuid, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& velocity) = 0;

        static Owned<AudioBackend> Create();
    };
}
