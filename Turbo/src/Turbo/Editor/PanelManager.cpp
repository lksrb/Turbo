#include "tbopch.h"
#include "PanelManager.h"

namespace Turbo {

    void PanelManager::OnProjectChanged(const Ref<Project>& project)
    {
        for (auto& [_, panel] : m_Panels)
        {
            panel->OnProjectChanged(project);
        }
    }

    void PanelManager::SetSceneContext(const Ref<Scene>& context)
    {
        for (auto& [_, panel] : m_Panels)
        {
            panel->OnSceneContextChanged(context);
        }
    }

    void PanelManager::OnDrawUI()
    {
        for (auto& [_, panel] : m_Panels)
        {
            panel->OnDrawUI();
        }
    }

    void PanelManager::OnEvent(Event& e)
    {
        for (auto& [_, panel] : m_Panels)
        {
            panel->OnEvent(e);
        }
    }
}
