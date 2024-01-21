#include "VisualScriptingPanel.h"

#include "../Nodes/NodeUI.h"

#include <imgui_internal.h>
#include <Turbo/UI/UI.h>
#include <Turbo/Script/Visual/Stack.h>

namespace Turbo::Ed {

    VisualScriptingPanel::VisualScriptingPanel()
    {
        GE::Config config;
        //config.CanvasSizeMode = GE::Config::CanvasSizeModeAlias::CenterOnly;
        m_Context = GE::CreateEditor(&config);
        GE::SetCurrentEditor(m_Context);

        // Testing nodes
        CreateNewNode(NodeType::Start, 10, 10);
        CreateNewNode(NodeType::SetTranslation, 200, 10);
        CreateNewNode(NodeType::Vector3Literal, 10, 200);
        CreateNewNode(NodeType::EntityLiteral, 13, 112);
        CreateNewNode(NodeType::Print, 476, -15);
        CreateNewNode(NodeType::Print, 732, 0);
        CreateNewNode(NodeType::Vector3Literal, 570, 128);

        // Center viewport
        GE::NavigateToContent(0.0f);

        // Instantiate script calls (temp)
        VisualScriptEngine::Init();
    }

    VisualScriptingPanel::~VisualScriptingPanel()
    {
        GE::DestroyEditor(m_Context);
    }

    void VisualScriptingPanel::OnDrawUI()
    {
        // Editor window
        ImGui::Begin("Visual Scripting Editor");

        if (ImGui::Button("Execute"))
        {
            static Entity s_TestEntity;

            if (!s_TestEntity)
            {
                s_TestEntity = m_SceneContext->CreateEntity("Script Entity");
            }
            //VisualScriptEngine::Serialize();
            VisualScriptEngine::OnRuntimeStart(m_SceneContext.Get());
            VisualScriptEngine::EntityOnStart(s_TestEntity);
        }

        // Editor workspace
        GE::Begin("Editor Workspace");

        // Draw nodes
        for (auto& node : VisualScriptEngine::GetNodes())
        {
            UI::DrawNode(node);
        }

        // Draw links
        for (auto& [uuid, link] : VisualScriptEngine::GetLinks())
        {
            UI::DrawLink(uuid, link);
        }

        // Handle creation of nodes or links
        if (GE::BeginCreate())
        {
            GE::PinId startPin, endPin;
            if (GE::QueryNewLink(&startPin, &endPin))
            {
                if (startPin && endPin) // both are valid, let's accept link
                {
                    // GE::AcceptNewItem() return true when user release mouse button.

                    auto pin0 = VisualScriptEngine::GetPin((UUID)startPin.Get());
                    auto pin1 = VisualScriptEngine::GetPin((UUID)endPin.Get());

                    // Pins from the same node cannot connect
                    bool validateConnection = pin1->Current != pin0->Current;

                    // Assure that we cannot put input into input and output into output
                    validateConnection &= pin1->Kind != pin0->Kind;

                    // Now check if pin types are compatible
                    //validateConnection &= pin1->AccessType == pin0->AccessType;

                    if (validateConnection)
                    {
                        if (GE::AcceptNewItem())
                        {
                            // Since we accepted new link, lets add one to our list of links.
                            VisualScriptEngine::Connect(pin0, pin1);
                        }
                    }
                    else
                    {
                        // Visualize link rejection
                        GE::RejectNewItem(ImVec4(0.35f, 0.35f, 0.35f, 1.0f));
                    }

                }
            }
        }
        GE::EndCreate(); // Wraps up object creation action handling.

        // Handle deletion action
        if (GE::BeginDelete())
        {
            // There may be many links marked for deletion, let's loop over them.
            GE::LinkId deletedLinkId;
            GE::PinId startPin, endPin;
            while (GE::QueryDeletedLink(&deletedLinkId, &startPin, &endPin))
            {
                // If you agree that link can be deleted, accept deletion.
                if (GE::AcceptDeletedItem())
                {
                    VisualScriptEngine::Disconnect(static_cast<u64>(deletedLinkId));
                }
            }
        }
        GE::EndDelete(); // Wrap up deletion action

        // End of interaction with editor.
        GE::End();

        ImGui::End();
    }

    void VisualScriptingPanel::OnEvent(Event& e)
    {
    }

    void VisualScriptingPanel::OnProjectChanged(const Ref<Project>& project)
    {
    }

    void VisualScriptingPanel::OnSceneContextChanged(const Ref<Scene>& context)
    {
        m_SceneContext = context;
    }

    ScriptNode* VisualScriptingPanel::CreateNewNode(NodeType type, f32 x, f32 y)
    {
        ScriptNode* node = VisualScriptEngine::Create(type, Entity());
        GE::SetNodePosition(GE::NodeId(node), ImVec2(x, y));
        return node;
    }

}
