#include "tbopch.h"
#include "AudioFile.h"

#include "Turbo/Core/FileSystem.h"

namespace Turbo
{
    AudioFile::AudioFile(std::string_view filepath)
    {
        Load(filepath);
    }

    AudioFile::AudioFile(AudioFile&& other) noexcept
    {
        Data = std::move(other.Data);
        ChunkSize = other.ChunkSize;
        FmtSize = other.FmtSize;
        AudioFormat = other.AudioFormat;
        NumChannels = other.NumChannels;
        SampleRate = other.SampleRate;
        ByteRate = other.ByteRate;
        BlockAlign = other.BlockAlign;
        BitsPerSample = other.BitsPerSample;
        ExtraParamSize = ExtraParamSize;
        DataOffset = other.DataOffset;

        // NOTE: memset does not work I assume?
        other.ChunkSize = 0;
        other.FmtSize = 0;
        other.AudioFormat = 0;
        other.NumChannels = 0;
        other.SampleRate = 0;
        other.ByteRate = 0;
        other.BlockAlign = 0;
        other.BitsPerSample = 0;
        other.ExtraParamSize = 0;
        other.DataOffset = 0;
    }

    AudioFile::~AudioFile()
    {
        Release();
    }

    AudioFile& AudioFile::operator=(AudioFile&& other) noexcept
    {
        if (this != &other)
        {
            Data = std::move(other.Data);
            ChunkSize = other.ChunkSize;
            FmtSize = other.FmtSize;
            AudioFormat = other.AudioFormat;
            NumChannels = other.NumChannels;
            SampleRate = other.SampleRate;
            ByteRate = other.ByteRate;
            BlockAlign = other.BlockAlign;
            BitsPerSample = other.BitsPerSample;
            ExtraParamSize = ExtraParamSize;
            DataOffset = other.DataOffset;

            // NOTE: memset does not work I assume?
            other.ChunkSize = 0;
            other.FmtSize = 0;
            other.AudioFormat = 0;
            other.NumChannels = 0;
            other.SampleRate = 0;
            other.ByteRate = 0;
            other.BlockAlign = 0;
            other.BitsPerSample = 0;
            other.ExtraParamSize = 0;
            other.DataOffset = 0;
        }

        return *this;
    }

    void AudioFile::Load(std::string_view filepath)
    {
        Buffer buffer = FileSystem::ReadBinary(filepath);

        if (!buffer)
        {
            TBO_ENGINE_ERROR("Could not load audio file! {}", filepath);
            return;
        }

        LoadFromMemory(buffer);
        buffer.Release();
    }

    void AudioFile::LoadFromMemory(const Buffer& buffer)
    {
        char headerChunkID[5] = {};
        char format[5] = {};
        char fmtSection[5] = {};
        char dataSection[5] = {};

        memcpy(headerChunkID, &buffer[0], 4);               // Riff section
        memcpy(format, &buffer[8], 4);                      // Format
        memcpy(fmtSection, &buffer[12], 4);                 // FMT section
        memcpy(&dataSection, &buffer[36], 4);               // Data section

        // strcmp(dataSection, "data")

        // Checking if the buffer is valid WAVE
        if (strcmp(headerChunkID, "RIFF") + strcmp(format, "WAVE") + strcmp(fmtSection, "fmt ") + strcmp(dataSection, "data") != 0)
        {
            TBO_ENGINE_ERROR("Bad format!");
            return;
        }

        memcpy(&ChunkSize, &buffer[4], 4);
        memcpy(&FmtSize, &buffer[16], 4);
        memcpy(&AudioFormat, &buffer[20], 2);
        memcpy(&NumChannels, &buffer[22], 2);
        memcpy(&SampleRate, &buffer[24], 4);
        memcpy(&ByteRate, &buffer[28], 4);
        memcpy(&BlockAlign, &buffer[32], 2);
        memcpy(&BitsPerSample, &buffer[34], 2);

        u32 audioDataSize;
        memcpy(&audioDataSize, &buffer[40], 4);
        Data.Allocate(audioDataSize);
        Data.Copy(&buffer[44], audioDataSize);
    }

    void AudioFile::Release()
    {
        Data.Release();
    }

    void AudioFile::Move(AudioFile&& other)
    {
        Data = std::move(other.Data);
        ChunkSize = other.ChunkSize;
        FmtSize = other.FmtSize;
        AudioFormat = other.AudioFormat;
        NumChannels = other.NumChannels;
        SampleRate = other.SampleRate;
        ByteRate = other.ByteRate;
        BlockAlign = other.BlockAlign;
        BitsPerSample = other.BitsPerSample;
        ExtraParamSize = ExtraParamSize;
        DataOffset = other.DataOffset;

        // NOTE: memset does not work I assume?
        other.ChunkSize = 0;
        other.FmtSize = 0;
        other.AudioFormat = 0;
        other.NumChannels = 0;
        other.SampleRate = 0;
        other.ByteRate = 0;
        other.BlockAlign = 0;
        other.BitsPerSample = 0;
        other.ExtraParamSize = 0;
        other.DataOffset = 0;
    }

}
