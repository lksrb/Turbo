#pragma once

#include "Turbo/Core/UUID.h"
#include "Turbo/Core/PrimitiveTypes.h"

#include <glm/glm.hpp>

namespace Turbo
{
    class Entity;
    class Scene;

    class Audio
    {
    public:
        enum class BackendType : u32
        {
            XAudio2 = 0
        };

        static void Init();
        static void Shutdown();

        static void OnRuntimeStart(Scene* context);
        static void OnRuntimeStop();

        static void Play(UUID uuid, bool loop);
        static void Resume(UUID uuid);
        static void Stop(UUID uuid);
        static void Pause(UUID uuid);
        static bool IsPlaying(UUID uuid);

        static void SetGain(UUID uuid, f32 gain);

        static void Register(UUID uuid, const std::string& filepath);
        static void UnRegister(UUID uuid);

        static void UpdateAudioListener(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& velocity);
        static void CalculateSpatial(UUID uuid, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& velocity);

        static BackendType GetAudioBackend();
    private:
    };
}
