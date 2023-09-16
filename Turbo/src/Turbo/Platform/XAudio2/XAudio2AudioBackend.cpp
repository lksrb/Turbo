#include "tbopch.h"
#include "XAudio2AudioBackend.h"

#include "Turbo/Audio/AudioFile.h"

#define TBO_XA2_CHECK(x) do { HRESULT hr = (x); TBO_ENGINE_ASSERT(hr == S_OK, hr); } while(0)

#define LEFT_AZIMUTH            3 * X3DAUDIO_PI / 2
#define RIGHT_AZIMUTH           X3DAUDIO_PI / 2
#define FRONT_LEFT_AZIMUTH      7 * X3DAUDIO_PI / 4
#define FRONT_RIGHT_AZIMUTH     X3DAUDIO_PI / 4
#define FRONT_CENTER_AZIMUTH    0.0f
#define LOW_FREQUENCY_AZIMUTH   X3DAUDIO_2PI
#define BACK_LEFT_AZIMUTH       5 * X3DAUDIO_PI / 4
#define BACK_RIGHT_AZIMUTH      3 * X3DAUDIO_PI / 4
#define BACK_CENTER_AZIMUTH     X3DAUDIO_PI

static constexpr ::Turbo::f32 s_ChannelAzimuths[9][8] =
{
    /* 0 */   { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    /* 1 */   { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    /* 2 */   { FRONT_LEFT_AZIMUTH, FRONT_RIGHT_AZIMUTH, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f },
    /* 2.1 */ { FRONT_LEFT_AZIMUTH, FRONT_RIGHT_AZIMUTH, LOW_FREQUENCY_AZIMUTH, 0.f, 0.f, 0.f, 0.f, 0.f },
    /* 4.0 */ { FRONT_LEFT_AZIMUTH, FRONT_RIGHT_AZIMUTH, BACK_LEFT_AZIMUTH, BACK_RIGHT_AZIMUTH, 0.f, 0.f, 0.f, 0.f },
    /* 4.1 */ { FRONT_LEFT_AZIMUTH, FRONT_RIGHT_AZIMUTH, LOW_FREQUENCY_AZIMUTH, BACK_LEFT_AZIMUTH, BACK_RIGHT_AZIMUTH, 0.f, 0.f, 0.f },
    /* 5.1 */ { FRONT_LEFT_AZIMUTH, FRONT_RIGHT_AZIMUTH, FRONT_CENTER_AZIMUTH, LOW_FREQUENCY_AZIMUTH, BACK_LEFT_AZIMUTH, BACK_RIGHT_AZIMUTH, 0.f, 0.f },
    /* 6.1 */ { FRONT_LEFT_AZIMUTH, FRONT_RIGHT_AZIMUTH, FRONT_CENTER_AZIMUTH, LOW_FREQUENCY_AZIMUTH, BACK_LEFT_AZIMUTH, BACK_RIGHT_AZIMUTH, BACK_CENTER_AZIMUTH, 0.f },
    /* 7.1 */ { FRONT_LEFT_AZIMUTH, FRONT_RIGHT_AZIMUTH, FRONT_CENTER_AZIMUTH, LOW_FREQUENCY_AZIMUTH, BACK_LEFT_AZIMUTH, BACK_RIGHT_AZIMUTH, LEFT_AZIMUTH, RIGHT_AZIMUTH }
};


#define TBO_CHOOSE_CHANNEL_AZIMUTHS(x) const_cast<::Turbo::f32*>(s_ChannelAzimuths[x])
#define TBO_MAX_CHANNELS 8

namespace Turbo {

    XAudio2AudioBackend::XAudio2AudioBackend()
    {
        // Create COM
        TBO_XA2_CHECK(CoInitializeEx(nullptr, COINIT_MULTITHREADED)); // COINIT_APARTMENTTHREADED for single threading?

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
        m_XMasterVoice->DestroyVoice();
        m_XMasterVoice = nullptr;

        m_XInstance->Release();
        m_XInstance = nullptr;

        // Uninialize COM a.k.a m_X3DInstance TODO: Investigate
        CoUninitialize();
        ZeroMemory(m_X3DInstance, sizeof(m_X3DInstance));

        TBO_ENGINE_INFO("XAudio2 backend successfully shutdown!");
    }

    IXAudio2SourceVoice* XAudio2AudioBackend::TryFindSourceWithUUID(UUID uuid)
    {
        auto it = m_AudioData.find(uuid);

        if (it == m_AudioData.end())
            return nullptr;

        return it->second.SourceVoice;
    }

    void XAudio2AudioBackend::SetupXA2Debugging()
    {
        XAUDIO2_DEBUG_CONFIGURATION debugConf = {};
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

    void XAudio2AudioBackend::OnRuntimeStart()
    {
        // FIXME: Maybe not necessary?
    }

    void XAudio2AudioBackend::OnRuntimeStop()
    {
        for (auto& [uuid, source] : m_AudioData)
        {
            TBO_XA2_CHECK(source.SourceVoice->Stop());
            TBO_XA2_CHECK(source.SourceVoice->FlushSourceBuffers());
            source.SourceVoice->DestroyVoice();
            source.Data.Release();
        }

        m_AudioData.clear();
    }

    void XAudio2AudioBackend::Play(UUID uuid, bool loop)
    {
        auto& [sourceVoice, audioBuffer, _] = m_AudioData.at(uuid);

        audioBuffer.LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;

        // Submit buffer to source
        TBO_XA2_CHECK(sourceVoice->SubmitSourceBuffer(&audioBuffer));

        // Start
        TBO_XA2_CHECK(sourceVoice->Start(0, XAUDIO2_COMMIT_NOW));
    }

    void XAudio2AudioBackend::Resume(UUID uuid)
    {
        IXAudio2SourceVoice* sourceVoice = TryFindSourceWithUUID(uuid);

        TBO_XA2_CHECK(sourceVoice->Start());
    }

    void XAudio2AudioBackend::Pause(UUID uuid)
    {
        IXAudio2SourceVoice* sourceVoice = TryFindSourceWithUUID(uuid);

        TBO_XA2_CHECK(sourceVoice->Stop());
    }

    void XAudio2AudioBackend::Stop(UUID uuid)
    {
        IXAudio2SourceVoice* sourceVoice = TryFindSourceWithUUID(uuid);

        if (!sourceVoice)
            return;

        TBO_XA2_CHECK(sourceVoice->Stop());
        TBO_XA2_CHECK(sourceVoice->FlushSourceBuffers());
    }

    bool XAudio2AudioBackend::IsPlaying(UUID uuid)
    {
        IXAudio2SourceVoice* sourceVoice = TryFindSourceWithUUID(uuid);

        XAUDIO2_VOICE_STATE state;
        sourceVoice->GetState(&state);
        return state.BuffersQueued != 0;
    }

    void XAudio2AudioBackend::SetGain(UUID uuid, f32 gain)
    {
        IXAudio2SourceVoice* sourceVoice = TryFindSourceWithUUID(uuid);

        TBO_XA2_CHECK(sourceVoice->SetVolume(gain));
    }

    // TODO: REWORK
    void XAudio2AudioBackend::Register(UUID uuid, const std::string& filepath)
    {
        if (m_AudioData.find(uuid) != m_AudioData.end())
        {
            TBO_ENGINE_WARN("Already registered!");
            return;
        }

        AudioFile audioFile(filepath);

        if (!audioFile)
            return;

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
        buffer.AudioBytes = static_cast<u32>(audioFile.Data.Size);	// Size of the audio buffer in bytes
        buffer.pAudioData = audioFile.Data.Data;		// Buffer containing audio data
        buffer.Flags = XAUDIO2_END_OF_STREAM;      // Tell the source voice not to expect any data after this buffer

        AudioData& file = m_AudioData[uuid];
        file.SourceVoice = sourceVoice;
        file.BufferInfo = buffer;
        file.Data = std::move(audioFile.Data);
    }

    void XAudio2AudioBackend::UnRegister(UUID uuid)
    {
        auto it = m_AudioData.find(uuid);

        if (it == m_AudioData.end())
        {
            TBO_ENGINE_ERROR("Failed to find clip to unregister!");
            return;
        }

        Stop(uuid);
        it->second.SourceVoice->DestroyVoice();
        it->second.Data.Release();
        m_AudioData.erase(it);
    }

    static const bool s_2D = false; // TODO: Figure out where this belongs

    void XAudio2AudioBackend::UpdateAudioListener(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& velocity)
    {
        m_AudioListener.Position = { position.x, position.y, position.z };
        m_AudioListener.Velocity = { velocity.x, velocity.y, velocity.z };

        // Omnidirectionality - TODO: Cone support for more detailed 3D sounds
        m_AudioListener.pCone = NULL;

        // NOTE: For now, orientation is not strictly necessary
        if (s_2D)
        {
#if 0
            glm::vec3 orientRight = {};
            orientRight.x = cos(rotation.y);
            orientRight.y = 0;
            orientRight.z = -sin(rotation.y);

            m_AudioListener.OrientFront = { orientFront.x, orientFront.y, orientFront.z };
            m_AudioListener.OrientTop = { orientTop.x, orientTop.y, orientTop.z };
            TBO_ENGINE_TRACE("OrientRight: {}; ", orientRight);
#endif
            m_AudioListener.OrientFront = { 0.0f, 0.0f, 0.0f };
            m_AudioListener.OrientTop = { 0.0f, 0.0f, 0.0f };

            return;
        }

        glm::vec3 orientFront = {};
        orientFront.x = cosf(rotation.x) * sinf(rotation.y);
        orientFront.y = -sinf(rotation.x);
        orientFront.z = cosf(rotation.x) * cosf(rotation.y);

        glm::vec3 orientTop = {};
        orientTop.x = sinf(rotation.x) * sinf(rotation.y);
        orientTop.x = cosf(rotation.x);
        orientTop.x = sinf(rotation.x) * cosf(rotation.y);

        m_AudioListener.OrientFront = { orientFront.x, orientFront.y, orientFront.z };
        m_AudioListener.OrientTop = { orientTop.x, orientTop.y, orientTop.z };
    }

    void XAudio2AudioBackend::CalculateSpatial(UUID uuid, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& velocity)
    {
        f32 outputMatrix[TBO_MAX_CHANNELS * TBO_MAX_CHANNELS]{ 1.0f };

        glm::vec3 orientFront = {};
        orientFront.x = cosf(rotation.x) * sinf(rotation.y);
        orientFront.y = -sinf(rotation.x);
        orientFront.z = cosf(rotation.x) * cosf(rotation.y);

        glm::vec3 orientTop = {};
        orientTop.x = sinf(rotation.x) * sinf(rotation.y);
        orientTop.x = cosf(rotation.x);
        orientTop.x = sinf(rotation.x) * cosf(rotation.y);

        auto& sourceVoice = m_AudioData.at(uuid).SourceVoice;

        XAUDIO2_VOICE_DETAILS details;
        sourceVoice->GetVoiceDetails(&details);
        u32 sourceInputChannels = details.InputChannels;
        u32 masterInputChannels = m_MasteringVoiceDetails.InputChannels;

        X3DAUDIO_EMITTER sourceEmitter;

        // Omnidirectionality - TODO: Cone support for more detailed 3D sounds
        sourceEmitter.pCone = NULL;
        //sourceEmitter.pCone = (X3DAUDIO_CONE*)&X3DAudioDefault_DirectionalCone;

        sourceEmitter.Position = { position.x, position.y, position.z };
        sourceEmitter.Velocity = { velocity.x, velocity.y, velocity.z };
        sourceEmitter.OrientTop = { orientTop.x, orientTop.y, orientTop.z };
        sourceEmitter.OrientFront = { orientFront.x, orientFront.y, orientFront.z };

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
        sourceEmitter.pChannelAzimuths = TBO_CHOOSE_CHANNEL_AZIMUTHS(sourceInputChannels);

        // DSP Settings
        X3DAUDIO_DSP_SETTINGS dspSettings = {};
        dspSettings.SrcChannelCount = sourceInputChannels;
        dspSettings.DstChannelCount = masterInputChannels;
        dspSettings.pMatrixCoefficients = outputMatrix;

        // Actual calculation
        X3DAudioCalculate(m_X3DInstance, &m_AudioListener, &sourceEmitter,
            X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER |
            X3DAUDIO_CALCULATE_LPF_DIRECT | X3DAUDIO_CALCULATE_REVERB,
            &dspSettings);

#if 0
        // TODO: Figure out what are filter parameters
        // And why 'SetFilterParameters' throws XAUDIO2_E_INVALID_CALL
        XAUDIO2_FILTER_PARAMETERS filterParameters = {};
        filterParameters.Type = LowPassFilter;
        filterParameters.Frequency = 1.0f; // dspSettings.LPFDirectCoefficient ? 2.0f * sinf(X3DAUDIO_PI / 6.0f * dspSettings.LPFDirectCoefficient) : 1.0f;
        filterParameters.OneOverQ = 1.0f;
        TBO_XA2_CHECK(sourceVoice->SetFilterParameters(&filterParameters, XAUDIO2_COMMIT_NOW));
        // Pitch
        f32 frequency = dspSettings.LPFDirectCoefficient ? 2.0f * sinf(X3DAUDIO_PI / 6.0f * dspSettings.LPFDirectCoefficient) : 1.0f;
        TBO_XA2_CHECK(sourceVoice->SetFrequencyRatio(1.0f));
#endif

        // TBO_XA2_CHECK(sourceVoice->SetVolume(1.0f)); 
        TBO_XA2_CHECK(sourceVoice->SetOutputMatrix(NULL, sourceInputChannels, m_MasteringVoiceDetails.InputChannels, outputMatrix));
    }
}
