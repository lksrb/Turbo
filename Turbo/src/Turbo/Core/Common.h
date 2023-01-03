#pragma once

/*
#ifndef _HAS_CXX17
    #error Please use C++17 or above to build this project!
#endif*/

#include <utility>

#define TBO_BIND_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

#include "Turbo/Core/Log.h"

#define TBO_BIT(x) 1 << x

#ifdef TBO_DEBUG
//Client log macros
#define TBO_TRACE(...)		::Turbo::Log::GetClientLogger()->trace(__VA_ARGS__)
#define TBO_INFO(...)		::Turbo::Log::GetClientLogger()->info(__VA_ARGS__)
#define TBO_WARN(...)		::Turbo::Log::GetClientLogger()->warn(__VA_ARGS__)
#define TBO_ERROR(...)		::Turbo::Log::GetClientLogger()->error(__VA_ARGS__)
#define TBO_FATAL(...)		::Turbo::Log::GetClientLogger()->critical(__VA_ARGS__)

//Core log macros
#define TBO_ENGINE_TRACE(...)	::Turbo::Log::GetEngineLogger()->trace(__VA_ARGS__)
#define TBO_ENGINE_INFO(...)	::Turbo::Log::GetEngineLogger()->info(__VA_ARGS__)
#define TBO_ENGINE_WARN(...)	::Turbo::Log::GetEngineLogger()->warn(__VA_ARGS__)
#define TBO_ENGINE_ERROR(...)	::Turbo::Log::GetEngineLogger()->error(__VA_ARGS__)
#define TBO_ENGINE_FATAL(...)	::Turbo::Log::GetEngineLogger()->critical(__VA_ARGS__)
#elif TBO_RELEASE
//Client log macros
#define TBO_ERROR(...)		
#define TBO_WARN(...)		
#define TBO_TRACE(...)		
#define TBO_INFO(...)			
#define TBO_FATAL(...)		

//Core log macros
#define TBO_ENGINE_TRACE(...)
#define TBO_ENGINE_INFO(...)
#define TBO_ENGINE_WARN(...)
#define TBO_ENGINE_ERROR(...)
#define TBO_ENGINE_FATAL(...)
#endif

#include "Turbo/Core/Assert.h"
#include "Turbo/Core/Memory.h"
#include "Turbo/Core/PrimitiveTypes.h"

