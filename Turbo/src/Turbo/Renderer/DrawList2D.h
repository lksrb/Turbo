#pragma once

#include "Font.h"
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
    class DrawList2D
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

        DrawList2D();
        DrawList2D(const DrawList2D&) = delete;
        ~DrawList2D();

        void Initialize();
        static Ref<DrawList2D> Create() { return Ref<DrawList2D>::Create(); }

        void Begin();
        void End();
        
        void SetCameraTransform(const glm::mat4& viewProjection);

        void AddQuad(const glm::mat4& transform, const glm::vec4& color, i32 entity);
        void AddSprite(const glm::mat4& transform, const glm::vec4& color, Ref<Texture2D> texture, const std::array<glm::vec2, 4>& textureCoords, f32 tiling, i32 entity);

        void AddLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color, i32 entity);
        void AddCircle(const glm::mat4& transform, const glm::vec4& color, f32 thickness, f32 fade, i32 entity);
        void AddRect(const glm::mat4& transform, const glm::vec4& color, i32 entity);

        void AddString(const glm::mat4& transform, const glm::vec4& color, Ref<Font> font, const std::string& string, f32 kerningOffset, f32 lineSpacing, i32 entity);

        Statistics GetStatistics() const { return m_Statistics; }

        void OnViewportResize(u32 width, u32 height);

        void SetTargetRenderPass(const Ref<RenderPass>& renderPass);
    private:
        void ResetStatistics() { m_Statistics.Reset(); }
        void Shutdown();

        void StartBatch();
        void FlushAndReset();
    private:
        static constexpr u32 MaxQuad = 2000;
        static constexpr u32 MaxQuadVertices = MaxQuad * 4;
        static constexpr u32 MaxQuadIndices = MaxQuad * 6;
        static constexpr u32 MaxTextureSlots = 32; // TODO: RenderCaps
        static constexpr u32 MaxFontTextureSlots = 2;
        static constexpr glm::vec4 m_QuadVertexPositions[4]
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
            glm::vec3 WorldPosition;
            glm::vec3 LocalPosition;
            glm::vec4 Color;
            f32 Thickness;
            f32 Fade;
            i32 EntityID;
        };

        struct LineVertex
        {
            glm::vec3 Position;
            glm::vec4 Color;
            i32 EntityID;
        };

        struct TextVertex
        {
            glm::vec3 Position;
            glm::vec4 Color;
            glm::vec2 TexCoord;
            u32 TextureIndex;
            i32 EntityID;
        };

        struct UBCamera
        {
            glm::mat4 ViewProjection;
        };

        // Quads
        QuadVertex* m_QuadVertexBufferBase = nullptr;
        QuadVertex* m_QuadVertexBufferPointer = nullptr;
        u32 m_QuadIndexCount = 0;

        Ref<VertexBuffer> m_QuadVertexBuffer;
        Ref<IndexBuffer> m_QuadIndexBuffer;
        Ref<Material> m_QuadMaterial;

        Ref<Shader> m_QuadShader;
        Ref<GraphicsPipeline> m_QuadPipeline;

        // Circles
        CircleVertex* m_CircleVertexBufferBase = nullptr;
        CircleVertex* m_CircleVertexBufferPointer = nullptr;
        u32 m_CircleIndexCount = 0;

        Ref<VertexBuffer> m_CircleVertexBuffer;

        Ref<Shader> m_CircleShader;
        Ref<GraphicsPipeline> m_CirclePipeline;

        // Lines
        LineVertex* m_LineVertexBufferBase = nullptr;
        LineVertex* m_LineVertexBufferPointer = nullptr;
        u32 m_LineVertexCount = 0;
        f32 m_LineWidth = 2.0f;

        Ref<VertexBuffer> m_LineVertexBuffer;

        Ref<Shader> m_LineShader;
        Ref<GraphicsPipeline> m_LinePipeline;

        // Text
        TextVertex* m_TextVertexBufferBase = nullptr;
        TextVertex* m_TextVertexBufferPointer = nullptr;
        u32 m_TextIndexCount = 0;

        Ref<VertexBuffer> m_TextVertexBuffer;
        Ref<Material> m_TextMaterial;

        Ref<Shader> m_TextShader;
        Ref<GraphicsPipeline> m_TextPipeline;

        Ref<Texture2D> m_WhiteTexture;
        Ref<UniformBufferSet> m_UniformBufferSet;

        Ref<RenderPass> m_TargerRenderPass;
        Ref<RenderCommandBuffer> m_RenderCommandBuffer;

        Statistics m_Statistics;

        // Texture slots
        std::array<Ref<Texture2D>, MaxTextureSlots> m_TextureSlots;
        u32 m_TextureSlotsIndex = 1;

        // Font Texture slots
        std::array<Ref<Texture2D>, MaxFontTextureSlots> m_FontTextureSlots;
        u32 m_FontTextureSlotsIndex = 0;

        glm::vec4 m_ClearColor = glm::vec4{ 0.0f };
        u32 m_ViewportWidth = 0, m_ViewportHeight = 0;
    };
}
