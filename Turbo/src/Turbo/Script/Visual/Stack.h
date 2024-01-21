#pragma once

#include "Turbo/Core/PrimitiveTypes.h"
#include "Turbo/Core/Assert.h"

#define TBO_CHECK_TYPE 0

namespace Turbo {

    struct AnyData
    {
        u8 Value[16]; // 16 bytes should be enough to represent everything we need
#if TBO_CHECK_TYPE
        u64 HashType = 0;
#endif
        AnyData() = default;

        template<typename T>
        AnyData(T value) : Value{ 0 }
        {
            static_assert(sizeof(T) <= 16, "Type is too large!");
            memcpy(Value, &value, sizeof(T));
#if TBO_CHECK_TYPE
            HashType = typeid(T).hash_code();
#endif
        }

        template<typename T>
        T As()
        {
            static_assert(sizeof(T) <= 16, "Type is too large!");

#if TBO_CHECK_TYPE
            TBO_ENGINE_ASSERT(HashType == typeid(T).hash_code(), "Types does not match.");
#endif
            T value;
            memcpy(&value, Value, sizeof(T));

            return value;
        }
    };

    // Growing First-In-First-Out stack implementation
    class Stack
    {
    public:
        Stack(u32 capacity = 128);
        ~Stack();

        AnyData* Top();
        AnyData* operator[](u32 index);

        // Return stack index
        template<typename T>
        u32 Push(T&& data)
        {
            if (m_Size == m_Capacity) [[unlikely]]
                Grow(m_Capacity * 2);

                *m_StackPtr = AnyData(std::forward<T>(data));
                ++m_StackPtr;
                return m_Size++;
        }

        // Return stack index
        u32 PushEmpty();

        template<typename T>
        T Pop()
        {
            TBO_ENGINE_ASSERT(m_Size > 0, "Stack is empty.");

            AnyData result;
            result = *Top();
            --m_StackPtr;
            --m_Size;
            return result.As<T>();
        }

        void Reset();

        u32 Size() const { return m_Size; }
    private:
        void Grow(u32 newCapacity);

    private:
        u32 m_Capacity = 0;
        u32 m_Size = 0;
        AnyData* m_StackPtr = nullptr;
        AnyData* m_Base = nullptr;
    };
}
