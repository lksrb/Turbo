#include "tbopch.h"
#include "IndexBuffer.h"

#include "Turbo/Platform/Vulkan/VulkanIndexBuffer.h"

namespace Turbo
{
    Ref<IndexBuffer> IndexBuffer::Create(const std::vector<u32>& indices)
    {
        return IndexBuffer::Create(indices.data(), (u32)indices.size());
    }

    Ref<IndexBuffer> IndexBuffer::Create(const u32* indices, u32 count)
    {
        return Ref<VulkanIndexBuffer>::Create(indices, count);
    }

}
