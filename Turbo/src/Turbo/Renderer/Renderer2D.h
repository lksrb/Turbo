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
        struct Statistics
        {
            u32 QuadIndexCount;
            u32 QuadCount;

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

        void Begin(const Camera& camera);
        void End();

        void DrawQuad(const glm::vec3& position, const glm::vec2& size = { 1.0f, 1.0f }, f32 rotation = 0.0f, const glm::vec4& color = { 1.0f,1.0f, 1.0f, 1.0f }, i32 entityID = -1);
        void DrawQuad(const glm::mat4& transform, const glm::vec4& color, i32 entityID = -1);
        void DrawSprite(const glm::mat4& transform, const glm::vec4& color, Ptr<Texture2D> texture, f32 tiling, i32 entityID /*= -1*/);
        void DrawSprite(const glm::mat4& transform, const glm::vec4& color, Ptr<SubTexture2D> subTexture, f32 tiling, i32 entityID /*= -1*/);

        Statistics GetStatistics() const { return m_Statistics; }

        void SetClearColor(const glm::vec4& color) { TBO_ENGINE_ASSERT(false, "Not implemented yet"); m_ClearColor = color; }
    private:
        void Initialize();
        void Shutdown();

        void StartBatch();
        void Flush();
    private:
        void InitializeRender();

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

        Statistics m_Statistics;

        // Quads
        QuadVertex* m_QuadVertexBufferBase;
        QuadVertex* m_QuadVertexBufferPointer;

        Ptr<VertexBuffer> m_QuadVertexBuffer;
        Ptr<IndexBuffer> m_QuadIndexBuffer;
        Ptr<Material> m_QuadMaterial;

        Ptr<Shader> m_QuadShader;
        Ptr<GraphicsPipeline> m_QuadPipeline;
        
        Ptr<Texture2D> m_WhiteTexture;

        Ptr<RenderPass> m_RenderPass;

        //Ptr<Image2D> m_DepthImage;

        Image2D* m_RenderImages[TBO_MAX_FRAMESINFLIGHT];
        FrameBuffer* m_Framebuffers[TBO_MAX_FRAMESINFLIGHT];

        Ptr<CommandBuffer> m_RenderBuffers[TBO_MAX_FRAMESINFLIGHT];

        // Texture slots
        std::array<Ptr<Texture2D>, MaxTextureSlots> m_TextureSlots;
        u32 m_TextureSlotsIndex;

        glm::vec4 m_ClearColor;

        u32 m_ViewportWidth;
        u32 m_ViewportHeight;

        bool m_BeginDraw;

        friend class Engine;
    };
}
