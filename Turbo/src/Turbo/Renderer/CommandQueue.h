#pragma once

#include "Turbo/Core/PrimitiveTypes.h"

namespace Turbo
{
    class CommandQueue
    {
    public:
        using RenderCommandFn = void(*)(void*);

        CommandQueue();
        ~CommandQueue();

        CommandQueue(const CommandQueue&) = delete;

        template<typename F>
        void Submit(F&& func)
        {
            auto size = sizeof(func);

            auto command = [](void* ptr)
            {
                auto pFunc = (F*)ptr;
                (*pFunc)();
                pFunc->~F();
            };

            void* memory = Allocate(command, sizeof(func));
            new(memory) F(std::forward<F>(func));
        }

        void Execute();
    private:
        void* Allocate(RenderCommandFn func, size_t size);

        u32 m_CommandCount = 0;
        u8* m_Buffer = nullptr;
        u8* m_BufferPointer = nullptr;
    };
}
