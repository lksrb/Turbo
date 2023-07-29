#include "tbopch.h"
#include "VertexBuffer.h"

#include "Turbo/Platform/Vulkan/VulkanVertexBuffer.h"

namespace Turbo
{
    Ref<VertexBuffer> VertexBuffer::Create(const void* vertices, u64 size)
    {
        return Ref<VulkanVertexBuffer>::Create(vertices, size);
    }

    Ref<VertexBuffer> VertexBuffer::Create(u64 size)
    {
        return Ref<VulkanVertexBuffer>::Create(nullptr, size);
    }
}
