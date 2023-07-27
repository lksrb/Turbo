#pragma once

#include "Turbo/Renderer/DrawList2D.h"

// Temporary
#include "Turbo/Renderer/RendererContext.h"
#include "Turbo/Renderer/Mesh.h"

#include <map>

namespace Turbo
{
    struct SceneEnvironment
    {

    };

    class SceneDrawList
    {
    public:
        struct Config
        {
            u32 ViewportWidth;
            u32 ViewportHeight;
        };

        struct Statistics
        {
            DrawList2D::Statistics Statistics2D;

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

        SceneDrawList(const SceneDrawList::Config& config);
        ~SceneDrawList();

        void Begin();
        void End();

        void AddStaticMesh(Ref<StaticMesh> mesh, const glm::mat4& transform, i32 entity = -1);
        void AddPointLight(const glm::vec3& position, const glm::vec3& radiance, f32 intensity = 1.0f, f32 radius = 10.0f, f32 fallOff = 1.0f);
        void AddSpotLight(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& radiance, f32 intensity = 5.0f, f32 innerCone = 12.5f, f32 outerCone = 17.5f);
        
        void AddQuad(const glm::vec3& position, const glm::vec2& size = { 1.0f, 1.0f }, f32 rotation = 0.0f, const glm::vec4& color = { 1.0f,1.0f, 1.0f, 1.0f }, i32 entity = -1);
        void AddQuad(const glm::mat4& transform, const glm::vec4& color, i32 entity = -1);
        void AddSprite(const glm::mat4& transform, const glm::vec4& color, Ref<Texture2D> texture, const std::array<glm::vec2, 4>& textureCoords, f32 tiling, i32 entity = -1);
        void AddBillboardQuad(const glm::vec3& position, const glm::vec2& size = { 1.0f, 1.0f }, const glm::vec4& color = {1.0f, 1.0f, 1.0f, 1.0f}, Ref<Texture2D> texture = nullptr, f32 tiling = 1.0f, i32 entity = -1);

        void AddLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color, i32 entity = -1);
        void AddCircle(const glm::mat4& transform, const glm::vec4& color, f32 thickness, f32 fade, i32 entity = -1);
        void AddRect(const glm::vec3& position, const glm::vec2& size = { 1.0f, 1.0f }, f32 rotation = 0.0f, const glm::vec4& color = { 1.0f,1.0f, 1.0f, 1.0f }, i32 entity = -1);
        void AddRect(const glm::mat4& transform, const glm::vec4& color = { 1.0f, 1.0f, 1.0f, 1.0f }, i32 entity = -1);

        void AddString(const glm::mat4& transform, const glm::vec4& color, Ref<Font> font, const std::string& string, f32 kerningOffset = 0.0f, f32 lineSpacing = 0.0f, i32 entity = -1);

        i32 ReadPixel(u32 x, u32 y);

        void OnViewportResize(u32 width, u32 height);
        u32 GetViewportWidth() const { return m_Config.ViewportWidth; }
        u32 GetViewportHeight() const { return m_Config.ViewportHeight; }
        Ref<Image2D> GetFinalImage() const;

        void SetSceneData(const SceneRendererData& data);
        SceneDrawList::Statistics GetStatistics() const { return m_Statistics; }
    private:
        void Init();
        void UpdateStatistics();
        void PreRender();
    private:
        static constexpr u32 MaxCubes = 100;
        static constexpr u32 MaxCubeVertices = 24 * MaxCubes;
        static constexpr u32 MaxCubeIndices = 6 * MaxCubes;

        static constexpr u32 MaxPointLights = 64;
        static constexpr u32 MaxSpotLights = 64;

        // For now
        static constexpr u32 MaxTransforms = 4096;

        struct UBCamera
        {
            glm::mat4 ViewProjectionMatrix;
            glm::mat4 InversedViewMatrix;
        };

        struct TransformData
        {
            glm::vec4 Tranform[4];
            i32 EntityID;
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
            PointLight PointLights[MaxPointLights];
            u32 PointLightCount = 0;
            SpotLight SpotLights[MaxSpotLights];
            u32 SpotLightCount = 0;
            
            inline PointLight& EmplacePointLight() { TBO_ENGINE_ASSERT(PointLightCount < MaxPointLights); return PointLights[PointLightCount++]; }
            inline SpotLight& EmplaceSpotLight() { TBO_ENGINE_ASSERT(SpotLightCount < MaxSpotLights); return SpotLights[SpotLightCount++]; }
        };

        struct DirectionalLight
        {
            glm::vec3 Direction;
        };

        // Uniquely describes a mesh
        // We can recycle meshes when they have same mesh and material
        // by adding another instance of the mesh
        struct MeshKey
        {
            Ref<StaticMesh> Mesh; // TODO: Assets
            u32 SubmeshIndex = 0;

            bool operator<(const MeshKey& other) const
            {
                if ((size_t)Mesh.Get() < (size_t)other.Mesh.Get())
                    return true;

                return (size_t)Mesh.Get() == (size_t)other.Mesh.Get() && SubmeshIndex < other.SubmeshIndex;
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
            std::vector<TransformData> Transforms;
            u32 TransformOffset;
        };

        Ref<Material> m_CubeMaterial;
        Ref<Texture2D> m_ContainerDiffuse, m_ContainerSpecular;

        // Static meshes
        std::map<MeshKey, MeshTransformMap> m_MeshTransformMap;
        std::map<MeshKey, DrawCommand> m_DrawCommands;
        Ref<VertexBuffer> m_MeshTransformBuffer;

        Ref<RenderCommandBuffer> m_RenderCommandBuffer;
        Ref<UniformBufferSet> m_UniformBufferSet;

        SceneRendererData m_SceneRendererData;

        LightEnvironment m_LightEnvironment;

        Ref<Shader> m_GeometryShader;
        Ref<GraphicsPipeline> m_GeometryPipeline;
        Ref<RenderPass> m_GeometryRenderPass;

        Ref<RenderPass> m_FinalRenderPass;
        Ref<DrawList2D> m_DrawList2D;
        Ref<FrameBuffer> m_TargetFrameBuffer;

        SceneDrawList::Statistics m_Statistics;
        SceneDrawList::Config m_Config;
    };
}

