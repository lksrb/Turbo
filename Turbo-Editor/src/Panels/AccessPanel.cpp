#include "tbopch.h"
#include "AccessPanel.h"

#include <imgui.h>

namespace Turbo::Ed {

    AccessPanel::AccessPanel()
        : m_Open(false), m_TextBuffer{ 0 }
    {
    }

    AccessPanel::~AccessPanel()
    {
    }
    /*TODO: imgui random id generator*/
    void AccessPanel::OnUIRender()
    {
        if (m_Open == false)
            return;

        ImGuiCenterWindow();

        ImGui::Begin("Quick Access", nullptr, ImGuiWindowFlags_NoDecoration);
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::SetKeyboardFocusHere();
        if (ImGui::InputText("##Command", m_TextBuffer, sizeof(m_TextBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
        {
            m_Input = m_TextBuffer;
            m_Callback(m_Input);
            Open(false);
        }
    
        ImGui::End();
    }

    void AccessPanel::Open(bool show)
    {
        m_Open = show;
        m_Input.Reset();

        if (show == false)
        {
            // Reset input text on close 
            memset(m_TextBuffer, 0, sizeof(m_TextBuffer));
        }
    }

    bool AccessPanel::IsOpened() const
    {
        return m_Open;
    }

    void AccessPanel::SetOnInputSendCallback(OnInputSendCallback callback)
    {
        m_Callback = callback;
    }

    void AccessPanel::ImGuiCenterWindow()
    {
        ImVec2 bigWindowPos = ImGui::GetMainViewport()->Pos;
        ImVec2 bigWindowSize = ImGui::GetMainViewport()->Size;
        ImVec2 smallWindowSize = ImGui::GetWindowSize();

        ImVec2 absl;
        absl.x = bigWindowPos.x + (bigWindowSize.x * 0.5f - smallWindowSize.x * 0.5f);
        absl.y = bigWindowPos.y + (bigWindowSize.y * 0.5f - smallWindowSize.y * 0.5f) + 39;

        ImGui::SetNextWindowPos(absl, ImGuiCond_Appearing);
    }
}
