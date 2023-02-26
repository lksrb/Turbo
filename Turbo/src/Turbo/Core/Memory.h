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
    class Allocator
    {
        // TODO: Memory manager
    };
}

#include "Turbo/Core/Refs.h"
