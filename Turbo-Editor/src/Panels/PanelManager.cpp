#include "PanelManager.h"

namespace Turbo::Ed
{
    PanelManager::PanelManager()
    {
    }

    PanelManager::~PanelManager()
    {
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
