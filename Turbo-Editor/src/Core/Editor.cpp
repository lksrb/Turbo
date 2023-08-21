#include "Editor.h"

#include "EditorIcons.h"

#include "../Panels/QuickAccessPanel.h"
#include "../Panels/ContentBrowserPanel.h"
#include "../Panels/AssetRegistryPanel.h"
#include "../Panels/CreateProjectPopupPanel.h"
#include "../Panels/AssetEditorPanel.h"

#include <Turbo/Editor/SceneHierarchyPanel.h>
#include <Turbo/Editor/EditorConsolePanel.h>

#include <Turbo/Audio/Audio.h>
#include <Turbo/Asset/AssetManager.h>
#include <Turbo/Debug/ScopeTimer.h>
#include <Turbo/Script/Script.h>
#include <Turbo/Scene/SceneSerializer.h>
#include <Turbo/Solution/ProjectSerializer.h>

#include <Turbo/Renderer/Font.h>
#include <Turbo/UI/UI.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <filesystem>
#include <sstream>

#ifdef TBO_PLATFORM_WIN32
#define TBO_VS2022_REGISTRY_KEY L"SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\App Paths\\devenv.exe"
#define TBO_GEN_SOLUTION_FILE "Win32-GenerateSolution.bat"
#endif

namespace Turbo::Ed {

    namespace Utils {

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
                    if (filter(msBuildPath) || msBuildPath == L"C:/")
                        break;

                    msBuildPath = msBuildPath.parent_path();
                }

                if (msBuildPath != L"C:/")
                {
                    return msBuildPath / "MSBuild/Current/Bin/MSBuild.exe";
                }
            }

            TBO_ASSERT(false, "Could not find MSBuild!");

            return {};
        }

        static const wchar_t* GetIDEToString(Editor::DevEnv ide)
        {
            switch (ide)
            {
                case Editor::DevEnv::None: return L"none";
                case Editor::DevEnv::VS2022: return L"vs2022";
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
        m_PanelManager = CreateOwned<PanelManager>();
        m_PanelManager->AddPanel<SceneHierarchyPanel>();
        //m_PanelManager->AddPanel<QuickAccessPanel>();
        m_PanelManager->AddPanel<ContentBrowserPanel>();
        m_PanelManager->AddPanel<EditorConsolePanel>();
        m_PanelManager->AddPanel<AssetRegistryPanel>([this](AssetHandle handle) // TODO: AssetRegistryPanel should own AssetEditorPanel
        {
            m_PanelManager->GetPanel<AssetEditorPanel>()->OpenAsset(handle);
        });
        m_PanelManager->AddPanel<AssetEditorPanel>();
        m_PanelManager->AddPanel<CreateProjectPopupPanel>(TBO_BIND_FN(Editor::CreateProject));

        // Render 
        m_ViewportDrawList = Ref<SceneDrawList>::Create(m_ViewportWidth, m_ViewportHeight);

        // TODO: Find some better icons
        m_PlayIcon = Texture2D::Create(Icons::PlayButton);
        m_StopIcon = Texture2D::Create(Icons::StopButton);
        m_Reset2DIcon = Texture2D::Create(Icons::Reset2DButton);
        m_CameraIcon = Texture2D::Create(Icons::Camera);
        m_DirectionalLightIcon = Texture2D::Create(Icons::DirectionalLight);
        m_PointLightIcon = Texture2D::Create(Icons::PointLight);
        m_SpotLightIcon = Texture2D::Create(Icons::SpotLight);

        m_EditorCamera = EditorCamera(30.0f, static_cast<f32>(m_ViewportWidth) / static_cast<f32>(m_ViewportHeight), 0.1f, 10000.0f);

        // Open sandbox project
        OpenProject(m_CurrentPath / "Mystery/Mystery.tproject");
    }

    void Editor::OnShutdown()
    {
        // Reset
        Project::SetActive(nullptr);
    }

    void Editor::OnUpdate()
    {
        m_ViewportDrawList->Begin();

        switch (m_SceneMode)
        {
            case SceneMode::Edit:
            {
                if (m_ViewportHovered)
                {
                    m_EditorCamera.OnUpdate(m_Time.DeltaTime);
                }

                m_CurrentScene->OnEditorUpdate(m_ViewportDrawList, m_EditorCamera, m_Time.DeltaTime);
                break;
            }
            case SceneMode::Play:
            {
                m_CurrentScene->OnRuntimeUpdate(m_ViewportDrawList, m_Time.DeltaTime);
                break;
            }
        }

        // Render debug lines
        OnOverlayRender();

        m_ViewportDrawList->End();
    }

    void Editor::OnDrawUI()
    {
        // Dockspace
        ImGuiDockNodeFlags dockSpaceFlags = ImGuiDockNodeFlags_None;

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
        if (dockSpaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
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
            //UI::OffsetCursorPosY(100);
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockSpaceFlags);
        }
        style.WindowMinSize.x = minWinSizeX;
        // --Dockspace

        // Toolbar
        {
            ImGuiWindowClass windowClass;
            windowClass.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
            ImGui::SetNextWindowClass(&windowClass);

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
            Ref<Texture2D> icon = m_SceneMode == SceneMode::Edit ? m_PlayIcon : m_StopIcon;
            ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));
            if (UI::ImageButton(icon, ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0))
            {
                if (m_SceneMode == SceneMode::Edit)
                    OnScenePlay();
                else if (m_SceneMode == SceneMode::Play)
                    OnSceneStop();
            }

            ImGui::SameLine();

            if (UI::ImageButton(m_Reset2DIcon, ImVec2(size, size), ImVec2(0, 1), ImVec2(1, 0), 0))
            {
                m_EditorCamera.ResetRotation();
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

            ImGuiWindowClass windowClass;
            windowClass.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
            ImGui::SetNextWindowClass(&windowClass);

            ImGui::Begin("Viewport");
            ImGui::PopStyleVar();

            m_ViewportHovered = ImGui::IsWindowHovered();
            m_ViewportFocused = ImGui::IsWindowFocused();
            Engine::Get().SetUIBlockEvents(!m_ViewportFocused && !m_ViewportHovered);

            ImVec2 viewportMinRegion = ImGui::GetWindowContentRegionMin();
            ImVec2 viewportMaxRegion = ImGui::GetWindowContentRegionMax();
            ImVec2 viewportOffset = ImGui::GetWindowPos();
            m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
            m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

            ImVec2 windowSize = ImGui::GetWindowSize();
            ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
            ImVec2 viewportPos = ImGui::GetMainViewport()->Pos;

            ImVec2 absoluteWindowsPos = { viewportOffset.x - viewportPos.x, viewportOffset.y - viewportPos.y };
            m_CurrentScene->SetViewportOffset((i32)absoluteWindowsPos.x, (i32)absoluteWindowsPos.y);

            auto [mx, my] = ImGui::GetMousePos();
            mx -= m_ViewportBounds[0].x;
            my -= m_ViewportBounds[0].y;
            glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
            my = viewportSize.y - my;
            i32 mouseX = (i32)mx;
            i32 mouseY = (i32)my;

            m_HoveredEntity = {};
            if (mouseX >= 0 && mouseY >= 0 && mouseX < (i32)m_ViewportWidth && mouseY < (i32)m_ViewportHeight)
            {
                i32 entityID = m_ViewportDrawList->ReadPixel(mouseX, mouseY);
                m_HoveredEntity = entityID != -1 ? Entity{ (entt::entity)entityID, m_CurrentScene.Get() } : Entity{};
            }
            if (m_ViewportWidth != viewportPanelSize.x || m_ViewportHeight != viewportPanelSize.y)
            {
                OnViewportResize((u32)viewportPanelSize.x, (u32)viewportPanelSize.y);
            }

            if (m_EditorScene)
            {
                UI::Image(m_ViewportDrawList->GetFinalImage(), viewportPanelSize, { 0, 1 }, { 1, 0 });
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
                    else if (path.extension() == ".tprefab" && m_SceneMode == SceneMode::Edit) // Only in edit mode for now
                    {
                        TBO_VERIFY(AssetManager::DeserializePrefab(path, m_CurrentScene.Get()), "Successfully deserialized prefab!");
                    }
                }

                ImGui::EndDragDropTarget();
            }

            // Gizmos
            // 
            // FIXME: Temporary solution
            if (m_SelectedEntity && m_GizmoType != -1 && m_SceneMode != SceneMode::Play)
            {
                glm::mat4 cameraProjection = m_EditorCamera.GetProjection();
                glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();
                if (m_SceneMode == SceneMode::Play)
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
                glm::mat4 transform = m_CurrentScene->GetWorldSpaceTransformMatrix(m_SelectedEntity);

                // Snapping
                bool snap = Input::IsKeyPressed(Key::LeftControl);
                f32 snapValue = 0.5f;

                if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
                    snapValue = 45.0f;

                f32 snap_values[] = { snapValue, snapValue, snapValue };
                ImGuizmo::AllowAxisFlip(false);
                ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
                (ImGuizmo::OPERATION)m_GizmoType, static_cast<ImGuizmo::MODE>(m_GizmoMode), glm::value_ptr(transform),
                nullptr, snap ? snap_values : nullptr);

                if (ImGuizmo::IsUsing())
                {
                    Entity parent = m_CurrentScene->FindEntityByUUID(m_SelectedEntity.GetParentUUID());

                    TransformComponent& entityTransform = m_SelectedEntity.Transform();

                    if (parent)
                    {
                        glm::mat4 parentTransform = m_CurrentScene->GetWorldSpaceTransformMatrix(parent);
                        // Cancel parents transforms
                        transform = glm::inverse(parentTransform) * transform;
                    }

                    glm::vec3 translation, rotation, scale;
                    Math::DecomposeTransform(transform, translation, rotation, scale);

                    glm::vec3 deltaRotation = rotation - entityTransform.Rotation;
                    entityTransform.Translation = translation;
                    entityTransform.Rotation += deltaRotation;
                    entityTransform.Scale = scale;
                }
            }

            ImGui::End();
        } // End viewport

        // Menu
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::BeginMenu("New"))
                {
                    bool isInEditMode = m_SceneMode == SceneMode::Edit;

                    if (ImGui::MenuItem("Project...", nullptr, nullptr, isInEditMode))
                    {
                        m_PanelManager->GetPanel<CreateProjectPopupPanel>()->Open();
                    }
                    if (ImGui::MenuItem("Scene", nullptr, nullptr, isInEditMode))
                    {
                        NewScene();
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

                if (ImGui::MenuItem("Reload Assembly", "Ctrl+R", nullptr, m_SceneMode == SceneMode::Edit))
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
                ImGui::MenuItem("ImGui demo window", nullptr, &m_ShowDemoWindow);
                if (ImGui::MenuItem("Asset Registry"))
                {
                    m_PanelManager->GetPanel<AssetRegistryPanel>()->Open();
                }
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        // TODO: Figure out where this goes
        m_SelectedEntity = m_PanelManager->GetPanel<SceneHierarchyPanel>()->GetSelectedEntity();

        if (m_ShowDemoWindow)
            ImGui::ShowDemoWindow();

        if constexpr (false)
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
            {
                ImGui::Checkbox("Physics Colliders", &m_ShowPhysicsColliders);
                ImGui::Checkbox("Show Scene Icons", &m_ShowSceneIcons);
            }

            ImGui::End();
        }

        ImGui::Begin("Statistics & Renderer");
        ImGui::Text("Hovered entity: %s", m_HoveredEntity ? m_HoveredEntity.GetName().c_str() : "");
        ImGui::Text("Timestep: %.5f ms", m_Time.DeltaTime.ms());
        ImGui::Text("StartTime: %.5f ms", m_Time.TimeSinceStart.ms());
        ImGui::Separator();

        // TODO: Better statistics
        auto stats = m_ViewportDrawList->GetStatistics();
        ImGui::Text("Quad Count: %d", stats.Statistics2D.QuadCount);
        ImGui::Text("Drawcalls: %d", stats.Statistics2D.DrawCalls + stats.DrawCalls);
        ImGui::Text("Instances: %d", stats.Instances);
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

        if (m_SceneMode == SceneMode::Edit && m_ViewportHovered)
            m_EditorCamera.OnEvent(e);

        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<KeyPressedEvent>(TBO_BIND_FN(Editor::OnKeyPressed));
        dispatcher.Dispatch<MouseButtonPressedEvent>(TBO_BIND_FN(Editor::OnMouseButtonPressed));
        dispatcher.Dispatch<WindowCloseEvent>(TBO_BIND_FN(Editor::OnWindowClosed));
        dispatcher.Dispatch<WindowResizeEvent>(TBO_BIND_FN(Editor::OnWindowResize));
    }

    bool Editor::OnKeyPressed(KeyPressedEvent& e)
    {
        bool isRepeated = e.IsRepeated();
        bool control = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
        bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);
        bool alt = Input::IsKeyPressed(Key::LeftAlt) || Input::IsKeyPressed(Key::RightAlt);

        bool isPlayMode = m_SceneMode == SceneMode::Play;

        switch (e.GetKeyCode())
        {
            // Gizmos
            case Key::Q:
            {
                if (m_ViewportHovered && !ImGuizmo::IsUsing())
                    m_GizmoType = -1;
                break;
            }
            case Key::W:
            {
                if (m_ViewportHovered && !ImGuizmo::IsUsing())
                    m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
                break;
            }
            case Key::E:
            {
                if (m_ViewportHovered && !ImGuizmo::IsUsing())
                    m_GizmoType = ImGuizmo::OPERATION::ROTATE;
                break;
            }
            case Key::R:
            {
                if (m_ViewportHovered && !ImGuizmo::IsUsing())
                    m_GizmoType = ImGuizmo::OPERATION::SCALE;
                break;
            }
            case Key::F:
            {
                if (m_SceneMode == SceneMode::Edit && !isRepeated && m_SelectedEntity && m_ViewportFocused)
                {
                    m_EditorCamera.Focus(m_CurrentScene->GetWorldSpaceTransform(m_SelectedEntity).Translation);
                }

                break;
            }
            case Key::D:
            {
                if (!isRepeated && control)
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
            case Key::M:
            {
                if (!isRepeated && m_ViewportFocused)
                {
                    m_GizmoMode = (m_GizmoMode + 1) % 2;
                }
                break;
            }

            case Key::X:
            {
                if (m_SceneMode == SceneMode::Edit && !isRepeated && control)
                {
                    m_SelectedEntity = m_PanelManager->GetPanel<SceneHierarchyPanel>()->GetSelectedEntity();
                    if (m_SelectedEntity)
                    {
                        m_CurrentScene->DestroyEntity(m_SelectedEntity);
                        m_SelectedEntity = {};
                        m_HoveredEntity = {};
                        m_PanelManager->GetPanel<SceneHierarchyPanel>()->SetSelectedEntity({});
                    }
                }
                break;
            }
            case Key::Escape:
            {
                // Set cursor back
                Input::SetCursorMode(CursorMode_Normal);

                break;
            }
            // Menu
            case Key::S:
            {
                if (!isRepeated)
                {
                    if (control && shift)
                        SaveSceneAs();
                    else if (control)
                        SaveScene();

                }
                break;
            }
            case Key::O:
            {
                if (!isRepeated && control)
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
        return false;
    }

    bool Editor::OnMouseButtonPressed(MouseButtonPressedEvent& e)
    {
        if (e.GetMouseButton() == Mouse::ButtonLeft)
        {
            if (m_ViewportHovered && (ImGuizmo::IsOver() == false || m_GizmoType == -1) && !Input::IsKeyPressed(Key::LeftAlt) && m_SceneMode == SceneMode::Edit)
            {
                ImGui::ClearActiveID();

                if (m_HoveredEntity)
                {
                    m_SelectedEntity = m_HoveredEntity;
                    m_PanelManager->GetPanel<SceneHierarchyPanel>()->SetSelectedEntity(m_SelectedEntity);
                }
            }
        }
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
        // Opens platform specific 
        if (configFilePath.empty())
        {
            configFilePath = Platform::OpenFileDialog(L"Open Project", L"Turbo Project (*.tproject)\0*.tproject\0");

            if (configFilePath.empty())
                return;
        }

        // Project deserialization
        auto project = Ref<Project>::Create();
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
            // Execute premake and wait for it to finish"
            Platform::Execute(config.ProjectDirectory / TBO_GEN_SOLUTION_FILE, Utils::GetIDEToString(m_CurrentIDE), config.ProjectDirectory, true);

            if (m_CurrentIDE == DevEnv::VS2022)
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
        auto project = Project::GetActive();

        if (!project || !m_EditorScene)
            return;

        std::string sceneName = m_EditorScenePath.stem().string();
        std::string title = std::format("TurboEditor | {0} - {1} | Vulkan", project->GetProjectName(), sceneName);
        Engine::Get().GetViewportWindow()->SetTitle(title.data());
    }

    void Editor::OnViewportResize(u32 width, u32 height)
    {
        m_ViewportWidth = width;
        m_ViewportHeight = height;

        m_CurrentScene->SetViewportSize(m_ViewportWidth, m_ViewportHeight);

        m_EditorCamera.SetViewportSize(width, height);
        m_ViewportDrawList->OnViewportResize(width, height);

        TBO_WARN("Viewport resized! {} {}", width, height);
    }

    void Editor::SaveProject()
    {
        SaveScene();

        // Save project
        auto project = Project::GetActive();
        ProjectSerializer serializer(project);
        TBO_VERIFY(serializer.Serialize(Project::GetProjectConfigPath()), "[Save Project] Successfully serialized project!");
    }

    void Editor::NewScene()
    {
        // Save current scene
        SaveScene();

        std::filesystem::path scenePath = Project::GetAssetsPath() / "Scenes/TestScene.tscene";

        Ref<Scene> newScene = Ref<Scene>::Create();
        newScene->CreateEntity("Start entity!");

        SceneSerializer serializer(newScene);
        TBO_VERIFY(serializer.Serialize(scenePath), "[NewScene] Successfully serialized scene!");

        OpenScene(scenePath);
    }

    void Editor::SaveScene()
    {
        auto project = Project::GetActive();

        if (!project)
        {
            TBO_ERROR("[SaveScene] No project loaded!");
            return;
        }

        if (m_EditorScenePath.empty())
        {
            SaveSceneAs();
            return;
        }

        SceneSerializer serializer(m_EditorScene);
        if (serializer.Serialize(m_EditorScenePath.string()))
        {
            TBO_INFO("[SaveScene] Successfully serialized to \"{0}\"!", m_EditorScenePath.stem().string());
            return;
        }

        TBO_ERROR("[SaveScene] Could not serialize to \"{0}\"!", m_EditorScenePath);
    }

    void Editor::OpenScene(std::filesystem::path filepath)
    {
        if (filepath.empty())
        {
            filepath = Platform::OpenFileDialog(L"Open Scene", L"Turbo Scene (*.tscene)\0*.tscene\0");

            if (filepath.empty())
            {
                TBO_WARN("[OpenScene] No scene selected!");
                return;
            }
        }

        m_EditorScene = Ref<Scene>::Create(true);

        // Set current scene
        m_CurrentScene = m_EditorScene;

        SceneSerializer serializer(m_EditorScene);
        TBO_VERIFY(serializer.Deserialize(filepath), "[OpenScene] Successfully deserialized scene!");

        m_EditorScene->SetViewportSize(m_ViewportWidth, m_ViewportHeight);

        m_PanelManager->SetSceneContext(m_EditorScene);

        m_EditorScenePath = filepath;

        UpdateWindowTitle();
    }

    void Editor::SaveSceneAs()
    {
        auto project = Project::GetActive();

        if (!project)
        {
            TBO_ERROR("[SaveSceneAs] No project loaded!");
            return;
        }

        if (!m_EditorScene)
        {
            TBO_ERROR("[SaveSceneAs] No scene loaded!");
            return;
        }

        auto filepath = Platform::SaveFileDialog("Turbo Scene (*.tscene)\0*.tscene\0");

        if (!filepath.empty())
        {
            if (filepath.extension() != ".tscene")
                filepath.concat(".tscene");

            SceneSerializer serializer(m_EditorScene);

            if (serializer.Serialize(filepath))
            {
                TBO_INFO("[SaveSceneAs] Successfully serialized scene to {0}!", filepath);
                UpdateWindowTitle();
            }

            return;
        }

        TBO_ERROR("Could not serialize scene to \"{0}\"!", filepath);
    }

    void Editor::OnScenePlay()
    {
        // Clear logs
        EditorConsolePanel::Clear();

        m_SceneMode = SceneMode::Play;

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
        m_SceneMode = SceneMode::Edit;

        m_RuntimeScene->OnRuntimeStop();
        m_RuntimeScene.Reset();

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
        Input::SetCursorMode(CursorMode_Normal);

        m_PanelManager->SetSceneContext(m_CurrentScene);
        m_PanelManager->GetPanel<SceneHierarchyPanel>()->SetSelectedEntity(m_SelectedEntity);
    }

    void Editor::Close()
    {
        if (m_SceneMode == SceneMode::Play)
        {
            OnSceneStop();
        }

        // TODO: Editor settings
        SaveProject();
    }

    void Editor::OnOverlayRender()
    {
        if (m_ShowPhysicsColliders)
        {
            // Box2D colliders
            auto box2dColliders = m_CurrentScene->GetAllEntitiesWith<BoxCollider2DComponent>();
            for (auto entity : box2dColliders)
            {
                auto& bc2d = box2dColliders.get<BoxCollider2DComponent>(entity);
                TransformComponent transform = m_CurrentScene->GetWorldSpaceTransform({ entity, m_CurrentScene.Get() });

                glm::vec3 translation = transform.Translation + glm::vec3(bc2d.Offset, 0.0f);
                glm::vec3 scale = transform.Scale * glm::vec3(bc2d.Size * 2.0f, 1.0f);

                glm::mat4 offsetTransform = glm::translate(glm::mat4(1.0f), translation)
                    * glm::rotate(glm::mat4(1.0f), transform.Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f))
                    * glm::scale(glm::mat4(1.0f), scale);

                m_ViewportDrawList->AddRect(offsetTransform, { 0.0f, 1.0f, 0.0f, 1.0f }, (i32)entity);
            }

            // Circle2D colliders
            auto circle2dColliders = m_CurrentScene->GetAllEntitiesWith<CircleCollider2DComponent>();
            for (auto entity : circle2dColliders)
            {
                auto& cc2d = circle2dColliders.get<CircleCollider2DComponent>(entity);
                TransformComponent transform = m_CurrentScene->GetWorldSpaceTransform({ entity, m_CurrentScene.Get() });

                glm::vec3 translation = transform.Translation + glm::vec3(cc2d.Offset, 0.0f);

                m_ViewportDrawList->AddDebugCircle(translation, glm::vec3(0.0f), transform.Scale.x * cc2d.Radius + 0.003f, { 0.0f, 1.0f, 0.0f, 1.0f }, (i32)entity);
            }

            // Box colliders
            auto boxColliders = m_CurrentScene->GetAllEntitiesWith<BoxColliderComponent>();
            for (auto entity : boxColliders)
            {
                auto& bc = boxColliders.get<BoxColliderComponent>(entity);

                TransformComponent transform = m_CurrentScene->GetWorldSpaceTransform({ entity, m_CurrentScene.Get() });

                glm::vec3 translation = transform.Translation + bc.Offset;
                glm::mat4 rotation = glm::toMat4(glm::quat(transform.Rotation));
                glm::vec3 scale = transform.Scale * (bc.Size * 2.0f);

                glm::mat4 offsetTransform = glm::translate(glm::mat4(1.0f), translation)
                    * rotation
                    * glm::scale(glm::mat4(1.0f), scale);

                m_ViewportDrawList->AddBoxWireframe(transform.GetTransform(), {0.0f, 1.0f, 0.0f, 1.0f}, (i32)entity);
            }

            // Sphere colliders
            auto sphereColliders = m_CurrentScene->GetAllEntitiesWith<SphereColliderComponent>();
            for (auto entity : sphereColliders)
            {
                auto& sc = sphereColliders.get<SphereColliderComponent>(entity);
                TransformComponent transform = m_CurrentScene->GetWorldSpaceTransform({ entity, m_CurrentScene.Get() });

                glm::vec3 translation = transform.Translation /* + sc.Offset */;

                // Facing forward, up, right
                m_ViewportDrawList->AddDebugCircle(translation, glm::vec3(0.0f), transform.Scale.x * sc.Radius + 0.003f, { 0.0f, 1.0f, 0.0f, 1.0f }, (i32)entity);
                m_ViewportDrawList->AddDebugCircle(translation, glm::vec3(glm::radians(90.0f), 0.0f, 0.0f), transform.Scale.x * sc.Radius + 0.003f, { 0.0f, 1.0f, 0.0f, 1.0f }, (i32)entity);
                m_ViewportDrawList->AddDebugCircle(translation, glm::vec3(0.0f, glm::radians(90.0f), 0.0f), transform.Scale.x * sc.Radius + 0.003f, { 0.0f, 1.0f, 0.0f, 1.0f }, (i32)entity);
            }
#if 1
            // Capsule colliders
            auto capsuleColliders = m_CurrentScene->GetAllEntitiesWith<CapsuleColliderComponent>();
            for (auto entity : capsuleColliders)
            {
                auto& cc = capsuleColliders.get<CapsuleColliderComponent>(entity);
                TransformComponent transform = m_CurrentScene->GetWorldSpaceTransform({ entity, m_CurrentScene.Get() });
                {
                    glm::vec3 translation = transform.Translation /* + sc.Offset */;
                    glm::mat4 rotation = glm::toMat4(glm::quat(transform.Rotation + glm::vec3(glm::pi<f32>() * 0.5f, 0.0f, 0.0f)));
                    glm::mat4 offsetTransform = glm::translate(glm::mat4(1.0f), translation)
                        * rotation
                        * glm::scale(glm::mat4(1.0f), glm::vec3(transform.Scale.x * cc.Radius + 0.003f));

                    m_ViewportDrawList->AddDebugCircle(offsetTransform, { 0.0f, 1.0f, 0.0f, 1.0f }, (i32)entity);
                }

                // TODO: Create elipse debug outline
                //m_ViewportDrawList->AddDebugElipse(translation, );
            }
#endif
        }

        // Icons
        if (m_ShowSceneIcons)
        {
            // Cameras
            auto cameras = m_CurrentScene->GetAllEntitiesWith<CameraComponent>();
            for (auto e : cameras)
            {
                Entity entity = { e , m_CurrentScene.Get() };
                m_ViewportDrawList->AddBillboardQuad(m_CurrentScene->GetWorldSpaceTransform(entity).Translation, { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, m_CameraIcon, 1.0f, (i32)e);
            }

            // Directional lights
            auto directionalLights = m_CurrentScene->GetAllEntitiesWith<DirectionalLightComponent>();
            for (auto e : directionalLights)
            {
                Entity entity = { e , m_CurrentScene.Get() };
                // TODO: Outlines
                //m_ViewportDrawList->AddCircle(offsetTransform, { 0.0f, 1.0f, 0.0f, 1.0f }, 0.035f, 0.005f, (i32)entity);
                m_ViewportDrawList->AddBillboardQuad(m_CurrentScene->GetWorldSpaceTransform(entity).Translation, { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, m_DirectionalLightIcon, 1.0f, (i32)e);
            }

            // Point lights
            auto pointlights = m_CurrentScene->GetAllEntitiesWith<PointLightComponent>();
            for (auto e : pointlights)
            {
                Entity entity = { e , m_CurrentScene.Get() };
                // TODO: Outlines
                //m_ViewportDrawList->AddCircle(offsetTransform, { 0.0f, 1.0f, 0.0f, 1.0f }, 0.035f, 0.005f, (i32)entity);
                m_ViewportDrawList->AddBillboardQuad(m_CurrentScene->GetWorldSpaceTransform(entity).Translation, { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, m_PointLightIcon, 1.0f, (i32)e);
            }

            // Spotlights
            auto spotlights = m_CurrentScene->GetAllEntitiesWith<SpotLightComponent>();
            for (auto e : spotlights)
            {
                Entity entity = { e , m_CurrentScene.Get() };
                m_ViewportDrawList->AddBillboardQuad(m_CurrentScene->GetWorldSpaceTransform(entity).Translation, { 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, m_SpotLightIcon, 1.0f, (i32)e);
            }
        }
    }

}
