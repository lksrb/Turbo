#pragma once

#include "Turbo/Core/Assert.h"

#include <memory>
#include <new>

namespace Turbo::Memory
{
    void Initialize();
    void Shutdown();
}

namespace Turbo
{
    /* template<typename T>
     using Scope = std::unique_ptr<T>;
     template<typename T, typename ... Args>
     constexpr Scope<T> CreateScope(Args&& ... args)
     {
         return std::make_unique<T>(std::forward<Args>(args)...);
     }

     template<typename T>
     using Ref = std::shared_ptr<T>;

     template<typename T, typename ... Args>
     constexpr Ref<T> CreateRef(Args&& ... args)
     {
         return std::make_shared<T>(std::forward<Args>(args)...);
     }

     template<typename T>
     using WeakRef = std::weak_ptr<T>;*/

    namespace
    {
        template<typename T>
        T* AllocateFromPool() {}
    }

    // Just a helper class for handling pointer
    template<typename T>
    class Ptr
    {
    public:
        Ptr(T* pointer = nullptr) : m_Pointer(pointer) {}
        ~Ptr() { m_Pointer = nullptr; }

        Ptr(const Ptr& other) : m_Pointer(other.m_Pointer){ }
        Ptr& operator=(const Ptr& other) { m_Pointer = other.m_Pointer; return *this; }

        void Release(T* otherPointer = nullptr) { delete m_Pointer; m_Pointer = nullptr; }

        T* operator->()
        {
            TBO_ENGINE_ASSERT(m_Pointer);
            return m_Pointer;
        }

        const T* operator->() const
        {
            TBO_ENGINE_ASSERT(m_Pointer);
            return m_Pointer;
        }

        operator T* ()
        {
            return m_Pointer;
        }

        operator T* () const
        {
            return m_Pointer;
        }

        //Ptr& operator++()
        //{
        //    TBO_ENGINE_ASSERT(m_Pointer);
        //    m_Pointer++;
        //    return *this;
        //}

        template<typename T2>
        T2* As()
        {
            TBO_ENGINE_ASSERT(m_Pointer);
            return static_cast<T2*>(m_Pointer);
        }

        template<typename T2>
        const T2* As() const
        {
            TBO_ENGINE_ASSERT(m_Pointer);
            return static_cast<const T2*>(m_Pointer);
        }

       /* template<typename T2>
        const Ptr<T2> As() const
        {
            TBO_ENGINE_ASSERT(m_Pointer);
            return Ptr<T2>(static_cast<const T2*>(m_Pointer));
        }*/
    private:
        T* m_Pointer;
    };

    class StackAllocator
    {
    public:

    private:
    };

    template<typename T, typename... Args>
    constexpr T* Create(Args&&... args)
    {
        T* object = AllocateFromPool<T>();
        new (object)T(std::move<Args>(args)...);

        return object;
    }

}
