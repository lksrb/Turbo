#include "tbopch.h"
#include "Audio.h"

#include "AudioBackend.h"

#include "Turbo/Scene/Scene.h"
#include "Turbo/Scene/Entity.h"

namespace Turbo
{
    // Global platform agnostic data, probably for storing global settings
    struct AudioData
    {
        Audio::BackendType AudioBackendType = Audio::BackendType::XAudio2;
        Ref<AudioBackend> CurrentAudioBackend;

        Scene* Context = nullptr;
    };

    static AudioData s_Data;

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

    void Audio::Play(UUID uuid, bool loop)
    {
        s_Data.CurrentAudioBackend->Play(uuid, loop);
    }

    void Audio::Resume(UUID uuid)
    {
        s_Data.CurrentAudioBackend->Resume(uuid);
    }

    void Audio::Pause(UUID uuid)
    {
        s_Data.CurrentAudioBackend->Pause(uuid);
    }

    void Audio::Stop(UUID uuid)
    {
        s_Data.CurrentAudioBackend->Stop(uuid);
    }

    bool Audio::IsPlaying(UUID uuid)
    {
        return s_Data.CurrentAudioBackend->IsPlaying(uuid);
    }

    void Audio::SetGain(UUID uuid, f32 gain)
    {
        s_Data.CurrentAudioBackend->SetGain(uuid, gain);
    }

    void Audio::Register(UUID uuid, const std::string& filepath)
    {
        s_Data.CurrentAudioBackend->Register(uuid, filepath);
    }

    void Audio::UnRegister(UUID uuid)
    {
        s_Data.CurrentAudioBackend->UnRegister(uuid);
    }

    void Audio::UpdateAudioListener(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& velocity)
    {
        s_Data.CurrentAudioBackend->UpdateAudioListener(position, rotation, velocity);
    }

    void Audio::CalculateSpatial(UUID uuid, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& velocity)
    {
        s_Data.CurrentAudioBackend->CalculateSpatial(uuid, position, rotation, velocity);
    }

    Audio::BackendType Audio::GetAudioBackend()
    {
        return s_Data.AudioBackendType;
    }
}
