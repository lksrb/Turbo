#include "ContentBrowserPanel.h"

#include <Turbo/UI/UI.h>
#include <Turbo/Core/Platform.h>
#include <Turbo/Solution/Project.h>
#include <Turbo/Scene/Scene.h>
#include <Turbo/Scene/Entity.h>
#include <Turbo/Asset/AssetManager.h>

#include <filesystem>

namespace Turbo::Ed
{
 
    ContentBrowserPanel::ContentBrowserPanel()
    {
        m_DirectoryIcon = Texture2D::Create("Resources/Icons/DirectoryIcon.png");
        m_FileIcon = Texture2D::Create("Resources/Icons/FileIcon.png");
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
        }

        ImGuiWindow* window = ImGui::GetCurrentWindow();
        ImRect windowContent = window->ContentRegionRect;

        // Handle scrolling
        windowContent.Max.y = window->ContentRegionRect.Max.y + window->Scroll.y;
        windowContent.Min.y = window->ContentRegionRect.Min.y + window->Scroll.y;

        // Whole window has this ability
        if (ImGui::BeginDragDropTargetCustom(windowContent, window->ID))
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SHP_DATA"))
            {
                Entity entity = *(Entity*)payload->Data;
                if (entity)
                {
                    // Serialize every component except
                    if (AssetManager::SerializeToPrefab(m_CurrentDirectory, entity))
                    {
                        TBO_ENGINE_INFO("Successfully serialized prefab!");
                    }
                }
            }

            ImGui::EndDragDropTarget();
        }

        static f32 padding = 16.0f;
        static f32 thumbnail_size = 128;
        f32 cellSize = thumbnail_size + padding;

        f32 panelWidth = ImGui::GetContentRegionAvail().x;
        i32 columnCount = (i32)(panelWidth / cellSize);
        if (columnCount < 1)
            columnCount = 1;

        ImGui::Columns(columnCount, 0, false);

        for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory.c_str()))
        {
            const auto& path = directoryEntry.path();

            // Filter
            if (path.extension() == ".csproj" || path.extension() == ".user")
                continue;

            const auto& relativePath = std::filesystem::relative(path, m_BasePath.c_str());
            const std::string& filenameString = relativePath.filename().string();

            ImGui::PushID(filenameString.c_str());
            Ref<Texture2D> icon = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon;

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            UI::ImageButton(icon, { thumbnail_size, thumbnail_size }, { 0,1 }, { 1,0 });
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

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
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
            ImGui::TextWrapped(filenameString.c_str());

            ImGui::NextColumn();

            ImGui::PopID();
        }

        ImGui::Columns(1);
        ImGui::SliderFloat("Thumbnail Size", &thumbnail_size, 16, 512);
        ImGui::SliderFloat("Thumbnail Padding", &padding, 0, 32);

        if (ImGui::BeginPopupContextWindow("##ContentBrowserPanel"))
        {
            static const char* s_PlatformFileExplorerName = "Open in Explorer";

            ImGui::Separator();
            if (ImGui::MenuItem(s_PlatformFileExplorerName))
            {
                Platform::OpenFileExplorer(m_CurrentDirectory);
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
        m_Context = context;
    }

}
