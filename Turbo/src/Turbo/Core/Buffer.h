#pragma once

#include "Turbo/Core/PrimitiveTypes.h"

namespace Turbo
{
    struct Buffer
    {
        Buffer() = default;
        Buffer(const Buffer&) = default;

        Buffer(u64 size)
        {
            Allocate(size);
        }

        void Allocate(u64 size)
        {
            Release();

            Data = new u8[size];
            Size = size;
        }

        void Release()
        {
            delete[] Data;
            Data = nullptr;
            Size = 0;
        }

        template<typename T>
        T* As()
        {
            return reinterpret_cast<T*>(Data);
        }

        operator bool() const
        {
            return (bool)Data;
        }

        u8* Data = nullptr;
        u64 Size = 0;
    };

    struct ScopedBuffer
    {
        ScopedBuffer(const Buffer& buffer)
            : m_Buffer(buffer)
        {
        }

        ScopedBuffer(u64 size)
            : m_Buffer(size)
        {
        }

        ~ScopedBuffer()
        {
            m_Buffer.Release();
        }

        uint8_t* Data() { return m_Buffer.Data; }
        u64 Size() { return m_Buffer.Size; }

        template<typename T>
        T* As()
        {
            return m_Buffer.As<T>();
        }

        operator bool() const { return m_Buffer; }
    private:
        Buffer m_Buffer;
    };
}
