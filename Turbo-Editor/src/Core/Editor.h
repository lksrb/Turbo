#pragma once

#include <Turbo.h>

namespace Turbo::Ed
{
    class Editor : public Application
    {
    public:
        enum class Mode : u32
        {
            Edit = 0,
            Play = 1
        };

        Editor(const Application::Config& config);
        ~Editor();
    protected: // Inherited
        void OnInitialize() override;
        void OnShutdown() override;
        void OnEvent(Event& event) override;
        void OnDrawUI() override;
        void OnUpdate() override;

        void OnDraw() override;
    private: // Events
        void OnViewportResize(u32 width, u32 height);

        bool OnKeyPressed(KeyPressedEvent& e);
        bool OnWindowClosed(WindowCloseEvent& e);
        bool OnMouseButtonPressed(MouseButtonPressedEvent& e);
    private:
        void OnScenePlay();
        void OnSceneStop();
    private: // Project
        void CreateProject(const std::filesystem::path& filepath);
        void SaveProject();
        void OpenProject(std::filesystem::path filepath = {});
    private: // Scene
        void SaveScene();
        void SaveSceneAs();
        void OpenScene(const std::filesystem::path& filepath = {});
    private: // Editor
        void UpdateWindowTitle();
        void Close();
    private:
        u32 m_ViewportWidth = 0, m_ViewportHeight = 0;

        i32 m_GizmoType = -1;
        glm::vec2 m_ViewportBounds[2] = {};

        EditorCamera m_EditorCamera;
        bool m_ShowPhysics2DColliders = false;
        bool m_ViewportHovered = false, m_ViewportFocused = false;
        Ref<Texture2D> m_PlayIcon, m_StopIcon;
        Ref<SceneRenderer> m_SceneRenderer;
        Mode m_EditorMode = Mode::Edit;
        Ref<Scene> m_EditorScene, m_RuntimeScene, m_CurrentScene;

        std::filesystem::path m_EditorScenePath;
        std::filesystem::path m_CurrentPath;

        Entity m_SelectedEntity, m_HoveredEntity;

        Ref<PanelManager> m_PanelManager;
    };
}
