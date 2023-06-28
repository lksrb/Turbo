#include "tbopch.h"
#include "VertexBufferLayout.h"

namespace Turbo
{
    namespace Utils
    {
        static u32 GetSizeFromAttributeType(AttributeType type)
        {
            switch (type)
            {
                case AttributeType::Float:      return 4 * 1;
                case AttributeType::Vec2:     return 4 * 2;
                case AttributeType::Vec3:     return 4 * 3;
                case AttributeType::Vec4:     return 4 * 4;
                case AttributeType::UInt:       return 4;
                case AttributeType::Int:        return 4;
            }

            TBO_ENGINE_ASSERT(false, "Invalid attribute type");
            return 0;
        }
    }

    VertexBufferLayout::VertexBufferLayout(const std::initializer_list<VertexAttribute>& attributes)
        : m_Attributes(attributes)
    {
        CalculateOffsetAndStride();
    }

    VertexBufferLayout::~VertexBufferLayout()
    {
    }

    void VertexBufferLayout::CalculateOffsetAndStride()
    {
        u32 offset = 0;
        for (auto& attribute : m_Attributes)
        {
            u32 size = Utils::GetSizeFromAttributeType(attribute.Type);
            attribute.Offset = offset;
            offset += size;
            m_Stride += size;
        }
    }

}

