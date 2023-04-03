#pragma once

#include "Turbo/Audio/AudioBackend.h"

#include <unordered_map>

#include <xaudio2.h>
#include <xaudio2fx.h>
#include <x3daudio.h>

namespace Turbo
{
    class XAudio2Debugger : public IXAudio2EngineCallback
    {
        virtual void OnProcessingPassStart() override
        {
        }

        virtual void OnProcessingPassEnd() override
        {
        }

        virtual void OnCriticalError(HRESULT error) override
        {
            TBO_ENGINE_ERROR("XAudio2 error: {}", error);
        }
    };
#if 0
    // Maybe not necessary
    class SourceVoiceCallback : public IXAudio2VoiceCallback
    {
    public:
    protected:
        void OnBufferStart(void* pBufferContext)
        {

        }
        void OnBufferEnd(void* pBufferContext)
        {
        }

        void OnVoiceProcessingPassStart(UINT32 bytesRequired) {}
        void OnVoiceProcessingPassEnd() {};

        void OnStreamEnd()
        {
        }
        void OnLoopEnd(void* pBufferContext)
        {
        }

        void OnVoiceError(void* pBufferContext, HRESULT error)
        {
            TBO_ENGINE_ERROR("XAudio2 error: {}", error);
        }
    };
#endif

    struct AudioSourceData
    {
        IXAudio2SourceVoice* SourceVoice;
        XAUDIO2_BUFFER Buffer;
    };

    class XAudio2AudioBackend : public AudioBackend
    {
    public:
        XAudio2AudioBackend();
        ~XAudio2AudioBackend();

        void OnRuntimeStart() override;
        void OnRuntimeStop() override;

        void RegisterAudioClip(Ref<AudioClip> audioClip) override;

        void Play(Ref<AudioClip> audioClip, bool loop) override;
        void Pause(Ref<AudioClip> audioClip) override;
        void StopAndClear(Ref<AudioClip> audioClip) override;
        void SetGain(Ref<AudioClip> audioClip, f32 gain) override;

        // Spatial calculations
        void UpdateAudioListener(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& velocity) override;
        void CalculateSpatial(Ref<AudioClip> audioClip, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& velocity) override;
    private:
        void StopAndClearAll();
        void SetupXA2Debugging();
    private:
        X3DAUDIO_LISTENER m_AudioListener;

        std::unordered_map<AudioClip*, AudioSourceData> m_AudioSources;

        IXAudio2* m_XInstance = nullptr;
        DWORD m_ChannelMask;
        XAUDIO2_VOICE_DETAILS m_MasteringVoiceDetails;
        IXAudio2MasteringVoice* m_XMasterVoice = nullptr;

        X3DAUDIO_HANDLE m_X3DInstance;

        XAudio2Debugger m_Debugger;
    };
}
