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

        Editor(const Application::Config& specification);
        ~Editor();
    protected: // Inherited
        void OnInitialize() override;
        void OnShutdown() override;
        void OnEvent(Event& event) override;
        void OnDrawUI() override;
        void OnUpdate() override;

        void OnDraw() override;
    private: // Events
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
        u32 m_ViewportWidth;
        u32 m_ViewportHeight;

        SceneRenderer* m_SceneRenderer;
        Mode m_EditorMode;
        Scene* m_CurrentScene;
        Project* m_CurrentProject;
        NewProjectModal m_NewProjectPopup;
        Filepath m_CurrentPath;
        AccessPanel m_CommandAccessPanel;
    };
}
