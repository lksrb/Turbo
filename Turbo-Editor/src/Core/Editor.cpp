#include "Editor.h"

#include "Turbo/Core/Platform.h"

#include "Turbo/Solution/Project.h"
#include "Turbo/Benchmark/ScopeTimer.h"

#include <imgui.h>

namespace Turbo::Ed
{
    Editor::Editor(const Application::Config& specification)
        : Application(specification), m_CurrentProject(nullptr), m_CurrentScene(nullptr), m_SceneRenderer(nullptr),
        m_EditorMode(Mode::Run), m_ViewportWidth(0), m_ViewportHeight(0)
    {
    }

    Editor::~Editor()
    {
    }

    static Ptr<Texture2D> t;

    void Editor::OnInitialize()
    {
        m_CurrentPath = Platform::GetCurrentPath();

        m_CommandAccessPanel.SetOnInputSendCallback(TBO_BIND_FN(Editor::OnInputSend));
        m_NewProjectPopup.SetCallback(TBO_BIND_FN(Editor::OnCreateProject));

        m_SceneRenderer = new SceneRenderer;

        t = Texture2D::Create({ "Resources/test.png" });
    }

    void Editor::OnShutdown()
    {
        delete t;

        delete m_CurrentProject;
        m_CurrentProject = nullptr;

        delete m_SceneRenderer;
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
        delete m_CurrentProject;
        m_CurrentProject = nullptr;

        // Create new project
        m_CurrentProject = Project::CreateDefault(info.RootDirectory, info.Name);

        // Set render scene
        m_CurrentScene = m_CurrentProject->GetDefaultScene();

        // Resize scene
        m_CurrentScene->OnViewportResize(m_ViewportWidth, m_ViewportHeight);

        UpdateTitle();

        return true;
    }

    void Editor::OnInputSend(const FString256& input)
    {
        TBO_INFO(input.c_str());
    }

    bool Editor::OnWindowClosed(WindowCloseEvent& e)
    {
        return true;
    }

    bool Editor::OnWindowResized(WindowResizeEvent& e)
    {
        m_ViewportWidth = e.GetWidth();
        m_ViewportHeight = e.GetHeight();

        m_SceneRenderer->SetViewportSize(m_ViewportWidth, m_ViewportHeight);

        if (m_CurrentScene)
            m_CurrentScene->OnViewportResize(m_ViewportWidth, m_ViewportHeight);

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
        // Right-click on viewport to access the magic menu
        ImGui::SetNextWindowSize({ 200.0f, 400.0f });
        if (ImGui::BeginPopupContextVoid("##ViewportContextMenu"))
        {
            ImGui::MenuItem("Project", nullptr, false, false);
            ImGui::Separator();
            if (ImGui::MenuItem("New Project..."))
            {
                NewProject();
            }
            if (ImGui::MenuItem("Open Project..."))
            {
                OpenProject();
            }

            // Temporary
            if(m_CurrentScene) 
            {
                if (ImGui::MenuItem("New Entity"))
                {
                    TBO_ENGINE_WARN("Entity Added!");
                    Entity e = m_CurrentScene->CreateEntity();

                    auto& transform = e.Transform();

                    static float x = 0.0f;
                    transform.Translation.y = x;
                    ++x;
                    auto& src = e.AddComponent<SpriteRendererComponent>();

                    src.Texture = t;
                }

                if (ImGui::MenuItem("New Camera Entity"))
                {
                    TBO_ENGINE_WARN("Camera Added!");
                    auto& camera = m_CurrentScene->CreateEntity().AddComponent<CameraComponent>();
                    camera.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
                }
            }


            ImGui::EndPopup();
        }

        ImGui::Begin("Statistics & Renderer2D");
        ImGui::Text("Timestep %.5f ms", Time.DeltaTime.ms());
        ImGui::Text("StartTime %.5f ms", Time.TimeSinceStart.ms());
        ImGui::Separator();

        Renderer2D::Statistics stats = m_SceneRenderer->GetRenderer2D().GetStatistics();
        ImGui::Text("Quad Indices %d", stats.QuadIndexCount);
        ImGui::Text("Quad Count %d", stats.QuadCount);
        ImGui::Text("Drawcalls %d", stats.DrawCalls);
        ImGui::End();

        ImGui::ShowDemoWindow();

        // Edit/Play bar
        {
            ImGui::Begin("Editor Console");
            if (ImGui::Button(s_ModeText.c_str()))
            {
                m_EditorMode = m_EditorMode == Mode::Edit ? Mode::Run : Mode::Edit;
                s_ModeText = m_EditorMode == Mode::Edit ? "Run" : "Edit";
            }
            ImGui::End();
        }

        m_CommandAccessPanel.OnUIRender();

        m_NewProjectPopup.OnUIRender();
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
            delete m_CurrentProject;
            m_CurrentProject = nullptr;
        }

        if (projectFilepath.Extension() == ".tproject")
        {
            m_CurrentProject = Project::Deserialize(projectFilepath);

            // Set render scene
            m_CurrentScene = m_CurrentProject->GetDefaultScene();

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

        FString64 defaultSceneName = m_CurrentProject->GetDefaultScene() ? m_CurrentProject->GetDefaultScene()->GetName() : "Empty";
        previousWindowTitle.Append(" - ");
        previousWindowTitle.Append(defaultSceneName);

        // Set new title
        Window->SetTitle(previousWindowTitle);
    }
}
