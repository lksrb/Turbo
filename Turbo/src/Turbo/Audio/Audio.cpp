#include "tbopch.h"
#include "Audio.h"

#include "AudioBackend.h"

#include "Turbo/Scene/Entity.h"

namespace Turbo
{
    // Global platform agnostic data, probably for storing global settings
    struct Audio::Data
    {
        BackendType AudioBackendType = BackendType::XAudio2;
        Ref<AudioBackend> CurrentAudioBackend;
    };

    static Audio::Data* s_Data = nullptr;

    void Audio::Init()
    {
        s_Data = new Audio::Data;
        s_Data->CurrentAudioBackend = AudioBackend::Create();
    }

    void Audio::Shutdown()
    {
        delete s_Data;
        s_Data = nullptr;
    }

    void Audio::RegisterAudioClip(Ref<AudioClip> audioClip)
    {
        s_Data->CurrentAudioBackend->RegisterAudioClip(audioClip);
    }

    Audio::BackendType Audio::GetAudioBackend()
    {
        return GetAudioContext()->AudioBackendType;
    }

    void Audio::PlayClip(Entity entity)
    {

    }

    Audio::Data* Audio::GetAudioContext()
    {
        TBO_ENGINE_ASSERT(s_Data, "AudioEngine has not been initialized!");
        return s_Data;
    }

}
