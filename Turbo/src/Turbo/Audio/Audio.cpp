#include "tbopch.h"
#include "Audio.h"

#include "AudioBackend.h"

#include "Turbo/Scene/Scene.h"
#include "Turbo/Scene/Entity.h"

namespace Turbo
{
    // Global platform agnostic data, probably for storing global settings
    struct Audio::Data
    {
        BackendType AudioBackendType = BackendType::XAudio2;
        Ref<AudioBackend> CurrentAudioBackend;

        Scene* Context = nullptr;
    };

    static Audio::Data s_Data;

    void Audio::Init()
    {
        s_Data.CurrentAudioBackend = AudioBackend::Create();
    }

    void Audio::Shutdown()
    {
        s_Data.CurrentAudioBackend.Reset();
        s_Data.Context = nullptr;
    }

    void Audio::OnRuntimeStart(Scene* context)
    {
        s_Data.Context = context;
    }

    void Audio::OnRuntimeStop()
    {
        s_Data.Context = nullptr;
    }

    void Audio::UpdateAudioListener(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& velocity)
    {
        s_Data.CurrentAudioBackend->UpdateAudioListener(position, rotation, velocity);
    }

    void Audio::CalculateSpatial(Ref<AudioClip> audioClip, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& velocity)
    {
        s_Data.CurrentAudioBackend->CalculateSpatial(audioClip, position, rotation, velocity);
    }

    void Audio::RegisterAudioClip(Ref<AudioClip> audioClip)
    {
        s_Data.CurrentAudioBackend->RegisterAudioClip(audioClip);
    }

    Audio::BackendType Audio::GetAudioBackend()
    {
        return s_Data.AudioBackendType;
    }
}
