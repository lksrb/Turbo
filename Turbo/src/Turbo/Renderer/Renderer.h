#pragma once

#include "CommandQueue.h"
#include "DrawList2D.h"

#include "Mesh.h"

namespace Turbo 
{
    class Renderer 
    {
    public:
        static void Init();
        static void Shutdown();

        static void BeginFrame();
        static void Render();

        template<typename F>
        static void Submit(F&& func)
        {
            GetCommandQueue().Submit(std::forward<F>(func));
        }

        static u32 GetCurrentFrame();

        static void SetLineWidth(Ref<RenderCommandBuffer> commandBuffer, f32 lineWidth);
        static void SetViewport(Ref<RenderCommandBuffer> commandBuffer, i32 x, i32 y, u32 width, u32 height, f32 minDepth = 0.0f, f32 maxDepth = 1.0f);
        static void SetScissor(Ref<RenderCommandBuffer> commandBuffer, i32 x, i32 y, u32 width, u32 height);

        static void BeginRenderPass(Ref<RenderCommandBuffer> commandBuffer, Ref<RenderPass> renderPass, const glm::vec4& clearColor);
        static void EndRenderPass(Ref<RenderCommandBuffer> commandBuffer);

        static void PushConstant(Ref<RenderCommandBuffer> commandBuffer, Ref<GraphicsPipeline> pipeline, u32 size, const void* data);
        static void Draw(Ref<RenderCommandBuffer> commandBuffer, Ref<VertexBuffer> vertexBuffer, Ref<UniformBufferSet> uniformBufferSet, Ref<GraphicsPipeline> pipeline, Ref<Shader> shader, u32 vertexCount);
        static void DrawIndexed(Ref<RenderCommandBuffer> commandBuffer, Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer, Ref<UniformBufferSet> uniformBufferSet, Ref<GraphicsPipeline> pipeline, Ref<Shader> shader, u32 indexCount);
        static void DrawInstanced(Ref<RenderCommandBuffer> commandBuffer, Ref<VertexBuffer> vertexBuffer, Ref<VertexBuffer> instanceBuffer, Ref<IndexBuffer> indexBuffer, Ref<UniformBufferSet> uniformBufferSet, Ref<GraphicsPipeline> pipeline, u32 instanceCount, u32 indicesPerInstance);
        static void DrawStaticMesh(Ref<RenderCommandBuffer> commandBuffer, Ref<StaticMesh> mesh, Ref<VertexBuffer> transformBuffer, Ref<UniformBufferSet> uniformBufferSet, Ref<GraphicsPipeline> pipeline, u32 transformOffset, u32 subMeshIndex, u32 instanceCount);
    private:
        static CommandQueue& GetCommandQueue();
    };
}
