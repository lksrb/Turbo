#include "NodeUI.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

#include "../Nodes/ImGuiNodeEditor.h"

#include <turbo/UI/UI.h>
#include <Turbo/Script/Visual/ScriptNode.h>
#include <glm/glm.hpp>

#include <IconsFontAwesome6.h>

namespace Turbo::UI {

    static ImRect ImGui_GetItemRect()
    {
        return ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
    }

    static void BeginNode(const ScriptNode& node)
    {
        GE::BeginNode(GE::NodeId(&node));
    }

    static void EndNode()
    {
        GE::EndNode();
    }

    static void BeginPin(const ScriptPin& pin)
    {
        GE::BeginPin(GE::PinId(pin.ID), (GE::PinKind)pin.Kind);
    }

    static void EndPin()
    {
        GE::EndPin();
    }

    namespace Detail {

        using DrawFP = void(*)(const ScriptNode& node);

        static void Node_Draw_StartEvent(const ScriptNode& node)
        {
            const float rounding = 10.0f;
            const float padding = 12.0f;

            UI::ScopedNodeStyleColor backgroundColor(GE::StyleColor_NodeBg, ImColor(50, 200, 50, 230));
            UI::ScopedNodeStyleColor nodeBorderColor(GE::StyleColor_NodeBorder, ImColor(20, 20, 20, 255));

            const auto pinBackground = GE::GetStyle().Colors[GE::StyleColor_NodeBg];

            BeginNode(node);

            // Title
            ImGui::TextUnformatted("Start Event");

            ImGui::SameLine();
            // Event Pin
            {
                UI::ScopedNodeStyleVar pinCorners(GE::StyleVar_PinCorners, ImDrawFlags_RoundCornersAll);
                UI::ScopedNodeStyleColor pinRectColor(GE::StyleColor_PinRect, ImColor(60, 180, 255, 150));
                UI::ScopedNodeStyleColor PinRectBorderColor(GE::StyleColor_PinRectBorder, ImColor(60, 180, 255, 150));

                //UI::ScopedNodeStyleVar nodePadding(GE::StyleVar_NodePadding, ImVec4(0, 0, 0, 0));
                //UI::ScopedNodeStyleVar nodeRounding(GE::StyleVar_NodeRounding, rounding);
                UI::ScopedNodeStyleVar sourceDirection(GE::StyleVar_SourceDirection, ImVec2(1.0f, 0.0f));
                UI::ScopedNodeStyleVar targetDirection(GE::StyleVar_TargetDirection, ImVec2(-1.0f, 0.0f));
                //UI::ScopedNodeStyleVar linkStrength(GE::StyleVar_LinkStrength, 0.0f);
                UI::ScopedNodeStyleVar pinBorderWidth(GE::StyleVar_PinBorderWidth, 1.0f);
                UI::ScopedNodeStyleVar pinRadius(GE::StyleVar_PinRadius, 0.0f);

                //ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 50.0f);

                UI::ScopedNodeStyleColor linkColor(GE::StyleColor_SelLinkBorder, ImColor(0xFF0000FF));

                BeginPin(node.Outputs[0]);
                GE::PinPivotAlignment(ImVec2(0.9f, 0.5f));
                GE::PinPivotSize(ImVec2(0, 0));
                UI::Icon(ImVec2(30.f, 30.f), UI::IconType::Square, node.Outputs[0].Connected.size(), ImVec4(0.9f, 0.1f, 0.1f, 1.0f), ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
                EndPin();
            }

            EndNode();
        }

        static void Node_Draw_SetTranslation(const ScriptNode& node)
        {
            BeginNode(node);
            ImGui::TextUnformatted("Set Translation");

            ImGui::BeginGroup();

            BeginPin(node.Inputs[0]);
            ImGui::Text("-> Event");
            EndPin();
            BeginPin(node.Inputs[1]);
            ImGui::Text("-> Object");
            EndPin();
            BeginPin(node.Inputs[2]);
            ImGui::Text("-> Value");
            EndPin();

            ImGui::EndGroup();
            ImGui::SameLine();
            ImGui::BeginGroup();

            BeginPin(node.Outputs[0]);
            ImGui::Text("Event ->");
            EndPin();
            BeginPin(node.Outputs[1]);
            ImGui::Text("Value ->");
            EndPin();

            ImGui::EndGroup();

            EndNode();
        }

        static void Node_Draw_Vector3Literal(const ScriptNode& node)
        {
            BeginNode(node);
            ImGui::TextUnformatted("Vector3 Literal");

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 50.0f);
            BeginPin(node.Outputs[0]);
            ImGui::Text("Value ->");
            EndPin();

            EndNode();
        }

        static void Node_Draw_EntityLiteral(const ScriptNode& node)
        {
            BeginNode(node);
            ImGui::TextUnformatted("Object Literal");

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 50.0f);
            BeginPin(node.Outputs[0]);
            ImGui::Text("This ->");
            EndPin();

            EndNode();
        }

        static void Node_Draw_AddValues(const ScriptNode& node)
        {
            BeginNode(node);
            ImGui::TextUnformatted("Add Values");

            ImGui::BeginGroup();

            BeginPin(node.Inputs[0]);
            ImGui::Text("-> Value");
            EndPin();

            BeginPin(node.Inputs[1]);
            ImGui::Text("-> Value");
            EndPin();

            ImGui::EndGroup();
            ImGui::SameLine();
            ImGui::BeginGroup();

            BeginPin(node.Outputs[0]);
            ImGui::Text("Value ->");
            EndPin();

            ImGui::EndGroup();

            EndNode();
        }

        static void Node_Draw_Print(const ScriptNode& node)
        {
            BeginNode(node);
            ImGui::TextUnformatted("Print");

            ImGui::BeginGroup();

            BeginPin(node.Inputs[0]);
            ImGui::Text("-> Event");
            EndPin();

            BeginPin(node.Inputs[1]);
            ImGui::Text("-> Value");
            EndPin();

            ImGui::EndGroup();
            ImGui::SameLine();
            ImGui::BeginGroup();

            BeginPin(node.Outputs[0]);
            ImGui::Text("Event ->");
            EndPin();

            ImGui::EndGroup();

            EndNode();
        }

        static void Node_Draw_ForLoop(const ScriptNode& node)
        {
            BeginNode(node);
            ImGui::TextUnformatted("For Loop");

            ImGui::BeginGroup();

            BeginPin(node.Inputs[0]);
            ImGui::Text("-> Event");
            EndPin();

            BeginPin(node.Inputs[1]);
            ImGui::Text("-> Min");
            EndPin();

            BeginPin(node.Inputs[2]);
            ImGui::Text("-> Max");
            EndPin();

            BeginPin(node.Inputs[3]);
            ImGui::Text("-> Step");
            EndPin();

            ImGui::EndGroup();
            ImGui::SameLine();
            ImGui::BeginGroup();

            BeginPin(node.Outputs[0]);
            ImGui::Text("Event ->");
            EndPin();

            BeginPin(node.Outputs[1]);
            ImGui::Text("Body ->");
            EndPin();

            BeginPin(node.Outputs[2]);
            ImGui::Text("Index ->");
            EndPin();

            ImGui::EndGroup();

            EndNode();
        }

        static DrawFP s_NodeDrawFunctions[(u32)NodeType::Count] =
        {
            Node_Draw_StartEvent,
            Node_Draw_SetTranslation,
            Node_Draw_Vector3Literal,
            Node_Draw_EntityLiteral,
            Node_Draw_AddValues,
            Node_Draw_Print,
            Node_Draw_ForLoop
        };

    }

    void UI::DrawNode(const ScriptNode& node)
    {
        Detail::s_NodeDrawFunctions[(u32)node.Type](node);
    }

    void UI::DrawLink(UUID linkUUID, const ScriptPinLink& link)
    {
        // TODO: Custom colors when linking
        GE::Link(GE::LinkId(linkUUID), GE::PinId(link.InputPinID), GE::PinId(link.OutputPinID), ImVec4(0.9f, 0.1f, 0.1f, 1.0f), 2.0f);
    }

}
