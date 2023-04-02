#include "tbopch.h"
#include "XAudio2AudioBackend.h"

#include "Turbo/Audio/AudioFile.h"

#define TBO_XA2_CHECK(x) do { HRESULT hr = (x); TBO_ENGINE_ASSERT(hr == S_OK);  } while(0)
#define CEAL_MAX_CHANNELS 8

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
        TBO_XA2_CHECK(m_XMasterVoice->GetChannelMask(&m_ChannelMask));

        // 3D audio 
        TBO_XA2_CHECK(X3DAudioInitialize(m_ChannelMask, X3DAUDIO_SPEED_OF_SOUND, m_X3DInstance));

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

        // Uninialize COM a.k.a m_X3DInstance TODO: Investigate
        CoUninitialize();
        ZeroMemory(m_X3DInstance, sizeof(m_X3DInstance));

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

        m_AudioSources[audioClip.Get()] = { sourceVoice, buffer };
    }

    void XAudio2AudioBackend::PlayAudioClip(Ref<AudioClip> audioClip)
    {
        auto& [sourceVoice, audioBuffer] = m_AudioSources.at(audioClip.Get());

        // Submit buffer to source
        TBO_XA2_CHECK(sourceVoice->SubmitSourceBuffer(&audioBuffer));

        // Start
        TBO_XA2_CHECK(sourceVoice->Start(0, XAUDIO2_COMMIT_NOW));
    }

    void XAudio2AudioBackend::UpdateAudioListener(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& velocity)
    {
        // Set position
        m_AudioListener.Position.x = position.x;
        m_AudioListener.Position.y = position.y;
        m_AudioListener.Position.z = position.z;

        // Set velocity
        m_AudioListener.Velocity.x = velocity.x;
        m_AudioListener.Velocity.y = velocity.y;
        m_AudioListener.Velocity.z = velocity.z;

        // Set orientation
        m_AudioListener.OrientTop.x = {};
        m_AudioListener.OrientTop.y = {};
        m_AudioListener.OrientTop.z = {};
        m_AudioListener.OrientFront.x = {};
        m_AudioListener.OrientFront.y = {};
        m_AudioListener.OrientFront.z = {};

        TBO_ENGINE_TRACE("Camera position {}", position);
        TBO_ENGINE_TRACE("Camera rotation {}", rotation);

        // Omnidirectionality - TODO: Cone support for more detailed 3D sounds
        m_AudioListener.pCone = NULL;
    }

    void XAudio2AudioBackend::CalculateSpatial(Ref<AudioClip> audioClip, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& velocity)
    {
        f32 outputMatrix[CEAL_MAX_CHANNELS * CEAL_MAX_CHANNELS]{ 1.0f };

        auto& [sourceVoice, _] = m_AudioSources.at(audioClip.Get());

        XAUDIO2_VOICE_DETAILS details;
        sourceVoice->GetVoiceDetails(&details);
        uint32_t sourceInputChannels = details.InputChannels;
        uint32_t masterInputChannels = m_Details.InputChannels;

        X3DAUDIO_EMITTER sourceEmitter;
        sourceEmitter.pCone = (X3DAUDIO_CONE*)&X3DAudioDefault_DirectionalCone; // Default cone 
        sourceEmitter.pCone = nullptr;

        // Set position
        sourceEmitter.Position.x = position.x;
        sourceEmitter.Position.y = position.y;
        sourceEmitter.Position.z = position.z;

        // Set velocity
        sourceEmitter.Velocity.x = velocity.x;
        sourceEmitter.Velocity.y = velocity.y;
        sourceEmitter.Velocity.z = velocity.z;

        // Set orientation
        sourceEmitter.OrientTop.x = {};
        sourceEmitter.OrientTop.y = {};
        sourceEmitter.OrientTop.z = {};
        sourceEmitter.OrientFront.x = {};
        sourceEmitter.OrientFront.y = {};
        sourceEmitter.OrientFront.z = {};

        // Other parameters
        sourceEmitter.InnerRadius = 1.0f;
        sourceEmitter.InnerRadiusAngle = X3DAUDIO_PI / 4.0f;
        sourceEmitter.ChannelCount = sourceInputChannels;
        sourceEmitter.ChannelRadius = 1.0f;
        sourceEmitter.pVolumeCurve = NULL;
        sourceEmitter.pLFECurve = NULL;
        sourceEmitter.pLPFDirectCurve = NULL;
        sourceEmitter.pLPFReverbCurve = NULL;
        sourceEmitter.pReverbCurve = NULL;
        sourceEmitter.CurveDistanceScaler = 1.0f;
        sourceEmitter.DopplerScaler = 0.0f;

        // DSP Settings
        X3DAUDIO_DSP_SETTINGS dspSettings{};
        dspSettings.SrcChannelCount = sourceInputChannels;
        dspSettings.DstChannelCount = masterInputChannels;
        dspSettings.pMatrixCoefficients = outputMatrix;

        // Actual calculation
        X3DAudioCalculate(m_X3DInstance, &m_AudioListener, &sourceEmitter,
            X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER |
            X3DAUDIO_CALCULATE_LPF_DIRECT | X3DAUDIO_CALCULATE_REVERB,
            &dspSettings);

        // Pitch = dspSettings.DopplerFactor;
        
        // Filter settings
        // TODO: Figure out what filtering actually means
        XAUDIO2_FILTER_PARAMETERS filterParameters = {};
        filterParameters.Type = LowPassFilter;
        filterParameters.Frequency = dspSettings.LPFDirectCoefficient ? 2.0f * sinf(X3DAUDIO_PI / 6.0f * dspSettings.LPFDirectCoefficient) : 1.0f;
        filterParameters.OneOverQ = 1.0f;

        //TBO_XA2_CHECK(sourceVoice->SetVolume(1.0f));
        //TBO_XA2_CHECK(sourceVoice->SetFrequencyRatio(1.0f));
        //TBO_XA2_CHECK(sourceVoice->SetOutputMatrix(NULL, sourceInputChannels, m_Details.InputChannels, outputMatrix));
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
