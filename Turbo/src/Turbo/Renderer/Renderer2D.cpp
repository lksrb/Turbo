#include "tbopch.h"
#include "Renderer2D.h"

#include "Turbo/Core/Engine.h"
#include "Turbo/Debug/ScopeTimer.h"

#include <glm/ext/matrix_transform.hpp>

namespace Turbo
{
    Renderer2D::Renderer2D()
    {
    }

    Renderer2D::~Renderer2D()
    {
        Shutdown();
    }

    void Renderer2D::Initialize()
    {
        // Render command buffer
        m_CommandBuffer = CommandBuffer::Create(CommandBufferLevel::Primary);

        // Default clear color
        m_ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

        // Quad setup
        {
            // Index buffer
            {
                u32* quadIndices = new u32[MaxIndices];
                u32 offset = 0;
                for (u32 i = 0; i < MaxIndices; i += 6)
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
                config.Size = MaxIndices * sizeof(u32);
                config.Indices = quadIndices;
                m_QuadIndexBuffer = IndexBuffer::Create(config);

                delete[] quadIndices;
            }
            // Vertex buffer
            m_QuadVertexBufferBase = m_QuadVertexBufferPointer = new QuadVertex[MaxVertices];

            m_QuadVertexBuffer = VertexBuffer::Create({ MaxVertices * sizeof(QuadVertex) });

            // Shader
            Shader::Config shaderConfig = {};
            shaderConfig.Language = ShaderLanguage::GLSL;
            shaderConfig.ShaderPath = "Assets\\Shaders\\Renderer2D_Quad.glsl";
            m_QuadShader = Shader::Create(shaderConfig);

            // Graphics pipeline
            GraphicsPipeline::Config config = {};
            config.Shader = m_QuadShader;
            config.Renderpass = m_TargetFramebuffer->GetConfig().Renderpass;
            config.Topology = PrimitiveTopology::Triangle;
            config.DepthTesting = true;
            config.TargetFramebuffer = m_TargetFramebuffer; 
            m_QuadPipeline = GraphicsPipeline::Create(config);
            m_QuadPipeline->Invalidate();

            // Material
            m_QuadMaterial = Material::Create({ m_QuadShader });
        }


        // White texture for texture-less quads
        m_WhiteTexture = Texture2D::Create(0xffffffff);

        // Set white texture at the top of the stack
        m_TextureSlots[0] = m_WhiteTexture;
    }

    void Renderer2D::Shutdown()
    {
        delete m_QuadVertexBufferBase;
    }

    void Renderer2D::Begin(const Camera& camera)
    {
        // Enable drawing
        m_BeginDraw = true;

        m_QuadMaterial->Set("u_Camera", camera.GetViewProjection());

        // Reset texture indexing
        m_TextureSlotsIndex = 1;

        // Reset textures
        for (u32 i = 1; i < m_TextureSlots.size(); ++i)
            m_TextureSlots[i] = nullptr;

        StartBatch();
    }

    void Renderer2D::StartBatch()
    {
        m_QuadVertexBufferPointer = m_QuadVertexBufferBase;

        // Reset statistics
        m_RenderInfo.Reset();
    }

    void Renderer2D::End()
    {
        Flush();

        m_BeginDraw = false;
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, f32 rotation, const glm::vec4& color, i32 entity)
    {
        TBO_ENGINE_ASSERT(m_BeginDraw, "Call Begin() before issuing a draw command!");

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::rotate(glm::mat4(1.0f), glm::radians(rotation), { 0.0f, 0.0f, 1.0f })
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        DrawQuad(transform, color, entity);
    }

    void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color, i32 entity)
    {
        TBO_ENGINE_ASSERT(m_BeginDraw, "Call Begin() before issuing a draw command!");

        /*if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices) // // TODO(Urby): Flushing batch renderer
            NextBatch();*/

        constexpr u32 quadVertexCount = 4;
        constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

        for (u32 i = 0; i < quadVertexCount; ++i)
        {
            m_QuadVertexBufferPointer->Position = transform * QuadVertexPositions[i];
            m_QuadVertexBufferPointer->Color = color;
            m_QuadVertexBufferPointer->EntityID = entity;
            m_QuadVertexBufferPointer->TextureIndex = 0;
            m_QuadVertexBufferPointer->TexCoord = textureCoords[i];
            m_QuadVertexBufferPointer->TilingFactor = 0.0f;
            m_QuadVertexBufferPointer++;
        }

        m_RenderInfo.QuadIndexCount += 6;

        m_RenderInfo.QuadCount++;
    }

    void Renderer2D::DrawSprite(const glm::mat4& transform, const glm::vec4& color, Ref<Texture2D> texture, f32 tiling, i32 entity /*= -1*/)
    {
        TBO_ENGINE_ASSERT(m_BeginDraw, "Call Begin() before issuing a draw command!");

        if (m_RenderInfo.QuadIndexCount >= Renderer2D::MaxIndices)
            TBO_ENGINE_ASSERT(false);

        u32 texture_index = 0; // White Texture
        constexpr u32 quad_vertex_count = 4;
        constexpr glm::vec2 texture_coords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

        if (texture)
        {
            for (u32 i = 1; i < m_TextureSlotsIndex; ++i)
            {
                // If the texture is in the stack, just modify textureIndex
                if (texture->GetHash() == m_TextureSlots[i]->GetHash())
                {
                    texture_index = i;
                    break;
                }
            }

            // If the texture is not present in texture stack, add it
            if (texture_index == 0)
            {
                TBO_ENGINE_ASSERT(m_TextureSlotsIndex < MaxTextureSlots); // TODO: Flush and reset

                texture_index = m_TextureSlotsIndex;
                m_TextureSlots[m_TextureSlotsIndex] = texture;
                m_TextureSlotsIndex++;
            }
        }

        for (u32 i = 0; i < quad_vertex_count; ++i)
        {
            m_QuadVertexBufferPointer->Position = transform * QuadVertexPositions[i];
            m_QuadVertexBufferPointer->Color = color;
            m_QuadVertexBufferPointer->EntityID = entity;
            m_QuadVertexBufferPointer->TextureIndex = texture_index;
            m_QuadVertexBufferPointer->TexCoord = texture_coords[i];
            m_QuadVertexBufferPointer->TilingFactor = tiling;
            m_QuadVertexBufferPointer++;
        }

        m_RenderInfo.QuadIndexCount += 6;

        m_RenderInfo.QuadCount++;
    }

    void Renderer2D::DrawSprite(const glm::mat4& transform, const glm::vec4& color, Ref<SubTexture2D> subtexture, f32 tiling, i32 entity)
    {
        TBO_ENGINE_ASSERT(m_BeginDraw, "Call Begin() before issuing a draw command!");

        if (m_RenderInfo.QuadIndexCount >= Renderer2D::MaxIndices)
            TBO_ENGINE_ASSERT(false); // TODO(Urby): Flush and reset

        u32 texture_index = 0; // White Texture
        constexpr u32 quad_vertex_count = 4;
        glm::vec2 texture_coords[4];
        memcpy(texture_coords, subtexture->GetCoords().data(), 4 * sizeof(glm::vec2));

        if (subtexture)
        {
            for (u32 i = 1; i < m_TextureSlotsIndex; ++i)
            {
                // If the texture is in the stack, just modify textureIndex
                if (subtexture->GetTexture()->GetHash() == m_TextureSlots[i]->GetHash())
                {
                    texture_index = i;
                    break;
                }
            }

            // If the texture is not present in texture stack, add it
            if (texture_index == 0)
            {
                TBO_ENGINE_ASSERT(m_TextureSlotsIndex < MaxTextureSlots); // TODO: Flush and reset

                texture_index = m_TextureSlotsIndex;
                m_TextureSlots[m_TextureSlotsIndex] = subtexture->GetTexture();
                m_TextureSlotsIndex++;
            }
        }

        for (u32 i = 0; i < quad_vertex_count; ++i)
        {
            m_QuadVertexBufferPointer->Position = transform * QuadVertexPositions[i];
            m_QuadVertexBufferPointer->Color = color;
            m_QuadVertexBufferPointer->EntityID = entity;
            m_QuadVertexBufferPointer->TextureIndex = texture_index;
            m_QuadVertexBufferPointer->TexCoord = texture_coords[i];
            m_QuadVertexBufferPointer->TilingFactor = tiling;
            m_QuadVertexBufferPointer++;
        }

        m_RenderInfo.QuadIndexCount += 6;

        m_RenderInfo.QuadCount++;
    }

	void Renderer2D::DrawCircle(const glm::mat4& transform, const glm::vec4& color, f32 thickness, f32 fade, i32 entity)
	{
        // TODO:
	}

	void Renderer2D::Flush()
    {
        Renderer::Submit([this]()
        {
            // Texture slots
            for (u32 i = 0; i < m_TextureSlots.size(); ++i)
            {
                if (m_TextureSlots[i])
                    m_QuadMaterial->Set("u_Textures", m_TextureSlots[i], i);
                else
                    m_QuadMaterial->Set("u_Textures", m_WhiteTexture, i);
            }

            m_QuadMaterial->Update();

            u32 dataSize = (u32)((u8*)m_QuadVertexBufferPointer - (u8*)m_QuadVertexBufferBase);

            m_CommandBuffer->Begin();

            Renderer::BeginRenderPass(m_CommandBuffer, m_TargetFramebuffer, m_ClearColor);
            if (dataSize)
            {
                m_QuadVertexBuffer->SetData(m_QuadVertexBufferBase, dataSize); // TODO: Figure out how to submit transfering data

                // Record buffer
                Renderer::DrawIndexed(m_CommandBuffer, m_QuadVertexBuffer, m_QuadIndexBuffer, m_QuadPipeline, m_QuadShader, m_RenderInfo.QuadIndexCount);
            }
            Renderer::EndRenderPass(m_CommandBuffer);

            m_CommandBuffer->End();
            m_CommandBuffer->Submit();
        });

        // Increment draw calls
        if(m_RenderInfo.QuadIndexCount)
            m_RenderInfo.DrawCalls++;
    }

}
