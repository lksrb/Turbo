#pragma once

#include <utility>

#define TBO_BIND_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

#define TBO_NOVTABLE __declspec(novtable) 

#define TBO_BIT(x) 1 << x

#include "Turbo/Core/Log.h"
#include "Turbo/Core/Assert.h"
#include "Turbo/Core/Memory.h"
#include "Turbo/Core/PrimitiveTypes.h"

