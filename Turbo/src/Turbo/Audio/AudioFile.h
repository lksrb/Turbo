#pragma once

#include "Turbo/Core/Buffer.h"

namespace Turbo
{
    struct AudioFile
    {
        u32 ChunkSize = 0;

        u32 FmtSize = 0; // 16 for PCM
        u16 AudioFormat = 0;
        u16 NumChannels = 0;
        u32 SampleRate = 0;
        u32 ByteRate = 0;
        u16 BlockAlign = 0;
        u16 BitsPerSample = 0;

        // if not PCM
        u16 ExtraParamSize = 0;

        // Data
        u32 DataOffset = 0;
        Buffer Data;

        AudioFile() = default;
        explicit AudioFile(std::string_view filepath);
        AudioFile& operator=(AudioFile&& other) noexcept;
        AudioFile(AudioFile&& other) noexcept;

        ~AudioFile();

        void Load(std::string_view filepath);
        void LoadFromMemory(const Buffer& buffer);

        void Release();

        inline operator bool() const { return (bool)Data; }
    private:
        void Move(AudioFile&& other);
    };
}
