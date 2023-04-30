#pragma once

#include "Common.h"

#include <filesystem>

#ifdef TBO_DEBUG
#if defined(TBO_PLATFORM_WIN32)
#define TBO_DEBUGBREAK() __debugbreak()
#elif defined(TBO_PLATFORM_LINUX)
#include <signal.h>
#define TBO_DEBUGBREAK() raise(SIGTRAP)
#else
#error "Platform doesn't support debugbreak yet!"
#endif

#else
#define TBO_DEBUGBREAK()
#endif
#define TBO_ENABLE_ASSERTS
#ifdef TBO_ENABLE_ASSERTS

#define TBO_EXPAND_MACRO(x) x
#define TBO_STRINGIFY_MACRO(x) #x

// Asserts
#define TBO_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { TBO##type##ERROR(msg, __VA_ARGS__); TBO_DEBUGBREAK(); } }
#define TBO_INTERNAL_ASSERT_WITH_MSG(type, check, ...) TBO_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
#define TBO_INTERNAL_ASSERT_NO_MSG(type, check) TBO_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", TBO_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

#define TBO_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
#define TBO_INTERNAL_ASSERT_GET_MACRO(...) TBO_EXPAND_MACRO( TBO_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, TBO_INTERNAL_ASSERT_WITH_MSG, TBO_INTERNAL_ASSERT_NO_MSG) )

// Currently accepts at least the condition and one additional parameter (the message) being optional
#define TBO_ASSERT(...) TBO_EXPAND_MACRO( TBO_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
#define TBO_ENGINE_ASSERT(...) TBO_EXPAND_MACRO( TBO_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_ENGINE_, __VA_ARGS__) )

// Verify
#define TBO_INTERNAL_VERIFY_IMPL(type, check, msg, ...) { if(check) { TBO##type##INFO(msg, __VA_ARGS__); } else { TBO_INTERNAL_ASSERT_IMPL(type, check, "Verification failed: {}", __VA_ARGS__); } }
#define TBO_INTERNAL_VERIFY_WITH_MSG(type, check, ...) TBO_INTERNAL_VERIFY_IMPL(type, check, "Verification successed: {0}", __VA_ARGS__)
#define TBO_INTERNAL_VERIFY_NO_MSG(type, check) TBO_INTERNAL_VERIFY_IMPL(type, check, "Verification '{0}' successed at {1}:{2}", TBO_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

#define TBO_INTERNAL_VERIFY_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
#define TBO_INTERNAL_VERIFY_GET_MACRO(...) TBO_EXPAND_MACRO( TBO_INTERNAL_VERIFY_GET_MACRO_NAME(__VA_ARGS__, TBO_INTERNAL_VERIFY_WITH_MSG, TBO_INTERNAL_VERIFY_NO_MSG) )

// Currently accepts at least the condition and one additional parameter (the message) being optional
#define TBO_VERIFY(...) TBO_EXPAND_MACRO( TBO_INTERNAL_VERIFY_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
#define TBO_ENGINE_VERIFY(...) TBO_EXPAND_MACRO( TBO_INTERNAL_VERIFY_GET_MACRO(__VA_ARGS__)(_ENGINE_, __VA_ARGS__) )
#else
#define TBO_ASSERT(...)
#define TBO_ENGINE_ASSERT(...)

#define TBO_VERIFY(...)
#define TBO_ENGINE_VERIFY(...)
#endif
