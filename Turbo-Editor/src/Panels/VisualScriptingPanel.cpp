#include "VisualScriptingPanel.h"

#include <Turbo/UI/UI.h>
#include <imgui_internal.h>


namespace Turbo::Ed {

    struct ObjectEntity
    {
        glm::vec3 Translation = glm::vec3(0.0f);
    };

    static ObjectEntity* s_Entity = new ObjectEntity;

    namespace Utils {

        static UI_Node CreateNode(NodeType type)
        {
            static u32 s_NodeCount = 0;

            UI_Node node;
            switch (type)
            {
                case NodeType::StartEvent:
                {
                    node = UI_Node(NodeType::StartEvent, 0, 1);
                    node.ExecutionFunction = [](std::stack<Data>& stack) { /* To avoid a branch in evaluation */ };
                    node.DebugName = "Start Node";
                    node.Outputs[0].Type = PinType::Event;
                    node.Dispatcher = true;
                    node.Build(s_NodeCount++);
                    break;
                }
                case NodeType::SetTranslation:
                {
                    node = UI_Node(NodeType::SetTranslation, 3, 2);
                    node.ExecutionFunction = [](std::stack<Data>& stack)
                    {
                        TBO_ASSERT(stack.size() == 2);

                        glm::vec3 translation = stack.top().As<glm::vec3>();
                        stack.pop();

                        ObjectEntity* obj = stack.top().As<ObjectEntity*>();
                        stack.pop();

                        obj->Translation = translation;

                        stack.emplace(obj->Translation);
                    };
                    node.DebugName = "Set Translation";
                    node.Inputs[0].Type = PinType::Event;
                    node.Inputs[1].Type = PinType::Value;
                    node.Inputs[2].Type = PinType::Value;

                    node.Outputs[0].Type = PinType::Event;
                    node.Outputs[1].Type = PinType::Value;
                    node.Build(s_NodeCount++);
                    break;
                }
                case NodeType::Vector3Literal:
                {
                    node = UI_Node(NodeType::Vector3Literal, 0, 1);
                    node.DebugName = "Vector3 Literal";
                    node.Outputs[0].Type = PinType::Value;
                    node.ExecutionFunction = [](std::stack<Data>& stack)
                    {
                        stack.emplace(glm::vec3(25.0f, 23.0f, 21.0f));
                    };
                    node.Build(s_NodeCount++);
                    break;
                }
                case NodeType::ObjectLiteral:
                {
                    node = UI_Node(NodeType::ObjectLiteral, 0, 1);
                    node.DebugName = "Object Literal";
                    node.Outputs[0].Type = PinType::Value;
                    node.ExecutionFunction = [](std::stack<Data>& stack)
                    {
                        stack.emplace(s_Entity);
                    };
                    node.Build(s_NodeCount++);
                    break;
                }
                case NodeType::Print:
                {
                    node = UI_Node(NodeType::Print, 2, 1);
                    node.DebugName = "Print";
                    node.Inputs[0].Type = PinType::Event;
                    node.Inputs[1].Type = PinType::Value;
                    node.Outputs[0].Type = PinType::Event;
                    node.ExecutionFunction = [](std::stack<Data>& stack)
                    {
                        TBO_INFO(stack.top().As<glm::vec3>().x);
                        stack.pop();
                    };
                    node.Build(s_NodeCount++);
                    break;
                }
                default:
                {
                    TBO_ASSERT(false, "Unknown node type!");
                    break;
                }
            }

            return node;
        }
    }

    VisualScriptingPanel::VisualScriptingPanel()
    {
        GE::Config config;
        //config.CanvasSizeMode = GE::Config::CanvasSizeModeAlias::CenterOnly;
        m_Context = GE::CreateEditor(&config);
        GE::SetCurrentEditor(m_Context);

        // PROBLEM: When the vector resizes, all ids that were put in the system are now invalid
        m_Nodes.reserve(10);
        m_Links.reserve(10);

        // Testing nodes
        auto& startNode = m_Nodes.emplace_back(Utils::CreateNode(NodeType::StartEvent));
        GE::SetNodePosition(startNode.GetID(), ImVec2(10, 10));

        auto& setTranslationNode = m_Nodes.emplace_back(Utils::CreateNode(NodeType::SetTranslation));
        GE::SetNodePosition(setTranslationNode.GetID(), ImVec2(200, 10));

        auto& vector3Literal = m_Nodes.emplace_back(Utils::CreateNode(NodeType::Vector3Literal));
        GE::SetNodePosition(vector3Literal.GetID(), ImVec2(10, 200));

        auto& objectNode = m_Nodes.emplace_back(Utils::CreateNode(NodeType::ObjectLiteral));
        GE::SetNodePosition(objectNode.GetID(), ImVec2(13, 112));

        auto& printNode = m_Nodes.emplace_back(Utils::CreateNode(NodeType::Print));
        GE::SetNodePosition(printNode.GetID(), ImVec2(476, -15));

        auto& printNode2 = m_Nodes.emplace_back(Utils::CreateNode(NodeType::Print));
        GE::SetNodePosition(printNode2.GetID(), ImVec2(732, 0));

        auto& vector3Literal2 = m_Nodes.emplace_back(Utils::CreateNode(NodeType::Vector3Literal));
        vector3Literal2.ExecutionFunction = [](std::stack<Data>& stack)
        {
            stack.emplace(glm::vec3(10.0f, 12.0f, 14.0f));
        };
        GE::SetNodePosition(vector3Literal2.GetID(), ImVec2(570, 128));

        // Link nodes for faster debug
        if constexpr (true) 
        {
            m_Links.emplace_back(GE::PinId(&setTranslationNode.Inputs[0]), GE::PinId(&startNode.Outputs[0]), ImVec4(0.9f, 0.1f, 0.1f, 1.0f), 2.0f);
            m_Links.emplace_back(GE::PinId(&setTranslationNode.Inputs[1]), GE::PinId(&objectNode.Outputs[0]), ImVec4(0.9f, 0.1f, 0.1f, 1.0f), 2.0f);
            m_Links.emplace_back(GE::PinId(&setTranslationNode.Inputs[2]), GE::PinId(&vector3Literal.Outputs[0]), ImVec4(0.9f, 0.1f, 0.1f, 1.0f), 2.0f);
            m_Links.emplace_back(GE::PinId(&printNode.Inputs[0]), GE::PinId(&setTranslationNode.Outputs[0]), ImVec4(0.9f, 0.1f, 0.1f, 1.0f), 2.0f);
            m_Links.emplace_back(GE::PinId(&printNode.Inputs[1]), GE::PinId(&setTranslationNode.Outputs[1]), ImVec4(0.9f, 0.1f, 0.1f, 1.0f), 2.0f);
            m_Links.emplace_back(GE::PinId(&printNode.Outputs[0]), GE::PinId(&printNode2.Inputs[0]), ImVec4(0.9f, 0.1f, 0.1f, 1.0f), 2.0f);
            m_Links.emplace_back(GE::PinId(&printNode2.Inputs[1]), GE::PinId(&vector3Literal2.Outputs[0]), ImVec4(0.9f, 0.1f, 0.1f, 1.0f), 2.0f);

            // Link both ways way
            startNode.Outputs[0].ConnectedPins.push_back(&setTranslationNode.Inputs[0]);
            setTranslationNode.Inputs[0].ConnectedPins.push_back(&startNode.Outputs[0]);

            objectNode.Outputs[0].ConnectedPins.push_back(&setTranslationNode.Inputs[1]);
            setTranslationNode.Inputs[1].ConnectedPins.push_back(&objectNode.Outputs[0]);

            vector3Literal.Outputs[0].ConnectedPins.push_back(&setTranslationNode.Inputs[2]);
            setTranslationNode.Inputs[2].ConnectedPins.push_back(&vector3Literal.Outputs[0]);

            setTranslationNode.Outputs[0].ConnectedPins.push_back(&printNode.Inputs[0]);
            printNode.Inputs[0].ConnectedPins.push_back(&setTranslationNode.Outputs[0]);

            setTranslationNode.Outputs[1].ConnectedPins.push_back(&printNode.Inputs[1]);
            printNode.Inputs[1].ConnectedPins.push_back(&setTranslationNode.Outputs[1]);

            printNode.Outputs[0].ConnectedPins.push_back(&printNode2.Inputs[0]);
            printNode2.Inputs[0].ConnectedPins.push_back(&printNode.Outputs[0]);

            vector3Literal2.Outputs[0].ConnectedPins.push_back(&printNode2.Inputs[1]);
            printNode2.Inputs[1].ConnectedPins.push_back(&vector3Literal2.Outputs[0]);
        }

        // Center viewport
        GE::NavigateToContent(0.0f);
    }

    VisualScriptingPanel::~VisualScriptingPanel()
    {
        GE::DestroyEditor(m_Context);
    }

    //static std::unordered_set<UI_Node> s_InvokeLaters;

    static void TraverseNodes(std::stack<Data>& stack, std::vector<UI_Node>& nodes, UI_Node* current, UI_Node* prev)
    {
        TBO_INFO(current->DebugName);

        if (current->IsFlowNode && current->IsProcessed)
        {
            TBO_WARN("This node is already processed.");
            return;
        }

        bool doProcess = current->Dispatcher;

        // Process inputs
        for (auto& inputPin : current->Inputs)
        {
            // Assure that pin is actually connected
            if (inputPin.ConnectedPins.empty())
                continue;

            // Avoid visiting same nodes
            UI_Node* next = &nodes[inputPin.ConnectedPins.front()->AssignedNodeIndex];
            if (next != prev)
            {
                TraverseNodes(stack, nodes, next, current);
            }
        }

        // Once inputs are processed and stack is filled with data execute node function (sort of lua stack idea)
        current->ExecutionFunction(stack);

        // Do not execute flow nodes multiple times since tranversing can have that side effect
        // Also do not execute outputs since they they only provide constant value
        if (current->IsFlowNode)
        {
            current->IsProcessed = true;

            // Once the node function is done, process outputs which may or may not 
            for (auto& outputPin : current->Outputs)
            {
                // Assure that pin is actually connected
                if (outputPin.ConnectedPins.empty())
                    continue;

                // Avoid visiting same nodes
                UI_Node* next = &nodes[outputPin.ConnectedPins.front()->AssignedNodeIndex];
                if (next != prev)
                {
                    TraverseNodes(stack, nodes, next, current);
                }
            }
        }
    }

    void VisualScriptingPanel::OnDrawUI()
    {
        // Editor window
        ImGui::Begin("Visual Scripting Editor");

        if (ImGui::Button("Execute"))
        {
            // Find execution point
            UI_Node* startNode = nullptr;
            for (auto& link : m_Links)
            {
                auto& outputNode = m_Nodes[link.OutputId.AsPointer<Pin>()->AssignedNodeIndex];

                if (outputNode.Type == NodeType::StartEvent)
                {
                    startNode = &outputNode;
                }
            }

            std::stack<Data> stack;

            if (startNode)
            {
                // Evaluation
                TraverseNodes(stack, m_Nodes, startNode, nullptr);

                // Reset
                for (auto& link : m_Links)
                {
                    auto& inputNode = m_Nodes[link.InputId.AsPointer<Pin>()->AssignedNodeIndex];
                    auto& outputNode = m_Nodes[link.OutputId.AsPointer<Pin>()->AssignedNodeIndex];

                    inputNode.IsProcessed = false;
                    outputNode.IsProcessed = false;
                }
            }
            else
            {
                TBO_ERROR("START NODE IS NOT CONNECTED TO ANYTHING!");
            }
        }

        // Editor workspace
        GE::Begin("Editor Workspace");

        // Draw nodes
        for (const auto& node : m_Nodes)
        {
            UI_Node::Draw(node);
        }

        // Draw links
        for (auto& linkInfo : m_Links)
        {
            GE::Link(linkInfo.Id, linkInfo.InputId, linkInfo.OutputId, linkInfo.Color, linkInfo.Thickness);
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

                    Pin* outputPin = startPin.AsPointer<Pin>();
                    Pin* inputPin = endPin.AsPointer<Pin>();

                    // Pins from the same node cannot connect
                    bool validateConnection = inputPin->AssignedNodeIndex != outputPin->AssignedNodeIndex;

                    // Assure that pin kind is not the same
                    // NOTE: For some reason this is not handled internally so we need to make sure this holds
                    validateConnection &= inputPin->Kind != outputPin->Kind;

                    // Now check if pin types are compatible
                    validateConnection &= inputPin->Type == outputPin->Type;

                    if (validateConnection)
                    {
                        if (GE::AcceptNewItem())
                        {
                            outputPin->ConnectedPins.push_back(inputPin);
                            inputPin->ConnectedPins.push_back(outputPin);

                            // Since we accepted new link, lets add one to our list of links.
                            m_Links.emplace_back(endPin, startPin, ImVec4(0.9f, 0.1f, 0.1f, 1.0f), 2.0f);
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
            while (GE::QueryDeletedLink(&deletedLinkId))
            {
                // If you agree that link can be deleted, accept deletion.
                if (GE::AcceptDeletedItem())
                {
                    // Then remove link from your data.
                    for (auto it = m_Links.begin(); it != m_Links.end(); ++it)
                    {
                        auto& link = *it;

                        if (link.Id == deletedLinkId)
                        {
                            Pin* inputPin = link.InputId.AsPointer<Pin>();
                            Pin* outputPin = link.OutputId.AsPointer<Pin>();
                            
                            inputPin->ConnectedPins.erase(std::find(inputPin->ConnectedPins.begin(), inputPin->ConnectedPins.end(), outputPin));
                            outputPin->ConnectedPins.erase(std::find(outputPin->ConnectedPins.begin(), outputPin->ConnectedPins.end(), inputPin));

                            m_Links.erase(it);
                            break;
                        }
                    }
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
    }
}
