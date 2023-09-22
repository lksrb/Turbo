#include "tbopch.h"
#include "Memory.h"

#if TBO_TRACK_MEMORY
    #define _CRTDBG_MAP_ALLOC
    #include <stdlib.h>
    #include <crtdbg.h>
#endif

namespace Turbo::Memory {
}

#if TBO_TRACK_MEMORY

_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
void* __CRTDECL operator new(size_t size)
{
    return _malloc_dbg(size, _NORMAL_BLOCK, __FILE__, __LINE__);
}

_NODISCARD _Ret_notnull_ _Post_writable_byte_size_(size) _VCRT_ALLOCATOR
void* __CRTDECL operator new(size_t size, const char* file, int line)
{
    return _malloc_dbg(size, _NORMAL_BLOCK, file, line);
}

void __CRTDECL operator delete(void* memory)
{
    return _free_dbg(memory, _NORMAL_BLOCK);
}

void __CRTDECL operator delete(void* memory, const char* file, int line)
{
    return _free_dbg(memory, _NORMAL_BLOCK);
}

#endif
