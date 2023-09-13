#pragma once

#include "Fly.h"
#include "Font.h"
#include "Renderer.h"
#include "ShaderLibrary.h"
#include "RenderCommandBuffer.h"
#include "Pipeline.h"
#include "Image2D.h"
#include "Material.h"
#include "Texture.h"
#include "RenderPass.h"
#include "FrameBuffer.h"
#include "UniformBuffer.h"
#include "VertexBuffer.h"
#include "Mesh.h"
#include "MaterialAsset.h"

#include "SceneRenderer.h"

namespace Turbo {

    class Renderer2D
    {
    public:
        Renderer2D();
        ~Renderer2D();
        Renderer2D(const Renderer2D&) = delete;

        void Initialize();

        void Begin();
        void End();

        void SetSceneData(const SceneRendererData& data);

        void SubmitQuad(const glm::mat4& transform, const glm::vec4& color, i32 entity);
        void SubmitSprite(const glm::mat4& transform, const glm::vec4& color, Ref<Texture2D> texture, const std::array<glm::vec2, 4>& textureCoords, f32 tiling, i32 entity);
        void SubmitBillboardQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, Ref<Texture2D> texture, f32 tiling, i32 entity);

        void SubmitLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color, i32 entity);
        void SubmitCircle(const glm::mat4& transform, const glm::vec4& color, f32 thickness, f32 fade, i32 entity);
        void SubmitDebugCircle(const glm::mat4& transform, const glm::vec4& color, i32 entity);
        void SubmitRect(const glm::mat4& transform, const glm::vec4& color, i32 entity);

        void SubmitString(const glm::mat4& transform, const glm::vec4& color, Ref<Font> font, const std::string& string, f32 kerningOffset, f32 lineSpacing, i32 entity);

        Renderer2DStatistics GetStatistics() const { return m_Statistics; }

        void OnViewportResize(u32 width, u32 height);

        i32 ReadPixel(u32 x, u32 y);

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
        Ref<Pipeline> m_QuadPipeline;

        // Circles
        CircleVertex* m_CircleVertexBufferBase = nullptr;
        CircleVertex* m_CircleVertexBufferPointer = nullptr;
        u32 m_CircleIndexCount = 0;

        Ref<VertexBuffer> m_CircleVertexBuffer;

        Ref<Shader> m_CircleShader;
        Ref<Pipeline> m_CirclePipeline;

        // Lines
        LineVertex* m_LineVertexBufferBase = nullptr;
        LineVertex* m_LineVertexBufferPointer = nullptr;
        u32 m_LineVertexCount = 0;
        f32 m_LineWidth = 2.0f;

        Ref<VertexBuffer> m_LineVertexBuffer;

        Ref<Shader> m_LineShader;
        Ref<Pipeline> m_LinePipeline;

        // Text
        TextVertex* m_TextVertexBufferBase = nullptr;
        TextVertex* m_TextVertexBufferPointer = nullptr;
        u32 m_TextIndexCount = 0;

        Ref<VertexBuffer> m_TextVertexBuffer;
        Ref<Material> m_TextMaterial;

        Ref<Shader> m_TextShader;
        Ref<Pipeline> m_TextPipeline;

        Ref<UniformBufferSet> m_UniformBufferSet;

        Ref<RenderPass> m_TargerRenderPass;
        Ref<RenderCommandBuffer> m_RenderCommandBuffer;

        Renderer2DStatistics m_Statistics;

        // Texture slots
        std::array<Ref<Texture2D>, MaxTextureSlots> m_TextureSlots;
        u32 m_TextureSlotsIndex = 1;

        // Font Texture slots
        std::array<Ref<Texture2D>, MaxFontTextureSlots> m_FontTextureSlots;
        u32 m_FontTextureSlotsIndex = 0;

        Fly<Ref<RendererBuffer>> m_SelectionBuffers;

        SceneRendererData m_SceneData;
        u32 m_ViewportWidth = 0, m_ViewportHeight = 0;
    };
}
