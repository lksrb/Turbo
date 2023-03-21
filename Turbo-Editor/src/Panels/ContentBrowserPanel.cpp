#include "ContentBrowserPanel.h"

#include <Turbo/UI/UI.h>
#include <Turbo/Core/Platform.h>
#include <Turbo/Solution/Project.h>

#include <filesystem>

namespace Turbo::Ed
{
    ContentBrowserPanel::ContentBrowserPanel()
    {
        m_DirectoryIcon = Texture2D::Create({ "Resources/Icons/DirectoryIcon.png" });
        m_FileIcon = Texture2D::Create({ "Resources/Icons/FileIcon.png" });
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

            const auto& relativePath = std::filesystem::relative(path, m_BasePath.c_str());
            const std::string& filenameString = relativePath.filename().string();

            ImGui::PushID(filenameString.c_str());
            Ref<Texture2D> icon = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon;

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            UI::ImageButton(icon, { thumbnail_size, thumbnail_size }, { 0,1 }, { 1,0 });
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
            {
                const wchar_t* item_path = path.c_str();
                size_t item_path_size = (wcslen(item_path) + 1) * sizeof(wchar_t);
                if (path.extension() == ".cs")
                {
                    ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM_SHP", item_path, item_path_size, ImGuiCond_Always);
                }
                else if (path.extension() == ".tscene")
                {
                    ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM_VIEWPORT", item_path, item_path_size, ImGuiCond_Always);
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
                        std::filesystem::path path_to_solution = m_BasePath / Project::GetProjectName();
                        path_to_solution.concat(".sln");

                        if (!Platform::Start("devenv.exe", path_to_solution.concat(path.string()).string()))
                        {
                            TBO_ERROR("Failed to open visual studio!");
                        }
                    } else if(path.extension() == ".sln")
                    {
                        // Opens Visual Studio, whatever version is registered first 
                        // TODO: Client should have an option which visual studio to open
                        if (!Platform::Start("devenv.exe", path.string()))
                        {
                            TBO_ERROR("Failed to open visual studio!");
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

        ImGui::End();
    }

    void ContentBrowserPanel::OnEvent(Event& e)
    {
    }

    void ContentBrowserPanel::SetProjectAssetPath()
    {
        m_BasePath = Project::GetAssetsPath();
        m_CurrentDirectory = Project::GetAssetsPath();
    }

}
