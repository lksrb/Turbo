#include "tbopch.h"
#include "DrawList2D.h"

#include "Font-Internal.h"

#include "Turbo/Core/Engine.h"
#include "Turbo/Debug/ScopeTimer.h"

#include <glm/ext/matrix_transform.hpp>

namespace Turbo
{
    DrawList2D::DrawList2D()
    {
    }

    DrawList2D::~DrawList2D()
    {
        Shutdown();
    }

    void DrawList2D::Initialize()
    {
        // Render command buffer
        m_RenderCommandBuffer = RenderCommandBuffer::Create();

        // Default clear color
        m_ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

        // Quad setup
        {
            // Vertex buffer
            m_QuadVertexBufferBase = m_QuadVertexBufferPointer = new QuadVertex[MaxQuadVertices];
            m_QuadVertexBuffer = VertexBuffer::Create({ MaxQuadVertices * sizeof(QuadVertex) });

            // Index buffer
            {
                u32* quadIndices = new u32[MaxQuadIndices];
                u32 offset = 0;
                for (u32 i = 0; i < MaxQuadIndices; i += 6)
                {
                    quadIndices[i + 0] = offset + 0;
                    quadIndices[i + 1] = offset + 1;
                    quadIndices[i + 2] = offset + 2;

                    quadIndices[i + 3] = offset + 2;
                    quadIndices[i + 4] = offset + 3;
                    quadIndices[i + 5] = offset + 0;

                    offset += 4;
                }

                IndexBuffer::Config config = {};
                config.Size = MaxQuadIndices * sizeof(u32);
                config.Indices = quadIndices;
                m_QuadIndexBuffer = IndexBuffer::Create(config);

                delete[] quadIndices;
            }

            // Shader
            Shader::Config shaderConfig = {};
            shaderConfig.Language = ShaderLanguage::GLSL;
            shaderConfig.ShaderPath = "Assets\\Shaders\\Renderer2D_Quad.glsl";
            m_QuadShader = Shader::Create(shaderConfig);

            // Graphics pipeline
            GraphicsPipeline::Config config = {};
            config.Shader = m_QuadShader;
            config.Topology = PrimitiveTopology::Triangle;
            config.Renderpass = m_TargerRenderPass;
            config.DepthTesting = true;
            config.TargetFramebuffer = m_TargerRenderPass->GetConfig().TargetFrameBuffer;
            m_QuadPipeline = GraphicsPipeline::Create(config);
            m_QuadPipeline->Invalidate();

            // Material
            m_QuadMaterial = Material::Create({ m_QuadShader });
        }

        // Circle setup
        {
            // Vertex buffer
            m_CircleVertexBufferBase = m_CircleVertexBufferPointer = new CircleVertex[MaxQuadVertices];
            m_CircleVertexBuffer = VertexBuffer::Create({ MaxQuadVertices * sizeof(CircleVertex) });

            // Index buffer from quads

            // Shader
            Shader::Config shaderConfig = {};
            shaderConfig.Language = ShaderLanguage::GLSL;
            shaderConfig.ShaderPath = "Assets\\Shaders\\Renderer2D_Circle.glsl";
            m_CircleShader = Shader::Create(shaderConfig);

            // Graphics pipeline
            GraphicsPipeline::Config config = {};
            config.Shader = m_CircleShader;
            config.Topology = PrimitiveTopology::Triangle;
            config.Renderpass = m_TargerRenderPass;
            config.DepthTesting = true;
            config.TargetFramebuffer = m_TargerRenderPass->GetConfig().TargetFrameBuffer;
            m_CirclePipeline = GraphicsPipeline::Create(config);
            m_CirclePipeline->Invalidate();

            // Material - Maybe not necessary
            // m_CircleMaterial = Material::Create({ m_CircleShader });
        }

        // Line setup
        {
            // Vertex buffer
            m_LineVertexBufferBase = m_LineVertexBufferPointer = new LineVertex[MaxQuadVertices];
            m_LineVertexBuffer = VertexBuffer::Create({ MaxQuadVertices * sizeof(CircleVertex) });

            // Index buffer is not needed

            // Shader
            Shader::Config shaderConfig = {};
            shaderConfig.Language = ShaderLanguage::GLSL;
            shaderConfig.ShaderPath = "Assets\\Shaders\\Renderer2D_Line.glsl";
            m_LineShader = Shader::Create(shaderConfig);

            // Graphics pipeline
            GraphicsPipeline::Config config = {};
            config.Shader = m_LineShader;
            config.Renderpass = m_TargerRenderPass;
            config.Topology = PrimitiveTopology::Line;
            config.DepthTesting = false;
            config.TargetFramebuffer = m_TargerRenderPass->GetConfig().TargetFrameBuffer;
            m_LinePipeline = GraphicsPipeline::Create(config);
            m_LinePipeline->Invalidate();
        }

        // Text setup
        {
            // Vertex buffer
            m_TextVertexBufferBase = m_TextVertexBufferPointer = new TextVertex[MaxQuadVertices];
            m_TextVertexBuffer = VertexBuffer::Create({ MaxQuadVertices * sizeof(TextVertex) });

            // Index buffer is not needed

            // Shader
            Shader::Config shaderConfig = {};
            shaderConfig.Language = ShaderLanguage::GLSL;
            shaderConfig.ShaderPath = "Assets\\Shaders\\Renderer2D_Text.glsl";
            m_TextShader = Shader::Create(shaderConfig);

            // Graphics PipeText
            GraphicsPipeline::Config config = {};
            config.Shader = m_TextShader;
            config.Topology = PrimitiveTopology::Triangle;
            config.Renderpass = m_TargerRenderPass;
            config.DepthTesting = false;
            config.TargetFramebuffer = m_TargerRenderPass->GetConfig().TargetFrameBuffer;
            m_TextPipeline = GraphicsPipeline::Create(config);
            m_TextPipeline->Invalidate();

            // Material
            m_TextMaterial = Material::Create({ m_TextShader });
        }

        // Create camera uniform buffer
        m_UniformBufferSet = UniformBufferSet::Create();
        m_UniformBufferSet->Create(0, 0, sizeof(UBCamera));

        // White texture for texture-less quads
        m_WhiteTexture = Texture2D::Create(0xffffffff);

        // Set white texture at the top of the stack
        m_TextureSlots[0] = m_WhiteTexture;
    }

    void DrawList2D::Shutdown()
    {
        delete[] m_QuadVertexBufferBase;
        delete[] m_CircleVertexBufferBase;
        delete[] m_LineVertexBufferBase;
        delete[] m_TextVertexBufferBase;
    }

    void DrawList2D::Begin()
    {
        ResetStatistics();
        StartBatch();
    }

    void DrawList2D::StartBatch()
    {
        // Reset texture indexing
        m_TextureSlotsIndex = 1;

        // Reset font texture indexing
        m_FontTextureSlotsIndex = 0;

        // Reset textures
        for (size_t i = 1; i < m_TextureSlots.size(); ++i)
            m_TextureSlots[i] = nullptr;

        // Reset font atlas textures
        for (size_t i = 0; i < m_FontTextureSlots.size(); ++i)
            m_FontTextureSlots[i] = nullptr;

        // Quads
        m_QuadIndexCount = 0;
        m_QuadVertexBufferPointer = m_QuadVertexBufferBase;

        // Circles
        m_CircleIndexCount = 0;
        m_CircleVertexBufferPointer = m_CircleVertexBufferBase;

        // Lines
        m_LineVertexCount = 0;
        m_LineVertexBufferPointer = m_LineVertexBufferBase;

        // Text
        m_TextIndexCount = 0;
        m_TextVertexBufferPointer = m_TextVertexBufferBase;
    }

    void DrawList2D::FlushAndReset()
    {
        End();
        StartBatch();
    }

    void DrawList2D::End()
    {
        m_RenderCommandBuffer->Begin();
        Renderer::BeginRenderPass(m_RenderCommandBuffer, m_TargerRenderPass, m_ClearColor);

        // Quads
        if (m_QuadIndexCount)
        {
            // Texture slots
            for (u32 i = 0; i < m_TextureSlots.size(); ++i)
            {
                if (m_TextureSlots[i])
                    m_QuadMaterial->Set("u_Textures", m_TextureSlots[i], i);
                else
                    m_QuadMaterial->Set("u_Textures", m_WhiteTexture, i);
            }

            u32 dataSize = (u32)((u8*)m_QuadVertexBufferPointer - (u8*)m_QuadVertexBufferBase);
            m_QuadVertexBuffer->SetData(m_QuadVertexBufferBase, dataSize);

            Renderer::DrawIndexed(m_RenderCommandBuffer, m_QuadVertexBuffer, m_QuadIndexBuffer, m_UniformBufferSet, m_QuadPipeline, m_QuadShader, m_QuadIndexCount);

            m_Statistics.DrawCalls++;
        }

        // Circles
        if (m_CircleIndexCount)
        {
            u32 dataSize = (u32)((u8*)m_CircleVertexBufferPointer - (u8*)m_CircleVertexBufferBase);
            m_CircleVertexBuffer->SetData(m_CircleVertexBufferBase, dataSize);

            Renderer::DrawIndexed(m_RenderCommandBuffer, m_CircleVertexBuffer, m_QuadIndexBuffer, m_UniformBufferSet, m_CirclePipeline, m_CircleShader, m_CircleIndexCount);
            m_Statistics.DrawCalls++;
        }

        // Lines
        if (m_LineVertexCount)
        {
            u32 dataSize = (u32)((u8*)m_LineVertexBufferPointer - (u8*)m_LineVertexBufferBase);
            m_LineVertexBuffer->SetData(m_LineVertexBufferBase, dataSize);

            Renderer::SetLineWidth(m_RenderCommandBuffer, m_LineWidth);
            Renderer::Draw(m_RenderCommandBuffer, m_LineVertexBuffer, m_UniformBufferSet, m_LinePipeline, m_LineShader, m_LineVertexCount);
            m_Statistics.DrawCalls++;
        }

        // Text
        if (m_TextIndexCount)
        {
            // Font texture slots
            for (u32 i = 0; i < m_FontTextureSlots.size(); ++i)
            {
                if (m_FontTextureSlots[i])
                    m_TextMaterial->Set("u_Textures", m_FontTextureSlots[i], i); // FIXME: Clear descriptors, because textures wont unbound automatically
                else
                    m_TextMaterial->Set("u_Textures", Font::GetDefaultFont()->GetAtlasTexture(), i);
            }

            u32 dataSize = (u32)((u8*)m_TextVertexBufferPointer - (u8*)m_TextVertexBufferBase);
            m_TextVertexBuffer->SetData(m_TextVertexBufferBase, dataSize);

            Renderer::DrawIndexed(m_RenderCommandBuffer, m_TextVertexBuffer, m_QuadIndexBuffer, m_UniformBufferSet, m_TextPipeline, m_TextShader, m_TextIndexCount);
            m_Statistics.DrawCalls++;
        }

        Renderer::EndRenderPass(m_RenderCommandBuffer);
        m_RenderCommandBuffer->End();
        m_RenderCommandBuffer->Submit();
    }

    void DrawList2D::SetCameraTransform(const glm::mat4& viewProjection)
    {
        // u_Camera, will be on the set on 0 and bound on 0
        Renderer::Submit([this, viewProjection]()
        {
            m_UniformBufferSet->SetData(0, 0, &viewProjection);
        });
    }

    void DrawList2D::AddQuad(const glm::mat4& transform, const glm::vec4& color, i32 entity)
    {
        if (m_QuadIndexCount >= DrawList2D::MaxQuadIndices)
        {
            FlushAndReset();
        }

        constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

        for (u32 i = 0; i < 4; ++i)
        {
            m_QuadVertexBufferPointer->Position = transform * m_QuadVertexPositions[i];
            m_QuadVertexBufferPointer->Color = color;
            m_QuadVertexBufferPointer->EntityID = entity;
            m_QuadVertexBufferPointer->TextureIndex = 0;
            m_QuadVertexBufferPointer->TexCoord = textureCoords[i];
            m_QuadVertexBufferPointer->TilingFactor = 0.0f;
            m_QuadVertexBufferPointer++;
        }

        m_QuadIndexCount += 6;

        m_Statistics.QuadCount++;
    }

    void DrawList2D::AddSprite(const glm::mat4& transform, const glm::vec4& color, Ref<Texture2D> texture, f32 tiling, i32 entity /*= -1*/)
    {
        if (m_QuadIndexCount >= DrawList2D::MaxQuadIndices)
        {
            FlushAndReset();
        }

        u32 textureIndex = 0; // White Texture
        glm::vec2 textureCoords[] = {
            { 0.0f, 0.0f },
            { 1.0f, 0.0f },
            { 1.0f, 1.0f },
            { 0.0f, 1.0f }
        };

        if (texture)
        {
            for (u32 i = 1; i < m_TextureSlotsIndex; ++i)
            {
                // If the texture is in the stack, just modify textureIndex
                if (texture->GetHash() == m_TextureSlots[i]->GetHash())
                {
                    textureIndex = i;
                    break;
                }
            }

            // If the texture is not present in texture stack, add it
            if (textureIndex == 0)
            {
                if (m_TextureSlotsIndex >= DrawList2D::MaxTextureSlots)
                {
                    FlushAndReset();
                } 

                textureIndex = m_TextureSlotsIndex;
                m_TextureSlots[m_TextureSlotsIndex] = texture;
                m_TextureSlotsIndex++;
            }
        }

        for (u32 i = 0; i < 4; ++i)
        {
            m_QuadVertexBufferPointer->Position = transform * m_QuadVertexPositions[i];
            m_QuadVertexBufferPointer->Color = color;
            m_QuadVertexBufferPointer->EntityID = entity;
            m_QuadVertexBufferPointer->TextureIndex = textureIndex;
            m_QuadVertexBufferPointer->TexCoord = textureCoords[i];
            m_QuadVertexBufferPointer->TilingFactor = tiling;
            m_QuadVertexBufferPointer++;
        }

        m_QuadIndexCount += 6;

        m_Statistics.QuadCount++;
    }

    void DrawList2D::AddSprite(const glm::mat4& transform, const glm::vec4& color, Ref<SubTexture2D> subTexture, f32 tiling, i32 entity)
    {
        if (m_QuadIndexCount >= DrawList2D::MaxQuadIndices)
        {
            FlushAndReset();
        }

        u32 textureIndex = 0; // White Texture
        glm::vec2 textureCoords[] = {
            { 0.0f, 0.0f },
            { 1.0f, 0.0f },
            { 1.0f, 1.0f },
            { 0.0f, 1.0f }
        };

        if (subTexture)
        {
            memcpy(textureCoords, subTexture->GetTextureCoords().data(), 4 * sizeof(glm::vec2));
            for (u32 i = 1; i < m_TextureSlotsIndex; ++i)
            {
                // If the texture is in the stack, just modify textureIndex
                if (subTexture->GetTexture()->GetHash() == m_TextureSlots[i]->GetHash())
                {
                    textureIndex = i;
                    break;
                }
            }

            // If the texture is not present in texture stack, add it
            if (textureIndex == 0)
            {
                if (m_TextureSlotsIndex >= DrawList2D::MaxTextureSlots)
                {
                    FlushAndReset();
                }
                
                textureIndex = m_TextureSlotsIndex;
                m_TextureSlots[m_TextureSlotsIndex] = subTexture->GetTexture();
                m_TextureSlotsIndex++;
            }
        }

        for (u32 i = 0; i < 4; ++i)
        {
            m_QuadVertexBufferPointer->Position = transform * m_QuadVertexPositions[i];
            m_QuadVertexBufferPointer->Color = color;
            m_QuadVertexBufferPointer->EntityID = entity;
            m_QuadVertexBufferPointer->TextureIndex = textureIndex;
            m_QuadVertexBufferPointer->TexCoord = textureCoords[i];
            m_QuadVertexBufferPointer->TilingFactor = tiling;
            m_QuadVertexBufferPointer++;
        }

        m_QuadIndexCount += 6;

        m_Statistics.QuadCount++;
    }

    void DrawList2D::AddLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color, i32 entity /*= -1*/)
    {
        m_LineVertexBufferPointer->Position = p0;
        m_LineVertexBufferPointer->Color = color;
        m_LineVertexBufferPointer->EntityID = entity;
        m_LineVertexBufferPointer++;

        m_LineVertexBufferPointer->Position = p1;
        m_LineVertexBufferPointer->Color = color;
        m_LineVertexBufferPointer->EntityID = entity;
        m_LineVertexBufferPointer++;

        m_LineVertexCount += 2;
    }

    void DrawList2D::AddCircle(const glm::mat4& transform, const glm::vec4& color, f32 thickness, f32 fade, i32 entity)
    {
        if (m_QuadIndexCount >= DrawList2D::MaxQuadIndices)
        {
            FlushAndReset();
        }

        for (u32 i = 0; i < 4; ++i)
        {
            m_CircleVertexBufferPointer->WorldPosition = transform * m_QuadVertexPositions[i];
            m_CircleVertexBufferPointer->LocalPosition = m_QuadVertexPositions[i] * 2.0f;
            m_CircleVertexBufferPointer->Color = color;
            m_CircleVertexBufferPointer->Fade = fade;
            m_CircleVertexBufferPointer->Thickness = thickness;
            m_CircleVertexBufferPointer->EntityID = entity;
            m_CircleVertexBufferPointer++;
        }

        m_CircleIndexCount += 6;

        m_Statistics.CircleCount++;
    }

    void DrawList2D::AddRect(const glm::mat4& transform, const glm::vec4& color, i32 entity)
    {
        glm::vec3 lineVertices[4];
        for (u32 i = 0; i < 4; ++i)
            lineVertices[i] = transform * m_QuadVertexPositions[i];

        AddLine(lineVertices[0], lineVertices[1], color, entity);
        AddLine(lineVertices[1], lineVertices[2], color, entity);
        AddLine(lineVertices[2], lineVertices[3], color, entity);
        AddLine(lineVertices[3], lineVertices[0], color, entity);
    }

    void DrawList2D::AddString(const glm::mat4& transform, const glm::vec4& color, Ref<Font> font, const std::string& string, f32 kerningOffset, f32 lineSpacing, i32 entity)
    {
        const auto& fontGeometry = font->GetMSDFData()->FontGeometry;
        const auto& metrics = fontGeometry.getMetrics();

        Ref<Texture2D> fontAtlas = font->GetAtlasTexture();

        f64 x = 0.0;
        f64 fsScale = 1.0 / (metrics.ascenderY - metrics.descenderY);
        f64 y = 0.0;
        f32 lineHeightOffset = 0.0f;

        const f64 spaceGlyphAdvance = fontGeometry.getGlyph(' ')->getAdvance();

        for (size_t i = 0; i < string.size(); ++i)
        {
            char character = string[i];

            if (character == '\r')
                continue;

            if (character == '\n')
            {
                x = 0; // TODO: Text aligning
                y -= fsScale * metrics.lineHeight + lineHeightOffset + lineSpacing;
                continue;
            }

            if (character == ' ')
            {
                f32 advance = static_cast<f32>(spaceGlyphAdvance);
                if (i < string.size() - 1) // TODO: Figure out monospacing
                {
                    char nextCharacter = string[i + 1];
                    double dAdvance;
                    fontGeometry.getAdvance(dAdvance, character, nextCharacter);
                    x += fsScale * dAdvance + kerningOffset;

                    advance = static_cast<f32>(dAdvance);
                }
                x += fsScale * advance + kerningOffset;

                continue;
            }

            auto glyph = fontGeometry.getGlyph(character);
            if (!glyph)
                glyph = fontGeometry.getGlyph('?');
            if (!glyph)
                continue;

            if (character == '\t')
            {
                x += 4.0 * (fsScale * spaceGlyphAdvance + kerningOffset); // 4x 
                continue;
            }

            f64 al, ab, ar, at;
            glyph->getQuadAtlasBounds(al, ab, ar, at);
            glm::vec2 texCoordMin((f32)al, (f32)ab);
            glm::vec2 texCoordMax((f32)ar, (f32)at);

            f64 pl, pb, pr, pt;
            glyph->getQuadPlaneBounds(pl, pb, pr, pt);
            glm::vec2 quadMin((f32)pl, (f32)pb);
            glm::vec2 quadMax((f32)pr, (f32)pt);

            quadMin *= fsScale;
            quadMax *= fsScale;
            quadMin += glm::vec2(x, y);
            quadMax += glm::vec2(x, y);

            f32 texelWidth = 1.0f / fontAtlas->GetWidth();
            f32 texelHeight = 1.0f / fontAtlas->GetHeight();
            texCoordMin *= glm::vec2(texelWidth, texelHeight);
            texCoordMax *= glm::vec2(texelWidth, texelHeight);

            // Render 
            m_TextVertexBufferPointer->Position = transform * glm::vec4(quadMin, 0.0f, 1.0f);
            m_TextVertexBufferPointer->Color = color;
            m_TextVertexBufferPointer->EntityID = entity;
            m_TextVertexBufferPointer->TextureIndex = m_FontTextureSlotsIndex;
            m_TextVertexBufferPointer->TexCoord = texCoordMin;
            m_TextVertexBufferPointer++;

            m_TextVertexBufferPointer->Position = transform * glm::vec4(quadMin.x, quadMax.y, 0.0f, 1.0f);
            m_TextVertexBufferPointer->Color = color;
            m_TextVertexBufferPointer->EntityID = entity;
            m_TextVertexBufferPointer->TextureIndex = m_FontTextureSlotsIndex;
            m_TextVertexBufferPointer->TexCoord = { texCoordMin.x, texCoordMax.y };
            m_TextVertexBufferPointer++;

            m_TextVertexBufferPointer->Position = transform * glm::vec4(quadMax, 0.0f, 1.0f);
            m_TextVertexBufferPointer->Color = color;
            m_TextVertexBufferPointer->EntityID = entity;
            m_TextVertexBufferPointer->TextureIndex = m_FontTextureSlotsIndex;
            m_TextVertexBufferPointer->TexCoord = texCoordMax;
            m_TextVertexBufferPointer++;

            m_TextVertexBufferPointer->Position = transform * glm::vec4(quadMax.x, quadMin.y, 0.0f, 1.0f);
            m_TextVertexBufferPointer->Color = color;
            m_TextVertexBufferPointer->EntityID = entity;
            m_TextVertexBufferPointer->TextureIndex = m_FontTextureSlotsIndex;
            m_TextVertexBufferPointer->TexCoord = { texCoordMax.x, texCoordMin.y };
            m_TextVertexBufferPointer++;

            m_TextIndexCount += 6;
            m_Statistics.QuadCount++;

            if (i < string.size() - 1)
            {
                char nextCharacter = string[i + 1];

                f64 advance;
                fontGeometry.getAdvance(advance, character, nextCharacter);

                x += fsScale * advance + kerningOffset;
            }
        }
    }

    void DrawList2D::OnViewportResize(u32 width, u32 height)
    {
        
    }

    void DrawList2D::SetTargetRenderPass(const Ref<RenderPass>& renderPass)
    {
        m_TargerRenderPass = renderPass;
    }
}
