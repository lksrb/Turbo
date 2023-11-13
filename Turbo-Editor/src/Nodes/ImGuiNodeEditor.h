#pragma once

#include <Turbo/Core/PrimitiveTypes.h>

#include <imgui_node_editor.h>
namespace GE = ax::NodeEditor;

// Visual Scripting Nodes
namespace Turbo::UI {

    struct ScopedNodeStyleColor
    {
        explicit ScopedNodeStyleColor(GE::StyleColor idx, const ImVec4& color, bool predicate = true)
            : Set(predicate)
        {
            if (Set)
                GE::PushStyleColor(idx, color);
        }

        explicit ScopedNodeStyleColor(GE::StyleColor idx, const ImColor& color, bool predicate = true)
            : Set(predicate)
        {
            if (Set)
                GE::PushStyleColor(idx, color);
        }

        ScopedNodeStyleColor(const ScopedNodeStyleColor&) = delete;

        ~ScopedNodeStyleColor()
        {
            if (Set)
                GE::PopStyleColor();
        }
    private:
        bool Set = false;
    };

    struct ScopedNodeStyleVar
    {
        explicit ScopedNodeStyleVar(GE::StyleVar idx, const ImVec4& value, bool predicate = true)
            : Set(predicate)
        {
            if (Set)
                GE::PushStyleVar(idx, value);
        }

        explicit ScopedNodeStyleVar(GE::StyleVar idx, const ImVec2& value, bool predicate = true)
            : Set(predicate)
        {
            if (Set)
                GE::PushStyleVar(idx, value);
        }

        explicit ScopedNodeStyleVar(GE::StyleVar idx, float value, bool predicate = true)
            : Set(predicate)
        {
            if (Set)
                GE::PushStyleVar(idx, value);
        }

        ScopedNodeStyleVar(const ScopedNodeStyleVar&) = delete;

        ~ScopedNodeStyleVar()
        {
            if (Set)
                GE::PopStyleVar();
        }
    private:
        bool Set = false;
    };

    enum class IconType : ImU32
    { 
        Flow = 0, 
        Circle, 
        Square, 
        Grid, 
        RoundSquare, 
        Diamond 
    };

    // Maybe mode this to Widgets.h ?

    // From imgui-node-editor blueprints example
    void DrawIcon(ImDrawList* drawList, const ImVec2& a, const ImVec2& b, IconType type, bool filled, ImU32 color, ImU32 innerColor);
    void Icon(const ImVec2& size, IconType type, bool filled, const ImVec4& color = ImVec4(1, 1, 1, 1), const ImVec4& innerColor = ImVec4(0, 0, 0, 0));

    // ID generators
    i32 GenerateID();
}
