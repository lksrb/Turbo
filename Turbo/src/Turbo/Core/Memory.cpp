#include "tbopch.h"
#include "Memory.h"

namespace Turbo 
{
    constexpr size_t DefaultPoolSize = size_t(100) * size_t(1024) * size_t(1024);

    struct MemoryInternal
    {
        u8* MemoryPool;

        MemoryInternal()
        {
            memset(this, 0, sizeof(*this));
        }
    };

    // Statically allocated
    static MemoryInternal s_Internal;

    void Memory::Initialize()
    {
        //s_Internal.MemoryPool = new u8[DefaultPoolSize];
        // TODO: Memory
    }

    void Memory::Shutdown()
    {
        //delete s_Internal.MemoryPool;
        //s_Internal.MemoryPool = nullptr;
    }

}
