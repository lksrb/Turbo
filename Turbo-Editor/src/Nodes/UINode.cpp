#include "UINode.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

#include <IconsFontAwesome6.h>

namespace Turbo::Ed {

    static inline ImRect ImGui_GetItemRect()
    {
        return ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
    }

    namespace Detail {

        using DrawFP = void(*)(const UI_Node&);

        static void Node_Draw_StartEvent(const UI_Node& node)
        {
            const float rounding = 10.0f;
            const float padding = 12.0f;

            UI::ScopedNodeStyleColor backgroundColor(GE::StyleColor_NodeBg, ImColor(50, 200, 50, 230));
            UI::ScopedNodeStyleColor nodeBorderColor(GE::StyleColor_NodeBorder, ImColor(20, 20, 20, 255));

            const auto pinBackground = GE::GetStyle().Colors[GE::StyleColor_NodeBg];

            GE::BeginNode(node.GetID());

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

                GE::BeginPin(node.Outputs[0].ID, GE::PinKind::Output);
                GE::PinPivotAlignment(ImVec2(0.9f, 0.5f));
                GE::PinPivotSize(ImVec2(0, 0));
                UI::Icon(ImVec2(30.f, 30.f), UI::IconType::Square, node.Outputs[0].ConnectedPins.size(), ImVec4(0.9f, 0.1f, 0.1f, 1.0f), ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
                GE::EndPin();
            }

            GE::EndNode();
        }

        static void Node_Draw_SetTranslation(const UI_Node& node)
        {
            GE::BeginNode(node.GetID());
            ImGui::TextUnformatted("Set Translation");

            ImGui::BeginGroup();

            GE::BeginPin(node.Inputs[0].ID, GE::PinKind::Input);
            ImGui::Text("-> Event");
            GE::EndPin();
            GE::BeginPin(node.Inputs[1].ID, GE::PinKind::Input);
            ImGui::Text("-> Object");
            GE::EndPin();
            GE::BeginPin(node.Inputs[2].ID, GE::PinKind::Input);
            ImGui::Text("-> Value");
            GE::EndPin();

            ImGui::EndGroup();
            ImGui::SameLine();
            ImGui::BeginGroup();

            GE::BeginPin(node.Outputs[0].ID, GE::PinKind::Output);
            ImGui::Text("Event ->");
            GE::EndPin();
            GE::BeginPin(node.Outputs[1].ID, GE::PinKind::Output);
            ImGui::Text("Value ->");
            GE::EndPin();

            ImGui::EndGroup();

            GE::EndNode();
        }

        static void Node_Draw_Vector3Literal(const UI_Node& node)
        {
            GE::BeginNode(node.GetID());
            ImGui::TextUnformatted("Vector3 Literal");

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 50.0f);
            GE::BeginPin(node.Outputs[0].ID, GE::PinKind::Output);
            ImGui::Text("Value ->");
            GE::EndPin();

            GE::EndNode();
        }

        static void Node_Draw_ObjectLiteral(const UI_Node& node)
        {
            GE::BeginNode(node.GetID());
            ImGui::TextUnformatted("Object Literal");

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 50.0f);
            GE::BeginPin(node.Outputs[0].ID, GE::PinKind::Output);
            ImGui::Text("This ->");
            GE::EndPin();

            GE::EndNode();
        }

        static void Node_Draw_Print(const UI_Node& node)
        {
            GE::BeginNode(node.GetID());
            ImGui::TextUnformatted("Print");

            ImGui::BeginGroup();

            GE::BeginPin(node.Inputs[0].ID, GE::PinKind::Input);
            ImGui::Text("-> Event");
            GE::EndPin();

            GE::BeginPin(node.Inputs[1].ID, GE::PinKind::Input);
            ImGui::Text("-> Value");
            GE::EndPin();

            ImGui::EndGroup();
            ImGui::SameLine();
            ImGui::BeginGroup();

            GE::BeginPin(node.Outputs[0].ID, GE::PinKind::Output);
            ImGui::Text("Event ->");
            GE::EndPin();

            ImGui::EndGroup();

            GE::EndNode();
        }

        static DrawFP s_NodeDrawFunctions[(u32)NodeType::Count] =
        {
            Node_Draw_StartEvent,
            Node_Draw_SetTranslation,
            Node_Draw_Vector3Literal,
            Node_Draw_ObjectLiteral,
            Node_Draw_Print
        };

    }

    void UI_Node::Draw(const UI_Node& node)
    {
        Detail::s_NodeDrawFunctions[(u32)node.Type](node);
    }
}
