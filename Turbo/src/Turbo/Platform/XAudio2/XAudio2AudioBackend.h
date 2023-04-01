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

        void RegisterAudioClip(Ref<AudioClip> audioClip) override;

    private:
        void SetupXA2Debugging();
    private:
        std::unordered_map<AudioClip*, AudioSourceData> m_AudioSources;

        XAUDIO2_VOICE_DETAILS m_MasteringVoiceDetails;
        DWORD m_ChannelMask;
        IXAudio2* m_XInstance = nullptr;
        IXAudio2MasteringVoice* m_XMasterVoice = nullptr;
        XAudio2Debugger m_Debugger;
    };
}
