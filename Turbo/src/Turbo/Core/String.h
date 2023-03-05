#pragma once

#include "Assert.h"

#include <string>
#include <vcruntime_string.h>

namespace Turbo 
{
    // Fixed String TODO: Add WideString variant
    template<typename CharType, size_t Capacity>
    struct StringB
    {
    public:
        struct Iterator
        {
            Iterator(CharType* pointer) : m_Pointer(pointer) {}

            Iterator& operator++()
            {
                ++m_Pointer;
                return *this;
            }

            Iterator& operator++(int)
            {
                Iterator temp = *this;
                ++(*this);
                return temp;
            }
            Iterator& operator--()
            {
                --m_Pointer;
                return *this;
            }

            Iterator& operator--(int)
            {
                Iterator temp = *this;
                --(*this);
                return temp;
            }

            CharType& operator*() const { return *m_Pointer; }

            bool operator==(const Iterator& other) const { return m_Pointer == other.m_Pointer; }
            bool operator!=(const Iterator& other) const { return !operator==(other); }
        private:
            CharType* m_Pointer;

            friend struct StringB;
        };
    public:
        StringB() = default;
        virtual ~StringB() = default;

        StringB(const std::string& str)
            : StringB(str.c_str())
        {
        }

        StringB(const CharType* other)
            : StringB()
        {
            if (other)
                Copy(other);
        }

        StringB(const Iterator& it) : StringB(it.m_Pointer) {}

        // Utilities
        Iterator Begin()
        {
            return Iterator(&m_Buffer[0]);
        }

        Iterator End()
        {
            return Iterator(&m_Buffer[0] + (m_Size)); // Null termination character
        }

        StringB& Append(const CharType* other)
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

        Iterator Erase(Iterator& where)
        {
            size_t index = where.m_Pointer - &m_Buffer[0];

            StringB after(++where);
            ClearRange(index, m_Size);
            Append(after);
            return Iterator(&m_Buffer[index]);
        }

        // Operators
        StringB& operator=(const CharType* other)
        {
            if (other)
                Copy(other);

            return *this;
        }

        StringB& operator=(CharType* other)
        {
            if (other)
                Copy((const CharType*)other);

            return *this;
        }

        StringB operator+(const StringB& other) { return StringB::Append(other); }
        //StringB operator+(const CharType* other) const { return StringB::Append(other.CStr()); }

        CharType& operator[](size_t index) { TBO_ENGINE_ASSERT(index < m_Size/*, "Index out of bounds!"*/); return m_Buffer[index]; }
        bool operator==(const CharType* other) const { return strcmp(m_Buffer, other) == 0; }
        bool operator!=(const CharType* other) const { return !(StringB::operator==(other)); }
        //const CharType* operator()() const { return m_Buffer; }

        bool Empty() const { return m_Size == 0; }
        void Clear() { ClearRange(0, m_Size); }
        inline size_t Size() const { return m_Size; }
        inline static constexpr size_t Cap() noexcept { return Capacity; }
        CharType* Data() { return &m_Buffer[0]; }
        const CharType* CStr() const { return m_Buffer; }
        //const CharType* operator()() { return &m_Buffer[0]; }
        template<typename... Args>
        static StringB Format(const StringB& format, Args&&... args)
        {
            //StringB string = (StringB(args) + ...);
            //
            //std::cout << string.CStr() << "\n";
            TBO_ENGINE_ASSERT(false);
            return {};
        }
    protected:
        void Copy(const CharType* src)
        {
            m_Size = strlen(src);
            TBO_ENGINE_ASSERT(m_Size < Capacity);
            strcpy_s(m_Buffer, Capacity, src);
        }

        void CopyRange(const CharType* src, size_t end_index)
        {
            TBO_ENGINE_ASSERT(m_Size >= 0);
            TBO_ENGINE_ASSERT(m_Size < Capacity);

            StringB copy = src;
            copy.m_Buffer[end_index] = '\0';

            strcpy_s(&m_Buffer[0], Capacity, copy.m_Buffer);

            m_Size = strlen(m_Buffer);
        }

        void ClearRange(size_t start, size_t end)
        {
            m_Size -= (end - start);
            TBO_ENGINE_ASSERT(m_Size >= 0);
            memset(m_Buffer + start, 0, end);
        }
    protected:
        size_t m_Size = 0;
        CharType m_Buffer[Capacity] = { 0 };
    };

    using String32  = StringB<char, 32>;
    using String    = StringB<char, 256>;
}

namespace std
{
    template <>
    struct hash<Turbo::String>
    {
        auto operator()(const Turbo::String& xyz) const -> size_t
        {
            return hash<Turbo::String>{}(xyz.CStr());
        }
    };
}  //

/*
template<typename OStream>
OStream& operator<<(OStream& os, const Turbo::String& string)
{
    return os << string.CStr();
}

template<typename OStream>
OStream& operator<<(OStream& os, const Turbo::String32& string)
{
    return os << string.CStr();
}

template<typename OStream>
OStream& operator<<(OStream& os, const Turbo::String64& string)
{
    return os << string.CStr();
}

template<typename OStream>
OStream& operator<<(OStream& os, const Turbo::String128& string)
{
    return os << string.CStr();
}
*/
