#pragma once

#include "Turbo/Renderer/Camera.h"
#include "Turbo/Renderer/DrawList2D.h"

namespace Turbo
{
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

        // 2D
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
        void SetCamera(const Camera& camera);
    private:
        Ref<RenderCommandBuffer> m_RenderCommandBuffer;

        Ref<RenderPass> m_CompositeRenderPass;
        Ref<DrawList2D> m_DrawList2D;

        SceneDrawList::Statistics m_Statistics;
        SceneDrawList::Config m_Config;

        friend class Scene;
    };
}

