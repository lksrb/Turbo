#include "Editor.h"

#include "../Panels/QuickAccessPanel.h"
#include "../Panels/ContentBrowserPanel.h"
#include "../Panels/CreateProjectPopupPanel.h"

#include <Turbo/Debug/ScopeTimer.h>
#include <Turbo/Editor/SceneHierarchyPanel.h>
#include <Turbo/Editor/EditorConsolePanel.h>
#include <Turbo/Script/Script.h>
#include <Turbo/Solution/ProjectSerializer.h>
#include <Turbo/Scene/SceneSerializer.h>
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

        // Panels
        m_PanelManager = Ref<PanelManager>::Create();
        m_PanelManager->AddPanel<SceneHierarchyPanel>();
        m_PanelManager->AddPanel<QuickAccessPanel>();
        m_PanelManager->AddPanel<ContentBrowserPanel>();
        m_PanelManager->AddPanel<EditorConsolePanel>();

        // TODO: Think about panel callbacks
        m_PanelManager->AddPanel<CreateProjectPopupPanel>()->SetCallback([this](const auto& path)
        {
            CreateProject(path);
        });

        // Render 
        m_SceneRenderer = Ref<SceneRenderer>::Create(SceneRenderer::Config{ m_Config.Width, m_Config.Height, true });

        m_PlayIcon = Texture2D::Create({ "Resources/Icons/PlayButton.png" });
        m_StopIcon = Texture2D::Create({ "Resources/Icons/StopButton.png" });

        m_EditorCamera = EditorCamera(30.0f, static_cast<f32>(m_Config.Width) / static_cast<f32>(m_Config.Height), 0.1f, 10000.0f);

        // Open sandbox project
        OpenProject(m_CurrentPath / "SandboxProject\\SandboxProject.tproject");
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
            ImGuiWindowFlags windowFlags = 0;
            windowFlags |= ImGuiWindowFlags_NoTitleBar;
            windowFlags |= ImGuiWindowFlags_NoMove;

            ImGuiWindowClass window_class;
            window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
            //ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 0.f));
            ImGui::SetNextWindowClass(&window_class);
            ImGui::Begin("Viewport");
            
            m_ViewportHovered = ImGui::IsWindowHovered();
            m_ViewportFocused = ImGui::IsWindowFocused();
            Engine->GetUI()->SetBlockEvents(!m_ViewportFocused && !m_ViewportHovered);

            ImVec2 viewportMinRegion = ImGui::GetWindowContentRegionMin();
            ImVec2 viewportMaxRegion = ImGui::GetWindowContentRegionMax();
            ImVec2 viewportOffset = ImGui::GetWindowPos();
            m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
            m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

            ImVec2 windowSize = ImGui::GetWindowSize();

            ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

            if (m_ViewportWidth != windowSize.x || m_ViewportHeight != windowSize.y)
            {
                OnViewportResize(static_cast<u32>(windowSize.x), static_cast<u32>(windowSize.y));
            }

            if (m_EditorScene)
            {
                UI::Image(m_SceneRenderer->GetFinalImage(), { viewportPanelSize.x,viewportPanelSize.y }, { 0, 1 }, { 1, 0 });
            }

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM_VIEWPORT"))
                {
                    std::filesystem::path path = (const wchar_t*)payload->Data;

                    if (path.extension() == ".tscene")
                    {
                        OpenScene(path);
                    }
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
            ImGui::PopStyleVar();
        }

        // Menu
        {
            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("New Project..."))
                    {
                        m_PanelManager->GetPanel<CreateProjectPopupPanel>()->Open();
                    }
                    if (ImGui::MenuItem("Open Project...", "Ctrl+O"))
                    {
                        OpenProject();
                    }
                    if (ImGui::MenuItem("Save Project", "Ctrl+S"))
                    {
                        SaveProject();
                    }
                    ImGui::Separator();

                    if (ImGui::MenuItem("Save Scene"))
                    {
                        SaveScene();
                    }
                    if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
                    {
                        SaveSceneAs();
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Exit", "Alt+F4"))
                    {
                        Close();
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Project"))
                {
                    if (ImGui::MenuItem("Reload Assembly", "Ctrl+R", nullptr, m_EditorMode == Mode::Edit))
                    {
                        Script::ReloadAssemblies();
                    }
                    ImGui::EndMenu();
                }

                ImGui::EndMenuBar();
            }
        }

        // Scene settings
        {
            ImGui::Begin("Scene settings");
            ImGui::Checkbox("Physics Colliders", &m_CurrentScene->ShowPhysics2DColliders);
            
            ImGui::End();
        }

        ImGui::Begin("Statistics & Renderer2D");
        ImGui::Text("Timestep: %.5f ms", Time.DeltaTime.ms());
        ImGui::Text("StartTime: %.5f ms", Time.TimeSinceStart.ms());
        ImGui::Separator();

        // TODO: Better statistics
        Renderer2D::Statistics stats = m_SceneRenderer->GetRenderer2D()->GetStatistics();
        ImGui::Text("Quad Count: %d", stats.QuadCount);
        ImGui::Text("Drawcalls: %d", stats.DrawCalls);
        ImGui::End();

        // Draw all panels
        m_PanelManager->OnDrawUI();

        // End dockspace
        ImGui::End();
    }

    void Editor::OnEvent(Event& e)
    {
        m_PanelManager->OnEvent(e);

        if (m_EditorMode == Mode::Edit)
            m_EditorCamera.OnEvent(e);

        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<KeyPressedEvent>(TBO_BIND_FN(Editor::OnKeyPressed));
        dispatcher.Dispatch<MouseButtonPressedEvent>(TBO_BIND_FN(Editor::OnMouseButtonPressed));
        dispatcher.Dispatch<WindowCloseEvent>(TBO_BIND_FN(Editor::OnWindowClosed));
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

            // Menu
            case Key::S:
            {
                if (control && shift)
                    SaveSceneAs();
                else if (control)
                    SaveScene();

                break;
            }
            case Key::O:
            {
                if (control && shift)
                    SaveSceneAs();
                break;
            }
        }

        return true;
    }

    bool Editor::OnWindowClosed(WindowCloseEvent& e)
    {
        Close();
        return false;
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

    void Editor::CreateProject(const std::filesystem::path& project_path)
    {
        // Create project config
        Project::Config config;
        config.AssetsDirectory = "Assets";
        config.Name = project_path.stem().string();
        config.ProjectDirectory = project_path;
        config.StartScenePath = "Scenes\\StartScene.tscene";

        // Set scene path
        m_EditorScenePath = config.ProjectDirectory / config.AssetsDirectory / config.StartScenePath;

        // Create directories
        std::filesystem::create_directory(project_path);
        std::filesystem::create_directory(project_path / config.AssetsDirectory);
        std::filesystem::create_directory(project_path / config.AssetsDirectory / "Scenes");

        // TODO: Templates
        Ref<Project> project = Ref<Project>::Create(config);
        Project::SetActive(project);

        {
            ProjectSerializer serializer(project);
            TBO_ASSERT(serializer.Serialize(Project::GetProjectConfigPath()));
        }

        Ref<Scene> scene = Ref<Scene>::Create();
        {
            Entity sprite = scene->CreateEntity("Sprite");
            sprite.AddComponent<SpriteRendererComponent>().Color = { 0.8f, 0.1f, 0.2f, 1.0 };
            sprite.AddComponent<Rigidbody2DComponent>();
            sprite.AddComponent<BoxCollider2DComponent>();
        }

        {
            Entity ground = scene->CreateEntity("Ground");
            ground.Transform().Translation.y = -2.0f;
            ground.Transform().Scale.y = 0.5f;
            ground.Transform().Scale.x = 5.0f;
            ground.AddComponent<SpriteRendererComponent>().Color = { 0.2f, 0.8f, 0.1f, 1.0 };
            ground.AddComponent<Rigidbody2DComponent>();
            ground.AddComponent<BoxCollider2DComponent>();
        }
        {
            Entity camera = scene->CreateEntity("Camera");
            camera.AddComponent<CameraComponent>();
        }
        SceneSerializer serializer(scene);
        TBO_ASSERT(serializer.Serialize(m_EditorScenePath))

        // ... and open project
        OpenProject(Project::GetProjectConfigPath());
    }

    void Editor::OpenProject(std::filesystem::path project_path)
    {
        // Opens platform specific 
        if (project_path.empty())
        {
            project_path = Platform::OpenFileDialog("Open Project", "Turbo Project(*.tproject)\0 * .tproject\0");

            if (project_path.empty())
                return;
        }

        // Project deserialization
        Ref<Project> project = Ref<Project>::Create();
        {
            ProjectSerializer serializer(project);
            TBO_ASSERT(serializer.Deserialize(project_path));
            TBO_INFO("Project loaded successfully!"); // TODO: TBO_VERIFY -> Prints when it successeds

            // Set it as new active project
            Project::SetActive(project);

        }
        g_AssetPath = project_path.parent_path();

        const auto& config = project->GetConfig();

        // All panels receive this event
        m_PanelManager->OnProjectChanged(project);

        // Load project assembly
        Script::LoadProjectAssembly(g_AssetPath / project->GetConfig().ScriptModulePath);

        // Open scene
        OpenScene(config.ProjectDirectory / config.AssetsDirectory / config.StartScenePath);
    }

    void Editor::UpdateWindowTitle()
    {
        auto& project = Project::GetActive();

        if (!project || !m_EditorScene)
            return;

        const std::string& scene_name = m_EditorScenePath.stem().string();
        const std::string& title = fmt::format("TurboEditor | {0} - {1} | Vulkan", project->GetProjectName(), scene_name);
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

        m_EditorScene = Ref<Scene>::Create();

        // Set current scene
        m_CurrentScene = m_EditorScene;

        SceneSerializer serializer(m_EditorScene);
        TBO_ASSERT(serializer.Deserialize(scene_path.string()), "Could not deserialize scene!");

        m_EditorScene->SetViewportSize(m_ViewportWidth, m_ViewportHeight);

        m_PanelManager->SetSceneContext(m_EditorScene);

        m_EditorScenePath = scene_path;

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
                TBO_INFO("Successfully serialized {0}!", filepath);
                UpdateWindowTitle();
            }
        }

        TBO_ERROR("Could not serialize {0}!", filepath);
    }

    void Editor::OnScenePlay()
    {
        // Clear logs
        EditorConsolePanel::Clear();

        m_EditorMode = Mode::Play;

        m_RuntimeScene = Scene::Copy(m_EditorScene);
        m_RuntimeScene->OnRuntimeStart();

        // Set current scene
        m_CurrentScene = m_RuntimeScene;

        // Select the same entity in runtime scene
        {
            UUID selected_entity_uuid = 0;
            m_SelectedEntity = m_PanelManager->GetPanel<SceneHierarchyPanel>()->GetSelectedEntity();

            if (m_SelectedEntity)
                selected_entity_uuid = m_SelectedEntity.GetUUID();

            m_SelectedEntity = m_RuntimeScene->FindEntityByUUID(selected_entity_uuid);
        }

        m_PanelManager->SetSceneContext(m_RuntimeScene);
        m_PanelManager->GetPanel<SceneHierarchyPanel>()->SetSelectedEntity(m_SelectedEntity);
    }

    void Editor::OnSceneStop()
    {
        m_EditorMode = Mode::Edit;

        m_RuntimeScene->OnRuntimeStop();
        m_RuntimeScene = m_EditorScene;

        // Set current scene
        m_CurrentScene = m_EditorScene;

        // Select the same entity in editor scene
        {
            UUID selected_entity_uuid = 0;
            m_SelectedEntity = m_PanelManager->GetPanel<SceneHierarchyPanel>()->GetSelectedEntity();

            if (m_SelectedEntity)
                selected_entity_uuid = m_SelectedEntity.GetUUID();

            m_SelectedEntity = m_EditorScene->FindEntityByUUID(selected_entity_uuid);
        }

        m_PanelManager->SetSceneContext(m_RuntimeScene);
        m_PanelManager->GetPanel<SceneHierarchyPanel>()->SetSelectedEntity(m_SelectedEntity);
    }

    void Editor::Close()
    {
        // TODO: Make this an option

        SaveProject(); // Auto save
    }
}
