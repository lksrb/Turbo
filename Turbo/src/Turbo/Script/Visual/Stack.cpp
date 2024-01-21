#include "tbopch.h"
#include "Stack.h"

namespace Turbo {

    Stack::Stack(u32 capacity)
    {
        Grow(capacity);
    }

    Stack::~Stack()
    {
        ::operator delete(m_Base, m_Capacity * sizeof(AnyData));
        m_StackPtr = m_Base = nullptr;
    }

    AnyData* Stack::Top()
    {
        return m_StackPtr - 1;
    }

    AnyData* Stack::operator[](u32 index)
    {
        TBO_ENGINE_ASSERT(index < m_Size, "Out of range.");
        return &m_Base[index];
    }

    u32 Stack::PushEmpty()
    {
        if (m_Size == m_Capacity) [[unlikely]]
            Grow(m_Capacity * 2);

            ++m_StackPtr;
            return m_Size++;
    }

    void Stack::Reset()
    {
        m_StackPtr = m_Base;
        m_Size = 0;
    }

	void Stack::Grow(u32 newCapacity)
    {
        // Allocate new block 
        AnyData* newBase = (AnyData*)::operator new(newCapacity * sizeof(AnyData));

        // Fast copy
        AnyData* dst = newBase;
        AnyData* src = m_Base;

        // Copy data to the new block
        for (u32 i = 0; i < m_Size; ++i)
            *(dst++) = *(src++);

        // Free old block 
        ::operator delete(m_Base, m_Capacity * sizeof(AnyData));

        // Assign everything
        m_Base = newBase;
        m_StackPtr = dst;
        m_Capacity = newCapacity;
    }
}
