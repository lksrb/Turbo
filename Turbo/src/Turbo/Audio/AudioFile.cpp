#include "tbopch.h"
#include "AudioFile.h"

#include "Turbo/Core/FileSystem.h"

namespace Turbo
{
    AudioFile::AudioFile(std::string_view filepath)
    {
        Load(filepath);
    }

    AudioFile::~AudioFile()
    {
        AudioData.Release();
    }

    void AudioFile::Load(std::string_view filepath)
    {
        Buffer buffer = FileSystem::ReadBinary(filepath);
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
        AudioData.Allocate(audioDataSize);
        AudioData.CopySection(&buffer[44], audioDataSize);
    }


}
