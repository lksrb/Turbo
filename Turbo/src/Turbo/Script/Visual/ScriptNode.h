#pragma once

#include "Turbo/Core/PrimitiveTypes.h"
#include "Turbo/Core/UUID.h"

#include "Stack.h"

#include <vector>

namespace Turbo {

    enum class NodeType : u32
    {
        Start = 0,
        SetTranslation,
        Vector3Literal,
        EntityLiteral,
        AddValues,
        Print,
        ForLoop,

        Count
    };

    enum class ValueType : u32
    {
        None = 0,
        Event = 1 << 0,
        Integer = 1 << 1,
        Float = 1 << 2,
        //String = 1 << 3,
        Vector2 = 1 << 4,
        Vector3 = 1 << 5,
        Vector4 = 1 << 6,
        Entity = 1 << 7,
        NodePtr,
        AdditiveValues = Integer | Float | Vector2 | Vector3 | Vector4,
        PrintableValues = AdditiveValues | Entity,
    };

    enum class ScriptPinKind : u32
    {
        Input,
        Output
    };

    struct ScriptNode;

    struct ScriptPin
    {
        UUID ID;
        i32 DataIndex = -1;
        ValueType Type = ValueType::None;
        std::vector<ScriptPin*> Connected;

        AnyData DataStorage;

        // Runtime
        ScriptPinKind Kind;
        ScriptNode* Current = nullptr;

        ScriptPin(ValueType type) : Type(type) {}
    };

    struct ScriptNode
    {
        const char* DebugName = "Unknown";
        NodeType Type = NodeType::Count;
        bool Evaluated = false;

        std::vector<ScriptPin> Inputs;
        std::vector<ScriptPin> Outputs;
    };

    struct ScriptPinLink
    {
        UUID InputPinID = 0;
        UUID OutputPinID = 0;
    };
}
