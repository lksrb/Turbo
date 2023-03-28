#pragma once

#include "Camera.h"

#include "RenderCommandBuffer.h"
#include "Shader.h"
#include "GraphicsPipeline.h"
#include "IndexBuffer.h"
#include "Image2D.h"
#include "Material.h"
#include "Texture2D.h"
#include "RendererContext.h"
#include "RenderPass.h"
#include "FrameBuffer.h"
#include "UniformBuffer.h"
#include "VertexBuffer.h"

#include <array>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace Turbo
{
    class Renderer2D
    {
    public:
        struct Statistics
        {
            u32 QuadCount;
            u32 CircleCount;
            u32 CircleIndexCount;
            u32 DrawCalls;

            Statistics() { Reset(); }

            void Reset()
            {
                memset(this, 0, sizeof(*this));
            }
        };

        Renderer2D();
        Renderer2D(const Renderer2D&) = delete;
        ~Renderer2D();

        void Initialize();
        static Ref<Renderer2D> Create() { return Ref<Renderer2D>::Create(); }

        void Begin2D(const Camera& camera);
        void End2D();

        void DrawQuad(const glm::vec3& position, const glm::vec2& size = { 1.0f, 1.0f }, f32 rotation = 0.0f, const glm::vec4& color = { 1.0f,1.0f, 1.0f, 1.0f }, i32 entity = -1);
        void DrawQuad(const glm::mat4& transform, const glm::vec4& color, i32 entity = -1);
        void DrawSprite(const glm::mat4& transform, const glm::vec4& color, Ref<Texture2D> texture, f32 tiling, i32 entity = -1);
        void DrawSprite(const glm::mat4& transform, const glm::vec4& color, Ref<SubTexture2D> subTexture, f32 tiling, i32 entity = -1);

        void DrawCircle(const glm::mat4& transform, const glm::vec4& color, f32 thickness, f32 fade, i32 entity = -1);

        Statistics GetStatistics() const { return m_Statistics; }

        void SetRenderTarget(const Ref<FrameBuffer>& frameBuffer) { m_TargetFramebuffer = frameBuffer; }
    private:
        void Shutdown();

        void StartBatch();
        void Flush();
    private:
        static constexpr u32 MaxQuad = 2000;
        static constexpr u32 MaxVertices = MaxQuad * 4;
        static constexpr u32 MaxIndices = MaxQuad * 6;
        static constexpr u32 MaxTextureSlots = 32; // LIMITED
        static constexpr glm::vec4 QuadVertexPositions[4]
        {
            { -0.5f, -0.5f, 0.0f, 1.0f },
            {  0.5f, -0.5f, 0.0f, 1.0f },
            {  0.5f,  0.5f, 0.0f, 1.0f },
            { -0.5f,  0.5f, 0.0f, 1.0f }
        };

        struct QuadVertex
        {
            glm::vec3 Position;
            glm::vec4 Color;
            glm::vec2 TexCoord;
            u32 TextureIndex;
            f32 TilingFactor;
            i32 EntityID;
        };

        struct CircleVertex
        {
            glm::vec3 Position;
            glm::vec4 Color;

            f32 Thickness;
            f32 Fade;
            i32 EntityID;
        };

        // Quads
        QuadVertex* m_QuadVertexBufferBase = nullptr;
        QuadVertex* m_QuadVertexBufferPointer = nullptr;

        Ref<VertexBuffer> m_QuadVertexBuffer;
        Ref<IndexBuffer> m_QuadIndexBuffer;
        Ref<Material> m_QuadMaterial;

        Ref<Shader> m_QuadShader;
        Ref<GraphicsPipeline> m_QuadPipeline;

        // Circle
        CircleVertex* m_CircleVertexBufferBase = nullptr;
        CircleVertex* m_CircleVertexBufferPointer = nullptr;

        Ref<VertexBuffer> m_CircleVertexBuffer;
        Ref<Material> m_CircleMaterial;

        Ref<Shader> m_CircleShader;
        Ref<GraphicsPipeline> m_CirclePipeline;

        struct UBCamera
        {
            glm::mat4 ViewProjection;
        };

        Ref<Texture2D> m_WhiteTexture;
        Ref<UniformBufferSet> m_UniformBufferSet;

        Ref<FrameBuffer> m_TargetFramebuffer;
        Ref<RenderCommandBuffer> m_RenderCommandBuffer;

        Statistics m_Statistics;

        u32 m_QuadIndexCount = 0;
        u32 m_CircleIndexCount = 0;

        // Texture slots
        std::array<Ref<Texture2D>, MaxTextureSlots> m_TextureSlots;
        u32 m_TextureSlotsIndex = 1;
        glm::vec4 m_ClearColor = glm::vec4{ 0.0f };
        u32 m_ViewportWidth = 0, m_ViewportHeight = 0;
        bool m_BeginDraw = false;

        friend class SceneRenderer;
    };
}
