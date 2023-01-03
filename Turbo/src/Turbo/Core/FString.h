#pragma once

#include "Assert.h"

#include <string>
#include <vcruntime_string.h>

namespace Turbo {
    // Fast string
    template<size_t Capacity>
    struct FString
    {
    public:
        FString() : m_Buffer{ 0 }, m_Size{ 0 } {}

        FString(const std::string& str)
            : FString(str.c_str())
        {
        }

        FString(const char* other) 
            : FString()
        {
            if (other)
                Copy(other);
        }

        FString& operator=(const char* other)
        {
            if (other)
                Copy(other);

            return *this;
        }

        FString& operator=(char* other)
        {
            if (other)
                Copy((const char*)other);

            return *this;
        }

        FString& Append(const char* other)
        {
            TBO_ENGINE_ASSERT(strlen(m_Buffer) + strlen(other) < Cap());
            if (other)
            {
                strcat_s(m_Buffer, other);
                m_Size = strlen(m_Buffer);
            }
            return *this;
        }

        FString& Append(const FString& other)
        {
            return FString::Append(other.c_str());
        }

        bool Empty() const
        {
            return m_Size == 0;
        }
        /*
                char& operator[](size_t index) final
                {
                    TBO_ENGINE_ASSERT(index <= m_Size);
                    return m_Buffer[index];
                }*/

        bool operator==(const char* other) const
        {
            return strcmp(m_Buffer, other) == 0;
        }

        bool operator!=(const char* other) const
        {
            return !(FString::operator==(other));
        }

        inline size_t Size() const { return m_Size; }
        inline constexpr size_t Cap() const { return Capacity; }

        void Reset()
        {
            memset(m_Buffer, 0, sizeof(m_Size));
        }

        const char* operator()() const
        {
            return m_Buffer;
        }

        char* data()
        {
            return &m_Buffer[0];
        }

        const char* c_str() const noexcept
        {
            return m_Buffer;
        }
    protected:
        void Copy(const char* src)
        {
            m_Size = strlen(src);
            TBO_ENGINE_ASSERT(m_Size < Capacity);
            strcpy_s(m_Buffer, Capacity, src);
        }
    protected:
        size_t m_Size;
        char m_Buffer[Capacity];
    };

    using FString32 = FString<32>;
    using FString64 = FString<64>;
    using FString128 = FString<128>;
    using FString256 = FString<256>;

}

namespace std
{
    template <>
    struct hash<Turbo::FString32>
    {
        auto operator()(const Turbo::FString32& xyz) const -> size_t
        {
            return hash<Turbo::FString32>{}(xyz.c_str());
        }
    };
}  //
