#pragma once

#include "MonoForwards.h"

namespace Turbo
{
    // REMINDER: Do not change the order
    enum class ScriptFieldType : u32 
    {
        Float = 0,
        Double,
        Bool,
        Char,
        Int,
        UInt,
        Short,
        UShort,
        Long,
        ULong,
        Byte,
        UByte,
        Vector2,
        Vector3,
        Vector4,
        Entity,
        Max,
        None = 0xffffffff
    };

    struct ScriptField
    {
        MonoClassField* MonoField;
        ScriptFieldType Type;
    };

    struct ScriptFieldInstance
    {
        ScriptField Field;
        u8 Buffer[16];

        ScriptFieldInstance()
        {
            memset(this, 0, sizeof(*this));
        }

        template<typename T>
        void SetValue(T val)
        {
            static_assert(sizeof(T) <= 16, "Type is too large!");

            memcpy_s(Buffer, sizeof(Buffer), &val, sizeof(T));
        }

        template<typename T>
        T GetValue() const
        {
            static_assert(sizeof(T) <= 16, "Type is too large!");

            T val;
            memcpy_s(&val, sizeof(T), Buffer, sizeof(T));

            return val;
        }
    };
}
