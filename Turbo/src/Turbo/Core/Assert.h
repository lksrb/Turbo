#pragma once

#include "Turbo/Core/Core.h"
#include "Turbo/Core/Log.h"

#include <filesystem>

// Asserts enable
#define TBO_ENABLE_ASSERTS 1

#if TBO_ENABLE_ASSERTS
    #define TBO_EXPAND_MACRO(x) x
    #define TBO_STRINGIFY_MACRO(x) #x
    
    // Asserts
    #define TBO_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { TBO##type##ERROR(msg, __VA_ARGS__); TBO_DEBUGBREAK(); } }
    #define TBO_INTERNAL_ASSERT_WITH_MSG(type, check, ...) TBO_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
    #define TBO_INTERNAL_ASSERT_NO_MSG(type, check) TBO_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", TBO_STRINGIFY_MACRO(check),  std::filesystem::path(__FILE__).filename().string(), __LINE__)
    
    #define TBO_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
    #define TBO_INTERNAL_ASSERT_GET_MACRO(...) TBO_EXPAND_MACRO( TBO_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, TBO_INTERNAL_ASSERT_WITH_MSG,  TBO_INTERNAL_ASSERT_NO_MSG) )
    
    // Currently accepts at least the condition and one additional parameter (the message) being optional
    #define TBO_ASSERT(...) TBO_EXPAND_MACRO( TBO_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
    #define TBO_ENGINE_ASSERT(...) TBO_EXPAND_MACRO( TBO_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_ENGINE_, __VA_ARGS__) )
#else
    #define TBO_ASSERT(...)
    #define TBO_ENGINE_ASSERT(...)
#endif
