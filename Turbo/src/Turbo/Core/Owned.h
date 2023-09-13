#pragma once

namespace Turbo {

    template<typename T>
    class Owned;

    template<typename T>
    class OwnedRef
    {
    public:
        template<typename T2>
        OwnedRef(const Owned<T2>& owned) 
            : m_RefPointer((T*)owned.Get())
        {
        }

        template<typename T2>
        inline OwnedRef<T2> As() const noexcept 
        {
            return OwnedRef<T2>(m_RefPointer);
        }

        inline const T& operator*() const noexcept { return *m_RefPointer; }
        inline T& operator*() noexcept { return *m_RefPointer; }

        inline T* operator->() noexcept { return m_RefPointer; }
        inline const T* operator->() const noexcept { return m_RefPointer; }

        inline operator bool() noexcept { return static_cast<bool>(m_RefPointer); }
        inline operator bool() const noexcept { return static_cast<bool>(m_RefPointer); }
    private:
        template<typename T2>
        explicit OwnedRef(T2* pointer)
            : m_RefPointer((T*)pointer)
        {
        }

        template<typename T2>
        friend class OwnedRef;

        OwnedRef() = default;

        T* m_RefPointer;

        template<typename T>
        friend class Owned;
    };

    // Behaves almost exactly like std::unique_ptr except we create an OwnedRef.
    // OwnedRef is a reference class containing the pointer to provide it when needed
    template<typename T>
    class Owned
    {
    public:
        constexpr Owned() noexcept 
            : m_Pointer(nullptr) 
        {
        }

        constexpr Owned(nullptr_t) noexcept 
            : m_Pointer(nullptr) 
        {
        }

        explicit Owned(T* pointer) noexcept 
            : m_Pointer(pointer) 
        {
        }

        inline ~Owned() noexcept 
        { 
            Reset();
        }

        template<typename T2>
        inline Owned(Owned<T2>&& other) noexcept 
            : m_Pointer(other.m_Pointer) 
        { 
            other.m_Pointer = nullptr; 
        }

        inline T* Get() noexcept { return m_Pointer; }
        inline const T* Get() const noexcept { return m_Pointer; }

        inline void Reset(T* pointer = nullptr) noexcept 
        { 
            delete m_Pointer; 
            m_Pointer = pointer; 
        }

        template<typename T2>
        inline OwnedRef<T2> As() const noexcept
        {
            return OwnedRef<T2>(m_Pointer);
        }

        template<typename... Args>
        static inline Owned<T> Create(Args&&... args)
        {
#if TBO_TRACK_MEMORY
            return Owned<T>(new(__FILE__, __LINE__) T(std::forward<Args>(args)...));
#else
            return Owned<T>(new T(std::forward<Args>(args)...));
#endif
        }

        template<typename T2>
        Owned& operator=(Owned<T2>&& right) noexcept
        {
            if (*this != right)
            {
                m_Pointer = (T*)right.m_Pointer;
                right.m_Pointer = nullptr;
            }
            return *this;
        }

        template<typename T2>
        bool operator==(const Owned<T2>& other) const
        {
            return m_Pointer == (T*)other.m_Pointer;
        }

        bool operator!=(const Owned<T>& other) const
        {
            return !(*this == other);
        }

        inline const T& operator*() const noexcept { return *m_Pointer; }
        inline T& operator*() noexcept { return *m_Pointer; }

        inline T* operator->() noexcept { return m_Pointer; }
        inline const T* operator->() const noexcept { return m_Pointer; }

        inline operator bool() noexcept { return static_cast<bool>(m_Pointer); }
        inline operator bool() const noexcept { return static_cast<bool>(m_Pointer); }

        inline Owned& operator=(nullptr_t) noexcept { Reset(); return *this; }

        // No copy constructor or copy assign
        Owned(const Owned&) = delete;
        Owned& operator=(const Owned&) = delete;
    private:
        template<typename T2>
        friend class Owned;

        T* m_Pointer;
    };

}
