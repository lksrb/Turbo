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

        void CopySection(const u8* beginIndex, u64 size)
        {
            TBO_ENGINE_ASSERT(Data);
            memcpy(Data, beginIndex, size);
        }

        template<typename T>
        T* As()
        {
            return reinterpret_cast<T*>(Data);
        }

        u8& operator[](u64 index)
        {
            TBO_ENGINE_ASSERT(index < Size, "Index is out of bounds!");
            return Data[index];
        }

        u8& operator[](u64 index) const
        {
            TBO_ENGINE_ASSERT(index < Size, "Index is out of bounds!");
            return Data[index];
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

        u8& operator[](u64 index)
        {
            TBO_ENGINE_ASSERT(index < Size(), "Index is out of bounds!");
            return m_Buffer[index];
        }

        u8* Data() { return m_Buffer.Data; }
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
