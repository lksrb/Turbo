#include "tbopch.h"
#include "SceneDrawList.h"

#include "Turbo/Scene/Scene.h"
#include "Turbo/Core/Engine.h"

#include "Turbo/Platform/Vulkan/VulkanSwapChain.h"
#include "Turbo/Platform/Vulkan/VulkanRenderCommandBuffer.h"
#include "Turbo/Platform/Vulkan/VulkanGraphicsPipeline.h"
#include "Turbo/Platform/Vulkan/VulkanImage2D.h"
#include "Turbo/Platform/Vulkan/VulkanShader.h"

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
        config.ClearOnLoad = true;
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
            m_MeshTransformBuffer = VertexBuffer::Create(MaxTransforms * sizeof(TransformData));

            RenderPass::Config config = {};
            config.TargetFrameBuffer = targetFrameBuffer;
            config.ClearOnLoad = true;
            m_GeometryRenderPass = RenderPass::Create(config);
            m_GeometryRenderPass->Invalidate();
            m_GeometryShader = Shader::Create({ ShaderLanguage::GLSL, "Assets/Shaders/StaticMesh.glsl" });

            GraphicsPipeline::Config pipelineConfig = {};
            pipelineConfig.Renderpass = m_GeometryRenderPass;
            pipelineConfig.DepthTesting = true;
            pipelineConfig.Shader = m_GeometryShader;
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
            };

            pipelineConfig.Topology = PrimitiveTopology::Triangle;
            m_GeometryPipeline = GraphicsPipeline::Create(pipelineConfig);
            m_GeometryPipeline->Invalidate();
        }

        // Create camera uniform buffer
        m_UniformBufferSet = UniformBufferSet::Create();
        m_UniformBufferSet->Create(0, 0, sizeof(UBCamera));
        m_UniformBufferSet->Create(0, 2, sizeof(PointLightData));
        // Wait for renderer2d to finish its work and do its own thing
        //m_CompositeRenderPass->DependsOn(m_SecondRenderPass);

        m_ContainerDiffuse = Texture2D::Create("Assets/Meshes/Backpack/1001_albedo.jpg");
        m_ContainerSpecular = Texture2D::Create("Assets/Meshes/Backpack/1001_metallic.jpg");

        m_CubeMaterial = Material::Create({ m_GeometryShader });
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
        m_DrawCommands.clear();
        m_MeshTransformMap.clear();

        m_Statistics.Reset();

        m_PointLights.Count = 0;

        m_DrawList2D->Begin();
    }

    void SceneDrawList::End()
    {
        m_DrawList2D->End();

        UpdateStatistics();
        return;

        PreRender();

        m_RenderCommandBuffer->Begin();
        Renderer::BeginRenderPass(m_RenderCommandBuffer, m_GeometryRenderPass, { 0.0f, 0.0f, 0.0f, 1 });

        m_CubeMaterial->Set("u_MaterialTexture", m_ContainerDiffuse, 0);
        m_CubeMaterial->Set("u_MaterialTexture", m_ContainerSpecular, 1);

        // Mesh rendering
        for (auto& [mk, drawCommand] : m_DrawCommands)
        {
            auto& transformMap = m_MeshTransformMap.at(mk);
            Renderer::DrawStaticMesh(m_RenderCommandBuffer, drawCommand.Mesh, m_MeshTransformBuffer, m_UniformBufferSet, m_GeometryPipeline, transformMap.TransformOffset, drawCommand.SubmeshIndex, drawCommand.InstanceCount);
        }

        //Renderer::PushConstant(m_RenderCommandBuffer, m_CubePipeline, 64, &m_ViewProjection);
        //Renderer::DrawInstanced(m_RenderCommandBuffer, m_CubeVertexBuffer, m_CubeInstanceBuffer, m_CubeIndexBuffer, m_UniformBufferSet, m_GeometryPipeline, m_CubeShader, (u32)m_CubeInstances.size(), 36);
        Renderer::EndRenderPass(m_RenderCommandBuffer);

        m_RenderCommandBuffer->End();
        m_RenderCommandBuffer->Submit();

        // NOTE: Drawing with multiple renderpasses works
        //m_DrawList2D->End();
        m_DrawList2D->Begin();

        UpdateStatistics();
    }

    void SceneDrawList::PreRender()
    {
        // Submit point lights
        Renderer::Submit([this]()
        {
            m_UniformBufferSet->SetData(0, 2, &m_PointLights);
        });

        // Set whole transform buffer and then offset it in draw call
        std::vector<TransformData> transformData;

        u32 offset = 0;
        for (auto& [mk, transformMap] : m_MeshTransformMap)
        {
            transformMap.TransformOffset = offset;

            for (auto& transform : transformMap.Transforms)
            {
                transformData.push_back(transform);
                offset += (u32)sizeof(TransformData);
            }
        }

        m_MeshTransformBuffer->SetData(transformData.data(), transformData.size() * sizeof(TransformData));
    }

    void SceneDrawList::AddStaticMesh(Ref<StaticMesh> mesh, const glm::mat4& transform, i32 entity)
    {
        u32 submeshIndex = 0;
        for (auto& submesh : mesh->GetSubmeshes())
        {
            glm::mat4 submeshTransform = transform * submesh.Transform;

            MeshKey key = { mesh, submeshIndex };
            auto& drawCommand = m_DrawCommands[key];
            drawCommand.Mesh = mesh;
            drawCommand.InstanceCount++;
            drawCommand.SubmeshIndex = submeshIndex++;

            // If a mesh is a duplicate, draw it as a another instance of the original mesh but with different transform
            // Store those transforms in a map and then offset in PreRender 
            auto& meshTransformData = m_MeshTransformMap[key];

            auto& currentTransform = meshTransformData.Transforms.emplace_back();
            currentTransform.Tranform[0] = submeshTransform[0];
            currentTransform.Tranform[1] = submeshTransform[1];
            currentTransform.Tranform[2] = submeshTransform[2];
            currentTransform.Tranform[3] = submeshTransform[3];
        }
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

        return;
        // Static meshes 
        for (auto& [mk, drawCommand] : m_DrawCommands)
        {
            m_Statistics.Instances += drawCommand.InstanceCount;
            m_Statistics.DrawCalls++;
        }
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

    void SceneDrawList::AddSprite(const glm::mat4& transform, const glm::vec4& color, Ref<Texture2D> texture, f32 tiling, i32 entity)
    {
        m_DrawList2D->AddSprite(transform, color, texture, tiling, entity);
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
