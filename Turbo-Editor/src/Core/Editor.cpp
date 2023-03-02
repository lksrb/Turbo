#include "Editor.h"

#include "Turbo/Core/Platform.h"

#include "Turbo/Solution/Project.h"
#include "Turbo/Benchmark/ScopeTimer.h"

#include "Turbo/UI/UI.h"

namespace Turbo::Ed
{
    Editor::Editor(const Application::Config& config)
        : Application(config)
    {
    }

    Editor::~Editor()
    {
    }

    void Editor::OnInitialize()
    {
        m_CurrentPath = Platform::GetCurrentPath();

        m_CommandAccessPanel.SetOnInputSendCallback(TBO_BIND_FN(Editor::OnInputSend));
        m_NewProjectPopup.SetCallback(TBO_BIND_FN(Editor::OnCreateProject));

        SceneRenderer::Config sceneRendererConfig = {};
        sceneRendererConfig.ViewportWidth = m_Config.Width;
        sceneRendererConfig.ViewportHeight= m_Config.Height;
        sceneRendererConfig.RenderIntoTexture = true; // Render it into a texture that will be displayed via imgui
        m_SceneRenderer = Ref<SceneRenderer>::Create(sceneRendererConfig);
    }

    void Editor::OnShutdown()
    {
        //m_CurrentProject.Reset();
    }

    void Editor::OnUpdate()
    {
        if (m_CurrentScene == nullptr)
            return;

        switch (m_EditorMode)
        {
            case Mode::Edit:
            {
                m_CurrentScene->OnEditorUpdate(Time.DeltaTime);
                break;
            }
            case Mode::Run:
            {
                m_CurrentScene->OnRuntimeUpdate(Time.DeltaTime);
                break;
            }
        }
    }

    void Editor::OnDraw()
    {
        if (m_CurrentScene == nullptr)
            return;

        switch (m_EditorMode)
        {
            case Mode::Edit:
            {
                m_CurrentScene->OnEditorRender(m_SceneRenderer);
                break;
            }
            case Mode::Run:
            {
                m_CurrentScene->OnRuntimeRender(m_SceneRenderer);
                break;
            }
        }
    }

    // --- Callbacks from imgui windows ---
    bool Editor::OnCreateProject(const ProjectInfo& info)
    {
        // Unload project
        m_CurrentProject.Reset();

        // Create new default project
        m_CurrentProject = Project::CreateDefault(info.RootDirectory, info.Name);

        // Set render scene
        m_CurrentScene = m_CurrentProject->GetStartupScene();

        // Resize scene
        m_CurrentScene->SetViewportSize(m_ViewportWidth, m_ViewportHeight);

        // Start scene

        UpdateTitle();

        return true;
    }

    void Editor::OnInputSend(const FString256& input)
    {
        TBO_INFO(input.CStr());
    }

    bool Editor::OnWindowClosed(WindowCloseEvent& e)
    {
        return true;
    }

    bool Editor::OnWindowResized(WindowResizeEvent& e)
    {
        //OnViewportResize(e.GetWidth(), e.GetHeight());

        return true;
    }

    bool Editor::OnKeyPressed(KeyPressedEvent& e)
    {
        //if (e.GetRepeatCount())
        //    return false;

        bool control = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
        bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);
        bool alt = Input::IsKeyPressed(Key::LeftAlt) || Input::IsKeyPressed(Key::RightAlt);

        switch (e.GetKeyCode())
        {
            case Key::X:
            {
                // Open Access Window 
                if (alt)
                {
                    m_CommandAccessPanel.Open(!m_CommandAccessPanel.IsOpened());
                }
                break;
            }
            case Key::Escape:
            {
                if (m_CommandAccessPanel.IsOpened())
                    m_CommandAccessPanel.Open(false);
                break;
            }
        }

        return true;
    }

    void Editor::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(TBO_BIND_FN(Editor::OnWindowClosed));
        dispatcher.Dispatch<WindowResizeEvent>(TBO_BIND_FN(Editor::OnWindowResized));
        dispatcher.Dispatch<KeyPressedEvent>(TBO_BIND_FN(Editor::OnKeyPressed));
    }

    static FString32 s_ModeText = "Run";

    void Editor::OnDrawUI()
    {
        // Dockspace
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_MenuBar;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
        // and handle the pass-thru hole, so we ask Begin() to not render a background.
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // all active windows docked into it will lose their parent and become undocked.
        // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
        // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace Demo", nullptr, window_flags);
        ImGui::PopStyleVar();

        ImGui::PopStyleVar(2);

        // Submit the DockSpace
        ImGuiIO& io = ImGui::GetIO();
        ImGuiStyle& style = ImGui::GetStyle();

        float minWinSizeX = style.WindowMinSize.x;
        style.WindowMinSize.x = 370.0f;
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("VulkanAppDockspace");

            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }
        style.WindowMinSize.x = minWinSizeX;
        // --Dockspace

        // Menu
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New Project..."))
                {
                    NewProject();
                }
                if (ImGui::MenuItem("Open Project..."))
                {
                    OpenProject();
                }
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        {
            ImGui::Begin("Viewport");

            auto& window_size = ImGui::GetWindowSize();

            ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

            if (m_ViewportWidth != window_size.x || m_ViewportHeight != window_size.y)
            {
                OnViewportResize(static_cast<u32>(window_size.x), static_cast<u32>(window_size.y));
            }

            if (m_CurrentScene && m_CurrentScene->Renderable())
            {
                UI::Image(m_SceneRenderer->GetFinalImage(), { viewportPanelSize.x,viewportPanelSize.y }, { 0, 1 }, { 1, 0 });
            }

            // Right-click on viewport to access the magic menu
            ImGui::SetNextWindowSize({ 200.0f, 400.0f });
            if (ImGui::BeginPopupContextWindow("##ViewportContextMenu"))
            {
                ImGui::MenuItem("Scene", nullptr, false, false);
                ImGui::Separator();
                if (ImGui::MenuItem("New Entity", nullptr, false, m_CurrentScene))
                {
                    TBO_ENGINE_WARN("Entity Added!");
                    Entity e = m_CurrentScene->CreateEntity();

                    auto& transform = e.Transform();

                    static f32 x = 0.0f;
                    transform.Translation.y = x;
                    ++x;
                    auto& src = e.AddComponent<SpriteRendererComponent>();
                }

                if (ImGui::MenuItem("New Camera Entity", nullptr, false, m_CurrentScene))
                {
                    TBO_ENGINE_WARN("Camera Added!");
                    auto& camera = m_CurrentScene->CreateEntity().AddComponent<CameraComponent>();
                    camera.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
                }
                ImGui::EndPopup();
            }

            ImGui::End();
        }

        ImGui::Begin("Statistics & Renderer2D");
        ImGui::Text("Timestep %.5f ms", Time.DeltaTime.ms());
        ImGui::Text("StartTime %.5f ms", Time.TimeSinceStart.ms());
        ImGui::Separator();

        Renderer2D::Statistics stats = m_SceneRenderer->GetRenderer2D()->GetStatistics();
        ImGui::Text("Quad Indices %d", stats.QuadIndexCount);
        ImGui::Text("Quad Count %d", stats.QuadCount);
        ImGui::Text("Drawcalls %d", stats.DrawCalls);
        ImGui::End();

        ImGui::ShowDemoWindow();

        // Edit/Play bar
        {
            ImGui::Begin("Editor Console");
            if (ImGui::Button(s_ModeText.CStr()))
            {
                m_EditorMode = m_EditorMode == Mode::Edit ? Mode::Run : Mode::Edit;
                s_ModeText = m_EditorMode == Mode::Edit ? "Run" : "Edit";
            }
            ImGui::End();
        }

        m_CommandAccessPanel.OnUIRender();

        m_NewProjectPopup.OnUIRender();

        // End dockspace
        ImGui::End();
    }

    void Editor::NewProject()
    {
        m_NewProjectPopup.Open();
    }

    bool Editor::OpenProject()
    {
        // Opens platform specific 
        Filepath& projectFilepath = Platform::OpenFileDialog("Open Project", "Turbo Project(*.tproject)\0 * .tproject\0");

        if (m_CurrentProject)
        {
            Filepath configFile = m_CurrentProject->GetRootDirectory();
            configFile /= m_CurrentProject->GetName();
            configFile.Append(".tproject");

            if (configFile == projectFilepath)
            {
                TBO_WARN("Project is already loaded!");
                return false;
            }

            // Unload project
            m_CurrentProject.Reset();
        }

        if (projectFilepath.Extension() == ".tproject")
        {
            m_CurrentProject = Project::Deserialize(projectFilepath);

            // Set render scene
            m_CurrentScene = m_CurrentProject->GetStartupScene();

            UpdateTitle();

            return true;
        }


        return false;
    }

    void Editor::UpdateTitle()
    {
        TBO_ASSERT(m_CurrentProject);

        Window->SetTitle("TurboEditor");

        FString64 previousWindowTitle = Window->GetTitle();

        previousWindowTitle.Append(" | ");
        previousWindowTitle.Append(m_CurrentProject->GetName());

        FString64 defaultSceneName = m_CurrentProject->GetStartupScene() ? m_CurrentProject->GetStartupScene()->GetName() : "Empty";
        previousWindowTitle.Append(" - ");
        previousWindowTitle.Append(defaultSceneName);

        // Set new title
        Window->SetTitle(previousWindowTitle);
    }

    void Editor::OnViewportResize(u32 width, u32 height)
    {
        m_ViewportWidth = width;
        m_ViewportHeight = height;

        if (m_CurrentScene)
            m_CurrentScene->SetViewportSize(m_ViewportWidth, m_ViewportHeight);
    }

}
