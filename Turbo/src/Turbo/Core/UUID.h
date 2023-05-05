#pragma once

#include "Turbo/Core/PrimitiveTypes.h"

#include <xhash>

namespace Turbo
{
    // UUID
    class UUID
    {
    public:
        UUID();
        UUID(u64 uuid);
        UUID(const UUID&) = default;

        constexpr static inline u64 Null = 0;

        operator u64() const { return m_UUID; }
    private:
        u64 m_UUID;
    };
}

// Hashing
namespace std
{
    template <typename T> struct hash;

    template<>
    struct hash<Turbo::UUID>
    {
        ::std::size_t operator()(const Turbo::UUID& uuid) const
        {
            return (Turbo::u64)uuid;
        }
    };
}
