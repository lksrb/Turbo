#include "tbopch.h"
#include "AudioBackend.h"

#include "Audio.h"

#include "Turbo/Platform/XAudio2/XAudio2AudioBackend.h"

namespace Turbo
{
    Ref<AudioBackend> AudioBackend::Create()
    {
        switch(Audio::GetAudioBackend())
        {
            case Audio::BackendType::XAudio2:
                return Ref<XAudio2AudioBackend>::Create();
        }

        TBO_ENGINE_ASSERT(false, "Unknown AudioBackend!");
        return nullptr;
    }

}
