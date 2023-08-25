#include "ContentBrowserPanel.h"

#include "../Core/EditorIcons.h"

#include <Turbo/UI/UI.h>
#include <Turbo/Core/Platform.h>
#include <Turbo/Scene/Scene.h>
#include <Turbo/Scene/Entity.h>
#include <Turbo/Scene/Prefab.h>
#include <Turbo/Renderer/Mesh.h>
#include <Turbo/Asset/AssetManager.h>

#include <filesystem>

namespace Turbo::Ed {

    ContentBrowserPanel::ContentBrowserPanel()
    {
        m_DirectoryIcon = Texture2D::Create(Icons::Directory);
        m_FileIcon = Texture2D::Create(Icons::File);
    }

    ContentBrowserPanel::~ContentBrowserPanel()
    {
    }

    void ContentBrowserPanel::OnDrawUI()
    {
        ImGui::Begin("Content Browser");

        if (m_CurrentDirectory != m_BasePath)
        {
            if (ImGui::Button("<-"))
            {
                m_CurrentDirectory = std::filesystem::path(m_CurrentDirectory.c_str()).parent_path().string();
            }

            ImGui::SameLine();
        }

        ImGui::Checkbox("Show all files", &m_ShowAllFiles);

        if (UI::BeginDragDropTargetWindow())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SHP_DATA"))
            {
                Entity entity = *(Entity*)payload->Data;
                if (entity)
                {

                    if (!entity.HasComponent<PrefabComponent>())
                    {
                        auto assetRegistry = Project::GetActive()->GetEditorAssetRegistry();
                        auto prefabPath = m_CurrentDirectory / std::format("{}.tprefab", entity.GetName());

                        auto& prefabComponent = entity.AddComponent<PrefabComponent>();
                        prefabComponent.Handle = assetRegistry->CreateAsset2<Prefab>(prefabPath, entity)->Handle;
                    }
                    else
                    {
                        // TODO: Probably add this as new function to AssetManager
                        auto& prefabComponent = entity.GetComponent<PrefabComponent>();
                        Ref<Prefab> asset = AssetManager::GetAsset<Prefab>(prefabComponent.Handle);

                        if (asset)
                        {
                            Asset::Serialize(AssetManager::GetAssetMetadata(prefabComponent.Handle), asset);
                            TBO_ENGINE_WARN("Serialized prefab '{}'!", prefabComponent.Handle);
                        }
                        else
                        {
                            TBO_ENGINE_ERROR("Failed to serialize prefab '{}'!", prefabComponent.Handle);
                        }

                    }
                }
            }

            UI::EndDragDropTargetWindow();
        }

        static f32 padding = 16.0f;
        static f32 thumbnailSize = 128;
        f32 cellSize = thumbnailSize + padding;

        f32 panelWidth = ImGui::GetContentRegionAvail().x;
        i32 columnCount = (i32)(panelWidth / cellSize);
        if (columnCount < 1)
            columnCount = 1;

        ImGui::Columns(columnCount, 0, false);

        static std::filesystem::path s_SelectedPath;

        bool wasAnyItemHovered = false;
        for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
        {
            const auto& path = directoryEntry.path();
            bool isDirectory = directoryEntry.is_directory();

            // Filter
            if (!isDirectory && Filter(path.extension()))
                continue;

            auto relativePath = std::filesystem::relative(path, m_BasePath.c_str());
            std::string filenameString = relativePath.filename().string();
            bool selected = s_SelectedPath == path;

            ImGui::PushID(filenameString.c_str());
            Ref<Texture2D> icon = isDirectory ? m_DirectoryIcon : m_FileIcon;

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            UI::ScopedStyleColor buttonColor(ImGuiCol_Button, { 0.2f, 0.3f, 0.8f, 0.8f }, selected);
            UI::ScopedStyleColor buttonHoveredColor(ImGuiCol_ButtonHovered, { 0.2f, 0.3f, 0.8f, 0.6f }, selected);
            if (UI::ImageButton(icon, { thumbnailSize, thumbnailSize }, ImVec2(0, 1), ImVec2(1, 0)))
            {
                s_SelectedPath = path;
            }
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
            {
                const wchar_t* itemPath = path.c_str();
                size_t itemPathSize = (wcslen(itemPath) + 1) * sizeof(wchar_t);
                if (path.extension() == ".cs" || path.extension() == ".png" || path.extension() == ".jpg")
                {
                    ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM_SHP", itemPath, itemPathSize, ImGuiCond_Always);
                }
                else if (path.extension() == ".tscene" || path.extension() == ".tprefab")
                {
                    ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM_VIEWPORT", itemPath, itemPathSize, ImGuiCond_Always);
                }
                ImGui::EndDragDropSource();
            }

            ImGui::PopStyleColor();

            if (ImGui::IsItemHovered())
            {
                wasAnyItemHovered = true;
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    s_SelectedPath = "";

                    if (directoryEntry.is_directory())
                    {
                        m_CurrentDirectory /= path.filename().string().c_str();
                    }
                    else
                    {
                        if (path.extension() == ".cs")
                        {
                            // Open specific script
                            if (!Platform::Execute(L"cmd /C start devenv.exe /Edit", path))
                            {
                                TBO_ERROR("Failed to open C# script!");
                            }
                        }
                    }
                }
            }

            ImGui::TextWrapped(filenameString.c_str());

            ImGui::NextColumn();

            ImGui::PopID();
        }

        if (!wasAnyItemHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            s_SelectedPath = "";
        }

        //ImGui::Columns();
        //ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
        //ImGui::SliderFloat("Thumbnail Padding", &padding, 0, 32);

        if (ImGui::BeginPopupContextWindow("##ContentBrowserPanel"))
        {
            if (ImGui::MenuItem("Open in Explorer"))
            {
                Platform::OpenFileExplorer(m_CurrentDirectory);
            }

            if (ImGui::BeginMenu("Import"))
            {
                if (ImGui::MenuItem("Static Mesh"))
                {
                    auto filepath = Platform::OpenFileDialog(L"Import Asset", L"Assets (*.fbx)\0*.fbx\0", m_CurrentDirectory);
                    Project::GetActive()->GetEditorAssetRegistry()->ImportAsset2<MeshSource>(filepath);
                }

                if (ImGui::MenuItem("Texture2D"))
                {
                    auto filepath = Platform::OpenFileDialog(L"Import Asset", L"Assets (*.png)\0*.png\0", m_CurrentDirectory);
                    Project::GetActive()->GetEditorAssetRegistry()->ImportAsset2<Texture2D>(filepath);
                }

                ImGui::EndMenu();
            }

            ImGui::EndPopup();
        }

        ImGui::End();
    }

    void ContentBrowserPanel::OnProjectChanged(const Ref<Project>& project)
    {
        m_CurrentDirectory = m_BasePath = Project::GetAssetsPath();
    }

    void ContentBrowserPanel::OnSceneContextChanged(const Ref<Scene>& context)
    {
        m_SceneContext = context;
    }

    bool ContentBrowserPanel::Filter(const std::filesystem::path& extension)
    {
        static std::unordered_set<std::filesystem::path> s_PermitLookup{ ".tmesh", ".tscene", ".tprefab", ".png", ".jpg", ".cs", ".ttex" };

        return !m_ShowAllFiles && s_PermitLookup.find(extension) == s_PermitLookup.end();
    }

}
