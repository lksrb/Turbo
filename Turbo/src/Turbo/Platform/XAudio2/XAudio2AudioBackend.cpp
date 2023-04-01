#include "tbopch.h"
#include "XAudio2AudioBackend.h"

#include "Turbo/Audio/AudioFile.h"

#define TBO_XA2_CHECK(x) do { HRESULT hr = (x); TBO_ENGINE_ASSERT(hr == S_OK);  } while(0)

namespace Turbo
{
    XAudio2AudioBackend::XAudio2AudioBackend()
    {
        // Create COM
        TBO_XA2_CHECK(CoInitializeEx(nullptr, COINIT_MULTITHREADED));

        // Create audio engine instance
        TBO_XA2_CHECK(XAudio2Create(&m_XInstance, XAUDIO2_DEBUG_ENGINE, XAUDIO2_DEFAULT_PROCESSOR));

        // Setup XAudio2 debug facilities
        SetupXA2Debugging();

        // Create mastering voice
        TBO_XA2_CHECK(m_XInstance->CreateMasteringVoice(&m_XMasterVoice, 2));

        // Query details about audio device
        m_XMasterVoice->GetVoiceDetails(&m_MasteringVoiceDetails);
        m_XMasterVoice->GetChannelMask(&m_ChannelMask);

        // 3D audio 

     /*   {
            static AudioFile audioFile("Assets/Audio/example_song_mono.wav");

            TBO_ENGINE_ASSERT(audioFile);

            // Populating WAVEFORMATEX structure
            WAVEFORMATEX wfx = { 0 };
            wfx.wFormatTag = WAVE_FORMAT_PCM;
            wfx.nChannels = audioFile.NumChannels;
            wfx.nSamplesPerSec = audioFile.SampleRate;
            wfx.wBitsPerSample = audioFile.BitsPerSample;
            wfx.cbSize = audioFile.ExtraParamSize;
            wfx.nBlockAlign = audioFile.BlockAlign;
            wfx.nAvgBytesPerSec = audioFile.ByteRate;

            //SourceVoiceCallback* callback = new SourceVoiceCallback;
            IXAudio2SourceVoice* testVoice;
            TBO_XA2_CHECK(m_XInstance->CreateSourceVoice(&testVoice, &wfx, NULL, XAUDIO2_DEFAULT_FREQ_RATIO, NULL, NULL, NULL));

            // Create buffer
            static XAUDIO2_BUFFER buffer = {};
            buffer.AudioBytes = static_cast<u32>(audioFile.AudioData.Size);	// Size of the audio buffer in bytes
            buffer.pAudioData = audioFile.AudioData.Data;		// Buffer containing audio data
            buffer.Flags = XAUDIO2_END_OF_STREAM;      // Tell the source voice not to expect any data after this buffer

            // Submit buffer to source
            TBO_XA2_CHECK(testVoice->SubmitSourceBuffer(&buffer));

            // Play audio buffer
            TBO_XA2_CHECK(testVoice->Start(0, XAUDIO2_COMMIT_NOW));

        / *    XAUDIO2_VOICE_STATE state;
            while (testVoice->GetState(&state), state.BuffersQueued > 0)
            {
            }* /
        }*/

        TBO_ENGINE_INFO("XAudio2 backend successfully initialized!");
    }

    XAudio2AudioBackend::~XAudio2AudioBackend()
    {
        // Release source voices
        for (auto& [_, source] : m_AudioSources)
        {
            // Wait for thread to clean up
#if 0
            while (source->StreamData.IsStreaming)
            {
            }

            SourceVoiceCallback* callback = (SourceVoiceCallback*)source->StreamData.StreamCallback;
            CEAL_DELETE(SourceVoiceCallback, callback);
#endif
            TBO_XA2_CHECK(source.SourceVoice->Stop());
            source.SourceVoice->DestroyVoice();
        }

        m_XMasterVoice->DestroyVoice();
        m_XMasterVoice = nullptr;

        m_XInstance->Release();
        m_XInstance = nullptr; 

        TBO_ENGINE_INFO("XAudio2 backend successfully shutdown!");
    }

    void XAudio2AudioBackend::RegisterAudioClip(Ref<AudioClip> audioClip)
    {
        if (m_AudioSources.find(audioClip.Get()) != m_AudioSources.end())
        {
            TBO_ENGINE_WARN("Already registered!");
            return;
        }
        const auto& audioFile = audioClip->m_AudioFile;

        // Populating WAVEFORMATEX structure
        WAVEFORMATEX wfx = { 0 };
        wfx.wFormatTag = WAVE_FORMAT_PCM;
        wfx.nChannels = audioFile.NumChannels;
        wfx.nSamplesPerSec = audioFile.SampleRate;
        wfx.wBitsPerSample = audioFile.BitsPerSample;
        wfx.cbSize = audioFile.ExtraParamSize;
        wfx.nBlockAlign = audioFile.BlockAlign;
        wfx.nAvgBytesPerSec = audioFile.ByteRate;

        // Create source voice
        IXAudio2SourceVoice* sourceVoice;
        TBO_XA2_CHECK(m_XInstance->CreateSourceVoice(&sourceVoice, &wfx, NULL, XAUDIO2_DEFAULT_FREQ_RATIO, NULL, NULL, NULL));

        // Create buffer
        XAUDIO2_BUFFER buffer = {};
        buffer.AudioBytes = static_cast<u32>(audioFile.AudioData.Size);	// Size of the audio buffer in bytes
        buffer.pAudioData = audioFile.AudioData.Data;		// Buffer containing audio data
        buffer.Flags = XAUDIO2_END_OF_STREAM;      // Tell the source voice not to expect any data after this buffer

        // Submit buffer to source
        TBO_XA2_CHECK(sourceVoice->SubmitSourceBuffer(&buffer));

        m_AudioSources[audioClip.Get()] = { sourceVoice, buffer };

        //TBO_XA2_CHECK(sourceVoice->Start(0, XAUDIO2_COMMIT_NOW));
    }

    void XAudio2AudioBackend::SetupXA2Debugging()
    {
        XAUDIO2_DEBUG_CONFIGURATION debugConf{};
        debugConf.LogFunctionName = true;

        // XAUDIO2_LOG_WARNINGS also enables XAUDIO2_LOG_ERRORS
        debugConf.TraceMask = XAUDIO2_LOG_ERRORS | XAUDIO2_LOG_WARNINGS |
            XAUDIO2_LOG_INFO | XAUDIO2_LOG_DETAIL;
        m_XInstance->SetDebugConfiguration(&debugConf);

        // Perfomance data
        XAUDIO2_PERFORMANCE_DATA performanceData;
        m_XInstance->GetPerformanceData(&performanceData);

        TBO_XA2_CHECK(m_XInstance->RegisterForCallbacks(&m_Debugger));
    }
}
