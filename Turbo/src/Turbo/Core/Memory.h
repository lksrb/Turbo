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

    // Light wrapper around shared_ptr
    template<typename T>
    class Ref
    {
    public:
        Ref() = default;

        explicit Ref(T* instance) 
            : m_Instance(instance)
        {
            if (m_Instance)
                m_Counter = new uint32_t{ 0 };

            IncRef();
        }

        Ref(const Ref<T>& other)
            : m_Instance(other.m_Instance), m_Counter(other.m_Counter)
        {
            IncRef();
        }

        template<typename T2>
        Ref(const Ref<T2>& other)
        {
            m_Instance = (T*)other.m_Instance;
            m_Counter = other.m_Counter;

            IncRef();
        }

        ~Ref()
        {
            DecRef();
        }

        Ref& operator=(std::nullptr_t)
        {
            DecRef();
            m_Instance = nullptr;
            m_Counter = nullptr;
            return *this;
        }

        Ref& operator=(const Ref<T>& other)
        {
            // Increase ref count to the other ref
            other.IncRef();

            // Decreace ref count to this, cause it is assign operator
            DecRef();

            m_Instance = other.m_Instance;
            m_Counter = other.m_Counter;

            return *this;
        }

        template<typename T2>
        Ref& operator=(const Ref<T2>& other)
        {
            other.IncRef();
            DecRef();

            m_Instance = other.m_Instance;
            m_Counter = other.m_Counter;
            return *this;
        }

        bool operator==(const Ref<T>& other) const
        {
            return m_Instance == other.m_Instance;
        }

        bool operator!=(const Ref<T>& other) const
        {
            return !(*this == other);
        }

        template<typename... Args>
        static Ref<T> Create(Args&&... args)
        {
            return Ref<T>(new T(std::forward<Args>(args)...));
        }

        operator bool() { return m_Instance != nullptr; }
        operator bool() const { return m_Instance != nullptr; }

        T* operator->() { return m_Instance; }
        const T* operator->() const { return m_Instance; }

        T& operator*() { return *m_Instance; }
        const T& operator*() const { return *m_Instance; }

        T* Get() { return  m_Instance; }
        const T* Get() const { return  m_Instance; }

        void Reset(T* instance = nullptr)
        {
            DecRef();

            // New instance
            m_Instance = instance;

            // Create counter only if newly assigned instance in not nullptr
            if(m_Instance)
                m_Counter = new uint32_t{ 0 };
        }

        template<typename T2>
        Ref<T2> As() const
        {
            return Ref<T2>(*this);
        }

    private:
        void IncRef() const
        {
            if (m_Instance && m_Counter)
            {
                ++(*m_Counter);
            }
        }

        void DecRef() const
        {
            if (m_Instance && m_Counter)
            {
                if (--(*m_Counter) == 0)
                {
                    delete m_Instance;
                    delete m_Counter;
                    m_Counter = nullptr;
                    m_Instance = nullptr;
                }
            }
        }
    private:
        mutable uint32_t* m_Counter = nullptr;
        mutable T* m_Instance = nullptr;

        template<typename T2>
        friend class Ref;
    };
}
