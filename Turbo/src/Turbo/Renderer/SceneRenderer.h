#pragma once

#include "Turbo/Asset/Asset.h"
#include "Turbo/Core/Owned.h"

#include <map>

namespace Turbo {

    class Font;
    class RenderCommandBuffer;
    class RendererBuffer;
    class Shader;
    class Pipeline;
    class Image2D;
    class Material;
    class Texture2D;
    class RenderPass;
    class FrameBuffer;
    class UniformBufferSet;
    class VertexBuffer;
    class IndexBuffer;
    class MaterialAsset;
    class StaticMesh;
    class TextureCube;
    class Renderer2D;

    struct SceneRendererData
    {
        glm::mat4 ViewProjectionMatrix = glm::mat4(1.0f);
        glm::mat4 InversedViewProjectionMatrix = glm::mat4(1.0f);
        glm::mat4 InversedViewMatrix = glm::mat4(1.0f);
        glm::mat4 ViewMatrix = glm::mat4(1.0f);
    };

    // TODO: Merge statistics
    struct Renderer2DStatistics
    {
        u32 QuadCount;
        u32 CircleCount;
        u32 CircleIndexCount;
        u32 DrawCalls;

        Renderer2DStatistics() { Reset(); }

        void Reset()
        {
            std::memset(this, 0, sizeof(*this));
        }
    };

    class SceneRenderer
    {
    public:
        struct Statistics
        {
            Renderer2DStatistics Statistics2D;

            u32 DrawCalls;
            u32 Instances;
            u32 Vertices;
            u32 Indices;

            Statistics()
            {
                Reset();
            }

            void Reset()
            {
                std::memset(this, 0, sizeof(*this));
            }
        };

        SceneRenderer(u32 width, u32 height);
        ~SceneRenderer();

        void Begin();
        void End();

        void SubmitStaticMesh(Ref<StaticMesh> mesh, Ref<MaterialAsset> material, const glm::mat4& transform, i32 entity = -1);
        void SubmitDirectionalLight(const glm::vec3& direction, const glm::vec3& radiance, f32 intensity = 1.0f);
        void SubmitPointLight(const glm::vec3& position, const glm::vec3& radiance, f32 intensity = 1.0f, f32 radius = 10.0f, f32 fallOff = 1.0f);
        void SubmitSpotLight(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& radiance, f32 intensity = 5.0f, f32 innerCone = 12.5f, f32 outerCone = 17.5f);

        void SubmitBoxWireframe(const glm::mat4& transform, const glm::vec4& color = glm::vec4(1.0f), i32 entity = -1);

        void SubmitQuad(const glm::vec3& position, const glm::vec2& size = { 1.0f, 1.0f }, f32 rotation = 0.0f, const glm::vec4& color = glm::vec4(1.0f), i32 entity = -1);
        void SubmitQuad(const glm::mat4& transform, const glm::vec4& color, i32 entity = -1);
        void SubmitSprite(const glm::mat4& transform, const glm::vec4& color, Ref<Texture2D> texture, const std::array<glm::vec2, 4>& textureCoords, f32 tiling, i32 entity = -1);
        void SubmitBillboardQuad(const glm::vec3& position, const glm::vec2& size = { 1.0f, 1.0f }, const glm::vec4& color = glm::vec4(1.0f), Ref<Texture2D> texture = nullptr, f32 tiling = 1.0f, i32 entity = -1);

        void SubmitLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color, i32 entity = -1);
        void SubmitCircle(const glm::mat4& transform, const glm::vec4& color, f32 thickness, f32 fade, i32 entity = -1);
        void SubmitDebugCircle(const glm::vec3& position, const glm::vec3& rotation, f32 radius = 0.5f, const glm::vec4& color = glm::vec4(1.0f), i32 entity = -1);
        void SubmitDebugCircle(const glm::mat4& transform, const glm::vec4& color = glm::vec4(1.0f), i32 entity = -1);
        void SubmitRect(const glm::vec3& position, const glm::vec2& size = { 1.0f, 1.0f }, f32 rotation = 0.0f, const glm::vec4& color = { 1.0f,1.0f, 1.0f, 1.0f }, i32 entity = -1);
        void SubmitRect(const glm::mat4& transform, const glm::vec4& color = glm::vec4(1.0f), i32 entity = -1);

        void SubmitString(const glm::mat4& transform, const glm::vec4& color, Ref<Font> font, const std::string& string, f32 kerningOffset = 0.0f, f32 lineSpacing = 0.0f, i32 entity = -1);

        i32 ReadPixel(u32 x, u32 y);

        void OnViewportResize(u32 width, u32 height);
        u32 GetViewportWidth() const { return m_ViewportWidth; }
        u32 GetViewportHeight() const { return m_ViewportHeight; }
        Ref<Image2D> GetFinalImage() const;

        void SetSceneData(const SceneRendererData& data);
        SceneRenderer::Statistics GetStatistics() const { return m_Statistics; }
    private:
        void Init();
        void UpdateStatistics();
        void PreRender();
    private:
        // For now
        static constexpr u32 MaxTransforms = 4096;

        static constexpr std::array<glm::vec4, 8> BoxWireframeVertices = {
            // Front face
            glm::vec4(-0.5f, -0.5f, 0.5f, 1.0f),
            glm::vec4(0.5f, -0.5f, 0.5f, 1.0f),
            glm::vec4(0.5f, 0.5f, 0.5f, 1.0f),
            glm::vec4(-0.5f, 0.5f, 0.5f, 1.0f),

            // Back face
            glm::vec4(-0.5f, -0.5f, -0.5f, 1.0f),
            glm::vec4(0.5f, -0.5f, -0.5f, 1.0f),
            glm::vec4(0.5f, 0.5f, -0.5f, 1.0f),
            glm::vec4(-0.5f, 0.5f, -0.5f, 1.0f),
        };

        struct UBCamera
        {
            glm::mat4 ViewProjectionMatrix;
            glm::mat4 InversedViewProjectionMatrix;
            glm::mat4 InversedViewMatrix;
        };

        struct TransformVertexData
        {
            glm::vec4 Tranform[4];
            i32 EntityID;
        };

        struct alignas(16) DirectionalLight
        {
            glm::vec4 Direction;
            glm::vec3 Radiance;
            f32 Intensity;
        };

        // Match the layout in shader
        // Padding set to 16 because of std140 
        struct alignas(16) PointLight
        {
            glm::vec4 Position;
            glm::vec3 Radiance;

            f32 Intensity;
            f32 Radius;
            f32 FallOff;
        };

        struct alignas(16) SpotLight
        {
            glm::vec4 Position;
            glm::vec4 Direction;
            glm::vec3 Radiance;

            f32 Intensity;
            f32 InnerCone;
            f32 OuterCone;
        };

        // Match the layout in shader
        // Padding set to 16 because of std140 
        // FIXME: Padding fuckery, currently works for 64 but other numbers are not tested
        struct alignas(16) LightEnvironment
        {
            static constexpr u32 MaxDirectionalLights = 64;
            static constexpr u32 MaxPointLights = 64;
            static constexpr u32 MaxSpotLights = 64;

            DirectionalLight DirectionalLights[MaxDirectionalLights];
            u32 DirectionalLightCount = 0;

            PointLight PointLights[MaxPointLights];
            u32 PointLightCount = 0;

            SpotLight SpotLights[MaxSpotLights];
            u32 SpotLightCount = 0;

            inline void Clear() { DirectionalLightCount = PointLightCount = SpotLightCount = 0; };

            inline DirectionalLight& EmplaceDirectionalLight() { TBO_ENGINE_ASSERT(DirectionalLightCount < MaxDirectionalLights); return DirectionalLights[DirectionalLightCount++]; }

            inline PointLight& EmplacePointLight() { TBO_ENGINE_ASSERT(PointLightCount < MaxPointLights); return PointLights[PointLightCount++]; }
            inline SpotLight& EmplaceSpotLight() { TBO_ENGINE_ASSERT(SpotLightCount < MaxSpotLights); return SpotLights[SpotLightCount++]; }
        };

        // Uniquely describes a mesh
        // We can recycle meshes when they have same mesh and material
        // by adding another instance of the mesh
        struct MeshKey
        {
            AssetHandle MeshHandle;
            u32 SubmeshIndex = 0;

            bool operator<(const MeshKey& other) const
            {
                if (MeshHandle < other.MeshHandle)
                    return true;

                return MeshHandle == other.MeshHandle && SubmeshIndex < other.SubmeshIndex;
            }
        };

        struct DrawCommand
        {
            Ref<StaticMesh> Mesh;
            u32 SubmeshIndex = 0;
            u32 InstanceCount = 0;
        };

        struct MeshTransformMap
        {
            std::vector<TransformVertexData> Transforms;
            u32 TransformOffset;
        };

        Ref<Material> m_CubeMaterial;
        Ref<Texture2D> m_ContainerDiffuse, m_ContainerSpecular;

        // Static meshes
        std::map<MeshKey, MeshTransformMap> m_MeshTransformMap;
        std::map<MeshKey, DrawCommand> m_DrawCommands;
        Ref<VertexBuffer> m_MeshTransformBuffer;
        TransformVertexData* m_TransformVertexData = nullptr;

        Ref<RenderCommandBuffer> m_RenderCommandBuffer;
        Ref<UniformBufferSet> m_UniformBufferSet;

        SceneRendererData m_SceneRendererData;

        LightEnvironment m_LightEnvironment;

        // Skybox
        Ref<Material> m_SkyboxMaterial;
        Ref<Pipeline> m_SkyboxPipeline;
        Ref<TextureCube> m_DefaultSkybox;

        // Shadows
        Ref<RenderPass> m_ShadowPass;
        Ref<Pipeline> m_ShadowPipeline;
        Ref<FrameBuffer> m_ShadowPassFrameBuffer;
        
        // Geometry
        Ref<Pipeline> m_GeometryPipeline;
        Ref<RenderPass> m_GeometryPass;

        Ref<RenderPass> m_FinalPass;
        Ref<FrameBuffer> m_TargetFrameBuffer;

        Owned<Renderer2D> m_Renderer2D;

        SceneRenderer::Statistics m_Statistics;

        u32 m_ViewportWidth = 0, m_ViewportHeight = 0;
    };
}

