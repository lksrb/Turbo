#include "tbopch.h"
#include "SceneDrawList.h"

#include "Turbo/Scene/Scene.h"
#include "Turbo/Core/Engine.h"

#include "Turbo/Platform/Vulkan/VulkanSwapChain.h"
#include "Turbo/Platform/Vulkan/VulkanRenderCommandBuffer.h"
#include "Turbo/Platform/Vulkan/VulkanGraphicsPipeline.h"
#include "Turbo/Platform/Vulkan/VulkanImage2D.h"
#include <Turbo/Platform/Vulkan/VulkanShader.h>

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
            m_CubeInstances.reserve(MaxCubes);
            m_CubeVertexBuffer = VertexBuffer::Create({ MaxCubeVertices * sizeof(CubeVertex) });

            // This will be the same as quad indicies
            // because cube is just 6 quads and we still need normals from those quads
            {
                u32* quadIndices = new u32[MaxCubeIndices];
                u32 offset = 0;
                for (u32 i = 0; i < MaxCubeIndices; i += 6)
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
                config.Size = MaxCubeIndices * sizeof(u32);
                config.Indices = quadIndices;
                m_CubeIndexBuffer = IndexBuffer::Create(config);

                delete[] quadIndices;
            }

            m_CubeInstanceBuffer = VertexBuffer::Create({ MaxCubes * sizeof(CubeInstance) });

            RenderPass::Config config = {};
            config.TargetFrameBuffer = targetFrameBuffer;
            config.ClearOnLoad = true;
            m_CubeRenderPass = RenderPass::Create(config);
            m_CubeRenderPass->Invalidate();
            m_CubeShader = Shader::Create({ ShaderLanguage::GLSL, "Assets/Shaders/Cube.glsl" });

            GraphicsPipeline::Config pipelineConfig = {};
            pipelineConfig.Renderpass = m_CubeRenderPass;
            pipelineConfig.DepthTesting = true;
            pipelineConfig.Shader = m_CubeShader;
            pipelineConfig.TargetFramebuffer = targetFrameBuffer;
            pipelineConfig.Layout = VertexBufferLayout
            {
                {AttributeType::Vec3, "a_VertexPosition" },
                {AttributeType::Vec3, "a_Normal" },
                {AttributeType::Vec2, "a_TexCoord" }
            };
            pipelineConfig.InstanceLayout = VertexBufferLayout
            {
                {AttributeType::Vec4, "a_TransformRow0" },
                {AttributeType::Vec4, "a_TransformRow1" },
                {AttributeType::Vec4, "a_TransformRow2" },
                {AttributeType::Vec4, "a_TransformRow3" },
                {AttributeType::Vec4, "a_Color" },
                {AttributeType::Int, "a_EntityID" }
            };

            pipelineConfig.Topology = PrimitiveTopology::Triangle;
            m_CubePipeline = GraphicsPipeline::Create(pipelineConfig);
            m_CubePipeline->Invalidate();
        }

        // Create camera uniform buffer
        m_UniformBufferSet = UniformBufferSet::Create();
        m_UniformBufferSet->Create(0, 0, sizeof(UBCamera));
        m_UniformBufferSet->Create(0, 2, sizeof(PointLightData));
        // Wait for renderer2d to finish its work and do its own thing
        //m_CompositeRenderPass->DependsOn(m_SecondRenderPass);

        m_ContainerDiffuse = Texture2D::Create("Assets/Textures/Container.png");
        m_ContainerSpecular = Texture2D::Create("Assets/Textures/ContainerSpecular.png");
        m_CubeMaterial = Material::Create({ m_CubeShader });
    }

    void SceneDrawList::SetSceneData(const SceneRendererData& data)
    {
        m_SceneRendererData = data;

        // NOTE: This is duplicate but keep it for now
        m_DrawList2D->SetCameraTransform(data.ViewProjectionMatrix);

        // u_Camera, will be on the set on 0 and bound on 0
        Renderer::Submit([this, data]()
        {
            m_UniformBufferSet->SetData(0, 0, &data);
        });
    }

    void SceneDrawList::Begin()
    {
        // Clear everything
        m_CubeInstances.clear();
        m_PointLights.Count = 0;
    }

    void SceneDrawList::End()
    {
        // Submit point lights
        Renderer::Submit([this]()
        {
            m_UniformBufferSet->SetData(0, 2, &m_PointLights);
        });

        m_RenderCommandBuffer->Begin();
        Renderer::BeginRenderPass(m_RenderCommandBuffer, m_CubeRenderPass, { 0.0f, 0.0f, 0.0f, 1 });

        // Data
        // TODO: This will be serialized in some model file (.fbx or someting)
        std::array<CubeVertex, 24> cubeVertices = {
            // Front face
            CubeVertex{ glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f) },  // Vertex 0
            CubeVertex{ glm::vec3(0.5f, -0.5f, 0.5f),  glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f) },  // Vertex 1
            CubeVertex{ glm::vec3(0.5f,  0.5f, 0.5f),  glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f) },  // Vertex 2
            CubeVertex{ glm::vec3(-0.5f,  0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f) },  // Vertex 3

            // Back face
            CubeVertex{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 0.0f) }, // Vertex 4
            CubeVertex{ glm::vec3(0.5f,  -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 0.0f) }, // Vertex 5
            CubeVertex{ glm::vec3(0.5f,   0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 1.0f) }, // Vertex 6
            CubeVertex{ glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 1.0f) }, // Vertex 7

            // Left face
            CubeVertex{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f) }, // Vertex 8
            CubeVertex{ glm::vec3(-0.5f, -0.5f, 0.5f),  glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f) }, // Vertex 9
            CubeVertex{ glm::vec3(-0.5f,  0.5f, 0.5f), glm::vec3(-1.0f, 0.0f, 0.0f) , glm::vec2(1.0f, 1.0f) },  // Vertex 10
            CubeVertex{ glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f) }, // Vertex 11

            // Right face
            CubeVertex{ glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(1.0f ,0.0f, 0.0f), glm::vec2(0.0f, 0.0f) }, // Vertex 12
            CubeVertex{ glm::vec3(0.5f, -0.5f,  0.5f), glm::vec3(1.0f ,0.0f, 0.0f), glm::vec2(1.0f, 0.0f) }, // Vertex 13
            CubeVertex{ glm::vec3(0.5f,  0.5f,  0.5f), glm::vec3(1.0f ,0.0f, 0.0f), glm::vec2(1.0f, 1.0f) }, // Vertex 14
            CubeVertex{ glm::vec3(0.5f,  0.5f, -0.5f), glm::vec3(1.0f ,0.0f, 0.0f), glm::vec2(0.0f, 1.0f) }, // Vertex 15

            // Top face
            CubeVertex{ glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f) },  // Vertex 16
            CubeVertex{ glm::vec3(0.5f,  0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f) },  // Vertex 17
            CubeVertex{ glm::vec3(0.5f,  0.5f,  0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f) },  // Vertex 18
            CubeVertex{ glm::vec3(-0.5f, 0.5f, 0.5f),  glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f) },  // Vertex 19

            // Bottom face
            CubeVertex{ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f) }, // Vertex 20
            CubeVertex{ glm::vec3(0.5f,  -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f) }, // Vertex 21
            CubeVertex{ glm::vec3(0.5f,  -0.5f,  0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f) }, // Vertex 22
            CubeVertex{ glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f) }, // Vertex 23
        };

        m_CubeMaterial->Set("u_MaterialTexture", m_ContainerDiffuse, 0);
        m_CubeMaterial->Set("u_MaterialTexture", m_ContainerSpecular, 1);

        m_CubeVertexBuffer->SetData(cubeVertices.data(), cubeVertices.size() * sizeof(CubeVertex));
        m_CubeInstanceBuffer->SetData(m_CubeInstances.data(), m_CubeInstances.size() * sizeof(CubeInstance));

        //Renderer::PushConstant(m_RenderCommandBuffer, m_CubePipeline, 64, &m_ViewProjection);
        Renderer::DrawInstanced(m_RenderCommandBuffer, m_CubeVertexBuffer, m_CubeInstanceBuffer, m_CubeIndexBuffer, m_UniformBufferSet, m_CubePipeline, m_CubeShader, (u32)m_CubeInstances.size(), 36);
        Renderer::EndRenderPass(m_RenderCommandBuffer);

        m_RenderCommandBuffer->End();
        m_RenderCommandBuffer->Submit();

        // NOTE: Drawing with multiple renderpasses works
        //m_DrawList2D->End();
        m_DrawList2D->Begin();

        UpdateStatistics();
    }

    void SceneDrawList::AddCube(const glm::mat4& transform, const glm::vec4& color, i32 entity)
    {
        auto& cube = m_CubeInstances.emplace_back();
        cube.Tranform[0] = transform[0];
        cube.Tranform[1] = transform[1];
        cube.Tranform[2] = transform[2];
        cube.Tranform[3] = transform[3];
        cube.EntityID = entity;
        cube.Color = color;
    }

    void SceneDrawList::AddPointLight(const glm::vec3& position, f32 intensity, f32 radius, f32 fallOff, i32 entityID)
    {
        PointLight& pointLight = m_PointLights[m_PointLights.Count];
        pointLight.Position = glm::vec4(position, 1.0f);
        pointLight.Intensity = intensity;
        pointLight.Radius = radius;
        pointLight.FallOff = fallOff;

        m_PointLights.Count++;
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
