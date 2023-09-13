#pragma once

#ifdef TBO_ENABLE_PROFILER
    #include <optick.h>

    #define TBO_PROFILE_FRAME(...)          OPTICK_FRAME(__VA_ARGS__)
    #define TBO_PROFILE_FUNC(...)           OPTICK_EVENT(__VA_ARGS__)
    #define TBO_PROFILE_TAG(NAME, ...)      OPTICK_TAG(NAME, __VA_ARGS__)
    #define TBO_PROFILE_SCOPE(NAME)         OPTICK_EVENT(NAME)
    #define TBO_PROFILE_SCOPE_DYNAMIC(NAME) OPTICK_EVENT_DYNAMIC(NAME)
    #define TBO_PROFILE_THREAD(...)         OPTICK_THREAD(__VA_ARGS__)
#else 
    #define TBO_PROFILE_FRAME(...)
    #define TBO_PROFILE_FUNC(...)
    #define TBO_PROFILE_TAG(NAME, ...)
    #define TBO_PROFILE_SCOPE(NAME)
    #define TBO_PROFILE_SCOPE_DYNAMIC(NAME)
    #define TBO_PROFILE_THREAD(...)
#endif
