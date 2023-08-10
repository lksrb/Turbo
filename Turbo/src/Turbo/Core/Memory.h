#pragma once

#include <memory>

namespace Turbo::Memory {

}

#define TBO_TRACK_MEMORY 0

#if TBO_TRACK_MEMORY

_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
void* __CRTDECL operator new(size_t size);

_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
void* __CRTDECL operator new(size_t size, const char* file, int line);

void __CRTDECL operator delete(void* memory);
void __CRTDECL operator delete(void* memory, const char* file, int line);

#endif

#include "Turbo/Core/Ref.h"
#include "Turbo/Core/Owned.h"
