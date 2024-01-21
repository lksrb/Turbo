#pragma once

#include "Turbo/Editor/EditorPanel.h"
#include "../Nodes/ImGuiNodeEditor.h"
#include <Turbo/Script/Visual/VisualScriptEngine.h>

namespace ax::NodeEditor {
    struct EditorContext;
}

namespace Turbo::Ed {

   /* struct NodeLink
    {
        GE::LinkId Id;
        GE::PinId  InputId;
        GE::PinId  OutputId;

        ImVec4 Color;
        f32 Thickness;

        NodeLink(GE::PinId inputId, GE::PinId outputId, const ImVec4& color, f32 thickness)
            : Id(this), InputId(inputId), OutputId(outputId), Color(color), Thickness(thickness)
        {
        }
    };*/

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
        ScriptNode* CreateNewNode(NodeType type, f32 x, f32 y);
    private:
        Ref<Scene> m_SceneContext;
        //std::list<NodeLink> m_Links;
        ax::NodeEditor::EditorContext* m_Context = nullptr;
    };

}
