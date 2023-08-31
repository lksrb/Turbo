#include "tbopch.h"
#include "DrawList2D.h"

#include "Renderer.h"
#include "ShaderLibrary.h"

#include "Font.h"
#include "Font-Internal.h"
#include "RenderCommandBuffer.h"
#include "GraphicsPipeline.h"
#include "IndexBuffer.h"
#include "Material.h"
#include "Texture.h"
#include "RenderPass.h"
#include "FrameBuffer.h"
#include "UniformBuffer.h"
#include "VertexBuffer.h"

#include "Turbo/Core/Application.h"
#include "Turbo/Core/Window.h"
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

        Window* window = Application::Get().GetViewportWindow();
        m_ViewportWidth = window->GetWidth();
        m_ViewportHeight = window->GetHeight();

        // Quad setup
        {
            // Vertex buffer
            m_QuadVertexBufferBase = m_QuadVertexBufferPointer = new QuadVertex[MaxQuadVertices];
            m_QuadVertexBuffer = VertexBuffer::Create(MaxQuadVertices * sizeof(QuadVertex));

            // Index buffer
            {
                std::vector<u32> quadIndices(MaxQuadIndices);
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

                m_QuadIndexBuffer = IndexBuffer::Create(quadIndices);
            }

            // Shader
            m_QuadShader = ShaderLibrary::Get("Renderer2D_Quad");

            // Graphics pipeline
            GraphicsPipeline::Config config = {};
            config.Shader = m_QuadShader;
            config.Topology = PrimitiveTopology::Triangle;
            config.Renderpass = m_TargerRenderPass;
            config.DepthTesting = true;
            config.SubpassIndex = 0;
            config.Layout = VertexBufferLayout
            {
                { AttributeType::Vec3, "a_Position" },
                { AttributeType::Vec4, "a_Color" },
                { AttributeType::Vec2, "a_TexCoord" },
                { AttributeType::UInt, "a_TexIndex" },
                { AttributeType::Float, "a_TilingFactor" },
                { AttributeType::Int, "a_EntityID" },
            };
            m_QuadPipeline = GraphicsPipeline::Create(config);
            m_QuadPipeline->Invalidate();

            // Material
            m_QuadMaterial = Material::Create({ m_QuadShader });
        }

        // Circle setup
        {
            // Vertex buffer
            m_CircleVertexBufferBase = m_CircleVertexBufferPointer = new CircleVertex[MaxQuadVertices];
            m_CircleVertexBuffer = VertexBuffer::Create(MaxQuadVertices * sizeof(CircleVertex));

            // Index buffer from quads

            // Shader
            m_CircleShader = ShaderLibrary::Get("Renderer2D_Circle");

            // Graphics pipeline
            GraphicsPipeline::Config config = {};
            config.Shader = m_CircleShader;
            config.Topology = PrimitiveTopology::Triangle;
            config.Renderpass = m_TargerRenderPass;
            config.DepthTesting = true;
            config.SubpassIndex = 1;
            config.Layout = VertexBufferLayout
            {
                { AttributeType::Vec3, "a_WorldPosition" },
                { AttributeType::Vec3, "a_LocalPosition" },
                { AttributeType::Vec4, "a_Color" },
                { AttributeType::Float, "a_Thickness" },
                { AttributeType::Float, "a_Fade" },
                { AttributeType::Int, "a_EntityID" },
            };
            m_CirclePipeline = GraphicsPipeline::Create(config);
            m_CirclePipeline->Invalidate();

            // Material - Maybe not necessary
            // m_CircleMaterial = Material::Create({ m_CircleShader });
        }

        // Line setup
        {
            // Vertex buffer
            m_LineVertexBufferBase = m_LineVertexBufferPointer = new LineVertex[MaxQuadVertices];
            m_LineVertexBuffer = VertexBuffer::Create(MaxQuadVertices * sizeof(CircleVertex));

            // Index buffer is not needed

            // Shader
            m_LineShader = ShaderLibrary::Get("Renderer2D_Line");

            // Graphics pipeline
            GraphicsPipeline::Config config = {};
            config.Shader = m_LineShader;
            config.Renderpass = m_TargerRenderPass;
            config.Topology = PrimitiveTopology::Line;
            config.DepthTesting = true;
            config.SubpassIndex = 2;
            config.Layout = VertexBufferLayout
            {
                { AttributeType::Vec3, "a_Position" },
                { AttributeType::Vec4, "a_Color" },
                { AttributeType::Int, "a_EntityID" },
            };
            m_LinePipeline = GraphicsPipeline::Create(config);
            m_LinePipeline->Invalidate();
        }

        // Text setup
        {
            // Vertex buffer
            m_TextVertexBufferBase = m_TextVertexBufferPointer = new TextVertex[MaxQuadVertices];
            m_TextVertexBuffer = VertexBuffer::Create(MaxQuadVertices * sizeof(TextVertex));

            // Index buffer is not needed

            // Shader
            m_TextShader = ShaderLibrary::Get("Renderer2D_Text");

            // Graphics PipeText
            GraphicsPipeline::Config config = {};
            config.Shader = m_TextShader;
            config.Topology = PrimitiveTopology::Triangle;
            config.Renderpass = m_TargerRenderPass;
            config.DepthTesting = false;
            config.SubpassIndex = 3;
            config.Layout = VertexBufferLayout
            {
                { AttributeType::Vec3, "a_Position" },
                { AttributeType::Vec4, "a_Color" },
                { AttributeType::Vec2, "a_TexCoord" },
                { AttributeType::UInt, "a_TexIndex" },
                { AttributeType::Int, "a_EntityID" },
            };
            m_TextPipeline = GraphicsPipeline::Create(config);
            m_TextPipeline->Invalidate();

            // Material
            m_TextMaterial = Material::Create({ m_TextShader });
        }

        // Create camera uniform buffer
        m_UniformBufferSet = UniformBufferSet::Create();
        m_UniformBufferSet->Create(0, 0, sizeof(UBCamera));

        // Set white texture at the top of the stack
        m_TextureSlots[0] = Renderer::GetWhiteTexture();

        // Selection buffer for mouse picking
        for (auto& buffer : m_SelectionBuffers)
        {
            RendererBuffer::Config config = {};
            config.Temporary = true;
            config.Size = m_ViewportWidth * m_ViewportHeight * sizeof(i32);
            config.UsageFlags = BufferUsageFlags_Transfer_Dst;
            config.MemoryFlags = MemoryPropertyFlags_HostVisible | MemoryPropertyFlags_HostCoherent | MemoryPropertyFlags_HostVisible;
            config.SetDefaultValue = true;
            config.DefaultValue = -1;
            buffer = RendererBuffer::Create(config);
        }
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
        TBO_PROFILE_FUNC();
 
        m_RenderCommandBuffer->Begin();

        // NOTE: Vertex buffers need render command buffer to copy data into GPU so they dont have to create their own
        // SetData - vkCmdCopyBuffer
        // Quads
        u32 dataSize = (u32)((u8*)m_QuadVertexBufferPointer - (u8*)m_QuadVertexBufferBase);
        m_QuadVertexBuffer->SetData(m_RenderCommandBuffer, m_QuadVertexBufferBase, dataSize);

        // Circles
        dataSize = (u32)((u8*)m_CircleVertexBufferPointer - (u8*)m_CircleVertexBufferBase);
        m_CircleVertexBuffer->SetData(m_RenderCommandBuffer, m_CircleVertexBufferBase, dataSize);

        // Lines
        dataSize = (u32)((u8*)m_LineVertexBufferPointer - (u8*)m_LineVertexBufferBase);
        m_LineVertexBuffer->SetData(m_RenderCommandBuffer, m_LineVertexBufferBase, dataSize);

        // Text
        dataSize = (u32)((u8*)m_TextVertexBufferPointer - (u8*)m_TextVertexBufferBase);
        m_TextVertexBuffer->SetData(m_RenderCommandBuffer, m_TextVertexBufferBase, dataSize);

        Renderer::BeginRenderPass(m_RenderCommandBuffer, m_TargerRenderPass);

        // Quads
        if (m_QuadIndexCount)
        {
            // Texture slots
            for (u32 i = 0; i < m_TextureSlots.size(); ++i)
            {
                if (m_TextureSlots[i])
                    m_QuadMaterial->Set("u_Textures", m_TextureSlots[i], i);
                else
                    m_QuadMaterial->Set("u_Textures", /* White texture */m_TextureSlots[0], i);
            }

            Renderer::DrawIndexed(m_RenderCommandBuffer, m_QuadVertexBuffer, m_QuadIndexBuffer, m_UniformBufferSet, m_QuadPipeline, m_QuadShader, m_QuadIndexCount);

            m_Statistics.DrawCalls++;
        }

        // Circles
        if (m_CircleIndexCount)
        {
            Renderer::DrawIndexed(m_RenderCommandBuffer, m_CircleVertexBuffer, m_QuadIndexBuffer, m_UniformBufferSet, m_CirclePipeline, m_CircleShader, m_CircleIndexCount);
            m_Statistics.DrawCalls++;
        }

        // Lines
        if (m_LineVertexCount)
        {
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

            Renderer::DrawIndexed(m_RenderCommandBuffer, m_TextVertexBuffer, m_QuadIndexBuffer, m_UniformBufferSet, m_TextPipeline, m_TextShader, m_TextIndexCount);
            m_Statistics.DrawCalls++;
        }

        Renderer::EndRenderPass(m_RenderCommandBuffer);

        // Copy selection buffer attachment from GPU memory to the host
        Renderer::CopyImageToBuffer(m_RenderCommandBuffer, m_TargerRenderPass->GetConfig().TargetFrameBuffer->GetAttachment(FrameBuffer::AttachmentType_SelectionBuffer), m_SelectionBuffers[Renderer::GetCurrentFrame()]);

        m_RenderCommandBuffer->End();
        m_RenderCommandBuffer->Submit();
    }

    void DrawList2D::SetSceneData(const SceneRendererData& data)
    {
        m_SceneData = data;
        // u_Camera, will be on the set on 0 and bound on 0
        // TODO: Maybe there could be only one camera buffer
        Renderer::Submit([this, data]()
        {
            m_UniformBufferSet->SetData(0, 0, &data.ViewProjectionMatrix);
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

    void DrawList2D::AddSprite(const glm::mat4& transform, const glm::vec4& color, Ref<Texture2D> texture, const std::array<glm::vec2, 4>& textureCoords, f32 tiling, i32 entity)
    {
        if (m_QuadIndexCount >= DrawList2D::MaxQuadIndices)
        {
            FlushAndReset();
        }

        u32 textureIndex = 0; // White Texture
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

    void DrawList2D::AddBillboardQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, Ref<Texture2D> texture, f32 tiling, i32 entity)
    {
        if (m_QuadIndexCount >= DrawList2D::MaxQuadIndices)
        {
            FlushAndReset();
        }

        u32 textureIndex = 0; // White Texture
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

        glm::mat4 cameraView = m_SceneData.ViewMatrix;
        glm::vec3 camRightWS = { cameraView[0][0], cameraView[1][0], cameraView[2][0] };
        glm::vec3 camUpWS = { cameraView[0][1], cameraView[1][1], cameraView[2][1] };

        m_QuadVertexBufferPointer->Position = position + camRightWS * (m_QuadVertexPositions[0].x) * size.x + camUpWS * m_QuadVertexPositions[0].y * size.y;
        m_QuadVertexBufferPointer->Color = color;
        m_QuadVertexBufferPointer->TexCoord = { 0.0f, 0.0f };
        m_QuadVertexBufferPointer->TextureIndex = textureIndex;
        m_QuadVertexBufferPointer->TilingFactor = tiling;
        m_QuadVertexBufferPointer->EntityID = entity;
        m_QuadVertexBufferPointer++;

        m_QuadVertexBufferPointer->Position = position + camRightWS * m_QuadVertexPositions[1].x * size.x + camUpWS * m_QuadVertexPositions[1].y * size.y;
        m_QuadVertexBufferPointer->Color = color;
        m_QuadVertexBufferPointer->TexCoord = { 1.0f, 0.0f };
        m_QuadVertexBufferPointer->TextureIndex = textureIndex;
        m_QuadVertexBufferPointer->TilingFactor = tiling;
        m_QuadVertexBufferPointer->EntityID = entity;
        m_QuadVertexBufferPointer++;

        m_QuadVertexBufferPointer->Position = position + camRightWS * m_QuadVertexPositions[2].x * size.x + camUpWS * m_QuadVertexPositions[2].y * size.y;
        m_QuadVertexBufferPointer->Color = color;
        m_QuadVertexBufferPointer->TexCoord = { 1.0f, 1.0f };
        m_QuadVertexBufferPointer->TextureIndex = textureIndex;
        m_QuadVertexBufferPointer->TilingFactor = tiling;
        m_QuadVertexBufferPointer->EntityID = entity;
        m_QuadVertexBufferPointer++;

        m_QuadVertexBufferPointer->Position = position + camRightWS * m_QuadVertexPositions[3].x * size.x + camUpWS * m_QuadVertexPositions[3].y * size.y;
        m_QuadVertexBufferPointer->Color = color;
        m_QuadVertexBufferPointer->TexCoord = { 0.0f, 1.0f };
        m_QuadVertexBufferPointer->TextureIndex = textureIndex;
        m_QuadVertexBufferPointer->TilingFactor = tiling;
        m_QuadVertexBufferPointer->EntityID = entity;
        m_QuadVertexBufferPointer++;

        m_QuadIndexCount += 6;

        m_Statistics.QuadCount++;
    }

    void DrawList2D::AddLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color, i32 entity)
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

    void DrawList2D::AddDebugCircle(const glm::mat4& transform, const glm::vec4& color, i32 entity)
    {
        i32 segments = 32;
        for (i32 i = 0; i < segments; i++)
        {
            f32 angle = 2.0f * glm::pi<f32>() * (f32)i / segments;
            glm::vec4 startPosition = { glm::cos(angle), glm::sin(angle), 0.0f, 1.0f };

            angle = 2.0f * glm::pi<f32>() * (f32)((i + 1) % segments) / segments;
            glm::vec4 endPosition = { glm::cos(angle), glm::sin(angle), 0.0f, 1.0f };

            glm::vec3 p0 = transform * startPosition;
            glm::vec3 p1 = transform * endPosition;
            AddLine(p0, p1, color, entity);
        }
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
        m_ViewportWidth = width;
        m_ViewportHeight = height;

        // Selection buffer for mouse picking
        for (auto& buffer : m_SelectionBuffers)
        {
            RendererBuffer::Config config = {};
            config.Temporary = true;
            config.Size = width * height * sizeof(i32);
            config.UsageFlags = BufferUsageFlags_Transfer_Dst;
            config.MemoryFlags = MemoryPropertyFlags_HostVisible | MemoryPropertyFlags_HostCoherent | MemoryPropertyFlags_HostVisible;
            config.SetDefaultValue = true;
            config.DefaultValue = -1;
            buffer = RendererBuffer::Create(config);
        }
    }

    i32 DrawList2D::ReadPixel(u32 x, u32 y)
    {
        u32 currentFrame = Renderer::GetCurrentFrame();
        const i32* data = (const i32*)m_SelectionBuffers[currentFrame]->GetData();

        size_t index = (y * m_ViewportWidth + x);
        if (index < (m_SelectionBuffers[currentFrame]->Size() / sizeof(i32)))
        {
            return data[index];
        }

        return -1;
    }

    void DrawList2D::SetTargetRenderPass(const Ref<RenderPass>& renderPass)
    {
        m_TargerRenderPass = renderPass;
    }
}
