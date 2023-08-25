#pragma once

#include "Turbo/Core/PrimitiveTypes.h"

namespace Turbo
{
    struct Buffer
    {
        Buffer() = default;
        Buffer(const Buffer&) = default;

        explicit Buffer(u64 size)
        {
            Allocate(size);
        }

        Buffer(Buffer&& other) noexcept
        {
            Data = other.Data;
            Size = other.Size;

            other.Data = nullptr;
            other.Size = 0;
        }

        Buffer& operator=(Buffer&& other) noexcept
        {
            if (this != &other)
            {
                Data = other.Data;
                Size = other.Size;

                other.Data = nullptr;
                other.Size = 0;
            }

            return *this;
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

        void Copy(const u8* srcData, u64 srcSize, u64 dstOffset = 0)
        {
            TBO_ENGINE_ASSERT(Data);
            memcpy(Data + dstOffset, srcData, srcSize);
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
