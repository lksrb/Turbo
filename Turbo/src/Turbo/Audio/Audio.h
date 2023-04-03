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

        static void PlayAudioClip(const Ref<AudioClip>& audioClip);

        static Ref<AudioClip> CreateAndRegisterClip(const std::string& filepath);

        static void UpdateAudioListener(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& velocity);
        static void CalculateSpatial(const Ref<AudioClip>& audioClip, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& velocity);

        static BackendType GetAudioBackend();
    private:
        static void RegisterAudioClip(const Ref<AudioClip>& audioClip);
    };
}
