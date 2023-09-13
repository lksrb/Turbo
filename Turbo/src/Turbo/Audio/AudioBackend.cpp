#include "tbopch.h"
#include "AudioBackend.h"

#include "AudioEngine.h"

#include "Turbo/Platform/XAudio2/XAudio2AudioBackend.h"

namespace Turbo {

    Owned<AudioBackend> AudioBackend::Create()
    {
        switch (AudioEngine::GetAudioBackend())
        {
            case AudioEngine::BackendType::XAudio2:
                return Owned<XAudio2AudioBackend>::Create();
        }

        TBO_ENGINE_ASSERT(false, "Unknown AudioBackend!");
        return nullptr;
    }

}
