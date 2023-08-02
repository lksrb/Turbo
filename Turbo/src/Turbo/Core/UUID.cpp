#include "tbopch.h"
#include "UUID.h"

#include <iostream>

#include <random>

namespace Turbo {

    // Lets keep this incase those numbers are not as random as I think
#if 0
    static std::random_device s_RandomDevice;
    static std::mt19937_64 s_Engine(s_RandomDevice());
    static std::uniform_int_distribution<u64> s_UniformDistribution;
    UUID::UUID()
        : m_UUID(s_UniformDistribution(s_RandomDevice))
    {
    }
#else
    // This should be random everytime the engine startup
    static const u64 s_RandomSeed = std::random_device{}();

    // To ensure that UUIDs have unique seeds
    static u64 s_UUIDCounter = 0;

    // Since we do not need cryptographically secure hashes
    // We can use this hash function to generate random UUID with practically zero cost
    u64 PCG_Hash()
    {
        u64 seed = s_RandomSeed * (++s_UUIDCounter);
        seed = seed ^ (seed >> 33);
        seed = seed * 0xff51afd7ed558ccd;
        seed = seed ^ (seed >> 33);
        seed = seed * 0xc4ceb9fe1a85ec53;
        seed = seed ^ (seed >> 33);
        return seed;
    }

    UUID::UUID()
        : m_UUID(PCG_Hash())
    {
    }
#endif

    UUID::UUID(u64 uuid)
        : m_UUID(uuid)
    {
    }
}
