#pragma once

#include "Camera.h"

#include "CommandBuffer.h"
#include "Shader.h"
#include "GraphicsPipeline.h"
#include "IndexBuffer.h"
#include "Image2D.h"
#include "Texture2D.h"
#include "RendererContext.h"
#include "RenderPass.h"
#include "FrameBuffer.h"
#include "VertexBuffer.h"
#include "Material.h"

#include <array>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace Turbo
{
    class Renderer2D
    {
    public:
        struct RenderInfo
        {
            u32 QuadCount;
            u32 QuadIndexCount;
            u32 DrawCalls;

            RenderInfo() { Reset(); }

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

        void Begin(const Camera& camera);
        void End();

        void DrawQuad(const glm::vec3& position, const glm::vec2& size = { 1.0f, 1.0f }, f32 rotation = 0.0f, const glm::vec4& color = { 1.0f,1.0f, 1.0f, 1.0f }, i32 entityID = -1);
        void DrawQuad(const glm::mat4& transform, const glm::vec4& color, i32 entityID = -1);
        void DrawSprite(const glm::mat4& transform, const glm::vec4& color, Ref<Texture2D> texture, f32 tiling, i32 entityID /*= -1*/);
        void DrawSprite(const glm::mat4& transform, const glm::vec4& color, Ref<SubTexture2D> subTexture, f32 tiling, i32 entityID /*= -1*/);

        RenderInfo GetRenderInfo() const { return m_RenderInfo; }

        void SetRenderTarget(Ref<FrameBuffer> framebuffer) { m_TargetFramebuffer = framebuffer; }
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
            u32 EntityID;
        };

        RenderInfo m_RenderInfo;

        // Quads
        QuadVertex* m_QuadVertexBufferBase = nullptr;
        QuadVertex* m_QuadVertexBufferPointer = nullptr;

        Ref<VertexBuffer> m_QuadVertexBuffer;
        Ref<IndexBuffer> m_QuadIndexBuffer;
        Ref<Material> m_QuadMaterial;

        Ref<Shader> m_QuadShader;
        Ref<GraphicsPipeline> m_QuadPipeline;

        Ref<Texture2D> m_WhiteTexture;

        Ref<FrameBuffer> m_TargetFramebuffer;

        Ref<CommandBuffer> m_CommandBuffer;

        // Texture slots
        std::array<Ref<Texture2D>, MaxTextureSlots> m_TextureSlots;
        u32 m_TextureSlotsIndex = 1;
        glm::vec4 m_ClearColor = glm::vec4{ 0.0f };
        u32 m_ViewportWidth = 0, m_ViewportHeight = 0;
        bool m_BeginDraw = false;

        friend class SceneRenderer;
    };
}
