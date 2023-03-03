#pragma once

#include <Turbo.h>

#include "../Panels/AccessPanel.h"
#include "../Panels/NewProjectModal.h"
#include "../Panels/SceneHierarchyPanel.h"

#include <glm/glm.hpp>

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
        bool OnWindowClosed(WindowCloseEvent& e);
        bool OnWindowResized(WindowResizeEvent& e);
        bool OnKeyPressed(KeyPressedEvent& e);
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

        Ref<Texture2D> m_PlayIcon, m_StopIcon;
        Ref<SceneRenderer> m_SceneRenderer;
        Mode m_EditorMode = Mode::Edit;
        Ref<Scene> m_EditorScene, m_RuntimeScene;
        Ref<Project> m_CurrentProject;
        Filepath m_CurrentPath;

        SceneHierarchyPanel m_SceneHierarchyPanel;
        NewProjectModal m_NewProjectPopup;
        AccessPanel m_CommandAccessPanel;
    };
}
