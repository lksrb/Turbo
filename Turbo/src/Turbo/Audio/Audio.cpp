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

        s_Data.CurrentAudioBackend->OnRuntimeStart();
    }

    void Audio::OnRuntimeStop()
    {
        s_Data.CurrentAudioBackend->OnRuntimeStop();

        s_Data.Context = nullptr;
    }

    void Audio::PlayAudioClip(const Ref<AudioClip>& audioClip)
    {
        s_Data.CurrentAudioBackend->PlayAudioClip(audioClip);
    }

    Ref<AudioClip> Audio::CreateAndRegisterClip(const std::string& filepath)
    {
        Ref<AudioClip> audioClip = Ref<AudioClip>::Create(filepath);
        if (audioClip->m_AudioFile)
        {
            Audio::RegisterAudioClip(audioClip);
            return audioClip;
        }

        TBO_ENGINE_ERROR("Could not create audioclip! {}", filepath.data());
        return nullptr;
    }

    void Audio::UpdateAudioListener(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& velocity)
    {
        s_Data.CurrentAudioBackend->UpdateAudioListener(position, rotation, velocity);
    }

    void Audio::CalculateSpatial(const Ref<AudioClip>& audioClip, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& velocity)
    {
        s_Data.CurrentAudioBackend->CalculateSpatial(audioClip, position, rotation, velocity);
    }

    void Audio::RegisterAudioClip(const Ref<AudioClip>& audioClip)
    {
        s_Data.CurrentAudioBackend->RegisterAudioClip(audioClip);
    }

    Audio::BackendType Audio::GetAudioBackend()
    {
        return s_Data.AudioBackendType;
    }
}
