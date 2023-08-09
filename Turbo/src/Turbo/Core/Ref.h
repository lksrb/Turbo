#pragma once

#include "PrimitiveTypes.h"
#include <atomic>

namespace Turbo {

    namespace RefUtils {
        void AddToLiveReferences(void* instance);
        void RemoveFromLiveReferences(void* instance);
        bool IsAlive(void* instance);
    }

    class RefCounted
    {
    public:
        u32 GetRefCount() const { return m_RefCount.load(); }

        virtual ~RefCounted() = default;
    private:
        void IncRef() const { ++m_RefCount; }
        void DecRef() const { --m_RefCount; }

        mutable std::atomic<u32> m_RefCount = 0;

        template<typename T>
        friend class Ref;
    };


    template<typename T>
    class Ref
    {
    public:
        Ref() : m_Instance(nullptr) {}

        Ref(std::nullptr_t n) : m_Instance(nullptr) {}

        Ref(T* instance)
            : m_Instance(instance)
        {
            static_assert(std::is_base_of<RefCounted, T>::value, "Class is not RefCounted!");

            IncRef();
        }

        template<typename T2>
        Ref(const Ref<T2>& other)
        {
            m_Instance = (T*)other.m_Instance;
            IncRef();
        }

        template<typename T2>
        Ref(Ref<T2>&& other)
        {
            m_Instance = (T*)other.m_Instance;
            other.m_Instance = nullptr;
        }

        ~Ref()
        {
            DecRef();
        }

        Ref(const Ref<T>& other)
            : m_Instance(other.m_Instance)
        {
            IncRef();
        }

        Ref& operator=(std::nullptr_t)
        {
            DecRef();
            m_Instance = nullptr;
            return *this;
        }

        Ref& operator=(const Ref<T>& other)
        {
            other.IncRef();
            DecRef();

            m_Instance = other.m_Instance;
            return *this;
        }

        template<typename T2>
        Ref& operator=(const Ref<T2>& other)
        {
            other.IncRef();
            DecRef();

            m_Instance = other.m_Instance;
            return *this;
        }

        template<typename T2>
        Ref& operator=(Ref<T2>&& other)
        {
            DecRef();

            m_Instance = other.m_Instance;
            other.m_Instance = nullptr;
            return *this;
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
            m_Instance = instance;
        }

        template<typename T2>
        Ref<T2> As() const
        {
            return Ref<T2>(*this);
        }

        template<typename... Args>
        static Ref<T> Create(Args&&... args)
        {
#if TBO_TRACK_MEMORY
            return Ref<T>(new(typeid(T).name()) T(std::forward<Args>(args)...));
#else
            return Ref<T>(new T(std::forward<Args>(args)...));
#endif
        }

        bool operator==(const Ref<T>& other) const
        {
            return m_Instance == other.m_Instance;
        }

        bool operator!=(const Ref<T>& other) const
        {
            return !(*this == other);
        }

        bool EqualsObject(const Ref<T>& other)
        {
            if (!m_Instance || !other.m_Instance)
                return false;

            return *m_Instance == *other.m_Instance;
        }
    private:
        void IncRef() const
        {
            if (m_Instance)
            {
                m_Instance->IncRef();
                RefUtils::AddToLiveReferences((void*)m_Instance);
            }
        }

        void DecRef() const
        {
            if (m_Instance)
            {
                m_Instance->DecRef();
                if (m_Instance->GetRefCount() == 0)
                {
                    delete m_Instance;
                    RefUtils::RemoveFromLiveReferences((void*)m_Instance);
                    m_Instance = nullptr;
                }
            }
        }

        template<class T2>
        friend class Ref;
        mutable T* m_Instance;
    };

    template<typename T>
    class WeakRef
    {
    public:
        WeakRef() = default;

        template<typename T2>
        WeakRef(const WeakRef<T2>& other)
        {
            m_Instance = (T*)other.m_Instance;
        }

        WeakRef(Ref<T> ref)
        {
            m_Instance = ref.Get();
        }

        WeakRef(T* instance)
        {
            m_Instance = instance;
        }

        bool IsValid() const { m_Instance ? return RefUtils::IsAlive(m_Instance) : false; }
        operator bool() const { return IsValid(); }

        T* operator->() { return m_Instance; }
        const T* operator->() const { return m_Instance; }

        T& operator*() { return *m_Instance; }
        const T& operator*() const { return *m_Instance; }


        template<typename T2>
        WeakRef<T2> As() const
        {
            return WeakRef<T2>(*this);
        }

    private:
        T* m_Instance = nullptr;

        template<typename T2>
        friend class WeakRef;
    };
}
