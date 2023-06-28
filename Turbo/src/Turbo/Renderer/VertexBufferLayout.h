#pragma once

#include "Turbo/Core/Common.h"

#include <vector>
#include <string>

namespace Turbo
{
    enum class AttributeType : u32
    {
        Float = 0,
        Vec2,
        Vec3,
        Vec4,

        UInt,
        Int
    };

    struct VertexAttribute
    {
        AttributeType Type;
        std::string_view DebugName;
        u32 Offset = 0;

        VertexAttribute(AttributeType type, std::string_view debugName)
            : Type(type), DebugName(debugName)
        {
        }
    };

    class VertexBufferLayout
    {
    public:
        VertexBufferLayout() = default;
        VertexBufferLayout(const std::initializer_list<VertexAttribute>& attributes);
        ~VertexBufferLayout();

        const std::vector<VertexAttribute>& GetAttributes() const { return m_Attributes; }
        u32 GetStride() const { return m_Stride; }
        u32 AttributeCount() const { return (u32)m_Attributes.size(); }

        [[nodiscard]] auto begin() { return m_Attributes.begin(); }
        [[nodiscard]] auto end() { return m_Attributes.end(); }
        [[nodiscard]] auto begin() const { return m_Attributes.begin(); }
        [[nodiscard]] auto end() const { return m_Attributes.end(); }
    private:
        void CalculateOffsetAndStride();

        std::vector<VertexAttribute> m_Attributes;
        u32 m_Stride = 0;
    };
}
