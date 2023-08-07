#pragma once

#include "Turbo/Core/Assert.h"

#include <memory>
#include <new>

//#define TBO_PROFILE_MEMORY

namespace Turbo
{
    class Memory
    {
    public:
        static void Initialize();
        static void Shutdown();
    };
}

namespace Turbo
{
    class Allocator
    {
        // TODO: Memory manager
    };
}

#include "Turbo/Core/Ref.h"
#include "Turbo/Core/Scopes.h"


