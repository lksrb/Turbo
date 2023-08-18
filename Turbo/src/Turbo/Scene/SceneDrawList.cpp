#include "tbopch.h"
#include "SceneDrawList.h"

#include "Turbo/Core/Engine.h"
#include "Turbo/Scene/Scene.h"
#include "Turbo/Renderer/ShaderLibrary.h"

#include "Turbo/Platform/Vulkan/VulkanSwapChain.h"
#include "Turbo/Platform/Vulkan/VulkanRenderCommandBuffer.h"
#include "Turbo/Platform/Vulkan/VulkanGraphicsPipeline.h"
#include "Turbo/Platform/Vulkan/VulkanImage2D.h"
#include "Turbo/Platform/Vulkan/VulkanShader.h"

namespace Turbo {

    SceneDrawList::SceneDrawList(u32 width, u32 height)
    {
        m_Config.ViewportWidth = width;
        m_Config.ViewportHeight = height;

        Init();
    }

    SceneDrawList::~SceneDrawList()
    {
    }

    void SceneDrawList::Init()
    {
        // Separate command buffer
        m_RenderCommandBuffer = RenderCommandBuffer::Create();

        // Target Framebuffer
        // This will be the main framebuffer
        FrameBuffer::Config frameBufferConfig = {};
        frameBufferConfig.Attachments = { FrameBuffer::AttachmentType_Color, FrameBuffer::AttachmentType_SelectionBuffer, FrameBuffer::AttachmentType_Depth };
        frameBufferConfig.ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
        m_TargetFrameBuffer = FrameBuffer::Create(frameBufferConfig);
        // Main render pass
        RenderPass::Config config = {};
        config.TargetFrameBuffer = m_TargetFrameBuffer;
        config.ClearOnLoad = false;
        //config.SubPassCount = 1;
        m_FinalRenderPass = RenderPass::Create(config);
        m_FinalRenderPass->Invalidate();

        m_TargetFrameBuffer->SetRenderPass(m_FinalRenderPass);
        m_TargetFrameBuffer->Invalidate(m_Config.ViewportWidth, m_Config.ViewportHeight);

        // Create draw list for 2D
        m_DrawList2D = CreateOwned<DrawList2D>();
        m_DrawList2D->SetTargetRenderPass(m_FinalRenderPass);
        m_DrawList2D->Initialize();

        // NOTE: First pass should clear everything
        // Geometry pass
        {
            m_MeshTransformBuffer = VertexBuffer::Create(MaxTransforms * sizeof(TransformData));

            RenderPass::Config config = {};
            config.TargetFrameBuffer = m_TargetFrameBuffer;
            config.ClearOnLoad = true;
            m_GeometryRenderPass = RenderPass::Create(config);
            m_GeometryRenderPass->Invalidate();
            m_GeometryShader = ShaderLibrary::Get("StaticMesh");

            GraphicsPipeline::Config pipelineConfig = {};
            pipelineConfig.Renderpass = m_GeometryRenderPass;
            pipelineConfig.DepthTesting = true;
            pipelineConfig.Shader = m_GeometryShader;
            pipelineConfig.Layout = VertexBufferLayout
            {
                { AttributeType::Vec3, "a_VertexPosition" },
                { AttributeType::Vec3, "a_Normal" },
                { AttributeType::Vec2, "a_TexCoord" }
            };
            pipelineConfig.InstanceLayout = VertexBufferLayout
            {
                { AttributeType::Vec4, "a_TransformRow0" },
                { AttributeType::Vec4, "a_TransformRow1" },
                { AttributeType::Vec4, "a_TransformRow2" },
                { AttributeType::Vec4, "a_TransformRow3" },
                { AttributeType::Int, "a_EntityID" },
            };

            pipelineConfig.Topology = PrimitiveTopology::Triangle;
            m_GeometryPipeline = GraphicsPipeline::Create(pipelineConfig);
            m_GeometryPipeline->Invalidate();
        }

        // Create camera uniform buffer
        m_UniformBufferSet = UniformBufferSet::Create();
        m_UniformBufferSet->Create(0, 0, sizeof(UBCamera));
        m_UniformBufferSet->Create(0, 2, sizeof(LightEnvironment));

        m_ContainerDiffuse = Texture2D::Create("SandboxProject/Assets/Meshes/Backpack/1001_albedo.jpg");
        m_ContainerSpecular = Texture2D::Create("SandboxProject/Assets/Meshes/Backpack/1001_metallic.jpg");
        m_CubeMaterial = Material::Create({ m_GeometryShader });

        {
            m_SkyboxShader = ShaderLibrary::Get("Skybox");

            GraphicsPipeline::Config config = {};
            config.Renderpass = m_GeometryRenderPass;
            config.DepthTesting = true;
            config.Shader = m_SkyboxShader;
            config.Topology = PrimitiveTopology::Triangle;
            config.Layout = VertexBufferLayout
            {
                { { AttributeType::Vec3, "a_Position" } }
            };
            m_SkyboxPipeline = GraphicsPipeline::Create(config);
            m_SkyboxPipeline->Invalidate();

            m_SkyboxMaterial = Material::Create({ m_SkyboxShader });
            m_DefaultSkybox = TextureCube::Create();
            m_SkyboxMaterial->Set("u_TextureCube", m_DefaultSkybox);
        }
    }

    void SceneDrawList::SetSceneData(const SceneRendererData& data)
    {
        m_SceneRendererData = data;

        // NOTE: This is duplicated but keep it for now
        m_DrawList2D->SetSceneData(data);

        // u_Camera, will be on the set on 0 and bound on 0
        Renderer::Submit([this, data]()
        {
            UBCamera camera;
            camera.InversedViewMatrix = data.InversedViewMatrix;
            camera.ViewProjectionMatrix = data.ViewProjectionMatrix;
            camera.InversedViewProjectionMatrix = data.InversedViewProjectionMatrix;

            m_UniformBufferSet->SetData(0, 0, &camera);
        });
    }

    void SceneDrawList::Begin()
    {
        // Clear everything
        m_DrawCommands.clear();
        m_MeshTransformMap.clear();

        m_Statistics.Reset();

        m_LightEnvironment.PointLightCount = 0;
        m_LightEnvironment.SpotLightCount = 0;

        m_DrawList2D->Begin(); 
    }

    void SceneDrawList::End()
    {
        m_RenderCommandBuffer->Begin();

        // Vertex buffers need render command buffer to copy data into GPU
        PreRender();

        Renderer::BeginRenderPass(m_RenderCommandBuffer, m_GeometryRenderPass);

        Ref<Texture2D> whiteTexture = m_DrawList2D->GetWhiteTexture();

        m_CubeMaterial->Set("u_MaterialTexture", m_ContainerDiffuse, 0);
        m_CubeMaterial->Set("u_MaterialTexture", m_ContainerSpecular, 1);

        // Mesh rendering
        for (auto& [mk, drawCommand] : m_DrawCommands)
        {
            auto& transformMap = m_MeshTransformMap.at(mk);
            Renderer::DrawStaticMesh(m_RenderCommandBuffer, drawCommand.Mesh, m_MeshTransformBuffer, m_UniformBufferSet, m_GeometryPipeline, transformMap.TransformOffset, drawCommand.SubmeshIndex, drawCommand.InstanceCount);
        }

        // Render skybox
        // NOTE: Not sure if rendering it here is the most efficient thing
        Renderer::DrawSkybox(m_RenderCommandBuffer, m_SkyboxPipeline, m_UniformBufferSet);

        Renderer::EndRenderPass(m_RenderCommandBuffer);

        m_RenderCommandBuffer->End();
        m_RenderCommandBuffer->Submit();

        // Draw 2D on top of 3D
        // Depth buffer remains uncleared so we can easily figure out the depth to avoid overdrawing
        m_DrawList2D->End();

        UpdateStatistics();
    }

    void SceneDrawList::PreRender()
    {
        // Submit point lights
        Renderer::Submit([this]()
        {
            m_UniformBufferSet->SetData(0, 2, &m_LightEnvironment);
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

        m_MeshTransformBuffer->SetData(m_RenderCommandBuffer, transformData.data(), transformData.size() * sizeof(TransformData));
    }

    void SceneDrawList::AddStaticMesh(Ref<StaticMesh> mesh, const glm::mat4& transform, i32 entity)
    {
        const auto& submeshes = mesh->GetMeshSource()->GetSubmeshes();
        for (u32 submeshIndex : mesh->GetSubmeshIndices())
        {
            const auto& submesh = submeshes[submeshIndex];
            glm::mat4 submeshTransform = transform * submesh.Transform;

            MeshKey key = { mesh->Handle, submeshIndex };
            auto& drawCommand = m_DrawCommands[key];
            drawCommand.Mesh = mesh;
            drawCommand.InstanceCount++;
            drawCommand.SubmeshIndex = submeshIndex;

            // If a mesh is a duplicate, draw it as a another instance of the original mesh but with different transform
            // Store those transforms in a map and then offset them in PreRender 
            auto& meshTransformData = m_MeshTransformMap[key];

            auto& currentTransform = meshTransformData.Transforms.emplace_back();
            currentTransform.Tranform[0] = submeshTransform[0];
            currentTransform.Tranform[1] = submeshTransform[1];
            currentTransform.Tranform[2] = submeshTransform[2];
            currentTransform.Tranform[3] = submeshTransform[3];
            currentTransform.EntityID = entity;
        }
    }

    void SceneDrawList::AddPointLight(const glm::vec3& position, const glm::vec3& radiance, f32 intensity, f32 radius, f32 fallOff)
    {
        PointLight& pointLight = m_LightEnvironment.EmplacePointLight();
        pointLight.Position = glm::vec4(position, 1.0f);
        pointLight.Radiance = radiance;
        pointLight.Intensity = intensity;
        pointLight.Radius = radius;
        pointLight.FallOff = fallOff;
    }

    void SceneDrawList::AddSpotLight(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& radiance, f32 intensity, f32 innerCone, f32 outerCone)
    {
        SpotLight& spotLight = m_LightEnvironment.EmplaceSpotLight();
        spotLight.Position = glm::vec4(position, 1.0f);
        spotLight.Direction = glm::vec4(direction, 0.0f);
        spotLight.Radiance = radiance;
        spotLight.Intensity = intensity;
        spotLight.InnerCone = glm::cos(glm::radians(innerCone));
        spotLight.OuterCone = glm::cos(glm::radians(outerCone));
    }

    void SceneDrawList::AddBoxWireframe(const glm::mat4& transform, const glm::vec4& color, i32 entity)
    {
        std::array<glm::vec3, BoxWireframeVertices.size()> lineVertices;
        for (u32 i = 0; i < BoxWireframeVertices.size(); ++i)
        {
            lineVertices[i] = transform * BoxWireframeVertices[i];
        }

        // Front
        AddLine(lineVertices[0], lineVertices[1], color, entity);
        AddLine(lineVertices[1], lineVertices[2], color, entity);
        AddLine(lineVertices[2], lineVertices[3], color, entity);
        AddLine(lineVertices[3], lineVertices[0], color, entity);

        // Back
        AddLine(lineVertices[4], lineVertices[5], color, entity);
        AddLine(lineVertices[5], lineVertices[6], color, entity);
        AddLine(lineVertices[6], lineVertices[7], color, entity);
        AddLine(lineVertices[7], lineVertices[4], color, entity);

        // Lines between front and back
        AddLine(lineVertices[0], lineVertices[4], color, entity);
        AddLine(lineVertices[1], lineVertices[5], color, entity);
        AddLine(lineVertices[2], lineVertices[6], color, entity);
        AddLine(lineVertices[3], lineVertices[7], color, entity);
    }

    void SceneDrawList::UpdateStatistics()
    {
        m_Statistics.Statistics2D = m_DrawList2D->GetStatistics();

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

    void SceneDrawList::AddSprite(const glm::mat4& transform, const glm::vec4& color, Ref<Texture2D> texture, const std::array<glm::vec2, 4>& textureCoords, f32 tiling, i32 entity)
    {
        m_DrawList2D->AddSprite(transform, color, texture, textureCoords, tiling, entity);
    }

    void SceneDrawList::AddBillboardQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, Ref<Texture2D> texture, f32 tiling, i32 entity)
    {
        m_DrawList2D->AddBillboardQuad(position, size, color, texture, tiling, entity);
    }

    void SceneDrawList::AddLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color, i32 entity)
    {
        m_DrawList2D->AddLine(p0, p1, color, entity);
    }

    void SceneDrawList::AddCircle(const glm::mat4& transform, const glm::vec4& color, f32 thickness, f32 fade, i32 entity)
    {
        m_DrawList2D->AddCircle(transform, color, thickness, fade, entity);
    }

    void SceneDrawList::AddDebugCircle(const glm::vec3& position, const glm::vec3& rotation, f32 radius, const glm::vec4& color, i32 entity)
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::rotate(glm::mat4(1.0f), rotation.x, { 1.0f, 0.0f, 0.0f })
            * glm::rotate(glm::mat4(1.0f), rotation.y, { 0.0f, 1.0f, 0.0f })
            * glm::rotate(glm::mat4(1.0f), rotation.z, { 0.0f, 0.0f, 1.0f })
            * glm::scale(glm::mat4(1.0f), glm::vec3(radius));

        AddDebugCircle(transform, color, entity);
    }

    void SceneDrawList::AddDebugCircle(const glm::mat4& transform, const glm::vec4& color, i32 entity)
    {
        m_DrawList2D->AddDebugCircle(transform, color, entity);
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

    i32 SceneDrawList::ReadPixel(u32 x, u32 y)
    {
        return m_DrawList2D->ReadPixel(x, y);
    }

    void SceneDrawList::OnViewportResize(u32 width, u32 height)
    {
        m_Config.ViewportWidth = width;
        m_Config.ViewportHeight = height;

        Renderer::Submit([this, width, height]()
        {
            m_FinalRenderPass->GetConfig().TargetFrameBuffer->Invalidate(width, height);
            m_DrawList2D->OnViewportResize(width, height);
        });
    }

    Ref<Image2D> SceneDrawList::GetFinalImage() const
    {
        // Returns final image that is produced by the draw list
        return m_FinalRenderPass->GetConfig().TargetFrameBuffer->GetAttachment(FrameBuffer::AttachmentType_Color);
    }
}
