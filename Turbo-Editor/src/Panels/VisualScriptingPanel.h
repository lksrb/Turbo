#pragma once

#include "Turbo/Editor/EditorPanel.h"

#include "../Nodes/UINode.h"

namespace ax::NodeEditor {
    struct EditorContext;
}

namespace Turbo::Ed {

    class VisualScriptingPanel : public EditorPanel
    {
    public:
        VisualScriptingPanel();
        ~VisualScriptingPanel();

        void OnDrawUI() override;
        void OnEvent(Event& e) override;
        void OnProjectChanged(const Ref<Project>& project) override;
        void OnSceneContextChanged(const Ref<Scene>& context) override;
    private:
        std::vector<UI_Node> m_Nodes;
        std::vector<LinkInfo> m_Links;
        ax::NodeEditor::EditorContext* m_Context = nullptr;
    };

}
