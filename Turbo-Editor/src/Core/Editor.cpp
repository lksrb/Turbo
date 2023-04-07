#include "Editor.h"

#include "../Panels/QuickAccessPanel.h"
#include "../Panels/ContentBrowserPanel.h"
#include "../Panels/CreateProjectPopupPanel.h"

#include <Turbo/Audio/Audio.h>
#include <Turbo/Debug/ScopeTimer.h>
#include <Turbo/Editor/SceneHierarchyPanel.h>
#include <Turbo/Editor/EditorConsolePanel.h>
#include <Turbo/Script/Script.h>
#include <Turbo/Solution/ProjectSerializer.h>
#include <Turbo/Scene/SceneSerializer.h>

#include <Turbo/Renderer/Font.h>
#include <Turbo/UI/UI.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ImGuizmo.h>

#include <filesystem>
#include <sstream>

namespace Turbo::Ed
{
    namespace Utils
    {
        static void FindAndReplace(std::string& content, const char* oldText, std::string newText)
        {
            std::replace(newText.begin(), newText.end(), '\\', '/');
            size_t pos = 0;
            while ((pos = content.find(oldText, pos)) != std::string::npos)
            {
                content.replace(pos, strlen(oldText), newText);
                pos += strlen(oldText);
            }
        }

        static void FindAndReplace(std::string& content, const char* oldText, const std::filesystem::path& newText)
        {
            FindAndReplace(content, oldText, newText.string());
        }
    }

    Editor::Editor(const Application::Config& config)
        : Application(config), m_ViewportWidth(config.Width), m_ViewportHeight(config.Height)
    {
        // TODO: Load imgui.ini file to retrieve viewport size
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
        m_SceneRenderer = Ref<SceneRenderer>::Create(SceneRenderer::Config{ m_ViewportWidth, m_ViewportHeight, true });

        m_PlayIcon = Texture2D::Create("Resources/Icons/PlayButton.png");
        m_StopIcon = Texture2D::Create("Resources/Icons/StopButton.png");

        m_EditorCamera = EditorCamera(30.0f, static_cast<f32>(m_ViewportWidth) / static_cast<f32>(m_ViewportHeight), 0.1f, 10000.0f);

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
            case Mode::SceneEdit:
            {
                m_EditorCamera.OnUpdate(Time.DeltaTime);

                m_EditorScene->OnEditorUpdate(Time.DeltaTime);
                break;
            }
            case Mode::ScenePlay:
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
            case Mode::SceneEdit:
            {
                m_EditorScene->OnEditorRender(m_SceneRenderer, m_EditorCamera);
                break;
            }
            case Mode::ScenePlay:
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
            Ref<Texture2D> icon = m_EditorMode == Mode::SceneEdit ? m_PlayIcon : m_StopIcon;
            ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));
            if (UI::ImageButton(icon, ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0))
            {
                if (m_EditorMode == Mode::SceneEdit)
                    OnScenePlay();
                else if (m_EditorMode == Mode::ScenePlay)
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
            ImGui::SetNextWindowClass(&window_class);
            ImGui::Begin("Viewport");
            ImGui::PopStyleVar();

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
                UI::Image(m_SceneRenderer->GetFinalImage(), { viewportPanelSize.x, viewportPanelSize.y }, { 0, 1 }, { 1, 0 });
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
            Entity selectedEntity = m_PanelManager->GetPanel<SceneHierarchyPanel>()->GetSelectedEntity();

            if (selectedEntity && m_GizmoType != -1)
            {
                glm::mat4 cameraProjection = m_EditorCamera.GetProjection();
                glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();
                if (m_EditorMode == Mode::ScenePlay)
                {
                    Entity cameraEntity = m_RuntimeScene->GetCameraEntity();
                    if (cameraEntity)
                    {
                        SceneCamera sceneCamera = cameraEntity.GetComponent<CameraComponent>().Camera;
                        cameraProjection = sceneCamera.GetProjection();
                        cameraView = glm::inverse(cameraEntity.Transform().GetMatrix());
                    }
                }
                ImGuizmo::SetOrthographic(false);
                ImGuizmo::SetDrawlist();

                ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);

                // Entity transform
                auto& transformComponent = selectedEntity.Transform();
                glm::mat4 transform = transformComponent.GetMatrix();

                // Snapping
                bool snap = Input::IsKeyPressed(Key::LeftControl);
                f32 snap_value = 0.5f;

                if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
                    snap_value = 45.0f;

                f32 snap_values[] = { snap_value, snap_value, snap_value };

                ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
                (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
                nullptr, snap ? snap_values : nullptr);

                if (ImGuizmo::IsUsing())
                {
                    glm::vec3 translation, rotation, scale;
                    Math::DecomposeTransform(transform, translation, rotation, scale);

                    glm::vec3 deltaRotation = rotation - transformComponent.Rotation;
                    transformComponent.Translation = translation;
                    transformComponent.Rotation += deltaRotation;
                    transformComponent.Scale = scale;
                }
            }

            ImGui::End();
        }

        static bool s_ShowDemoWindow = false;

        // Menu
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
                auto& config = Project::GetActive()->GetConfig();

                if (ImGui::MenuItem("Reload Assembly", "Ctrl+R", nullptr, m_EditorMode == Mode::SceneEdit))
                {
                    Script::ReloadAssemblies();
                }

                if (ImGui::MenuItem("Open solution on start", nullptr, config.OpenSolutionOnStart))
                {
                    Project::SetOpenSolutionOnStart(!config.OpenSolutionOnStart);
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View"))
            {
                ImGui::MenuItem("ImGui demo window", nullptr, &s_ShowDemoWindow);
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        if (s_ShowDemoWindow)
            ImGui::ShowDemoWindow();

        {
            static ImVec2 uv0 = { 0, 1 };
            static ImVec2 uv1 = { 1, 0 };

            ImGui::Begin("Font Atlas");
            ImGui::DragFloat2("UV0", &uv0[0]);
            ImGui::DragFloat2("UV1", &uv1[0]);

            Ref<Font> defaultFont = Font::GetDefaultFont();
            UI::Image(defaultFont->GetAtlasTexture(), { 512, 512 }, uv0, uv1);
            ImGui::End();
        }

        // Scene settings
        {
            ImGui::Begin("Scene settings");
            if (m_CurrentScene)
                ImGui::Checkbox("Physics Colliders", &m_CurrentScene->ShowPhysics2DColliders);

            ImGui::End();
        }

        // Audio clip configuration
        {
            ImGui::Begin("Audio Clip");

            if (m_CurrentScene)
            {
                auto& view = m_CurrentScene->GetAllEntitiesWith<AudioSourceComponent>();

                for (auto& e : view)
                {
                    auto& audioSource = view.get<AudioSourceComponent>(e);

                    if (audioSource.Clip && audioSource.PlayOnStart)
                    {
                        if (ImGui::Button("Play"))
                            Audio::Play(audioSource.Clip, audioSource.Loop);
                        ImGui::NewLine();
                    }
                }
            }

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

        if (m_EditorMode == Mode::SceneEdit)
            m_EditorCamera.OnEvent(e);

        EventDispatcher dispatcher(e);

        dispatcher.Dispatch<KeyPressedEvent>(TBO_BIND_FN(Editor::OnKeyPressed));
        dispatcher.Dispatch<MouseButtonPressedEvent>(TBO_BIND_FN(Editor::OnMouseButtonPressed));
        dispatcher.Dispatch<WindowCloseEvent>(TBO_BIND_FN(Editor::OnWindowClosed));
    }

    bool Editor::OnKeyPressed(KeyPressedEvent& e)
    {
        if (e.GetRepeatCount() > 1)
            return false;

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

    void Editor::CreateProject(std::filesystem::path projectPath)
    {
        // Create and recursively copy template project
        std::filesystem::copy("Resources/NewProjectTemplate", projectPath, std::filesystem::copy_options::recursive);

        std::filesystem::path turboWorkspace = m_CurrentPath.parent_path();
        std::filesystem::path configFile = projectPath / projectPath.stem().concat(".tproject");

        // TODO: Make this more dynamic

        // Format Premake5 
        {
            std::ifstream inStream(projectPath / "premake5.lua");
            TBO_ENGINE_ASSERT(inStream);
            std::stringstream ss;
            ss << inStream.rdbuf();
            inStream.close();

            std::string content = ss.str();
            Utils::FindAndReplace(content, "%PROJECT_NAME%", projectPath.stem());
            Utils::FindAndReplace(content, "%TURBO_PATH%", turboWorkspace);

            std::ofstream outStream(projectPath / "premake5.lua");
            TBO_ENGINE_ASSERT(outStream);
            outStream << content;
            outStream.close();
        }

        // Format .tproject file
        {
            std::ifstream inStream(projectPath / "NewProjectTemplate.tproject");
            TBO_ENGINE_ASSERT(inStream);
            std::stringstream ss;
            ss << inStream.rdbuf();
            inStream.close();

            std::filesystem::remove(projectPath / "NewProjectTemplate.tproject");

            std::string content = ss.str();
            Utils::FindAndReplace(content, "%PROJECT_NAME%", projectPath.stem());

            std::ofstream outStream(configFile);
            TBO_ENGINE_ASSERT(outStream);
            outStream << content;
            outStream.close();
        }

        // Open copied template project
        OpenProject(configFile);
    }

    void Editor::OpenProject(std::filesystem::path configFilePath)
    {
        using namespace std::chrono_literals;

        // Opens platform specific 
        if (configFilePath.empty())
        {
            configFilePath = Platform::OpenFileDialog("Open Project", "Turbo Project(*.tproject)\0 * .tproject\0");

            if (configFilePath.empty())
                return;
        }

        // Project deserialization
        Ref<Project> project = Ref<Project>::Create();
        {
            ProjectSerializer serializer(project);
            TBO_ASSERT(serializer.Deserialize(configFilePath));
            TBO_INFO("Project loaded successfully!"); // TODO: TBO_VERIFY -> Prints when it successeds

            // Set it as new active project
            Project::SetActive(project);

        }

        // All panels receive that project has been sent
        m_PanelManager->OnProjectChanged(project);

        // Building assemblies
        const auto& projectPath = configFilePath.parent_path();
        const auto& config = project->GetConfig();
        const auto& assemblyPath = projectPath / config.ScriptModulePath;

        // Check if client configured project to open IDE on start:
        // If yes => open solution and run build solution on that project
        // If no => run build option to compile it anyway
        // NOTE: Best solution would be to create some mini builder module that would take care of building a solution when needed

        //std::thread loaderThread = std::thread([=]() 
        {
            std::wstring solutionFile = configFilePath.stem().wstring();
            solutionFile.append(L".sln");
            bool assembliesExists = std::filesystem::exists(assemblyPath);
            if (config.OpenSolutionOnStart)
                Platform::Execute(L"Win32-GenerateSolution.bat", solutionFile, projectPath);
            else if (assembliesExists)
                Platform::Execute(projectPath / "Win32-GenerateSolution.bat", solutionFile.append(L" Debug"), projectPath);

            // FIXME: Not ideal, maybe implement loader thread?
            while (!assembliesExists)
            {
                // Wait for it to build...
                std::this_thread::sleep_for(1s);
                // Check if it exists
                assembliesExists = std::filesystem::exists(assemblyPath);
            }
            // Load project assembly
            Script::LoadProjectAssembly(assemblyPath);

            // Open scene
            OpenScene(config.ProjectDirectory / config.AssetsDirectory / config.StartScenePath);
        };
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

        // Save project
        auto& active = Project::GetActive();
        ProjectSerializer serializer(active);
        TBO_ASSERT(serializer.Serialize(Project::GetProjectConfigPath()));
        TBO_INFO("Project saved successfully!"); // TODO: TBO_VERIFY -> Prints when it successeds
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
        std::filesystem::path scenePath = filepath;

        if (scenePath.empty())
        {
            scenePath = Platform::OpenFileDialog("Open Scene", "Turbo Scene(*.tscene)\0 * .tscene\0");
            if (scenePath.empty())
                return; // No scene selected

        }

        m_EditorScene = Ref<Scene>::Create();

        // Set current scene
        m_CurrentScene = m_EditorScene;

        SceneSerializer serializer(m_EditorScene);
        TBO_ASSERT(serializer.Deserialize(scenePath.string()), "Could not deserialize scene!");

        m_EditorScene->SetViewportSize(m_ViewportWidth, m_ViewportHeight);

        m_PanelManager->SetSceneContext(m_EditorScene);

        m_EditorScenePath = scenePath;

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

        m_EditorMode = Mode::ScenePlay;

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
        m_EditorMode = Mode::SceneEdit;

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
