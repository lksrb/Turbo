#pragma once

#include "Assert.h"

#include <string>
#include <vcruntime_string.h>

namespace Turbo {
    // Fast string
    template<size_t Capacity>
    struct StringB
    {
    public:
        StringB() = default;

        StringB(const std::string& str)
            : StringB(str.c_str())
        {
        }

        StringB(const char* other) 
            : StringB()
        {
            if (other)
                Copy(other);
        }

        StringB& operator=(const char* other)
        {
            if (other)
                Copy(other);

            return *this;
        }

        StringB& operator=(char* other)
        {
            if (other)
                Copy((const char*)other);

            return *this;
        }

        StringB& Append(const char* other)
        {
            TBO_ENGINE_ASSERT(strlen(m_Buffer) + strlen(other) < Cap());
            if (other)
            {
                strcat_s(m_Buffer, other);
                m_Size = strlen(m_Buffer);
            }
            return *this;
        }

        StringB& Append(const StringB& other)
        {
            return StringB::Append(other.CStr());
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
            return !(StringB::operator==(other));
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

        const char* CStr() const noexcept
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
        size_t m_Size = 0;
        char m_Buffer[Capacity] = { 0 };
        
    };

    using String32 = StringB<32>;
    using String64 = StringB<64>;
    using String128 = StringB<128>;
    using String = StringB<256>;
}

namespace std
{
    template <>
    struct hash<Turbo::String32>
    {
        auto operator()(const Turbo::String32& xyz) const -> size_t
        {
            return hash<Turbo::String32>{}(xyz.CStr());
        }
    };
}  //
