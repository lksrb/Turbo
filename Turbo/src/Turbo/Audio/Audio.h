#pragma once

#include "AudioClip.h"

#include "Turbo/Core/PrimitiveTypes.h"

#include <glm/glm.hpp>

namespace Turbo
{
    class Entity;
    class Scene;

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

        static void OnRuntimeStart(Scene* context);
        static void OnRuntimeStop();

        static void UpdateAudioListener(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& velocity);
        static void CalculateSpatial(Ref<AudioClip> audioClip, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& velocity);

        static void RegisterAudioClip(Ref<AudioClip> audioClip);

        static BackendType GetAudioBackend();
    private:
    };
}
