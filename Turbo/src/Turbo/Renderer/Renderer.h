#pragma once

#include "CommandQueue.h"

namespace Turbo {

    class IndexBuffer;
    class GraphicsPipeline;
    class StaticMesh;
    class RenderCommandBuffer;
    class UniformBufferSet;
    class Shader;
    class RendererContext;
    class RenderPass;
    class Image2D;
    class RendererBuffer;
    class Texture2D;
    class TextureCube;
    class UniformBuffer;
    class VertexBuffer;
    class MaterialAsset;

    class Renderer
    {
    public:
        static void Init();
        static void Shutdown();

        static void BeginFrame();
        static void Render();
        static void Wait();

        template<typename F>
        static void Submit(F&& func)
        {
            GetCommandQueue().Submit(std::forward<F>(func));
        }

        /**
         * Resource queue manages vulkan handles on runtime, if vulkan wrappers are invalidated, vulkan handles will be submitted, to this queue
         * and safely released when the resource is not used.
         */
        template<typename F>
        static void SubmitResourceFree(F&& func)
        {
            GetResourceQueue().Submit(std::forward<F>(func));
        }

        // For now this is implemented just for FrameBuffer and Image2D since those are the only objects
        // that are getting resized
        template<typename F>
        static void SubmitRuntimeResourceFree(F&& func)
        {
            GetResourceRuntimeQueue().Submit(std::forward<F>(func));
        }

        static void SetLineWidth(Ref<RenderCommandBuffer> commandBuffer, f32 lineWidth);
        static void SetViewport(Ref<RenderCommandBuffer> commandBuffer, i32 x, i32 y, u32 width, u32 height, f32 minDepth = 0.0f, f32 maxDepth = 1.0f);
        static void SetScissor(Ref<RenderCommandBuffer> commandBuffer, i32 x, i32 y, u32 width, u32 height);

        static void BeginRenderPass(Ref<RenderCommandBuffer> commandBuffer, Ref<RenderPass> renderPass);
        static void EndRenderPass(Ref<RenderCommandBuffer> commandBuffer);

        static void PushConstant(Ref<RenderCommandBuffer> commandBuffer, Ref<GraphicsPipeline> pipeline, u32 size, const void* data);
        static void Draw(Ref<RenderCommandBuffer> commandBuffer, Ref<VertexBuffer> vertexBuffer, Ref<UniformBufferSet> uniformBufferSet, Ref<GraphicsPipeline> pipeline, Ref<Shader> shader, u32 vertexCount);
        static void DrawIndexed(Ref<RenderCommandBuffer> commandBuffer, Ref<VertexBuffer> vertexBuffer, Ref<IndexBuffer> indexBuffer, Ref<UniformBufferSet> uniformBufferSet, Ref<GraphicsPipeline> pipeline, Ref<Shader> shader, u32 indexCount);
        static void DrawInstanced(Ref<RenderCommandBuffer> commandBuffer, Ref<VertexBuffer> vertexBuffer, Ref<VertexBuffer> instanceBuffer, Ref<IndexBuffer> indexBuffer, Ref<UniformBufferSet> uniformBufferSet, Ref<GraphicsPipeline> pipeline, u32 instanceCount, u32 indicesPerInstance);
        static void DrawStaticMesh(Ref<RenderCommandBuffer> commandBuffer, Ref<StaticMesh> mesh, Ref<VertexBuffer> transformBuffer, Ref<UniformBufferSet> uniformBufferSet, Ref<GraphicsPipeline> pipeline, u32 transformOffset, u32 subMeshIndex, u32 instanceCount);
        static void DrawSkybox(Ref<RenderCommandBuffer> commandBuffer, Ref<GraphicsPipeline> pipeline, Ref<UniformBufferSet> uniformBufferSet);

        static void CopyImageToBuffer(Ref<RenderCommandBuffer> commandBuffer, Ref<Image2D> image, Ref<RendererBuffer> rendererBuffer);
        static RendererContext* GetContext();
        static u32 GetCurrentFrame();
        static Ref<Texture2D> GetWhiteTexture();
        static Ref<MaterialAsset> GetWhiteMaterial();
    private:
        static CommandQueue& GetResourceRuntimeQueue();
        static CommandQueue& GetResourceQueue();
        static CommandQueue& GetCommandQueue();
    };

}
