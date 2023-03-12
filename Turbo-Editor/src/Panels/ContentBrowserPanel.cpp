#include "ContentBrowserPanel.h"

#include <Turbo/UI/UI.h>

#include <filesystem>

namespace Turbo::Ed
{
    extern std::filesystem::path g_AssetPath;

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

        if (m_CurrentDirectory != g_AssetPath)
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

            const auto& relativePath = std::filesystem::relative(path, g_AssetPath.c_str());
            const std::string& filenameString = relativePath.filename().string();

            ImGui::PushID(filenameString.c_str());
            Ref<Texture2D> icon = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon;

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            UI::ImageButton(icon, { thumbnail_size, thumbnail_size }, { 0,1 }, { 1,0 });
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
            {
                if (path.extension() == ".tscene")
                {
                    const wchar_t* itemPath = path.c_str();
                    ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t), ImGuiCond_Always);
                }
                ImGui::EndDragDropSource();
            }

            ImGui::PopStyleColor();

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                if (directoryEntry.is_directory())
                    m_CurrentDirectory /= path.filename().string().c_str();
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
        m_CurrentDirectory = g_AssetPath;
    }

}
