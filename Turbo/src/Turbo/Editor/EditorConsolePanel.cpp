#include "tbopch.h"
#include "EditorConsolePanel.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace Turbo
{
    static EditorConsolePanel* s_Instance;

    EditorConsolePanel::EditorConsolePanel()
    {
        TBO_ENGINE_ASSERT(s_Instance == nullptr, "EditorConsolePanel already exists!");
        s_Instance = this;
    }

    EditorConsolePanel::~EditorConsolePanel()
    {
        s_Instance = nullptr;
    }

    void EditorConsolePanel::OnDrawUI()
    {
        ImGui::Begin("Editor Console", nullptr, ImGuiWindowFlags_NoNavFocus);

        if (ImGui::Button("Clear"))
            m_MessageBufferCount = 0;

        ImGui::SameLine();

        ImGui::Checkbox("Auto scrolling", &m_AutoScroll);

        ImGui::SameLine();
        // TODO: ImageButtons
        static bool info = true, warn = true, error = true;
        if (ImGui::Checkbox("Info", &info))
        {
            m_MessageFilter ^= ConsoleMessage::Category::Info;
        }
        ImGui::SameLine();

        if (ImGui::Checkbox("Warn", &warn))
        {
            m_MessageFilter ^= ConsoleMessage::Category::Warn;
        }
        ImGui::SameLine();

        if (ImGui::Checkbox("Error", &error))
        {
            m_MessageFilter ^= ConsoleMessage::Category::Error;
        }
        ImGui::Separator();

        if (m_MessageBufferCount != 0)
            DrawMessages();

        ImGui::End();
    }

    void EditorConsolePanel::OnEvent(Event& e)
    {
    }

    void EditorConsolePanel::PushMessage(const ConsoleMessage& message)
    {
        if (s_Instance == nullptr)
            return;

        s_Instance->PushMessageInternal(message);
    }

    void EditorConsolePanel::Clear()
    {
        if (s_Instance == nullptr)
            return;

        s_Instance->m_MessageBufferCount = 0;
    }

    void EditorConsolePanel::DrawMessages()
    {
        ImGui::BeginChild("Logging");

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);

        // Iterate throough messages and draw them
        for (u32 i = 0; i < m_MessageBufferCount; ++i)
        {
            const auto& message = m_MessageBuffer[i];

            if (!(m_MessageFilter & message.MessageCategory))
                continue;

            // Color text matching their category
            ImVec4 textColor = { 1.0f , 1.0f, 1.0f, 1.0f };
            switch (message.MessageCategory)
            {
                case ConsoleMessage::Category::Info:  textColor = { 0.0f, 1.0f, 0.0f, 1.0f }; break;
                case ConsoleMessage::Category::Warn:  textColor = { 1.0f, 1.0f, 0.0f, 1.0f }; break;
                case ConsoleMessage::Category::Error: textColor = { 1.0f, 0.0f, 0.0f, 1.0f }; break;
            }

            // Automatic scrolling
            if (m_AutoScroll)
                ImGui::SetScrollY(ImGui::GetScrollMaxY());

            // Offset text
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 2.0f);
            ImGui::PushStyleColor(ImGuiCol_Text, textColor);
            ImGui::Text(message.Text.c_str());
            ImGui::PopStyleColor();

            // Display frequently logged messages
            if (message.Count > 1)
            {
                ImGui::SameLine();
                ImGui::Text("(%d)", message.Count);
            }
        }

        ImGui::EndChild();
    }

    void EditorConsolePanel::PushMessageInternal(const ConsoleMessage& message)
    {
        for (u32 i = 0; i < m_MessageBufferCount; ++i)
        {
            if (m_MessageBuffer[i].ID == message.ID)
            {
                m_MessageBuffer[i].Count++;
                return;
            }
        }

        m_MessageBuffer[m_MessageBufferCount++] = message;

        // Just reset the buffer
        if (m_MessageBufferCount == m_MessageBuffer.size())
            m_MessageBufferCount = 0;
    }

}
