#pragma once

#include "Turbo/Renderer/DrawList2D.h"

// Temporary
#include "Turbo/Renderer/RendererContext.h"

#include "Turbo/Renderer/Mesh.h"

#include <map>

namespace Turbo
{
    struct SceneRendererData
    {
        glm::mat4 ViewProjectionMatrix = glm::mat4(1.0f);
        glm::mat4 InversedViewMatrix = glm::mat4(1.0f);
    };

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
        void AddPointLight(const glm::vec3& position, f32 intensity = 1.0f, f32 radius = 10.0f, f32 fallOff = 1.0f, i32 entityID = -1);
        
        void AddQuad(const glm::vec3& position, const glm::vec2& size = { 1.0f, 1.0f }, f32 rotation = 0.0f, const glm::vec4& color = { 1.0f,1.0f, 1.0f, 1.0f }, i32 entity = -1);
        void AddQuad(const glm::mat4& transform, const glm::vec4& color, i32 entity = -1);
        void AddSprite(const glm::mat4& transform, const glm::vec4& color, Ref<Texture2D> texture, f32 tiling, i32 entity = -1);

        void AddLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color, i32 entity = -1);
        void AddCircle(const glm::mat4& transform, const glm::vec4& color, f32 thickness, f32 fade, i32 entity = -1);
        void AddRect(const glm::vec3& position, const glm::vec2& size = { 1.0f, 1.0f }, f32 rotation = 0.0f, const glm::vec4& color = { 1.0f,1.0f, 1.0f, 1.0f }, i32 entity = -1);
        void AddRect(const glm::mat4& transform, const glm::vec4& color = { 1.0f,1.0f, 1.0f, 1.0f }, i32 entity = -1);

        void AddString(const glm::mat4& transform, const glm::vec4& color, Ref<Font> font, const std::string& string, f32 kerningOffset = 0.0f, f32 lineSpacing = 0.0f, i32 entity = -1);

        void OnViewportResize(u32 width, u32 height);
        u32 GetViewportWidth() const { return m_Config.ViewportWidth; }
        u32 GetViewportHeight() const { return m_Config.ViewportHeight; }
        Ref<Image2D> GetFinalImage() const;

        SceneDrawList::Statistics GetStatistics() const { return m_Statistics; }
    private:
        void Init();
        void SetSceneData(const SceneRendererData& data);
        void UpdateStatistics();
        void PreRender();
    private:
        static constexpr u32 MaxCubes = 100;
        static constexpr u32 MaxCubeVertices = 24 * MaxCubes;
        static constexpr u32 MaxCubeIndices = 6 * MaxCubes;

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
        };

        // Match the layout in shader
        // Padding set to 16 because of std140 
        struct alignas(16) PointLight
        {
            glm::vec4 Position;

            f32 Intensity;
            f32 Radius;
            f32 FallOff;
        };

        // Match the layout in shader
        // Padding set to 16 because of std140 
        // FIXME: Padding fuckery, currently works for 32 but other numbers are not tested
        struct alignas(16) PointLightData
        {
            u32 Count = 0;
            PointLight PointLights[64];

            PointLight& operator[](u32 index)
            {
                TBO_ENGINE_ASSERT(index < 64);
                return PointLights[index];
            }
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

        PointLightData m_PointLights;

        Ref<Shader> m_GeometryShader;
        Ref<GraphicsPipeline> m_GeometryPipeline;
        Ref<RenderPass> m_GeometryRenderPass;

        Ref<RenderPass> m_FinalRenderPass;
        Ref<DrawList2D> m_DrawList2D;

        SceneDrawList::Statistics m_Statistics;
        SceneDrawList::Config m_Config;

        friend class Scene;
    };
}

