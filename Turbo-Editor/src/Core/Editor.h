#pragma once

#include <Turbo.h>

namespace Turbo::Ed {

    class Editor : public Application
    {
    public:
        enum class SceneMode : u32
        {
            Edit = 0,
            Play
        };

        enum class DevEnv : u32
        {
            None = 0,
            VS2022
        };

        Editor(const Application::Config& config);
        ~Editor();
    protected: // Inherited
        void OnInitialize() override;
        void OnShutdown() override;
        void OnEvent(Event& event) override;
        void OnDrawUI() override;
        void OnUpdate() override;
    private: // Events
        void OnViewportResize(u32 width, u32 height);

        bool OnKeyPressed(KeyPressedEvent& e);
        bool OnWindowClosed(WindowCloseEvent& e);
        bool OnWindowResize(WindowResizeEvent& e);
        bool OnMouseButtonPressed(MouseButtonPressedEvent& e);
    private:
        void OnScenePlay();
        void OnSceneStop();
    private: // Project
        void CreateProject(std::filesystem::path filepath);
        void SaveProject();
        void OpenProject(std::filesystem::path filepath = {});
    private: // Scene
        void NewScene();
        void SaveScene();
        void SaveSceneAs();
        void OpenScene(std::filesystem::path filepath = {});
    private: // Editor
        void OnOverlayRender();
        void UpdateWindowTitle();
        void Close();
    private:
        u32 m_ViewportWidth = 0, m_ViewportHeight = 0;
        i32 m_GizmoMode = 0;
        i32 m_GizmoType = -1;
        glm::vec2 m_ViewportBounds[2] = {};

        bool m_ShowDemoWindow = false;
        bool m_ShowAssetRegistryPanel = false;

        EditorCamera m_EditorCamera;
        bool m_ShowPhysicsColliders = false, m_ShowSceneIcons = true;
        bool m_ViewportHovered = false, m_ViewportFocused = false;
        Ref<Texture2D> m_PlayIcon, m_StopIcon, m_Reset2DIcon, m_CameraIcon,
            m_DirectionalLightIcon, m_PointLightIcon, m_SpotLightIcon;
        Ref<SceneDrawList> m_ViewportDrawList;
        SceneMode m_SceneMode = SceneMode::Edit;
        Ref<Scene> m_EditorScene, m_RuntimeScene, m_CurrentScene;

        std::filesystem::path m_EditorScenePath;
        std::filesystem::path m_CurrentPath;

        DevEnv m_CurrentIDE = DevEnv::VS2022;
        std::filesystem::path m_MSBuildPath;

        Entity m_SelectedEntity, m_HoveredEntity;

        Owned<PanelManager> m_PanelManager;
    };
}
