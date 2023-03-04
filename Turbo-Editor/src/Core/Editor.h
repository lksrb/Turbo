#pragma once

#include <Turbo.h>

#include "../Panels/AccessPanel.h"
#include "../Panels/NewProjectModal.h"
#include "../Panels/SceneHierarchyPanel.h"

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
        void OnInputSend(const String& input);
        bool OnCreateProject(const ProjectInfo& info);

        bool OnKeyPressed(KeyPressedEvent& e);
        bool OnMouseButtonPressed(MouseButtonPressedEvent& e);
    private:
        void OnScenePlay();
        void OnSceneStop();
    private: // Project
        bool OpenProject(const Filepath& filepath = {});
        void NewProject();

        void SaveProject();
        void SaveScene();
        void SaveSceneAs();

        void UpdateWindowTitle();
    private:
        u32 m_ViewportWidth = 0, m_ViewportHeight = 0;

        i32 m_GizmoType = -1;
        glm::vec2 m_ViewportBounds[2];

        EditorCamera m_EditorCamera;

        bool m_ViewportHovered = false, m_ViewportFocused = false;
        Ref<Texture2D> m_PlayIcon, m_StopIcon;
        Ref<SceneRenderer> m_SceneRenderer;
        Mode m_EditorMode = Mode::Edit;
        Ref<Scene> m_EditorScene, m_RuntimeScene;
        Ref<Project> m_CurrentProject;
        Filepath m_CurrentPath;

        Entity m_SelectedEntity, m_HoveredEntity;

        SceneHierarchyPanel m_SceneHierarchyPanel;
        NewProjectModal m_NewProjectPopup;
        AccessPanel m_CommandAccessPanel;
    };
}
