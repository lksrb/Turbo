#include "tbopch.h"
#include "AudioEngine.h"

#include "AudioBackend.h"

#include "Turbo/Scene/Scene.h"
#include "Turbo/Scene/Entity.h"

namespace Turbo
{
    // Global platform agnostic data, probably for storing global settings
    struct AudioInternal
    {
        AudioEngine::BackendType AudioBackendType = AudioEngine::BackendType::XAudio2;
        Owned<AudioBackend> CurrentAudioBackend;

        Ref<Scene> Context;
    };

    static AudioInternal s_AudioEngine;

    void AudioEngine::Init()
    {
        s_AudioEngine.CurrentAudioBackend = AudioBackend::Create();
    }

    void AudioEngine::Shutdown()
    {
        s_AudioEngine.CurrentAudioBackend.Reset();
        s_AudioEngine.Context.Reset();
    }

    void AudioEngine::OnRuntimeStart(const Ref<Scene>& context)
    {
        s_AudioEngine.Context = context;

        s_AudioEngine.CurrentAudioBackend->OnRuntimeStart();
    }

    void AudioEngine::OnRuntimeStop()
    {
        s_AudioEngine.CurrentAudioBackend->OnRuntimeStop();
        s_AudioEngine.Context.Reset();
    }

    void AudioEngine::Play(UUID uuid, bool loop)
    {
        s_AudioEngine.CurrentAudioBackend->Play(uuid, loop);
    }

    void AudioEngine::Resume(UUID uuid)
    {
        s_AudioEngine.CurrentAudioBackend->Resume(uuid);
    }

    void AudioEngine::Pause(UUID uuid)
    {
        s_AudioEngine.CurrentAudioBackend->Pause(uuid);
    }

    void AudioEngine::Stop(UUID uuid)
    {
        s_AudioEngine.CurrentAudioBackend->Stop(uuid);
    }

    bool AudioEngine::IsPlaying(UUID uuid)
    {
        return s_AudioEngine.CurrentAudioBackend->IsPlaying(uuid);
    }

    void AudioEngine::SetGain(UUID uuid, f32 gain)
    {
        s_AudioEngine.CurrentAudioBackend->SetGain(uuid, gain);
    }

    void AudioEngine::Register(UUID uuid, const std::string& filepath)
    {
        s_AudioEngine.CurrentAudioBackend->Register(uuid, filepath);
    }

    void AudioEngine::UnRegister(UUID uuid)
    {
        s_AudioEngine.CurrentAudioBackend->UnRegister(uuid);
    }

    void AudioEngine::UpdateAudioListener(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& velocity)
    {
        s_AudioEngine.CurrentAudioBackend->UpdateAudioListener(position, rotation, velocity);
    }

    void AudioEngine::CalculateSpatial(UUID uuid, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& velocity)
    {
        s_AudioEngine.CurrentAudioBackend->CalculateSpatial(uuid, position, rotation, velocity);
    }

    AudioEngine::BackendType AudioEngine::GetAudioBackend()
    {
        return s_AudioEngine.AudioBackendType;
    }
}
