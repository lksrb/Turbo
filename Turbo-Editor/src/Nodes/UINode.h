#pragma once

#include "ImGuiNodeEditor.h"

#include <Turbo/Core/PrimitiveTypes.h>
#include <glm/glm.hpp>
#include <vector>
#include <stack>
#include <list>

namespace Turbo::Ed {

    struct Data
    {
        u8 Value[16]; // 16 bytes should be enough to represent everything we need

        Data() = default;

        template<typename T>
        Data(T value) : Value{ 0 }
        {
            static_assert(sizeof(T) <= 16, "Type is too large!");
            memcpy(Value, &value, sizeof(T));
        }

        template<typename T>
        T As()
        {
            static_assert(sizeof(T) <= 16, "Type is too large!");

            T value;
            memcpy(&value, Value, sizeof(T));

            return value;
        }
    };

    struct UI_Node;

    // Struct to hold basic information about connection between
    // pins. Note that connection (aka. link) has its own ID.
    // This is useful later with dealing with selections, deletion
    // or other operations.
    struct LinkInfo
    {
        GE::LinkId Id;
        GE::PinId  InputId;
        GE::PinId  OutputId;

        ImVec4 Color;
        f32 Thickness;

        LinkInfo(GE::PinId inputId, GE::PinId outputId, const ImVec4& color, f32 thickness)
            : Id(this), InputId(inputId), OutputId(outputId), Color(color), Thickness(thickness)
        {
        }
    };

    enum class NodeType : u32
    {
        StartEvent = 0,
        SetTranslation,
        Vector3Literal,
        ObjectLiteral,
        Print,

        Count
    };

    enum class PinType : u32
    {
        Event = 0,
        Value
    };

    struct Pin
    {
        GE::PinId ID; // PinID should never change
        u32 AssignedNodeIndex;
        GE::PinKind Kind;
        PinType Type;

        std::list<Pin*> ConnectedPins;
    };

    struct UI_Node final
    {
        using ExecFunc = void(*)(std::stack<Data>& stack);

        const char* DebugName = "Unknown";
        NodeType Type;
        ExecFunc ExecutionFunction;
        bool IsFlowNode = false; // Is dynamic?
        bool IsProcessed = false;
        bool Dispatcher = false;

        std::vector<Pin> Inputs;
        std::vector<Pin> Outputs;

        UI_Node() = default;

        UI_Node(NodeType type, u32 inputCount, u32 outputCount)
            : Type(type), Inputs(inputCount), Outputs(outputCount)
        {
        }

        void Build(u32 index)
        {
            // Generate unique ids for each pin
            for (u32 i = 0; i < Inputs.size(); ++i)
            {
                auto& input = Inputs[i];
                input.ID = GE::PinId(&input);
                input.AssignedNodeIndex = index;
                input.Kind = GE::PinKind::Input;

                if (input.Type == PinType::Event)
                    IsFlowNode = true;
            }

            for (u32 i = 0; i < Outputs.size(); ++i)
            {
                auto& output = Outputs[i];
                output.ID = GE::PinId(&output);
                output.AssignedNodeIndex = index;
                output.Kind = GE::PinKind::Output;

                if (output.Type == PinType::Event)
                    IsFlowNode = true;
            }
        }

        inline GE::NodeId GetID() const { return GE::NodeId(this); }

        static void Draw(const UI_Node& node);
    };
}
