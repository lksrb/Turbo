#pragma once

#include "Turbo/Renderer/RenderCommandQueue.h"
#include "Turbo/Renderer/Renderer2D.h"

namespace Turbo 
{
    class Renderer 
    {
    public:
        static void Initialize();
        static void Shutdown();

        static RenderCommandQueue& GetRenderCommandQueue();
        static RenderCommandQueue& GetSecondaryCommandQueue();
        static Renderer2D& GetRenderer2D();

        template<typename F>
        static void Submit(F&& func)
        {
            auto size = sizeof(func);

            auto command = [](void* ptr)
            {
                auto pFunc = (F*)ptr;
                (*pFunc)();
                pFunc->~F();
            };

            void* memory = GetRenderCommandQueue().Allocate(command, sizeof(func));
            new(memory) F(std::forward<F>(func));
        }

        template<typename F>
        static void SubmitSecondary(F&& func)
        {
            auto size = sizeof(func);

            auto command = [](void* ptr)
            {
                auto pFunc = (F*)ptr;
                (*pFunc)();
                pFunc->~F();
            };

            void* memory = GetSecondaryCommandQueue().Allocate(command, sizeof(func));
            new(memory) F(std::forward<F>(func));
        }

        static void BuildSecondary();
    private:
        static void Begin();
        static void Render();
    private:
        friend class Engine;
    };
}
