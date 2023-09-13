#include "tbopch.h"
#include "SceneRenderer.h"

#include "Renderer2D.h"

#include <glm/gtc/matrix_transform.hpp>

#include <stdio.h>

namespace Turbo {

    SceneRenderer::SceneRenderer(u32 width, u32 height)
    {
        m_ViewportWidth = width;
        m_ViewportHeight = height;

        Init();
    }

    SceneRenderer::~SceneRenderer()
    {
        tdelete[] m_TransformVertexData;
        m_TransformVertexData = nullptr;
    }

    void SceneRenderer::Init()
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
        m_FinalPass = RenderPass::Create(config);
        m_FinalPass->Invalidate();

        m_TargetFrameBuffer->SetRenderPass(m_FinalPass);
        m_TargetFrameBuffer->Invalidate(m_ViewportWidth, m_ViewportHeight);

        // Create draw list for 2D
        m_Renderer2D = Owned<Renderer2D>::Create();
        m_Renderer2D->SetTargetRenderPass(m_FinalPass);
        m_Renderer2D->Initialize();

        // NOTE: First pass should clear everything
        // Geometry pass
        {
            m_MeshTransformBuffer = VertexBuffer::Create(MaxTransforms * sizeof(TransformVertexData));
            m_TransformVertexData = tnew TransformVertexData[MaxTransforms];

            RenderPass::Config config = {};
            config.TargetFrameBuffer = m_TargetFrameBuffer;
            config.ClearOnLoad = true;
            m_GeometryPass = RenderPass::Create(config);
            m_GeometryPass->Invalidate();

            Pipeline::Config pipelineConfig = {};
            pipelineConfig.Renderpass = m_GeometryPass;
            pipelineConfig.DepthTesting = true;
            pipelineConfig.Shader = ShaderLibrary::Get("StaticMesh");
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
            m_GeometryPipeline = Pipeline::Create(pipelineConfig);
            m_GeometryPipeline->Invalidate();
        }

        // Skybox
        {
            Pipeline::Config config = {};
            config.Renderpass = m_GeometryPass;
            config.DepthTesting = true;
            config.Shader = ShaderLibrary::Get("Skybox");
            config.Topology = PrimitiveTopology::Triangle;
            config.Layout = VertexBufferLayout
            {
                { AttributeType::Vec3, "a_Position" }
            };
            m_SkyboxPipeline = Pipeline::Create(config);
            m_SkyboxPipeline->Invalidate();

            m_SkyboxMaterial = Material::Create({ config.Shader });
            m_DefaultSkybox = TextureCube::Create();
            m_SkyboxMaterial->Set("u_TextureCube", m_DefaultSkybox);
        }

#if 0
        // Shadow Map
        {
            FrameBuffer::Config frameBufferConfig = {};
            frameBufferConfig.Attachments = { FrameBuffer::AttachmentType_Depth };
            frameBufferConfig.Width = m_TargetFrameBuffer->GetConfig().Width;
            frameBufferConfig.Height = m_TargetFrameBuffer->GetConfig().Height;

            m_ShadowPassFrameBuffer = FrameBuffer::Create(frameBufferConfig);

            RenderPass::Config config = {};
            config.TargetFrameBuffer = m_ShadowPassFrameBuffer;
            config.ClearOnLoad = false;
            m_ShadowPass = RenderPass::Create(config);
            m_ShadowPass->Invalidate();
        }
#endif

        // Create camera uniform buffer
        m_UniformBufferSet = UniformBufferSet::Create();
        m_UniformBufferSet->Create(0, 0, sizeof(UBCamera));
        m_UniformBufferSet->Create(0, 2, sizeof(LightEnvironment));

        m_ContainerDiffuse = Texture2D::Create("SandboxProject/Assets/Meshes/Backpack/1001_albedo.jpg");
        m_ContainerSpecular = Texture2D::Create("SandboxProject/Assets/Meshes/Backpack/1001_metallic.jpg");
        m_CubeMaterial = Material::Create({ ShaderLibrary::Get("StaticMesh") });
    }

    void SceneRenderer::SetSceneData(const SceneRendererData& data)
    {
        TBO_PROFILE_FUNC();

        m_SceneRendererData = data;

        // NOTE: This is duplicated but keep it for now
        m_Renderer2D->SetSceneData(data);

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

    void SceneRenderer::Begin()
    {
        TBO_PROFILE_FUNC();

        // Clear everything
        m_DrawCommands.clear();
        m_MeshTransformMap.clear();

        m_Statistics.Reset();

        m_LightEnvironment.Clear();

        m_Renderer2D->Begin();
    }

    void SceneRenderer::End()
    {
        TBO_PROFILE_FUNC();
        m_RenderCommandBuffer->Begin();

        // Vertex buffers need render command buffer to copy data into GPU
        PreRender();

        Renderer::BeginRenderPass(m_RenderCommandBuffer, m_GeometryPass);

        Ref<Texture2D> whiteTexture = Renderer::GetWhiteTexture();

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
        m_Renderer2D->End();

        UpdateStatistics();
    }

    void SceneRenderer::PreRender()
    {
        TBO_PROFILE_FUNC();

        // Submit point lights
        Renderer::Submit([this]()
        {
            m_UniformBufferSet->SetData(0, 2, &m_LightEnvironment);
        });

        // Map mesh transform map to a flat buffer
        u32 offset = 0;
        for (auto& [_, transformMap] : m_MeshTransformMap)
        {
            transformMap.TransformOffset = offset * sizeof(TransformVertexData);
            for (auto& transform : transformMap.Transforms)
            {
                m_TransformVertexData[offset] = transform;
                offset++;
            }
        }

        m_MeshTransformBuffer->SetData(m_RenderCommandBuffer, m_TransformVertexData, offset * sizeof(TransformVertexData));
    }

    void SceneRenderer::SubmitStaticMesh(Ref<StaticMesh> mesh, Ref<MaterialAsset> material, const glm::mat4& transform, i32 entity)
    {
        //material = material ? material : Renderer::GetWhiteMaterial();

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

    void SceneRenderer::SubmitDirectionalLight(const glm::vec3& direction, const glm::vec3& radiance, f32 intensity)
    {
        DirectionalLight& directionalLight = m_LightEnvironment.EmplaceDirectionalLight();
        directionalLight.Direction = glm::vec4(direction, 1.0);
        directionalLight.Radiance = radiance;
        directionalLight.Intensity = intensity;
    }

    void SceneRenderer::SubmitPointLight(const glm::vec3& position, const glm::vec3& radiance, f32 intensity, f32 radius, f32 fallOff)
    {
        PointLight& pointLight = m_LightEnvironment.EmplacePointLight();
        pointLight.Position = glm::vec4(position, 1.0f);
        pointLight.Radiance = radiance;
        pointLight.Intensity = intensity;
        pointLight.Radius = radius;
        pointLight.FallOff = fallOff;
    }

    void SceneRenderer::SubmitSpotLight(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& radiance, f32 intensity, f32 innerCone, f32 outerCone)
    {
        SpotLight& spotLight = m_LightEnvironment.EmplaceSpotLight();
        spotLight.Position = glm::vec4(position, 1.0f);
        spotLight.Direction = glm::vec4(direction, 0.0f);
        spotLight.Radiance = radiance;
        spotLight.Intensity = intensity;
        spotLight.InnerCone = glm::cos(glm::radians(innerCone));
        spotLight.OuterCone = glm::cos(glm::radians(outerCone));
    }

    void SceneRenderer::SubmitBoxWireframe(const glm::mat4& transform, const glm::vec4& color, i32 entity)
    {
        std::array<glm::vec3, BoxWireframeVertices.size()> lineVertices;
        for (u32 i = 0; i < BoxWireframeVertices.size(); ++i)
        {
            lineVertices[i] = transform * BoxWireframeVertices[i];
        }

        // Front
        SubmitLine(lineVertices[0], lineVertices[1], color, entity);
        SubmitLine(lineVertices[1], lineVertices[2], color, entity);
        SubmitLine(lineVertices[2], lineVertices[3], color, entity);
        SubmitLine(lineVertices[3], lineVertices[0], color, entity);

        // Back
        SubmitLine(lineVertices[4], lineVertices[5], color, entity);
        SubmitLine(lineVertices[5], lineVertices[6], color, entity);
        SubmitLine(lineVertices[6], lineVertices[7], color, entity);
        SubmitLine(lineVertices[7], lineVertices[4], color, entity);

        // Lines between front and back
        SubmitLine(lineVertices[0], lineVertices[4], color, entity);
        SubmitLine(lineVertices[1], lineVertices[5], color, entity);
        SubmitLine(lineVertices[2], lineVertices[6], color, entity);
        SubmitLine(lineVertices[3], lineVertices[7], color, entity);
    }

    void SceneRenderer::UpdateStatistics()
    {
        m_Statistics.Statistics2D = m_Renderer2D->GetStatistics();

        // Static meshes 
        for (auto& [mk, drawCommand] : m_DrawCommands)
        {
            m_Statistics.Instances += drawCommand.InstanceCount;
            m_Statistics.DrawCalls++;
        }
    }

    void SceneRenderer::SubmitQuad(const glm::vec3& position, const glm::vec2& size, f32 rotation, const glm::vec4& color, i32 entity)
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::rotate(glm::mat4(1.0f), glm::radians(rotation), { 0.0f, 0.0f, 1.0f })
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        SubmitQuad(transform, color, entity);
    }

    void SceneRenderer::SubmitQuad(const glm::mat4& transform, const glm::vec4& color, i32 entity)
    {
        m_Renderer2D->SubmitQuad(transform, color, entity);
    }

    void SceneRenderer::SubmitSprite(const glm::mat4& transform, const glm::vec4& color, Ref<Texture2D> texture, const std::array<glm::vec2, 4>& textureCoords, f32 tiling, i32 entity)
    {
        m_Renderer2D->SubmitSprite(transform, color, texture, textureCoords, tiling, entity);
    }

    void SceneRenderer::SubmitBillboardQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, Ref<Texture2D> texture, f32 tiling, i32 entity)
    {
        m_Renderer2D->SubmitBillboardQuad(position, size, color, texture, tiling, entity);
    }

    void SceneRenderer::SubmitLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color, i32 entity)
    {
        m_Renderer2D->SubmitLine(p0, p1, color, entity);
    }

    void SceneRenderer::SubmitCircle(const glm::mat4& transform, const glm::vec4& color, f32 thickness, f32 fade, i32 entity)
    {
        m_Renderer2D->SubmitCircle(transform, color, thickness, fade, entity);
    }

    void SceneRenderer::SubmitDebugCircle(const glm::vec3& position, const glm::vec3& rotation, f32 radius, const glm::vec4& color, i32 entity)
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::rotate(glm::mat4(1.0f), rotation.x, { 1.0f, 0.0f, 0.0f })
            * glm::rotate(glm::mat4(1.0f), rotation.y, { 0.0f, 1.0f, 0.0f })
            * glm::rotate(glm::mat4(1.0f), rotation.z, { 0.0f, 0.0f, 1.0f })
            * glm::scale(glm::mat4(1.0f), glm::vec3(radius));

        SubmitDebugCircle(transform, color, entity);
    }

    void SceneRenderer::SubmitDebugCircle(const glm::mat4& transform, const glm::vec4& color, i32 entity)
    {
        m_Renderer2D->SubmitDebugCircle(transform, color, entity);
    }

    void SceneRenderer::SubmitRect(const glm::vec3& position, const glm::vec2& size, f32 rotation, const glm::vec4& color, i32 entity)
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::rotate(glm::mat4(1.0f), glm::radians(rotation), { 0.0f, 0.0f, 1.0f })
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        SubmitRect(transform, color, entity);
    }

    void SceneRenderer::SubmitRect(const glm::mat4& transform, const glm::vec4& color, i32 entity)
    {
        m_Renderer2D->SubmitRect(transform, color, entity);
    }

    void SceneRenderer::SubmitString(const glm::mat4& transform, const glm::vec4& color, Ref<Font> font, const std::string& string, f32 kerningOffset, f32 lineSpacing, i32 entity)
    {
        m_Renderer2D->SubmitString(transform, color, font, string, kerningOffset, lineSpacing, entity);
    }

    i32 SceneRenderer::ReadPixel(u32 x, u32 y)
    {
        return m_Renderer2D->ReadPixel(x, y);
    }

    void SceneRenderer::OnViewportResize(u32 width, u32 height)
    {
        m_ViewportWidth = width;
        m_ViewportHeight = height;

        Renderer::Submit([this, width, height]()
        {
            m_FinalPass->GetConfig().TargetFrameBuffer->Invalidate(width, height);
            m_Renderer2D->OnViewportResize(width, height);
        });
    }

    Ref<Image2D> SceneRenderer::GetFinalImage() const
    {
        // Returns final image that is produced by the draw list
        return m_FinalPass->GetConfig().TargetFrameBuffer->GetAttachment(FrameBuffer::AttachmentType_Color);
    }
}
