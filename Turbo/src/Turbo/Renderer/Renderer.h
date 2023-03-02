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

        static void Begin();
        static void Render();

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

        static RenderCommandQueue& GetRenderCommandQueue();
        static u32 GetCurrentFrame();

        static void SetViewport(Ref<CommandBuffer> commandbuffer, i32 x, i32 y, u32 width, u32 height, f32 min_depth = 0.0f, f32 max_depth = 1.0f);
        static void SetScissor(Ref<CommandBuffer> commandbuffer, i32 x, i32 y, u32 width, u32 height);

        static void BeginRenderPass(Ref<CommandBuffer> commandbuffer, Ref<FrameBuffer> frame_buffer, const glm::vec4& clear_color);
        static void EndRenderPass(Ref<CommandBuffer> commandbuffer);

        static void DrawIndexed(Ref<CommandBuffer> commandbuffer, Ref<VertexBuffer> vertexbuffer, Ref<IndexBuffer> indexbuffer, Ref<GraphicsPipeline> pipeline, Ref<Shader> shader, u32 index_count);
    private:
    };
}
