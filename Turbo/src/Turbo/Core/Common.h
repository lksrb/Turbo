#pragma once

/*
#ifndef _HAS_CXX17
    #error Please use C++17 or above to build this project!
#endif*/

#include <utility>

#define TBO_BIND_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

#define TBO_NOVTABLE __declspec(novtable) 

#define TBO_INTERFACE struct __declspec(novtable)

#include "Turbo/Core/Log.h"

#define TBO_BIT(x) 1 << x

#include "Turbo/Core/Assert.h"
#include "Turbo/Core/Memory.h"
#include "Turbo/Core/PrimitiveTypes.h"

