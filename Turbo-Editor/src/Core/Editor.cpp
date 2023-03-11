#include "Editor.h"

#include "../Panels/SceneHierarchyPanel.h"
#include "../Panels/QuickAccessPanel.h"
#include "../Panels/ContentBrowserPanel.h"
#include "../Panels/CreateProjectPopupPanel.h"

#include <Turbo/Scene/SceneSerializer.h>
#include <Turbo/Benchmark/ScopeTimer.h>
#include <Turbo/UI/UI.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ImGuizmo.h>

#include <filesystem>

namespace Turbo::Ed
{
    extern std::filesystem::path g_AssetPath = {};

    Editor::Editor(const Application::Config& config)
        : Application(config)
    {
    }

    Editor::~Editor()
    {
    }

    void Editor::OnInitialize()
    {
        m_CurrentPath = std::filesystem::current_path();

        g_AssetPath = m_CurrentPath / "Assets";

        // Panels
        m_PanelManager = Ref<PanelManager>::Create();
        m_PanelManager->AddPanel<SceneHierarchyPanel>();
        m_PanelManager->AddPanel<QuickAccessPanel>();
        m_PanelManager->AddPanel<ContentBrowserPanel>();

        // TODO: Think about panel callbacks
        m_PanelManager->AddPanel<CreateProjectPopupPanel>()->SetCallback([this](const std::filesystem::path& path)
        {
            // Unload active project, create new project, set it as new active project
            Project::Create(path);

            auto& active = Project::GetActive();

            // Set render scene
            m_EditorScene = active->GetStartupScene();

            // Resize scene
            m_EditorScene->SetViewportSize(m_ViewportWidth, m_ViewportHeight);

            // Set context scene
            m_PanelManager->GetPanel<SceneHierarchyPanel>()->SetContext(m_EditorScene);

            UpdateWindowTitle();
        });

        // Render 
        m_SceneRenderer = Ref<SceneRenderer>::Create(SceneRenderer::Config{ m_Config.Width, m_Config.Height, true });

        m_PlayIcon = Texture2D::Create({ "Resources/Icons/PlayButton.png" });
        m_StopIcon = Texture2D::Create({ "Resources/Icons/StopButton.png" });

        m_EditorCamera = EditorCamera(30.0f, static_cast<f32>(m_Config.Width) / static_cast<f32>(m_Config.Height), 0.1f, 10000.0f);

        // Open sandbox project
        OpenProject(g_AssetPath / "SandboxProject\\SandboxProject.tproject");
    }

    void Editor::OnShutdown()
    {
    }

    void Editor::OnUpdate()
    {
        if (!m_EditorScene)
            return;

        switch (m_EditorMode)
        {
            case Mode::Edit:
            {
                m_EditorCamera.OnUpdate(Time.DeltaTime);

                m_EditorScene->OnEditorUpdate(Time.DeltaTime);
                break;
            }
            case Mode::Play:
            {
                m_RuntimeScene->OnRuntimeUpdate(Time.DeltaTime);
                break;
            }
        }
    }

    void Editor::OnDraw()
    {
        if (!m_EditorScene)
            return;

        switch (m_EditorMode)
        {
            case Mode::Edit:
            {
                m_EditorScene->OnEditorRender(m_SceneRenderer, m_EditorCamera);
                break;
            }
            case Mode::Play:
            {
                m_RuntimeScene->OnRuntimeRender(m_SceneRenderer);
                break;
            }
        }
    }

    void Editor::OnEvent(Event& e)
    {
        m_PanelManager->OnEvent(e);

        if (m_EditorMode == Mode::Edit)
            m_EditorCamera.OnEvent(e);

        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<KeyPressedEvent>(TBO_BIND_FN(Editor::OnKeyPressed));
        dispatcher.Dispatch<MouseButtonPressedEvent>(TBO_BIND_FN(Editor::OnMouseButtonPressed));
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
            // Gizmos
            case Key::Q:
            {
                if (!ImGuizmo::IsUsing())
                    m_GizmoType = -1;
                break;
            }
            case Key::W:
            {
                if (!ImGuizmo::IsUsing())
                    m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
                break;
            }
            case Key::E:
            {
                if (!ImGuizmo::IsUsing())
                    m_GizmoType = ImGuizmo::OPERATION::ROTATE;
                break;
            }
            case Key::R:
            {
                if (!ImGuizmo::IsUsing())
                    m_GizmoType = ImGuizmo::OPERATION::SCALE;
                break;
            }
        }

        return true;
    }

    bool Editor::OnMouseButtonPressed(MouseButtonPressedEvent& e)
    {
        if (e.GetMouseButton() == Mouse::ButtonLeft)
        {
            if (m_ViewportHovered && !ImGuizmo::IsOver() && !Input::IsKeyPressed(Key::LeftAlt))
                m_PanelManager->GetPanel<SceneHierarchyPanel>()->SetSelectedEntity(m_HoveredEntity);
        }
        return false;
    }

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

        f32 minWinSizeX = style.WindowMinSize.x;
        style.WindowMinSize.x = 370.0f;
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("VulkanAppDockspace");

            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }
        style.WindowMinSize.x = minWinSizeX;
        // --Dockspace

        // Toolbar
        {
            ImGuiWindowClass window_class;
            window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
            ImGui::SetNextWindowClass(&window_class);

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            auto& colors = ImGui::GetStyle().Colors;
            const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
            const auto& buttonActive = colors[ImGuiCol_ButtonActive];
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

            ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
            f32 size = ImGui::GetWindowHeight() - 4.0f;
            Ref<Texture2D> icon = m_EditorMode == Mode::Edit ? m_PlayIcon : m_StopIcon;
            ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));
            if (UI::ImageButton(icon, ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0))
            {
                if (m_EditorMode == Mode::Edit)
                    OnScenePlay();
                else if (m_EditorMode == Mode::Play)
                    OnSceneStop();
            }
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(3);
            ImGui::End();
        }

        // Viewport
        {
            ImGui::Begin("Viewport");

            m_ViewportHovered = ImGui::IsWindowHovered();
            m_ViewportFocused = ImGui::IsWindowFocused();
            Engine::Get().GetUI()->SetBlockEvents(!m_ViewportFocused && !m_ViewportHovered);

            ImVec2 viewportMinRegion = ImGui::GetWindowContentRegionMin();
            ImVec2 viewportMaxRegion = ImGui::GetWindowContentRegionMax();
            ImVec2 viewportOffset = ImGui::GetWindowPos();
            m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
            m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

            ImVec2 window_size = ImGui::GetWindowSize();

            ImVec2 viewport_panel_size = ImGui::GetContentRegionAvail();

            if (m_ViewportWidth != window_size.x || m_ViewportHeight != window_size.y)
            {
                OnViewportResize(static_cast<u32>(window_size.x), static_cast<u32>(window_size.y));
            }

            if (m_EditorScene)
            {
                UI::Image(m_SceneRenderer->GetFinalImage(), { viewport_panel_size.x,viewport_panel_size.y }, { 0, 1 }, { 1, 0 });
            }

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                {
                    const wchar_t* path = (const wchar_t*)payload->Data;
                    OpenScene(path);
                }

                ImGui::EndDragDropTarget();
            }

            // Right-click on viewport to access the magic menu
            if (!m_EditorCamera.IsControlling())
            {
                ImGui::SetNextWindowSize({ 200.0f, 400.0f });
                if (ImGui::BeginPopupContextWindow("##ViewportContextMenu"))
                {
                    ImGui::MenuItem("Scene", nullptr, false, false);
                    ImGui::Separator();
                    if (ImGui::MenuItem("New Entity", nullptr, false, m_EditorScene))
                    {
                        TBO_WARN("Entity Added!");
                        Entity e = m_EditorScene->CreateEntity();

                        auto& transform = e.Transform();

                        static f32 x = 0.0f;
                        transform.Translation.y = x;
                        ++x;
                        auto& src = e.AddComponent<SpriteRendererComponent>();

                        Ref<Texture2D> texture2d = Texture2D::Create({ "Assets/Textures/smile.png" });

                        if (texture2d->IsLoaded())
                            src.Texture = texture2d;
                    }

                    if (ImGui::MenuItem("New Camera Entity", nullptr, false, m_EditorScene))
                    {
                        TBO_WARN("Camera Added!");
                        auto& camera = m_EditorScene->CreateEntity().AddComponent<CameraComponent>();
                        camera.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
                    }
                    ImGui::EndPopup();
                }
            }

            // Gizmos
            Entity selected_entity = m_PanelManager->GetPanel<SceneHierarchyPanel>()->GetSelectedEntity();

            if (selected_entity && m_GizmoType != -1)
            {
                // Editor camera
                glm::mat4 camera_projection = m_EditorCamera.GetProjection();
                glm::mat4 camera_view = m_EditorCamera.GetViewMatrix();

                ImGuizmo::SetOrthographic(false);
                ImGuizmo::SetDrawlist();

                ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);

                // Entity transform
                auto& transform_cmp = selected_entity.Transform();
                glm::mat4 transform = transform_cmp.GetMatrix();

                // Snapping
                bool snap = Input::IsKeyPressed(Key::LeftControl);
                f32 snap_value = 0.5f;

                if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
                    snap_value = 45.0f;

                f32 snap_values[] = { snap_value, snap_value, snap_value };

                ImGuizmo::Manipulate(glm::value_ptr(camera_view), glm::value_ptr(camera_projection),
                (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
                nullptr, snap ? snap_values : nullptr);

                if (ImGuizmo::IsUsing())
                {
                    glm::vec3 translation, rotation, scale;
                    Math::DecomposeTransform(transform, translation, rotation, scale);

                    glm::vec3 deltaRotation = rotation - transform_cmp.Rotation;
                    transform_cmp.Translation = translation;
                    transform_cmp.Rotation += deltaRotation;
                    transform_cmp.Scale = scale;
                }
            }

            ImGui::End();
        }

        // Menu
        {
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
                    if (ImGui::MenuItem("Save Project"))
                    {
                        SaveProject();
                    }
                    if (ImGui::MenuItem("Save Scene"))
                    {
                        SaveScene();
                    }
                    if (ImGui::MenuItem("Save Scene As..."))
                    {
                        SaveSceneAs();
                    }
                    ImGui::EndMenu();
                }

                ImGui::EndMenuBar();
            }
        }


        ImGui::Begin("Statistics & Renderer2D");
        ImGui::Text("Timestep %.5f ms", Time.DeltaTime.ms());
        ImGui::Text("StartTime %.5f ms", Time.TimeSinceStart.ms());
        ImGui::Separator();

        Renderer2D::RenderInfo stats = m_SceneRenderer->GetRenderer2D()->GetRenderInfo();
        ImGui::Text("Quad Indices %d", stats.QuadIndexCount);
        ImGui::Text("Quad Count %d", stats.QuadCount);
        ImGui::Text("Drawcalls %d", stats.DrawCalls);
        ImGui::End();

        // EditorConsole
        {
            ImGui::Begin("Editor Console");
            ImGui::End();
        }

        // Draw all panels
        m_PanelManager->OnDrawUI();

        // End dockspace
        ImGui::End();
    }

    void Editor::NewProject()
    {
        m_PanelManager->GetPanel<CreateProjectPopupPanel>()->Open();
    }

    void Editor::OpenProject(const std::filesystem::path& filepath)
    {
        std::filesystem::path project_path = filepath;

        // Opens platform specific 
        if (filepath.empty())
        {
            project_path = Platform::OpenFileDialog("Open Project", "Turbo Project(*.tproject)\0 * .tproject\0");

            if (project_path.empty())
                return;
        }

        if (Project::Open(project_path))
        {
            TBO_ENGINE_INFO("Project loaded successfully!");

            // Set render scene
            m_EditorScene = Project::GetActive()->GetStartupScene();
            m_EditorScene->SetViewportSize(m_ViewportWidth, m_ViewportHeight);
            m_PanelManager->GetPanel<SceneHierarchyPanel>()->SetContext(m_EditorScene);
            
            // Reset selected entity
            m_PanelManager->GetPanel<SceneHierarchyPanel>()->SetSelectedEntity();

            m_EditorScenePath = m_EditorScene->GetFullPath();

            UpdateWindowTitle();

            return;
        }

        TBO_ERROR("Could not open project! (\"{0}\")", project_path);
    }

    void Editor::UpdateWindowTitle()
    {
        auto& active = Project::GetActive();

        if (!active || !m_EditorScene)
            return;

        std::string scene_name = m_EditorScene->GetName();
        std::string title = fmt::format("TurboEditor | {0} - {1} | Vulkan", active->GetName(), scene_name);
        Window->SetTitle(title.data());
    }

    void Editor::OnViewportResize(u32 width, u32 height)
    {
        m_ViewportWidth = width;
        m_ViewportHeight = height;

        if (m_EditorScene)
            m_EditorScene->SetViewportSize(m_ViewportWidth, m_ViewportHeight);

        m_EditorCamera.SetViewportSize(width, height);
        m_SceneRenderer->OnViewportSize(width, height);
    }

    void Editor::SaveProject()
    {
        SaveScene();
        // TODO: Save project's configuration
    }


    void Editor::SaveScene()
    {
        if (m_EditorScenePath.empty())
        {
            SaveSceneAs();
            return;
        }

        SceneSerializer serializer(m_EditorScene);
        if (serializer.Serialize(m_EditorScenePath.string()))
        {
            TBO_INFO("Successfully serialized \"{0}\"!", m_EditorScenePath.stem().string());
            return;
        }

        TBO_ERROR("Could not serialize \"{0}\"!", m_EditorScenePath);
    }

    void Editor::OpenScene(const std::filesystem::path& filepath /*= {}*/)
    {
        std::filesystem::path scene_path = filepath;

        if (scene_path.empty())
        {
            scene_path = Platform::OpenFileDialog("Open Scene", "Turbo Scene(*.tscene)\0 * .tscene\0");
            if (scene_path.empty())
                return; // No scene selected

        }

        m_EditorScene = Ref<Scene>::Create(Scene::Config{ scene_path.stem().string()});

        SceneSerializer serializer(m_EditorScene);
        TBO_ASSERT(serializer.Deserialize(scene_path.string()), "Could not deserialize scene!");

        m_EditorScene->SetViewportSize(m_ViewportWidth, m_ViewportHeight);
        m_PanelManager->GetPanel<SceneHierarchyPanel>()->SetContext(m_EditorScene);

        m_EditorScenePath = m_EditorScene->GetFullPath();

        UpdateWindowTitle();
    }

    void Editor::SaveSceneAs()
    {
        auto& active = Project::GetActive();

        if (!active)
        {
            TBO_ERROR("No project loaded!");
            return;
        }

        if (!m_EditorScene)
        {
            TBO_ERROR("No scene loaded!");
            return;
        }

        std::filesystem::path& filepath = Platform::SaveFileDialog("Turbo Scene(*.tscene)\0 * .tscene\0");

        if (!filepath.empty())
        {
            if (filepath.extension() != ".tscene")
                filepath.concat(".tscene");

            SceneSerializer serializer(m_EditorScene);
            
            if (serializer.Serialize(filepath.string()))
            {
                TBO_INFO("Successfully serialized \"{0}\"!", m_EditorScene->GetName());
                UpdateWindowTitle();
            }
        }

        TBO_ERROR("Could not serialize \"{0}\"!", m_EditorScene->GetName());
    }

    void Editor::OnScenePlay()
    {
        m_EditorMode = Mode::Play;

        m_RuntimeScene = Scene::Copy(m_EditorScene);
        m_RuntimeScene->OnRuntimeStart();

        // Select the same entity in runtime scene
        {
            UUID selected_entity_uuid = 0;
            m_SelectedEntity = m_PanelManager->GetPanel<SceneHierarchyPanel>()->GetSelectedEntity();

            if (m_SelectedEntity)
                selected_entity_uuid = m_SelectedEntity.GetUUID();

            m_SelectedEntity = m_RuntimeScene->GetEntityByUUID(selected_entity_uuid);
            m_PanelManager->GetPanel<SceneHierarchyPanel>()->SetSelectedEntity(m_SelectedEntity);
        }

        m_PanelManager->GetPanel<SceneHierarchyPanel>()->SetContext(m_RuntimeScene);
    }

    void Editor::OnSceneStop()
    {
        m_EditorMode = Mode::Edit;

        m_RuntimeScene->OnRuntimeStop();
        m_RuntimeScene = m_EditorScene;

        // Select the same entity in editor scene
        {
            UUID selected_entity_uuid = 0;
            m_SelectedEntity = m_PanelManager->GetPanel<SceneHierarchyPanel>()->GetSelectedEntity();

            if (m_SelectedEntity)
                selected_entity_uuid = m_SelectedEntity.GetUUID();

            m_SelectedEntity = m_EditorScene->GetEntityByUUID(selected_entity_uuid);
            m_PanelManager->GetPanel<SceneHierarchyPanel>()->SetSelectedEntity(m_SelectedEntity);
        }

        m_PanelManager->GetPanel<SceneHierarchyPanel>()->SetContext(m_EditorScene);
    }
}
