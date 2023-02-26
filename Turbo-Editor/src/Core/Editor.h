#pragma once

#include <Turbo.h>

#include "../Panels/AccessPanel.h"
#include "../Panels/NewProjectModal.h"

#include <glm/glm.hpp>

namespace Turbo::Ed
{
    class Editor : public Application
    {
    public:
        enum class Mode : u32
        {
            Edit = 0,
            Run = 1
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
        void OnInputSend(const FString256& input);
        bool OnCreateProject(const ProjectInfo& info);
        bool OnWindowClosed(WindowCloseEvent& e);
        bool OnWindowResized(WindowResizeEvent& e);
        bool OnKeyPressed(KeyPressedEvent& e);
    private: // Project
        bool OpenProject();
        void NewProject();

        void UpdateTitle();
    private:
        u32 m_ViewportWidth = 0, m_ViewportHeight = 0;

        Ref<SceneRenderer> m_SceneRenderer;
        Mode m_EditorMode = Mode::Edit;
        Ref<Scene> m_CurrentScene;
        Ref<Project> m_CurrentProject;
        NewProjectModal m_NewProjectPopup;
        Filepath m_CurrentPath;
        AccessPanel m_CommandAccessPanel;
    };
}
