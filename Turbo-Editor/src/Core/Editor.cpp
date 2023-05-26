#include "Editor.h"

#include "../Panels/QuickAccessPanel.h"
#include "../Panels/ContentBrowserPanel.h"
#include "../Panels/CreateProjectPopupPanel.h"

#include <Turbo/Audio/Audio.h>
#include <Turbo/Asset/AssetManager.h>
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

#ifdef TBO_PLATFORM_WIN32
    #define TBO_VS2022_REGISTRY_KEY L"SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\App Paths\\devenv.exe"
    #define TBO_GEN_SOLUTION_FILE "Win32-GenerateSolution.bat"
#endif

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

        static std::filesystem::path GetMSBuildPath()
        {
            std::filesystem::path pathToVS = Platform::GetRegistryValue(RootKey::LocalMachine, TBO_VS2022_REGISTRY_KEY);
            TBO_ASSERT(!pathToVS.empty(), "Visual Studio 2022 is not installed!");

            auto filter = [](const std::filesystem::path& path)
            {
                const auto& name = path.stem();
                return name == L"Community" || name == L"Professional" || name == L"Enterprise";
            };

            if (!pathToVS.empty())
            {
                // Path to MSBuild
                std::filesystem::path msBuildPath = pathToVS;

                while (msBuildPath.has_parent_path())
                {
                    if (filter(msBuildPath) || msBuildPath == L"C:\\")
                        break;

                    msBuildPath = msBuildPath.parent_path();
                }

                if (msBuildPath != L"C:\\")
                {
                    return msBuildPath / "MSBuild\\Current\\Bin\\MSBuild.exe";
                }
            }

            TBO_ASSERT(false, "Could not find MSBuild!");

            return {};
        }

        static const wchar_t* GetIDEToString(Editor::IDE ide)
        {
            switch (ide)
            {
                case Editor::IDE::None: return L"none";
                case Editor::IDE::VisualStudio2022: return L"vs2022";
            }

            TBO_ENGINE_ERROR("Invalid IDE!");
            return L"";
        }
    }

    Editor::Editor(const Application::Config& config)
        : Application(config), m_ViewportWidth(config.Width), m_ViewportHeight(config.Height)
    {
    }

    Editor::~Editor()
    {
    }

    void Editor::OnInitialize()
    {
        m_CurrentPath = std::filesystem::current_path();

        // Get path to msbuild for building assemblies
        // NOTE: Who's responsibility is this?
        m_MSBuildPath = Utils::GetMSBuildPath();

        // Panels
        m_PanelManager = Ref<PanelManager>::Create();
        m_PanelManager->AddPanel<SceneHierarchyPanel>();
        m_PanelManager->AddPanel<QuickAccessPanel>();
        m_PanelManager->AddPanel<ContentBrowserPanel>();
        m_PanelManager->AddPanel<EditorConsolePanel>();
        m_PanelManager->AddPanel<CreateProjectPopupPanel>(TBO_BIND_FN(Editor::CreateProject));

        // Render 
        m_SceneRenderer = Ref<SceneRenderer>::Create(SceneRenderer::Config{ m_ViewportWidth, m_ViewportHeight, true });

        m_PlayIcon = Texture2D::Create("Resources/Icons/PlayButton.png");
        m_StopIcon = Texture2D::Create("Resources/Icons/StopButton.png");

        m_EditorCamera = EditorCamera(30.0f, static_cast<f32>(m_ViewportWidth) / static_cast<f32>(m_ViewportHeight), 0.1f, 10000.0f);

        // Open sandbox project
        OpenProject(m_CurrentPath / "GunNRun\\GunNRun.tproject");
    }

    void Editor::OnShutdown()
    {
    }

    void Editor::OnUpdate()
    {
        switch (m_SceneMode)
        {
            case Mode::SceneEdit:
            {
                if (m_ViewportHovered)
                    m_EditorCamera.OnUpdate(Time.DeltaTime);

                m_CurrentScene->OnEditorUpdate(Time.DeltaTime);
                break;
            }
            case Mode::ScenePlay:
            {
                m_CurrentScene->OnRuntimeUpdate(Time.DeltaTime);
                break;
            }
        }
    }

    void Editor::OnDraw()
    {
        switch (m_SceneMode)
        {
            case Mode::SceneEdit:
            {
                m_CurrentScene->OnEditorRender(m_SceneRenderer, m_EditorCamera);
                break;
            }
            case Mode::ScenePlay:
            {
                m_CurrentScene->OnRuntimeRender(m_SceneRenderer);
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
            Ref<Texture2D> icon = m_SceneMode == Mode::SceneEdit ? m_PlayIcon : m_StopIcon;
            ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));
            if (UI::ImageButton(icon, ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0))
            {
                if (m_SceneMode == Mode::SceneEdit)
                    OnScenePlay();
                else if (m_SceneMode == Mode::ScenePlay)
                    OnSceneStop();
            }
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(3);
            ImGui::End();
        }

        // Draw all panels
        m_PanelManager->OnDrawUI();

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
            Engine::Get().GetUI()->SetBlockEvents(!m_ViewportFocused && !m_ViewportHovered);

            ImVec2 viewportMinRegion = ImGui::GetWindowContentRegionMin();
            ImVec2 viewportMaxRegion = ImGui::GetWindowContentRegionMax();
            ImVec2 viewportOffset = ImGui::GetWindowPos();
            m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
            m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

            ImVec2 windowSize = ImGui::GetWindowSize();
            ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

            m_CurrentScene->SetViewportOffset((u32)viewportOffset.x, (u32)viewportOffset.y);

            if (m_ViewportWidth != windowSize.x || m_ViewportHeight != windowSize.y)
            {
                OnViewportResize((u32)windowSize.x, (u32)windowSize.y);
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
                    else if (path.extension() == ".tprefab" && m_SceneMode == Mode::SceneEdit) // Only in edit mode for now
                    {
                        TBO_VERIFY(AssetManager::DeserializePrefab(path, m_CurrentScene.Get()), "Successfully deserialized prefab!");
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
                            src.SubTexture = SubTexture2D::CreateFromTexture(texture2d);
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
            m_SelectedEntity = m_PanelManager->GetPanel<SceneHierarchyPanel>()->GetSelectedEntity();

            // FIXME: Temporary solution
            if (m_SelectedEntity && m_GizmoType != -1 && m_SceneMode != Mode::ScenePlay)
            {
                glm::mat4 cameraProjection = m_EditorCamera.GetProjection();
                glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();
                if (m_SceneMode == Mode::ScenePlay)
                {
                    Entity cameraEntity = m_RuntimeScene->FindPrimaryCameraEntity();
                    if (cameraEntity)
                    {
                        SceneCamera sceneCamera = cameraEntity.GetComponent<CameraComponent>().Camera;
                        cameraProjection = sceneCamera.GetProjection();
                        cameraView = glm::inverse(cameraEntity.Transform().GetTransform());
                    }
                }

                ImGuizmo::SetOrthographic(false);
                ImGuizmo::SetDrawlist();

                ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);

                // Entity transform
                auto& transformComponent = m_SelectedEntity.Transform();
                glm::mat4 transform = transformComponent.GetTransform();

                // Snapping
                bool snap = Input::IsKeyPressed(Key::LeftControl);
                f32 snapValue = 0.5f;

                if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
                    snapValue = 45.0f;

                f32 snap_values[] = { snapValue, snapValue, snapValue };

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
                if (ImGui::BeginMenu("New"))
                {
                    if (ImGui::MenuItem("Project..."))
                    {
                        m_PanelManager->GetPanel<CreateProjectPopupPanel>()->Open();
                    }
                    if (ImGui::MenuItem("Scene", nullptr, nullptr, m_SceneMode == Mode::SceneEdit))
                    {
                        CreateScene();
                    }

                    ImGui::EndMenu();
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

                if (ImGui::MenuItem("Open C# Project"))
                {
                    // Open specific script
                    if (!Platform::Execute(L"cmd /C start devenv.exe", config.ProjectDirectory / (config.Name + ".sln")))
                    {
                        TBO_ERROR("Failed to open C# script!");
                    }
                }

                if (ImGui::MenuItem("Reload Assembly", "Ctrl+R", nullptr, m_SceneMode == Mode::SceneEdit))
                {
                    Script::ReloadAssemblies();
                }

                if (ImGui::MenuItem("Open solution on start", nullptr, config.OpenSolutionOnStart))
                {
                    config.OpenSolutionOnStart = !config.OpenSolutionOnStart;
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
        ImGui::Separator();

        Scene::Statistics sceneStats = m_CurrentScene->GetStatistics();

        ImGui::Text("Scene: ");
        ImGui::Text("Max Entities %d", sceneStats.MaxEntities);
        ImGui::Text("Current Entities %d", sceneStats.CurrentEntities);
        ImGui::End();

        // End dockspace
        ImGui::End();
    }

    void Editor::OnEvent(Event& e)
    {
        m_PanelManager->OnEvent(e);

        if (m_SceneMode == Mode::SceneEdit && m_ViewportHovered)
            m_EditorCamera.OnEvent(e);

        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<KeyPressedEvent>(TBO_BIND_FN(Editor::OnKeyPressed));
        dispatcher.Dispatch<MouseButtonPressedEvent>(TBO_BIND_FN(Editor::OnMouseButtonPressed));
        dispatcher.Dispatch<WindowCloseEvent>(TBO_BIND_FN(Editor::OnWindowClosed));
        dispatcher.Dispatch<WindowResizeEvent>(TBO_BIND_FN(Editor::OnWindowResize));
    }

    bool Editor::OnKeyPressed(KeyPressedEvent& e)
    {
        if (e.GetRepeatCount() > 1)
            return false;

        bool control = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
        bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);
        bool alt = Input::IsKeyPressed(Key::LeftAlt) || Input::IsKeyPressed(Key::RightAlt);

        bool sceneHierarchyPanelFocused = m_PanelManager->GetPanel<SceneHierarchyPanel>()->IsFocused();
        bool isPlayMode = m_SceneMode == Mode::ScenePlay;

        switch (e.GetKeyCode())
        {
            // Gizmos
            case Key::Q:
            {
                if (!sceneHierarchyPanelFocused && !ImGuizmo::IsUsing())
                    m_GizmoType = -1;
                break;
            }
            case Key::W:
            {
                if (!sceneHierarchyPanelFocused && !ImGuizmo::IsUsing())
                    m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
                break;
            }
            case Key::E:
            {
                if (!sceneHierarchyPanelFocused && !ImGuizmo::IsUsing())
                    m_GizmoType = ImGuizmo::OPERATION::ROTATE;
                break;
            }
            case Key::R:
            {
                if (!sceneHierarchyPanelFocused && !ImGuizmo::IsUsing())
                    m_GizmoType = ImGuizmo::OPERATION::SCALE;
                break;
            }
            case Key::D:
            {
                if (control)
                {
                    m_SelectedEntity = m_PanelManager->GetPanel<SceneHierarchyPanel>()->GetSelectedEntity();
                    if (m_SelectedEntity)
                    {
                        m_SelectedEntity = m_CurrentScene->DuplicateEntity(m_SelectedEntity);
                        m_PanelManager->GetPanel<SceneHierarchyPanel>()->SetSelectedEntity(m_SelectedEntity);
                        break;
                    }
                }
                break;
            }
            case Key::Escape:
            {
                // Set cursor back
                Input::SetCursorMode(CursorMode::Arrow);
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
                if (control)
                    OpenProject();
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

    bool Editor::OnWindowResize(WindowResizeEvent& e)
    {
        //OnViewportResize(m_ViewportX, m_ViewportY, e.GetWidth(), e.GetHeight());
        //m_CurrentScene->SetViewportSize(e.GetWidth(), e.GetHeight());
        return false;
    }

    bool Editor::OnMouseButtonPressed(MouseButtonPressedEvent& e)
    {/*
        if (e.GetMouseButton() == Mouse::ButtonLeft) // TODO: Object picking
        {
            if (m_ViewportHovered && !ImGuizmo::IsOver() && !Input::IsKeyPressed(Key::LeftAlt))
                m_PanelManager->GetPanel<SceneHierarchyPanel>()->SetSelectedEntity(m_HoveredEntity);
        }*/ 
        return false;
    }

    void Editor::CreateProject(std::filesystem::path projectPath)
    {
        // Create and recursively copy template project
        std::filesystem::copy("Resources/NewProjectTemplate", projectPath, std::filesystem::copy_options::recursive);

        std::filesystem::path turboWorkspace = m_CurrentPath.parent_path();
        std::filesystem::path configFile = projectPath / projectPath.stem().concat(".tproject");

        // Format Premake5 
        {
            std::ifstream inStream(projectPath / "premake5.lua");
            TBO_ASSERT(inStream);
            std::stringstream ss;
            ss << inStream.rdbuf();
            inStream.close();

            std::string content = ss.str();
            Utils::FindAndReplace(content, "%PROJECT_NAME%", projectPath.stem());
            Utils::FindAndReplace(content, "%TURBO_PATH%", turboWorkspace);

            std::ofstream outStream(projectPath / "premake5.lua");
            TBO_ASSERT(outStream);
            outStream << content;
            outStream.close();
        }

        // Format .tproject file
        {
            std::ifstream inStream(projectPath / "NewProjectTemplate.tproject");
            TBO_ASSERT(inStream);
            std::stringstream ss;
            ss << inStream.rdbuf();
            inStream.close();

            std::filesystem::remove(projectPath / "NewProjectTemplate.tproject");

            std::string content = ss.str();
            Utils::FindAndReplace(content, "%PROJECT_NAME%", projectPath.stem());

            std::ofstream outStream(configFile);
            TBO_ASSERT(outStream);
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
            TBO_VERIFY(serializer.Deserialize(configFilePath), "Project loaded!");

            // Set it as new active project
            Project::SetActive(project);
        }

        // All panels receive that project has been changed
        m_PanelManager->OnProjectChanged(project);

        const auto& config = project->GetConfig();

        // Building assemblies
        {
            // Execute premake and wait for it to finish
            Platform::Execute(config.ProjectDirectory / TBO_GEN_SOLUTION_FILE, Utils::GetIDEToString(m_CurrentIDE), config.ProjectDirectory, true);

            if (m_CurrentIDE == IDE::VisualStudio2022)
            {
                // Execute MSBuild and wait for it 
                Platform::Execute(m_MSBuildPath, L"", config.ProjectDirectory, true);
            }

            TBO_ASSERT(std::filesystem::exists(config.ProjectDirectory / config.ScriptModulePath), "No assemblies found!");

            // Load project assembly
            Script::LoadProjectAssembly(config.ProjectDirectory / config.ScriptModulePath);
        }

        // Open scene
        OpenScene(config.ProjectDirectory / config.AssetsDirectory / config.StartScenePath);
    }

    void Editor::UpdateWindowTitle()
    {
        auto& project = Project::GetActive();

        if (!project || !m_EditorScene)
            return;

        const std::string& sceneName = m_EditorScenePath.stem().string();
        const std::string& title = fmt::format("TurboEditor | {0} - {1} | Vulkan", project->GetProjectName(), sceneName);
        Engine::Get().GetViewportWindow()->SetTitle(title.data());
    }

    void Editor::OnViewportResize(u32 width, u32 height)
    {
        m_ViewportWidth = width;
        m_ViewportHeight = height;

        m_CurrentScene->SetViewportSize(m_ViewportWidth, m_ViewportHeight);

        m_EditorCamera.SetViewportSize(width, height);
        m_SceneRenderer->OnViewportSize(width, height);
    }

    void Editor::SaveProject()
    {
        SaveScene();

        // Save project
        auto& active = Project::GetActive();
        ProjectSerializer serializer(active);
        TBO_VERIFY(serializer.Serialize(Project::GetProjectConfigPath()), "Saving project");
    }

    void Editor::CreateScene()
    {
        // Save current scene
        SaveScene();

        std::filesystem::path scenePath = Project::GetAssetsPath() / "Scenes\\TestScene.tscene";

        Ref<Scene> newScene = Ref<Scene>::Create();
        newScene->CreateEntity("Start entity!");

        SceneSerializer serializer(newScene);
        TBO_VERIFY(serializer.Serialize(scenePath), "New scene serialization");

        OpenScene(scenePath);
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

        m_SceneMode = Mode::ScenePlay;

        m_RuntimeScene = Scene::Copy(m_EditorScene);
        m_RuntimeScene->OnRuntimeStart();

        // Set current scene
        m_CurrentScene = m_RuntimeScene;

        // Select the same entity in runtime scene
        {
            UUID selectedEntityUUID = 0;
            m_SelectedEntity = m_PanelManager->GetPanel<SceneHierarchyPanel>()->GetSelectedEntity();

            if (m_SelectedEntity)
                selectedEntityUUID = m_SelectedEntity.GetUUID();

            m_SelectedEntity = m_RuntimeScene->FindEntityByUUID(selectedEntityUUID);
        }

        m_PanelManager->SetSceneContext(m_CurrentScene);
        m_PanelManager->GetPanel<SceneHierarchyPanel>()->SetSelectedEntity(m_SelectedEntity);
    }

    void Editor::OnSceneStop()
    {
        m_SceneMode = Mode::SceneEdit;

        m_RuntimeScene->OnRuntimeStop();

        // Copy some scene settings
        // FIXME: Should not be a scene setting anyway -> Rendering
        m_EditorScene->ShowPhysics2DColliders = m_RuntimeScene->ShowPhysics2DColliders;
        m_RuntimeScene = m_EditorScene;

        // Set current scene
        m_CurrentScene = m_EditorScene;

        // Select the same entity in editor scene
        {
            UUID selectedEntityUUID = 0;
            m_SelectedEntity = m_PanelManager->GetPanel<SceneHierarchyPanel>()->GetSelectedEntity();

            if (m_SelectedEntity)
                selectedEntityUUID = m_SelectedEntity.GetUUID();

            m_SelectedEntity = m_EditorScene->FindEntityByUUID(selectedEntityUUID);
        }

        // Reset cursor
        Input::SetCursorMode(CursorMode::Arrow);

        m_PanelManager->SetSceneContext(m_CurrentScene);
        m_PanelManager->GetPanel<SceneHierarchyPanel>()->SetSelectedEntity(m_SelectedEntity);
    }

    void Editor::Close()
    {
        if (m_RuntimeScene)
            OnSceneStop();
        
        // TODO: Editor settings
        SaveProject();
    }
}
