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
        ImGui::Begin("Editor Console");
        
        if (ImGui::Button("Clear"))
            m_MessageBufferCount = 0;

        ImGui::Separator();

        for (u32 i = 0; i < m_MessageBufferCount; ++i)
        {
            ImGui::Text(m_MessageBuffer[i].Message.c_str());
        }

        ImGui::End();
    }

    void EditorConsolePanel::OnEvent(Event& e)
    {
    }

    void EditorConsolePanel::PushMessage(const ConsoleMessage& message)
    {
        TBO_ENGINE_ASSERT(s_Instance, "EditorConsolePanel does not exist!");
        s_Instance->PushMessageInternal(message);
    }

    void EditorConsolePanel::PushMessageInternal(const ConsoleMessage& message)
    {
        TBO_ENGINE_ASSERT(m_MessageBufferCount < m_MessageBuffer.size());
        m_MessageBuffer[m_MessageBufferCount++] = message;
    }

}
