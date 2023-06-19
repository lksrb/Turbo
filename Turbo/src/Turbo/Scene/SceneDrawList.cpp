#include "tbopch.h"
#include "SceneDrawList.h"

#include "Turbo/Scene/Scene.h"
#include "Turbo/Core/Engine.h"

#include "Turbo/Platform/Vulkan/VulkanSwapChain.h"
#include "Turbo/Platform/Vulkan/VulkanRenderCommandBuffer.h"
#include "Turbo/Platform/Vulkan/VulkanGraphicsPipeline.h"
#include "Turbo/Platform/Vulkan/VulkanImage2D.h"

namespace Turbo
{
    SceneDrawList::SceneDrawList(const SceneDrawList::Config& config)
        : m_Config(config)
    {
        Init();
    }

    SceneDrawList::~SceneDrawList()
    {
    }

    void SceneDrawList::Init()
    {
        // Separate command buffer
        m_RenderCommandBuffer = RenderCommandBuffer::Create();

        // Color attachment
        FrameBuffer::Attachment colorAttachment = {};
        colorAttachment.ColorMask = FrameBuffer::ColorWriteMask_RGBA;
        colorAttachment.EnableBlend = true;
        colorAttachment.BlendOperation = FrameBuffer::BlendOperation_Add;
        colorAttachment.SrcBlendFactor = FrameBuffer::BlendFactor_SrcAlpha;
        colorAttachment.DstBlendFactor = FrameBuffer::BlendFactor_OneMinus_SrcAlpha;

        // Target Framebuffer
        // This will be the main canvas
        FrameBuffer::Config frameBufferConfig = {};
        frameBufferConfig.ColorAttachment = colorAttachment;
        frameBufferConfig.EnableDepthTesting = true;
        Ref<FrameBuffer> targetFrameBuffer = FrameBuffer::Create(frameBufferConfig);

        // Main render pass
        RenderPass::Config config = {};
        config.TargetFrameBuffer = targetFrameBuffer;
        config.ClearOnLoad = false;
        m_FinalRenderPass = RenderPass::Create(config);
        m_FinalRenderPass->Invalidate();

        targetFrameBuffer->SetRenderPass(m_FinalRenderPass);
        targetFrameBuffer->Invalidate(m_Config.ViewportWidth, m_Config.ViewportHeight);

        // Create Renderer2D
        m_DrawList2D = DrawList2D::Create();
        m_DrawList2D->SetTargetRenderPass(m_FinalRenderPass);
        m_DrawList2D->Initialize();
        
        // NOTE: First pass should clear everything
        // Cube pass
        {
            size_t maxVertices = 24 * 10;
            m_CubeVertexBuffer = VertexBuffer::Create({ maxVertices * sizeof(CubeVertex) });

            RenderPass::Config config = {};
            config.TargetFrameBuffer = targetFrameBuffer;
            config.ClearOnLoad = true;
            m_CubeRenderPass = RenderPass::Create(config);
            m_CubeRenderPass->Invalidate();
            m_CubeShader = Shader::Create({ ShaderLanguage::GLSL, "Assets/Shaders/Cube.glsl" });

            GraphicsPipeline::Config pipelineConfig = {};
            pipelineConfig.Renderpass = m_CubeRenderPass;
            pipelineConfig.DepthTesting = false;
            pipelineConfig.Shader = m_CubeShader;
            pipelineConfig.TargetFramebuffer = targetFrameBuffer;
            pipelineConfig.Topology = PrimitiveTopology::Triangle;
            m_CubePipeline = GraphicsPipeline::Create(pipelineConfig);
            m_CubePipeline->Invalidate();
        }

        // Create camera uniform buffer
        m_UniformBufferSet = UniformBufferSet::Create();
        m_UniformBufferSet->Create(0, 0, sizeof(UBCamera));
        // Wait for renderer2d to finish its work and do its own thing
        //m_CompositeRenderPass->DependsOn(m_SecondRenderPass);
    }

    void SceneDrawList::SetCamera(const Camera& camera)
    {
        // NOTE: This is duplicate but keep it for now
        m_DrawList2D->SetCameraTransform(camera.GetViewProjection());

        // u_Camera, will be on the set on 0 and bound on 0
        Renderer::Submit([this, camera]()
        {
            m_UniformBufferSet->SetData(0, 0, &camera.GetViewProjection());
        });
    }

    void SceneDrawList::Begin()
    {
        m_DrawList2D->Begin();
    }

    void SceneDrawList::End()
    {
        RenderGeometry();

        // NOTE: Drawing with multiple renderpasses works
        //m_DrawList2D->End();

        UpdateStatistics();
    }

    void SceneDrawList::RenderGeometry()
    {
        // Should not overdraw
        m_RenderCommandBuffer->Begin();
        Renderer::BeginRenderPass(m_RenderCommandBuffer, m_CubeRenderPass, { 0, 0, 0, 1 });
        
        // Data
        CubeVertex cubeVertices[] = {
/*
              -0.5, -0.5,  0.0,   // Vertex 0
               0.5, -0.5,  0.0,   // Vertex 1
               0.5,  0.5,  0.0,   // Vertex 2
               0.5,  0.5,  0.0,   // Vertex 3
              -0.5,  0.5,  0.0,   // Vertex 4
              -0.5, -0.5,  0.0    // Vertex 5
             */
            // Front face
            glm::vec3(-0.5f, -0.5f, 0.5f),   // Vertex 0
            glm::vec3( 0.5f, -0.5f, 0.5f),   // Vertex 1
            glm::vec3( 0.5f,  0.5f, 0.5f),   // Vertex 2
            glm::vec3(-0.5f,  0.5f, 0.5f),   // Vertex 3

            // Back face
            glm::vec3(-0.5f, -0.5f, -0.5f),  // Vertex 4
            glm::vec3( 0.5f, -0.5f, -0.5f),  // Vertex 5
            glm::vec3( 0.5f,  0.5f, -0.5f),  // Vertex 6
            glm::vec3(-0.5f,  0.5f, -0.5f),  // Vertex 7

            // Left face
            glm::vec3(-0.5f, -0.5f, -0.5f),  // Vertex 8
            glm::vec3(-0.5f, -0.5f, 0.5f),   // Vertex 9
            glm::vec3(-0.5f,  0.5f, 0.5f),   // Vertex 10
            glm::vec3(-0.5f,  0.5f, -0.5f),  // Vertex 11

            // Right face
            glm::vec3(0.5f, -0.5f, -0.5f),  // Vertex 12
            glm::vec3(0.5f, -0.5f, 0.5f ),  // Vertex 13
            glm::vec3(0.5f,  0.5f, 0.5f ),  // Vertex 14
            glm::vec3(0.5f,  0.5f, -0.5f),  // Vertex 15

             // Top face
             glm::vec3(-0.5f, 0.5f, -0.5f),   // Vertex 16
             glm::vec3( 0.5f, 0.5f, -0.5f),   // Vertex 17
             glm::vec3( 0.5f, 0.5f, 0.5f ),    // Vertex 18
             glm::vec3(-0.5f, 0.5f, 0.5f ),    // Vertex 19

             // Bottom face
             glm::vec3(-0.5f, -0.5f, -0.5f),  // Vertex 20
             glm::vec3( 0.5f, -0.5f, -0.5f),  // Vertex 21
             glm::vec3( 0.5f, -0.5f, 0.5f ),   // Vertex 22
             glm::vec3(-0.5f, -0.5f, 0.5f )   // Vertex 23
        };
        glm::vec3 cubeVertices2[] = {
            // Front face
            glm::vec3(-0.5, -0.5, 0.5),   // Vertex 0
            glm::vec3(0.5, -0.5, 0.5),   // Vertex 1
            glm::vec3(0.5, 0.5, 0.5),   // Vertex 2
            glm::vec3(-0.5, -0.5, 0.5),   // Vertex 0
            glm::vec3(0.5, 0.5, 0.5),   // Vertex 2
            glm::vec3(-0.5, 0.5, 0.5),   // Vertex 3

            // Back face
            glm::vec3(-0.5, -0.5, -0.5),   // Vertex 4
            glm::vec3(0.5, -0.5, -0.5),   // Vertex 5
            glm::vec3(0.5, 0.5, -0.5),   // Vertex 6
            glm::vec3(-0.5, -0.5, -0.5),   // Vertex 4
            glm::vec3(0.5, 0.5, -0.5),   // Vertex 6
            glm::vec3(-0.5, 0.5, -0.5),   // Vertex 7

            // Left face
            glm::vec3(-0.5, -0.5, -0.5),   // Vertex 8
            glm::vec3(-0.5, -0.5, 0.5),   // Vertex 9
            glm::vec3(-0.5, 0.5, 0.5),   // Vertex 10
            glm::vec3(-0.5, -0.5, -0.5),   // Vertex 8
            glm::vec3(-0.5, 0.5, 0.5),   // Vertex 10
            glm::vec3(-0.5, 0.5, -0.5),   // Vertex 11

            // Right face
            glm::vec3(0.5, -0.5, -0.5),   // Vertex 12
            glm::vec3(0.5, -0.5, 0.5),   // Vertex 13
            glm::vec3(0.5, 0.5, 0.5),   // Vertex 14
            glm::vec3(0.5, -0.5, -0.5),   // Vertex 12
            glm::vec3(0.5, 0.5, 0.5),   // Vertex 14
            glm::vec3(0.5, 0.5, -0.5),   // Vertex 15

            // Top face
            glm::vec3(-0.5, 0.5, -0.5),   // Vertex 16
            glm::vec3(0.5, 0.5, -0.5),   // Vertex 17
            glm::vec3(0.5, 0.5, 0.5),   // Vertex 18
            glm::vec3(-0.5, 0.5, -0.5),   // Vertex 16
            glm::vec3(0.5, 0.5, 0.5),   // Vertex 18
            glm::vec3(-0.5, 0.5, 0.5),   // Vertex 19

            // Bottom face
            glm::vec3(-0.5, -0.5, -0.5),   // Vertex 20
            glm::vec3(0.5, -0.5, -0.5),   // Vertex 21
            glm::vec3(0.5, -0.5, 0.5),   // Vertex 22
            glm::vec3(-0.5, -0.5, -0.5),   // Vertex 20
            glm::vec3(0.5, -0.5, 0.5),   // Vertex 22
            glm::vec3(-0.5, -0.5, 0.5)    // Vertex 23
    };

        size_t count = sizeof(cubeVertices2) / sizeof(cubeVertices2[0]);
        m_CubeVertexBuffer->SetData(&cubeVertices2, count * sizeof(CubeVertex));
        Renderer::Draw(m_RenderCommandBuffer, m_CubeVertexBuffer, m_UniformBufferSet, m_CubePipeline, m_CubeShader, count);

        Renderer::EndRenderPass(m_RenderCommandBuffer);
        m_RenderCommandBuffer->End();
        m_RenderCommandBuffer->Submit();
    }

    void SceneDrawList::UpdateStatistics()
    {
        m_Statistics.Statistics2D = m_DrawList2D->GetStatistics();
    }

    void SceneDrawList::AddQuad(const glm::vec3& position, const glm::vec2& size, f32 rotation, const glm::vec4& color, i32 entity)
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::rotate(glm::mat4(1.0f), glm::radians(rotation), { 0.0f, 0.0f, 1.0f })
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        AddQuad(transform, color, entity);
    }

    void SceneDrawList::AddQuad(const glm::mat4& transform, const glm::vec4& color, i32 entity)
    {
        m_DrawList2D->AddQuad(transform, color, entity);
    }

    void SceneDrawList::AddSprite(const glm::mat4& transform, const glm::vec4& color, Ref<SubTexture2D> subTexture, f32 tiling, i32 entity)
    {
        m_DrawList2D->AddSprite(transform, color, subTexture, tiling, entity);
    }

    void SceneDrawList::AddLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color, i32 entity)
    {
        m_DrawList2D->AddLine(p0, p1, color, entity);
    }

    void SceneDrawList::AddCircle(const glm::mat4& transform, const glm::vec4& color, f32 thickness, f32 fade, i32 entity)
    {
        m_DrawList2D->AddCircle(transform, color, thickness, fade, entity);
    }

    void SceneDrawList::AddRect(const glm::vec3& position, const glm::vec2& size, f32 rotation, const glm::vec4& color, i32 entity)
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::rotate(glm::mat4(1.0f), glm::radians(rotation), { 0.0f, 0.0f, 1.0f })
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        AddRect(transform, color, entity);
    }

    void SceneDrawList::AddRect(const glm::mat4& transform, const glm::vec4& color, i32 entity)
    {
        m_DrawList2D->AddRect(transform, color, entity);
    }

    void SceneDrawList::AddString(const glm::mat4& transform, const glm::vec4& color, Ref<Font> font, const std::string& string, f32 kerningOffset, f32 lineSpacing, i32 entity)
    {
        m_DrawList2D->AddString(transform, color, font, string, kerningOffset, lineSpacing, entity);
    }

    void SceneDrawList::OnViewportResize(u32 width, u32 height)
    {
        m_Config.ViewportWidth = width;
        m_Config.ViewportHeight = height;

        // Should framebuffer resize?
        /*Renderer::Submit([this, width, height]()
        {
            //m_CompositeRenderPass->GetConfig().TargetFrameBuffer->Invalidate(width, height);
            //m_DrawList2D->OnViewportResize(width, height);
        });*/
    }

    Ref<Image2D> SceneDrawList::GetFinalImage() const
    {
        // Returns final image that is produced by the draw list
        return m_FinalRenderPass->GetConfig().TargetFrameBuffer->GetColorAttachment();
    }
}
