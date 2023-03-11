#include "tbopch.h"
#include "QuickAccessPanel.h"

#include <Turbo/Core/Input.h>

#include <imgui.h>

namespace Turbo::Ed
{
    QuickAccessPanel::QuickAccessPanel()
    {
    }

    QuickAccessPanel::~QuickAccessPanel()
    {
    }

    void QuickAccessPanel::Open(bool show)
    {
        m_Open = show;
        m_Input.clear();

        if (show == false)
        {
            // Reset input text on close 
            memset(m_TextBuffer, 0, sizeof(m_TextBuffer));
        }
    }

    // TODO: ImGui random id generator
    void QuickAccessPanel::OnDrawUI()
    {
        if (m_Open == false)
            return;

        // Center window
        ImVec2 bigWindowPos = ImGui::GetMainViewport()->Pos;
        ImVec2 bigWindowSize = ImGui::GetMainViewport()->Size;
        ImVec2 smallWindowSize = ImGui::GetWindowSize();

        ImVec2 absl;
        absl.x = bigWindowPos.x + (bigWindowSize.x * 0.5f - smallWindowSize.x * 0.5f);
        absl.y = bigWindowPos.y + (bigWindowSize.y * 0.5f - smallWindowSize.y * 0.5f) + 39;

        ImGui::SetNextWindowPos(absl, ImGuiCond_Appearing);
        ImGui::Begin("Quick Access", nullptr, ImGuiWindowFlags_NoDecoration);
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::SetKeyboardFocusHere();
        if (ImGui::InputText("##Command", m_TextBuffer, sizeof(m_TextBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
        {
            m_Input = m_TextBuffer;
            TBO_INFO(m_Input.c_str());
            Open(false);
        }

        ImGui::End();
    }

    void QuickAccessPanel::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<KeyPressedEvent>(TBO_BIND_FN(QuickAccessPanel::OnKeyPressed));
    }

    bool QuickAccessPanel::OnKeyPressed(KeyPressedEvent& e)
    {
        // Doesnt work because ImGui consumes all the inputs
        bool alt = Input::IsKeyPressed(Key::LeftAlt) || Input::IsKeyPressed(Key::RightAlt);

        switch (e.GetKeyCode())
        {
            case Key::X:
            {
                if (alt)
                    Open(!IsOpened());
                break;
            }
            case Key::Escape:
            {
                if (IsOpened())
                    Open(false);
                break;
            }
        }

        return true;
    }

}
