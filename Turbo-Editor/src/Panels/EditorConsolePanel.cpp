#include "EditorConsolePanel.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace Turbo::Ed
{

    void EditorConsolePanel::OnDrawUI()
    {
        ImGui::Begin("Editor Console");
        ImGui::End();
    }

    void EditorConsolePanel::OnEvent(Event& e)
    {
    }

}
