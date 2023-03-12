#include "CreateProjectPopupPanel.h"

#include <Turbo/Core/Platform.h>
#include <Turbo/Solution/Project.h>

#include <IconsFontAwesome6.h>
#include <imgui.h>
#include <filesystem>

namespace Turbo::Ed
{
    static char s_ProjectName[64]{ 0 };
    static char s_ProjectDirectoryPath[256]{ 0 };
    static char s_ProjectFullPath[256]{ 0 };

    CreateProjectPopupPanel::CreateProjectPopupPanel()
    {
        strcpy_s(s_ProjectDirectoryPath, "C:\\dev\\TurboProjects");
        strcpy_s(s_ProjectName, "TurboProject1");
    }

    CreateProjectPopupPanel::~CreateProjectPopupPanel()
    {
    }

    void CreateProjectPopupPanel::SetCallback(const CreateProjectCallback& callback)
    {
        m_Callback = callback;
    }

    void CreateProjectPopupPanel::OnDrawUI()
    {
        if (!m_Open)
            return;

        ImGui::OpenPopup("New Project...");

        // Always center this window when appearing
        ImGuiViewport* viewport = ImGui::GetMainViewport();

        ImVec2 center = viewport->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSizeConstraints({ 640, 360 }, viewport->Size);
        ImGui::SetNextWindowSize({ viewport->Size.x / 2, viewport->Size.y / 2 }, ImGuiCond_FirstUseEver);

        if (ImGui::BeginPopupModal("New Project...", &m_Open))
        {
            memset(s_ProjectFullPath, 0, sizeof(s_ProjectFullPath));

            strcat_s(s_ProjectFullPath, s_ProjectDirectoryPath);
            strcat_s(s_ProjectFullPath, "\\");
            strcat_s(s_ProjectFullPath, s_ProjectName);
            ImGui::Text("Project name");
            float width = ImGui::GetContentRegionAvail().x;
            ImGui::SetNextItemWidth(width - 30.0f);

            ImGui::InputTextWithHint("##ProjectName", "<Enter a project name>", s_ProjectName, sizeof(s_ProjectName));
            ImGui::NextColumn();
            ImGui::Text("Location");

            bool locationError = std::filesystem::exists(s_ProjectFullPath);

            if (locationError)
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{ 0.8f, 0.2f, 0.3f, 1.0f });
            ImGui::SetNextItemWidth(width - 30.0f);
            ImGui::InputTextWithHint("##ProjectLocation", "<Enter a valid path>", s_ProjectFullPath, sizeof(s_ProjectFullPath));
            if (locationError)
                ImGui::PopStyleColor();

            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f });
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.1f, 0.1f, 0.1f, 1.0f });
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.1f, 0.1f, 0.1f });
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 2.0f);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2.0f);
            if (ImGui::Button(ICON_FA_FOLDER, { 25.0f, 25.0f }))
            {
                const std::filesystem::path& filepath = Platform::OpenBrowseFolderDialog("Select Location", s_ProjectDirectoryPath);

                if (!filepath.empty())
                {
                    strcpy_s(s_ProjectDirectoryPath, filepath.string().c_str());
                }
            }

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - 110.0f);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y - 25.0f);

            if (ImGui::Button("Cancel", { 50.0f, 25.0f }))
            {
                ImGui::CloseCurrentPopup();
                m_Open = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Create", { 50.0f, 25.0f })
                && s_ProjectDirectoryPath[0] != '\0'
                && s_ProjectName[0] != '\0')
            {
                if (!locationError)
                {
                    m_Callback(s_ProjectFullPath);

                    ImGui::CloseCurrentPopup();
                    m_Open = false;
                }
            }

            ImGui::PopStyleColor(3);
            ImGui::PopStyleVar();

            ImGui::EndPopup();
        }
    }

    void CreateProjectPopupPanel::OnEvent(Event& e)
    {
    }

}
