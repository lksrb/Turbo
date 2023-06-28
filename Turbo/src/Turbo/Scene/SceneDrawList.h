#pragma once

#include "Turbo/Renderer/DrawList2D.h"

// Temporary
#include "Turbo/Renderer/RendererContext.h"

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
        };

        SceneDrawList(const SceneDrawList::Config& config);
        ~SceneDrawList();

        void Begin();
        void End();

        void AddCube(const glm::mat4& transform, const glm::vec4& color = { 1.0f,1.0f, 1.0f, 1.0f }, i32 entity = -1);
        void AddPointLight(const glm::vec3& position, f32 intensity = 1.0f, f32 radius = 10.0f, f32 fallOff = 1.0f, i32 entityID = -1);
        
        void AddQuad(const glm::vec3& position, const glm::vec2& size = { 1.0f, 1.0f }, f32 rotation = 0.0f, const glm::vec4& color = { 1.0f,1.0f, 1.0f, 1.0f }, i32 entity = -1);
        void AddQuad(const glm::mat4& transform, const glm::vec4& color, i32 entity = -1);
        void AddSprite(const glm::mat4& transform, const glm::vec4& color, Ref<SubTexture2D> subTexture, f32 tiling, i32 entity = -1);

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
    private:
        static constexpr u32 MaxCubes = 100;
        static constexpr u32 MaxCubeVertices = 24 * MaxCubes;
        static constexpr u32 MaxCubeIndices = 6 * MaxCubes;

        struct UBCamera
        {
            glm::mat4 ViewProjectionMatrix;
            glm::mat4 InversedViewMatrix;
        };

        struct CubeVertex
        {
            glm::vec3 Position;
            glm::vec3 Normal;
            glm::vec2 TexCoord;
        };

        struct CubeInstance
        {
            // We cannot send mat4, there is not an appropriate VkFormat for it
            glm::vec4 Tranform[4];
            glm::vec4 Color;
            i32 EntityID;
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

        Ref<Material> m_CubeMaterial;
        Ref<Texture2D> m_ContainerDiffuse, m_ContainerSpecular;
        std::vector<CubeInstance> m_CubeInstances;

        Ref<RenderCommandBuffer> m_RenderCommandBuffer;
        Ref<UniformBufferSet> m_UniformBufferSet;

        SceneRendererData m_SceneRendererData;

        PointLightData m_PointLights;

        // Cubes for now
        Ref<VertexBuffer> m_CubeInstanceBuffer;
        Ref<VertexBuffer> m_CubeVertexBuffer;
        Ref<IndexBuffer> m_CubeIndexBuffer;

        Ref<Shader> m_CubeShader;
        Ref<GraphicsPipeline> m_CubePipeline;
        Ref<RenderPass> m_CubeRenderPass;

        Ref<RenderPass> m_FinalRenderPass;
        Ref<DrawList2D> m_DrawList2D;

        SceneDrawList::Statistics m_Statistics;
        SceneDrawList::Config m_Config;

        friend class Scene;
    };
}

