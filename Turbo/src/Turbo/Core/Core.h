#pragma once

// Platform preprocessors

#ifdef TBO_DEBUG
    #if defined(TBO_PLATFORM_WIN32)
        #if _MSC_VER && !__INTEL_COMPILER
            #define TBO_DEBUGBREAK() __debugbreak()
        #endif
    #elif defined(TBO_PLATFORM_LINUX)
        #include <signal.h>
        #define TBO_DEBUGBREAK() raise(SIGTRAP)
    #endif
    
    // If platform does not support debugbreak, use assert instead
    #ifndef TBO_DEBUGBREAK
        #include <assert.h>
        #define TBO_DEBUGBREAK() assert(false)
    #endif
#else
    #define TBO_DEBUGBREAK()
#endif

// Useful preprocessors
#define TBO_BIND_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }
#define TBO_BIT(x) 1 << x

// No virtual table feature is only available in MSC
#if _MSC_VER && !__INTEL_COMPILER
    #define TBO_NOVTABLE __declspec(novtable) 
#else
    #define TBO_NOVTABLE
#endif

#include "Assert.h"
